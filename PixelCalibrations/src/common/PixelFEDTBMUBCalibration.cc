// Modified by Jennifer Vaughan 2007/06/01
// $Id: PixelAddressLevelCalibration.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelFEDTBMUBCalibration.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelDecodedFEDRawData.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "PixelCalibrations/include/PixelTBMDACScanInfo.h"

#include <toolbox/convertstring.h>

#include "TFile.h"
#include "TStyle.h"

#include "iomanip"

#include <algorithm>

using namespace pos;
using namespace std;

//PixelFEDTBMUBCalibration::PixelFEDTBMUBCalibration() : PixelFEDCalibrationBase()
//{
//  std::cout << "Greetings from the PixelFEDTBMUBCalibration default constructor." << std::endl;
//}

PixelFEDTBMUBCalibration::PixelFEDTBMUBCalibration(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  std::cout << "Greetings from the PixelFEDTBMUBCalibration copy constructor." << std::endl;
}

xoap::MessageReference PixelFEDTBMUBCalibration::execute(xoap::MessageReference msg)
{
  assert(0);
	typedef unsigned char bits8;

	PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
	assert(tempCalibObject!=0);

	// Get target UB from the PixelCalibConfiguration object, if it exists.
	unsigned int target_UB = 135;
	if ( tempCalibObject->parameterValue("TargetUB") != "" )
	{
		target_UB = atoi(tempCalibObject->parameterValue("TargetUB").c_str());
	}
	
	unsigned int DualTBMMaxScanStepDiff = 6;
	if ( tempCalibObject->parameterValue("DualTBMMaxScanStepDiff") != "" )
	{
		DualTBMMaxScanStepDiff = atoi(tempCalibObject->parameterValue("DualTBMMaxScanStepDiff").c_str());
	}

	// Get other parameters
	bool printFEDRawData = false;
	if ( tempCalibObject->parameterValue("printFEDRawData") == "yes" || tempCalibObject->parameterValue("printFEDRawData") == "true" )
		printFEDRawData = true;
	bool printScan = false;
	if ( tempCalibObject->parameterValue("printScan") == "yes" || tempCalibObject->parameterValue("printScan") == "true" )
		printScan = true;

	PixelTBMDACScanInfo scanInfo( *tempCalibObject );

	Attribute_Vector parameters(2);
	parameters[0].name_="WhatToDo";
	parameters[1].name_="Step";
	Receive(msg, parameters);
	const unsigned int currentStep = atoi(parameters[1].value_.c_str());
//	unsigned int plotXValue = currentStep; // value to be used on the plot
	std::string plotXVarName = "scan step";
//	if ( scanInfo.DACsToScan().size() == 1 )
//	{
//		PixelTBMDACScanInfo::TBMDAC dac = *(scanInfo.DACsToScan().begin());
//		plotXValue = scanInfo.scanValue( dac, currentStep );
//		plotXVarName = scanInfo.name( dac );
//	}

	std::vector<std::pair<unsigned int,std::vector<unsigned int> > > fedsAndChannels=tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_ );
  //                     FED number,               channels

	if (parameters[0].value_=="RetrieveData")
	{
		for (unsigned int ifed=0; ifed<fedsAndChannels.size(); ++ifed)
		{
			unsigned int fednumber=fedsAndChannels[ifed].first;
			unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);

			for (unsigned int ichannel=0; ichannel<fedsAndChannels[ifed].second.size(); ++ichannel)
			{
				uint32_t buffer[pos::fifo1TranspDepth]; // problem on a 64-bit machine?
				unsigned int channel=fedsAndChannels[ifed].second[ichannel];

				std::vector<PixelROCName> ROCsOnThisChannel=theNameTranslation_->getROCsFromFEDChannel(fednumber, channel);

				int status = FEDInterface_[vmeBaseAddress]->drain_transBuffer(channel, buffer);
				if (status<0) {
				  std::cout<<"PixelFEDTBMUBCalibration::execute() -- Could not drain FIFO 1 of FED Channel "<<channel<<" in transparent mode!"<<std::endl;
				  diagService_->reportError("PixelFEDTBMUBCalibration::execute() -- Could not drain FIFO 1 in transparent mode!",DIAGWARN);
				}
				 
				// Decode the buffer.
				std::vector<unsigned int> hitsOnROCs(ROCsOnThisChannel.size(), 0); // 0 hits on each channel
				PixelDecodedFEDRawData decodedRawData(buffer, 100., 100., 150., hitsOnROCs);
				if (printFEDRawData) decodedRawData.printToStream(cout);

				if ( status == (int)pos::fifo1TranspDepth )
				{
					if (printFEDRawData) std::cout << "WARNING: Transparent buffer contains 2 events.  Ignoring this iteration.\n";
					continue;
				}

				// Check whether the decoded data is valid.
				if ( !decodedRawData.valid() || decodedRawData.numROCs() != ROCsOnThisChannel.size() )
				{
					if (printFEDRawData) std::cout << "WARNING: Decoding of FED raw data was unsuccessful.  Ignoring this iteration.\n";
					continue;
				}

				PixelChannel thisChannel = theNameTranslation_->ChannelFromFEDChannel(fednumber, channel);

				// Add values from the decoded data to the moment.
				TBM_UB_[thisChannel].addEntry(currentStep, decodedRawData.TBMHeader().UB1());
				TBM_UB_[thisChannel].addEntry(currentStep, decodedRawData.TBMHeader().UB2());
				TBM_UB_[thisChannel].addEntry(currentStep, decodedRawData.TBMHeader().UB3());
				TBM_UB_[thisChannel].addEntry(currentStep, decodedRawData.TBMTrailer().UB1());
				TBM_UB_[thisChannel].addEntry(currentStep, decodedRawData.TBMTrailer().UB2());

				// Done filling in information from the decoded data.

			} // end of loop over channels on this FED board
			VMEPtr_[vmeBaseAddress]->write("LRES",0x80000000); // Local Reset
			VMEPtr_[vmeBaseAddress]->write("CLRES",0x80000000); // *** MAURO ***
		} // end of loop over FEDs in this crate
	}
	else if (parameters[0].value_=="Analyse")
	{
		// Remove gray background from plots.
		TStyle plainStyle("Plain", "a plain style");
		plainStyle.cd();
		
		TFile outputFile((outputDir()+"/"+tempCalibObject->mode()+"Calibration_FEDCrate"+stringF(crate_)+".root").c_str(), "RECREATE", ("plots from "+tempCalibObject->mode()+" calibration in FED crate # "+stringF(crate_)).c_str());
		
		const std::set<PixelChannel>& channelsToCalibrate = tempCalibObject->channelList();
		
		std::map <PixelChannel, double> recommended_scanStep_values;
		std::map <PixelChannel, std::string> failedChannels;
		std::map <PixelChannel, std::string> warningChannels;

		// Loop over all channels in the list of channels to calibrate.  For each one, make an entry either in recommended_scanStep_values or in failedChannels.
		for ( std::set<PixelChannel>::const_iterator channelsToCalibrate_itr = channelsToCalibrate.begin(); channelsToCalibrate_itr != channelsToCalibrate.end(); channelsToCalibrate_itr++ )
		{
			std::map<PixelChannel, PixelScanRecord>::iterator TBM_UB_itr = TBM_UB_.find(*channelsToCalibrate_itr);
			if ( TBM_UB_itr == TBM_UB_.end() )
			{
				failedChannels[*channelsToCalibrate_itr] = "NoValidDecodedTriggers";
				continue;
			}
			PixelScanRecord& UBScanForThisChannel = TBM_UB_itr->second;
			UBScanForThisChannel.setTitle(TBM_UB_itr->first.channelname()+": TBM UB vs "+plotXVarName);
			UBScanForThisChannel.setXVarName(plotXVarName);
			UBScanForThisChannel.setYVarName("TBM UB");
			
			if (printScan)
			{
				std::cout << "**** For channel " << TBM_UB_itr->first << " ****\n";
				TBM_UB_itr->second.printScanToStream(std::cout, target_UB, itoa(target_UB));
			}
			if ( UBScanForThisChannel.crossingPointFound(target_UB) )
			{
				double recommended_scanStep = UBScanForThisChannel.crossingPointFullPrecision();
				if (printScan) cout << "Recommended "<<plotXVarName<<" = " << recommended_scanStep << "\n\n";
				recommended_scanStep_values[TBM_UB_itr->first] = recommended_scanStep;
			}
			else if ( UBScanForThisChannel.crossingPointFound(target_UB, PixelScanRecord::kFirst) )
			{
				warningChannels[TBM_UB_itr->first] = "MultipleCrossings";
				double recommended_scanStep = UBScanForThisChannel.crossingPointFullPrecision();
				if (printScan) cout << "WARNING: The UB crosses the target multiple times.\n";
				if (printScan) cout << "Recommended "<<plotXVarName<<" = " << recommended_scanStep << "\n\n";
				recommended_scanStep_values[TBM_UB_itr->first] = recommended_scanStep;
			}
			else
			{
				if ( UBScanForThisChannel.maxY().mean() < target_UB )
				{
					failedChannels[TBM_UB_itr->first] = "UBTooLow";
					if (printScan) cout << "The UB is always below the target.\n\n";
				}
				else if ( UBScanForThisChannel.minY().mean() > target_UB )
				{
					failedChannels[TBM_UB_itr->first] = "UBTooHigh";
					if (printScan) cout << "The UB is always above the target.\n\n";
				}
				else
				{
					failedChannels[TBM_UB_itr->first] = "Can'tFindCrossing";
					if (printScan) cout << "Even though the target lies inside the scan range, no crossing could be found.\n\n";
				}
			}
			
			// Add labels to specify scan ranges.
			TPaveText leftLabel (0.00                                  , 0.00, 0.05+0.11*scanInfo.DACsToScan().size(), 0.05, "BRNDC");
			TPaveText rightLabel(0.94-0.11*scanInfo.DACsToScan().size(), 0.00, 1.00                                  , 0.05, "BRNDC");
			std::string leftLabelText = "", rightLabelText = "";
			leftLabelText += "step 0: ";
			rightLabelText += "step "+stringF(scanInfo.scanNumSteps()-1)+": ";
			for ( std::set<PixelTBMDACScanInfo::TBMDAC>::const_iterator dac_itr = scanInfo.DACsToScan().begin(); dac_itr != scanInfo.DACsToScan().end(); dac_itr++ )
			{
				if ( dac_itr != scanInfo.DACsToScan().begin() )
				{
					leftLabelText  += ", ";
					rightLabelText += ", ";
				}
				leftLabelText  += scanInfo.shortName(*dac_itr)+" = "+stringF( scanInfo.scanValue(*dac_itr, 0) );
				rightLabelText += scanInfo.shortName(*dac_itr)+" = "+stringF( scanInfo.scanValue(*dac_itr, scanInfo.scanNumSteps()-1) );
			}
			leftLabel.AddText(leftLabelText.c_str());
			rightLabel.AddText(rightLabelText.c_str());
			std::vector<TPaveText> extraTextBoxes; extraTextBoxes.push_back(leftLabel); extraTextBoxes.push_back(rightLabel);
			
			UBScanForThisChannel.printPlot("rootFile", kBlue, extraTextBoxes);
		}
		// recommended_scanStep_values is now filled with the recommended scanStep values for each channel.

		// Write out new configuration files.
		unsigned int totalTBMs = 0, TBMsInOtherCrates = 0, TBMsNotTested = 0, succeededTBMs = 0, failedTBMs = 0;
		std::map<PixelTBMDACScanInfo::TBMDAC, Moments> delta_TBMDAC_values, new_TBMDAC_values;
		Moments dualTBM_scanStepDiff;
		std::map <PixelModuleName, std::string> dualTBMWarnings;
		// Loop over all modules (TBMs) in the configuration.
		std::vector<PixelModuleName>::const_iterator module_name = theDetectorConfiguration_->getModuleList().begin();
		for (;module_name!=theDetectorConfiguration_->getModuleList().end();++module_name)
		{
			totalTBMs++;
			
			// Get the settings for the TBM on this module.
			PixelTBMSettings *TBMSettingsForThisModule=0;
			std::string moduleNameString=(module_name->modulename());
			PixelConfigInterface::get(TBMSettingsForThisModule, "pixel/tbm/"+moduleNameString, *theGlobalKey_);
			assert(TBMSettingsForThisModule!=0);
			
			// Get hardware address(es) of this TBM.
			std::set<PixelChannel> channelsOnThisModule = theNameTranslation_->getChannelsOnModule(*module_name);
			assert( channelsOnThisModule.size() == 1 || channelsOnThisModule.size() == 2 ); // should have either 1 or 2 links from a TBM
			
			std::vector<double> new_scanSteps_onModule;
			std::set<unsigned int> crates;
			unsigned int channelsOnThisModuleToCalibrate = 0;
			unsigned int UBTooHigh = 0, UBTooLow = 0, MultipleCrossings = 0; // number of channels with these error messages
			for ( std::set<PixelChannel>::const_iterator channelsOnThisModule_itr = channelsOnThisModule.begin(); channelsOnThisModule_itr != channelsOnThisModule.end(); channelsOnThisModule_itr++ )
			{
				// Record crate number.
				const PixelHdwAddress& channelHdwAddress = theNameTranslation_->getHdwAddress(*channelsOnThisModule_itr);
				crates.insert(theFEDConfiguration_->crateFromFEDNumber(channelHdwAddress.fednumber()));
				
				if ( channelsToCalibrate.find(*channelsOnThisModule_itr) == channelsToCalibrate.end() )
				{
					continue;
				}
				channelsOnThisModuleToCalibrate++;
				
				// Record recommended scanStep.
				std::map<PixelChannel, double>::const_iterator foundScanStep = recommended_scanStep_values.find(*channelsOnThisModule_itr);
				if ( foundScanStep != recommended_scanStep_values.end() ) new_scanSteps_onModule.push_back(foundScanStep->second);
				else // set the best possible DAC value, but still consider the channel failed
				{
					std::map<PixelChannel, std::string>::const_iterator foundFailure = failedChannels.find(*channelsOnThisModule_itr);
					if (foundFailure != failedChannels.end())
					{
						if ( foundFailure->second == "UBTooHigh" )
						{
							UBTooHigh++;
							new_scanSteps_onModule.push_back((double)(TBM_UB_[*channelsOnThisModule_itr].maxX()));
						}
						if ( foundFailure->second == "UBTooLow" )
						{
							UBTooLow++;
							new_scanSteps_onModule.push_back((double)(TBM_UB_[*channelsOnThisModule_itr].minX()));
						}
					}
				}
			}
			const unsigned int failedChannelsWithNewDACSettings = UBTooHigh + UBTooLow + MultipleCrossings;
			assert( failedChannelsWithNewDACSettings <= new_scanSteps_onModule.size() );
			assert( crates.size() == 1 );
			if ( *(crates.begin()) != crate_ )
			{
				TBMsInOtherCrates++;
				continue; // Ignore if this TBM is not in the crate controlled by this PixelFEDSupervisor.
			}
			if ( channelsOnThisModuleToCalibrate == 0 )
			{
				TBMsNotTested++;
				continue;
			}
			if ( new_scanSteps_onModule.size() == 0 )
			{
				failedTBMs++;
				continue;
			}
			
			if ( failedChannelsWithNewDACSettings != 0 ) failedTBMs++;    // but still write new DAC settings for this TBM
			else                                         succeededTBMs++;
			
			// Get new scanStep value, do more checking for dual TBMs.
			double new_scanStep_value = *(max_element(new_scanSteps_onModule.begin(),new_scanSteps_onModule.end()));
			if ( channelsOnThisModule.size()==2 )
			{
				if ( new_scanSteps_onModule.size()==2 && failedChannelsWithNewDACSettings==0 )
				{
					double scanStepDiff = fabs(new_scanSteps_onModule[0] - new_scanSteps_onModule[1]);
					dualTBM_scanStepDiff.push_back(scanStepDiff);
					if ( scanStepDiff > DualTBMMaxScanStepDiff )
					{
						char tempString[100];
						sprintf(tempString, "ScanStep_Diff_%.1f", scanStepDiff);
						dualTBMWarnings[*module_name] = tempString;
					}
				}
				if ( channelsOnThisModuleToCalibrate == 1 )
				{
					dualTBMWarnings[*module_name] = "1/2ChannelsNotTested";
				}
			}

			// Now change DACs to the recommended values.
			for ( std::set<PixelTBMDACScanInfo::TBMDAC>::const_iterator dac_itr = scanInfo.DACsToScan().begin(); dac_itr != scanInfo.DACsToScan().end(); dac_itr++ )
			{
				bits8 new_dac_value = (bits8)(scanInfo.scanValue( *dac_itr, new_scanStep_value )+0.5); // round to the nearest integer
				new_TBMDAC_values[*dac_itr].push_back(new_dac_value);
				//if      ( *dac_itr == PixelTBMDACScanInfo::kAnalogInputBias )
				//{
				//	delta_TBMDAC_values[*dac_itr].push_back(new_dac_value-TBMSettingsForThisModule->getAnalogInputBias());
				//	TBMSettingsForThisModule->setAnalogInputBias( new_dac_value );
				//}
				//else if ( *dac_itr == PixelTBMDACScanInfo::kAnalogOutputBias )
				//{
				//	delta_TBMDAC_values[*dac_itr].push_back(new_dac_value-TBMSettingsForThisModule->getAnalogOutputBias());
				//	TBMSettingsForThisModule->setAnalogOutputBias( new_dac_value );
				//}
				//else if ( *dac_itr == PixelTBMDACScanInfo::kAnalogOutputGain )
				//{
				//	delta_TBMDAC_values[*dac_itr].push_back(new_dac_value-TBMSettingsForThisModule->getAnalogOutputGain());
				//	TBMSettingsForThisModule->setAnalogOutputGain( new_dac_value );
				//}
				//else assert(0);
			}

			// Write out new file.
			std::string moduleName = module_name->modulename();
			TBMSettingsForThisModule->writeASCII(outputDir());
			std::cout << "Wrote TBM settings for module:" << moduleName << endl;
			
			delete TBMSettingsForThisModule;
		} // end of loop over modules

		assert( totalTBMs-TBMsInOtherCrates-TBMsNotTested-succeededTBMs-failedTBMs == 0 );
		// Summary report.
		cout << "\n";
		cout << "--------------------------------------------\n";
		cout << "TBM UB Calibration Report for FED crate #"<<crate_<<"\n";
		cout << "--------------------------------------------\n";
		cout << setw(4) << totalTBMs << " TBMs in the configuration, of these:\n";
		cout << setw(4) << TBMsInOtherCrates << " TBMs are not in this crate.\n";
		cout << setw(4) << TBMsNotTested << " TBMs in this crate were not tested.\n";
		cout << setw(4) << succeededTBMs-dualTBMWarnings.size() << " TBMs had a fully successful calibration.\n";
		cout << setw(4) << dualTBMWarnings.size() << " dual TBMs succeeded, but with warnings.\n";
		cout << setw(4) << failedTBMs << " TBMs failed the calibration.\n";
		cout << setw(4) << warningChannels.size() << " individual channels had warnings but were considered successful.\n";

		cout << "\nTarget TBM UB level = " << target_UB << "\n";

		for ( std::set<PixelTBMDACScanInfo::TBMDAC>::const_iterator dac_itr = scanInfo.DACsToScan().begin(); dac_itr != scanInfo.DACsToScan().end(); dac_itr++ )
		{
			cout << "\nSummary of new "<<scanInfo.name(*dac_itr)<<" values (including failed TBMs):\n";
			cout <<   "mean     =" << setw(6)<<right << new_TBMDAC_values[*dac_itr].mean()   << "\n";
			cout <<   "std.dev. =" << setw(6)<<right << new_TBMDAC_values[*dac_itr].stddev() << "\n";
			cout <<   "smallest =" << setw(6)<<right << new_TBMDAC_values[*dac_itr].min()    << "\n";
			cout <<   "largest  =" << setw(6)<<right << new_TBMDAC_values[*dac_itr].max()    << "\n";
	
			cout << "\nSummary of changes from old to new "<<scanInfo.name(*dac_itr)<<" values (new-old):\n";
			cout <<   "mean          =" << setw(6)<<right << delta_TBMDAC_values[*dac_itr].mean()   << "\n";
			cout <<   "std.dev.      =" << setw(6)<<right << delta_TBMDAC_values[*dac_itr].stddev() << "\n";
			cout <<   "most negative =" << setw(6)<<right << delta_TBMDAC_values[*dac_itr].min()    << "\n";
			cout <<   "most positive =" << setw(6)<<right << delta_TBMDAC_values[*dac_itr].max()    << "\n";
		}
		
		cout << "\nDual TBMs Summary:\n";
		cout <<   "Number of dual TBMs for which both channels were\n";
		cout <<   "independently calibrated successfully: "<< dualTBM_scanStepDiff.count() <<"\n";
		cout <<   "Number of scan steps between recommended DAC settings on each TBM:\n";
		cout <<   "mean     =" << setw(6)<<right << dualTBM_scanStepDiff.mean()   << "\n";
		cout <<   "std.dev. =" << setw(6)<<right << dualTBM_scanStepDiff.stddev() << "\n";
		cout <<   "smallest =" << setw(6)<<right << dualTBM_scanStepDiff.min()    << "\n";
		cout <<   "largest  =" << setw(6)<<right << dualTBM_scanStepDiff.max()    << "\n";

		cout << "\nFailed channels (total of "<<failedChannels.size()<<"):\n";
		if (failedChannels.size() > 0)
		{
			cout <<   " (A dual TBM will fail if either of its channels fails.)\n";
			cout <<   "Module               TBM Ch. FED# FEDCh. Failure Mode\n";
			for ( std::map< PixelChannel, std::string>::iterator failedChannels_itr = failedChannels.begin(); failedChannels_itr != failedChannels.end(); failedChannels_itr++ )
			{
				// Get info about this channel.
				const PixelHdwAddress& channelHdwAddress=theNameTranslation_->getHdwAddress(failedChannels_itr->first);
				const unsigned int fednumber = channelHdwAddress.fednumber();
				const unsigned int channel = channelHdwAddress.fedchannel();
				
				cout << setw(25)<<  left<< failedChannels_itr->first.module()
					<< setw(2) << right<< failedChannels_itr->first.TBMChannel() <<"   "
					<< setw(3) << right<< fednumber <<"   "
					<< setw(3) << right<< channel   <<"  "
									<<left<< failedChannels_itr->second <<endl;
			}
			cout << "\nFailure mode descriptions:\n";
			cout <<   "NoValidDecodedTriggers = This channel was never successfully decoded on any\n";
			cout <<   "                           trigger.  No new DAC values." << endl;
			cout <<   "UBTooLow               = For all DAC values,\n";
			cout <<   "                           the UB level was below the target ("<<target_UB<<")." << endl;
			cout <<   "UBTooHigh              = For all DAC values,\n";
			cout <<   "                           the UB level was above the target ("<<target_UB<<")." << endl;
			cout <<   "Can'tFindCrossing      = Even though the target lies inside the scan range,\n";
			cout <<   "                           no crossing could be found.  This is probably\n";
			cout <<   "                           due to points ignored due to large uncertainties.\n";
		}
		cout << "\nChannels with warnings (total of "<<warningChannels.size()<<"):\n";
		if (warningChannels.size() > 0)
		{
			cout <<   "Module               TBM Ch. FED# FEDCh. Warning\n";
			for ( std::map< PixelChannel, std::string>::iterator warningChannels_itr = warningChannels.begin(); warningChannels_itr != warningChannels.end(); warningChannels_itr++ )
			{
				// Get info about this channel.
				const PixelHdwAddress& channelHdwAddress=theNameTranslation_->getHdwAddress(warningChannels_itr->first);
				const unsigned int fednumber = channelHdwAddress.fednumber();
				const unsigned int channel = channelHdwAddress.fedchannel();
				
				cout << setw(25)<<  left<< warningChannels_itr->first.module()
					<< setw(2) << right<< warningChannels_itr->first.TBMChannel() <<"   "
					<< setw(3) << right<< fednumber <<"   "
					<< setw(3) << right<< channel   <<"  "
									<<left<< warningChannels_itr->second <<endl;
			}
			cout << "\nWarning descriptions:\n";
			cout <<   "MultipleCrossings      = The UB level crossed the target more than once.\n";
			cout <<   "                            The crossing at smallest\n";
			cout <<   "                            DAC values was selected.\n";
		}
		cout << "\nWarnings for dual TBMs (total of "<<dualTBMWarnings.size()<<"):\n";
		if (dualTBMWarnings.size() > 0)
		{
			cout <<   " (New DAC settings were written out for these TBMs.)\n";
			cout <<   "Module                        Failure Mode\n";
			for ( std::map< PixelModuleName, std::string>::iterator dualTBMWarnings_itr = dualTBMWarnings.begin(); dualTBMWarnings_itr != dualTBMWarnings.end(); dualTBMWarnings_itr++ )
			{
				cout << setw(30)<<  left<< dualTBMWarnings_itr->first <<"  "
									<<left<< dualTBMWarnings_itr->second <<endl;
			}
			cout << "\nWarning descriptions:\n";

			cout <<   "ScanStep_Diff_[x]        = The recommended DAC settings on the\n";
			cout <<   "                             two channels differed by [x] scan steps,\n";
			cout <<   "                             which exceeds the limit of "<<DualTBMMaxScanStepDiff<<".\n";
			cout <<   "1/2ChannelsNotTested     = One of the two channels on this TBM was not tested.\n";
			cout <<   "                             The DAC settings are based\n";
			cout <<   "                             on the other channel.\n";
		}

		cout << "--------------------------------------" << endl;

		// Clear the member data.
		TBM_UB_.clear();
		
		outputFile.Write();
		outputFile.Close();
	} // end of "Analyze" block
	else
	{
		cout << "ERROR: PixelFEDTBMUBCalibration::execute() does not understand the WhatToDo command, "<< parameters[0].value_ <<", sent to it.\n";
		assert(0);
	}

	xoap::MessageReference reply = MakeSOAPMessageReference("FEDCalibrationsDone");
	return reply;
}


void PixelFEDTBMUBCalibration::initializeFED(){

  setFEDModeAndControlRegister(0x0,0x30019);

}

xoap::MessageReference PixelFEDTBMUBCalibration::beginCalibration(xoap::MessageReference msg){

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDTBMUBCalibration::endCalibration(xoap::MessageReference msg){

  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

