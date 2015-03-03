#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"
#include "PixelCalibrations/include/PixelFEDTBMDelayCalibration.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"
#include "PixelUtilities/PixelFEDDataTools/include/ErrorFIFODecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO1DigDecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO2Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO2DigDecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO3Decoder.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include <iomanip>

using namespace pos;

PixelFEDTBMDelayCalibration::PixelFEDTBMDelayCalibration(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr)
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  std::cout << "In PixelFEDTBMDelayCalibration copy ctor()" << std::endl;
}

void PixelFEDTBMDelayCalibration::initializeFED() {
  setFEDModeAndControlRegister(0x8, 0x30010);
  printIfSlinkHeaderMessedup_off();
}

xoap::MessageReference PixelFEDTBMDelayCalibration::beginCalibration(xoap::MessageReference msg) {
  std::cout << "In PixelFEDTBMDelayCalibration::beginCalibration()" << std::endl;

  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  std::string root_fn(outputDir() + "/TBMDelay.root");
  cout << "writing histograms to file " << root_fn << endl;
  rootf = new TFile(root_fn.c_str(), "create");
  assert(rootf->IsOpen());

  tempCalibObject->writeASCII(outputDir());

  DumpFIFOs = tempCalibObject->parameterValue("DumpFIFOs") == "yes";

  //  const std::vector<PixelROCName>& rocs = tempCalibObject->rocList();
  //PixelRootDirectoryMaker rootDirs(rocs, rootf);

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

  const TString sdecode[nDecode] = {
    "F11nTBMHeader", "F11nTBMHeaders", "F11nTBMTrailer", "F11nTBMTrailers", "F11nROCHeaders",
    "F17nTBMHeader", "F17nTBMHeaders", "F17nTBMTrailer", "F17nTBMTrailers", "F17nROCHeaders",
    "F3fifoErr", "F3wrongRoc", "F3wrongPix", "F3rightPix"
  };

  for (int idecode = 0; idecode < nDecode; ++idecode) {
    for (size_t i = 0; i < dacsToScan.size(); ++i) {
      const std::string& iname = dacsToScan[i];
      const TString itname(iname.c_str());
      const std::vector<unsigned>& ivals = tempCalibObject->scanValues(iname);
      const size_t ni = ivals.size();
      std::vector<double> ibins(ni+1);
      for (size_t k = 0; k < ni; ++k)
	ibins[k] = double(ivals[k]);
      ibins[ni] = ibins[ni-1] + (ibins[ni-1] - ibins[ni-2]);

      TH1F* h = new TH1F(itname + "_" + sdecode[idecode], sdecode[idecode] + ";" + itname + ";ntrig", ni, &ibins[0]);
      h->SetStats(0);
      scans1d[idecode].push_back(h);

      for (size_t j = i+1; j < dacsToScan.size(); ++j) {
	const std::string jname = dacsToScan[j];
	const TString jtname(jname.c_str());
	const std::vector<unsigned>& jvals = tempCalibObject->scanValues(jname);
	const size_t nj = jvals.size();
	std::vector<double> jbins(nj+1);
	for (size_t k = 0; k < nj; ++k)
	  jbins[k] = double(jvals[k]);
	jbins[nj] = jbins[nj-1] + (jbins[nj-1] - jbins[nj-2]);
      
	TH2F* h2 = new TH2F(jtname + "_v_" + itname + "_" + sdecode[idecode], sdecode[idecode] + ";" + itname + ";" + jtname, ni, &ibins[0], nj, &jbins[0]);
	h2->SetStats(0);
	scans2d[idecode].push_back(h2);

	for (size_t l = j+1; l < dacsToScan.size(); ++l) {
	  const std::string lname = dacsToScan[l];
	  const TString ltname(lname.c_str());
	  const std::vector<unsigned>& lvals = tempCalibObject->scanValues(lname);
	  const size_t nl = lvals.size();
	  std::vector<double> lbins(nl+1);
	  for (size_t k = 0; k < nl; ++k)
	    lbins[k] = double(lvals[k]);
	  lbins[nl] = lbins[nl-1] + (lbins[nl-1] - lbins[nl-2]);
      
	  TH3F* h3 = new TH3F(ltname + "_v_" + jtname + "_v_" + itname + "_" + sdecode[idecode], sdecode[idecode] + ";" + itname + ";" + jtname + ";" + ltname, ni, &ibins[0], nj, &jbins[0], nl, &lbins[0]);
	  h3->SetStats(0);
	  scans3d[idecode].push_back(h3);
	}
      }
    }
  }
      
  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDTBMDelayCalibration::execute(xoap::MessageReference msg) {
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
    cout << "ERROR: PixelFEDTBMDelayCalibration::execute() does not understand the WhatToDo command, "<< parameters[0].value_ <<", sent to it.\n";
    assert(0);
  }

  xoap::MessageReference reply = MakeSOAPMessageReference("FEDCalibrationsDone");
  return reply;
}

xoap::MessageReference PixelFEDTBMDelayCalibration::endCalibration(xoap::MessageReference msg) {
  std::cout << "In PixelFEDTBMDelayCalibration::endCalibration()" << std::endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

void PixelFEDTBMDelayCalibration::RetrieveData(unsigned state) {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  const std::vector<PixelROCName>& rocs = tempCalibObject->rocList();
  typedef std::set< std::pair<unsigned int, unsigned int> > colrow_t;
  const colrow_t colrows = tempCalibObject->pixelsWithHits(state);

  const std::vector<std::pair<unsigned, std::vector<unsigned> > >& fedsAndChannels = tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_);

  if (DumpFIFOs) std::cout << "NEW FEDTBMDelay TRIGGER " << event_ << " state " << state << " ";
  std::map<std::string, unsigned int> currentDACValues;
  for (unsigned dacnum = 0; dacnum < tempCalibObject->numberOfScanVariables(); ++dacnum) {
    const std::string& dacname = tempCalibObject->scanName(dacnum);
    const unsigned dacvalue = tempCalibObject->scanValue(tempCalibObject->scanName(dacnum), state);
    currentDACValues[dacname] = dacvalue;
    if (DumpFIFOs) std::cout << dacname << " " << dacvalue << " ";
  }
  if (DumpFIFOs) std::cout << std::endl;

  for (unsigned ifed = 0; ifed < fedsAndChannels.size(); ++ifed) {
    const unsigned fednumber = fedsAndChannels[ifed].first;
    const unsigned long vmeBaseAddress = theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
    PixelFEDInterface* iFED = FEDInterface_[vmeBaseAddress];

    const int MaxChips = 8;
    uint32_t buffer1[MaxChips][pos::fifo1TranspDepth];
    uint32_t buffer2[MaxChips][pos::fifo2Depth];
    uint64_t buffer3[pos::slinkDepth];
    uint32_t bufferErr[36*1024];
    int status2[MaxChips] = {0};
    for (int chip = 1; chip <= 7; chip += 2) {
      if (chip == 1 || chip == 7)
	iFED->drainDigTransFifo(chip, buffer1[chip]);
      status2[chip] = iFED->drainDataFifo2(chip, buffer2[chip]);
    }
    const int status3 = iFED->spySlink64(buffer3);
    const int statusErr = iFED->drainErrorFifo(bufferErr);

    FIFO1DigDecoder* decode1[MaxChips] = {0};
    FIFO2DigDecoder* decode2[MaxChips] = {0};
    for (int chip = 1; chip <= 7; chip += 2) {
      if (chip == 1 || chip == 7)
	decode1[chip] = new FIFO1DigDecoder(buffer1[chip]);
      decode2[chip] = new FIFO2DigDecoder(buffer2[chip], status2[chip]);
    }
    FIFO3Decoder* decode3 = 0;
    if (status3 > 0)
      decode3 = new FIFO3Decoder(buffer3);
    ErrorFIFODecoder decodeErr(bufferErr, statusErr);

    assert(F17nTBMHeader - F11nTBMHeader == 5);
    for (int FIFO1Chip = 0; FIFO1Chip < 2; ++FIFO1Chip) {
      const int chip = FIFO1Chip ? 7 : 1;
      const int arroff = FIFO1Chip ? 5 : 0;
      FillEm(state, F11nTBMHeader + arroff,
	     int(decode1[chip]->tbm_header_l[0].size() != 0) + 
	     int(decode1[chip]->tbm_header_l[1].size() != 0));
      FillEm(state, F11nTBMHeaders + arroff, 
	     decode1[chip]->tbm_header_l[0].size() + 
	     decode1[chip]->tbm_header_l[1].size());
      FillEm(state, F11nTBMTrailer + arroff,
	     int(decode1[chip]->tbm_trailer_l[0].size() != 0) + 
	     int(decode1[chip]->tbm_trailer_l[1].size() != 0));
      FillEm(state, F11nTBMTrailers + arroff,
	     decode1[chip]->tbm_trailer_l[0].size() + 
	     decode1[chip]->tbm_trailer_l[1].size());
      FillEm(state, F11nROCHeaders + arroff,
	     decode1[chip]->roc_header_l[0].size() + 
	     decode1[chip]->roc_header_l[1].size());
    }

    if (DumpFIFOs) {
      int col2=-1, row2=-1;
      std::cout << "FIFO 2 buffer sizes: ";
      for (int chip = 1; chip <= 7; chip += 2)
	std::cout << std::setw(4) << status2[chip] << " ";
      std::cout << endl;
      for (int chip = 1; chip <= 7; chip += 2) {
	if (chip == 1 || chip == 7) {
	  bool trans_all_ff = false;
	  int trans_found = 0;
	  uint32_t pattern = 0;
	  uint32_t* data  = buffer1[chip];
	  uint32_t* datae = data + 1023;
	  std::cout << "-------------------------------------------" << std::endl;
	  std::cout << "Contents of transparent FIFO 1 for chip = " << chip << std::endl;
	  std::cout << "-------------------------------------------" << std::endl;
	  if (*data == 0xffffffff && *datae == 0xffffffff) {
	    int nbeg = 0, nend = 0;
	    while (*data == 0xffffffff && data != datae)
	      ++nbeg, ++data;
	    if (data == datae) {
	      trans_all_ff = true;
	      std::cout << "all 0xFFFFFFFF" << std::endl;
	    }
	    else {
	      while (*datae == 0xffffffff)
		++nend, --datae;
	      trans_found = datae-data+1;
	      std::cout << nbeg << " 0xFFFFFFFF then " << trans_found << " words:" << std::endl;
	      while (data != datae + 1) {
		uint32_t d = *data;
		uint16_t h(d >> 16);
		uint16_t l(d & 0xFFFF);
		uint16_t ab[2] = {h, l};
		std::cout << std::hex << std::setw(4) << h << std::dec << " ";
		std::cout << std::hex << std::setw(4) << l << std::dec << "  ";
		for (int j = 0; j < 2; ++j) {
		  for (int i = 15; i >= 0; --i) {
		    std::cout << ((ab[j] & (1 << i)) ? "1" : "0");
		    if (i % 4 == 0) std::cout << " ";
		  }
		  std::cout << "  ";
		}
		std::cout << std::endl;
		++data;
	      }
	      std::cout << "then " << nend << " 0xFFFFFFFF" << std::endl;
	    }
	  }
	  else {
	    pattern = *((uint32_t*)data);
	    bool same = true;
	    while (data != datae + 1) {
	      uint32_t p = *((uint32_t*)data);
	      if (p != pattern)
		same = false;
	      data += 4;
	    }
	    if (same)
	      std::cout << "1024 repetitions of " << std::hex << pattern << std::dec << std::endl;
	    else {
	      uint8_t* data8 = (uint8_t*)buffer1[chip];
	      std::cout << "rw | ";
	      for (int j = 0; j < 64; ++j)
		std::cout << std::setw(2) << j << " ";
	      std::cout << std::endl;
	      for (int i = 0; i < 64; ++i) {
		std::cout << std::setw(2) << i << " | ";
		for (int j = 0; j < 64; ++j)
		  std::cout << std::hex << std::setw(2) << unsigned(data8[i*64+j]) << std::dec << " ";
		std::cout << std::endl;
	      }
	    }
	  }

	  std::cout << "FIFO1DigDecoder thinks:\n";
	  decode1[chip]->printToStream(std::cout);
	}
	//if (chip != 1 && chip != 7 && !trans_all_ff)
	//  std::cout << "bad trans_all_ff: chip is " << chip << std::endl;

	std::cout << "----------------------------------" << std::endl;
	if (status2[chip] < 0)
	  std::cout << "Spy FIFO 2 for chip = " << chip << " status = " << status2[chip] << std::endl;
	else {
	  std::cout << "Contents of Spy FIFO 2 for chip = " << chip << "(status2 = " << status2[chip] << ")" <<std::endl;
	  std::cout << "----------------------------------" << std::endl;
	  for (int i = 0; i <= status2[chip]; ++i) {
	    uint32_t d = buffer2[chip][i];
	    uint32_t dh = d & 0xf0;
	    if (dh == 0x70 || dh == 0x10 || dh == 0xc0)
	      std::cout << "\n";
	    if (d > 0xFF)
	      std::cout << "\nweird word: " << std::hex << d << "\n";
	    else 
	      std::cout << std::setw(2) << std::hex << d << std::dec << " ";
	  }
	  std::cout << "\n----------------------------------" << std::endl;
	}
	std::cout << "FIFO2DigDecoder thinks:\n";
	decode2[chip]->printToStream(std::cout);
	if (decode2[chip]->n_hits() > 6) {
	  col2 = decode2[chip]->hits()[0].col;
	  row2 = decode2[chip]->hits()[0].row;
	}
      }
      if (status2[1] > 0 && status2[3] > 0) {
	cout<<"decodePTrans return: " << decodePTrans(buffer2[1],buffer2[3],16)<<endl;
	cout<<"decodePTrans2 return: " << decodePTrans2(buffer2[1],buffer2[3],16)<<endl;
	cout << "decodePTrans3: " << endl;
	decodePTrans3(buffer2[1], buffer2[3], 64);
      }

      std::cout << "----------------------" << std::endl;
      std::cout << "Contents of Spy FIFO 3" << std::endl;
      std::cout << "----------------------" << std::endl;
      for (int i = 0; i <= status3; ++i)
	std::cout << "Clock " << std::setw(2) << i << " = 0x " << std::hex << std::setw(8) << (buffer3[i]>>32) << " " << std::setw(8) << (buffer3[i] & 0xFFFFFFFF) << std::dec << std::endl;
      if (status3 > 0) {
	std::cout << "FIFO3Decoder thinks:\n"
		  << "nhits: " << decode3->nhits() << "\n";
	for (unsigned i = 0; i < decode3->nhits(); ++i)
	  std::cout << "#" << i << ": ch: " << decode3->channel(i)
		    << " rocid: " << decode3->rocid(i) << " dcol: " << decode3->dcol(i)
		    << " pxl: " << decode3->pxl(i) << " pulseheight: " << decode3->pulseheight(i)
		    << " col: " << decode3->column(i) << " row: " << decode3->row(i) << std::endl;
	std::cout << "(fifo2 col: " << col2 << " row: " << row2 << "   fifo3 dcol: " << decode3->dcol(0) << " pxl: " << decode3->pxl(0) << " col: " << decode3->column(0) << " row: " << decode3->row(0) << ")\n";
      }
      std::cout << "Contents of Error FIFO" << std::endl;
      std::cout << "----------------------" << std::endl;
      for (int i = 0; i <= statusErr; ++i)
	std::cout << "Clock " << i << " = 0x" << std::hex << bufferErr[i] << std::dec << std::endl;
      std::cout << "ErrorFIFODecoder thinks:\n";
      decodeErr.printToStream(std::cout);
    }

    if (status3 <= 0) {
      //std::cout << "ERROR reading a fifo on FED # " << fednumber << " in crate # " << crate_ << ": status3 = " << status3 << endl;
      FillEm(state, F3fifoErr, 1);
      //std::cout << "readDigFEDStatus(): ";
      //iFED->readDigFEDStatus(false);
    }
    else {
      unsigned nskip = 0;
      for (unsigned ihit = 0; ihit < decode3->nhits(); ++ihit) {
	const unsigned channel = decode3->channel(ihit);
	const unsigned rocid = decode3->rocid(ihit);
	assert(rocid > 0);

	const PixelROCName& roc = theNameTranslation_->ROCNameFromFEDChannelROC(fednumber, channel, rocid-1);

	// Skip if this ROC is not on the list of ROCs to calibrate.
	// Also skip if we're in singleROC mode, and this ROC is not being calibrated right now.
	vector<PixelROCName>::const_iterator foundROC = find(rocs.begin(), rocs.end(), roc);
	if (foundROC == rocs.end()) { // || !tempCalibObject->scanningROCForState(roc, state)) {
	  FillEm(state, F3wrongRoc, 1);
	  ++nskip;
	}
	else {
	  const unsigned col = decode3->column(ihit);
	  const unsigned row = decode3->row(ihit);
      
	  if (colrows.find(std::make_pair(col, row)) == colrows.end())
	    FillEm(state, F3wrongPix, 1);
	  else
	    FillEm(state, F3rightPix, 1);
	}
      }
    }
  }
}

void PixelFEDTBMDelayCalibration::Analyze() {
  rootf->Write();
  rootf->Close();
}

void PixelFEDTBMDelayCalibration::FillEm(unsigned state, int which, float c) {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  int k2 = 0, k3 = 0;
  for (size_t i = 0; i < dacsToScan.size(); ++i) {
    const std::string& iname = dacsToScan[i];
    const double ival(tempCalibObject->scanValue(iname, state));
    scans1d[which][i]->Fill(ival, c);
    
    for (size_t j = i+1; j < dacsToScan.size(); ++j, ++k2) {
      const std::string jname = dacsToScan[j];
      const double jval(tempCalibObject->scanValue(jname, state));
      scans2d[which][k2]->Fill(ival, jval, c);

      for (size_t l = j+1; l < dacsToScan.size(); ++l, ++k3) {
	const std::string lname = dacsToScan[l];
	const double lval(tempCalibObject->scanValue(lname, state));
	scans3d[which][k3]->Fill(ival, jval, lval, c);
      }
    }
  }
}
