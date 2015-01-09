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

#include "PixelCalibrations/include/PixelFEDGainAliveSCurveCalibrationWithSLink.h"

using namespace pos;


PixelFEDGainAliveSCurveCalibrationWithSLink::PixelFEDGainAliveSCurveCalibrationWithSLink(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  std::cout << "Greetings from the PixelFEDGainAliveSCurveCalibrationWithSLink copy constructor." << std::endl;
}

xoap::MessageReference PixelFEDGainAliveSCurveCalibrationWithSLink::execute(xoap::MessageReference msg)
{

  cout << "PixelFEDGainAliveSCurveCalibrationWithSLink::execute should never be called"
       << endl;

  xoap::MessageReference reply = MakeSOAPMessageReference("GainAliveSCurveCalibrationWithSLinkWithPixelsDone");
  return reply;

}

void PixelFEDGainAliveSCurveCalibrationWithSLink::initializeFED(){

  setFEDModeAndControlRegister(0x0,0x30010);

}

xoap::MessageReference PixelFEDGainAliveSCurveCalibrationWithSLink::beginCalibration(xoap::MessageReference msg){

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDGainAliveSCurveCalibrationWithSLink::endCalibration(xoap::MessageReference msg){
  cout << "In PixelFEDGainAliveSCurveCalibrationWithSLink::endCalibration()" << endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

