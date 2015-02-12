#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"
#include "PixelCalibrations/include/PixelFEDTBMDelayCalibration.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"
#include "PixelUtilities/PixelFEDDataTools/include/ErrorFIFODecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO2Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO3Decoder.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"

using namespace pos;
using namespace std;

PixelFEDTBMDelayCalibration::PixelFEDTBMDelayCalibration(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr)
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  cout << "Greetings from the PixelFEDTBMDelayCalibration copy constructor." << endl;
}

void PixelFEDTBMDelayCalibration::initializeFED() {
  setFEDModeAndControlRegister(0x8,0x30010);
}

xoap::MessageReference PixelFEDTBMDelayCalibration::beginCalibration(xoap::MessageReference msg) {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  std::string retr_fn(outputDir() + "/RETR.txt");
  cout << "writing RETR lines to file " << retr_fn << endl;
  retrf.open(retr_fn);

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
      dacstoscan.push_back(dacname);
  }

  if (dacstoscan.empty()) {
    cout << "no dacs in scan?" << endl;
    assert(0);
  }

  h_nfiforeaderrors = new TH1F("nfiforeaderrors", "", 1, 0, 1);
  h_nerrors = new TH1F("nerrors", "", 50, 0, 50);
  h_nhits = new TH1F("nhits", "", 50, 0, 50);
  h_nskip = new TH1F("nskip", "", 50, 0, 50);

  const TString sdecode[nDecode] = { "wrongPix", "rightPix" };

  for (int idecode = 0; idecode < nDecode; ++idecode) {
    for (size_t i = 0; i < dacstoscan.size(); ++i) {
      const std::string& iname = dacstoscan[i];
      const TString itname(iname.c_str());
      const std::vector<unsigned>& ivals = tempCalibObject->scanValues(iname);
      const int ni = ivals.size();
      const double imin = tempCalibObject->scanValueMin(iname);
      const double imax = tempCalibObject->scanValueMax(iname);
      
      TH1F* h = new TH1F(itname + "_" + sdecode[idecode], sdecode[idecode] + ";" + itname + ";ntrig", ni, imin, imax);
      scans1d[idecode].push_back(h);

      for (size_t j = i+1; j < dacstoscan.size(); ++j) {
	const std::string jname = dacstoscan[j];
	const TString jtname(jname.c_str());
	const std::vector<unsigned>& jvals = tempCalibObject->scanValues(jname);
	const int nj = jvals.size();
	const double jmin = tempCalibObject->scanValueMin(jname);
	const double jmax = tempCalibObject->scanValueMin(jname);
      
	TH2F* h2 = new TH2F(jtname + "_v_" + itname + "_" + sdecode[idecode], sdecode[idecode] + ";" + itname + ";" + jtname, ni, imin, imax, nj, jmin, jmax);
	scans2d[idecode].push_back(h2);
      }
    }
  }
      
  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDTBMDelayCalibration::endCalibration(xoap::MessageReference msg) {
  cout << "close RETR output file" << endl;
  retrf.close();
  
  cout << "In PixelFEDTBMDelayCalibration::endCalibration()" << endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDTBMDelayCalibration::execute(xoap::MessageReference msg) {
  Attribute_Vector parameters(2);
  parameters[0].name_="WhatToDo";
  parameters[1].name_="StateNum";
  Receive(msg, parameters);

  const unsigned state = atoi(parameters[1].value_.c_str());

  if (parameters[0].value_=="RetrieveData")
    RetrieveData(state);
  else if (parameters[0].value_=="Analyze")
    Analyze();
  else {
    cout << "ERROR: PixelFEDTBMDelayCalibration::execute() does not understand the WhatToDo command, "<< parameters[0].value_ <<", sent to it.\n";
    assert(0);
  }

  xoap::MessageReference reply = MakeSOAPMessageReference("FEDCalibrationsDone");
  return reply;
}

void PixelFEDTBMDelayCalibration::RetrieveData(unsigned state) {
  assert(retrf.is_open());

  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  const std::vector<PixelROCName>& rocs = tempCalibObject->rocList();
  typedef std::set< std::pair<unsigned int, unsigned int> > colrow_t;
  const colrow_t colrows = tempCalibObject->pixelsWithHits(state);

  const std::vector<std::pair<unsigned, std::vector<unsigned> > >& fedsAndChannels = tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_);

  retrf << "RETR event " << event_ << " state " << state << " ";
  std::cout << "RETR event " << event_ << " state " << state << " ";
  std::map<std::string, unsigned int> currentDACValues;
  for (unsigned dacnum = 0; dacnum < tempCalibObject->numberOfScanVariables(); ++dacnum) {
    const std::string& dacname = tempCalibObject->scanName(dacnum);
    const unsigned dacvalue = tempCalibObject->scanValue(tempCalibObject->scanName(dacnum), state);
    currentDACValues[dacname] = dacvalue;
    retrf << dacname << " " << dacvalue << " ";
    std::cout << dacname << " " << dacvalue << " ";
  }
  std::cout << std::endl;

  for (unsigned ifed = 0; ifed < fedsAndChannels.size(); ++ifed) {
    const unsigned fednumber = fedsAndChannels[ifed].first;
    const unsigned long vmeBaseAddress = theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
    PixelFEDInterface* iFED = FEDInterface_[vmeBaseAddress];

    uint64_t buffer3[pos::slinkDepth];
    uint32_t bufferErr[36*1024];
    const int status3 = iFED->spySlink64(buffer3);
    const int statusErr = iFED->drainErrorFifo(bufferErr);

    if (status3 <= 0) {
      retrf << "ERROR reading a fifo on FED # " << fednumber << " in crate # " << crate_ << ": status3 = " << status3 << endl;
      std::cout << "ERROR reading a fifo on FED # " << fednumber << " in crate # " << crate_ << ": status3 = " << status3 << endl;
      h_nfiforeaderrors->Fill(0);
      usleep(1000);
      std::cout << "readDigFEDStatus(): ";
      iFED->readDigFEDStatus(false);
      continue;
    }

    FIFO3Decoder decode3(buffer3);
    ErrorFIFODecoder decodeErr(bufferErr, statusErr);

    retrf << "nerr " << statusErr << " ";
    h_nerrors->Fill(statusErr);

    const unsigned nhits = decode3.nhits();
    retrf << "nhits " << nhits << " {hits ";
    h_nhits->Fill(nhits);

    if (DumpFIFOs) {
      for (int chip = 1; chip <= 7; chip += 2) {
	uint32_t buffer2[pos::fifo2Depth];
	const int status2 = iFED->drainDataFifo2(chip, buffer2);
	std::cout << "----------------------------------" << std::endl;
	std::cout << "Contents of Spy FIFO 2 for chip = " << chip << std::endl;
	std::cout << "----------------------------------" << std::endl;
	for (int i = 0; i <= status2; ++i)
	  std::cout << "Clock " << i << " = 0x" << std::hex << buffer2[i] << std::dec << std::endl;
	std::cout << "FIFO2Decoder thinks:\n";
	FIFO2Decoder decode2(buffer2, status2);
	decode2.printToStream(std::cout);
	std::cout << "----------------------------------" << std::endl;
      }

      std::cout << "----------------------" << std::endl;
      std::cout << "Contents of Spy FIFO 3" << std::endl;
      std::cout << "----------------------" << std::endl;
      for (int i = 0; i <= status3; ++i)
	std::cout << "Clock " << i << " = 0x" << std::hex << buffer3[i] << std::dec << std::endl;
      std::cout << "FIFO3Decoder thinks:\n"
		<< "nhits: " << decode3.nhits() << "\n";
      for (unsigned i = 0; i < decode3.nhits(); ++i)
	std::cout << "#" << i << ": ch: " << decode3.channel(i)
		  << " rocid: " << decode3.rocid(i) << " dcol: " << decode3.dcol(i)
		  << " pxl: " << decode3.pxl(i) << " pulseheight: " << decode3.pulseheight(i)
		  << " col: " << decode3.column(i) << " row: " << decode3.row(i) << std::endl;

      std::cout << "Contents of Error FIFO" << std::endl;
      std::cout << "----------------------" << std::endl;
      for (int i = 0; i <= statusErr; ++i)
	std::cout << "Clock " << i << " = 0x" << std::hex << bufferErr[i] << std::dec << std::endl;
      std::cout << "ErrorFIFODecoder thinks:\n";
      decodeErr.printToStream(std::cout);
    }

    unsigned nskip = 0;

    for (unsigned ihit = 0; ihit < nhits; ++ihit) {
      const unsigned channel = decode3.channel(ihit);
      const unsigned rocid = decode3.rocid(ihit);
      assert(rocid > 0);

      const PixelROCName& roc = theNameTranslation_->ROCNameFromFEDChannelROC(fednumber, channel, rocid-1);

      int idecode3 = 0;

      // Skip if this ROC is not on the list of ROCs to calibrate.
      // Also skip if we're in singleROC mode, and this ROC is not being calibrated right now.
      vector<PixelROCName>::const_iterator foundROC = find(rocs.begin(), rocs.end(), roc);
      if (foundROC == rocs.end()) { // || !tempCalibObject->scanningROCForState(roc, state)) {
	++nskip;
	continue;
      }

      const unsigned col = decode3.column(ihit);
      const unsigned row = decode3.row(ihit);
      
      retrf << "hit #" << ihit << " roc " << roc << " (" << rocid << ") ch " << channel << " col " << col << " row " << row << " ";

      if (colrows.find(std::make_pair(col, row)) == colrows.end())
	idecode3 = wrongPix;
      else
	idecode3 = rightPix;

      int k = 0;
      for (size_t i = 0; i < dacstoscan.size(); ++i) {
	const std::string& iname = dacstoscan[i];
	const unsigned ival = tempCalibObject->scanValue(iname, state);
	scans1d[idecode3][i]->Fill(ival);
	
	for (size_t j = i+1; j < dacstoscan.size(); ++j, ++k) {
	  const std::string jname = dacstoscan[j];
	  const unsigned jval = tempCalibObject->scanValue(jname, state);
	  scans2d[idecode3][k]->Fill(ival, jval);
	}
      }
    }

    h_nskip->Fill(nskip);
    retrf << "hits} nskip " << nskip << endl; 

    usleep(1000);
    std::cout << "readDigFEDStatus(): ";
    iFED->readDigFEDStatus(false);

  }
}

void PixelFEDTBMDelayCalibration::Analyze() {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  rootf->Write();
  rootf->Close();
}
