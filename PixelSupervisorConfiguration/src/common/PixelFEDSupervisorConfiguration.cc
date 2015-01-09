// $Id: PixelFEDSupervisorConfiguration.cc,v 1.13 2009/01/09 04:30:53 yw266 Exp $: PixelSupervisorConfiguration.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelSupervisorConfiguration/include/PixelFEDSupervisorConfiguration.h"

using namespace pos;

PixelFEDSupervisorConfiguration::PixelFEDSupervisorConfiguration(std::string* runNumber, 
								 std::string* outputDir, xdaq::Application* app):PixelSupervisorConfigurationBase(runNumber, outputDir), Pixelb2inCommander(app)
{
  theGlobalKey_=0;
  theNameTranslation_=0;
  theDetectorConfiguration_=0;
  theFEDConfiguration_=0;
  theCalibObject_=0;
  theGlobalDelay25_=0;
  crate_=0;
  diagService_=0;
  console_=0;
  workloop_=0;
  tempTransmitter_=0;
  physicsRunning_=0;

}

PixelFEDSupervisorConfiguration::PixelFEDSupervisorConfiguration(const PixelFEDSupervisorConfiguration & tempConfiguration):PixelSupervisorConfigurationBase(tempConfiguration), Pixelb2inCommander(tempConfiguration)
{
  
  //just shallow copying for the moment
  //pointers
  theGlobalKey_ = tempConfiguration.theGlobalKey_;
  theNameTranslation_ = tempConfiguration.theNameTranslation_;
  theDetectorConfiguration_ = tempConfiguration.theDetectorConfiguration_;
  theFEDConfiguration_ = tempConfiguration.theFEDConfiguration_;
  theCalibObject_ = tempConfiguration.theCalibObject_;
  theGlobalDelay25_ = tempConfiguration.theGlobalDelay25_;
  
  VMEPtr_ = tempConfiguration.VMEPtr_;
  FEDInterface_ = tempConfiguration.FEDInterface_;
  FEDInterfaceFromFEDnumber_ = tempConfiguration.FEDInterfaceFromFEDnumber_; 
  dataFIFO1_ = tempConfiguration.dataFIFO1_;
  dataFIFO2_ = tempConfiguration.dataFIFO2_;
  dataFIFO3_ = tempConfiguration.dataFIFO3_;
  errorFIFO_ = tempConfiguration.errorFIFO_;
  tempFIFO_ = tempConfiguration.tempFIFO_;
  ttsFIFO_ = tempConfiguration.ttsFIFO_;
  vmeBaseAddressAndFEDNumberAndChannels_ = tempConfiguration.vmeBaseAddressAndFEDNumberAndChannels_;
  
  crate_ = tempConfiguration.crate_;
  diagService_ = tempConfiguration.diagService_;

  console_=tempConfiguration.console_;

  workloop_=tempConfiguration.workloop_;
  tempTransmitter_=tempConfiguration.tempTransmitter_;
  physicsRunning_=tempConfiguration.physicsRunning_;

}


