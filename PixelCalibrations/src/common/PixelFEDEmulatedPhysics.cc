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

#include "PixelCalibrations/include/PixelFEDEmulatedPhysics.h"

using namespace pos;


PixelFEDEmulatedPhysics::PixelFEDEmulatedPhysics(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  std::cout << "Greetings from the PixelFEDEmulatedPhysics copy constructor." << std::endl;
}

xoap::MessageReference PixelFEDEmulatedPhysics::execute(xoap::MessageReference msg)
{

  //FIXME this code needs to be fixed! (ryd)
  //Broken in migration.
  assert(0);
  //workloop_->submit(physicsRunning_);
  diagService_->reportError("Emulated Physics data taking job submitted to the workloop", DIAGINFO);
  //*console_<<"Configure. Emulated Physics data taking job submitted to the workloop"<<std::endl;
      
  xoap::MessageReference reply = MakeSOAPMessageReference("EmulatedPhysicsWithPixelsDone");
  return reply;

}

void PixelFEDEmulatedPhysics::initializeFED(){

  setFEDModeAndControlRegister(0x8,0x30014);
  setSpecialDac(1);
  workloop_->submit(physicsRunning_);
  // Fill playback memory with contents of Calib file 
  xoap::MessageReference fillTestDACmsg=MakeSOAPMessageReference("FillTestDAC");
  fillTestDAC(fillTestDACmsg);    

}

xoap::MessageReference PixelFEDEmulatedPhysics::beginCalibration(xoap::MessageReference msg){

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDEmulatedPhysics::endCalibration(xoap::MessageReference msg){
  cout << "In PixelFEDEmulatedPhysics::endCalibration()" << endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

