// $Id: 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2007, Cornell U.			                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd	                 				 *
 *                                                                       *
 *************************************************************************/

#include "PixelCalibrations/include/Pixel2DEfficiencyScan.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"

#include <toolbox/convertstring.h>

using namespace pos;
using namespace std;

Pixel2DEfficiencyScan::Pixel2DEfficiencyScan(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  std::cout << "Greetings from the Pixel2DEfficiencyScan copy constructor." << std::endl;
}

void Pixel2DEfficiencyScan::beginCalibration(){

  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  std::string mode = tempCalibObject->mode();

  diagService_->reportError("PixelSupervisor:: --- Running for " + mode + "Calibration ---",DIAGINFO);
  //cout << "\nPixelSupervisor:: --- Running for "<<mode<<"Calibration ---";


}


void Pixel2DEfficiencyScan::endCalibration(){

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


bool Pixel2DEfficiencyScan::execute()
{

  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  if (event_%tempCalibObject->nTriggersPerPattern()==0) {

    unsigned int nConfigs=tempCalibObject->nConfigurations();
    unsigned int j=event_/tempCalibObject->nTriggersPerPattern();

    if (j%(nConfigs/50)==0) {
      diagService_->reportError(stringF(j) + " configurations out of " + stringF(nConfigs) + " have been done.",DIAGDEBUG);
      double percentageOfJob=((double)j*100)/((double)nConfigs);
      this->setPercentageOfJob(percentageOfJob);
      std::cout<<(j*100)/nConfigs<<"% done."<<std::endl;
    }
    
    fecTimer_.start();
    nextFECConfig(event_);
    fecTimer_.stop();
    
  }
  
  ttcTimer_.start();
  sendTTCCalSync();
  ttcTimer_.stop();
  
  fedTimer_.start();
  readData(event_);
  fedTimer_.stop();
    
  return (event_+1!=tempCalibObject->nTriggersTotal());
  
}

std::vector<std::string> Pixel2DEfficiencyScan::calibrated(){

  std::vector<std::string> tmp;

  return tmp;

}
