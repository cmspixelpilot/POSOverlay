// $Id: PixelThresholdCalDelayCalibration.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2007-2008, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: Anders Ryd			                 		 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelVcThrCalibration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"

#include <toolbox/convertstring.h>
#include <iomanip>

using namespace pos;
using namespace std;

PixelVcThrCalibration::PixelVcThrCalibration(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  std::cout << "Greetings from the PixelVcThrCalibration copy constructor." << std::endl;
}

bool PixelVcThrCalibration::execute()
{

  //cout << "In PixelVcThrCalibration::execute" << endl;

  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  std::string mode = tempCalibObject->mode();

  unsigned int event=0;

  assert(tempCalibObject!=0);

  diagService_->reportError("PixelSupervisor:: --- Running for " + mode + "Calibration ---",DIAGINFO);
  //cout << "\nPixelSupervisor:: --- Running for "<<mode<<"Calibration ---";

  std::string reply;

  PixelTimer fecTimer, ttcTimer, fedTimer1, fedTimer2, totalTimer;

  totalTimer.start();
  unsigned int nConfigs=tempCalibObject->nConfigurations();
  for (unsigned int j=0;j<nConfigs;++j) {

    if (j%(nConfigs/50)==0) {
      diagService_->reportError(stringF(j) + " configurations out of " + stringF(nConfigs) + " have been done.",DIAGDEBUG);
      
      double percentageOfJob=((double)j*100)/((double)nConfigs);
      this->setPercentageOfJob(percentageOfJob);
		
      std::cout << setprecision(2) << percentageOfJob<< std::setprecision(6) << "% complete" << std::endl;
      // std::cout<<(j*100)/nConfigs<<"% done."<<std::endl;
    }
    
    fecTimer.start();
   
    
    Attribute_Vector parameters(1);
    parameters[0].name_="Event";
    parameters[0].value_=itoa(event);       // Event#
    std::string action="CalibRunningThreshold";
    sendToFEC(action, parameters);
    
    fecTimer.stop();

    for (unsigned int i_event=0;i_event<tempCalibObject->nTriggersPerPattern();++i_event)
      {

	ttcTimer.start();

	sendTTCCalSync();
	
	ttcTimer.stop();

	fedTimer2.start();
	
	Attribute_Vector parameters(1);
	parameters[0].name_="Event";
	parameters[0].value_=itoa(event);   
	  
	std::string action="FEDCalibrations";
	sendToFED(action, parameters);

       
	fedTimer2.stop();

	event++;

      }
  }
  
  totalTimer.stop();

  diagService_->reportError("--- Running for " + mode + " Done ---",DIAGINFO);
  // cout<<"--- Running for "<<mode<<" Done ---\n";

  cout << "Timing summary: total time=" << totalTimer.tottime() << endl;
  cout << "FEC total calls        :"<<fecTimer.ntimes()
       << " total time:"<<fecTimer.tottime()
       << "  avg time:"<<fecTimer.avgtime() << endl;
  cout << "TTC total calls        :"<<ttcTimer.ntimes()
       << " total time:"<<ttcTimer.tottime()
       << "  avg time:"<<ttcTimer.avgtime() << endl;
  cout << "FED readout total calls:"<<fedTimer2.ntimes()
       << " total time:"<<fedTimer2.tottime()
       << "  avg time:"<<fedTimer2.avgtime() << endl;

  return false;
  
}

std::vector<std::string> PixelVcThrCalibration::calibrated(){

  std::vector<std::string> tmp;

  tmp.push_back("dac");  

  return tmp;

}
