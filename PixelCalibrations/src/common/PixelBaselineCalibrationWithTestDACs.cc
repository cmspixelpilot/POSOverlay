// Modified by Jennifer Vaughan 2007/06/01
// $Id: PixelBaselineCalibration.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelBaselineCalibrationWithTestDACs.h"

#include <toolbox/convertstring.h>

using namespace pos;

//PixelBaselineCalibrationWithTestDACs::PixelBaselineCalibrationWithTestDACs() : PixelCalibrationBase()
//{
//  std::cout << "Greetings from the PixelBaselineCalibrationWithTestDACs default constructor." << std::endl;
//}

PixelBaselineCalibrationWithTestDACs::PixelBaselineCalibrationWithTestDACs(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  std::cout << "Greetings from the PixelBaselineCalibrationWithTestDACs copy constructor." << std::endl;
}

bool PixelBaselineCalibrationWithTestDACs::execute()
{

 int MaxIterations=15, iteration=0;

 // Iterate over all FED boards and channels and release their baselines
 std::map <unsigned int, std::set<unsigned int> > fedsAndChannels=theDetectorConfiguration_->getFEDsAndChannels(theNameTranslation_);
 std::map <unsigned int, std::set<unsigned int> >::iterator i_fedsAndChannels=fedsAndChannels.begin();
 

 for (;i_fedsAndChannels!=fedsAndChannels.end();++i_fedsAndChannels) {

   unsigned long fednumber=i_fedsAndChannels->first;
   unsigned int fedcrate=theFEDConfiguration_->crateFromFEDNumber(fednumber);
   unsigned int fedVMEBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);

   Attribute_Vector parametersToFED(2);
   parametersToFED[0].name_="VMEBaseAddress";
   parametersToFED[1].name_="FEDChannel";
   parametersToFED[0].value_=itoa(fedVMEBaseAddress);
   parametersToFED[1].value_="*";

   if (Send(PixelFEDSupervisors_[fedcrate], "BaselineRelease", parametersToFED)!="BaselineReleaseDone"){
   std::cout<<"Baseline couldn't be released for FED #"<<fednumber<<" situated in crate "<<fedcrate<<" and VME bas address = 0x"<<std::hex<<fedVMEBaseAddress<<std::endl;
   }
   
 }
 
 // Send Triggers and Retrieve data
 bool continueIterating;
 do {

   sendTTCCalSync();
  
   continueIterating=false;
   std::string action="FEDCalibrations";
   sendToFED(action, continueIterating);
   iteration+=1;

 } while (continueIterating && iteration<=MaxIterations);


 for (;i_fedsAndChannels!=fedsAndChannels.end();++i_fedsAndChannels) {

   unsigned long fednumber=i_fedsAndChannels->first;
   unsigned int fedcrate=theFEDConfiguration_->crateFromFEDNumber(fednumber);
   unsigned int fedVMEBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
   
   Attribute_Vector parametersToFED(2);
   parametersToFED[0].name_="VMEBaseAddress";
   parametersToFED[1].name_="FEDChannel";
   parametersToFED[0].value_=itoa(fedVMEBaseAddress);
   parametersToFED[1].value_="*";

   if (Send(PixelFEDSupervisors_[fedcrate], "BaselineHold", parametersToFED)!="BaselineReleaseDone"){
     std::cout<<"Baseline couldn't be released for FED #"<<fednumber<<" situated in crate "<<fedcrate<<" and VME base 					address = 0x"<<std::hex<<fedVMEBaseAddress<<std::endl;
   }
   
 }

 if (iteration==MaxIterations) {
   std::cout<<"PixelFEDSupervisor::FED Baselines didn't converge to 512+-5 after "<<iteration<<" iterations!"<<std::endl;
 } else {
   std::cout<<"PixelFEDSupervisor::FED Baselines converged after "<<iteration<<" iterations."<<std::endl;
 }

 return false;

}


std::vector<std::string> PixelBaselineCalibrationWithTestDACs::calibrated(){

  std::vector<std::string> tmp;

  tmp.push_back("fedcard");

  return tmp;

}

void PixelBaselineCalibrationWithTestDACs::sendToFED(std::string& action, bool& continueIterating){

  std::string receiveMsg;
  bool flag=true; 
  
  Supervisors::iterator i_PixelFEDSupervisor;
  
  if(sendingMode_==""){
   
    std::vector<int> messageIDs;

    for(i_PixelFEDSupervisor=PixelFEDSupervisors_.begin();i_PixelFEDSupervisor!=PixelFEDSupervisors_.end();++i_PixelFEDSupervisor){
   
      int messageID=send(i_PixelFEDSupervisor->second, action, flag);
      messageIDs.push_back(messageID);
      receiveMsg=waitForReplyWithReturn(messageID);
      
      if(receiveMsg=="FEDBaselineCalibrationWithTestDACsIterating") continueIterating=true;
    }
  }
  else if(sendingMode_=="yes"){
    for(i_PixelFEDSupervisor=PixelFEDSupervisors_.begin();i_PixelFEDSupervisor!=PixelFEDSupervisors_.end();++i_PixelFEDSupervisor){
      std::string response=Send(i_PixelFEDSupervisor->second, action);
      std::cout<<"Calibration iteration response from FED Supervisor on crate "<<i_PixelFEDSupervisor->first<<" is: "<<response<<std::endl;
      if (response=="FEDBaselineCalibrationWithTestDACsIterating") continueIterating=true; 
    }
  }
   
}
