/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelLinearityVsVsfCalibration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"

#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"

#include <toolbox/convertstring.h>

using namespace pos;

//PixelLinearityVsVsfCalibration::PixelLinearityVsVsfCalibration() : PixelCalibrationBase()
//{
//  std::cout << "Greetings from the PixelLinearityVsVsfCalibration default constructor." << std::endl;
//}

PixelLinearityVsVsfCalibration::PixelLinearityVsVsfCalibration(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr)
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  std::cout << "Greetings from the PixelLinearityVsVsfCalibration copy constructor." << std::endl;
}

void PixelLinearityVsVsfCalibration::beginCalibration()
{
	// Check that PixelCalibConfiguration settings make sense.
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);
	
	if ( tempCalibObject->containsScan(k_DACName_Vsf) && !(tempCalibObject->scanValuesMixedAcrossROCs(k_DACName_Vsf) || tempCalibObject->singleROC()) )
	{
		std::cout << "ERROR:  Scans over "<<k_DACName_Vsf<<" must be run with scan values mixed across the ROCs on a channel, or must be run in SingleROC mode.  To run this calibration, add \"mix\" to the end of the line beginning \"Scan: Vsf\" in calib.dat, or run in SingleROC mode.  Now aborting..."<<std::endl;
		assert(0); // not elegant, maybe a better way to do this
	}
	
	if ( !(tempCalibObject->singleROC()) && tempCalibObject->maxNumHitsPerROC() > 2 )
	{
		std::cout << "ERROR:  FIFO3 will overflow with more than two hits on each ROC.  To run this calibration, use 2 or less hits per ROC, or use SingleROC mode.  Now aborting..."<<std::endl;
		assert(0); // not elegant, maybe a better way to do this
	}

}

bool PixelLinearityVsVsfCalibration::execute()
{
	reportProgress(0.05);
	
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);

	Attribute_Vector parametersToFED(2);
	parametersToFED[0].name_="WhatToDo";  parametersToFED[0].value_="RetrieveData";
	parametersToFED[1].name_="StateNum";
	
	unsigned int state = event_/(tempCalibObject->nTriggersPerPattern());
	parametersToFED[1].value_=itoa(state);

	// Configure all TBMs and ROCs according to the PixelCalibConfiguration settings, but only when it's time for a new configuration.
	if ( event_%(tempCalibObject->nTriggersPerPattern()) == 0 ) 
	{
		commandToAllFECCrates("CalibRunning");
	}

	// Send trigger to all TBMs and ROCs.
	sendTTCCalSync();

	// Read out data from each FED.
	commandToAllFEDCrates("FEDCalibrations", parametersToFED);

	unsigned int eventsSoFar = event_ + 1;
	return ( eventsSoFar < tempCalibObject->nTriggersTotal() ); // keep going until we've triggered enough times
}

void PixelLinearityVsVsfCalibration::endCalibration()
{
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);
	//assert( event_ == tempCalibObject->nTriggersTotal() );
	
	Attribute_Vector parametersToFED(2);
	parametersToFED[0].name_="WhatToDo";
	parametersToFED[1].name_="StateNum";  parametersToFED[1].value_="0";
	
	// Sending SOAP message to concerned FED Supervisors to analyse data
	parametersToFED[0].value_="Analyze";
	commandToAllFEDCrates("FEDCalibrations", parametersToFED);
}


std::vector<std::string> PixelLinearityVsVsfCalibration::calibrated(){

  std::vector<std::string> tmp;

  tmp.push_back("dac");

  return tmp;

}
