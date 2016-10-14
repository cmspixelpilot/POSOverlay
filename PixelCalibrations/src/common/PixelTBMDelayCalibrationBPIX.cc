#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"
#include "PixelCalibrations/include/PixelTBMDelayCalibrationBPIX.h"

//#include <toolbox/convertstring.h>

using namespace pos;
using namespace std;

PixelTBMDelayCalibrationBPIX::PixelTBMDelayCalibrationBPIX(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr)
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  std::cout << "Greetings from the PixelTBMDelayCalibrationBPIX copy constructor." << std::endl;
}

void PixelTBMDelayCalibrationBPIX::beginCalibration() {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  // Check that PixelCalibConfiguration settings make sense.
	
  if (!tempCalibObject->singleROC() && tempCalibObject->maxNumHitsPerROC() > 2 && tempCalibObject->parameterValue("OverflowWarning") != "no") {
    std::cout << "ERROR:  FIFO3 will overflow with more than two hits on each ROC.  To run this calibration, use 2 or less hits per ROC, or use SingleROC mode.  Now aborting..." << std::endl;
    assert(0);
  }

  if (!tempCalibObject->containsScan("TBMADelay") && !tempCalibObject->containsScan("TBMBDelay") && !tempCalibObject->containsScan("TBMPLL"))
    std::cout << "warning: none of TBMADelay, TBMBDelay, TBMPLLDelay found in scan variable list!" <<std::endl;

  ToggleChannels = tempCalibObject->parameterValue("ToggleChannels") == "yes";
  CycleScopeChannels = tempCalibObject->parameterValue("CycleScopeChannels") == "yes";
  DelayBeforeFirstTrigger = tempCalibObject->parameterValue("DelayBeforeFirstTrigger") == "yes";
  DelayEveryTrigger = tempCalibObject->parameterValue("DelayEveryTrigger") == "yes";
  
}

bool PixelTBMDelayCalibrationBPIX::execute() {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  const bool firstOfPattern = event_ % tempCalibObject->nTriggersPerPattern() == 0;
  const unsigned state = event_/(tempCalibObject->nTriggersPerPattern());
  reportProgress(0.05);

  // Configure all TBMs and ROCs according to the PixelCalibConfiguration settings, but only when it's time for a new configuration.
  if (firstOfPattern) {
    if (ToggleChannels) commandToAllFEDCrates("ToggleChannels");
    commandToAllFECCrates("CalibRunning");
  }

  if (firstOfPattern) {
    std::cout << "Sleeping 5 seconds for feds to re-acquire phases" << std::endl;
    sleep(5);
  }

  if (CycleScopeChannels) {
    const int em36 = event_ % 36;
    const int which = em36 / 9;
    const int channel = em36 % 9;
    std::cout << "fiddling with SetScopeChannel event_ = " << event_ << " % 36 = " << em36 << " which = " << which << " channel = " << channel << std::endl;

    Attribute_Vector parametersToFED(2);
    parametersToFED[0].name_ = "Which"; parametersToFED[0].value_ = itoa(which);
    parametersToFED[1].name_ = "Ch";    parametersToFED[1].value_ = itoa(channel);
    commandToAllFEDCrates("SetScopeChannel", parametersToFED);
  }

  // should take this out
  commandToAllFEDCrates("JMTJunk");

  if (DelayBeforeFirstTrigger && firstOfPattern)
    usleep(1000);

  if (DelayEveryTrigger)
    usleep(100000);

  // Send trigger to all TBMs and ROCs.
  sendTTCCalSync();

  // Read out data from each FED.
  Attribute_Vector parametersToFED(2);
  parametersToFED[0].name_ = "WhatToDo"; parametersToFED[0].value_ = "RetrieveData";
  parametersToFED[1].name_ = "StateNum"; parametersToFED[1].value_ = itoa(state);
  commandToAllFEDCrates("FEDCalibrations", parametersToFED);

  return event_ + 1 < tempCalibObject->nTriggersTotal();
}

void PixelTBMDelayCalibrationBPIX::endCalibration() {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);
  assert(event_ == tempCalibObject->nTriggersTotal());
	
  Attribute_Vector parametersToFED(2);
  parametersToFED[0].name_ = "WhatToDo"; parametersToFED[0].value_ = "Analyze";
  parametersToFED[1].name_ = "StateNum"; parametersToFED[1].value_ = "0";
  commandToAllFEDCrates("FEDCalibrations", parametersToFED);

}

std::vector<std::string> PixelTBMDelayCalibrationBPIX::calibrated() {
  std::vector<std::string> tmp;  
  tmp.push_back("tbm");
  return tmp;
}
