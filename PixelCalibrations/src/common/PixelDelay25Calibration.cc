// $Id: PixelDelay25Calibration.cc,v 1.32 2010/04/28 19:05:44 joshmt Exp $: PixelDelay25Calibration.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelDelay25Calibration.h"

#include <toolbox/convertstring.h>

using namespace pos;
using namespace std;

PixelDelay25Calibration::PixelDelay25Calibration(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  //  cout << "Greetings from the PixelDelay25Calibration copy constructor." <<endl;

}

void PixelDelay25Calibration::beginCalibration() {

  string action="BeginCalibration";
  sendToTKFEC(action);
}

void PixelDelay25Calibration::endCalibration() {
  //  cout<<"In PixelDelay25Calibration::endCalibration()"<<endl;
  string action="EndCalibration";
  sendToTKFEC(action);
}

bool PixelDelay25Calibration::execute(){

   //std::cout << "The execute member function has been called from PixelDelay25Calibrations." << std::endl;
  //  PixelDelay25Calib* tempDelay25 = dynamic_cast <PixelDelay25Calib*> (theCalibObject_);
  //  assert(tempDelay25!=0);
  //diagService_->reportError("--- Running for Delay 25 Scan ---",DIAGINFO);
  //std::cout<<"\n--- Running for Delay 25 Scan ---"<<std::endl;
  
  bool anyaretrue=false;
  Supervisors::iterator i_PixelTKFECSupervisor;
  for (i_PixelTKFECSupervisor=PixelTKFECSupervisors_.begin();i_PixelTKFECSupervisor!=PixelTKFECSupervisors_.end();++i_PixelTKFECSupervisor) {
    string response = Send(i_PixelTKFECSupervisor->second, "Delay25");
    
    //   cout<<"Instance = "<<i_PixelTKFECSupervisor->first<<" response = "<<response<<endl;
    
    if(response=="KeepGoing") {
      anyaretrue=true;
    }
  }

  return anyaretrue;

}

std::vector<std::string> PixelDelay25Calibration::calibrated(){

  std::vector<std::string> tmp;

  tmp.push_back("portcard");

  return tmp;

}
