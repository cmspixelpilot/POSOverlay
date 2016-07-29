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

  const std::string OverrideFifo1Channel_str = tempCalibObject->parameterValue("OverrideFifo1Channel");
  int OverrideFifo1Channel = -1;
  if (OverrideFifo1Channel_str.size()) {
    OverrideFifo1Channel = strtoul(OverrideFifo1Channel_str.c_str(), 0, 10);
    if (OverrideFifo1Channel < 0 || OverrideFifo1Channel > 23) {
      std::cout << "OverrideFifo1Channel " << OverrideFifo1Channel << " not understood" << std::endl;
      assert(0);
    }
  }

  typedef std::set< std::pair<unsigned int, unsigned int> > colrow_t;
  const colrow_t colrows = tempCalibObject->pixelsWithHits(0);
  if (colrows.size() != 1) {
    std::cout << "must use exactly one pixel for score scan!\n";
    assert(0);
  }
  const int col = colrows.begin()->first;
  const int row = colrows.begin()->second;
  const int dc = col/2;
  const int pxl = 160 - 2*row + col%2;
  printf("col %i row %i -> dc %i pxl %i\n", col, row, dc, pxl);

  const std::vector<std::pair<unsigned, std::vector<unsigned> > >& fedsAndChannels = tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_);
  bool all_feds_ph1 = true;
  assert(fedsAndChannels.size() == 1); // JMTBAD need to book separate histograms per fed
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
      if (OverrideFifo1Channel >= 0)
        f->setChannelOfInterest(OverrideFifo1Channel);
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
    if (dacvals.size() > 1)
      dacsToScan.push_back(dacname);
  }

  if (dacsToScan.empty() && tempCalibObject->parameterValue("NoScanOK") != "yes") {
    cout << "no dacs in scan?" << endl;
    assert(0);
  }

  if (dacsToScan.size() < 3)
    BookEm("");

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
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

  xoap::MessageReference reply = MakeSOAPMessageReference("FEDCalibrationsDone");
  return reply;
}

xoap::MessageReference PixelFEDTBMDelayCalibrationWithScores::endCalibration(xoap::MessageReference msg) {
  std::cout << "In PixelFEDTBMDelayCalibrationWithScores::endCalibration()" << std::endl;
  timer.printStats();
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

void PixelFEDTBMDelayCalibrationWithScores::RetrieveData(unsigned state) {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  if (Dumps) std::cout << "NEW FEDTBMDelay TRIGGER " << event_ << " state " << state << " ";

  typedef std::set< std::pair<unsigned int, unsigned int> > colrow_t;
  const colrow_t colrows = tempCalibObject->pixelsWithHits(state);
  if (colrows.size() != 1) {
    std::cout << "must use exactly one pixel for score scan!\n";
    assert(0);
  }
  if (Dumps) {
    std::cout << "Expected hits: ";
    for (colrow_t::const_iterator cr = colrows.begin(); cr != colrows.end(); ++cr)
      std::cout << "c " << cr->first << " r " << cr->second << " ";
    std::cout << std::endl;
  }

  const std::vector<std::pair<unsigned, std::vector<unsigned> > >& fedsAndChannels = tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_);

  std::map<std::string, unsigned int> currentDACValues;
  for (unsigned dacnum = 0; dacnum < tempCalibObject->numberOfScanVariables(); ++dacnum) {
    const std::string& dacname = tempCalibObject->scanName(dacnum);
    const unsigned dacvalue = tempCalibObject->scanValue(tempCalibObject->scanName(dacnum), state);
    currentDACValues[dacname] = dacvalue;
    if (Dumps) std::cout << dacname << " " << dacvalue << " ";
  }
  if (Dumps) std::cout << std::endl;

  // for scan over ADelay, BDelay, and PLL, we split the output in multiple files so one long scan dying doesn't lose all the data
  if (dacsToScan.size() >= 3 && currentDACValues["TBMPLL"] != lastTBMPLL) {
    lastTBMPLL = currentDACValues["TBMPLL"];
    BookEm(TString::Format("TBMPLL%03i", lastTBMPLL));
  }

  //////

  timer.start();

  for (unsigned ifed = 0; ifed < fedsAndChannels.size(); ++ifed) {
    const unsigned fednumber = fedsAndChannels[ifed].first;
    const unsigned long vmeBaseAddress = theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
    PixelPh1FEDInterface* fed = dynamic_cast<PixelPh1FEDInterface*>(FEDInterface_[vmeBaseAddress]);
    const PixelFEDCard& fedcard = FEDInterface_[vmeBaseAddress]->getPixelFEDCard();
    const int F1fiber = fedcard.TransScopeCh + 1;
    std::vector<int> fibers_OK(25, 0);

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
      const int which_score  = nFib01ScoresOK  + fiber-1;
      const int which_scoreA = nFib01AScoresOK + fiber-1;
      const int which_scoreB = nFib01BScoresOK + fiber-1;
      const bool fiber_OK = scoreA == perfect_score && scoreB == perfect_score;
      if (fiber_OK) fibers_OK[fiber] = 1;
      FillEm(state, which_scoreA, scoreA == perfect_score);
      FillEm(state, which_scoreB, scoreB == perfect_score);
      FillEm(state, which_score, fiber_OK);
    }

    // also look at the one fiber in fifo1 for a cross check

    if (Dumps) {
      fed->readTransparentFIFO();
      fed->readSpyFIFO();
    }

    PixelPh1FEDInterface::digfifo1 d = fed->readFIFO1(Dumps);
    if (Dumps) printf("n tbm h a: %i b: %i  tbm t a: %i b: %i  roc h a: %i b: %i\n", d.a.n_tbm_h, d.b.n_tbm_h, d.a.n_tbm_t, d.b.n_tbm_t, d.a.n_roc_h, d.b.n_roc_h);

    FillEm(state, nF1TBMHeaders,  d.a.n_tbm_h + d.b.n_tbm_h);
    FillEm(state, nF1TBMTrailers, d.a.n_tbm_t + d.b.n_tbm_t);
    FillEm(state, nF1ROCHeaders,  d.a.n_roc_h + d.b.n_roc_h);
    FillEm(state, nF1Hits,        d.a.hits.size() + d.b.hits.size());

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

    FillEm(state, nF1CorrectHits, correct[0] + correct[1]);
    FillEm(state, nF1WrongHits,   wrong[0] + wrong[1]);
      
    FillEm(state, nF1TBMAHeaders,  d.a.n_tbm_h);
    FillEm(state, nF1TBMATrailers, d.a.n_tbm_t);
    FillEm(state, nF1ROCAHeaders,  d.a.n_roc_h);
    FillEm(state, nF1AHits,        d.a.hits.size());
    FillEm(state, nF1ACorrectHits, correct[0]);
    FillEm(state, nF1AWrongHits,   wrong[0]);

    FillEm(state, nF1TBMBHeaders,  d.b.n_tbm_h);
    FillEm(state, nF1TBMBTrailers, d.b.n_tbm_t);
    FillEm(state, nF1ROCBHeaders,  d.b.n_roc_h);
    FillEm(state, nF1BHits,        d.b.hits.size());
    FillEm(state, nF1BCorrectHits, correct[1]);
    FillEm(state, nF1BWrongHits,   wrong[1]);

    const int shouldbe = 8 * int(colrows.size());
    const bool F1OK =
      d.a.n_tbm_h == 1 && d.a.n_tbm_t == 1 && d.a.n_roc_h == 8 &&
      d.b.n_tbm_h == 1 && d.b.n_tbm_t == 1 && d.b.n_roc_h == 8 &&
      correct[0] == shouldbe &&
      correct[1] == shouldbe &&
      wrong[0] == 0 &&
      wrong[1] == 0;

    FillEm(state, nF1OK, F1OK);

    if (Dumps) {
      printf("correct out of %i: a: %i b: %i   wrong: a: %i b: %i  F1OK? %i thatscore? %i agree? %i\n", shouldbe, correct[0], correct[1], wrong[0], wrong[1], F1OK, fibers_OK[F1fiber], F1OK == fibers_OK[F1fiber]);
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
  CloseRootf();
}

void PixelFEDTBMDelayCalibrationWithScores::CloseRootf() {
  if (rootf) {
    rootf->Write();
    rootf->Close();
    delete rootf;
  }
}

void PixelFEDTBMDelayCalibrationWithScores::BookEm(const TString& path) {
  TString root_fn;
  if (path == "")
    root_fn.Form("%s/TBMDelay.root", outputDir().c_str());
  else
    root_fn.Form("%s/TBMDelay_%s.root", outputDir().c_str(), path.Data());
  cout << "writing histograms to file " << root_fn << endl;
  CloseRootf();
  rootf = new TFile(root_fn, "create");
  assert(rootf->IsOpen());

  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  static const TString sdecode[nDecode] = {
    "nF1OK",
    "nF1TBMHeaders", "nF1TBMTrailers", "nF1ROCHeaders", "nF1Hits", "nF1CorrectHits", "nF1WrongHits",
    "nF1TBMAHeaders", "nF1TBMATrailers", "nF1ROCAHeaders", "nF1AHits", "nF1ACorrectHits", "nF1AWrongHits",
    "nF1TBMBHeaders", "nF1TBMBTrailers", "nF1ROCBHeaders", "nF1BHits", "nF1BCorrectHits", "nF1BWrongHits",
    "nFib01ScoresOK", "nFib02ScoresOK", "nFib03ScoresOK", "nFib04ScoresOK",
    "nFib05ScoresOK", "nFib06ScoresOK", "nFib07ScoresOK", "nFib08ScoresOK",
    "nFib09ScoresOK", "nFib10ScoresOK", "nFib11ScoresOK", "nFib12ScoresOK",
    "nFib13ScoresOK", "nFib14ScoresOK", "nFib15ScoresOK", "nFib16ScoresOK",
    "nFib17ScoresOK", "nFib18ScoresOK", "nFib19ScoresOK", "nFib20ScoresOK",
    "nFib21ScoresOK", "nFib22ScoresOK", "nFib23ScoresOK", "nFib24ScoresOK",
    "nFib01AScoresOK", "nFib02AScoresOK", "nFib03AScoresOK", "nFib04AScoresOK",
    "nFib05AScoresOK", "nFib06AScoresOK", "nFib07AScoresOK", "nFib08AScoresOK",
    "nFib09AScoresOK", "nFib10AScoresOK", "nFib11AScoresOK", "nFib12AScoresOK",
    "nFib13AScoresOK", "nFib14AScoresOK", "nFib15AScoresOK", "nFib16AScoresOK",
    "nFib17AScoresOK", "nFib18AScoresOK", "nFib19AScoresOK", "nFib20AScoresOK",
    "nFib21AScoresOK", "nFib22AScoresOK", "nFib23AScoresOK", "nFib24AScoresOK",
    "nFib01BScoresOK", "nFib02BScoresOK", "nFib03BScoresOK", "nFib04BScoresOK",
    "nFib05BScoresOK", "nFib06BScoresOK", "nFib07BScoresOK", "nFib08BScoresOK",
    "nFib09BScoresOK", "nFib10BScoresOK", "nFib11BScoresOK", "nFib12BScoresOK",
    "nFib13BScoresOK", "nFib14BScoresOK", "nFib15BScoresOK", "nFib16BScoresOK",
    "nFib17BScoresOK", "nFib18BScoresOK", "nFib19BScoresOK", "nFib20BScoresOK",
    "nFib21BScoresOK", "nFib22BScoresOK", "nFib23BScoresOK", "nFib24BScoresOK",
  };

  for (int idecode = 0; idecode < nDecode; ++idecode) {
    scans1d[idecode].clear();
    scans2d[idecode].clear();

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
	TH1F* h = new TH1F(itname + "_" + sdecode[idecode], sdecode[idecode] + ";" + itname + ";ntrig", ni, &ibins[0]);
	h->SetStats(0);
	scans1d[idecode].push_back(h);
      }

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

	TH2F* h2 = new TH2F(jtname + "_v_" + itname + "_" + sdecode[idecode], sdecode[idecode] + ";" + itname + ";" + jtname, ni, &ibins[0], nj, &jbins[0]);
	h2->SetStats(0);
	scans2d[idecode].push_back(h2);
      }
    }
  }
}

void PixelFEDTBMDelayCalibrationWithScores::FillEm(unsigned state, int which, float c) {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  int k2 = 0;
  for (size_t i = 0; i < dacsToScan.size(); ++i) {
    const std::string& iname = dacsToScan[i];
    const double ival(tempCalibObject->scanValue(iname, state));

    if (dacsToScan.size() == 1)
      scans1d[which][i]->Fill(ival, c);
    
    for (size_t j = i+1; j < dacsToScan.size(); ++j, ++k2) {
      const std::string jname = dacsToScan[j];
      const double jval(tempCalibObject->scanValue(jname, state));
      scans2d[which][k2]->Fill(ival, jval, c);
    }
  }
}

