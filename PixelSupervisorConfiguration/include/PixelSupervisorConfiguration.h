/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelSupervisorConfiguration_h_
#define _PixelSupervisorConfiguration_h_

#include "xdaq/Application.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/ApplicationStubImpl.h"
#include "xdaq/exception/Exception.h"
#include "xdata/Boolean.h"

#include "xdaq/NamespaceURI.h"

// gio
// #include <diagbag/DiagBagWizard.h>
// #include "DiagCompileOptions.h"

#include "CalibFormats/SiPixelObjects/interface/PixelConfigKey.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelNameTranslation.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDetectorConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelAMC13Config.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTKFECConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFECConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFEDConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelPortcardMap.h"
#include "CalibFormats/SiPixelObjects/interface/PixelConfigFile.h"
#include "PixelUtilities/Pixelb2inUtilities/include/Pixelb2inCommander.h"
#include "xdaq/Application.h"
#include "PixelSupervisorConfiguration/include/PixelSupervisorConfigurationBase.h"

// temporary DiagSystem wrapper
#include "PixelSupervisorConfiguration/include/DiagWrapper.h"

//class PixelSupervisorConfiguration : public PixelSupervisorConfigurationBase{
class PixelSupervisorConfiguration : public PixelSupervisorConfigurationBase, public Pixelb2inCommander{
 public:

  PixelSupervisorConfiguration(std::string* runNumber, 
  			       std::string* outputDir, xdaq::Application* app);

  // PixelSupervisorConfiguration(std::string* runNumber, 
  //			       std::string* outputDir);

  PixelSupervisorConfiguration( const PixelSupervisorConfiguration & );
  
  ~PixelSupervisorConfiguration(){};

  /* const std::map<xdata::UnsignedIntegerT, xdaq::ApplicationDescriptor*>& getPixelFECDescriptors() const { return PixelFECSupervisors_; }
  const std::map<xdata::UnsignedIntegerT, xdaq::ApplicationDescriptor*>& getPixelFEDDescriptors() const { return PixelFEDSupervisors_; }
  const std::map<xdata::UnsignedIntegerT, xdaq::ApplicationDescriptor*>& getPixelTKFECDescriptors() const { return PixelTKFECSupervisors_; }
  const std::map<xdata::UnsignedIntegerT, xdaq::ApplicationDescriptor*>& getPixelLTCDescriptors() const { return PixelLTCSupervisors_; }
  const std::map<xdata::UnsignedIntegerT, xdaq::ApplicationDescriptor*>& getPixelTTCDescriptors() const { return PixeliCISupervisors_; }
  const std::map<xdata::UnsignedIntegerT, xdaq::ApplicationDescriptor*>& getPixelDCSDescriptors() const { return PixelDCSSupervisors_; } */

  std::vector<xdaq::ApplicationDescriptor*> getPixelTTCDescriptors() const;
  std::vector<xdaq::ApplicationDescriptor*> getPixelTTCControllerDescriptors() const;

  const xdaq::ApplicationDescriptor* getPixelFECDescriptor(xdata::UnsignedIntegerT instance) { return  PixelFECSupervisors_[instance]; }
  const xdaq::ApplicationDescriptor* getPixelFEDDescriptor(xdata::UnsignedIntegerT instance) { return  PixelFEDSupervisors_[instance]; }
  const xdaq::ApplicationDescriptor* getPixelTKFECDescriptor(xdata::UnsignedIntegerT instance) { return  PixelTKFECSupervisors_[instance]; }
  const xdaq::ApplicationDescriptor* getPixelLTCDescriptor(xdata::UnsignedIntegerT instance) { return  PixelLTCSupervisors_[instance]; }
  const xdaq::ApplicationDescriptor* getPixelTTCDescriptor(xdata::UnsignedIntegerT instance) { return  PixelTTCSupervisors_[instance]; }
  const xdaq::ApplicationDescriptor* getPixelTTCControllDescriptor(xdata::UnsignedIntegerT instance) { return  PixelTTCControllers_[instance]; }
  const xdaq::ApplicationDescriptor* getPixelDCSDescriptor(xdata::UnsignedIntegerT instance) { return  PixelDCSSupervisors_[instance]; }
  const xdaq::ApplicationDescriptor* getPixelDCStoTrkFECDpInterface(xdata::UnsignedIntegerT instance) { return PixelDCStoTrkFECDpInterface_[instance]; }
  const xdaq::ApplicationDescriptor* getPixelSlinkMonitor(xdata::UnsignedIntegerT instance) { return PixelSlinkMonitors_[instance]; }

  const pos::PixelConfigKey* getGlobalKey() const { return theGlobalKey_; }
  const pos::PixelCalibBase* getCalibObject() const { return theCalibObject_; }
  const pos::PixelNameTranslation* getNameTranslation() const { return theNameTranslation_; }
  const pos::PixelDetectorConfig* getDetectorConfiguration() const { return theDetectorConfiguration_; }
  const pos::PixelTKFECConfig* getTKFECConfiguration() const { return theTKFECConfiguration_; }
  const pos::PixelFECConfig* getFECConfiguration() const { return theFECConfiguration_; }
  const pos::PixelFEDConfig* getFEDConfiguration() const { return theFEDConfiguration_; }
  std::map<std::string,pos::PixelPortCardConfig*> * getmapNamePortCard();
  const pos::PixelPortcardMap* getPortcardMap() const { return thePortcardMap_; }
  
  inline std::string stringF(int number) { stringstream ss; ss << number; return ss.str(); };
  inline std::string stringF(const char* text) { stringstream ss; ss << text; return ss.str(); };

 protected:

  typedef std::map<xdata::UnsignedIntegerT, xdaq::ApplicationDescriptor*> Supervisors;
  typedef std::map<xdata::UnsignedIntegerT, std::string > SupervisorStates;
  // key is the crate number
  
  Supervisors PixelFECSupervisors_, 
              PixelFEDSupervisors_, 
              PixelTKFECSupervisors_,
              PixelLTCSupervisors_,
              PixelTTCSupervisors_,
              PixelTTCControllers_,
              PixelAMC13Controllers_,
              PixelDCSSupervisors_,
              PixelDCStoTrkFECDpInterface_,
              PixelDCSFSMInterface_,
              psxServers_,
              PixelSlinkMonitors_;
              
  SupervisorStates statePixelFECSupervisors_,
                   statePixelFEDSupervisors_,
                   statePixelTKFECSupervisors_,
                   statePixelLTCSupervisors_,
                   statePixelTTCSupervisors_,
                   statePixelDCSFSMInterface_,
                   statePixelDCStoTrkFECDpInterface_;


  pos::PixelConfigKey *theGlobalKey_;
  pos::PixelCalibBase *theCalibObject_;
  pos::PixelNameTranslation *theNameTranslation_;
  pos::PixelDetectorConfig *theDetectorConfiguration_;
  pos::PixelTKFECConfig *theTKFECConfiguration_;
  pos::PixelFECConfig *theFECConfiguration_;
  pos::PixelFEDConfig *theFEDConfiguration_;
  pos::PixelLTCConfig* theLTCConfig_;
  pos::PixelTTCciConfig* theTTCciConfig_;
  pos::PixelAMC13Config* theAMC13Config_;
  pos::PixelPortcardMap *thePortcardMap_;
  // DiagBagWizard* diagService_;
  DiagWrapper* diagService_;
  static const int DIAGDEBUG = 0;
  static const int DIAGTRACE = 1;
  static const int DIAGUSERINFO = 2;
  static const int DIAGINFO = 3;
  static const int DIAGWARN = 4;
  static const int DIAGERROR = 5;
  static const int DIAGFATAL = 6;

  void clearMapNamePortCard() ;
  void writeAllFEDCards(std::vector<std::string> filenames);
  void writeAllPortcards(std::vector<std::string> filenames);
  void writeAllDACSettings(std::vector<std::string> filenames);
  void writeAllTBMSettings(std::vector<std::string> filenames);

  bool isAliasOK(std::string alias);
  void fillBadAliasList();

  // TTC and TCDS switches
  xdata::Boolean useTTC_;
  xdata::Boolean useTCDS_;
  std::string TCDSSessionID_;
  
 private:

  std::set<std::string> badAliases_; //list of aliases we never want to see

  //must access this through getmapNamePortcard()
  std::map<std::string,pos::PixelPortCardConfig*> mapNamePortCard_;
  void fillMapNamePortCard() ;

  bool contains(const std::vector<std::string>& filenames,std::string filename);
  PixelSupervisorConfiguration();
  
};

#endif
