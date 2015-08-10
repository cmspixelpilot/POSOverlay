// $Id: PixelGainAliveSCurveCalibration.cc,v 1.31 2012/10/01 22:07:36 mdunser Exp $

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2009 Cornell University			         *
 * All rights reserved.                                                  *
 * Authors: A. Ryd, S. Das, S. Stroiney, Y. Weng, J. Vaughan             *                                    
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelGainAliveSCurveCalibration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"

#include <toolbox/convertstring.h>
#include <iomanip>

using namespace pos;
using namespace std;

namespace {
  const bool PRINT = false;
}

PixelGainAliveSCurveCalibration::PixelGainAliveSCurveCalibration(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  cout << "Greetings from the PixelGainAliveSCurveCalibration copy constructor." << endl;
}

void PixelGainAliveSCurveCalibration::beginCalibration(){

  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  mode_ = tempCalibObject->mode();
  nConfigs_=tempCalibObject->nConfigurations();
  nTriggersTotal_=tempCalibObject->nTriggersTotal();

  useLTC_=false;
  if ( tempCalibObject->parameterValue("useLTC") == "yes" ){
    cout << "PixelGainAliveSCurveCalibrationWithSLink::beginCalibration "
         << "will use LTC triggers." << endl;
    useLTC_=true;
  }

  sendTTCTBMReset();
 
  diagService_->reportError("PixelSupervisor:: --- Running for " + mode_ + "Calibration ---",DIAGINFO);
  //cout << "\nPixelSupervisor:: --- Running for "<<mode<<"Calibration ---";


}


void PixelGainAliveSCurveCalibration::endCalibration()
{

  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  mode_ = tempCalibObject->mode();

  diagService_->reportError("--- Running for " + mode_ + " Done ---",DIAGINFO);
  
  cout << "FEC total calls        :"<<fecTimer_.ntimes()
       << " total time:"<<fecTimer_.tottime()
       << "  avg time:"<<fecTimer_.avgtime() << endl;
  cout << "TTC total calls        :"<<ttcTimer_.ntimes()
       << " total time:"<<ttcTimer_.tottime()
       << "  avg time:"<<ttcTimer_.avgtime() << endl;
  cout << "FED readout total calls:"<<fedTimer_.ntimes()
       << " total time:"<<fedTimer_.tottime()
       << "  avg time:"<<fedTimer_.avgtime() << endl;

}


bool PixelGainAliveSCurveCalibration::execute()
{
  if (PRINT) printf("PixelGainAliveSCurveCalibration EXECUTE %i\n", int(event_));
  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  if (event_%tempCalibObject->nTriggersPerPattern()==0) {

    unsigned int nConfigs=tempCalibObject->nConfigurations();
    unsigned int patternNumber=event_/tempCalibObject->nTriggersPerPattern();
    unsigned int nPatternsForPrinting = nConfigs/50;
    if(nPatternsForPrinting == 0){
      nPatternsForPrinting = 1;
    }
    
    if(patternNumber%nPatternsForPrinting == 0){
      diagService_->reportError(stringF(patternNumber) + " configurations out of " + stringF(nConfigs) + " have been done.",DIAGDEBUG);
      
      double percentageOfJob=((double)patternNumber*100)/((double)nConfigs);
      this->setPercentageOfJob(percentageOfJob);
      cout << tempCalibObject->mode() << ": " << setprecision(2) << percentageOfJob<< setprecision(6) << "% complete" << endl;
    }
        
    fecTimer_.start();
    //if(false) { ////
    if (PRINT) printf("PixelGainAliveSCurveCalibration GOING TO NEXT FEC CONFIG\n");
    nextFECConfig(event_);
    if (PRINT) printf("PixelGainAliveSCurveCalibration BACK FROM NEXT FEC CONFIG\n");
    //} // end if false
    fecTimer_.stop();

  }

  //usleep(10000);	// 0.01 s. FIXME. Temporary throttling
  
  ttcTimer_.start();

  //if(false) { ////
  if(useLTC_) {
    //int sentTriggers =
    sendLTCCalSync(1);
  } else {
    sendTTCCalSync();
  }
  //} // end if 

  ttcTimer_.stop();

  if (PRINT) printf("PixelGainAliveSCurveCalibration TRIGGER, READ FED\n");
  fedTimer_.start();

  Attribute_Vector parameters(8);
  parameters[0].name_="FIFO";     parameters[0].value_="3";
  parameters[1].name_="Mode";     parameters[1].value_="Normal";
  parameters[2].name_="ShipTo";   parameters[2].value_="File";
  parameters[3].name_="Additional";parameters[3].value_=itoa(0);  // should be VCal value
  parameters[4].name_="Filename"; parameters[4].value_=mode_;
  parameters[5].name_="Channel"; parameters[5].value_="*";
  parameters[6].name_="Time";     parameters[6].value_="First";
  parameters[7].name_="VMEBaseAddress";

  
  //if(false) {  ////////////////////

  bool flag=false;
  Supervisors::iterator i_PixelFEDSupervisor; 
  string action="ReadDataAndErrorFIFO";
  if(sendingMode_!="yes"){
    
    vector<int> messageIDs;
    
    for (i_PixelFEDSupervisor=PixelFEDSupervisors_.begin();i_PixelFEDSupervisor!=PixelFEDSupervisors_.end();++i_PixelFEDSupervisor) {
      
      if (event_==nTriggersTotal_-1) {
	parameters[6].value_="Last";  //closes output file
      } 

      parameters[7].value_="*";
      
      int MessageID=send(i_PixelFEDSupervisor->second, action, flag, parameters);
      messageIDs.push_back(MessageID);
    }
    for (vector<int>::iterator itr=messageIDs.begin(), itr_end=messageIDs.end(); itr!=itr_end; ++itr){
      flag=waitForReply(*itr);
    }
  } else if(sendingMode_=="yes"){

    for (i_PixelFEDSupervisor=PixelFEDSupervisors_.begin();i_PixelFEDSupervisor!=PixelFEDSupervisors_.end();++i_PixelFEDSupervisor) {
      
      if (event_==nTriggersTotal_-1) {
	parameters[6].value_="Last";  //closes output file
      } 
      
      parameters[7].value_="*";
      string reply=Send(i_PixelFEDSupervisor->second, action, parameters);
      if (reply!="ReadDataAndErrorFIFODone") {
	diagService_->reportError("Reading SpyFIFO could not be done!", DIAGWARN);
	cout<<"   Reading SpyFIFO"<<" could not be done. reply="<<reply<<endl;
      }
    }
  }

  //} // end if False /////////////

  fedTimer_.stop();
  if (PRINT) printf("PixelGainAliveSCurveCalibration FED READ DONE\n");
  return (event_+1!=tempCalibObject->nTriggersTotal());
  
}

std::vector<std::string> PixelGainAliveSCurveCalibration::calibrated(){

  vector<string> tmp;

  return tmp;

}











