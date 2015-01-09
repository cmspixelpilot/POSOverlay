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
(All parameters are optional.  If not specified, they default to the values given in PixelAOHAndFEDChannelMappingTestParameters.h.)
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

#include "PixelCalibrations/include/PixelAOHAndFEDChannelMappingTest.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"

#include <toolbox/convertstring.h>

#include "iomanip"

using namespace pos;
using namespace std;

//PixelAOHAndFEDChannelMappingTest::PixelAOHAndFEDChannelMappingTest() : PixelCalibrationBase()
//{
//  std::cout << "Greetings from the PixelAOHAndFEDChannelMappingTest default constructor." << std::endl;
//}

PixelAOHAndFEDChannelMappingTest::PixelAOHAndFEDChannelMappingTest(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr)
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  std::cout << "Greetings from the PixelAOHAndFEDChannelMappingTest copy constructor." << std::endl;
}

bool PixelAOHAndFEDChannelMappingTest::execute()
{
	std::cout << "\nNow starting PixelAOHAndFEDChannelMappingTest::execute().\n";
	
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);

	unsigned int AOHBiasMin      = 0;
	if ( tempCalibObject->parameterValue("ScanMin") != "" ) AOHBiasMin = atoi(tempCalibObject->parameterValue("ScanMin").c_str());
	unsigned int AOHBiasMax      = 50;
	if ( tempCalibObject->parameterValue("ScanMax") != "" ) AOHBiasMax = atoi(tempCalibObject->parameterValue("ScanMax").c_str());
	unsigned int AOHBiasStepSize = 5;
	if ( tempCalibObject->parameterValue("ScanStepSize") != "" ) AOHBiasStepSize = atoi(tempCalibObject->parameterValue("ScanStepSize").c_str());

	const std::set<PixelChannel>& channelsToTest = tempCalibObject->channelList();

	// Set to 2V peak-to-peak ADC range, instead of 1V.
	Attribute_Vector parametersToSetADCRange(1);
	parametersToSetADCRange[0].name_="ADCRange"; parametersToSetADCRange[0].value_="2V";
	commandToAllFEDCrates("SetADC1V2VEnMass", parametersToSetADCRange);

	// Turn off automatic baseline correction.
	commandToAllFEDChannels("BaselineRelease");

	// Set all FED offsets to uniform and high values, to make sure that the black level isn't stuck at the top of the range.
	Attribute_Vector parametersToFED_SetFEDOffsets(2);
	parametersToFED_SetFEDOffsets[0].name_ = "FEDReceiverInputOffset";
	parametersToFED_SetFEDOffsets[1].name_ = "FEDChannelOffset";
	parametersToFED_SetFEDOffsets[0].value_ = "8";
	parametersToFED_SetFEDOffsets[1].value_ = "255";
	commandToAllFEDCrates("SetFEDOffsetsEnMass", parametersToFED_SetFEDOffsets);

	// Ensure that there will be no hits output.
	commandToAllFECCrates("ClrCalEnMass");
	commandToAllFECCrates("DisableHitsEnMass");

	// Reset all FEDs.  If you don't do this, the first time you read the transparent buffer it will contain old data.
	commandToAllFEDCrates("ResetFEDsEnMass");
	
	// info for summary
	std::map< PixelChannel, std::string > failures;
	std::map< PixelChannel, std::string > unconnected;
	// Loop over channels and test the connectivity of each one.
	for ( std::set<PixelChannel>::const_iterator channelsToTest_itr = channelsToTest.begin(); channelsToTest_itr != channelsToTest.end(); channelsToTest_itr++ )
	{
		// Get info about this channel.
		const PixelHdwAddress& channelHdwAddress=theNameTranslation_->getHdwAddress(*channelsToTest_itr);
		const unsigned int fednumber = channelHdwAddress.fednumber();
		const unsigned int fedcrate = theFEDConfiguration_->crateFromFEDNumber(fednumber);
		const unsigned int fedchannel = channelHdwAddress.fedchannel();
		
		const std::pair< std::string, int > portCardAndAOH = thePortcardMap_->PortCardAndAOH(*channelsToTest_itr);
		const std::string portCardName = portCardAndAOH.first; assert(portCardName!="none");
		const int AOHNumber = portCardAndAOH.second;
		// Record black levels for each AOH bias.
		for (unsigned int AOHBias = AOHBiasMin; AOHBias <= AOHBiasMax; AOHBias += AOHBiasStepSize)
		{	
			// Set AOH bias
			SetAOHBiasOneChannel(portCardName, AOHNumber, AOHBias);
			unsigned int Ntriggers=tempCalibObject->nTriggersPerPattern();
	
			// Record black level on this FED channel.
			triggeringLoop(fedcrate, fednumber, fedchannel, AOHBias, Ntriggers);
		} // end of loop over AOHBias values
		
		// Set AOH bias back to the default value.
		std::map<std::string,PixelPortCardConfig*>::iterator mapNamePortCard_itr = getmapNamePortCard()->find(portCardName);
		assert( mapNamePortCard_itr != getmapNamePortCard()->end() );
		PixelPortCardConfig* thisPortCardConfig = mapNamePortCard_itr->second;
		unsigned int defaultAOHBias = thisPortCardConfig->getAOHBias(AOHNumber);
		SetAOHBiasOneChannel(portCardName, AOHNumber, defaultAOHBias);
		
		// Ask if this channel's black level varied as AOH bias changed.
		Attribute_Vector parametersToFED(4);
		parametersToFED[0].name_ = "WhatToDo"; parametersToFED[0].value_ = "CheckForChangeInBlackLevel";
		parametersToFED[1].name_ = "fednumber"; parametersToFED[1].value_ = itoa(fednumber);
		parametersToFED[2].name_ = "fedchannel"; parametersToFED[2].value_ = itoa(fedchannel);
		parametersToFED[3].name_ = "AOHBias"; parametersToFED[3].value_ = "null";
	
		xoap::MessageReference replyFromFED = SendWithSOAPReply(PixelFEDSupervisors_[fedcrate], "FEDCalibrations", parametersToFED);
		
		Attribute_Vector returnValuesFromFED(1);
		returnValuesFromFED[0].name_="BlackLevelChanged";
		Receive(replyFromFED, returnValuesFromFED);
		if ( returnValuesFromFED[0].value_ == "NoData" || returnValuesFromFED[0].value_ == "StuckAt0" || returnValuesFromFED[0].value_ == "StuckAt1023")
		{
			failures[*channelsToTest_itr] = returnValuesFromFED[0].value_;
		}
		else if (returnValuesFromFED[0].value_ == "no")
		{
			unconnected[*channelsToTest_itr] = "AOHNotConnected";
		}
		else if (returnValuesFromFED[0].value_ == "yes")
		{
		}
		else assert(0);
	}

	// Prepare summary.
	unsigned int totalChannelsInConfig = theNameTranslation_->getChannels(*theDetectorConfiguration_).size();
	unsigned int testedChannels = channelsToTest.size();
	
	// Print summary.
	cout << "\n";
	cout << "------------------------------------------------------------\n";
	cout << "           AOH and FED Channel Mapping Test Report\n";
	cout << "------------------------------------------------------------\n";
	cout << setw(4)<<right << totalChannelsInConfig                  << " channels in the configuration, of these:\n";
	cout << setw(4)<<right << totalChannelsInConfig - testedChannels << " were not tested\n";
	cout << setw(4)<<right << failures.size()                        << " had an inconclusive result due to another failure\n";
	cout << setw(4)<<right << testedChannels-failures.size()-unconnected.size() << " are correctly connected\n";
	cout << setw(4)<<right << unconnected.size()                     << " are incorrectly connected\n";
	
	cout << "\nFailed or unconnected channels (total of "<<failures.size()+unconnected.size()<<"):\n";
	if (failures.size()+unconnected.size() > 0)
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
		for ( std::map< PixelChannel, std::string>::iterator unconnected_itr = unconnected.begin(); unconnected_itr != unconnected.end(); unconnected_itr++ )
		{
			// Get info about this channel.
			const PixelHdwAddress& channelHdwAddress=theNameTranslation_->getHdwAddress(unconnected_itr->first);
			const unsigned int fednumber = channelHdwAddress.fednumber();
			const unsigned int channel = channelHdwAddress.fedchannel();
			
			const std::pair< std::string, int > portCardAndAOH = thePortcardMap_->PortCardAndAOH(unconnected_itr->first);
			const std::string portCardName = portCardAndAOH.first; assert(portCardName!="none");
			const int AOHNumber = portCardAndAOH.second;
			
			cout << setw(25)<<  left<< unconnected_itr->first.module()
				<< setw(2) << right<< unconnected_itr->first.TBMChannel() <<"  "
				<< setw(19)<<  left<< portCardName
				<< setw(2) << right<< AOHNumber <<"  "
				<< setw(3) << right<< fednumber <<"   "
				<< setw(3) << right<< channel   <<"  "
								<<left<< unconnected_itr->second <<endl;
		}
		cout << "\nDescriptions:\n";
		cout <<   "--Inconclusive Test--\n";
		cout <<   "NoData            = No data was found on this FED channel.\n";
		cout <<   "StuckAt0          = The black level was    0 for all AOH bias values.\n";
		cout <<   "StuckAt1023       = The black level was 1023 for all AOH bias values.\n";
		cout <<   "--Incorrect Connection--\n";
		cout <<   "AOHNotConnected   = This AOH is not on the same\n";
		cout <<   "                      optical link as this FED channel.\n";
		
	}
	cout << "------------------------------------------------------------"<<endl;

	std::cout << "PixelAOHAndFEDChannelMappingTest::execute() is finished.  Now returning..."<<endl;;
	return false;
} // end of execute() function

void PixelAOHAndFEDChannelMappingTest::SetAOHBiasOneChannel(std::string portCardName, unsigned int AOHNumber, unsigned int AOHBiasValue)
{
	Attribute_Vector parametersToTKFEC_SetAOHBiasOneChannel(3);
	parametersToTKFEC_SetAOHBiasOneChannel[0].name_="PortCardName";
	parametersToTKFEC_SetAOHBiasOneChannel[1].name_="AOHNumber";
	parametersToTKFEC_SetAOHBiasOneChannel[2].name_="AOHBias";
	parametersToTKFEC_SetAOHBiasOneChannel[0].value_=portCardName;
	parametersToTKFEC_SetAOHBiasOneChannel[1].value_=itoa(AOHNumber);
	parametersToTKFEC_SetAOHBiasOneChannel[2].value_=itoa(AOHBiasValue);
	
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

void PixelAOHAndFEDChannelMappingTest::triggeringLoop(unsigned int fedcrate, unsigned int fednumber, unsigned int fedchannel, unsigned int AOHBias, unsigned int Ntriggers)
{
  std::string action="FEDCalibrations";
  
  
  if(sendingMode_==""){

    std::vector<int> messageIDs;
    bool flag=false;

    for (unsigned int i_event=0;i_event<Ntriggers;++i_event)
      {
	// Send trigger.
	sendTTCCalSync();
	Attribute_Vector parametersToFED(4);
	parametersToFED[0].name_ = "WhatToDo"; parametersToFED[0].value_ = "MeasureBlackLevel";
	parametersToFED[1].name_ = "fednumber"; parametersToFED[1].value_ = itoa(fednumber);
	parametersToFED[2].name_ = "fedchannel"; parametersToFED[2].value_ = itoa(fedchannel);
	parametersToFED[3].name_ = "AOHBias"; parametersToFED[3].value_ = itoa(AOHBias);
	
	int messageID=send(PixelFEDSupervisors_[fedcrate], action, flag, parametersToFED);
	
	messageIDs.push_back(messageID);
      }
      
    for (std::vector<int>::iterator itr=messageIDs.begin(), itr_end=messageIDs.end(); itr!=itr_end; ++itr){
      
      flag=waitForReply(*itr);
    }
  }
  else if(sendingMode_=="yes"){
    for (unsigned int i_event=0;i_event<Ntriggers;++i_event)
      {
	// Send trigger.
	sendTTCCalSync();
	Attribute_Vector parametersToFED(4);
	parametersToFED[0].name_ = "WhatToDo"; parametersToFED[0].value_ = "MeasureBlackLevel";
	parametersToFED[1].name_ = "fednumber"; parametersToFED[1].value_ = itoa(fednumber);
	parametersToFED[2].name_ = "fedchannel"; parametersToFED[2].value_ = itoa(fedchannel);
	parametersToFED[3].name_ = "AOHBias"; parametersToFED[3].value_ = itoa(AOHBias);
	
	if (Send(PixelFEDSupervisors_[fedcrate], action, parametersToFED)!="AOHAndFEDChannelMappingTestDone")
	  {
	    diagService_->reportError("AOHAndFEDChannelMappingTest in FED crate # " + stringF(fedcrate) + " could not be done!",DIAGWARN);
	  }
      }
  }

}

std::vector<std::string> PixelAOHAndFEDChannelMappingTest::calibrated(){

  std::vector<std::string> tmp;
  
  return tmp;

}
