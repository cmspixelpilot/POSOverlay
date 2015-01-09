/*************************************************************************
 * XDAQ Components for Pixel Online Software                             *
 * Copyright (C) 2007, Cornell University		                 *
 * All rights reserved.                                                  *
 * Authors: Souvik Das          					 *
  *************************************************************************/

#include "PixelCalibrations/include/PixelGainAliveSCurveCalibrationWithSLink.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include <toolbox/convertstring.h>


using namespace pos;
using namespace std;

PixelGainAliveSCurveCalibrationWithSLink::PixelGainAliveSCurveCalibrationWithSLink(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  std::cout << "Greetings from the PixelGainAliveSCurveCalibrationWithSLink copy constructor." << std::endl;
}

void PixelGainAliveSCurveCalibrationWithSLink::beginCalibration(){

  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  mode_ = tempCalibObject->mode();
  nConfigs_=tempCalibObject->nConfigurations();
  nTriggersTotal_=tempCalibObject->nTriggersTotal();
  nTriggersPerPattern_=tempCalibObject->nTriggersPerPattern();
  missingTriggers_=0;

  useLTC_=false;
  if ( tempCalibObject->parameterValue("useLTC") == "yes" ){
    cout << "PixelGainAliveSCurveCalibrationWithSLink::beginCalibration "
	 << "will use LTC triggers." << endl;
    useLTC_=true;
  }

  diagService_->reportError("PixelSupervisor:: --- Running for " + mode_ + "Calibration ---",DIAGINFO);
  //cout << "\nPixelSupervisor:: --- Running for "<<mode<<"Calibration ---";


}


void PixelGainAliveSCurveCalibrationWithSLink::endCalibration(){

  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  mode_ = tempCalibObject->mode();

  diagService_->reportError("--- Running for " + mode_ + " Done ---",DIAGINFO);
  // cout<<"--- Running for "<<mode<<" Done ---\n";
  
  cout << "FEC total calls        :"<<fecTimer_.ntimes()
       << " total time:"<<fecTimer_.tottime()
       << "  avg time:"<<fecTimer_.avgtime() << endl;
  cout << "TTC total calls        :"<<ttcTimer_.ntimes()
       << " total time:"<<ttcTimer_.tottime()
       << "  avg time:"<<ttcTimer_.avgtime() << endl;

}


bool PixelGainAliveSCurveCalibrationWithSLink::execute(){

  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);
  
  //Please note that event_ here counts number of configurations!!!
  
  unsigned int nConfigs=tempCalibObject->nConfigurations();
  
  if(nextEvent_) {
    if (event_%(nConfigs/50)==0) {
      diagService_->reportError(stringF(event_) + " configurations out of " + stringF(nConfigs) + " have been done.",DIAGDEBUG);
      std::cout<<(event_*100)/nConfigs<<"% done."<<std::endl;
    }
  }
    
  if(nextEvent_) {
    fecTimer_.start();
    nextFECConfig(event_*nTriggersPerPattern_);
    fecTimer_.stop();
  }
  
  ttcTimer_.start();
  if (useLTC_){
    int sentTriggers = 0;
    if(nextEvent_) {
      //cout << "This is the start of event " << event_ << endl;
      sentTriggers = sendLTCCalSync(nTriggersPerPattern_);
      missingTriggers_ = nTriggersPerPattern_ - sentTriggers;
    } else {
      sentTriggers = sendLTCCalSync(missingTriggers_);
      missingTriggers_ = missingTriggers_ - sentTriggers;
    }
    if(missingTriggers_ > 0) {
      //cout << "Some triggers were missing!  This event will be repeated." << endl;
      nextEvent_ = false;
    } else if (missingTriggers_ == 0) {
      nextEvent_ = true;
    } else {
      //cout << "More triggers were generated than were requested!" << endl;
      //Not sure what to do about this case
      missingTriggers_ = 0;
      nextEvent_ = true;
    }

  }
  else{
    for(unsigned int ii=0;ii<nTriggersPerPattern_;ii++){
      sendTTCCalSync();
    }
  }
  ttcTimer_.stop();
  
  //Even if we are on the last event, do not return false if nextEvent_ is false
  if(nextEvent_) {
    return (event_+1!=tempCalibObject->nConfigurations());
  } else {
    return true;
  }
  
}

std::vector<std::string> PixelGainAliveSCurveCalibrationWithSLink::calibrated(){

  std::vector<std::string> tmp;

  return tmp;

}

