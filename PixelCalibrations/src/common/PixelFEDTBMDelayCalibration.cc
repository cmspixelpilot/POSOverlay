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
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr), rootf(0)
{
  std::cout << "In PixelFEDTBMDelayCalibration copy ctor()" << std::endl;
}

void PixelFEDTBMDelayCalibration::initializeFED() {
  setFEDModeAndControlRegister(0x8, 0x30010);
  printIfSlinkHeaderMessedup_off();
  sendResets();
}

xoap::MessageReference PixelFEDTBMDelayCalibration::beginCalibration(xoap::MessageReference msg) {
  std::cout << "In PixelFEDTBMDelayCalibration::beginCalibration()" << std::endl;

  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  tempCalibObject->writeASCII(outputDir());

  DumpFIFOs = tempCalibObject->parameterValue("DumpFIFOs") == "yes";
  PrintHits = tempCalibObject->parameterValue("PrintHits") == "yes";

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

  if (dacsToScan.size() < 3)
    BookEm("");

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
  if (PrintHits) {
    std::cout << "ZZ ";
    for (colrow_t::const_iterator cr = colrows.begin(); cr != colrows.end(); ++cr)
      std::cout << "c " << cr->first << " r " << cr->second << " ";
    std::cout << std::endl;
  }

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

  if (dacsToScan.size() >= 3 && currentDACValues["TBMPLL"] != lastTBMPLL) {
    lastTBMPLL = currentDACValues["TBMPLL"];
    BookEm(TString::Format("TBMPLL%03i", lastTBMPLL));
  }

  //////

  for (unsigned ifed = 0; ifed < fedsAndChannels.size(); ++ifed) {
    const unsigned fednumber = fedsAndChannels[ifed].first;
    const unsigned long vmeBaseAddress = theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
    PixelFEDInterface* iFED = FEDInterface_[vmeBaseAddress];
    iFED->readDigFEDStatus(false, false);

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
    const uint32_t fifoStatus = iFED->getFifoStatus();

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

    if (fifoStatus & 0x01) FillEm(state, F11almostFull, 1);
    if (fifoStatus & 0x04) FillEm(state, F13almostFull, 1);
    if (fifoStatus & 0x10) FillEm(state, F15almostFull, 1);
    if (fifoStatus & 0x40) FillEm(state, F17almostFull, 1);
    if (fifoStatus & 0x02) FillEm(state, F21almostFull, 1);
    if (fifoStatus & 0x08) FillEm(state, F23almostFull, 1);
    if (fifoStatus & 0x20) FillEm(state, F25almostFull, 1);
    if (fifoStatus & 0x80) FillEm(state, F27almostFull, 1);
    if (fifoStatus & 0x100) FillEm(state, F31almostFull, 1);
    if (fifoStatus & 0x200) FillEm(state, F37almostFull, 1);
    
    assert(F17nTBMHeader - F11nTBMHeader == 7);
    for (int FIFO1Chip = 0; FIFO1Chip < 2; ++FIFO1Chip) {
      const int chip = FIFO1Chip ? 7 : 1;
      const int arroff = FIFO1Chip ? 7 : 0;
      if (PrintHits) std::cout << "F1" << chip << " ";

      FIFO1DigDecoder* d = decode1[chip];
      FillEm(state, F11nTBMHeader + arroff,
	     int(d->tbm_header_l[0].size() != 0) + 
	     int(d->tbm_header_l[1].size() != 0));
      FillEm(state, F11nTBMHeaders + arroff, 
	     d->tbm_header_l[0].size() + 
	     d->tbm_header_l[1].size());
      FillEm(state, F11nTBMTrailer + arroff,
	     int(d->tbm_trailer_l[0].size() != 0) + 
	     int(d->tbm_trailer_l[1].size() != 0));
      FillEm(state, F11nTBMTrailers + arroff,
	     d->tbm_trailer_l[0].size() + 
	     d->tbm_trailer_l[1].size());
      FillEm(state, F11nROCHeaders + arroff,
	     d->roc_header_l[0].size() + 
	     d->roc_header_l[1].size());

      int nwrong = 0, nright = 0;
      for (int tbm = 0; tbm < 2; ++tbm) {
	assert(d->roc_hit_col[tbm].size() == d->roc_hit_row[tbm].size());
	for (size_t h = 0; h < d->roc_hit_col[tbm].size(); ++h) {
	  const int col = d->roc_hit_col[tbm][h];
	  const int row = d->roc_hit_row[tbm][h];
	  if (PrintHits) std::cout << "c " << col << " r " << row << " ";

	  if ((col == 8 && row == 277) || (col == 17 && row == 276))
	    ++nright;
	  else
	    ++nwrong;
	}
      }
      FillEm(state, F11wrongPix + arroff, nwrong);
      FillEm(state, F11rightPix + arroff, nright);
      if (PrintHits) std::cout << std::endl;
    }

    for (int chip = 1; chip <= 7; chip += 2) {
      if (PrintHits) std::cout << "F2" << chip << " ";
      FIFO2DigDecoder* d = decode2[chip];
      int arroff = (chip-1)/2*6;
      FillEm(state, F21nTBMHeader  + arroff, int(d->tbm_header_found_));
      FillEm(state, F21nTBMTrailer + arroff, int(d->tbm_trailer_found_));
      FillEm(state, F21nROCHeaders + arroff, int(d->roc_headers_.size()));
      int nwrong = 0, nright = 0;
      for (size_t h = 0; h< d->hits_.size(); ++h) {
	if (PrintHits) std::cout << "c " << d->hits_[h].col << " r " << d->hits_[h].row << " ";
	const int col = d->hits_[h].col;
	const int row = d->hits_[h].row;
	if ((col == 8 && row == 277) || (col == 17 && row == 276))
	  ++nright;
	else
	  ++nwrong;
      }
      FillEm(state, F21wrongPix + arroff, nwrong);
      FillEm(state, F21rightPix + arroff, nright);
      FillEm(state, F21dangling + arroff, int(d->dangling_hit_info_));
      if (PrintHits) std::cout << std::endl;
    }

    if (PrintHits) std::cout << "F3X ";
    if (status3 <= 0)
      FillEm(state, F3fifoErr, 1);
    else {
      for (unsigned ihit = 0; ihit < decode3->nhits(); ++ihit) {
	const unsigned channel = decode3->channel(ihit);
	const unsigned rocid = decode3->rocid(ihit);
	assert(rocid > 0);

	const PixelROCName& roc = theNameTranslation_->ROCNameFromFEDChannelROC(fednumber, channel, rocid-1);

	// Skip if this ROC is not on the list of ROCs to calibrate.
	// Also skip if we're in singleROC mode, and this ROC is not being calibrated right now.
	vector<PixelROCName>::const_iterator foundROC = find(rocs.begin(), rocs.end(), roc);
	if (foundROC == rocs.end()) // || !tempCalibObject->scanningROCForState(roc, state))
	  FillEm(state, F3wrongRoc, 1);
	else {
	  const unsigned col = decode3->column(ihit);
	  const unsigned row = decode3->row(ihit);
	  if (PrintHits) std::cout << "c " << col << " r " << row << " ";
	  if (colrows.find(std::make_pair(col, row)) == colrows.end())
	    FillEm(state, F3wrongPix, 1);
	  else
	    FillEm(state, F3rightPix, 1);
	}
      }
    }
    if (PrintHits) std::cout << std::endl;

    //////

    if (DumpFIFOs) {
      std::cout << "FIFO statuses:\n";
      iFED->dump_FifoStatus(fifoStatus);

      int col2=-1, row2=-1;
      std::cout << "FIFO 2 buffer sizes: ";
      for (int chip = 1; chip <= 7; chip += 2)
	std::cout << std::setw(4) << status2[chip] << " ";
      std::cout << endl;

      /*
      std::cout << "event numbers: ";
      const int evnum = decode1[1]->event_number[0][0];
      bool evnumok = true;
      bool badfifo1decode = false;
      for (int chip = 1; chip <= 7; chip += 2) {
	std::cout << "chip " << chip << ": ";
	if (chip == 1 || chip == 7) {
	  if (decode1[chip]->tbm_header_l[0].size() != 1 || 
	      decode1[chip]->tbm_header_l[1].size() != 1 || 
	      decode1[chip]->tbm_trailer_l[0].size() != 1 || 
	      decode1[chip]->tbm_trailer_l[1].size() != 1)
	    badfifo1decode = true;
	  std::cout << "f1: " << decode1[chip]->event_number[0][0] << " " << decode1[chip]->event_number[1][0] << " ";
	  if (decode1[chip]->event_number[0][0] != evnum || decode1[chip]->event_number[1][0] != evnum)
	    evnumok = false;
	}
	std::cout << "f2: " << decode2[chip]->event_number_ << " ";
	if (int(decode2[chip]->event_number_) != evnum)
	  evnumok = false;
      }
      std::cout << std::endl << "fifo1 decodes ok? " << (badfifo1decode ? "no" : "yes") << " evnums ok? " << (evnumok ? "yes" : "no") << std::endl;
      */

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
		  << "nhits: " << decode3->nhits() << std::endl;
	for (unsigned i = 0; i < decode3->nhits(); ++i) {
	  const PixelROCName& rocname = theNameTranslation_->ROCNameFromFEDChannelROC(fednumber, decode3->channel(i), decode3->rocid(i)-1);
	  std::cout << "#" << i << ": ch: " << decode3->channel(i)
		    << " rocid: " << decode3->rocid(i)
		    << " (" << rocname << ")"
		    << " dcol: " << decode3->dcol(i)
		    << " pxl: " << decode3->pxl(i) << " pulseheight: " << decode3->pulseheight(i)
		    << " col: " << decode3->column(i) << " row: " << decode3->row(i) << std::endl;
	}
	if (decode3->nhits() > 0)
	  std::cout << "(fifo2 col: " << col2 << " row: " << row2 << "   fifo3 dcol: " << decode3->dcol(0) << " pxl: " << decode3->pxl(0) << " col: " << decode3->column(0) << " row: " << decode3->row(0) << ")\n";
      }
      std::cout << "Contents of Error FIFO" << std::endl;
      std::cout << "----------------------" << std::endl;
      for (int i = 0; i <= statusErr; ++i)
	std::cout << "Clock " << i << " = 0x" << std::hex << bufferErr[i] << std::dec << std::endl;
      std::cout << "ErrorFIFODecoder thinks:\n";
      decodeErr.printToStream(std::cout);
    }

    //////

    for (int chip = 1; chip <= 7; chip += 2) {
      if (chip == 1 || chip == 7)
	delete decode1[chip];
      delete decode2[chip];
    }
    delete decode3;
  }

  sendResets();
}

void PixelFEDTBMDelayCalibration::Analyze() {
  CloseRootf();
}

void  PixelFEDTBMDelayCalibration::CloseRootf() {
  if (rootf) {
    rootf->Write();
    rootf->Close();
    delete rootf;
  }
}

void PixelFEDTBMDelayCalibration::BookEm(const TString& path) {
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
    "F11almostFull", "F13almostFull", "F15almostFull", "F17almostFull",
    "F21almostFull", "F23almostFull", "F25almostFull", "F27almostFull",
    "F31almostFull", "F37almostFull",
    "F11nTBMHeader", "F11nTBMHeaders", "F11nTBMTrailer", "F11nTBMTrailers", "F11nROCHeaders", "F11wrongPix", "F11rightPix",
    "F17nTBMHeader", "F17nTBMHeaders", "F17nTBMTrailer", "F17nTBMTrailers", "F17nROCHeaders", "F17wrongPix", "F17rightPix",
    "F21nTBMHeader", "F21nTBMTrailer", "F21nROCHeaders", "F21wrongPix", "F21rightPix", "F21dangling",
    "F23nTBMHeader", "F23nTBMTrailer", "F23nROCHeaders", "F23wrongPix", "F23rightPix", "F23dangling",
    "F25nTBMHeader", "F25nTBMTrailer", "F25nROCHeaders", "F25wrongPix", "F25rightPix", "F25dangling",
    "F27nTBMHeader", "F27nTBMTrailer", "F27nROCHeaders", "F27wrongPix", "F27rightPix", "F27dangling",
    "F3fifoErr", "F3wrongRoc", "F3wrongPix", "F3rightPix"
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

void PixelFEDTBMDelayCalibration::FillEm(unsigned state, int which, float c) {
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
