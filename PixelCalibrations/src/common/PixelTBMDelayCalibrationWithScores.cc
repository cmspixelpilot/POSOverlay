#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"
#include "PixelCalibrations/include/PixelTBMDelayCalibrationWithScores.h"

using namespace pos;

PixelTBMDelayCalibrationWithScores::PixelTBMDelayCalibrationWithScores(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr)
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
}

void PixelTBMDelayCalibrationWithScores::beginCalibration() {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  fec_timer.setName("FEC programming");
  sleep_timer.setName("Sleeping");
  trig_timer.setName("Triggering");
  fed_timer.setName("FED soaping");

  // Check that PixelCalibConfiguration settings make sense.
	
  if (!tempCalibObject->containsScan("TBMADelay") && !tempCalibObject->containsScan("TBMBDelay") && !tempCalibObject->containsScan("TBMPLL"))
    std::cout << "warning: none of TBMADelay, TBMBDelay, TBMPLLDelay found in scan variable list!" <<std::endl;
}

bool PixelTBMDelayCalibrationWithScores::execute() {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  const bool firstOfPattern = event_ % tempCalibObject->nTriggersPerPattern() == 0;
  const unsigned state = event_/(tempCalibObject->nTriggersPerPattern());
  reportProgress(0.05);

  // Configure all TBMs and ROCs according to the PixelCalibConfiguration settings, but only when it's time for a new configuration.
  if (firstOfPattern) {
    std::cout << "New TBM delay state " << state << std::endl;
    fec_timer.start();
    commandToAllFECCrates("CalibRunning");
    fec_timer.stop();
  }

  if (firstOfPattern) {
    std::cout << "Sleeping 5 seconds for feds to re-acquire phases" << std::endl;
    sleep_timer.start();
    sleep(5);
    sleep_timer.stop();
  }

  // Send trigger to all TBMs and ROCs.
  trig_timer.start();
  sendTTCCalSync();
  trig_timer.stop();

  // Read out data from each FED.
  Attribute_Vector parametersToFED(2);
  parametersToFED[0].name_ = "WhatToDo"; parametersToFED[0].value_ = "RetrieveData";
  parametersToFED[1].name_ = "StateNum"; parametersToFED[1].value_ = itoa(state);
  fed_timer.start();
  commandToAllFEDCrates("FEDCalibrations", parametersToFED);
  fed_timer.stop();

  return event_ + 1 < tempCalibObject->nTriggersTotal();
}

void PixelTBMDelayCalibrationWithScores::endCalibration() {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);
  assert(event_ == tempCalibObject->nTriggersTotal());

  fec_timer.printStats();
  sleep_timer.printStats();
  trig_timer.printStats();
  fed_timer.printStats();

  Attribute_Vector parametersToFED(2);
  parametersToFED[0].name_ = "WhatToDo"; parametersToFED[0].value_ = "Analyze";
  parametersToFED[1].name_ = "StateNum"; parametersToFED[1].value_ = "0";
  commandToAllFEDCrates("FEDCalibrations", parametersToFED);
}

std::vector<std::string> PixelTBMDelayCalibrationWithScores::calibrated() {
  std::vector<std::string> tmp;
  return tmp;
}
