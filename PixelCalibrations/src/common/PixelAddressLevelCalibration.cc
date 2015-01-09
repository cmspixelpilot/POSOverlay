/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/
// $Id: PixelAddressLevelCalibration.cc,v 1.23 2009/09/17 10:21:05 kreis Exp $: PixelAddressLevelCalibration.cc,v 1.1 


#include "PixelCalibrations/include/PixelAddressLevelCalibration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"


#include <toolbox/convertstring.h>
#include <iomanip>
using namespace pos;
using namespace std;


PixelAddressLevelCalibration::PixelAddressLevelCalibration(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  std::cout << "[PixelAddressLevelCalibration::PixelAddressLevelCalibration]" 
	    << std::endl;
}


void PixelAddressLevelCalibration::beginCalibration(){

  tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  totalTimer.start();

  cout << "[PixelAddressLevelCalibration::beginCalibration()] nconfigs = "
       << tempCalibObject->nConfigurations() << endl;

  fecTimer.start();
  
  std::string action="ClrCalEnMass";
  sendToFEC(action);

  fecTimer.stop();

}


void PixelAddressLevelCalibration::endCalibration(){

  totalTimer.stop();

  cout << "Timing summary: total time=" << totalTimer.tottime() << endl;
  cout << "FEC total calls        :"<<fecTimer.ntimes()<<" total time:"<<fecTimer.tottime()
       << "  avg time:"<<fecTimer.avgtime() << endl;
  cout << "FED total calls        :"<<fedTimer.ntimes()
       <<" total time:"<<fedTimer.tottime()
       << "  avg time:"<<fedTimer.avgtime() << endl;
  cout << "TTC total calls        :"<<ttcTimer.ntimes()
       <<" total time:"<<ttcTimer.tottime()
       << "  avg time:"<<ttcTimer.avgtime() << endl;

}


bool PixelAddressLevelCalibration::execute(){
  
  //cout << "[PixelAddressLevelCalibration::execute] entered"<<endl;

  
  Attribute_Vector parametersToFED(2);
  parametersToFED[0].name_="Hits";	parametersToFED[0].value_="No";
  parametersToFED[1].name_="First";	parametersToFED[1].value_="True";

  unsigned int nPrint=(int)tempCalibObject->nTriggersTotal()/20;
  if (nPrint==0) nPrint=1;

  unsigned int Ntriggers=tempCalibObject->nTriggersPerPattern();
  
  int j=event_-2;

  //The first 2 iterations have no pixel hits. This is to 
  //help PixelFEDSupervisor identify the B and UB levels.    
  if (j>=0){ 
    parametersToFED[0].value_="Yes";      
    if (j%nPrint==0) {
      
      double percentageOfJob=((double)100*j)/((double)tempCalibObject->nTriggersTotal());
      this->setPercentageOfJob(percentageOfJob);
		
      std::cout << "[PixelAddressLevelCalibration::execute()] " << setprecision(2) << percentageOfJob<< std::setprecision(6) << "% complete" << endl;
    
    }
  }

  if (j>=0&&j%Ntriggers==0){
    fecTimer.start();

    std::string action="CalibRunning";
    sendToFEC(action);
    
    fecTimer.stop();
  }

  ttcTimer.start();
  sendTTCCalSync();
  ttcTimer.stop();
      
  fedTimer.start();

  std::string action="FEDCalibrations";
  sendToFED(action,parametersToFED);

  fedTimer.stop();

  parametersToFED[1].value_="False";
 
  return (j+1!=(int)tempCalibObject->nTriggersTotal());

  
  

}

std::vector<std::string> PixelAddressLevelCalibration::calibrated(){

  std::vector<std::string> tmp;

  tmp.push_back("fedcard");

  return tmp;

}

