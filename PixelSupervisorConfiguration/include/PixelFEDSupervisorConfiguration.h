/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFEDSupervisorConfiguration_h_
#define _PixelFEDSupervisorConfiguration_h_

#include "xdaq/Application.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/ApplicationStubImpl.h"
#include "xdaq/exception/Exception.h"

#include "xdaq/NamespaceURI.h"

#include <diagbag/DiagBagWizard.h>
#include "DiagCompileOptions.h"

#include "CalibFormats/SiPixelObjects/interface/PixelConfigKey.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelNameTranslation.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDetectorConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFECConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFEDConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelGlobalDelay25.h"
#include "PixelFEDInterface/include/PixelFEDInterface.h"
#include "CalibFormats/SiPixelObjects/interface/PixelConfigFile.h"
#include "VMEDevice.hh" 

#include "PixelSupervisorConfigurationBase.h"
#include "PixelUtilities/Pixelb2inUtilities/include/Pixelb2inCommander.h"
#include "xdaq/Application.h"

class PixelFEDSupervisorConfiguration : public PixelSupervisorConfigurationBase, public Pixelb2inCommander{

  public:

  //Will have pointers to runNumber abd outputDir in FEDSupervisor
  PixelFEDSupervisorConfiguration(std::string* runNumber, 
				  std::string* outputDir, xdaq::Application* app);
  PixelFEDSupervisorConfiguration( const PixelFEDSupervisorConfiguration & );

  ~PixelFEDSupervisorConfiguration(){};

 protected:

  pos::PixelConfigKey *theGlobalKey_;
  pos::PixelNameTranslation *theNameTranslation_;
  pos::PixelDetectorConfig *theDetectorConfiguration_;
  pos::PixelFEDConfig *theFEDConfiguration_;
  pos::PixelCalibBase *theCalibObject_;
  pos::PixelGlobalDelay25 *theGlobalDelay25_;

  typedef map <unsigned long, HAL::VMEDevice*> VMEPointerMap;
  typedef map <unsigned long, PixelFEDInterface*> FEDInterfaceMap;
  typedef map <unsigned long, std::stringstream*> FIFO;
  
  VMEPointerMap VMEPtr_;
  FEDInterfaceMap FEDInterface_;
  FEDInterfaceMap FEDInterfaceFromFEDnumber_;
  FIFO dataFIFO1_, dataFIFO2_, dataFIFO3_, errorFIFO_, tempFIFO_, ttsFIFO_;

  std::map<std::pair<unsigned long, unsigned int>, std::set<unsigned int> > vmeBaseAddressAndFEDNumberAndChannels_;
  //         VME Base Address  FED Number              Channel

  unsigned long crate_;
  DiagBagWizard * diagService_;

  std::stringstream* console_;

  // A WorkLoop and a Job just for it!
  toolbox::task::WorkLoop *workloop_;
  toolbox::task::ActionSignature *tempTransmitter_;
  toolbox::task::ActionSignature *physicsRunning_;

 private:

  //Should never call default constuctor
  PixelFEDSupervisorConfiguration(); 

};

#endif
