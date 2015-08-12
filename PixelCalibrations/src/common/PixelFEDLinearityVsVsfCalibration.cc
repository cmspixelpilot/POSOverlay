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

#include "PixelCalibrations/include/PixelFEDLinearityVsVsfCalibration.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO3Decoder.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"
#include "CalibFormats/SiPixelObjects/interface/PixelMaxVsf.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"

#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"

#include "TCanvas.h"
#include "TFrame.h"
#include "TH1F.h"
#include "TPaveText.h"
#include "TText.h"
#include "TFile.h"
#include "TF1.h"
#include "TPaveStats.h"
#include "TStyle.h"
#include "TTree.h"

#include <toolbox/convertstring.h>

#include "iomanip"

using namespace pos;
using namespace std;

//PixelFEDLinearityVsVsfCalibration::PixelFEDLinearityVsVsfCalibration() : PixelFEDCalibrationBase()
//{
//  std::cout << "Greetings from the PixelFEDLinearityVsVsfCalibration default constructor." << std::endl;
//}

PixelFEDLinearityVsVsfCalibration::PixelFEDLinearityVsVsfCalibration(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  std::cout << "Greetings from the PixelFEDLinearityVsVsfCalibration copy constructor." << std::endl;
}

xoap::MessageReference PixelFEDLinearityVsVsfCalibration::execute(xoap::MessageReference msg)
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
		cout << "ERROR: PixelFEDLinearityVsVsfCalibration::execute() does not understand the WhatToDo command, "<< parameters[0].value_ <<", sent to it.\n";
		assert(0);
	}

	xoap::MessageReference reply = MakeSOAPMessageReference("FEDCalibrationsDone");
	return reply;
}

void PixelFEDLinearityVsVsfCalibration::RetrieveData(unsigned int state)
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
			if (rocid==0){
			  cout << "Error in decoding data, status="<<status<<endl;
			  cout << "nhits  ="<<nhits << endl;
			  cout << "ihit   ="<<ihit << endl;
			  cout << "channel="<<channel << endl;
			  cout << "rocid  ="<<rocid << endl;
			  for(int i=0;i<status;i++){
			    cout << "buffer["<<i<<"]="<<hex<<buffer64[i]<<dec<<endl;
			  }
			}
			assert(rocid>0);

			PixelROCName roc=theNameTranslation_->ROCNameFromFEDChannelROC(fednumber, channel, rocid-1);
			
			// Skip if this ROC is not on the list of ROCs to calibrate.
			std::vector<PixelROCName>::const_iterator foundROC = find(ROCsToCalibrate.begin(), ROCsToCalibrate.end(), roc);
			if ( foundROC == ROCsToCalibrate.end() ) continue;
			
			// Skip if we're in singleROC mode, and this ROC is not being calibrated right now.
			if ( !(tempCalibObject->scanningROCForState(roc, state)) ) continue;

			// Get info about which row and column this is.
			std::pair <unsigned int, unsigned int> columnAndRow(decode.column(ihit), decode.row(ihit));
			
			// Skip if this pixel wasn't supposed to have a hit.
			std::set< std::pair<unsigned int, unsigned int> > allHitPixels = tempCalibObject->pixelsWithHits(state);
			if ( allHitPixels.find( columnAndRow ) == allHitPixels.end() ) continue;

			// Add the pulse height from the hit to this ROC's scan data.
			unsigned int Vsf_value = tempCalibObject->scanValue(k_DACName_Vsf, state, roc);
			unsigned int Vcal_value = tempCalibObject->scanValue(k_DACName_Vcal, state, roc);
			
			if ( Vcal_value == 0 ) cout << "\nERROR: For " << roc << ", Vsf = " << Vsf_value << ", Vcal = " << Vcal_value << ": found a hit with pulse height " << decode.pulseheight(ihit) << endl << "       But there should be no hit when Vcal = 0." << endl;
			
			PH_vs_Vcal_[roc][columnAndRow][Vsf_value].addEntry(Vcal_value, decode.pulseheight(ihit));
		}
	} // end of loop over FEDs in this crate
}

void PixelFEDLinearityVsVsfCalibration::Analyze()
{
	// Remove gray background from plots, and output fit parameters.
	TStyle plainStyle("Plain", "a plain style");
	plainStyle.cd();
	plainStyle.SetOptFit(0111);
	
	PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
	assert(tempCalibObject!=0);
	
	TFile outputFile((outputDir()+"/"+tempCalibObject->mode()+"Calibration_FEDCrate"+stringF(crate_)+".root").c_str(), "RECREATE", ("plots from "+tempCalibObject->mode()+" calibration in FED crate # "+stringF(crate_)).c_str());
	
	outputFile.SetCompressionLevel(9); // reduce file size
	
	// Parameters for nonlinearity measure.
	bool useIntegrated2ndOver1stDerivative = false;
	if ( tempCalibObject->parameterValue("useIntegrated2ndOver1stDerivative") == "yes" || tempCalibObject->parameterValue("useIntegrated2ndOver1stDerivative") == "true" )
		useIntegrated2ndOver1stDerivative = true;
	double integralMinVcal = 50./7.;
	if ( tempCalibObject->parameterValue("integralMinVcal") != "" )
		integralMinVcal = atof(tempCalibObject->parameterValue("integralMinVcal").c_str());
	double integralMaxVcal = 400./7.;
	if ( tempCalibObject->parameterValue("integralMaxVcal") != "" )
		integralMaxVcal = atof(tempCalibObject->parameterValue("integralMaxVcal").c_str());
	
	// Parameters for Vsf determination algorithm.
	bool absoluteNonlinearityThreshold = true;
	if ( tempCalibObject->parameterValue("absoluteNonlinearityThreshold") == "no" || tempCalibObject->parameterValue("absoluteNonlinearityThreshold") == "false" )
		absoluteNonlinearityThreshold = false;
	double nonlinearityThreshold = 1.4;
	if ( tempCalibObject->parameterValue("nonlinearityThreshold") != "" )
		nonlinearityThreshold = atof(tempCalibObject->parameterValue("nonlinearityThreshold").c_str());
		
	const std::vector<PixelROCName> ROCsToCalibrate=tempCalibObject->rocList();
	
	std::map <PixelROCName, bits8 > recommended_Vsf_values;
	unsigned int totalROCs = 0, ROCsCalibratedInOtherCrates = 0;
	std::map<pos::PixelROCName, std::string> failures;
	std::map<pos::PixelROCName, std::string> reducedVsfs;

	PixelMaxVsf* maxVsfs = 0;
	PixelConfigInterface::get(maxVsfs,"pixel/maxvsf/",*theGlobalKey_);
	assert( maxVsfs != 0 );

	PixelTimer fitTimer;
	PixelTimer analysisTimer;
	analysisTimer.start();

	branch theBranch;
	branch_sum theBranch_sum;
	TDirectory* dirSummaries = gDirectory->mkdir("SummaryTrees","SummaryTrees");
	dirSummaries->cd();
	TTree* tree = new TTree("PassState","PassState");
	tree->Branch("PassState",&theBranch,"pass/F:rocName/C",4096000);
	TTree* tree_sum = new TTree("SummaryInfo","SummaryInfo");
	tree_sum->Branch("SummaryInfo",&theBranch_sum,"delta_Vsf/F:new_Vsf/F:rocName/C",4096000);  
    
	outputFile.cd();


	PixelRootDirectoryMaker rootDirs(ROCsToCalibrate,gDirectory);

	// Loop over all ROCs in the list of ROCs to calibrate.
	unsigned int ROCsSoFar = 0, lastROCReported = 0;
	Moments numPixelsUsedPerROC;
	Moments VsfStddevs, VsfReductions;
	for( std::vector<PixelROCName>::const_iterator ROCsToCalibrate_itr = ROCsToCalibrate.begin(); ROCsToCalibrate_itr != ROCsToCalibrate.end(); ROCsToCalibrate_itr++ )
	{

	  theBranch.pass=1;
	  std::string rocName=(*ROCsToCalibrate_itr).rocname();
	  strcpy(theBranch.rocName,rocName.c_str());
		rootDirs.cdDirectory(*(ROCsToCalibrate_itr));
		
		if ( ROCsSoFar == 0 || (double(ROCsSoFar - lastROCReported))/(double(ROCsToCalibrate.size())) > 0.05 )
		{
			lastROCReported = ROCsSoFar;
			cout << "Analyzing " << tempCalibObject->mode() << ": " << setprecision(2) << 100.*(double(ROCsSoFar))/(double(ROCsToCalibrate.size())) << std::setprecision(6) << "% complete" << endl;
		}
		ROCsSoFar++;
		
		// Get hardware address of this ROC.
		const PixelHdwAddress *aROC_hardware=theNameTranslation_->getHdwAddress(*ROCsToCalibrate_itr);
		unsigned int fednumber = aROC_hardware->fednumber();
		unsigned int crate=theFEDConfiguration_->crateFromFEDNumber(fednumber);
		if (crate!=crate_)
		{
			ROCsCalibratedInOtherCrates++;
			continue; // Ignore if this ROC is not in the crate controlled by this PixelFEDSupervisor.
		}

		std::map <pos::PixelROCName, map < std::pair <unsigned int, unsigned int>, map <unsigned int, PixelScanRecord> > >::iterator foundROC = PH_vs_Vcal_.find(*ROCsToCalibrate_itr);
		if ( foundROC == PH_vs_Vcal_.end() ) // no data found for this ROC
		{
			failures[*ROCsToCalibrate_itr] = "NoHitsFound";
			theBranch.pass=0;
			tree->Fill();
			continue;
		}

		// Loop over pixels with hits on this ROC.
		std::vector<unsigned int> Vsf_values_forThisROC; Vsf_values_forThisROC.clear();
		for ( map < std::pair <unsigned int, unsigned int>, map <unsigned int, PixelScanRecord> >::iterator pixel_itr = foundROC->second.begin(); pixel_itr != foundROC->second.end(); pixel_itr++ )
		{
			unsigned int column = pixel_itr->first.first;
			unsigned int row    = pixel_itr->first.second;
			
			char tempString[500];
	
			// Create output plots.
			PixelScanRecord nonlinearityScan;
			nonlinearityScan.setXVarName("Vsf");
			nonlinearityScan.setYVarName("Non-linearity parameter");
			if ( useIntegrated2ndOver1stDerivative )
			{
				sprintf(tempString, "%s, column %i, row %i: Non-linearity ( 1/2 #int_{%.0f}^{%.0f} |f''(Vcal)/f'(Vcal)| dVcal ) vs. Vsf", (*ROCsToCalibrate_itr).rocname().c_str(), column, row, integralMinVcal, integralMaxVcal);
			}
			else
			{
				sprintf(tempString, "%s, column %i, row %i: Non-linearity ( xmid/xsize )", (*ROCsToCalibrate_itr).rocname().c_str(), column, row);
			}
			nonlinearityScan.setTitle(tempString);
			
			for ( std::map<unsigned int, PixelScanRecord>::iterator Vsf_itr = pixel_itr->second.begin(); Vsf_itr != pixel_itr->second.end(); Vsf_itr++ )
			{
				unsigned int Vsf = Vsf_itr->first;
				
				// Get the plot of the scan, and write it out.
				PixelScanRecord& PH_vs_Vcal_thisVsf = Vsf_itr->second;
				PH_vs_Vcal_thisVsf.setXVarName("Vcal");
				PH_vs_Vcal_thisVsf.setYVarName("Pulse Height");
				PH_vs_Vcal_thisVsf.setTitle((*ROCsToCalibrate_itr).rocname()+", column "+stringF(column)+", row "+stringF(row)+": Vsf = "+stringF(Vsf));
				TGraphErrors plot = PH_vs_Vcal_thisVsf.makePlot();
				//plot.Write(("TGraphErrors for "+PH_vs_Vcal_thisVsf.title()).c_str());
				
				// Create a canvas to hold the scan and the fit result.
				TCanvas canvas(PH_vs_Vcal_thisVsf.title().c_str(), PH_vs_Vcal_thisVsf.title().c_str());
				plot.Draw("AP");
				
				// Fit the scan.
				sprintf(tempString, "func_%s_column_%i_row_%i_Vsf_%i", (*ROCsToCalibrate_itr).rocname().c_str(), column, row, Vsf);
				TF1* func = new TF1(tempString, "[0] + [1]*tanh((x-[2])/[3])", 0, 255);
				func->SetParNames("ymid", "ysize", "xmid", "xsize");
				func->SetParameters(160,60,50,50);
				func->SetParLimits(0, 0, 255);
				func->SetParLimits(1, 0, 255);
				func->SetParLimits(2, 0, 255);
				func->SetParLimits(3, 0, 255);
				fitTimer.start();
				int status = plot.Fit(func, "QR");
				fitTimer.stop();
				canvas.Draw();
				TPaveStats * stats = (TPaveStats*)plot.GetListOfFunctions()->FindObject("stats");
				if(stats!=0)
				{
					stats->SetY1NDC(0.2); //new y start position
					stats->SetY2NDC(0.5); //new y end position
				}
				
				TPaveText* nonlinearityBox = 0;
				if ( status == 0 && func->GetParameter("xsize") > 10.) // successful fit
				{
					// Calculate linearity parameter.
					double xmid  = func->GetParameter("xmid");
					double xsize = func->GetParameter("xsize");
					
					double nonlinearityParameter;
					if ( useIntegrated2ndOver1stDerivative )
					{
						double ulow  = (integralMinVcal-xmid)/xsize;
						double uhigh = (integralMaxVcal-xmid)/xsize;
						assert( uhigh - ulow >= 0. );
						
						if      ( ulow < 0. && uhigh < 0. )
						{
							nonlinearityParameter = log( cosh(ulow)/cosh(uhigh) );
						}
						else if ( ulow < 0. && 0. <= uhigh )
						{
							nonlinearityParameter = log( cosh(uhigh)*cosh(ulow) );
						}
						else if ( 0. <= ulow && 0. <= uhigh )
						{
							nonlinearityParameter = log( cosh(uhigh)/cosh(ulow) );
						}
						else assert(0);
						
						assert( nonlinearityParameter >= -0.1 );
						
						sprintf(tempString, "Non-linearity ( 1/2 #int_{%.0f}^{%.0f} |f''(Vcal)/f'(Vcal)| dVcal ) = %f", integralMinVcal, integralMaxVcal, nonlinearityParameter);
					}
					else
					{
						nonlinearityParameter = xmid/xsize;
						sprintf(tempString, "Non-linearity ( xmid/xsize ) = %f", nonlinearityParameter);
					}
					
					nonlinearityBox = new TPaveText(0.5,0.12,0.96,0.2,"NDC");
					nonlinearityBox->AddText(tempString);
					nonlinearityBox->Draw();
					nonlinearityScan.setPoint(Vsf, nonlinearityParameter);
				}
				
				((TPad*)(&canvas))->Write();
	
				if (nonlinearityBox != 0) delete nonlinearityBox;
				delete func;
				
			} // end of loop over Vsf values
		
			// Create a plot of the non-linearity parameter and output it..
			TGraphErrors nonlinearityPlot = nonlinearityScan.makePlot();
			//nonlinearityPlot.Write(("TGraphErrors for "+nonlinearityScan.title()).c_str());
			
			// Determine the optimal Vsf.
			double thisROCAndPixelNonlinearityThreshold = nonlinearityThreshold;
			if ( !absoluteNonlinearityThreshold )
			{
				// Set nonlinearity threshold to be "nonlinearityThreshold" times the last point of the scan.
				thisROCAndPixelNonlinearityThreshold = nonlinearityScan.getPoint(nonlinearityScan.maxX()).mean()*nonlinearityThreshold;
			}
			if ( nonlinearityScan.crossingPointFound(thisROCAndPixelNonlinearityThreshold, PixelScanRecord::kLast) )
			{
				Vsf_values_forThisROC.push_back(nonlinearityScan.crossingPoint());
			}
			
			// Print plot with the optimal Vsf marked.
			nonlinearityScan.printPlot(); // to ROOT file
			
		} // end of loop over pixels with hits on this ROC
		
		if ( Vsf_values_forThisROC.size() == 0 )
		{
			failures[*ROCsToCalibrate_itr] = "NoGoodVsfFound";
			theBranch.pass=0;
			tree->Fill();
			continue;
		}
		
		// Eliminate any outliers among the Vsf values.
		while ( Vsf_values_forThisROC.size() > 3 )
		{
			// Check for outliers.
			Moments Vsf_Moments(Vsf_values_forThisROC);
			std::set<unsigned int> removeThese;
			for ( std::vector<unsigned int>::const_iterator Vsf_values_forThisROC_itr = Vsf_values_forThisROC.begin(); Vsf_values_forThisROC_itr != Vsf_values_forThisROC.end(); Vsf_values_forThisROC_itr++ )
			{
				unsigned int thisVsfValue = *Vsf_values_forThisROC_itr;
				Moments allOtherVsfs = Vsf_Moments;
				allOtherVsfs.removeEntry( thisVsfValue );
				if ( fabs( thisVsfValue - allOtherVsfs.mean() ) > 2.5*allOtherVsfs.stddev() ) removeThese.insert(thisVsfValue);
			}
			
			// If there's nothing to remove, then we're done.
			if ( removeThese.empty() ) break;
			
			// Remove any outliers that were found.
			std::vector<unsigned int> temp;
			for ( std::vector<unsigned int>::const_iterator Vsf_values_forThisROC_itr = Vsf_values_forThisROC.begin(); Vsf_values_forThisROC_itr != Vsf_values_forThisROC.end(); Vsf_values_forThisROC_itr++ )
			{
				unsigned int thisVsfValue = *Vsf_values_forThisROC_itr;
				if ( removeThese.find(thisVsfValue) == removeThese.end() ) temp.push_back(thisVsfValue);
			}
			Vsf_values_forThisROC = temp;
		}
		
		assert( Vsf_values_forThisROC.size() > 0 );
		
		Moments goodVsfs(Vsf_values_forThisROC);
		recommended_Vsf_values[*ROCsToCalibrate_itr] = (bits8)((int)(goodVsfs.mean()+0.5));
		numPixelsUsedPerROC.push_back( Vsf_values_forThisROC.size() );
		if ( goodVsfs.count() >= 2 ) VsfStddevs.push_back( goodVsfs.stddev() );
		
		// Truncate Vsf to the maximum allowed value, if necessary.
		unsigned int maxVsf;
		bool success = maxVsfs->getVsf( *ROCsToCalibrate_itr, maxVsf );
		assert( success );
		if ( (unsigned int)(recommended_Vsf_values[*ROCsToCalibrate_itr]) > maxVsf )
		{
			reducedVsfs[*ROCsToCalibrate_itr] = "VsfReduced"+stringF((unsigned int)(recommended_Vsf_values[*ROCsToCalibrate_itr]))+"->"+stringF(maxVsf);
			VsfReductions.push_back((unsigned int)(recommended_Vsf_values[*ROCsToCalibrate_itr]) - maxVsf);
			recommended_Vsf_values[*ROCsToCalibrate_itr] = (bits8)(maxVsf);
		}
		tree->Fill();	
	} // end of loop over ROCs to calibrate.
	
	
	delete maxVsfs;
	analysisTimer.stop();

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
			  delta_Vsf_values.push_back(foundVsf->second-DACSettingsForThisModule->getDACSettings(nameOfThisROC)->getVsf());
			  
			  theBranch_sum.delta_Vsf= foundVsf->second-DACSettingsForThisModule->getDACSettings(nameOfThisROC)->getVsf();
			  
			  new_Vsf_values.push_back( foundVsf->second );

			  theBranch_sum.new_Vsf=foundVsf->second;

			  DACSettingsForThisModule->getDACSettings(nameOfThisROC)->setVsf(foundVsf->second);
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

	outputFile.Write();
	outputFile.Close();

	assert( recommended_Vsf_values.size() == ROCsToCalibrate.size()-failures.size()-ROCsCalibratedInOtherCrates );

	// Summary report.
	cout << "\n\n";
	cout <<   "Total fit time: " << fitTimer.tottime() << endl;
	cout <<   "Total analysis time: " << analysisTimer.tottime() << endl;
	cout << "\n";
	cout << "--------------------------------------\n";
	cout << " Linearity vs Vsf Calibration Report\n";
	cout << "--------------------------------------\n";
	cout << setw(4) << totalROCs << " ROCs in the configuration, of these:\n";
	cout << setw(4) << totalROCs-ROCsToCalibrate.size() << " ROCs were not tested\n";
	cout << setw(4) << ROCsCalibratedInOtherCrates << " ROCs were tested by other PixelFEDSupervisors (other crates)\n";
	cout << setw(4) << ROCsToCalibrate.size()-failures.size()-ROCsCalibratedInOtherCrates << " ROCs had a successful calibration\n";
	cout << setw(4) << failures.size() << " ROCs failed the calibration\n";
	cout << setw(4) << reducedVsfs.size() << " of the successful ROCs had Vsf reduced to the maximum allowed value\n";

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

	cout << "\nAmong successful ROCs, number of pixels used in determining Vsf.\n";
	cout <<   "(Outliers or failed fits are ignored when determining Vsf.)\n";
	cout <<   "mean     =" << setw(6)<<right << numPixelsUsedPerROC.mean()   << "\n";
	cout <<   "std.dev. =" << setw(6)<<right << numPixelsUsedPerROC.stddev() << "\n";
	cout <<   "smallest =" << setw(6)<<right << numPixelsUsedPerROC.min()    << "\n";
	cout <<   "largest  =" << setw(6)<<right << numPixelsUsedPerROC.max()    << "\n";
	
	cout << "\nAmong ROCs with at least two pixels giving good Vsf values,\n";
	cout <<   "standard deviation per ROC of those values.\n";
	cout <<   "mean     =" << setw(6)<<right << VsfStddevs.mean()   << "\n";
	cout <<   "std.dev. =" << setw(6)<<right << VsfStddevs.stddev() << "\n";
	cout <<   "smallest =" << setw(6)<<right << VsfStddevs.min()    << "\n";
	cout <<   "largest  =" << setw(6)<<right << VsfStddevs.max()    << "\n";

	cout << "\nAmong ROCs which had Vsf reduced, size of the reduction:\n";
	cout <<   "mean     =" << setw(6)<<right << VsfReductions.mean()   << "\n";
	cout <<   "std.dev. =" << setw(6)<<right << VsfReductions.stddev() << "\n";
	cout <<   "smallest =" << setw(6)<<right << VsfReductions.min()    << "\n";
	cout <<   "largest  =" << setw(6)<<right << VsfReductions.max()    << "\n";

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
		cout <<   "NoGoodVsfFound  = The algorithm failed to find a good Vsf value" << endl;;
		cout <<   "                    from any pixel's scan of non-linearity vs. Vsf." << endl;
	}
	cout << "\nROCs on which Vsf was reduced to the maximum allowed value (total of " << reducedVsfs.size() << "):\n";
	if ( reducedVsfs.size() != 0 )
	{
		cout <<   "ROC" << string(42-3, ' ') << "Vsf Reduction\n";
		for ( std::map<PixelROCName, std::string>::const_iterator reducedVsfsitr = reducedVsfs.begin(); reducedVsfsitr != reducedVsfs.end(); reducedVsfsitr++ )
		{
			cout << reducedVsfsitr->first << string(42-min(42,(int)reducedVsfsitr->first.rocname().size()),' ') << reducedVsfsitr->second << endl;
		}
	}

	cout << "--------------------------------------" << endl;

	// Clear the member data.
	PH_vs_Vcal_.clear();
}

void PixelFEDLinearityVsVsfCalibration::initializeFED(){

  setFEDModeAndControlRegister(0x8,0x30010);

}

xoap::MessageReference PixelFEDLinearityVsVsfCalibration::beginCalibration(xoap::MessageReference msg){

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDLinearityVsVsfCalibration::endCalibration(xoap::MessageReference msg){
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}
