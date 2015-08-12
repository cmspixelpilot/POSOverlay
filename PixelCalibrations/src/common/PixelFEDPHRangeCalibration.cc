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

#include "PixelCalibrations/include/PixelFEDPHRangeCalibration.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO3Decoder.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"

#include "TCanvas.h"
#include "TFrame.h"
#include "TH1F.h"
#include "TPaveText.h"
#include "TText.h"
#include "TFile.h"
#include "TStyle.h"
#include "TGraphErrors.h"
#include "TGraph2DErrors.h"
#include "TMarker.h"
#include "TH1I.h"
#include "TTree.h"

#include <toolbox/convertstring.h>

#include "iomanip"

using namespace pos;
using namespace std;

//PixelFEDPHRangeCalibration::PixelFEDPHRangeCalibration() : PixelFEDCalibrationBase()
//{
//  std::cout << "Greetings from the PixelFEDPHRangeCalibration default constructor." << std::endl;
//}

PixelFEDPHRangeCalibration::PixelFEDPHRangeCalibration(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  std::cout << "Greetings from the PixelFEDPHRangeCalibration copy constructor." << std::endl;
}

xoap::MessageReference PixelFEDPHRangeCalibration::execute(xoap::MessageReference msg)
{
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
		cout << "ERROR: PixelFEDPHRangeCalibration::execute() does not understand the WhatToDo command, "<< parameters[0].value_ <<", sent to it.\n";
		assert(0);
	}

	xoap::MessageReference reply = MakeSOAPMessageReference("FEDCalibrationsDone");
	return reply;
}

void PixelFEDPHRangeCalibration::RetrieveData(unsigned int state)
{
	PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
	assert(tempCalibObject!=0);
	
	const std::vector<PixelROCName> ROCsToCalibrate=tempCalibObject->rocList();
	
	std::vector<std::pair<unsigned int,std::vector<unsigned int> > > fedsAndChannels = tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_ );
	               //     FED number,              channels
	
	for (unsigned int ifed=0; ifed<fedsAndChannels.size(); ++ifed)
	{
		unsigned int fednumber=fedsAndChannels[ifed].first;
		unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);

		uint64_t buffer64[4096];
		int status = FEDInterface_[vmeBaseAddress]->spySlink64(buffer64);
		if (status<=0)
		{
			cout << "ERROR reading FIFO3 on FED # " << fednumber << " in crate # " << crate_ << "." << endl;
			continue;
		}

		FIFO3Decoder decode(buffer64);

		unsigned int nhits=decode.nhits();
		for (unsigned int ihit=0;ihit<nhits;ihit++)
		{
			unsigned int channel=decode.channel(ihit);
			unsigned int rocid=decode.rocid(ihit);
			assert(rocid>0);

			PixelROCName roc=theNameTranslation_->ROCNameFromFEDChannelROC(fednumber, channel, rocid-1);
			
			// Skip if this ROC is not on the list of ROCs to calibrate.
			std::vector<PixelROCName>::const_iterator foundROC = find(ROCsToCalibrate.begin(), ROCsToCalibrate.end(), roc);
			if ( foundROC == ROCsToCalibrate.end() ) continue;
			
			// Skip if we're in singleROC mode, and this ROC is not being calibrated right now.
			if ( !(tempCalibObject->scanningROCForState(roc, state)) ) continue;

			std::map<std::string, unsigned int> currentDACValues;
			for ( unsigned int dacNum = 0; dacNum < tempCalibObject->numberOfScanVariables(); dacNum++ )
			{
				if (tempCalibObject->scanName(dacNum) != k_DACName_Vcal) 
				{
					currentDACValues[tempCalibObject->scanName(dacNum)] = tempCalibObject->scanValue(tempCalibObject->scanName(dacNum), state, roc);
				}
			}

			unsigned int currentVcalValue = tempCalibObject->scanValue(k_DACName_Vcal, state, roc);

			// Add the pulse height from the hit to this ROC's scan data.
			PH_vs_Vcal_[roc][currentDACValues].addEntry(currentVcalValue, decode.pulseheight(ihit));
		}
	} // end of loop over FEDs in this crate
}

void PixelFEDPHRangeCalibration::Analyze()
{
	// Remove gray background from plots.
	TStyle plainStyle("Plain", "a plain style");
	plainStyle.cd();
	
	PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
	assert(tempCalibObject!=0);
	
	TFile outputFile((outputDir()+"/"+tempCalibObject->mode()+"Calibration_FEDCrate"+stringF(crate_)+".root").c_str(), "RECREATE", ("plots from "+tempCalibObject->mode()+" calibration in FED crate # "+stringF(crate_)).c_str());
	
	double minPH = 300./4.;
	if ( tempCalibObject->parameterValue("minPH") != "" )
		minPH = atof(tempCalibObject->parameterValue("minPH").c_str());
	double maxPH = 254.;
	if ( tempCalibObject->parameterValue("maxPH") != "" )
		maxPH = atof(tempCalibObject->parameterValue("maxPH").c_str());
	
	const std::vector<PixelROCName> ROCsToCalibrate=tempCalibObject->rocList();
	
	std::map <PixelROCName, std::map<std::string, unsigned int> > recommended_DAC_values;
	unsigned int totalROCs = 0, ROCsCalibratedInOtherCrates = 0;
	std::map<pos::PixelROCName, std::string> failures;

	branch theBranch;
	branch_sum theBranch_sum;
	TDirectory* dirSummaries = gDirectory->mkdir("SummaryTrees","SummaryTrees");
	dirSummaries->cd();
	TTree* tree = new TTree("PassState","PassState");
	tree->Branch("PassState",&theBranch,"pass/F:rocName/C",4096000);
	TTree* tree_sum = new TTree("SummaryInfo","SummaryInfo");
	tree_sum->Branch("SummaryInfo",&theBranch_sum,"new_VIbias_PH/F:delta_VIbias_PH/F:new_VOffsetOp/F:delta_VOffsetOp/F:new_VIon/F:delta_VIon/F:new_VOffsetRO/F:delta_VOffsetRO/F:rocName/C",4096000);  
	
        outputFile.cd();

	PixelRootDirectoryMaker rootDirs(ROCsToCalibrate,gDirectory);

	// Loop over all ROCs in the list of ROCs to calibrate.
	for( std::vector<PixelROCName>::const_iterator ROCsToCalibrate_itr = ROCsToCalibrate.begin(); ROCsToCalibrate_itr != ROCsToCalibrate.end(); ROCsToCalibrate_itr++ )
	{

	  theBranch.pass=1;
	  std::string rocName = (*ROCsToCalibrate_itr).rocname();
	  strcpy(theBranch.rocName, rocName.c_str());
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

		const std::map<pos::PixelROCName, std::map <std::map<std::string, unsigned int>, PixelScanRecord> >::const_iterator foundROC = PH_vs_Vcal_.find(*ROCsToCalibrate_itr);
		if ( foundROC == PH_vs_Vcal_.end() ) // no data found for this ROC
		{
			failures[*ROCsToCalibrate_itr] = "NoHitsFound";
			theBranch.pass=0;
			tree->Fill();
			continue;
		}
		
		std::vector<std::string> plotAxisTitles;
		const std::map<std::map<std::string, unsigned int>, PixelScanRecord>::const_iterator DACSettings_begin_itr = foundROC->second.begin();
		if ( DACSettings_begin_itr != foundROC->second.end() )
		{
			const std::map<std::string, unsigned int>& DACSettings = DACSettings_begin_itr->first;
			for ( std::map<std::string, unsigned int>::const_iterator DACs_itr = DACSettings.begin(); DACs_itr != DACSettings.end(); DACs_itr++ )
			{
				const std::string& DAC = DACs_itr->first;
				assert( tempCalibObject->containsScan(DAC) );
				if ( tempCalibObject->scanValueMin(DAC) != tempCalibObject->scanValueMax(DAC) ) plotAxisTitles.push_back(DAC);
			}
		}
		
		TGraphErrors* maxPHGraph1D = 0;
		TGraphErrors* minPHGraph1D = 0;
		TGraphErrors* PHRangeGraph1D = 0;
		TGraph2DErrors* maxPHGraph2D = 0;
		TGraph2DErrors* minPHGraph2D = 0;
		TGraph2DErrors* PHRangeGraph2D = 0;
		bool plotsInitialized = false;
		unsigned int numPlotPoints = 0;
		
		// Find the DAC values with the greatest spread.
		bool acceptableSettingsFound = false;
		double highestSpread = -1;
		unsigned int maxNumScanPoints = 0;
		for ( std::map<std::map<std::string, unsigned int>, PixelScanRecord>::const_iterator DACSettings_itr = foundROC->second.begin(); DACSettings_itr != foundROC->second.end(); DACSettings_itr++ )
		{	
			const std::map<std::string, unsigned int>& DACSettings = DACSettings_itr->first;
			const PixelScanRecord& scan = DACSettings_itr->second;
			
			if ( plotAxisTitles.size() == 1 )
			{
				if ( !plotsInitialized )
				{
					maxPHGraph1D = new TGraphErrors(); maxPHGraph1D->SetTitle((ROCsToCalibrate_itr->rocname()+": max PH vs "+plotAxisTitles[0]).c_str());
					minPHGraph1D = new TGraphErrors(); minPHGraph1D->SetTitle((ROCsToCalibrate_itr->rocname()+": min PH vs "+plotAxisTitles[0]).c_str());
					PHRangeGraph1D = new TGraphErrors(); PHRangeGraph1D->SetTitle((ROCsToCalibrate_itr->rocname()+": PH range vs "+plotAxisTitles[0]).c_str());
					
					plotsInitialized = true;
				}
				
				std::map<std::string, unsigned int>::const_iterator foundDAC = DACSettings.find(plotAxisTitles[0]);
				assert( foundDAC != DACSettings.end() );
				unsigned int dacX = foundDAC->second;
				
				maxPHGraph1D->SetPoint( numPlotPoints, dacX, scan.maxY().mean() );
				maxPHGraph1D->SetPointError( numPlotPoints, 0, scan.maxY().uncertainty() );
				minPHGraph1D->SetPoint( numPlotPoints, dacX, scan.minY().mean() );
				minPHGraph1D->SetPointError( numPlotPoints, 0, scan.minY().uncertainty() );
				PHRangeGraph1D->SetPoint( numPlotPoints, dacX, scan.maxY().mean() - scan.minY().mean() );
				PHRangeGraph1D->SetPointError( numPlotPoints, 0, sqrt(pow(scan.maxY().uncertainty(),2)+pow(scan.minY().uncertainty(),2)) );
				numPlotPoints++;
			}
			
			if ( plotAxisTitles.size() == 2 )
			{
				if ( !plotsInitialized )
				{
					maxPHGraph2D = new TGraph2DErrors(); maxPHGraph2D->SetTitle((ROCsToCalibrate_itr->rocname()+": max PH vs "+plotAxisTitles[0]+"(horiz axis) and "+plotAxisTitles[1]+"(vert axis)").c_str());
					minPHGraph2D = new TGraph2DErrors(); minPHGraph2D->SetTitle((ROCsToCalibrate_itr->rocname()+": min PH vs "+plotAxisTitles[0]+"(horiz axis) and "+plotAxisTitles[1]+"(vert axis)").c_str());
					PHRangeGraph2D = new TGraph2DErrors(); PHRangeGraph2D->SetTitle((ROCsToCalibrate_itr->rocname()+": PH range vs "+plotAxisTitles[0]+"(horiz axis) and "+plotAxisTitles[1]+"(vert axis)").c_str());
					
					plotsInitialized = true;
				}
				
				std::map<std::string, unsigned int>::const_iterator foundDAC = DACSettings.find(plotAxisTitles[0]);
				assert( foundDAC != DACSettings.end() );
				unsigned int dacX = foundDAC->second;
				foundDAC = DACSettings.find(plotAxisTitles[1]);
				assert( foundDAC != DACSettings.end() );
				unsigned int dacY = foundDAC->second;
				
				maxPHGraph2D->SetPoint( numPlotPoints, dacX, dacY, scan.maxY().mean() );
				maxPHGraph2D->SetPointError( numPlotPoints, 0, 0, scan.maxY().uncertainty() );
				minPHGraph2D->SetPoint( numPlotPoints, dacX, dacY, scan.minY().mean() );
				minPHGraph2D->SetPointError( numPlotPoints, 0, 0, scan.minY().uncertainty() );
				PHRangeGraph2D->SetPoint( numPlotPoints, dacX, dacY, scan.maxY().mean() - scan.minY().mean() );
				PHRangeGraph2D->SetPointError( numPlotPoints, 0, 0, sqrt(pow(scan.maxY().uncertainty(),2)+pow(scan.minY().uncertainty(),2)) );
				numPlotPoints++;
			}
			
			maxNumScanPoints = std::max(maxNumScanPoints, scan.numScanPoints());
			if ( scan.numScanPoints() <= 1 ) continue;
			if ( scan.minY().mean() < minPH ) continue;
			if ( scan.maxY().mean() > maxPH ) continue;
			
			double spread = scan.maxY().mean() - scan.minY().mean();
			assert( spread > -0.1 );
			if ( !acceptableSettingsFound || spread > highestSpread )
			{
				highestSpread = spread;
				recommended_DAC_values[*ROCsToCalibrate_itr] = DACSettings_itr->first;
				acceptableSettingsFound = true;
			}
		}
		
		if ( !acceptableSettingsFound )
		{
			if ( maxNumScanPoints > 1 ) failures[*ROCsToCalibrate_itr] = "NoSettingsGivePHInRange";
			else                        failures[*ROCsToCalibrate_itr] = "OnlyOneVcalProducesHits";
			theBranch.pass=0;
		}
		
		// Write the plots.
		std::map<std::string, unsigned int> newDACSettingsForThisROC;
		std::map <PixelROCName, std::map<std::string, unsigned int> >::const_iterator foundDACSettingsForROC = recommended_DAC_values.find(*ROCsToCalibrate_itr);
		if ( foundDACSettingsForROC != recommended_DAC_values.end() )
		{
			newDACSettingsForThisROC = foundDACSettingsForROC->second;
		}
		
		if ( maxPHGraph1D != 0 && minPHGraph1D != 0 && PHRangeGraph1D != 0 )
		{
			const std::string dacName = plotAxisTitles[0];
			unsigned int dacValue = 0;
			if ( newDACSettingsForThisROC.size() > 0 )
			{
				std::map<std::string, unsigned int>::const_iterator foundDAC = newDACSettingsForThisROC.find(plotAxisTitles[0]);
				assert( foundDAC != newDACSettingsForThisROC.end() );
				assert( dacName == foundDAC->first );
				dacValue = foundDAC->second;
			}
			
			assert( tempCalibObject->containsScan(dacName) );
			
			TPaveText colorLegend(0.60,0.13,0.90,0.30,"BRNDC");
			TText* thisLine = colorLegend.AddText("Max PH and limit"); thisLine->SetTextColor(kBlue);
						thisLine = colorLegend.AddText("Min PH and limit"); thisLine->SetTextColor(kMagenta);
						thisLine = colorLegend.AddText("Difference"); thisLine->SetTextColor(kRed);
			if ( newDACSettingsForThisROC.size() > 0 ) {thisLine = colorLegend.AddText(("New "+dacName+" value").c_str());}
			else                                       {thisLine = colorLegend.AddText(("No new "+dacName+" value found").c_str());}
			thisLine->SetTextColor(kGreen);
			
			const std::string title = (*ROCsToCalibrate_itr).rocname()+": PH vs "+plotAxisTitles[0];
			TCanvas c(title.c_str(), title.c_str());
			TH1F* frame = c.DrawFrame(tempCalibObject->scanValueMin(dacName)-1,0-2,tempCalibObject->scanValueMax(dacName)+1,255+2);
			frame->SetXTitle(plotAxisTitles[0].c_str());
			frame->SetYTitle("PH counts");
			frame->SetTitle(title.c_str());
			
			TLine minPHLine( tempCalibObject->scanValueMin(dacName)-1,minPH,tempCalibObject->scanValueMax(dacName)+1,minPH );
			minPHLine.SetLineColor(kMagenta); minPHLine.SetLineStyle(kDashed); minPHLine.Draw();
			TLine maxPHLine( tempCalibObject->scanValueMin(dacName)-1,maxPH,tempCalibObject->scanValueMax(dacName)+1,maxPH );
			maxPHLine.SetLineColor(kBlue)   ; maxPHLine.SetLineStyle(kDashed); maxPHLine.Draw();
			TLine newSettingLine( dacValue, 0-2, dacValue, 255+2 );
			newSettingLine.SetLineColor(kGreen); newSettingLine.SetLineStyle(kDashed);
			if ( newDACSettingsForThisROC.size() > 0 ) newSettingLine.Draw();
			
			colorLegend.Draw();
			
			maxPHGraph1D->SetMarkerStyle(kFullCircle);   maxPHGraph1D->SetMarkerColor(kBlue);    maxPHGraph1D->Draw("P");
			minPHGraph1D->SetMarkerStyle(kFullCircle);   minPHGraph1D->SetMarkerColor(kMagenta); minPHGraph1D->Draw("P");
			PHRangeGraph1D->SetMarkerStyle(kFullCircle); PHRangeGraph1D->SetMarkerColor(kRed);   PHRangeGraph1D->Draw("P");
			
			c.Write();
		}
		
		WritePlot( maxPHGraph2D, newDACSettingsForThisROC, plotAxisTitles );
		WritePlot( minPHGraph2D, newDACSettingsForThisROC, plotAxisTitles );
		WritePlot( PHRangeGraph2D, newDACSettingsForThisROC, plotAxisTitles );
		
		tree->Fill();
	} // end of loop over ROCs to calibrate.

	// Create summary plots.
	outputFile.cd();
	TDirectory* summaryDir = outputFile.mkdir("summaryPlots");
	assert( summaryDir != 0 );
	summaryDir->cd();
	std::map<std::string, TH1I*> recommended_DAC_values_histograms; // TH1Is will be deleted when outputFile is closed.
	std::map<std::pair<PixelModuleName, std::string>, TH1I*> recommended_DAC_values_moduleHistograms;
	TH1F* PHRange_histogram = 0;
	std::map<PixelModuleName, TH1F*> PHRange_moduleHistograms;
	TH1F* maxPH_histogram = 0;
	std::map<PixelModuleName, TH1F*> maxPH_moduleHistograms;
	TH1F* minPH_histogram = 0;
	std::map<PixelModuleName, TH1F*> minPH_moduleHistograms;
	for ( std::map <PixelROCName, std::map<std::string, unsigned int> >::const_iterator recommended_DAC_values_itr = recommended_DAC_values.begin(); recommended_DAC_values_itr != recommended_DAC_values.end(); recommended_DAC_values_itr++ )
	{
		const PixelROCName& roc = recommended_DAC_values_itr->first;
		PixelModuleName module = theNameTranslation_->getChannelForROC(roc).module();
		const std::map<std::string, unsigned int>& DAC_values = recommended_DAC_values_itr->second;
		
		// Get min and max PH at the chosen DAC settings for this ROC.
		const std::map<pos::PixelROCName, std::map <std::map<std::string, unsigned int>, PixelScanRecord> >::const_iterator foundROC = PH_vs_Vcal_.find(roc);
		assert( foundROC != PH_vs_Vcal_.end() );
		const std::map <std::map<std::string, unsigned int>, PixelScanRecord>::const_iterator foundScanAtNewDACValues = foundROC->second.find( DAC_values );
		assert( foundScanAtNewDACValues != foundROC->second.end() );
		const PixelScanRecord& scan = foundScanAtNewDACValues->second;
		const double scanMin = scan.minY().mean();
		const double scanMax = scan.maxY().mean();
		
		// Fill PHRange, minPH, and maxPH histograms for this module.
		std::map<PixelModuleName, TH1F*>::iterator foundPHRangeHist = PHRange_moduleHistograms.find(module);
		if ( foundPHRangeHist == PHRange_moduleHistograms.end() )
		{
			std::string title = module.modulename()+": PH range";
			PHRange_moduleHistograms[module] = new TH1F(title.c_str(), title.c_str(), 256, 0, 256);
			PHRange_moduleHistograms[module]->GetXaxis()->SetTitle("PH range");
			foundPHRangeHist = PHRange_moduleHistograms.find(module);
			assert( foundPHRangeHist != PHRange_moduleHistograms.end() );
		}
		foundPHRangeHist->second->Fill(scanMax-scanMin);
		
		std::map<PixelModuleName, TH1F*>::iterator foundMaxPHHist = maxPH_moduleHistograms.find(module);
		if ( foundMaxPHHist == maxPH_moduleHistograms.end() )
		{
			std::string title = module.modulename()+": Max PH";
			maxPH_moduleHistograms[module] = new TH1F(title.c_str(), title.c_str(), 256, 0, 256);
			maxPH_moduleHistograms[module]->GetXaxis()->SetTitle("Max PH");
			foundMaxPHHist = maxPH_moduleHistograms.find(module);
			assert( foundMaxPHHist != maxPH_moduleHistograms.end() );
		}
		foundMaxPHHist->second->Fill(scanMax);
		
		std::map<PixelModuleName, TH1F*>::iterator foundMinPHHist = minPH_moduleHistograms.find(module);
		if ( foundMinPHHist == minPH_moduleHistograms.end() )
		{
			std::string title = module.modulename()+": Min PH";
			minPH_moduleHistograms[module] = new TH1F(title.c_str(), title.c_str(), 256, 0, 256);
			minPH_moduleHistograms[module]->GetXaxis()->SetTitle("Min PH");
			foundMinPHHist = minPH_moduleHistograms.find(module);
			assert( foundMinPHHist != minPH_moduleHistograms.end() );
		}
		foundMinPHHist->second->Fill(scanMin);
		
		// Fill PHRange, minPH, and maxPH histograms for all modules.
		if ( PHRange_histogram == 0 )
		{
			std::string title = "all ROCs: PH range";
			PHRange_histogram = new TH1F(title.c_str(), title.c_str(), 256, 0, 256);
			PHRange_histogram->GetXaxis()->SetTitle("PH range");
		}
		PHRange_histogram->Fill(scanMax-scanMin);
		
		if ( maxPH_histogram == 0 )
		{
			std::string title = "all ROCs: Max PH";
			maxPH_histogram = new TH1F(title.c_str(), title.c_str(), 256, 0, 256);
			maxPH_histogram->GetXaxis()->SetTitle("Max PH");
		}
		maxPH_histogram->Fill(scanMax);
		
		if ( minPH_histogram == 0 )
		{
			std::string title = "all ROCs: Min PH";
			minPH_histogram = new TH1F(title.c_str(), title.c_str(), 256, 0, 256);
			minPH_histogram->GetXaxis()->SetTitle("Min PH");
		}
		minPH_histogram->Fill(scanMin);
		
		// Fill new DAC value histograms.
		for ( std::map<std::string, unsigned int>::const_iterator DAC_values_itr = DAC_values.begin(); DAC_values_itr != DAC_values.end(); DAC_values_itr++ )
		{
			const std::string& dac = DAC_values_itr->first;
			const std::pair<PixelModuleName, std::string> moduleAndDAC( module, dac );
			const unsigned int& value = DAC_values_itr->second;
			
			// Fill histogram for this module.
			std::map<std::pair<PixelModuleName, std::string>, TH1I*>::iterator foundDACValueHist = recommended_DAC_values_moduleHistograms.find(moduleAndDAC);
			if ( foundDACValueHist == recommended_DAC_values_moduleHistograms.end() )
			{
				std::string title = module.modulename()+": new "+dac+" values";
				recommended_DAC_values_moduleHistograms[moduleAndDAC] = new TH1I(title.c_str(), title.c_str(), 256, 0, 256);
				recommended_DAC_values_moduleHistograms[moduleAndDAC]->GetXaxis()->SetTitle(dac.c_str());
				foundDACValueHist = recommended_DAC_values_moduleHistograms.find(moduleAndDAC);
				assert( foundDACValueHist != recommended_DAC_values_moduleHistograms.end() );
			}
			foundDACValueHist->second->Fill(value);
			
			// Fill histogram for all modules.
			std::map<std::string, TH1I*>::iterator foundHist = recommended_DAC_values_histograms.find(dac);
			if ( foundHist == recommended_DAC_values_histograms.end() )
			{
				std::string title = "all ROCs: new "+dac+" values";
				recommended_DAC_values_histograms[dac] = new TH1I(title.c_str(), title.c_str(), 256, 0, 256);
				recommended_DAC_values_histograms[dac]->GetXaxis()->SetTitle(dac.c_str());
				foundHist = recommended_DAC_values_histograms.find(dac);
				assert( foundHist != recommended_DAC_values_histograms.end() );
			}
			foundHist->second->Fill(value);
		}
	}

	// Write out new configuration files.
	std::map<std::string, Moments> delta_DAC_values, new_DAC_values;
	// Loop over all modules in the configuration.
	for (std::vector<PixelModuleName>::const_iterator module_name = theDetectorConfiguration_->getModuleList().begin(); module_name != theDetectorConfiguration_->getModuleList().end(); ++module_name)
	{
		// First we need to get the DAC settings for the ROCs on this module.
		PixelDACSettings *DACSettingsForThisModule=0; // Each PixelDACSettings object contains the settings for all ROCS on a particular module.
		std::string modulePath = module_name->modulename();
		PixelConfigInterface::get(DACSettingsForThisModule, "pixel/dac/"+modulePath, *theGlobalKey_);
		assert(DACSettingsForThisModule!=0);

		// Now loop over ROCs on this module and change DAC values to the recommended values.
		bool changedOne = false;
		for ( unsigned int ROCNumber = 0; ROCNumber < DACSettingsForThisModule->numROCs(); ROCNumber++ )
		{
			totalROCs++;
			PixelROCName nameOfThisROC = DACSettingsForThisModule->getDACSettings(ROCNumber).getROCName();
			
			std::string rocName = nameOfThisROC.rocname();
			strcpy(theBranch_sum.rocName,rocName.c_str());
			
			// Check whether this ROC is in the list of ROCs with new DAC values.  If so, change them in the configuration.
			std::map<PixelROCName, std::map<std::string, unsigned int> >::const_iterator foundROC = recommended_DAC_values.find(nameOfThisROC);
			if ( foundROC != recommended_DAC_values.end() )
			{
				changedOne = true;
				for ( std::map<std::string, unsigned int>::const_iterator DAC_values_itr = foundROC->second.begin(); DAC_values_itr != foundROC->second.end(); DAC_values_itr++ )
				{
				  std::string dac = DAC_values_itr->first;			
				  unsigned int newValue = DAC_values_itr->second;
				  delta_DAC_values[dac].push_back( (int)(newValue) - (int)(DACSettingsForThisModule->getDACSettings(nameOfThisROC)->getDac(dac)) );
				  new_DAC_values[dac].push_back( newValue );

				  if(0) //dac.compare(k_DACName_VIbias_PH)==0)
				    {
				      theBranch_sum.new_VIbias_PH=newValue;
				      theBranch_sum.delta_VIbias_PH=(int)(newValue)-(int)(DACSettingsForThisModule->getDACSettings(nameOfThisROC)->getDac(dac));
				    }else if(0) //dac.compare(k_DACName_VOffsetOp)==0)
				    {
				      theBranch_sum.new_VOffsetOp=newValue;
				      theBranch_sum.delta_VOffsetOp=(int)(newValue)-(int)(DACSettingsForThisModule->getDACSettings(nameOfThisROC)->getDac(dac));
				    }else if(0) //dac.compare(k_DACName_VIon)==0)
				    {
				      theBranch_sum.new_VIon=newValue;
				      theBranch_sum.delta_VIon=(int)(newValue) - (int)(DACSettingsForThisModule->getDACSettings(nameOfThisROC)->getDac(dac));
				    }else if(0) //dac.compare(k_DACName_VOffsetRO)==0)
				    {
				      theBranch_sum.new_VOffsetRO=newValue;
				      theBranch_sum.delta_VOffsetRO=(int)(newValue) - (int)(DACSettingsForThisModule->getDACSettings(nameOfThisROC)->getDac(dac));
				    } 
				  DACSettingsForThisModule->getDACSettings(nameOfThisROC)->setDac(dac, newValue);
				}
			}
			tree_sum->Fill();	
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
	cout << "\n";
	cout << "--------------------------------------\n";
	cout << "Pulse Height Range Calibration Report\n";
	cout << "--------------------------------------\n";
	cout << setw(4) << totalROCs << " ROCs in the configuration, of these:\n";
	cout << setw(4) << totalROCs-ROCsToCalibrate.size() << " ROCs were not tested\n";
	cout << setw(4) << ROCsCalibratedInOtherCrates << " ROCs were tested by other PixelFEDSupervisors (other crates)\n";
	cout << setw(4) << ROCsToCalibrate.size()-failures.size()-ROCsCalibratedInOtherCrates << " ROCs had a successful calibration\n";
	cout << setw(4) << failures.size() << " ROCs failed the calibration\n";
	
	assert( new_DAC_values.size() == delta_DAC_values.size() );
	for ( std::map<std::string, Moments>::const_iterator new_DAC_values_itr = new_DAC_values.begin(); new_DAC_values_itr != new_DAC_values.end(); new_DAC_values_itr++ )
	{
		assert( ROCsToCalibrate.size()-failures.size()-ROCsCalibratedInOtherCrates == new_DAC_values_itr->second.count() );
		cout << "\nSummary of new "<<new_DAC_values_itr->first<<" values:\n";
		cout <<   "mean     =" << setw(6)<<right << new_DAC_values_itr->second.mean()   << "\n";
		cout <<   "std.dev. =" << setw(6)<<right << new_DAC_values_itr->second.stddev() << "\n";
		cout <<   "smallest =" << setw(6)<<right << new_DAC_values_itr->second.min()    << "\n";
		cout <<   "largest  =" << setw(6)<<right << new_DAC_values_itr->second.max()    << "\n";
	
		std::map<std::string, Moments>::const_iterator delta_DAC_values_itr = delta_DAC_values.find(new_DAC_values_itr->first);
		assert( ROCsToCalibrate.size()-failures.size()-ROCsCalibratedInOtherCrates == delta_DAC_values_itr->second.count() );
		cout << "\nSummary of changes from old to new "<<new_DAC_values_itr->first<<" values (new-old):\n";
		cout <<   "mean          =" << setw(6)<<right << delta_DAC_values_itr->second.mean()   << "\n";
		cout <<   "std.dev.      =" << setw(6)<<right << delta_DAC_values_itr->second.stddev() << "\n";
		cout <<   "most negative =" << setw(6)<<right << delta_DAC_values_itr->second.min()    << "\n";
		cout <<   "most positive =" << setw(6)<<right << delta_DAC_values_itr->second.max()    << "\n";
	}

	cout << "\nFailed ROCs (total of " << failures.size() << "):\n";
	if ( failures.size() != 0 )
	{
		cout <<   "ROC" << string(42-3, ' ') << "Failure Mode\n";
		for ( std::map<PixelROCName, std::string>::const_iterator failuresitr = failures.begin(); failuresitr != failures.end(); failuresitr++ )
		{
			cout << failuresitr->first << string(42-min(42,(int)failuresitr->first.rocname().size()),' ') << failuresitr->second << endl;
		}
		cout << "\nFailure mode descriptions:\n";
		cout <<   "NoHitsFound             = No hits were found for this ROC." << endl;
		cout <<   "OnlyOneVcalProducesHits = For all DAC settings, at most one Vcal\n";
		cout <<   "                            value produced hits." << endl;
		cout <<   "NoSettingsGivePHInRange = No DAC settings in this scan place the\n";
		cout <<   "                            pulse height entirely within\n";
		cout <<   "                            the range "<<(int)(minPH+0.5)<<"-"<<(int)(maxPH+0.5)<<"." << endl;
	}
	
	cout << "--------------------------------------" << endl;

	// Clear the member data.
	PH_vs_Vcal_.clear();

	outputFile.Write();
	outputFile.Close();
}

void PixelFEDPHRangeCalibration::initializeFED(){

  setFEDModeAndControlRegister(0x8,0x30010);

}

xoap::MessageReference PixelFEDPHRangeCalibration::beginCalibration(xoap::MessageReference msg){

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDPHRangeCalibration::endCalibration(xoap::MessageReference msg){
  cout << "In PixelFEDPHRangeCalibration::endCalibration()" << endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

void PixelFEDPHRangeCalibration::WritePlot( TGraph2DErrors* graph, const std::map<std::string, unsigned int>& DACSettings, const std::vector<std::string>& plotAxisTitles ) const
{
	if ( graph == 0 ) return;
	
	TCanvas plotCanvas( graph->GetTitle(), graph->GetTitle() );
	//graph->GetXaxis()->SetTitle(plotAxisTitles[0].c_str());
	//graph->GetYaxis()->SetTitle(plotAxisTitles[1].c_str());
	//graph->GetZaxis()->SetTitle("PH range");
	graph->Draw("colz");
	
	// Draw marker indicating new DAC settings.
	TPaveText colorLegend(0.75,0.92,0.99,0.99,"BRNDC");
	TText* thisLine = colorLegend.AddText("New DAC values"); thisLine->SetTextColor(kCyan);
	TMarker* marker = 0;
	if ( DACSettings.size() > 0 )
	{
		colorLegend.Draw();
		std::map<std::string, unsigned int>::const_iterator foundDAC = DACSettings.find(plotAxisTitles[0]);
		assert( foundDAC != DACSettings.end() );
		unsigned int dacX = foundDAC->second;
		foundDAC = DACSettings.find(plotAxisTitles[1]);
		assert( foundDAC != DACSettings.end() );
		unsigned int dacY = foundDAC->second;
		marker = new TMarker(dacX, dacY, 29); // 29 = ROOT code for a filled star
		marker->SetMarkerColor(kCyan);
		marker->SetMarkerSize(3);
		marker->Draw();
	}
	
	plotCanvas.Write();
	delete graph;
	if ( marker != 0 ) delete marker;
}
