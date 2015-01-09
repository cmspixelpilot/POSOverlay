/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelTKFECSupervisorConfiguration_h_
#define _PixelTKFECSupervisorConfiguration_h_

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
#include "CalibFormats/SiPixelObjects/interface/PixelConfigFile.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelNameTranslation.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDetectorConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTKFECConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelPortCardConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelPortcardMap.h"
#include "CalibFormats/SiPixelObjects/interface/PixelPortCardSettingNames.h"
#include "CalibFormats/SiPixelObjects/interface/PixelGlobalDelay25.h"

#include "FecVmeRingDevice.h"
#include "APIAccess.h"

#include "PixelSupervisorConfigurationBase.h"
//#include "VMEDevice.hh" 

class PixelTKFECSupervisorConfiguration : public PixelSupervisorConfigurationBase {
 public:

  //Will have pointers to runNumber and outputDir in TKFECSupervisor
  PixelTKFECSupervisorConfiguration(std::string* runNumber, 
				  std::string* outputDir);
  PixelTKFECSupervisorConfiguration( const PixelTKFECSupervisorConfiguration & );
  
  ~PixelTKFECSupervisorConfiguration(){};

 protected:

  pos::PixelConfigKey *theGlobalKey_;
  std::map<std::string,pos::PixelPortCardConfig*> mapNamePortCard_;
  pos::PixelPortcardMap* thePortcardMap_;
  pos::PixelTKFECConfig *theTKFECConfiguration_;
  pos::PixelNameTranslation *theNameTranslation_;
  pos::PixelFECConfig *theFECConfiguration_;
  std::map<xdata::UnsignedIntegerT, xdaq::ApplicationDescriptor*> PixelFECSupervisors_;
  pos::PixelGlobalDelay25 *theGlobalDelay25_;

  unsigned long crate_;

  pos::PixelCalibBase *theCalibObject_;

  DiagBagWizard *diagService_;

  FecAccess *fecAccess_;

  // A WorkLoop and a Job just for it!
  /*
  toolbox::task::WorkLoop *workloop_;
  toolbox::task::ActionSignature *tempTransmitter_;
  toolbox::task::ActionSignature *physicsRunning_;
  */

  /* void setupOutputDir(); */

/*   std::string outputDir(); */

/*   std::string runDir(); */

// private:

//  std::string *runNumber_;
//  std::string *outputDir_;

  //Should never call default constuctor
 PixelTKFECSupervisorConfiguration():PixelSupervisorConfigurationBase(){};

};

#endif
