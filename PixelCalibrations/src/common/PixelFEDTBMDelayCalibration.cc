#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"
#include "PixelCalibrations/include/PixelFEDTBMDelayCalibration.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"
#include "PixelUtilities/PixelFEDDataTools/include/ErrorFIFODecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/ColRowAddrDecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/DigScopeDecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/DigTransDecoder.h"
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
}

xoap::MessageReference PixelFEDTBMDelayCalibration::beginCalibration(xoap::MessageReference msg) {
  std::cout << "In PixelFEDTBMDelayCalibration::beginCalibration()" << std::endl;

  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  tempCalibObject->writeASCII(outputDir());

  OnlyFIFO3 = tempCalibObject->parameterValue("OnlyFIFO3") == "yes";
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

  // for scan over ADelay, BDelay, and PLL, we split the output in multiple files so one long scan dying doesn't lose all the data
  if (dacsToScan.size() >= 3 && currentDACValues["TBMPLL"] != lastTBMPLL) {
    lastTBMPLL = currentDACValues["TBMPLL"];
    BookEm(TString::Format("TBMPLL%03i", lastTBMPLL));
  }

  //////

  for (unsigned ifed = 0; ifed < fedsAndChannels.size(); ++ifed) {
    const unsigned fednumber = fedsAndChannels[ifed].first;
    const unsigned long vmeBaseAddress = theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
    PixelFEDInterfaceBase* iFED = FEDInterface_[vmeBaseAddress];

    uint32_t fifoStatus = 0;
    const int MaxChans = 37;    
    uint32_t bufferFifo1[MaxChans][1024];
    int statusFifo1[MaxChans] = {0};
    const int MaxChips = 8;
    uint32_t bufferT[MaxChips][256];
    uint32_t bufferS[MaxChips][256];
    uint64_t buffer3[2048];
    uint32_t bufferErr[36*1024];
    int statusS[MaxChips] = {0};
    int status3 = 0;
    int statusErr = 0;
    DigTransDecoder* decodeT[MaxChips] = {0};
    DigScopeDecoder* decodeS[MaxChips] = {0};
    FIFO3Decoder* decode3 = 0;
    ErrorFIFODecoder* decodeErr = 0;

    status3 = iFED->spySlink64(buffer3);
    if (status3 > 0)
      decode3 = new FIFO3Decoder(buffer3);

    if (!OnlyFIFO3) {
      fifoStatus = iFED->getFifoStatus();
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

      for (int ch = 1; ch <= 36; ++ch)
	statusFifo1[ch] = 0; //iFED->drainFifo1(ch, bufferFifo1[ch], 1024);

      for (int chip = 1; chip <= 7; chip += 2) {
	if (chip == 1 || chip == 7) {
	  //iFED->drainDigTransFifo(chip, bufferT[chip]);
	  decodeT[chip] = new DigTransDecoder(bufferT[chip]);
	}

	statusS[chip] = 0; //iFED->drainDataFifo2(chip, bufferS[chip]);
	decodeS[chip] = new DigScopeDecoder(bufferS[chip], statusS[chip]);
      }

      statusErr = iFED->drainErrorFifo(bufferErr);
      decodeErr = new ErrorFIFODecoder(bufferErr, statusErr);

      
      assert(FT7nTBMHeader - FT1nTBMHeader == 7);
      for (int TransChip = 0; TransChip < 2; ++TransChip) {
        const int chip = TransChip ? 7 : 1;
        const int arroff = TransChip ? 7 : 0;
        if (PrintHits) std::cout << "FT" << chip << " ";
  
        DigTransDecoder* d = decodeT[chip];
        FillEm(state, FT1nTBMHeader + arroff,
  	     int(d->tbm_header_l[0].size() != 0) + 
  	     int(d->tbm_header_l[1].size() != 0));
        FillEm(state, FT1nTBMHeaders + arroff, 
  	     d->tbm_header_l[0].size() + 
  	     d->tbm_header_l[1].size());
        FillEm(state, FT1nTBMTrailer + arroff,
  	     int(d->tbm_trailer_l[0].size() != 0) + 
  	     int(d->tbm_trailer_l[1].size() != 0));
        FillEm(state, FT1nTBMTrailers + arroff,
  	     d->tbm_trailer_l[0].size() + 
  	     d->tbm_trailer_l[1].size());
        FillEm(state, FT1nROCHeaders + arroff,
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
        FillEm(state, FT1wrongPix + arroff, nwrong);
        FillEm(state, FT1rightPix + arroff, nright);
        if (PrintHits) std::cout << std::endl;
      }
  
      for (int chip = 1; chip <= 7; chip += 2) {
        if (PrintHits) std::cout << "FS" << chip << " ";
        DigScopeDecoder* d = decodeS[chip];
        int arroff = (chip-1)/2*6;
        FillEm(state, FS1nTBMHeader  + arroff, int(d->tbm_header_found_));
        FillEm(state, FS1nTBMTrailer + arroff, int(d->tbm_trailer_found_));
        FillEm(state, FS1nROCHeaders + arroff, int(d->roc_headers_.size()));
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
        FillEm(state, FS1wrongPix + arroff, nwrong);
        FillEm(state, FS1rightPix + arroff, nright);
        FillEm(state, FS1dangling + arroff, int(d->dangling_hit_info_));
        if (PrintHits) std::cout << std::endl;
      }
    }

    if (PrintHits) std::cout << "F3X ";
    if (status3 <= 0)
      FillEm(state, F3fifoErr, 1);
    else {
      for (unsigned ihit = 0; ihit < decode3->nhits(); ++ihit) {
	const unsigned channel = decode3->channel(ihit);
	const unsigned rocid = decode3->rocid(ihit);
	const unsigned col = decode3->column(ihit);
	const unsigned row = decode3->row(ihit);

	// Skip if this ROC is not on the list of ROCs to calibrate.
	// Also skip if we're in singleROC mode, and this ROC is not being calibrated right now.
	PixelROCName roc;
	vector<PixelROCName>::const_iterator foundROC;
	if (rocid > 0) {
	  roc = theNameTranslation_->ROCNameFromFEDChannelROC(fednumber, channel, rocid-1);
	  foundROC = find(rocs.begin(), rocs.end(), roc);
	}

	if (rocid == 0 || foundROC == rocs.end()) {// || !tempCalibObject->scanningROCForState(roc, state))
	  std::cout << "!! wrong roc: " << roc << " ch " << channel << " col " << col << " row " << row << std::endl;
	  FillEm(state, F3wrongRoc, 1);
	}
	else {
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
      if (!OnlyFIFO3) {
	//iFED->readDigFEDStatus(false, false);

	//std::cout << "FIFO statuses:\n";
	//iFED->dump_FifoStatus(fifoStatus);

	std::cout << "-----------------------------------" << std::endl;
	std::cout << "Contents of FIFO 1 for all channels" << std::endl;
	std::cout << "-----------------------------------" << std::endl;
	for (int ch = 1; ch <= 36; ++ch) {
	  std::cout << "ch #" << std::setw(2) << ch << ": status: " << statusFifo1[ch] << "\n";
	  if (statusFifo1[ch] > 0) {
	    for (int i = 0; i < statusFifo1[ch]; ++i) {
	      uint32_t w = bufferFifo1[ch][i];
	      std::cout << "Word " << std::setw(4) << std::setfill(' ') << i << " = 0x " << std::hex << std::setw(4) << std::setfill('0') << (bufferFifo1[ch][i]>>16) << " " << std::setw(4) << std::setfill('0') << (bufferFifo1[ch][i] & 0xFFFF) << std::dec << "  ";
	      for (int j = 31; j >= 0; --j){
		if (w & (1 << j)) std::cout << "1";
		else std::cout << "0";
		if (j % 4 == 0) std::cout << " ";
	      }
	      std::cout << std::setfill(' ') << "  ";
	      uint32_t ch = (w >> 26) & 0x3f;
	      uint32_t mk = (w >> 21) & 0x1f;
	      uint32_t az = (w >> 8) & 0x1fff;
	      uint32_t dc = (w >> 16) & 0x1f;
	      uint32_t px = (w >> 8) & 0xff;
	      uint32_t f8 = w & 0xff;
	      std::cout << "ch " << ch << " ";
	      if (mk == 0x1f) {
		std::cout << "header ";
		if (az != 0)
		  std::cout << "(w/o 13 zeros) ";
		std::cout << " trg# " << f8;
	      }
	      else if (mk == 0x1e) {
		std::cout << "trailer ";
		if (az & 8)
		  std::cout << "inv#rocs ";
		if (az & 1)
		  std::cout << ">192 px/ch ";
		std::cout << "fsm errbyte: " << (az >> 1) << " trailer word " << std::hex << f8 << std::dec;
	      }
	      else { 
		if (az == 0)
		  std::cout << "roc header  lastdac: " << std::hex << f8 << std::dec;
		else
		  std::cout << "hit dcol " << std::setw(2) << dc << " pxl " << std::setw(3) << px << " ph " << std::setw(3) << f8;
	      }
	      std::cout << std::endl;
	    }
	  }
	}

	int colS=-1, rowS=-1;
	std::cout << "Scope FIFO buffer sizes: ";
	for (int chip = 1; chip <= 7; chip += 2)
	  std::cout << std::setw(4) << statusS[chip] << " ";
	std::cout << endl;

	/*
	  std::cout << "event numbers: ";
	  const int evnum = decodeT[1]->event_number[0][0];
	  bool evnumok = true;
	  bool badtransdecode = false;
	  for (int chip = 1; chip <= 7; chip += 2) {
	  std::cout << "chip " << chip << ": ";
	  if (chip == 1 || chip == 7) {
	  if (decode1[chip]->tbm_header_l[0].size() != 1 || 
	  decode1[chip]->tbm_header_l[1].size() != 1 || 
	  decode1[chip]->tbm_trailer_l[0].size() != 1 || 
	  decode1[chip]->tbm_trailer_l[1].size() != 1)
	  badtransdecode = true;
	  std::cout << "f1: " << decodeT[chip]->event_number[0][0] << " " << decodeT[chip]->event_number[1][0] << " ";
	  if (decodeT[chip]->event_number[0][0] != evnum || decodeT[chip]->event_number[1][0] != evnum)
	  evnumok = false;
	  }
	  std::cout << "f2: " << decodeS[chip]->event_number_ << " ";
	  if (int(decodeS[chip]->event_number_) != evnum)
	  evnumok = false;
	  }
	  std::cout << std::endl << "fifo1 decodes ok? " << (badfifo1decode ? "no" : "yes") << " evnums ok? " << (evnumok ? "yes" : "no") << std::endl;
	*/

	for (int chip = 1; chip <= 7; chip += 2) {
	  if (chip == 1 || chip == 7) {
	    bool trans_all_ff = false;
	    int trans_found = 0;
	    uint32_t pattern = 0;
	    uint32_t* data  = bufferT[chip];
	    uint32_t* datae = data + 255;
	    std::cout << "-----------------------------------------\n";
	    std::cout << "Contents of transparent FIFO for chip = " << chip << std::endl;
	    std::cout << "-----------------------------------------\n";
	    if (*data == 0xffffffff && *datae == 0xffffffff) {
	      int nbeg = 0, nend = 0;
	      while (*data == 0xffffffff && data != datae)
		++nbeg, ++data;
	      if (data == datae) {
		trans_all_ff = true;
		std::cout << "all 0xFFFFFFFF" << std::endl;
	      }
	      else {
		std::vector<char> bits[2]; // ha
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
		      char bit = (ab[j] & (1 << i)) ? '1' : '0';
		      bits[!j].push_back(bit);
		      std::cout << bit;
		      if (i % 4 == 0) std::cout << " ";
		    }
		    std::cout << "  ";
		  }
		  std::cout << std::endl;
		  ++data;
		}
		std::cout << "then " << nend << " 0xFFFFFFFF" << std::endl;

		std::cout << "try to align with headers:\n";
		const int nroccands = 8;
		for (int j = 0; j < 2; ++j) {
		  std::cout << "tbm " << j << ":\n";
		  int besttbmhead = -1;
		  int besttbmheadcount = -1;
		  int besttbmtrail = -1;
		  int besttbmtrailcount = -1;
		  std::vector<int> bestroc(nroccands, -1);
		  std::vector<int> bestroccount(nroccands, -1);
		  std::vector<int> bestrocalign(nroccands, -1);
		  int count = -1;
		  const int nbits = bits[j].size();
		  if (nbits < 12)
		    std::cout << "not enough bits\n";
		  else {
		    for (int i = 0; i < nbits - 12; ++i) {
		      count = 
			int(bits[j][i   ] == '0') +
			int(bits[j][i+ 1] == '1') +
			int(bits[j][i+ 2] == '1') +
			int(bits[j][i+ 3] == '1') +
			int(bits[j][i+ 4] == '1') +
			int(bits[j][i+ 5] == '1') +
			int(bits[j][i+ 6] == '1') +
			int(bits[j][i+ 7] == '1') +
			int(bits[j][i+ 8] == '1') +
			int(bits[j][i+ 9] == '1') +
			int(bits[j][i+10] == '0') +
			int(bits[j][i+11] == '0');
		      if (count > besttbmheadcount) {
			besttbmheadcount = count;
			besttbmhead = i;
		      }

		      count = 
			int(bits[j][i   ] == '0') +
			int(bits[j][i+ 1] == '1') +
			int(bits[j][i+ 2] == '1') +
			int(bits[j][i+ 3] == '1') +
			int(bits[j][i+ 4] == '1') +
			int(bits[j][i+ 5] == '1') +
			int(bits[j][i+ 6] == '1') +
			int(bits[j][i+ 7] == '1') +
			int(bits[j][i+ 8] == '1') +
			int(bits[j][i+ 9] == '1') +
			int(bits[j][i+10] == '1') +
			int(bits[j][i+11] == '0');
		      if (count > besttbmtrailcount) {
			besttbmtrailcount = count;
			besttbmtrail = i;
		      }

		      count = 
			int(bits[j][i   ] == '0') +
			int(bits[j][i+ 1] == '1') +
			int(bits[j][i+ 2] == '1') +
			int(bits[j][i+ 3] == '1') +
			int(bits[j][i+ 4] == '1') +
			int(bits[j][i+ 5] == '1') +
			int(bits[j][i+ 6] == '1') +
			int(bits[j][i+ 7] == '1') +
			int(bits[j][i+ 8] == '1') +
			int(bits[j][i+ 9] == '0');
		      const int align = (i - (besttbmhead + 12 + 16)) % 12;
		      if (align == 0) {
			for (int k = 0; k < nroccands; ++k) {
			  if (count > bestroccount[k]) {
			    int tmpcount = bestroccount[k];
			    int tmp = bestroc[k];
			    bestroccount[k] = count;
			    bestroc[k] = i;
			    for (int l = nroccands-1; l > k+1; --l) {
			      bestroccount[l] = bestroccount[l-1];
			      bestroc[l] = bestroc[l-1];
			    }
			    if (k < nroccands-1) {
			      bestroccount[k+1] = tmpcount;
			      bestroc[k+1] = tmp;
			    }
			    break;
			  }
			}
		      }
		    }

		    std::cout << "best match of tbm header  at " << std::setw(4) << nbeg*16 + besttbmhead  << " with count " << besttbmheadcount << "\n";
		    std::cout << "best match of tbm trailer at " << std::setw(4) << nbeg*16 + besttbmtrail << " with count " << besttbmtrailcount << "\n";
		    if ((besttbmtrail - (besttbmhead + 12 + 16)) % 12 != 0)
		      std::cout << "  ^ tbm trailer misaligned wrt tbm header!\n";
		    std::cout << "matches of roc headers:\n";
		    int bestroccountsum = 0;
		    for (int k = 0; k < nroccands; ++k) {
		      std::cout << "  at " << std::setw(4) << nbeg*16 + bestroc[k] << " with count " << bestroccount[k] << "\n";
		      if (k < 8) {
			bestroccountsum += bestroccount[k];
			if ((bestroc[k] - (besttbmhead + 12 + 16)) % 12 != 0)
			  std::cout << "    ^ roc header misaligned wrt tbm header!\n";
		      }
		    }

		    //		  std::vector<std::pair<int, int> > roccands;
		    //		  for (int k = 0; k < nroccands; ++k) {
		    //		    if (bestroccount[k] == 10) {
		    //		      roccands.push_back(std::make_pair(
		    //		  }
		  

		    if (besttbmheadcount != 12 || besttbmtrailcount != 12 || bestroccountsum != 80)
		      std::cout << "problem with headers or trailers!\n";

		    std::cout << "print, aligning only with tbm header, and guessing where roc headers and hit bits should be based on " << tempCalibObject->maxNumHitsPerROC() << " hits / roc in calib\n";
		    std::cout << "throw away: ";
		    for (int i = 0; i < besttbmhead; ++i) {
		      std::cout << bits[j][i];
		      if (i % 4 == 3) std::cout << " ";
		    }
		    std::cout << "\n";

		    std::cout << "tbm header: ";
		    for (int i = besttbmhead; i < besttbmhead+12; ++i) {
		      std::cout << bits[j][i];
		    }
		    std::cout << "  payload: ";
		    for (int i = besttbmhead+12; i < besttbmhead+12+2*8; ++i) {
		      std::cout << bits[j][i];
		      const int id = i-(besttbmhead+12);
		      if (id == 7 || id == 9) std::cout << " ";
		    }
		    std::cout << "\n";

		    const int nhitsperroc = tempCalibObject->maxNumHitsPerROC();
		    const int nbitsperroc = 12 + 3*8*nhitsperroc;
		    for (int k = 0; k < 8; ++k) {
		      const int ib = besttbmhead+12+2*8 + nbitsperroc*k;
		      const int ic = besttbmhead+12+2*8 + nbitsperroc*k + 12;
		      const int ie = besttbmhead+12+2*8 + nbitsperroc*(k+1);
		      std::cout << "roc " << k << " header: ";
		      for (int i = ib; i < ic; ++i) {
			std::cout << bits[j][i];
			if ((i-ib) == 9) std::cout << " ";
		      }
		      std::cout << "\nhits:\n";
		      for (int i = ic; i < ie; ++i) {
			std::cout << bits[j][i];
			const int id = (i - ic) % 24;
			if (id == 5 || id == 14 || id == 18 || id == 19) std::cout << " ";
			else if (id == 23) std::cout << "\n";
		      }
		    }

		    std::cout << "tbm trailer: ";
		    {
		      const int ib = besttbmhead+12+2*8 + nbitsperroc*8;
		      const int ie = besttbmhead+12+2*8 + nbitsperroc*8 + 12;
		      for (int i = ib; i < ie; ++i)
			std::cout << bits[j][i];
		    }
		    std::cout << "  payload: ";
		    {
		      const int ib = besttbmhead+12+2*8 + nbitsperroc*8 + 12;
		      const int ie = besttbmhead+12+2*8 + nbitsperroc*8 + 12 + 16;
		      for (int i = ib; i < ie; ++i) {
			std::cout << bits[j][i];
			if ((i-ib) % 4 == 3) std::cout << " ";
		      }
		    }
		    std::cout << std::endl;
		  }
		}
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
		std::cout << "256 repetitions of " << std::hex << pattern << std::dec << std::endl;
	      else {
		uint8_t* data8 = (uint8_t*)bufferT[chip];
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

	    std::cout << "DigTransDecoder thinks:\n";
	    decodeT[chip]->printToStream(std::cout);
	  }
	  //if (chip != 1 && chip != 7 && !trans_all_ff)
	  //  std::cout << "bad trans_all_ff: chip is " << chip << std::endl;

	  std::cout << "----------------------------------" << std::endl;
	  if (statusS[chip] < 0)
	    std::cout << "Scope FIFO for chip = " << chip << " status = " << statusS[chip] << std::endl;
	  else {
	    std::cout << "Contents of Scope FIFO for chip = " << chip << "(statusS = " << statusS[chip] << ")" <<std::endl;
	    std::cout << "----------------------------------" << std::endl;
	    for (int i = 0; i <= statusS[chip]; ++i) {
	      uint32_t d = bufferS[chip][i];
	      uint32_t dh = d & 0xf0;
	      if (dh == 0x70 || dh == 0x10 || dh == 0xc0)
		std::cout << "\n";
	      if (d & 0xFFFFFF00)
		std::cout << std::setw(6) << std::hex << ((d&0xFFFFFF00)>>8) << " " << std::setw(2) << std::hex << (d&0xFF) << std::dec << " ";
	      else 
		std::cout << std::setw(2) << std::hex << (d&0xFF) << std::dec << " ";
	    }
	    std::cout << "\n----------------------------------" << std::endl;
	  }
	  std::cout << "DigScopeDecoder thinks:\n";
	  decodeS[chip]->printToStream(std::cout);
	  if (decodeS[chip]->n_hits() > 6) {
	    colS = decodeS[chip]->hits()[0].col;
	    rowS = decodeS[chip]->hits()[0].row;
	  }
	}
	if (statusS[1] > 0 && statusS[3] > 0) {
	  cout<<"decodePTrans return: " << decodePTrans(bufferS[1],bufferS[3],16)<<endl;
	  cout<<"decodePTrans2 return: " << decodePTrans2(bufferS[1],bufferS[3],16)<<endl;
	  cout << "decodePTrans3: " << endl;
	  decodePTrans3(bufferS[1], bufferS[3], 64);
	}
      }

      std::cout << "----------------------" << std::endl;
      std::cout << "Contents of Spy FIFO 3" << std::endl;
      std::cout << "----------------------" << std::endl;
      for (int i = 0; i <= status3; ++i)
	std::cout << "Clock " << std::setw(2) << i << " = 0x " << std::hex << std::setw(8) << (buffer3[i]>>32) << " " << std::setw(8) << (buffer3[i] & 0xFFFFFFFF) << std::dec << std::endl;
      if (status3 > 0) {
	std::cout << "FIFO3Decoder thinks:\n"
		  << "nhits: " << decode3->nhits() << std::endl;
	int hits_by_ch[37] = {0};
	int hits_by_roc[37][9] = {{0}};
	unsigned lastroc = 0;
	for (unsigned i = 0; i < decode3->nhits(); ++i) {
	  const PixelROCName& rocname = theNameTranslation_->ROCNameFromFEDChannelROC(fednumber, decode3->channel(i), decode3->rocid(i)-1);
          unsigned rocidm1 = decode3->rocid(i)-1;
          if (rocidm1 > 7)
            rocidm1 = 8;
	  ++hits_by_ch[decode3->channel(i)];
	  ++hits_by_roc[decode3->channel(i)][rocidm1];
	  if (rocidm1 < 8 && lastroc != 0 && decode3->rocid(i) != lastroc) {
	    std::cout << "\n";
	    lastroc = decode3->rocid(i);
	  }
	  std::cout << "#" << i << ": ch: " << decode3->channel(i)
		    << " rocid: " << decode3->rocid(i)
		    << " (" << rocname << ")"
		    << " dcol: " << decode3->dcol(i)
		    << " pxl: " << decode3->pxl(i) << " pulseheight: " << decode3->pulseheight(i)
		    << " col: " << decode3->column(i) << " row: " << decode3->row(i) << std::endl;
	}
	std::cout << "Nhits by channel:\n";
	for (int i = 1; i <= 36; ++i)
	  if (hits_by_ch[i])
	    std::cout << "Ch " << std::setw(2) << i << ": " << std::setw(3) << hits_by_ch[i] << "\n";
	std::cout << "Nhits by roc:\n";
	for (int i = 1; i <= 36; ++i)
	  for (int j = 0; j < 8; ++j)
	    if (hits_by_roc[i][j])
	      std::cout << "Ch " << std::setw(2) << i << " roc " << j << ": " << std::setw(3) << hits_by_roc[i][j] << "\n";
	//if (decode3->nhits() > 0)
	//  std::cout << "(fifo2 col: " << colS << " row: " << rowS << "   fifo3 dcol: " << decode3->dcol(0) << " pxl: " << decode3->pxl(0) << " col: " << decode3->column(0) << " row: " << decode3->row(0) << ")\n";
      }

      if (!OnlyFIFO3) {
	std::cout << "Contents of Error FIFO" << std::endl;
	std::cout << "----------------------" << std::endl;
	for (int i = 0; i <= statusErr; ++i)
	  std::cout << "Clock " << i << " = 0x" << std::hex << bufferErr[i] << std::dec << std::endl;
	std::cout << "ErrorFIFODecoder thinks:\n";
	decodeErr->printToStream(std::cout);
      }
    }

    //////

    for (int chip = 1; chip <= 7; chip += 2) {
      if (chip == 1 || chip == 7)
	delete decodeT[chip];
      delete decodeS[chip];
    }

    delete decode3;
    delete decodeErr;
  }

  for (unsigned ifed = 0; ifed < fedsAndChannels.size(); ++ifed) {
    const unsigned fednumber = fedsAndChannels[ifed].first;
    const unsigned long vmeBaseAddress = theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
    FEDInterface_[vmeBaseAddress]->sendResets(3);
  }
}

void PixelFEDTBMDelayCalibration::Analyze() {
  CloseRootf();
}

void PixelFEDTBMDelayCalibration::CloseRootf() {
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
    "FT1nTBMHeader", "FT1nTBMHeaders", "FT1nTBMTrailer", "FT1nTBMTrailers", "FT1nROCHeaders", "FT1wrongPix", "FT1rightPix",
    "FT7nTBMHeader", "FT7nTBMHeaders", "FT7nTBMTrailer", "FT7nTBMTrailers", "FT7nROCHeaders", "FT7wrongPix", "FT7rightPix",
    "FS1nTBMHeader", "FS1nTBMTrailer", "FS1nROCHeaders", "FS1wrongPix", "FS1rightPix", "FS1dangling",
    "FS3nTBMHeader", "FS3nTBMTrailer", "FS3nROCHeaders", "FS3wrongPix", "FS3rightPix", "FS3dangling",
    "FS5nTBMHeader", "FS5nTBMTrailer", "FS5nROCHeaders", "FS5wrongPix", "FS5rightPix", "FS5dangling",
    "FS7nTBMHeader", "FS7nTBMTrailer", "FS7nROCHeaders", "FS7wrongPix", "FS7rightPix", "FS7dangling",
    "F3fifoErr", "F3wrongRoc", "F3wrongPix", "F3rightPix"
  };

  for (int idecode = 0; idecode < nDecode; ++idecode) {
    scans1d[idecode].clear();
    scans2d[idecode].clear();

    if (OnlyFIFO3 && idecode < F3fifoErr)
      continue;

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
