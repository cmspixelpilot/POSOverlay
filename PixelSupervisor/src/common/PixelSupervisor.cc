// $Id: PixelSupervisor.cc,v 1.230 2012/10/01 22:07:37 mdunser Exp $
/**************************************************************************
 * XDAQ Components for Pixel Online Software                              *
 * Copyright (C) 2007, Cornell University		                  *
 * All rights reserved.                                                   *
 * Authors: Souvik Das, Anders Ryd, Karl Ecklund			  *
  *************************************************************************/

/* jmt:
idea for the future -- revive the lastMessage_ variable to really hold the last error
that was handled. Basically, in any catch block, put e.what() into lastMessage_.

Then have this reported to the L1FM via inError()

but this is non-trivial enough that I don't want to do it during steady running
*/

// Change to the new (version X) style state changes. d.k. 11/1/2012
// Modification from Manuel (default alias). 2/14

#include "PixelSupervisor/include/PixelSupervisor.h"

//gio
#include <toolbox/convertstring.h>
#include <diagbag/DiagBagWizard.h>
#include "DiagCompileOptions.h"
#include "b2in/utils/MessengerCacheListener.h"
#include "xdata/String.h"


#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelConfigKey.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "CalibFormats/SiPixelObjects/interface/PixelNameTranslation.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDetectorConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTKFECConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFECConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFEDConfig.h"

#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSSOAPCommander.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSPVSSCommander.h"

#include "PixelUtilities/PixelJobControlUtilities/include/PixelJobControlMonitor.h"

#include "PixelCalibrations/include/PixelCalibrationFactory.h"
#include "PixelCalibrations/include/PixelCalibrationBase.h"
#include "PixelCalibrations/include/PixelConfigDataUpdates.h"

#include "pstream.h"

#include <iomanip>
#include <cstdlib>

#include "xcept/Exception.h"
#include "xdaq/exception/ApplicationInstantiationFailed.h"



using namespace pos;

//
// provides factory method for instantion of PixelSupervisor application
XDAQ_INSTANTIATOR_IMPL(PixelSupervisor)

PixelSupervisor::PixelSupervisor(xdaq::ApplicationStub * s)
                          throw (xdaq::exception::Exception)
                          :xdaq::Application(s),
                          PixelSupervisorConfiguration(&runNumber_,&outputDir_, this),
                          SOAPCommander(this),
                          executeReconfMethodMutex(toolbox::BSem::FULL),
                          rcmsStateNotifier_(getApplicationLogger(), getApplicationDescriptor(),getApplicationContext()),
			  extratimers_(false)
{

  diagService_ = new DiagBagWizard(
                                   ("ReconfigurationModule") ,
                                   this->getApplicationLogger(),
                                   getApplicationDescriptor()->getClassName(),
                                   getApplicationDescriptor()->getInstance(),
                                   getApplicationDescriptor()->getLocalId(),
                                   (xdaq::WebApplication *)this,
                                   "Pixel",
                                   "PixelSupervisor"
                                   );


  DIAG_DECLARE_USER_APP

    //uncomment these if you want to test the operation of the diagSystem
    // diagService_->reportError("The DiagSystem is installed --- this is a bogus error message",DIAGTRACE);
    //diagService_->reportError("The DiagSystem is installed --- this is a bogus error message",DIAGDEBUG);
    //diagService_->reportError("The DiagSystem is installed --- this is a bogus error message",DIAGINFO);
    //diagService_->reportError("The DiagSystem is installed --- this is a bogus error message",DIAGWARN);
    diagService_->reportError("The DiagSystem is installed --- this is a bogus error message",DIAGUSERINFO);
  //  diagService_->reportError("The DiagSystem is installed --- this is a bogus error message",DIAGERROR);

  // xoap bindings
  xoap::bind(this, &PixelSupervisor::Initialize, "Initialize", XDAQ_NS_URI);
  xoap::bind(this, &PixelSupervisor::ColdReset, "ColdReset", XDAQ_NS_URI);
  xoap::bind(this, &PixelSupervisor::Configure, "Configure", XDAQ_NS_URI );
  xoap::bind(this, &PixelSupervisor::Halt, "Halt", XDAQ_NS_URI );
  xoap::bind(this, &PixelSupervisor::Start, "Start", XDAQ_NS_URI );
  xoap::bind(this, &PixelSupervisor::Stop, "Stop", XDAQ_NS_URI );
  xoap::bind(this, &PixelSupervisor::Pause, "Pause", XDAQ_NS_URI );
  xoap::bind(this, &PixelSupervisor::Done, "Done", XDAQ_NS_URI );
  xoap::bind(this, &PixelSupervisor::PrepareTTSTestMode, "TTSTestMode", XDAQ_NS_URI );
  xoap::bind(this, &PixelSupervisor::TestTTS, "TestTTS", XDAQ_NS_URI );
  xoap::bind(this, &PixelSupervisor::Resume, "Resume", XDAQ_NS_URI);
  xoap::bind(this, &PixelSupervisor::reset, "Reset", XDAQ_NS_URI );
  xoap::bind(this, &PixelSupervisor::Recover, "Recover", XDAQ_NS_URI );
  xoap::bind(this, &PixelSupervisor::FSMStateRequest, "FSMStateRequest", XDAQ_NS_URI);
  xoap::bind(this, &PixelSupervisor::FSMStateNotification, "FSMStateNotification", XDAQ_NS_URI);

  //Adding Soft Error Detection Stuff

  xoap::bind(this, &PixelSupervisor::DetectSoftError, "DetectSoftError", XDAQ_NS_URI );
  xoap::bind(this, &PixelSupervisor::FixSoftError, "FixSoftError", XDAQ_NS_URI );

  xoap::bind(this, &PixelSupervisor::DetectDegradation, "DetectDegradation", XDAQ_NS_URI );

  xoap::bind(this, &PixelSupervisor::Reconfigure, "Reconfigure", XDAQ_NS_URI );

  xoap::bind(this, &PixelSupervisor::ResetTBM, "ResetTBM", XDAQ_NS_URI );
  xoap::bind(this, &PixelSupervisor::ResetROC, "ResetROC", XDAQ_NS_URI );
  xoap::bind(this, &PixelSupervisor::ResetCCU, "ResetCCU", XDAQ_NS_URI );

  xoap::bind(this, &PixelSupervisor::StatusNotification, "StatusNotification", XDAQ_NS_URI);

  xgi::bind(this, &PixelSupervisor::Default, "Default");
  xgi::bind(this, &PixelSupervisor::Experimental, "Experimental");
  xgi::bind(this, &PixelSupervisor::StateMachineXgiHandler, "StateMachineXgiHandler");
  xgi::bind(this, &PixelSupervisor::LowLevelXgiHandler, "LowLevelXgiHandler");
  xgi::bind(this, &PixelSupervisor::ExperimentalXgiHandler, "ExperimentalXgiHandler");

  //DIAGNOSTIC REQUESTED CALLBACK
  xgi::bind(this,&PixelSupervisor::configureDiagSystem, "configureDiagSystem");
  xgi::bind(this,&PixelSupervisor::applyConfigureDiagSystem, "applyConfigureDiagSystem");
  xgi::bind(this,&PixelSupervisor::callDiagSystemPage, "callDiagSystemPage");

  //b2in callback registration
  b2in::nub::bind(this, &PixelSupervisor::b2inEvent);

  fsm_.addState('I', "Initial", this, &PixelSupervisor::stateInitial);
  fsm_.addState('H', "Halted", this, &PixelSupervisor::stateHalted);
  fsm_.addState('c', "Configuring", this, &PixelSupervisor::stateConfiguring);
  fsm_.addState('C', "Configured", this, &PixelSupervisor::stateConfigured);
  fsm_.addState('R', "Running", this, &PixelSupervisor::stateRunning);
  fsm_.addState('D', "Done", this, &PixelSupervisor::stateDone);
  fsm_.addState('P', "Paused", this, &PixelSupervisor::statePaused);
  fsm_.addState('v', "Recovering", this, &PixelSupervisor::stateRecovering);
  fsm_.addState('T', "TTSTestMode", this, &PixelSupervisor::stateTTSTestMode);

  //Adding Soft Error Detection Stuff

  fsm_.addState('r', "RunningSoftErrorDetected", this, &PixelSupervisor::stateRunningSoftErrorDetected);
  fsm_.addState('s', "FixingSoftError", this, &PixelSupervisor::stateFixingSoftError);
  fsm_.addState('S', "FixedSoftError", this, &PixelSupervisor::stateFixedSoftError);

  fsm_.addState('d', "RunningDegraded", this, &PixelSupervisor::stateRunningDegraded);

  fsm_.setStateName('F',"Error");
  fsm_.setFailedStateTransitionAction(this, &PixelSupervisor::enteringError);
  fsm_.setFailedStateTransitionChanged(this, &PixelSupervisor::inError);

  fsm_.addStateTransition('I', 'H', "Initialize");
  fsm_.addStateTransition('H', 'H', "ColdReset");
  fsm_.addStateTransition('H', 'c', "Configure", this, &PixelSupervisor::transitionHaltedToConfiguring);
  fsm_.addStateTransition('c', 'c', "Configure");
  fsm_.addStateTransition('c', 'C', "ConfiguringDone");
  fsm_.addStateTransition('C', 'R', "Start");
  fsm_.addStateTransition('R', 'C', "Stop");
  fsm_.addStateTransition('R', 'P', "Pause");
  fsm_.addStateTransition('P', 'R', "Resume");
  fsm_.addStateTransition('P', 'C', "Stop");
  fsm_.addStateTransition('C', 'H', "Halt");
  fsm_.addStateTransition('P', 'H', "Halt");
  fsm_.addStateTransition('R', 'H', "Halt");
  fsm_.addStateTransition('H', 'T', "PrepareTTSTestMode");
  fsm_.addStateTransition('T', 'T', "TestTTS");
  fsm_.addStateTransition('T', 'H', "Halt");
  fsm_.addStateTransition('R', 'D', "Done");
  fsm_.addStateTransition('P', 'D', "Done");
  fsm_.addStateTransition('D', 'C', "Stop");
  fsm_.addStateTransition('D', 'H', "Halt");
  fsm_.addStateTransition('F', 'v', "Recover");
  fsm_.addStateTransition('v', 'v', "Recover");
  fsm_.addStateTransition('v', 'H', "RecoverDone");

  fsm_.addStateTransition('R', 'd', "DetectDegradation");
  fsm_.addStateTransition('d', 'C', "Stop");
  fsm_.addStateTransition('d', 'P', "Pause");
  fsm_.addStateTransition('d', 'D', "Done");
  fsm_.addStateTransition('d', 'F', "Failure");
  fsm_.addStateTransition('d', 'r', "DetectSoftError");

  //Adding Soft Error Detection Stuff

  fsm_.addStateTransition('R', 'r', "DetectSoftError");
  fsm_.addStateTransition('r', 'r', "DetectSoftError");
  fsm_.addStateTransition('R', 's', "FixSoftError");
  fsm_.addStateTransition('r', 's', "FixSoftError");
  fsm_.addStateTransition('s', 's', "FixSoftError");
  fsm_.addStateTransition('s', 'S', "FixingSoftErrorDone");
  fsm_.addStateTransition('r', 'C', "Stop");
  fsm_.addStateTransition('r', 'P', "Pause");
  fsm_.addStateTransition('S', 'R', "ResumeFromSoftError");

  //fireevent("fail") is supposed to work even if the transition is not defined
  //but it does not actually seem to work so we will define the transistions manually
  fsm_.addStateTransition('H','F',"Failure");
  fsm_.addStateTransition('c','F',"Failure");
  fsm_.addStateTransition('C','F',"Failure");
  fsm_.addStateTransition('R','F',"Failure");
  fsm_.addStateTransition('D','F',"Failure");
  fsm_.addStateTransition('P','F',"Failure");
  fsm_.addStateTransition('v','F',"Failure");
  fsm_.addStateTransition('T','F',"Failure");
  fsm_.addStateTransition('F','F',"Failure");

  //Adding Soft Error Detection Stuff

  fsm_.addStateTransition('r','F',"Failure");
  fsm_.addStateTransition('s','F',"Failure");
  fsm_.addStateTransition('S','F',"Failure");

  fsm_.setInitialState('I');
  fsm_.reset();

  posOutputDirs_=getenv("POS_OUTPUT_DIRS");
  console_=new std::stringstream();
  this->getApplicationDescriptor()->setAttribute("icon","pixel/PixelSupervisor/html/pixelsupervisor.gif");

  theGlobalKey_=0;
  theNameTranslation_=0;
  theTKFECConfiguration_=0;
  theFECConfiguration_=0;
  theFEDConfiguration_=0;
  theCalibObject_=0;
  theCalibAlgorithm_=0;
  theTTCciConfig_=0;

  calibWorkloop_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("WaitingWorkLoop", "waiting");
  calibJob_ = toolbox::task::bind(this, &PixelSupervisor::CalibRunning, "CalibRunning");
  jobcontrolWorkloop_ = 0;
  jobcontrolTask_ = 0;
  jobcontrolmon_=0;

  // Export a "State" variable that reflects the state of the state machine
  state_=fsm_.getStateName(fsm_.getCurrentState());
  // getApplicationInfoSpace()->fireItemAvailable("stateName", &state_);

  TTCRunning_=false;
  autoDone_=false;
  autoRefresh_=false;
  percentageConfigured_=0;
  runBeginCalibration_=false;
  runNumberFromLastFile_=false;

  //allow for a 'light' configuration while in configured state?
  string reconfflag = (getenv("RECONFIGURATIONFLAG")==0) ? "no" : getenv("RECONFIGURATIONFLAG");
  reconfigureActive_ = (reconfflag=="ALLOW") ? true : false;

  lastMessage_="";

  //Diag configure timer
  std::stringstream timerName;
  timerName << getApplicationDescriptor()->getContextDescriptor()->getURL() << ":";
  timerName << getApplicationDescriptor()->getClassName() << ":" << getApplicationDescriptor()->getLocalId() << ":" << getApplicationDescriptor()->getInstance();
  toolbox::task::Timer * timer = toolbox::task::getTimerFactory()->createTimer(timerName.str());
  toolbox::TimeInterval interval(AUTO_UP_CONFIGURE_DELAY,0);
  toolbox::TimeVal start;
  start = toolbox::TimeVal::gettimeofday() + interval;
  timer->schedule( this, start,  0, "" );

  // Check the infospace to see if run numbers should be booked from a database
  useRunInfo_=false;
  getApplicationInfoSpace()->fireItemAvailable("UseRunInfo", &useRunInfo_);
  getApplicationInfoSpace()->fireItemAvailable("DataBaseConnection", &dbConnection_);
  getApplicationInfoSpace()->fireItemAvailable("DataBaseUsername", &dbUsername_);
  getApplicationInfoSpace()->fireItemAvailable("RunSequence", &runSequence_);

  // Infospace for XDAQ to RCMS
  getApplicationInfoSpace()->fireItemAvailable("rcmsStateListener", rcmsStateNotifier_.getRcmsStateListenerParameter());
  getApplicationInfoSpace()->fireItemAvailable("foundRcmsStateListener", rcmsStateNotifier_.getFoundRcmsStateListenerParameter());
  configurationTimer_.setName("PixelSupervisorConfigurationTimer");

  // Check infospace for TCDS/TTC running
  useTCDS_=false;
  useTTC_=true;
  TTCSupervisorApplicationName_="ttc::TTCciControl"; // pixel::ici::PixeliCISupervisor
  if (useTCDS_) TTCSupervisorApplicationName_="pixel::ici::PixeliCISupervisor";
  LTCSupervisorApplicationName_="";
  if (useTCDS_) LTCSupervisorApplicationName_="pixel::tcds::PixelPISupervisor";
  getApplicationInfoSpace()->fireItemAvailable("UseTTC", &useTTC_);
  getApplicationInfoSpace()->fireItemAvailable("UseTCDS", &useTCDS_);
  getApplicationInfoSpace()->fireItemAvailable("TTCSupervisorApplicationName", &TTCSupervisorApplicationName_);
  getApplicationInfoSpace()->fireItemAvailable("LTCSupervisorApplicationName", &LTCSupervisorApplicationName_);

  //  assert(useTTC_ != useTCDS_);
}

//gio
void PixelSupervisor::timeExpired (toolbox::task::TimerEvent& e)
{
  DIAG_EXEC_FSM_INIT_TRANS
}

// DiagSystem XGI Binding
void PixelSupervisor::callDiagSystemPage(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  diagService_->getDiagSystemHtmlPage(in, out,getApplicationDescriptor()->getURN());
}


void PixelSupervisor::Default (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception) {

  autoDone_=false;

  std::string currentState=fsm_.getStateName(fsm_.getCurrentState());

  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  //*out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  *out<<"<head>"<<std::endl;
  if (autoRefresh_ || currentState == "Configuring" || currentState == "Running") {
    const int refresh_delay = currentState == "Running" ? 10 : 2;
    *out << " <meta HTTP-EQUIV=\"Refresh\" CONTENT=\"" << refresh_delay << "; URL=Default\"/>" <<endl;
  }
  *out<<"</head>"<<std::endl;
  xgi::Utils::getPageHeader(*out, "PixelSupervisor", fsm_.getStateName(fsm_.getCurrentState()));

  std::set<std::string> allInputs=fsm_.getInputs();
  std::set<std::string> clickableInputs=fsm_.getInputs(fsm_.getCurrentState());
  if ( theCalibObject_==0 ) clickableInputs.erase("Done");
  clickableInputs.erase("Failure");
  std::set<std::string>::iterator i;

  std::string url="/"+getApplicationDescriptor()->getURN();

  *out<<"<body>"<<std::endl;

  // Rendering State Machine GUI
  *out<<"  <h2>Pixel DAQ Finite State Machine</h2>"                                                                               <<std::endl;
  *out<<"  <h3>---SVN version---</h3>"                                                                                            <<std::endl;
  *out<<"  If in doubt, click <a href=\""<<url<<"\">here</a> to refresh"                                                          <<std::endl;
  *out<<"  <form name=\"input\" method=\"get\" action=\""<<url<<"/StateMachineXgiHandler"<<"\" enctype=\"multipart/form-data\">"  <<std::endl;
  if (autoRefresh_) {
    *out<<"    <p align=\"right\"><input type=\"submit\" name=\"StateInput\" value=\"AutoRefreshOFF\"/></p>"                      <<std::endl;
  } else {
    *out<<"    <p align=\"right\"><input type=\"submit\" name=\"StateInput\" value=\"AutoRefreshON\"/></p>"                       <<std::endl;
  }

  *out<<"  <table border=1 cellpadding=10 cellspacing=0>"                                                                         <<std::endl;
  *out<<"    <tr>"                                                                                                                <<std::endl;
  *out<<"      <td>"                                                                                                              <<std::endl;
  *out<<"        Current State: <font color=\""<<getHtmlColorFromState(currentState)<<"\"><b>"<<currentState<<"</b></font><br/>"        <<std::endl;
  if (theGlobalKey_!=0) {
   *out<<"        Run Type: "<<runType_<<"<br/>"                                                                                   <<std::endl;
   *out<<"        Configuration Key: "<<theGlobalKey_->key()<<"<br/>"                                                              <<std::endl;
  }
  *out<<"        Run Number: "<<runNumber_  <<"<br/>"                                                                                      <<std::endl;
  if (currentState=="Running" && theCalibAlgorithm_!=0) {
    double percentageOfJob=theCalibAlgorithm_->getPercentageOfJob();
    *out <<setprecision(2) << percentageOfJob<< std::setprecision(6) << "% complete" <<std::endl;
  }

  *out<<"<p>States of underlying supervisors:<br>"<<endl;
  if (useTCDS_){
    for ( SupervisorStates::const_iterator iSup=statePixelTTCSupervisors_.begin() ; iSup!=statePixelTTCSupervisors_.end() ; ++iSup)
      *out<<"PixeliCISupervisor: <font color=\""<<getHtmlColorFromState(iSup->second)<<"\"><b>"<<iSup->second<<"</b></font><br>"<<endl;
    for ( SupervisorStates::const_iterator iSup=statePixelLTCSupervisors_.begin() ; iSup!=statePixelLTCSupervisors_.end() ; ++iSup)
      *out<<"PixelPISupervisor: <font color=\""<<getHtmlColorFromState(iSup->second)<<"\"><b>"<<iSup->second<<"</b></font><br>"<<endl;
  }
  for ( SupervisorStates::const_iterator iSup=statePixelDCSFSMInterface_.begin() ; iSup!=statePixelDCSFSMInterface_.end() ; ++iSup)
    *out<<"PixelDCSFSMInterface: <font color=\""<<getHtmlColorFromState(iSup->second)<<"\"><b>"<<iSup->second<<"</b></font><br>"<<endl;
  for ( SupervisorStates::const_iterator iSup=statePixelTKFECSupervisors_.begin() ; iSup!=statePixelTKFECSupervisors_.end() ; ++iSup)
    *out<<"PixelTKFECSupervisor "<<iSup->first<<": <font color=\""<<getHtmlColorFromState(iSup->second)<<"\"><b>"<<iSup->second<<"</b></font><br>"<<endl;
  for ( SupervisorStates::const_iterator iSup=statePixelFECSupervisors_.begin() ; iSup!=statePixelFECSupervisors_.end() ; ++iSup)
    *out<<"PixelFECSupervisor "<<iSup->first<<": <font color=\""<<getHtmlColorFromState(iSup->second)<<"\"><b>"<<iSup->second<<"</b></font><br>"<<endl;
  for ( SupervisorStates::const_iterator iSup=statePixelFEDSupervisors_.begin() ; iSup!=statePixelFEDSupervisors_.end() ; ++iSup)
    *out<<"PixelFEDSupervisor "<<iSup->first<<": <font color=\""<<getHtmlColorFromState(iSup->second)<<"\"><b>"<<iSup->second<<"</b></font><br>"<<endl;

  *out<<"      </td>"                                                                                                             <<std::endl;
  *out<<"      <td colspan=5>"                                                                                                   <<std::endl;

  //display of status message received from SendStatus
  if (lastMessage_.size() !=0 ) {
    time_t msgtime=lastMessageTime_.tv_sec;
    tm *now=gmtime(&msgtime);
    char timestr[10];
    if (now!=NULL) sprintf(timestr,"%02d:%02d:%02d", now->tm_hour, now->tm_min,now->tm_sec);
    else sprintf(timestr,"**:**:**");
    *out<<"<input type=\"submit\" name=\"StateInput\" value=\"ClearStatus\"/> "                                                  <<std::endl
	<<" Last Status ["<<timestr<<"]: <br/>"<<lastMessage_<<"<br/>"                                                           <<std::endl;
  }

  if (jobcontrolWorkloop_ !=0 && jobcontrolmon_!=0) {
    *out<<jobcontrolmon_->getHtml()<<endl;
  }

  *out<<"      </td>"                                                                                                             <<std::endl;

  *out<<"      <td colspan=5>"                                                                                                   <<std::endl;

  //list of aliases for quick Reconfiguration for fine delay scan of globaldelay25
  if (reconfigureActive_ && (currentState=="Configured" || currentState=="Paused") && (theCalibObject_==0)) {

    for (vector <pair<string, unsigned int> >::iterator i_aliasesAndKeys=aliasesAndKeys_.begin();
         i_aliasesAndKeys!=aliasesAndKeys_.end(); ++i_aliasesAndKeys) {
      string alias=i_aliasesAndKeys->first;
      if (alias.substr(0,7) == "Physics" && isAliasOK(alias)) { //only display aliases that start with 'Physics'
        *out<<"        <input type=\"radio\" name=\"Alias\" value=\""<<alias<<"\">"<<alias<<"<br/>"                               <<endl;
      }
    }
    *out<<"        <input type=\"submit\" name=\"StateInput\" value=\"Reconfigure\"/>"                                           <<endl;
    *out<<"<br/>"<<endl;
  }

  if (currentState=="Halted") {

    for (std::vector <std::pair<std::string, unsigned int> >::iterator i_aliasesAndKeys=aliasesAndKeys_.begin();
         i_aliasesAndKeys!=aliasesAndKeys_.end(); ++i_aliasesAndKeys) {
      std::string alias=i_aliasesAndKeys->first;
      if ( (alias[0]!='T' || alias[1]!='T' || alias[2]!='C') && isAliasOK(alias)) {
        *out<<"        <input type=\"radio\" name=\"Alias\" value=\""<<alias<<"\">"<<alias<<"<br/>"                               <<std::endl;
      }
    }

  } else if (currentState=="Configuring") {

    *out<<"        "<<percentageConfigured_<<"% complete"                                                                         <<std::endl;

  } else if (currentState=="Configured") {

    if (useRunInfo_) {

      *out<<"      Run Number will be booked from the Run Info database. <br/>"<<std::endl;
      *out<<"      Please enter the "<<std::endl;
      *out<<"      database Login Name <input type=\"text\" name=\"dbLogin\"/>, <br/>"<<std::endl;
      *out<<"      and the corresponding password <input type=\"password\" name=\"password\"/>"<<std::endl;

    } else {

      *out<<"        Run Number<br/>"                                                                                               <<std::endl;
      runNumberFromLastFile_=true;
      ifstream fin((posOutputDirs_+"/LastRunNumber.txt").c_str());
      unsigned int lastRunNumber, newRunNumber;
      fin>>lastRunNumber;
      fin.close();
      newRunNumber=lastRunNumber+1;
      *out<<"        <input type=\"text\" name=\"RunNumber\" value=\""<<newRunNumber<<"\"/><br/>"                                   <<std::endl;

    }

  } else if (currentState=="Done") {

    if (updates_->nTypes()==0) {
      *out<<"        The current calibration:"<<theCalibObject_->mode()<<" did not produce any new calibration data"<<"<br/>"     <<std::endl;
    } else {
      *out<<"        The current calibration:"<<theCalibObject_->mode()<<" produced the following configuration data:"<<"<br/>"  <<std::endl;
      for(unsigned int i=0;i<updates_->nTypes();i++){
        std::string configType=updates_->type(i);
        *out<<"        Insert data for:"<<configType<<"<input type=\"radio\" name=\""
            <<configType<<"\" value=\"Yes\" />Yes, <input type=\"radio\" name=\""
            <<configType<<"\" value=\"No\" checked=\"checked\" /> No"<<"<br/>"                                                    <<std::endl;
        std::vector<std::string> aliases=updates_->aliases(i);
        if (aliases.size()!=0){
          *out <<"        Update the any of the following aliases:"<<"<br/>"                                                      <<std::endl;
          for(unsigned int j=0;j<aliases.size();j++){
          *out<<aliases[j]<<": <input type=\"radio\" name=\""<<configType+aliases[j]<<"\" value=\"Yes\" />Yes, <input type=\"radio\" name=\""
                          <<configType+aliases[j]<<"\" value=\"No\" checked=\"checked\" /> No"<<"<br/>"                           <<std::endl;
          }
        }
      }
    }

  } else if (currentState=="TTSTestMode") {

    *out<<"        FED Number to TTS Test: <input type=\"text\" name=\"TTS_TEST_FED_ID\"/><br/>"                                  <<std::endl;
    *out<<"        TTS Test Type (PATTERN/CYCLE): <input type=\"text\" name=\"TTS_TEST_TYPE\"/><br/>"                             <<std::endl;
    *out<<"        TTS Test Pattern (Integer 0-15): <input type=\"text\" name=\"TTS_TEST_PATTERN\"/><br/>"                        <<std::endl;
    *out<<"        TTS Test Cycles (Integer <8191): <input type=\"text\" name=\"TTS_TEST_CYCLES\"/><br/>"                         <<std::endl;

  }

  *out<<"      </td>"                                                                                                             <<std::endl;
  *out<<"    </tr>"                                                                                                               <<std::endl;

/*
  //display FSM transition buttons
  //unsigned int counter=1;
  *out<<"    <tr>"                                                                                                                <<std::endl;
  for (i=allInputs.begin();i!=allInputs.end();i++) {
    //if (counter%8==0) *out<<"   </tr> <tr>"                                                                                       <<std::endl;
    //counter++;
    *out<<"      <td>"                                                                                                            <<std::endl;
    if (clickableInputs.find(*i)!=clickableInputs.end()) {
      *out<<"        <input type=\"submit\" name=\"StateInput\" value=\""<<(*i)<<"\"/>"                                           <<std::endl;
    } else {
      *out<<"        <input type=\"submit\" disabled=\"true\" name=\"StateInput\" value=\""<<(*i)<<"\"/>"                         <<std::endl;
    }
    *out<<"      </td>"                                                                                                           <<std::endl;
  }

  *out<<"    </tr>"                                                                                                               <<std::endl;
*/

  //display FSM transition buttons (alternate)
  *out<<"    <tr>"                                                                                                                <<std::endl;
  for (i=clickableInputs.begin();i!=clickableInputs.end();i++) {
    *out<<"      <td>"                                                                                                            <<std::endl;
    *out<<"        <input type=\"submit\" name=\"StateInput\" value=\""<<(*i)<<"\"/>"                                           <<std::endl;
    *out<<"      </td>"                                                                                                           <<std::endl;
  }
  *out<<"    </tr>"                                                                                                               <<std::endl;
  ////////////////////////////////////////////

  *out<<"  </table>"                                                                                                              <<std::endl;
  *out<<"  </form>"                                                                                                               <<std::endl;
  *out<<"  <hr/>"                                                                                                                 <<std::endl;


  // DiagSystem GUI
  std::string urlDiag_ = "/"; \
  urlDiag_ += getApplicationDescriptor()->getURN(); \
  urlDiag_ += "/callDiagSystemPage"; \
  *out << "<h2> Error Dispatcher </h2> "<<std::endl;
  *out << "<a href=" << urlDiag_ << ">Configure DiagSystem</a>" <<std::endl;
  *out << " <hr/> " << std::endl;

  // Rendering Low Level GUI

  std::set<std::string> allLowLevelInputs;
  std::set<std::string> clickableLowLevelInputs;
  for (std::vector <std::pair<std::string, unsigned int> >::iterator i_aliasesAndKeys=aliasesAndKeys_.begin();
         i_aliasesAndKeys!=aliasesAndKeys_.end(); ++i_aliasesAndKeys) {
    std::string alias=i_aliasesAndKeys->first;
    if(alias[0]=='T' && alias[1]=='T' && alias[2]=='C') {
      std::string ttcalias = alias.substr(3);
      allLowLevelInputs.insert(ttcalias);
      if(!TTCRunning_) {
	clickableLowLevelInputs.insert(ttcalias);
      }
    }
  }
  if(!allLowLevelInputs.empty()) {
    allLowLevelInputs.insert("StopPeriodic");
  }
  if(TTCRunning_) {
    clickableLowLevelInputs.insert("StopPeriodic");
  }

  std::set<std::string>::iterator lli;

  *out<<"  <h2>Low Level Commands</h2>"                                                                                           <<std::endl;

  //reset buttons//////
  *out<<"    <tr>"                                                                                                                <<std::endl;
  std::set<std::string> resets;
  resets.insert("ResetTBM");
  resets.insert("ResetROC");
  resets.insert("ResetCCU");
  resets.insert("ResyncTCDS");
  resets.insert("HardResetTCDS");
  *out<<"  <form name=\"input\" method=\"get\" action=\""<<url<<"/LowLevelXgiHandler"<<"\" enctype=\"multipart/form-data\">"      <<std::endl;

  for( set<std::string>::const_iterator iter = resets.begin(); iter != resets.end(); ++iter ) {
    *out<<"      <td>"                                                                                                            <<std::endl;
    *out<<"<input type=\"submit\"";
    if (currentState=="Running" || currentState=="Initial" || currentState=="Configuring" ) *out<<" disabled=\"true\"";
    *out<<"name=\"LowLevelCommand\" value=\""<<*iter<<"\"/>"                                                                         <<std::endl;
    *out<<"      </td>"                                                                                                           <<std::endl;
  }
  *out<<"    </tr>"                                                                                                               <<std::endl;
  ///////////////////

  if (currentState=="Paused" || currentState=="Configured" || currentState=="Halted") {
    *out<<"  <br/>"                                                                                    <<std::endl;
    for (lli=allLowLevelInputs.begin();lli!=allLowLevelInputs.end();lli++) {
      if (clickableLowLevelInputs.find(*lli)!=clickableLowLevelInputs.end()) {
        *out<<"  <input type=\"submit\" name=\"LowLevelCommand\" value=\""<<(*lli)<<"\"/>"                                        <<std::endl;
      } else {
        *out<<"<input type=\"submit\" disabled=\"true\" name=\"LowLevelCommand\" value=\""<<(*lli)<<"\"/>"                        <<std::endl;
      }
    }
  }

  *out<<"  </form>"                                                                                                               <<std::endl;
  *out<<"  <hr/>"                                                                                                                 <<std::endl;

  // Rendering output in text box
  *out<<"  <h2> Console Output </h2>"                                                                                             <<std::endl;
  *out<<"  <textarea rows=\"10\" cols=\"70\" readonly>"                                                                           <<std::endl;
  *out<<(console_->str())                                                                                                         <<std::endl;

  *out<<"  </textarea>"                                                                                                           <<std::endl;

  // Render any Calibration GUI if running

  *out<<"</body>"                                                                                                                 <<std::endl;
  *out<<"</html>"                                                                                                                 <<std::endl;
}

//--------------------------------------- Experimental AJAX GUI ----------------------------------------------

void PixelSupervisor::Experimental (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{

  std::string url="/"+getApplicationDescriptor()->getURN();

  *out<<"<html>"                                                                                                                  <<std::endl;
  *out<<" <head>"                                                                                                                 <<std::endl;
  *out<<"  <script language=\"JavaScript\" src=\"../pixel/PixelSupervisor/html/Experimental.js\"></script>"                       <<std::endl;
  *out<<" </head>"                                                                                                                <<std::endl;
  *out<<" <body onload=\"onPageLoad('"<<url<<"')\">"                                                                              <<std::endl;
  *out<<"  <h1>PixelSupervisor</h1>"                                                                                              <<std::endl;
  *out<<"  <div id=\"part_FSM\">"                                                                                                 <<std::endl;
  *out<<"   FSM command <button onclick=\"fsmCommand('None')\">Attach</button>"                                                   <<std::endl;
  *out<<"  </div> "                                                                                                               <<std::endl;
  *out<<" </body> "                                                                                                               <<std::endl;
  *out<<"</html>"                                                                                                                 <<std::endl;

}

void PixelSupervisor::ExperimentalXgiHandler (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{

  // First deal with the incoming Xgi message in the HTTP request
  cgicc::Cgicc cgi(in);
  std::string fsmInput=cgi.getElement("FSMInput")->getValue(); // This should be in a try catch
  if (fsmInput=="Initialize") {
    xoap::MessageReference msg = MakeSOAPMessageReference("Initialize");
    xoap::MessageReference reply = Initialize(msg);
    if (Receive(reply)!="InitializeDone") *console_<<"All underlying Supervisors could not be initialized by browser button!"<<endl;
  } else if (fsmInput=="Configure") {
    Attribute_Vector parametersXgi(1);
    parametersXgi.at(0).name_="RUN_KEY";  parametersXgi.at(0).value_=cgi.getElement("Alias")->getValue();
    xoap::MessageReference msg = MakeSOAPMessageReference("Configure", parametersXgi);
    xoap::MessageReference reply = Configure(msg);
    if (Receive(reply)!="ConfigureDone") *console_<<"All underlying Supervisors could not be configured by browser button!"<<endl;
  } else if (fsmInput=="Start") {
    Attribute_Vector parametersXgi(1);
    parametersXgi.at(0).name_="RUN_NUMBER";
    parametersXgi.at(0).value_=cgi.getElement("RunNumber")->getValue();
    xoap::MessageReference msg = MakeSOAPMessageReference("Start", parametersXgi);
    xoap::MessageReference reply = Start(msg);
    if (Receive(reply)!="StartDone") *console_<<"All underlying Supervisors could not be started by browser button!"<<endl;
  } else if (fsmInput=="Stop") {
    xoap::MessageReference msg;
    if (state_=="Done") {
      vector<string> names;
      names=updates_->getNames();
      Attribute_Vector parametersXgi(names.size());
      for(unsigned int i=0;i<names.size();i++){
        parametersXgi.at(i).name_=names[i]; parametersXgi.at(i).value_=cgi.getElement(names[i])->getValue();
      }
      msg=MakeSOAPMessageReference("Stop", parametersXgi);
    } else {
      msg=MakeSOAPMessageReference("Stop");
    }
    xoap::MessageReference reply = Stop(msg);
    if (Receive(reply)!="StopDone") *console_<<"All underlying Supervisors could not be stopped by browser button!"<<endl;
  } else if (fsmInput=="Pause") {
    xoap::MessageReference msg = MakeSOAPMessageReference("Pause");
    xoap::MessageReference reply = Pause(msg);
    if (Receive(reply)!="PauseDone") *console_<<"All underlying Supervisors could not be paused by browser button!"<<endl;
  } else if (fsmInput=="Resume") {
    xoap::MessageReference msg = MakeSOAPMessageReference("Resume");
    xoap::MessageReference reply = Resume(msg);
    if (Receive(reply)!="ResumeDone") *console_<<"All underlying Supervisors could not be resumed by browser button!"<<endl;
  } else if (fsmInput=="Halt"&&(!autoDone_)) {
    xoap::MessageReference msg;
    if (state_=="Done") {
      vector<string> names;
      names=updates_->getNames();
      Attribute_Vector parametersXgi(names.size());
      for(unsigned int i=0;i<names.size();i++){
        parametersXgi.at(i).name_=names[i]; parametersXgi.at(i).value_=cgi.getElement(names[i])->getValue();
      }
      msg=MakeSOAPMessageReference("Halt", parametersXgi);
    } else {
      msg=MakeSOAPMessageReference("Halt");
    }
    xoap::MessageReference reply = Halt(msg);
    if (Receive(reply)!="HaltDone") *console_<<"All underlying Supervisors could not be halted by browser button!"<<endl;
  } else if (fsmInput=="Done") {
    xoap::MessageReference msg = MakeSOAPMessageReference("Done");
    xoap::MessageReference reply = Done(msg);
    if (Receive(reply)!="DoneDone") *console_<<"Calibration could not be finished!"<<endl;
  } else if (fsmInput=="PrepareTTSTestMode") {
    xoap::MessageReference msg = MakeSOAPMessageReference("TTSTestMode");
    xoap::MessageReference reply = PrepareTTSTestMode(msg);
    if (Receive(reply)!="PrepareTTSTestModeDone") *console_<<"All underlying PixelFEDSupervisors could not be prepared for TTS Test Mode by the browser button!"<<std::endl;
  } else if (fsmInput=="TestTTS") {
    Attribute_Vector parametersXgi(4);
    parametersXgi[0].name_="TTS_TEST_FED_ID";  parametersXgi[0].value_=cgi.getElement("TTS_TEST_FED_ID")->getValue();
    parametersXgi[1].name_="TTS_TEST_TYPE";    parametersXgi[1].value_=cgi.getElement("TTS_TEST_TYPE")->getValue();
    parametersXgi[2].name_="TTS_TEST_PATTERN"; parametersXgi[2].value_=cgi.getElement("TTS_TEST_PATTERN")->getValue();
    parametersXgi[3].name_="TTS_TEST_SEQUENCE_REPEAT";  parametersXgi[3].value_=cgi.getElement("TTS_TEST_CYCLES")->getValue();
    xoap::MessageReference msg = MakeSOAPMessageReference("TestTTS", parametersXgi);
    xoap::MessageReference reply = TestTTS(msg);
    if (Receive(reply)!="TestTTSDone") *console_<<"All underlying PixelFEDSupervisors could not be TTS Tested by the browser button!"<<std::endl;
  } else if (fsmInput=="DetectDegradation") {
    xoap::MessageReference msg = MakeSOAPMessageReference("DetectDegradation");
    xoap::MessageReference reply = DetectDegradation(msg);
    if (Receive(reply)!="DetectDegradationDone") *console_<<"Soft error could not be detected by browser button!"<<endl;
  } else if (fsmInput=="DetectSoftError") {
    xoap::MessageReference msg = MakeSOAPMessageReference("DetectSoftError");
    xoap::MessageReference reply = DetectSoftError(msg);
    if (Receive(reply)!="DetectSoftErrorDone") *console_<<"Soft error could not be detected by browser button!"<<endl;
  } else if (fsmInput=="FixSoftError") {
    xoap::MessageReference msg = MakeSOAPMessageReference("FixSoftError");
    xoap::MessageReference reply = FixSoftError(msg);
    if (Receive(reply)!="FixSoftErrorDone") *console_<<"Soft error could not fixed in all underlying Supervisors by browser button!"<<endl;
  }


  // Reply back with an updated FSM table in the HTTP response
  std::set<std::string> allInputs=fsm_.getInputs();
  std::set<std::string> clickableInputs=fsm_.getInputs(fsm_.getCurrentState());
  cgicc::HTTPResponseHeader response("HTTP/1.1", 200, "OK");
  response.addHeader("Content-Length", "100");
  response.addHeader("Content-Type", "text/html");
  out->setHTTPResponseHeader(response);
  std::string currentState=std::string(state_);
  *out<<std::endl;
  *out<<"The FSM State is "<<currentState<<std::endl;
  *out<<"<table border=1 cellpadding=10 cellspacing=0>"                                                                       <<std::endl;
  *out<<" <tr>"                                                                                                              <<std::endl;
  *out<<"  <td>"                                                                                                            <<std::endl;
  *out<<"   Current State: "<<currentState<<"<br/>"                                                                        <<std::endl;
  *out<<"   Run Type: "<<runType_<<"<br/>"                                                                                 <<std::endl;
  *out<<"   Run Number: "<<runNumber_                                                                                      <<std::endl;
  *out<<"  </td>"                                                                                                           <<std::endl;
  *out<<"  <td>"                                                                                                 <<std::endl;
  *out<<"   Parameters Here<br/>"<<std::endl;
  if (currentState=="Halted") {
    for (std::vector <std::pair<std::string, unsigned int> >::iterator i_aliasesAndKeys=aliasesAndKeys_.begin();
         i_aliasesAndKeys!=aliasesAndKeys_.end(); ++i_aliasesAndKeys) {
      std::string alias=i_aliasesAndKeys->first;
      if(alias[0]!='T' || alias[1]!='T' || alias[2]!='C') {
        *out<<"   <input type=\"radio\" name=\"Alias\" value=\""<<alias<<"\">"<<alias<<"<br/>"                               <<std::endl;
      }
    }
  } else if (currentState=="Configuring") {
    *out<<"   "<<percentageConfigured_<<"% complete"                                                                         <<std::endl;
  } else if (currentState=="Configured") {
    *out<<"   Run Number<br/>"                                                                                               <<std::endl;
    runNumberFromLastFile_=true;
    ifstream fin((posOutputDirs_+"/LastRunNumber.txt").c_str());
    unsigned int lastRunNumber, newRunNumber;
    fin>>lastRunNumber;
    fin.close();
    newRunNumber=lastRunNumber+1;
    *out<<"   <input type=\"text\" name=\"RunNumber\" value=\""<<newRunNumber<<"\"/><br/>"                                   <<std::endl;
  } else if (currentState=="Done") {

    if (updates_->nTypes()==0) {
      *out<<"   The current calibration:"<<theCalibObject_->mode()<<" did not produce any new calibration data"<<"<br/>"     <<std::endl;
    } else {
      *out<<"   The current calibration:"<<theCalibObject_->mode()<<" produced the following configuration data:"<<"<br/>"   <<std::endl;
      for(unsigned int i=0;i<updates_->nTypes();i++){
        std::string configType=updates_->type(i);
        *out<<"   Insert data for:"<<configType<<"<input type=\"radio\" name=\""
            <<configType<<"\" value=\"Yes\" />Yes, <input type=\"radio\" name=\""
            <<configType<<"\" value=\"No\" checked=\"checked\" /> No"<<"<br/>"                                               <<std::endl;
        std::vector<std::string> aliases=updates_->aliases(i);
        if (aliases.size()!=0){
          *out <<"   Update any of the following aliases:"<<"<br/>"                                                      <<std::endl;
          for(unsigned int j=0;j<aliases.size();j++){
            if(aliases[j] == "Default"){  // added by manuel
	      *out<<" Automatically saved in " << aliases[j] <<"<input type=\"hidden\" name=\"" <<(configType+aliases[j])<<"\" value=\"Yes\"/>"
		  <<"<br/>"     <<std::endl; } // manuel
	    else{ // manuel
	      *out<<aliases[j]<<": <input type=\"radio\" name=\""<<(configType+aliases[j])<<"\" value=\"Yes\" />Yes, <input type=\"radio\" name=\""
		  <<(configType+aliases[j])<<"\" value=\"No\" checked=\"checked\" /> No"<<"<br/>"                           <<std::endl;}
          }
        }
      }
    }
  } else if (currentState=="TTSTestMode") {
    *out<<"   FED Number to TTS Test: <input type=\"text\" name=\"TTS_TEST_FED_ID\"/><br/>"                                  <<std::endl;
    *out<<"   TTS Test Type (PATTERN/CYCLE): <input type=\"text\" name=\"TTS_TEST_TYPE\"/><br/>"                             <<std::endl;
    *out<<"   TTS Test Pattern (Integer 0-15): <input type=\"text\" name=\"TTS_TEST_PATTERN\"/><br/>"                        <<std::endl;
    *out<<"   TTS Test Cycles (Integer <8191): <input type=\"text\" name=\"TTS_TEST_CYCLES\"/><br/>"                         <<std::endl;
  }
  *out<<"  </td>"<<std::endl;
  *out<<" </tr>"<<std::endl;
  *out<<" <tr>"<<std::endl;
  *out<<"  <td colspan=2>"<<std::endl;
  *out<<"   Finite State Machine Commands here<br/>"<<std::endl;
  std::set<std::string>::iterator i;
  for (i=allInputs.begin();i!=allInputs.end();i++) {
    if (clickableInputs.find(*i)!=clickableInputs.end()) {
      *out<<"   <button onclick=\"fsmCommand('"<<(*i)<<"')\">"<<(*i)<<"</button>"<<std::endl;
    }
  }
  *out<<"  </td>"<<std::endl;
  *out<<" </tr>"<<std::endl;
  *out<<"</table>"<<std::endl;




}

//---------------------------------------------------------------------------------------------------------------

void PixelSupervisor::StateMachineXgiHandler (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception) {
  cgicc::Cgicc cgi(in);

  std::string Command=cgi.getElement("StateInput")->getValue();

  if (Command=="Initialize") {
    xoap::MessageReference msg = MakeSOAPMessageReference("Initialize");
    xoap::MessageReference reply = Initialize(msg);
    if (Receive(reply)!="InitializeDone") *console_<<"All underlying Supervisors could not be initialized by browser button!"<<endl;
  } else if (Command=="Configure") {
    Attribute_Vector parametersXgi(1);
    parametersXgi.at(0).name_="RUN_KEY";
    cgicc::const_form_iterator alias = cgi.getElement("Alias");
    //check that the user actually selected a radio button
    if(alias != (*cgi).end() && ! alias->isEmpty())  parametersXgi.at(0).value_=alias->getValue();
    else                                             parametersXgi.at(0).value_="InvalidInput";
    xoap::MessageReference msg = MakeSOAPMessageReference("Configure", parametersXgi);
    xoap::MessageReference reply = Configure(msg);
    if (Receive(reply)!="ConfigureDone") *console_<<"All underlying Supervisors could not be configured by browser button!"<<endl;
  } else if (Command=="Reconfigure") {
    Attribute_Vector parametersXgi(1);
    parametersXgi.at(0).name_="RUN_KEY";
    cgicc::const_form_iterator alias = cgi.getElement("Alias");
    //check that the user actually selected a radio button
    if(alias != (*cgi).end() && ! alias->isEmpty())  parametersXgi.at(0).value_=alias->getValue();
    else                                             parametersXgi.at(0).value_="InvalidInput";
    xoap::MessageReference msg = MakeSOAPMessageReference("Reconfigure", parametersXgi);
    xoap::MessageReference reply = Reconfigure(msg);
    if (Receive(reply)!="ReconfigureDone") *console_<<"All underlying Supervisors could not be reconfigured by browser button!"<<endl;
  } else if (Command=="Start") {

    Attribute_Vector parametersXgi(1);
    parametersXgi.at(0).name_="RUN_NUMBER";

    bool problem=false;
    if (useRunInfo_) {

      std::string dbLogin=cgi.getElement("dbLogin")->getValue();
      std::string password=cgi.getElement("password")->getValue();
      unsigned int runNumber=bookRunNumber(dbLogin, password);
      if (runNumber == 0) {
	diagService_->reportError("Problem booking run number. Check username and password!",DIAGERROR);
	problem=true;
      }
      std::string runNumber_string=itoa(runNumber);
      parametersXgi.at(0).value_=runNumber_string;
      writeRunInfo(dbLogin, password, runNumber_string, "RunAlias", runType_);
      writeRunInfo(dbLogin, password, runNumber_string, "ConfigurationKey", itoa(theGlobalKey_->key()));

    } else {

      parametersXgi.at(0).value_=cgi.getElement("RunNumber")->getValue();

    }

    if (!problem) {
      xoap::MessageReference msg = MakeSOAPMessageReference("Start", parametersXgi);
      xoap::MessageReference reply = Start(msg);
      if (Receive(reply)!="StartDone") *console_<<"All underlying Supervisors could not be started by browser button!"<<endl;
    }
  } else if (Command=="Stop") {
    xoap::MessageReference msg;
    if (state_=="Done") {
      vector<string> names;
      names=updates_->getNames();
      Attribute_Vector parametersXgi(names.size());
      for(unsigned int i=0;i<names.size();i++){
	parametersXgi.at(i).name_=names[i]; parametersXgi.at(i).value_=cgi.getElement(names[i])->getValue();
      }
      msg=MakeSOAPMessageReference("Stop", parametersXgi);
    } else {
      msg=MakeSOAPMessageReference("Stop");
    }
    xoap::MessageReference reply = Stop(msg);
    if (Receive(reply)!="StopDone") *console_<<"All underlying Supervisors could not be stopped by browser button!"<<endl;
  } else if (Command=="Pause") {
    xoap::MessageReference msg = MakeSOAPMessageReference("Pause");
    xoap::MessageReference reply = Pause(msg);
    if (Receive(reply)!="PauseDone") *console_<<"All underlying Supervisors could not be paused by browser button!"<<endl;
  } else if (Command=="Resume") {
    xoap::MessageReference msg = MakeSOAPMessageReference("Resume");
    xoap::MessageReference reply = Resume(msg);
    if (Receive(reply)!="ResumeDone") *console_<<"All underlying Supervisors could not be resumed by browser button!"<<endl;
  } else if (Command=="Recover") {
    xoap::MessageReference msg = MakeSOAPMessageReference("Recover");
    xoap::MessageReference reply = Recover(msg);
    if (Receive(reply)!="RecoverDone") *console_<<"All underlying Supervisors could not be recovered by browser button!"<<endl;
  } else if (Command=="Halt"&&(!autoDone_)) {
    xoap::MessageReference msg;
    if (state_=="Done") {
      vector<string> names;
      names=updates_->getNames();
      Attribute_Vector parametersXgi(names.size());
      for(unsigned int i=0;i<names.size();i++){
	parametersXgi.at(i).name_=names[i]; parametersXgi.at(i).value_=cgi.getElement(names[i])->getValue();
      }
      msg=MakeSOAPMessageReference("Halt", parametersXgi);

    } else {
      msg=MakeSOAPMessageReference("Halt");
    }
    xoap::MessageReference reply = Halt(msg);
    if (Receive(reply)!="HaltDone") *console_<<"All underlying Supervisors could not be halted by browser button!"<<endl;
  } else if (Command=="Done") {
    xoap::MessageReference msg = MakeSOAPMessageReference("Done");
    xoap::MessageReference reply = Done(msg);
    if (Receive(reply)!="DoneDone") *console_<<"Calibration could not be finished!"<<endl;
  } else if (Command=="DoneCheck") {
  // Respond to Http Request
    if (!calibWorkloop_->isActive()) {
      cgicc::HTTPResponseHeader response("HTTP/1.1", 200, "OK");
      response.addHeader("Content-Length", "100");
      response.addHeader("Content-Type", "text/html");
      response.addHeader("Change");
      out->setHTTPResponseHeader(response);
    }
  } else if (Command=="PrepareTTSTestMode") {
    xoap::MessageReference msg = MakeSOAPMessageReference("TTSTestMode");
    xoap::MessageReference reply = PrepareTTSTestMode(msg);
    if (Receive(reply)!="PrepareTTSTestModeDone") *console_<<"All underlying PixelFEDSupervisors could not be prepared for TTS Test Mode by the browser button!"<<std::endl;
  } else if (Command=="TestTTS") {
    Attribute_Vector parametersXgi(4);
    parametersXgi[0].name_="TTS_TEST_FED_ID";  parametersXgi[0].value_=cgi.getElement("TTS_TEST_FED_ID")->getValue();
    parametersXgi[1].name_="TTS_TEST_TYPE";    parametersXgi[1].value_=cgi.getElement("TTS_TEST_TYPE")->getValue();
    parametersXgi[2].name_="TTS_TEST_PATTERN"; parametersXgi[2].value_=cgi.getElement("TTS_TEST_PATTERN")->getValue();
    parametersXgi[3].name_="TTS_TEST_SEQUENCE_REPEAT";  parametersXgi[3].value_=cgi.getElement("TTS_TEST_CYCLES")->getValue();
    xoap::MessageReference msg = MakeSOAPMessageReference("TestTTS", parametersXgi);
    xoap::MessageReference reply = TestTTS(msg);
    if (Receive(reply)!="TestTTSDone") *console_<<"All underlying PixelFEDSupervisors could not be TTS Tested by the browser button!"<<std::endl;
  } else if (Command=="AutoRefreshON") {
    autoRefresh_=true;
  } else if (Command=="AutoRefreshOFF") {
    autoRefresh_=false;
  } else if (Command=="ClearAll") {
    ClearErrors("All");
  } else if (Command=="ClearStatus") {
    ClearErrors("Status");
  }
  // ADDED MKS for Cold reset use
  else if (Command=="ColdReset") {
   xoap::MessageReference msg = MakeSOAPMessageReference("ColdReset");
   xoap::MessageReference reply = ColdReset(msg);
   if (Receive(reply)!="ColdResetDone") *console_<<"problem!"<<endl;
  } else if (Command=="DetectDegradation") {
    xoap::MessageReference msg = MakeSOAPMessageReference("DetectDegradation");
    xoap::MessageReference reply = DetectDegradation(msg);
    if (Receive(reply)!="DetectDegradationDone") *console_<<"Soft error could not be detected by browser button!"<<endl;
  } else if (Command=="DetectSoftError") {
    xoap::MessageReference msg = MakeSOAPMessageReference("DetectSoftError");
    xoap::MessageReference reply = DetectSoftError(msg);
    if (Receive(reply)!="DetectSoftErrorDone") *console_<<"Soft error could not be detected by browser button!"<<endl;
  } else if (Command=="FixSoftError") {
    xoap::MessageReference msg = MakeSOAPMessageReference("FixSoftError");
    xoap::MessageReference reply = FixSoftError(msg);
    if (Receive(reply)!="FixSoftErrorDone") *console_<<"Soft error could not fixed in all underlying Supervisors by browser button!"<<endl;
  }

  this->Default(in, out);

}

void PixelSupervisor::LowLevelXgiHandler (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception) {

  cgicc::Cgicc cgi(in);
  std::string input = cgi.getElement("LowLevelCommand")->getValue();

  if (input=="ResetTBM") {
    xoap::MessageReference msg = MakeSOAPMessageReference("ResetTBM");
    xoap::MessageReference reply = ResetTBM(msg);
    if (Receive(reply)!="ResetTBMDone") *console_<<"ResetTBM by browser button failed!"<<std::endl;
  } else if (input=="ResetROC") {
    xoap::MessageReference msg = MakeSOAPMessageReference("ResetROC");
    xoap::MessageReference reply = ResetROC(msg);
    if (Receive(reply)!="ResetROCDone") *console_<<"ResetROC by browser button failed!"<<std::endl;
  } else if (input=="ResetCCU") {
    xoap::MessageReference msg = MakeSOAPMessageReference("ResetCCU");
    xoap::MessageReference reply = ResetCCU(msg);
    if (Receive(reply)!="ResetCCUDone") *console_<<"ResetCCU by browser button failed!"<<std::endl;
  }
  //  else if (input=="ResyncTCDS") {
  //    *console_<<"Calling tcdsTTCResync"<<std::endl;
  //    // tcdsTTCResync();
  //  } else if (input=="HardResetTCDS") {
  //    *console_<<"Calling tcdsTTCHardReset"<<std::endl;
  //    // tcdsTTCHardReset();
  //  }

  else {

    if (useTTC_) {
      Supervisors::iterator i_PixelTTCSupervisor;
      for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor) {
        std::string reply = Send(i_PixelTTCSupervisor->second, input);
        if (reply!= input+"Response") {
          diagService_->reportError("PixelTTCSupervisor supervising crate #"+stringF(i_PixelTTCSupervisor->first) + " could not be execute "+input+" command.",DIAGERROR);
          *console_<<"PixelTTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first)<<" could not execute "<<input<<"command."<<std::endl;
        }
      }

      if(input=="StopPeriodic") {
        TTCRunning_=false;
      } else {
        TTCRunning_=true;
      }
    }

  }

  this->Default(in, out);
}

string PixelSupervisor::getHtmlColorFromState( const string & state ) {

  if (state == "Configuring") return "#FF00FF";
  else if (state == "Configured") return "#7FFF00";
  else if (state == "Running") return "GREEN";
  else if (state == "Enabled") return "GREEN";
  else if (state == "Done") return "#006400";
  else if (state == "Paused") return "#1E90FF";
  else if (state == "Recovering") return "#FFA500";
  else if (state == "Error") return "RED";

  return "BLACK";
}

xoap::MessageReference PixelSupervisor::Initialize (xoap::MessageReference msg) throw (xoap::exception::Exception)
{

  diagService_->reportError("Entered SOAP message callback method PixelSupervisor::Initialize", DIAGINFO);
  *console_<<"Entered SOAP message callback method PixelSupervisor::Initialize"<<std::endl;

  if (useTCDS_)
    {
      TCDSSessionID_=toolbox::toString("#%d",rand());
      diagService_->reportError("Created TCDS session ID: "+TCDSSessionID_, DIAGINFO);
      *console_<<"Created TCDS session ID: " + TCDSSessionID_<<std::endl;
      std::cout <<"Created TCDS session ID: " + TCDSSessionID_<<std::endl;
    }

  fillBadAliasList(); //method of PixelSupervisorConfiguration, to be run once per POS session

  // Get all TTCciControls/PixelTTCSupervisors in the "daq" group.

  try {
    std::set<xdaq::ApplicationDescriptor*> set_PixelTTCSupervisor = getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptors(TTCSupervisorApplicationName_);
    for (std::set<xdaq::ApplicationDescriptor*>::iterator i_set_PixelTTCSupervisor=set_PixelTTCSupervisor.begin();
         i_set_PixelTTCSupervisor!=set_PixelTTCSupervisor.end();
         ++i_set_PixelTTCSupervisor) {
      PixelTTCSupervisors_.insert(make_pair((*i_set_PixelTTCSupervisor)->getInstance(), *(i_set_PixelTTCSupervisor)));
    }
    diagService_->reportError(stringF(PixelTTCSupervisors_.size()) + " TTCSupervisor(s) of type " + TTCSupervisorApplicationName_.toString() + " found in the \"daq\" group in the Configuration XML file", DIAGINFO);
    *console_<<PixelTTCSupervisors_.size()<<" TTCSupervisor(s) of type " + TTCSupervisorApplicationName_.toString() + " found in the \"daq\" group in the Configuration XML file"<<std::endl;
  } catch (xdaq::exception::Exception& e) {
    diagService_->reportError("No TTCSupervisor of type " + TTCSupervisorApplicationName_.toString() + " found in the \"daq\" group in the Configuration XML file", DIAGERROR);
  }

  if (useTCDS_) {

    try {
      std::set<xdaq::ApplicationDescriptor*> set_PixelLTCSupervisor = getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptors(LTCSupervisorApplicationName_);
      for (std::set<xdaq::ApplicationDescriptor*>::iterator i_set_PixelLTCSupervisor=set_PixelLTCSupervisor.begin();
           i_set_PixelLTCSupervisor!=set_PixelLTCSupervisor.end();
           ++i_set_PixelLTCSupervisor) {
        PixelLTCSupervisors_.insert(make_pair((*i_set_PixelLTCSupervisor)->getInstance(), *(i_set_PixelLTCSupervisor)));
      }
      diagService_->reportError(stringF(PixelLTCSupervisors_.size()) + " LTCSupervisor(s) of type " + LTCSupervisorApplicationName_.toString() + " found in the \"daq\" group in the Configuration XML file", DIAGINFO);
      *console_<<PixelLTCSupervisors_.size()<<" LTCSupervisor(s) of type " + LTCSupervisorApplicationName_.toString() + " found in the \"daq\" group in the Configuration XML file"<<std::endl;
    } catch (xdaq::exception::Exception& e) {
      diagService_->reportError("No LTCSupervisor of type " + LTCSupervisorApplicationName_.toString() + " found in the \"daq\" group in the Configuration XML file", DIAGERROR);
    }


    try
      {
        std::set<xdaq::ApplicationDescriptor*> set_PixelTTCControllers = getApplicationContext()->getDefaultZone()->getApplicationGroup("tcds")->getApplicationDescriptors("tcds::ici::ICIController");
        for (std::set<xdaq::ApplicationDescriptor*>::iterator i_set_PixelTTCController=set_PixelTTCControllers.begin();
             i_set_PixelTTCController!=set_PixelTTCControllers.end();
             ++i_set_PixelTTCController)
          {
            PixelTTCControllers_.insert(make_pair((*i_set_PixelTTCController)->getInstance(), *(i_set_PixelTTCController)));
          }
        diagService_->reportError(stringF(PixelTTCControllers_.size()) + " TTCController(s) of type tcds::ici::ICIController found in the \"tcds\" group in the Configuration XML file", DIAGINFO);
        *console_<<PixelTTCControllers_.size()<<" TTCController(s) of type tcds::ici::ICIController found in the \"tcds\" group in the Configuration XML file"<<std::endl;
      }
    catch (xdaq::exception::Exception& e)
      {
        diagService_->reportError("No TTCController found in the \"tcds\" group in the Configuration XML file.", DIAGERROR);
      }



    // handshake with ICISupervisors and status reset to Initial
    try
      {
        Supervisors::iterator i_PixelTTCSupervisor;
        for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor)
          {

            Attribute_Vector parameters(1);
            parameters[0].name_="TCDSSessionID";
            parameters[0].value_=TCDSSessionID_;
            string reply = Send(i_PixelTTCSupervisor->second, "Handshake", parameters);
            diagService_->reportError("TTCSupervisor #" + stringF(i_PixelTTCSupervisor->first) + ": Sending Handshake reply: " + reply, DIAGINFO);
            *console_<< "TTCSupervisor #" << (i_PixelTTCSupervisor->first) << ": Sending Handshake reply: " << reply <<std::endl;
            if (reply!= "HandshakeResponse")
              {
                diagService_->reportError("TTCSupervisor supervising crate #"+stringF(i_PixelTTCSupervisor->first) + " could not receive TCDS session ID.",DIAGERROR);
                *console_<<"TTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first)<<" could not receive TCDS session ID for initialisation step: "<<reply<<std::endl;
              }
          }

        for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor)
          {
            string reply = Send(i_PixelTTCSupervisor->second, "Initialize");
            diagService_->reportError("TTCSupervisor #" + stringF(i_PixelTTCSupervisor->first) + ": Sending Initialize reply: " + reply, DIAGINFO);
            *console_<<"TTCSupervisor #" << (i_PixelTTCSupervisor->first) << ": Sending Initialize reply: " << reply <<std::endl;
            if (reply!= "InitializeResponse")
              {
                diagService_->reportError("TTCSupervisor supervising crate #"+stringF(i_PixelTTCSupervisor->first) + " could not be halted for initialisation step.",DIAGERROR);
                *console_<<"TTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first)<<" could not be halted for initialisation step: "<<reply<<std::endl;
              }
          }

        for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor)
          {
            std::string fsmState=Send(i_PixelTTCSupervisor->second, "QueryFSMState");
            statePixelTTCSupervisors_.insert(make_pair(i_PixelTTCSupervisor->first, fsmState));
          }
      }
    catch( xcept::Exception & e )
      {
        diagService_->reportError("Failed to Halt TTCSupervisor with exception: "+string(e.what()),DIAGERROR);
      }

    // handshake with PISupervisors and status reset to Initial
    try
      {
        Supervisors::iterator i_PixelLTCSupervisor;
        for (i_PixelLTCSupervisor=PixelLTCSupervisors_.begin();i_PixelLTCSupervisor!=PixelLTCSupervisors_.end();++i_PixelLTCSupervisor)
          {

            Attribute_Vector parameters(1);
            parameters[0].name_="TCDSSessionID";
            parameters[0].value_=TCDSSessionID_;
            string reply = Send(i_PixelLTCSupervisor->second, "Handshake", parameters);
            diagService_->reportError("LTCSupervisor #" + stringF(i_PixelLTCSupervisor->first) + ": Sending Handshake reply: " + reply, DIAGINFO);
            *console_<< "LTCSupervisor #" << (i_PixelLTCSupervisor->first) << ": Sending Handshake reply: " << reply <<std::endl;
            if (reply!= "HandshakeResponse")
              {
                diagService_->reportError("LTCSupervisor supervising crate #"+stringF(i_PixelLTCSupervisor->first) + " could not receive TCDS session ID.",DIAGERROR);
                *console_<<"LTCSupervisor supervising crate #"<<(i_PixelLTCSupervisor->first)<<" could not receive TCDS session ID for initialisation step: "<<reply<<std::endl;
              }
          }

        for (i_PixelLTCSupervisor=PixelLTCSupervisors_.begin();i_PixelLTCSupervisor!=PixelLTCSupervisors_.end();++i_PixelLTCSupervisor)
          {
            string reply = Send(i_PixelLTCSupervisor->second, "Initialize");
            diagService_->reportError("LTCSupervisor #" + stringF(i_PixelLTCSupervisor->first) + ": Sending Initialize reply: " + reply, DIAGINFO);
            *console_<<"LTCSupervisor #" << (i_PixelLTCSupervisor->first) << ": Sending Initialize reply: " << reply <<std::endl;
            if (reply!= "InitializeResponse")
              {
                diagService_->reportError("LTCSupervisor supervising crate #"+stringF(i_PixelLTCSupervisor->first) + " could not be halted for initialisation step.",DIAGERROR);
                *console_<<"LTCSupervisor supervising crate #"<<(i_PixelLTCSupervisor->first)<<" could not be halted for initialisation step: "<<reply<<std::endl;
              }
          }

        for (i_PixelLTCSupervisor=PixelLTCSupervisors_.begin();i_PixelLTCSupervisor!=PixelLTCSupervisors_.end();++i_PixelLTCSupervisor)
          {
            std::string fsmState=Send(i_PixelLTCSupervisor->second, "QueryFSMState");
            statePixelLTCSupervisors_.insert(make_pair(i_PixelLTCSupervisor->first, fsmState));
          }
      }
    catch( xcept::Exception & e )
      {
        diagService_->reportError("Failed to Halt LTCSupervisor with exception: "+string(e.what()),DIAGERROR);
      }


  }

  // Get all PixelFECSupervisors in the "daq" group mentioned in the Configuration XML file, and
  // Try to handshake with them by asking for their FSM state.
  try {
    std::set<xdaq::ApplicationDescriptor*> set_PixelFECSupervisors = getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptors("PixelFECSupervisor");

    for (std::set<xdaq::ApplicationDescriptor*>::iterator i_set_PixelFECSupervisor=set_PixelFECSupervisors.begin(); i_set_PixelFECSupervisor!=set_PixelFECSupervisors.end(); ++i_set_PixelFECSupervisor) {
      try {
        std::string fsmState=Send(*i_set_PixelFECSupervisor, "FSMStateRequest");
        PixelFECSupervisors_.insert(make_pair((*i_set_PixelFECSupervisor)->getInstance(), *(i_set_PixelFECSupervisor)));
        statePixelFECSupervisors_.insert(make_pair((*i_set_PixelFECSupervisor)->getInstance(), fsmState));
        diagService_->reportError("PixelFECSupervisor instance "+stringF((*i_set_PixelFECSupervisor)->getInstance())+" is in FSM state "+fsmState, DIAGDEBUG);
        *console_<<"PixelFECSupervisor instance "<<(*i_set_PixelFECSupervisor)->getInstance()<<" is in FSM state "<<fsmState<<std::endl;
      } catch (xdaq::exception::Exception& e) {
        diagService_->reportError("PixelFECSupervisor instance "+stringF((*i_set_PixelFECSupervisor)->getInstance())+" could not report its FSM state", DIAGERROR);
      }
    }
  } catch (xdaq::exception::Exception& e) {
    diagService_->reportError("No PixelFECSupervisor found in the \"daq\" group in the Configuration XML file.", DIAGERROR);
  }
  //////Testing XaaS////////////////////////
  //  try
  //    {
  //      std::string const msg = "This is an xdaq::exception::ApplicationInstantiationFailed test message send from the PixelSupervisor";
  //      XCEPT_RAISE(xdaq::exception::ApplicationInstantiationFailed, msg.c_str());
  //    }
  //  catch (xdaq::exception::ApplicationInstantiationFailed& exceptionObj)
  //    {
  //      std::stringstream msg;
  //      msg << "This is a XaaS test message send from the PixelSupervisor";
  //      XCEPT_DECLARE_NESTED(xdaq::exception::ApplicationInstantiationFailed, f, msg.str(),exceptionObj);
  //      //XCEPT_DECLARE(xmas::sensor::exception::Exception, exceptionObj, msg.str());
  //      this->notifyQualified("warning", f);
  //      std::cout << "caught an exception" << std::endl;
  //      //return;
  //    }
  /////////////////////////////////////////////

  // Get all PixelFEDSupervisors in the "daq" group mentioned in the Configuration XML file, and
  // Try to handshake with them by asking for their FSM state.
  try {
    std::set<xdaq::ApplicationDescriptor*> set_PixelFEDSupervisors = getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptors("PixelFEDSupervisor");
    for (std::set<xdaq::ApplicationDescriptor*>::iterator i_set_PixelFEDSupervisor=set_PixelFEDSupervisors.begin(); i_set_PixelFEDSupervisor!=set_PixelFEDSupervisors.end(); ++i_set_PixelFEDSupervisor) {
      try {
        std::string fsmState=Send(*i_set_PixelFEDSupervisor, "FSMStateRequest");
        PixelFEDSupervisors_.insert(make_pair((*i_set_PixelFEDSupervisor)->getInstance(), *(i_set_PixelFEDSupervisor)));
        statePixelFEDSupervisors_.insert(make_pair((*i_set_PixelFEDSupervisor)->getInstance(), fsmState));
        diagService_->reportError("PixelFEDSupervisor instance "+stringF((*i_set_PixelFEDSupervisor)->getInstance())+" is in FSM state "+fsmState, DIAGDEBUG);
        *console_<<"PixelFEDSupervisor instance "<<(*i_set_PixelFEDSupervisor)->getInstance()<<" is in FSM state "<<fsmState<<std::endl;
      } catch (xdaq::exception::Exception& e) {
        diagService_->reportError("PixelFEDSupervisor instance "+stringF((*i_set_PixelFEDSupervisor)->getInstance())+" could not report its FSM state", DIAGERROR);
      }
    }
  } catch (xdaq::exception::Exception& e) {
    diagService_->reportError("No PixelFEDSupervisor found in the \"daq\" group in the Configuration XML file.", DIAGERROR);
  }

  // Get all PixelTKFECSupervisors in the "daq" group mentioned in the Configuration XML file, and
  // Try to handshake with them by asking for their FSM state.
  try {
    std::set<xdaq::ApplicationDescriptor*> set_PixelTKFECSupervisors = getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptors("PixelTKFECSupervisor");
    for (std::set<xdaq::ApplicationDescriptor*>::iterator i_set_PixelTKFECSupervisor=set_PixelTKFECSupervisors.begin(); i_set_PixelTKFECSupervisor!=set_PixelTKFECSupervisors.end(); ++i_set_PixelTKFECSupervisor) {
      try {
        std::string fsmState=Send(*i_set_PixelTKFECSupervisor, "FSMStateRequest");
        PixelTKFECSupervisors_.insert(make_pair((*i_set_PixelTKFECSupervisor)->getInstance(), *(i_set_PixelTKFECSupervisor)));
        statePixelTKFECSupervisors_.insert(make_pair((*i_set_PixelTKFECSupervisor)->getInstance(), fsmState));
        diagService_->reportError("PixelTKFECSupervisor instance "+stringF((*i_set_PixelTKFECSupervisor)->getInstance())+" is in FSM state "+fsmState, DIAGDEBUG);
	cout<<"[PixelSupervisor::Initialize] PixelTKFECSupervisor "<<(*i_set_PixelTKFECSupervisor)->getInstance() <<" has FSM state = "<<fsmState<<endl;
        *console_<<"PixelTKFECSupervisor instance "<<(*i_set_PixelTKFECSupervisor)->getInstance()<<" is in FSM state "<<fsmState<<std::endl;
      } catch (xdaq::exception::Exception& e) {
        diagService_->reportError("PixelTKFECSupervisor instance "+stringF((*i_set_PixelTKFECSupervisor)->getInstance())+" could not report its FSM state", DIAGERROR);
      }
    }
  } catch (xdaq::exception::Exception& e) {
    diagService_->reportError("No PixelTKFECSupervisor found in the \"daq\" group in the Configuration XML file.", DIAGERROR);
  }

  //DCS FSMStateRequest
  try {
    std::set<xdaq::ApplicationDescriptor*> set_PixelDCSFSMInterface = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptors("PixelDCSFSMInterface");
    cout<<"[PixelSupervisor::Initialize] found "<< set_PixelDCSFSMInterface.size() <<" copy of PixelDCSFSMInterface"<<endl; //debug
    for (std::set<xdaq::ApplicationDescriptor*>::iterator i_set_PixelDCSFSMInterface=set_PixelDCSFSMInterface.begin();
	 i_set_PixelDCSFSMInterface!=set_PixelDCSFSMInterface.end(); ++i_set_PixelDCSFSMInterface) {

      std::string fsmState=Send(*i_set_PixelDCSFSMInterface, "FSMStateRequest");
      cout<<"[PixelSupervisor::Initialize] PixelDCSFSMInterface has FSM state = "<<fsmState<<endl;
      PixelDCSFSMInterface_.insert(make_pair((*i_set_PixelDCSFSMInterface)->getInstance(), *(i_set_PixelDCSFSMInterface)));
      statePixelDCSFSMInterface_.insert(make_pair((*i_set_PixelDCSFSMInterface)->getInstance(), fsmState));
      if (PixelDCSFSMInterface_.size() >1) diagService_->reportError("[PixelSupervisor::Initialize] There is more than one PixelDCSFSMInterface!",DIAGERROR);
    }
  } catch (xdaq::exception::Exception& e) {
    diagService_->reportError("[PixelSupervisor::Initialize] PixelDCSFSMInterface not found in the XML file.", DIAGWARN);
  }


  // Get all psxServers in the "dcs" group
  try {
    std::set<xdaq::ApplicationDescriptor*> set_psxServers = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptors("psx");
    for (std::set<xdaq::ApplicationDescriptor*>::iterator i_set_psxServer=set_psxServers.begin();
	 i_set_psxServer!=set_psxServers.end();
	 ++i_set_psxServer) {
      psxServers_.insert(make_pair((*i_set_psxServer)->getInstance(), *(i_set_psxServer)));
    }

    diagService_->reportError(stringF(psxServers_.size()) + " psxServer(s) found in the \"daq\" found.",DIAGINFO);
    *console_<<psxServers_.size()<<" psxServers(s) found in the \"dcs\" group."<<std::endl;

  } catch (xdaq::exception::Exception& e) {
    diagService_->reportError("No psxServers(s) found in the \"dcs\" group.",DIAGWARN);
    *console_<<"No psxServer(s) found in the \"dcs\" group."<<std::endl;
  }

  // Get all PixelDCStoTrkFECDpInterfaces
  try {
    std::set<xdaq::ApplicationDescriptor*> set_PixelDCStoTrkFECDpInterface = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptors("PixelDCStoTrkFECDpInterface");
    for (std::set<xdaq::ApplicationDescriptor*>::iterator i_set_PixelDCStoTrkFECDpInterface=set_PixelDCStoTrkFECDpInterface.begin();
          i_set_PixelDCStoTrkFECDpInterface!=set_PixelDCStoTrkFECDpInterface.end();
          ++i_set_PixelDCStoTrkFECDpInterface) {
            PixelDCStoTrkFECDpInterface_.insert(make_pair((*i_set_PixelDCStoTrkFECDpInterface)->getInstance(), *(i_set_PixelDCStoTrkFECDpInterface)));
	    //unlike for other supervisors, the FSM state is hard coded here
	    statePixelDCStoTrkFECDpInterface_.insert(make_pair((*i_set_PixelDCStoTrkFECDpInterface)->getInstance(), "Halted"));
    }

    diagService_->reportError(" PixelDCStoTrkFECDpInterface(s) have been found in the \"dcs\" group.",DIAGINFO);
    *console_<<PixelDCStoTrkFECDpInterface_.size()<<" PixelDCStoTrkFECDpInterface(s) have been found in the \"dcs\" group."<<std::endl;
  } catch (xdaq::exception::Exception& e) {
    diagService_->reportError("No PixelDCStoTrkFECDpInterface(s) have been found in the \"dcs\" group.",DIAGWARN);
    *console_<<"No PixelDCStoTrkFECDpInterface(s) have been found in the \"dcs\" group."<<std::endl;
  }


  // Get all PixelSlinkMonitors in the "daq" group
  try {
    std::set<xdaq::ApplicationDescriptor*> set_PixelSlinkMonitors = getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptors("PixelSlinkMonitor");
    for (std::set<xdaq::ApplicationDescriptor*>::iterator i_set_PixelSlinkMonitor=set_PixelSlinkMonitors.begin();
	 i_set_PixelSlinkMonitor!=set_PixelSlinkMonitors.end();
	 ++i_set_PixelSlinkMonitor) {
      PixelSlinkMonitors_.insert(make_pair((*i_set_PixelSlinkMonitor)->getInstance(), *(i_set_PixelSlinkMonitor)));
    }
    diagService_->reportError(stringF(PixelSlinkMonitors_.size()) + " PixelSlinkMonitor(s) found in the \"daq\" group.",DIAGDEBUG);
    *console_<<PixelSlinkMonitors_.size()<<" PixelSlinkMonitor(s) found in the \"daq\" group."<<std::endl;

  } catch (xdaq::exception::Exception& e) {
    diagService_->reportError("No PixelSlinkMonitor(s) found in the \"daq\" group.",DIAGDEBUG);
    *console_<<"No PixelSlinkMonitor(s) found in the \"daq\" group."<<std::endl;
  }


  //start JobControl monitoring
  try {
    set<xdaq::ApplicationDescriptor*> jclist = getApplicationContext()->getDefaultZone()->getApplicationGroup("jc")->getApplicationDescriptors("jobcontrol");
    if (jclist.size() >0) {
      jobcontrolWorkloop_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("jobcontrolWorkloop", "waiting");
      jobcontrolTask_ = toolbox::task::bind(this, &PixelSupervisor::jobcontrol_workloop, "jobcontrol_workloop");
      jobcontrolWorkloop_->submit(jobcontrolTask_);
      jobcontrolWorkloop_->activate();
      cout<<"Job Control Workloop activated!"<<endl;
    }
  } catch (xcept::Exception & e) {
    //mostly this will happen when the jc group does not exist
    diagService_->reportError("Failed to start Job Control monitoring. Exception: "+string(e.what()),DIAGWARN);
    jobcontrolWorkloop_ =0; //use this as a key to whether we are doing this monitoring
  }

  // Initialize all PixelFEDSupervisors
  Supervisors::iterator i_PixelFEDSupervisor;
  for (i_PixelFEDSupervisor=PixelFEDSupervisors_.begin();i_PixelFEDSupervisor!=PixelFEDSupervisors_.end();++i_PixelFEDSupervisor) {
    std::string reply = Send(i_PixelFEDSupervisor->second, "Initialize");
    if (reply!="InitializeDone") {
      diagService_->reportError("PixelFEDSupervisor supervising crate "+stringF(i_PixelFEDSupervisor->first) + " could not be initialized!",DIAGFATAL);
      *console_<<"PixelFEDSupervisor supervising crate "<<(i_PixelFEDSupervisor->first)<<" could not be initialized!"<<std::endl;
    }
  }

  // Initialize all PixelSlinkMonitors
  if (!PixelSlinkMonitors_.empty()) {
    Supervisors::iterator i_PixelSlinkMonitor;
    for (i_PixelSlinkMonitor=PixelSlinkMonitors_.begin();i_PixelSlinkMonitor!=PixelSlinkMonitors_.end();++i_PixelSlinkMonitor) {
      std::string reply = Send(i_PixelSlinkMonitor->second, "Initialize");
      if (reply!="InitializeDone") {
	diagService_->reportError("PixelSlinkMonitor #"+stringF(i_PixelSlinkMonitor->first)+" could not be initialized!", DIAGERROR);
	*console_<<"PixelSlinkMonitor #"<<(i_PixelSlinkMonitor->first)<<" could not be initialized!"<<std::endl;
      }
    }
  }

  // Initialize all PixelTKFECSupervisors
  Supervisors::iterator i_PixelTKFECSupervisor;
  for (i_PixelTKFECSupervisor=PixelTKFECSupervisors_.begin();i_PixelTKFECSupervisor!=PixelTKFECSupervisors_.end();++i_PixelTKFECSupervisor) {
    std::string reply = Send(i_PixelTKFECSupervisor->second, "Initialize");
    if (reply!="InitializeDone") {
      diagService_->reportError("PixelTKFECSupervisor supervising crate "+stringF(i_PixelTKFECSupervisor->first) + " could not be initialized!",DIAGERROR);
      *console_<<"PixelTKFECSupervisor supervising crate "<<(i_PixelTKFECSupervisor->first)<<" could not be initialized!"<<std::endl;
    }
  }

  // Initialize all PixelFECSupervisors
  Supervisors::iterator i_PixelFECSupervisor;
  for (i_PixelFECSupervisor=PixelFECSupervisors_.begin();i_PixelFECSupervisor!=PixelFECSupervisors_.end();++i_PixelFECSupervisor) {
    std::string reply = Send(i_PixelFECSupervisor->second, "Initialize");
    if (reply!="InitializeDone") {
      diagService_->reportError("PixelFECSupervisor supervising crate "+stringF(i_PixelFECSupervisor->first) + " could not be initialized!",DIAGFATAL);
      *console_<<"PixelFECSupervisor supervising crate "<<(i_PixelFECSupervisor->first)<<" could not be initialized!"<<std::endl;
    }
  }

  // Initialize all PixelDCSFSMInterfaces
  Supervisors::iterator i_PixelDCSFSMInterface;
  for (i_PixelDCSFSMInterface=PixelDCSFSMInterface_.begin();i_PixelDCSFSMInterface!=PixelDCSFSMInterface_.end();++i_PixelDCSFSMInterface) {
    std::string reply = Send(i_PixelDCSFSMInterface->second, "Initialize");
    if (reply!="InitializeDone") {
      diagService_->reportError("PixelDCSFSMInterface supervising crate "+stringF(i_PixelDCSFSMInterface->first) + " could not be initialized!",DIAGFATAL);
      *console_<<"PixelDCSFSMInterface supervising crate "<<(i_PixelDCSFSMInterface->first)<<" could not be initialized!"<<std::endl;
    }
  }

  // XDAQ To RCMS notifier will detect the RCMS instance here
  rcmsStateNotifier_.findRcmsStateListener();

  try {
    toolbox::Event::Reference e(new toolbox::Event("Initialize", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    XCEPT_RETHROW(xoap::exception::Exception, "Invalid Command.", e);
    diagService_->reportError("Invalid FSM command Initialize", DIAGWARN);
    *console_<<"Invalid FSM Command"<<std::endl;
  }

  xoap::MessageReference reply=MakeSOAPMessageReference ("InitializeDone");
  diagService_->reportError("Exiting SOAP message callback method PixelSupervisor::Initialize",DIAGINFO);
  *console_<<"Exiting SOAP message callback method PixelSupervisor::Initialize"<<std::endl;

  return reply;
}

xoap::MessageReference PixelSupervisor::ColdReset (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  diagService_->reportError("Entering SOAP callback for ColdReset.",DIAGINFO);
  *console_<<"PixelSupervisor: Entering SOAP callback for ColdReset."<<std::endl;
  std::string response( "ColdResetDone" );
  //std::cout << "Entering cold reset" <<std::endl;
  if (useTTC_) {
    try
      {
    	Supervisors::iterator i_PixelTTCSupervisor;
    	for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor)
          {
            // loop over the TTCci systems. Should be 2 as of Jan 2012
            string reply = Send(i_PixelTTCSupervisor->second, "reset");
            //std::cout << reply << std::endl;
            if (reply != "TTCciControlFSMReset")
              {
  		// attempt reset (response might be resetResponse)
      		diagService_->reportError("PixelTTCSupervisor supervising crate #"+stringF(i_PixelTTCSupervisor->first) + " could not be reset.",DIAGERROR);
      		*console_<<"PixelTTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first)
                         <<" could not be reset: "<<reply<<std::endl;
      		response="ColdResetFailed"; // not sure
              }
            else
              {

      		reply = Send(i_PixelTTCSupervisor->second, "coldReset");
  		//std::cout << reply << std::endl;
      		if (reply != "coldResetResponse")
                  {
                    // attempt cold reset
                    diagService_->reportError("PixelTTCSupervisor supervising crate #"+stringF(i_PixelTTCSupervisor->first) + " could not be coldReset.",DIAGERROR);
                    *console_<<"PixelTTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first)
                             <<" could not be coldReset: "<<reply<<std::endl;
                    response="ColdResetFailed"; // not sure
                  }
              }
          }
      }
    catch( xcept::Exception & e )
      {
  	diagService_->reportError("Failed to coldReset with exception: "+string(e.what()),DIAGERROR);
  	fsmTransition("Failure"); //fire FSM transition
      }
  }
  if (useTCDS_) {
    diagService_->reportError("Failed to coldReset: Not yet implemented for TCDS",DIAGERROR);
  }

  //std::cout << "Exiting cold reset" << std::endl;
  xoap::MessageReference reply=MakeSOAPMessageReference(response);
  return reply;
}

xoap::MessageReference PixelSupervisor::Configure (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  diagService_->reportError("Entering SOAP callback for Configure.",DIAGINFO);
  *console_<<"PixelSupervisor: Entering SOAP callback for Configure."<<std::endl;

  if (state_!="Halted") {
    diagService_->reportError("Configure transition is not allowed from state: "+state_.toString(),DIAGWARN);
    return MakeSOAPMessageReference("ConfigureNotDone");
  }

  configurationTimer_.start();

  // Retrieve the Run Type which is equivalent to the Alias
  // Advertize the Alias
  // Retrieve the Global Key from the database using the Alias
  // Advertize the Global Key
  Attribute_Vector parametersReceived(1);
  parametersReceived[0].name_="RUN_KEY";
  Receive(msg, parametersReceived);
  runType_=parametersReceived[0].value_;
  if (runType_=="InvalidInput") return MakeSOAPMessageReference("ConfigureNotDone");
  diagService_->reportError("PixelSupervisor:: Run Type = "+runType_,DIAGINFO);
  *console_<<"PixelSupervisor:: Run Type = "<<runType_<<std::endl;
  unsigned int globalKey=PixelConfigInterface::getAliases_map().find(runType_)->second;
  theGlobalKey_=new PixelConfigKey(globalKey);
  printConfiguration(*theGlobalKey_);

  //clear errors
  ClearErrors("All");

  xoap::MessageReference reply=MakeSOAPMessageReference("ConfigureDone");
  // That's it! Step to the Configuring state, and
  // relegate all further configuring to the stateConfiguring method.
  bool isOK = fsmTransition("Configure") ;
  if (!isOK) reply=MakeSOAPMessageReference("ConfigureFailed");

  diagService_->reportError("Exiting SOAP callback for Configure.",DIAGINFO);
  *console_<<"PixelSupervisor: Exiting SOAP callback for Configure."<<std::endl;

  return reply;

}

xoap::MessageReference PixelSupervisor::Reconfigure (xoap::MessageReference msg)
{
  diagService_->reportError("Entering SOAP callback for Reconfigure.",DIAGINFO);

  if (state_!="Configured" && state_!="Paused") { //sanity
    diagService_->reportError("Reconfigure transition is not allowed from state: "+state_.toString(),DIAGWARN);
    return MakeSOAPMessageReference("ReconfigureNotDone");
  }

  PixelTimer reconfigureTimer;
  reconfigureTimer.start();
  try {

    // **** this part is just like the normal Configure

    if (useTCDS_) {
      // ConfigureTCDS();
    }

    //retrieve the global key for the given alias
    Attribute_Vector parametersReceived(1);
    parametersReceived[0].name_="RUN_KEY";
    Receive(msg, parametersReceived);
    runType_=parametersReceived[0].value_;
    if (runType_=="InvalidInput") return MakeSOAPMessageReference("ReconfigureNotDone");
    diagService_->reportError("Reconfiguring with Run Type = "+runType_,DIAGINFO);
    //fetch global key for this new alias
    unsigned int globalKey=PixelConfigInterface::getAliases_map().find(runType_)->second;

    //for the global configuration variables (theGlobalKey_), we don't want to delete them
    //until we are sure that reconfiguration has succeeded. The same strategy should be used
    //in the other supervisors
    PixelConfigKey* newGlobalKey=new PixelConfigKey(globalKey); //make the new global key
    if (newGlobalKey==0) XCEPT_RAISE(xdaq::exception::Exception,"Reconfigure failed to create a new global key!");

    diagService_->reportError("Reconfiguring with configuration key="+stringF(newGlobalKey->key()), DIAGINFO);

    // **** adapted from ::stateConfiguring
    // These are parameters sets which will be sent to the underlying Supervisors
    Attribute_Vector parametersToTKFEC(1),  parametersToFEC(1),   parametersToFED(1);

    parametersToFEC[0].name_="GlobalKey";	  parametersToFEC[0].value_=itoa(globalKey);
    parametersToFED[0].name_="GlobalKey";   parametersToFED[0].value_=itoa(globalKey);
    parametersToTKFEC[0].name_="GlobalKey"; parametersToTKFEC[0].value_=itoa(globalKey);


    Supervisors::iterator i_PixelTKFECSupervisor;
    for (i_PixelTKFECSupervisor=PixelTKFECSupervisors_.begin();i_PixelTKFECSupervisor!=PixelTKFECSupervisors_.end();++i_PixelTKFECSupervisor) {
      string reply = Send(i_PixelTKFECSupervisor->second, "Reconfigure", parametersToTKFEC);
      if (reply!="ReconfigureDone") XCEPT_RAISE(xdaq::exception::Exception,"Failed to reconfigure TKFEC");
    }

    Supervisors::iterator i_PixelFECSupervisor;
    for (i_PixelFECSupervisor=PixelFECSupervisors_.begin();i_PixelFECSupervisor!=PixelFECSupervisors_.end();++i_PixelFECSupervisor) {
      string reply = Send(i_PixelFECSupervisor->second, "Reconfigure", parametersToFEC);
      if (reply!="ReconfigureDone") XCEPT_RAISE(xdaq::exception::Exception,"Failed to reconfigure FEC");
    }

    Supervisors::iterator i_PixelFEDSupervisor;
    for (i_PixelFEDSupervisor=PixelFEDSupervisors_.begin();i_PixelFEDSupervisor!=PixelFEDSupervisors_.end();++i_PixelFEDSupervisor) {
      string reply = Send(i_PixelFEDSupervisor->second, "Reconfigure", parametersToFED);
      if (reply!="ReconfigureDone") XCEPT_RAISE(xdaq::exception::Exception,"Failed to reconfigure FED");
    }

    //now we know that reconfiguration has been successful
    delete theGlobalKey_; //clean up the old global key
    theGlobalKey_=newGlobalKey;

  } catch (xcept::Exception & e) {
    diagService_->reportError("Failed to reconfigure with exception: "+string(e.what()),DIAGERROR);
    //this leaves the hardware in an unknown state -- or at least, a state which can only be determined by
    //expert study of the logs; but i want to avoid going to an Error state
  }
  reconfigureTimer.stop();
  cout<<"Time for reconfigure = "<<reconfigureTimer.tottime()<<endl;

  diagService_->reportError("Exiting SOAP callback for Reconfigure.",DIAGINFO);
  xoap::MessageReference reply=MakeSOAPMessageReference("ReconfigureDone");

  return reply;
}

xoap::MessageReference PixelSupervisor::ResetTBM (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  diagService_->reportError("Entering SOAP callback for ResetTBM.",DIAGINFO);
  //   *console_<<"PixelSupervisor: Entering SOAP callback for ResetTBM."<<std::endl;

  xoap::MessageReference replymsg=MakeSOAPMessageReference("ResetTBMDone");

  if (useTTC_) {
    Attribute_Vector parametersToTTC(2);
    parametersToTTC[0].name_="xdaq:CommandPar";
    parametersToTTC[0].value_="Execute Sequence";
    parametersToTTC[1].name_="xdaq:sequence_name";
    parametersToTTC[1].value_="ResetTBM";

    Supervisors::iterator i_PixelTTCSupervisor;
    for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor) {
      if (Send(i_PixelTTCSupervisor->second, "userCommand", parametersToTTC)!="userPixelTTCSupervisorResponse") {
        cout<<"PixelTTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first)<<" could not be used!"<<endl;
        diagService_->reportError("PixelTTCSupervisor supervising crate #"+stringF(i_PixelTTCSupervisor->first) + " could not Reset TBM.",DIAGERROR);
        *console_<<"PixelTTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first)<<" could not Reset TBM."<<std::endl;
        replymsg=MakeSOAPMessageReference("ResetTBMFailed");
      }
    }
  }

  if (useTCDS_) {
    Attribute_Vector paramToTTC(1);
    paramToTTC[0].name_="actionRequestorId";
    paramToTTC[0].value_=TCDSSessionID_;
    Variable_Vector varToTTC(1);
    varToTTC[0].name_="bgoNumber";
    varToTTC[0].type_="unsignedInt";
    varToTTC[0].payload_="14";


    Supervisors::iterator i_PixelTTCController;
    for (i_PixelTTCController=PixelTTCControllers_.begin();i_PixelTTCController!=PixelTTCControllers_.end();++i_PixelTTCController) {
      if (Send(i_PixelTTCController->second, "SendBgo", paramToTTC, varToTTC)!="SendBgoResponse") {
        std::cout<<"PixelTTCController #"<<(i_PixelTTCController->first)<<" could not be used! Maybe it is not yet configured?"<<std::endl;
        diagService_->reportError("PixelTTCController #"+stringF(i_PixelTTCController->first) + " could not Reset TBM.",DIAGERROR);
        *console_<<"PixelTTCController #"<<(i_PixelTTCController->first)<<" could not Reset TBM."<<std::endl;
        replymsg=MakeSOAPMessageReference("ResetTBMFailed");
      }
    }
  }

  diagService_->reportError("Exiting SOAP callback for ResetTBM.",DIAGINFO);
  //   *console_<<"PixelSupervisor: Exiting SOAP callback for ResetTBM."<<std::endl;

  return replymsg;
}

xoap::MessageReference PixelSupervisor::ResetROC (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  diagService_->reportError("Entering SOAP callback for ResetROC.",DIAGINFO);
  //   *console_<<"PixelSupervisor: Entering SOAP callback for ResetROC."<<std::endl;

  xoap::MessageReference replymsg=MakeSOAPMessageReference("ResetROCDone");

  if (useTTC_) {

    Attribute_Vector parametersToTTC(2);
    parametersToTTC[0].name_="xdaq:CommandPar";
    parametersToTTC[0].value_="Execute Sequence";
    parametersToTTC[1].name_="xdaq:sequence_name";
    parametersToTTC[1].value_="ResetROC";

    Supervisors::iterator i_PixelTTCSupervisor;
    for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor) {
      if (Send(i_PixelTTCSupervisor->second, "userCommand", parametersToTTC)!="userPixelTTCSupervisorResponse") {
        cout<<"PixelTTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first)<<" could not be used!"<<endl;
        diagService_->reportError("PixelTTCSupervisor supervising crate #"+stringF(i_PixelTTCSupervisor->first) + " could not Reset ROC.",DIAGERROR);
        *console_<<"PixelTTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first)<<" could not Reset ROC."<<std::endl;
        replymsg=MakeSOAPMessageReference("ResetROCFailed");
      }
    }
  }


  if (useTCDS_) {
    Attribute_Vector paramToTTC(1);
    paramToTTC[0].name_="actionRequestorId";
    paramToTTC[0].value_=TCDSSessionID_;
    Variable_Vector varToTTC(1);
    varToTTC[0].name_="bgoNumber";
    varToTTC[0].type_="unsignedInt";
    varToTTC[0].payload_="15";

    Supervisors::iterator i_PixelTTCController;
    for (i_PixelTTCController=PixelTTCControllers_.begin();i_PixelTTCController!=PixelTTCControllers_.end();++i_PixelTTCController) {
      if (Send(i_PixelTTCController->second, "SendBgo", paramToTTC, varToTTC)!="SendBgoResponse") {
        std::cout<<"PixelTTCController #"<<(i_PixelTTCController->first)<<" could not be used! Maybe it is not yet configured?"<<std::endl;
        diagService_->reportError("PixelTTCController #"+stringF(i_PixelTTCController->first) + " could not Reset ROC.",DIAGERROR);
        *console_<<"PixelTTCController #"<<(i_PixelTTCController->first)<<" could not Reset ROC."<<std::endl;
        replymsg=MakeSOAPMessageReference("ResetROCFailed");
      }
    }
  }

  diagService_->reportError("Exiting SOAP callback for ResetROC.",DIAGINFO);
  //   *console_<<"PixelSupervisor: Exiting SOAP callback for ResetROC."<<std::endl;
  return replymsg;
}

xoap::MessageReference PixelSupervisor::ResetCCU (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  diagService_->reportError("Entering SOAP callback for ResetCCU.",DIAGINFO);
  //  *console_<<"PixelSupervisor: Entering SOAP callback for ResetCCU."<<std::endl;

  xoap::MessageReference replymsg=MakeSOAPMessageReference("ResetCCUDone");

  Supervisors::iterator i_PixelTKFECSupervisor;
  for (i_PixelTKFECSupervisor=PixelTKFECSupervisors_.begin();i_PixelTKFECSupervisor!=PixelTKFECSupervisors_.end();++i_PixelTKFECSupervisor) {
    std::string reply = Send(i_PixelTKFECSupervisor->second, "ResetCCU");
    if (reply!= "ResetCCUResponse") {
      diagService_->reportError("PixelTKFECSupervisor supervising crate #"+stringF(i_PixelTKFECSupervisor->first) + " could not Reset CCU.",DIAGERROR);
      *console_<<"PixelTKFECSupervisor supervising crate #"<<(i_PixelTKFECSupervisor->first)<<" could not Reset CCU."<<std::endl;
      replymsg=MakeSOAPMessageReference("ResetCCUFailed");
    }
  }


  diagService_->reportError("Exiting SOAP callback for ResetCCU.",DIAGINFO);
  //  *console_<<"PixelSupervisor: Exiting SOAP callback for ResetCCU."<<std::endl;
  return replymsg;

}

xoap::MessageReference PixelSupervisor::FSMStateNotification (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  diagService_->reportError("Entering SOAP callback for FSMStateNotification.",DIAGTRACE);
  *console_<<"PixelSupervisor: Entering SOAP callback for FSMStateNotification."<<std::endl;

  Attribute_Vector parameters(3);
  parameters[0].name_="Supervisor";
  parameters[1].name_="Instance";
  parameters[2].name_="FSMState";
  Receive(msg, parameters);

  diagService_->reportError("[PixelSupervisor::FSMStateNotification] Supervisor = "+parameters[0].value_+" Instance = "+parameters[1].value_+" FSMState = "+parameters[2].value_,DIAGINFO);

  if (parameters[0].value_=="PixelFECSupervisor") {
    statePixelFECSupervisors_[atoi(parameters[1].value_.c_str())]=parameters[2].value_;
  }
  if (parameters[0].value_=="PixelTTCSupervisor") {
    statePixelTTCSupervisors_[atoi(parameters[1].value_.c_str())]=parameters[2].value_;
  }
  if (parameters[0].value_=="PixelTKFECSupervisor") {
    statePixelTKFECSupervisors_[atoi(parameters[1].value_.c_str())]=parameters[2].value_;
  }
  if (parameters[0].value_=="PixelFEDSupervisor") {
    statePixelFEDSupervisors_[atoi(parameters[1].value_.c_str())]=parameters[2].value_;
  }
  if (parameters[0].value_=="PixelDCSFSMInterface") {
    statePixelDCSFSMInterface_[atoi(parameters[1].value_.c_str())]=parameters[2].value_;
  }
  if (parameters[0].value_=="PixelDCStoTrkFECDpInterface" || parameters[0].value_=="PixelDCSDpInterface") {
    //in case the actual transition is Configuring->Halted, we want to lie and go from Configuring->Configured
    string newstate=parameters[2].value_;
    if ( statePixelDCStoTrkFECDpInterface_[atoi(parameters[1].value_.c_str())]=="Configuring" && newstate == "Halted" )
      newstate = "Configured";
    statePixelDCStoTrkFECDpInterface_[atoi(parameters[1].value_.c_str())]=newstate;
  }

  try {
    //If anybody reports that they are in error, then go to Error
    if ( parameters[2].value_=="Error" ) {
      toolbox::Event::Reference e(new toolbox::Event("Failure", this));
      fsm_.fireEvent(e);
    }
    else if (state_=="Recovering" && parameters[2].value_=="Halted") {
      //this is what we want to see
      diagService_->reportError("Reached Halted state. Will restart recovery process.",DIAGDEBUG);
      toolbox::Event::Reference e(new toolbox::Event("Recover", this));
      fsm_.fireEvent(e);
    }
    else if (state_=="Recovering" && parameters[2].value_=="Configured") {
      //the recovery process is waiting for a supervisor to finish configuring
      //fire a Recover transition to re-enter stateRecovering
      diagService_->reportError("Reached Configured state. Will restart recovery process.",DIAGDEBUG);
      toolbox::Event::Reference e(new toolbox::Event("Recover", this));
      fsm_.fireEvent(e);
    }
    else if (state_=="Configuring" && parameters[2].value_=="Configured") {
      percentageConfigured_+=10; //crude, but better than always 0
      toolbox::Event::Reference e(new toolbox::Event("Configure", this));
      fsm_.fireEvent(e);
    } else if (state_=="FixingSoftError" && parameters[2].value_=="FixedSoftError") {
      toolbox::Event::Reference e(new toolbox::Event("FixSoftError", this));
      fsm_.fireEvent(e);
    }
  } catch (toolbox::fsm::exception::Exception & ex) { //unlikely that anything other than the fsm transition will throw
    diagService_->reportError("[PixelSupervisor::FSMStateNotification] Invalid command: "+string(ex.what()),DIAGERROR);
  }

  diagService_->reportError("Exiting SOAP callback for FSMStateNotification for supervisor " + parameters[0].value_ + " instance #" + parameters[1].value_ + " state: " + parameters[2].value_, DIAGTRACE);
  *console_<< "PixelSupervisor: Exiting SOAP callback for FSMStateNotification for supervisor " << parameters[0].value_.c_str() << " instance #" << parameters[1].value_.c_str() << " state: " << parameters[2].value_.c_str() << std::endl;
  xoap::MessageReference reply=MakeSOAPMessageReference("FSMStateNotificationReceived");
  return reply;
}

unsigned int PixelSupervisor::getNumberOfSupervisorsInState(const SupervisorStates & ss, const string & state) {
  unsigned int nInState=0;
  SupervisorStates::const_iterator i;
  for (i = ss.begin() ; i != ss.end() ; ++i) {
    if (i->second == state) nInState++;
  }
  return nInState;
}

unsigned int PixelSupervisor::getNumberOfSupervisorsNotInState(const SupervisorStates & ss, const string & state) {
  unsigned int notInState=0;
  SupervisorStates::const_iterator i;
  for (i = ss.begin() ; i != ss.end() ; ++i) {
    if (i->second != state) notInState++;
  }
  return notInState;
}

xoap::MessageReference PixelSupervisor::StatusNotification (xoap::MessageReference msg) throw (xoap::exception::Exception)
{

  Attribute_Vector parameters(3);
  parameters[0].name_="Description";
  parameters[0].value_="Message";
  parameters[1].name_="Time";
  parameters[1].value_="0";
  parameters[2].name_="usec";
  parameters[2].value_="0";
  Receive(msg, parameters);

  lastMessage_ = parameters[0].value_;
  lastMessageTime_.tv_sec = atoi(parameters[1].value_.c_str());
  lastMessageTime_.tv_usec = atoi(parameters[2].value_.c_str());

  xoap::MessageReference reply=MakeSOAPMessageReference("StatusNotificationReceived");
  return reply;

}

xoap::MessageReference PixelSupervisor::Halt (xoap::MessageReference msg) throw (xoap::exception::Exception) {

  diagService_->reportError("Entering transition HALT",DIAGINFO);
  *console_<<"--- Halting ---"<<std::endl;

  if (state_!="Running" && state_!="RunningSoftErrorDetected" && state_!="RunningDegraded" && state_!="Done" && state_!="Configured" && state_!="Paused" && state_!="TestTTS") {
    diagService_->reportError("Halt transition is not allowed from state: "+state_.toString(),DIAGWARN);
    return MakeSOAPMessageReference("HaltFailed");
  }

  std::string response="HaltDone";

  if ( theCalibObject_!=0 ) {

    if ( state_ == "Paused" ) {

      calibWorkloop_->remove(calibJob_);
      diagService_->reportError("[PixelSupervisor::Halt] Removed job from the calib workloop.",DIAGINFO);

    } else if ( state_ == "Running" ) {

      calibWorkloop_->cancel();
      diagService_->reportError( "[PixelSupervisor::Halt] Cancelled the calib workloop.",DIAGINFO);
      calibWorkloop_->remove(calibJob_);
      diagService_->reportError("[PixelSupervisor::Halt] Removed job from the calib workloop.",DIAGINFO);
      diagService_->reportError("[PixelSupervisor::Halt] Attempting to run end calibration.",DIAGINFO);
      theCalibAlgorithm_->runEndCalibration();

    } else if ( state_ == "Done" ) {

      vector<string> names=updates_->getNames();
      Attribute_Vector parameters(names.size());
      for(unsigned int i=0;i<names.size();i++){
        parameters.at(i).name_=names[i];
      }
      Receive(msg, parameters);
      unsigned int nTypes=updates_->nTypes();
      for(unsigned int iType=0;iType<nTypes;iType++){
        std::string type=updates_->type(iType);
	diagService_->reportError("[PixelSupervisor::Halt] iType, type="+stringF(iType)+" "+type,DIAGINFO);
        bool update=false;
        bool found=false;
        for(unsigned int i=0;i<names.size();i++){
          if (parameters.at(i).name_==type){
            assert(found==false);
            found=true;
            update=parameters.at(i).value_=="Yes";
          }
        }
        assert(found);
        if (update) {
	diagService_->reportError( "[PixelSupervisor::Halt] will update data for type:"+type,DIAGINFO);
        //now will look for aliases to uupdate;
        vector<string> aliasesToUpdate;
        vector<string> aliases=updates_->aliases(iType);
        for (unsigned int iAlias=0;iAlias<aliases.size();iAlias++){
          std::string alias=aliases[iAlias];
          bool updateAlias=false;
          bool foundAlias=false;
          for(unsigned int i=0;i<names.size();i++){
            if (parameters.at(i).name_==type+alias){
              assert(foundAlias==false);
              foundAlias=true;
              updateAlias=parameters.at(i).value_=="Yes";
              }
            }
            assert(foundAlias);
            if (updateAlias) aliasesToUpdate.push_back(alias);
          }
        updateConfig(type, aliasesToUpdate);
        }
      }

    } else if ( state_ == "Configured" ){

    } else {assert(0);}

    delete theCalibAlgorithm_;
    theCalibAlgorithm_ = 0;
  }

  // Clear mapNamePortCard_
  clearMapNamePortCard();

  try { //sending SOAP

  Supervisors::iterator i_PixelFEDSupervisor;
  for (i_PixelFEDSupervisor=PixelFEDSupervisors_.begin();i_PixelFEDSupervisor!=PixelFEDSupervisors_.end();++i_PixelFEDSupervisor) {
    std::string reply = Send(i_PixelFEDSupervisor->second, "Halt");
    if (reply!= "HaltDone") {
      diagService_->reportError("PixelFEDSupervisor supervising crate #"+stringF(i_PixelFEDSupervisor->first) + " could not be halted.",DIAGERROR);
      *console_<<"PixelFEDSupervisor supervising crate #"<<(i_PixelFEDSupervisor->first)<<" could not be halted."<<std::endl;
      response="HaltFailed";
    } else {
      statePixelFEDSupervisors_[i_PixelFEDSupervisor->first]="Halted";
    }
  }

  if (fsm_.getStateName(fsm_.getCurrentState())!="TTSTestMode") {

    TCDSSessionID_=toolbox::toString("#%d",rand());
    diagService_->reportError("Created new TCDS session ID: "+TCDSSessionID_, DIAGINFO);
    *console_<<"Created new TCDS session ID: " + TCDSSessionID_<<std::endl;
    std::cout <<"Created new TCDS session ID: " + TCDSSessionID_<<std::endl;


    Supervisors::iterator i_PixelTTCSupervisor;
    for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor) {
      std::string reply;
      if (useTTC_) {
        reply = Send(i_PixelTTCSupervisor->second, "reset");
      }
      if (useTCDS_) {
        reply = Send(i_PixelTTCSupervisor->second, "Halt");
      }
      if ((useTCDS_ && reply!= "HaltResponse") || (useTTC_ && reply!= "TTCciControlFSMReset")) {
        diagService_->reportError("PixelTTCSupervisor supervising crate #"+stringF(i_PixelTTCSupervisor->first) + " could not be halted.",DIAGERROR);
        *console_<<"PixelTTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first)<<" could not be halted: "<<reply<<std::endl;
        response="HaltFailed";
      }
      else{
        if (useTCDS_){
	  std::string fsmState=Send(i_PixelTTCSupervisor->second, "QueryFSMState");
	  statePixelTTCSupervisors_[i_PixelTTCSupervisor->first]=fsmState;

          Attribute_Vector parameters(1);
          parameters[0].name_="TCDSSessionID";
          parameters[0].value_=TCDSSessionID_;
          reply = Send(i_PixelTTCSupervisor->second, "Handshake", parameters);
          diagService_->reportError("TTCSupervisor: Sending Handshake reply: " + reply, DIAGINFO);
          *console_<<"TTCSupervisor: Sending Handshake reply: " << reply <<std::endl;
          if (reply!= "HandshakeResponse"){
            diagService_->reportError("TTCSupervisor supervising crate #"+stringF(i_PixelTTCSupervisor->first) + " could not receive new TCDS session ID.",DIAGERROR);
            *console_<<"TTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first)<<" could not receive new TCDS session ID for halting step: "<<reply<<std::endl;
          }
        }
      }
    }

    if (useTCDS_) {
      Supervisors::iterator i_PixelLTCSupervisor;
      for (i_PixelLTCSupervisor=PixelLTCSupervisors_.begin();i_PixelLTCSupervisor!=PixelLTCSupervisors_.end();++i_PixelLTCSupervisor) {
        std::string reply = Send(i_PixelLTCSupervisor->second, "Halt");
        if (reply!= "HaltResponse") {
          diagService_->reportError("PixelLTCSupervisor supervising crate #"+stringF(i_PixelLTCSupervisor->first) + " could not be halted.",DIAGERROR);
          *console_<<"PixelLTCSupervisor supervising crate #"<<(i_PixelLTCSupervisor->first)<<" could not be halted: "<<reply<<std::endl;
          response="HaltFailed";
        }
        else{
          std::string fsmState=Send(i_PixelLTCSupervisor->second, "QueryFSMState");
          statePixelLTCSupervisors_[i_PixelLTCSupervisor->first]=fsmState;

          Attribute_Vector parameters(1);
          parameters[0].name_="TCDSSessionID";
          parameters[0].value_=TCDSSessionID_;
          reply = Send(i_PixelLTCSupervisor->second, "Handshake", parameters);
          diagService_->reportError("LTCSupervisor: Sending Handshake reply: " + reply, DIAGINFO);
          *console_<<"LTCSupervisor: Sending Handshake reply: " << reply <<std::endl;
          if (reply!= "HandshakeResponse"){
            diagService_->reportError("LTCSupervisor supervising crate #"+stringF(i_PixelLTCSupervisor->first) + " could not receive new TCDS session ID.",DIAGERROR);
            *console_<<"LTCSupervisor supervising crate #"<<(i_PixelLTCSupervisor->first)<<" could not receive new TCDS session ID for halting step: "<<reply<<std::endl;
          }
        }
      }
    }




    Supervisors::iterator i_PixelFECSupervisor;
    for (i_PixelFECSupervisor=PixelFECSupervisors_.begin();i_PixelFECSupervisor!=PixelFECSupervisors_.end();++i_PixelFECSupervisor) {
      std::string reply = Send(i_PixelFECSupervisor->second, "Halt");
      if (reply!= "HaltDone") {
        diagService_->reportError("PixelFECSupervisor supervising crate #"+stringF(i_PixelFECSupervisor->first) + " could not be halted.",DIAGERROR);
        *console_<<"PixelFECSupervisor supervising crate #"<<(i_PixelFECSupervisor->first)<<" could not be halted."<<std::endl;
        response="HaltFailed";
      } else {
        statePixelFECSupervisors_[i_PixelFECSupervisor->first]="Halted";
      }
    }

    Supervisors::iterator i_PixelTKFECSupervisor;
    for (i_PixelTKFECSupervisor=PixelTKFECSupervisors_.begin();i_PixelTKFECSupervisor!=PixelTKFECSupervisors_.end();++i_PixelTKFECSupervisor) {
      std::string reply = Send(i_PixelTKFECSupervisor->second, "Halt");
      if (reply!= "HaltDone") {
        diagService_->reportError("PixelTKFECSupervisor supervising crate #"+stringF(i_PixelTKFECSupervisor->first) + " could not be halted.",DIAGERROR);
        *console_<<"PixelTKFECSupervisor supervising crate #"<<(i_PixelTKFECSupervisor->first)<<" could not be halted."<<std::endl;
        response="HaltFailed";
      }
    }

    if (!PixelSlinkMonitors_.empty()) {
      Supervisors::iterator i_PixelSlinkMonitor;
      for (i_PixelSlinkMonitor=PixelSlinkMonitors_.begin();i_PixelSlinkMonitor!=PixelSlinkMonitors_.end();++i_PixelSlinkMonitor) {
        std::string reply = Send(i_PixelSlinkMonitor->second, "Halt");
        if (reply!= "HaltDone") {
          diagService_->reportError("PixelSlinkMonitor supervising crate #"+stringF(i_PixelSlinkMonitor->first) + " could not be halted.",DIAGERROR);
          *console_<<"PixelSlinkMonitor supervising crate #"<<(i_PixelSlinkMonitor->first)<<" could not be halted."<<std::endl;
        }
      }
    }

    if (!PixelDCStoTrkFECDpInterface_.empty()) {
      Supervisors::iterator i_PixelDCStoTrkFECDpInterface;
      for (i_PixelDCStoTrkFECDpInterface=PixelDCStoTrkFECDpInterface_.begin(); i_PixelDCStoTrkFECDpInterface!=PixelDCStoTrkFECDpInterface_.end(); ++i_PixelDCStoTrkFECDpInterface) {
        std::string reply=Send(i_PixelDCStoTrkFECDpInterface->second, "Halt");
        if (reply!="HaltDone") {
          diagService_->reportError("PixelDCStoTrkFECDpInterface #"+stringF(i_PixelDCStoTrkFECDpInterface->first) + " could not be halted.",DIAGERROR);
          *console_<<"PixelDCStoTrkFECDpInterface #"<<(i_PixelDCStoTrkFECDpInterface->first)<<" could not be halted."<<std::endl;
        }
      }
    }

    if (!PixelDCSFSMInterface_.empty()) {
      Supervisors::iterator i_PixelDCSFSMInterface;
      for (i_PixelDCSFSMInterface=PixelDCSFSMInterface_.begin(); i_PixelDCSFSMInterface!=PixelDCSFSMInterface_.end(); ++i_PixelDCSFSMInterface) {
        std::string reply=Send(i_PixelDCSFSMInterface->second, "Halt");
        if (reply!="HaltDone") {
          diagService_->reportError("PixelDCSFSMInterface #"+stringF(i_PixelDCSFSMInterface->first) + " could not be halted.",DIAGERROR);
          *console_<<"PixelDCSFSMInterface #"<<(i_PixelDCSFSMInterface->first)<<" could not be halted."<<std::endl;
        }
      }
    }
  }
  } catch ( xcept::Exception & err) {
    diagService_->reportError("Halt transition failed with exception: "+string(err.what()),DIAGERROR);
    response="HaltFailed";
  }

  if (response=="HaltDone") {
    delete theGlobalKey_;
    theGlobalKey_=0;
    delete theCalibObject_;
    theCalibObject_=0;

    fsmTransition("Halt"); //fire FSM transition
    *console_<<"---------------"<<std::endl;
  } else { // Do not go to the Halted state, do not delete Global Keys etc
    fsmTransition("Failure"); //fire FSM transition
    *console_<<"--- Halting Failed! ---"<<std::endl;
  }

  diagService_->reportError("Exiting transition HALT",DIAGINFO);

  return MakeSOAPMessageReference(response);

}

xoap::MessageReference PixelSupervisor::Recover(xoap::MessageReference msg)  {

  if (state_!="Error") return MakeSOAPMessageReference("RecoverFailed"); //sanity

  //we don't do much here. Just move to the Recovering state

  //we could add a check of the JobControl status to make sure that all processes are alive
  //we could also send FSMStateRequests in order to get the state variables refreshed
  //but this should not be necessary

  bool isOK = fsmTransition("Recover") ;
  if (!isOK) return MakeSOAPMessageReference("RecoverFailed");
  return MakeSOAPMessageReference("RecoverDone");

}

void PixelSupervisor::stateRecovering(toolbox::fsm::FiniteStateMachine & fsm)  {

  //Goal is to reach the Halted state, starting in the Error state
  //the fundamental assumption will be that the state variables are correct

  state_=fsm_.getStateName(fsm_.getCurrentState());

  if ( theCalibObject_ != 0 ) {
    diagService_->reportError( "[PixelSupervisor::stateRecovering] A calibration is active",DIAGDEBUG);
    try {
      if ( calibWorkloop_->isActive() ) {
	calibWorkloop_->cancel();
	diagService_->reportError( "[PixelSupervisor::stateRecovering] Cancelled the calib workloop.",DIAGINFO);
      }
      calibWorkloop_->remove(calibJob_); //this may fail
    } catch (xcept::Exception & e) {
      diagService_->reportError( "[PixelSupervisor::stateRecovering] Caught an exception while ending the calib workloop: "
				 +string(e.what()),DIAGDEBUG);
    }

    //I think we should not try to run the endCalibration method
    //if we are in Error than we will not allow the calibration results, if any, to be saved
    delete theCalibAlgorithm_;
    theCalibAlgorithm_ = 0;
  }

  clearMapNamePortCard();

  try {
    //we need to determine if any supervisors are still in Error.
    //if they are, then proceed to recovery
    //if everybody is Halted, then we can return to Halted

    unsigned int nError = getNumberOfSupervisorsInState(statePixelFEDSupervisors_, "Error") ;
    nError += getNumberOfSupervisorsInState(statePixelFECSupervisors_, "Error");
    nError += getNumberOfSupervisorsInState(statePixelTKFECSupervisors_, "Error");
    nError += getNumberOfSupervisorsInState(statePixelDCSFSMInterface_, "Error");
    if (useTCDS_){
          nError += getNumberOfSupervisorsInState(statePixelTTCSupervisors_, "Error");
          nError += getNumberOfSupervisorsInState(statePixelLTCSupervisors_, "Error");
    }
    if (nError==0) { //great! now test for Halted!
      unsigned int notHalted = getNumberOfSupervisorsNotInState(statePixelFEDSupervisors_, "Halted") ;
      notHalted += getNumberOfSupervisorsNotInState(statePixelFECSupervisors_, "Halted");
      notHalted += getNumberOfSupervisorsNotInState(statePixelTKFECSupervisors_, "Halted");
      notHalted += getNumberOfSupervisorsNotInState(statePixelDCSFSMInterface_, "Halted");\
      if (useTCDS_){
        notHalted += getNumberOfSupervisorsNotInState(statePixelTTCSupervisors_, "Halted");
        notHalted += getNumberOfSupervisorsNotInState(statePixelLTCSupervisors_, "Halted");
      }
      if (notHalted==0) {
	//we're halted!
	delete theGlobalKey_;
	theGlobalKey_=0;
	delete theCalibObject_;
	theCalibObject_=0;
	toolbox::Event::Reference e(new toolbox::Event("RecoverDone", this));
	fsm_.fireEvent(e);
      }
      else { //nobody is in Error, but not everybody is Halted
	//this could happen if the error is recovered but somebody is still configuring
	diagService_->reportError("Error recovery successful. Waiting for Supervisors to Halt",DIAGDEBUG);
	//	return;
      }
    }
    else { //still have errors
      diagService_->reportError("A Supervisor is still in Error",DIAGDEBUG);
      //proceed with recovery of the supervisors (below)
    }

    bool returnnow;
    //FEDSupervisor
    returnnow =  recoverSupervisors( PixelFEDSupervisors_, statePixelFEDSupervisors_ );
    if (returnnow) return;

    Supervisors::iterator i_PixelTTCSupervisor;
    for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor) {
      std::string reply;
      if (useTTC_) {
        reply = Send(i_PixelTTCSupervisor->second, "reset");
      }
      if (useTCDS_) {
        reply = Send(i_PixelTTCSupervisor->second, "Halt");
      }
      if ((useTTC_ && reply!= "TTCciControlFSMReset") || (useTCDS_ && reply!= "HaltResponse")) {

        *console_<<"PixelTTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first) <<" could not be halted: "<<reply<<std::endl;
        ostringstream err;
        err<<"PixelTTCSupervisor #"<<i_PixelTTCSupervisor->first<<" could not be halted: "<<reply;
        //	XCEPT_RAISE(xdaq::exception::Exception, err.str());
        //spit out a warning and continue...
        diagService_->reportError(err.str(),DIAGINFO);
      }
      if (useTCDS_) {
	std::string fsmState=Send(i_PixelTTCSupervisor->second, "QueryFSMState");
	statePixelTTCSupervisors_[i_PixelTTCSupervisor->first]=fsmState;
      }
    }

    if (useTCDS_) {
      Supervisors::iterator i_PixelLTCSupervisor;
      for (i_PixelLTCSupervisor=PixelLTCSupervisors_.begin();i_PixelLTCSupervisor!=PixelLTCSupervisors_.end();++i_PixelLTCSupervisor) {
        std::string reply = Send(i_PixelLTCSupervisor->second, "Halt");
        if (useTCDS_ && reply!= "HaltResponse") {
          *console_<<"PixelLTCSupervisor supervising crate #"<<(i_PixelLTCSupervisor->first) <<" could not be halted: "<<reply<<std::endl;
          ostringstream err;
          err<<"PixelLTCSupervisor #"<<i_PixelLTCSupervisor->first<<" could not be halted: "<<reply;
          //      XCEPT_RAISE(xdaq::exception::Exception, err.str());
          //spit out a warning and continue...
          diagService_->reportError(err.str(),DIAGINFO);
        }
        std::string fsmState=Send(i_PixelLTCSupervisor->second, "QueryFSMState");
        statePixelLTCSupervisors_[i_PixelLTCSupervisor->first]=fsmState;
      }
    }

    //FECSupervisor
    returnnow =  recoverSupervisors( PixelFECSupervisors_, statePixelFECSupervisors_ );
    if (returnnow) return;

    //TKFECSupervisor
    returnnow =  recoverSupervisors( PixelTKFECSupervisors_, statePixelTKFECSupervisors_ );
    if (returnnow) return;


    //FIXME need to add PixelSlinkMonitor code (see ::Halt)

    //we now keep track of the state of this guy (although we are not 100% honest)
    //but because this guy has a simple FSM and a valid H->H transition, then
    //let's just Halt it
    if (!PixelDCStoTrkFECDpInterface_.empty()) {
      Supervisors::iterator i_PixelDCStoTrkFECDpInterface;
      for (i_PixelDCStoTrkFECDpInterface=PixelDCStoTrkFECDpInterface_.begin(); i_PixelDCStoTrkFECDpInterface!=PixelDCStoTrkFECDpInterface_.end(); ++i_PixelDCStoTrkFECDpInterface) {
	string reply=Send(i_PixelDCStoTrkFECDpInterface->second, "Halt");
	if (reply!="HaltDone") {
	  ostringstream err;
	  err<<"PixelDCStoTrkFECDpInterface #"<<i_PixelDCStoTrkFECDpInterface->first<< " could not be halted.";
	  XCEPT_RAISE(xdaq::exception::Exception, err.str());
	}
      }
    }

    //DCSFSMInterface
    if (!PixelDCSFSMInterface_.empty()) { //not sure if this protection is necessary
      returnnow =  recoverSupervisors( PixelDCSFSMInterface_, statePixelDCSFSMInterface_ );
      if (returnnow) return;
    }

    //the final step is to delete the global key etc. That is done in FSMStateNotification.

  } catch ( xcept::Exception & e ) {
    //If we have a failure here then we return to the Error state
    diagService_->reportError("Failure while trying to recover from Error state. Caught exception: "+string(e.what()),DIAGERROR);
    fsmTransition("Failure") ;
  }
}

bool PixelSupervisor::recoverSupervisors( const Supervisors & supervisor, const SupervisorStates & state ) {
  //returns true if we need to immediately return from calling function

  for ( Supervisors::const_iterator iSupervisor=supervisor.begin();iSupervisor!=supervisor.end();++iSupervisor) {

    const string supervisorName = iSupervisor->second->getClassName();

    string action="Halt";

    const string istate= state.find(iSupervisor->first)->second;
    if  (istate=="Configuring") {
      //this could happen if, for example, the FEDSupervisor goes into Error while the FECSupervisor is Configuring.
      //if there is a quick attempt at recovery the FECSupervisor will still be Configuring
      //so we wait for the Configuring supervisor to finish, then Halt it
      //of course if it subsequently goes into Error, then we must Recover it
      diagService_->reportError(supervisorName+" #"+stringF(iSupervisor->first) + " is still configuring. Recover process is waiting for it to finish",DIAGINFO);
      return true;
    }
    else if (istate=="Error") {
      //in this case we want to send a Recover message to the offending Supervisor
      action="Recover";
    }
    else if (istate=="Halted") {  //this is where we want to be!
      continue;
    }  //we should be able to simply Halt from any other state

    string reply = Send(iSupervisor->second, action);
    string expectedReply = action + "Done";

    if (reply!= expectedReply) {
      //at this point we're out of options
      ostringstream err;
      err<<supervisorName<<" #"<<iSupervisor->first<< " failed to execute "<<action;
      XCEPT_RAISE(xdaq::exception::Exception, err.str());
    }
  }

  return false;

}

xoap::MessageReference PixelSupervisor::Start (xoap::MessageReference msg) {

  diagService_->reportError("Entering transition START",DIAGINFO);
  *console_<<"--- Starting ---"<<std::endl;

  if (state_!="Configured") {
    diagService_->reportError("Start transition is not allowed from state: "+state_.toString(),DIAGWARN);
    return MakeSOAPMessageReference("StartFailed");
  }

  Attribute_Vector parameters(1);
  parameters[0].name_="RUN_NUMBER";
  try {
    Receive(msg, parameters);
    runNumber_=parameters[0].value_;
  }
  catch (xcept::Exception & e) {
    diagService_->reportError("Failed to get run number with exception: "+string(e.what()),DIAGERROR);
    return MakeSOAPMessageReference("StartFailed");
  }

  setupOutputDir();

  if (!useRunInfo_ && runNumberFromLastFile_) {
    if (atoi(runNumber_.c_str())!=0){
      ofstream fout((posOutputDirs_+"/LastRunNumber.txt").c_str());
      fout<<runNumber_;
      fout.close();
    }
  }

  diagService_->reportError("Start Run "+runNumber_,DIAGINFO);
  *console_<<"PixelSupervisor::Start Run "<<runNumber_<<"."<<std::endl;

  std::string response="StartDone";

  try{
    Supervisors::iterator i_PixelTKFECSupervisor;
    for (i_PixelTKFECSupervisor=PixelTKFECSupervisors_.begin();i_PixelTKFECSupervisor!=PixelTKFECSupervisors_.end();++i_PixelTKFECSupervisor) {
      std::string reply = Send(i_PixelTKFECSupervisor->second, "Start", parameters);
      if (reply!= "StartDone") {
        diagService_->reportError("PixelTKFECSupervisor supervising crate #"+stringF(i_PixelTKFECSupervisor->first) + " could not be started.",DIAGERROR);
        *console_<<"PixelTKFECSupervisor supervising crate #"<<(i_PixelTKFECSupervisor->first)<<" could not be started."<<std::endl;
        response="StartFailed";
      }
    }
  } catch (xcept::Exception & e) {
    diagService_->reportError("Failure while sending Start SOAP to PixelTKFECSupervisors. Exception: "+string(e.what()),DIAGERROR);
    response="StartFailed";
  }

  try {
    Supervisors::iterator i_PixelFECSupervisor;
    for (i_PixelFECSupervisor=PixelFECSupervisors_.begin();i_PixelFECSupervisor!=PixelFECSupervisors_.end();++i_PixelFECSupervisor) {
      std::string reply = Send(i_PixelFECSupervisor->second, "Start", parameters);
      if (reply!= "StartDone") {
        diagService_->reportError("PixelFECSupervisor supervising crate #"+stringF(i_PixelFECSupervisor->first) + " could not be started.",DIAGERROR);
        *console_<<"PixelFECSupervisor supervising crate #"<<(i_PixelFECSupervisor->first)<<" could not be started."<<std::endl;
        response="StartFailed";
      }
    }
  } catch (xcept::Exception & e) {
    diagService_->reportError("Failure while sending Start SOAP to PixelFECSupervisors. Exception: "+string(e.what()),DIAGERROR);
    response="StartFailed";
  }

  try {
    Supervisors::iterator i_PixelFEDSupervisor;
    for (i_PixelFEDSupervisor=PixelFEDSupervisors_.begin();i_PixelFEDSupervisor!=PixelFEDSupervisors_.end();++i_PixelFEDSupervisor) {
      std::string reply = Send(i_PixelFEDSupervisor->second, "Start", parameters);
      if (reply!= "StartDone") {
        diagService_->reportError("PixelFEDSupervisor supervising crate #"+stringF(i_PixelFEDSupervisor->first) + " could not be started.",DIAGERROR);
        *console_<<"PixelFEDSupervisor supervising crate #"<<(i_PixelFEDSupervisor->first)<<" could not be started."<<std::endl;
        response="StartFailed";
      }
    }
  } catch (xcept::Exception & e) {
    diagService_->reportError("Failure while sending Start SOAP to PixelFEDSupervisors. Exception: "+string(e.what()),DIAGERROR);
    response="StartFailed";
  }

  try {
    Supervisors::iterator i_PixelTTCSupervisor;
    for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor) {
      std::string reply;
      if (useTTC_)
        reply = Send(i_PixelTTCSupervisor->second, "enable");
      if (useTCDS_) {
        Attribute_Vector parametersToTCDS(1);
        parametersToTCDS[0].name_="RunNumber"; parametersToTCDS[0].value_=parameters[0].value_;
        reply = Send(i_PixelTTCSupervisor->second, "Start", parametersToTCDS);
      }
      if ((useTTC_ && reply!= "enableResponse") || (useTCDS_ && reply!= "StartResponse")) {
        diagService_->reportError("PixelTTCSupervisor supervising crate #"+stringF(i_PixelTTCSupervisor->first) + " could not be started.",DIAGERROR);
        *console_<<"PixelTTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first) <<" could not be started: "<<reply<<std::endl;
        response="StartFailed";
      }
      else{
	if(useTCDS_){
	  std::string fsmState=Send(i_PixelTTCSupervisor->second, "QueryFSMState");
	  statePixelTTCSupervisors_[i_PixelTTCSupervisor->first]=fsmState;
	}
      }
    }
  } catch (xcept::Exception & e) {
    diagService_->reportError("Failure while sending Start SOAP to PixelTTCSupervisor Supervisors. Exception: "+string(e.what()),DIAGERROR);
    response="StartFailed";
  }


  if (useTCDS_) {

    try {
      Supervisors::iterator i_PixelLTCSupervisor;
      for (i_PixelLTCSupervisor=PixelLTCSupervisors_.begin();i_PixelLTCSupervisor!=PixelLTCSupervisors_.end();++i_PixelLTCSupervisor) {
        std::string reply;
        Attribute_Vector parametersToTCDS(1);
        parametersToTCDS[0].name_="RunNumber"; parametersToTCDS[0].value_=parameters[0].value_;
        reply = Send(i_PixelLTCSupervisor->second, "Start", parametersToTCDS);

        if (reply!= "StartResponse") {
          diagService_->reportError("PixelLTCSupervisor supervising crate #"+stringF(i_PixelLTCSupervisor->first) + " could not be started.",DIAGERROR);
          *console_<<"PixelLTCSupervisor supervising crate #"<<(i_PixelLTCSupervisor->first) <<" could not be started: "<<reply<<std::endl;
          response="StartFailed";
        }
        else{
          std::string fsmState=Send(i_PixelLTCSupervisor->second, "QueryFSMState");
          statePixelLTCSupervisors_[i_PixelLTCSupervisor->first]=fsmState;
        }
      }
    } catch (xcept::Exception & e) {
      diagService_->reportError("Failure while sending Start SOAP to PixelLTCSupervisor Supervisors. Exception: "+string(e.what()),DIAGERROR);
      response="StartFailed";
    }

  }


  diagService_->reportError("We are about to send Start to PixelSlinkMonitor",DIAGERROR);
  try {
    if (!PixelSlinkMonitors_.empty()) {
      Attribute_Vector parametersToSlinkMonitor(2);
      parametersToSlinkMonitor[0]=parameters[0];
      parametersToSlinkMonitor[1].name_="Monitor"; parametersToSlinkMonitor[1].value_="Data";
      Supervisors::iterator i_PixelSlinkMonitor;
      for (i_PixelSlinkMonitor=PixelSlinkMonitors_.begin();i_PixelSlinkMonitor!=PixelSlinkMonitors_.end();++i_PixelSlinkMonitor) {
        std::string reply = Send(i_PixelSlinkMonitor->second, "Start", parametersToSlinkMonitor);
        if (reply!= "StartDone") {
  	diagService_->reportError("PixelSlinkMonitor crate #"+stringF(i_PixelSlinkMonitor->first) + " could not be started.",DIAGERROR);
  	*console_<<"PixelSlinkMonitor supervising crate #"<<(i_PixelSlinkMonitor->first)<<" could not be configured."<<std::endl;
        }
      }
    }
  } catch (xcept::Exception & e) {
    diagService_->reportError("Failure while sending Start SOAP to PixelSlinkMonitor Supervisors. Exception: "+string(e.what()),DIAGERROR);
    response="StartFailed";
  }

  // Start the calibration, if this is a calibration run.
  if ( theCalibObject_!=0 && response!="StartFailed" )
    {
      std::string mode = theCalibObject_->mode();

      //Creating pointers to give to PixelCalibrationBase object
      PixelSupervisorConfiguration* pixSupConfPtr = dynamic_cast <PixelSupervisorConfiguration*> (this);

      SOAPCommander* soapCmdrPtr = dynamic_cast <SOAPCommander*> (this);

      PixelDCSSOAPCommander dcsSoapCommander(this);

//      if (psxServers_.size()!=1) {
//	diagService_->reportError("PixelSupervisor::stateRunning psxServers_.size()="+stringF(psxServers_.size())+"\n"+"Expect to have exactly one psxServer in configuration.",DIAGFATAL);
//      //::abort();
//      }
//      PixelDCSPVSSCommander pvssCommander(this, psxServers_.begin()->second);

      PixelCalibrationFactory calibMaker;

      theCalibAlgorithm_=calibMaker.getCalibration(mode,
						   pixSupConfPtr,
						   soapCmdrPtr,
						   &dcsSoapCommander,
						   0 //&pvssCommander
						   );

      if (theCalibAlgorithm_==0){
	diagService_->reportError("[PixelSupervisor::Start] Could not find calibration for mode="+mode,DIAGFATAL);
	assert(theCalibAlgorithm_!=0);
      }

      vector<string> paths=theCalibAlgorithm_->calibrated();

      updates_=new PixelConfigDataUpdates(paths);

      updates_->print();

      runBeginCalibration_=true;
      calibWorkloop_->submit(calibJob_);

      diagService_->reportError("[PixelSupervisor::Start]: Calib job submitted to the workloop.",DIAGINFO);

      calibWorkloop_->activate();

      diagService_->reportError("PixelSupervisor::Start: Calib workloop activated.",DIAGINFO);

    }


  if (response=="StartDone") {

    ofstream configKeyFile;
    configKeyFile.open((outputDir()+"/PixelConfigurationKey.txt").c_str(), ios::app);
    if (configKeyFile.is_open()) {
      configKeyFile<<"Pixel Run Alias / Run Type = "<<runType_<<std::endl;
      configKeyFile<<"Pixel Global Configuration Key = "<<theGlobalKey_->key()<<std::endl<<std::endl;
      configKeyFile.close();
    }
    else {
      fsmTransition("Failure");
      *console_<<"--- Cannot write output file to: " << outputDir() << " ---"<<std::endl;
    }

    fsmTransition("Start");
    *console_<<"---------------"<<std::endl;
  } else {
    fsmTransition("Failure");
    *console_<<"--- Start Failed! ---"<<std::endl;
  }
  diagService_->reportError("Exiting transition START",DIAGINFO);
  return MakeSOAPMessageReference(response);

}

xoap::MessageReference PixelSupervisor::Stop (xoap::MessageReference msg) throw (xoap::exception::Exception) {

  diagService_->reportError("Entering transition STOP",DIAGINFO);
  *console_<<"--- Stopping ---"<<std::endl;

  if (state_!="Running" && state_!="RunningSoftErrorDetected" && state_!="RunningDegraded" && state_!="Paused") {
    diagService_->reportError("Stop transition is not allowed from state: "+state_.toString(),DIAGWARN);
    return MakeSOAPMessageReference("StopFailed");
  }
  std::string response="StopDone";

  if ( theCalibObject_!=0 )
  {
    if ( state_ == "Paused" )
    {
      calibWorkloop_->remove(calibJob_);

      diagService_->reportError( "PixelSupervisor::Stop: Removed job from the calib workloop.",DIAGINFO);
    }
    else if ( state_ == "Running" )
    {
      calibWorkloop_->cancel();
      diagService_->reportError("PixelSupervisor::Stop: Cancelled the calib workloop.",DIAGINFO);
      calibWorkloop_->remove(calibJob_);
      diagService_->reportError("PixelSupervisor::Stop: Removed job from the calib workloop.",DIAGINFO);
    }
    else if ( state_ == "Done" )
    {
      Attribute_Vector parameters(1);
      parameters.at(0).name_="Move";
      Receive(msg, parameters);
      if (parameters.at(0).value_=="Yes") {

	// Move the result of the calibration into configuration

      }
    }
    else assert(0);

    delete theCalibAlgorithm_;
    theCalibAlgorithm_ = 0;
  }

  try {

  Supervisors::iterator i_PixelTKFECSupervisor;
  for (i_PixelTKFECSupervisor=PixelTKFECSupervisors_.begin();i_PixelTKFECSupervisor!=PixelTKFECSupervisors_.end();++i_PixelTKFECSupervisor) {
    std::string reply = Send(i_PixelTKFECSupervisor->second, "Stop");
    if (reply!= "StopDone") {
      diagService_->reportError("PixelTKFECSupervisor supervising crate #"+stringF(i_PixelTKFECSupervisor->first) + " could not be stopped.",DIAGERROR);
      *console_<<"PixelTKFECSupervisor supervising crate #"<<(i_PixelTKFECSupervisor->first)<<" could not be stopped."<<std::endl;
      response="StopFailed";
    }
  }

  Supervisors::iterator i_PixelFECSupervisor;
  for (i_PixelFECSupervisor=PixelFECSupervisors_.begin();i_PixelFECSupervisor!=PixelFECSupervisors_.end();++i_PixelFECSupervisor) {
    std::string reply = Send(i_PixelFECSupervisor->second, "Stop");
    if (reply!= "StopDone") {
      diagService_->reportError("PixelFECSupervisor supervising crate #"+stringF(i_PixelFECSupervisor->first) + " could not be stopped.",DIAGERROR);
      *console_<<"PixelFECSupervisor supervising crate #"<<(i_PixelFECSupervisor->first)<<" could not be stopped."<<std::endl;
      response="StopFailed";
    }
  }

  Supervisors::iterator i_PixelFEDSupervisor;
  for (i_PixelFEDSupervisor=PixelFEDSupervisors_.begin();i_PixelFEDSupervisor!=PixelFEDSupervisors_.end();++i_PixelFEDSupervisor) {
    std::string reply = Send(i_PixelFEDSupervisor->second, "Stop");
    if (reply!= "StopDone") {
      diagService_->reportError("PixelFEDSupervisor supervising crate #"+stringF(i_PixelFEDSupervisor->first) + " could not be stopped.",DIAGERROR);
      *console_<<"PixelFEDSupervisor supervising crate #"<<(i_PixelFEDSupervisor->first)<<" could not be stopped."<<std::endl;
      response="StopFailed";
    }
  }

  // TTCci control has no equivalent of a Stop transition. So we must do Halt then Configure.
  Supervisors::iterator i_PixelTTCSupervisor;
  if (useTTC_) {
    for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor) {
      std::string reply = Send(i_PixelTTCSupervisor->second, "stop"); // DO we have to also configure here?????????????????  d.k.
      if (reply != "stopResponse") {
        diagService_->reportError("PixelTTCSupervisor supervising crate #"+stringF(i_PixelTTCSupervisor->first) + " could not be stopped.",DIAGERROR);
        *console_<<"PixelTTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first) <<" could not be stopped: "<<reply<<std::endl;
        response="StopFailed";
      }
    }
  }
  else if (useTCDS_) {
    for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor) {
      std::string reply = Send(i_PixelTTCSupervisor->second, "Stop");
      if (reply != "StopResponse") {
        diagService_->reportError("PixelTTCSupervisor supervising crate #"+stringF(i_PixelTTCSupervisor->first) + " could not be configured after halting for stop transition.",DIAGERROR);
        *console_<<"PixelTTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first) <<" could not be configured after halting for stop transition: "<<reply<<std::endl;
        response="StopFailed";
      }
      std::string fsmState=Send(i_PixelTTCSupervisor->second, "QueryFSMState");
      statePixelTTCSupervisors_[i_PixelTTCSupervisor->first]=fsmState;
    }
  }



  if (useTCDS_) {
    Supervisors::iterator i_PixelLTCSupervisor;
    for (i_PixelLTCSupervisor=PixelLTCSupervisors_.begin();i_PixelLTCSupervisor!=PixelLTCSupervisors_.end();++i_PixelLTCSupervisor) {
      std::string reply = Send(i_PixelLTCSupervisor->second, "Stop");
      if (reply != "StopResponse") {
        diagService_->reportError("PixelLTCSupervisor supervising crate #"+stringF(i_PixelLTCSupervisor->first) + " could not be configured after halting for stop transition.",DIAGERROR);
        *console_<<"PixelLTCSupervisor supervising crate #"<<(i_PixelLTCSupervisor->first) <<" could not be configured after halting for stop transition: "<<reply<<std::endl;
        response="StopFailed";
      }
      std::string fsmState=Send(i_PixelLTCSupervisor->second, "QueryFSMState");
      statePixelLTCSupervisors_[i_PixelLTCSupervisor->first]=fsmState;
    }
  }



  if (!PixelSlinkMonitors_.empty()) {
    Supervisors::iterator i_PixelSlinkMonitor;
    for (i_PixelSlinkMonitor=PixelSlinkMonitors_.begin();i_PixelSlinkMonitor!=PixelSlinkMonitors_.end();++i_PixelSlinkMonitor) {
      std::string reply = Send(i_PixelSlinkMonitor->second, "Stop");
      if (reply!= "StopDone") {
	diagService_->reportError("PixelSlinkMonitor supervising crate #"+stringF(i_PixelSlinkMonitor->first) + " could not be stoppped!",DIAGERROR);
	*console_<<"PixelSlinkMonitor supervising crate #"<<(i_PixelSlinkMonitor->first)<<" could not be stopped!"<<std::endl;
      }
    }
  }
  } catch (xcept::Exception & err) {
    diagService_->reportError("Stop transition failed with exception: "+string(err.what()),DIAGERROR);
    response="StopFailed";
  }

  if (response=="StopDone") {
    fsmTransition("Stop");
    *console_<<"--------------------"<<std::endl;
  } else {
    fsmTransition("Failure");
    *console_<<"--- Stopping Failed ---"<<std::endl;
  }

  diagService_->reportError("Exiting transition STOP",DIAGINFO);
  return MakeSOAPMessageReference(response);

}

xoap::MessageReference PixelSupervisor::Pause (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  PixelTimer pausetimer;
  pausetimer.start();

  diagService_->reportError("Entering transition PAUSE",DIAGINFO);
  *console_<<"--- Pausing ---"<<std::endl;

  if (state_!="Running"&&state_!="RunningSoftErrorDetected" && state_!="RunningDegraded") {
    diagService_->reportError("Pause transition is not allowed from state: "+state_.toString(),DIAGWARN);
    return MakeSOAPMessageReference("PauseFailed");
  }

  std::string response="PauseDone";

  if( theCalibObject_!=0 )
    {
      calibWorkloop_->cancel();
      diagService_->reportError("PixelSupervisor::Pause: Calib workloop canceled for pausing",DIAGINFO);
    }

  try {

  Supervisors::iterator i_PixelTKFECSupervisor;
  for (i_PixelTKFECSupervisor=PixelTKFECSupervisors_.begin();i_PixelTKFECSupervisor!=PixelTKFECSupervisors_.end();++i_PixelTKFECSupervisor) {
    std::string reply = Send(i_PixelTKFECSupervisor->second, "Pause");
    if (reply!= "PauseDone") {
      diagService_->reportError("PixelTKFECSupervisor supervising crate #"+stringF(i_PixelTKFECSupervisor->first) + " could not be paused.",DIAGERROR);
      *console_<<"PixelTKFECSupervisor supervising crate #"<<(i_PixelTKFECSupervisor->first)<<" could not be paused."<<std::endl;
      response="PauseFailed";
    }
  }

  Supervisors::iterator i_PixelFECSupervisor;
  for (i_PixelFECSupervisor=PixelFECSupervisors_.begin();i_PixelFECSupervisor!=PixelFECSupervisors_.end();++i_PixelFECSupervisor) {
    std::string reply = Send(i_PixelFECSupervisor->second, "Pause");
    if (reply!= "PauseDone") {
      diagService_->reportError("PixelFECSupervisor supervising crate #"+stringF(i_PixelFECSupervisor->first) + " could not be paused.",DIAGERROR);
      *console_<<"PixelFECSupervisor supervising crate #"<<(i_PixelFECSupervisor->first)<<" could not be paused."<<std::endl;
      response="PauseFailed";
    }
  }

  Supervisors::iterator i_PixelFEDSupervisor;
  for (i_PixelFEDSupervisor=PixelFEDSupervisors_.begin();i_PixelFEDSupervisor!=PixelFEDSupervisors_.end();++i_PixelFEDSupervisor) {
    std::string reply = Send(i_PixelFEDSupervisor->second, "Pause");
    if (reply!= "PauseDone") {
      diagService_->reportError("PixelFEDSupervisor supervising crate #"+stringF(i_PixelFEDSupervisor->first) + " could not be paused.",DIAGERROR);
      *console_<<"PixelFEDSupervisor supervising crate #"<<(i_PixelFEDSupervisor->first)<<" could not be paused."<<std::endl;
      response="PauseFailed";
    }
  }

  Supervisors::iterator i_PixelTTCSupervisor;
  for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor) {
    std::string reply;
    if (useTTC_) {
      reply = Send(i_PixelTTCSupervisor->second, "suspend");
    }
    if (useTCDS_) {
      reply = Send(i_PixelTTCSupervisor->second, "Suspend");
    }
    if ((useTTC_ && reply!= "suspendResponse") || (useTCDS_ && reply!= "SuspendResponse")) {
      diagService_->reportError("PixelTTCSupervisor supervising crate #"+stringF(i_PixelTTCSupervisor->first) + " could not be paused.",DIAGERROR);
      *console_<<"PixelTTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first) <<" could not be paused."<<reply<<std::endl;
      response="PauseFailed";
    }
    if (useTCDS_) {
      std::string fsmState=Send(i_PixelTTCSupervisor->second, "QueryFSMState");
      statePixelTTCSupervisors_[i_PixelTTCSupervisor->first]=fsmState;
    }
  }


  if (useTCDS_) {
    Supervisors::iterator i_PixelLTCSupervisor;
    for (i_PixelLTCSupervisor=PixelLTCSupervisors_.begin();i_PixelLTCSupervisor!=PixelLTCSupervisors_.end();++i_PixelLTCSupervisor) {
      std::string reply = Send(i_PixelLTCSupervisor->second, "Suspend");

      if (reply!= "SuspendResponse") {
        diagService_->reportError("PixelLTCSupervisor supervising crate #"+stringF(i_PixelLTCSupervisor->first) + " could not be paused.",DIAGERROR);
        *console_<<"PixelLTCSupervisor supervising crate #"<<(i_PixelLTCSupervisor->first) <<" could not be paused."<<reply<<std::endl;
        response="PauseFailed";
      }

      std::string fsmState=Send(i_PixelLTCSupervisor->second, "QueryFSMState");
      statePixelLTCSupervisors_[i_PixelLTCSupervisor->first]=fsmState;
    }
  }



  if (!PixelSlinkMonitors_.empty()) {
    Supervisors::iterator i_PixelSlinkMonitor;
    for (i_PixelSlinkMonitor=PixelSlinkMonitors_.begin();i_PixelSlinkMonitor!=PixelSlinkMonitors_.end();++i_PixelSlinkMonitor) {
      std::string reply = Send(i_PixelSlinkMonitor->second, "Pause");
      if (reply!= "PauseDone") {
	diagService_->reportError("PixelSlinkMonitor supervising crate #"+stringF(i_PixelSlinkMonitor->first) + " could not be paused!",DIAGERROR);
	*console_<<"PixelSlinkMonitor supervising crate #"<<(i_PixelSlinkMonitor->first)<<" could not be paused!"<<std::endl;
      }
    }
  }

  } catch (xcept::Exception & e) {
    diagService_->reportError("Pause failed with exception: "+string(e.what()),DIAGERROR);
    response="PauseFailed";
  }

  if (response=="PauseDone") {
    fsmTransition("Pause");
    *console_<<"--------------------"<<std::endl;
  } else { // Do not advance to next state
    fsmTransition("Failure");
    *console_<<"--- Pausing Failed! ---"<<std::endl;
  }

  pausetimer.stop();
  diagService_->reportError("Exiting transition PAUSE "+stringF(pausetimer.tottime()),DIAGINFO);
  return MakeSOAPMessageReference(response);
}

xoap::MessageReference PixelSupervisor::Resume (xoap::MessageReference msg) throw (xoap::exception::Exception)
{

  diagService_->reportError("Entering transition RESUME",DIAGINFO);
  *console_<<"--- Resuming ---"<<std::endl;

  if (state_!="Paused") {
    diagService_->reportError("Resume transition is not allowed from state: "+state_.toString(),DIAGWARN);
    return MakeSOAPMessageReference("ResumeFailed");
  }
  std::string response="ResumeDone";

  try {

  Supervisors::iterator i_PixelTKFECSupervisor;
  for (i_PixelTKFECSupervisor=PixelTKFECSupervisors_.begin();i_PixelTKFECSupervisor!=PixelTKFECSupervisors_.end();++i_PixelTKFECSupervisor) {
    std::string reply = Send(i_PixelTKFECSupervisor->second, "Resume");
    if (reply!= "ResumeDone") {
      diagService_->reportError("PixelTKFECSupervisor supervising crate #"+stringF(i_PixelTKFECSupervisor->first) + " could not be resumed.",DIAGERROR);
      *console_<<"PixelTKFECSupervisor supervising crate #"<<(i_PixelTKFECSupervisor->first)<<" could not be resumed."<<std::endl;
      response="ResumeFailed";
    }
  }

  Supervisors::iterator i_PixelFECSupervisor;
  for (i_PixelFECSupervisor=PixelFECSupervisors_.begin();i_PixelFECSupervisor!=PixelFECSupervisors_.end();++i_PixelFECSupervisor) {
    std::string reply = Send(i_PixelFECSupervisor->second, "Resume");
    if (reply!= "ResumeDone") {
      diagService_->reportError("PixelFECSupervisor supervising crate #"+stringF(i_PixelFECSupervisor->first) + " could not be resumed.",DIAGERROR);
      *console_<<"PixelFECSupervisor supervising crate #"<<(i_PixelFECSupervisor->first)<<" could not be resumed."<<std::endl;
      response="ResumeFailed";
    }
  }

  Supervisors::iterator i_PixelFEDSupervisor;
  for (i_PixelFEDSupervisor=PixelFEDSupervisors_.begin();i_PixelFEDSupervisor!=PixelFEDSupervisors_.end();++i_PixelFEDSupervisor) {
    std::string reply = Send(i_PixelFEDSupervisor->second, "Resume");
    if (reply!= "ResumeDone") {
      diagService_->reportError("PixelFEDSupervisor supervising crate #"+stringF(i_PixelFEDSupervisor->first) + " could not be resumed.",DIAGERROR);
      *console_<<"PixelFEDSupervisor supervising crate #"<<(i_PixelFEDSupervisor->first)<<" could not be resumed."<<std::endl;
      response="ResumeFailed";
    }
  }

  Supervisors::iterator i_PixelTTCSupervisor;
  for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor) {
    std::string reply;
    if (useTTC_) {
      reply = Send(i_PixelTTCSupervisor->second, "enable");
    }
    if (useTCDS_) {
      reply = Send(i_PixelTTCSupervisor->second, "Resume");
    }
    if ((useTTC_ && reply!= "enableResponse") | (useTCDS_ && reply!= "ResumeResponse")) {
      diagService_->reportError("PixelTTCSupervisor supervising crate #"+stringF(i_PixelTTCSupervisor->first) + " could not be resumed.",DIAGERROR);
      *console_<<"PixelTTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first) <<" could not be resumed: "<<reply<<std::endl;
      response="ResumeFailed";
    }
    if (useTCDS_) {
      std::string fsmState=Send(i_PixelTTCSupervisor->second, "QueryFSMState");
      statePixelTTCSupervisors_[i_PixelTTCSupervisor->first]=fsmState;
    }
  }

  if (useTCDS_) {
    Supervisors::iterator i_PixelLTCSupervisor;
    for (i_PixelLTCSupervisor=PixelLTCSupervisors_.begin();i_PixelLTCSupervisor!=PixelLTCSupervisors_.end();++i_PixelLTCSupervisor) {
      std::string reply = Send(i_PixelLTCSupervisor->second, "Resume");
      if (reply!= "ResumeResponse") {
        diagService_->reportError("PixelLTCSupervisor supervising crate #"+stringF(i_PixelLTCSupervisor->first) + " could not be resumed.",DIAGERROR);
        *console_<<"PixelLTCSupervisor supervising crate #"<<(i_PixelLTCSupervisor->first) <<" could not be resumed: "<<reply<<std::endl;
        response="ResumeFailed";
      }
      std::string fsmState=Send(i_PixelLTCSupervisor->second, "QueryFSMState");
      statePixelLTCSupervisors_[i_PixelLTCSupervisor->first]=fsmState;
    }
  }


  if (!PixelSlinkMonitors_.empty()) {
    Supervisors::iterator i_PixelSlinkMonitor;
    for (i_PixelSlinkMonitor=PixelSlinkMonitors_.begin();i_PixelSlinkMonitor!=PixelSlinkMonitors_.end();++i_PixelSlinkMonitor) {
      std::string reply = Send(i_PixelSlinkMonitor->second, "Resume");
      if (reply!= "ResumeDone") {
	diagService_->reportError("PixelSlinkMonitor supervising crate #"+stringF(i_PixelSlinkMonitor->first) + " could not be resumed!",DIAGERROR);
	*console_<<"PixelSlinkMonitor supervising crate #"<<(i_PixelSlinkMonitor->first)<<" could not be resumed!"<<std::endl;
      }
    }
  }
  } catch (xcept::Exception & e) {
    diagService_->reportError("Resume failed with exception: "+string(e.what()),DIAGERROR);
    response="ResumeFailed";
  }

  if ( theCalibObject_!=0 )
  {
    calibWorkloop_->activate();
    diagService_->reportError("PixelSupervisor::Resume: Activated workloop again.",DIAGINFO);
  }

  if (response=="ResumeDone") {
    fsmTransition("Resume");
    *console_<<"--------------------"<<std::endl;
  } else {
    fsmTransition("Failure");
    *console_<<"--- Resuming Failed! ---"<<std::endl;
  }

  diagService_->reportError("Exiting transition RESUME",DIAGINFO);
  return MakeSOAPMessageReference(response);
}

xoap::MessageReference PixelSupervisor::Done (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  std::string response="DoneDone";

  if ( state_ == "Done" ) return MakeSOAPMessageReference(response);

  assert( state_ == "Running" || state_ == "Paused" );

  diagService_->reportError("PixelSupervisor::Done: workloop active: "+stringF(calibWorkloop_->isActive())+", workloop type: "+calibWorkloop_->getType(),DIAGINFO);

  if ( state_ == "Running" ) // state_ is the previous state
  {
    calibWorkloop_->cancel();
    diagService_->reportError("PixelSupervisor::Done:  Calib workloop cancelled." ,DIAGINFO);
  }

  calibWorkloop_->remove(calibJob_);

  diagService_->reportError("PixelSupervisor::Done: Removed the calib job from the workloop." ,DIAGINFO);
  if (response=="DoneDone") {
    try {
      toolbox::Event::Reference e(new toolbox::Event("Done", this));
      fsm_.fireEvent(e);
    } catch (toolbox::fsm::exception::Exception & e) {
      XCEPT_RETHROW(xoap::exception::Exception, "Invalid Command.", e);
    }
    *console_<<"--------------------"<<std::endl;
  } else {
    *console_<<"--- Going to Done Failed! ---"<<std::endl;
  }

  return MakeSOAPMessageReference(response);
}

xoap::MessageReference PixelSupervisor::PrepareTTSTestMode (xoap::MessageReference msg) throw (xoap::exception::Exception)
{

  diagService_->reportError("Entering transition PrepareTTSTestMode",DIAGINFO);
  *console_<<"--- Entering PreparingTTSTestMode ---"<<std::endl;

  std::string response="PrepareTTSTestModeDone";

  Supervisors::iterator i_PixelFEDSupervisor;
  for (i_PixelFEDSupervisor=PixelFEDSupervisors_.begin();i_PixelFEDSupervisor!=PixelFEDSupervisors_.end();++i_PixelFEDSupervisor) {
    std::string reply = Send(i_PixelFEDSupervisor->second, "PrepareTTSTestMode");
    if (reply!= "PrepareTTSTestModeDone") {
      diagService_->reportError("PixelFEDSupervisor supervising crate #"+stringF(i_PixelFEDSupervisor->first) + " could not be prepared for TTS Test Mode.",DIAGERROR);
      *console_<<"PixelFEDSupervisor supervising crate #"<<(i_PixelFEDSupervisor->first)<<" could not be prepared for TTS Test Mode!"<<std::endl;
      response="PrepareTTSTestModeFailed";
    }
  }

  if (response=="PrepareTTSTestModeDone") {
    try {
      toolbox::Event::Reference e(new toolbox::Event("PrepareTTSTestMode", this));
      fsm_.fireEvent(e);
    } catch (toolbox::fsm::exception::Exception & e) {
      XCEPT_RETHROW(xoap::exception::Exception, "Invalid Command.", e);
    }
    *console_<<"-------------------"<<std::endl;
  } else {
    *console_<<"--- Preparing TTS Test Mode Failed! ---"<<std::endl;
  }

  diagService_->reportError("Exiting transition PrepareTTSTestMode",DIAGINFO);
  *console_<<"--- Exiting PreparingTTSTestMode ---"<<std::endl;
  return MakeSOAPMessageReference(response);

}

xoap::MessageReference PixelSupervisor::TestTTS (xoap::MessageReference msg) throw (xoap::exception::Exception)
{

  diagService_->reportError("Entering transition TestTTS",DIAGINFO);
  *console_<<"--- Entering transition TestTTS ---"<<std::endl;
  std::string response="TestTTS";

  Attribute_Vector parameters(4);
  parameters[0].name_="TTS_TEST_FED_ID";
  parameters[1].name_="TTS_TEST_TYPE";
  parameters[2].name_="TTS_TEST_PATTERN";
  parameters[3].name_="TTS_TEST_SEQUENCE_REPEAT";
  Receive(msg, parameters);

  Supervisors::iterator i_PixelFEDSupervisor;
  for (i_PixelFEDSupervisor=PixelFEDSupervisors_.begin();i_PixelFEDSupervisor!=PixelFEDSupervisors_.end();++i_PixelFEDSupervisor) {
    std::string reply = Send(i_PixelFEDSupervisor->second, "TestTTS", parameters);
    if (reply!= "TestTTSDone") {
      diagService_->reportError("PixelFEDSupervisor supervising crate #"+stringF(i_PixelFEDSupervisor->first) + " could not be TTS tested.",DIAGERROR);
      *console_<<"PixelFEDSupervisor supervising crate #"<<(i_PixelFEDSupervisor->first)<<" could not be TTS tested!"<<std::endl;
      response="TestTTSFailed";
    }
  }

  if (response=="TestTTS") {
    try {
      toolbox::Event::Reference e(new toolbox::Event("TestTTS", this));
      fsm_.fireEvent(e);
    } catch (toolbox::fsm::exception::Exception & e) {
      XCEPT_RETHROW(xoap::exception::Exception, "Invalid Command.", e);
    }
    *console_<<"-------------------"<<std::endl;
    diagService_->reportError("-------------------",DIAGINFO);
  } else {
    *console_<<"--- Testing TTS Failed! ---"<<std::endl;
    diagService_->reportError("--- Testing TTS Failed! ---",DIAGINFO);

  }

  diagService_->reportError("Exiting transition TestTTS",DIAGINFO);
  *console_<<"--- Exiting TestTTS ---"<<std::endl;
  diagService_->reportError("--- Exiting TestTTS ---",DIAGINFO);
  return MakeSOAPMessageReference(response);

}

xoap::MessageReference PixelSupervisor::FSMStateRequest (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  return MakeSOAPMessageReference(state_);
}

void PixelSupervisor::enteringError (toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception)
{
  toolbox::fsm::FailedEvent & fe = dynamic_cast<toolbox::fsm::FailedEvent&>(*e);
  ostringstream errstr;
  errstr<<"Failure performing transition from: "
	<< fe.getFromState()
	<<  " to: "
	<< fe.getToState()
	<< " exception: " << fe.getException().what();
  diagService_->reportError(errstr.str(),DIAGERROR);

}

void PixelSupervisor::stateInitial (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception)
{
  diagService_->reportError("--- PixelSupervisor is in its Initial state ---",DIAGINFO);
  state_ = fsm_.getStateName (fsm_.getCurrentState());

  diagService_->reportError("PixelSupervisor::stateInitial: workloop active: "+stringF(calibWorkloop_->isActive())+", workloop type: "+calibWorkloop_->getType(),DIAGINFO);

}

void PixelSupervisor::stateResetting (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception)
{
  state_ = fsm_.getStateName (fsm_.getCurrentState());
}

void PixelSupervisor::statePaused (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception)
{
  state_ = fsm_.getStateName (fsm_.getCurrentState());
  try {
    rcmsStateNotifier_.stateChanged("Paused", "");
  }
  catch(xcept::Exception &e) {
    diagService_->reportError("Failed to notify state change : "+ xcept::stdformat_exception_history(e),DIAGERROR);
  }

  diagService_->reportError("PixelSupervisor::statePaused: workloop active: "+stringF(calibWorkloop_->isActive())+", workloop type: "+calibWorkloop_->getType(),DIAGINFO);
}

void PixelSupervisor::stateRunning (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception)
{

  state_ = fsm_.getStateName (fsm_.getCurrentState());
  try {
    rcmsStateNotifier_.stateChanged("Running", "");
  }
  catch(xcept::Exception &e) {
    diagService_->reportError("Failed to notify state change : "+ xcept::stdformat_exception_history(e),DIAGERROR);
  }

  diagService_->reportError("PixelSupervisor::stateRunning: workloop active: "+stringF(calibWorkloop_->isActive())+", workloop type: "+calibWorkloop_->getType(),DIAGINFO);

}

void PixelSupervisor::stateDone (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception)
{
  if ( state_ == "Running" ) // state_ is the previous state
  {
    if ( calibWorkloop_->isActive() ) calibWorkloop_->cancel();
    diagService_->reportError("PixelSupervisor::stateDone:  Calib workloop cancelled.",DIAGINFO);
  }

  state_ = fsm_.getStateName (fsm_.getCurrentState());

  diagService_->reportError("PixelSupervisor::stateDone: workloop active: "+stringF(calibWorkloop_->isActive())+", workloop type: "+calibWorkloop_->getType(),DIAGINFO);
}

void PixelSupervisor::stateHalted (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception)
{
  runNumber_="";
  runType_="";
  percentageConfigured_=0.0;
  aliasesAndKeys_=PixelConfigInterface::getAliases();
  diagService_->reportError("PixelSupervisor::stateHalted: aliases and keys reloaded",DIAGINFO);
  state_ = fsm_.getStateName (fsm_.getCurrentState());
  diagService_->reportError("PixelSupervisor::stateHalted: workloop active: "+stringF(calibWorkloop_->isActive())+", workloop type: "+calibWorkloop_->getType(),DIAGINFO);

  //(hopefully) make RCMS aware of successful Recovery
  //need to test that: (a) this works and (b) this does not break anything (regular Halt transition)
  //update -- condition (b) seems to be satisfied. not yet sure about condition (a)
  try {
    rcmsStateNotifier_.stateChanged("Halted", "");
  }
  catch(xcept::Exception &e)
  {
    diagService_->reportError("Failed to notify state change : "+ xcept::stdformat_exception_history(e),DIAGERROR);
  }

}

void PixelSupervisor::stateConfiguring (toolbox::fsm::FiniteStateMachine & fsm)
{

  // this is all in one function, but practically one supervisor is configured after the other

  PixelTimer GlobalTimer;
  if (extratimers_) {
    GlobalTimer.setVerbose(true); GlobalTimer.setName("PixelSupervisor::stateConfiguring");
    GlobalTimer.start("stateConfiguring start");
  }

  // Update the state_ member data so that Infospace may publish this information
  // Advertize more on webpage, console etc
  state_=fsm_.getStateName(fsm_.getCurrentState());
  diagService_->reportError("PixelSupervisor::stateConfiguring: Entering",DIAGINFO);
  *console_<<"PixelSupervisor::stateConfiguring: Entering"<<std::endl;

  // We're going to time various aspects of configuration
  PixelTimer FEDConfigureTime;
  PixelTimer FECConfigureTime;
  PixelTimer TTCConfigureTime;

  // These are parameters sets which will be sent to the underlying Supervisors
  Attribute_Vector parametersToTKFEC(1),
                   parametersToFEC(1),
                   parametersToFED(1),
                   parametersToLTC(1),
                   parametersToDCSFSM(1);

  unsigned int globalKey=theGlobalKey_->key();
  parametersToFEC[0].name_="GlobalKey";	  parametersToFEC[0].value_=itoa(globalKey);
  parametersToFED[0].name_="GlobalKey";   parametersToFED[0].value_=itoa(globalKey);
  parametersToTKFEC[0].name_="GlobalKey"; parametersToTKFEC[0].value_=itoa(globalKey);
  parametersToLTC[0].name_="GlobalKey";   parametersToLTC[0].value_=itoa(globalKey);
  parametersToDCSFSM[0].name_="GlobalKey";	  parametersToDCSFSM[0].value_=itoa(globalKey);

  try { //posting SOAP, etc

    //configure PixelDCSFSMInterface
    if (!PixelDCSFSMInterface_.empty() ) {
      diagService_->reportError("[PixelSupervisor::stateConfiguring] PixelDCSFSMInterface exists...",DIAGDEBUG);
      Supervisors::iterator i_PixelDCSFSMInterface;
      for (i_PixelDCSFSMInterface=PixelDCSFSMInterface_.begin(); i_PixelDCSFSMInterface!=PixelDCSFSMInterface_.end(); ++i_PixelDCSFSMInterface) {
        unsigned int instance = i_PixelDCSFSMInterface->first;

        diagService_->reportError("[PixelSupervisor::stateConfiguring] DCSFSMInterface instance "+stringF(instance)+" has state "+statePixelDCSFSMInterface_[instance],DIAGINFO);

        std::string fsmState=Send(i_PixelDCSFSMInterface->second, "FSMStateRequest");

        if (statePixelDCSFSMInterface_[instance]=="Halted") {
          statePixelDCSFSMInterface_[instance]="Configuring";
          std::string reply = Send(i_PixelDCSFSMInterface->second, "Configure", parametersToDCSFSM);
          if (reply!="ConfigureDone") {
            XCEPT_RAISE (xdaq::exception::Exception,"PixelDCSFSMInterface configuration failure");
          }
        }
        else if (statePixelDCSFSMInterface_[instance]=="Initial") {
          diagService_->reportError("[PixelSupervisor::stateConfiguring] PixelDCSFSMInterface is stuck in the Initial state!",DIAGERROR);
        }
        if (statePixelDCSFSMInterface_[instance]!="Configured") { //it is not done yet
          diagService_->reportError("[PixelSupervisor::stateConfiguring] DCS Interface not configured yet",DIAGINFO);
          return;
        }
      }
    }


  if (extratimers_) {
    GlobalTimer.stop("got GlobalKey");
    GlobalTimer.start("Configure TTC");
  }

  bool proceed = true;
  bool configuredTTCs = true;
  bool configuredLTCs = true;


  Supervisors::iterator i_PixelTTCSupervisor;
  for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor) {
    // first check that PixelTTCSupervisors are not already configured, configuring only allowed from halted
    if (statePixelTTCSupervisors_[i_PixelTTCSupervisor->first] != "Configured") {
      // update state for TCDS, old TTC system updates the PixelSupervisor automatically
      if (useTCDS_) {
        std::string fsmState=Send(i_PixelTTCSupervisor->second, "QueryFSMState");
        statePixelTTCSupervisors_[i_PixelTTCSupervisor->first]= fsmState;
      }
    }
    if (statePixelTTCSupervisors_[i_PixelTTCSupervisor->first] != "Configured") {
      configuredTTCs = false;
    }
  }


  if (useTCDS_) {

    Supervisors::iterator i_PixelLTCSupervisor;
    for (i_PixelLTCSupervisor=PixelLTCSupervisors_.begin();i_PixelLTCSupervisor!=PixelLTCSupervisors_.end();++i_PixelLTCSupervisor) {
      // first check that PixelLTCSupervisors are not already configured, configuring only allowed from halted
      if (statePixelLTCSupervisors_[i_PixelLTCSupervisor->first] != "Configured") {
        // update state for TCDS, old LTC system updates the PixelSupervisor automatically
        std::string fsmState=Send(i_PixelLTCSupervisor->second, "QueryFSMState");
        statePixelLTCSupervisors_[i_PixelLTCSupervisor->first]= fsmState;
        std::cout << "the LTC FSM state is: "  << fsmState << std::endl;
      }
      if (statePixelLTCSupervisors_[i_PixelLTCSupervisor->first] != "Configured") {
        configuredLTCs = false;
      }
    }
  }


  if (!configuredTTCs) {

    if (!PixelTTCSupervisors_.empty()) {
      // Send a SOAP message to PixelTTCSupervisor
      //if(useTTC_){
        SendConfigurationToTTC();
        //}
      diagService_->reportError("PixelSupervisor::stateConfiguring: Sending SOAP messages to PixelTTCSupervisors",DIAGINFO);

      TTCConfigureTime.start();
      for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor) {
        // Configuring only allowed from halted
        if (statePixelTTCSupervisors_[i_PixelTTCSupervisor->first] == "Halted") {
          std::string reply;
          if (useTTC_)
            reply = Send(i_PixelTTCSupervisor->second, "configure");
          if (useTCDS_) {
            statePixelTTCSupervisors_[i_PixelTTCSupervisor->first]="Configuring";
            reply = Send(i_PixelTTCSupervisor->second, "Configure");
            *console_<< "Received reply from PixelTTCSupervisor #" << stringF(i_PixelTTCSupervisor->first) << ": " << reply << std::endl;
          }
          if ((useTTC_ && reply != "configureResponse") || (useTCDS_ && reply != "ConfigureResponse")) {
            *console_ << "PixelTTCSupervisor supervising crate #" << (i_PixelTTCSupervisor->first) << " could not be configured: " << reply << std::endl;
            XCEPT_RAISE (xdaq::exception::Exception,"PixelTTCSupervisor configuration failure");
          }
        }
      }
      TTCConfigureTime.stop();
    }
    else {
      diagService_->reportError("[PixelSupervisor::stateConfiguring] no TTCSupervisors exist...",DIAGFATAL);
    }
    proceed = false; // do not continue with other supervisors before TTCSupervisors are not configured
  }
  if (extratimers_) {
    GlobalTimer.stop("TTC configured");
    GlobalTimer.start("Configure TKFEC");
  }



  if (!configuredLTCs && useTCDS_) {
    if (!PixelLTCSupervisors_.empty()) {
      // Send a SOAP message to PixelLTCSupervisor
      SendConfigurationToLTC();

      diagService_->reportError("PixelSupervisor::stateConfiguring: Sending SOAP messages to PixelLTCSupervisors",DIAGINFO);
      Supervisors::iterator i_PixelLTCSupervisor;
      for (i_PixelLTCSupervisor=PixelLTCSupervisors_.begin();i_PixelLTCSupervisor!=PixelLTCSupervisors_.end();++i_PixelLTCSupervisor) {
        // Configuring only allowed from halted
        if (statePixelLTCSupervisors_[i_PixelLTCSupervisor->first] == "Halted") {
          std::string reply;

          statePixelLTCSupervisors_[i_PixelLTCSupervisor->first]="Configuring";
          reply = Send(i_PixelLTCSupervisor->second, "Configure");
          *console_<< "Received reply from PixelLTCSupervisor #" << stringF(i_PixelLTCSupervisor->first) << ": " << reply << std::endl;

          if (reply != "ConfigureResponse") {
            *console_ << "PixelLTCSupervisor supervising crate #" << (i_PixelLTCSupervisor->first) << " could not be configured: " << reply << std::endl;
            XCEPT_RAISE (xdaq::exception::Exception,"PixelLTCSupervisor configuration failure");
          }
        }
      }
    }
    else {
      diagService_->reportError("[PixelSupervisor::stateConfiguring] no LTCSupervisors exist...",DIAGFATAL);
    }
    proceed = false; // do not continue with other supervisors before LTCSupervisors are not configured
  }





  // Send a SOAP message to PixelTKFECSupervisor to configure
  // Also time the procedure
  bool configuredTKFECs=true;

  if (!PixelTKFECSupervisors_.empty()) {
    diagService_->reportError("[PixelSupervisor::stateConfiguring] TKFECs exist...",DIAGINFO);
    Supervisors::iterator i_PixelTKFECSupervisor;
    for (i_PixelTKFECSupervisor=PixelTKFECSupervisors_.begin();i_PixelTKFECSupervisor!=PixelTKFECSupervisors_.end();++i_PixelTKFECSupervisor) {
      unsigned int instance=i_PixelTKFECSupervisor->first;

      diagService_->reportError("[PixelSupervisor::stateConfiguring] TKFECs instance "+stringF(instance)+" has state "+statePixelTKFECSupervisors_[instance],DIAGINFO);

      if (statePixelTKFECSupervisors_[instance]=="Halted") {

        diagService_->reportError("PixelSupervisor::stateConfiguring: Sending SOAP messages to PixelTKFECSupervisor "+stringF(instance),DIAGINFO);

        statePixelTKFECSupervisors_[instance]="Configuring";
        std::string reply = Send(i_PixelTKFECSupervisor->second, "Configure", parametersToTKFEC);
        if (reply!= "ConfigureDone") {
          *console_<<"PixelTKFECSupervisor supervising crate #"<<(i_PixelTKFECSupervisor->first)<<" could not be configured."<<std::endl;
          XCEPT_RAISE (xdaq::exception::Exception,"PixelTKFECSupervisor configuration failure, instance="+string(itoa(instance)));
        }
      }
      if (statePixelTKFECSupervisors_[instance]!="Configured") {
        diagService_->reportError("[PixelSupervisor::stateConfiguring] TKFEC not configured yet",DIAGINFO);
        configuredTKFECs=false;
      }
    }
  }

  if (!configuredTKFECs) return;

  if (extratimers_) {
    GlobalTimer.stop("TKFEC configured");
    GlobalTimer.start("Configure PxlFEC");
  }

  // Send a SOAP message to PixelFECSupervisor
  if (!PixelFECSupervisors_.empty()) {
    diagService_->reportError("[PixelSupervisor::stateConfiguring] FECs exist...",DIAGINFO);
    Supervisors::iterator i_PixelFECSupervisor;
    for (i_PixelFECSupervisor=PixelFECSupervisors_.begin();i_PixelFECSupervisor!=PixelFECSupervisors_.end();++i_PixelFECSupervisor) {
      unsigned int instance=i_PixelFECSupervisor->first;
      std::string fsmState=statePixelFECSupervisors_[instance];

      diagService_->reportError("[PixelSupervisor::stateConfiguring] FEC instance "+stringF(instance)+" has state "+fsmState,DIAGINFO);

      if (fsmState=="Halted") {
        FECConfigureTime.start();

        diagService_->reportError("PixelSupervisor::stateConfiguring: Sending SOAP messages to PixelFECSupervisor "+instance,DIAGINFO);

        statePixelFECSupervisors_[instance]="Configuring";
	      //don't actually have to send the global key here anymore, but it doesn't hurt either
        std::string reply = Send(i_PixelFECSupervisor->second, "Configure", parametersToFEC);
        FECConfigureTime.stop();
        if (reply!= "ConfigureDone") {
          *console_<<"PixelFECSupervisor supervising crate #"<<stringF(instance)<<" could not be configured."<<std::endl;
          XCEPT_RAISE (xdaq::exception::Exception,"PixelFECSupervisor configuration failure, instance="+string(itoa(instance)));
        }
      }
      if (fsmState!="Configured") {

        diagService_->reportError("[PixelSupervisor::stateConfiguring] Cannot proceed to Configured because PixelFECSupervisor instance "+stringF(instance)+" is not Configured yet.",DIAGINFO);
        proceed=false;
      }
    }
  }
  else {
    diagService_->reportError("[PixelSupervisor::stateConfiguring] no FECs exist...",DIAGFATAL);
  }

  // Pixel FEDs can only configure once clock is present
  if (configuredTTCs) {

    // Send a SOAP message to PixelFEDSupervisor
    if (!PixelFEDSupervisors_.empty()) {

      diagService_->reportError("[PixelSupervisor::stateConfiguring] FEDs exist...",DIAGINFO);
      Supervisors::iterator i_PixelFEDSupervisor;
      for (i_PixelFEDSupervisor=PixelFEDSupervisors_.begin();i_PixelFEDSupervisor!=PixelFEDSupervisors_.end();++i_PixelFEDSupervisor) {
        unsigned int instance=i_PixelFEDSupervisor->first;
        std::string fsmState=statePixelFEDSupervisors_[instance];

        diagService_->reportError("[PixelSupervisor::stateConfiguring] FED instance "+stringF(instance)+" has state "+fsmState,DIAGINFO);

        if (fsmState=="Halted") {
          FEDConfigureTime.start();

  	      diagService_->reportError("PixelSupervisor::stateConfiguring: Sending SOAP messages to PixelFEDSupervisor "+stringF(instance),DIAGINFO);

          statePixelFEDSupervisors_[instance]="Configuring";
          std::string reply = Send(i_PixelFEDSupervisor->second, "Configure", parametersToFED);
          FEDConfigureTime.stop();
          if (reply!= "ConfigureDone") {
            *console_<<"PixelFEDSupervisor supervising crate #"<<(i_PixelFEDSupervisor->first)<<" could not be configured."<<std::endl;
  	        XCEPT_RAISE (xdaq::exception::Exception,"PixelFEDSupervisor configuration failure, instance="+string(itoa(instance)));
          } else {
  	        diagService_->reportError("PixelSupervisor::stateConfiguring: Received SOAP reply from PixelFEDSupervisors",DIAGINFO);
          }
        }

        if (fsmState!="Configured") {
  	      diagService_->reportError("[PixelSupervisor::stateConfiguring] Cannot proceed to Configured because PixelFEDSupervisor instance "+stringF(instance)+" is not Configured yet.",DIAGINFO);
          proceed=false;
        }
      }
    }
    else {
      diagService_->reportError("[PixelSupervisor::stateConfiguring] no FEDs exist...",DIAGFATAL);
    }
  }
  else {
    if (!PixelFEDSupervisors_.empty()) {
      diagService_->reportError("[PixelSupervisor::stateConfiguring] no TTCs exist, cannot configure FEDs!",DIAGFATAL);
      XCEPT_RAISE (xdaq::exception::Exception,"Cannot configure PixelFEDSupervisors without TTCs!");
    }
  }


  // Send a SOAP message to PixelDCStoTrkFECDpInterface
  if (!PixelDCStoTrkFECDpInterface_.empty()) {

    Supervisors::iterator i_PixelDCStoTrkFECDpInterface;
    for (i_PixelDCStoTrkFECDpInterface=PixelDCStoTrkFECDpInterface_.begin(); i_PixelDCStoTrkFECDpInterface!=PixelDCStoTrkFECDpInterface_.end(); ++i_PixelDCStoTrkFECDpInterface) {
      if (  statePixelDCStoTrkFECDpInterface_[i_PixelDCStoTrkFECDpInterface->first] == "Halted" ) {

	      diagService_->reportError("PixelSupervisor::stateConfiguring: Sending SOAP message to PixelDCStoTrkFECDpInterface",DIAGINFO);
        std::string reply = Send(i_PixelDCStoTrkFECDpInterface->second, "Configure");
        //at the moment this class never returns anything other than ConfigureDone...so no need to look at the reply
        cout<<"done sending soap to PixelDCStoTrkFECDpInterface"<<endl;
        statePixelDCStoTrkFECDpInterface_[i_PixelDCStoTrkFECDpInterface->first] = "Configuring";
      }
      else if (  statePixelDCStoTrkFECDpInterface_[i_PixelDCStoTrkFECDpInterface->first] == "Configuring" ) {
        proceed=false;
        cout<<"PixelDCStoTrkFECDpInterface is configuring"<<endl;

      }
      //if the state is Configured, then we are good to go
    }
  }

  if (proceed) {
    if (extratimers_) {
      GlobalTimer.stop();
      GlobalTimer.start("Configure SLinkMonitor and DCStoTrkFECDpInterface");
    }

    // Send a SOAP message to PixelSlinkMonitor
    if (!PixelSlinkMonitors_.empty()) {
      diagService_->reportError("PixelSupervisor::stateConfiguring: Sending SOAP messages to PixelSlinkMonitor",DIAGINFO);
      Supervisors::iterator i_PixelSlinkMonitor;
      for (i_PixelSlinkMonitor=PixelSlinkMonitors_.begin();i_PixelSlinkMonitor!=PixelSlinkMonitors_.end();++i_PixelSlinkMonitor) {
        std::string reply = Send(i_PixelSlinkMonitor->second, "Configure");
        if (reply!= "ConfigureDone") {
          *console_<<"PixelFEDSupervisor supervising crate #"<<(i_PixelSlinkMonitor->first)<<" could not be configured."<<std::endl;
	  XCEPT_RAISE (xdaq::exception::Exception,"PixelSlinkMonitor configuration failure");
        }
      }
    }

    // Advertize the timing results

    diagService_->reportError("PixelSupervisor::stateConfiguration: TTC configuration time  :"+stringF(TTCConfigureTime.tottime()),DIAGINFO);
    //    diagService_->reportError("PLEASE IGNORE: PixelSupervisor::stateConfiguration: FED configuration time  :"+stringF(FEDConfigureTime.tottime()),DIAGINFO);
    //    diagService_->reportError("PLEASE IGNORE: PixelSupervisor::stateConfiguration: FEC configuration time  :"+stringF(FECConfigureTime.tottime()),DIAGINFO);

    try {
      toolbox::Event::Reference e(new toolbox::Event("ConfiguringDone", this));
      fsm_.fireEvent(e);
    } catch (toolbox::fsm::exception::Exception & e) {
      diagService_->reportError("[PixelSupervisor::stateConfiguring] Invalid command "+stringF(e.what()),DIAGERROR);
    }

  }

  } catch (xdaq::exception::Exception & e) {
    diagService_->reportError("[PixelSupervisor::stateConfiguring] Failed to configure with exception: "+string(e.what()),DIAGERROR);
    try {
      toolbox::Event::Reference ev(new toolbox::Event("Failure", this));
      fsm_.fireEvent(ev);
    } catch (...) {
      diagService_->reportError("[PixelSupervisor::stateConfiguring] Failed to transfer FSM to Error state",DIAGFATAL);
    }

  }

  if (extratimers_)  GlobalTimer.stop("done with fireEvent");


  // Advertize the exiting of this method
  diagService_->reportError("PixelSupervisor::stateConfiguring: Exiting",DIAGINFO);
  *console_<<"PixelSupervisor::stateConfiguring: Exiting"<<std::endl;


}

void PixelSupervisor::stateConfigured (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception)
{

  state_ = fsm_.getStateName (fsm_.getCurrentState());

  // Notify RCMS of having entered the Configured state
  try
  {
    //rcmsStateNotifier_.stateChanged(state_.toString(), "");
    rcmsStateNotifier_.stateChanged("Configured", stringF(theGlobalKey_->key()) );
  }
  catch(xcept::Exception &e)
  {
    diagService_->reportError("Failed to notify state change : "+ xcept::stdformat_exception_history(e),DIAGERROR);
  }

  PixelTimer debugTimer;
  if (extratimers_) {
    debugTimer.setName("PixelSupervisor::stateConfigured");
    debugTimer.printTime("RCMS notified of Configured state");
  }
  if (configurationTimer_.started() ) {
    configurationTimer_.stop();

    string confsource(getenv("PIXELCONFIGURATIONBASE"));
    if (confsource != "DB") confsource = "files";

    diagService_->reportError("Total configuration time ["+confsource+"] = "+stringF(configurationTimer_.tottime()),DIAGUSERINFO);
    configurationTimer_.reset();
  }

  diagService_->reportError( "PixelSupervisor::stateConfigured: workloop active: "+stringF(calibWorkloop_->isActive())+", workloop type: "+calibWorkloop_->getType(),DIAGDEBUG);

}

void PixelSupervisor::stateTTSTestMode (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception)
{
  state_ = fsm_.getStateName (fsm_.getCurrentState());
}


void PixelSupervisor::inError (toolbox::fsm::FiniteStateMachine & fsm) throw (toolbox::fsm::exception::Exception)
{
  state_ = fsm_.getStateName (fsm_.getCurrentState());
  rcmsStateNotifier_.stateChanged("Error", "");
}

xoap::MessageReference PixelSupervisor::reset (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  diagService_->reportError("New state before reset is: " + fsm_.getStateName (fsm_.getCurrentState()),DIAGINFO);

  fsm_.reset();

  xoap::MessageReference reply = xoap::createMessage();
  xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName responseName = envelope.createName("ResetResponse", "xdaq", XDAQ_NS_URI);
  (void) envelope.getBody().addBodyElement ( responseName );

  diagService_->reportError("New state after reset is: " + fsm_.getStateName (fsm_.getCurrentState()),DIAGINFO);

  return reply;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////// FSM State Transition Functions //////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void PixelSupervisor::transitionHaltedToConfiguring (toolbox::Event::Reference e)
{
  //exceptions of type toolbox::fsm::exception::Exception will automatically be caught
  //and cause a transition to the Error state

  PixelTimer debugTimer;
  debugTimer.setName("PixelSupervisor::transitionHaltedToConfiguring");
  if (extratimers_) debugTimer.printTime("getting theDetectorConfiguration");

  try {

    //preConfigure the FECs
    Attribute_Vector parametersToFEC(1);
    unsigned int globalKey=theGlobalKey_->key();
    parametersToFEC[0].name_="GlobalKey";	  parametersToFEC[0].value_=itoa(globalKey);
    Supervisors::iterator i_PixelFECSupervisor;
    for (i_PixelFECSupervisor=PixelFECSupervisors_.begin();i_PixelFECSupervisor!=PixelFECSupervisors_.end();++i_PixelFECSupervisor) {
      string reply = Send(i_PixelFECSupervisor->second, "preConfigure", parametersToFEC);
      if (reply!="preConfigureDone") {
	unsigned int instance=i_PixelFECSupervisor->first;
	XCEPT_RAISE(toolbox::fsm::exception::Exception, "Preconfigure command to FEC "+stringF(instance)+" failed");
      }
    }

  // Retrieve the Pixel Detector Configuration from database
  PixelConfigInterface::get(theDetectorConfiguration_, "pixel/detconfig/", *theGlobalKey_);
  if (theDetectorConfiguration_==0) XCEPT_RAISE(toolbox::fsm::exception::Exception, "Failed to load detector configuration!");

  if (extratimers_)  debugTimer.printTime("getting the name translation");
  // Retrieve the Pixel Name Translation from database
  diagService_->reportError("Retrieving Name Translation table from database... ", DIAGTRACE);
  *console_<<"Retrieving Name Translation from database... ";
  PixelConfigInterface::get(theNameTranslation_, "pixel/nametranslation/", *theGlobalKey_);
  if (theNameTranslation_==0) XCEPT_RAISE(toolbox::fsm::exception::Exception, "Failed to load nametranslation!");
  diagService_->reportError("... Name Translation table retrieved.", DIAGTRACE);
  *console_<<"done."<<std::endl;

  if (extratimers_)  debugTimer.printTime("getting the calib object");
  // Retrieve the Calibration Object from database
  // We cannot assert for theCalibObject==0 because it could be a Physics Run
  PixelConfigInterface::get(theCalibObject_, "pixel/calib/", *theGlobalKey_);

  if (extratimers_)  debugTimer.printTime("Building ROC and module lists");
  // Build ROC and module lists.
  if(dynamic_cast <PixelCalibConfiguration*> (theCalibObject_)!=0){
    (dynamic_cast <PixelCalibConfiguration*> (theCalibObject_))->buildROCAndModuleLists(theNameTranslation_, theDetectorConfiguration_);
  }

  if (extratimers_)  debugTimer.printTime("Getting TKFEC and portcard info");
  // Retrieve slow-I2C settings from database if there exists a PixelTKFECInterface
  if (!PixelTKFECSupervisors_.empty()) {

    // Retrieve the Tracker-FEC Configuration from datbase
    diagService_->reportError("Retrieving TKFEC Configuration Table... ", DIAGTRACE);
    *console_<<"Retrieving TKFEC Configuration Table... ";
    PixelConfigInterface::get(theTKFECConfiguration_, "pixel/tkfecconfig/", *theGlobalKey_);
    if (theTKFECConfiguration_==0)  XCEPT_RAISE(toolbox::fsm::exception::Exception, "Failed to load TKFEC Configuration!");
    diagService_->reportError("... TKFEC Configuration Table retrieved.", DIAGTRACE);
    *console_<<"done."<<std::endl;

    // Retrieve the PortCard Map from database
    diagService_->reportError("Retrieving PortCard Map... ", DIAGTRACE);
    *console_<<"Retrieving PortCard Map... ";
    PixelConfigInterface::get(thePortcardMap_, "pixel/portcardmap/", *theGlobalKey_);
    if (thePortcardMap_==0)  XCEPT_RAISE(toolbox::fsm::exception::Exception, "Failed to load portcardmap!");
    diagService_->reportError("... PortCard Map retrieved.", DIAGTRACE);
    *console_<<"done."<<std::endl;

    // PortCard settings (mapNamePortCard) are dynamically loaded when needed
  }

  // Retrieve Pixel-FEC configuration from database if PixelFECSupervisor exists
  if (!PixelFECSupervisors_.empty()) {
    diagService_->reportError("Retrieving FEC Configuration Table... ", DIAGTRACE);
    *console_<<"Retrieving FEC Configuration Table... ";
    PixelConfigInterface::get(theFECConfiguration_, "pixel/fecconfig/", *theGlobalKey_);
    diagService_->reportError("... FEC Configuration Table retrieved.", DIAGTRACE);
    *console_<<"done."<<std::endl;
  }

  // Retrieve the Pixel-FED configuration from database if PixelFEDSupervisor exists
  if (!PixelFEDSupervisors_.empty()) {
    diagService_->reportError("Retrieving FED configuration... ",DIAGTRACE);
    *console_<<"Retrieving FED configuration... ";
    PixelConfigInterface::get(theFEDConfiguration_, "pixel/fedconfig/", *theGlobalKey_);
    diagService_->reportError("... FED Configuration Table retrieved.", DIAGTRACE);
    *console_<<"done."<<std::endl;
  }

  if (theFECConfiguration_==0 || theFEDConfiguration_==0) XCEPT_RAISE(toolbox::fsm::exception::Exception, "Failed to get FEC or FED Configuration!");
  }
  catch (toolbox::fsm::exception::Exception & e) {
    //    throw;
    this->notifyQualified("error",e);
  }
  catch (std::exception & e) {  //translate std::exception to the correct type
    XCEPT_RAISE(toolbox::fsm::exception::Exception, string(e.what()));
  }

  if (extratimers_)  debugTimer.printTime("Done");

}

bool PixelSupervisor::fsmTransition(const string & transition) {
  //try to reduce code duplication by putting this code here
  //return true if the transition was successful
  bool isok=true;

  try {
    toolbox::Event::Reference e(new toolbox::Event(transition, this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    isok=false;
    ostringstream err;
    err<<"Failed to transition FSM from current state: "<<state_.toString()<<" with transition: "
       << transition<<". Exception: "<<e.what();
    diagService_->reportError(err.str(),DIAGERROR);
  }
  return isok;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

bool PixelSupervisor::CalibRunning(toolbox::task::WorkLoop * w1)
{
	assert(theCalibAlgorithm_!=0);

	if (runBeginCalibration_) {
	  theCalibAlgorithm_->runBeginCalibration();
	  runBeginCalibration_=false;
	}

	bool cont = theCalibAlgorithm_->runEvent();

	if ( cont == false ) {
	  theCalibAlgorithm_->runEndCalibration();
	  toolbox::Event::Reference e(new toolbox::Event("Done", this));
	  fsm_.fireEvent(e);
	  autoDone_=true;
	}

	return cont;
}

bool PixelSupervisor::jobcontrol_workloop(toolbox::task::WorkLoop * w1) {
  //we do not start this workloop unless some jobcontrol exists

  ::sleep(15); //we will want to tune this delay

  bool cont=true;

  if (jobcontrolmon_==0) {
    jobcontrolmon_ = new PixelJobControlMonitor(getApplicationContext());
  }

  SOAPCommander* soapCmdrPtr = dynamic_cast <SOAPCommander*> (this);
  jobcontrolmon_->doCheck(soapCmdrPtr);

  return cont;

}

void PixelSupervisor::printConfiguration(PixelConfigKey& theGlobalKey){

  std::vector<std::pair< std::string, unsigned int> > versions=PixelConfigInterface::getVersions(theGlobalKey);

  diagService_->reportError("Configuration key="+stringF(theGlobalKey.key()), DIAGINFO);
  for(unsigned int i=0;i<versions.size();i++){

    diagService_->reportError(versions[i].first+" "+stringF(versions[i].second)+"\n", DIAGDEBUG);
  }

}

void PixelSupervisor::ClearErrors(std::string which) {

  if (which=="All" || which == "Status") {
    lastMessage_ = "";
  }
}


void PixelSupervisor::updateConfig(string path, vector<string> aliases){

  //this is a bit of a hack...
  std::string search;
  if (path=="fedcard"){
    search="ls "+outputDir()+"/params_fed_*.dat > "+
                 outputDir()+"/filelist.txt";
  }
  else if (path=="dac"){
    search="ls "+outputDir()+"/ROC_DAC_module_*.dat > "+
                 outputDir()+"/filelist.txt";
  }
  else if (path=="portcard"){
    search="ls "+outputDir()+"/portcard_*.dat > "+
                 outputDir()+"/filelist.txt";
  }
  else if (path=="tbm"){
    search="ls "+outputDir()+"/TBM_module_*.dat > "+
                 outputDir()+"/filelist.txt";
  }
  else {

    diagService_->reportError("[PixelSupervisor::updateConfig] path="+path+" is unknown.", DIAGERROR);
    assert(0);
  }

  system(search.c_str());

  ifstream intmp((outputDir()+"/filelist.txt").c_str());
  assert(intmp.good()&&!intmp.eof());
  vector<string> filenames;
  string filename;
  intmp >> filename;
  while (!intmp.eof()){
    filenames.push_back(filename);
    intmp >> filename;
  }

  if (path=="fedcard"){
    writeAllFEDCards(filenames);
  }
  else if (path=="dac"){
    writeAllDACSettings(filenames);
  }
  else if (path=="portcard"){
    writeAllPortcards(filenames);
  }
  else if (path=="tbm"){
    writeAllTBMSettings(filenames);
  }
  else {

    diagService_->reportError("[PixelSupervisor::updateConfig] path="+path+" is unknown.", DIAGERROR);
    assert(0);
  }


  if (path=="fedcard"){
    search="ls "+outputDir()+"/params_fed_*.dat > "+
                 outputDir()+"/filelist.txt";
  }
  else if (path=="dac"){
    search="ls "+outputDir()+"/ROC_DAC_module_*.dat > "+
                 outputDir()+"/filelist.txt";
  }
  else if (path=="portcard"){
    search="ls "+outputDir()+"/portcard_*.dat > "+
                 outputDir()+"/filelist.txt";
  }
  else if (path=="tbm"){
    search="ls "+outputDir()+"/TBM_module_*.dat > "+
                 outputDir()+"/filelist.txt";
  }
  else {

    diagService_->reportError("[PixelSupervisor::updateConfig] path="+path+" is unknown.", DIAGERROR);
    assert(0);
  }

  system(search.c_str());


  ifstream in((outputDir()+"/filelist.txt").c_str());
  assert(in.good()&&!in.eof());

  unsigned int version=0;

  if (path=="fedcard"){

    vector<PixelFEDCard*> objects;

    in >> filename;

    while (!in.eof()){

      PixelFEDCard* fedcard= new PixelFEDCard(filename);

      fedcard->setAuthor(theCalibObject_->mode());
      fedcard->setComment(runNumber_);

      objects.push_back(fedcard);

      in >> filename;

    }

    version=PixelConfigInterface::put(objects,path);

    for(unsigned int i=0;i<objects.size();i++){
      delete objects[i];
    }
  }

  if (path=="dac"){

    vector<PixelDACSettings*> objects;

    in >> filename;

    while (!in.eof()){

      PixelDACSettings* fedcard= new PixelDACSettings(filename);

      fedcard->setAuthor(theCalibObject_->mode());
      fedcard->setComment(runNumber_);

      objects.push_back(fedcard);

      in >> filename;

    }

    version=PixelConfigInterface::put(objects,path);

    for(unsigned int i=0;i<objects.size();i++){
      delete objects[i];
    }
  }


  if (path=="portcard"){

    vector<PixelPortCardConfig*> objects;

    in >> filename;

    while (!in.eof()){

      PixelPortCardConfig* fedcard= new PixelPortCardConfig(filename);

      fedcard->setAuthor(theCalibObject_->mode());
      fedcard->setComment(runNumber_);

      objects.push_back(fedcard);

      in >> filename;

    }

    version=PixelConfigInterface::put(objects,path);

    for(unsigned int i=0;i<objects.size();i++){
      delete objects[i];
    }
  }


  if (path=="tbm"){

    vector<PixelTBMSettings*> objects;

    in >> filename;

    while (!in.eof()){

      PixelTBMSettings* fedcard= new PixelTBMSettings(filename);

      fedcard->setAuthor(theCalibObject_->mode());
      fedcard->setComment(runNumber_);

      objects.push_back(fedcard);

      in >> filename;

    }

    version=PixelConfigInterface::put(objects,path);

    for(unsigned int i=0;i<objects.size();i++){
      delete objects[i];
    }
  }

  diagService_->reportError("[PixelSupervisor::updateConfig] inserted new "+path+" with version="+stringF(version), DIAGINFO);

  for (unsigned int iAlias=0;iAlias<aliases.size();iAlias++){
    diagService_->reportError("[PixelSupervisor::updateConfig] updated "+path+" alias "+aliases[iAlias]+" to version="+stringF(version), DIAGINFO);

    PixelConfigInterface::addVersionAlias(path,version,aliases[iAlias]);

  }

  try {
    PixelConfigInterface::commit(0) ;
  }
  catch (xdaq::exception::Exception &e) {
    diagService_->reportError(e.what(),DIAGERROR);
    toolbox::Event::Reference ev(new toolbox::Event("Failure", this)); //FIXME..i should catch possible exceptions here....
    fsm_.fireEvent(ev);
  }

}

unsigned int PixelSupervisor::bookRunNumber(std::string dbLogin, std::string password)
{

  // std::string bookingCommand=std::string(getenv("JAVA_HOME"))+"/bin/java -jar "+std::string(getenv("RCMS_HOME"))+"/framework/utilities/runinfo/test/src/rcms/utilities/runinfo/runnumberbooker.jar";
  std::string bookingCommand=std::string(getenv("JAVA_HOME"))+"/bin/java -jar "+std::string(getenv("HOME"))+"/.functionmanagers/runnumberbooker.jar";
  bookingCommand+=" "+dbConnection_.toString();
  bookingCommand+=" "+dbLogin;
  bookingCommand+=" "+password;
  bookingCommand+=" "+dbUsername_.toString();
  bookingCommand+=" "+runSequence_.toString();

  diagService_->reportError("[PixelSupervisor::bookRunNumber] Booking command = "+bookingCommand, DIAGINFO);

  // Use pstream to execute this
  redi::ipstream book(bookingCommand.c_str());

  // Parse it and extract the run number
  std::string line;
  string::size_type runNumberPosition=std::string::npos;
  unsigned int runNumber=0;
  do {
    std::getline(book, line);
    runNumberPosition=line.find("RUN_NUMBER");
  } while (runNumberPosition==std::string::npos && book.good());
  if (runNumberPosition!=std::string::npos) {

    diagService_->reportError("[PixelSupervisor::bookRunNumber] runNumber line = "+line+", so run number = "+line.substr(11), DIAGINFO);

    runNumber=atoi(line.substr(11).c_str());
  }

  return runNumber;

}

void PixelSupervisor::writeRunInfo(std::string dbLogin, std::string password, std::string runNumber, std::string parameterName, std::string parameterValue)
{

  // std::string writingCommand=std::string(getenv("JAVA_HOME"))+"/bin/java -jar "+std::string(getenv("RCMS_HOME"))+"/framework/utilities/runinfo/test/src/rcms/utilities/runinfo/runinfowriter.jar";
  std::string writingCommand=std::string(getenv("JAVA_HOME"))+"/bin/java -jar "+std::string(getenv("HOME"))+"/.functionmanagers/runinfowriter.jar";
  writingCommand+=" "+dbConnection_.toString();
  writingCommand+=" "+dbLogin;
  writingCommand+=" "+password;
  writingCommand+=" "+runNumber;
  writingCommand+=" "+parameterName;
  writingCommand+=" "+parameterValue;
  writingCommand+=" "+runSequence_.toString();

  diagService_->reportError("[PixelSupervisor::writeRunInfo] Writing command = "+writingCommand, DIAGINFO);
  // Use pstream to execute this
  redi::ipstream book(writingCommand.c_str());

}

void PixelSupervisor::b2inEvent(toolbox::mem::Reference* msg, xdata::Properties& plist) throw (b2in::nub::exception::Exception){

  std::string getReply = plist.getProperty("b2in-eventing:action");
  std::string getmessageId=plist.getProperty("messageID");
  int messageID=atoi(getmessageId.c_str());
  std::string receiveMsg=plist.getProperty("returnValue");
  // CHECK WHAT IS IN THE MESSAGE
  if ( getReply == "return" ){

    this->removeMsgID(messageID, receiveMsg);
  }

  return;

}


xoap::MessageReference PixelSupervisor::MakeSOAPConfigMessage(const std::string &app_class_name, const std::string &parameter_name, const std::string &parameter_value) {

   const std::string PROPERTIES_NS_URI = "urn:xdaq-application:" + app_class_name;
   const std::string XSI_NS = "http://www.w3.org/2001/XMLSchema-instance";

   xoap::MessageReference msg = xoap::createMessage();

   xoap::SOAPEnvelope env = msg->getEnvelope();
   env.addNamespaceDeclaration ("xsi", XSI_NS);

   xoap::SOAPBody body = env.getBody();
   xoap::SOAPName cmdName = env.createName("ParameterSet","xdaq",XDAQ_NS_URI);
   xoap::SOAPBodyElement bodyElem = body.addBodyElement(cmdName);

   xoap::SOAPName name =  env.createName("properties","p",PROPERTIES_NS_URI);
   xoap::SOAPElement element = bodyElem.addChildElement(name);

   xoap::SOAPName name3 = env.createName("type","xsi",XSI_NS);
   element.addAttribute(name3, "soapenc:Struct");


   xoap::SOAPName name4 =  env.createName("Configuration","p",PROPERTIES_NS_URI);
   xoap::SOAPElement element2 = element.addChildElement(name4);

   xoap::SOAPName name5 = env.createName("type","xsi",XSI_NS);
   element2.addAttribute(name5, "xsd:string");

   element2.addTextNode(parameter_value);

   return msg;

}

void PixelSupervisor::SendConfigurationToTTC() {

  const bool DEBUG = true;
  if(DEBUG) std::cout<<" Load ttci configuration file"<<std::endl;

  PixelConfigInterface::get(theTTCciConfig_, "pixel/ttcciconfig/", *theGlobalKey_);
  if (theTTCciConfig_==0)  XCEPT_RAISE (xdaq::exception::Exception,"Failed to load PixelTTCSupervisor configuration data");

  stringstream &in = theTTCciConfig_->getTTCConfigStream();

  std::string text = "";
  std::string line;

  int linenr = 0;
  

  while (!in.eof())
    {
      std::getline(in,line);
      if (useTCDS_){
        if (linenr==0){
          std::string::size_type pos = line.find("#TCDS");
          if (pos == std::string::npos){
            diagService_->reportError("The provided configuration does not start with #TCDS and is therefore not accepted as a valid TCDS configuration! The default configuration will be used.",DIAGERROR);
            std::cout << "using default iCI configuration" << std::endl;
            return;
          }
          else{
            diagService_->reportError("The provided configuration is a valid TCDS configuration.",DIAGINFO);   
            std::cout << "using provided iCI configuration" << std::endl;
          }
        }
      }
      if (!useTCDS_){
        // remove comment lines in order not to have [ and ]                                                                                                                                                                                
        // in the config file because this causes problems                                                                                                                                                                                  
        // with the XDAQ application which look for [file=...]                                                                                                                                                                              
        // but does not require that all the character                                                                                                                                                                                      
        // are consecutive...                                                                                                                                                                                                                
        std::string::size_type pos = line.find('#');

        if (pos != std::string::npos)
          line.erase(pos);
      }
      text += line + "\n";
      linenr++;
    }

  //send the message
  Supervisors::iterator i_PixelTTCSupervisor;
  for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor) {
    if (useTTC_) {
      std::string app_class_name = TTCSupervisorApplicationName_;
      std::string parameter_name = "Configuration";
      xoap::MessageReference msg = MakeSOAPConfigMessage(app_class_name,parameter_name,text);
      std::string reply = "";

      // Add "reset" before sending the Configuration, according to the TTC TWiki
      reply = Send(i_PixelTTCSupervisor->second, "reset");
      std::cout << endl<<"THIS IS THE REPLY: " << reply << endl;
      if (reply != "TTCciControlFSMReset") std::cout << " Wrong reply, should we raise an exception? "<< endl;
      reply = Send(i_PixelTTCSupervisor->second, msg);
      std::cout << "The reply, just out of curiosity, was " << reply << std::endl;
      if (reply=="Fault") XCEPT_RAISE (xdaq::exception::Exception,"PixelTTCSupervisor returned SOAP reply Fault!");
    }
    if (useTCDS_){
      Attribute_Vector parameters(1);
      parameters[0].name_="TCDSConfigString";
      parameters[0].value_=text;
      string reply = Send(i_PixelTTCSupervisor->second, "ReceiveConfigString", parameters);
      diagService_->reportError("TTCSupervisor #" + stringF(i_PixelTTCSupervisor->first) + ": Sending TCDSConfiguration reply: " + reply, DIAGINFO);
      *console_<< "TTCSupervisor #" << (i_PixelTTCSupervisor->first) << ": Sending TCDSConfiguration reply: " << reply <<std::endl;
      if (reply!= "ReceiveConfigStringResponse") {
        diagService_->reportError("TTCSupervisor supervising crate #"+stringF(i_PixelTTCSupervisor->first) + " could not receive TCDS configuration.",DIAGERROR);
        *console_<<"TTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first)<<" could not receive TCDS configuration: "<<reply<<std::endl;
      }
    }
  }
  return;
}


void PixelSupervisor::SendConfigurationToLTC() {

  const bool DEBUG = true;
  if(DEBUG) std::cout<<" Load ltc configuration file"<<std::endl;

  PixelConfigInterface::get(theLTCConfig_, "pixel/ltcconfig/", *theGlobalKey_);
  if (theLTCConfig_==0)  XCEPT_RAISE (xdaq::exception::Exception,"Failed to load PixelLTCSupervisor configuration data");

  stringstream &in = theLTCConfig_->getLTCConfigStream();

  std::string text = "";
  std::string line;
  int linenr = 0;

  while (!in.eof())
    {
      std::getline(in,line);
      if (useTCDS_){
        if (linenr==0){ 
          std::string::size_type pos = line.find("#TCDS");
          if (pos == std::string::npos){
            diagService_->reportError("The provided configuration does not start with #TCDS and is therefore not accepted as a valid TCDS configuration! The default configuration will be used.",DIAGERROR);
            std::cout << "using default PI configuration" << std::endl;
            return;
          }
          else{ 
            diagService_->reportError("The provided configuration is a valid TCDS configuration.",DIAGINFO);
            std::cout << "using provided PI configuration" << std::endl;
          }
        }
      }
      if (!useTCDS_){
        // remove comment lines in order not to have [ and ]
        // in the config file because this causes problems
        // with the XDAQ application which look for [file=...]
        // but does not require that all the character
        // are consecutive...
        std::string::size_type pos = line.find('#');

        if (pos != std::string::npos)
          line.erase(pos);
      }
      text += line + "\n";
      linenr++;
    }
  //send the message
  Supervisors::iterator i_PixelLTCSupervisor;
  for (i_PixelLTCSupervisor=PixelLTCSupervisors_.begin();i_PixelLTCSupervisor!=PixelLTCSupervisors_.end();++i_PixelLTCSupervisor) {
    if (useTCDS_){
      Attribute_Vector parameters(1);
      parameters[0].name_="TCDSConfigString";
      parameters[0].value_=text;
      string reply = Send(i_PixelLTCSupervisor->second, "ReceiveConfigString", parameters);
      diagService_->reportError("LTCSupervisor #" + stringF(i_PixelLTCSupervisor->first) + ": Sending TCDSConfiguration reply: " + reply, DIAGINFO);
      *console_<< "LTCSupervisor #" << (i_PixelLTCSupervisor->first) << ": Sending TCDSConfiguration reply: " << reply <<std::endl;
      if (reply!= "ReceiveConfigStringResponse") {
        diagService_->reportError("LTCSupervisor supervising crate #"+stringF(i_PixelLTCSupervisor->first) + " could not receive TCDS configuration.",DIAGERROR);
        *console_<<"LTCSupervisor supervising crate #"<<(i_PixelLTCSupervisor->first)<<" could not receive TCDS configuration: "<<reply<<std::endl;
      }
    }
  }
  return;
}




xoap::MessageReference PixelSupervisor::DetectSoftError (xoap::MessageReference msg)
{
  diagService_->reportError("Entering SOAP callback for DetectSoftError.",DIAGINFO);
  *console_<<"PixelSupervisor: Entering SOAP callback for DetectSoftError."<<std::endl;

  if (state_!="Running"&&state_!="RunningSoftErrorDetected"&& state_!="RunningDegraded") {
    diagService_->reportError("DetectSoftError transition is not allowed from state: "+state_.toString(),DIAGWARN);
    return MakeSOAPMessageReference("DetectSoftErrorNotDone");
  }

  //clear errors
  ClearErrors("All");

  xoap::MessageReference reply=MakeSOAPMessageReference("DetectSoftErrorDone");

  bool isOK = fsmTransition("DetectSoftError") ;
  if (!isOK) reply=MakeSOAPMessageReference("DetectSoftErrorFailed");

  diagService_->reportError("Exiting SOAP callback for DetectSoftError.",DIAGINFO);
  *console_<<"PixelSupervisor: Exiting SOAP callback for DetectSoftError."<<std::endl;

  return reply;

}

xoap::MessageReference PixelSupervisor::DetectDegradation (xoap::MessageReference msg)
{
  diagService_->reportError("Entering SOAP callback for DetectDegradation.",DIAGINFO);
  *console_<<"PixelSupervisor: Entering SOAP callback for DetectDegradation."<<std::endl;

  if (state_!="Running") {

    diagService_->reportError("DetectDegradation transition is not allowed from state: "+state_.toString(),DIAGWARN);
    return MakeSOAPMessageReference("DetectDegradationNotDone");
  }

  //clear errors
  ClearErrors("All");

  xoap::MessageReference reply=MakeSOAPMessageReference("DetectDegradationDone");

  bool isOK = fsmTransition("DetectDegradation") ;
  if (!isOK) reply=MakeSOAPMessageReference("DetectDegradationFailed");

  diagService_->reportError("Exiting SOAP callback for DetectDegradation.",DIAGINFO);
  *console_<<"PixelSupervisor: Exiting SOAP callback for DetectDegradation."<<std::endl;

  return reply;

}

void PixelSupervisor::stateRunningSoftErrorDetected (toolbox::fsm::FiniteStateMachine & fsm)
{

  state_ = fsm_.getStateName (fsm_.getCurrentState());
  *console_<<"Trying to notify RCMS of RunningSoftErrorDetected"<<std::endl;
  try {
    rcmsStateNotifier_.stateChanged("RunningSoftErrorDetected", "");
  }
  catch(xcept::Exception &e) {
    diagService_->reportError("Failed to notify state change : "+ xcept::stdformat_exception_history(e),DIAGERROR);
  }

  diagService_->reportError("PixelSupervisor::stateRunningSoftErrorDetected: workloop active: "+stringF(calibWorkloop_->isActive())+", workloop type: "+calibWorkloop_->getType(),DIAGINFO);

}

void PixelSupervisor::stateRunningDegraded (toolbox::fsm::FiniteStateMachine & fsm)
{

  state_ = fsm_.getStateName (fsm_.getCurrentState());
  *console_<<"Trying to notify RCMS of RunningDegradedDetected"<<std::endl;
  try {
    rcmsStateNotifier_.stateChanged("RunningDegradedDetected", "");
  }
  catch(xcept::Exception &e) {
    diagService_->reportError("Failed to notify state change : "+ xcept::stdformat_exception_history(e),DIAGERROR);
  }

  diagService_->reportError("PixelSupervisor::stateRunningDegraded: workloop active: "+stringF(calibWorkloop_->isActive())+", workloop type: "+calibWorkloop_->getType(),DIAGINFO);
}


xoap::MessageReference PixelSupervisor::FixSoftError (xoap::MessageReference msg)
{
  diagService_->reportError("Entering SOAP callback for FixSoftError.",DIAGUSERINFO);
  *console_<<"PixelSupervisor: Entering SOAP callback for FixSoftError."<<std::endl;

  if (state_!="Running" && state_!="RunningSoftErrorDetected" && state_!="FixingSoftError" && state_!="RunningDegraded") {
    diagService_->reportError("FixSoftError transition is not allowed from state: "+state_.toString(),DIAGWARN);
    return MakeSOAPMessageReference("FixSoftErrorNotDone");
  }

  if( theCalibObject_!=0 )
    {
      calibWorkloop_->cancel();
      diagService_->reportError("PixelSupervisor::Pause: Calib workloop canceled during FixingSoftError",DIAGUSERINFO);
    }

  //clear errors
  ClearErrors("All");

  xoap::MessageReference reply=MakeSOAPMessageReference("FixSoftErrorDone");
  // That's it! Step to the FixingSoftError state, and
  // relegate all further configuring to the stateFixingSoftError method.
  bool isOK = fsmTransition("FixSoftError") ;
  if (!isOK) reply=MakeSOAPMessageReference("FixSoftErrorFailed");

  diagService_->reportError("Exiting SOAP callback for FixSoftError.",DIAGINFO);
  *console_<<"PixelSupervisor: Exiting SOAP callback for FixSoftError."<<std::endl;

  return reply;
}

void PixelSupervisor::stateFixingSoftError (toolbox::fsm::FiniteStateMachine & fsm)
{

  //Copy structure of stateConfiguring
  //Use only those objects that we want to reconfigure (TBD)

  PixelTimer GlobalTimer;
  if (extratimers_) {
    GlobalTimer.setVerbose(true); GlobalTimer.setName("PixelSupervisor::stateFixingSoftError");
    GlobalTimer.start("stateFixingSoftError start");
  }

  // Update the state_ member data so that Infospace may publish this information
  // Advertize more on webpage, console etc
  state_=fsm_.getStateName(fsm_.getCurrentState());
  diagService_->reportError("PixelSupervisor::stateFixingSoftError: Entering",DIAGUSERINFO);
  *console_<<"PixelSupervisor::stateFixingSoftError: Entering"<<std::endl;

  // We're going to time various aspects of configuration
  PixelTimer FEDFixSoftErrorTime;
  PixelTimer FECFixSoftErrorTime;
  PixelTimer TTCFixSoftErrorTime;

  // These are parameters sets which will be sent to the underlying Supervisors
  Attribute_Vector parametersToTKFEC(1),
                   parametersToFEC(1),
                   parametersToFED(1),
                   parametersToLTC(1),
                   parametersToDCSFSM(1);

  unsigned int globalKey=theGlobalKey_->key();
  parametersToFEC[0].name_="GlobalKey";	  parametersToFEC[0].value_=itoa(globalKey);
  parametersToFED[0].name_="GlobalKey";   parametersToFED[0].value_=itoa(globalKey);
  parametersToTKFEC[0].name_="GlobalKey"; parametersToTKFEC[0].value_=itoa(globalKey);
  parametersToLTC[0].name_="GlobalKey";   parametersToLTC[0].value_=itoa(globalKey);
  parametersToDCSFSM[0].name_="GlobalKey";	  parametersToDCSFSM[0].value_=itoa(globalKey);

  try { //posting SOAP, etc

  //     //FixSoftError PixelDCSFSMInterface
  //     if (!PixelDCSFSMInterface_.empty() ) {
  //       diagService_->reportError("[PixelSupervisor::stateFixingSoftError] PixelDCSFSMInterface exists...",DIAGDEBUG);
  //       Supervisors::iterator i_PixelDCSFSMInterface;
  //       for (i_PixelDCSFSMInterface=PixelDCSFSMInterface_.begin(); i_PixelDCSFSMInterface!=PixelDCSFSMInterface_.end(); ++i_PixelDCSFSMInterface) {
  //         unsigned int instance = i_PixelDCSFSMInterface->first;
  //
  //         diagService_->reportError("[PixelSupervisor::stateFixingSoftError] DCSFSMInterface instance "+stringF(instance)+" has state "+statePixelDCSFSMInterface_[instance],DIAGINFO);
  //
  //         std::string fsmState=Send(i_PixelDCSFSMInterface->second, "FSMStateRequest");
  //
  //         if (statePixelDCSFSMInterface_[instance]=="Running"||statePixelDCSFSMInterface_[instance]=="RunningSoftErrorDetected") {
  //     	statePixelDCSFSMInterface_[instance]="FixingSoftError";
  //           std::string reply = Send(i_PixelDCSFSMInterface->second, "FixSoftError", parametersToDCSFSM);
  //     	if (reply!="FixSoftErrorDone") {
  //     	  XCEPT_RAISE (xdaq::exception::Exception,"PixelDCSFSMInterface configuration failure");
  //     	}
  //         }
  //         //else if (statePixelDCSFSMInterface_[instance]=="Initial") {
  //         //	diagService_->reportError("[PixelSupervisor::stateFixingSoftError] PixelDCSFSMInterface is stuck in the Initial state!",DIAGERROR);
  //         //}
  //         if (statePixelDCSFSMInterface_[instance]!="FixedSoftError") { //it is not done yet
  //     	diagService_->reportError("[PixelSupervisor::stateFixingSoftError] DCS Interface not FixedSoftError yet",DIAGINFO);
  //     	return;
  //         }
  //       }
  //     }


  if (extratimers_)  {GlobalTimer.stop("got GlobalKey");
    GlobalTimer.start("FixSoftError TKFEC");}

  // Send a SOAP message to PixelTKFECSupervisor to FixSoftError
  // Also time the procedure

  bool FixedSoftErrorTKFECs=true;
  bool initialMessages=false;

  if (!PixelTKFECSupervisors_.empty()) {
    diagService_->reportError("[PixelSupervisor::stateFixingSoftError] TKFECs exist...",DIAGINFO);
    Supervisors::iterator i_PixelTKFECSupervisor;
    for (i_PixelTKFECSupervisor=PixelTKFECSupervisors_.begin();i_PixelTKFECSupervisor!=PixelTKFECSupervisors_.end();++i_PixelTKFECSupervisor) {
      unsigned int instance=i_PixelTKFECSupervisor->first;

      diagService_->reportError("[PixelSupervisor::stateFixingSoftError] TKFECs instance "+stringF(instance)+" has state "+statePixelTKFECSupervisors_[instance],DIAGINFO);

      if (statePixelTKFECSupervisors_[instance]=="Running"||statePixelTKFECSupervisors_[instance]=="RunningSoftErrorDetected") {

	diagService_->reportError("PixelSupervisor::stateFixingSoftError: Sending SOAP messages to PixelTKFECSupervisor "+stringF(instance),DIAGINFO);

	statePixelTKFECSupervisors_[instance]="FixingSoftError";
        std::string reply = Send(i_PixelTKFECSupervisor->second, "FixSoftError", parametersToTKFEC);
        if (reply!= "FixSoftErrorDone") {
          *console_<<"PixelTKFECSupervisor supervising crate #"<<(i_PixelTKFECSupervisor->first)<<" could not be FixedSoftError."<<std::endl;
	  XCEPT_RAISE (xdaq::exception::Exception,"PixelTKFECSupervisor configuration failure, instance="+string(itoa(instance)));
        }
	initialMessages=true;
      }
      if (statePixelTKFECSupervisors_[instance]!="FixedSoftError") {
	diagService_->reportError("[PixelSupervisor::stateFixingSoftError] TKFEC not FixedSoftError yet",DIAGINFO);
	FixedSoftErrorTKFECs=false;
      }
    }
  }
  //if (!FixedSoftErrorTKFECs&&!initialMessages) return;
  if (!FixedSoftErrorTKFECs) return;

  if (extratimers_) {    GlobalTimer.stop();
    GlobalTimer.start("FixSoftError PxlFEC");}

  bool proceed=true;

  // Send a SOAP message to PixelFECSupervisor
  if (!PixelFECSupervisors_.empty()) {
    diagService_->reportError("[PixelSupervisor::stateFixingSoftError] FECs exist...",DIAGINFO);
    Supervisors::iterator i_PixelFECSupervisor;
    for (i_PixelFECSupervisor=PixelFECSupervisors_.begin();i_PixelFECSupervisor!=PixelFECSupervisors_.end();++i_PixelFECSupervisor) {
      unsigned int instance=i_PixelFECSupervisor->first;
      std::string fsmState=statePixelFECSupervisors_[instance];

      diagService_->reportError("[PixelSupervisor::stateFixingSoftError] FEC instance "+stringF(instance)+" has state "+fsmState,DIAGINFO);

      if (fsmState=="Running"||fsmState=="RunningSoftErrorDetected"||state_=="RunningDegraded") {
        FECFixSoftErrorTime.start();

	diagService_->reportError("PixelSupervisor::stateFixingSoftError: Sending SOAP messages to PixelFECSupervisor "+instance,DIAGINFO);

        statePixelFECSupervisors_[instance]="FixingSoftError";
	//don't actually have to send the global key here anymore, but it doesn't hurt either
        std::string reply = Send(i_PixelFECSupervisor->second, "FixSoftError", parametersToFEC);
        FECFixSoftErrorTime.stop();
        if (reply!= "FixSoftErrorDone") {
          *console_<<"PixelFECSupervisor supervising crate #"<<stringF(instance)<<" could not be FixedSoftError."<<std::endl;
	  XCEPT_RAISE (xdaq::exception::Exception,"PixelFECSupervisor configuration failure, instance="+string(itoa(instance)));
        }
      }
      if (fsmState!="FixedSoftError") {

	diagService_->reportError("[PixelSupervisor::stateFixingSoftError] Cannot proceed to FixedSoftError because PixelFECSupervisor instance "+stringF(instance)+" is not FixedSoftError yet.",DIAGINFO);
        proceed=false;
      }
    }
  }
  else{
    diagService_->reportError("[PixelSupervisor::stateFixingSoftError] no FECs exist...",DIAGFATAL);
  }


  // Send a SOAP message to PixelFEDSupervisor
  if (!PixelFEDSupervisors_.empty()) {

    diagService_->reportError("[PixelSupervisor::stateFixingSoftError] FEDs exist...",DIAGINFO);
    Supervisors::iterator i_PixelFEDSupervisor;
    for (i_PixelFEDSupervisor=PixelFEDSupervisors_.begin();i_PixelFEDSupervisor!=PixelFEDSupervisors_.end();++i_PixelFEDSupervisor) {
      unsigned int instance=i_PixelFEDSupervisor->first;
      std::string fsmState=statePixelFEDSupervisors_[instance];

      diagService_->reportError("[PixelSupervisor::stateFixingSoftError] FED instance "+stringF(instance)+" has state "+fsmState,DIAGINFO);

      if (fsmState=="Running"||fsmState=="RunningSoftErrorDetected"||state_=="RunningDegraded") {
        FEDFixSoftErrorTime.start();

	diagService_->reportError("PixelSupervisor::stateFixingSoftError: Sending SOAP messages to PixelFEDSupervisor "+stringF(instance),DIAGINFO);

        statePixelFEDSupervisors_[instance]="FixingSoftError";
        std::string reply = Send(i_PixelFEDSupervisor->second, "FixSoftError", parametersToFED);
        FEDFixSoftErrorTime.stop();
        if (reply!= "FixSoftErrorDone") {
          *console_<<"PixelFEDSupervisor supervising crate #"<<(i_PixelFEDSupervisor->first)<<" could not be FixedSoftError."<<std::endl;
	  XCEPT_RAISE (xdaq::exception::Exception,"PixelFEDSupervisor configuration failure, instance="+string(itoa(instance)));
        } else {
	  diagService_->reportError("PixelSupervisor::stateFixingSoftError: Received SOAP reply from PixelFEDSupervisors",DIAGINFO);
        }
      }

      if (fsmState!="FixedSoftError") {
	diagService_->reportError("[PixelSupervisor::stateFixingSoftError] Cannot proceed to FixedSoftError because PixelFEDSupervisor instance "+stringF(instance)+" is not FixedSoftError yet.",DIAGINFO);
          proceed=false;
      }
    }
  }
  else{

    diagService_->reportError("[PixelSupervisor::stateFixingSoftError] no FEDs exist...",DIAGFATAL);
  }

  //// Send a SOAP message to PixelDCStoTrkFECDpInterface
  //if (!PixelDCStoTrkFECDpInterface_.empty()) {
  //
  //  Supervisors::iterator i_PixelDCStoTrkFECDpInterface;
  //  for (i_PixelDCStoTrkFECDpInterface=PixelDCStoTrkFECDpInterface_.begin(); i_PixelDCStoTrkFECDpInterface!=PixelDCStoTrkFECDpInterface_.end(); ++i_PixelDCStoTrkFECDpInterface) {
  //    if (  statePixelDCStoTrkFECDpInterface_[i_PixelDCStoTrkFECDpInterface->first] == "Running" || statePixelDCStoTrkFECDpInterface_[i_PixelDCStoTrkFECDpInterface->first] == "RunningSoftErrorDetected" ) {
  //
  //	diagService_->reportError("PixelSupervisor::stateFixingSoftError: Sending SOAP message to PixelDCStoTrkFECDpInterface",DIAGINFO);
  //	std::string reply = Send(i_PixelDCStoTrkFECDpInterface->second, "FixSoftError");
  //	//at the moment this class never returns anything other than FixSoftErrorDone...so no need to look at the reply
  //      if (reply!= "FixSoftErrorDone") {
  //        *console_<<"PixelDCStoTrkFECDpInterface supervising crate #"<<(i_PixelDCStoTrkFECDpInterface->first)<<" could not be FixedSoftError."<<std::endl;
  //	  XCEPT_RAISE (xdaq::exception::Exception,"PixelFEDSupervisor configuration failure, instance="+string(itoa(instance)));
  //      } else {
  //	  diagService_->reportError("PixelSupervisor::stateFixingSoftError: Received SOAP reply from PixelFEDSupervisors",DIAGINFO);
  //      }
  //	cout<<"done sending soap to PixelDCStoTrkFECDpInterface"<<endl;
  //	statePixelDCStoTrkFECDpInterface_[i_PixelDCStoTrkFECDpInterface->first] = "FixingSoftError";
  //    }
  //    else if (  statePixelDCStoTrkFECDpInterface_[i_PixelDCStoTrkFECDpInterface->first] == "Configuring" ) {
  //	proceed=false;
  //	cout<<"PixelDCStoTrkFECDpInterface is configuring"<<endl;
  //
  //    }
  //    //if the state is FixedSoftError, then we are good to go
  //  }
  //}

  if (proceed&&FixedSoftErrorTKFECs) {

  //       // Send a SOAP message to PixelLTCSupervisor
  //       if(!PixelLTCSupervisors_.empty()) {
  //
  //         diagService_->reportError("PixelSupervisor::stateFixingSoftError: Sending SOAP messages to PixelLTCSupervisors",DIAGINFO);
  //         Supervisors::iterator i_PixelLTCSupervisor;
  //         for (i_PixelLTCSupervisor=PixelLTCSupervisors_.begin();i_PixelLTCSupervisor!=PixelLTCSupervisors_.end();++i_PixelLTCSupervisor) {
  //           std::string reply = Send(i_PixelLTCSupervisor->second, "FixSoftError", parametersToLTC);
  //           if (reply!= "FixSoftErrorResponse") {
  //             *console_<<"PixelLTCSupervisor supervising crate #"<<(i_PixelLTCSupervisor->first)<<" could not be FixedSoftError."<<std::endl;
  //     	  XCEPT_RAISE (xdaq::exception::Exception,"PixelLTCSupervisor configuration failure");
  //           }
  //         }
  //       }
  //
  //       // Send a SOAP message to PixelTTCSupervisor
  //       SendConfigurationToTTC();
  //
  //       diagService_->reportError("PixelSupervisor::stateFixingSoftError: Sending SOAP messages to PixelTTCSupervisors",DIAGINFO);
  //
  //       TTCFixSoftErrorTime.start();
  //       Supervisors::iterator i_PixelTTCSupervisor;
  //       for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor) {
  //         std::string reply = Send(i_PixelTTCSupervisor->second, "FixSoftError");
  //         if (reply!= "FixSoftErrorResponse") {
  //           *console_<<"PixelTTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first)<<" could not be FixedSoftError."<<std::endl;
  //     	XCEPT_RAISE (xdaq::exception::Exception,"PixelTTCSupervisor configuration failure");
  //         }
  //       }
  //       TTCFixSoftErrorTime.stop();
  //       if (extratimers_) {   GlobalTimer.stop();
  //         GlobalTimer.start("FixSoftError SLinkMonitor and DCStoTrkFECDpInterface");}
  //
  //       // Send a SOAP message to PixelSlinkMonitor
  //       if (!PixelSlinkMonitors_.empty()) {
  //         diagService_->reportError("PixelSupervisor::stateFixingSoftError: Sending SOAP messages to PixelSlinkMonitor",DIAGINFO);
  //         Supervisors::iterator i_PixelSlinkMonitor;
  //         for (i_PixelSlinkMonitor=PixelSlinkMonitors_.begin();i_PixelSlinkMonitor!=PixelSlinkMonitors_.end();++i_PixelSlinkMonitor) {
  //           std::string reply = Send(i_PixelSlinkMonitor->second, "FixSoftError");
  //           if (reply!= "FixSoftErrorDone") {
  //             *console_<<"PixelFEDSupervisor supervising crate #"<<(i_PixelSlinkMonitor->first)<<" could not be FixedSoftError."<<std::endl;
  //     	  XCEPT_RAISE (xdaq::exception::Exception,"PixelSlinkMonitor configuration failure");
  //           }
  //         }
  //       }
  //
  //       // Advertize the timing results
  //
  //       diagService_->reportError("PixelSupervisor::stateFixingSoftError: TTC configuration time  :"+stringF(TTCFixSoftErrorTime.tottime()),DIAGINFO);
  //       //    diagService_->reportError("PLEASE IGNORE: PixelSupervisor::stateFixingSoftError: FED configuration time  :"+stringF(FEDFixSoftErrorTime.tottime()),DIAGINFO);
  //       //    diagService_->reportError("PLEASE IGNORE: PixelSupervisor::stateFixingSoftError: FEC configuration time  :"+stringF(FECFixSoftErrorTime.tottime()),DIAGINFO);
  //
    try {
      toolbox::Event::Reference e(new toolbox::Event("FixingSoftErrorDone", this));
      fsm_.fireEvent(e);
    } catch (toolbox::fsm::exception::Exception & e) {
      diagService_->reportError("[PixelSupervisor::stateFixingSoftError] Invalid command "+stringF(e.what()),DIAGERROR);
    }

  }

  } catch (xdaq::exception::Exception & e) {
    diagService_->reportError("[PixelSupervisor::stateFixingSoftError] Failed to FixSoftError with exception: "+string(e.what()),DIAGERROR);
    try {
      toolbox::Event::Reference ev(new toolbox::Event("Failure", this));
      fsm_.fireEvent(ev);
    } catch (...) {
      diagService_->reportError("[PixelSupervisor::stateFixingSoftError] Failed to transfer FSM to Error state",DIAGFATAL);
    }

  }

  if (extratimers_)  GlobalTimer.stop("done with fireEvent");


  // Advertize the exiting of this method
  diagService_->reportError("PixelSupervisor::stateFixingSoftError: Exiting",DIAGUSERINFO);
  *console_<<"PixelSupervisor::stateFixingSoftError: Exiting"<<std::endl;
}



void PixelSupervisor::stateFixedSoftError (toolbox::fsm::FiniteStateMachine & fsm)
{

  diagService_->reportError("PixelSupervisor::stateFixedSoftError: Entering state FixedSoftError",DIAGINFO);
  *console_<<"PixelSupervisor::stateFixedSoftError: --- Returning to Running ---"<<std::endl;

  try {

  Supervisors::iterator i_PixelTKFECSupervisor;
  for (i_PixelTKFECSupervisor=PixelTKFECSupervisors_.begin();i_PixelTKFECSupervisor!=PixelTKFECSupervisors_.end();++i_PixelTKFECSupervisor) {
    std::string reply = Send(i_PixelTKFECSupervisor->second, "ResumeFromSoftError");
    if (reply!= "ResumeFromSoftErrorDone") {
      diagService_->reportError("PixelSupervisor::stateFixedSoftError: PixelTKFECSupervisor supervising crate #"+stringF(i_PixelTKFECSupervisor->first) + " could not be resumed.",DIAGERROR);
      *console_<<"PixelSupervisor::stateFixedSoftError: PixelTKFECSupervisor supervising crate #"<<(i_PixelTKFECSupervisor->first)<<" could not be resumed."<<std::endl;
      fsmTransition("Failure");
      *console_<<"PixelSupervisor::stateFixedSoftError: --- Resuming from Soft Error Failed! ---"<<std::endl;
    }
  }

  Supervisors::iterator i_PixelFECSupervisor;
  for (i_PixelFECSupervisor=PixelFECSupervisors_.begin();i_PixelFECSupervisor!=PixelFECSupervisors_.end();++i_PixelFECSupervisor) {
    std::string reply = Send(i_PixelFECSupervisor->second, "ResumeFromSoftError");
    if (reply!= "ResumeFromSoftErrorDone") {
      diagService_->reportError("PixelSupervisor::stateFixedSoftError: PixelFECSupervisor supervising crate #"+stringF(i_PixelFECSupervisor->first) + " could not be resumed.",DIAGERROR);
      *console_<<"PixelSupervisor::stateFixedSoftError: PixelFECSupervisor supervising crate #"<<(i_PixelFECSupervisor->first)<<" could not be resumed."<<std::endl;
      fsmTransition("Failure");
      *console_<<"PixelSupervisor::stateFixedSoftError: --- Resuming from Soft Error Failed! ---"<<std::endl;
    }
  }

  Supervisors::iterator i_PixelFEDSupervisor;
  for (i_PixelFEDSupervisor=PixelFEDSupervisors_.begin();i_PixelFEDSupervisor!=PixelFEDSupervisors_.end();++i_PixelFEDSupervisor) {
    std::string reply = Send(i_PixelFEDSupervisor->second, "ResumeFromSoftError");
    if (reply!= "ResumeFromSoftErrorDone") {
      diagService_->reportError("PixelSupervisor::stateFixedSoftError: PixelFEDSupervisor supervising crate #"+stringF(i_PixelFEDSupervisor->first) + " could not be resumed.",DIAGERROR);
      *console_<<"PixelSupervisor::stateFixedSoftError: PixelFEDSupervisor supervising crate #"<<(i_PixelFEDSupervisor->first)<<" could not be resumed."<<std::endl;
      fsmTransition("Failure");
      *console_<<"PixelSupervisor::stateFixedSoftError: --- Resuming from Soft Error Failed! ---"<<std::endl;
    }
  }

 //     Supervisors::iterator i_PixelTTCSupervisor;
 //     for (i_PixelTTCSupervisor=PixelTTCSupervisors_.begin();i_PixelTTCSupervisor!=PixelTTCSupervisors_.end();++i_PixelTTCSupervisor) {
 //       std::string reply = Send(i_PixelTTCSupervisor->second, "Resume");
 //       if (reply!= "ResumeResponse") {
 //         diagService_->reportError("PixelTTCSupervisor supervising crate #"+stringF(i_PixelTTCSupervisor->first) + " could not be resumed.",DIAGERROR);
 //         *console_<<"PixelTTCSupervisor supervising crate #"<<(i_PixelTTCSupervisor->first)<<" could not be resumed."<<std::endl;
 //         fsmTransition("Failure");
 //         *console_<<"PixelSupervisor::stateFixedSoftError: --- Resuming from Soft Error Failed! ---"<<std::endl;
 //       }
 //     }
 //
 //     if(!PixelLTCSupervisors_.empty()) {
 //       Supervisors::iterator i_PixelLTCSupervisor;
 //       for (i_PixelLTCSupervisor=PixelLTCSupervisors_.begin();i_PixelLTCSupervisor!=PixelLTCSupervisors_.end();++i_PixelLTCSupervisor) {
 //         std::string reply = Send(i_PixelLTCSupervisor->second, "Resume");
 //         if (reply!= "ResumeResponse") {
 //     	diagService_->reportError("PixelSupervisor::stateFixedSoftError: PixelLTCSupervisor supervising crate #"+stringF(i_PixelLTCSupervisor->first) + " could not be resumed.",DIAGERROR);
 //     	*console_<<"PixelSupervisor::stateFixedSoftError: PixelLTCSupervisor supervising crate #"<<(i_PixelLTCSupervisor->first)<<" could not be resumed."<<std::endl;
 //         fsmTransition("Failure");
 //         *console_<<"PixelSupervisor::stateFixedSoftError: --- Resuming from Soft Error Failed! ---"<<std::endl;
 //         }
 //       }
 //     }
 //
 //     if (!PixelSlinkMonitors_.empty()) {
 //       Supervisors::iterator i_PixelSlinkMonitor;
 //       for (i_PixelSlinkMonitor=PixelSlinkMonitors_.begin();i_PixelSlinkMonitor!=PixelSlinkMonitors_.end();++i_PixelSlinkMonitor) {
 //         std::string reply = Send(i_PixelSlinkMonitor->second, "Resume");
 //         if (reply!= "ResumeDone") {
 //     	diagService_->reportError("PixelSupervisor::stateFixedSoftError: PixelSlinkMonitor supervising crate #"+stringF(i_PixelSlinkMonitor->first) + " could not be resumed!",DIAGERROR);
 //     	*console_<<"PixelSupervisor::stateFixedSoftError: PixelSlinkMonitor supervising crate #"<<(i_PixelSlinkMonitor->first)<<" could not be resumed!"<<std::endl;
 //         fsmTransition("Failure");
 //         *console_<<"PixelSupervisor::stateFixedSoftError: --- Resuming from Soft Error Failed! ---"<<std::endl;
 //         }
 //       }
 //     }

  } catch (xcept::Exception & e) {
    diagService_->reportError("ResumeFromSoftError failed with exception: "+string(e.what()),DIAGERROR);
    fsmTransition("Failure");
    *console_<<"PixelSupervisor::stateFixedSoftError: --- Resuming from Soft Error Failed! ---"<<std::endl;
  }

  if ( theCalibObject_!=0 )
  {
    calibWorkloop_->activate();
    diagService_->reportError("PixelSupervisor::stateFixedSoftError: Activated calibWorkloop again.",DIAGINFO);
  }

  try {
    toolbox::Event::Reference e(new toolbox::Event("ResumeFromSoftError", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    diagService_->reportError("[PixelSupervisor::stateFixedSoftError] Invalid command "+stringF(e.what()),DIAGERROR);
  }

}

