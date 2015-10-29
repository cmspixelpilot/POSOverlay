/*************************************************************************
 * XDAQ Components for Pixel Online Software                             *
 * Copyright (C) 2007, Cornell University   	                         *
 * All rights reserved.                                                  *
 * Authors: Souvik Das, Anders Ryd, Karl Ecklund   		         *
 *************************************************************************/
// Fix the bug in workloop if not FED enabled.
// Merge Physics and EmulatedPhysics
// Move ResetFED before workloop start. d.dk. 7/14

#define READ_LASTDAC  // Enable the last dac writing
#define PILOT_FED
#define PILOT_TRANSSCOPE

#include "PixelFEDSupervisor/include/PixelFEDSupervisor.h"

#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO1Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO2Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO3Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/ErrorFIFODecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelDecodedFEDRawData.h"
#include "PixelUtilities/PixelFEDDataTools/include/AddressLevels.h"
#include "PixelUtilities/PixelFEDDataTools/include/TemperatureFIFODecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelLastDACTemperature.h"
#include "PixelUtilities/PixelSharedDataTools/include/SharedObject.h"
#include "PixelUtilities/PixelSharedDataTools/include/SharedObjectOwner.h"
#include "PixelUtilities/PixelSharedDataTools/include/PixelErrorCollection.h"
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"

#include "PixelCalibrations/include/PixelCalibrationFactory.h"

#include "PixelFEDInterface/include/PixelFEDFifoData.h"

#include "xdaq/Application.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/exception/Exception.h"
#include "xdaq/NamespaceURI.h"
#include "xdata/String.h"
#include "xdata/UnsignedInteger.h"
#include "PixelUtilities/Pixelb2inUtilities/include/Pixelb2inCommander.h"



#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelConfigurationVerifier.h"
#include "PixelCalibrations/include/PixelEfficiency2D.h"

#include "diagbag/DiagBagWizard.h"
#include "DiagCompileOptions.h"
#include "toolbox/convertstring.h" // for stringF(int) etc
#include "toolbox/BSem.h"
#include "iomanip"

#include <stdlib.h>
#include <iostream>
#include <limits>
#include <sys/time.h>

using namespace pos;

const unsigned int numberOfFEDs=40;
const unsigned int channelsPerFED=36;
const unsigned int opticalReceiversPerFED=3;

//
// provides factory method for instantion of PixelFEDSupervisor application
//
XDAQ_INSTANTIATOR_IMPL(PixelFEDSupervisor)
////////////////////////////////////////////////////////////////////////////
// Constructor

PixelFEDSupervisor::PixelFEDSupervisor(xdaq::ApplicationStub * s) 
                                throw (xdaq::exception::Exception)
                                : xdaq::Application(s),
                                SOAPCommander(this),
                                PixelFEDSupervisorConfiguration(&runNumber_,&outputDir_,this), 
                                executeReconfMethodMutex(toolbox::BSem::FULL),                                
                                fsm_("urn:toolbox-task-workloop:PixelFEDSupervisor"),
                                m_lock(toolbox::BSem::FULL)
        ,physicsRunningSentRunningDegraded(false)
        ,physicsRunningSentSoftErrorDetected(false)
				,phlock_(new toolbox::BSem(toolbox::BSem::FULL,true))
				,workloopContinue_(false)
{
  //gio
  diagService_ = new DiagBagWizard(
                                   ("ReconfigurationModule") ,
                                   this->getApplicationLogger(),
                                   getApplicationDescriptor()->getClassName(),
                                   getApplicationDescriptor()->getInstance(),
                                   getApplicationDescriptor()->getLocalId(),
                                   (xdaq::WebApplication *)this,
                                   "Pixel",
                                   "FEDSupervisor"
                                   );


  DIAG_DECLARE_USER_APP

  diagService_->reportError("The DiagSystem is installed --- this is a bogus error message",DIAGUSERINFO);

  //PixelFEDSpySupervisor
  std::stringstream ss;
  for(int i = 1;i<=6;i++) {  //FIXME why only 6?  What if there are more FEDS? How should they be counted when there are 3 crates?
    ss<<"SpyErrorCollection"<<i;
    pixel::SharedObject<pixel::PixelErrorCollection>* obj = new pixel::SharedObject<pixel::PixelErrorCollection >(ss.str());
    ErrorCollectionDataOwner.addSharedObject(obj);
    ss.str("");
  }
 
  // SOAP Bindings to State Machine Inputs
  xoap::bind(this, &PixelFEDSupervisor::Initialize, "Initialize", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::Configure, "Configure", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::Start, "Start", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::Stop, "Stop", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::Pause, "Pause", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::Resume, "Resume", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::Halt, "Halt", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::PrepareTTSTestMode, "PrepareTTSTestMode", XDAQ_NS_URI );
  xoap::bind(this, &PixelFEDSupervisor::TestTTS, "TestTTS", XDAQ_NS_URI );
  xoap::bind(this, &PixelFEDSupervisor::Reset, "Reset", XDAQ_NS_URI );
  xoap::bind(this, &PixelFEDSupervisor::Recover, "Recover", XDAQ_NS_URI);

  xoap::bind(this, &PixelFEDSupervisor::Reconfigure, "Reconfigure", XDAQ_NS_URI);

  // A SOAP callback used for generic handshaking by retrieving the FSM state
  xoap::bind(this, &PixelFEDSupervisor::FSMStateRequest, "FSMStateRequest", XDAQ_NS_URI);

  // SOAP Bindings to Low Level Commands and Specific Algorithms
  xoap::bind(this, &PixelFEDSupervisor::ReloadFirmware, "ReloadFirmware", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::ResetFEDs, "ResetFEDs", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::SetChannelOffsets, "SetChannelOffsets", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::FillTestDAC, "FillTestDAC", XDAQ_NS_URI );
  xoap::bind(this, &PixelFEDSupervisor::EnableFIFO3, "EnableFIFO3", XDAQ_NS_URI );
  xoap::bind(this, &PixelFEDSupervisor::VMETrigger, "VMETrigger", XDAQ_NS_URI );
  xoap::bind(this, &PixelFEDSupervisor::BaselineRelease, "BaselineRelease", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::BaselineHold, "BaselineHold", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::BaselineMonitor, "BaselineMonitor", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::ReadFIFO, "ReadFIFO", XDAQ_NS_URI );
  xoap::bind(this, &PixelFEDSupervisor::ReadErrorFIFO, "ReadErrorFIFO", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::ReadDataAndErrorFIFO, "ReadDataAndErrorFIFO", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::ReadLastDACFIFO, "readLastDACFIFO", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::SetADC1V2VEnMass, "SetADC1V2VEnMass", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::SetADC1V2VOneChannel, "SetADC1V2VOneChannel", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::SetFEDOffsetsEnMass, "SetFEDOffsetsEnMass", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::ResetFEDsEnMass, "ResetFEDsEnMass", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::SetPrivateWord, "SetPrivateWord", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::ToggleChannels, "ToggleChannels", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::SetScopeChannel, "SetScopeChannel", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::SetScopeChannels, "SetScopeChannels", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::JMTJunk, "JMTJunk", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::ArmDigFEDOSDFifo, "ArmDigFEDOSDFifo", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::ReadDigFEDOSDFifo, "ReadDigFEDOSDFifo", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::ReadTTSFIFO, "ReadTTSFIFO", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::ReadLastDACFIFO, "ReadLastDACFIFO", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::SetPhasesDelays, "SetPhasesDelays", XDAQ_NS_URI);
  xoap::bind(this, &PixelFEDSupervisor::SetControlRegister, "SetControlRegister", XDAQ_NS_URI);

  xoap::bind(this, &PixelFEDSupervisor::FEDCalibrations, "FEDCalibrations", XDAQ_NS_URI);

  xoap::bind(this, &PixelFEDSupervisor::beginCalibration, "BeginCalibration", XDAQ_NS_URI );
  xoap::bind(this, &PixelFEDSupervisor::endCalibration, "EndCalibration", XDAQ_NS_URI );
  xoap::bind(this, &PixelFEDSupervisor::calibrationEvent, "calibrationEvent", XDAQ_NS_URI );

  // Soft Error Stuff
  xoap::bind(this, &PixelFEDSupervisor::FixSoftError, "FixSoftError", XDAQ_NS_URI );
  xoap::bind(this, &PixelFEDSupervisor::ResumeFromSoftError, "ResumeFromSoftError", XDAQ_NS_URI);

  b2in::nub::bind(this, &PixelFEDSupervisor::b2inEvent);

#ifdef RUBUILDER
  // I2O Binding with RU Builder TA Unit
  i2o::bind(this,&PixelFEDSupervisor::callback_TA_CREDIT,I2O_TA_CREDIT,XDAQ_ORGANIZATION_ID);
#endif

  xgi::bind(this, &PixelFEDSupervisor::Default, "Default");
  xgi::bind(this, &PixelFEDSupervisor::StateMachineXgiHandler, "StateMachineXgiHandler");
  xgi::bind(this, &PixelFEDSupervisor::LowLevelCommands, "LowLevelCommands");
  xgi::bind(this, &PixelFEDSupervisor::LowLevelXgiHandler, "LowLevelXgiHandler");

  //DIAGNOSTIC REQUESTED CALLBACK
  xgi::bind(this,&PixelFEDSupervisor::configureDiagSystem, "configureDiagSystem");
  xgi::bind(this,&PixelFEDSupervisor::applyConfigureDiagSystem, "applyConfigureDiagSystem");
  xgi::bind(this,&PixelFEDSupervisor::callDiagSystemPage, "callDiagSystemPage");

  // Defining the states of the State Machine
  fsm_.addState('I', "Initial", this, &PixelFEDSupervisor::stateChanged);
  fsm_.addState('H', "Halted", this, &PixelFEDSupervisor::stateChanged);
  fsm_.addState('c', "Configuring", this, &PixelFEDSupervisor::stateConfiguring);
  fsm_.addState('C', "Configured", this, &PixelFEDSupervisor::stateChanged);
  fsm_.addState('R', "Running", this, &PixelFEDSupervisor::stateChanged);
  fsm_.addState('P', "Paused", this, &PixelFEDSupervisor::stateChanged);
  fsm_.addState('T', "TTSTestMode", this, &PixelFEDSupervisor::stateChanged);

  //Adding Soft Error Detection Stuff

  fsm_.addState('s', "FixingSoftError", this, &PixelFEDSupervisor::stateFixingSoftError);
  fsm_.addState('S', "FixedSoftError", this, &PixelFEDSupervisor::stateChanged);

  fsm_.setStateName('F',"Error");
  //fsm_.setFailedStateTransitionAction(this, &PixelFEDSupervisor::enteringError);
  fsm_.setFailedStateTransitionChanged(this, &PixelFEDSupervisor::stateChanged);

  // Defining the transitions of the State Machine
  fsm_.addStateTransition('I', 'H', "Initialize");
  fsm_.addStateTransition('H', 'c', "Configure");
  fsm_.addStateTransition('c', 'C', "ConfiguringDone");
  fsm_.addStateTransition('C', 'R', "Start");
  fsm_.addStateTransition('R', 'C', "Stop");
  fsm_.addStateTransition('R', 'P', "Pause");
  fsm_.addStateTransition('P', 'C', "Stop");
  fsm_.addStateTransition('P', 'R', "Resume");
  fsm_.addStateTransition('C', 'H', "Halt");
  fsm_.addStateTransition('P', 'H', "Halt");
  fsm_.addStateTransition('R', 'H', "Halt");
  fsm_.addStateTransition('F', 'H', "Halt");
  fsm_.addStateTransition('H', 'T', "PrepareTTSTestMode");
  fsm_.addStateTransition('T', 'T', "TestTTS");

  //Adding Soft Error Detection Stuff
  
  fsm_.addStateTransition('R', 's', "FixSoftError");
  fsm_.addStateTransition('s', 'S', "FixingSoftErrorDone");
  fsm_.addStateTransition('S', 'R', "ResumeFromSoftError");

  //error transitions
  fsm_.addStateTransition('c', 'F', "Failure");
  fsm_.addStateTransition('C', 'F', "Failure");
  fsm_.addStateTransition('P', 'F', "Failure");
  fsm_.addStateTransition('R', 'F', "Failure");
  fsm_.addStateTransition('F', 'F', "Failure");

  //Adding Soft Error Detection Stuff

  fsm_.addStateTransition('s','F',"Failure");
  fsm_.addStateTransition('S','F',"Failure");

  fsm_.setInitialState('I');
  fsm_.reset();

  // Miscellaneous variables initialised
  if (getenv("BUILD_HOME")==0){
    htmlbase_=std::string(getenv("XDAQ_ROOT"))+"/htdocs/PixelFEDSupervisor/html/";
    datbase_=std::string(getenv("XDAQ_ROOT"))+"/dat/PixelFEDInterface/dat/";
  }
  else{
    htmlbase_=std::string(getenv("BUILD_HOME"))+"/pixel/PixelFEDSupervisor/html/";
    datbase_=std::string(getenv("BUILD_HOME"))+"/pixel/PixelFEDInterface/dat/";  
  }


  crate_=this->getApplicationDescriptor()->getInstance();
  console_=new std::stringstream();
  eventNumber_=0;
  countLoopsThisRun_ = -1;
  this->getApplicationDescriptor()->setAttribute("icon","pixel/PixelFEDSupervisor/html/PixelFEDSupervisor.bmp");


  addressTablePtr_=0;
  busAdapter_=0;
  theGlobalKey_=0;
  theNameTranslation_=0;
  theDetectorConfiguration_=0;
  theFEDConfiguration_=0;
  theCalibObject_=0;
  theGlobalDelay25_=0;

  PixelSupervisor_=0;
  theFEDCalibrationBase_=0;

  // BEGIN - PixelMonitor: Robert

  // Create infospace:
  toolbox::net::URN monitorable = this->createQualifiedInfoSpace("PixelFEDSupervisor");
  std::cout << "InfoSpace was created" << std::endl;
  monitorInfoSpace = xdata::getInfoSpaceFactory()->get(monitorable.toString());

  monitorInfoSpace->lock(); // Thread safety

  // Initialize infospace variables.
  crateNumberPtr = new xdata::UnsignedInteger32();
  runNumberPtr = new xdata::UnsignedInteger32();
  crateTimeStampPtr = new xdata::UnsignedInteger32();
  crateTablePtr = new xdata::Table();
  errTable = new xdata::Table();
  errTableTimeStampPtr = new xdata::UnsignedInteger32();
  
  *crateNumberPtr = this->getApplicationDescriptor()->getInstance(); 
  *runNumberPtr = 0; // Not yet valid.
  *crateTimeStampPtr = time(NULL);
  *errTableTimeStampPtr = time(NULL);

  // Publish Infospace values.
  monitorInfoSpace->fireItemAvailable("crateTable", crateTablePtr);
  monitorInfoSpace->fireItemAvailable("crateNumber", crateNumberPtr);
  monitorInfoSpace->fireItemAvailable("runNumber", runNumberPtr);
  monitorInfoSpace->fireItemAvailable("errorTable", errTable);
  monitorInfoSpace->fireItemAvailable("crateTimeStamp", crateTimeStampPtr);
  monitorInfoSpace->fireItemAvailable("errTableTimeStamp", errTableTimeStampPtr);

  monitorInfoSpace->unlock(); // Thread safety.

  // Initialize variables for crate table and fed table.
  fedNumberPtr = new xdata::UnsignedInteger32();
  fedTablePtr = new xdata::Table();

  channelNumberPtr = new xdata::UnsignedInteger32();
  baselineCorrectionMeanPtr  = new xdata::Float();
  baselineCorrectionStdDevPtr  = new xdata::Float();
  
  numNORErrors = new xdata::UnsignedInteger32();
  numOOSErrors = new xdata::UnsignedInteger32();
  numTimeOutErrors = new xdata::UnsignedInteger32();
  
  *fedNumberPtr = 0;
  *channelNumberPtr = 0;
  *baselineCorrectionMeanPtr = 0; 
  *baselineCorrectionStdDevPtr = 0; 
  *numNORErrors = 0; 
  *numOOSErrors = 0;
  *numTimeOutErrors = 0;
  
  // END - PixelMonitor: Robert
  
  // Exporting the FSM state to this application's default InfoSpace
  state_=fsm_.getStateName(fsm_.getCurrentState());
  getApplicationInfoSpace()->fireItemAvailable("stateName", &state_);


  // Initialize the WorkLoop
  workloop_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("PixelFEDSupervisorWorkLoop", "waiting");
  //tempTransmitter_ = toolbox::task::bind (this, &PixelFEDSupervisor::readLastDACFIFO_workloop_soap, "readLastDACFIFO");
  physicsRunning_=toolbox::task::bind (this, &PixelFEDSupervisor::PhysicsRunning, "PhysicsRunning");

  // Add Application Descriptor of RU
  //RU_=getApplicationContext()->getApplicationGroup()->getApplicationDescriptor("I2OExample",0);
#ifdef RUBUILDER
  I2OEventDataBlockSender::CreateDefaultMemoryPool(this,400,30);
#endif

  std::stringstream timerName;
  timerName << getApplicationDescriptor()->getContextDescriptor()->getURL() << ":";
  timerName << getApplicationDescriptor()->getClassName() << ":" << getApplicationDescriptor()->getLocalId() << ":" << getApplicationDescriptor()->getInstance();
  toolbox::task::Timer * timer = toolbox::task::getTimerFactory()->createTimer(timerName.str());
  toolbox::TimeInterval interval(AUTO_UP_CONFIGURE_DELAY,0);
  toolbox::TimeVal start;
  start = toolbox::TimeVal::gettimeofday() + interval;
  timer->schedule( this, start,  0, "" );

}

PixelFEDSupervisor::~PixelFEDSupervisor()
{
  delete phlock_;
}

// Object responsible for sending data to shared memory listeners
pixel::SharedObjectOwner<pixel::PixelErrorCollection> PixelFEDSupervisor::ErrorCollectionDataOwner;

void PixelFEDSupervisor::Default (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{

  //gio
  // diagService_->reportError("Access PixelFEDSupervisor",DIAGINFO);
  //

  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  HTML2XGI(out, htmlbase_+"/Head.htm");
  *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  xgi::Utils::getPageHeader(*out, "Pixel Front End Driver Supervisor", fsm_.getStateName(fsm_.getCurrentState()));


  // Rendering the State Machine GUI

  std::set<std::string> allInputs=fsm_.getInputs();
  std::set<std::string> clickableInputs=fsm_.getInputs(fsm_.getCurrentState());
  std::set<std::string>::iterator i;
  std::string currentState=fsm_.getStateName(fsm_.getCurrentState());

  *out<<"<body>"<<endl;
  *out<<"<h2> Finite State Machine </h2>"<<endl;

  std::string url="/"+getApplicationDescriptor()->getURN();
  *out<<"If in doubt, click <a href=\""<<url<<"\">here</a> to refresh"<<std::endl;
  *out<<"<form name=\"input\" method=\"get\" action=\""<<url+"/StateMachineXgiHandler"<<"\" enctype=\"multipart/form-data\">"<<endl;

  *out<<" <table border cellpadding=10 cellspacing=0>"<<endl;
  *out<<"  <tr>"<<endl;
  *out<<"   <td> <b>Current State</b> <br/>"<<fsm_.getStateName(fsm_.getCurrentState())<<"</td>"<<endl;
  *out<<"   <td colspan=8>"<<endl;

  if (currentState=="Halted") {

    *out<<"    Global Key<br/>"<<std::endl;
    *out<<"    <input type=\"text\" name=\"GlobalKey\"/>"<<std::endl;

  } else if (currentState=="Configured") {

    *out<<"    Run Number<br/>"<<std::endl;
    *out<<"    <input type=\"text\" name=\"RunNumber\"/><br/>"<<std::endl;
 
  } else if (currentState=="TTSTestMode") {
    
    *out<<"       FED Number to TTS Test: <input type=\"text\" name=\"TTS_TEST_FED_ID\"/><br/>"<<std::endl;
    *out<<"       TTS Test Type (PATTERN/CYCLE): <input type=\"text\" name=\"TTS_TEST_TYPE\"/><br/>"<<std::endl;
    *out<<"       TTS Test Pattern (Integer 0-15): <input type=\"text\" name=\"TTS_TEST_PATTERN\"/><br/>"<<std::endl;
    *out<<"       TTS Test Cycles (Integer <8191): <input type=\"text\" name=\"TTS_TEST_CYCLES\"/><br/>"<<std::endl;
    
  }
 
  *out<<"   </td>"<<endl;
  *out<<"  </tr>"<<endl;
  *out<<"  <tr>"<<endl;

  for (i=allInputs.begin();i!=allInputs.end();i++)
    {
      *out<<"   <td>"<<endl;
      if (clickableInputs.find(*i)!=clickableInputs.end())
	*out<<"    <input type=\"submit\" name=\"Command\" value=\""<<(*i)<<"\"/>"<<endl;
      else
	*out<<"    <input type=\"submit\" disabled=\"true\" name=\"Command\" value=\""<<(*i)<<"\"/>"<<endl;
      *out<<"   </td>"<<endl;
    }

  *out<<"  </tr>"<<endl;
  *out<<" </table>"<<endl;
  *out<<"</form>"<<endl;

  *out<<"<br/>"<<endl;
  *out<<"<h2> Console Output </h2>"<<endl;
  *out<<"	<textarea rows=\"30\" cols=\"100\" readonly>"<<std::endl;
  *out<<(console_->str())<<std::endl;
  *out<<"	</textarea>"<<std::endl;

  *out<<"<hr/>"<<endl;

  // DiagSystem GUI
  std::string urlDiag_ = "/"; \
  urlDiag_ += getApplicationDescriptor()->getURN(); \
  urlDiag_ += "/callDiagSystemPage"; \
  *out << "<h2> Error Dispatcher </h2> "<<std::endl;
  *out << "<a href=" << urlDiag_ << ">Configure DiagSystem</a>" <<std::endl;
  *out << " <hr/> " << std::endl;

  // Rendering Low Level GUI

  *out<<"<h2>Low Level Commands</h2>"<<endl;

  if (fsm_.getStateName(fsm_.getCurrentState())=="Configured" ||
      fsm_.getStateName(fsm_.getCurrentState())=="Running" ||
      fsm_.getStateName(fsm_.getCurrentState())=="Paused") {
    for (FEDInterfaceMap::iterator iFED=FEDInterface_.begin();iFED!=FEDInterface_.end();++iFED) {
      std::string vmeBaseAddress_string=itoa(iFED->first);
      *out<<"<a href=\""<<url+"/LowLevelCommands?FEDBaseAddress="<<vmeBaseAddress_string<<"\" target=\"_blank\">"<<"FED with Base Address 0x"<<hex<<atoi(vmeBaseAddress_string.c_str())<<dec<<"</a><br/>"<<std::endl;
    }
  }

  if (fsm_.getStateName(fsm_.getCurrentState())=="Running") {
    *out<<"<br/><h2>Calibration Results</h2>"<<std::endl;
    *out<<"<a href=\""<<outputDir().substr(std::string(getenv("POS_OUTPUT_DIRS")).size())<<"/FEDBaselineCalibration_"<<crate_<<".html\">FED Baseline Calibration Results</a><br/>"<<std::endl;
    *out<<"<a href=\""<<outputDir().substr(std::string(getenv("POS_OUTPUT_DIRS")).size())<<"/AddressLevels_"<<crate_<<".html\" target=\"_blank\">Address Levels Calibration Results</a><br/>"<<std::endl;
  }

  *out<<"</body>"<<endl;

  *out<<"</html>"<<endl;
}

void PixelFEDSupervisor::LowLevelCommands (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{
  cgicc::Cgicc cgi(in);

  std::string vmeBaseAddress_string=cgi.getElement("FEDBaseAddress")->getValue();
  unsigned long vmeBaseAddress=atoi(vmeBaseAddress_string.c_str());
  //unsigned int fednumber=theFEDConfiguration_->FEDNumberFromCrateAndVMEBaseAddress(crate_, vmeBaseAddress);
  PixelFEDInterface* iFED=FEDInterface_[vmeBaseAddress];
  PixelFEDCard fedCard=iFED->getPixelFEDCard();

  std::string url="/"+getApplicationDescriptor()->getURN();

  *out<<"<html>"<<std::endl;

  *out<<"<head>"<<std::endl;
  *out<<"	<script language=\"JavaScript\" src=\"/pixel/PixelUtilities/PixelGUIUtilities/slider/slider.js\"></script>"<<std::endl;
  *out<<"	<script language=\"JavaScript\" src=\"/pixel/PixelUtilities/PixelGUIUtilities/vectorgraphics/wz_jsgraphics.js\"></script>"<<std::endl;
  *out<<"       <script language=\"JavaScript\" src=\"/pixel/PixelFEDSupervisor/html/ChannelScopeAJAX.js\"></script>"<<std::endl;

  *out<<"	<script language=\"JavaScript\">"<<std::endl;

  *out<<"	var A_TPL = {"<<std::endl;
  *out<<"		'b_vertical' : false, "<<std::endl;
  *out<<"		'b_watch' : true, "<<std::endl;
  *out<<"		'n_controlWidth': 255, "<<std::endl;
  *out<<"		'n_controlHeight': 16, "<<std::endl;
  *out<<"		'n_sliderWidth' : 19, "<<std::endl;
  *out<<"		'n_sliderHeight' : 16, "<<std::endl;
  *out<<"		'n_pathLeft' : 1, "<<std::endl;
  *out<<"		'n_pathTop' : 0, "<<std::endl;
  *out<<"		'n_pathLength' : 233, "<<std::endl;
  *out<<"		's_imgControl' : '/pixel/PixelUtilities/PixelGUIUtilities/slider/img/channeloffsetdac_bg.gif',"<<std::endl;
  *out<<"		's_imgSlider' : '/pixel/PixelUtilities/PixelGUIUtilities/slider/img/channeloffsetdac_sl.gif',"<<std::endl;
  *out<<"		'n_zIndex' : 1 "<<std::endl;
  *out<<"	}"<<std::endl;

  *out<<"	var B_TPL = {"<<std::endl;
  *out<<"		'b_vertical' : true, "<<std::endl;
  *out<<"		'b_watch' : true, "<<std::endl;
  *out<<"		'n_controlWidth': 17, "<<std::endl;
  *out<<"		'n_controlHeight': 126, "<<std::endl;
  *out<<"		'n_sliderWidth' : 17, "<<std::endl;
  *out<<"		'n_sliderHeight' : 9, "<<std::endl;
  *out<<"		'n_pathLeft' : 1, "<<std::endl;
  *out<<"		'n_pathTop' : 2, "<<std::endl;
  *out<<"		'n_pathLength' : 120, "<<std::endl;
  *out<<"		's_imgControl' : '/pixel/PixelUtilities/PixelGUIUtilities/slider/img/inputoffset_bg.gif',"<<std::endl;
  *out<<"		's_imgSlider' : '/pixel/PixelUtilities/PixelGUIUtilities/slider/img/inputoffset_sl.gif',"<<std::endl;
  *out<<"		'n_zIndex' : 1 "<<std::endl;
  *out<<"	}"<<std::endl;

  for (unsigned int ichannel=0;ichannel<channelsPerFED;++ichannel){
    *out<<"	var A_INIT"<<ichannel<<" = {"<<std::endl;
    *out<<"		's_form' : 0, "<<std::endl;
    *out<<"		's_name' : 'ChannelOffsetDAC"<<ichannel<<"', "<<std::endl;
    *out<<"		'n_minValue' : 0, "<<std::endl;
    *out<<"		'n_maxValue' : 255, "<<std::endl;
    *out<<"		'n_value' : "<<fedCard.offs_dac[ichannel]<<", "<<std::endl;
    *out<<"		'n_step' : 1 "<<std::endl;
    *out<<"	}"<<std::endl;
	}

  for (unsigned int iopto=0;iopto<opticalReceiversPerFED;++iopto){
    *out<<"	var B_INIT"<<iopto<<" = {"<<std::endl;
    *out<<"		's_form' : 0, "<<std::endl;
    *out<<"		's_name' : 'OpticalReceiverInput"<<iopto<<"', "<<std::endl;
    *out<<"		'n_minValue' : 0, "<<std::endl;
    *out<<"		'n_maxValue' : 15, "<<std::endl;
    *out<<"		'n_value' : "<<fedCard.opt_inadj[iopto]<<", "<<std::endl;
    *out<<"		'n_step' : 1 "<<std::endl;
    *out<<"	}"<<std::endl;
  }

  *out<<"	</script>"<<std::endl;

  *out<<"</head>"<<std::endl;

  *out<<"<body>"<<std::endl;

 

  *out<<"<h1> FED with Base Address 0x"<<std::hex<<vmeBaseAddress<<std::dec<<"</h1>"<<std::endl<<std::endl;

#ifndef PILOT_FED
  // GUI for FED Channel Offset DACs and Optical Receiver Input Offsets
  *out<<"<form name=\"input\" method=\"get\" action=\""<<url+"/LowLevelXgiHandler"<<"\" enctype=\"multipart/form-data\">"<<std::endl;
  *out<<"<table border=1>"<<std::endl;
  for (unsigned int ichannel=0;ichannel<channelsPerFED;++ichannel){
    *out<<" <tr>"<<std::endl;
    if (ichannel%12==0){
      unsigned int iopto=ichannel/12;
      *out<<" <td rowspan=\"12\">"<<std::endl;
      *out<<"  Optical Receiver <br> Input Offset<br/>"<<std::endl;
      *out<<"  <input type=\"text\" name=\"OpticalReceiverInput"<<iopto<<"\" id=\"OpticalReceiverInput"<<iopto<<"\" size=\"2\"/>"<<std::endl;
      *out<<"  <script language=\"JavaScript\">"<<"new slider(B_INIT"<<iopto<<", B_TPL);"<<"</script>"<<std::endl;
      *out<<" </td>"<<std::endl;
    }
    *out<<"  <td width=\"255\">"<<std::endl;
    *out<<"   FED Channel "<<ichannel+1<<" Offset DAC ";
    *out<<"   <input type=\"text\" name=\"ChannelOffsetDAC"<<ichannel<<"\" id=\"ChannelOffsetDAC"<<ichannel<<"\" size=\"3\"/><br/>";
    *out<<"   <script language=\"JavaScript\">"<<"new slider(A_INIT"<<ichannel<<", A_TPL);"<<"</script>"<<std::endl;
    *out<<"  </td>"<<std::endl;
    *out<<"  <td>"<<std::endl;
    *out<<"   <img id=\"channelScope"<<ichannel+1<<"\" onclick=\"toggleChannelScope('"<<url<<"', '"<<vmeBaseAddress_string<<"', "<<fednumber<<", 'channelScope"<<ichannel+1<<"', "<<ichannel+1<<")\" src=\"/pixel/PixelRun/FIFO1Signal_"<<fednumber<<"_"<<ichannel+1<<".gif\" />"<<std::endl;
    *out<<"  </td>"<<std::endl;
    *out<<" </tr>"<<std::endl;
  }
  *out<<"</table>"<<std::endl;
  *out<<"<input type=\"Submit\" name=\"Command\" value=\"SetChannelOffsets\">"<<std::endl;
  *out<<"<input type=\"hidden\" name=\"FEDBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<endl;
  *out<<"</form>"<<std::endl<<std::endl;
#endif

  // GUI for the FED Control and Mode Registers
  unsigned int cReg=fedCard.Ccntrl;
  unsigned int mReg=fedCard.modeRegister;
  std::string checked1, checked2;
  *out<<"<form name=\"input\" method=\"get\" action=\""<<url+"/LowLevelXgiHandler"<<"\" enctype=\"multipart/form-data\">"<<std::endl;

  *out<<"<fieldset><legend>Control and Mode Registers</legend>"<<std::endl;
  *out<<"Control Register<br/><br/>"<<std::endl;

  *out<<"Transparent Mode ";
  if ((cReg & 0x1)==0) {checked1="checked"; checked2="";}
  else {checked1=""; checked2="checked";}
  *out<<"<input type=\"radio\" name=\"TransparentMode\" value=\"Disable\" "<<checked1<<">Disable";
  *out<<"<input type=\"radio\" name=\"TransparentMode\" value=\"Enable\" "<<checked2<<">Enable<br>"<<std::endl;

  *out<<"Transparent Gate Start by ";
  if ((cReg & 0x2)==0) {checked1="checked"; checked2="";}
  else {checked1=""; checked2="checked";}
  *out<<"<input type=\"radio\" name=\"TranspGateStart\" value=\"L1A\" "<<checked1<<">L1A";
  *out<<"<input type=\"radio\" name=\"TranspGateStart\" value=\"OPTO\" "<<checked2<<">VME or EFT (OPTO Module)<br>"<<std::endl;

  *out<<"Use simulated test-DAC ";
  if ((cReg & 0x4)==0) {checked1="checked"; checked2="";}
  else {checked1=""; checked2="checked";}
  *out<<"<input type=\"radio\" name=\"DACData\" value=\"Disable\" "<<checked1<<">Disable";
  *out<<"<input type=\"radio\" name=\"DACData\" value=\"Enable\" "<<checked2<<">Enable<br>"<<std::endl;

  *out<<"Event number generated by ";
  if ((cReg & 0x8)==0) {checked1="checked"; checked2="";}
  else {checked1=""; checked2="checked";}
  *out<<"<input type=\"radio\" name=\"EventNumber\" value=\"TTC\" "<<checked1<<">TTC";
  *out<<"<input type=\"radio\" name=\"EventNumber\" value=\"VME\" "<<checked2<<">VME<br>"<<std::endl;

  *out<<"L1A Triggers from TTCrx  ";
  if ((cReg & 0x10)==0) {checked1="checked"; checked2="";}
  else {checked1=""; checked2="checked";}
  *out<<"<input type=\"radio\" name=\"L1A\" value=\"Disable\" "<<checked1<<">Disable";
  *out<<"<input type=\"radio\" name=\"L1A\" value=\"Enable\" "<<checked2<<">Enable<br>"<<std::endl;

  *out<<"EFT Signals from the OPTO Module ";
  if ((cReg & 0x20)==0) {checked1="checked"; checked2="";}
  else {checked1=""; checked2="checked";}
  *out<<"<input type=\"radio\" name=\"EFT\" value=\"Disable\" "<<checked1<<">Disable";
  *out<<"<input type=\"radio\" name=\"EFT\" value=\"Enable\" "<<checked2<<">Enable<br>"<<std::endl;

  *out<<"TTSReady ";
  if ((cReg & 0x10000)==0) {checked1="checked"; checked2="";}
  else {checked1=""; checked2="checked";}
  *out<<"<input type=\"radio\" name=\"TTSReady\" value=\"Disable\" "<<checked1<<">Disable";
  *out<<"<input type=\"radio\" name=\"TTSReady\" value=\"Enable\" "<<checked2<<">Enable<br>"<<std::endl;

  *out<<"TTS Error ";
  if ((cReg & 0x20000)==0) {checked1="checked"; checked2="";}
  else {checked1=""; checked2="checked";}
  *out<<"<input type=\"radio\" name=\"TTSError\" value=\"Disable\" "<<checked1<<">Disable";
  *out<<"<input type=\"radio\" name=\"TTSError\" value=\"Enable\" "<<checked2<<">Enable<br>"<<std::endl;

  *out<<"TTS Out of Sync ";
  if ((cReg & 0x40000)==0) {checked1="checked"; checked2="";}
  else {checked1=""; checked2="checked";}
  *out<<"<input type=\"radio\" name=\"OUTofSYN\" value=\"Disable\" "<<checked1<<">Disable";
  *out<<"<input type=\"radio\" name=\"OUTofSYN\" value=\"Enable\" "<<checked2<<">Enable<br/><br/><br/>"<<std::endl;

  *out<<"Mode Register<br/><br/>"<<std::endl;

  *out<<"S-Link ";
  if ((mReg & 0x1)==0) {checked1=""; checked2="checked";}
  else {checked1="checked"; checked2="";}
  *out<<"<input type=\"radio\" name=\"SLink\" value=\"Disable\" "<<checked1<<">Disable";
  *out<<"<input type=\"radio\" name=\"SLink\" value=\"Enable\" "<<checked2<<">Enable<br>"<<std::endl;

  *out<<"Write Spy Memory ";
  if ((mReg & 0x2)==0) {checked1=""; checked2="checked";}
  else {checked1="checked"; checked2="";}
  *out<<"<input type=\"radio\" name=\"SpyMem\" value=\"Disable\" "<<checked1<<">Disable";
  *out<<"<input type=\"radio\" name=\"SpyMem\" value=\"Enable\" "<<checked2<<">Enable<br>"<<std::endl;

  *out<<"S-Link ";
  if ((mReg & 0x4)==0) {checked1=""; checked2="checked";}
  else {checked1="checked"; checked2="";}
  *out<<"<input type=\"radio\" name=\"URESET\" value=\"Enable\" "<<checked1<<">Reset, or"<<std::endl;
  *out<<"<input type=\"radio\" name=\"URESET\" value=\"Disable\" "<<checked2<<">Let it be<br/>";

  *out<<"S-Link LFF ";
  if ((mReg & 0x8)==0) {checked1=""; checked2="checked";}
  else {checked1="checked"; checked2="";}
  *out<<"<input type=\"radio\" name=\"LFF\" value=\"Disable\" "<<checked1<<">Ignore";
  *out<<"<input type=\"radio\" name=\"LFF\" value=\"Enable\" "<<checked2<<">Consider<br>"<<std::endl;

  *out<<"<input type=\"submit\" name=\"Command\" value=\"SetControlRegister\"/>"<<std::endl;
  *out<<"</fieldset>"<<std::endl;

  *out<<"<input type=\"hidden\" name=\"FEDBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<endl;
  *out<<"</form>"<<endl;

  // The Various FIFOs are formatted neatly in a Table
  *out<<"<table border=1>"<<std::endl;
  *out<<" <tr>"<<std::endl;
  // Data FIFO 1
  *out<<"  <td bgcolor=\"LightBlue\">"<<std::endl;
  *out<<"   <h3>Data FIFO 1</h3>"<<std::endl;
  *out<<"   <form name=\"input\" method=\"get\" action=\""<<url+"/LowLevelXgiHandler"<<"\" enctype=\"multipart/form-data\">"<<std::endl;
  *out<<"    <input type=\"radio\" name=\"Mode\" value=\"Transparent\"";
  if ((cReg & 0x1)==1) { *out<<" checked "; }
  *out<<">Transparent Mode, </input>, <input type=\"radio\" name=\"Mode\" value=\"Normal\"";
  if ((cReg & 0x1)==0) { *out<<" checked "; }
  *out<<">Normal Mode</input><br/>"<<std::endl;
  *out<<"    Channel: <select name=\"Channel\">"<<std::endl;
  for (unsigned int ichannel=1;ichannel<=channelsPerFED;++ichannel) {
    *out<<"     <option value=\""<<ichannel<<"\">"<<ichannel<<std::endl;
  }
  *out<<"    </select>"<<std::endl;
  *out<<"    <input type=\"submit\" name=\"Command\" value=\"ReadDataFIFO1\"/>"<<std::endl;
  *out<<"    <input type=\"hidden\" name=\"FEDBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<endl;
  *out<<"   </form>"<<endl;
  *out<<"   <textarea rows=\"50\" cols=\"50\" readonly>"<<std::endl;
  *out<<((dataFIFO1_[vmeBaseAddress])->str())<<std::endl;
  *out<<"   </textarea>"<<std::endl;
  *out<<"  </td>"<<std::endl;
  // Data FIFO 2
  *out<<"  <td bgcolor=\"AquaMarine\">"<<std::endl;
  *out<<"   <h3>Data FIFO 2</h3>"<<std::endl;
  *out<<"   <form name=\"input\" method=\"get\" action=\""<<url+"/LowLevelXgiHandler"<<"\" enctype=\"multipart/form-data\">"<<std::endl;
  *out<<"    <input type=\"submit\" name=\"Command\" value=\"ReadDataFIFO2\"/>"<<std::endl;
  *out<<"    <input type=\"hidden\" name=\"FEDBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<endl;
  *out<<"   </form>"<<std::endl;
  *out<<"   <textarea rows=\"50\" cols=\"50\" readonly>"<<std::endl;
  *out<<((dataFIFO2_[vmeBaseAddress])->str())<<std::endl;
  *out<<"   </textarea>"<<std::endl;
  *out<<"  </td>"<<std::endl;
  // Data FIFO 3
  *out<<"  <td bgcolor=\"Cyan\">"<<std::endl;
  *out<<"   <h3>Data FIFO 3</h3>"<<std::endl;
  *out<<"   <form name=\"input\" method=\"get\" action=\""<<url+"/LowLevelXgiHandler"<<"\" enctype=\"multipart/form-data\">"<<std::endl;
  *out<<"    <input type=\"submit\" name=\"Command\" value=\"ReadDataFIFO3\"/>"<<std::endl;
  *out<<"    <input type=\"hidden\" name=\"FEDBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<endl;
  *out<<"   </form>"<<std::endl;
  *out<<"   <textarea rows=\"50\" cols=\"50\" readonly>"<<std::endl;
  *out<<((dataFIFO3_[vmeBaseAddress])->str())<<std::endl;
  *out<<"   </textarea>"<<std::endl;
  *out<<"  </td>"<<std::endl;
  *out<<" </tr>"<<std::endl;
  *out<<" <tr>"<<std::endl;
  // Error FIFO
  *out<<"  <td bgcolor=\"Green\">"<<std::endl;
  *out<<"   <h3>Error FIFO</h3>"<<std::endl;
  *out<<"   <form name=\"input\" method=\"get\" action=\""<<url+"/LowLevelXgiHandler"<<"\" enctype=\"multipart/form-data\">"<<std::endl;
  *out<<"    <input type=\"submit\" name=\"Command\" value=\"ReadErrorFIFO\"/>"<<std::endl;
  *out<<"    <input type=\"hidden\" name=\"FEDBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<std::endl;
  *out<<"   </form>"<<std::endl;
  *out<<"   <textarea rows=\"50\" cols=\"50\" readonly>"<<std::endl;
  *out<<((errorFIFO_[vmeBaseAddress])->str())<<std::endl;
  *out<<"   </textarea>"<<std::endl;
  *out<<"  </td>"<<std::endl;
  // Temperature FIFO
  *out<<"  <td bgcolor=\"Coral\">"<<std::endl;
  *out<<"   <h3>Last DAC / Temperature FIFO</h3>"<<std::endl;
  *out<<"   <form name=\"input\" method=\"get\" action=\""<<url+"/LowLevelXgiHandler"<<"\" enctype=\"multipart/form-data\">"<<std::endl;
  *out<<"    <input type=\"submit\" name=\"Command\" value=\"ReadLastDACFIFO\"/>"<<std::endl;
  *out<<"    <input type=\"hidden\" name=\"FEDBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<std::endl;
  *out<<"   </form>"<<std::endl;
  *out<<"   <textarea rows=\"50\" cols=\"50\" readonly>"<<std::endl;
  *out<<((tempFIFO_[vmeBaseAddress])->str())<<std::endl;
  *out<<"   </textarea>"<<std::endl;
  *out<<"  </td>"<<std::endl;
  // TTS FIFO
  *out<<"  <td bgcolor=\"Olive\">"<<std::endl;
  *out<<"   <h3>Trigger & Throttling System FIFO</h3>"<<std::endl;
  *out<<"   <form name=\"input\" method=\"get\" action=\""<<url+"/LowLevelXgiHandler"<<"\" enctype=\"multipart/form-data\">"<<std::endl;
  *out<<"    <input type=\"submit\" name=\"Command\" value=\"ReadTTSFIFO\"/>"<<std::endl;
  *out<<"    <input type=\"hidden\" name=\"FEDBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<endl;
  *out<<"   </form>"<<std::endl<<std::endl;
  *out<<"   <textarea rows=\"50\" cols=\"50\" readonly>"<<std::endl;
  *out<<((ttsFIFO_[vmeBaseAddress])->str())<<std::endl;
  *out<<"   </textarea>"<<std::endl;
  *out<<"  </td>"<<std::endl;
  *out<<" </tr>"<<std::endl;
  *out<<"</table>"<<std::endl;

  //Enable FIFO 3 -- Is this required any more?
  *out<<"<form name=\"input\" method=\"get\" action=\""<<url+"/LowLevelXgiHandler"<<"\" enctype=\"multipart/form-data\">";
  HTML2XGI(out, htmlbase_+"/EnableFIFO3.htm");
  *out<<"<input type=\"hidden\" name=\"FEDBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<endl;
  *out<<"</form>"<<endl;

#ifndef PILOT_FED
  // Baseline Release, Set and Hold
  *out<<"<table width=100% border=1>"<<endl;
  *out<<"<tr>"<<endl;

  *out<<"<td>"<<endl;
  *out<<"  <form name=\"input\" method=\"get\" action=\""<<url+"/LowLevelXgiHandler"<<"\" enctype=\"multipart/form-data\">";
  *out<<"    <input type=\"hidden\" name=\"FEDBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<endl;
  *out<<"    <input type=\"submit\" name=\"Command\" value=\"BaselineRelease\"/>"<<endl;
  *out<<"  </form>"<<endl;
  *out<<"</td>"<<endl;

  *out<<"<td>"<<endl;
  *out<<"  <form name=\"input\" method=\"get\" action=\""<<url+"/LowLevelXgiHandler"<<"\" enctype=\"multipart/form-data\">";
  *out<<"    <input type=\"hidden\" name=\"FEDBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<endl;
  *out<<"    FPGA N  Target Baseline = <input type=\"text\" name=\"Nbaseln\" value=\""<<(fedCard.Nbaseln & 0xfff)<<"\"/><br/>"<<endl;
  *out<<"    FPGA NC Target Baseline = <input type=\"text\" name=\"NCbaseln\" value=\""<<(fedCard.NCbaseln & 0xfff)<<"\"/><br/>"<<endl;
  *out<<"    FPGA SC Target Baseline = <input type=\"text\" name=\"SCbaseln\" value=\""<<(fedCard.SCbaseln & 0xfff)<<"\"/><br/>"<<endl;
  *out<<"    FPGA SC Target Baseline = <input type=\"text\" name=\"Sbaseln\" value=\""<<(fedCard.Sbaseln & 0xfff)<<"\"/><br/>"<<endl;
  *out<<"   <input type=\"submit\" name=\"Command\" value=\"BaselineSet\"/>"<<endl;
  *out<<"  </form>"<<endl;
  *out<<"</td>"<<endl;

  *out<<"<td>"<<endl;
  *out<<"  <form name=\"input\" method=\"get\" action=\""<<url+"/LowLevelXgiHandler"<<"\" enctype=\"multipart/form-data\">";
  *out<<"    <input type=\"hidden\" name=\"FEDBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<endl;
  *out<<"    <input type=\"submit\" name=\"Command\" value=\"BaselineHold\"/>"<<endl;
  *out<<"  </form><br/>"<<endl;
  *out<<"</td>"<<endl;
  *out<<"</table>"<<endl;

  // Set Phases and Delays
  *out<<"<form name=\"input\" method=\"get\" action=\""<<url+"/LowLevelXgiHandler"<<"\" enctype=\"multipart/form-data\">";
  HTML2XGI(out, htmlbase_+"/SetPhasesDelays.htm");
  *out<<"<input type=\"hidden\" name=\"FEDBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<endl;
  *out<<"</form>"<<endl;
#endif

  // Reload Firmware and Reset FED
  *out<<"<form name=\"input\" method=\"get\" action=\""<<url+"/LowLevelXgiHandler"<<"\" enctype=\"multipart/form-data\">";
  *out<<"<input type=\"submit\" name=\"Command\" value=\"ReloadFirmware\"/>"<<std::endl;
  *out<<"<input type=\"hidden\" name=\"FEDBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<endl;
  *out<<"</form>"<<std::endl<<std::endl;

  *out<<"<form name=\"input\" method=\"get\" action=\""<<url+"/LowLevelXgiHandler"<<"\" enctype=\"multipart/form-data\">";
  *out<<"<input type=\"submit\" name=\"Command\" value=\"ResetFEDs\"/>"<<std::endl;
  *out<<"<input type=\"hidden\" name=\"FEDBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<endl;
  *out<<"</form>"<<std::endl<<std::endl;

  *out<<"</body>"<<std::endl;

  *out<<"</html>"<<std::endl;
}

//gio
void PixelFEDSupervisor::timeExpired (toolbox::task::TimerEvent& e)
{
  DIAG_EXEC_FSM_INIT_TRANS
}

// DiagSystem XGI Binding
void PixelFEDSupervisor::callDiagSystemPage(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  diagService_->getDiagSystemHtmlPage(in, out,getApplicationDescriptor()->getURN());
}



void PixelFEDSupervisor::StateMachineXgiHandler (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{
  cgicc::Cgicc cgi(in);

  std::string Command=cgi.getElement("Command")->getValue();

  if (Command=="Initialize")
    {
      xoap::MessageReference msg = MakeSOAPMessageReference("Initialize");
      xoap::MessageReference reply = Initialize(msg);
      if (Receive(reply)=="InitializeDone") *console_<<"Hooray, the FED has been initialised!"<<endl;
    }
  else if (Command=="Configure")
    {
      Attribute_Vector parametersXgi(1);
      parametersXgi.at(0).name_="GlobalKey";		parametersXgi.at(0).value_=cgi.getElement("GlobalKey")->getValue();

      xoap::MessageReference msg = MakeSOAPMessageReference("Configure", parametersXgi);
      xoap::MessageReference reply = Configure(msg);
      if (Receive(reply)=="ConfigureDone") *console_<<"Hooray, the FED has been configured!"<<endl;
    }
  else if (Command=="Start")
    {
      Attribute_Vector parametersXgi(1);
      parametersXgi.at(0).name_="RUN_NUMBER";  parametersXgi.at(0).value_=cgi.getElement("RunNumber")->getValue();

      xoap::MessageReference msg = MakeSOAPMessageReference("Start", parametersXgi);
      xoap::MessageReference reply = Start(msg);
      if (Receive(reply)=="StartDone") *console_<<"Hooray, the FED is running!"<<endl;
    }
  else if (Command=="Stop")
    {
      xoap::MessageReference msg = MakeSOAPMessageReference("Stop");
      xoap::MessageReference reply = Stop(msg);
      if (Receive (reply)=="StopDone") *console_<<"FEDs stopped from GUI."<<std::endl;
    }
  else if (Command=="Pause")
	{
	  xoap::MessageReference msg = MakeSOAPMessageReference("Pause");
	  xoap::MessageReference reply = Pause(msg);
	  if (Receive(reply)=="PauseDone") *console_<<"Hooray, the FED has been paused!"<<endl;
	}
  else if (Command=="Resume")
    {
      xoap::MessageReference msg = MakeSOAPMessageReference("Resume");
      xoap::MessageReference reply = Resume(msg);
      if (Receive(reply)=="ResumeDone") *console_<<"Hooray, the FED has resumed!"<<endl;
    }
  else if (Command=="Halt")
    {
      xoap::MessageReference msg = MakeSOAPMessageReference("Halt");
      xoap::MessageReference reply = Halt(msg);
      if (Receive(reply)=="HaltDone") *console_<<"Hooray, the FED has been halted!"<<endl;
    }
  else if (Command=="PrepareTTSTestMode") 
    {
      xoap::MessageReference msg = MakeSOAPMessageReference("PrepareTTSTestMode");
      xoap::MessageReference reply = PrepareTTSTestMode(msg);
      if (Receive(reply)!="PrepareTTSTestModeDone") *console_<<"All underlying PixelFEDSupervisors could not be prepared for TTS Test Mode by the browser button!"<<std::endl;
    } 
  else if (Command=="TestTTS") 
    {
      Attribute_Vector parametersXgi(4);
      parametersXgi[0].name_="TTS_TEST_FED_ID";  parametersXgi[0].value_=cgi.getElement("TTS_TEST_FED_ID")->getValue();
      parametersXgi[1].name_="TTS_TEST_TYPE";    parametersXgi[1].value_=cgi.getElement("TTS_TEST_TYPE")->getValue();
      parametersXgi[2].name_="TTS_TEST_PATTERN"; parametersXgi[2].value_=cgi.getElement("TTS_TEST_PATTERN")->getValue();
      parametersXgi[3].name_="TTS_TEST_SEQUENCE_REPEAT";  parametersXgi[3].value_=cgi.getElement("TTS_TEST_CYCLES")->getValue();
      xoap::MessageReference msg = MakeSOAPMessageReference("TestTTS", parametersXgi);
      xoap::MessageReference reply = TestTTS(msg);
      if (Receive(reply)!="TestTTSDone") *console_<<"All underlying PixelFEDSupervisors could not be TTS Tested by the browser button!"<<std::endl;
    }
  this->Default(in, out);
}

void PixelFEDSupervisor::LowLevelXgiHandler (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{
  cgicc::Cgicc cgi(in);

  std::string Command=cgi.getElement("Command")->getValue();
  std::string vmeBaseAddress_string=cgi.getElement("FEDBaseAddress")->getValue();
  
  if (Command=="SetChannelOffsets") {
assert(0);
    Attribute_Vector parametersXgi(channelsPerFED+opticalReceiversPerFED+1);
      
    for (unsigned int ichannel=0;ichannel<channelsPerFED;++ichannel){
      parametersXgi.at(ichannel).name_="ChannelOffsetDAC"+itoa(ichannel);
      parametersXgi.at(ichannel).value_=cgi.getElement("ChannelOffsetDAC"+itoa(ichannel))->getValue();
    }
    for (unsigned int iopto=0;iopto<opticalReceiversPerFED;++iopto){
      parametersXgi.at(channelsPerFED+iopto).name_="OpticalReceiverInput"+itoa(iopto);
      parametersXgi.at(channelsPerFED+iopto).value_=cgi.getElement("OpticalReceiverInput"+itoa(iopto))->getValue();
    }
    parametersXgi.at(channelsPerFED+opticalReceiversPerFED).name_="VMEBaseAddress";
    parametersXgi.at(channelsPerFED+opticalReceiversPerFED).value_=vmeBaseAddress_string;

    xoap::MessageReference msg = MakeSOAPMessageReference("SetChannelOffsets", parametersXgi);
    xoap::MessageReference reply = SetChannelOffsets(msg);
    if (Receive(reply)!="SetChannelOffsetsDone") { 
      diagService_->reportError("Channel Offset DACs or Optical Receiver Input Offsets could not be set!",DIAGWARN);
    }
      
  } else if (Command=="SetControlRegister") {

    Attribute_Vector parametersXgi(3);
    unsigned long controlRegister=0, modeRegister=0;
    
    if (cgi.getElement("TransparentMode")->getValue()=="Enable") controlRegister+=1;
    if (cgi.getElement("TranspGateStart")->getValue()=="OPTO") controlRegister+=2;
    if (cgi.getElement("DACData")->getValue()=="Enable") controlRegister+=4;
    if (cgi.getElement("EventNumber")->getValue()=="VME") controlRegister+=8;
    if (cgi.getElement("L1A")->getValue()=="Enable") controlRegister+=16;
    if (cgi.getElement("EFT")->getValue()=="Enable") controlRegister+=32;
    if (cgi.getElement("TTSReady")->getValue()=="Enable") controlRegister+=65536;
    if (cgi.getElement("TTSError")->getValue()=="Enable") controlRegister+=131072;
    if (cgi.getElement("OUTofSYN")->getValue()=="Enable") controlRegister+=262144;
    
    if (cgi.getElement("SLink")->getValue()=="Disable") modeRegister+=1;
    if (cgi.getElement("SpyMem")->getValue()=="Disable") modeRegister+=2;
    if (cgi.getElement("URESET")->getValue()=="Enable") modeRegister+=4;
    if (cgi.getElement("LFF")->getValue()=="Disable") modeRegister+=8;
    
    parametersXgi.at(0).name_="ControlRegister";	parametersXgi.at(0).value_=itoa(controlRegister);
    parametersXgi.at(1).name_="ModeRegister";	parametersXgi.at(1).value_=itoa(modeRegister);
    parametersXgi.at(2).name_="VMEBaseAddress";	parametersXgi.at(2).value_=vmeBaseAddress_string;
    
    xoap::MessageReference msg = MakeSOAPMessageReference("SetControlRegister", parametersXgi);
    xoap::MessageReference reply = SetControlRegister(msg);
    if (Receive(reply)!="SetControlRegisterDone") {
      diagService_->reportError("The Control Register of the FED could not be set from the GUI!",DIAGWARN);
    }

  } else if (Command=="ReadDataFIFO1") {

    Attribute_Vector parametersXgi(8);
    parametersXgi[0].name_="FIFO";           parametersXgi[0].value_="1";
    parametersXgi[1].name_="Mode";           parametersXgi[1].value_=cgi.getElement("Mode")->getValue();
    parametersXgi[2].name_="ShipTo";         parametersXgi[2].value_="Screen";
    parametersXgi[3].name_="Filename";       parametersXgi[3].value_="None";
    parametersXgi[4].name_="Additional";     parametersXgi[4].value_="None";
    parametersXgi[5].name_="Channel";        parametersXgi[5].value_=cgi.getElement("Channel")->getValue();
    parametersXgi[6].name_="Time";           parametersXgi[6].value_="First";
    parametersXgi[7].name_="VMEBaseAddress"; parametersXgi[7].value_=vmeBaseAddress_string;

    xoap::MessageReference msg = MakeSOAPMessageReference("ReadFIFO", parametersXgi);
    xoap::MessageReference reply = ReadFIFO(msg);
    if (Receive(reply)!="ReadFIFODone") {
      diagService_->reportError("The FIFO could not be read! in"+parametersXgi[1].value_+parametersXgi[5].value_+"!",DIAGERROR);
    }

  } else if (Command=="ReadDataFIFO2") {

    Attribute_Vector parametersXgi(8);
    parametersXgi[0].name_="FIFO";           parametersXgi[0].value_="2";
    parametersXgi[1].name_="Mode";           parametersXgi[1].value_="Normal";
    parametersXgi[2].name_="ShipTo";         parametersXgi[2].value_="Screen";
    parametersXgi[3].name_="Filename";       parametersXgi[3].value_="None";
    parametersXgi[4].name_="Additional";     parametersXgi[4].value_="None";
    parametersXgi[5].name_="Channel";        parametersXgi[5].value_="None";
    parametersXgi[6].name_="Time";           parametersXgi[6].value_="First";
    parametersXgi[7].name_="VMEBaseAddress"; parametersXgi[7].value_=vmeBaseAddress_string;

    xoap::MessageReference msg = MakeSOAPMessageReference("ReadFIFO", parametersXgi);
    xoap::MessageReference reply = ReadFIFO(msg);
    if (Receive(reply)!="ReadFIFODone") {
      diagService_->reportError("The FIFO could not be read!",DIAGERROR);
    }

  } else if (Command=="ReadDataFIFO3") {

    Attribute_Vector parametersXgi(8);
    parametersXgi[0].name_="FIFO";           parametersXgi[0].value_="3";
    parametersXgi[1].name_="Mode";           parametersXgi[1].value_="Normal";
    parametersXgi[2].name_="ShipTo";         parametersXgi[2].value_="Screen";
    parametersXgi[3].name_="Filename";       parametersXgi[3].value_="None";
    parametersXgi[4].name_="Additional";     parametersXgi[4].value_="None";
    parametersXgi[5].name_="Channel";        parametersXgi[5].value_="None";
    parametersXgi[6].name_="Time";           parametersXgi[6].value_="First";
    parametersXgi[7].name_="VMEBaseAddress"; parametersXgi[7].value_=vmeBaseAddress_string;

    xoap::MessageReference msg = MakeSOAPMessageReference("ReadFIFO", parametersXgi);
    xoap::MessageReference reply = ReadFIFO(msg);
    if (Receive(reply)!="ReadFIFODone") {
      diagService_->reportError("The FIFO could not be read!",DIAGERROR);
    }

  } else if (Command=="ReadErrorFIFO") {

    Attribute_Vector parametersXgi(4);
    parametersXgi[0].name_="ShipTo";         parametersXgi[0].value_="Screen";
    parametersXgi[1].name_="Filename";       parametersXgi[1].value_="None";
    parametersXgi[2].name_="VMEBaseAddress"; parametersXgi[2].value_=vmeBaseAddress_string;
    parametersXgi[3].name_="Time";           parametersXgi[3].value_="First";
	  
    xoap::MessageReference msg = MakeSOAPMessageReference("ReadErrorFIFO", parametersXgi);
    xoap::MessageReference reply = ReadErrorFIFO(msg);
    if (Receive(reply)!="ReadErrorFIFODone") {
      diagService_->reportError("The Error FIFO could not be read!",DIAGERROR);		  
    }

  } else if (Command=="ReadLastDACFIFO") {
	  
    xoap::MessageReference soapRequest = MakeSOAPMessageReference("readLastDACFIFO");
    xoap::MessageReference soapResponse = ReadLastDACFIFO(soapRequest);
    if ( Receive(soapResponse) != "readLastDACFIFODone" ) {
      std::cerr << "The last DAC FIFO could not be read!" << std::endl;
      diagService_->reportError("The last DAC FIFO could not be read!", DIAGERROR);		  
    }

  } else if (Command=="ReadTTSFIFO") {

    Attribute_Vector parametersXgi(1);
    parametersXgi[0].name_="VMEBaseAddress";parametersXgi[0].value_=vmeBaseAddress_string;
	  
    xoap::MessageReference msg = MakeSOAPMessageReference("ReadTTSFIFO", parametersXgi);
    xoap::MessageReference reply = ReadTTSFIFO(msg);
    if (Receive(reply)!="ReadTTSFIFODone") {
      diagService_->reportError("The TTS FIFO could not be read!",DIAGERROR);		  
    }

  }  else if (Command=="BaselineRelease") {
assert(0);
    Attribute_Vector parametersXgi(2);
    parametersXgi[0].name_="VMEBaseAddress";parametersXgi[0].value_=vmeBaseAddress_string;
    parametersXgi[1].name_="FEDChannel"; parametersXgi[1].value_="*";
    
    xoap::MessageReference msg = MakeSOAPMessageReference("BaselineRelease", parametersXgi);
    xoap::MessageReference reply=BaselineRelease(msg);
    if (Receive(reply)!="BaselineReleaseDone") {
      diagService_->reportError("Baselines could not be released!",DIAGERROR);		  		  
    }
 
  } else if (Command=="BaselineSet") {
assert(0);    
    Attribute_Vector parametersXgi(5);
    parametersXgi[0].name_="VMEBaseAddress"; parametersXgi[0].value_=vmeBaseAddress_string;
    parametersXgi[1].name_="Nbaseln"; parametersXgi[1].value_=cgi.getElement("Nbaseln")->getValue();
    parametersXgi[2].name_="NCbaseln"; parametersXgi[2].value_=cgi.getElement("NCbaseln")->getValue();
    parametersXgi[3].name_="SCbaseln"; parametersXgi[3].value_=cgi.getElement("SCbaseln")->getValue();
    parametersXgi[4].name_="Sbaseln"; parametersXgi[4].value_=cgi.getElement("Sbaseln")->getValue();
    
    xoap::MessageReference msg = MakeSOAPMessageReference("BaselineSet", parametersXgi);
    xoap::MessageReference reply = BaselineSet(msg);
    if (Receive(reply)!="BaselineSetDone") {
      diagService_->reportError("Baselines could not be set!",DIAGERROR);		  		  	
    }

  } else if (Command=="BaselineHold") {
assert(0);
    Attribute_Vector parametersXgi(2);
    parametersXgi[0].name_="VMEBaseAddress";parametersXgi[0].value_=vmeBaseAddress_string;
    parametersXgi[1].name_="FEDChannel"; parametersXgi[1].value_="*";
    
    xoap::MessageReference msg = MakeSOAPMessageReference("BaselineHold", parametersXgi);
    xoap::MessageReference reply=BaselineHold(msg);
    if (Receive(reply)!="BaselineHoldDone") {
      diagService_->reportError("Baselines could not be held!",DIAGERROR);		  		  
    }

  } else if (Command=="EnableFIFO3") {

    Attribute_Vector parametersXgi(1);
    parametersXgi[0].name_="VMEBaseAddress";parametersXgi[0].value_=vmeBaseAddress_string;

    xoap::MessageReference msg = MakeSOAPMessageReference("EnableFIFO3", parametersXgi);
    xoap::MessageReference reply = EnableFIFO3(msg);
    if (Receive(reply)!="EnableFIFO3Done") {
      diagService_->reportError("The FIFO 3 could not be enabled!",DIAGERROR);
    }

  } else if (Command=="ReloadFirmware") {

    Attribute_Vector parametersXgi(1);
    parametersXgi[0].name_="VMEBaseAddress";parametersXgi[0].value_=vmeBaseAddress_string;

    xoap::MessageReference msg = MakeSOAPMessageReference("ReloadFirmware", parametersXgi);
    xoap::MessageReference reply = ReloadFirmware(msg);
    if (Receive(reply)!="ReloadFirmwareDone") {
      diagService_->reportError("The firmware could not be reloaded on all FEDs!",DIAGERROR);
    }

  } else if (Command=="ResetFEDs") {
    
    Attribute_Vector parametersXgi(1);
    parametersXgi[0].name_="VMEBaseAddress";parametersXgi[0].value_=vmeBaseAddress_string;
    
    xoap::MessageReference msg = MakeSOAPMessageReference("ResetFEDs", parametersXgi);
    xoap::MessageReference reply = ResetFEDs(msg);
    if (Receive(reply)!="ResetFEDsDone") {
      diagService_->reportError("All the FEDs could not be reset!",DIAGERROR);
    }

  } else if (Command=="SetPhasesDelays") {
       assert(0);
    Attribute_Vector parametersXgi(4);
    parametersXgi[0].name_="Channel";	parametersXgi[0].value_=cgi.getElement("Channel")->getValue();
    parametersXgi[1].name_="Phase";		parametersXgi[1].value_=cgi.getElement("Phase")->getValue();
    parametersXgi[2].name_="Delay";		parametersXgi[2].value_=cgi.getElement("Delay")->getValue();
    parametersXgi[3].name_="VMEBaseAddress";parametersXgi[3].value_=vmeBaseAddress_string;

    xoap::MessageReference msg = MakeSOAPMessageReference("SetPhasesDelays", parametersXgi);
    xoap::MessageReference reply = SetPhasesDelays(msg);
    if (Receive(reply)!="SetPhasesDelaysDone") {
      diagService_->reportError("The phases and delays could not be enabled!",DIAGERROR);
    }

  } else if (Command=="UpdateChannelScope") {
    
    unsigned long vmeBaseAddress=atoi(vmeBaseAddress_string.c_str());
    unsigned int fednumber=theFEDConfiguration_->FEDNumberFromCrateAndVMEBaseAddress(crate_, vmeBaseAddress);
    std::string fednumber_string=itoa(fednumber);
    std::string channel_string=cgi.getElement("Channel")->getValue();
    
    PixelFEDInterface *iFED=FEDInterface_[vmeBaseAddress];
    
    int newEventNumber=iFED->readEventCounter();
    if (newEventNumber!=eventNumber_) {

      Attribute_Vector parameters(8);      
      parameters[0].name_="FIFO";           parameters[0].value_="1";
      parameters[1].name_="Mode";           parameters[1].value_="Transparent";
      parameters[2].name_="ShipTo";         parameters[2].value_="GIF";
      parameters[3].name_="Filename";       parameters[3].value_="FIFO1Signal_"+fednumber_string+"_"+channel_string+".gif";
      parameters[4].name_="Channel";        parameters[4].value_=cgi.getElement("Channel")->getValue();
      parameters[5].name_="VMEBaseAddress"; parameters[5].value_=vmeBaseAddress_string;
      parameters[6].name_="Additional";     parameters[6].value_="N/A";
      parameters[7].name_="Time";           parameters[7].value_="N/A";

      xoap::MessageReference msg = MakeSOAPMessageReference("ReadFIFO", parameters);
      xoap::MessageReference reply = ReadFIFO(msg);
      
      eventNumber_=newEventNumber;
      cgicc::HTTPResponseHeader response("HTTP/1.1", 200, "OK");
      response.addHeader("Content-Length", "100");
      response.addHeader("Content-Type", "text/html");
      out->setHTTPResponseHeader(response);

    }

  }

  this->LowLevelCommands(in, out);
}

xoap::MessageReference PixelFEDSupervisor::Initialize (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
	
  //initialize the shared objects //FIXME, why is this done again, having been in the constructor too?
  std::stringstream ss;
  for(int i = 1;i<=6;i++) {//FIXME why only 6?  What if there are more FEDS? How should they be counted when there are 3 crates?
    ss<<"SpyErrorCollection"<<i;
    pixel::SharedObject<pixel::PixelErrorCollection>* obj = ErrorCollectionDataOwner.getSharedObject(ss.str());
    obj->putObject(new pixel::PixelErrorCollection);		
    ss.str("");
  }
  //ErrorCollectionDataOwner.fireObjectUpdateEvent("SpyErrorCollection1");
	
  diagService_->reportError("--- INITIALIZE ---",DIAGINFO);
 
  // Detect PixelSupervisor
  try {
    PixelSupervisor_=getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor("PixelSupervisor", 0);

    diagService_->reportError("PixelFEDSupervisor::Initialize - Instance 0 of PixelSupervisor found.",DIAGINFO);
    *console_<<"PixelFEDSupervisor::Initialize - Instance 0 of PixelSupervisor found."<<std::endl;
  } catch (xdaq::exception::Exception& e) {
    PixelSupervisor_=0;
    
    diagService_->reportError("PixelFEDSupervisor::Initialize - Instance 0 of PixelSupervisor found.",DIAGERROR);
    *console_<<"PixelFEDSupervisor::Initialize - Instance 0 of PixelSupervisor not found!"<<std::endl;    
  }
  
  // Detect PixelDCStoFEDDpInterface
  try {
    PixelDCStoFEDDpInterface_ = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor("PixelDCStoFEDDpInterface", 0);
    diagService_->reportError("1 PixelDCStoFEDDpInterface descriptor found",DIAGINFO);
    *console_<<"PixelFEDSupervisor::Initialize - Instance 0 of PixelDCStoFEDDpInferface found."<<std::endl;
  } catch (xdaq::exception::Exception& e) {
    //I don't think we ever use this class, so lower severity from WARN to INFO
    diagService_->reportError("No PixelDCStoFEDDpInterface descriptor found",DIAGINFO);
    *console_<<"PixelFEDSupervisor::Initialize - Instance 0 of PixelDCStoFEDDpInterface not found!"<<std::endl;
  }

  try {
    toolbox::Event::Reference e(new toolbox::Event("Initialize", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    
    *console_<<"[PixelFEDSupervisor::Initialize] Initialize is an invalid command for the "<<state_.toString()<<" state."<<std::endl;
    diagService_->reportError("[PixelFEDSupervisor::Initialize] Initialize is an invalid command for the current state."+state_.toString(), DIAGERROR);
  }

  xoap::MessageReference reply=MakeSOAPMessageReference ("InitializeDone");
  // diagService_->reportError("PixelFEDSupervisor:: --- Initialising Done ---",DIAGINFO);
  diagService_->reportError("--- INIZIALIZATION DONE ---",DIAGINFO);
  return reply;
}

xoap::MessageReference PixelFEDSupervisor::Configure (xoap::MessageReference msg) throw (xoap::exception::Exception)
{

  diagService_->reportError("--- CONFIGURE ---",DIAGINFO);
  
  // Extract the Global Key from the SOAP message
  // Update the Global Key member data
  // Advertize the Global Key
  Attribute_Vector parameters(1);
  parameters[0].name_="GlobalKey";
  Receive(msg, parameters);
  theGlobalKey_ = new PixelConfigKey(atoi(parameters[0].value_.c_str()));
  if (theGlobalKey_==0) {
    diagService_->reportError("Failure to create GlobalKey",DIAGERROR);
    return MakeSOAPMessageReference("ConfigureFailed");
  }
  diagService_->reportError("The global key is " + stringF(theGlobalKey_->key()),DIAGDEBUG);
  diagService_->reportError("PixelFEDSupervisor::Configure: The Global Key was received as "+parameters.at(0).value_,DIAGDEBUG);
  *console_<<"PixelFEDSupervisor::Configure: The Global Key was received as "<<parameters.at(0).value_<<std::endl;
  
  xoap::MessageReference reply=MakeSOAPMessageReference("ConfigureDone");

  // That's it! Step to the Configuring state, and
  // relegate all further configuring to the stateConfiguring method.
  try {
    toolbox::Event::Reference e(new toolbox::Event("Configure", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    *console_<<"[PixelFEDSupervisor::Configure] Configure is an invalid command for the "<<state_.toString()<<" state."<<std::endl;
    diagService_->reportError("[PixelFEDSupervisor::Configure] Configure is an invalid command for the current state."+state_.toString(), DIAGERROR);
    reply=MakeSOAPMessageReference("ConfigureFailed");
  }
  
  diagService_->reportError("--- CONFIGURATION DONE ---",DIAGINFO);
  diagService_->reportError("PixelFEDSupervisor::Configure: A prompt SOAP reply is sent back before exiting function",DIAGINFO);
  
  return reply;
  
}

    

xoap::MessageReference PixelFEDSupervisor::Start (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  diagService_->reportError("--- START ---",DIAGINFO);
  Attribute_Vector parameter(1);
  parameter[0].name_="RUN_NUMBER";
  Receive(msg, parameter);
  runNumber_=parameter[0].value_;
  diagService_->reportError("Received SOAP message to Start. Run Number = "+runNumber_, DIAGINFO);
  unsigned int runNumberInt = atol((parameter[0].value_).c_str());
  if (runNumberInt > std::numeric_limits< xdata::UnsignedInteger32 >::max() || runNumberInt < 0) {
    diagService_->reportError("ERROR: Run Number = "+stringF(runNumberInt)+ "is greater than numerical limit for xdata::UnsignedInteger32: "+stringF(std::numeric_limits< xdata::UnsignedInteger32 >::max())+" or negative", DIAGERROR);
    return MakeSOAPMessageReference("StartFailed");
  }
  
  countLoopsThisRun_=-1;


  // BEGIN - PixelMonitor: Robert
  
  // Publish new run number to infospace.
  * runNumberPtr = atol(runNumber_.c_str());
  *crateTimeStampPtr = time(NULL);
  *errTableTimeStampPtr = time(NULL);
  
  // Book error table
  errTable->clear();
  errTable->addColumn("fedNumber","unsigned int 32");
  errTable->addColumn("channelNumber","unsigned int 32");
  errTable->addColumn("NORErrors","unsigned int 32");
  errTable->addColumn("OOSErrors","unsigned int 32");
  errTable->addColumn("TimeOutErrors","unsigned int 32"); 
  
  // Clear monitoring maps, e.g. baseline correction
  baselineCorrectionMap.clear();
  crateTablePtr->clear();
  errorCountMap.clear();
  lffMap.clear();
  fifoStatusMap.clear();
  ttsMap.clear();
  ttsStateChangeCounterMap.clear();
  NttcrxResets.clear();
  // don't clear maps that reflect counters in hardware
  // NttcrxSBError, NttcrxDBError, NttcrxSEU, NlockNorth

  // Update flashlist.
  std::list<std::string> names;
  names.push_back("runNumber");
  names.push_back("crateTimeStamp");
  monitorInfoSpace->fireItemGroupChanged(names, this);
  
  // END - PixelMonitor: Robert

  setupOutputDir();

  if ( (theCalibObject_==0) || (theCalibObject_->mode()=="EmulatedPhysics") ) {

    std::map<std::pair<unsigned long, unsigned int>, std::set<unsigned int> >::iterator i_vmeBaseAddressAndFEDNumberAndChannels=vmeBaseAddressAndFEDNumberAndChannels_.begin();
    for (;i_vmeBaseAddressAndFEDNumberAndChannels!=vmeBaseAddressAndFEDNumberAndChannels_.end();++i_vmeBaseAddressAndFEDNumberAndChannels) {
      unsigned int fednumber=i_vmeBaseAddressAndFEDNumberAndChannels->first.second;
      
      fedStuckInBusy_[fednumber] = 0; //clear the stuck in busy counter
      //      cout<<"fedStuckInBusy for fed "<<fednumber<<" set to "<< fedStuckInBusy_[fednumber]<<endl; //JMT very verbose debug info

      dataFile_[fednumber]=fopen((outputDir_+"/PhysicsDataFED_"+itoa(fednumber)+"_"+runNumber_+".dmp").c_str(), "wb");
      dataFileT_[fednumber]=fopen((outputDir_+"/TransFifo_"+itoa(fednumber)+"_"+runNumber_+".dmp").c_str(), "wb");
      dataFileS_[fednumber]=fopen((outputDir_+"/ScopeFifo_"+itoa(fednumber)+"_"+runNumber_+".dmp").c_str(), "wb");
      errorFile_[fednumber]=fopen((outputDir_+"/ErrorDataFED_"+itoa(fednumber)+"_"+runNumber_+".err").c_str(), "wb");
      setbuf(errorFile_[fednumber],NULL);  //disable buffering
      ttsFile_[fednumber]=fopen((outputDir_+"/TTSDataFED_"+itoa(fednumber)+"_"+runNumber_+".tts").c_str(), "wb");
      setbuf(ttsFile_[fednumber],NULL);  //disable buffering
      uint64_t runNumber64=atol(runNumber_.c_str());
      uint32_t runNumber32=atol(runNumber_.c_str());


      fwrite(&runNumber64, sizeof(uint64_t), 1, dataFile_[fednumber]);
      fwrite(&runNumber32, sizeof(uint32_t), 1, errorFile_[fednumber]);
#ifdef READ_LASTDAC
      lastDacFile_[fednumber]=fopen((outputDir_+"/LastDacFED_"+itoa(fednumber)+"_"+runNumber_+".ld").c_str(), "wb");
#endif
    }

    EndOfRunFEDReset(); //reset FED, clear SLink (at Start despite its name)	 

    //i don't know if we really have to protect a bool with a lock. But it can't hurt
    phlock_->take(); workloopContinue_=true; physicsRunningSentSoftErrorDetected = false;
    physicsRunningSentRunningDegraded = false;  phlock_->give();
    workloop_->activate();

    diagService_->reportError("PixelFEDSupervisor::Start. Calib object == 0. Physics data taking workloop activated.", DIAGINFO);
    *console_<<"Start. Calib Object == 0. Physics data taking workloop activated."<<std::endl;
  }
 
  // Create the FEDCalibration object.
  if ( theCalibObject_ != 0 ) {

    //Create pointers to give to the calibration object
    PixelFEDSupervisorConfiguration* pixFEDSupConfPtr = dynamic_cast <PixelFEDSupervisorConfiguration*> (this);
    SOAPCommander* soapCmdrPtr = dynamic_cast <SOAPCommander*> (this);
    theFEDCalibrationBase_=0;

    std::string mode=theCalibObject_->mode();

    PixelCalibrationFactory calibFactory;
    
    theFEDCalibrationBase_ = calibFactory.getFEDCalibration(mode,
							    pixFEDSupConfPtr, 
							    soapCmdrPtr);
    
    if (theFEDCalibrationBase_!=0){
      //Set control and mode register
      theFEDCalibrationBase_->initializeFED();
    }
  }

    
  try {
    toolbox::Event::Reference e(new toolbox::Event("Start", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
   
    *console_<<"[PixelFEDSupervisor::Start] Start is an invalid command for the "<<state_.toString()<<" state."<<std::endl;
    diagService_->reportError("[PixelFEDSupervisor::Start] Start is an invalid command for the current state."+state_.toString(), DIAGERROR);
  }

  xoap::MessageReference reply = MakeSOAPMessageReference("StartDone");
  return reply;
}

xoap::MessageReference PixelFEDSupervisor::Stop (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  PixelTimer stoptimer;
  stoptimer.start();
  diagService_->reportError("--- STOP ---",DIAGINFO);
  *console_<<"--- Stopping ---"<<std::endl;

  if ( (theCalibObject_==0) || (theCalibObject_->mode()=="EmulatedPhysics") ) {

    if (fsm_.getStateName(fsm_.getCurrentState())=="Running") {
      
      phlock_->take(); workloopContinue_=false; phlock_->give();
      workloop_->cancel();
      diagService_->reportError("PixelFEDSupervisor::Stop. Calib object == 0, physics workloop is cancelled.", DIAGINFO);
      *console_<<"Stop. CalibObject == 0. Physics workloop is cancelled."<<std::endl;
      
      std::map<std::pair<unsigned long, unsigned int>, std::set<unsigned int> >::iterator i_vmeBaseAddressAndFEDNumberAndChannels=vmeBaseAddressAndFEDNumberAndChannels_.begin();
      for (;i_vmeBaseAddressAndFEDNumberAndChannels!=vmeBaseAddressAndFEDNumberAndChannels_.end();++i_vmeBaseAddressAndFEDNumberAndChannels) {
	unsigned int fednumber=i_vmeBaseAddressAndFEDNumberAndChannels->first.second;
	diagService_->reportError("About to close files for fednumber="+stringF(fednumber), DIAGDEBUG);
	fclose(dataFile_[fednumber]);    dataFile_[fednumber]=0;
	fclose(dataFileT_[fednumber]);    dataFileT_[fednumber]=0;
	fclose(dataFileS_[fednumber]);    dataFileS_[fednumber]=0;
	fclose(errorFile_[fednumber]);   errorFile_[fednumber]=0;
        fclose(ttsFile_[fednumber]);     ttsFile_[fednumber]=0;
#ifdef READ_LASTDAC
        fclose(lastDacFile_[fednumber]); lastDacFile_[fednumber]=0;
#endif
      }
      diagService_->reportError("PixelFEDSupervisor::Stop. Done closing output files", DIAGINFO);

      
    } else if (fsm_.getStateName(fsm_.getCurrentState())=="Paused") {

      diagService_->reportError("PixelFEDSupervisor::Stop. Calib object == 0, physics workloop is cancelled.", DIAGINFO);
      *console_<<"Stop. CalibObject == 0. Physics workloop is cancelled."<<std::endl;
      
      std::map<std::pair<unsigned long, unsigned int>, std::set<unsigned int> >::iterator i_vmeBaseAddressAndFEDNumberAndChannels=vmeBaseAddressAndFEDNumberAndChannels_.begin();
      for (;i_vmeBaseAddressAndFEDNumberAndChannels!=vmeBaseAddressAndFEDNumberAndChannels_.end();++i_vmeBaseAddressAndFEDNumberAndChannels) {
        unsigned int fednumber=i_vmeBaseAddressAndFEDNumberAndChannels->first.second;
        fclose(dataFile_[fednumber]);     dataFile_[fednumber]=0;
        fclose(dataFileT_[fednumber]);     dataFileT_[fednumber]=0;
        fclose(dataFileS_[fednumber]);     dataFileS_[fednumber]=0;
        fclose(errorFile_[fednumber]);    errorFile_[fednumber]=0;
        fclose(ttsFile_[fednumber]);      ttsFile_[fednumber]=0;
#ifdef READ_LASTDAC
        fclose(lastDacFile_[fednumber]);  lastDacFile_[fednumber]=0;
#endif
      }
    }
    
    EndOfRunFEDReset(); //reset FED, clear SLink

  }

  closeOutputFiles();
  reportStatistics();

  if(theFEDCalibrationBase_) {
    diagService_->reportError("[PixelFEDSupervisor::Stop] There was a FEDCalibrationBase_", DIAGINFO);
    delete theFEDCalibrationBase_;
    theFEDCalibrationBase_=0;
    diagService_->reportError("But not anymore!", DIAGINFO);
  }

  try {
    toolbox::Event::Reference e(new toolbox::Event("Stop", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
   
    *console_<<"[PixelFEDSupervisor::Stop] Stop is an invalid command for the "<<state_.toString()<<" state."<<std::endl;
    diagService_->reportError("[PixelFEDSupervisor::Stop] Stop is an invalid command for the current state."+state_.toString(), DIAGERROR);
  }

  *console_<<"------------------"<<std::endl;  

  stoptimer.stop();
  diagService_->reportError("-- Exit STOP -- "+stringF(stoptimer.tottime()), DIAGINFO);

  xoap::MessageReference reply = MakeSOAPMessageReference("StopDone");
  return reply;
}



xoap::MessageReference PixelFEDSupervisor::Pause (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  PixelTimer pausetimer;
  pausetimer.start();
  diagService_->reportError("--- PAUSE ---",DIAGINFO);
  *console_<<"--- Pausing ---"<<std::endl;

  try {
    if ( (theCalibObject_==0) || (theCalibObject_->mode()=="EmulatedPhysics") ) {
      
      phlock_->take(); workloopContinue_=false; phlock_->give();
      workloop_->cancel();
      diagService_->reportError("Pause. Physics data taking workloop cancelled.", DIAGINFO);
      *console_<<"Pause. Physics data taking workloop cancelled."<<std::endl;
      
    }

    toolbox::Event::Reference e(new toolbox::Event("Pause", this));
    fsm_.fireEvent(e);

  } catch (toolbox::fsm::exception::Exception & e) {

    *console_<<"[PixelFEDSupervisor::Pause] Pause is an invalid command for the "<<state_.toString()<<" state."<<std::endl;
    diagService_->reportError("[PixelFEDSupervisor::Pause] Pause is an invalid command for the current state."+state_.toString(), DIAGERROR);

  }
  pausetimer.stop();
  diagService_->reportError("--- Exit PAUSE --- "+stringF(pausetimer.tottime()),DIAGINFO);
  xoap::MessageReference reply = MakeSOAPMessageReference("PauseDone");
  return reply;
}

xoap::MessageReference PixelFEDSupervisor::Resume (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  diagService_->reportError("--- RESUME ---",DIAGINFO);
  *console_<<"--- Resuming ---"<<std::endl;

  if ( (theCalibObject_==0) || (theCalibObject_->mode()=="EmulatedPhysics") ) {

    phlock_->take(); workloopContinue_=true; physicsRunningSentSoftErrorDetected = false;
    physicsRunningSentRunningDegraded = false;  phlock_->give();
    workloop_->activate();
    diagService_->reportError("Resume. Physics data taking workloop activated.", DIAGINFO);
    *console_<<"Resume. Physics data taking workloop activated."<<std::endl;

  }

  try {

    toolbox::Event::Reference e(new toolbox::Event("Resume", this));
    fsm_.fireEvent(e);

  } catch (toolbox::fsm::exception::Exception & e) {

    *console_<<"[PixelFEDSupervisor::Resume] Resume is an invalid command for the "<<state_.toString()<<" state."<<std::endl;
    diagService_->reportError("[PixelFEDSupervisor::Resume] Resume is an invalid command for the current state."+state_.toString(), DIAGERROR);


  }

  xoap::MessageReference reply = MakeSOAPMessageReference("ResumeDone");
  return reply;
}

xoap::MessageReference PixelFEDSupervisor::Halt (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  PixelTimer halttimer;
  halttimer.start();
  diagService_->reportError("--- HALT ---",DIAGINFO);
  
  if ( (theCalibObject_==0) || (theCalibObject_->mode()=="EmulatedPhysics") ) {

    if (fsm_.getStateName(fsm_.getCurrentState())=="Configured") {

      workloop_->remove(physicsRunning_);
       diagService_->reportError("PixelFEDSupervisor::Halt from Configured. Removed Physics data taking job from workloop.", DIAGINFO);
      *console_<<"Halt from Configured. Removed Physics data taking job from workloop."<<std::endl;

    } else if (fsm_.getStateName(fsm_.getCurrentState())=="Running") {

      diagService_->reportError("About to cancel workloop", DIAGDEBUG);
      phlock_->take(); workloopContinue_=false; phlock_->give();
      workloop_->cancel();
      workloop_->remove(physicsRunning_);
      diagService_->reportError("PixelFEDSupervisor::Halt. Physics workloop is cancelled.", DIAGINFO);

      std::map<std::pair<unsigned long, unsigned int>, std::set<unsigned int> >::iterator i_vmeBaseAddressAndFEDNumberAndChannels=vmeBaseAddressAndFEDNumberAndChannels_.begin();
      for (;i_vmeBaseAddressAndFEDNumberAndChannels!=vmeBaseAddressAndFEDNumberAndChannels_.end();++i_vmeBaseAddressAndFEDNumberAndChannels) {
        unsigned int fednumber=i_vmeBaseAddressAndFEDNumberAndChannels->first.second;
	diagService_->reportError("About to close output file for FED#"+stringF(fednumber), DIAGDEBUG);
        fclose(dataFile_[fednumber]);     dataFile_[fednumber]=0;
        fclose(dataFileT_[fednumber]);     dataFileT_[fednumber]=0;
        fclose(dataFileS_[fednumber]);     dataFileS_[fednumber]=0;
        fclose(errorFile_[fednumber]);    errorFile_[fednumber]=0;
        fclose(ttsFile_[fednumber]);      ttsFile_[fednumber]=0;
#ifdef READ_LASTDAC
        fclose(lastDacFile_[fednumber]);  lastDacFile_[fednumber]=0;
#endif
      }

      diagService_->reportError("Halt from Running. Cancelled Physics data taking workloop, removed job from it and closed files.", DIAGINFO);
      *console_<<"Halt from Running. Cancelled Physics data taking workloop, removed job from it and closed file."<<std::endl;

    } else if (fsm_.getStateName(fsm_.getCurrentState())=="Paused") {

      workloop_->remove(physicsRunning_);
      std::map<std::pair<unsigned long, unsigned int>, std::set<unsigned int> >::iterator i_vmeBaseAddressAndFEDNumberAndChannels=vmeBaseAddressAndFEDNumberAndChannels_.begin();
      for (;i_vmeBaseAddressAndFEDNumberAndChannels!=vmeBaseAddressAndFEDNumberAndChannels_.end();++i_vmeBaseAddressAndFEDNumberAndChannels) {
        unsigned int fednumber=i_vmeBaseAddressAndFEDNumberAndChannels->first.second;
        fclose(dataFile_[fednumber]);    dataFile_[fednumber]=0;
        fclose(dataFileT_[fednumber]);    dataFileT_[fednumber]=0;
        fclose(dataFileS_[fednumber]);    dataFileS_[fednumber]=0;
        fclose(errorFile_[fednumber]);   errorFile_[fednumber]=0;
        fclose(ttsFile_[fednumber]);     ttsFile_[fednumber]=0;
#ifdef READ_LASTDAC
        fclose(lastDacFile_[fednumber]); lastDacFile_[fednumber]=0;
#endif
      }
      diagService_->reportError("Halt from Paused. Removed Physics data taking job from workloop.Closed file.", DIAGINFO);
      *console_<<"Halt from Paused. Removed Physics data taking job from workloop.Closed file."<<std::endl;

    }

    EndOfRunFEDReset(); //reset FED, clear SLink

  }

  closeOutputFiles();
  reportStatistics();

  if(theFEDCalibrationBase_) {
    diagService_->reportError("[PixelFEDSupervisor::Halt] There was a FEDCalibrationBase_", DIAGTRACE);
    delete theFEDCalibrationBase_;
    theFEDCalibrationBase_=0;
    diagService_->reportError("But not anymore!", DIAGTRACE);
  }

  //FIXME why don't we cleanup global configuration data (theCalibObject_ theGlobalKey_ etc) here?

  string action="Halt";
  xoap::MessageReference reply = MakeSOAPMessageReference("HaltDone");
  //there is hardware access here
  try {
    deleteHardware();
  }
  catch (HAL::BusAdapterException & hwe) {
    diagService_->reportError("Hardware error while Halting the FEDs. Exception: "+string(hwe.what()), DIAGERROR);
    action="Failure";
    reply = MakeSOAPMessageReference("HaltFailed");
  }
  catch (xcept::Exception & xdaqe) {
    diagService_->reportError("XDAQ error while Halting the FEDs. Exception: "+string(xdaqe.what()), DIAGERROR);
    action="Failure";
    reply = MakeSOAPMessageReference("HaltFailed");
  }
  catch (...) {
    diagService_->reportError("Unknown error while Halting the FEDs", DIAGERROR);
    action="Failure";
    reply = MakeSOAPMessageReference("HaltFailed");
  }

  try {
    toolbox::Event::Reference e(new toolbox::Event(action, this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    *console_<<"[PixelFEDSupervisor::Halt] "<<action<<" is an invalid command for the "<<state_.toString()<<" state."<<std::endl;
    diagService_->reportError("[PixelFEDSupervisor::Halt] "+action+" is an invalid command for the current state."+state_.toString(), DIAGERROR);
    reply = MakeSOAPMessageReference("HaltFailed");
  }
  
  halttimer.stop();
  diagService_->reportError("-- Exit HALT -- "+stringF(halttimer.tottime()),DIAGINFO);
  return reply;
}

xoap::MessageReference PixelFEDSupervisor::Recover(xoap::MessageReference msg) {

  diagService_->reportError("-- Enter RECOVER --",DIAGINFO);
  if (state_!="Error") return MakeSOAPMessageReference("RecoverFailed"); //sanity

  string response="RecoverDone";

  //  if (theCalibObject_==0) {
  if (workloop_!=0) {
    try {
      if (workloop_->isActive()) {
	phlock_->take(); workloopContinue_=false; phlock_->give();
	workloop_->cancel(); //hopefully exception safe 
      }
      workloop_->remove(physicsRunning_); //will throw if the task was not submitted
    }
    catch (xcept::Exception & e) {
      diagService_->reportError("Failed to remove FED physics workloop (probably ok): "+string(e.what()), DIAGDEBUG);
    }
    
  }
  //  }

  //in HALT this is done only for Physics running (theCalibObject_==0).
  //now added code to always set these file pointers to 0 immediately after fclose.
  //this code should then be safe to run anytime
  std::map<std::pair<unsigned long, unsigned int>, std::set<unsigned int> >::iterator i_vmeBaseAddressAndFEDNumberAndChannels=vmeBaseAddressAndFEDNumberAndChannels_.begin();
  for (;i_vmeBaseAddressAndFEDNumberAndChannels!=vmeBaseAddressAndFEDNumberAndChannels_.end();++i_vmeBaseAddressAndFEDNumberAndChannels) {
    unsigned int fednumber=i_vmeBaseAddressAndFEDNumberAndChannels->first.second;
    if ( dataFile_.find(fednumber) != dataFile_.end() ) {
      if (dataFile_[fednumber] != 0)    fclose(dataFile_[fednumber]);
    }
    if ( dataFileT_.find(fednumber) != dataFileT_.end() ) {
      if (dataFileT_[fednumber] != 0)    fclose(dataFileT_[fednumber]);
    }
    if ( dataFileS_.find(fednumber) != dataFileS_.end() ) {
      if (dataFileS_[fednumber] != 0)    fclose(dataFileS_[fednumber]);
    }
    if ( errorFile_.find(fednumber) != errorFile_.end() ) {
      if (errorFile_[fednumber] != 0)   fclose(errorFile_[fednumber]);
    }
    if ( ttsFile_.find(fednumber) != ttsFile_.end() ) {
      if (ttsFile_[fednumber] != 0)     fclose(ttsFile_[fednumber]);
    }
#ifdef READ_LASTDAC
    if ( lastDacFile_.find(fednumber) != lastDacFile_.end() ) {
      if (lastDacFile_[fednumber] != 0) fclose(lastDacFile_[fednumber]);
    }
#endif

  }

  //this is safe to run anytime
  closeOutputFiles();
  reportStatistics();

  if(theFEDCalibrationBase_!=0) {
    diagService_->reportError("[PixelFEDSupervisor::Recover] There was a FEDCalibrationBase_", DIAGDEBUG);
    delete theFEDCalibrationBase_;
    theFEDCalibrationBase_=0;
    diagService_->reportError("But not anymore!", DIAGDEBUG);
  }

  //HALT does not usually clean up theCalibObject_. I guess it doesn't really matter
  delete theCalibObject_; theCalibObject_=0;

  //in case of hardware failure, deleteHardware will throw an exception
  //The exception will mean that deleteHardware is not fully executed
  //If that happends, then we fail to recover. The user can either try again or fix the hardware problem
  try {
    deleteHardware();
    try {
      toolbox::Event::Reference e(new toolbox::Event("Halt", this));
      fsm_.fireEvent(e);
    } catch (toolbox::fsm::exception::Exception & e) {
      diagService_->reportError("[PixelFEDSupervisor::Recover] Halt is an invalid command for the current state. "+state_.toString(), DIAGERROR);
      response="RecoverFailed";
    }
  }
  catch (std::exception & er) {
    response="RecoverFailed";
    diagService_->reportError("[PixelFEDSupervisor::Recover] Returning to Error state. Recovery failed with exception: "+string(er.what()), DIAGERROR);
    try {
      toolbox::Event::Reference e(new toolbox::Event("Failure", this));
      fsm_.fireEvent(e);
    } catch (toolbox::fsm::exception::Exception & e) {
      diagService_->reportError("Failure of FSM to go from Recovering to Error! exception: "+string(e.what()), DIAGFATAL);
    }
  }

  diagService_->reportError("-- Exit RECOVER --",DIAGINFO);
  return MakeSOAPMessageReference(response);

}

xoap::MessageReference PixelFEDSupervisor::Reconfigure (xoap::MessageReference msg)
{
  diagService_->reportError("--- Enter RECONFIGURE ---",DIAGINFO);
  xoap::MessageReference reply=MakeSOAPMessageReference("ReconfigureDone");

  PixelTimer reconfigureTimer;
  reconfigureTimer.start();

  try {

    Attribute_Vector parameters(1);
    parameters[0].name_="GlobalKey";
    Receive(msg, parameters);
    PixelConfigKey* newGlobalKey = new PixelConfigKey(atoi(parameters[0].value_.c_str()));
    if (newGlobalKey==0) {
      diagService_->reportError("Reconfigure failed to create GlobalKey",DIAGERROR);
      return MakeSOAPMessageReference("ReconfigureFailed");
    }
    diagService_->reportError("Reconfigure will use global key = " + stringF(newGlobalKey->key()),DIAGDEBUG);

    PixelGlobalDelay25* newGlobalDelay25=0;
    PixelConfigInterface::get(newGlobalDelay25, "pixel/globaldelay25/", *newGlobalKey);
    if (newGlobalDelay25==0) XCEPT_RAISE(xdaq::exception::Exception,"Reconfigure transition found globaldelay25 to be empty.");

    for ( FEDInterfaceMap::const_iterator ifed=FEDInterface_.begin() ; ifed!=FEDInterface_.end() ; ++ifed ) {
      cout<<"Reconfiguring TTCrxDelay for FED at address 0x"<<hex<<ifed->first<<dec<<endl; //DEBUG
      ifed->second->
	TTCRX_I2C_REG_WRITE(0,newGlobalDelay25->getTTCrxDelay(ifed->second->getPixelFEDCard().FineDes1Del));
      //(more would be needed for calibration mode)
    }

    delete theGlobalKey_;
    theGlobalKey_=newGlobalKey;
    delete theGlobalDelay25_;
    theGlobalDelay25_=newGlobalDelay25;
  }
  catch (exception & e) {
    diagService_->reportError("Reconfiguration failed with exception: "+string(e.what()),DIAGERROR);
    reply=MakeSOAPMessageReference("ReconfigureFailed");
  }

  reconfigureTimer.stop();
  cout<<"Time for FED reconfiguration = "<<reconfigureTimer.tottime()<<endl;

  diagService_->reportError("--- Exit RECONFIGURE ---",DIAGINFO);
  return reply;

}

xoap::MessageReference PixelFEDSupervisor::PrepareTTSTestMode (xoap::MessageReference msg) throw (xoap::exception::Exception)
{

  diagService_->reportError("Entering transition PrepareTTSTestMode",DIAGINFO);
  *console_<<"--- Entering PreparingTTSTestMode ---"<<std::endl;
  std::string response="PrepareTTSTestModeDone";

  unsigned int globalKey=0; // THIS IS TERRIBLE! WORK THIS OUT!!! -Souvik
  diagService_->reportError("Retrieving Global Key from DataBase...", DIAGINFO);
  *console_<<"Configure: Retrieving Global Key from DataBase... ";
  theGlobalKey_ = new PixelConfigKey(globalKey);
  assert(theGlobalKey_!=0);
  *console_<<"done."<<std::endl;
  diagService_->reportError("Retrieving Global Key from DataBase done.", DIAGINFO);

  unsigned long controlRegister=458768;
  unsigned long modeRegister=8;
  job_Configure();

  for (FEDInterfaceMap::iterator iFED=FEDInterface_.begin();iFED!=FEDInterface_.end();++iFED) {
    iFED->second->setControlRegister(controlRegister);
    iFED->second->setModeRegister(modeRegister);
    unsigned long controlRegister_check=iFED->second->getControlRegister();
    unsigned long modeRegister_check=iFED->second->getModeRegister();
    if (controlRegister_check!=controlRegister) {

      diagService_->reportError("PixelFEDSupervisor::PrepareTTSTestMode - Control Register =  "+itoa(controlRegister_check), DIAGINFO);

      //response="PrepareTTSTestModeFailed";
    }
    if (modeRegister_check!=modeRegister) {
      diagService_->reportError("PixelFEDSupervisor::PrepareTTSTestMode - Mode Register = "+itoa(modeRegister_check), DIAGINFO);
    
      //response="PrepareTTSTestModeFailed";
    }
  }

  if (response=="PrepareTTSTestModeDone") {
    try {
      toolbox::Event::Reference e(new toolbox::Event("PrepareTTSTestMode", this));
      fsm_.fireEvent(e);
    } catch (toolbox::fsm::exception::Exception & e) {
      
      *console_<<"[PixelFEDSupervisor::PrepareTTSTestMode] PrepareTTSTestMode is an invalid command for the "<<state_.toString()<<" state."<<std::endl;
      diagService_->reportError("[PixelFEDSupervisor::PrepareTTSTestMode] PrepareTTSTestMode is an invalid command for the current state."+state_.toString(), DIAGERROR);
    } 
    *console_<<"-------------------"<<std::endl;
    diagService_->reportError("-------------------", DIAGINFO);
  } else {
    *console_<<"--- Preparing TTS Test Mode Failed! ---"<<std::endl;
    diagService_->reportError("--- Preparing TTS Test Mode Failed! ---", DIAGINFO);
  }
  
  diagService_->reportError("Exiting transition PrepareTTSTestMode",DIAGINFO);
  *console_<<"--- Exiting PreparingTTSTestMode ---"<<std::endl;
  return MakeSOAPMessageReference(response);

}

xoap::MessageReference PixelFEDSupervisor::TestTTS (xoap::MessageReference msg) throw (xoap::exception::Exception)
{

  diagService_->reportError("Entering transition TestTTS",DIAGINFO);
  *console_<<"--- Entering transition TestTTS ---"<<std::endl;
  
  std::string response="TestTTSDone";

  Attribute_Vector parameters(4);
  parameters[0].name_="TTS_TEST_FED_ID";
  parameters[1].name_="TTS_TEST_TYPE";
  parameters[2].name_="TTS_TEST_PATTERN";
  parameters[3].name_="TTS_TEST_SEQUENCE_REPEAT";
  Receive(msg, parameters);
  unsigned int fednumber=atoi(parameters[0].value_.c_str());
  unsigned int fedCrate=theFEDConfiguration_->crateFromFEDNumber(fednumber);

  if (fedCrate==crate_) {

    unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
    diagService_->reportError("FED Crate="+stringF(crate_)+" VME Base Address=0x"+htoa(vmeBaseAddress),DIAGINFO);

    if (parameters[1].value_=="PATTERN") {
      unsigned long pattern=atol(parameters[2].value_.c_str());
      diagService_->reportError("retrieved pattern = "+pattern,DIAGINFO);
      FEDInterface_[vmeBaseAddress]->testTTSbits(pattern, 1);
    }

    else if (parameters[1].value_=="CYCLE") {
      unsigned int cycles=atoi(parameters[3].value_.c_str());
      for (unsigned int cycle=1; cycle<=cycles; ++cycle) {
        for (unsigned long pattern=0; pattern<16; ++pattern) {
	  FEDInterface_[vmeBaseAddress]->testTTSbits(pattern, 1);
	  usleep(1); // Frequency of commands should not exceed 5 MHz. Ref: FSM for Level 1 FM by Alex Oh
        }
      }
    }

  }

  if (response=="TestTTSDone") {
    try {
      toolbox::Event::Reference e(new toolbox::Event("TestTTS", this));
      fsm_.fireEvent(e);
      state_=fsm_.getStateName(fsm_.getCurrentState());
    } catch (toolbox::fsm::exception::Exception & e) {
      *console_<<"[PixelFEDSupervisor::TestTTS] TestTTS is an invalid command for the "<<state_.toString()<<" state."<<std::endl;
      diagService_->reportError("[PixelFEDSupervisor::TestTTS] TestTTS is an invalid command for the current state."+state_.toString(), DIAGERROR);

    }
    *console_<<"-------------------"<<std::endl;
    diagService_->reportError("-------------------", DIAGINFO);
  } else {
    *console_<<"--- Testing TTS Failed! ---"<<std::endl;
    diagService_->reportError("--- Testing TTS Failed! ---", DIAGINFO);
  }

  diagService_->reportError("Exiting transition TestTTS",DIAGINFO);
  *console_<<"--- Exiting TestTTS ---"<<std::endl;
  
  return MakeSOAPMessageReference(response);

}  

xoap::MessageReference PixelFEDSupervisor::Reset (xoap::MessageReference msg) throw (xoap::exception::Exception) {

	diagService_->reportError("--- RESET ---",DIAGINFO);
  	diagService_->reportError("New state before reset is: " + fsm_.getStateName (fsm_.getCurrentState()),DIAGINFO);

	fsm_.reset();
	state_ = fsm_.getStateName (fsm_.getCurrentState());

	xoap::MessageReference reply = xoap::createMessage();
	xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
	xoap::SOAPName responseName = envelope.createName("ResetDone", "xdaq", XDAQ_NS_URI);
	(void) envelope.getBody().addBodyElement ( responseName );

	diagService_->reportError("New state after reset is: " + fsm_.getStateName (fsm_.getCurrentState()),DIAGINFO);

	return reply;
}

///////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Finite State Machine ///////////////////////////////
////////////////////// State Entry and Transition Functions ///////////////////////
///////////////////////////////////////////////////////////////////////////////////

void PixelFEDSupervisor::stateChanged(toolbox::fsm::FiniteStateMachine &fsm) //throw (toolbox::fsm::exception::Exception)
{
 try {
  state_=fsm.getStateName(fsm.getCurrentState());
  if (PixelSupervisor_!=0) {
    Attribute_Vector parameters(3);
    parameters[0].name_="Supervisor"; parameters[0].value_="PixelFEDSupervisor";
    parameters[1].name_="Instance";   parameters[1].value_=itoa(crate_);
    parameters[2].name_="FSMState";   parameters[2].value_=state_;
    Send(PixelSupervisor_, "FSMStateNotification", parameters);
  }
  diagService_->reportError("New state is:" +std::string(state_),DIAGTRACE);
 }
 catch (xcept::Exception & ex) {
   ostringstream err;
   err<<"Failed to report FSM state "<<state_.toString()<<" to PixelSupervisor. Exception: "<<ex.what();
   diagService_->reportError(err.str(),DIAGERROR);
 }
}

// void PixelFEDSupervisor::statePaused(toolbox::fsm::FiniteStateMachine &fsm) //throw (toolbox::fsm::exception::Exception)
// {
//   diagService_->reportError("--- statePaused ---",DIAGTRACE);
  
//   try {

//   state_=fsm.getStateName(fsm.getCurrentState());
//   if (PixelSupervisor_!=0) {
//     Attribute_Vector parameters(3);
//     parameters[0].name_="Supervisor"; parameters[0].value_="PixelFEDSupervisor";
//     parameters[1].name_="Instance";   parameters[1].value_=itoa(crate_);
//     parameters[2].name_="FSMState";   parameters[2].value_=state_;
//     Send(PixelSupervisor_, "FSMStateNotification", parameters);
//   }
//   diagService_->reportError("New state is:" +std::string(state_),DIAGTRACE);
//  }
//  catch (xcept::Exception & ex) {
//    ostringstream err;
//    err<<"Failed to report FSM state "<<state_.toString()<<" to PixelSupervisor. Exception: "<<ex.what();
//    diagService_->reportError(err.str(),DIAGERROR);
//  }
// }


void PixelFEDSupervisor::stateConfiguring(toolbox::fsm::FiniteStateMachine &fsm) //throw (toolbox::fsm::exception::Exception)
{

  // Update the state_ member data so that Infospace may publish this information
  stateChanged(fsm);

  try {
  // We're going to time various aspects of configuration
  PixelTimer totalTimer;
  PixelTimer getFEDCardTimer, configBoardTimer;
  totalTimer.start();  

  // Extract the Calib object corresponding to the Global Key
  diagService_->reportError("Retrieving Calib object from DataBase...", DIAGINFO);
  *console_<<"Configure: Retrieving Calib object from DataBase... ";
  //PixelConfigInterface::setMode(true) ;
  PixelConfigInterface::get(theCalibObject_, "pixel/calib/", *theGlobalKey_);
  *console_<<"done."<<std::endl;
  diagService_->reportError("Retrieving Calib object from DataBase done.", DIAGINFO);

  // Using the Global Key configure the FED boards
  job_Configure(); // job_Configure needs theCalibObject_ to be already loaded

  if(dynamic_cast <PixelCalibConfiguration*> (theCalibObject_)!=0){
    (dynamic_cast <PixelCalibConfiguration*> (theCalibObject_))->buildROCAndModuleLists(theNameTranslation_, theDetectorConfiguration_);
  }

    for (std::map<std::pair<unsigned long, unsigned int>, std::set<unsigned int> >::iterator i_vmeBaseAddressAndFEDNumberAndChannels=vmeBaseAddressAndFEDNumberAndChannels_.begin();
				 i_vmeBaseAddressAndFEDNumberAndChannels!=vmeBaseAddressAndFEDNumberAndChannels_.end();
				 ++i_vmeBaseAddressAndFEDNumberAndChannels)
	  {
      unsigned long vmeBaseAddress=i_vmeBaseAddressAndFEDNumberAndChannels->first.first;
      PixelFEDInterface *iFED=FEDInterface_[vmeBaseAddress];

      // Set up XY mechanism
      iFED->setXY( 8, 500000 );
      iFED->resetXYCount();
	
    }
	

  // From the Calib object's mode
  // decide what the Control and Mode registers for the various chips on the FEDs must be
  // decide which if any jobs to submit to the main workloop
  if (theCalibObject_==0) { // This must be a Physics Run

    // Set the Control and Mode registers in all configured FEDs
    // Commented out to use the default values in the parames file.
    //for (FEDInterfaceMap::iterator iFED=FEDInterface_.begin();iFED!=FEDInterface_.end();++iFED) { 
    //  iFED->second->setControlRegister(0x70010);
    //  iFED->second->setModeRegister(0x0);
    //}

    // Submit the physics job to the workoop
    workloop_->submit(physicsRunning_);
    diagService_->reportError("Physics data taking job submitted to the workloop", DIAGINFO);
    *console_<<"Configure: Physics data taking job submitted to the workloop"<<std::endl;      

  }
  
  #ifdef RUBUILDER
    // Turning on RU Builder
    m_controller.configure_the_ru_builder();
    m_controller.enable_the_ru_builder();
  #endif

  totalTimer.stop();

  diagService_->reportError("PixelFEDSupervisor::stateConfiguring: FED configuration: total time="+stringF(totalTimer.tottime()), DIAGINFO);
  *console_<<"stateConfiguring: FED configuration: total time=" << totalTimer.tottime() << std::endl;
  diagService_->reportError("PixelFEDSupervisor::stateConfiguring: Triggering transition to the Configured state.", DIAGINFO);
  *console_<<"stateConfiguring: Triggering transition to the Configured state."<<std::endl;
  try {
    toolbox::Event::Reference e(new toolbox::Event("ConfiguringDone", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    diagService_->reportError("Failed to transition to Configured state! Exception: "+string(e.what()),DIAGERROR);
  }  
  } catch (std::exception & e) {
    diagService_->reportError("Failed to configure FED with exception: "+string(e.what()),DIAGERROR);
    try {
      toolbox::Event::Reference ev(new toolbox::Event("Failure", this));
      fsm_.fireEvent(ev);
    } catch (toolbox::fsm::exception::Exception & e2) {
      diagService_->reportError("Failed to transition to Failed state! Exception: "+string(e2.what()),DIAGFATAL);
    }

  }

}

bool PixelFEDSupervisor::job_Configure ()
{
  cout << " job configure " << endl;
  PixelTimer getFEDCardTimer, configBoardTimer;

  // ASCII address table
  HAL::VMEAddressTableASCIIReader reader(datbase_+"/FEDAddressMap.dat");
  //  HAL::VMEAddressTable *
  addressTablePtr_=new HAL::VMEAddressTable( "VMEMemory address table", reader );
  
  // Getting the bus adapter for the FED.
#ifdef VMEDUMMY
  diagService_->reportError("I think I have no hardware and will use a dummy bus adapter.",DIAGTRACE);
  busAdapter_ = new HAL::VMEDummyBusAdapter();
#else
  diagService_->reportError("I think I own the whole VME crate and will create a bus adapter.",DIAGTRACE);

  busAdapter_ = new HAL::CAENLinuxBusAdapter(HAL::CAENLinuxBusAdapter::V2718,0,0,HAL::CAENLinuxBusAdapter::A3818) ;

//  // The link has to depend on the crate number
//  // crate =1, link=1
//  // crate =2, link=2
//  // crate =3, link=1
//  int mytmplnk = 0;
//  if(crate_>=1 && crate_<3) mytmplnk = crate_;
//  else if(crate_==3) mytmplnk = 1; // for fpix, crate 3 is link 1
//  const int link = mytmplnk;  // bpix has 2 crates = 1 supervisors 
//  cout << " Bus Adapter: " << " crate # = " << crate_ << " link "<<link
//       <<" crate "<<crate_<<endl;
//  busAdapter_  = new HAL::CAENLinuxBusAdapter( HAL::CAENLinuxBusAdapter::V2718,
//  				     link,0, HAL::CAENLinuxBusAdapter::A3818 );
//  cout << " Bus Adapter 2 " << endl;
//  //busAdapter_ = new HAL::CAENLinuxBusAdapter(HAL::CAENLinuxBusAdapter::V2718); //optical

  //busAdapter_ = new HAL::CAENLinuxBusAdapter(HAL::CAENLinuxBusAdapter::V1718); //usb d.k. 3/07
  diagService_->reportError("Got a CAEN Linux Bus Adapter to the FED",DIAGTRACE);
#endif  

  diagService_->reportError("Will now get Name translation",DIAGTRACE);
  //PixelConfigInterface::setMode(true) ;
  PixelConfigInterface::get(theNameTranslation_, "pixel/nametranslation/", *theGlobalKey_); 
  if (theNameTranslation_==0) XCEPT_RAISE(xdaq::exception::Exception, "Failed to load name translation!");

  diagService_->reportError("Will now get detector configuration",DIAGTRACE);
  PixelConfigInterface::get(theDetectorConfiguration_, "pixel/detconfig/", *theGlobalKey_);    
  if (theDetectorConfiguration_==0) XCEPT_RAISE(xdaq::exception::Exception, "Failed to load detconfig!");

  diagService_->reportError("Will now get fed configuration",DIAGTRACE);
  PixelConfigInterface::get(theFEDConfiguration_, "pixel/fedconfig/", *theGlobalKey_);	
  if (theFEDConfiguration_==0) XCEPT_RAISE(xdaq::exception::Exception, "Failed to load the FED Configuration!");
  
  diagService_->reportError("Will now get global delay25 configuration",DIAGTRACE);
  PixelConfigInterface::get(theGlobalDelay25_, "pixel/globaldelay25/", *theGlobalKey_);
  if (theGlobalDelay25_==0) diagService_->reportError("Global delay in Delay25 is not specified. Using the default Delay25 settings.", DIAGINFO);

  //diagService_->reportError("Will now get the calibration object",DIAGTRACE);

  std::map <unsigned int, std::set<unsigned int> > fedsAndChannels=theDetectorConfiguration_->getFEDsAndChannels(theNameTranslation_);
  std::map <unsigned int, std::set<unsigned int> >::iterator i_fedsAndChannels=fedsAndChannels.begin();
  for (;i_fedsAndChannels!=fedsAndChannels.end();++i_fedsAndChannels) {
    unsigned long fednumber=i_fedsAndChannels->first;
    unsigned int fedcrate=theFEDConfiguration_->crateFromFEDNumber(fednumber);
    if (fedcrate==crate_) {
      unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
      std::set <unsigned int> channels=i_fedsAndChannels->second;
      vmeBaseAddressAndFEDNumberAndChannels_.insert(make_pair(make_pair(vmeBaseAddress, fednumber), channels));
      
      VMEPtr_[vmeBaseAddress]=new HAL::VMEDevice(*addressTablePtr_, *busAdapter_, vmeBaseAddress);
      FEDInterface_[vmeBaseAddress]=new PixelFEDInterface(VMEPtr_[vmeBaseAddress]);
      FEDInterfaceFromFEDnumber_[fednumber]=FEDInterface_[vmeBaseAddress];
      /////////////////
      bool pilotFED = true;
      if ( pilotFED ) {
	cout << " pilotFED load fpga  "<< endl;
	FEDInterface_[vmeBaseAddress]->loadFPGADigFED();
      }
      /////////////////
      FEDInterface_[vmeBaseAddress]->reset();
      dataFIFO1_[vmeBaseAddress]=new std::stringstream();
      dataFIFO2_[vmeBaseAddress]=new std::stringstream();
      dataFIFO3_[vmeBaseAddress]=new std::stringstream();
      errorFIFO_[vmeBaseAddress]=new std::stringstream();
      tempFIFO_[vmeBaseAddress]=new std::stringstream();
      ttsFIFO_[vmeBaseAddress]=new std::stringstream();

      getFEDCardTimer.start();
      PixelFEDCard *theFEDCard;

      //PixelConfigInterface::setMode(true);
      PixelConfigInterface::get(theFEDCard, "pixel/fedcard/"+itoa(fednumber), *theGlobalKey_);
      getFEDCardTimer.stop();
      if (theFEDCard==0) XCEPT_RAISE(xdaq::exception::Exception, "Failed to load FED card for FED="+string(itoa(fednumber)));
      configBoardTimer.start();

      //Check consistency of the configuration
      PixelConfigurationVerifier theVerifier;
      theVerifier.checkChannelEnable(theFEDCard,
				     theNameTranslation_,
				     theDetectorConfiguration_);
       
      FEDInterface_[vmeBaseAddress]->setupFromDB(*theFEDCard);
      FEDInterface_[vmeBaseAddress]->resetSlink();
      FEDInterface_[vmeBaseAddress]->testTTSbits(0,0);
      FEDInterface_[vmeBaseAddress]->storeEnbableBits();

      // Delay the FED output by the same amount as the Delay25 input clock is so that we don't need to perform a ClockPhaseCalibration.
      // We will apply global delay only in Physics mode
      if (theGlobalDelay25_!=0 && theCalibObject_==0){
	FEDInterface_[vmeBaseAddress]->
	  TTCRX_I2C_REG_WRITE(0,theGlobalDelay25_->getTTCrxDelay(FEDInterface_[vmeBaseAddress]->getPixelFEDCard().FineDes1Del)); 
	// Would be needed in calibration mode additionally:
	// FEDInterface_[vmeBaseAddress]->
	//  getPixelFEDCard().FineDes1Del=theGlobalDelay25_->getTTCrxDelay(FEDInterface_[vmeBaseAddress]->getPixelFEDCard().FineDes1Del);
      }

      configBoardTimer.stop();

      //Print out the FED firmware dates
      FEDInterface_[vmeBaseAddress]->get_VMEFirmwareDate();
      for(int i=0;i<5;i++){
        FEDInterface_[vmeBaseAddress]->get_FirmwareDate(i);
      }
      if (pilotFED)
	FEDInterface_[vmeBaseAddress]->get_PiggyFirmwareVer();
    }

  }

  diagService_->reportError("FED getFEDBoards total calls:"+stringF(getFEDCardTimer.ntimes())+" total time:"+stringF(getFEDCardTimer.tottime())+"  avg time:"+stringF(getFEDCardTimer.avgtime()),DIAGINFO);
  
  diagService_->reportError("FED card configure :"+stringF(configBoardTimer.ntimes())+" total time:"+stringF(configBoardTimer.tottime())+"  avg time:"+stringF(configBoardTimer.avgtime()), DIAGINFO);
 
  return false;

}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Jobs for the WorkLoop ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

// Send Last DAC (temperature) readings to Pixel DCS System by SOAP

xoap::MessageReference PixelFEDSupervisor::MakeSOAPMessageReference_readLastDACFIFO(const char* tagName)
{
assert(0);
  xoap::MessageReference message = xoap::createMessage();
  xoap::SOAPEnvelope envelope = message->getSOAPPart().getEnvelope();
  xoap::SOAPName commandName = envelope.createName(tagName, "xdaq", XDAQ_NS_URI);
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPElement commandElement = body.addBodyElement(commandName);

  for ( std::map<std::pair<unsigned long, unsigned int>, std::set<unsigned int> >::iterator i_vmeBaseAddressAndFEDNumberAndChannels = vmeBaseAddressAndFEDNumberAndChannels_.begin();
	i_vmeBaseAddressAndFEDNumberAndChannels != vmeBaseAddressAndFEDNumberAndChannels_.end(); ++i_vmeBaseAddressAndFEDNumberAndChannels ) {
    unsigned long vmeBaseAddress = i_vmeBaseAddressAndFEDNumberAndChannels->first.first;
    unsigned int fedNumber = i_vmeBaseAddressAndFEDNumberAndChannels->first.second;
    PixelFEDInterface* iFED = FEDInterface_[vmeBaseAddress];  

    xoap::SOAPName fedBoardName = envelope.createName("fedBoard", "", XDAQ_NS_URI);
    xoap::SOAPName fedNumberName = envelope.createName("number", "", XDAQ_NS_URI);
    xoap::SOAPElement fedBoardElement = commandElement.addChildElement(fedBoardName);
    fedBoardElement.addAttribute(fedNumberName, itoa(fedNumber));

    uint32_t buffer[1024];
    unsigned int numWords = iFED->drainTemperatureFifo(buffer);
    
    //diagService_->reportError("--> numWords = "+stringF(numWords), DIAGINFO);

    TemperatureFIFODecoder temperatureFIFODecoder(buffer, numWords);
    
    //diagService_->reportError("FED VME Address = 0x"+htoa(vmeBaseAddress)+ " -------" , DIAGINFO);

    temperatureFIFODecoder.printBuffer(std::cout);

    const std::list<PixelLastDACTemperature>& lastDACTemperatureReadings = temperatureFIFODecoder.getLastDACTemperatures();
    for ( std::list<PixelLastDACTemperature>::const_iterator lastDACTemperatureReading = lastDACTemperatureReadings.begin();
	  lastDACTemperatureReading != lastDACTemperatureReadings.end(); ++lastDACTemperatureReading ) {
      unsigned int fedChannel = lastDACTemperatureReading->getFEDChannel();
      unsigned int readOutChipId = lastDACTemperatureReading->getReadOutChipId();
      unsigned int dacValue = lastDACTemperatureReading->getDACValue();

/*
    for ( unsigned int iWord = 0; iWord < numWords; ++iWord ) {
      uint32_t word = buffer[iWord];
      unsigned int fedChannel = ((word & 0xfc000000) >> 26);
      unsigned int readOutChipId = ((word & 0x3e00000) >> 21);
      unsigned int dacValue = (word & 0xff);
 */



      xoap::SOAPName dataPointName = envelope.createName("dp", "", XDAQ_NS_URI);
      xoap::SOAPName channelName = envelope.createName("fedChannel", "", XDAQ_NS_URI);
      xoap::SOAPName readOutChipName = envelope.createName("roc", "", XDAQ_NS_URI);
      xoap::SOAPElement dataPointElement = fedBoardElement.addChildElement(dataPointName);
      dataPointElement.addAttribute(channelName, itoa(fedChannel));
      dataPointElement.addAttribute(readOutChipName, itoa(readOutChipId));
      dataPointElement.addTextNode(itoa(dacValue));
    }
  }

  return message;
}

bool PixelFEDSupervisor::ReadLastDACFIFO_workloop_SOAP(toolbox::task::WorkLoop *w1) 
{
assert(0);
  std::ostringstream diagMessage;
  diagMessage << "<PixelFEDSupervisor " << this->getApplicationDescriptor()->getInstance() << " ::ReadLastDACFIFO_workloop_SOAP>:";
  diagService_->reportError(diagMessage.str(), DIAGINFO);

  xoap::MessageReference soapRequest = MakeSOAPMessageReference_readLastDACFIFO("updateDpValue");
  
  std::string soapResponse = Send(PixelDCStoFEDDpInterface_, soapRequest);
  
  if ( soapResponse != "updateDpValueDone" ) {
    std::cerr << "<PixelFEDSupervisor " << this->getApplicationDescriptor()->getInstance() << " ::readLastDACFIFO>:"
	      << " Failed to send last DAC temperatures to PixelDCStoFEDDpInterface" << std::endl;
    diagService_->reportError("Failed to send last DAC temperatures to PixelDCStoFEDDpInterface", DIAGDEBUG);
  }
  
  return true; // ensures that this goes on forever unless the WorkLoop is cancelled!
}

// Send Last DAC (temperature) readings to Pixel DCS System by I2O
bool PixelFEDSupervisor::ReadLastDACFIFO_workloop_I2O(toolbox::task::WorkLoop *w1) 
{
assert(0);
  std::ostringstream diagMessage;
  diagMessage << "<PixelFEDSupervisor " << this->getApplicationDescriptor()->getInstance() << " ::ReadLastDACFIFO_workloop_I20>:";
  diagService_->reportError(diagMessage.str(), DIAGINFO);

  for ( std::map<std::pair<unsigned long, unsigned int>, std::set<unsigned int> >::iterator i_vmeBaseAddressAndFEDNumberAndChannels = vmeBaseAddressAndFEDNumberAndChannels_.begin();
	i_vmeBaseAddressAndFEDNumberAndChannels != vmeBaseAddressAndFEDNumberAndChannels_.end(); ++i_vmeBaseAddressAndFEDNumberAndChannels ) {
    unsigned long vmeBaseAddress = i_vmeBaseAddressAndFEDNumberAndChannels->first.first;

    U64 fedNumber = i_vmeBaseAddressAndFEDNumberAndChannels->first.second; // FIXME ?

    PixelFEDInterface* iFED = FEDInterface_[vmeBaseAddress];  
    
    uint32_t buffer[1024];
    uint32_t numWords = iFED->drainTemperatureFifo(buffer);
    
    I2OPrivateMessageSender ship(this, numWords*sizeof(unsigned long));
    ship.setDestinationDescriptor(PixelDCStoFEDDpInterface_);
    ship.setTransactionContext(fedNumber);
    ship.setXFunctionCode(20);
    memcpy(ship.getTrailingBuffer(), buffer, numWords*sizeof(unsigned long));
    ship.send();
  }

  return true;
}
//----------------------------------------------------------------------
// Write Error FIFO and Data FIFO 3 contents to files
bool PixelFEDSupervisor::PhysicsRunning(toolbox::task::WorkLoop *w1) {


  const bool useSharedMemory = false; //  KEEP false, set to true in order to test shared memory
  const bool readSpyFifo3  = true; // true;
  const bool readErrorFifo = true;
  bool readTTSFifo = true; //not a const so we can make it true at the beginning of a run, for instance.
  const bool readBaselineCorr = false;
  const bool readLastDACFifo  = true;
  const bool readFifoStatusAndLFF = true;
  const bool useSEURecovery = false; // Enable SEU recovery mechanism
  const bool timing = false;        // print output from Pixel Timers on each exit from the loop
  const bool localPrint = false; 
  
  //::sleep(1); return true; //disable physics workloop

  PixelTimer wlTimer;
  wlTimer.start();

  int hitsseen = 0;

  // Skip if there are no active feds in this fed
  if( vmeBaseAddressAndFEDNumberAndChannels_.size() == 0 ) return true;
  
  try { //hardware and SOAP

    //    static std::map <unsigned long, map <unsigned int, Moments> > baselineAdj; //FIXME not used?
  std::map<std::pair<unsigned long, unsigned int>, std::set<unsigned int> >::iterator i_vmeBaseAddressAndFEDNumberAndChannels=vmeBaseAddressAndFEDNumberAndChannels_.begin();

  if(localPrint) cout<<" loops "<<countLoopsThisRun_<<endl;

  PixelFEDInterface *iFED=FEDInterface_[(i_vmeBaseAddressAndFEDNumberAndChannels->first.first)];
  int newEventNumber=iFED->readEventCounter();
  bool newEvent = newEventNumber!=eventNumber_; // If the Event Number of any FED incremented, it'd mean an increment for all FEDs - sdas
  bool firstCallThisRun = errorCountMap.empty() && ttsStateChangeCounterMap.empty() && lffMap.empty() && fifoStatusMap.empty();
  
  if(localPrint) cout<<" event "<<newEventNumber<<" "<<eventNumber_<<" "<<newEvent<<" "<<firstCallThisRun <<endl;

  // logic to spy one FED on each entry to workloop, rotating through
  static unsigned int lastFEDSpied;
  static bool spyNextFED;
  static int countLoops = -1;
  bool spiedFED = false;
  if (firstCallThisRun) {
    lastFEDSpied = 32768;//nonsense number
    spyNextFED = true;
    if(localPrint) cout<<" spy "<<lastFEDSpied<<" "<<spyNextFED<<endl;
  }


  if(countLoopsThisRun_<100) readTTSFifo = true;
  if(localPrint) cout << "countLoops=" << countLoops << ", countLoopsThisRun_=" << countLoopsThisRun_ << ", newEventNumber=" << newEventNumber << ", readTTSFifo=" << readTTSFifo << endl; //Ben - for making sure the counters work for the above line
   
  PixelTimer statusTimer, statusTimerHW;
  PixelTimer spyTimer,spyTimerHW;
  PixelTimer errTimer,errTimerHW;
  PixelTimer ttsTimer,ttsTimerHW;
  PixelTimer blTimer, blTimerHW;
  
  //errTable->clear();
  //errTable->addColumn("fedNumber","unsigned int 32");
  //errTable->addColumn("channelNumber","unsigned int 32");
  //errTable->addColumn("NORErrors","unsigned int 32");
  //errTable->addColumn("OOSErrors","unsigned int 32");
  //errTable->addColumn("TimeOutErrors","unsigned int 32");
  
  //  if (newEventNumber!=eventNumber_) {

  countLoops++;
  countLoopsThisRun_++;
  unsigned int counter = 1;

  if(localPrint) cout<<" FED loop: event "<<newEventNumber<<" countLoops "<<countLoops
		     <<" countLoopsThisRun "<<countLoopsThisRun_
		     <<" counter "<<counter<<endl;

  for (;i_vmeBaseAddressAndFEDNumberAndChannels!=vmeBaseAddressAndFEDNumberAndChannels_.end();++i_vmeBaseAddressAndFEDNumberAndChannels) {

      //check if we want to immediately kill this workloop
      //return 'true' because we don't want workloop_->cancel to throw an exception
      phlock_->take(); if (workloopContinue_)  phlock_->give(); else {phlock_->give(); return true;}

      unsigned long vmeBaseAddress=i_vmeBaseAddressAndFEDNumberAndChannels->first.first;
      unsigned int fednumber=i_vmeBaseAddressAndFEDNumberAndChannels->first.second;
      iFED=FEDInterface_[vmeBaseAddress];


      // SEU stuff. We want to check here if we're already in RunningDegraded
      if (useSEURecovery && iFED->runDegraded && !physicsRunningSentRunningDegraded
          && fsm_.getStateName(fsm_.getCurrentState()) == "Running") {
        cout << "FED " << fednumber << " tells us to go to RunningDegraded" << endl;
	try {
	  if (PixelSupervisor_!=0) {
	    Attribute_Vector parameters(2);
	    parameters[0].name_="Supervisor"; parameters[0].value_="PixelFEDSupervisor";
	    parameters[1].name_="Instance";   parameters[1].value_=itoa(crate_);
	    Send(PixelSupervisor_, "DetectDegradation", parameters);
	    physicsRunningSentRunningDegraded = true;
	  }
	}
	catch (xcept::Exception & ex) {
	  ostringstream err;
	  err<<"Failed to send DetectDegradation to PixelSupervisor. Exception: "<<ex.what();
	  diagService_->reportError(err.str(),DIAGERROR);
	  try {
		toolbox::Event::Reference ev(new toolbox::Event("Failure", this)); //comment this out only for testing
		fsm_.fireEvent(ev);
	  } catch (toolbox::fsm::exception::Exception & e2) {
	    diagService_->reportError("PixelFEDSupervisor::RunningDegraded: Failed to transition to Failed state!",DIAGFATAL);
	  }
	}
      } // end if

      // Check for SEUs
      if (useSEURecovery && !physicsRunningSentSoftErrorDetected) {

	// Check for channels that got turned off, likely due to SEUs
	if (iFED->checkFEDChannelSEU() ) {
	  diagService_->reportError("Detected soft error using FED channel monitoring",DIAGWARN);
	  DetectSoftError(); // send a message to PixelSupervisor
	}
	  
	// Check the XY register to see if it's caught any OOS
	int dummy = iFED->getXYCount();
	if (dummy!=0 ) {
	  cout << "OOS Storm detected using XY methanism in FED " << fednumber << std::endl;
	  cout <<"XY Register value was " << hex << dummy << dec 
	       <<" OOS low ch: " << (dummy&0x3f) << " OOS hi ch: " << ((dummy&0xfc0)>>6) 
	       <<" Trailer lo ch: " << ((dummy&0x3f000)>>12) << " Trailer Hi ch: " << ((dummy&0xfc0000)>>18) << std::endl;	
	  diagService_->reportError("Detected soft error using XY mechanism",DIAGWARN);
	  DetectSoftError(); // send a message to PixelSupervisor
	}
      } // if useSEURecovery


      // special test data payload
      // each time workloop is entered, add 0-39 hits on each channel of each FED
      // randomly chosen, channel by channel
      //      if (fednumber == 6 || fednumber == 34 || fednumber == 35 || fednumber==38 ||fednumber==22) {
      if (false && countLoops==0) {   // Special test, normally disabled 
	const int SubAddr_Sim[9]={0x3c000,0x5c000,0x7c000,0x9c000,0xbc000,0xdc000,0xfc000,0x11c000,0x13c000};
	//SubAddr_Sim[0]=0x3c000; 
	//SubAddr_Sim[1]=0x5c000; 
	//SubAddr_Sim[2]=0x7c000;
	//SubAddr_Sim[3]=0x9c000; 
	//SubAddr_Sim[4]=0xbc000; 
	//SubAddr_Sim[5]=0xdc000;
	//SubAddr_Sim[6]=0xfc000; 
	//SubAddr_Sim[7]=0x11c000; 
	//SubAddr_Sim[8]=0x13c000;

	const unsigned int data=1; // 0;     // # hits/ROC in normal event
  // const unsigned int bigdata=30; // # hits/ROC in a large event
  // const unsigned int norm=10000; // 1/norm chance of hitting the jackpot
  // const int wait=10;  // time to wait before shutting off large payload
       
	for(int jk=0;jk<9;jk++){//loop through all 9 channels in each FPGA
	  cout << "set FED 0x" << hex << vmeBaseAddress << dec << " channel " << jk+1 << " nhits/roc=" << data <<SubAddr_Sim[jk]<<endl;
	  VMEPtr_[vmeBaseAddress]->write("LAD_N", data,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);
	  VMEPtr_[vmeBaseAddress]->write("LAD_NC",data,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);
	  VMEPtr_[vmeBaseAddress]->write("LAD_SC",data,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);
	  VMEPtr_[vmeBaseAddress]->write("LAD_S", data,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);
	}

	// for(int jk=0;jk<9;jk++){//loop through all 9 channels in each FPGA
	//   if (rand()%norm==0) {
	//     cout << "set FED 0x" << hex << vmeBaseAddress << dec << " channel " << jk+1 << " nhits/roc=" << bigdata <<endl;
	//     VMEPtr_[vmeBaseAddress]->write("LAD_N",bigdata,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);
	//     usleep(wait);
	//     VMEPtr_[vmeBaseAddress]->write("LAD_N",data,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);
	//   }
	// }

	// for(int jk=0;jk<9;jk++){//loop through all 9 channels in each FPGA
	//   if (rand()%norm==0) {
	//     cout << "set FED 0x" << hex << vmeBaseAddress << dec << " channel " << jk+10 << " nhits/roc=" << bigdata <<endl;
	//     VMEPtr_[vmeBaseAddress]->write("LAD_N",bigdata,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);
	//     usleep(wait);
	//     VMEPtr_[vmeBaseAddress]->write("LAD_N",data,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);
	//   }
	// }

	// for(int jk=0;jk<9;jk++){//loop through all 9 channels in each FPGA
	//   if (rand()%norm==0) {
	//     cout << "set FED 0x" << hex << vmeBaseAddress << dec << " channel " << jk+19 << " nhits/roc=" << bigdata <<endl;
	//     VMEPtr_[vmeBaseAddress]->write("LAD_N",bigdata,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);
	//     usleep(wait);
	//     VMEPtr_[vmeBaseAddress]->write("LAD_N",data,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);
	//   }
	// }

	// for(int jk=0;jk<9;jk++){//loop through all 9 channels in each FPGA
	//   if (rand()%norm==0) {
	//     cout << "set FED 0x" << hex << vmeBaseAddress << dec << " channel " << jk+28 << " nhits/roc=" << bigdata <<endl;
	//     VMEPtr_[vmeBaseAddress]->write("LAD_N",bigdata,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);
	//     usleep(wait);
	//     VMEPtr_[vmeBaseAddress]->write("LAD_N",data,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);
	//   }
	// }

      } // end if, for special test payload
    
      statusTimer.start();

      // Read the FIFO status register and LFF  
      uint32_t LFFbit = 0; //want to keep this in wider scope so that we can see it later
      if(readFifoStatusAndLFF ) { // LFF and fifo status appear to be available at 40 MHz

	// Read fifo status
	statusTimerHW.start();
        unsigned int fstat=iFED->getFifoStatus();
	statusTimerHW.stop();
        iFED->dump_FifoStatus(fstat); // Verbose?  This produces output via cout.
	fstat=fstat&0x3ff;  
        if(fstat!=0) diagService_->reportError(" FIFO Status for event number "+stringF(newEventNumber)+" "+htoa(fstat), DIAGINFO);

	if(localPrint) cout<<"ReadFifoStatus: stat "<<hex<<fstat<<dec<<endl;

	// accumulate statistics in map of moments
	for (unsigned int ibit=0; ibit<10;ibit++) {
	  fifoStatusMap[fednumber][ibit].push_back( (fstat >> ibit) & 0x1LL );
	}

	// Read event counter register (including Link Full Flag)
        //ReadSpyPause[31]    SLinkLFF[30]   all zero [29..24]   EventCounter[23..0]
	statusTimerHW.start();
	VMEPtr_[vmeBaseAddress]->read("RdEventCntr",&LFFbit); // old event counter  //FIXME why are we doing vme access here directly and not via FEDInterface?
	statusTimerHW.stop();
	LFFbit = LFFbit&0x40000000;  // get the LFF bit (latched at a trigger)
	if (localPrint) cout << "LFFbit = " << std::hex << LFFbit << std::dec << std::endl;
	if(LFFbit!=0 && !(iFED->getPixelFEDCard().modeRegister & 0x8)) diagService_->reportError("FEDID:"+stringF(fednumber)+" LFF status 0x"+htoa(LFFbit)+" for event "+stringF(newEventNumber), DIAGINFO); 
	// accumulate statistics in map of moments
	LFFbit = LFFbit>>30;
	lffMap[fednumber].push_back(LFFbit);

	//Read TTCrx status register & error counters
	
	if(firstCallThisRun && NttcrxResets.find(fednumber)==NttcrxResets.end()) NttcrxResets[fednumber]=0; // initialize at run start if not seen already

	int ttcrx_stat = iFED->TTCRX_I2C_REG_READ(22);
	if(0xe0!=ttcrx_stat) { // detected a reset or ttcrx not ready
	  std::cout << "Physics Running: FED "<< fednumber << " TTCrx status !=0xe0 found " << hex << ttcrx_stat << dec <<std::endl;
	  if(0x10&ttcrx_stat) {//watch dog fired
	    NttcrxResets[fednumber]++;
	    std::cout<<"Autoreset detected!" <<std::endl;
	  }
	  if(!0x80&ttcrx_stat) std::cout<<"PLL not ready!" <<std::endl;
	  if(!0x40&ttcrx_stat) std::cout<<"DLL not ready!" <<std::endl;
	  if(!0x20&ttcrx_stat) std::cout<<"Frame not synchronized!" <<std::endl;
	  iFED->TTCRX_I2C_REG_WRITE(22,0); //reset watch dog 
	}
	
	int ttcrx_serr = iFED->TTCRX_I2C_REG_READ(8);
	ttcrx_serr+= (iFED->TTCRX_I2C_REG_READ(9)<<8);
	int ttcrx_derr =  iFED->TTCRX_I2C_REG_READ(10);
	int ttcrx_seu = iFED->TTCRX_I2C_REG_READ(11);

	NttcrxSBError[fednumber]=ttcrx_serr;
	NttcrxDBError[fednumber]=ttcrx_derr;
	NttcrxSEU[fednumber]=ttcrx_seu;

	// read the number of PLL locks in FED fpga
	unsigned int data=0;
	VMEPtr_[vmeBaseAddress]->read("LAD_N",&data,0x198000);
	unsigned int locks = ((data&0xf8000000)>>27);
	if(NlockNorth.find(fednumber)!=NlockNorth.end() && locks>NlockNorth[fednumber]) {
	  std::cout << "Physics Running FED=" << fednumber << " # of NorthFPGA locks changed from " << NlockNorth[fednumber] << " to " << locks << std::endl;
	}
	NlockNorth[fednumber] = locks;

	// debug printout
	if(lffMap[fednumber].count() >= 50 ) {
          toolbox::TimeVal timeOfDay=toolbox::TimeVal::gettimeofday();
	  ofstream statusFile;
	  statusFile.open((outputDir()+"/StatusFED_"+itoa(fednumber)+"_"+runNumber_+".txt").c_str(), ios::app);
	  statusFile <<"Time: "<<timeOfDay.toString("%c", timeOfDay.tz());

	  statusFile << " FED="<<fednumber<< " LFF status=" <<100*lffMap[fednumber].mean() <<"% (Since last printout)" << std::endl;
	  statusFile << "hits seen: " << hitsseen << std::endl;
	  statusFile << "FIFO I Almost Full: N(1-9)="<<100*fifoStatusMap[fednumber][0].mean() << "% NC(10-18)=" <<100*fifoStatusMap[fednumber][2].mean() 
		     << "% SC(19-27)=" <<100*fifoStatusMap[fednumber][4].mean() << "% S(27-36)=" <<100*fifoStatusMap[fednumber][6].mean() << "%" << std::endl;
	  statusFile << "FIFO II Nearly Full: N(1-9)="<<100*fifoStatusMap[fednumber][1].mean() << "% NC(10-18)=" <<100*fifoStatusMap[fednumber][3].mean() 
		     << "% SC(19-27)=" <<100*fifoStatusMap[fednumber][5].mean() << "% S(27-36)=" <<100*fifoStatusMap[fednumber][7].mean() << "%" << std::endl;
	  statusFile << "FIFO III Almost Full: UP="<<100*fifoStatusMap[fednumber][8].mean() << "% DOWN=" <<100*fifoStatusMap[fednumber][9].mean() << "%" << std::endl;
	  statusFile << "This run: Number of TTS State changes= " << ttsStateChangeCounterMap[fednumber];
	  statusFile << " Number of errors read=" << errorCountMap[fednumber][0] << " including resets" << std::endl;
	  statusFile << " Number of TTCrx resets=" << NttcrxResets[fednumber] << " single-bit errs=" << NttcrxSBError[fednumber] << " # double-bit errs=" << NttcrxDBError[fednumber] 
		     << " SEU = " << NttcrxSEU[fednumber] << std::endl;
	  statusFile << " Number of North FPGA PLL locks=" << NlockNorth[fednumber] << std::endl;

          //Look to see if fifo-fill level flag has flipped
          {unsigned int data=0; VMEPtr_[vmeBaseAddress]->read("LAD_C",&data,0x120000);
	   if(data==0){//fifo level counters have latched, may not be correct on first pass
	   //Center FPGA
           VMEPtr_[vmeBaseAddress]->read("LAD_C",&data,0x128000);
           statusFile << "fifo III level Up: "<<dec<<(data&0x1fff)<<" Dn: "<<dec<<((data&0x3ffe000)>>13)<<std::endl;
           VMEPtr_[vmeBaseAddress]->read("LAD_C",&data,0x130000);
           statusFile <<"Event for fifo level "<<dec<<data<<std::endl;
	   
           //North FPGA 
           VMEPtr_[vmeBaseAddress]->read("LAD_N",&data,0x5c000);
           statusFile <<"North fifo II level Up: "<<dec<<(data&0x1fff)<<" Dn: "<<dec<<((data&0x3ffe000)>>13)<<std::endl;
	   VMEPtr_[vmeBaseAddress]->read("NWrRdCntrReg", &data);
           statusFile <<"N  skipped channels: "<<dec;
	   for (int jj = 0; jj < 9; ++jj) if (data & (1 << jj)) statusFile << jj+1 << " ";
	   statusFile <<std::endl;
           VMEPtr_[vmeBaseAddress]->read("LAD_N",&data,0x7c000);
           statusFile <<"N  fifo I level ch  1: "<<dec<<(data&0x3ff)<<"/  2: "<<dec<<((data&0xffc00)>>10)<<"/  3: "<<dec<<((data&0x3ff00000)>>20)<<std::endl;
           VMEPtr_[vmeBaseAddress]->read("LAD_N",&data,0xdc000);
           statusFile <<"N  fifo I level ch  4: "<<dec<<(data&0x3ff)<<"/  5: "<<dec<<((data&0xffc00)>>10)<<"/  6: "<<dec<<((data&0x3ff00000)>>20)<<std::endl;
	   VMEPtr_[vmeBaseAddress]->read("LAD_N",&data,0x13c000);
           statusFile <<"N  fifo I level ch  7: "<<dec<<(data&0x3ff)<<"/  8: "<<dec<<((data&0xffc00)>>10)<<"/  9: "<<dec<<((data&0x3ff00000)>>20)<<std::endl;
	   
           //North Center FPGA 
           VMEPtr_[vmeBaseAddress]->read("LAD_NC",&data,0x5c000);
           statusFile <<"North Center fifo II level Up: "<<dec<<(data&0x1fff)<<" Dn: "<<dec<<((data&0x3ffe000)>>13)<<std::endl;
	   VMEPtr_[vmeBaseAddress]->read("NCWrRdCntrReg", &data);
           statusFile <<"NC skipped channels: "<<dec;
	   for (int jj = 0; jj < 9; ++jj) if (data & (1 << jj)) statusFile << jj+10 << " ";
	   statusFile <<std::endl;
           VMEPtr_[vmeBaseAddress]->read("LAD_NC",&data,0x7c000);
           statusFile <<"NC  fifo I level ch  10: "<<dec<<(data&0x3ff)<<"/  11: "<<dec<<((data&0xffc00)>>10)<<"/  12: "<<dec<<((data&0x3ff00000)>>20)<<std::endl;
           VMEPtr_[vmeBaseAddress]->read("LAD_NC",&data,0xdc000);
           statusFile <<"NC  fifo I level ch  13: "<<dec<<(data&0x3ff)<<"/  14: "<<dec<<((data&0xffc00)>>10)<<"/  15: "<<dec<<((data&0x3ff00000)>>20)<<std::endl;
	   VMEPtr_[vmeBaseAddress]->read("LAD_NC",&data,0x13c000);
           statusFile <<"NC  fifo I level ch  16: "<<dec<<(data&0x3ff)<<"/  17: "<<dec<<((data&0xffc00)>>10)<<"/  18: "<<dec<<((data&0x3ff00000)>>20)<<std::endl;
	   
           //South Center FPGA 
           VMEPtr_[vmeBaseAddress]->read("LAD_SC",&data,0x5c000);
           statusFile <<"South Center fifo II level Up: "<<dec<<(data&0x1fff)<<" Dn: "<<dec<<((data&0x3ffe000)>>13)<<std::endl;
	   VMEPtr_[vmeBaseAddress]->read("SCWrRdCntrReg", &data);
           statusFile <<"SC  skipped channels: "<<dec;
	   for (int jj = 0; jj < 9; ++jj) if (data & (1 << jj)) statusFile << jj+19 << " ";
	   statusFile <<std::endl;
           VMEPtr_[vmeBaseAddress]->read("LAD_SC",&data,0x7c000);
           statusFile <<"SC  fifo I level ch  19: "<<dec<<(data&0x3ff)<<"/  20: "<<dec<<((data&0xffc00)>>10)<<"/  21: "<<dec<<((data&0x3ff00000)>>20)<<std::endl;
           VMEPtr_[vmeBaseAddress]->read("LAD_SC",&data,0xdc000);
           statusFile <<"SC  fifo I level ch  22: "<<dec<<(data&0x3ff)<<"/  23: "<<dec<<((data&0xffc00)>>10)<<"/  24: "<<dec<<((data&0x3ff00000)>>20)<<std::endl;
	   VMEPtr_[vmeBaseAddress]->read("LAD_SC",&data,0x13c000);
           statusFile <<"SC  fifo I level ch  25: "<<dec<<(data&0x3ff)<<"/  26: "<<dec<<((data&0xffc00)>>10)<<"/  27: "<<dec<<((data&0x3ff00000)>>20)<<std::endl;
	   
           //South FPGA 
           VMEPtr_[vmeBaseAddress]->read("LAD_S",&data,0x5c000);
           statusFile <<"South fifo II level Up: "<<dec<<(data&0x1fff)<<" Dn: "<<dec<<((data&0x3ffe000)>>13)<<std::endl;
	   VMEPtr_[vmeBaseAddress]->read("SWrRdCntrReg", &data);
           statusFile <<"S  skipped channels: "<<dec;
	   for (int jj = 0; jj < 9; ++jj) if (data & (1 << jj)) statusFile << jj+28 << " ";
	   statusFile <<std::endl;
           VMEPtr_[vmeBaseAddress]->read("LAD_S",&data,0x7c000);
           statusFile <<"S  fifo I level ch  28: "<<dec<<(data&0x3ff)<<"/  29: "<<dec<<((data&0xffc00)>>10)<<"/  30: "<<dec<<((data&0x3ff00000)>>20)<<std::endl;
           VMEPtr_[vmeBaseAddress]->read("LAD_S",&data,0xdc000);
           statusFile <<"S  fifo I level ch  31: "<<dec<<(data&0x3ff)<<"/  32: "<<dec<<((data&0xffc00)>>10)<<"/  33: "<<dec<<((data&0x3ff00000)>>20)<<std::endl;
	   VMEPtr_[vmeBaseAddress]->read("LAD_S",&data,0x13c000);
           statusFile <<"S  fifo I level ch  34: "<<dec<<(data&0x3ff)<<"/  35: "<<dec<<((data&0xffc00)>>10)<<"/  36: "<<dec<<((data&0x3ff00000)>>20)<<std::endl;
	   
           }//end of fill level latch test

          //Set the register for another fifo fill level read regardless of previous:means 1st time ==0 questionalble
	  VMEPtr_[vmeBaseAddress]->write("LAD_C",0x1,HAL::HAL_NO_VERIFY,0x120000);
          //statusFile <<"Fifo fill level latch set at event "<<dec<<newEventNumber<<std::endl;//not needed, event is included in payload
	  }//end of fifo fill level context
	  
	  statusFile << std::endl;
	  	  
	  uint32_t ondata=0;
	  unsigned int errTableRowNumber = 0;
	  unsigned int errorRowNumberCorrection = (crate_ - 1) * 16;
	  if(localPrint) cout<<" Error report"<<endl;

          for (int iw=1;iw<37;iw++){
            VMEPtr_[vmeBaseAddress]->read("LAD_C",&ondata,(0x080000+0x4*iw));
            if((ondata&0x3fff)>0) {
 	      statusFile << " chnl: "<<iw<<" Num. of OOS: " << (ondata&0x3fff) << std::endl;
 	      if(localPrint) cout<< " chnl: "<<iw<<" Num. of OOS: " << (ondata&0x3fff) << std::endl;

	    }
            if((ondata&0xffffc000)>0) {
	      statusFile<<" chnl: "<<iw<<" Num. of NOR Errs: " << ((ondata&0xffffc000)>>14) << std::endl;
	      if(localPrint) cout<<" chnl: "<<iw<<" Num. of NOR Errs: " <<((ondata&0xffffc000)>>14)<<endl;
	    }

            errTableRowNumber = ((fednumber - errorRowNumberCorrection) *36)+(iw-1);

	    *numNORErrors = ((ondata&0xffffc000)>>14);
	    *numOOSErrors = (ondata&0x3fff);
	    
	    *fedNumberPtr = fednumber;
	    *channelNumberPtr = iw;
	    
	    errTable->setValueAt(errTableRowNumber,"fedNumber", *fedNumberPtr);
	    errTable->setValueAt(errTableRowNumber,"channelNumber", *channelNumberPtr);
	    errTable->setValueAt(errTableRowNumber,"NORErrors", *numNORErrors);
	    errTable->setValueAt(errTableRowNumber,"OOSErrors", *numOOSErrors);
	    	    
	    // Add the timouts (not enabled yet, needs new firmware) Enable 21/9/12 d.k.
	    VMEPtr_[vmeBaseAddress]->read("LAD_C",&ondata,(0x088000+0x4*iw));

	    if((ondata&0x3fff)>0) {
	      statusFile << " chnl: "<<iw<<" Num. of Timeouts: " << (ondata) << std::endl;	   
	      if(localPrint) cout<<" chnl: "<<iw<<" Num. of Timeouts: " <<(ondata)<<endl;	   
	    }
	    *numTimeOutErrors = (ondata);
	    errTable->setValueAt(errTableRowNumber,"TimeOutErrors", *numTimeOutErrors);

	  }
	  
	  VMEPtr_[vmeBaseAddress]->read("LAD_C",&ondata,0x098000);
	  if(ondata>0) statusFile << " Number of fake events sent: " << dec << ondata << std::endl;
	  
	  // Update time stamp.
	  ////*errTableTimeStampPtr = time(NULL);
	  
	  // Update flash list
	  ////monitorInfoSpace->fireItemValueChanged("errorTable", this);
	  	  
	  statusFile.close();
	  //	  std::cout << "wrote status file " << std::endl;
	  // reset the counters
	  lffMap[fednumber].clear();
	  for (int ii=0; ii<10;ii++) {fifoStatusMap[fednumber][ii].clear();}
	}
      }
      statusTimer.stop();

      iFED->readDigFEDStatus(false, false);

      // Drain DataFIFO 3 and write to file
      //if(readSpyFifo3 && newEvent  && spyNextFED && !spiedFED ) {
      if(readSpyFifo3 && newEvent ) {
	if(localPrint) cout<<" Read spy fifo3"<<endl;
	spyTimer.start();
	lastFEDSpied=fednumber;
	spiedFED=true;  //resets to false on each entrance to PhysicsRunning

	// Drain DataFIFO 3 and write to file
	spyTimerHW.start();
	uint64_t buffer64[4096];
	if(iFED->isNewEvent(1)) {//this checks for zeros - 0=New event, spy fifo not ready ready to be read
	  iFED->enableSpyMemory(0);
	  if (iFED->isWholeEvent(1)) {//this checks for 1's - spy fifo ready to be read

#ifdef PILOT_TRANSSCOPE
	    if (countLoops % 10 == 0) {
#if 0
	      static int ncalls = 0;
	      static unsigned long first_us;
	      ++ncalls;
	      if (ncalls == 1) {
		timeval first_time;
		gettimeofday(&first_time, 0);
		first_us = 1e6*first_time.tv_sec + first_time.tv_usec;
	      }
	      if (ncalls && ncalls % 10 == 0) {
		timeval this_time;
		gettimeofday(&this_time, 0);
		unsigned long this_us = 1e6*this_time.tv_sec + this_time.tv_usec;
		cout << ncalls << " wholeEvent reads in " << (this_us - first_us)/1e6 << endl;
	      }
#endif
	      // disable triggers while we read
	      const uint32_t ctrlreg_orig = iFED->getPixelFEDCard().Ccntrl;
	      const int printlevel_orig = iFED->get_Printlevel();
	      if (ctrlreg_orig & 0x10) {
		iFED->set_Printlevel_silent(0);
		iFED->getPixelFEDCard().Ccntrl = ctrlreg_orig & 0xffffffef;
		iFED->loadControlRegister();
	      }

	      usleep(1000);

	      //	    const int MaxChans = 37;    
	      //	    uint32_t bufferFifo1[MaxChans][1024];
	      //	    int statusFifo1[MaxChans] = {0};
	      //	    for (int ch = 1; ch <= 36; ++ch)
	      //	      statusFifo1[ch] = iFED->drainFifo1(ch, bufferFifo1[ch], 1024);
	      //#if 0

	      const int MaxChips = 8;
	      uint32_t bufferT[MaxChips][256];
	      uint8_t bufferS[MaxChips][1024];
	      int statusS[MaxChips] = {0};
	      for (int chip = 1; chip <= 7; chip += 2) {
		if (chip == 1 || chip == 7) {
		  iFED->drainDigTransFifo(chip, bufferT[chip]);
		  fwrite(bufferT[chip], sizeof(uint32_t), 256, dataFileT_[fednumber]);
		}
		uint32_t bufferS_temp[1024] = {0};
		statusS[chip] = iFED->drainDataFifo2(chip, bufferS_temp);
		if (statusS[chip] > 0) {
		  for (int jj = 0; jj < 1024; ++jj)
		    bufferS[chip][jj] = bufferS_temp[jj] & 0xff;
		  fwrite(&statusS[chip], sizeof(int), 1, dataFileS_[fednumber]);
		  fwrite(bufferS[chip], sizeof(uint8_t), statusS[chip], dataFileS_[fednumber]);
		}
	      }

	      if (ctrlreg_orig & 0x10) {
		iFED->getPixelFEDCard().Ccntrl = ctrlreg_orig;
		iFED->loadControlRegister();
		iFED->set_Printlevel_silent(printlevel_orig);
	      }

	      //#endif
	    }
#endif //PILOT_TRANSSCOPE

	    int dataLength=iFED->spySlink64(buffer64);
	    spyTimerHW.stop();
	    FIFO3Decoder decode3(buffer64);
	    hitsseen += decode3.nhits();
	    if(localPrint) { cout<<" fifo3 length "<<dataLength<<endl;
	      if (dataLength) {
		for (int i = 0; i <= dataLength; ++i)
		  std::cout << "Clock " << std::setw(2) << i << " = 0x " << std::hex << std::setw(8) << (buffer64[i]>>32) << " " << std::setw(8) << (buffer64[i] & 0xFFFFFFFF) << std::dec << std::endl;
		std::cout << "FIFO3Decoder thinks:\n" << "nhits: " << decode3.nhits() << std::endl;
		for (unsigned i = 0; i < decode3.nhits(); ++i) {
		  //const PixelROCName& rocname = theNameTranslation_->ROCNameFromFEDChannelROC(fednumber, decode3.channel(i), decode3.rocid(i)-1);
		  std::cout << "#" << i << ": ch: " << decode3.channel(i)
			    << " rocid: " << decode3.rocid(i)
		    //    << " (" << rocname << ")"
			    << " dcol: " << decode3.dcol(i)
			    << " pxl: " << decode3.pxl(i) << " pulseheight: " << decode3.pulseheight(i)
			    << " col: " << decode3.column(i) << " row: " << decode3.row(i) << std::endl;
		}
	      }
	    }

	    if(dataLength>0){  // Add protection from Will
	      fwrite(buffer64, sizeof(uint64_t), dataLength, dataFile_[fednumber]);
	    } else {
	      std::cout<<"FED ID = "<<fednumber<<" SpySlink read Failure";
	      std::cout<<", VME Base Address = 0x"<<std::hex<<vmeBaseAddress<<std::dec;
	      std::cout<<", Event Number = "<<newEventNumber<<" Wrong data length "<<dataLength<<std::endl;
	      // dump first 10 words
	      for (int i=0; i<=4;++i) std::cout<<" "<<i<<" = 0x"<<std::hex<<buffer64[i]<<std::dec<<std::endl;
	    }  // end protection from will


	    // Compare FED ID from SLink Header and present FED Number
	    unsigned int fednumber_header=(buffer64[0] & 0x00000000000fff00)>>8;
	    if (fednumber_header!=fednumber) {
	      diagService_->reportError("FED ID = "+stringF(fednumber)+" while FED ID from SLink Header = "+stringF(fednumber_header), DIAGINFO);
	      diagService_->reportError(", VME Base Address = 0x"+htoa(vmeBaseAddress), DIAGINFO);
	      diagService_->reportError(", Event Number = "+stringF(newEventNumber), DIAGINFO);
	    }
	  } else {
	    
	    diagService_->reportError("PixelFEDSupervisor::PhysicsRunning - Whole event not found in FED ID "+stringF(fednumber)+" after 1 second", DIAGINFO);
	    spyTimerHW.stop();
	    // can we bail out of the FED loop when this happens, e.g. triggers have stopped?
	  }
	} else {
	  diagService_->reportError("PixelFEDSupervisor::PhysicsRunning - Spy-fifo3 New Event never seen for FED ID "+stringF(fednumber)+" after 1 second", DIAGINFO);
	  spyTimerHW.stop();
	    // can we bail out of the FED loop when this happens, e.g. triggers have stopped?
	}
	spyTimerHW.start();
	iFED->enableSpyMemory(1);//prevent always disabled
	spyTimerHW.stop();
	spyTimer.stop();

      } // if readSpyFifo3
      spyNextFED=(lastFEDSpied==fednumber || lastFEDSpied==32768);  //we'll spy the next FED, but only if !spiedFED on this entrance

      // Shared objects	
      //Get the proper name for the shared object counter
      stringstream ss;
      ss<<"SpyErrorCollection"<<counter;
	
      //construct the shared object
      pixel::SharedObject<pixel::PixelErrorCollection >* dat;
      if(useSharedMemory) {  //FIXME hacked 
	dat = ErrorCollectionDataOwner.getSharedObject(ss.str()); //here's where we failed to allocate memory
      }
      counter++;
      
      //Drain Error FIFO and write to file (Do this every time workloop is called)
      errTimer.start();
      if(readErrorFifo) {
	uint32_t errBuffer[(8*errorDepth)];
	errTimerHW.start();
	unsigned int errorLength=iFED->drainErrorFifo(errBuffer); // (int)errorLength -> (unsigned int)errorLength
	errTimerHW.stop();
	if (errorLength>0) {
	  if(localPrint) { cout<<"Error fifo not empty for FED "<<fednumber<<" "<<errorLength<<endl;
	    ErrorFIFODecoder decodeErr(errBuffer, errorLength);
	    decodeErr.printToStream(std::cout);
	  }
	  // add time and error length
	  struct timeval tv;
	  gettimeofday(&tv, NULL);
	  double tt1 = tv.tv_sec;
	  //double tt2 = tv.tv_usec;
	  fwrite(&tt1,         sizeof(double),       1, errorFile_[fednumber]);
	  // sizeof(unsigned long) -> sizeof(unsigned int)
	  fwrite(&errorLength, sizeof(unsigned int), 1, errorFile_[fednumber]); 
	  fwrite(errBuffer, sizeof(uint32_t), errorLength, errorFile_[fednumber]);      
	  //fflush(errorFile_[fednumber]);  //be sure buffer is flushed // now unbuffered

	  //Set the errors
	  if (useSharedMemory) {
	    dat->getWritableAddress()->setErrors(errBuffer,errorLength);
	    //fire the update event
	    ErrorCollectionDataOwner.fireObjectUpdateEvent(ss.str());
	  }

	  //fill map
	  errorCountMap[fednumber][0]+=errorLength;
	  // could unpack the errors and count by channel
	}
	else if(firstCallThisRun) {
	  errorCountMap[fednumber][0]=0;
	}
      } // if readErrorFifo
      errTimer.stop();



      // Read the TTS fifo (Do this every time workloop is called)
      ttsTimer.start();
      if(readTTSFifo) {
	uint32_t ttsBuffer[ttsDepth];  // FIXME should be uint32_t?
	ttsTimerHW.start();
	int ttsLength=iFED->drainTTSFifo(ttsBuffer);
	ttsTimerHW.stop();
	if (ttsLength>0) {	  
	  for (unsigned int i=0; i<(unsigned int)ttsLength; ++i) {   

	    diagService_->reportError("[PixelFEDSupervisor::PhysicsRunning] FED ID "+stringF(fednumber)+" TTS word stored in buffer["+stringF(i)+"] = 0x"+htoa(ttsBuffer[i]), DIAGINFO);  
	  }
	  fwrite(ttsBuffer, sizeof(uint32_t), ttsLength, ttsFile_[fednumber]);	  // FIXME uint32_t if ttsBuffer changes type above
	  ttsStateChangeCounterMap[fednumber]+=ttsLength;  //record number of transitions
	  ttsMap[fednumber]=ttsBuffer[ttsLength-1];  //keep current status (last state)

	  //note that the fedStuckInBusy logic is only looking at the last TTS state.
	  //so we could miss a transition out of Busy if the last state ends up being BUSY. This is a flaw that could be fixed.
	  unsigned int lastTtsFlag = 0xF & ttsMap[fednumber];
	  if (LFFbit == 0 && (lastTtsFlag == 0x4 ) ) { //no LFF and we are in BUSY
	    fedStuckInBusy_[fednumber] = fedStuckInBusy_[fednumber]+1;
	    cout<<"No LFF and last TTS status is BUSY! fed = "<<fednumber<<" fedStuckInBusy = "<<fedStuckInBusy_[fednumber] <<endl; //JMT debug
	  }
	  else if ((lastTtsFlag != 0x4) || (LFFbit != 0)) {  //either we are not in BUSY or there is LFF
	    fedStuckInBusy_[fednumber] = 0; //that's it. reset the counter to zero
	    //	    cout<<"Cleared fedStuckInBusy! fed = "<<fednumber<<" fedStuckInBusy = "<<fedStuckInBusy_[fednumber] <<endl; //JMT debug
	  }
	}
	else if(firstCallThisRun) {
	  ttsStateChangeCounterMap[fednumber]=0;
	  //	  ttsMap[fednumber]=iFED->getLastTTS();  // not implemented (not a hardware call, but refreshed by the drainTTSFifo call)
	}
	else { //no change since last loop!
	  if ( LFFbit == 0 && fedStuckInBusy_[fednumber]>0) {
	    fedStuckInBusy_[fednumber] = fedStuckInBusy_[fednumber]+1;
	    cout<<"Incremented fedStuckInBusy! fed = "<<fednumber<<" fedStuckInBusy = "<<fedStuckInBusy_[fednumber] <<endl; //JMT debug
	  }
	}
      } // if readTTSFifo

      if (fedStuckInBusy_[fednumber] >5) 
	diagService_->reportError("[fedStuckInBusy] Found FED "+stringF(fednumber)+" stuck in BUSY for "+stringF(fedStuckInBusy_[fednumber])+" iterations of the loop!",DIAGWARN);
      

      ttsTimer.stop();


#ifndef PILOT_FED
      // Readout the baseline correction (Do this every time workloop is called)
      // loops over all FEDs
      blTimer.start();
      if(readBaselineCorr) {
	
	Attribute_Vector parametersToBaselineMonitor(3);
	parametersToBaselineMonitor[0].name_="VMEBaseAddress"; parametersToBaselineMonitor[0].value_=itoa(vmeBaseAddress);
	parametersToBaselineMonitor[1].name_="ShipTo";         parametersToBaselineMonitor[1].value_="File";
	parametersToBaselineMonitor[2].name_="Time";           parametersToBaselineMonitor[2].value_="None";
	xoap::MessageReference request=MakeSOAPMessageReference("BaselineMonitor", parametersToBaselineMonitor);
	blTimerHW.start();
	xoap::MessageReference reply=BaselineMonitor(request);
	blTimerHW.stop();
	if (Receive(reply)!="BaselineMonitorDone") {
	  diagService_->reportError("PixelFEDSupervisor "+stringF(this->getApplicationDescriptor()->getInstance())+" ::PhysicsRunning."+" Baseline Monitoring during Physics Running could not be done!",DIAGDEBUG);
	}
      } // if readBaselineCorr	
      blTimer.stop();
#endif

      // Read the LastDAC FIFO
#ifdef READ_LASTDAC
      //if(readLastDACFifo && newEvent ) { 
      if(readLastDACFifo) {
	if(newEvent && (countLoops%100)==0 ) {  // readout only 1/100 events 
	
	  uint32_t buffer[2048];
	  unsigned int numWords = iFED->drainTemperatureFifo(buffer);
	  
	  //diagService_->reportError("--> numWords = "+stringF(numWords),DIAGINFO);
	  fwrite(buffer, sizeof(uint32_t), numWords, lastDacFile_[fednumber]);  // (unsigned long)buffer -> (uint32_t)buffer
	  
	  // Comment out the code below. It will flood the diagService with tons of messages. d,k, 20/1/11
	  // 	TemperatureFIFODecoder temperatureFIFODecoder(buffer, numWords);
	  // 	diagService_->reportError( "FED VME Address = 0x"+htoa(vmeBaseAddress)+" -------" ,DIAGINFO);
	  // 	temperatureFIFODecoder.printBuffer(std::cout);
	  // 	const std::list<PixelLastDACTemperature>& lastDACTemperatureReadings = temperatureFIFODecoder.getLastDACTemperatures();
	  // 	for ( std::list<PixelLastDACTemperature>::const_iterator lastDACTemperatureReading = lastDACTemperatureReadings.begin();
	  // 	      lastDACTemperatureReading != lastDACTemperatureReadings.end(); ++lastDACTemperatureReading ) {
	  // 	  unsigned int fedChannel = lastDACTemperatureReading->getFEDChannel();
	  // 	  unsigned int readOutChipId = lastDACTemperatureReading->getReadOutChipId();
	  // 	  unsigned int dacValue = lastDACTemperatureReading->getDACValue();
	  // 	  diagService_->reportError(" fedchannel, roc, value "+stringF(fedChannel)+" "+stringF(readOutChipId)+" "+stringF(dacValue),DIAGINFO);	
	  // 	}
	} // if skip
      } // is lastdac
#endif

    } // End FED loop
    
    // Update time stamp.
    *errTableTimeStampPtr = time(NULL);
	  
    // Update flash list
    monitorInfoSpace->fireItemValueChanged("errorTable", this);
    /*cout << "FlashList is updated" << endl;
    int a(-99); int b(-99); int c(-99);  int d(-99); int e(-99);
    for (unsigned int j = 0; j < errTable->getRowCount(); j++){
      a = *((xdata::UnsignedInteger32*)errTable->getValueAt(j,"fedNumber"));
      b = *((xdata::UnsignedInteger32*)errTable->getValueAt(j,"channelNumber"));
      c = *((xdata::UnsignedInteger32*)errTable->getValueAt(j,"NORErrors"));
      d = *((xdata::UnsignedInteger32*)errTable->getValueAt(j,"OOSErrors"));
      e = *((xdata::UnsignedInteger32*)errTable->getValueAt(j,"TimeOutErrors"));
      cout<<"FED = "<<a<<" CH = "<<b<<" NOR = "<<c<<" OOS = "<<d<<" TimeOut = "<<e<< endl;
    }
    */
    // End and update the event counter
    eventNumber_=newEventNumber;

    //}  // if new event
  // tuning output from timers
  wlTimer.stop();

  if (timing) {
    std::cout << "PhysicsRunning total time=" << stringF(wlTimer.tottime()) << std::endl;
    
    std::cout << "PhysicsRunning readFifoStatus times=" << stringF(statusTimer.ntimes()) << " total time =" << stringF(statusTimer.tottime()) 
	      << " avg time =" << stringF(statusTimer.avgtime()) << std::endl;
    std::cout << "PhysicsRunning readSpy times=" << stringF(spyTimer.ntimes()) << " total time =" << stringF(spyTimer.tottime()) 
	      << " avg time =" << stringF(spyTimer.avgtime()) << std::endl;
    std::cout << "PhysicsRunning readErrorFifo=" << stringF(errTimer.ntimes()) << " total time =" << stringF(errTimer.tottime()) 
	      << " avg time =" << stringF(errTimer.avgtime()) << std::endl;
    std::cout << "PhysicsRunning readttsFifo=" << stringF(ttsTimer.ntimes()) << " total time =" << stringF(ttsTimer.tottime()) 
	      << " avg time =" << stringF(ttsTimer.avgtime()) << std::endl;
    std::cout << "PhysicsRunning readBaseline=" << stringF(blTimer.ntimes()) << " total time =" << stringF(blTimer.tottime()) 
	      << " avg time =" << stringF(blTimer.avgtime()) << std::endl;
    
    std::cout << "PhysicsRunning readFifoStatusHW times=" << stringF(statusTimerHW.ntimes()) << " total time =" << stringF(statusTimerHW.tottime()) 
	      << " avg time =" << stringF(statusTimerHW.avgtime()) << std::endl;
    std::cout << "PhysicsRunning readSpyHW times=" << stringF(spyTimerHW.ntimes()) << " total time =" << stringF(spyTimerHW.tottime()) 
	      << " avg time =" << stringF(spyTimerHW.avgtime()) << std::endl;
    std::cout << "PhysicsRunning readErrorFifoHW times=" << stringF(errTimerHW.ntimes()) << " total time =" << stringF(errTimerHW.tottime()) 
	      << " avg time =" << stringF(errTimerHW.avgtime()) << std::endl;
    std::cout << "PhysicsRunning readttsFifoHW times=" << stringF(ttsTimerHW.ntimes()) << " total time =" << stringF(ttsTimerHW.tottime()) 
	      << " avg time =" << stringF(ttsTimerHW.avgtime()) << std::endl;
    std::cout << "PhysicsRunning readBaselineHW times=" << stringF(blTimerHW.ntimes()) << " total time =" << stringF(blTimerHW.tottime()) 
	      << " avg time =" << stringF(blTimerHW.avgtime()) << std::endl;
  }

  } catch ( HAL::BusAdapterException & hardwareError ) {
    diagService_->reportError("Hardware error in the FED Physics workloop. Message: "+string(hardwareError.what()),DIAGERROR);
  }
  catch (xcept::Exception & xdaqError ) {
    diagService_->reportError("XDAQ error in the FED Physics workloop. Message: "+string(xdaqError.what()),DIAGERROR);
  }
  catch (...) {
    diagService_->reportError("Unknown exception caught in the FED Physics workloop.",DIAGERROR);
  }
  wlTimer.stop();
  return true;
}

xoap::MessageReference PixelFEDSupervisor::FSMStateRequest (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  return MakeSOAPMessageReference(state_);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// LOW LEVEL COMMANDS //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

xoap::MessageReference PixelFEDSupervisor::ReadLastDACFIFO (xoap::MessageReference message) throw (xoap::exception::Exception) 
{
  xoap::MessageReference soapRequest = MakeSOAPMessageReference_readLastDACFIFO("readLastDACFIFOResponse");
  
  return soapRequest;
}

xoap::MessageReference PixelFEDSupervisor::SetChannelOffsets (xoap::MessageReference msg) throw (xoap::exception::Exception) {
assert(0);
  Attribute_Vector parameters(channelsPerFED+opticalReceiversPerFED+1);
  for (unsigned int ichannel=0;ichannel<channelsPerFED;++ichannel) parameters.at(ichannel).name_="ChannelOffsetDAC"+itoa(ichannel);
  for (unsigned int iopto=0;iopto<opticalReceiversPerFED;++iopto) parameters.at(channelsPerFED+iopto).name_="OpticalReceiverInput"+itoa(iopto);
  parameters.at(channelsPerFED+opticalReceiversPerFED).name_="VMEBaseAddress";
  Receive (msg, parameters);

  xoap::MessageReference reply = MakeSOAPMessageReference("SetChannelOffsetsDone");

  unsigned long vmeBaseAddress=atoi(parameters.at(channelsPerFED+opticalReceiversPerFED).value_.c_str());
  PixelFEDCard fedCard=FEDInterface_[vmeBaseAddress]->getPixelFEDCard();

  for (unsigned int ichannel=0;ichannel<channelsPerFED;++ichannel) fedCard.offs_dac[ichannel]=atoi(parameters.at(ichannel).value_.c_str());
  for (unsigned int iopto=0;iopto<opticalReceiversPerFED;++iopto) fedCard.offs_dac[iopto]=atoi(parameters.at(channelsPerFED+iopto).value_.c_str());

  FEDInterface_[vmeBaseAddress]->setupFromDB(fedCard);

  //ryd - I do not understand what this is doing...
  //changed to to write to /tmp... FIXME
  fedCard.writeASCII("/tmp/params_fed_1.dat"); // should write to fednumber

  return reply;
}

xoap::MessageReference PixelFEDSupervisor::ReloadFirmware (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  Attribute_Vector parameters(1);
  parameters[0].name_="VMEBaseAddress";
  Receive(msg, parameters);

  xoap::MessageReference reply = MakeSOAPMessageReference("ReloadFirmwareDone");

  FEDInterface_[atoi(parameters[0].value_.c_str())]->loadFPGA();
  if (FEDInterface_[atoi(parameters[0].value_.c_str())]->reset()==-1) reply=MakeSOAPMessageReference("ReloadFirmwareNotDone");

  return reply;
}

xoap::MessageReference PixelFEDSupervisor::ResetFEDs (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  Attribute_Vector parameters(1);
  parameters[0].name_="VMEBaseAddress";
  Receive(msg, parameters);

  xoap::MessageReference reply = MakeSOAPMessageReference("ResetFEDsDone");

  VMEPtr_[atoi(parameters[0].value_.c_str())]->write("LRES",0x80000000);

  return reply;
}

xoap::MessageReference PixelFEDSupervisor::FillTestDAC (xoap::MessageReference msg) throw (xoap::exception::Exception) {
assert(0);
  std::string reply="FillTestDACDone";

  if (theCalibObject_==0) {

    *console_<<"FillTestDAC - theCalibObject_ doesn't exist!"<<std::endl;
    diagService_->reportError("The theCalibObject_ doesn't exist.", DIAGERROR);
    reply="FillTestDACFailed";

  } else {

    std::vector<unsigned int> pulseTrain1(256), pulseTrain2(256), pulseTrain3(256);
    PixelFEDTestDAC* testDACCalib=dynamic_cast<PixelFEDTestDAC*>(theCalibObject_);
    assert(testDACCalib);
    pulseTrain1=testDACCalib->dacs();
    pulseTrain2=testDACCalib->dacs();
    pulseTrain3=testDACCalib->dacs();
    //for(unsigned int ii=0;ii<256;ii++){      
    //  pulseTrain1[ii]=500
    //  if (ii<30&&ii>20)  pulseTrain1[ii]=300;
    //}

    pulseTrain2=pulseTrain1;
    pulseTrain3=pulseTrain1;

    for (unsigned int i=0; i<pulseTrain1.size(); ++i) {

      diagService_->reportError("PixelFEDSupervisor::FillTestDAC. pulseTrain1["+stringF(i)+"]="+stringF(pulseTrain1[i]),DIAGINFO);
    }

    for (FEDInterfaceMap::iterator iFED=FEDInterface_.begin();iFED!=FEDInterface_.end();++iFED) {
      //iFED->second->fillDACRegister(pulseTrain1, pulseTrain2, pulseTrain3);
      iFED->second->fillDACRegister(pulseTrain1, pulseTrain2, pulseTrain3);
    }

  }

  return MakeSOAPMessageReference(reply);
}

xoap::MessageReference PixelFEDSupervisor::EnableFIFO3 (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  Attribute_Vector parameters(1);
  parameters[0].name_="VMEBaseAddress";
  Receive(msg, parameters);

  xoap::MessageReference reply=MakeSOAPMessageReference("EnableFIFO3Done");

  if (FEDInterface_[atoi(parameters[0].value_.c_str())]->enableSpyMemory(1) == -1) reply=MakeSOAPMessageReference("EnableFIFO3NotDone");

  return reply;
}

xoap::MessageReference PixelFEDSupervisor::VMETrigger (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  Attribute_Vector parameters(1);
  parameters[0].name_="VMEBaseAddress";
  Receive(msg, parameters);

  xoap::MessageReference reply=MakeSOAPMessageReference("VMETriggerDone");

  if (FEDInterface_[atoi(parameters[0].value_.c_str())]->generateVMETrigger() == -1) reply=MakeSOAPMessageReference("VMETriggerNotDone");

  return reply;
}

xoap::MessageReference PixelFEDSupervisor::BaselineRelease (xoap::MessageReference msg) throw (xoap::exception::Exception) {
assert(0);
  Attribute_Vector parameters(2);
  parameters[0].name_="VMEBaseAddress";
  parameters[1].name_="FEDChannel";
  Receive(msg, parameters);

  xoap::MessageReference reply=MakeSOAPMessageReference("BaselineReleaseDone");

  if (parameters[1].value_=="*") {
    FEDInterface_[atoi(parameters[0].value_.c_str())]->BaselineCorr_off();
  } else {
    int channel=atoi(parameters[1].value_.c_str());
    if (channel<1 || channel>36){
      diagService_->reportError("Invalid Channel passed to PixelFEDSupervisor::BaselineHold. Channel= " + parameters[1].value_,DIAGERROR);
      assert(0);
    } else {
      FEDInterface_[atoi(parameters[0].value_.c_str())]->BaselineCorr_off(channel);
      diagService_->reportError("Turned off baseline in channel " + parameters[1].value_,DIAGTRACE);
    }
  }

  return reply;
}

xoap::MessageReference PixelFEDSupervisor::BaselineSet (xoap::MessageReference msg) throw (xoap::exception::Exception) {
assert(0);
  Attribute_Vector parameters(5);
  parameters[0].name_="VMEBaseAddress";
  parameters[1].name_="Nbaseln";
  parameters[2].name_="NCbaseln";
  parameters[3].name_="SCbaseln";
  parameters[4].name_="Sbaseln";
  Receive(msg, parameters);

  xoap::MessageReference reply=MakeSOAPMessageReference("BaselineSetDone");

  FEDInterface_[atoi(parameters[0].value_.c_str())]->set_BaselineCorr(1, atol(parameters[1].value_.c_str()));
  FEDInterface_[atoi(parameters[0].value_.c_str())]->set_BaselineCorr(2, atol(parameters[2].value_.c_str()));
  FEDInterface_[atoi(parameters[0].value_.c_str())]->set_BaselineCorr(3, atol(parameters[3].value_.c_str()));
  FEDInterface_[atoi(parameters[0].value_.c_str())]->set_BaselineCorr(4, atol(parameters[4].value_.c_str()));

  return reply;
}

xoap::MessageReference PixelFEDSupervisor::BaselineHold (xoap::MessageReference msg) throw (xoap::exception::Exception) {
assert(0);
  Attribute_Vector parameters(2);
  parameters[0].name_="VMEBaseAddress";
  parameters[1].name_="FEDChannel";
  Receive(msg, parameters);

  xoap::MessageReference reply=MakeSOAPMessageReference("BaselineHoldDone");


  if (parameters[1].value_=="*") {
    FEDInterface_[atoi(parameters[0].value_.c_str())]->BaselineCorr_on();
  } else {
    int channel=atoi(parameters[1].value_.c_str());
    if (channel<1 || channel>36){
      diagService_->reportError("Invalid Channel passed to PixelFEDSupervisor::BaselineHold. Channel= " + parameters[1].value_,DIAGERROR);
      assert(0);
    } else {
      FEDInterface_[atoi(parameters[0].value_.c_str())]->BaselineCorr_on(channel);
    }
  }

  return reply;
}

xoap::MessageReference PixelFEDSupervisor::BaselineMonitor (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
assert(0);
  Attribute_Vector parameters(3);
  parameters[0].name_="VMEBaseAddress";
  parameters[1].name_="ShipTo";
  parameters[2].name_="Time";
  Receive(msg, parameters);

  FEDInterfaceMap::const_iterator beginFED,endFED;

  // BEGIN - PixelFEDMonitor: Robert

  // Flag to indicate updated baseline calibration information available.
  bool newBaselineData = false;

  // END - PixelFEDMonitor: Robert

  if (parameters[0].value_=="*") {
    beginFED=FEDInterface_.begin();
    endFED=FEDInterface_.end();
  } else {
    unsigned long vmeBaseAddress = atoi(parameters[0].value_.c_str());
    beginFED=FEDInterface_.find(vmeBaseAddress);
    assert(beginFED!=FEDInterface_.end());
    endFED=beginFED;
    ++endFED;
  }

  FEDInterfaceMap::const_iterator iterFED=beginFED;

  for(;iterFED!=endFED;++iterFED){

    unsigned long vmeBaseAddress = iterFED->first;
    unsigned long fednumber = theFEDConfiguration_->FEDNumberFromCrateAndVMEBaseAddress(crate_,vmeBaseAddress);

    static std::map <unsigned long, map <unsigned int, Moments> > baselineCorrection;  

    uint64_t enabledChans=FEDInterface_[vmeBaseAddress]->getPixelFEDCard().enabledChannels();
    for (unsigned int channel=0;channel<36;++channel) {
      if (((enabledChans & (0x1LL << channel))>>channel)==1) {  //d.k. add LL 12/07
        Moments *baselineCorrectionOfChannel=&(baselineCorrection[vmeBaseAddress][channel]);
        baselineCorrectionOfChannel->push_back(FEDInterface_[vmeBaseAddress]->get_BaselineCorr(channel+1));  //FIXME could be faster with void PixelFEDInterface::get_BaselineCorr(int *)
        // if ((baselineCorrectionOfChannel->count()+990)%1000==0) {
        if ((baselineCorrectionOfChannel->count()+9990)%10000==0) {  // prescale by additinal 10 d.k. 6/6/11
          toolbox::TimeVal timeOfDay=toolbox::TimeVal::gettimeofday();
          if (parameters[1].value_=="Console") {
	    diagService_->reportError("Time: "+timeOfDay.toString("%c", timeOfDay.tz()),DIAGINFO);
            diagService_->reportError(" : Baseline Correction for FED 0x"+htoa(vmeBaseAddress)+", Channel "+itoa(channel+1) +" has Mean="+stringF(baselineCorrectionOfChannel->mean())+" and StdDev="+stringF(baselineCorrectionOfChannel->stddev()),DIAGINFO);
        
	  } else if (parameters[1].value_=="File") {
            ofstream baselineFile;
            baselineFile.open((outputDir()+"/BaselineCorrectionFED_"+itoa(fednumber)+"_"+runNumber_+".txt").c_str(), ios::app);
            baselineFile<<"Time: "<<timeOfDay.toString("%c", timeOfDay.tz());
            baselineFile<<" : Baseline Correction for FED 0x"<<std::hex<<vmeBaseAddress<<std::dec<<", Channel "<<(channel+1)
                        <<" has Mean="<<baselineCorrectionOfChannel->mean()
                        <<" and StdDev="<<baselineCorrectionOfChannel->stddev()<<std::endl;
            baselineFile.close();            
          }

	  // BEGIN - PixelFEDMonitor: Robert
	  
	  newBaselineData = true;	

	  // Put baseline correction information into map.  Real channel number is one-based! 
	  baselineCorrectionMap[fednumber][channel + 1].first = baselineCorrectionOfChannel->mean();
	  baselineCorrectionMap[fednumber][channel + 1].second = baselineCorrectionOfChannel->stddev();
	  
	  // Uncomment for debug
	  // std::cout << "BC for fed " << fednumber << " and channel " << (channel +1) << " is " << 
	  //	baselineCorrectionOfChannel->mean() << std::endl;

	  // END - PixelFEDMonitor: Robert

          baselineCorrectionOfChannel->clear();	
        }
      }
    }

    // BEGIN - PixelFEDMonitor: Robert
    // Update flashlist with dump of map if new baseline correction data available.
    if (newBaselineData == true)
      {
	unsigned int crateTableRowNumber = 0;

	// Reset crate table.
	crateTablePtr->clear();
	crateTablePtr->addColumn("fedNumber","unsigned int 32");	
  	crateTablePtr->addColumn("fedTable","table");

	std::map<unsigned int, std::map<unsigned int, pair<float,float> > >::iterator fedIter = baselineCorrectionMap.begin();
	while (fedIter != baselineCorrectionMap.end())
	  {
	    unsigned int fedTableRowNumber = 0;
	    
	    // Reset fed table.
	    fedTablePtr->clear();
	    fedTablePtr->addColumn("channelNumber","unsigned int 32");	
  	    fedTablePtr->addColumn("baselineCorrectionMean","float");	
  	    fedTablePtr->addColumn("baselineCorrectionStdDev","float");	

	    *fedNumberPtr = fedIter->first;
	    std::map<unsigned int, pair<float,float> >::iterator channelIter = fedIter->second.begin();
	    while (channelIter != fedIter->second.end())
	      {
		*channelNumberPtr = channelIter->first;
		*baselineCorrectionMeanPtr = channelIter->second.first; 
		*baselineCorrectionStdDevPtr = channelIter->second.second;

		fedTablePtr->setValueAt(fedTableRowNumber,"channelNumber",*channelNumberPtr);
		fedTablePtr->setValueAt(fedTableRowNumber,"baselineCorrectionMean",*baselineCorrectionMeanPtr);
		fedTablePtr->setValueAt(fedTableRowNumber,"baselineCorrectionStdDev",*baselineCorrectionStdDevPtr);
		
		channelIter++;
		fedTableRowNumber++;
	      }
	    crateTablePtr->setValueAt(crateTableRowNumber,"fedNumber",*fedNumberPtr);      
	    crateTablePtr->setValueAt(crateTableRowNumber,"fedTable",*fedTablePtr);      

	    fedIter++;
	    crateTableRowNumber++;
	  }
	
	// Update time stamp.
	*crateTimeStampPtr = time(NULL);
	
	// Update flashlist.
	std::list<std::string> names;
	names.push_back("crateTimeStamp");
	monitorInfoSpace->fireItemGroupChanged(names, this);
      }
    // END - PixelFEDMonitor: Robert

    if (parameters[2].value_=="Last") {
      baselineCorrection.clear();
    } 
       
  }


  xoap::MessageReference reply=MakeSOAPMessageReference("BaselineMonitorDone");
  return reply;
}


xoap::MessageReference PixelFEDSupervisor::SetControlRegister(xoap::MessageReference msg) throw (xoap::exception::Exception) {
  xoap::MessageReference reply=MakeSOAPMessageReference("SetControlRegisterDone");

  Attribute_Vector parameters(3);
  parameters[0].name_="ControlRegister";
  parameters[1].name_="ModeRegister";
  parameters[2].name_="VMEBaseAddress";
  Receive(msg, parameters);

 
  diagService_->reportError("Control Register = " + stringF(parameters[0].value_.c_str()),DIAGDEBUG);
  diagService_->reportError("Mode Register= " + stringF(parameters[1].value_.c_str()),DIAGDEBUG);
 
  PixelFEDInterface* iFED=FEDInterface_[atoi(parameters[2].value_.c_str())];

  PixelFEDCard iPixelFEDCard=iFED->getPixelFEDCard();
  iPixelFEDCard.Ccntrl=atoi(parameters[0].value_.c_str());
  iPixelFEDCard.modeRegister=atoi(parameters[1].value_.c_str());
  if (iFED->setupFromDB(iPixelFEDCard) == -1) reply=MakeSOAPMessageReference("SetControlRegisterFailed");

  return reply;
}

xoap::MessageReference PixelFEDSupervisor::ReadFIFO (xoap::MessageReference msg) throw (xoap::exception::Exception) {

  Attribute_Vector parameters(8);

  parameters[0].name_="FIFO";
  parameters[1].name_="Mode";
  parameters[2].name_="ShipTo";
  parameters[3].name_="Filename";
  parameters[4].name_="Additional";
  parameters[5].name_="Channel";
  parameters[6].name_="Time";
  parameters[7].name_="VMEBaseAddress";
  Receive(msg, parameters);

  FEDInterfaceMap::const_iterator beginFED,endFED;

  if (parameters[7].value_=="*") {
    beginFED=FEDInterface_.begin();
    endFED=FEDInterface_.end();

  } else {
    unsigned long vmeBaseAddress = atoi(parameters[7].value_.c_str());
    beginFED=FEDInterface_.find(vmeBaseAddress);
    assert(beginFED!=FEDInterface_.end());
    endFED=beginFED;
    ++endFED;
  }


  FEDInterfaceMap::const_iterator iterFED=beginFED;

  for(;iterFED!=endFED;++iterFED){

    try {
      PixelFEDInterface* iFED=iterFED->second; 

      unsigned long vmeBaseAddress = iterFED->first;

      if (parameters[0].value_=="1") {

	if (parameters[1].value_=="Transparent") {
	  
	  if (parameters[5].value_=="*") {
	    
	    diagService_->reportError("Reading FIFO 1 of all channels in transparent mode is not implemented yet!", DIAGERROR);
	    
	  } else {
	    
	    uint32_t buffer[pos::fifo1TranspDepth];
	    unsigned int channel=atoi(parameters[5].value_.c_str());
	    int status=iFED->drain_transBuffer(channel, buffer);	        
	    if (status<0) {
	      diagService_->reportError("The contents of SpyFIFO 1, channel " + stringF(channel) + " transparent mode could not be drained into the buffer.", DIAGERROR);
	      
	    }
	    
	    if (parameters[2].value_=="Screen") {
	      
	      //dataFIFO1_[vmeBaseAddress]->str()="";
	      
	      // HACK -- Take out later! FIXME
	      ofstream fout;
	      fout.open("/tmp/DebugFIFO1Data.txt", ios::app);
	      for (unsigned int i=0;i<=pos::fifo1TranspDepth;++i) {
		fout<<"FED channel="<<channel<<"buffer["<<i<<"]=0x"<<std::hex<<buffer[i]<<std::dec<<" , ADC[i"<<i<<"]="<<((buffer[i] & 0xffc00000) >> 22)<<std::endl;
	      }
	      fout<<"----------------------------------------------------------------------------"<<std::endl;
	      fout.close();
	      ///////////////////////////
	      
	      dataFIFO1_[vmeBaseAddress]=new std::stringstream();
	      *(dataFIFO1_[vmeBaseAddress])<<"Contents of Spy FIFO 1 in Transparent Mode"<<std::endl;
	      *(dataFIFO1_[vmeBaseAddress])<<"----------------------"<<std::endl;
	      for (unsigned int i=0;i<=pos::fifo1TranspDepth;++i) {
		*(dataFIFO1_[vmeBaseAddress])<<"Word at clock #"<<i<<" = 0x"<<hex<<buffer[i]<<dec<<", ADC= "<<((buffer[i] & 0xffc00000) >> 22)<<endl;
	      }
	      *(dataFIFO1_[vmeBaseAddress])<<"----------------------"<<std::endl;
	      
	    } else if (parameters[2].value_=="Console") {
	    
	      diagService_->reportError("Contents of Spy FIFO 1 in Transparent Mode", DIAGINFO);
	      diagService_->reportError("----------------------", DIAGINFO);

	    for (unsigned int i=0;i<=pos::fifo1TranspDepth;++i) {
	      // diagService_->reportError("Word at clock #"+stringF(i)+" = 0x"+htoa(buffer[i])+", ADC= "+itoa((buffer[i] & 0xffc00000)+22)<, DIAGINFO);
	      std::cout<<"Word at clock #"<<i<<" = 0x"<<hex<<buffer[i]<<dec<<", ADC= "<<((buffer[i] & 0xffc00000) >> 22)<<endl;
	    }

	     diagService_->reportError("----------------------", DIAGINFO);

	  } else if (parameters[2].value_=="File") {

	    ofstream fout;
	    if (parameters[6].value_=="True") fout.open(("/tmp/"+parameters[3].value_).c_str(), ios::trunc);
	    if (parameters[6].value_=="False") fout.open(("/tmp/"+parameters[3].value_).c_str(), ios::app);
	    for (unsigned int i=0;i<=pos::fifo1TranspDepth;++i) {
	      fout<<parameters[4].value_<<" "<<parameters[5].value_<<" "<<i<<" "<<dec<<((buffer[i] & 0xffc00000) >> 22)<<endl;
	    }
	    fout.close();
	    
	  } else if (parameters[2].value_=="GIF") {

	    PixelDecodedFEDRawData decodedRawData(buffer, 100., 100., 150., 0., 100., 0., 150.);
	    std::string tbmSignalFilename=parameters[3].value_;
	    decodedRawData.drawToFile("/tmp/"+tbmSignalFilename);

	  } else {

	    diagService_->reportError("Sorry, shipping FIFO 1 Transparent mode to "+parameters[2].value_+" has not been implemented", DIAGERROR);

	  }

	}

      } else if (parameters[1].value_=="Normal") {

	if (parameters[5].value_=="*") {

	  diagService_->reportError("Reading FIFO 1 of all channels in normal mode is not implemented yet!", DIAGERROR);

	} else {

	  uint32_t buffer[pos::fifo1NormalDepth];
	  unsigned int channel=atoi(parameters[5].value_.c_str());
	  int status=iFED->drainFifo1(channel, buffer);
	  if (status<0) {
	    diagService_->reportError("The contents of SpyFIFO 1, channel " + stringF(channel) + "could not be drained into the buffer.", DIAGERROR);
	   
	  }

	  if (parameters[2].value_=="Screen") {
	    
	    *(dataFIFO1_[vmeBaseAddress])<<"Contents of Spy FIFO 1 in Normal Mode"<<std::endl;
	    *(dataFIFO1_[vmeBaseAddress])<<"----------------------"<<std::endl;
	    for (unsigned int i=0;i<(unsigned int)status;++i) {
	      *(dataFIFO1_[vmeBaseAddress])<<"FIFO 1 word at position "<<i<<" = 0x"<<hex<<buffer[i]<<dec<<endl;
	    }
	    *(dataFIFO1_[vmeBaseAddress])<<"----------------------"<<std::endl;

	  } else if (parameters[2].value_=="Console") {

	    diagService_->reportError("This is FED VME=0x"+htoa(vmeBaseAddress)+", channel "+itoa(channel)+" on Spy FIFO 1 in Normal Mode.", DIAGINFO);
	    PixelFEDFifoData::decodeNormalData(buffer, 255);

	  } else if (parameters[2].value_=="File") {
      
	    ofstream fout(("/tmp/"+parameters[3].value_).c_str(), ios::app);
	    for (unsigned int i=0;i<=pos::fifo1NormalDepth;++i) {
	      fout<<"FED VME=0x"<<hex<<vmeBaseAddress<<dec<<", Channel "<<channel<<", word "<<i<<" is 0x"<<hex<<buffer[i]<<endl;
	    }
	    fout.close();

	  } else {

	    diagService_->reportError("Sorry, shipping FIFO 1 Normal Mode to "+parameters[2].value_+" has not been implemented", DIAGERROR);

	  }

	}

      }

    } else if (parameters[0].value_=="2") {

      uint32_t buffer[pos::fifo2Depth];
      int status=iFED->drainDataFifo2(buffer);
      if (status>0) {
	
	FIFO2Decoder decodedFIFO2data(buffer, status);

	if (parameters[2].value_=="Screen") {
	  dataFIFO2_[vmeBaseAddress]->str()="";
	  decodedFIFO2data.printToStream(*(dataFIFO2_[vmeBaseAddress]));
	} else if (parameters[2].value_=="Console") {
	  decodedFIFO2data.printToStream(std::cout);
	}

      } else {
	
	diagService_->reportError("PixelFEDSupervisor::ReadFIFO -- Cannot read FIFO2!", DIAGERROR);
	
      }

    } else if (parameters[0].value_=="3") {

      if (parameters[2].value_=="Screen") {
	uint64_t buffer64[pos::slinkDepth];
	int status=iFED->spySlink64(buffer64);

	if (status>0) {
	  dataFIFO3_[vmeBaseAddress]->str()="";
	  *(dataFIFO3_[vmeBaseAddress])<<"Contents of Spy FIFO 3"<<std::endl;
	  *(dataFIFO3_[vmeBaseAddress])<<"----------------------"<<std::endl;
	  for (int i=0; i<=status;++i) {
	    *(dataFIFO3_[vmeBaseAddress])<<"Clock "<<i<<" = 0x"<<std::hex<<buffer64[i]<<std::dec<<std::endl;
	  }
	}

      } else if (parameters[2].value_=="RUBuilder") {
#ifdef RUBUILDER
	uint64_t buffer64[512];
	int status=iFED->spySlink64(buffer64);
	if (status>=0) {
	  for (unsigned int i=0;i<=(unsigned int)status;++i) {
	    diagService_->reportError("FED VME="+parameters[7].value_+", word "+itoa(i)+" = "+htoa(buffer64[i]), DIAGINFO);
	  }

	  int rub_event_number = 10;

	  { // Shipping one superfragment to EVM
	    I2OEventDataBlockSender sender(this);
	    sender.setXFunctionCode(I2O_EVM_TRIGGER);
	    sender.setEventNumber(rub_event_number);
	    sender.setDestinationDescriptor(getApplicationContext()->getApplicationGroup()->getApplicationDescriptor("EVM",0));
	    sender.setBlockNb(0);
	    sender.setSuperFragmentNb(0);
	    sender.setNbSuperFragmentsInEvent(2);
	    sender.setNbBlocksInSuperFragment(1);
	    sender.send();
	    diagService_->reportError("sending TA->EVM",DIAGTRACE);
	    
	  }

	  { // Shipping the other superfragment to the RU
	    I2OEventDataBlockSender ship(this, status*sizeof(uint64_t));
	    ship.setDestinationDescriptor(getApplicationContext()->getApplicationGroup()->getApplicationDescriptor("RU",0));
	    ship.setXFunctionCode(I2O_RU_DATA_READY);
	    ship.setEventNumber(rub_event_number);
	    ship.setNbBlocksInSuperFragment(1);
	    ship.setNbSuperFragmentsInEvent(2);
	    ship.setSuperFragmentNb(1);
	    ship.setBlockNb(0);

	    memcpy(ship.getTrailingBuffer(),buffer64,status*sizeof(uint64_t));
	    ship.send();
	    diagService_->reportError("sending RUI->RU",DIAGTRACE);
	  }
	}
#endif
      }	else if (parameters[2].value_=="File") {

  

	uint64_t buffer64[4096];

	if (fileopen_.find(vmeBaseAddress)==fileopen_.end()){
	  fileopen_[vmeBaseAddress]=false;
	}

	if (fileopen_[vmeBaseAddress]==false) {

	  unsigned int fedid=iFED->getPixelFEDCard().fedNumber;
	  assert(false==fileopen_[vmeBaseAddress]);
	  fout_[vmeBaseAddress]=fopen((outputDir()+"/"+parameters[3].value_+"_"+itoa(fedid)+"_"+runNumber_+".dmp").c_str(), "wb");  // C I/O
	  
	  diagService_->reportError("outputDir: "+outputDir(),DIAGINFO);
	  
	  (dynamic_cast <PixelCalibConfiguration*> (theCalibObject_))->writeASCII(outputDir());
	  theNameTranslation_->writeASCII(outputDir());
	  theDetectorConfiguration_->writeASCII(outputDir());
	  uint64_t runNumber=atol(runNumber_.c_str());
	  fwrite(&runNumber, sizeof(uint64_t), 1, fout_[vmeBaseAddress]);
	  fileopen_[vmeBaseAddress]=true;
	}

	int status=iFED->spySlink64(buffer64);
	
	if (status>=0 && fileopen_[vmeBaseAddress]==true) {
	  if (iFED->get_Printlevel()&4) {
	    std::cout << "Contents of Spy FIFO 3 for fedid " << iFED->getPixelFEDCard().fedNumber<<std::endl;
	    std::cout <<"----------------------"<<std::endl;
	    for (int i=0; i<=status;++i)
	      std::cout<<"Clock "<<i<<" = 0x"<<std::hex<<buffer64[i]<<std::dec<<std::endl;

	    FIFO3Decoder decode(buffer64);
	    std::cout << "here's what the decoder thinks:\n"
		      << "nhits: " << decode.nhits() << "\n";
	    for (unsigned jmt = 0; jmt < decode.nhits(); ++jmt) {
	      std::cout << "#" << jmt << ": channel: " << decode.channel(jmt)
			<< " rocid: " << decode.rocid(jmt) << " dcol: " << decode.dcol(jmt)
			<< " pxl: " << decode.pxl(jmt) << " pulseheight: " << decode.pulseheight(jmt)
			<< " col: " << decode.column(jmt) << " row: " << decode.row(jmt) << std::endl;
	    }
	    std::cout << "readDigFEDStatus(): ";
	    iFED->readDigFEDStatus(true, false);
	  }
	  fwrite(buffer64, sizeof(uint64_t), status, fout_[vmeBaseAddress]);
	}
	if (status < 0 && iFED->get_Printlevel()&4)
	  std::cout << "Spy FIFO3 for fedid " << iFED->getPixelFEDCard().fedNumber << " status is  " << status <<std::endl;

	if (parameters[6].value_=="Last" && fileopen_[vmeBaseAddress]==true) {
	  assert(true==fileopen_[vmeBaseAddress]);
	  fclose(fout_[vmeBaseAddress]);     // C I/o
	  fileopen_[vmeBaseAddress]=false;
	}

      }
    }

  } catch (HAL::HardwareAccessException& e) {
      diagService_->reportError("Caught HAL::HardwareAccessException : "+string(e.what()),DIAGERROR);
  } catch (exception e) {
    diagService_->reportError("*** Unknown exception occurred",DIAGWARN); 
  }
  }

  xoap::MessageReference reply = MakeSOAPMessageReference("ReadFIFODone");  
  return reply;
}

xoap::MessageReference PixelFEDSupervisor::ReadErrorFIFO (xoap::MessageReference msg) throw (xoap::exception::Exception) {

  xoap::MessageReference reply = MakeSOAPMessageReference("ReadErrorFIFODone");
  
  //diagService_->reportError("Read error fifo",DIAGINFO);

  Attribute_Vector parameters(4);

  parameters[0].name_="ShipTo";
  parameters[1].name_="Filename";
  parameters[2].name_="VMEBaseAddress";
  parameters[3].name_="Time";
  Receive(msg, parameters);

  FEDInterfaceMap::const_iterator beginFED,endFED;

  if (parameters[2].value_=="*") {
    beginFED=FEDInterface_.begin();
    endFED=FEDInterface_.end();
  } else {
    unsigned long vmeBaseAddress = atoi(parameters[2].value_.c_str());
    beginFED=FEDInterface_.find(vmeBaseAddress);
    assert(beginFED!=FEDInterface_.end());
    endFED=beginFED;
    ++endFED;
  }


  FEDInterfaceMap::const_iterator iterFED=beginFED;

  for(;iterFED!=endFED;++iterFED){

    PixelFEDInterface* iFED=iterFED->second; 

    unsigned long vmeBaseAddress = iterFED->first;
      

    uint32_t errBuffer[(36*1024)]; //max size?

    if (errBufferOpen_.find(vmeBaseAddress)==errBufferOpen_.end()){
      errBufferOpen_[vmeBaseAddress]=false;
    }


    int errCount=iFED->drainErrorFifo(errBuffer);
 
    if (parameters[0].value_=="File") {


      if (errBufferOpen_[vmeBaseAddress]==false) {
	unsigned int fedid=iFED->getPixelFEDCard().fedNumber;
	assert(errBufferOpen_[vmeBaseAddress]==false);
	errorFile2_[vmeBaseAddress]=fopen((outputDir_+"/"+parameters[1].value_+"_"+itoa(fedid)+"_"+runNumber_+".err").c_str(), "wb");
	unsigned long runNumber=atol(runNumber_.c_str());
	fwrite(&runNumber, sizeof(unsigned long), 1, errorFile2_[vmeBaseAddress]);
	errBufferOpen_[vmeBaseAddress]=true;
      }

      if (errCount>=0 && errBufferOpen_[vmeBaseAddress]==true) {
	//diagService_->reportError("Errors(s) from FED at VME "+ stringF(vmeBaseAddress),DIAGERROR);
	if (iFED->get_Printlevel()&4) {
	  ErrorFIFODecoder decodedErrorFIFO(errBuffer, errCount); // Suppress disabled channels later
	  decodedErrorFIFO.printToStream(std::cout);
	}
	fwrite(errBuffer, sizeof(unsigned long), errCount, errorFile2_[vmeBaseAddress]);
	
	
	for (int i_err=0; i_err<errCount; ++i_err) {

	  PixelFEDFifoData decoder;

	  decoder.decodeErrorFifo(errBuffer[i_err]);

	  unsigned int errorCode=(errBuffer[i_err] & 0x03e00000)>>21;
	  
	  //std::cout<<"errBuffer[i_err]=0x"<<hex<<errBuffer[i_err]<<" errorCode=0x"<<errorCode<<dec<<std::endl;
	  switch (errorCode) {
	  
	  case 0x1f: decoder.decodeErrorFifo(errBuffer[i_err]); break;
	    // case 0x1e: std::cout<<"Trailer Error."<<std::endl; break; Too many of these!
	  case 0x1d: decoder.decodeErrorFifo(errBuffer[i_err]); break;
	  case 0x1c: decoder.decodeErrorFifo(errBuffer[i_err]); break;
	  case 0x1b: decoder.decodeErrorFifo(errBuffer[i_err]); break;
	  case 0x1a: std::cout<<"Gap."<<std::endl; break;
	  }
	
	}
      }
      
      if (parameters[3].value_=="Last" && errBufferOpen_[vmeBaseAddress]==true) {
	assert(errBufferOpen_[vmeBaseAddress]==true);
	fclose(errorFile2_[vmeBaseAddress]);
	errBufferOpen_[vmeBaseAddress]=false;
      }
    } else if (parameters[0].value_=="Screen") {
      
      //uint64_t enabledChans=iFED->getPixelFEDCard().enabledChannels();
      ErrorFIFODecoder decodedErrorFIFO(errBuffer, errCount); // Suppress disabled channels later
      decodedErrorFIFO.printToStream(*(errorFIFO_[vmeBaseAddress]));
      
    } else if (parameters[0].value_=="Console") {
    
      //uint64_t enabledChans=iFED->getPixelFEDCard().enabledChannels();
      ErrorFIFODecoder decodedErrorFIFO(errBuffer, errCount); // Suppress disabled channels later
      decodedErrorFIFO.printToStream(std::cout);
    
    } else {
    
      diagService_->reportError("PixelFEDSupervisor::ReadErrorFIFO: Ship To = " + parameters[0].value_ + " not recognised.",DIAGERROR);
    
      assert(0);
      
    }
  }

  return reply;
  
}

xoap::MessageReference PixelFEDSupervisor::ReadTTSFIFO (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  for (FEDInterfaceMap::iterator iFED=FEDInterface_.begin();iFED!=FEDInterface_.end();++iFED) {
    uint32_t buffer[ttsDepth];
    unsigned int wordcount=iFED->second->drainTTSFifo(buffer);

    *(ttsFIFO_[iFED->first])<<"Wordcount = "<<wordcount<<std::endl;
    for (unsigned int i=0;i<=wordcount;++i){
      unsigned long word=buffer[i];
      *(ttsFIFO_[iFED->first])<<"TTS word in slot "<<i<<" is = 0x"<<hex<<word<<dec;
      
      switch (word&0xf) {
      case 0:	*(ttsFIFO_[iFED->first])<<"**TTS**Disconnected    "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;	break;
      case 1:	*(ttsFIFO_[iFED->first])<<"**TTS**Overflow Warning"<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;	break;
      case 2:	*(ttsFIFO_[iFED->first])<<"**TTS**Out of Sync     "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;	break;
      case 3:	*(ttsFIFO_[iFED->first])<<"**TTS**Undefined       "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;	break;
      case 4:	*(ttsFIFO_[iFED->first])<<"**TTS**Busy            "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;	break;
      case 5:	*(ttsFIFO_[iFED->first])<<"**TTS**Undefined       "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;	break;
      case 6:	*(ttsFIFO_[iFED->first])<<"**TTS**Undefined       "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;	break;
      case 7:	*(ttsFIFO_[iFED->first])<<"**TTS**Undefined       "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;	break;
      case 8:	*(ttsFIFO_[iFED->first])<<"**TTS**Ready           "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;	break;
      case 9:	*(ttsFIFO_[iFED->first])<<"**TTS**Undefined       "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;	break;
      case 10:*(ttsFIFO_[iFED->first])<<"**TTS**Undefined       "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;	break;
      case 11:*(ttsFIFO_[iFED->first])<<"**TTS**Undefined       "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;	break;
      case 12:*(ttsFIFO_[iFED->first])<<"**TTS**Error           "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;	break;
      case 13:*(ttsFIFO_[iFED->first])<<"**TTS**Undefined       "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;	break;
      case 14:*(ttsFIFO_[iFED->first])<<"**TTS**Undefined       "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;	break;
      case 15:*(ttsFIFO_[iFED->first])<<"**TTS**Disconnected    "<<" TTS Event = "<<dec<<((word&0xffffff0)>>4)<<endl;	break;
      }
    }
    *(ttsFIFO_[iFED->first])<<std::endl;
  }
  
  xoap::MessageReference reply=MakeSOAPMessageReference("ReadTTSFifoDone");
  return reply;
}

xoap::MessageReference PixelFEDSupervisor::ReadDataAndErrorFIFO (xoap::MessageReference msg) throw (xoap::exception::Exception) {

  std::string reply_string="ReadDataAndErrorFIFODone";

  static unsigned int counter=0;
  //const int prescale=17;
  const int prescale=999999;  // disable error&baseline readout to check the time savings
  //const int prescale=1;

  if (Receive(ReadFIFO(msg))!="ReadFIFODone") {
    reply_string="ReadDataAndErrorFIFOFailed";
    cout << reply_string << " after just ReadFIFO" << endl;
    diagService_->reportError("PixelFEDSupervisor::ReadDataAndErrorFIFO -- Reading data FIFO failed!",DIAGERROR);
  }

  if (counter%prescale==0){

    if (Receive(ReadErrorFIFO(msg))!="ReadErrorFIFODone") {
      reply_string="ReadDataAndErrorFIFOFailed";
      cout << reply_string << " after ReadErrorFIFO" << endl;
      diagService_->reportError("PixelFEDSupervisor::ReadDataAndErrorFIFO -- Reading error FIFO failed!",DIAGERROR);
    }

#ifndef PILOT_FED
    Attribute_Vector parameters(3);
    parameters[0].name_="VMEBaseAddress";
    parameters[1].name_="ShipTo";
    parameters[2].name_="Time";
    Receive(msg, parameters);
    msg=MakeSOAPMessageReference("BaselineMonitor", parameters);

    if (Receive(BaselineMonitor(msg))!="BaselineMonitorDone") {
      reply_string="ReadDataAndErrorFIFOFailed";
      cout << reply_string << " after BaselineMonitor" << endl;
      diagService_->reportError("PixelFEDSupervisor::ReadDataAndErrorFIFO -- Reading baseline correction failed!",DIAGERROR);
    }
#endif
  }

  
  counter++;

  xoap::MessageReference reply=MakeSOAPMessageReference(reply_string);
  return reply;

}

xoap::MessageReference PixelFEDSupervisor::SetADC1V2VEnMass (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
assert(0);
	Attribute_Vector inputParameters(1);
	inputParameters[0].name_ ="ADCRange";
	Receive(msg, inputParameters);
	int setting;
	if      (inputParameters[0].value_ == "1V") setting = 0;
	else if (inputParameters[0].value_ == "2V") setting = 1;
	else assert(0);

	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
        assert(tempCalibObject!=0);

	const std::set<PixelChannel>& channelList = tempCalibObject->channelList();

	// Loop over channels in the configuration.
	for ( std::set<PixelChannel>::const_iterator channelList_itr = channelList.begin(); channelList_itr != channelList.end(); channelList_itr++ )
	{
		const PixelHdwAddress& channelHdwAddress = theNameTranslation_->getHdwAddress(*channelList_itr);
		unsigned int crate=theFEDConfiguration_->crateFromFEDNumber( channelHdwAddress.fednumber() );
		if (crate!=crate_) continue;
		unsigned int VMEBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber( channelHdwAddress.fednumber() );

		FEDInterface_[VMEBaseAddress]->set_adc_1v2v( setting, channelHdwAddress.fedchannel() );
	}

	xoap::MessageReference reply=MakeSOAPMessageReference("SetADC1V2VEnMassDone");
	return reply;
}

xoap::MessageReference PixelFEDSupervisor::SetADC1V2VOneChannel (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
assert(0);
	Attribute_Vector inputParameters(3);
	inputParameters[0].name_ ="ADCRange";
	inputParameters[1].name_ ="VMEBaseAddress";
	inputParameters[2].name_ ="FEDChannel";
	Receive(msg, inputParameters);
	int VMEBaseAddress = atoi(inputParameters[1].value_.c_str());
	assert( FEDInterface_.find(VMEBaseAddress) != FEDInterface_.end() );
	int channel = atoi(inputParameters[2].value_.c_str());
	assert( 1 <= channel && channel <= 36 );
	
	int previousSetting = FEDInterface_[VMEBaseAddress]->get_adc_1v2v( channel );
	assert( previousSetting == 0 || previousSetting == 1 );
	
	int setting;
	if      (inputParameters[0].value_ == "1V") setting = 0;
	else if (inputParameters[0].value_ == "2V") setting = 1;
	else assert(0);
	FEDInterface_[VMEBaseAddress]->set_adc_1v2v( setting, channel );

	Attribute_Vector returnValues(1);
	returnValues[0].name_="PreviousADCRange";
	if ( previousSetting == 0 ) returnValues[0].value_ = "1V";
	else                        returnValues[0].value_ = "2V";
	xoap::MessageReference reply=MakeSOAPMessageReference("SetADC1V2VOneChannelDone", returnValues);
	return reply;
}

xoap::MessageReference PixelFEDSupervisor::SetFEDOffsetsEnMass (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
assert(0);
	Attribute_Vector inputParameters(2);
	inputParameters[0].name_ ="FEDReceiverInputOffset";
	inputParameters[1].name_ ="FEDChannelOffset";
	Receive(msg, inputParameters);

	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
        assert(tempCalibObject!=0);

	const std::set<PixelChannel>& channelList = tempCalibObject->channelList();

	// Loop over channels in the configuration.
	for ( std::set<PixelChannel>::const_iterator channelList_itr = channelList.begin(); channelList_itr != channelList.end(); channelList_itr++ )
	{
		const PixelHdwAddress& channelHdwAddress = theNameTranslation_->getHdwAddress(*channelList_itr);
		unsigned int crate=theFEDConfiguration_->crateFromFEDNumber( channelHdwAddress.fednumber() );
		if (crate!=crate_) continue;
		unsigned int VMEBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber( channelHdwAddress.fednumber() );
		unsigned int channel = channelHdwAddress.fedchannel();

		if ( inputParameters[0].value_ != "unchanged" )
		{
			int newFEDReceiverInputOffset = atoi( inputParameters[0].value_.c_str() );
			assert( 0 <= newFEDReceiverInputOffset && newFEDReceiverInputOffset <= 15 );
			FEDInterface_[VMEBaseAddress]->getPixelFEDCard().opt_inadj[(channel-1)/12] = newFEDReceiverInputOffset;
			FEDInterface_[VMEBaseAddress]->set_opto_params();
		}

		if ( inputParameters[1].value_ != "unchanged" )
		{
			int newFEDChannelOffset = atoi( inputParameters[1].value_.c_str() );
			assert( 0 <= newFEDChannelOffset && newFEDChannelOffset <= 255 );
			FEDInterface_[VMEBaseAddress]->getPixelFEDCard().offs_dac[channel-1] = newFEDChannelOffset;
			FEDInterface_[VMEBaseAddress]->set_offset_dacs();
		}
	}

	xoap::MessageReference reply=MakeSOAPMessageReference("SetFEDOffsetsEnMassDone");
	return reply;
}


void PixelFEDSupervisor::EndOfRunFEDReset() {
  
  std::map<std::pair<unsigned long, unsigned int>, std::set<unsigned int> >::iterator i_vmeBaseAddressAndFEDNumberAndChannels=vmeBaseAddressAndFEDNumberAndChannels_.begin();
  for (; i_vmeBaseAddressAndFEDNumberAndChannels!=vmeBaseAddressAndFEDNumberAndChannels_.end(); ++i_vmeBaseAddressAndFEDNumberAndChannels) {
    unsigned long vmeBaseAddress = i_vmeBaseAddressAndFEDNumberAndChannels->first.first;
    unsigned int fedNumber = i_vmeBaseAddressAndFEDNumberAndChannels->first.second;
    try {
      
      //send resets to FEDs
      cout<<"Sending resets to FED "<<fedNumber<<" ..."<<flush;
      VMEPtr_[vmeBaseAddress]->write("LRES",0x80000000);
      VMEPtr_[vmeBaseAddress]->write("CLRES",0x80000000);
      FEDInterface_[vmeBaseAddress]->resetSlink();

       //reset fake event counter
      uint32_t resword=(1<<23);
      VMEPtr_[vmeBaseAddress]->write("LAD_C",resword,HAL::HAL_NO_VERIFY,0x1c8000);
      //reset center OOS counter
      resword=(1<<15);
      VMEPtr_[vmeBaseAddress]->write("LAD_C",resword,HAL::HAL_NO_VERIFY,0x1c8000);
      
      // reset the error-fifo
      VMEPtr_[vmeBaseAddress]->write("NWrResetPls", 0x80000000 );
      VMEPtr_[vmeBaseAddress]->write("NCWrResetPls",0x80000000 );
      VMEPtr_[vmeBaseAddress]->write("SCWrResetPls",0x80000000 );
      VMEPtr_[vmeBaseAddress]->write("SWrResetPls", 0x80000000 );

      cout<<"done"<<endl;

    } catch (HAL::BusAdapterException & hwe) {
      diagService_->reportError("Caught HAL exception while resetting FED "+stringF(fedNumber)+" :" +string(hwe.what()),DIAGERROR);
    } catch (xcept::Exception & err) { //i've got no idea what kind of exception might be thrown
      diagService_->reportError("Caught XDAQ exception while resetting FED "+stringF(fedNumber)+" :" +string(err.what()),DIAGERROR);
    } catch (...) {
      diagService_->reportError("Caught unknown exception while resetting FED "+stringF(fedNumber),DIAGERROR);
    }

  }
  
}

xoap::MessageReference PixelFEDSupervisor::ResetFEDsEnMass (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
        assert(tempCalibObject!=0);

	std::vector<std::pair<unsigned int,std::vector<unsigned int> > > fedsAndChannels=tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_ );
  //                     FED number,               channels

	std::set<unsigned int> resetFEDs;

	for (unsigned int ifed=0; ifed<fedsAndChannels.size(); ++ifed)
	{
		unsigned int fednumber=fedsAndChannels[ifed].first;
		
		// Skip if this FED has already been reset.
		if ( resetFEDs.find(fednumber) != resetFEDs.end() ) continue;
		
		resetFEDs.insert(fednumber);
		unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
		VMEPtr_[vmeBaseAddress]->write("LRES",0x80000000); // Local Reset
		VMEPtr_[vmeBaseAddress]->write("CLRES",0x80000000); // *** MAURO ***
	} // end of loop over FEDs in this crate

	xoap::MessageReference reply=MakeSOAPMessageReference("ResetFEDsEnMassDone");
	return reply;
} 

xoap::MessageReference PixelFEDSupervisor::SetPrivateWord (xoap::MessageReference msg) throw (xoap::exception::Exception)
{

  Attribute_Vector parametersReceived(1);
  parametersReceived[0].name_="PrivateWord";
  Receive(msg, parametersReceived);  

  unsigned int pWord=atoi(parametersReceived[0].value_.c_str());

  FEDInterfaceMap::iterator iFED=FEDInterface_.begin();

  for(;iFED!=FEDInterface_.end();iFED++){
    //cout << "PixelFEDSupervisor::SetPrivateWord:"<<hex<<pWord<<dec<<endl;
    iFED->second->set_PrivateWord(pWord);
  }

  xoap::MessageReference reply=MakeSOAPMessageReference("SetPrivateWordDone");
  return reply;

}

xoap::MessageReference PixelFEDSupervisor::ToggleChannels(xoap::MessageReference msg) throw (xoap::exception::Exception) {
  for (FEDInterfaceMap::iterator iFED = FEDInterface_.begin(); iFED != FEDInterface_.end(); iFED++)
    iFED->second->toggle_chnls_offon();

  xoap::MessageReference reply=MakeSOAPMessageReference("ToggleChannelsDone");
  return reply;
}

xoap::MessageReference PixelFEDSupervisor::SetScopeChannel(xoap::MessageReference msg) throw (xoap::exception::Exception) {
  Attribute_Vector parametersReceived(2);
  parametersReceived[0].name_ = "Which";
  parametersReceived[1].name_ = "Ch";
  Receive(msg, parametersReceived);

  unsigned int ch = atoi(parametersReceived[1].value_.c_str());
  assert(ch <= 8);
  int which = -1;
  if      (parametersReceived[0].value_ == "N")  which = 0;
  else if (parametersReceived[0].value_ == "NC") which = 1;
  else if (parametersReceived[0].value_ == "SC") which = 2;
  else if (parametersReceived[0].value_ == "S")  which = 3;
  else {
    which = atoi(parametersReceived[0].value_.c_str());
    if (which < 0 || which > 3)
      assert(0);
  }

  cout << " PixelFEDSupervisor::SetScopeChannel which = " << parametersReceived[0].value_ << " = " << which << ", channel = " << parametersReceived[1].value_ << " = " << ch << endl;
  for (FEDInterfaceMap::iterator iFED = FEDInterface_.begin(); iFED != FEDInterface_.end(); iFED++)
    iFED->second->set_ScopeChannel(which, ch);

  xoap::MessageReference reply=MakeSOAPMessageReference("SetScopeChannelDone");
  return reply;
}

xoap::MessageReference PixelFEDSupervisor::SetScopeChannels(xoap::MessageReference msg) throw (xoap::exception::Exception) {
  Attribute_Vector parametersReceived(4);
  parametersReceived[0].name_ = "Nch";
  parametersReceived[1].name_ = "NCch";
  parametersReceived[2].name_ = "SCch";
  parametersReceived[3].name_ = "Sch";
  Receive(msg, parametersReceived);

  unsigned int Nch  = atoi(parametersReceived[0].value_.c_str());
  unsigned int NCch = atoi(parametersReceived[1].value_.c_str());
  unsigned int SCch = atoi(parametersReceived[2].value_.c_str());
  unsigned int Sch  = atoi(parametersReceived[3].value_.c_str());

  for (FEDInterfaceMap::iterator iFED = FEDInterface_.begin(); iFED != FEDInterface_.end(); iFED++)
    iFED->second->set_ScopeChannels(Nch, NCch, SCch, Sch);

  xoap::MessageReference reply=MakeSOAPMessageReference("SetScopeChannelsDone");
  return reply;
}

xoap::MessageReference PixelFEDSupervisor::JMTJunk(xoap::MessageReference msg) throw (xoap::exception::Exception) {
  static bool done = false;
  if (!done) {
    for (FEDInterfaceMap::iterator iFED = FEDInterface_.begin(); iFED != FEDInterface_.end(); iFED++)
      iFED->second->set_ROCskip();
    done = true;
  }

  xoap::MessageReference reply=MakeSOAPMessageReference("JMTJunkDone");
  return reply;
}

xoap::MessageReference PixelFEDSupervisor::ArmDigFEDOSDFifo(xoap::MessageReference msg) throw (xoap::exception::Exception) {
  Attribute_Vector parametersReceived(4);
  parametersReceived[0].name_ = "VMEBaseAddress";
  parametersReceived[1].name_ = "Channel";
  parametersReceived[2].name_ = "RocHi";
  parametersReceived[3].name_ = "RocLo";
  Receive(msg, parametersReceived);

  unsigned int VMEBaseAddress = atoi(parametersReceived[0].value_.c_str());
  unsigned int channel = atoi(parametersReceived[1].value_.c_str());
  unsigned int rochi = atoi(parametersReceived[2].value_.c_str());
  unsigned int roclo = atoi(parametersReceived[3].value_.c_str());

  FEDInterface_[VMEBaseAddress]->armDigFEDOSDFifo(channel, rochi, roclo);

  xoap::MessageReference reply=MakeSOAPMessageReference("ArmDigFEDOSDFifoDone");
  return reply;
}

xoap::MessageReference PixelFEDSupervisor::ReadDigFEDOSDFifo(xoap::MessageReference msg) throw (xoap::exception::Exception) {
  Attribute_Vector parametersReceived(2);
  parametersReceived[0].name_ = "VMEBaseAddress";
  parametersReceived[1].name_ = "Channel";
  Receive(msg, parametersReceived);

  unsigned int VMEBaseAddress = atoi(parametersReceived[0].value_.c_str());
  unsigned int channel = atoi(parametersReceived[1].value_.c_str());
  uint32_t data = FEDInterface_[VMEBaseAddress]->readDigFEDOSDFifo(channel);
  std::cout << "ReadDigFEDOSDFifo: RocHi: " << ((data & 0xFFFF0000) >> 16) << " RocLo: " << (data & 0xFFFF) << std::endl;

  xoap::MessageReference reply=MakeSOAPMessageReference("ReadDigFEDOSDFifoDone");
  return reply;
}

xoap::MessageReference PixelFEDSupervisor::SetPhasesDelays (xoap::MessageReference msg) throw (xoap::exception::Exception) {
assert(0);
  xoap::MessageReference reply=MakeSOAPMessageReference("SetPhasesDelaysDone");
  Attribute_Vector parametersReceived(4);
  
  parametersReceived[0].name_="Channel";
  parametersReceived[1].name_="Phase";
  parametersReceived[2].name_="Delay";
  parametersReceived[3].name_="VMEBaseAddress";
  Receive(msg, parametersReceived);
  
  if (FEDInterface_[atoi(parametersReceived[3].value_.c_str())]->setClockDelayAndPhase(atoi(parametersReceived[0].value_.c_str()),	atoi(parametersReceived[2].value_.c_str()), atoi(parametersReceived[1].value_.c_str())) == -1) reply=MakeSOAPMessageReference("SetPhasesDelaysNotDone");
  
  return reply;
}

void PixelFEDSupervisor::callback_TA_CREDIT(toolbox::mem::Reference *ref) throw(i2o::exception::Exception)
  {
#ifdef RUBUILDER
  I2OTACreditReceiver r(ref);
  m_lock.take();
  m_credits += r.getNbCredits();
  m_lock.give();
  diagService_->reportError("Received Credits",DIAGTRACE);
#endif
  }

xoap::MessageReference PixelFEDSupervisor::FEDCalibrations(xoap::MessageReference msg) throw (xoap::exception::Exception) {

  return theFEDCalibrationBase_->execute(msg);

}

xoap::MessageReference PixelFEDSupervisor::beginCalibration(xoap::MessageReference msg) throw (xoap::exception::Exception){

  if (theFEDCalibrationBase_!=0){

    return theFEDCalibrationBase_->beginCalibration(msg);
  }

  return MakeSOAPMessageReference("PixelFEDSupervisor::beginCalibration Default");

}

xoap::MessageReference PixelFEDSupervisor::endCalibration(xoap::MessageReference msg) throw (xoap::exception::Exception){

  if (theFEDCalibrationBase_!=0){  
    return theFEDCalibrationBase_->endCalibration(msg);
  }

  return MakeSOAPMessageReference("PixelFEDSupervisor::endCalibration Default");


}

xoap::MessageReference PixelFEDSupervisor::calibrationEvent(xoap::MessageReference msg) throw (xoap::exception::Exception){

  assert(theFEDCalibrationBase_!=0);
  return theFEDCalibrationBase_->execute(msg);

}



///////////////////////////////////////////////////////////////////////////////////////
void PixelFEDSupervisor::deleteHardware() {

  FEDInterfaceMap::iterator iFED;
  for (iFED=FEDInterface_.begin(); iFED!=FEDInterface_.end(); ++iFED)
  {
    if (iFED->second != 0) { //in case the hardware access throws an exception and then this is run again
      iFED->second->setControlRegister(0x8);
      iFED->second->setModeRegister(0x1);
      delete iFED->second;
      iFED->second=0;
    }
  }
  FEDInterface_.clear();

  for (VMEPointerMap::iterator i=VMEPtr_.begin(); i!=VMEPtr_.end(); ++i)
    {
      delete i->second;
      i->second=0;
    }
  VMEPtr_.clear();
  if(addressTablePtr_!=0) 
    {
      delete addressTablePtr_;
      addressTablePtr_ = (HAL::VMEAddressTable*) 0;
    }
  else 
    {
      diagService_->reportError("PixelFEDSupervisor::deleteHardware() invoked when addressTablePtr_ is null!",DIAGINFO);
    }

  if (busAdapter_!=0) 
    {
      delete busAdapter_; 
      busAdapter_=0;
    }
  else 
    {
      diagService_->reportError("PixelFEDSupervisor::deleteHardware() invoked when busAdapter_ is null!",DIAGINFO);
    }
}


void PixelFEDSupervisor::b2inEvent(toolbox::mem::Reference* msg, xdata::Properties& plist) throw (b2in::nub::exception::Exception){

  std::string action=plist.getProperty("action");
  xdata::UnsignedIntegerT returnedLid=PixelSupervisor_->getLocalId();
  char buffer[50];
  sprintf(buffer,"%d", returnedLid);
  std::string returnedId(buffer);

  plist.setProperty("urn:b2in-protocol:lid",returnedId);
  
  std::map<std::string, std::string, std::less<std::string> >& propertiesMap = plist.getProperties();
  Attribute_Vector attrib;
  
  for(std::map<std::string, std::string, std::less<std::string> >::iterator itr=propertiesMap.begin(), itr_end=propertiesMap.end(); itr!=itr_end; ++itr){
    
    Attribute attribute; 
    attribute.name_=itr->first;
    attribute.value_=itr->second;
    attrib.push_back(attribute);
  }

  std::string receiveMsg;
  
  if(action=="ReadDataAndErrorFIFO"){
    
    xoap::MessageReference soapMsg=this->MakeSOAPMessageReference("ReadDataAndErrorFIFO", attrib);

    receiveMsg = Receive(this->ReadDataAndErrorFIFO(soapMsg));


  }
  else if(action=="FEDCalibrations"){
    
    xoap::MessageReference soapMsg=this->MakeSOAPMessageReference("FEDCalibrations", attrib);

    receiveMsg = Receive(this->FEDCalibrations(soapMsg));
  
    
  }
  else if(action=="BeginCalibration"){
    
    xoap::MessageReference soapMsg=this->MakeSOAPMessageReference("BeginCalibration");
    
    if (theFEDCalibrationBase_!=0){  
      theFEDCalibrationBase_->beginCalibration(soapMsg);
    }

  }
  else if(action=="EndCalibration"){

    xoap::MessageReference soapMsg=this->MakeSOAPMessageReference("EndCalibration");

    if (theFEDCalibrationBase_!=0){  
      theFEDCalibrationBase_->endCalibration(soapMsg);
  }

  }
  else if(action=="ResetFEDsEnMass"){

    xoap::MessageReference soapMsg=this->MakeSOAPMessageReference("ResetFEDsEnMass",attrib);

    receiveMsg = Receive(this->ResetFEDsEnMass(soapMsg));

  }
  else if(action=="JMTJunk"){
    xoap::MessageReference soapMsg=this->MakeSOAPMessageReference("JMTJunk", attrib);
    std::string reciveMsg = Receive(this->JMTJunk(soapMsg));
  }
  else if(action=="ToggleChannels"){
    xoap::MessageReference soapMsg=this->MakeSOAPMessageReference("ToggleChannels", attrib);
    std::string reciveMsg = Receive(this->ToggleChannels(soapMsg));
  }
  else if(action=="SetScopeChannel"){
    xoap::MessageReference soapMsg=this->MakeSOAPMessageReference("SetScopeChannel", attrib);
    std::string reciveMsg = Receive(this->SetScopeChannel(soapMsg));
  }
  else if(action=="SetScopeChannels"){
    xoap::MessageReference soapMsg=this->MakeSOAPMessageReference("SetScopeChannels", attrib);
    std::string reciveMsg = Receive(this->SetScopeChannels(soapMsg));
  }
  else if(action=="SetADC1V2VEnMass"){
assert(0);
    xoap::MessageReference soapMsg=this->MakeSOAPMessageReference("SetADC1V2VEnMass", attrib);
    
    std::string reciveMsg = Receive(this->SetADC1V2VEnMass(soapMsg));
  }
  else if(action=="SetFEDOffsetsEnMass"){
assert(0);
    xoap::MessageReference soapMsg=this->MakeSOAPMessageReference("SetFEDOffsetsEnMass", attrib);
    
    std::string reciveMsg = Receive(this->SetFEDOffsetsEnMass(soapMsg));

  }
  else if(action=="BaselineRelease"){
assert(0);    
    xoap::MessageReference soapMsg=this->MakeSOAPMessageReference("BaselineRelease", attrib);
    
    std::string reciveMsg = Receive(this->BaselineRelease(soapMsg));
  }
  else if(action=="BaselineHold"){
assert(0);
    xoap::MessageReference soapMsg=this->MakeSOAPMessageReference("BaselineHold", attrib);
    
    std::string reciveMsg = Receive(this->BaselineHold(soapMsg));
  }
  
  plist.setProperty("returnValue", receiveMsg);  

  this->sendReply(plist);  
 
}


void PixelFEDSupervisor::closeOutputFiles() {
  diagService_->reportError("-- closing FED output files --",DIAGDEBUG);

  for (FEDInterfaceMap::iterator iFED=FEDInterface_.begin();iFED!=FEDInterface_.end();++iFED) {
    unsigned long vmeBaseAddress=iFED->first;
    if (true==fileopen_[vmeBaseAddress]) { //close output file
      cout<<"Output file is open for vme address "<<vmeBaseAddress<<flush;
      fclose(fout_[vmeBaseAddress]);     // C I/o
      fileopen_[vmeBaseAddress]=false;
      cout<<"...now it is closed"<<endl;
    }
    if (true==errBufferOpen_[vmeBaseAddress]) { //close error file
      cout<<"Error file is open for vme address "<<vmeBaseAddress<<flush;
      fclose(errorFile2_[vmeBaseAddress]);
      errBufferOpen_[vmeBaseAddress]=false;
      cout<<"...now it is closed"<<endl;
    }
  }

  
  fout_.clear();
  fileopen_.clear();
  errorFile2_.clear();
  errBufferOpen_.clear();

  diagService_->reportError("-- done closing FED output files --",DIAGDEBUG);

}

void PixelFEDSupervisor::reportStatistics() {
  // prints end of run summary for monitoring

  if(theNameTranslation_==0) {return;} //protect
  std::map <unsigned int, std::set<unsigned int> > fedsAndChannels=theDetectorConfiguration_->getFEDsAndChannels(theNameTranslation_);
  std::map <unsigned int, std::set<unsigned int> >::iterator i_fedsAndChannels=fedsAndChannels.begin();
  for (;i_fedsAndChannels!=fedsAndChannels.end();++i_fedsAndChannels) {
    unsigned long fednumber=i_fedsAndChannels->first;
    unsigned int fedcrate=theFEDConfiguration_->crateFromFEDNumber(fednumber);
    if (fedcrate!=crate_) {continue;}
    if (!lffMap.empty()) {
      std::cout << "FED="<<fednumber<< " LFF status=" <<100*lffMap[fednumber].mean() <<"%" << std::endl;
      std::cout << "FIFO I Almost Full: N(1-9)="<<100*fifoStatusMap[fednumber][0].mean() << "% NC(10-18)=" <<100*fifoStatusMap[fednumber][2].mean() 
		<< "% SC(19-27)=" <<100*fifoStatusMap[fednumber][4].mean() << "% S(27-36)=" <<100*fifoStatusMap[fednumber][6].mean() << "%" << std::endl;
      std::cout << "FIFO II Nearly Full: N(1-9)="<<100*fifoStatusMap[fednumber][1].mean() << "% NC(10-18)=" <<100*fifoStatusMap[fednumber][3].mean()  
		<< "% SC(19-27)=" <<100*fifoStatusMap[fednumber][5].mean() << "% S(27-36)=" <<100*fifoStatusMap[fednumber][7].mean() << "%" << std::endl;
      std::cout << "FIFO III Almost Full: UP="<<100*fifoStatusMap[fednumber][8].mean() << "% DOWN=" <<100*fifoStatusMap[fednumber][9].mean() << "%" << std::endl;
    }
    if (!errorCountMap.empty() ) {
      std::cout << "Number of errors read=" << errorCountMap[fednumber][0] << std::endl;
    }
    if (!ttsStateChangeCounterMap.empty() ) {
      std::cout << "Number of TTS State changes = " << ttsStateChangeCounterMap[fednumber] << std::endl;
    }
    if (!NttcrxSBError.empty() ) {
      std::cout << "Number of TTCrx resets=" << NttcrxResets[fednumber] << " single-bit errs=" << NttcrxSBError[fednumber] << " # double-bit errs=" << NttcrxDBError[fednumber] 
		<< " SEU = " << NttcrxSEU[fednumber] << std::endl;

    }
    if (!NlockNorth.empty() ) {
      std::cout << " Number of North FPGA PLL locks=" << NlockNorth[fednumber] << std::endl;
    }
    std::cout << std::endl;
  } //fedAndChannels

  return;
}

void PixelFEDSupervisor::DetectSoftError() {
    try {
     if (PixelSupervisor_!=0 && !physicsRunningSentSoftErrorDetected) {
       Attribute_Vector parameters(2);
       parameters[0].name_="Supervisor"; parameters[0].value_="PixelFEDSupervisor";
       parameters[1].name_="Instance";   parameters[1].value_=itoa(crate_);
       Send(PixelSupervisor_, "DetectSoftError", parameters);
       physicsRunningSentSoftErrorDetected = true;
     }
    }
    catch (xcept::Exception & ex) {
      ostringstream err;
      err<<"Failed to send DetectSoftError to PixelSupervisor. Exception: "<<ex.what();
      diagService_->reportError(err.str(),DIAGERROR);
      try {
	toolbox::Event::Reference ev(new toolbox::Event("Failure", this)); //comment this out only for testing
	fsm_.fireEvent(ev);
      } catch (toolbox::fsm::exception::Exception & e2) {
	diagService_->reportError("PixelFEDSupervisor::stateFixingSoftError: Failed to transition to Failed state!",DIAGFATAL);
      }
    }
    return;
}



xoap::MessageReference PixelFEDSupervisor::FixSoftError (xoap::MessageReference msg)
{

  diagService_->reportError("--- FixSoftError ---",DIAGINFO);
  
  // Extract the Global Key from the SOAP message
  // Update the Global Key member data
  // Advertize the Global Key
  // Attribute_Vector parameters(1);
  // parameters[0].name_="GlobalKey";
  // Receive(msg, parameters);
  // if(theGlobalKey_ != 0) delete theGlobalKey_;
  // theGlobalKey_ = new PixelConfigKey(atoi(parameters[0].value_.c_str()));
  if (theGlobalKey_==0) {
    diagService_->reportError("GlobalKey does not exist",DIAGERROR);
    return MakeSOAPMessageReference("FixSoftErrorFailed");
  }

  xoap::MessageReference reply=MakeSOAPMessageReference("FixSoftErrorDone");

  physicsRunningSentSoftErrorDetected = false;


  // That's it! Step to the FixingSoftError state, and
  // relegate all further fixing to the stateFixingSoftError method
  try {
    toolbox::Event::Reference e(new toolbox::Event("FixSoftError", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    *console_<<"[PixelFEDSupervisor::FixSoftError] FixSoftError is an invalid command for the "<<state_.toString()<<" state."<<std::endl;
    diagService_->reportError("[PixelFEDSupervisor::FixSoftError] FixSoftError is an invalid command for the current state."+state_.toString(), DIAGERROR);
    reply=MakeSOAPMessageReference("FixSoftErrorFailed");
  }
  
  diagService_->reportError("--- FixSoftError DONE ---",DIAGINFO);
  diagService_->reportError("PixelFEDSupervisor::FixSoftError: A prompt SOAP reply is sent back before exiting function",DIAGINFO);
  
  return reply;
  
}

void PixelFEDSupervisor::stateFixingSoftError(toolbox::fsm::FiniteStateMachine &fsm) //throw (toolbox::fsm::exception::Exception)
{

  // Update the state_ member data so that Infospace may publish this information
  stateChanged(fsm);

  PixelTimer FixingSoftErrorTimer;
  FixingSoftErrorTimer.start();
  diagService_->reportError("--- FIXINGSOFTERROR ---",DIAGINFO);
  *console_<<"--- FIXINGSOFTERROR ---"<<std::endl;

  try {

      //Code from Pause/Resume with space for any needed function.  For now just a state transition.....
    if ( (theCalibObject_==0) || (theCalibObject_->mode()=="EmulatedPhysics") ) {
        
      phlock_->take(); workloopContinue_=false; phlock_->give();
        workloop_->cancel();
        diagService_->reportError("PixelFEDSupervisor::stateFixingSoftError: Physics data taking workloop cancelled.", DIAGINFO);
        *console_<<"PixelFEDSupervisor::stateFixingSoftError: Physics data taking workloop cancelled."<<std::endl;
        
      }
      
      //Between could go anything for reprogramming....
		
      // Loop through FEDs
      for (std::map<std::pair<unsigned long, unsigned int>, std::set<unsigned int> >::iterator i_vmeBaseAddressAndFEDNumberAndChannels=vmeBaseAddressAndFEDNumberAndChannels_.begin();
	   i_vmeBaseAddressAndFEDNumberAndChannels!=vmeBaseAddressAndFEDNumberAndChannels_.end();
	   ++i_vmeBaseAddressAndFEDNumberAndChannels)
	{
	  unsigned long vmeBaseAddress=i_vmeBaseAddressAndFEDNumberAndChannels->first.first;
	  
	  PixelFEDInterface *iFED=FEDInterface_[vmeBaseAddress];
	  iFED->checkSEUCounters(3);
	  iFED->resetEnbableBits();
	  iFED->resetXYCount();
	  
	}
      
      if ( (theCalibObject_==0) || (theCalibObject_->mode()=="EmulatedPhysics") ) {
        
        phlock_->take(); workloopContinue_=true; physicsRunningSentSoftErrorDetected = false;
        physicsRunningSentRunningDegraded = false;  phlock_->give();
        workloop_->activate();
        diagService_->reportError("PixelFEDSupervisor::ResumeFromSoftError. Physics data taking workloop activated.", DIAGINFO);
        *console_<<"PixelFEDSupervisor::ResumeFromSoftError. Physics data taking workloop activated."<<std::endl;
        
      }  
    
    toolbox::Event::Reference e(new toolbox::Event("FixingSoftErrorDone", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {

    diagService_->reportError("PixelFEDSupervisor::stateFixingSoftError: Detected Error: "+string(e.what()),DIAGERROR);
    try {
      toolbox::Event::Reference ev(new toolbox::Event("Failure", this)); //comment this out only for testing
      fsm_.fireEvent(ev);
    } catch (toolbox::fsm::exception::Exception & e2) {
      diagService_->reportError("PixelFEDSupervisor::stateFixingSoftError: Failed to transition to Failed state!",DIAGFATAL);
    }
    return;

  }
  FixingSoftErrorTimer.stop();
  diagService_->reportError("--- Exit PixelFEDSupervisor::stateFixingSoftError --- "+stringF(FixingSoftErrorTimer.tottime()),DIAGINFO);

}

xoap::MessageReference PixelFEDSupervisor::ResumeFromSoftError (xoap::MessageReference msg)
{
  diagService_->reportError("--- RESUMEFROMSOFTERROR ---",DIAGINFO);
  *console_<<"--- Resuming From Soft Error ---"<<std::endl;

  xoap::MessageReference reply = MakeSOAPMessageReference("ResumeFromSoftErrorDone");

  try {

    toolbox::Event::Reference e(new toolbox::Event("ResumeFromSoftError", this));
    fsm_.fireEvent(e);

  } catch (toolbox::fsm::exception::Exception & e) {

    *console_<<"[PixelFEDSupervisor::ResumeFromSoftError] ResumeFromSoftError is an invalid command for the "<<state_.toString()<<" state."<<std::endl;
    diagService_->reportError("[PixelFEDSupervisor::ResumeFromSoftError] ResumeFromSoftError is an invalid command for the current state."+state_.toString(), DIAGERROR);

    reply = MakeSOAPMessageReference("ResumeFromSoftErrorFailed");


  }

  return reply;
}
