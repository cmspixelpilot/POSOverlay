/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelAOHGainCalibration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"

#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"

#include <toolbox/convertstring.h>

#include <iomanip>

using namespace pos;
using namespace std;

PixelAOHGainCalibration::PixelAOHGainCalibration(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr)
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  std::cout << "Greetings from the PixelAOHGainCalibration copy constructor." << std::endl;
  
	DACNames_[kAnalogInputBias]  = "AnalogInputBias";
	DACNames_[kAnalogOutputBias] = "AnalogOutputBias";
	DACNames_[kAnalogOutputGain] = "AnalogOutputGain";
}

std::string PixelAOHGainCalibration::DACName(unsigned int index) const
{
	std::map<unsigned int, std::string>::const_iterator foundIndex = DACNames_.find(index);
	assert( foundIndex != DACNames_.end() );
	return foundIndex->second;
}

void PixelAOHGainCalibration::beginCalibration()
{
	// Ensure that there will be no hits output.
	commandToAllFECCrates("ClrCalEnMass");
	commandToAllFECCrates("DisableHitsEnMass");
	
	// Reset all FEDs.  If you don't do this, the first time you read the transparent buffer it will contain old data.
	commandToAllFEDCrates("ResetFEDsEnMass");
	
	// Set TBM DACs to uniform (and high) values.
	Attribute_Vector parametersToFEC(3);
	parametersToFEC[kAnalogInputBias].name_ =DACName(kAnalogInputBias);  parametersToFEC[kAnalogInputBias].value_ ="160";
	parametersToFEC[kAnalogOutputBias].name_=DACName(kAnalogOutputBias); parametersToFEC[kAnalogOutputBias].value_="110";
	parametersToFEC[kAnalogOutputGain].name_=DACName(kAnalogOutputGain); parametersToFEC[kAnalogOutputGain].value_="240";

	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);

	if ( tempCalibObject->parameterValue("SetAnalogInputBias")  != "" )
		parametersToFEC[kAnalogInputBias].value_  = tempCalibObject->parameterValue("SetAnalogInputBias");
	if ( tempCalibObject->parameterValue("SetAnalogOutputBias") != "" )
		parametersToFEC[kAnalogOutputBias].value_ = tempCalibObject->parameterValue("SetAnalogOutputBias");
	if ( tempCalibObject->parameterValue("SetAnalogOutputGain") != "" )
		parametersToFEC[kAnalogOutputGain].value_ = tempCalibObject->parameterValue("SetAnalogOutputGain");
	
	commandToAllFECCrates("SetTBMDACsEnMass", parametersToFEC);
}

bool PixelAOHGainCalibration::execute()
{
	// Collect info from the calib object.
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);
	
	const unsigned int NSteps = 4; // 4 AOH gain settings
	const unsigned int NTriggersPerStep=tempCalibObject->nTriggersPerPattern();
	const unsigned int NTriggersTotal = NSteps*NTriggersPerStep;
	
	reportProgress( 0.05, std::cout, NTriggersTotal );
	
	const unsigned int currentStep = event_/NTriggersPerStep; assert( currentStep < NSteps );
	const unsigned int currentScanValue = currentStep; // goes from 0 to 3
	
	char AOHGainString[100];
	sprintf(AOHGainString, "%i", currentScanValue);
	
	// Write the new DAC setting, if this is the first event at this scan point.
	if ( event_%NTriggersPerStep == 0 )
	{
		Attribute_Vector parametersToTKFEC(1);
		parametersToTKFEC[0].name_ = "AOHGain";
		parametersToTKFEC[0].value_ = AOHGainString;
		
		commandToAllTKFECCrates("SetAOHGainEnMass", parametersToTKFEC);
	}
	
	// Send trigger to all TBMs and ROCs.
	sendTTCCalSync();
	
	// Read out data from each FED.
	Attribute_Vector parametersToFED(3);
	parametersToFED[0].name_="WhatToDo";  parametersToFED[0].value_ = "RetrieveData";
	parametersToFED[1].name_="AOHGain";   parametersToFED[1].value_ = AOHGainString;
	parametersToFED[2].name_="Channel";   parametersToFED[2].value_ = "unset";

	commandToAllFEDCrates("FEDCalibrations", parametersToFED);

	const unsigned int eventsSoFar = event_ + 1;
	return ( eventsSoFar < NTriggersTotal ); // keep going until we've triggered enough times
} // end of execute() function

void PixelAOHGainCalibration::endCalibration()
{
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);
	
	unsigned int MaxTBMUB = 100;
	if ( tempCalibObject->parameterValue("MaxTBMUB") != "" )
	{
		MaxTBMUB = atoi(tempCalibObject->parameterValue("MaxTBMUB").c_str());
	}
	
	Attribute_Vector parametersToFED(3);
	parametersToFED[0].name_="WhatToDo";  parametersToFED[0].value_ = "Analyze";
	parametersToFED[1].name_="AOHGain";   parametersToFED[1].value_ = "0"; // not used, can set to anything
	parametersToFED[2].name_="Channel";   parametersToFED[2].value_ = "unset";
	
	// Sending SOAP message to concerned FED Supervisors to analyse data
	commandToAllFEDCrates("FEDCalibrations", parametersToFED);
	
	// Retrieve the new TBM gain settings and any errors.
	std::map< std::string, std::map< unsigned int, unsigned int > > AOHGains;
	std::map< PixelChannel, std::string > failures;
	
	const std::set<PixelChannel>& channelsToCalibrate = tempCalibObject->channelList();
	for ( std::set<PixelChannel>::const_iterator channelsToCalibrate_itr = channelsToCalibrate.begin(); channelsToCalibrate_itr != channelsToCalibrate.end(); channelsToCalibrate_itr++ )
	{
		// Get info about this channel.
		const PixelHdwAddress& channelHdwAddress = theNameTranslation_->getHdwAddress(*channelsToCalibrate_itr);
		const unsigned int fednumber = channelHdwAddress.fednumber();
		const unsigned int fedcrate=theFEDConfiguration_->crateFromFEDNumber( fednumber );
		
		const std::pair< std::string, int > portCardAndAOH = thePortcardMap_->PortCardAndAOH(*channelsToCalibrate_itr);
		const std::string portCardName = portCardAndAOH.first; assert(portCardName!="none");
		const int AOHNumber = portCardAndAOH.second;
		
		// Ask the FED supervisor what the new AOH gain value is, and whether there was an error.
		parametersToFED[0].value_ = "RetrieveAOHGain";
		parametersToFED[1].value_ = "0"; // not used, can set to anything
		parametersToFED[2].value_ = channelsToCalibrate_itr->channelname();
		xoap::MessageReference replyFromFED = SendWithSOAPReply(PixelFEDSupervisors_[fedcrate], "FEDCalibrations", parametersToFED);
		Attribute_Vector returnValuesFromFED(2);
		returnValuesFromFED[0].name_="AOHGain";
		returnValuesFromFED[1].name_="Error";
		Receive(replyFromFED, returnValuesFromFED);
		
		assert( returnValuesFromFED[0].value_ != "none" || returnValuesFromFED[1].value_ != "none");
		if (returnValuesFromFED[0].value_ != "none") AOHGains[portCardName][AOHNumber] = atoi(returnValuesFromFED[0].value_.c_str());
		if (returnValuesFromFED[1].value_ != "none") failures[*channelsToCalibrate_itr] = returnValuesFromFED[1].value_;
	}
	
	Moments AOHGains_Moments, AOHGainChanges_Moments;
	
	// Write new port card config files.
	for ( std::map< std::string, std::map< unsigned int, unsigned int > >::iterator portCardName_itr = AOHGains.begin(); portCardName_itr != AOHGains.end(); ++portCardName_itr )
	{
		const std::string portCardName = portCardName_itr->first;
		const std::map<std::string,PixelPortCardConfig*>::iterator mapNamePortCard_itr = getmapNamePortCard()->find(portCardName);
		assert( mapNamePortCard_itr != getmapNamePortCard()->end() );
		PixelPortCardConfig* thisPortCardConfig = mapNamePortCard_itr->second;
		
		for ( std::map< unsigned int, unsigned int >::iterator AOHNumber_itr = portCardName_itr->second.begin(); AOHNumber_itr != portCardName_itr->second.end(); ++AOHNumber_itr )
		{
			const unsigned int AOHNumber = AOHNumber_itr->first;
			const unsigned int newAOHGain = AOHNumber_itr->second;
			const unsigned int oldAOHGain = thisPortCardConfig->getAOHGain(AOHNumber);
			thisPortCardConfig->setAOHGain(AOHNumber, AOHNumber_itr->second);
			
			// Tabulate summary values.
			AOHGains_Moments.push_back(newAOHGain);
			AOHGainChanges_Moments.push_back((int)newAOHGain-(int)oldAOHGain);
		}
		
		thisPortCardConfig->writeASCII(outputDir());
		std::cout << "Wrote portcard_"+portCardName+".dat\n";
	}
	
	// Print summary.
	unsigned int totalChannelsInConfig = theNameTranslation_->getChannels(*theDetectorConfiguration_).size();
	unsigned int testedChannels = channelsToCalibrate.size();
	cout << "\n";
	cout << "------------------------------------------------------------\n";
	cout << "                AOH Gain Calibration Report\n";
	cout << "------------------------------------------------------------\n";
	cout << setw(4)<<right << totalChannelsInConfig                  << " channels in the configuration, of these:\n";
	cout << setw(4)<<right << totalChannelsInConfig - testedChannels << " were not tested\n";
	cout << setw(4)<<right << testedChannels-failures.size()         << " had a successful calibration\n";
	cout << setw(4)<<right << failures.size()                        << " failed the calibration\n";
	
	cout << "\nSummary of new AOH gain values (including failed channels):\n";
	cout <<   "mean     =" << setw(6)<<right << AOHGains_Moments.mean()   << "\n";
	cout <<   "std.dev. =" << setw(6)<<right << AOHGains_Moments.stddev() << "\n";
	cout <<   "smallest =" << setw(6)<<right << AOHGains_Moments.min()    << "\n";
	cout <<   "largest  =" << setw(6)<<right << AOHGains_Moments.max()    << "\n";

	cout << "\nSummary of changes from old settings (new - old):\n";
	cout <<   "mean          =" << setw(6)<<right << AOHGainChanges_Moments.mean()   << "\n";
	cout <<   "std.dev.      =" << setw(6)<<right << AOHGainChanges_Moments.stddev() << "\n";
	cout <<   "most negative =" << setw(6)<<right << AOHGainChanges_Moments.min()    << "\n";
	cout <<   "most positive =" << setw(6)<<right << AOHGainChanges_Moments.max()    << "\n";
	
	cout << "\nFailed channels (total of "<<failures.size()<<"):\n";
	if (failures.size() > 0)
	{
		cout <<   "Module               TBM Ch. Port card name   AOH# FED# FEDCh. Failure Mode\n";
		for ( std::map< PixelChannel, std::string>::iterator failures_itr = failures.begin(); failures_itr != failures.end(); failures_itr++ )
		{
			// Get info about this channel.
			const PixelHdwAddress& channelHdwAddress=theNameTranslation_->getHdwAddress(failures_itr->first);
			const unsigned int fednumber = channelHdwAddress.fednumber();
			const unsigned int channel = channelHdwAddress.fedchannel();
			
			const std::pair< std::string, int > portCardAndAOH = thePortcardMap_->PortCardAndAOH(failures_itr->first);
			const std::string portCardName = portCardAndAOH.first; assert(portCardName!="none");
			const int AOHNumber = portCardAndAOH.second;
			
			cout << setw(25)<<  left<< failures_itr->first.module()
				<< setw(2) << right<< failures_itr->first.TBMChannel() <<"  "
				<< setw(19)<<  left<< portCardName
				<< setw(2) << right<< AOHNumber <<"  "
				<< setw(3) << right<< fednumber <<"   "
				<< setw(3) << right<< channel   <<"  "
								<<left<< failures_itr->second <<endl;
		}
		cout << "\nFailure mode descriptions:\n";
		cout <<   "NoValidDecodedTriggers = This channel was never successfully decoded on any\n";
		cout <<   "                           trigger.  No new AOH gain value." << endl;
		cout <<   "UBTooHigh              = For all AOH gain values,\n";
		cout <<   "                           the UB level was above the target ("<<MaxTBMUB<<")." << endl;
	}
	
	cout << "--------------------------------------" << endl;
}

std::vector<std::string> PixelAOHGainCalibration::calibrated(){

  std::vector<std::string> tmp;

  tmp.push_back("portcard");

  return tmp;

}
