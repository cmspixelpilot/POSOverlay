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

#include "PixelCalibrations/include/PixelFEDVsfAndVHldDelCalibration.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO3Decoder.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"

#include "TCanvas.h"
#include "TFrame.h"
#include "TH1F.h"
#include "TPaveText.h"
#include "TText.h"
#include "TFile.h"
#include "TStyle.h"
#include "TDirectory.h"
#include "TTree.h"

#include <toolbox/convertstring.h>

#include "iomanip"

using namespace pos;
using namespace std;

//PixelFEDVsfAndVHldDelCalibration::PixelFEDVsfAndVHldDelCalibration() : PixelFEDCalibrationBase()
//{
//  std::cout << "Greetings from the PixelFEDVsfAndVHldDelCalibration default constructor." << std::endl;
//}

PixelFEDVsfAndVHldDelCalibration::PixelFEDVsfAndVHldDelCalibration(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  std::cout << "Greetings from the PixelFEDVsfAndVHldDelCalibration copy constructor." << std::endl;
}

xoap::MessageReference PixelFEDVsfAndVHldDelCalibration::execute(xoap::MessageReference msg)
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
		cout << "ERROR: PixelFEDVsfAndVHldDelCalibration::execute() does not understand the WhatToDo command, "<< parameters[0].value_ <<", sent to it.\n";
		assert(0);
	}

	xoap::MessageReference reply = MakeSOAPMessageReference("FEDCalibrationsDone");
	return reply;
}

void PixelFEDVsfAndVHldDelCalibration::RetrieveData(unsigned int state)
{
	PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
	assert(tempCalibObject!=0);
	
	const std::vector<PixelROCName> ROCsToCalibrate=tempCalibObject->rocList();
	
	std::vector<std::pair<unsigned int,std::vector<unsigned int> > > fedsAndChannels = tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_ );
	               //     FED number,              channels
	
	bool getVsfFromConfig = !(tempCalibObject->containsScan(k_DACName_Vsf));
	
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

			unsigned int Vsf_value;
			if ( getVsfFromConfig ) Vsf_value = VsfFromConfig(roc);
			else                    Vsf_value = tempCalibObject->scanValue(k_DACName_Vsf, state, roc);
			
			unsigned int VHldDel_value = tempCalibObject->scanValue(k_DACName_VHldDel, state, roc);

			// Add the pulse height from the hit to this ROC's scan data.
			PH_vs_VHldDel_[roc][Vsf_value].addEntry(VHldDel_value, decode.pulseheight(ihit));
		}
	} // end of loop over FEDs in this crate
}

void PixelFEDVsfAndVHldDelCalibration::Analyze()
{
	// Remove gray background from plots.
	TStyle plainStyle("Plain", "a plain style");
	plainStyle.cd();
	
	PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
	assert(tempCalibObject!=0);
	
	TFile outputFile((outputDir()+"/"+tempCalibObject->mode()+"Calibration_FEDCrate"+stringF(crate_)+".root").c_str(), "RECREATE", ("plots from "+tempCalibObject->mode()+" calibration in FED crate # "+stringF(crate_)).c_str());
	
	// Decide whether to look for the best Vsf, or get it from the config.
	bool getVsfFromConfig = !(tempCalibObject->containsScan(k_DACName_Vsf));
	
	// Decide whether to look for the best VHldDel.
	unsigned int numVHldDelScanPoints = 1 + (int)((tempCalibObject->scanValueMax(k_DACName_VHldDel) - tempCalibObject->scanValueMin(k_DACName_VHldDel))/tempCalibObject->scanValueStep(k_DACName_VHldDel));
	bool findBestVHldDel = false;
	if ( numVHldDelScanPoints > 2 ) findBestVHldDel = true;
	
	unsigned int minVHldDel = (int)tempCalibObject->scanValueMin(k_DACName_VHldDel);
	unsigned int maxVHldDel = (int)tempCalibObject->scanValueMax(k_DACName_VHldDel);
	
	const std::vector<PixelROCName> ROCsToCalibrate=tempCalibObject->rocList();
	
	std::map <PixelROCName, bits8 > recommended_Vsf_values;
	std::map <PixelROCName, bits8 > recommended_VHldDel_values;
	unsigned int totalROCs = 0, ROCsCalibratedInOtherCrates = 0;
	std::map<pos::PixelROCName, std::string> failures;
	std::map<pos::PixelROCName, std::string> reducedVsfs;
	Moments VsfReductions;

	PixelMaxVsf* maxVsfs = 0;
	if (!getVsfFromConfig)
	{
		PixelConfigInterface::get(maxVsfs,"pixel/maxvsf/",*theGlobalKey_);
		assert( maxVsfs != 0 );
	}

	branch theBranch;
	branch_sum theBranch_sum;
	TDirectory* dirSummaries = gDirectory->mkdir("SummaryTrees","SummaryTrees");
	dirSummaries->cd();

	TTree* tree = new TTree("PassState","PassState");
	TTree* tree_sum =new TTree("SummaryInfo","SummaryInfo");
    
	tree->Branch("PassState",&theBranch,"pass/F:rocName/C",4096000);
	tree_sum->Branch("SummaryInfo",&theBranch_sum,"delta_Vsf/F:new_Vsf/F:delta_VHldDel/F:new_VHldDel/F:rocName/C",4096000);
	outputFile.cd();

	PixelRootDirectoryMaker rootDirs(ROCsToCalibrate,gDirectory);

	// Loop over all ROCs in the list of ROCs to calibrate.
	for( std::vector<PixelROCName>::const_iterator ROCsToCalibrate_itr = ROCsToCalibrate.begin(); ROCsToCalibrate_itr != ROCsToCalibrate.end(); ROCsToCalibrate_itr++ )
	  {
	    
	    theBranch.pass=1;
	    std::string rocName=(*ROCsToCalibrate_itr).rocname();
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

	    std::map <pos::PixelROCName, map<unsigned int, PixelScanRecord> >::iterator foundROC = PH_vs_VHldDel_.find(*ROCsToCalibrate_itr);
	    if ( foundROC == PH_vs_VHldDel_.end() ) // no data found for this ROC
	      {
		failures[*ROCsToCalibrate_itr] = "NoHitsFound";
		theBranch.pass=0;
		tree->Fill();
		continue;
	      }

	    // Create scans of PH vs Vsf, and add labels to all scans.
	    PixelScanRecord PH_vs_Vsf_atMinVHldDel;
	    PH_vs_Vsf_atMinVHldDel.setXVarName("Vsf");
	    PH_vs_Vsf_atMinVHldDel.setYVarName("Pulse Height");
	    PH_vs_Vsf_atMinVHldDel.setTitle("VHldDel = " + stringF(minVHldDel));
		
	    PixelScanRecord PH_vs_Vsf_atMaxVHldDel;
	    PH_vs_Vsf_atMaxVHldDel.setXVarName("Vsf");
	    PH_vs_Vsf_atMaxVHldDel.setYVarName("Pulse Height");
	    PH_vs_Vsf_atMaxVHldDel.setTitle("VHldDel = " + stringF(maxVHldDel));

	    for ( std::map<unsigned int, PixelScanRecord>::iterator Vsf_itr = foundROC->second.begin(); Vsf_itr != foundROC->second.end(); Vsf_itr++ )
	      {
		PixelScanRecord& PH_vs_VHldDel_thisVsf = Vsf_itr->second;
		PH_vs_VHldDel_thisVsf.setXVarName("VHldDel");
		PH_vs_VHldDel_thisVsf.setYVarName("Pulse Height");
		PH_vs_VHldDel_thisVsf.setTitle(foundROC->first.rocname()+": PH vs VHldDel, Vsf = "+stringF(Vsf_itr->first));
		if (findBestVHldDel) PH_vs_VHldDel_thisVsf.printPlot();
		Moments pointAtMinVHldDel = PH_vs_VHldDel_thisVsf.getPoint( minVHldDel );
		if ( pointAtMinVHldDel.count() > 0 ) PH_vs_Vsf_atMinVHldDel.setPoint( Vsf_itr->first, pointAtMinVHldDel );
		Moments pointAtMaxVHldDel = PH_vs_VHldDel_thisVsf.getPoint( maxVHldDel );
		if ( pointAtMaxVHldDel.count() > 0 ) PH_vs_Vsf_atMaxVHldDel.setPoint( Vsf_itr->first, pointAtMaxVHldDel );
	      }
		

		// Look for the optimal Vsf, if applicable.
		if ( !getVsfFromConfig )
		{
			PixelScanRecord maxMinDifference = PH_vs_Vsf_atMaxVHldDel - PH_vs_Vsf_atMinVHldDel;
			if ( maxMinDifference.crossingPointFound(0, PixelScanRecord::kLast) )
			{
				recommended_Vsf_values[*ROCsToCalibrate_itr] = maxMinDifference.crossingPoint();
			}
			else
			{
				failures[*ROCsToCalibrate_itr] = "NoGoodVsfFound";
				theBranch.pass=0;
			}
			
			// Print scans vs Vsf.
			std::string title = (*ROCsToCalibrate_itr).rocname() + ": PH vs Vsf";
			
			int intersectionPoint = -1;
			std::map <PixelROCName, bits8 >::const_iterator foundIntersection = recommended_Vsf_values.find(*ROCsToCalibrate_itr);
			if (foundIntersection != recommended_Vsf_values.end()) intersectionPoint = foundIntersection->second;
			
			TGraphErrors PH_vs_Vsf_atMinVHldDel_plot = PH_vs_Vsf_atMinVHldDel.makePlot(kBlue);
			TGraphErrors PH_vs_Vsf_atMaxVHldDel_plot = PH_vs_Vsf_atMaxVHldDel.makePlot(kMagenta);
			
			TPaveText colorLegend(0.15,0.75,0.45,0.90,"BRNDC");
			TText* thisLine = colorLegend.AddText(("VHldDel = "+stringF(minVHldDel)).c_str()); thisLine->SetTextColor(kBlue);
					thisLine = colorLegend.AddText(("VHldDel = "+stringF(maxVHldDel)).c_str()); thisLine->SetTextColor(kMagenta);
			if (intersectionPoint>=0) {thisLine = colorLegend.AddText("Intersection"); thisLine->SetTextColor(kGreen);}
			
			TCanvas c(title.c_str(), title.c_str(), 800, 600);
			c.GetFrame()->SetFillColor(kWhite);
			
			double plotMin=0;
			double plotMax=1023;
			if((PH_vs_Vsf_atMinVHldDel.numScanPoints()>0) && (PH_vs_Vsf_atMaxVHldDel.numScanPoints()>0)){
			  plotMin = min(PH_vs_Vsf_atMinVHldDel.minY().mean(),PH_vs_Vsf_atMaxVHldDel.minY().mean())-5;
			  plotMax = max(PH_vs_Vsf_atMinVHldDel.maxY().mean(),PH_vs_Vsf_atMaxVHldDel.maxY().mean())+5;
			}
			else if(PH_vs_Vsf_atMinVHldDel.numScanPoints()==0){
			  plotMin=PH_vs_Vsf_atMaxVHldDel.minY().mean()-5;
			  plotMax=PH_vs_Vsf_atMaxVHldDel.maxY().mean()+5;
			}
			else if(PH_vs_Vsf_atMaxVHldDel.numScanPoints()==0){
			  plotMin=PH_vs_Vsf_atMinVHldDel.minY().mean()-5;
			  plotMax=PH_vs_Vsf_atMinVHldDel.maxY().mean()+5;
			}
			
			TH1F* frame = c.DrawFrame(minVHldDel, plotMin, maxVHldDel, plotMax);
			frame->SetXTitle("Vsf");
			frame->SetYTitle("Pulse Height");
			frame->SetTitle(title.c_str());
			colorLegend.Draw();
			PH_vs_Vsf_atMinVHldDel_plot.Draw("P");
			PH_vs_Vsf_atMaxVHldDel_plot.Draw("P");
			TLine intersectionLine( intersectionPoint, plotMin, intersectionPoint, plotMax );
			intersectionLine.SetLineColor(kGreen); intersectionLine.SetLineStyle(kDashed);
			if (intersectionPoint>=0) intersectionLine.Draw();
			((TPad*)(&c))->Write();
			
			// Truncate Vsf to the maximum allowed value, if necessary.
			unsigned int maxVsf;
			bool success = maxVsfs->getVsf( *ROCsToCalibrate_itr, maxVsf );
			assert( success );
			if ( intersectionPoint > (int)maxVsf )
			{
				reducedVsfs[*ROCsToCalibrate_itr] = "VsfReduced"+stringF((unsigned int)(recommended_Vsf_values[*ROCsToCalibrate_itr]))+"->"+stringF(maxVsf);
				VsfReductions.push_back((unsigned int)(recommended_Vsf_values[*ROCsToCalibrate_itr]) - maxVsf);
				recommended_Vsf_values[*ROCsToCalibrate_itr] = (bits8)(maxVsf);
			}
		}

	    tree->Fill();
	    // Look for the optimal VHldDel, if applicable.
	    std::map <PixelROCName, bits8 >::const_iterator foundVsf = recommended_Vsf_values.find(*ROCsToCalibrate_itr);
	    if ( (getVsfFromConfig || foundVsf != recommended_Vsf_values.end())
		 && findBestVHldDel )
	      {
		// Determine which VHldDel scan is closest to the best Vsf value.
		map <unsigned int, PixelScanRecord>::iterator bestScan = foundROC->second.begin(); // temporary value
		if ( getVsfFromConfig )
		  {
		    bestScan = foundROC->second.find( VsfFromConfig(*ROCsToCalibrate_itr) );
		    assert( bestScan != foundROC->second.end() );
		  }
		else if ( foundVsf != recommended_Vsf_values.end() )
		  {
		    int bestVsf = foundVsf->second;
		    for ( std::map<unsigned int, PixelScanRecord>::iterator Vsf_itr = foundROC->second.begin(); Vsf_itr != foundROC->second.end(); Vsf_itr++ )
		      {
			int thisVsf = Vsf_itr->first;
			int closestVsf = bestScan->first;
			if ( abs( thisVsf - bestVsf ) < abs( closestVsf - bestVsf ) ) bestScan = Vsf_itr;
		      }
		  }
			
		// In the best scan, find the point where the pulse height is highest.
		recommended_VHldDel_values[*ROCsToCalibrate_itr] = (int)(bestScan->second.interpolatedHighestPoint().first+0.5);
			
		// Replace the plot of the best scan with a different version with a new name and the highest point marked.
		gDirectory->Delete((bestScan->second.title()+";*").c_str());
		bestScan->second.setTitle(bestScan->second.title()+" (best Vsf)");
		bestScan->second.printPlot();
	      }
		
	  } // end of loop over ROCs to calibrate.
	
	if ( maxVsfs != 0 ) delete maxVsfs;

	// Write out new configuration files.
	Moments delta_Vsf_values, new_Vsf_values, delta_VHldDel_values, new_VHldDel_values;
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
		    std::string rocName = nameOfThisROC.rocname();
		    strcpy(theBranch_sum.rocName,rocName.c_str());
			
		    // Check whether this ROC is in the list of ROCs with new Vsf values.  If so, change Vsf in the configuration.
		    std::map<PixelROCName, bits8>::const_iterator foundVsf = recommended_Vsf_values.find(nameOfThisROC);
		    if ( foundVsf != recommended_Vsf_values.end() )
		      {
			changedOne = true;
			delta_Vsf_values.push_back( foundVsf->second - DACSettingsForThisModule->getDACSettings(nameOfThisROC)->getVsf() );
			new_Vsf_values.push_back( foundVsf->second );
			
			theBranch_sum.delta_Vsf=foundVsf->second-DACSettingsForThisModule->getDACSettings(nameOfThisROC)->getVsf();
			theBranch_sum.new_Vsf=foundVsf->second;
			DACSettingsForThisModule->getDACSettings(nameOfThisROC)->setVsf(foundVsf->second);

			

		
		      }
			
		    // Check whether this ROC is in the list of ROCs with new VHldDel values.  If so, change VHldDel in the configuration.
		    std::map<PixelROCName, bits8>::const_iterator foundVHldDel = recommended_VHldDel_values.find(nameOfThisROC);
		    if ( foundVHldDel != recommended_VHldDel_values.end() )
		      {
			changedOne = true;
			delta_VHldDel_values.push_back( foundVHldDel->second - DACSettingsForThisModule->getDACSettings(nameOfThisROC)->getVHldDel() );
			new_VHldDel_values.push_back( foundVHldDel->second );

			theBranch_sum.delta_VHldDel=foundVHldDel->second-DACSettingsForThisModule->getDACSettings(nameOfThisROC)->getVHldDel();
			theBranch_sum.new_VHldDel=foundVHldDel->second;

			DACSettingsForThisModule->getDACSettings(nameOfThisROC)->setVHldDel(foundVHldDel->second);
				
			
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
	cout << "Vsf and VHldDel Calibration Report\n";
	cout << "--------------------------------------\n";
	cout << setw(4) << totalROCs << " ROCs in the configuration, of these:\n";
	cout << setw(4) << totalROCs-ROCsToCalibrate.size() << " ROCs were not tested\n";
	cout << setw(4) << ROCsCalibratedInOtherCrates << " ROCs were tested by other PixelFEDSupervisors (other crates)\n";
	cout << setw(4) << ROCsToCalibrate.size()-failures.size()-ROCsCalibratedInOtherCrates << " ROCs had a successful calibration\n";
	cout << setw(4) << failures.size() << " ROCs failed the calibration\n";
	if (!getVsfFromConfig)
	cout << setw(4) << reducedVsfs.size() << " of the successful ROCs had Vsf reduced to the maximum allowed value\n";

	if ( getVsfFromConfig )
	{
		cout << "\nVsf was taken from the configuration,\n";
		cout <<   "so no new Vsf values were written.\n";
	}
	else
	{
		assert( ROCsToCalibrate.size()-failures.size()-ROCsCalibratedInOtherCrates == new_Vsf_values.count() );
		cout << "\nSummary of new Vsf values:\n";
		cout <<   "mean     =" << setw(6)<<right << new_Vsf_values.mean()   << "\n";
		cout <<   "std.dev. =" << setw(6)<<right << new_Vsf_values.stddev() << "\n";
		cout <<   "smallest =" << setw(6)<<right << new_Vsf_values.min()    << "\n";
		cout <<   "largest  =" << setw(6)<<right << new_Vsf_values.max()    << "\n";
	
		cout << "\nSummary of changes from old to new Vsf values (new-old):\n";
		cout <<   "mean          =" << setw(6)<<right << delta_Vsf_values.mean()   << "\n";
		cout <<   "std.dev.      =" << setw(6)<<right << delta_Vsf_values.stddev() << "\n";
		cout <<   "most negative =" << setw(6)<<right << delta_Vsf_values.min()    << "\n";
		cout <<   "most positive =" << setw(6)<<right << delta_Vsf_values.max()    << "\n";
		
		cout << "\nAmong ROCs which had Vsf reduced, size of the reduction:\n";
		cout <<   "mean     =" << setw(6)<<right << VsfReductions.mean()   << "\n";
		cout <<   "std.dev. =" << setw(6)<<right << VsfReductions.stddev() << "\n";
		cout <<   "smallest =" << setw(6)<<right << VsfReductions.min()    << "\n";
		cout <<   "largest  =" << setw(6)<<right << VsfReductions.max()    << "\n";
	}
	
	if ( !findBestVHldDel )
	{
		cout << "\nVHldDel was not scanned,\n";
		cout <<   "so no new VHldDel values were written.\n";
	}
	else
	{
		if ( !getVsfFromConfig && (int)(tempCalibObject->scanValueStep(k_DACName_VHldDel)) > 1 )
		{
			cout << "\nWARNING: To determine the optimal VHldDel, recommend writing the Vsf values\n";
			cout <<   "determined here to the configuration, and then rerunning using those Vsf\n";
			cout <<   "settings instead of scanning Vsf.  The VHldDel values determined here may\n";
			cout <<   "use Vsf settings that differ somewhat from the optimal settings." << endl;
		}
		
		assert( ROCsToCalibrate.size()-failures.size()-ROCsCalibratedInOtherCrates == new_VHldDel_values.count() );
		cout << "\nSummary of new VHldDel values:\n";
		cout <<   "mean     =" << setw(6)<<right << new_VHldDel_values.mean()   << "\n";
		cout <<   "std.dev. =" << setw(6)<<right << new_VHldDel_values.stddev() << "\n";
		cout <<   "smallest =" << setw(6)<<right << new_VHldDel_values.min()    << "\n";
		cout <<   "largest  =" << setw(6)<<right << new_VHldDel_values.max()    << "\n";
	
		cout << "\nSummary of changes from old to new VHldDel values (new-old):\n";
		cout <<   "mean          =" << setw(6)<<right << delta_VHldDel_values.mean()   << "\n";
		cout <<   "std.dev.      =" << setw(6)<<right << delta_VHldDel_values.stddev() << "\n";
		cout <<   "most negative =" << setw(6)<<right << delta_VHldDel_values.min()    << "\n";
		cout <<   "most positive =" << setw(6)<<right << delta_VHldDel_values.max()    << "\n";
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
		cout <<   "NoHitsFound     = No hits were found for this ROC." << endl;
		cout <<   "NoGoodVsfFound  = At no value of Vsf was the pulse height\n";
		cout <<   "                    at VHldDel = "<<minVHldDel<<" equal to the value\n";
		cout <<   "                    at VHldDel = "<<maxVHldDel<<"." << endl;
	}
	if (!getVsfFromConfig)
	{
		cout << "\nROCs on which Vsf was reduced to the maximum allowed value (total of " << reducedVsfs.size() << "):\n";
		if ( reducedVsfs.size() != 0 )
		{
			cout <<   "ROC" << string(42-3, ' ') << "Vsf Reduction\n";
			for ( std::map<PixelROCName, std::string>::const_iterator reducedVsfsitr = reducedVsfs.begin(); reducedVsfsitr != reducedVsfs.end(); reducedVsfsitr++ )
			{
				cout << reducedVsfsitr->first << string(42-min(42,(int)reducedVsfsitr->first.rocname().size()),' ') << reducedVsfsitr->second << endl;
			}
		}
	}

	cout << "--------------------------------------" << endl;

	// Clear the member data.
	PH_vs_VHldDel_.clear();
	VsfValuesFromConfig_.clear();
	
	outputFile.Write();
	outputFile.Close();
}

void PixelFEDVsfAndVHldDelCalibration::initializeFED(){

  setFEDModeAndControlRegister(0x8,0x30010);

}

xoap::MessageReference PixelFEDVsfAndVHldDelCalibration::beginCalibration(xoap::MessageReference msg){

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDVsfAndVHldDelCalibration::endCalibration(xoap::MessageReference msg){
  cout << "In PixelFEDVsfAndVHldDelCalibration::endCalibration()" << endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

unsigned int PixelFEDVsfAndVHldDelCalibration::VsfFromConfig(const PixelROCName& roc)
{
	std::map<PixelROCName, unsigned int>::iterator VsfValuesFromConfig_found = VsfValuesFromConfig_.find(roc);
	if ( VsfValuesFromConfig_found != VsfValuesFromConfig_.end() )
	{
		return VsfValuesFromConfig_found->second;
	}
	else
	{
		const PixelModuleName& module = theNameTranslation_->getChannelForROC(roc).module();
		PixelDACSettings *DACSettingsForThisModule=0;
		std::string modulePath = module.modulename();
		PixelConfigInterface::get(DACSettingsForThisModule, "pixel/dac/"+modulePath, *theGlobalKey_);
		assert(DACSettingsForThisModule!=0);
		PixelROCDACSettings* thisROCDACSettings = DACSettingsForThisModule->getDACSettings(roc);
		VsfValuesFromConfig_[roc] = thisROCDACSettings->getVsf();
		delete DACSettingsForThisModule;
		
		return VsfValuesFromConfig_[roc];
	}
}
