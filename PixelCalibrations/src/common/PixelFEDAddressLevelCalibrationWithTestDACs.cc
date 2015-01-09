// Modified by Jennifer Vaughan 2007/06/01
// $Id: PixelAddressLevelCalibrationWithTestDACs.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelFEDAddressLevelCalibrationWithTestDACs.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/AddressLevels.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO1Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"
#include "PixelUtilities/PixelFEDDataTools/include/TestDACTools.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelDecodedFEDRawData.h"

#include <toolbox/convertstring.h>

using namespace pos;
using namespace std;

PixelFEDAddressLevelCalibrationWithTestDACs::PixelFEDAddressLevelCalibrationWithTestDACs(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDAddressLevelCalibrationBase(tempConfiguration,mySOAPCmdr)
{
  cout << "[PixelFEDAddressLevelCalibrationWithTestDACs::PixelFEDAddressLevelCalibrationWithTestDACs]" << endl;
}

xoap::MessageReference PixelFEDAddressLevelCalibrationWithTestDACs::execute(xoap::MessageReference msg)
{

  Attribute_Vector parameters(2);
  parameters[0].name_="Hits";
  parameters[1].name_="First";
  Receive(msg, parameters);

  bool noHits=parameters[0].value_=="No";
 
  if (parameters[0].value_!="Analyse") {
    analyze(noHits);
  } 

  xoap::MessageReference reply = MakeSOAPMessageReference("AddressLevelCalibrationWithTestDACsDone");
  return reply;
}


void PixelFEDAddressLevelCalibrationWithTestDACs::initializeFED(){

  setFEDModeAndControlRegister(0x0,0x1d);
  baselinecorr_off();
  xoap::MessageReference fillTestDACmsg=MakeSOAPMessageReference("FillTestDAC");
  fillTestDAC(fillTestDACmsg);

}

xoap::MessageReference PixelFEDAddressLevelCalibrationWithTestDACs::beginCalibration(xoap::MessageReference msg){

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");


  map<pair<unsigned long, unsigned int>, set<unsigned int> >::iterator fediterator;
  set<unsigned int>::iterator ichannel;

  for (fediterator=vmeBaseAddressAndFEDNumberAndChannels_.begin(); fediterator!=vmeBaseAddressAndFEDNumberAndChannels_.end(); ++fediterator) {
    unsigned int fednumber=fediterator->first.second;
    set<unsigned int> channels=fediterator->second;
    vector<unsigned int> theChannels;
    for (ichannel=channels.begin(); ichannel!=channels.end(); ++ichannel) {
      unsigned int channel=*ichannel;
      theChannels.push_back(channel);
      vector<PixelROCName> ROCs=theNameTranslation_->getROCsFromFEDChannel(fednumber, channel);
      for (unsigned int i=0;i<ROCs.size();i++){
	aROC_string_.push_back(ROCs[i]);
      }
    }
    pair<unsigned int, vector<unsigned int> > thePair(fednumber,theChannels);
    fedsAndChannels_.push_back(thePair);
  }


  initAddressLevelCalibration();

  
  return reply;
}

xoap::MessageReference PixelFEDAddressLevelCalibrationWithTestDACs::endCalibration(xoap::MessageReference msg){
  cout << "[PixelFEDAddressLevelCalibrationWithTestDACs::endCalibration()]" 
       << endl;
  summary();
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

