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

#include "PixelCalibrations/include/PixelFEDAOHBiasCalibration.h"
#include "PixelCalibrations/include/PixelAOHBiasCalibrationParameters.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelDecodedFEDRawData.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"

#include "TCanvas.h"
#include "TFrame.h"
#include "TH1F.h"
#include "TPaveText.h"
#include "TText.h"
#include "TLine.h"

#include <toolbox/convertstring.h>

#include "iomanip"

using namespace pos;
using namespace std;
using namespace PixelAOHBiasCalibrationParameters;

const bool debug = false;

//PixelFEDAOHBiasCalibration::PixelFEDAOHBiasCalibration() : PixelFEDCalibrationBase()
//{
//  std::cout << "Greetings from the PixelFEDAOHBiasCalibration default constructor." << std::endl;
//}

PixelFEDAOHBiasCalibration::PixelFEDAOHBiasCalibration(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  std::cout << "Greetings from the PixelFEDAOHBiasCalibration copy constructor." << std::endl;
}

xoap::MessageReference PixelFEDAOHBiasCalibration::execute(xoap::MessageReference msg)
{
	Attribute_Vector parameters(k_NumVars);
	parameters[k_WhatToDo].name_="WhatToDo";
	parameters[k_AOHBias].name_="AOHBias";
	parameters[k_FEDNumber].name_="FEDNumber";
	parameters[k_FEDChannel].name_="FEDChannel";
	Receive(msg, parameters);

	xoap::MessageReference reply = MakeSOAPMessageReference("AOHBiasDone");

	if(debug) cout<<"PixelFEDAOHBiasCalibration:execute "<<parameters[k_WhatToDo].value_<<endl;

	if (parameters[k_WhatToDo].value_=="RetrieveDataFromFullDecode" || parameters[k_WhatToDo].value_=="RetrieveDataFromTimeSlots" || parameters[k_WhatToDo].value_=="MeasureBlackLevels")
	{
		unsigned int AOHBias = atoi(parameters[k_AOHBias].value_.c_str());
		std::string step = parameters[k_WhatToDo].value_;
		RetrieveData(AOHBias, step);
	}
	else if (parameters[k_WhatToDo].value_=="FindSaturationPoint")
	{
		assert( parameters[k_FEDNumber].value_ != "unset" );
		assert( parameters[k_FEDChannel].value_ != "unset" );
		unsigned int FEDNumber = atoi(parameters[k_FEDNumber].value_.c_str());
		unsigned int FEDChannel = atoi(parameters[k_FEDChannel].value_.c_str());
		reply = FindSaturationPoint(FEDNumber, FEDChannel);
	}
	else if (parameters[k_WhatToDo].value_=="RaiseFEDReceiverInputOffsets")
	{
		reply = RaiseFEDReceiverInputOffsets();
	}
	else if (parameters[k_WhatToDo].value_=="RetrieveBlackLevel")
	{
		assert( parameters[k_FEDNumber].value_ != "unset" );
		assert( parameters[k_FEDChannel].value_ != "unset" );
		unsigned int FEDNumber = atoi(parameters[k_FEDNumber].value_.c_str());
		unsigned int FEDChannel = atoi(parameters[k_FEDChannel].value_.c_str());
		reply = RetrieveBlackLevel(FEDNumber, FEDChannel);
	}
	else if (parameters[k_WhatToDo].value_=="WriteConfigFiles")
	{
		reply = WriteConfigFiles();
	}
	else
	{
		cout << "ERROR: PixelFEDAOHBiasCalibration::execute() does not understand the WhatToDo command, "<< parameters[k_WhatToDo].value_ <<", sent to it.\n";
		assert(0);
	}

	return reply;
}

void PixelFEDAOHBiasCalibration::RetrieveData(unsigned int AOHBias, std::string step)
{
	PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
	assert(tempCalibObject!=0);

	if(debug) cout<<"PixelFEDAOHBiasCalibration:RetriveData "<<AOHBias<<endl;

	bool printFEDRawData = k_printFEDRawData_default;
	if ( tempCalibObject->parameterValue("printFEDRawData") == "yes" || tempCalibObject->parameterValue("printFEDRawData") == "true" )
		printFEDRawData = true;
		
	bool printFEDOffsetAdjustments = k_printFEDOffsetAdjustments_default;
	if ( tempCalibObject->parameterValue("printFEDOffsetAdjustments") == "yes" || tempCalibObject->parameterValue("printFEDOffsetAdjustments") == "true" )
		printFEDOffsetAdjustments = true;
	
	unsigned int maxFEDReceiverInputOffset = atoi( k_MaxFEDReceiverInputOffset_default.c_str() );
	if ( tempCalibObject->parameterValue("MaxFEDReceiverInputOffset") != "" )
		maxFEDReceiverInputOffset = atoi( tempCalibObject->parameterValue("MaxFEDReceiverInputOffset").c_str() );
	
	std::vector<std::pair<unsigned int,std::vector<unsigned int> > > fedsAndChannels=tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_ );
	//                     FED number,               channels
	
	for ( std::vector<std::pair<unsigned int,std::vector<unsigned int> > >::iterator fedsAndChannels_itr = fedsAndChannels.begin(); fedsAndChannels_itr != fedsAndChannels.end(); fedsAndChannels_itr++ )
	{
		unsigned int fednumber=fedsAndChannels_itr->first;
		unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);

		for (std::vector<unsigned int>::iterator channel_itr = fedsAndChannels_itr->second.begin(); channel_itr != fedsAndChannels_itr->second.end(); channel_itr++)
		{
			unsigned int channel = *channel_itr;
			
			uint32_t buffer[pos::fifo1TranspDepth]; // problem on a 64-bit machine?

			std::vector<PixelROCName> ROCsOnThisChannel=theNameTranslation_->getROCsFromFEDChannel(fednumber, channel);

			if(debug) cout<<"PixelFEDAOHBiasCalibration drain "<<fednumber<<" "<<channel<<" "<<ROCsOnThisChannel.size()<<endl;

			int status = FEDInterface_[vmeBaseAddress]->drain_transBuffer(channel, buffer);
			if (status<0) {
				std::cout<<"PixelFEDAOHBiasCalibration::execute() -- Could not drain FIFO 1 of FED Channel "<<channel<<" in transparent mode!"<<std::endl;
				diagService_->reportError("PixelFEDAOHBiasCalibration::execute() -- Could not drain FIFO 1 in transparent mode!",DIAGWARN);
			}
				
			if(debug) cout<<"PixelFEDAOHBiasCalibration after drain "<<" "<<step<<endl;

			// Decode the buffer.
			std::vector<unsigned int> hitsOnROCs(ROCsOnThisChannel.size(), 0); // 0 hits on each channel
			if(debug) cout<<"PixelFEDAOHBiasCalibration decode "<<hitsOnROCs.size()<<" "<<step<<endl;

			PixelDecodedFEDRawData decodedRawData(buffer, 50., 50., 75., hitsOnROCs);
			if (printFEDRawData) decodedRawData.printToStream(cout);

			if ( status == (int)pos::fifo1TranspDepth )
			{
				if (printFEDRawData) std::cout << "WARNING: Transparent buffer contains 2 events.  Ignoring this iteration.\n";
				continue;
			}
			
			if ( decodedRawData.badBuffer() )
			{
				if (printFEDRawData) std::cout << "WARNING: Bad buffer from FED.  Ignoring this iteration.\n";
				continue;
			}

			// Gather values from the decoded data.
			if ( step=="RetrieveDataFromFullDecode" )
			{
				// Check whether the decoded data is valid.
				if ( !decodedRawData.valid() || decodedRawData.numROCs() != ROCsOnThisChannel.size() )
				{
					if (printFEDRawData) std::cout << "WARNING: Decoding of FED raw data was unsuccessful.  Ignoring this iteration.\n";
					continue;
				}
				
				TBMHeaderStart_[fednumber][channel].addEntry(decodedRawData.TBMHeader().startPosition());
				TBMTrailerStart_[fednumber][channel].addEntry(decodedRawData.TBMTrailer().startPosition());
				
				//std::cout << "TBMTrailerStart: totalEntries = "<<TBMTrailerStart_[fednumber][channel].totalEntries()<<", mode = "<<TBMTrailerStart_[fednumber][channel].mode()<<", modeNumEntries = "<<TBMTrailerStart_[fednumber][channel].modeNumEntries()<<", nextMostEntries = "<<TBMTrailerStart_[fednumber][channel].nextMostEntries()<<"\n";
			}
			else if ( step=="RetrieveDataFromTimeSlots" )
			{
				// If reliable time slots were not found previously, don't record anything.
				if ( badTimeSlotInfo(fednumber, channel) )
				{
					// Only record the baseline black, from the first 10 slots of the transparent buffer.
					Moments thisTriggerBaselineB;
					for ( unsigned int slot = 0; slot < 10; slot++ )
					{
						thisTriggerBaselineB.push_back( decodedRawData.adcValue(slot) );
					}
					BaselineB_[fednumber][channel].addEntry(AOHBias, thisTriggerBaselineB.mean() );
					
					//std::cout << "WARNING: no reliable time slot info for FED "<<fednumber<<", channel "<<channel<<".  Omitting this channel.\n";
					continue;
				}
				
				const std::pair<TimeSlotCode, unsigned int> timeSlotCode = TimeSlotCodeForChannel(fednumber, channel);
				
				const unsigned int TBMHeaderStartPoint = timeSlotCode.second;
				const unsigned int TBMTrailerStartPoint = timeSlotCode.second + (TBMTrailerStart_[fednumber][channel].topTwoValues()[0].first-TBMHeaderStart_[fednumber][channel].topTwoValues()[0].first);
				
				Moments thisTBMUB, thisTBMB;
				
				if ( timeSlotCode.first == kGoodSingleTimeSlot )
				{
					thisTBMUB.push_back( decodedRawData.adcValue( TBMHeaderStartPoint ) );
					thisTBMUB.push_back( decodedRawData.adcValue( TBMHeaderStartPoint + 1 ) );
					thisTBMUB.push_back( decodedRawData.adcValue( TBMHeaderStartPoint + 2 ) );
					thisTBMB.push_back( decodedRawData.adcValue( TBMHeaderStartPoint + 3 ) );
					
					thisTBMUB.push_back( decodedRawData.adcValue( TBMTrailerStartPoint ) );
					thisTBMUB.push_back( decodedRawData.adcValue( TBMTrailerStartPoint + 1 ) );
					thisTBMB.push_back( decodedRawData.adcValue( TBMTrailerStartPoint + 2 ) );
					thisTBMB.push_back( decodedRawData.adcValue( TBMTrailerStartPoint + 3 ) );
				}
				else if ( timeSlotCode.first == kGoodJumpingByOneTimeSlot )
				{
					// Only take readings where we're sure it's UB or B, given that the data may start one time slot later.
					thisTBMUB.push_back( decodedRawData.adcValue( TBMHeaderStartPoint + 1 ) );
					thisTBMUB.push_back( decodedRawData.adcValue( TBMHeaderStartPoint + 2 ) );
					
					thisTBMUB.push_back( decodedRawData.adcValue( TBMTrailerStartPoint + 1 ) );
					thisTBMB.push_back( decodedRawData.adcValue( TBMTrailerStartPoint + 3 ) );
				}
				else assert(0);
				
				if ( thisTBMUB.mean() < 0+5 && thisTBMB.mean() > 1023-5 )
				{
					std::cout << "WARNING: On FED number "<<fednumber<<", channel "<<channel<<", TBM B-UB difference is too large -- B & UB do not both fit in range.  Change the AOH gains and/or TBM DACs to reduce it.\n";
					std::cout << "Cannot continue, now exiting...\n";
					assert(0);
				}
				
				if ( thisTBMUB.mean() < 0+5 ) // UB is nearly out of range low, lower FED offset to compensate
				{
					PixelFEDCard& fedCard = FEDInterface_[vmeBaseAddress]->getPixelFEDCard();
					assert( 0 <= fedCard.offs_dac[channel-1] && fedCard.offs_dac[channel-1] < 256 );
					
					// Lower the channel offset, if possible.
					if ( fedCard.offs_dac[channel-1] > 0 )
					{
						if (printFEDOffsetAdjustments) cout << "On FED number "<<fednumber<<", channel "<<channel<<", lowering channel offset (AOHBias = "<<AOHBias<<").\n";
						fedCard.offs_dac[channel-1] -= 64;
						if ( fedCard.offs_dac[channel-1] < 0 ) fedCard.offs_dac[channel-1] = 0;
						FEDInterface_[vmeBaseAddress]->set_offset_dacs();
					}
					else
					{
						if (printFEDOffsetAdjustments) std::cout << "WARNING: On FED number "<<fednumber<<", channel "<<channel<<", cannot further lower the FED offsets to raise the signal within range (AOHBias = "<<AOHBias<<").\n";
					}
				}
				else if ( thisTBMB.mean() > 1023-5 ) // B is nearly out of range high, raise FED offset to compensate
				{
					PixelFEDCard& fedCard = FEDInterface_[vmeBaseAddress]->getPixelFEDCard();
					assert( 0 <= fedCard.offs_dac[channel-1] && fedCard.offs_dac[channel-1] < 256 );
					
					// Raise the channel offset, if possible.
					if ( fedCard.offs_dac[channel-1] < 255 )
					{
						if (printFEDOffsetAdjustments) cout << "On FED number "<<fednumber<<", channel "<<channel<<", raising channel offset (AOHBias = "<<AOHBias<<").\n";
						fedCard.offs_dac[channel-1] += 64;
						if ( fedCard.offs_dac[channel-1] > 255 ) fedCard.offs_dac[channel-1] = 255;
						FEDInterface_[vmeBaseAddress]->set_offset_dacs();
					}
					else
					{
						if (printFEDOffsetAdjustments) std::cout << "WARNING: On FED number "<<fednumber<<", channel "<<channel<<", cannot further raise the FED offsets to lower the signal within range (AOHBias = "<<AOHBias<<").\n";
					}
				}
				else // all is well, add this to the record of B, UB, and the difference
				{
					TBMB_[fednumber][channel].addEntry(AOHBias, thisTBMB.mean() );
					TBMUB_[fednumber][channel].addEntry(AOHBias, thisTBMUB.mean() );
					//noise Valeria 
					if ( timeSlotCode.first != kGoodJumpingByOneTimeSlot ) TBMBNoise_[fednumber][channel].addEntry(AOHBias, 100*thisTBMB.stddev() );
					TBMUBNoise_[fednumber][channel].addEntry(AOHBias, 100*thisTBMUB.stddev() );
					//end Valeria
					TBMBUBDiff_[fednumber][channel].addEntry( AOHBias, thisTBMB.mean() - thisTBMUB.mean() );
					//std::cout<<thisTBMB.mean()<<" "<<thisTBMUB.mean()<<" "<<thisTBMB.stddev()<<" "<<thisTBMUB.stddev()<<endl;
				}
			}
			else if ( step=="MeasureBlackLevels" )
			{
				// If reliable time slots were not found previously, don't record anything.
				if ( badTimeSlotInfo(fednumber, channel) )
				{
					//std::cout << "WARNING: no reliable time slot info for FED "<<fednumber<<", channel "<<channel<<".  Omitting this channel.\n";
					continue;
				}
				
				const std::pair<TimeSlotCode, unsigned int> timeSlotCode = TimeSlotCodeForChannel(fednumber, channel);
				
				const unsigned int TBMHeaderStartPoint = timeSlotCode.second;
				const unsigned int TBMTrailerStartPoint = timeSlotCode.second + (TBMTrailerStart_[fednumber][channel].topTwoValues()[0].first-TBMHeaderStart_[fednumber][channel].topTwoValues()[0].first);
				
				Moments thisTBMUB, thisTBMB;
				
				if ( timeSlotCode.first == kGoodSingleTimeSlot )
				{
					TBMB_Moments_[fednumber][channel].push_back( decodedRawData.adcValue( TBMHeaderStartPoint + 3 ) );
					TBMB_Moments_[fednumber][channel].push_back( decodedRawData.adcValue( TBMTrailerStartPoint + 2 ) );
					TBMB_Moments_[fednumber][channel].push_back( decodedRawData.adcValue( TBMTrailerStartPoint + 3 ) );
				}
				else if ( timeSlotCode.first == kGoodJumpingByOneTimeSlot )
				{
					// Only take readings where we're sure it's UB or B, given that the data may start one time slot later.
					TBMB_Moments_[fednumber][channel].push_back( decodedRawData.adcValue( TBMTrailerStartPoint + 3 ) );
				}
				else assert(0);
			}
			else assert(0);

			// Done filling in information from the decoded data.

		} // end of loop over channels on this FED board
		VMEPtr_[vmeBaseAddress]->write("LRES",0x80000000); // Local Reset
		VMEPtr_[vmeBaseAddress]->write("CLRES",0x80000000); // *** MAURO ***
	} // end of loop over FEDs in this crate
} // end of RetrieveData()

xoap::MessageReference PixelFEDAOHBiasCalibration::FindSaturationPoint(unsigned int fedNumber, unsigned int channel)
{
	PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
        assert(tempCalibObject!=0);

	// Get parameters from the calib object
	bool printAOHBiasAdjustments = k_printAOHBiasAdjustments_default;
	if ( tempCalibObject->parameterValue("printAOHBiasAdjustments") == "yes" || tempCalibObject->parameterValue("printAOHBiasAdjustments") == "true" )
		printAOHBiasAdjustments = true;
	
	unsigned int AOHBiasMin      = k_ScanMin_default;
	if ( tempCalibObject->parameterValue("ScanMin") != "" ) AOHBiasMin = atoi(tempCalibObject->parameterValue("ScanMin").c_str());
	unsigned int AOHBiasMax      = k_ScanMax_default;;
	if ( tempCalibObject->parameterValue("ScanMax") != "" ) AOHBiasMax = atoi(tempCalibObject->parameterValue("ScanMax").c_str());

	int SaturationPointOffset = k_SaturationPointOffset_default;
	if ( tempCalibObject->parameterValue("SaturationPointOffset") != "" ) SaturationPointOffset = atoi(tempCalibObject->parameterValue("SaturationPointOffset").c_str());

	// Check whether good time slot info was found for the given channel
	if ( badTimeSlotInfo(fedNumber, channel) )
	{
		// Make plot
		char title[100];
		sprintf(title, "FED %i, channel %i: Baseline B (1st 10 time slots) vs AOH bias", fedNumber, channel);
		
		TGraphErrors BaselineBPlot = BaselineB_[fedNumber][channel].makePlot(kBlue);
	
		TPaveText colorLegend(0.15,0.75,0.45,0.90,"BRNDC");
		colorLegend.AddText("TIME SLOTS NOT FOUND");
		TText* thisLine = colorLegend.AddText("Baseline B"); thisLine->SetTextColor(kBlue);
		thisLine = colorLegend.AddText("(1st 10 slots of the"); thisLine->SetTextColor(kBlue);
		thisLine = colorLegend.AddText("transparent buffer)"); thisLine->SetTextColor(kBlue);

		TCanvas c(title, title, 800, 600);
		c.GetFrame()->SetFillColor(kWhite);
		TH1F* frame = c.DrawFrame(AOHBiasMin,0-5,AOHBiasMax,1023+5);
		frame->SetXTitle("AOH bias");
		frame->SetYTitle("ADC counts");
		frame->SetTitle(title);
		colorLegend.Draw();
		BaselineBPlot.Draw("P");
		((TPad*)(&c))->Write();
		
		Attribute_Vector returnValues(1);
		returnValues[0].name_="AOHBiasSaturationPoint"; returnValues[0].value_ = timeSlotErrorCode(fedNumber, channel);
		xoap::MessageReference reply=MakeSOAPMessageReference("AOHBiasDone", returnValues);
		return reply;
	}
	
	// Check whether the given channel has a recorded scan.
	std::map<unsigned int, map <unsigned int, PixelScanRecord > >::iterator fedNumber_itr = TBMBUBDiff_.find(fedNumber);
	if (fedNumber_itr == TBMBUBDiff_.end())
	{
		Attribute_Vector returnValues(1);
		returnValues[0].name_="AOHBiasSaturationPoint"; returnValues[0].value_="AOHBiasScanNotFound";
		xoap::MessageReference reply=MakeSOAPMessageReference("AOHBiasDone", returnValues);
		return reply;
	}
	
	std::map<unsigned int, PixelScanRecord >::iterator channel_itr = fedNumber_itr->second.find(channel);
	if (channel_itr == fedNumber_itr->second.end())
	{
		Attribute_Vector returnValues(1);
		returnValues[0].name_="AOHBiasSaturationPoint"; returnValues[0].value_="AOHBiasScanNotFound";
		xoap::MessageReference reply=MakeSOAPMessageReference("AOHBiasDone", returnValues);
		return reply;
	}
	
	// Get the saturation point.
	const bool saturationPointFound = TBMBUBDiff_[fedNumber][channel].saturationPointFound(0.20);
	int saturationPoint = -1;
	if ( saturationPointFound ) saturationPoint = TBMBUBDiff_[fedNumber][channel].saturationPoint() + SaturationPointOffset;
	
	// Make plot
	char title[100];
	sprintf(title, "FED %i, channel %i: TBM B & UB vs AOH bias", fedNumber, channel);
	
	TGraphErrors TBMBPlot = TBMB_[fedNumber][channel].makePlot(kBlue);
	TGraphErrors TBMUBPlot = TBMUB_[fedNumber][channel].makePlot(kMagenta);
	TGraphErrors TBMBUBDiffPlot = TBMBUBDiff_[fedNumber][channel].makePlot(kRed);
	//Valeria
	TGraphErrors TBMBNoisePlot = TBMBNoise_[fedNumber][channel].makePlot(kBlack);
	TGraphErrors TBMUBNoisePlot = TBMUBNoise_[fedNumber][channel].makePlot(kGreen);
	// end Valeria	

	TPaveText colorLegend(0.15,0.75,0.45,0.90,"BRNDC");
	TText* thisLine = colorLegend.AddText("TBM B"); thisLine->SetTextColor(kBlue);
	thisLine = colorLegend.AddText("TBM UB"); thisLine->SetTextColor(kMagenta);
	thisLine = colorLegend.AddText("TBM B-UB Difference"); thisLine->SetTextColor(kRed);
	thisLine = colorLegend.AddText("TBM B - Noise"); thisLine->SetTextColor(kBlack);
	thisLine = colorLegend.AddText("TBM UB - Noise"); thisLine->SetTextColor(kGreen);

	if (saturationPointFound) {thisLine = colorLegend.AddText(("saturation point + "+itoa(SaturationPointOffset)).c_str());}
	else                      {thisLine = colorLegend.AddText("no saturation point found");}
	thisLine->SetTextColor(kGreen);
	
	TCanvas c(title, title, 800, 600);
	c.GetFrame()->SetFillColor(kWhite);
	TH1F* frame = c.DrawFrame(AOHBiasMin,0-5,AOHBiasMax,1023+5);
	frame->SetXTitle("AOH bias");
	frame->SetYTitle("ADC counts");
	frame->SetTitle(title);
	colorLegend.Draw();
	TBMBPlot.Draw("P");
	TBMUBPlot.Draw("P");
	TBMBUBDiffPlot.Draw("P");
	TBMBNoisePlot.Draw("P");
	TBMUBNoisePlot.Draw("P");
	TLine saturationLine( saturationPoint, 0-5, saturationPoint, 1023+5 );
	saturationLine.SetLineColor(kGreen); saturationLine.SetLineStyle(kDashed);
	if (saturationPointFound) saturationLine.Draw();
	((TPad*)(&c))->Write();
	
	// Check whether the B & UB levels changed over the scan.
	if ( TBMBUBDiff_[fedNumber][channel].constant(100.) )
	{
		Attribute_Vector returnValues(1);
		returnValues[0].name_="AOHBiasSaturationPoint"; returnValues[0].value_="NoVariation";
		xoap::MessageReference reply=MakeSOAPMessageReference("AOHBiasDone", returnValues);
		return reply;
	}
	
	// Check whether a saturation point was found.
	if (!saturationPointFound)
	{
		Attribute_Vector returnValues(1);
		returnValues[0].name_="AOHBiasSaturationPoint"; returnValues[0].value_="NoSaturationPt";
		xoap::MessageReference reply=MakeSOAPMessageReference("AOHBiasDone", returnValues);
		return reply;
	}
	
	if (printAOHBiasAdjustments) std::cout << "For FED number "<<fedNumber<<", channel "<<channel<<": B-UB difference saturates at AOHBias = " << saturationPoint << "\n";
	
	char returnString[100];
	sprintf(returnString, "%i", saturationPoint);
	Attribute_Vector returnValues(1);
	returnValues[0].name_="AOHBiasSaturationPoint"; returnValues[0].value_=returnString;
	xoap::MessageReference reply=MakeSOAPMessageReference("AOHBiasDone", returnValues);
	return reply;
} // end of FindSaturationPoint()

xoap::MessageReference PixelFEDAOHBiasCalibration::RaiseFEDReceiverInputOffsets()
{
	PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
        assert(tempCalibObject!=0);
	
	bool printFEDOffsetAdjustments = k_printFEDOffsetAdjustments_default;
	if ( tempCalibObject->parameterValue("printFEDOffsetAdjustments") == "yes" || tempCalibObject->parameterValue("printFEDOffsetAdjustments") == "true" )
		printFEDOffsetAdjustments = true;
	
	unsigned int maxFEDReceiverInputOffset = atoi( k_MaxFEDReceiverInputOffset_default.c_str() );
	if ( tempCalibObject->parameterValue("MaxFEDReceiverInputOffset") != "" )
		maxFEDReceiverInputOffset = atoi( tempCalibObject->parameterValue("MaxFEDReceiverInputOffset").c_str() );
	unsigned int target_B_max = k_TargetBMax_default;
	if ( tempCalibObject->parameterValue("TargetBMax") != "" )
		target_B_max = atoi( tempCalibObject->parameterValue("TargetBMax").c_str() );
	
	std::set< std::pair< unsigned int, unsigned int > > raisedFEDReceiverInputOffsets;
	//                   FED number    receiver number (0-3)
	
	// Loop over FEDs and channels for which we collected black levels.
	for (std::map<unsigned int, map <unsigned int, Moments > >::iterator fedNumber_itr = TBMB_Moments_.begin(); fedNumber_itr!=TBMB_Moments_.end();++fedNumber_itr)
	{
		for (std::map<unsigned int, Moments >::iterator channel_itr = fedNumber_itr->second.begin(); channel_itr!=fedNumber_itr->second.end();++channel_itr)
		{
			unsigned int fedNumber = fedNumber_itr->first;
			unsigned int channel = channel_itr->first;
			std::pair<unsigned int, unsigned int> fedNumberReceiverPair( fedNumber, (channel-1)/12 );
			unsigned long VMEBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fedNumber);
			
			// If this channel was previously found to have a bad scan, ignore it.
			if ( TBMBUBDiff_[fedNumber][channel].constant(100.) || !(TBMBUBDiff_[fedNumber][channel].saturationPointFound()) ) continue;
			
			// If this FED receiver input offset was already raised, don't do anything else to it.
			if ( raisedFEDReceiverInputOffsets.find( fedNumberReceiverPair ) != raisedFEDReceiverInputOffsets.end() ) continue;
			
			// IF this FED receiver input offset is already at maximum, don't increase it more.
			if ( FEDInterface_[VMEBaseAddress]->getPixelFEDCard().opt_inadj[(channel-1)/12] >= (int)maxFEDReceiverInputOffset ) continue;
			
			// If the black level is above the target range, increase the offset to lower the black level.
			if ( TBMB_Moments_[fedNumber][channel].mean() > target_B_max )
			{
				raisedFEDReceiverInputOffsets.insert( fedNumberReceiverPair );
				if (printFEDOffsetAdjustments) cout << "On FED number "<<fedNumber<<", channel "<<channel<<", raising FED receiver input offset from "<< FEDInterface_[VMEBaseAddress]->getPixelFEDCard().opt_inadj[(channel-1)/12] << " to " << FEDInterface_[VMEBaseAddress]->getPixelFEDCard().opt_inadj[(channel-1)/12] + 1 << "\n";
				FEDInterface_[VMEBaseAddress]->getPixelFEDCard().opt_inadj[(channel-1)/12] += 1;
				FEDInterface_[VMEBaseAddress]->set_opto_params();
			}
		}
	}
	
	// Clear the stored black levels.
	TBMB_Moments_.clear();
	
	Attribute_Vector returnValues(1);
	returnValues[0].name_="ThisCrateDone";
	if ( raisedFEDReceiverInputOffsets.empty() ) returnValues[0].value_="yes";
	else                                         returnValues[0].value_="no";
	xoap::MessageReference reply=MakeSOAPMessageReference("AOHBiasDone", returnValues);
	return reply;
}

xoap::MessageReference PixelFEDAOHBiasCalibration::RetrieveBlackLevel(unsigned int fedNumber, unsigned int channel)
{
	PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
        assert(tempCalibObject!=0);

	// Check whether the given channel has a recorded black level.
	std::map<unsigned int, map <unsigned int, Moments > >::iterator fedNumber_itr = TBMB_Moments_.find(fedNumber);
	if (fedNumber_itr == TBMB_Moments_.end())
	{
		Attribute_Vector returnValues(1);
		returnValues[0].name_="BlackLevel"; returnValues[0].value_="ChannelNotFound";
		xoap::MessageReference reply=MakeSOAPMessageReference("AOHBiasDone", returnValues);
		return reply;
	}
	
	std::map<unsigned int, Moments >::iterator channel_itr = fedNumber_itr->second.find(channel);
	if (channel_itr == fedNumber_itr->second.end())
	{
		Attribute_Vector returnValues(1);
		returnValues[0].name_="BlackLevel"; returnValues[0].value_="ChannelNotFound";
		xoap::MessageReference reply=MakeSOAPMessageReference("AOHBiasDone", returnValues);
		return reply;
	}
	
	// Check whether the recorded Moments contains any data.
	if ( TBMB_Moments_[fedNumber][channel].count() == 0 )
	{
		Attribute_Vector returnValues(1);
		returnValues[0].name_="BlackLevel"; returnValues[0].value_="ChannelNotFound";
		xoap::MessageReference reply=MakeSOAPMessageReference("AOHBiasDone", returnValues);
		return reply;
	}
	
	// Get the saturation point.
	double blackLevel = TBMB_Moments_[fedNumber][channel].mean();
	
	// Clear the black level for the next measurement.
	TBMB_Moments_[fedNumber][channel].clear();

	char returnString[100];
	sprintf(returnString, "%f", blackLevel);
	Attribute_Vector returnValues(1);
	returnValues[0].name_="BlackLevel"; returnValues[0].value_=returnString;
	xoap::MessageReference reply=MakeSOAPMessageReference("AOHBiasDone", returnValues);
	return reply;
}

xoap::MessageReference PixelFEDAOHBiasCalibration::WriteConfigFiles()
{
	// Write out a FED config file for each FED that had at least one channel with a successful AOHBias scan.
	// Also return info about the FED receiver input offsets.
	int FEDReceiverInputOffsetSum = 0;
	unsigned int FEDReceiverInputOffsetNum = 0;
	int FEDReceiverInputOffsetMin = 16;
	int FEDReceiverInputOffsetMax = -1;
	
	for (std::map<unsigned int, map <unsigned int, PixelScanRecord > >::iterator fedNumber_itr = TBMBUBDiff_.begin(); fedNumber_itr!=TBMBUBDiff_.end();++fedNumber_itr)
	{
		unsigned int fedNumber = fedNumber_itr->first;
		unsigned long VMEBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fedNumber);
		
		std::set<unsigned int> opticalReceiversChanged;
		
		for ( std::map<unsigned int, PixelScanRecord >::iterator channel_itr = fedNumber_itr->second.begin(); channel_itr != fedNumber_itr->second.end(); channel_itr++ )
		{
			unsigned int channel = channel_itr->first;
			
			// If this channel was previously found to have a bad scan, ignore it.
			if ( badTimeSlotInfo(fedNumber, channel) || TBMBUBDiff_[fedNumber][channel].constant(100.) || !(TBMBUBDiff_[fedNumber][channel].saturationPointFound()) ) continue;
			
			// Move on if we've already taken care of this optical receiver.
			if ( opticalReceiversChanged.find((channel-1)/12) != opticalReceiversChanged.end() ) continue;
			
			opticalReceiversChanged.insert((channel-1)/12);
			int thisChannelFEDReceiverInputOffset = FEDInterface_[VMEBaseAddress]->getPixelFEDCard().opt_inadj[(channel-1)/12];
			FEDReceiverInputOffsetSum += thisChannelFEDReceiverInputOffset;
			FEDReceiverInputOffsetNum++;
			if ( thisChannelFEDReceiverInputOffset < FEDReceiverInputOffsetMin ) FEDReceiverInputOffsetMin = thisChannelFEDReceiverInputOffset;
			if ( thisChannelFEDReceiverInputOffset > FEDReceiverInputOffsetMax ) FEDReceiverInputOffsetMax = thisChannelFEDReceiverInputOffset;
		}
		
		if ( opticalReceiversChanged.size() > 0 )
		{
			FEDInterface_[VMEBaseAddress]->getPixelFEDCard().restoreControlAndModeRegister();
			FEDInterface_[VMEBaseAddress]->getPixelFEDCard().writeASCII(outputDir());
			std::cout << "Wrote config file for FED #"<<fedNumber<<"\n";
		}
	}
	double FEDReceiverInputOffsetMean = 0.;
	if (FEDReceiverInputOffsetNum > 0) FEDReceiverInputOffsetMean = ((double)(FEDReceiverInputOffsetSum))/((double)(FEDReceiverInputOffsetNum));
	
	char meanString[100]; sprintf(meanString, "%f", FEDReceiverInputOffsetMean);
	Attribute_Vector returnValues(4);
	returnValues[0].name_="FEDReceiverInputOffsetMean"; returnValues[0].value_=meanString;
	returnValues[1].name_="FEDReceiverInputOffsetNum";  returnValues[1].value_=itoa(FEDReceiverInputOffsetNum);
	returnValues[2].name_="FEDReceiverInputOffsetMin";  returnValues[2].value_=itoa(FEDReceiverInputOffsetMin);
	returnValues[3].name_="FEDReceiverInputOffsetMax";  returnValues[3].value_=itoa(FEDReceiverInputOffsetMax);
	xoap::MessageReference reply=MakeSOAPMessageReference("AOHBiasDone", returnValues);
	
	// Clear member data.
	TBMHeaderStart_.clear();
	TBMTrailerStart_.clear();
	TBMUB_.clear();
	TBMB_.clear();
	TBMBUBDiff_.clear();
	TBMUBNoise_.clear();
	TBMB_Moments_.clear();
	
	return reply;
}

bool PixelFEDAOHBiasCalibration::badTimeSlotInfo(unsigned int fedNumber, unsigned int channel)
{
	const std::pair<TimeSlotCode, unsigned int> timeSlotCode = TimeSlotCodeForChannel(fedNumber, channel);
	
	return ( timeSlotCode.first != kGoodSingleTimeSlot && timeSlotCode.first != kGoodJumpingByOneTimeSlot );
}

std::pair<PixelFEDAOHBiasCalibration::TimeSlotCode, unsigned int> PixelFEDAOHBiasCalibration::TimeSlotCodeForChannel(unsigned int fedNumber, unsigned int channel)
{
	const PixelMode& TBMHeaderStart = TBMHeaderStart_[fedNumber][channel];
	const unsigned int totalEntries = TBMHeaderStart.totalEntries();
	
	if ( totalEntries <= 20 ) return std::pair<TimeSlotCode, unsigned int>( kNotEnoughTimeSlotEntries, totalEntries );
	
	std::vector<std::pair< int, unsigned int > > topTwoValues = TBMHeaderStart.topTwoValues();
	
	assert( topTwoValues.size() != 0 );
	if ( ((double)(topTwoValues[0].second))/((double)(totalEntries)) >= 0.95 )
	{
		//std::cout << "Time slot: " << topTwoValues[0].first << std::endl;
		return std::pair<TimeSlotCode, unsigned int>( kGoodSingleTimeSlot, topTwoValues[0].first );
	}
	
	if ( topTwoValues.size() >= 2 )
	{
		if (
		        ((double)(topTwoValues[0].second+topTwoValues[1].second))/((double)(totalEntries)) >= 0.95
		     && abs(topTwoValues[0].first-topTwoValues[1].first) == 1
		   )
		{
			return std::pair<TimeSlotCode, unsigned int>( kGoodJumpingByOneTimeSlot, std::min(topTwoValues[0].first, topTwoValues[1].first) );
		}
	}
	
	return std::pair<TimeSlotCode, unsigned int>( kTimeSlotFluctuates, 0 );
	
}

std::string PixelFEDAOHBiasCalibration::timeSlotErrorCode(unsigned int fedNumber, unsigned int channel)
{
	assert( badTimeSlotInfo(fedNumber, channel) );
	char tempString[100];
	
	const std::pair<TimeSlotCode, unsigned int> timeSlotCode = TimeSlotCodeForChannel(fedNumber, channel);
	
	if ( timeSlotCode.first == kNotEnoughTimeSlotEntries )
	{
		sprintf(tempString, "Only%iTimeSlotReadings", timeSlotCode.second);
	}
	else if ( timeSlotCode.first == kTimeSlotFluctuates )
	{
		sprintf(tempString, "TimeSlotFluctuates");
	}
	else assert(0);
	return tempString;
}

void PixelFEDAOHBiasCalibration::initializeFED(){

  setFEDModeAndControlRegister(0x0,0x30019);
  setBlackUBTrans();
}

xoap::MessageReference PixelFEDAOHBiasCalibration::beginCalibration(xoap::MessageReference msg){

	// Remove gray background from plots.
	plainStyle_ = new TStyle("Plain", "a plain style");
	plainStyle_->cd();
	
	PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
	assert(tempCalibObject!=0);
	
	outputFile_ = new TFile((outputDir()+"/"+tempCalibObject->mode()+"Calibration_FEDCrate"+stringF(crate_)+".root").c_str(), "RECREATE", ("plots from "+tempCalibObject->mode()+" calibration in FED crate # "+stringF(crate_)).c_str());

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDAOHBiasCalibration::endCalibration(xoap::MessageReference msg){

	delete plainStyle_;
	delete outputFile_;

  cout << "In PixelFEDAOHBiasCalibration::endCalibration()" << endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

