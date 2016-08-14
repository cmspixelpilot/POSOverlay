#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"
#include "PixelCalibrations/include/PixelFEDTBMDelayCalibrationWithScores.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "PixelFEDInterface/include/PixelPh1FEDInterface.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include <iomanip>

using namespace pos;

PixelFEDTBMDelayCalibrationWithScores::PixelFEDTBMDelayCalibrationWithScores(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr)
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr), rootf(0)
{
}

void PixelFEDTBMDelayCalibrationWithScores::initializeFED() {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  DoFifo1 = tempCalibObject->parameterValue("DoFifo1") == "yes";

  const std::string OverrideFifo1Fiber_str = tempCalibObject->parameterValue("OverrideFifo1Fiber");
  OverrideFifo1Fiber = -1;
  if (OverrideFifo1Fiber_str.size()) {
    OverrideFifo1Fiber = strtoul(OverrideFifo1Fiber_str.c_str(), 0, 10);
    if (OverrideFifo1Fiber < 0 || OverrideFifo1Fiber > 23) {
      std::cout << "OverrideFifo1Fiber " << OverrideFifo1Fiber << " not understood" << std::endl;
      assert(0);
    }
  }

  typedef std::set< std::pair<unsigned int, unsigned int> > colrow_t;
  const colrow_t colrows = tempCalibObject->pixelsWithHits(0);
  if (colrows.size() != 1) {
    std::cout << "must use exactly one pixel for score scan!\n";
    assert(0);
  }
  the_col = int(colrows.begin()->first);
  the_row = int(colrows.begin()->second);
  const int dc = the_col/2;
  const int pxl = 160 - 2*the_row + the_col%2;
  std::cout << "Expected hit in col " << the_col << " row " << the_row << " -> dc " << dc << " pxl " << pxl << "\n";

  const std::vector<std::pair<unsigned, std::vector<unsigned> > >& fedsAndChannels = tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_);
  bool all_feds_ph1 = true;
  for (unsigned ifed = 0; ifed < fedsAndChannels.size(); ++ifed) {
    const unsigned fednumber = fedsAndChannels[ifed].first;
    const unsigned long vmeBaseAddress = theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
    PixelFEDInterfaceBase* iFED = FEDInterface_[vmeBaseAddress];
    PixelPh1FEDInterface* f = dynamic_cast<PixelPh1FEDInterface*>(iFED);
    if (f == 0) {
      all_feds_ph1 = false;
      std::cout << "FED with VME base addr 0x" << std::hex << vmeBaseAddress << std::dec << " isn't a Ph1 FED!\n";
    }
    else {
      f->disableBE(true);
      f->setPixelForScore(dc, pxl);
      if (OverrideFifo1Fiber >= 0)
        f->setChannelOfInterest(OverrideFifo1Fiber);
    }
  }
  assert(all_feds_ph1);
}

xoap::MessageReference PixelFEDTBMDelayCalibrationWithScores::beginCalibration(xoap::MessageReference msg) {
  std::cout << "In PixelFEDTBMDelayCalibrationWithScores::beginCalibration()" << std::endl;

  timer.setName("FED readout");

  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  tempCalibObject->writeASCII(outputDir());

  Dumps = tempCalibObject->parameterValue("Dumps") == "yes";

  for (unsigned dacnum = 0; dacnum < tempCalibObject->numberOfScanVariables(); ++dacnum) {
    const std::string& dacname = tempCalibObject->scanName(dacnum);
    std::vector<unsigned int> dacvals = tempCalibObject->scanValues(dacname);
    if (dacvals.size() > 1) {
      dacsToScan.push_back(dacname);
      assert(dacname == "TBMPLL" || dacname == "TBMADelay" || dacname == "TBMBDelay");
    }
  }

  if (dacsToScan.empty() && tempCalibObject->parameterValue("NoScanOK") != "yes") {
    cout << "no dacs in scan?" << endl;
    assert(0);
  }

  rootf = new TFile(TString::Format("%s/TBMDelay.root", outputDir().c_str()), "create");
  assert(rootf->IsOpen());

  return MakeSOAPMessageReference("BeginCalibrationDone");
}

xoap::MessageReference PixelFEDTBMDelayCalibrationWithScores::execute(xoap::MessageReference msg) {
  Attribute_Vector parameters(2);
  parameters[0].name_ = "WhatToDo";
  parameters[1].name_ = "StateNum";
  Receive(msg, parameters);

  const unsigned state = atoi(parameters[1].value_.c_str());

  if (parameters[0].value_ == "RetrieveData")
    RetrieveData(state);
  else if (parameters[0].value_ == "Analyze")
    Analyze();
  else {
    cout << "ERROR: PixelFEDTBMDelayCalibrationWithScores::execute() does not understand the WhatToDo command, "<< parameters[0].value_ <<", sent to it.\n";
    assert(0);
  }

  return MakeSOAPMessageReference("FEDCalibrationsDone");
}

xoap::MessageReference PixelFEDTBMDelayCalibrationWithScores::endCalibration(xoap::MessageReference msg) {
  std::cout << "In PixelFEDTBMDelayCalibrationWithScores::endCalibration()" << std::endl;
  timer.printStats();

  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);

  typedef std::vector<std::pair<unsigned, std::vector<unsigned> > > duh_t;
  const duh_t& fedsAndChannels = tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_);
  
  for (duh_t::const_iterator fednumber_channels = fedsAndChannels.begin(); fednumber_channels != fedsAndChannels.end(); ++fednumber_channels) {
    const unsigned fednumber = fednumber_channels->first;
    for (std::vector<unsigned>::const_iterator fedchannel = fednumber_channels->second.begin(); fedchannel != fednumber_channels->second.end(); ++fedchannel) {
      if (*fedchannel % 2 != 0)
        continue;

      const int fiber = *fedchannel / 2;
      const PixelChannel& pxch = theNameTranslation_->ChannelFromFEDChannel(fednumber, *fedchannel);
      const PixelModuleName& mod = pxch.module();
        
      PixelTBMSettings* tbm = 0;
      PixelConfigInterface::get(tbm, "pixel/tbm/" + mod.modulename(), *theGlobalKey_);
      assert(tbm != 0);

      std::map<std::string, unsigned> old_vals;
      old_vals["TBMPLL"] = tbm->getTBMPLLDelay();
      old_vals["TBMADelay"] = tbm->getTBMADelay();
      old_vals["TBMBDelay"] = tbm->getTBMBDelay();
      std::cout << "TBM settings for " << mod << " ADelay: " << int(tbm->getTBMADelay()) << " BDelay: " << int(tbm->getTBMBDelay()) << " PLL: " << int(tbm->getTBMPLLDelay()) << std::endl;

      if (dacsToScan.size() == 1 && dacsToScan[0] == "TBMPLL") {
        TH1F* h = dynamic_cast<TH1F*>(scans[Key(fednumber, -fiber, "ScoreOK")][0]);
        int best_v = -1;
        int best_dist = 1000000;
        for (int ibin = 1; ibin <= h->GetNbinsX(); ++ibin) {
          const unsigned c = round(h->GetBinContent(ibin));
          if (c != tempCalibObject->nTriggersPerPattern()) // could not require perfection I guess 
            continue;
          const int v = h->GetXaxis()->GetBinLowEdge(ibin);
          int dist = 0;
          if (dacsToScan[0] == "TBMPLL")
            dist = abs(int(old_vals["TBMPLL"] >> 5) - (v>>5)) + abs(int((old_vals["TBMPLL"]&0x1c)>>2) - ((v&0x1c)>>2));
          else
            assert(0);

          if (dist < best_dist) {
            std::cout << "new best dist " << dist << " at " << v << std::endl;
            best_v = v;
            best_dist = dist;
          }
        }

        if (best_dist != 1000000) {
          std::cout << "-> ";
          if (best_dist != 0) std::cout << " NEW ";
          std::cout << " TBMPLL value " << best_v << std::endl;
          assert(best_v >=0 && best_v <= 255);
          tbm->setTBMPLLDelay((unsigned char)(best_v));
        }
        else
          std::cout << "no new TBMPLL value" << std::endl;
      }
      else {
        std::cout << "not implemented for n-d scan\n";
      }

      tbm->writeASCII(outputDir());
      delete tbm; // we own it?
    }
  }

  return MakeSOAPMessageReference("EndCalibrationDone");
}

void PixelFEDTBMDelayCalibrationWithScores::RetrieveData(unsigned state) {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  typedef std::set< std::pair<unsigned int, unsigned int> > colrow_t;
  const colrow_t colrows = tempCalibObject->pixelsWithHits(state);
  if (colrows.size() != 1) { // checked in initialize fed but need to keep checking 
    std::cout << "must use exactly one pixel for score scan!\n";
    assert(0);
  }
  else if (int(colrows.begin()->first) != the_col || int(colrows.begin()->second) != the_row) {
    std::cout << "must use same pixel always for score scan!\n"; // JMTBAD or we could re-set the pixel in the fed
    assert(0);
  }

  if (Dumps) {
    std::cout << "New FEDTBMDelay event " << event_ << " state " << state << " "; 
    for (unsigned dacnum = 0; dacnum < tempCalibObject->numberOfScanVariables(); ++dacnum) {
      const std::string& dacname = tempCalibObject->scanName(dacnum);
      const unsigned dacvalue = tempCalibObject->scanValue(tempCalibObject->scanName(dacnum), state);
      std::cout << dacname << " -> " << dacvalue << " ";
    }
    std::cout << std::endl;
  }

  const std::vector<std::pair<unsigned, std::vector<unsigned> > >& fedsAndChannels = tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_);

  //////

  timer.start();

  for (unsigned ifed = 0; ifed < fedsAndChannels.size(); ++ifed) {
    const unsigned fednumber = fedsAndChannels[ifed].first;
    if (Dumps) std::cout << "FED NUMBER " << fednumber << "\n";
    const unsigned long vmeBaseAddress = theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
    PixelPh1FEDInterface* fed = dynamic_cast<PixelPh1FEDInterface*>(FEDInterface_[vmeBaseAddress]);
    const PixelFEDCard& fedcard = FEDInterface_[vmeBaseAddress]->getPixelFEDCard();
    const int F1fiber = OverrideFifo1Fiber != -1 ? OverrideFifo1Fiber : fedcard.TransScopeCh + 1;
    std::vector<int> fibers_OK(25, 0);
    Key key(fednumber);

    for (int fiber = 1; fiber <= 24; ++fiber) {

      const int chA = fiber * 2 - 1;
      const int chB = fiber * 2;

      const uint32_t scoreA = fed->getScore(chA);
      const uint32_t scoreB = fed->getScore(chB);

      if (Dumps) {
        const uint32_t chs[2] = {chA, chB};
        const uint32_t scores[2] = {scoreA, scoreB};
        std::cout << "fiber " << fiber << " scores:\n";
        for (size_t i = 0; i < 2; ++i)
          std::cout << "ch " << std::setw(2) << chs[i] << ": wc DDDDDDDD rrrrrrrr RRRRRRRR TH\n"
                    << "       " << std::setw(2) << (scores[i] >> 26) << " "
                    << std::bitset<8>((scores[i] >> 18) & 0xFF) << " "
                    << std::bitset<8>((scores[i] >> 10) & 0xFF) << " "
                    << std::bitset<8>((scores[i] >>  2) & 0xFF) << " "
                    << std::bitset<2>(scores[i] & 0x3)
                    << "\n";
      }

      const uint32_t perfect_score = 0x4bfc03ff; // want 18 words, 8 hits, 8 roc headers, tbm header, trailer
      const bool fiber_OK = scoreA == perfect_score && scoreB == perfect_score;
      if (fiber_OK) fibers_OK[fiber] = 1;
      FillEm(state, key(chA,    "ScoreOK"), scoreA == perfect_score);
      FillEm(state, key(chB,    "ScoreOK"), scoreB == perfect_score);
      FillEm(state, key(-fiber, "ScoreOK"), fiber_OK);
    }

    // also look at the one fiber in fifo1 for a cross check

    if (DoFifo1) {
      const int F1chA = F1fiber*2 - 1;
      const int F1chB = F1fiber*2;

      if (Dumps) {
        fed->readTransparentFIFO();
        fed->readSpyFIFO();
      }

      PixelPh1FEDInterface::digfifo1 d = fed->readFIFO1(Dumps);
      if (Dumps) printf("n tbm h a: %i b: %i  tbm t a: %i b: %i  roc h a: %i b: %i\n", d.a.n_tbm_h, d.b.n_tbm_h, d.a.n_tbm_t, d.b.n_tbm_t, d.a.n_roc_h, d.b.n_roc_h);

      FillEm(state, key(-F1fiber, "nF1TBMHeaders"),  d.a.n_tbm_h + d.b.n_tbm_h);
      FillEm(state, key(-F1fiber, "nF1TBMTrailers"), d.a.n_tbm_t + d.b.n_tbm_t);
      FillEm(state, key(-F1fiber, "nF1ROCHeaders"),  d.a.n_roc_h + d.b.n_roc_h);
      FillEm(state, key(-F1fiber, "nF1Hits"),        d.a.hits.size() + d.b.hits.size());

      int correct[2] = {0};
      int wrong[2] = {0};
      for (int aorb = 0; aorb < 2; ++aorb) {
        const PixelPh1FEDInterface::encfifo1& z = d.aorb(aorb);
        for (size_t i = 0; i < z.hits.size(); ++i) {
          const PixelPh1FEDInterface::encfifo1hit& h = z.hits[i];
          if (colrows.find(std::make_pair(h.col, h.row)) != colrows.end())
            ++correct[aorb];
        }
      }
      wrong[0] = d.a.hits.size() - correct[0];
      wrong[1] = d.b.hits.size() - correct[1];

      FillEm(state, key(-F1fiber, "nF1CorrectHits"), correct[0] + correct[1]);
      FillEm(state, key(-F1fiber, "nF1WrongHits"),   wrong[0] + wrong[1]);
      
      FillEm(state, key(F1chA, "nF1TBMAHeaders"),  d.a.n_tbm_h);
      FillEm(state, key(F1chA, "nF1TBMATrailers"), d.a.n_tbm_t);
      FillEm(state, key(F1chA, "nF1ROCAHeaders"),  d.a.n_roc_h);
      FillEm(state, key(F1chA, "nF1AHits"),        d.a.hits.size());
      FillEm(state, key(F1chA, "nF1ACorrectHits"), correct[0]);
      FillEm(state, key(F1chA, "nF1AWrongHits"),   wrong[0]);

      FillEm(state, key(F1chB, "nF1TBMBHeaders"),  d.b.n_tbm_h);
      FillEm(state, key(F1chB, "nF1TBMBTrailers"), d.b.n_tbm_t);
      FillEm(state, key(F1chB, "nF1ROCBHeaders"),  d.b.n_roc_h);
      FillEm(state, key(F1chB, "nF1BHits"),        d.b.hits.size());
      FillEm(state, key(F1chB, "nF1BCorrectHits"), correct[1]);
      FillEm(state, key(F1chB, "nF1BWrongHits"),   wrong[1]);

      const int shouldbe = 8 * int(colrows.size());
      const bool isF1OK =
        d.a.n_tbm_h == 1 && d.a.n_tbm_t == 1 && d.a.n_roc_h == 8 &&
        d.b.n_tbm_h == 1 && d.b.n_tbm_t == 1 && d.b.n_roc_h == 8 &&
        correct[0] == shouldbe &&
        correct[1] == shouldbe &&
        wrong[0] == 0 &&
        wrong[1] == 0;

      FillEm(state, key(-F1fiber, "F1OK"), isF1OK);

      if (Dumps) printf("correct out of %i: a: %i b: %i   wrong: a: %i b: %i  F1OK? %i thatscore? %i agree? %i\n", shouldbe, correct[0], correct[1], wrong[0], wrong[1], isF1OK, fibers_OK[F1fiber], isF1OK == fibers_OK[F1fiber]);
    }

    if (Dumps) {
      printf("fibers OK by score:");
      for (int fiber = 1; fiber <= 24; ++fiber)
        if (fibers_OK[fiber])
          printf(" %i", fiber);
      printf("\n");
    }
  }

  timer.stop();
}

void PixelFEDTBMDelayCalibrationWithScores::Analyze() {
  if (rootf) {
    rootf->Write();
    rootf->Close();
    delete rootf;
  }
}

void PixelFEDTBMDelayCalibrationWithScores::BookEm(const Key& key) {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  TString keyname = key.name();

  for (size_t i = 0; i < dacsToScan.size(); ++i) {
    const std::string& iname = dacsToScan[i];
    const TString itname(iname.c_str());
    std::vector<unsigned> ivals = tempCalibObject->scanValues(iname);
    std::sort(ivals.begin(), ivals.end());
    const size_t ni = ivals.size();
    std::vector<double> ibins(ni+1);
    for (size_t k = 0; k < ni; ++k)
      ibins[k] = double(ivals[k]);
    ibins[ni] = ibins[ni-1] + (ibins[ni-1] - ibins[ni-2]);

    if (dacsToScan.size() == 1) {
      TH1F* h = new TH1F(keyname + "_" + itname, keyname + ";" + itname + ";", ni, &ibins[0]);
      h->SetStats(0);
      scans[key].push_back(h);
    }
    else
      for (size_t j = i+1; j < dacsToScan.size(); ++j) {
        const std::string jname = dacsToScan[j];
        const TString jtname(jname.c_str());
        std::vector<unsigned> jvals = tempCalibObject->scanValues(jname);
        std::sort(jvals.begin(), jvals.end());
        const size_t nj = jvals.size();
        std::vector<double> jbins(nj+1);
        for (size_t k = 0; k < nj; ++k)
          jbins[k] = double(jvals[k]);
        jbins[nj] = jbins[nj-1] + (jbins[nj-1] - jbins[nj-2]);

        TH2F* h2 = new TH2F(keyname + "_" + jtname + "V" + itname, keyname + ";" + itname + ";" + jtname, ni, &ibins[0], nj, &jbins[0]);
        h2->SetStats(0);
        scans[key].push_back(h2);
      }
  }
}

void PixelFEDTBMDelayCalibrationWithScores::FillEm(unsigned state, const Key& key, float c) {
  if (scans.find(key) == scans.end())
    BookEm(key);

  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  int k2 = 0;
  for (size_t i = 0; i < dacsToScan.size(); ++i) {
    const std::string& iname = dacsToScan[i];
    const double ival(tempCalibObject->scanValue(iname, state));

    if (dacsToScan.size() == 1)
      scans[key][i]->Fill(ival, c);
    else 
      for (size_t j = i+1; j < dacsToScan.size(); ++j, ++k2) {
        const std::string jname = dacsToScan[j];
        const double jval(tempCalibObject->scanValue(jname, state));
        dynamic_cast<TH2F*>(scans[key][k2])->Fill(ival, jval, c);
      }
  }
}
