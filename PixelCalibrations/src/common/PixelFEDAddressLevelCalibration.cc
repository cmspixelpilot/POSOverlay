// $Id: PixelFEDAddressLevelCalibration.cc,v 1.23 2008/08/11 09:06:02 aryd Exp $

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelFEDAddressLevelCalibration.h"
#include "PixelCalibrations/include/PixelFEDAddressLevelCalibrationBase.h"
// #include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"

// #include "TH1F.h"
// #include "TH2F.h"

#include <toolbox/convertstring.h>

using namespace pos;
using namespace std;


PixelFEDAddressLevelCalibration::PixelFEDAddressLevelCalibration(const PixelFEDSupervisorConfiguration& tempConfiguration, SOAPCommander* mySOAPCmdr) 
  :PixelFEDAddressLevelCalibrationBase(tempConfiguration,mySOAPCmdr)
{
  cout << "[PixelFEDAddressLevelCalibration::PixelFEDAddressLevelCalibration]" 
       << endl;
}

xoap::MessageReference PixelFEDAddressLevelCalibration::execute(xoap::MessageReference msg)
{

  Attribute_Vector parameters(2);
  parameters[0].name_="Hits";
  parameters[1].name_="First";
  Receive(msg, parameters);
  
  eventNumber++;

  bool noHits=parameters[0].value_=="No";

  if (parameters[0].value_!="Analyse"){
    analyze(noHits);
  }

  xoap::MessageReference reply = MakeSOAPMessageReference("AddressLevelCalibrationWithPixelsDone");
  return reply;
}

void PixelFEDAddressLevelCalibration::initializeFED(){

  setFEDModeAndControlRegister(0x0,0x30019);
}

xoap::MessageReference PixelFEDAddressLevelCalibration::beginCalibration(xoap::MessageReference msg){

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");

  tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject!=0);
  

  fedsAndChannels_=tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_ );

  aROC_string_=tempCalibObject->rocList();

  initAddressLevelCalibration();

  return reply;
}

xoap::MessageReference PixelFEDAddressLevelCalibration::endCalibration(xoap::MessageReference msg){

  summary();

  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

