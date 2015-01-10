/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelFEDTBMDelayCalibration.h"
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

//PixelFEDTBMDelayCalibration::PixelFEDTBMDelayCalibration() : PixelFEDCalibrationBase()
//{
//  std::cout << "Greetings from the PixelFEDTBMDelayCalibration default constructor." << std::endl;
//}

PixelFEDTBMDelayCalibration::PixelFEDTBMDelayCalibration(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  std::cout << "Greetings from the PixelFEDTBMDelayCalibration copy constructor." << std::endl;
}

xoap::MessageReference PixelFEDTBMDelayCalibration::execute(xoap::MessageReference msg)
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
		cout << "ERROR: PixelFEDTBMDelayCalibration::execute() does not understand the WhatToDo command, "<< parameters[0].value_ <<", sent to it.\n";
		assert(0);
	}

	xoap::MessageReference reply = MakeSOAPMessageReference("FEDCalibrationsDone");
	return reply;
}

void PixelFEDTBMDelayCalibration::RetrieveData(unsigned int state)
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

			std::cout << "hit " << roc << " ch " << channel << " col " << decode.column(ihit) << " row " << decode.row(ihit);

			std::map<std::string, unsigned int> currentDACValues;
			for ( unsigned int dacNum = 0; dacNum < tempCalibObject->numberOfScanVariables(); dacNum++ )
			{
				if (tempCalibObject->scanName(dacNum) != k_DACName_Vcal) 
				{
					currentDACValues[tempCalibObject->scanName(dacNum)] = tempCalibObject->scanValue(tempCalibObject->scanName(dacNum), state, roc);
				}
				//				if (tempCalibObject->scanName(dacNum).compare(0,3,"TBM") == 0)
				cout << " " << tempCalibObject->scanName(dacNum) << " " << tempCalibObject->scanValue(tempCalibObject->scanName(dacNum), state, roc);
			}
			cout << "\n";
			
			// Add the pulse height from the hit to this ROC's scan data.
			//hit_counts[roc][currentDACValues][std::make_pair(decode.column(ihit), decode.row(ihit))] += 1;
		}
	} // end of loop over FEDs in this crate
}

void PixelFEDTBMDelayCalibration::Analyze()
{
	// Remove gray background from plots.
	TStyle plainStyle("Plain", "a plain style");
	plainStyle.cd();
	
	PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
	assert(tempCalibObject!=0);
	
	hit_counts.clear();

	//outputFile.Write();
	//outputFile.Close();
}

void PixelFEDTBMDelayCalibration::initializeFED(){

  setFEDModeAndControlRegister(0x8,0x30010);

}

xoap::MessageReference PixelFEDTBMDelayCalibration::beginCalibration(xoap::MessageReference msg){

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDTBMDelayCalibration::endCalibration(xoap::MessageReference msg){
  cout << "In PixelFEDTBMDelayCalibration::endCalibration()" << endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}
