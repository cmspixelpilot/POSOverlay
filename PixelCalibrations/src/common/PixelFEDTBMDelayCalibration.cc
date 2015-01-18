#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"
#include "PixelCalibrations/include/PixelFEDTBMDelayCalibration.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "PixelUtilities/PixelFEDDataTools/include/ErrorFIFODecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO3Decoder.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"

//#include "TCanvas.h"
//#include "TFrame.h"
//#include "TH1F.h"
//#include "TPaveText.h"
//#include "TText.h"
//#include "TFile.h"
//#include "TStyle.h"
//#include "TGraphErrors.h"
//#include "TGraph2DErrors.h"
//#include "TMarker.h"
//#include "TH1I.h"
//#include "TTree.h"

//#include <toolbox/convertstring.h>

//#include "iomanip"

using namespace pos;
using namespace std;

PixelFEDTBMDelayCalibration::PixelFEDTBMDelayCalibration(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr)
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  cout << "Greetings from the PixelFEDTBMDelayCalibration copy constructor." << endl;
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
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  const std::vector<PixelROCName>& ROCsToCalibrate = tempCalibObject->rocList();
  const std::vector<std::pair<unsigned, std::vector<unsigned> > >& fedsAndChannels = tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_);

  cout << "RETR state " << state << " ";
  std::map<std::string, unsigned int> currentDACValues;
  for (unsigned dacnum = 0; dacnum < tempCalibObject->numberOfScanVariables(); ++dacnum) {
    const std::string& dacname = tempCalibObject->scanName(dacnum);
    const unsigned dacvalue = tempCalibObject->scanValue(tempCalibObject->scanName(dacnum), state);
    currentDACValues[dacname] = dacvalue;
    cout << dacname << " " << dacvalue << " ";
  }

  for (unsigned ifed = 0; ifed < fedsAndChannels.size(); ++ifed) {
    const unsigned fednumber = fedsAndChannels[ifed].first;
    const unsigned long vmeBaseAddress = theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);

    uint64_t buffer64[4096];
    const int status = FEDInterface_[vmeBaseAddress]->spySlink64(buffer64);
    if (status <= 0) {
      cout << "ERROR reading FIFO3 on FED # " << fednumber << " in crate # " << crate_ << "." << endl;
      continue;
    }

    uint32_t errBuffer[36*1024];
    const int errCount = FEDInterface_[vmeBaseAddress]->drainErrorFifo(errBuffer);
    ErrorFIFODecoder errors(errBuffer, errCount);
    cout << "nerr " << errCount << " ";

    FIFO3Decoder decode(buffer64);
    const unsigned nhits = decode.nhits();
    cout << "nhits " << nhits << " ";

    for (unsigned ihit = 0; ihit < nhits; ++ihit) {
      const unsigned channel = decode.channel(ihit);
      const unsigned rocid = decode.rocid(ihit);
      assert(rocid > 0);

      const PixelROCName& roc = theNameTranslation_->ROCNameFromFEDChannelROC(fednumber, channel, rocid-1);

      cout << "hit #" << ihit << " roc " << roc << " (" << rocid << ") ";
//// Skip if this ROC is not on the list of ROCs to calibrate.
//vector<PixelROCName>::const_iterator foundROC = find(ROCsToCalibrate.begin(), ROCsToCalibrate.end(), roc);
//if (foundROC == ROCsToCalibrate.end())
//	continue;
//
//// Skip if we're in singleROC mode, and this ROC is not being calibrated right now.
//if (!tempCalibObject->scanningROCForState(roc, state))
//	continue;

      cout << " ch " << channel << " col " << decode.column(ihit) << " row " << decode.row(ihit) << " ";
    }

    cout << endl;
  }
}

void PixelFEDTBMDelayCalibration::Analyze() {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject!=0);

  hit_counts.clear();

  //outputFile.Write();
  //outputFile.Close();
}

void PixelFEDTBMDelayCalibration::initializeFED() {
  setFEDModeAndControlRegister(0x8,0x30010);
}

xoap::MessageReference PixelFEDTBMDelayCalibration::beginCalibration(xoap::MessageReference msg) {
  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDTBMDelayCalibration::endCalibration(xoap::MessageReference msg) {
  cout << "In PixelFEDTBMDelayCalibration::endCalibration()" << endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}
