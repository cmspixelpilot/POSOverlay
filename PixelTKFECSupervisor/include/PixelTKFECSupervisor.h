//--*-C++-*--
/**************************************************************************
 * XDAQ Components for Pixel Online Software                              *
 * Copyright (C) 2007, Cornell University		                  *
 * All rights reserved.                                                   *
 * Authors: Souvik Das, Anders Ryd, Karl Ecklund			  *
  *************************************************************************/

#ifndef _PixelTKFECSupervisor_h_
#define _PixelTKFECSupervisor_h_

//#define VMEDUMMY

#include "xdaq/Application.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/ApplicationStubImpl.h"
#include "xdaq/exception/Exception.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/NamespaceURI.h"

#include "xdata/UnsignedLong.h"
#include "xdata/String.h"
#include "xdata/Integer.h"
#include "xdata/Integer32.h"

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/AttachmentPart.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPMessage.h"
#include "xoap/SOAPBody.h"
#include "xoap/SOAPPart.h"
#include "xoap/DOMParser.h"

#include "xoap/domutils.h"
//#include "xcept/tools.h"

#include "xoap/Method.h"

// Includes for the GUI
#include "xdaq/WebApplication.h"
#include "xgi/Method.h"
#include "xgi/Utils.h"
#include "cgicc/HTMLClasses.h"

// gio
#include <diagbag/DiagBagWizard.h>
#include "DiagCompileOptions.h"

#include "toolbox/fsm/AsynchronousFiniteStateMachine.h"
#include "toolbox/fsm/FailedEvent.h"
#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "toolbox/task/WorkLoopFactory.h"
#include "toolbox/task/WaitingWorkLoop.h"
#include "toolbox/task/Action.h"
#include "toolbox/lang/Class.h"
#include "toolbox/net/URN.h"
#include "toolbox/BSem.h"

#include "VMEAddressTable.hh"
#include "VMEAddressTableXMLFileReader.hh"
#include "VMEAddressTableASCIIReader.hh"

#ifdef VMEDUMMY
#include "VMEDummyBusAdapter.hh"
#else
#include "CAENLinuxBusAdapter.hh"
#endif

//#include "CAENVMElib.h"  // CAEN library prototypes  NOT needed anymore? kme 10/05/06
#include "VMEDevice.hh" // add d.k.

#include "PixelCalibrations/include/PixelTKFECCalibrationBase.h"
#include "PixelSupervisorConfiguration/include/PixelTKFECSupervisorConfiguration.h"
#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"
#include "PixelUtilities/PixelGUIUtilities/include/HTML2XGI.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelPowerMap4602.h"

//for debugging
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"

#define BPIX

class PixelTKFECSupervisor: public xdaq::Application, public SOAPCommander, public toolbox::task::TimerListener, public PixelTKFECSupervisorConfiguration

{

public:

  // gio
  toolbox::BSem executeReconfMethodMutex;
  DiagBagWizard * diagService_;
  //
  
  XDAQ_INSTANTIATOR();
  
  PixelTKFECSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);
  ~PixelTKFECSupervisor();
  //gio
  void timeExpired (toolbox::task::TimerEvent& e);
  //
  void helpMe( char *programName );
  void createFecAccess ( int argc, char **argv, int *cnt , unsigned int slot);
  void readTemp();
  xoap::MessageReference readDCU(xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  xoap::MessageReference readDCU_fakeSOAP(xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  void portcardI2CDevice(FecAccess *fecAccess,
			 tscType8 fecAddress,
			 tscType8 ringAddress,
			 tscType8 ccuAddress,
			 tscType8 channelAddress,
			 tscType8 deviceAddress,
			 enumDeviceType modeType,
			 unsigned int value,
			 int flag);
  
  int portcardI2CDeviceRead(FecAccess *fecAccess,
			    tscType8 fecAddress,
			    tscType8 ringAddress,
			    tscType8 ccuAddress,
			    tscType8 channelAddress,
			    tscType8 deviceAddress,
			    enumDeviceType modeType,
			    int flag);
  
  void Default(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  void XgiHandler(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  void CCUBoardGUI(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  void CCUBoardGUI_XgiHandler(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  
  xoap::MessageReference Initialize (xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  xoap::MessageReference Configure (xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  xoap::MessageReference Start (xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  xoap::MessageReference Stop (xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  xoap::MessageReference Pause (xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  xoap::MessageReference Resume (xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  xoap::MessageReference Halt (xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  xoap::MessageReference Recover (xoap::MessageReference msg) ;
  xoap::MessageReference Reconfigure (xoap::MessageReference msg);
  xoap::MessageReference Reset (xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  
  xoap::MessageReference FixSoftError (xoap::MessageReference msg) ;
  xoap::MessageReference ResumeFromSoftError (xoap::MessageReference msg) ;
  
#ifdef BPIX
  xoap::MessageReference PIAReset (xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
#endif
  
  xoap::MessageReference FSMStateRequest (xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  xoap::MessageReference SetDelay (xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  //stuff for the PixelTKFECSupervisor.cc file remember to include a line in the header file
  xoap::MessageReference SetDelayEnMass(xoap::MessageReference msg);
  xoap::MessageReference SetAOHGainEnMass (xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  xoap::MessageReference SetAOHBiasEnMass (xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  xoap::MessageReference SetAOHBiasOneChannel (xoap::MessageReference msg);// throw (xoap::exception::Exception);
  xoap::MessageReference TKFECCalibrations (xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  xoap::MessageReference beginCalibration(xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  xoap::MessageReference endCalibration(xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  xoap::MessageReference ResetCCU(xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  xoap::MessageReference AOH (xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  xoap::MessageReference fsmStateNotification(xoap::MessageReference msg) ; //throw (xoap::exception::Exception);
  bool SetAOHBias(std::string portCardName, unsigned int AOHNumber, unsigned int AOHBiasValue);
  bool SetPortCardSetting(const pos::PixelPortCardConfig* portCardConfig, unsigned int deviceAddress, unsigned int settingValue);
  
  // With PLL_CTR4 and PLL_CTR5, use these instead of SetPortCardSetting().  Need to input the last PLL_CTR2 value written -- this can be obtained from the portCardConfig if PLL_CTR2 has not been changed since Configuring.
  bool SetPLL_CTR4(const pos::PixelPortCardConfig* portCardConfig, unsigned int settingValue, unsigned int last_PLL_CTR2_value);
  bool SetPLL_CTR5(const pos::PixelPortCardConfig* portCardConfig, unsigned int settingValue, unsigned int last_PLL_CTR2_value);
  bool ReadDCU_workloop(toolbox::task::WorkLoop *w1);
  xoap::MessageReference ReadDCU_workloop_fakeSOAP(xoap::MessageReference msg);	// method to test exception handling
  bool checkForResets(toolbox::task::WorkLoop *w1);
  void stateChanged(toolbox::fsm::FiniteStateMachine &fsm); //throw (toolbox::fsm::exception::Exception);
  void stateConfiguring(toolbox::fsm::FiniteStateMachine &fsm) ;
  void stateConfigured(toolbox::fsm::FiniteStateMachine &fsm) ;
  void stateFixingSoftError(toolbox::fsm::FiniteStateMachine &fsm) ;
  void transitionHaltedToConfiguring (toolbox::Event::Reference e) ; //throw (toolbox::fsm::exception::Exception);
  void enteringError(toolbox::Event::Reference e);
  
protected:
  
  toolbox::fsm::AsynchronousFiniteStateMachine fsm_;
  xdata::String state_;

private:
  
  bool doDCULoop_;
  bool printReadBack_; //debug use only; prints written and readback data
  bool debug_;         // debug printout (intended originally for DCU stuff)
  xdata::Integer readDCU_workloop_usleep_;    // the number of microseconds to sleep ReadDCU_workloop

  // A Workloop and a Job for it!
  toolbox::task::WorkLoop *workloop_;
  toolbox::task::ActionSignature *readDCU_;
  //another workloop, for the reset check
  bool doResetCheck_;
  toolbox::task::WorkLoop *resetCheckWorkloop_;
  toolbox::task::ActionSignature *resetCheck_;
  xdaq::ApplicationDescriptor* PixelDCStoTKFECDpInterface_;
  xdaq::ApplicationDescriptor* PixelDCSFSMInterface_;        
  xdaq::ApplicationDescriptor* PixelSupervisor_;
  PixelPowerMap4602 powerMap_;
  std::string runNumber_;
  std::string outputDir_;
  PixelTKFECCalibrationBase* theTKFECCalibrationBase_;
  
  void cleanupGlobalConfigData() ;
  void printPIAinfo(int slot,int ring,int ccu,int channel) ;
  void printI2Cinfo(int slot,int ring,int ccu,int channel) ;
  void writePIADataReg(int slot,int ring,int ccu,int channel,tscType8 value) ;
  void printCCUCRE(int slot,int ring,int ccu) ;
  void disablePIAchannels(int slot,int ring,int ccu)  ;
  void enablePIAchannels(int slot,int ring,int ccu)  ;
  bool programPortcards(bool);
  void DIAG_CONFIGURE_CALLBACK();
  void DIAG_APPLY_CALLBACK();

  /* xgi method called when the link <display_diagsystem> is clicked */
  void callDiagSystemPage(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

  PixelTimer GlobalTimer_;
  PixelTimer fsmStateNotificationTimer_;
  bool extratimers_;
  toolbox::BSem* dculock_;
  toolbox::BSem* rclock_;
  toolbox::BSem* hardwarelock_;
  bool workloopContinue_;
  bool workloopContinueRC_;
  bool suppressHardwareError_;

};
#endif
