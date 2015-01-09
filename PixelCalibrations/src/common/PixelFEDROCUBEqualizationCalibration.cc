// $Id: PixelFEDROCUBEqualizationCalibration.cc,v 1.42 2012/02/17 15:42:16 mdunser Exp $: PixelAddressLevelCalibration.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelFEDROCUBEqualizationCalibration.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelDecodedFEDRawData.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"

#include <toolbox/convertstring.h>

#include "TFile.h"
#include "TStyle.h"
#include "TH2F.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TPaveText.h"
#include "TROOT.h"
#include "TTree.h"
#include "TBranch.h"

#include "iomanip"

#define BPIX

using namespace pos;
using namespace std;

//PixelFEDROCUBEqualizationCalibration::PixelFEDROCUBEqualizationCalibration() : PixelFEDCalibrationBase()
//{
//  std::cout << "Greetings from the PixelFEDROCUBEqualizationCalibration default constructor." << std::endl;
//}

PixelFEDROCUBEqualizationCalibration::PixelFEDROCUBEqualizationCalibration(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  std::cout << "Greetings from the PixelFEDROCUBEqualizationCalibration copy constructor." << std::endl;
}

xoap::MessageReference PixelFEDROCUBEqualizationCalibration::execute(xoap::MessageReference msg)
{
  assert(0);
	Attribute_Vector parameters(2);
	parameters[0].name_="WhatToDo";
	parameters[1].name_="StateNum";
	Receive(msg, parameters);
	
	unsigned int state = atoi(parameters[1].value_.c_str());

	if (parameters[0].value_=="RetrieveData")
	{
		RetrieveData(state);
	}
	else if (parameters[0].value_=="Analyze")
	{
		Analyze();
	} // end of "Analyze" block
	else
	{
		cout << "ERROR: PixelFEDROCUBEqualizationCalibration::execute() does not understand the WhatToDo command, "<< parameters[0].value_ <<", sent to it.\n";
		assert(0);
	}

	xoap::MessageReference reply = MakeSOAPMessageReference("ROCUBEqualizationDone");
	return reply;
}

void PixelFEDROCUBEqualizationCalibration::RetrieveData(unsigned int state)
{
	PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
	assert(tempCalibObject!=0);

	bool printFEDRawData = false;
	if ( tempCalibObject->parameterValue("printFEDRawData") == "yes" || tempCalibObject->parameterValue("printFEDRawData") == "true" )
		printFEDRawData = true;
	
	const std::vector<PixelROCName> ROCsToCalibrate=tempCalibObject->rocList();
	
	std::vector<std::pair<unsigned int,std::vector<unsigned int> > > fedsAndChannels = tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_ );
	               //     FED number,              channels
	
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
				std::cout<<"PixelFEDROCUBEqualizationCalibration::execute() -- Could not drain FIFO 1 of FED Channel "<<channel<<" in transparent mode!"<<std::endl;
				diagService_->reportError("PixelFEDROCUBEqualizationCalibration::execute() -- Could not drain FIFO 1 in transparent mode!",DIAGWARN);
			}

			// Decode the buffer.
			if ( !tempCalibObject->noHits() )
			{
				std::cout << "WARNING: This calibration requires that there are no hits, so the hits you specified in the calib object (e.g., calib.dat) have been cleared.  To eliminate this message, please change the calib object to produce no hits." << std::endl;
			}
			std::vector<unsigned int> hitsOnROCs(ROCsOnThisChannel.size(), 0); // 0 hits on each channel
			PixelDecodedFEDRawData decodedRawData(buffer, 100., 100., 150., hitsOnROCs);
			if (printFEDRawData)
			{
				std::cout << "*** Raw Data for FED # " << fednumber << ", channel " << channel << " ***" << std::endl;
				decodedRawData.printToStream(cout);
				std::cout << "**************************\n";
				std::cout << std::endl;
			}

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

			// Add values from the decoded data to the moments.
			TBM_UB_[fednumber][channel].push_back(decodedRawData.TBMHeader().UB1());
			TBM_UB_[fednumber][channel].push_back(decodedRawData.TBMHeader().UB2());
			TBM_UB_[fednumber][channel].push_back(decodedRawData.TBMHeader().UB3());
			TBM_UB_[fednumber][channel].push_back(decodedRawData.TBMTrailer().UB1());
			TBM_UB_[fednumber][channel].push_back(decodedRawData.TBMTrailer().UB2());

			TBM_B_[fednumber][channel].push_back(decodedRawData.TBMHeader().B());
			TBM_B_[fednumber][channel].push_back(decodedRawData.TBMTrailer().B1());
			TBM_B_[fednumber][channel].push_back(decodedRawData.TBMTrailer().B2());

			// Fill info about the ROC UBs and Bs.
			// Loop over ROCs on this channel.
			for( unsigned int ROCNumber = 0; ROCNumber < ROCsOnThisChannel.size(); ROCNumber++)
			{
				// Check whether this ROC is in the list of ROCs we want to calibrate.  If not, skip to the next.
				std::vector<PixelROCName>::const_iterator foundROC = find(ROCsToCalibrate.begin(), ROCsToCalibrate.end(), ROCsOnThisChannel[ROCNumber]);
				if ( foundROC == ROCsToCalibrate.end() ) continue;
				
				// Skip if we're in singleROC mode, and this ROC is not being calibrated right now.
				if ( !(tempCalibObject->scanningROCForState(ROCsOnThisChannel[ROCNumber], state)) ) continue;

				// Ignore this iteration if the readings were 0 or 1023.
				if ( decodedRawData.ROCOutput(ROCNumber).header().UB() <= 0 || decodedRawData.ROCOutput(ROCNumber).header().UB() >= 1023 || decodedRawData.ROCOutput(ROCNumber).header().B() <= 0 || decodedRawData.ROCOutput(ROCNumber).header().B() >= 1023 )
				{
					continue;
				}

				unsigned int VIbias_DAC_value = 0; //tempCalibObject->scanValue(k_DACName_VIbias_DAC, state , ROCsOnThisChannel[ROCNumber]);

				// Fill in UB and B readings for this ROC.
				ROC_UB_[ROCsOnThisChannel[ROCNumber]].addEntry(VIbias_DAC_value, decodedRawData.ROCOutput(ROCNumber).header().UB());
				ROC_B_[ROCsOnThisChannel[ROCNumber]].addEntry(VIbias_DAC_value, decodedRawData.ROCOutput(ROCNumber).header().B());
			}

			// Done filling in information from the decoded data.

		} // end of loop over channels on this FED board
		VMEPtr_[vmeBaseAddress]->write("LRES",0x80000000); // Local Reset
		VMEPtr_[vmeBaseAddress]->write("CLRES",0x80000000); // *** MAURO ***
	} // end of loop over FEDs in this crate
}

// Class used in sorting the ROCs for the summary.
class PixelROCOrderer
{
	public:
	
	enum {kNoValidDecodedTriggers, kCantFindCrossing, kROCUBTooLow, kSuccess, kROCUBTooHigh, kNumCodes};
	
	PixelROCOrderer( unsigned int classificationCode, double orderingWithinCode, PixelROCName roc )
	{
		assert( classificationCode < kNumCodes );
		classificationCode_ = classificationCode;
		orderingWithinCode_ = orderingWithinCode;
		roc_ = roc;
	}
	
	unsigned int code() const {return classificationCode_;}
	const PixelROCName& roc() const {return roc_;}
	
	const bool operator<(const PixelROCOrderer& another) const
	{
		if ( classificationCode_ < another.classificationCode_ ) return true;
		if ( classificationCode_ == another.classificationCode_ && orderingWithinCode_ < another.orderingWithinCode_ ) return true;
		return false;
	}
	
	private:
	unsigned int classificationCode_;
	double orderingWithinCode_;
	PixelROCName roc_;
};

void PixelFEDROCUBEqualizationCalibration::Analyze()
{
#if 0
	// Remove gray background from plots.
	TStyle plainStyle("Plain", "a plain style");
	plainStyle.SetOptStat(0); // Suppress statistics box.
	plainStyle.cd();
	
	PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
	assert(tempCalibObject!=0);
	
	TFile outputFile((outputDir()+"/"+tempCalibObject->mode()+"Calibration_FEDCrate"+stringF(crate_)+".root").c_str(), "RECREATE", ("plots from "+tempCalibObject->mode()+" calibration in FED crate # "+stringF(crate_)).c_str());
	
	bool printScan = false;
	if ( tempCalibObject->parameterValue("printScan") == "yes" || tempCalibObject->parameterValue("printScan") == "true" )
		printScan = true;
	
#ifdef BPIX
	int minTarget_UB = 0;
	if ( tempCalibObject->parameterValue("TargetUB") != "" )
	{
	  minTarget_UB = atoi(tempCalibObject->parameterValue("TargetUB").c_str());
	  cout<<" Set the minimum ROC UB to "<<minTarget_UB;
	}
#endif

	const std::vector<PixelROCName> ROCsToCalibrate=tempCalibObject->rocList();
	
	std::map <PixelROCName, bits8 > recommended_VIbias_DAC_values;
	unsigned int totalROCs = 0, ROCsCalibratedInOtherCrates = 0, succeededROCs = 0;
	std::map<pos::PixelROCName, std::string> failures;
	std::map<pos::PixelROCName, std::string> warnings;

	// This variable is used to order the ROCs in the summary plot.  It is filled in as we loop over ROCs looking for the best VIbias_DAC.
	std::vector<PixelROCOrderer> ROCOrderForSummary;

	branch theBranch;
	branch_sum theBranch_sum;
	TDirectory* dirSummaries = gDirectory->mkdir("SummaryTrees","SummaryTrees");
	dirSummaries->cd();
	TTree* tree = new TTree("PassState","PassState");
	tree->Branch("PassState",&theBranch,"pass/F:rocName/C", 4096000);
	TTree* tree_sum = new TTree("SummaryInfo","SummaryInfo");
	tree_sum->Branch("SummaryInfo",&theBranch_sum,"new_VIbias_DAC/F:delta_VIbias_DAC/F:rocName/C", 4096000);  
      
	outputFile.cd();

	PixelRootDirectoryMaker rootDirs(ROCsToCalibrate,gDirectory);

	// Loop over all ROCs in the list of ROCs to calibrate.
	assert( ROC_UB_.size() == ROC_B_.size() );
	for( std::vector<PixelROCName>::const_iterator ROCsToCalibrate_itr = ROCsToCalibrate.begin(); ROCsToCalibrate_itr != ROCsToCalibrate.end(); ROCsToCalibrate_itr++ )
	{
	  theBranch.pass=1;
	  std::string rocName = (*ROCsToCalibrate_itr).rocname();
	  strcpy(theBranch.rocName,rocName.c_str());
	       
		rootDirs.cdDirectory(*(ROCsToCalibrate_itr));
		
		// Get hardware address of this ROC.
		const PixelHdwAddress *aROC_hardware=theNameTranslation_->getHdwAddress(*ROCsToCalibrate_itr);
		unsigned int fednumber = aROC_hardware->fednumber();
		unsigned int crate=theFEDConfiguration_->crateFromFEDNumber(fednumber);
		if (crate!=crate_)
		{
			ROCsCalibratedInOtherCrates++;
			continue; // Ignore if this ROC is not in the crate controlled by this PixelFEDSupervisor.
		}
		unsigned int channel=aROC_hardware->fedchannel();

		std::map<PixelROCName, PixelScanRecord>::iterator foundROC = ROC_UB_.find(*ROCsToCalibrate_itr);
		if ( foundROC == ROC_UB_.end() ) // no scan found for this ROC
		{
			failures[*ROCsToCalibrate_itr] = "NoValidDecodedTriggers";
			ROCOrderForSummary.push_back(PixelROCOrderer(PixelROCOrderer::kNoValidDecodedTriggers, 0, *ROCsToCalibrate_itr));
			continue;
		}
		
		PixelScanRecord& UBScanForThisROC = foundROC->second;
		std::map <unsigned int, map <unsigned int, Moments> >::const_iterator foundfednumber = TBM_UB_.find(fednumber);
		assert(foundfednumber != TBM_UB_.end());
		std::map <unsigned int, Moments>::const_iterator foundchannel = foundfednumber->second.find(channel);
		assert(foundchannel != foundfednumber->second.end());
		double target_UB = foundchannel->second.mean();
#ifdef BPIX
		if(target_UB<minTarget_UB) target_UB=minTarget_UB;
#endif
		
		UBScanForThisROC.setTitle(ROCsToCalibrate_itr->rocname()+": ROC UB vs "+k_DACName_VIbias_DAC);
		UBScanForThisROC.setXVarName(k_DACName_VIbias_DAC);
		UBScanForThisROC.setYVarName("ROC UB");
		if (printScan)
		{
			std::cout << "**** For ROC " << *ROCsToCalibrate_itr << " ****\n";
			UBScanForThisROC.printScanToStream(std::cout, target_UB, "TBM UB");
		}
		if ( UBScanForThisROC.crossingPointFound(target_UB) )
		{
		  // This is a hack to prevent the selection of very low points in case of double crosings d.k. 8/2/11
		  if(UBScanForThisROC.crossingPoint() < 20 ) { // accept only high enough values
		    cout<<k_DACName_VIbias_DAC<<" skip points below 20 "<<UBScanForThisROC.crossingPoint()<<endl;
		  } else {
		    recommended_VIbias_DAC_values[*ROCsToCalibrate_itr] = UBScanForThisROC.crossingPoint();
		    succeededROCs++;
		    if (printScan) cout << "Recommended "<<k_DACName_VIbias_DAC<<" = " << (int)(recommended_VIbias_DAC_values[*ROCsToCalibrate_itr]) << "\n\n";
		    
		    ROCOrderForSummary.push_back(PixelROCOrderer(PixelROCOrderer::kSuccess, (int)(recommended_VIbias_DAC_values[*ROCsToCalibrate_itr]), *ROCsToCalibrate_itr));
		  }
		}
		else if ( UBScanForThisROC.crossingPointFound(target_UB, PixelScanRecord::kFirst) )
		{
			warnings[*ROCsToCalibrate_itr] = "MultipleCrossings";
			recommended_VIbias_DAC_values[*ROCsToCalibrate_itr] = UBScanForThisROC.crossingPoint();
			succeededROCs++;
			if (printScan) cout << "WARNING: The ROC UB crosses the TBM UB multiple times.\n";
			if (printScan) cout << "Recommended "<<k_DACName_VIbias_DAC<<" = " << (int)(recommended_VIbias_DAC_values[*ROCsToCalibrate_itr]) << "\n\n";
			
			ROCOrderForSummary.push_back(PixelROCOrderer(PixelROCOrderer::kSuccess, (int)(recommended_VIbias_DAC_values[*ROCsToCalibrate_itr]), *ROCsToCalibrate_itr));
		}
		else
		{
			if ( UBScanForThisROC.maxY().mean() < target_UB )
			{
				failures[*ROCsToCalibrate_itr] = "ROCUBTooLow";
				recommended_VIbias_DAC_values[*ROCsToCalibrate_itr] = UBScanForThisROC.minX();
				
				ROCOrderForSummary.push_back(PixelROCOrderer(PixelROCOrderer::kROCUBTooLow, UBScanForThisROC.maxY().mean(), *ROCsToCalibrate_itr));
			}
			else if ( UBScanForThisROC.minY().mean() > target_UB )
			{
				failures[*ROCsToCalibrate_itr] = "ROCUBTooHigh";
				recommended_VIbias_DAC_values[*ROCsToCalibrate_itr] = UBScanForThisROC.maxX();
				
				ROCOrderForSummary.push_back(PixelROCOrderer(PixelROCOrderer::kROCUBTooHigh, UBScanForThisROC.minY().mean(), *ROCsToCalibrate_itr));
			}
			else
			{
				failures[*ROCsToCalibrate_itr] = "Can'tFindCrossing";
				
				ROCOrderForSummary.push_back(PixelROCOrderer(PixelROCOrderer::kCantFindCrossing, UBScanForThisROC.minY().mean(), *ROCsToCalibrate_itr));
			}
			
			 theBranch.pass=0;
		}
		
		// Output plot.
		UBScanForThisROC.printPlot();
		tree->Fill();
	} // end of loop over ROCs to calibrate.

	// Create summary plot.
	const double ymin = -512;
	const double ymax = 512;
	const unsigned int nbinsy = (unsigned int)(ymax-ymin + 1 + 0.1);

	std::sort(ROCOrderForSummary.begin(), ROCOrderForSummary.end());
	
	gROOT->cd(); // Create this histogram outside the output file; otherwise it will be deleted twice -- once when closing the file and once when deleting summaryPlot.
	TH2F summaryPlot("ROCUBEqualizationSummaryHistogram", "summary histogram for ROC UB equalization calibration", ROCOrderForSummary.size(), 0+0.5, ROCOrderForSummary.size()+0.5, nbinsy, ymin-0.5, ymax+0.5);
	rootDirs.cdDirectory("");
	summaryPlot.GetXaxis()->SetTitle("ROC index");
	summaryPlot.GetYaxis()->SetTitle("ROC UB - TBM UB");
	// bin content = VIbias_DAC
	
	unsigned int currentROC = 1;
	unsigned int ROCsWithCode[PixelROCOrderer::kNumCodes];
	for( unsigned int i = 0; i < PixelROCOrderer::kNumCodes; i++ ) ROCsWithCode[i] = 0;
	for( std::vector<PixelROCOrderer>::const_iterator ROC_itr = ROCOrderForSummary.begin(); ROC_itr != ROCOrderForSummary.end(); ROC_itr++, currentROC++ )
	{
		assert( currentROC <= (unsigned int)summaryPlot.GetNbinsX() );

		ROCsWithCode[ROC_itr->code()] += 1;

		std::map<PixelROCName, PixelScanRecord>::iterator foundROC = ROC_UB_.find(ROC_itr->roc());
		if ( foundROC == ROC_UB_.end() ) // no scan found for this ROC
		{
			//for ( int j = 1; j <= summaryPlot.GetNbinsY(); j++ ) summaryPlot.SetBinContent(currentROC, j, failureCode);
		}
		else
		{
			PixelScanRecord& UBScanForThisROC = foundROC->second;
			
			// Get TBM UB.
			const PixelHdwAddress *aROC_hardware=theNameTranslation_->getHdwAddress(ROC_itr->roc());
			unsigned int fednumber = aROC_hardware->fednumber();
			unsigned int channel   = aROC_hardware->fedchannel();
			std::map <unsigned int, map <unsigned int, Moments> >::const_iterator foundfednumber = TBM_UB_.find(fednumber);
			assert(foundfednumber != TBM_UB_.end());
			std::map <unsigned int, Moments>::const_iterator foundchannel = foundfednumber->second.find(channel);
			assert(foundchannel != foundfednumber->second.end());
			const double TBMUB = foundchannel->second.mean();
			
			for ( int j = 1; j <= summaryPlot.GetNbinsY(); j++ )
			{
				const double ROCUBMinusTBMUB = ymin + (ymax-ymin)*((double)(j-1))/((double)(summaryPlot.GetNbinsY()-1));
				if (UBScanForThisROC.crossingPointFound(ROCUBMinusTBMUB + TBMUB, PixelScanRecord::kLast))
				{
					const double VIbias_DAC_value = UBScanForThisROC.crossingPoint();
					summaryPlot.SetBinContent(currentROC, j, VIbias_DAC_value);
				}
			}
		}
	}
	TCanvas summaryCanvas("ROCUBEqualizationSummary", "summary plot for ROC UB equalization calibration");
	summaryPlot.Draw("COLZ"); // Draw with a color scale.
	
	TPaveText* noDataBox = 0;
	if ( ROCsWithCode[PixelROCOrderer::kNoValidDecodedTriggers] != 0 )
	{
		noDataBox = new TPaveText(summaryPlot.GetXaxis()->GetXmin(), summaryPlot.GetYaxis()->GetXmin(), ROCsWithCode[PixelROCOrderer::kNoValidDecodedTriggers]+0.5, summaryPlot.GetYaxis()->GetXmax(), "bl");
		TText* text = noDataBox->AddText("NO DATA");
		text->SetTextAngle(90);
		noDataBox->SetTextColor(kRed);
		noDataBox->SetBorderSize(1);
		noDataBox->SetLineColor(kRed);
		noDataBox->SetFillColor(kRed);
		noDataBox->SetFillStyle(3004);
		noDataBox->Draw();
	}
	TLine successLeftLine;
	unsigned int ROCsLeftOfSuccessLeftLine = 0;
	for (unsigned int i = 0; i < PixelROCOrderer::kSuccess; i++) ROCsLeftOfSuccessLeftLine += ROCsWithCode[i];
	if ( ROCsLeftOfSuccessLeftLine != 0 )
	{
		successLeftLine = TLine( ROCsLeftOfSuccessLeftLine+0.5, summaryPlot.GetYaxis()->GetXmin(), ROCsLeftOfSuccessLeftLine+0.5, summaryPlot.GetYaxis()->GetXmax() );
		successLeftLine.SetLineColor(kGreen); successLeftLine.SetLineStyle(kSolid);
		successLeftLine.Draw();
	}
	TLine successRightLine;
	unsigned int ROCsLeftOfSuccessRightLine = 0;
	for (unsigned int i = 0; i <= PixelROCOrderer::kSuccess; i++) ROCsLeftOfSuccessRightLine += ROCsWithCode[i];
	if ( ROCsLeftOfSuccessRightLine != ROCOrderForSummary.size() )
	{
		successRightLine = TLine( ROCsLeftOfSuccessRightLine+0.5, summaryPlot.GetYaxis()->GetXmin(), ROCsLeftOfSuccessRightLine+0.5, summaryPlot.GetYaxis()->GetXmax() );
		successRightLine.SetLineColor(kGreen); successRightLine.SetLineStyle(kSolid);
		successRightLine.Draw();
	}
	
	TLine targetLine( summaryPlot.GetXaxis()->GetXmin(), 0, summaryPlot.GetXaxis()->GetXmax(), 0 );
	targetLine.SetLineColor(kBlack); targetLine.SetLineStyle(kSolid);
	targetLine.Draw();
	
	((TPad*)(&summaryCanvas))->Write();
	delete noDataBox;
	// Done with summary plot.

	// Write out new configuration files.
	Moments delta_VIbias_DAC_values, new_VIbias_DAC_values;
	// Loop over all modules in the configuration.
	for (std::vector<PixelModuleName>::const_iterator module_name = theDetectorConfiguration_->getModuleList().begin(); module_name != theDetectorConfiguration_->getModuleList().end(); ++module_name)
	{
		// First we need to get the DAC settings for the ROCs on this module.
		PixelDACSettings *DACSettingsForThisModule=0; // Each PixelDACSettings object contains the settings for all ROCS on a particular module.
		std::string modulePath = module_name->modulename();
		PixelConfigInterface::get(DACSettingsForThisModule, "pixel/dac/"+modulePath, *theGlobalKey_);
		assert(DACSettingsForThisModule!=0);

		// Now loop over ROCs on this module and change each VIbias_DAC value to the recommended value.
		bool changedOne = false;
		for ( unsigned int ROCNumber = 0; ROCNumber < DACSettingsForThisModule->numROCs(); ROCNumber++ )
		{
			totalROCs++;
			PixelROCName nameOfThisROC = DACSettingsForThisModule->getDACSettings(ROCNumber).getROCName();
			// Check whether this ROC is in the list of ROCs with new VIbias_DAC values.  If not, skip to the next.
			std::map<PixelROCName, bits8>::const_iterator foundROC = recommended_VIbias_DAC_values.find(nameOfThisROC);
			if ( foundROC == recommended_VIbias_DAC_values.end() ) continue;

			// Change VIbias_DAC for this ROC.
			changedOne = true;
			delta_VIbias_DAC_values.push_back( foundROC->second - DACSettingsForThisModule->getDACSettings(nameOfThisROC)->getVIbias_DAC() );
			new_VIbias_DAC_values.push_back( foundROC->second );
			
			std::string rocName_=nameOfThisROC.rocname();
			strcpy(theBranch_sum.rocName, rocName_.c_str());

			theBranch_sum.new_VIbias_DAC=foundROC->second;
			theBranch_sum.delta_VIbias_DAC=foundROC->second-DACSettingsForThisModule->getDACSettings(nameOfThisROC)->getVIbias_DAC();
			tree_sum->Fill();

			DACSettingsForThisModule->getDACSettings(nameOfThisROC)->setVIbias_DAC(foundROC->second);
			// Set VIbias_roc to the value used in this calibration.
			for ( unsigned int i = 0; i < tempCalibObject->numberOfScanVariables(); i++ )
			{
				if ( tempCalibObject->scanName(i) == k_DACName_VIbias_roc )
				{
					assert( tempCalibObject->scanValueMin(k_DACName_VIbias_roc) == tempCalibObject->scanValueMax(k_DACName_VIbias_roc) );
					DACSettingsForThisModule->getDACSettings(nameOfThisROC)->setVIbias_roc( (int)(tempCalibObject->scanValueMin(k_DACName_VIbias_roc)) );
					break;
				}
			}
		} // end of loop over ROCs on this module

		// Write out new file.
		if ( changedOne )
		{
			std::string moduleName = module_name->modulename();
			DACSettingsForThisModule->writeASCII(outputDir());
			std::cout << "Wrote dac settings for module:" << moduleName << endl;
		}
		
		delete DACSettingsForThisModule;
	} // end of loop over modules

	// Summary report.
	assert( succeededROCs+failures.size()+ROCsCalibratedInOtherCrates == ROCsToCalibrate.size() );
	cout << "\n";
	cout << "---------------------------------------------------------\n";
	cout << "ROC UB Equalization Calibration Report for FED crate #"<<crate_<<"\n";
	cout << "---------------------------------------------------------\n";
	cout << setw(4) << totalROCs << " ROCs in the configuration, of these:\n";
	cout << setw(4) << totalROCs-ROCsToCalibrate.size() << " ROCs were not tested.\n";
	cout << setw(4) << ROCsCalibratedInOtherCrates << " ROCs were tested by other PixelFEDSupervisors (other crates).\n";
	cout << setw(4) << succeededROCs << " ROCs had a successful calibration.\n";
	cout << setw(4) << failures.size() << " ROCs failed the calibration.\n";
	cout << setw(4) << warnings.size() << " of the successful ROCs had warnings.\n";

	cout << "\nSummary of new "<<k_DACName_VIbias_DAC<<" values (including failed ROCs):\n";
	cout <<   "mean     =" << setw(6)<<right << new_VIbias_DAC_values.mean()   << "\n";
	cout <<   "std.dev. =" << setw(6)<<right << new_VIbias_DAC_values.stddev() << "\n";
	cout <<   "smallest =" << setw(6)<<right << new_VIbias_DAC_values.min()    << "\n";
	cout <<   "largest  =" << setw(6)<<right << new_VIbias_DAC_values.max()    << "\n";

	cout << "\nSummary of changes from old to new "<<k_DACName_VIbias_DAC<<" values (new-old):\n";
	cout <<   "mean          =" << setw(6)<<right << delta_VIbias_DAC_values.mean()   << "\n";
	cout <<   "std.dev.      =" << setw(6)<<right << delta_VIbias_DAC_values.stddev() << "\n";
	cout <<   "most negative =" << setw(6)<<right << delta_VIbias_DAC_values.min()    << "\n";
	cout <<   "most positive =" << setw(6)<<right << delta_VIbias_DAC_values.max()    << "\n";

	cout << "\nFailed ROCs (total of " << failures.size() << "):\n";
	if ( failures.size() != 0 )
	{
		cout <<   "ROC" << string(42-3, ' ') << "Failure Mode\n";
		for ( std::map<PixelROCName, std::string>::const_iterator failures_itr = failures.begin(); failures_itr != failures.end(); failures_itr++ )
		{
			cout << failures_itr->first << string(42-min(42,(int)failures_itr->first.rocname().size()),' ') << failures_itr->second << endl;
		}
		cout << "\nFailure mode descriptions:\n";
		cout <<   "NoValidDecodedTriggers = The channel which this ROC is on was never\n";
		cout <<   "                           successfully decoded on any trigger." << endl;
		cout <<   "ROCUBTooLow            = For all values of "<<k_DACName_VIbias_DAC<<",\n";
		cout <<   "                           the ROC UB level was below the TBM UB." << endl;
		cout <<   "ROCUBTooHigh           = For all values of "<<k_DACName_VIbias_DAC<<",\n";
		cout <<   "                           the ROC UB level was above the TBM UB." << endl;
		cout <<   "Can'tFindCrossing      = Even though the TBM UB lies inside the ROC UB range,\n";
		cout <<   "                           no crossing could be found.  This is probably\n";
		cout <<   "                           due to points ignored due to large uncertainties.\n";
	}
	
	cout << "\nROCs with warnings (total of " << warnings.size() << "):\n";
	if ( warnings.size() != 0 )
	{
		cout <<   "ROC" << string(42-3, ' ') << "Warning\n";
		for ( std::map<PixelROCName, std::string>::const_iterator warnings_itr = warnings.begin(); warnings_itr != warnings.end(); warnings_itr++ )
		{
			cout << warnings_itr->first << string(42-min(42,(int)warnings_itr->first.rocname().size()),' ') << warnings_itr->second << endl;
		}
		cout << "\nWarning descriptions:\n";
		cout <<   "MultipleCrossings      = The ROC UB crossed the TBM UB more than once.\n";
		cout <<   "                            The crossing at smallest\n";
		cout <<   "                            "<<k_DACName_VIbias_DAC<<" was selected.\n";
	}

	cout << "--------------------------------------" << endl;

	// Clear the member data.
	TBM_UB_.clear();
	TBM_B_.clear();
	ROC_UB_.clear();
	ROC_B_.clear();
	
	outputFile.Write();
	outputFile.Close();
#endif
}

void PixelFEDROCUBEqualizationCalibration::initializeFED(){

  setFEDModeAndControlRegister(0x0,0x30019);

}

xoap::MessageReference PixelFEDROCUBEqualizationCalibration::beginCalibration(xoap::MessageReference msg){

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDROCUBEqualizationCalibration::endCalibration(xoap::MessageReference msg){
  cout << "In PixelFEDROCUBEqualizationCalibration::endCalibration()" << endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

