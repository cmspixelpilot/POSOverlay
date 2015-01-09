/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelTBMUBCalibration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "PixelCalibrations/include/PixelTBMDACScanInfo.h"

#include <toolbox/convertstring.h>

using namespace pos;

//PixelTBMUBCalibration::PixelTBMUBCalibration() : PixelCalibrationBase()
//{
//  std::cout << "Greetings from the PixelTBMUBCalibration default constructor." << std::endl;
//}

PixelTBMUBCalibration::PixelTBMUBCalibration(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr)
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  std::cout << "Greetings from the PixelTBMUBCalibration copy constructor." << std::endl;
  
}

void PixelTBMUBCalibration::beginCalibration()
{
	// Ensure that there will be no hits output.
	commandToAllFECCrates("ClrCalEnMass");
	commandToAllFECCrates("DisableHitsEnMass");
	
	// Reset all FEDs.  If you don't do this, the first time you read the transparent buffer it will contain old data.
	commandToAllFEDCrates("ResetFEDsEnMass");
}

bool PixelTBMUBCalibration::execute()
{
	// Collect info from the calib object.
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);
	
	PixelTBMDACScanInfo scanInfo( *tempCalibObject );
	
	const unsigned int NSteps = scanInfo.scanNumSteps();
	const unsigned int NTriggersPerStep=tempCalibObject->nTriggersPerPattern();
	const unsigned int NTriggersTotal = NSteps*NTriggersPerStep;
	
	reportProgress( 0.05, std::cout, NTriggersTotal );
	
	const unsigned int currentStep = event_/NTriggersPerStep; assert( currentStep < NSteps );
	
	// Write the new DAC settings, if this is the first event at this scan point.
	if ( event_%NTriggersPerStep == 0 )
	{
		Attribute_Vector parametersToFEC(3);
		parametersToFEC[PixelTBMDACScanInfo::kAnalogInputBias].name_  = scanInfo.name(PixelTBMDACScanInfo::kAnalogInputBias);
		parametersToFEC[PixelTBMDACScanInfo::kAnalogOutputBias].name_ = scanInfo.name(PixelTBMDACScanInfo::kAnalogOutputBias);
		parametersToFEC[PixelTBMDACScanInfo::kAnalogOutputGain].name_ = scanInfo.name(PixelTBMDACScanInfo::kAnalogOutputGain);
		parametersToFEC[PixelTBMDACScanInfo::kAnalogInputBias].value_  ="unchanged";
		parametersToFEC[PixelTBMDACScanInfo::kAnalogOutputBias].value_ ="unchanged";
		parametersToFEC[PixelTBMDACScanInfo::kAnalogOutputGain].value_ ="unchanged";
	
		for ( std::set<PixelTBMDACScanInfo::TBMDAC>::const_iterator dac_itr = scanInfo.DACsToScan().begin(); dac_itr != scanInfo.DACsToScan().end(); dac_itr++ )
		{
			parametersToFEC[*dac_itr].value_ = itoa(scanInfo.scanValue(*dac_itr, currentStep));
		}

		// Configure all TBMs with these DAC values.
		commandToAllFECCrates("SetTBMDACsEnMass", parametersToFEC);
	}
	
	// Send trigger to all TBMs and ROCs.
	sendTTCCalSync();
	
	// Read out data from each FED.
	Attribute_Vector parametersToFED(2);
	parametersToFED[0].name_="WhatToDo";  parametersToFED[0].value_ = "RetrieveData";
	parametersToFED[1].name_="Step";      parametersToFED[1].value_ = itoa(currentStep);

	commandToAllFEDCrates("FEDCalibrations", parametersToFED);

	const unsigned int eventsSoFar = event_ + 1;
	return ( eventsSoFar < NTriggersTotal ); // keep going until we've triggered enough times
} // end of execute() function

void PixelTBMUBCalibration::endCalibration()
{
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);
	
	Attribute_Vector parametersToFED(2);
	parametersToFED[0].name_="WhatToDo";  parametersToFED[0].value_ = "Analyse";
	parametersToFED[1].name_="Step";      parametersToFED[1].value_ = "0"; // not used, can set to anything
	
	// Sending SOAP message to concerned FED Supervisors to analyse data
	commandToAllFEDCrates("FEDCalibrations", parametersToFED);
}


std::vector<std::string> PixelTBMUBCalibration::calibrated(){

  std::vector<std::string> tmp;

  tmp.push_back("tbm");

  return tmp;

}
