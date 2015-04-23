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

#include "PixelCalibrations/include/PixelCalDelCalibration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"

#include <toolbox/convertstring.h>
#include <iomanip>
using namespace pos;
using namespace std;

PixelCalDelCalibration::PixelCalDelCalibration(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  std::cout << "Greetings from the PixelCalDelCalibration copy constructor." << std::endl;
}


void PixelCalDelCalibration::beginCalibration(){

  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  std::string mode = tempCalibObject->mode();

  diagService_->reportError("PixelSupervisor:: --- Running for " + mode + "Calibration ---",DIAGINFO);
  //cout << "\nPixelSupervisor:: --- Running for "<<mode<<"Calibration ---";


}


void PixelCalDelCalibration::endCalibration(){

  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  std::string mode = tempCalibObject->mode();

  diagService_->reportError("--- Running for " + mode + " Done ---",DIAGINFO);
  // cout<<"--- Running for "<<mode<<" Done ---\n";
  
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




bool PixelCalDelCalibration::execute(){

  //cout << "[PixelCalDelCalibration::execute()] event_="<<event_<<endl;

  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  if (event_%tempCalibObject->nTriggersPerPattern()==0) {

    unsigned int nConfigs=tempCalibObject->nConfigurations();
    unsigned int j=event_/tempCalibObject->nTriggersPerPattern();

    if (j%(nConfigs/50)==0) {
      diagService_->reportError(stringF(j) + " configurations out of " + stringF(nConfigs) + " have been done.",DIAGDEBUG);

      double percentageOfJob=((double)j*100)/((double)nConfigs);
      this->setPercentageOfJob(percentageOfJob);
		
      std::cout << setprecision(2) << percentageOfJob<< std::setprecision(6) << "% complete" << std::endl;
     
    }
    
    fecTimer_.start();
    nextFECConfig(event_);
    fecTimer_.stop();
    
  }
  
  ttcTimer_.start();
  //usleep(1000); // tests by Clemens
  sendTTCCalSync();
  ttcTimer_.stop();
  
  fedTimer_.start();
  readData(event_);
  fedTimer_.stop();
    
  return (event_+1!=tempCalibObject->nTriggersTotal());
    
}

std::vector<std::string> PixelCalDelCalibration::calibrated(){

  std::vector<std::string> tmp;

  tmp.push_back("dac");
  
  return tmp;

}
