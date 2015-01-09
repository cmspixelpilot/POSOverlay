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

#include "PixelCalibrations/include/PixelFEDAOHAndFEDChannelMappingTest.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelDecodedFEDRawData.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"

#include <toolbox/convertstring.h>

#include "iomanip"

using namespace pos;
using namespace std;

//PixelFEDAOHAndFEDChannelMappingTest::PixelFEDAOHAndFEDChannelMappingTest() : PixelFEDCalibrationBase()
//{
//  std::cout << "Greetings from the PixelFEDAOHAndFEDChannelMappingTest default constructor." << std::endl;
//}

PixelFEDAOHAndFEDChannelMappingTest::PixelFEDAOHAndFEDChannelMappingTest(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  std::cout << "Greetings from the PixelFEDAOHAndFEDChannelMappingTest copy constructor." << std::endl;
}

xoap::MessageReference PixelFEDAOHAndFEDChannelMappingTest::execute(xoap::MessageReference msg)
{
	Attribute_Vector parameters(4);
	parameters[0].name_ = "WhatToDo";
	parameters[1].name_ = "fednumber";
	parameters[2].name_ = "fedchannel";
	parameters[3].name_ = "AOHBias";
	Receive(msg, parameters);

	xoap::MessageReference reply = MakeSOAPMessageReference("AOHAndFEDChannelMappingTestDone");

	if (parameters[0].value_=="MeasureBlackLevel")
	{
		MeasureBlackLevel(atoi(parameters[1].value_.c_str()), atoi(parameters[2].value_.c_str()), atoi(parameters[3].value_.c_str()));
	}
	else if (parameters[0].value_=="CheckForChangeInBlackLevel")
	{
		reply = CheckForChangeInBlackLevel(atoi(parameters[1].value_.c_str()), atoi(parameters[2].value_.c_str()));
	}
	else
	{
		cout << "ERROR: PixelFEDAOHAndFEDChannelMappingTest::execute() does not understand the WhatToDo command, "<< parameters[0].value_ <<", sent to it.\n";
		assert(0);
	}

	return reply;
}

void PixelFEDAOHAndFEDChannelMappingTest::MeasureBlackLevel(unsigned int fednumber, unsigned int fedchannel, unsigned int AOHBias)
{
	PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
	assert(tempCalibObject!=0);
	bool printFEDRawData = false;
	if ( tempCalibObject->parameterValue("printFEDRawData") == "yes" || tempCalibObject->parameterValue("printFEDRawData") == "true" )
		printFEDRawData = true;
	
	unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
	
	uint32_t buffer[pos::fifo1TranspDepth]; // problem on a 64-bit machine?
	int status = FEDInterface_[vmeBaseAddress]->drain_transBuffer(fedchannel, buffer);
	if (status<0) {
		std::cout<<"PixelFEDAOHAndFEDChannelMappingTest::execute() -- Could not drain FIFO 1 of FED Channel "<<fedchannel<<" in transparent mode!"<<std::endl;
		diagService_->reportError("PixelFEDAOHAndFEDChannelMappingTest::execute() -- Could not drain FIFO 1 in transparent mode!",DIAGWARN);
	}

	std::vector<PixelROCName> ROCsOnThisChannel=theNameTranslation_->getROCsFromFEDChannel(fednumber, fedchannel);
	std::vector<unsigned int> hitsOnROCs(ROCsOnThisChannel.size(), 0); // 0 hits on each channel
	PixelDecodedFEDRawData decodedRawData(buffer, 20., 100., 150., hitsOnROCs);
	if (printFEDRawData)
	{
		std::cout << "*** Raw Data for FED # " << fednumber << ", channel " << fedchannel << " ***" << std::endl;
		decodedRawData.printToStream(cout);
		std::cout << "**************************\n";
		std::cout << std::endl;
	}
	
	if ( decodedRawData.badBuffer() )
	{
		if (printFEDRawData) std::cout << "WARNING: Bad buffer from FED.  Ignoring this iteration.\n";
		return;
	}
	
	if ( status == (int)pos::fifo1TranspDepth )
	{
		if (printFEDRawData) std::cout << "WARNING: Transparent buffer contains 2 events.  Ignoring this iteration.\n";
		return;
	}

	BVsAOHBias_[fednumber][fedchannel].addEntries(AOHBias, decodedRawData.initialBlack().getMoments());
	
	VMEPtr_[vmeBaseAddress]->write("LRES",0x80000000); // Local Reset
	VMEPtr_[vmeBaseAddress]->write("CLRES",0x80000000); // *** MAURO ***
}

xoap::MessageReference PixelFEDAOHAndFEDChannelMappingTest::CheckForChangeInBlackLevel(unsigned int fednumber, unsigned int fedchannel)
{
	PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
	assert(tempCalibObject!=0);
	
	bool printScan = false;
	if ( tempCalibObject->parameterValue("printScan") == "yes" || tempCalibObject->parameterValue("printScan") == "true" )
		printScan = true;
	
	Attribute_Vector returnValues(1);
	returnValues[0].name_="BlackLevelChanged";
	
	std::map <unsigned int, map <unsigned int, PixelScanRecord> >::iterator thisFEDFound = BVsAOHBias_.find(fednumber);
	if ( thisFEDFound == BVsAOHBias_.end() )
	{
		returnValues[0].value_="NoData";
		return MakeSOAPMessageReference("AOHAndFEDChannelMappingTestDone", returnValues);
	}
	std::map<unsigned int, PixelScanRecord>::iterator thisFEDChannelFound = thisFEDFound->second.find(fedchannel);
	if ( thisFEDChannelFound == thisFEDFound->second.end() )
	{
		returnValues[0].value_="NoData";
		return MakeSOAPMessageReference("AOHAndFEDChannelMappingTestDone", returnValues);
	}
	PixelScanRecord& scanForThisChannel = thisFEDChannelFound->second;
	
	scanForThisChannel.setTitle("AOHAndFEDChannelMappingTest, FED # "+itoa(fednumber)+", channel "+itoa(fedchannel));
	scanForThisChannel.setXVarName("AOH Bias");
	scanForThisChannel.setYVarName("Black Level (ADC counts)");
	
	if ( printScan )
	{
		cout << "**** For FED # " << fednumber << ", channel " << fedchannel << " ****" << endl;
		scanForThisChannel.printScanToStream(cout);
	}
	
	scanForThisChannel.printPlot();
	
	if ( scanForThisChannel.uncertaintyWeightedAverage() < 0. + 1e-5 )
	{
		returnValues[0].value_="StuckAt0";
		return MakeSOAPMessageReference("AOHAndFEDChannelMappingTestDone", returnValues);
	}
	if ( scanForThisChannel.uncertaintyWeightedAverage() > 1023. - 1e-5 )
	{
		returnValues[0].value_="StuckAt1023";
		return MakeSOAPMessageReference("AOHAndFEDChannelMappingTestDone", returnValues);
	}
	
	if ( scanForThisChannel.maxY().mean() - scanForThisChannel.minY().mean() < 1024./4 )
	{
		returnValues[0].value_="no";
		return MakeSOAPMessageReference("AOHAndFEDChannelMappingTestDone", returnValues);
	}
	else
	{
		returnValues[0].value_="yes";
		return MakeSOAPMessageReference("AOHAndFEDChannelMappingTestDone", returnValues);
	}
}

void PixelFEDAOHAndFEDChannelMappingTest::initializeFED(){

  setFEDModeAndControlRegister(0x0,0x30019);
  setBlackUBTrans();
}

xoap::MessageReference PixelFEDAOHAndFEDChannelMappingTest::beginCalibration(xoap::MessageReference msg){

	// Remove gray background from plots.
	plainStyle_ = new TStyle("Plain", "a plain style");
	plainStyle_->cd();
	
	PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
	assert(tempCalibObject!=0);
	
	outputFile_ = new TFile((outputDir()+"/"+tempCalibObject->mode()+"Calibration_FEDCrate"+stringF(crate_)+".root").c_str(), "RECREATE", ("plots from "+tempCalibObject->mode()+" calibration in FED crate # "+stringF(crate_)).c_str());

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDAOHAndFEDChannelMappingTest::endCalibration(xoap::MessageReference msg){

	delete plainStyle_;
	delete outputFile_;

  cout << "In PixelFEDAOHAndFEDChannelMappingTest::endCalibration()" << endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}
