// Modified by Jennifer Vaughan 2007/06/01
// $Id: PixelThresholdCalDelayCalibrationFIFO1.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelThresholdCalDelayCalibrationFIFO1.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"

#include <toolbox/convertstring.h>

using namespace pos;
using namespace std;

//PixelThresholdCalDelayCalibrationFIFO1::PixelThresholdCalDelayCalibrationFIFO1() : PixelCalibrationBase()
//{
//  std::cout << "Greetings from the PixelThresholdCalDelayCalibrationFIFO1 default constructor." << std::endl;
//}

PixelThresholdCalDelayCalibrationFIFO1::PixelThresholdCalDelayCalibrationFIFO1(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  std::cout << "Greetings from the PixelThresholdCalDelayCalibrationFIFO1 copy constructor." << std::endl;
}

bool PixelThresholdCalDelayCalibrationFIFO1::execute()
{
  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  std::string mode = tempCalibObject->mode();

  unsigned int event=0;

  assert(tempCalibObject!=0);

  diagService_->reportError("PixelSupervisor:: --- Running for " + mode + "Calibration ---",DIAGINFO);
  //cout << "\nPixelSupervisor:: --- Running for "<<mode<<"Calibration ---";

  std::string reply;

  PixelTimer fecTimer, ttcTimer, fedTimer2, totalTimer;

  totalTimer.start();
  unsigned int nConfigs=tempCalibObject->nConfigurations();
  for (unsigned int j=0;j<nConfigs;++j) {

    if (j%(nConfigs/50)==0) {
      diagService_->reportError(stringF(j) + " configurations out of " + stringF(nConfigs) + " have been done.",DIAGDEBUG);
      std::cout<<(j*100)/nConfigs<<"% done."<<std::endl;
    }
     
    fecTimer.start();
    
    Supervisors::iterator i_PixelFECSupervisor;
    for (i_PixelFECSupervisor=PixelFECSupervisors_.begin();i_PixelFECSupervisor!=PixelFECSupervisors_.end();++i_PixelFECSupervisor) {
      
      Attribute_Vector parameters(1);
      parameters[0].name_="Event";
      parameters[0].value_=itoa(event);       // Event#
      
      if (Send(i_PixelFECSupervisor->second, "CalibRunningThreshold", parameters)!="CalibRunningThresholdDone") {
	diagService_->reportError("The error message = " + reply,DIAGERROR);
	//cout<<"The error message = "<<reply<<endl;
      }
      
    }    
  
    fecTimer.stop();

    for (unsigned int i_event=0;i_event<tempCalibObject->nTriggersPerPattern();++i_event)
      {
	
	Supervisors::iterator i_PixelFEDSupervisor;

	ttcTimer.start();

	sendTTCCalSync();

	ttcTimer.stop();

	fedTimer2.start();

	//cout<<"Tell the PixelFEDSupervisors to read data from their SpyFIFO lines."<<std::endl;
	for (i_PixelFEDSupervisor=PixelFEDSupervisors_.begin();i_PixelFEDSupervisor!=PixelFEDSupervisors_.end();++i_PixelFEDSupervisor) {
	    std::vector<std::pair<unsigned int, std::vector<unsigned int> > > theFEDCardAndChannels=tempCalibObject->fedCardsAndChannels(i_PixelFEDSupervisor->first, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_);
	    for (unsigned int k=0;k<theFEDCardAndChannels.size();++k)
	      {
		unsigned int fednumber=theFEDCardAndChannels.at(k).first;
		unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);

		Attribute_Vector parameters(2);
		parameters[0].name_="Event";
		parameters[0].value_=itoa(event);       // Event#

		parameters[1].name_="VMEBaseAddress";
		parameters[1].value_=itoa(vmeBaseAddress);

		reply=Send(i_PixelFEDSupervisor->second,"FEDCalibrations", parameters);
		
		if (reply!="ThresholdCalDelayDone") {
		   diagService_->reportError("Reading SpyFIFO on FED #" + stringF(fednumber) + " could not be done!",DIAGWARN);
		  //  cout<<"   Reading SpyFIFO on FED #"<<fednumber<<" could not be done!"<<endl;
		}
	      }
	  }

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


std::vector<std::string> PixelThresholdCalDelayCalibrationFIFO1::calibrated(){

  std::vector<std::string> tmp;

  tmp.push_back("dac");

  return tmp;

}
