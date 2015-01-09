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

#include "PixelCalibrations/include/PixelFEDGainAliveSCurveCalibration.h"

using namespace pos;


PixelFEDGainAliveSCurveCalibration::PixelFEDGainAliveSCurveCalibration(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  std::cout << "Greetings from the PixelFEDGainAliveSCurveCalibration copy constructor." << std::endl;
}

xoap::MessageReference PixelFEDGainAliveSCurveCalibration::execute(xoap::MessageReference msg)
{

  cout << "PixelFEDGainAliveSCurveCalibration::execute should never be called"
       << endl;

  xoap::MessageReference reply = MakeSOAPMessageReference("GainAliveSCurveCalibrationWithPixelsDone");
  return reply;

}

void PixelFEDGainAliveSCurveCalibration::initializeFED(){

  setFEDModeAndControlRegister(0x8,0x30010);

}

xoap::MessageReference PixelFEDGainAliveSCurveCalibration::beginCalibration(xoap::MessageReference msg){

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDGainAliveSCurveCalibration::endCalibration(xoap::MessageReference msg){
  cout << "In PixelFEDGainAliveSCurveCalibration::endCalibration()" << endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

