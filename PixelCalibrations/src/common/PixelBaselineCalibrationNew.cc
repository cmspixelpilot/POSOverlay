//--*-C++-*--
// $Id: PixelBaselineCalibrationNew.cc,v 1.2 2009/09/17 10:21:05 kreis Exp $
/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2009, Cornell University		                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd, S. Das, J. Vaughan, Y. Weng, J. Thompson		 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelBaselineCalibrationNew.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "PixelUtilities/Pixelb2inUtilities/include/Pixelb2inCommander.h"
#include <toolbox/convertstring.h>
#include "xdata/UnsignedInteger.h"

using namespace pos;
using namespace std;
//PixelBaselineCalibrationNew::PixelBaselineCalibrationNew() : PixelCalibrationBase()
//{
//  cout << "Greetings from the PixelBaselineCalibrationNew default constructor." << endl;
//}

PixelBaselineCalibrationNew::PixelBaselineCalibrationNew(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  //cout << "Greetings from the PixelBaselineCalibrationNew copy constructor." << endl;
}

void PixelBaselineCalibrationNew::beginCalibration()
{
  //  cout << "Greetings from the PixelBaselineCalibrationNew beginCalibration() method" << endl;
  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  //initialize counter and max value
  iteration_=0;
  MaxIterations_=15;

  string tolerance = tempCalibObject->parameterValue("tolerance") ;
  if (tolerance =="") tolerance_=5;
  else {
    tolerance_= atof( tolerance.c_str() );
    if (tolerance_<=0) tolerance_=5;    
  }
  
  // Turn off automatic baseline correction.
  commandToAllFEDChannels("BaselineRelease");
  
  // Ensure that there will be no hits output.
  commandToAllFECCrates("ClrCalEnMass");
}

void PixelBaselineCalibrationNew::endCalibration()
{
  //  cout << "Greetings from the PixelBaselineCalibrationNew endCalibration() method" << endl;
  commandToAllFEDChannels("BaselineHold");

  xdaq::ApplicationDescriptor* PixelSupervisor=app_->getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor("PixelSupervisor", 0);
  string msg = "PixelBaselineCalibrationNew: done in "+itoa(iteration_)+" iterations out of a max of "+itoa(MaxIterations_);
  SendStatus(PixelSupervisor, msg);

  if (iteration_>=MaxIterations_) 
    cout<<"FED Baseline didn't converge to +-"<<tolerance_<<" of the target after "<<iteration_<<" iterations!"<<endl;
  else 
    cout<<"FED Channel Baseline Calibration Done after "<<iteration_<<" iterations"<<endl;

}


bool PixelBaselineCalibrationNew::execute()
{
  
  xdaq::ApplicationDescriptor* PixelSupervisor=app_->getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor("PixelSupervisor", 0);
  string msg = "PixelBaselineCalibrationNew: Iteration= "+itoa(iteration_)+" (max="+itoa(MaxIterations_)+")";
  SendStatus(PixelSupervisor, msg);
  
  sendTTCCalSync();
  
  bool continueIterating=false;
  
  string action="FEDCalibrations";
  sendToFED(action, continueIterating);
  iteration_++;
  
  return (continueIterating && iteration_<=MaxIterations_);
}

vector<string> PixelBaselineCalibrationNew::calibrated(){

  vector<string> tmp;

  tmp.push_back("fedcard");

  return tmp;

}

void PixelBaselineCalibrationNew::sendToFED(string& action, bool& continueIterating){

  string receiveMsg;
  bool flag=true; 
  
  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  set<unsigned int> fedcrates=tempCalibObject->getFEDCrates(theNameTranslation_, theFEDConfiguration_);

  if(sendingMode_!="yes"){
   
    vector<int> messageIDs;

    for (set<unsigned int>::iterator itr=fedcrates.begin(), itr_end=fedcrates.end(); itr!=itr_end; itr++) {
   
      int messageID=send(PixelFEDSupervisors_[(*itr)], action, flag);
      messageIDs.push_back(messageID);

    }

    for (vector<int>::iterator itr=messageIDs.begin(), itr_end=messageIDs.end(); itr!=itr_end; itr++) {

      receiveMsg=waitForReplyWithReturn(*itr);

      cout << "[PixelBaselineCalibrationNew::sendToFED] receiveMsg:"
	   << receiveMsg << endl;

      if(receiveMsg=="FEDBaselineCalibrationWithPixelsIterating") continueIterating=true;
    }
  }
  else if(sendingMode_=="yes"){
    for (set<unsigned int>::iterator itr=fedcrates.begin(), itr_end=fedcrates.end(); itr!=itr_end; ++itr){
      string response=Send(PixelFEDSupervisors_[(*itr)], action);
      if (response=="FEDBaselineCalibrationWithPixelsIterating") continueIterating=true; 
    }
  }
   
}
