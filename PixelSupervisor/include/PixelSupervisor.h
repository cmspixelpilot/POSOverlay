/**************************************************************************
 * XDAQ Components for Pixel Online Software                              *
 * Copyright (C) 2007, Cornell University		                  *
 * All rights reserved.                                                   *
 * Authors: Souvik Das, Anders Ryd, Karl Ecklund			  *
  *************************************************************************/

#ifndef _PixelSupervisor_h_
#define _PixelSupervisor_h_

#include <stdint.h>

#include "xdaq/Application.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/Zone.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/ApplicationStubImpl.h"
#include "xdaq/exception/Exception.h"

#include "xdaq/NamespaceURI.h"

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPBody.h"
#include "xoap/domutils.h"
#include "xoap/Method.h"

#include "xdata/Integer.h"
#include "xdata/String.h"
#include "xdata/UnsignedLong.h"
#include "xdata/Properties.h"


#include "xgi/Utils.h"
#include "xgi/Method.h"
#include "cgicc/CgiDefs.h"
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

#include "toolbox/fsm/FiniteStateMachine.h"
#include "toolbox/fsm/FailedEvent.h"

#include "xdaq2rc/RcmsStateNotifier.h"

//#include "unistd.h"

#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"
#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"
#include "PixelSupervisorConfiguration/include/PixelSupervisorConfiguration.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"

// gio
#include "diagbag/DiagBagWizard.h"
#include "DiagCompileOptions.h"

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "toolbox/task/Timer.h"
#include "toolbox/task/TimerFactory.h"
#include "toolbox/task/TimerListener.h"
#include "toolbox/TimeInterval.h"
#include "xcept/Exception.h"


class pos::PixelConfigKey;
class PixelConfigDataUpdates;
class PixelJobControlMonitor;

class PixelSupervisor: public xdaq::Application, public PixelSupervisorConfiguration, public SOAPCommander, public toolbox::task::TimerListener
{
 public:

  toolbox::BSem executeReconfMethodMutex;

  XDAQ_INSTANTIATOR();

  /// PixelSupervisor Constructor.
  /// Bindings made to SOAP messages. Finite State Machine defined.
  PixelSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);
  ~PixelSupervisor(){};

  //gio
  void timeExpired (toolbox::task::TimerEvent& e);
  //

  void Default(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  void Experimental(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  void StateMachineXgiHandler(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  void LowLevelXgiHandler(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  void ExperimentalXgiHandler(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);

  ///
  /// SOAP Callback to reset the state machine
  xoap::MessageReference reset (xoap::MessageReference msg) throw (xoap::exception::Exception);

  /**
     1. Control Configuration and Function Manager are loaded.
  **/
  void stateInitial (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception);
	
  /**
     1. XDAQ applications are running.
     2. Hardware is running.
     3. Triggers are accepted.
     4. Triggers are not sent.
     5. Data is sent / read out.
  **/
  void statePaused (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception);

  /**
     1. XDAQ applications are running.
     2. Hardware is running.
     3. Triggers are accepted.
     4. Triggers are sent.
     5. Data is sent / read out.
  **/
  void stateRunning (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception);

  /**
     1. Control hierarchy is instantiated.
     2. XDAQ executives are running and configured.
     3. XDAQ applications are loaded and instantiated.
     4. DCS nodes are allocated.
  **/
  void stateHalted (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception);

  /**
     1. XDAQ applications are configured.
     2. Run parameters have been distributed.
     3. Hardware is configured.
     4. I2O connections are established, no data is sent or read out.
     5. Triggers are not sent.
  **/
  void transitionHaltedToConfiguring (toolbox::Event::Reference e) ; //throw (toolbox::fsm::exception::Exception)
  void stateConfiguring (toolbox::fsm::FiniteStateMachine & fsm) ; //throw (toolbox::fsm::exception::Exception);
  void stateConfigured (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception);
       
  void stateTTSTestMode (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception);

  void inError (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception);
  //void stateResetting (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception);
  void stateRecovering (toolbox::fsm::FiniteStateMachine & fsm);

  void stateDone (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception);
  void stateResetting (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception);
  //Soft Error Stuff:
  void stateRunningSoftErrorDetected (toolbox::fsm::FiniteStateMachine & fsm) ;
  void stateRunningDegraded (toolbox::fsm::FiniteStateMachine & fsm) ;
  void stateFixingSoftError (toolbox::fsm::FiniteStateMachine & fsm) ;
  void stateFixedSoftError (toolbox::fsm::FiniteStateMachine & fsm) ;


  /**
     1. Check the existence of Job Control services on the hosts running XDAQ applications.
     2. Start XDAQ executives if they are not running and configure them.
     3. Check for the existence of XDAQ applications.
     4. Check for the existence of DCS nodes and allocate them.
     5. Execute additional commands necessary to bring the system to the Halted state.
  **/
  xoap::MessageReference Initialize(xoap::MessageReference msg) throw (xoap::exception::Exception);
  
  /**
        Perform a reset of the hardware. (e.g. re-load FPGAs from PROM). 
        The action should include all steps necessary after power-up of hardware
  **/
  xoap::MessageReference ColdReset(xoap::MessageReference msg) throw (xoap::exception::Exception);

  /**
     1. Retrieve from the Database the electronics parameters corresponding to the received Configuration Key.
     2. Load the hardware registers and memories (LUTs, etc) with the relevant data.
     3. Veriy the content of the downloaded data (read back and compare).
     4. Execute additional commands necessary to the bring the system to the "Configuring" state.
  **/
  xoap::MessageReference Configure(xoap::MessageReference msg) throw (xoap::exception::Exception);

  /**
     1. Start a new run.
     2. Enable the hardware for triggering and data taking.
     3. Execute additional commands necessary to bring the system into the Running state.
  **/
  xoap::MessageReference Start(xoap::MessageReference msg) ;


  xoap::MessageReference Stop(xoap::MessageReference msg) throw (xoap::exception::Exception);

  /**
     1. Execute commands necessary to bring the system into the Paused state.
  **/
  xoap::MessageReference Pause(xoap::MessageReference msg) throw (xoap::exception::Exception);

  /**
     1. Execute commands necessary to bring the system into the Running state.
  **/
  xoap::MessageReference Resume(xoap::MessageReference msg) throw (xoap::exception::Exception);

  /**
     1. Terminate current run.
     2. Conditions data associated with the termination of a run may be stored in the Conditions Database.
     3. Execute commands necessary to bring the system to the Halted state.
  **/
  xoap::MessageReference Halt(xoap::MessageReference msg) throw (xoap::exception::Exception);

  /**
     After encountering an Error, attempt to resynchronize the Supervisors in the Halted state
  **/
  xoap::MessageReference Recover(xoap::MessageReference msg);

  //Soft Error Detection Stuff:

  xoap::MessageReference DetectSoftError(xoap::MessageReference msg);
  xoap::MessageReference DetectDegradation(xoap::MessageReference msg);
  xoap::MessageReference FixSoftError(xoap::MessageReference msg);

  /**
     Quick reconfiguration; used for timing scan
  **/
  xoap::MessageReference Reconfigure(xoap::MessageReference msg) ;

  xoap::MessageReference PrepareTTSTestMode (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference TestTTS (xoap::MessageReference msg) throw (xoap::exception::Exception);

  xoap::MessageReference Done(xoap::MessageReference msg) throw (xoap::exception::Exception);
        
  xoap::MessageReference FSMStateRequest(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference FSMStateNotification(xoap::MessageReference msg) throw (xoap::exception::Exception);

  xoap::MessageReference ResetTBM(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference ResetROC(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference ResetCCU(xoap::MessageReference msg) throw (xoap::exception::Exception);

  xoap::MessageReference StatusNotification(xoap::MessageReference msg) throw (xoap::exception::Exception);

  void enteringError (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);

  bool CalibRunning(toolbox::task::WorkLoop * w1);

  bool jobcontrol_workloop(toolbox::task::WorkLoop * w1);

  /**
     b2in calls are received and parsed here.
  **/        
  void b2inEvent(toolbox::mem::Reference* msg, xdata::Properties& plist) throw (b2in::nub::exception::Exception);
  
  xoap::MessageReference MakeSOAPConfigMessage(const std::string &app_class_name, const std::string &parameter_name, const std::string &parameter_value);

  void SendConfigurationToTTC();


 protected:
	
  toolbox::fsm::FiniteStateMachine fsm_;
  xdata::String state_; // used to reflect the current state to the outside world
  bool autoDone_;
  std::string runType_, runNumber_, outputDir_;
  std::string posOutputDirs_;
  std::stringstream* console_;

  xdaq2rc::RcmsStateNotifier rcmsStateNotifier_;


  bool TTCRunning_;
  bool autoRefresh_;

  // For calib running
  PixelCalibrationBase* theCalibAlgorithm_;
  toolbox::task::WorkLoop * calibWorkloop_;
  toolbox::task::ActionSignature * calibJob_;

  toolbox::task::WorkLoop * jobcontrolWorkloop_;
  toolbox::task::ActionSignature * jobcontrolTask_;

  void asynchronousExceptionNotification(xcept::Exception& e){}
  
  
 private:

  void updateConfig(std::string path, std::vector<std::string> aliases);      
  void printConfiguration(pos::PixelConfigKey& theGlobalKey);
  unsigned int getNumberOfSupervisorsInState(const SupervisorStates & ss, const string & state) ;
  unsigned int getNumberOfSupervisorsNotInState(const SupervisorStates & ss, const string & state) ;
  bool recoverSupervisors( const Supervisors & supervisor,  const SupervisorStates & state ) ;
  std::string getHtmlColorFromState( const std::string & state );
  bool fsmTransition (const std::string & transition);
  PixelConfigDataUpdates* updates_;

  PixelJobControlMonitor* jobcontrolmon_;

  xdata::Boolean useRunInfo_;
  xdata::String dbConnection_;
  xdata::String runSequence_;
  xdata::String dbUsername_;
  unsigned int bookRunNumber(std::string dbLogin, std::string password);
  void writeRunInfo(std::string dbLogin, std::string password, std::string runNumber, std::string parameterName, std::string parameterValue);

  std::vector <std::pair<std::string, unsigned int> > aliasesAndKeys_;

  float percentageConfigured_;
  bool runBeginCalibration_;

  bool reconfigureActive_;

  std::string lastMessage_;
  timeval lastMessageTime_;



  void ClearErrors(std::string which);

  void DIAG_CONFIGURE_CALLBACK();
  void DIAG_APPLY_CALLBACK();

  /* xgi method called when the link <display_diagsystem> is clicked */
  void callDiagSystemPage(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

  bool extratimers_;
  PixelTimer configurationTimer_;

};

#endif
