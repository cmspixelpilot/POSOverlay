// Modified by Jennifer Vaughan 2007/06/01
// $Id: PixelBaselineCalibration.cc,v 1.1 

#include "PixelCalibrations/include/PixelBaselineCalibration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "PixelUtilities/Pixelb2inUtilities/include/Pixelb2inCommander.h"
#include <toolbox/convertstring.h>
#include "xdata/UnsignedInteger.h"

using namespace pos;
//PixelBaselineCalibration::PixelBaselineCalibration() : PixelCalibrationBase()
//{
//  std::cout << "Greetings from the PixelBaselineCalibration default constructor." << std::endl;
//}

PixelBaselineCalibration::PixelBaselineCalibration(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  std::cout << "Greetings from the PixelBaselineCalibration copy constructor." << std::endl;
}

bool PixelBaselineCalibration::execute()
{

 PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
 assert(tempCalibObject!=0);

 int MaxIterations=14, iteration=0;

 std::set<unsigned int> fedcrates=tempCalibObject->getFEDCrates(theNameTranslation_, theFEDConfiguration_);
 std::set<unsigned int>::iterator ifedcrate;

  // Turn off automatic baseline correction.
  commandToAllFEDChannels("BaselineRelease");

  // Ensure that there will be no hits output.
  commandToAllFECCrates("ClrCalEnMass");

  xdaq::ApplicationDescriptor* PixelSupervisor=app_->getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor("PixelSupervisor", 0);

 bool continueIterating; 
 do {
   std::string msg = "PixelBaselineCalibration: Iteration= "+itoa(iteration)+" (max="+itoa(MaxIterations)+")";
   SendStatus(PixelSupervisor, msg);

   sendTTCCalSync();
  
   continueIterating=false;


   std::string action="FEDCalibrations";
   sendToFED(action, continueIterating);
   iteration+=1;


 } while (continueIterating && iteration<=MaxIterations);

 commandToAllFEDChannels("BaselineHold");
 
 if (iteration>=MaxIterations) 
   std::cout<<"FED Baseline didn't converge to +-5 of the target after "<<iteration<<" iterations!"<<std::endl;
 else 
   std::cout<<"FED Channel Baseline Calibration Done after "<<iteration<<" iterations"<<std::endl;

  std::string msg = "PixelBaselineCalibration: done in "+itoa(iteration)+" iterations out of a max of "+itoa(MaxIterations);
  SendStatus(PixelSupervisor, msg);

 return false;

}

std::vector<std::string> PixelBaselineCalibration::calibrated(){

  std::vector<std::string> tmp;

  tmp.push_back("fedcard");

  return tmp;

}

void PixelBaselineCalibration::sendToFED(std::string& action, bool& continueIterating){

  std::string receiveMsg;
  bool flag=true; 

  continueIterating=false;
  
  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  std::set<unsigned int> fedcrates=tempCalibObject->getFEDCrates(theNameTranslation_, theFEDConfiguration_);

  if(sendingMode_!="yes"){
   
    std::vector<int> messageIDs;

    for (std::set<unsigned int>::iterator itr=fedcrates.begin(), itr_end=fedcrates.end(); itr!=itr_end; itr++) {
   
      int messageID=send(PixelFEDSupervisors_[(*itr)], action, flag);
      messageIDs.push_back(messageID);

    }

    for (std::vector<int>::iterator itr=messageIDs.begin(), itr_end=messageIDs.end(); itr!=itr_end; itr++) {

      receiveMsg=waitForReplyWithReturn(*itr);

      cout << "[PixelBaselineCalibration::sendToFED] receiveMsg:"
	   << receiveMsg << endl;

      if(receiveMsg=="FEDBaselineCalibrationWithPixelsIterating") continueIterating=true;
    }
  }
  else if(sendingMode_=="yes"){
    for (std::set<unsigned int>::iterator itr=fedcrates.begin(), itr_end=fedcrates.end(); itr!=itr_end; ++itr){
      std::string response=Send(PixelFEDSupervisors_[(*itr)], action);
      if (response=="FEDBaselineCalibrationWithPixelsIterating") continueIterating=true; 
    }
  }
   
}
