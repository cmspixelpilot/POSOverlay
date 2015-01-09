/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelROCUBEqualizationCalibration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"

#include <toolbox/convertstring.h>

using namespace pos;

//PixelROCUBEqualizationCalibration::PixelROCUBEqualizationCalibration() : PixelCalibrationBase()
//{
//  std::cout << "Greetings from the PixelROCUBEqualizationCalibration default constructor." << std::endl;
//}

PixelROCUBEqualizationCalibration::PixelROCUBEqualizationCalibration(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr)
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  std::cout << "Greetings from the PixelROCUBEqualizationCalibration copy constructor." << std::endl;
}

void PixelROCUBEqualizationCalibration::beginCalibration()
{
	// Reset all FEDs.  If you don't do this, the first time you read the transparent buffer it will contain old data.
	commandToAllFEDCrates("ResetFEDsEnMass");
}

bool PixelROCUBEqualizationCalibration::execute()
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
	// Clear any hits that may be specified.
	if ( event_%(tempCalibObject->nTriggersPerPattern()) == 0 ) 
	{
		commandToAllFECCrates("CalibRunning");

		// Ensure that there will be no hits output.
		commandToAllFECCrates("ClrCalEnMass");
		commandToAllFECCrates("DisableHitsEnMass");
	}

	// Send trigger to all TBMs and ROCs.
	sendTTCCalSync();

	// Read out data from each FED.
	commandToAllFEDCrates("FEDCalibrations", parametersToFED);
	unsigned int eventsSoFar = event_ + 1;
	return ( eventsSoFar < tempCalibObject->nTriggersTotal() ); // keep going until we've triggered enough times
}

void PixelROCUBEqualizationCalibration::endCalibration()
{
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);
	if ( event_ != tempCalibObject->nTriggersTotal() )
	{
		std::cout << "ERROR: Calibration did not finish.  To eliminate this message and analyze the data, let it run to completion." << std::endl;
		return;
	}
	
	Attribute_Vector parametersToFED(2);
	parametersToFED[0].name_="WhatToDo";
	parametersToFED[1].name_="StateNum";  parametersToFED[1].value_="0";
	
	// Sending SOAP message to concerned FED Supervisors to analyse data
	parametersToFED[0].value_="Analyze";
	commandToAllFEDCrates("FEDCalibrations", parametersToFED);
}

std::vector<std::string> PixelROCUBEqualizationCalibration::calibrated(){

  std::vector<std::string> tmp;

  tmp.push_back("dac");

  return tmp;

}
