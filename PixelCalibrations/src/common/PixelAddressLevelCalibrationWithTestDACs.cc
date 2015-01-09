// Modified by Jennifer Vaughan 2007/06/01
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

#include "PixelCalibrations/include/PixelAddressLevelCalibrationWithTestDACs.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"

#include <toolbox/convertstring.h>

using namespace pos;
using namespace std;

PixelAddressLevelCalibrationWithTestDACs::PixelAddressLevelCalibrationWithTestDACs(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  std::cout << "Greetings from the PixelAddressLevelCalibrationWithTestDACs copy constructor." << std::endl;
}

bool PixelAddressLevelCalibrationWithTestDACs::execute()
{

  std::cout << "In PixelAddressLevelCalibrationWithTestDACs::execute()"<< std::endl;

 int MaxIterations=150, iteration=1;
 std::string response;

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
     std::cout<<"Baseline couldn't be released for FED #"<<fednumber<<" situated in crate "<<fedcrate<<" and VME base 					address = 0x"<<hex<<fedVMEBaseAddress<<std::endl;
   }
   
 }
 
 // Send Triggers and Retrieve data
 do {

   sendTTCCalSync();
  
   Attribute_Vector parametersToFED(2);
   parametersToFED[0].name_="Hits";  parametersToFED[0].value_="Yes";
   parametersToFED[1].name_="First"; parametersToFED[1].value_="False";
   if (iteration==1) parametersToFED[1].value_="True";
   if (iteration==MaxIterations) parametersToFED[0].value_="Analyse";

   std::string action="FEDCalibrations";
   sendToFED(action, parametersToFED);
   iteration+=1;

 } while (iteration<=MaxIterations);

 return false;

}

std::vector<std::string> PixelAddressLevelCalibrationWithTestDACs::calibrated(){

  std::vector<std::string> tmp;

  tmp.push_back("fedcard");

  return tmp;

}


