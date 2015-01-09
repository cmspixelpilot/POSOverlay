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

#include "PixelCalibrations/include/PixelFEDAOHGainCalibration.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelDecodedFEDRawData.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"

#include <toolbox/convertstring.h>

#include "TFile.h"
#include "TStyle.h"

#include "iomanip"

#include <algorithm>

using namespace pos;
using namespace std;

//PixelFEDAOHGainCalibration::PixelFEDAOHGainCalibration() : PixelFEDCalibrationBase()
//{
//  std::cout << "Greetings from the PixelFEDAOHGainCalibration default constructor." << std::endl;
//}

PixelFEDAOHGainCalibration::PixelFEDAOHGainCalibration(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  std::cout << "Greetings from the PixelFEDAOHGainCalibration copy constructor." << std::endl;
}

xoap::MessageReference PixelFEDAOHGainCalibration::execute(xoap::MessageReference msg)
{
	PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
	assert(tempCalibObject!=0);

	unsigned int MaxTBMUB = 100;
	if ( tempCalibObject->parameterValue("MaxTBMUB") != "" )
	{
		MaxTBMUB = atoi(tempCalibObject->parameterValue("MaxTBMUB").c_str());
	}

	bool printFEDRawData = false;
	if ( tempCalibObject->parameterValue("printFEDRawData") == "yes" || tempCalibObject->parameterValue("printFEDRawData") == "true" )
		printFEDRawData = true;
	bool printScan = false;
	if ( tempCalibObject->parameterValue("printScan") == "yes" || tempCalibObject->parameterValue("printScan") == "true" )
		printScan = true;


	Attribute_Vector parameters(3);
	parameters[0].name_="WhatToDo";
	parameters[1].name_="AOHGain";
	parameters[2].name_="Channel";
	Receive(msg, parameters);
	unsigned int AOHGain = atoi(parameters[1].value_.c_str());

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

				const int baselineCorrection=FEDInterface_[vmeBaseAddress]->get_BaselineCorr(channel);

				int status = FEDInterface_[vmeBaseAddress]->drain_transBuffer(channel, buffer);
				if (status<0) {
				  std::cout<<"PixelFEDAOHGainCalibration::execute() -- Could not drain FIFO 1 of FED Channel "<<channel<<" in transparent mode!"<<std::endl;
				  diagService_->reportError("PixelFEDAOHGainCalibration::execute() -- Could not drain FIFO 1 in transparent mode!",DIAGWARN);
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
				TBM_UB_[thisChannel].addEntry(AOHGain, decodedRawData.TBMHeader().UB1());
				TBM_UB_[thisChannel].addEntry(AOHGain, decodedRawData.TBMHeader().UB2());
				TBM_UB_[thisChannel].addEntry(AOHGain, decodedRawData.TBMHeader().UB3());
				TBM_UB_[thisChannel].addEntry(AOHGain, decodedRawData.TBMTrailer().UB1());
				TBM_UB_[thisChannel].addEntry(AOHGain, decodedRawData.TBMTrailer().UB2());
				
				baselineCorrection_[thisChannel].addEntry(AOHGain, baselineCorrection);

				// Done filling in information from the decoded data.

			} // end of loop over channels on this FED board
			VMEPtr_[vmeBaseAddress]->write("LRES",0x80000000); // Local Reset
			VMEPtr_[vmeBaseAddress]->write("CLRES",0x80000000); // *** MAURO ***
		} // end of loop over FEDs in this crate
	}
	else if (parameters[0].value_=="Analyze")
	{
		// Remove gray background from plots.
		TStyle plainStyle("Plain", "a plain style");
		plainStyle.cd();
		
		TFile outputFile((outputDir()+"/"+tempCalibObject->mode()+"Calibration_FEDCrate"+stringF(crate_)+".root").c_str(), "RECREATE", ("plots from "+tempCalibObject->mode()+" calibration in FED crate # "+stringF(crate_)).c_str());
		
		const std::set<PixelChannel>& channelsToCalibrate = tempCalibObject->channelList();
		
		recommended_AOHGain_values_.clear();
		failures_.clear();
		PixelRootDirectoryMaker rootDirs(channelsToCalibrate,gDirectory);
		// Loop over all channels in the list of channels to calibrate.  For each one, make an entry in recommended_AOHGain_values_ and/or in failures_.
		for ( std::set<PixelChannel>::const_iterator channelsToCalibrate_itr = channelsToCalibrate.begin(); channelsToCalibrate_itr != channelsToCalibrate.end(); channelsToCalibrate_itr++ )
		{
			rootDirs.cdDirectory(*(channelsToCalibrate_itr));
			
			std::map<PixelChannel, PixelScanRecord>::iterator TBM_UB_itr = TBM_UB_.find(*channelsToCalibrate_itr);
			if ( TBM_UB_itr == TBM_UB_.end() )
			{
				failures_[*channelsToCalibrate_itr] = "NoValidDecodedTriggers";
				continue;
			}
			PixelScanRecord& UBScanForThisChannel = TBM_UB_itr->second;
			UBScanForThisChannel.setTitle(TBM_UB_itr->first.channelname()+": TBM UB vs AOH gain");
			UBScanForThisChannel.setXVarName("AOH gain");
			UBScanForThisChannel.setYVarName("TBM UB");
			
			std::map<PixelChannel, PixelScanRecord>::iterator baselineCorrection_itr = baselineCorrection_.find(*channelsToCalibrate_itr);
			assert( baselineCorrection_itr != baselineCorrection_.end() );
			PixelScanRecord& baselineCorrectionScanForThisChannel = baselineCorrection_itr->second;
			baselineCorrectionScanForThisChannel.setTitle(TBM_UB_itr->first.channelname()+": FED automatic baseline correction vs AOH gain");
			baselineCorrectionScanForThisChannel.setXVarName("AOH gain");
			baselineCorrectionScanForThisChannel.setYVarName("FED automatic baseline correction");
			
			if (printScan)
			{
				std::cout << "**** For channel " << TBM_UB_itr->first << " ****\n";
				TBM_UB_itr->second.printScanToStream(std::cout);
			}
			
			// Find the lowest AOH gain that puts TBM UB below the threshold.
			recommended_AOHGain_values_[*channelsToCalibrate_itr] = 0;
			for ( ; ; recommended_AOHGain_values_[*channelsToCalibrate_itr]++ )
			{
				if ( recommended_AOHGain_values_[*channelsToCalibrate_itr] == 4 ) // failed to find a low enough TBM UB
				{
					recommended_AOHGain_values_[*channelsToCalibrate_itr] = 3;
					failures_[*channelsToCalibrate_itr] = "UBTooHigh";
					break;
				}
				Moments point = UBScanForThisChannel.getPoint(recommended_AOHGain_values_[*channelsToCalibrate_itr]);
				if ( point.count() == 0 ) continue; // ignore if there's no data at this x value
				if ( point.mean() < MaxTBMUB ) break;
			}
			
			UBScanForThisChannel.printPlot();
			baselineCorrectionScanForThisChannel.printPlot();
		}
		// recommended_AOHGain_values_ is now filled with the recommended AOHGain values for each channel.

		// On dual TBMs, check whether we can reduce the difference in UB levels by raising one of the two AOH gains.  If so, do it.
		std::vector<PixelModuleName> modulesToCalibrate = theDetectorConfiguration_->getModuleList();
		
		for (std::vector<PixelModuleName>::const_iterator modulesToCalibrate_itr = modulesToCalibrate.begin(); modulesToCalibrate_itr != modulesToCalibrate.end();++modulesToCalibrate_itr)
		  {
			std::set<PixelChannel> channelsOnThisModule = theNameTranslation_->getChannelsOnModule(*modulesToCalibrate_itr);
			
			if ( channelsOnThisModule.size() == 1 ) continue; // nothing to do for single TBMs

			//std::cout << "Dual TBM Section: channelsOnThisModule.size()=" << channelsOnThisModule.size() << " and (*modulesToCalibrate_itr)="
                        //          << (*modulesToCalibrate_itr) << std::endl;
			//std::cout << "I am crate " << crate_ << std::endl;
			//std::cout << "Looking for crate (from FED number) "
                        //          << theFEDConfiguration_->crateFromFEDNumber(theNameTranslation_->firstHdwAddress(*modulesToCalibrate_itr).fednumber()) << std::endl;

			if (crate_ != theFEDConfiguration_->crateFromFEDNumber(theNameTranslation_->firstHdwAddress(*modulesToCalibrate_itr).fednumber())) continue;
			
			assert( channelsOnThisModule.size() == 2 ); // should have either 1 or 2 links from a TBM
			
			std::set<PixelChannel>::const_iterator channelsOnThisModule_itr = channelsOnThisModule.begin();
			PixelChannel firstChannel = *channelsOnThisModule_itr;
			channelsOnThisModule_itr++;
			PixelChannel secondChannel = *channelsOnThisModule_itr;
			
			if ( failures_.find(firstChannel) != failures_.end() || failures_.find(secondChannel) != failures_.end() ) continue; // skip if one of the channels failed

			if ( TBM_UB_.find( firstChannel) == TBM_UB_.end() || TBM_UB_.find(secondChannel) == TBM_UB_.end()) continue;

			assert( recommended_AOHGain_values_.find( firstChannel) != recommended_AOHGain_values_.end() );
			assert( recommended_AOHGain_values_.find(secondChannel) != recommended_AOHGain_values_.end() );
			
			const double  firstChannelTBMUB = TBM_UB_[ firstChannel].getPoint(recommended_AOHGain_values_[ firstChannel]).mean();
			const double secondChannelTBMUB = TBM_UB_[secondChannel].getPoint(recommended_AOHGain_values_[secondChannel]).mean();
			PixelChannel higherChannel, lowerChannel;
			if ( firstChannelTBMUB > secondChannelTBMUB )
			{
				higherChannel = firstChannel;
				lowerChannel = secondChannel;
			}
			else
			{
				higherChannel = secondChannel;
				lowerChannel = firstChannel;
			}
			
			if (recommended_AOHGain_values_[higherChannel]==3) continue; // can't raise this AOH gain any further
			
			if (
			    fabs( TBM_UB_[higherChannel].getPoint(recommended_AOHGain_values_[higherChannel] + 1).mean() -
			          TBM_UB_[ lowerChannel].getPoint(recommended_AOHGain_values_[ lowerChannel]    ).mean() )
			    <
			    fabs( TBM_UB_[higherChannel].getPoint(recommended_AOHGain_values_[higherChannel]).mean() -
			          TBM_UB_[ lowerChannel].getPoint(recommended_AOHGain_values_[ lowerChannel]).mean() )
			   )
			{
				recommended_AOHGain_values_[higherChannel] += 1;
			}
			assert( recommended_AOHGain_values_[ firstChannel] <= 3 );
			assert( recommended_AOHGain_values_[secondChannel] <= 3 );
		}

		// Clear the member data.
		TBM_UB_.clear();
		
		outputFile.Write();
		outputFile.Close();
	} // end of "Analyze" block
	else if (parameters[0].value_=="RetrieveAOHGain")
	{
		PixelChannel channel(parameters[2].value_);
		std::map <PixelChannel, unsigned int>::const_iterator foundAOHGain = recommended_AOHGain_values_.find(channel);
		std::map <PixelChannel, std::string>::const_iterator foundFailure = failures_.find(channel);
		assert( foundAOHGain != recommended_AOHGain_values_.end() || foundFailure != failures_.end() );
		
		Attribute_Vector returnValues(2);
		returnValues[0].name_="AOHGain"; returnValues[0].value_="none";
		returnValues[1].name_="Error";   returnValues[1].value_="none";
		if ( foundAOHGain != recommended_AOHGain_values_.end() ) returnValues[0].value_ = itoa(foundAOHGain->second);
		if ( foundFailure != failures_.end() ) returnValues[1].value_ = foundFailure->second;
		xoap::MessageReference reply=MakeSOAPMessageReference("TBMUBDone", returnValues);
		return reply;
	}
	else
	{
		cout << "ERROR: PixelFEDAOHGainCalibration::execute() does not understand the WhatToDo command, "<< parameters[0].value_ <<", sent to it.\n";
		assert(0);
	}

	xoap::MessageReference reply = MakeSOAPMessageReference("TBMUBDone");
	return reply;
}


void PixelFEDAOHGainCalibration::initializeFED(){

  setFEDModeAndControlRegister(0x0,0x30019);
	setBlackUBTrans();

}

xoap::MessageReference PixelFEDAOHGainCalibration::beginCalibration(xoap::MessageReference msg){

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDAOHGainCalibration::endCalibration(xoap::MessageReference msg){

  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

