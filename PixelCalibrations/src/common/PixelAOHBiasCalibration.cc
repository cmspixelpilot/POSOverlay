/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

/***************************************************************
PARAMETERS (specified in calib.dat)
(All parameters are optional.  If not specified, they default to the values given in PixelAOHBiasCalibrationParameters.h.)
Name                         Description
ScanMin                      low end of AOHBias scan range
ScanMax                      high end of AOHBias scan range
ScanStepSize                 step size for AOHBias scan
TargetBMin                   The TargetBMin and TargetBMax parameters specify the allowed range
TargetBMax                   for the coarse baseline adjustment at the end of this calibration.
MaxFEDReceiverInputOffset    largest allowed value of the FED receiver input offset, which can range from 0 to 15
SetAnalogInputBias           TBM setting to use for all channels -- should be set high
SetAnalogOutputBias          TBM setting to use for all channels -- should be set high
SetAnalogOutputGain          TBM setting to use for all channels -- should be set high
printFEDRawData              whether to print decoded transparent buffer
printFEDOffsetAdjustments    whether to print a message when the FED offsets are changed
printAOHBiasAdjustments      whether to print a message when the AOHBias values are changed during the baseline adjustment
plotFileType                 file type for output scan plots (eps, png, ...) or "none" to produce no plots
***************************************************************/

#include "PixelCalibrations/include/PixelAOHBiasCalibration.h"
#include "PixelCalibrations/include/PixelAOHBiasCalibrationParameters.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelPortCardSettingNames.h"

#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"

#include <toolbox/convertstring.h>

#include "iomanip"

using namespace pos;
using namespace std;
using namespace PixelAOHBiasCalibrationParameters;

//PixelAOHBiasCalibration::PixelAOHBiasCalibration() : PixelCalibrationBase()
//{
//  std::cout << "Greetings from the PixelAOHBiasCalibration default constructor." << std::endl;
//}

PixelAOHBiasCalibration::PixelAOHBiasCalibration(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr)
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  std::cout << "Greetings from the PixelAOHBiasCalibration copy constructor." << std::endl;
  
  currentStep_ = kBeginning;
}

void PixelAOHBiasCalibration::beginCalibration()
{
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);
	
	std::string maxFEDReceiverInputOffsetString = k_MaxFEDReceiverInputOffset_default;
	if ( tempCalibObject->parameterValue("MaxFEDReceiverInputOffset") != "" )
		maxFEDReceiverInputOffsetString = tempCalibObject->parameterValue("MaxFEDReceiverInputOffset");
	
	const std::set<PixelChannel>& channelsToCalibrate = tempCalibObject->channelList();
	
	// Set to 2V peak-to-peak ADC range, instead of 1V.
	Attribute_Vector parametersToSetADCRange(3);
	parametersToSetADCRange[0].name_="ADCRange"; parametersToSetADCRange[0].value_="2V";
	parametersToSetADCRange[1].name_="VMEBaseAddress";
	parametersToSetADCRange[2].name_="FEDChannel";
	for ( std::set<PixelChannel>::const_iterator channelsToCalibrate_itr = channelsToCalibrate.begin(); channelsToCalibrate_itr != channelsToCalibrate.end(); channelsToCalibrate_itr++ )
	{
		const PixelHdwAddress& channelHdwAddress = theNameTranslation_->getHdwAddress(*channelsToCalibrate_itr);
		unsigned int fednumber=channelHdwAddress.fednumber();
		unsigned int fedcrate=theFEDConfiguration_->crateFromFEDNumber(fednumber);
		unsigned int fedVMEBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
		unsigned int fedchannel=channelHdwAddress.fedchannel();
		parametersToSetADCRange[1].value_=itoa(fedVMEBaseAddress);
		parametersToSetADCRange[2].value_=itoa(fedchannel);
		
		xoap::MessageReference replyFromFED = SendWithSOAPReply(PixelFEDSupervisors_[fedcrate], "SetADC1V2VOneChannel", parametersToSetADCRange);
		Attribute_Vector returnValuesFromFED(1);
		returnValuesFromFED[0].name_="PreviousADCRange";
		Receive(replyFromFED, returnValuesFromFED);
		assert( returnValuesFromFED[0].value_ == "1V" || returnValuesFromFED[0].value_ == "2V" );
		if ( returnValuesFromFED[0].value_ == "1V" )
		{
			FEDChannelsWith1Vpp_.insert(std::pair<unsigned int, unsigned int>(fednumber, fedchannel));
		}
	}

	// Turn off automatic baseline correction.
	commandToAllFEDChannels("BaselineRelease");

	// Ensure that there will be no hits output.
	commandToAllFECCrates("ClrCalEnMass");
	commandToAllFECCrates("DisableHitsEnMass");

	// Set TBM DAC settings to uniform (and high) values.
	enum {kAnalogInputBias = 0, kAnalogOutputBias = 1, kAnalogOutputGain = 2};
	
	Attribute_Vector parametersToFEC(3);
	parametersToFEC[kAnalogInputBias].name_ ="AnalogInputBias";  parametersToFEC[kAnalogInputBias].value_ = k_SetAnalogInputBias_default;
	parametersToFEC[kAnalogOutputBias].name_="AnalogOutputBias"; parametersToFEC[kAnalogOutputBias].value_= k_SetAnalogOutputBias_default;
	parametersToFEC[kAnalogOutputGain].name_="AnalogOutputGain"; parametersToFEC[kAnalogOutputGain].value_= k_SetAnalogOutputGain_default;

	if ( tempCalibObject->parameterValue("SetAnalogInputBias")  != "" )
		parametersToFEC[kAnalogInputBias].value_  = tempCalibObject->parameterValue("SetAnalogInputBias");
	if ( tempCalibObject->parameterValue("SetAnalogOutputBias") != "" )
		parametersToFEC[kAnalogOutputBias].value_ = tempCalibObject->parameterValue("SetAnalogOutputBias");
	if ( tempCalibObject->parameterValue("SetAnalogOutputGain") != "" )
		parametersToFEC[kAnalogOutputGain].value_ = tempCalibObject->parameterValue("SetAnalogOutputGain");

	// Configure all TBMs with these DAC values.
	commandToAllFECCrates("SetTBMDACsEnMass", parametersToFEC);
	
	// Set all FED offsets to uniform values.
	SetAllFEDOffsets(maxFEDReceiverInputOffsetString, "255");
	
	// Reset all FEDs.  If you don't do this, the first time you read the transparent buffer it will contain old data.
	commandToAllFEDCrates("ResetFEDsEnMass");
}

std::string PixelAOHBiasCalibration::stepDescription(unsigned int step)
{
	if      ( step == kFindTimeSlots )         return "Find time slots";
	else if ( step == kMeasureBUBSeparation )  return "Measure B-UB separation vs. AOH bias";
	else if ( step == kFindSaturationPoints )  return "Calculate the new AOH bias values";
	else if ( step == kAdjustFEDOffsets )      return "Adjust the black level with FED offsets";
	else if ( step == kAdjustBlackWithAOHBias) return "Adjust the black level with AOH bias";
	else assert(0);
}

bool PixelAOHBiasCalibration::execute()
{
	PixelTimer timer;
	
	if      ( currentStep_ == kBeginning ) std::cout << "AOHBias: initialization complete" << std::endl;	
	else if ( currentStep_ == kDone )      std::cout << "AOHBias: all steps complete" << std::endl;
	else
	{
		std::cout << "AOHBias: now working on step " <<currentStep_<< " / " << kDone-1 << " : " << stepDescription(currentStep_) << std::endl;
		timer.start();
	}
	
	if ( currentStep_ == kBeginning )
	{
		// Do nothing and go on to the next step.
	}
	else if ( currentStep_ == kFindTimeSlots )
	{
		FindTimeSlots();
	}
	else if ( currentStep_ == kMeasureBUBSeparation )
	{
		MeasureBUBSeparation();
	}
	else if ( currentStep_ == kFindSaturationPoints )
	{
		FindSaturationPoints();
	}
	else if ( currentStep_ == kAdjustFEDOffsets )
	{
		AdjustFEDOffsets();
	}
	else if ( currentStep_ == kAdjustBlackWithAOHBias )
	{
		AdjustBlackWithAOHBias();
	}
	else if ( currentStep_ == kDone )
	{
		return false;
	}
	else assert(0);
	
	if ( currentStep_ != kBeginning )
	{
		timer.stop();
		std::cout << "                    ... step " <<currentStep_<< " / " << kDone-1 << " took " << timer.tottime() << " seconds" << endl;
	}
	
	currentStep_ += 1;
	return true;
}

Attribute_Vector PixelAOHBiasCalibration::SetupParametersToFED() const
{
	Attribute_Vector parametersToFED(k_NumVars);
	parametersToFED[k_WhatToDo].name_="WhatToDo";
	parametersToFED[k_AOHBias].name_="AOHBias";                         parametersToFED[k_AOHBias].value_="unset";
	parametersToFED[k_FEDNumber].name_="FEDNumber";                     parametersToFED[k_FEDNumber].value_="unset";
	parametersToFED[k_FEDChannel].name_="FEDChannel";                   parametersToFED[k_FEDChannel].value_="unset";
	
	return parametersToFED;
}

void PixelAOHBiasCalibration::FindTimeSlots()
{
	assert( currentStep_ == kFindTimeSlots );
	
	Attribute_Vector parametersToFED = SetupParametersToFED();
	
	// Loop over AOHBias values with full decoding.  Only keep data from valid decodes.  Keep track of the time slot where the UB data begin.
	parametersToFED[k_WhatToDo].value_="RetrieveDataFromFullDecode";
	AOHBiasLoop(parametersToFED);
}

void PixelAOHBiasCalibration::MeasureBUBSeparation()
{
	assert( currentStep_ == kMeasureBUBSeparation );
	
	Attribute_Vector parametersToFED = SetupParametersToFED();
	
	// Loop again over AOHBias values.  This time take values from the time slots determined above.  Keep data for all triggers.
	parametersToFED[k_WhatToDo].value_="RetrieveDataFromTimeSlots";
	AOHBiasLoop(parametersToFED);
}

void PixelAOHBiasCalibration::FindSaturationPoints()
{
	assert( currentStep_ == kFindSaturationPoints );
	
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);
	
	Attribute_Vector parametersToFED = SetupParametersToFED();

	const std::set<PixelChannel>& channelsToCalibrate = tempCalibObject->channelList();
	
	// Find the AOHBias values that saturate the B-UB difference, and set the AOHs to those values
	for ( std::set<PixelChannel>::const_iterator channelsToCalibrate_itr = channelsToCalibrate.begin(); channelsToCalibrate_itr != channelsToCalibrate.end(); channelsToCalibrate_itr++ )
	{
		// Get info about this channel.
		const PixelHdwAddress& channelHdwAddress = theNameTranslation_->getHdwAddress(*channelsToCalibrate_itr);
		const unsigned int fednumber = channelHdwAddress.fednumber();
		const unsigned int fedcrate=theFEDConfiguration_->crateFromFEDNumber( fednumber );
		const unsigned int channel = channelHdwAddress.fedchannel();
		
		const std::pair< std::string, int > portCardAndAOH = thePortcardMap_->PortCardAndAOH(*channelsToCalibrate_itr);
		const std::string portCardName = portCardAndAOH.first; assert(portCardName!="none");
		const int AOHNumber = portCardAndAOH.second;
		
		// Ask the FED supervisor what the AOH bias saturation point was.
		parametersToFED[k_WhatToDo].value_="FindSaturationPoint";
		parametersToFED[k_FEDNumber].value_ = itoa(fednumber);
		parametersToFED[k_FEDChannel].value_ = itoa(channel);
	 
	       	xoap::MessageReference replyFromFED = SendWithSOAPReply(PixelFEDSupervisors_[fedcrate], "FEDCalibrations", parametersToFED);
		Attribute_Vector returnValuesFromFED(1);
		returnValuesFromFED[0].name_="AOHBiasSaturationPoint";
		Receive(replyFromFED, returnValuesFromFED);
		std::string AOHBiasSaturationPoint = returnValuesFromFED[0].value_;
		parametersToFED[k_FEDNumber].value_="unset";
		parametersToFED[k_FEDChannel].value_="unset";
		
		//                                    contains "TimeSlotReadings"
		if ( AOHBiasSaturationPoint.find("TimeSlotReadings") != string::npos || AOHBiasSaturationPoint == "TimeSlotFluctuates" || AOHBiasSaturationPoint == "AOHBiasScanNotFound" || AOHBiasSaturationPoint == "NoVariation" || AOHBiasSaturationPoint == "NoSaturationPt" )
		{
			failures_[*channelsToCalibrate_itr] = AOHBiasSaturationPoint;
			continue;
		}
		
		AOHBiasSaturationPoints_[portCardName][AOHNumber] = atoi(AOHBiasSaturationPoint.c_str());
		currentAOHBiasValues_[portCardName][AOHNumber] = atoi(AOHBiasSaturationPoint.c_str());
		
		// Set the AOH bias to the saturation value.
		SetAOHBiasToCurrentValue(portCardName, AOHNumber);
	} // end of loop over channels to calibrate
}

void PixelAOHBiasCalibration::AdjustFEDOffsets()
{
	assert( currentStep_ == kAdjustFEDOffsets );
	
	// Set peak-to-peak ADC range back to 1V on channels where it was 1V originally
	Attribute_Vector parametersToSetADCRange(3);
	parametersToSetADCRange[0].name_="ADCRange"; parametersToSetADCRange[0].value_="1V";
	parametersToSetADCRange[1].name_="VMEBaseAddress";
	parametersToSetADCRange[2].name_="FEDChannel";
	for ( std::set< std::pair<unsigned int, unsigned int> >::iterator FEDChannelsWith1Vpp_itr = FEDChannelsWith1Vpp_.begin(); FEDChannelsWith1Vpp_itr != FEDChannelsWith1Vpp_.end(); ++FEDChannelsWith1Vpp_itr )
	{
		const unsigned int fednumber=FEDChannelsWith1Vpp_itr->first;
		const unsigned int fedcrate=theFEDConfiguration_->crateFromFEDNumber(fednumber);
		const unsigned int fedVMEBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
		const unsigned int fedchannel=FEDChannelsWith1Vpp_itr->second;
		parametersToSetADCRange[1].value_=itoa(fedVMEBaseAddress);
		parametersToSetADCRange[2].value_=itoa(fedchannel);
		
		xoap::MessageReference replyFromFED = SendWithSOAPReply(PixelFEDSupervisors_[fedcrate], "SetADC1V2VOneChannel", parametersToSetADCRange);
		Attribute_Vector returnValuesFromFED(1);
		returnValuesFromFED[0].name_="PreviousADCRange";
		Receive(replyFromFED, returnValuesFromFED);
		assert( returnValuesFromFED[0].value_ == "2V" );
	}

	// Set all FED offsets to uniform values.
	SetAllFEDOffsets("0", "127");

	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);
	
	const std::set<unsigned int> fedcrates=tempCalibObject->getFEDCrates(theNameTranslation_, theFEDConfiguration_);

	Attribute_Vector parametersToFED = SetupParametersToFED();

	// Raise FED receiver input offsets until all black levels are in or below the target black range.
	std::set<unsigned int> unfinishedCrates = fedcrates;
	unsigned int Ntriggers=tempCalibObject->nTriggersPerPattern();
	while ( !unfinishedCrates.empty() )
	{
		parametersToFED[k_WhatToDo].value_="MeasureBlackLevels";
		triggeringLoop(parametersToFED, unfinishedCrates, Ntriggers);
		parametersToFED[k_WhatToDo].value_="RaiseFEDReceiverInputOffsets";
		for (std::set<unsigned int>::iterator ifedcrate=fedcrates.begin();ifedcrate!=fedcrates.end();++ifedcrate)
		{
			if ( unfinishedCrates.find(*ifedcrate) == unfinishedCrates.end() ) continue; // skip if this crate is done already
			
			xoap::MessageReference replyFromFED = SendWithSOAPReply(PixelFEDSupervisors_[(*ifedcrate)], "FEDCalibrations", parametersToFED);
			Attribute_Vector returnValuesFromFED(1);
			returnValuesFromFED[0].name_="ThisCrateDone";
			Receive(replyFromFED, returnValuesFromFED);
			std::string reply = returnValuesFromFED[0].value_;
			assert (reply=="yes" || reply=="no");
			
			if (reply=="yes") unfinishedCrates.erase(*ifedcrate);
		}
	}
}

void PixelAOHBiasCalibration::AdjustBlackWithAOHBias()
{
	assert( currentStep_ == kAdjustBlackWithAOHBias );
	
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);

	bool printAOHBiasAdjustments = k_printAOHBiasAdjustments_default;
	if ( tempCalibObject->parameterValue("printAOHBiasAdjustments") == "yes" || tempCalibObject->parameterValue("printAOHBiasAdjustments") == "true" )
		printAOHBiasAdjustments = true;

	unsigned int target_B_min = k_TargetBMin_default;
	if ( tempCalibObject->parameterValue("TargetBMin") != "" )
		target_B_min = atoi( tempCalibObject->parameterValue("TargetBMin").c_str() );
	unsigned int target_B_max = k_TargetBMax_default;
	if ( tempCalibObject->parameterValue("TargetBMax") != "" )
		target_B_max = atoi( tempCalibObject->parameterValue("TargetBMax").c_str() );
	assert( target_B_max > target_B_min );

	unsigned int AOHBiasMin      = k_ScanMin_default;
	if ( tempCalibObject->parameterValue("ScanMin") != "" ) AOHBiasMin = atoi(tempCalibObject->parameterValue("ScanMin").c_str());
	unsigned int AOHBiasMax      = k_ScanMax_default;;
	if ( tempCalibObject->parameterValue("ScanMax") != "" ) AOHBiasMax = atoi(tempCalibObject->parameterValue("ScanMax").c_str());

	Attribute_Vector parametersToFED = SetupParametersToFED();

	const std::set<unsigned int> fedcrates=tempCalibObject->getFEDCrates(theNameTranslation_, theFEDConfiguration_);

	const std::set<PixelChannel>& channelsToCalibrate = tempCalibObject->channelList();


	// Adjust AOHBias values to put all black levels in range.
	std::set< std::pair< unsigned int, unsigned int> > unfinishedChannels;
	//                   FED number    FED channel
	std::map< unsigned int, std::map< unsigned int, double > > previousBlackLevels;
	//        FED number              FED channel
	std::map<unsigned int, std::set<unsigned int> > fedsAndChannelsMap = tempCalibObject->getFEDsAndChannels(theNameTranslation_);
	for ( std::map<unsigned int, std::set<unsigned int> >::iterator fednumber_itr = fedsAndChannelsMap.begin(); fednumber_itr != fedsAndChannelsMap.end(); ++fednumber_itr)
	{
		for ( std::set<unsigned int>::iterator channel_itr = fednumber_itr->second.begin(); channel_itr != fednumber_itr->second.end(); ++channel_itr)
		{
			unfinishedChannels.insert( std::pair< unsigned int, unsigned int>(fednumber_itr->first, *channel_itr) );
			previousBlackLevels[fednumber_itr->first][*channel_itr] = 0;
		}
	}
	bool firstIteration = true;
	while(!unfinishedChannels.empty())
	{
		parametersToFED[k_WhatToDo].value_="MeasureBlackLevels";
		unsigned int Ntriggers=tempCalibObject->nTriggersPerPattern();
		triggeringLoop(parametersToFED, fedcrates, Ntriggers);
		
		// On each channel, ask the FED supervisor for the black level, then adjust AOH bias accordingly.
		for ( std::set<PixelChannel>::const_iterator channelsToCalibrate_itr = channelsToCalibrate.begin(); channelsToCalibrate_itr != channelsToCalibrate.end(); channelsToCalibrate_itr++ )
		{
			// Get info about this channel.
			const PixelHdwAddress& channelHdwAddress = theNameTranslation_->getHdwAddress(*channelsToCalibrate_itr);
			const unsigned int fednumber = channelHdwAddress.fednumber();
			const unsigned int fedcrate=theFEDConfiguration_->crateFromFEDNumber( fednumber );
			const unsigned int channel = channelHdwAddress.fedchannel();
			const std::pair< unsigned int, unsigned int> fednumberChannelPair(fednumber, channel);
			if ( unfinishedChannels.find(fednumberChannelPair) == unfinishedChannels.end() ) continue; // skip channels that are already finished
			
			const std::pair< std::string, int > portCardAndAOH = thePortcardMap_->PortCardAndAOH(*channelsToCalibrate_itr);
			const std::string portCardName = portCardAndAOH.first; assert(portCardName!="none");
			const int AOHNumber = portCardAndAOH.second;
			
			// If this channel has an error on it, erase it from the list of unfinished channels and continue.
			if ( failures_.find(*channelsToCalibrate_itr) != failures_.end() )
			{
				unfinishedChannels.erase(fednumberChannelPair);
				continue;
			}
			
			// Ask the FED supervisor what the AOH bias saturation point was.
			parametersToFED[k_WhatToDo].value_="RetrieveBlackLevel";
			parametersToFED[k_FEDNumber].value_ = itoa(fednumber);
			parametersToFED[k_FEDChannel].value_ = itoa(channel);
			
			xoap::MessageReference replyFromFED = SendWithSOAPReply(PixelFEDSupervisors_[fedcrate], "FEDCalibrations", parametersToFED);
			Attribute_Vector returnValuesFromFED(1);
			returnValuesFromFED[0].name_="BlackLevel";
			Receive(replyFromFED, returnValuesFromFED);
			const std::string BlackLevel = returnValuesFromFED[0].value_;
			parametersToFED[k_FEDNumber].value_="unset";
			parametersToFED[k_FEDChannel].value_="unset";
			
			assert(BlackLevel != "ChannelNotFound");
			
			const double newBlackLevel = atof(BlackLevel.c_str());
			const double previousBlackLevel = previousBlackLevels[fednumber][channel];
			
			assert(target_B_max > target_B_min);
			// Decide whether to increase AOHBias, decrease it, or leave it alone.
			if      ( (newBlackLevel > (double)target_B_max) &&
						(firstIteration || previousBlackLevel > (double)target_B_min - (newBlackLevel-(double)target_B_max) )
					)
			{
				if ( currentAOHBiasValues_[portCardName][AOHNumber] == AOHBiasMin )
				{
					// This channel failed.
					if (printAOHBiasAdjustments) std::cout << "On FED "<<fednumber<<", channel "<<channel<<", failed to reach target black level -- cannot lower AOHBias below "<<AOHBiasMin<<" (black level = "<<newBlackLevel<<")\n";
					
					failures_[*channelsToCalibrate_itr] = "BlackTooHigh";
			
					currentAOHBiasValues_[portCardName].erase(AOHNumber);
					if ( currentAOHBiasValues_[portCardName].size() == 0 ) currentAOHBiasValues_.erase(portCardName);
					unfinishedChannels.erase(fednumberChannelPair);
					continue;
				}
				// Decrease AOHBias to lower black level
				if (printAOHBiasAdjustments) std::cout << "On FED "<<fednumber<<", channel "<<channel<<", lowering AOHBias from "<<currentAOHBiasValues_[portCardName][AOHNumber]<<" to "<<currentAOHBiasValues_[portCardName][AOHNumber]-1<<" (black level = "<<newBlackLevel<<")\n";
				currentAOHBiasValues_[portCardName][AOHNumber] -= 1;
			}
			else if ( (newBlackLevel < (double)target_B_min) &&
						(firstIteration || previousBlackLevel < (double)target_B_max + ((double)target_B_min-newBlackLevel) )
					)
			{
				if ( currentAOHBiasValues_[portCardName][AOHNumber] == AOHBiasMax )
				{
					// This channel failed.
					if (printAOHBiasAdjustments) std::cout << "On FED "<<fednumber<<", channel "<<channel<<", failed to reach target black level -- cannot raise AOHBias above "<<AOHBiasMax<<" (black level = "<<newBlackLevel<<")\n";
					
					failures_[*channelsToCalibrate_itr] = "BlackTooLow";
			
					currentAOHBiasValues_[portCardName].erase(AOHNumber);
					if ( currentAOHBiasValues_[portCardName].size() == 0 ) currentAOHBiasValues_.erase(portCardName);
					unfinishedChannels.erase(fednumberChannelPair);
					continue;
				}
				// Increase AOHBias to raise black level.
				if (printAOHBiasAdjustments) std::cout << "On FED "<<fednumber<<", channel "<<channel<<", raising AOHBias from "<<currentAOHBiasValues_[portCardName][AOHNumber]<<" to "<<currentAOHBiasValues_[portCardName][AOHNumber]+1<<" (black level = "<<newBlackLevel<<")\n";
				currentAOHBiasValues_[portCardName][AOHNumber] += 1;
			}
			else
			{
				// The black level is in the target range, or this is the best AOHBias setting.
				if (printAOHBiasAdjustments) std::cout << "On FED "<<fednumber<<", channel "<<channel<<", done adjusting AOHBias (AOHBias = "<<currentAOHBiasValues_[portCardName][AOHNumber]<<", black level = "<<newBlackLevel<<")\n";
				if ( currentAOHBiasValues_[portCardName][AOHNumber] < AOHBiasSaturationPoints_[portCardName][AOHNumber] )
				{
					if (printAOHBiasAdjustments) std::cout << "On FED "<<fednumber<<", channel "<<channel<<", WARNING: AOHBias was decreased to "<<currentAOHBiasValues_[portCardName][AOHNumber]<<" from the saturation value of "<<AOHBiasSaturationPoints_[portCardName][AOHNumber]<<".\n";
					char warningString[100]; sprintf(warningString, "AOHBiasLowered_%i->%i", AOHBiasSaturationPoints_[portCardName][AOHNumber], currentAOHBiasValues_[portCardName][AOHNumber]);
					warnings_[*channelsToCalibrate_itr] = warningString;
				}
				finalBlackLevels_.push_back(newBlackLevel);
				unfinishedChannels.erase(fednumberChannelPair);
				continue;
			}
			
			// Set the AOH bias to the new value.	
			SetAOHBiasToCurrentValue(portCardName, AOHNumber);
			
			previousBlackLevels[fednumber][channel] = newBlackLevel;
		} // end of loop over channels to calibrate
		firstIteration = false;
	}
	// finished adjusting AOHBias values

}

void PixelAOHBiasCalibration::endCalibration()
{
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);
	
	std::string maxFEDReceiverInputOffsetString = k_MaxFEDReceiverInputOffset_default;
	if ( tempCalibObject->parameterValue("MaxFEDReceiverInputOffset") != "" )
		maxFEDReceiverInputOffsetString = tempCalibObject->parameterValue("MaxFEDReceiverInputOffset");
	
	unsigned int AOHBiasMin      = k_ScanMin_default;
	if ( tempCalibObject->parameterValue("ScanMin") != "" ) AOHBiasMin = atoi(tempCalibObject->parameterValue("ScanMin").c_str());
	unsigned int AOHBiasMax      = k_ScanMax_default;;
	if ( tempCalibObject->parameterValue("ScanMax") != "" ) AOHBiasMax = atoi(tempCalibObject->parameterValue("ScanMax").c_str());

	int SaturationPointOffset = k_SaturationPointOffset_default;
	if ( tempCalibObject->parameterValue("SaturationPointOffset") != "" ) SaturationPointOffset = atoi(tempCalibObject->parameterValue("SaturationPointOffset").c_str());

	unsigned int target_B_min = k_TargetBMin_default;
	if ( tempCalibObject->parameterValue("TargetBMin") != "" )
		target_B_min = atoi( tempCalibObject->parameterValue("TargetBMin").c_str() );
	unsigned int target_B_max = k_TargetBMax_default;
	if ( tempCalibObject->parameterValue("TargetBMax") != "" )
		target_B_max = atoi( tempCalibObject->parameterValue("TargetBMax").c_str() );
	assert( target_B_max > target_B_min );
	
	const std::set<unsigned int> fedcrates=tempCalibObject->getFEDCrates(theNameTranslation_, theFEDConfiguration_);
	
	const std::set<PixelChannel>& channelsToCalibrate = tempCalibObject->channelList();
	
	// Turn automatic baseline correction back on.
	commandToAllFEDChannels("BaselineHold");

	std::cout << "\n-----Config files written-----\n";

	// Write port card config files and tabulate summary values.
	unsigned int succeededChannels = 0;
	for ( std::map< std::string, std::map< unsigned int, unsigned int > >::iterator portCardName_itr = currentAOHBiasValues_.begin(); portCardName_itr != currentAOHBiasValues_.end(); ++portCardName_itr )
	{
		std::string portCardName = portCardName_itr->first;
		std::map<std::string,PixelPortCardConfig*>::iterator mapNamePortCard_itr = getmapNamePortCard()->find(portCardName);
		assert( mapNamePortCard_itr != getmapNamePortCard()->end() );
		PixelPortCardConfig* thisPortCardConfig = mapNamePortCard_itr->second;
		
		for ( std::map< unsigned int, unsigned int >::iterator AOHNumber_itr = portCardName_itr->second.begin(); AOHNumber_itr != portCardName_itr->second.end(); ++AOHNumber_itr )
		{
			unsigned int AOHNumber = AOHNumber_itr->first;
			unsigned int AOHBiasAddress = thisPortCardConfig->AOHBiasAddressFromAOHNumber(AOHNumber);
			thisPortCardConfig->setdeviceValues(AOHBiasAddress, currentAOHBiasValues_[portCardName][AOHNumber]);
			
			// Tabulate summary values.
			succeededChannels++;
			AOHBiasSaturationPoints_Moments_.push_back( AOHBiasSaturationPoints_[portCardName][AOHNumber] );
			finalAOHBiasValues_Moments_.push_back( currentAOHBiasValues_[portCardName][AOHNumber] );
			finalMinusSaturationAOHBiasValues_Moments_.push_back(  (int)(currentAOHBiasValues_[portCardName][AOHNumber]) - (int)(AOHBiasSaturationPoints_[portCardName][AOHNumber]) );
		}
		
		thisPortCardConfig->writeASCII(outputDir());
		std::cout << "Wrote portcard_"+portCardName+".dat"<<endl;
	}
	
	// Write FED config files.
	double FEDReceiverInputOffsetMean = 0.;
	unsigned int FEDReceiverInputOffsetNum = 0;
	int FEDReceiverInputOffsetMin = 16;
	int FEDReceiverInputOffsetMax = -1;
	Attribute_Vector parametersToFED = SetupParametersToFED();
	parametersToFED[k_WhatToDo].value_="WriteConfigFiles";
	for (std::set<unsigned int>::iterator ifedcrate=fedcrates.begin();ifedcrate!=fedcrates.end();++ifedcrate)
	  {     
	    xoap::MessageReference replyFromFED = SendWithSOAPReply(PixelFEDSupervisors_[(*ifedcrate)], "FEDCalibrations", parametersToFED);
		
		Attribute_Vector returnValuesFromFED(4);
		returnValuesFromFED[0].name_="FEDReceiverInputOffsetMean";
		returnValuesFromFED[1].name_="FEDReceiverInputOffsetNum";
		returnValuesFromFED[2].name_="FEDReceiverInputOffsetMin";
		returnValuesFromFED[3].name_="FEDReceiverInputOffsetMax";
		Receive(replyFromFED, returnValuesFromFED);
		const double       crateMean = atof(returnValuesFromFED[0].value_.c_str());
		const unsigned int crateNum  = atoi(returnValuesFromFED[1].value_.c_str());
		const int          crateMin  = atoi(returnValuesFromFED[2].value_.c_str());
		const int          crateMax  = atoi(returnValuesFromFED[3].value_.c_str());
		
		if ( FEDReceiverInputOffsetNum+crateNum > 0 ) FEDReceiverInputOffsetMean = (((double)(FEDReceiverInputOffsetNum))*FEDReceiverInputOffsetMean + ((double)(crateNum))*crateMean)/((double)(FEDReceiverInputOffsetNum+crateNum));
		FEDReceiverInputOffsetNum += crateNum;
		if ( crateMin < FEDReceiverInputOffsetMin ) FEDReceiverInputOffsetMin = crateMin;
		if ( crateMax > FEDReceiverInputOffsetMax ) FEDReceiverInputOffsetMax = crateMax;
	}
	
	std::cout << "------------------------------\n";
	
	// Prepare summary.
	unsigned int totalChannelsInConfig = theNameTranslation_->getChannels(*theDetectorConfiguration_).size();
	unsigned int testedChannels = channelsToCalibrate.size();
	assert( totalChannelsInConfig >= testedChannels );
	assert( testedChannels >= succeededChannels );
	assert( testedChannels-succeededChannels == failures_.size() );
	assert( finalBlackLevels_.count() == succeededChannels );
	assert( warnings_.size() <= succeededChannels );
	
	// Print summary.
	cout << "\n";
	cout << "------------------------------------------------------------\n";
	cout << "                AOH Bias Calibration Report\n";
	cout << "------------------------------------------------------------\n";
	cout << setw(4)<<right << totalChannelsInConfig                  << " channels in the configuration, of these:\n";
	cout << setw(4)<<right << totalChannelsInConfig - testedChannels << " were not tested\n";
	cout << setw(4)<<right << succeededChannels-warnings_.size()      << " had a fully successful calibration\n";
	cout << setw(4)<<right << warnings_.size()                        << " succeeded, but with warnings\n";
	cout << setw(4)<<right << testedChannels-succeededChannels       << " failed the calibration\n";
	
	cout << "\nSaturation point offset = " << SaturationPointOffset << "\n";
	
	cout << "\nSummary of AOHBias (saturation point + offset) values:\n";
	cout <<   "mean     =" << setw(6)<<right << AOHBiasSaturationPoints_Moments_.mean()   << "\n";
	cout <<   "std.dev. =" << setw(6)<<right << AOHBiasSaturationPoints_Moments_.stddev() << "\n";
	cout <<   "smallest =" << setw(6)<<right << AOHBiasSaturationPoints_Moments_.min()    << "\n";
	cout <<   "largest  =" << setw(6)<<right << AOHBiasSaturationPoints_Moments_.max()    << "\n";
	
	cout << "\nSummary of final AOHBias settings:\n";
	cout <<   "mean     =" << setw(6)<<right << finalAOHBiasValues_Moments_.mean()   << "\n";
	cout <<   "std.dev. =" << setw(6)<<right << finalAOHBiasValues_Moments_.stddev() << "\n";
	cout <<   "smallest =" << setw(6)<<right << finalAOHBiasValues_Moments_.min()    << "\n";
	cout <<   "largest  =" << setw(6)<<right << finalAOHBiasValues_Moments_.max()    << "\n";

	cout << "\nSummary of changes from (saturation point + offset) to final setting:\n";
	cout <<   "mean          =" << setw(6)<<right << finalMinusSaturationAOHBiasValues_Moments_.mean()   << "\n";
	cout <<   "std.dev.      =" << setw(6)<<right << finalMinusSaturationAOHBiasValues_Moments_.stddev() << "\n";
	cout <<   "most negative =" << setw(6)<<right << finalMinusSaturationAOHBiasValues_Moments_.min()    << "\n";
	cout <<   "most positive =" << setw(6)<<right << finalMinusSaturationAOHBiasValues_Moments_.max()    << "\n";
	
	cout << "\nSummary of new FED receiver input offset settings (max allowed = "<<maxFEDReceiverInputOffsetString<<"):\n";
	cout <<   "mean     =" << setw(6)<<right;
	if (FEDReceiverInputOffsetNum>0) cout << FEDReceiverInputOffsetMean; else cout<<"N/A";
	cout << "\n";
	cout <<   "smallest =" << setw(6)<<right;
	if (FEDReceiverInputOffsetNum>0) cout << FEDReceiverInputOffsetMin; else cout<<"N/A";
	cout << "\n";
	cout <<   "largest  =" << setw(6)<<right;
	if (FEDReceiverInputOffsetNum>0) cout << FEDReceiverInputOffsetMax; else cout<<"N/A";
	cout << "\n";
	cout <<   "All channel offsets were set to 127.\n";
	
	cout << "\nSummary of final black levels (not including failed channels):\n";
	cout <<   "mean     =" << setw(6)<<right << finalBlackLevels_.mean()   << "\n";
	cout <<   "std.dev. =" << setw(6)<<right << finalBlackLevels_.stddev() << "\n";
	cout <<   "smallest =" << setw(6)<<right << finalBlackLevels_.min()    << "\n";
	cout <<   "largest  =" << setw(6)<<right << finalBlackLevels_.max()    << "\n";
	cout <<   "The target range was "<<target_B_min<<" to "<<target_B_max<<".\n";
	
	cout << "\nFailed channels (total of "<<failures_.size()<<"):\n";
	if (failures_.size() > 0)
	{
		cout <<   "Module               TBM Ch. Port card name   AOH# FED# FEDCh. Failure Mode\n";
		for ( std::map< PixelChannel, std::string>::iterator failures_itr = failures_.begin(); failures_itr != failures_.end(); failures_itr++ )
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
		cout <<   "Only[N]TimeSlotReadings = In the initial scan to determine time slots, the\n";
		cout <<   "                            transparent buffer could only be decoded for\n";
		cout <<   "                            N of the triggers.  Need at least 21.\n";
		cout <<   "TimeSlotFluctuates      = The TBM header and trailer time slots varied\n";
		cout <<   "                            too much during the initial scan.\n";
		cout <<   "AOHBiasScanNotFound     = The AOH bias scan produced no usable information.\n";
		cout <<   "NoVariation             = The B-UB difference did not vary during the scan.\n";
		cout <<   "                            (See plot.)\n";
		cout <<   "NoSaturationPt          = No B-UB saturation point was found.  (See plot.)\n";
		cout <<   "BlackTooLow             = Black level cannot be raised above "<<target_B_min<<",\n";
		cout <<   "                            even when AOHBias = "<<AOHBiasMax<<".\n";
		cout <<   "BlackTooHigh            = Black level cannot be lowered below "<<target_B_max<<",\n";
		cout <<   "                            even when AOHBias = "<<AOHBiasMin<<""<<endl;
	}
	
	if (warnings_.size() > 0)
	{
		cout << "\nWarning on "<<warnings_.size()<<" channels:\n";
		cout <<   "Module               TBM Ch. Port card name   AOH# FED# FEDCh. Failure Mode\n";
		for ( std::map< PixelChannel, std::string>::iterator warnings_itr = warnings_.begin(); warnings_itr != warnings_.end(); warnings_itr++ )
		{
			// Get info about this channel.
			const PixelHdwAddress& channelHdwAddress=theNameTranslation_->getHdwAddress(warnings_itr->first);
			const unsigned int fednumber = channelHdwAddress.fednumber();
			const unsigned int channel = channelHdwAddress.fedchannel();
			
			const std::pair< std::string, int > portCardAndAOH = thePortcardMap_->PortCardAndAOH(warnings_itr->first);
			const std::string portCardName = portCardAndAOH.first; assert(portCardName!="none");
			const int AOHNumber = portCardAndAOH.second;
			
			cout << setw(25)<<  left<< warnings_itr->first.module()
				<< setw(2) << right<< warnings_itr->first.TBMChannel() <<"  "
				<< setw(19)<<  left<< portCardName
				<< setw(2) << right<< AOHNumber <<"  "
				<< setw(3) << right<< fednumber <<"   "
				<< setw(3) << right<< channel   <<"  "
								<<left<< warnings_itr->second <<endl;
		}
		cout << "\nWarning type descriptions:\n";
		cout <<   "AOHBiasLowered_[S]->[F] = AOHBias was lowered from the saturation+offset\n";
		cout <<   "                          value of [S] to a final value of [F] in order\n";
		cout <<   "                          to bring the black level within range."<<endl;
	}
	
	cout << "-------------------------------------------------------------\n";
	cout << endl;
	
	// Clear member data.
	FEDChannelsWith1Vpp_.clear();
	AOHBiasSaturationPoints_.clear();
	currentAOHBiasValues_.clear();
	finalBlackLevels_.clear();
	failures_.clear();
	warnings_.clear();
	AOHBiasSaturationPoints_Moments_.clear();
	finalAOHBiasValues_Moments_.clear();
	finalMinusSaturationAOHBiasValues_Moments_.clear();
}

void PixelAOHBiasCalibration::SetAllFEDOffsets(std::string FEDReceiverInputOffset, std::string FEDChannelOffset)
{
	Attribute_Vector parametersToFED_SetFEDOffsets(2);
	parametersToFED_SetFEDOffsets[0].name_ = "FEDReceiverInputOffset";
	parametersToFED_SetFEDOffsets[1].name_ = "FEDChannelOffset";
	parametersToFED_SetFEDOffsets[0].value_ = FEDReceiverInputOffset;
	parametersToFED_SetFEDOffsets[1].value_ = FEDChannelOffset;
	commandToAllFEDCrates("SetFEDOffsetsEnMass", parametersToFED_SetFEDOffsets);
}

void PixelAOHBiasCalibration::AOHBiasLoop(Attribute_Vector parametersToFED)
{
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);
	
	unsigned int AOHBiasMin      = k_ScanMin_default;
	if ( tempCalibObject->parameterValue("ScanMin") != "" ) AOHBiasMin = atoi(tempCalibObject->parameterValue("ScanMin").c_str());
	unsigned int AOHBiasMax      = k_ScanMax_default;;
	if ( tempCalibObject->parameterValue("ScanMax") != "" ) AOHBiasMax = atoi(tempCalibObject->parameterValue("ScanMax").c_str());
	unsigned int AOHBiasStepSize = k_ScanStepSize_default;
	if ( tempCalibObject->parameterValue("ScanStepSize") != "" ) AOHBiasStepSize = atoi(tempCalibObject->parameterValue("ScanStepSize").c_str());
	
	const std::set<unsigned int> fedcrates=tempCalibObject->getFEDCrates(theNameTranslation_, theFEDConfiguration_);
	const std::set<unsigned int> TKFECcrates=tempCalibObject->getTKFECCrates(thePortcardMap_, *getmapNamePortCard(), theTKFECConfiguration_);
	
	for (unsigned int AOHBias = AOHBiasMin; AOHBias <= AOHBiasMax; AOHBias += AOHBiasStepSize)
	{	
		parametersToFED[k_AOHBias].value_=itoa(AOHBias);
		
		// Set AOH bias
		Attribute_Vector parametersToTKFEC(1);
		parametersToTKFEC[0].name_="AOHBias"; parametersToTKFEC[0].value_=itoa(AOHBias);
		commandToAllTKFECCrates("SetAOHBiasEnMass", parametersToTKFEC);

		unsigned int Ntriggers=tempCalibObject->nTriggersPerPattern();

		triggeringLoop(parametersToFED, fedcrates, Ntriggers);

	} // end of loop over AOHBias values
}

void PixelAOHBiasCalibration::triggeringLoop(Attribute_Vector parametersToFED, std::set<unsigned int> fedcrates, unsigned int Ntriggers)
{
	for (unsigned int i_event=0;i_event<Ntriggers;++i_event)
	{
		// Send trigger to all TBMs and ROCs.
		sendTTCCalSync();

		// Read out data from each FED.
		// This is not replaced with commandToAllFEDCrates("AOHBias", parametersToFED) because we may not be doing all FED crates.
		for (std::set<unsigned int>::iterator ifedcrate=fedcrates.begin();ifedcrate!=fedcrates.end();++ifedcrate)
		{
		  if (Send(PixelFEDSupervisors_[(*ifedcrate)], "FEDCalibrations", parametersToFED)!="AOHBiasDone")
			{
				diagService_->reportError("AOHBias in FED crate # " + stringF((*ifedcrate)) + " could not be done!",DIAGWARN);
			}
		}
	}
}

void PixelAOHBiasCalibration::SetAOHBiasToCurrentValue(std::string portCardName, int AOHNumber)
{
	Attribute_Vector parametersToTKFEC_SetAOHBiasOneChannel(3);
	parametersToTKFEC_SetAOHBiasOneChannel[0].name_="PortCardName";
	parametersToTKFEC_SetAOHBiasOneChannel[1].name_="AOHNumber";
	parametersToTKFEC_SetAOHBiasOneChannel[2].name_="AOHBias";
	parametersToTKFEC_SetAOHBiasOneChannel[0].value_=portCardName;
	parametersToTKFEC_SetAOHBiasOneChannel[1].value_=itoa(AOHNumber);
	parametersToTKFEC_SetAOHBiasOneChannel[2].value_=itoa(currentAOHBiasValues_[portCardName][AOHNumber]);
	
	std::map<std::string,PixelPortCardConfig*>::const_iterator mapNamePortCard_itr = getmapNamePortCard()->find(portCardName);
	assert( mapNamePortCard_itr != getmapNamePortCard()->end() );
	const PixelPortCardConfig* thisPortCardConfig = mapNamePortCard_itr->second;
	const std::string TKFECID = thisPortCardConfig->getTKFECID();
	unsigned int TKFECcrate = theTKFECConfiguration_->crateFromTKFECID(TKFECID);
	
	if (Send(PixelTKFECSupervisors_[TKFECcrate], "SetAOHBiasOneChannel", parametersToTKFEC_SetAOHBiasOneChannel)!="SetAOHBiasOneChannelDone")
	{
		diagService_->reportError("SetAOHBiasOneChannel could not be done!",DIAGERROR);
	}
}

std::vector<std::string> PixelAOHBiasCalibration::calibrated(){

  std::vector<std::string> tmp;

  tmp.push_back("portcard");
  tmp.push_back("fedcard");

  return tmp;

}
