// $Id: PixelAddressLevelCalibration.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelClockPhaseCalibration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"

#include <toolbox/convertstring.h>

using namespace pos;
using namespace std;


PixelClockPhaseCalibration::PixelClockPhaseCalibration(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  std::cout << "Greetings from the PixelClockPhaseCalibration copy constructor." << std::endl;
}

void PixelClockPhaseCalibration::beginCalibration(){

  tempCalibObject_ = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject_!=0);

  diagService_->reportError("--- Running for Clock Delay and Phase Calibration ---",DIAGINFO);
  cout << "\n--- Running for Clock Delay and Phase Calibration ---\n";

  if (tempCalibObject_==0) {
    diagService_->reportError("The PixelCalibConfigurationObject doesn't exist! Breaking out of Clock Phase and Delay Calibration Routine.",DIAGERROR);
    cout<<"The PixelCalibConfigurationObject doesn't exist! Breaking out of Clock Phase and Delay Calibration Routine."<<endl;
    return ;
  }

  Attribute_Vector parametersToFED(2);
  parametersToFED[0].name_="VMEBaseAddress";
  parametersToFED[1].name_="FEDChannel";


  fedsAndChannels_ = tempCalibObject_->getFEDsAndChannels(theNameTranslation_);

  std::map <unsigned int, std::set<unsigned int> >::iterator i_fedsAndChannels;

  //Reset FEDs to clear stale data in FIFO1
  for (i_fedsAndChannels=fedsAndChannels_.begin();i_fedsAndChannels!=fedsAndChannels_.end();++i_fedsAndChannels){
    Attribute_Vector resetFED(1);
    unsigned int fednumber=i_fedsAndChannels->first;
    
    unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
    unsigned int crate=theFEDConfiguration_->crateFromFEDNumber(fednumber);
    resetFED[0].name_="VMEBaseAddress";
    resetFED[0].value_=itoa(vmeBaseAddress);
    string response=Send(PixelFEDSupervisors_[crate], 
		  "ResetFEDs", resetFED);

    std::set<unsigned int>::const_iterator ichannel=
      i_fedsAndChannels->second.begin();

    for(;ichannel!=i_fedsAndChannels->second.end();++ichannel){

      unsigned int fedchannel=*ichannel;

      parametersToFED[0].value_=itoa(vmeBaseAddress);
      parametersToFED[1].value_=itoa(fedchannel);

      //Note that we need to turn off the baseline correction
      //as the correction gets very confused when we use
      //invalid delay and phase settings.
      if (Send(PixelFEDSupervisors_[crate],
	       "BaselineRelease", parametersToFED)!=
	  "BaselineReleaseDone"){
	std::cout<<"Baseline couldn't be released for FED #"
		 <<fednumber<<" situated in crate "
		 <<crate<<" and VME baseaddress = 0x"
		 <<std::endl;
      }
    }
  }


}


bool PixelClockPhaseCalibration::execute() {
  
  std::map <unsigned int, std::set<unsigned int> >::iterator i_fedsAndChannels;
  
  unsigned int phase=event_/16;
  unsigned int delay=event_%16;

  Attribute_Vector delayFED(4), readFED(2);
  
  delayFED[0].name_="Channel";
  delayFED[1].name_="Phase";	delayFED[1].value_=itoa(phase);
  delayFED[2].name_="Delay";	delayFED[2].value_=itoa(delay);
  delayFED[3].name_="VMEBaseAddress";
  
  readFED[0].name_="Delay";	   readFED[0].value_=itoa(delay*2+phase);
  readFED[1].name_="Filename"; readFED[1].value_="ClockPhaseCalibration.dmp";
  for (i_fedsAndChannels=fedsAndChannels_.begin();i_fedsAndChannels!=fedsAndChannels_.end();++i_fedsAndChannels){
    unsigned int fednumber=i_fedsAndChannels->first;
    unsigned int crate=theFEDConfiguration_->crateFromFEDNumber(fednumber);
    unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
    delayFED[3].value_=itoa(vmeBaseAddress);
    std::set<unsigned int>::iterator i_channel=i_fedsAndChannels->second.begin();
    for (;i_channel!=i_fedsAndChannels->second.end();++i_channel){
      unsigned int channel=*i_channel;
      delayFED[0].value_=itoa((int)channel);
      string response=Send(PixelFEDSupervisors_[crate], "SetPhasesDelays", delayFED);  //FIXME!!!
      if (response!="SetPhasesDelaysDone") {
	diagService_->reportError("The phase and delay of the FED could not be set!",DIAGERROR);
	cout<<"The phase and delay of the FED could not be set!"<<endl;
      }
    }
  }

     
  // Give 10 trigger for this phase and delay and collect data
  
  for (unsigned int ntrig=0;ntrig<10;ntrig++){

   sendTTCCalSync();
   
   std::string action="FEDCalibrations";
   sendToFED(action, readFED);
  }       
  
  return (event_<32-1);

}

void PixelClockPhaseCalibration::endCalibration(){

}

std::vector<std::string> PixelClockPhaseCalibration::calibrated(){

  std::vector<std::string> tmp;

  tmp.push_back("fedcard");

  return tmp;

}
