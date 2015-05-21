//--*-C++-*--
// $Id: PixelTKFECSupervisor.cc,v 1.131 2012/10/01 22:07:38 mdunser Exp $
/**************************************************************************
 * XDAQ Components for Pixel Online Software                              *
 * Copyright (C) 2007, Cornell University		                  *
 * All rights reserved.                                                   *
 * Authors: Souvik Das, Anders Ryd, Karl Ecklund			  *
  *************************************************************************/
// Modified version, portcard configuration is moved out to a separate method. d.k. 7/12/2011

#include <fstream>
#include <iostream>
#include <string>
#include <iomanip>        // for test writing of DCU data. Would not be needed in future
#include <ctime>

/* To talk to the TK-FEC, from FecSoftwareV3_0
 */
#include "FecVmeRingDevice.h"

#include <diagbag/DiagBagWizard.h>
#include "DiagCompileOptions.h"
#include <toolbox/convertstring.h>

#include "toolbox/task/Timer.h"
#include "toolbox/task/TimerFactory.h"
#include "toolbox/task/TimerListener.h"
#include "toolbox/TimeInterval.h"

//#include "toolbox/fsm/FiniteStateMachine.h"  //i think this include is not needed!
#include "toolbox/fsm/FailedEvent.h"
#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "APIAccess.h"

#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "PixelTKFECSupervisor/include/PixelTKFECSupervisor.h"
#include "CalibFormats/SiPixelObjects/interface/PixelPortCardSettingNames.h"
#include "PixelCalibrations/include/PixelCalibrationFactory.h"

#include "PixelUtilities/PixelTKFECDataTools/include/PortCardDCU.h"

#include <unistd.h>

using namespace std;
using namespace pos;
using namespace pos::PortCardSettingNames;

//#define USELESS 
//#define READ_DCU  // to enable DCU readout
#define SKIP_LV_CHECK  // to skip the LV check, for testing without power
//#define DO_RESET // do ccu reset at each ccu readout in the workloop
#define PRINT_MORE // print results for each readout in the workloop
#define DO_PORTCARD_RECOVERY  // reprogram portcards at resume and seu recovery
#define DO_READCCU  // readout some CCU registers before the PLL test readout
#define DO_READPIA  // read the PIA ports

#define MYTEST  // special tests 

#define NO_PILOT_RESET // don't send crate/ring resets for pilot blade since this turns off/on modules on BmO

const bool DEBUG = true; // some additinal debuging messages
const bool do_force_ccu_readout = false; // force reading of CCU for each workloop (also force reset if enabled)

static bool errorCounterFlag_ = false ;
static FILE *stdchan_ = stderr ;
//const bool dcudebug_=false;
/*To talk to the TK-FEC, from FecSoftwareV3_0
*/

XDAQ_INSTANTIATOR_IMPL(PixelTKFECSupervisor)

PixelTKFECSupervisor::PixelTKFECSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception) 
  : xdaq::Application(s)
  , SOAPCommander(this)
  , PixelTKFECSupervisorConfiguration(&runNumber_,&outputDir_)
  , executeReconfMethodMutex(toolbox::BSem::FULL)
  , fsm_("urn:toolbox-task-workloop:PixelTKFECSupervisor")
  , debug_(false)
  , readDCU_workloop_usleep_(1000000)
  , extratimers_(true)
  , dculock_(new toolbox::BSem(toolbox::BSem::FULL,true))
  , rclock_(new toolbox::BSem(toolbox::BSem::FULL,true))
  , hardwarelock_(new toolbox::BSem(toolbox::BSem::FULL,true))
  , workloopContinue_(false)
  , workloopContinueRC_(false)
  , suppressHardwareError_(false)
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
                                   "PixelTKFECSupervisor"
                                   );


  DIAG_DECLARE_USER_APP

  diagService_->reportError("The DiagSystem is installed --- this is a bogus error message",DIAGUSERINFO);

  // A SOAP callback used for generic handshaking by retrieving the FSM state
  xoap::bind(this, &PixelTKFECSupervisor::FSMStateRequest, "FSMStateRequest", XDAQ_NS_URI);


  //Binding SOAP Callbacks to State Machine Commmands
  xoap::bind(this, &PixelTKFECSupervisor::Initialize, "Initialize", XDAQ_NS_URI );		            
  xoap::bind(this, &PixelTKFECSupervisor::Configure, "Configure", XDAQ_NS_URI );
  xoap::bind(this, &PixelTKFECSupervisor::Start, "Start", XDAQ_NS_URI );
  xoap::bind(this, &PixelTKFECSupervisor::Stop, "Stop", XDAQ_NS_URI);
  xoap::bind(this, &PixelTKFECSupervisor::Pause, "Pause", XDAQ_NS_URI );
  xoap::bind(this, &PixelTKFECSupervisor::Resume, "Resume", XDAQ_NS_URI );
  xoap::bind(this, &PixelTKFECSupervisor::Halt, "Halt", XDAQ_NS_URI );
  xoap::bind(this, &PixelTKFECSupervisor::Recover, "Recover", XDAQ_NS_URI );

  //special reconfiguration of globaldelay25
  xoap::bind(this, &PixelTKFECSupervisor::Reconfigure, "Reconfigure", XDAQ_NS_URI );

  //Binding SOAP Callback to Low Level Commands
  xoap::bind(this, &PixelTKFECSupervisor::SetDelay, "SetDelay", XDAQ_NS_URI );
  xoap::bind(this, &PixelTKFECSupervisor::SetDelayEnMass, "SetDelayEnMass", XDAQ_NS_URI );
  xoap::bind(this, &PixelTKFECSupervisor::SetAOHGainEnMass, "SetAOHGainEnMass", XDAQ_NS_URI );
  xoap::bind(this, &PixelTKFECSupervisor::SetAOHBiasEnMass, "SetAOHBiasEnMass", XDAQ_NS_URI );
  xoap::bind(this, &PixelTKFECSupervisor::SetAOHBiasOneChannel, "SetAOHBiasOneChannel", XDAQ_NS_URI );
  xoap::bind(this, &PixelTKFECSupervisor::TKFECCalibrations, "Delay25", XDAQ_NS_URI);
  xoap::bind(this, &PixelTKFECSupervisor::readDCU, "readDCU", XDAQ_NS_URI );			            
  xoap::bind(this, &PixelTKFECSupervisor::readDCU_fakeSOAP, "readDCU_fakeSOAP", XDAQ_NS_URI );			            
  xoap::bind(this, &PixelTKFECSupervisor::ReadDCU_workloop_fakeSOAP, "ReadDCU_workloop_fakeSOAP", XDAQ_NS_URI );			            
  xoap::bind(this, &PixelTKFECSupervisor::fsmStateNotification, "fsmStateNotification", XDAQ_NS_URI);

  xoap::bind(this, &PixelTKFECSupervisor::beginCalibration, "BeginCalibration", XDAQ_NS_URI );
  xoap::bind(this, &PixelTKFECSupervisor::endCalibration, "EndCalibration", XDAQ_NS_URI );

  xoap::bind(this, &PixelTKFECSupervisor::ResetCCU, "ResetCCU", XDAQ_NS_URI );			            

  //Soft Error Stuff
  xoap::bind(this, &PixelTKFECSupervisor::FixSoftError, "FixSoftError", XDAQ_NS_URI );
  xoap::bind(this, &PixelTKFECSupervisor::ResumeFromSoftError, "ResumeFromSoftError", XDAQ_NS_URI);

  // Binding XGI Callbacks for messages from the browser
  xgi::bind(this, &PixelTKFECSupervisor::Default, "Default");
  xgi::bind(this, &PixelTKFECSupervisor::XgiHandler, "XgiHandler");
  xgi::bind(this, &PixelTKFECSupervisor::CCUBoardGUI, "CCUBoardGUI");
  xgi::bind(this, &PixelTKFECSupervisor::CCUBoardGUI_XgiHandler, "CCUBoardGUI_XgiHandler");

  //DIAGNOSTIC REQUESTED CALLBACK
  xgi::bind(this,&PixelTKFECSupervisor::configureDiagSystem, "configureDiagSystem");
  xgi::bind(this,&PixelTKFECSupervisor::applyConfigureDiagSystem, "applyConfigureDiagSystem");
  xgi::bind(this,&PixelTKFECSupervisor::callDiagSystemPage, "callDiagSystemPage");

  // Defining the states of the State Machine
  fsm_.addState('I', "Initial" ,this, &PixelTKFECSupervisor::stateChanged);
  fsm_.addState('H', "Halted" ,this, &PixelTKFECSupervisor::stateChanged);
  fsm_.addState('c', "Configuring" ,this, &PixelTKFECSupervisor::stateConfiguring);
  fsm_.addState('C', "Configured" ,this, &PixelTKFECSupervisor::stateConfigured);
  fsm_.addState('R', "Running" ,this, &PixelTKFECSupervisor::stateChanged);
  fsm_.addState('P', "Paused" ,this, &PixelTKFECSupervisor::stateChanged);

  //Adding Soft Error Detection Stuff

  fsm_.addState('s', "FixingSoftError", this, &PixelTKFECSupervisor::stateFixingSoftError);
  fsm_.addState('S', "FixedSoftError", this, &PixelTKFECSupervisor::stateChanged);


  fsm_.setStateName('F',"Error");

  // Defining the transitions of the State Machine
  fsm_.addStateTransition('I', 'H', "Initialize");
  fsm_.addStateTransition('H', 'c', "Configure", this, &PixelTKFECSupervisor::transitionHaltedToConfiguring);
  fsm_.addStateTransition('c', 'c', "Configure");
  fsm_.addStateTransition('c', 'C', "ConfiguringDone");
  fsm_.addStateTransition('C', 'R', "Start");
  fsm_.addStateTransition('R', 'C', "Stop");
  fsm_.addStateTransition('R', 'P', "Pause");
  fsm_.addStateTransition('P', 'R', "Resume");
  fsm_.addStateTransition('C', 'H', "Halt");
  fsm_.addStateTransition('P', 'H', "Halt");
  fsm_.addStateTransition('R', 'H', "Halt");
  fsm_.addStateTransition('F', 'H', "Halt");
  fsm_.addStateTransition('P', 'C', "Stop");
  fsm_.addStateTransition('F', 'I', "Reset"); //this should almost certainly be removed

  //Adding Soft Error Detection Stuff
  
  fsm_.addStateTransition('R', 's', "FixSoftError");
  fsm_.addStateTransition('s', 'S', "FixingSoftErrorDone");
  fsm_.addStateTransition('S', 'R', "ResumeFromSoftError");

  fsm_.addStateTransition('c', 'F', "Failure");
  fsm_.addStateTransition('F', 'F', "Failure");

  //Adding Soft Error Detection Stuff

  fsm_.addStateTransition('s','F',"Failure");
  fsm_.addStateTransition('S','F',"Failure");

  //in case a transition fails
  fsm_.setFailedStateTransitionAction(this, &PixelTKFECSupervisor::enteringError);
  fsm_.setFailedStateTransitionChanged(this, &PixelTKFECSupervisor::stateChanged);

  fsm_.setInitialState('I');
  fsm_.reset();



  // Initialize parameters defined by .xml configuration file

  // WARNING: parameters defined by .xml configuration file
  //	      are not initialized yet in constructor;
  getApplicationInfoSpace()->fireItemAvailable("readDCU_workloop_usleep", &readDCU_workloop_usleep_);

  // Miscellaneous variables most of which I do not understand

  crate_=this->getApplicationDescriptor()->getInstance();

  theGlobalKey_=0;
  theTKFECConfiguration_=0;
  theNameTranslation_=0;
  theFECConfiguration_=0;
  fecAccess_=0;
  thePortcardMap_=0;
  theCalibObject_=0;
  theGlobalDelay25_=0;


  // Initialize the Workloop
  printReadBack_ = false;  //set to true to dump programmed & readback data
  doDCULoop_ = false;  // Use the DCU readout loop
  if(doDCULoop_) {
    workloop_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("PixelTKFECSupervisorWorkLoop", "waiting");
    readDCU_=toolbox::task::bind(this, &PixelTKFECSupervisor::ReadDCU_workloop, "ReadDCU_workloop");
  }

  // Control the reset check workloop 
  doResetCheck_= true;  // Disable the RESET check workloop, Seems to interefer with the clock scan. d.k. 2/4/2012
  if(doResetCheck_) {
    resetCheckWorkloop_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("ResetCheckWorkLoop", "waiting");
    resetCheck_=toolbox::task::bind(this, &PixelTKFECSupervisor::checkForResets, "checkForResets");
  }

  PixelDCStoTKFECDpInterface_=0;
  PixelDCSFSMInterface_=0;
	
  std::stringstream timerName;
  timerName << getApplicationDescriptor()->getContextDescriptor()->getURL() << ":";
  timerName << getApplicationDescriptor()->getClassName() << ":" << getApplicationDescriptor()->getLocalId() << ":" << getApplicationDescriptor()->getInstance();
  toolbox::task::Timer * timer = toolbox::task::getTimerFactory()->createTimer(timerName.str());
  toolbox::TimeInterval interval(AUTO_UP_CONFIGURE_DELAY,0);
  toolbox::TimeVal start;
  start = toolbox::TimeVal::gettimeofday() + interval;
  timer->schedule( this, start,  0, "" );	

  GlobalTimer_.setName("PixelTKFECSupervisor");
  GlobalTimer_.setVerbose(true);  

  // Exporting the FSM state to this application's default InfoSpace
  state_=fsm_.getStateName(fsm_.getCurrentState());
  getApplicationInfoSpace()->fireItemAvailable("stateName", &state_);
	
}

PixelTKFECSupervisor::~PixelTKFECSupervisor()
{
  delete dculock_;
}

//gio
void PixelTKFECSupervisor::timeExpired (toolbox::task::TimerEvent& e)
{
  DIAG_EXEC_FSM_INIT_TRANS
}
    
// DiagSystem XGI Binding
void PixelTKFECSupervisor::callDiagSystemPage(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  diagService_->getDiagSystemHtmlPage(in, out,getApplicationDescriptor()->getURN());
}


void PixelTKFECSupervisor::helpMe ( char *programName ){
        std::cout<<"help"<<std::endl;
}


/** Create the FEC Access class depending on the version of the FecSoftware and the FEC type
 */

void PixelTKFECSupervisor::createFecAccess ( int argc, char **argv, int *cnt, unsigned int slot ) {


        fecAccess_ = FecAccess::createFecAccess ( argc, argv, cnt, false) ;
  
	if (!fecAccess_) {
	  std::cerr << "Creation of FecAccess failed. fecAccess pointer null." << std::endl ; 
	  exit (EXIT_FAILURE) ; ; //FIXME! this is not a proper failure mechanism....
	}
	
	setFecType (fecAccess_->getFecBusType()) ;
	
}
//
//=================================================================================================
// Not clear what flag is for? Reuse it, flag=1 rethrow exception, flag=0 not.
void PixelTKFECSupervisor::portcardI2CDevice(FecAccess *fecAccess,
		       tscType8 fecAddress,
		       tscType8 ringAddress,
		       tscType8 ccuAddress,
		       tscType8 channelAddress,
		       tscType8 deviceAddress,
		       enumDeviceType modeType,
		       unsigned int value,
		       int flag){

  //std::cout << "fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, value:"
  //    <<(int)fecAddress<<" "<<(int)ringAddress<<std::hex<<" "<<(int)ccuAddress<<" "
  //	    <<(int)channelAddress<<" "<<(int)deviceAddress<<std::dec<<" "<<value<<std::endl;


    keyType index = buildCompleteKey(fecAddress,ringAddress,ccuAddress,channelAddress,deviceAddress) ; 
    try {
      fecAccess_->addi2cAccess (index, 
				modeType,
				MODE_SHARE) ;
    }
    catch (FecExceptionHandler e) {
      
      std::cout << "------------ Exception ---------- fecAccess " << std::endl ;
      std::cout << e.what()  << std::endl ;
      std::cout << "---------------------------------" << std::endl ;
      
      if (errorCounterFlag_) displayStatus ( &e, 0, fecAccess_, stdchan_ ) ;

      fecAccess_->removei2cAccess(index); //jmt test this...i think it should be there

      if(flag==1) throw;
    }


  try{
    //	  printf("Value of the device : 0x%x\n",fecAccess->read(index));
    //	  printf("Value to write to the device : ");
    //	  printf("Writing 0x%x to the device...\n",value);
    fecAccess_->write(index, value) ;	
    //	  printf("Check... => new value of the device : 0x%x\n",fecAccess_->read(index));
  }
  catch(FecExceptionHandler e){
    std::cout<<"--------- Exception --------- write "<<std::endl;
    std::cout<< e.what() <<std::endl;
    std::cout<<"-----------------------------"<<std::endl;
    if(errorCounterFlag_)displayStatus(&e,0,fecAccess_,stdchan_);
    // display all registers
    if (e.getFecRingRegisters() != NULL ) FecRingRegisters::displayAllRegisters ( *(e.getFecRingRegisters()) ) ;
    // Original frame
    if (e.getFAck() != NULL) std::cerr << FecRingRegisters::decodeFrame (e.getFAck()) << std::endl ;
    if (e.getDAck() != NULL) std::cerr << FecRingRegisters::decodeFrame (e.getDAck()) << std::endl ;
    if (e.getFAck() != NULL) std::cerr << FecRingRegisters::decodeFrame (e.getFAck()) << std::endl ;

    fecAccess_->removei2cAccess(index); //jmt test this...i think it should be there

    if(flag==1)throw;
  }

  fecAccess_->removei2cAccess(index);

  return;
}
//============================================================================================
int PixelTKFECSupervisor::portcardI2CDeviceRead(FecAccess *fecAccess,
						tscType8 fecAddress,
						tscType8 ringAddress,
						tscType8 ccuAddress,
						tscType8 channelAddress,
						tscType8 deviceAddress,
						enumDeviceType modeType,
						int flag){

  //std::cout << "fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, value:"
  //    <<(int)fecAddress<<" "<<(int)ringAddress<<std::hex<<" "<<(int)ccuAddress<<" "
  //	    <<(int)channelAddress<<" "<<(int)deviceAddress<<std::dec<<" "<<value<<std::endl;

  int value=-1;

  keyType index = buildCompleteKey(fecAddress,ringAddress,ccuAddress,channelAddress,deviceAddress) ; 
  try {
    fecAccess_->addi2cAccess (index, 
			      modeType,
			      MODE_SHARE) ;
  }
  catch (FecExceptionHandler e) {
    
    std::cout << "------------ Exception ----------" << std::endl ;
    std::cout << e.what()  << std::endl ;
    std::cout << "---------------------------------" << std::endl ;
    
    if (errorCounterFlag_) displayStatus ( &e, 0, fecAccess_, stdchan_ ) ;
    
    std::cout << "ERROR IN: fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, value:"
	      <<(int)fecAddress<<" "<<(int)ringAddress<<std::hex<<" "<<(int)ccuAddress<<" "
	      <<(int)channelAddress<<" "<<(int)deviceAddress<<std::dec<<" "<<value<<std::endl;
    fecAccess_->removei2cAccess(index); //to be tested
    
    if(flag==1) throw;
  }
    

  try{
    //	  printf("Value of the device : 0x%x\n",fecAccess->read(index));
    //	  printf("Value to write to the device : ");
    //	  printf("Writing 0x%x to the device...\n",value);
    value=fecAccess_->read(index) ;	
    //	  printf("Check... => new value of the device : 0x%x\n",fecAccess_->read(index));
  }
  catch(FecExceptionHandler e){

    std::cout<<"--------- Exception ---------"<<std::endl;
    std::cout<< e.what() <<std::endl;
    std::cout<<"-----------------------------"<<std::endl;
    if(errorCounterFlag_)displayStatus(&e,0,fecAccess_,stdchan_);

    // display all registers
    if (e.getFecRingRegisters() != NULL ) FecRingRegisters::displayAllRegisters ( *(e.getFecRingRegisters()) ) ;
    // Original frame
    if (e.getFAck() != NULL) std::cerr << FecRingRegisters::decodeFrame (e.getFAck()) << std::endl ;
    if (e.getDAck() != NULL) std::cerr << FecRingRegisters::decodeFrame (e.getDAck()) << std::endl ;
    if (e.getFAck() != NULL) std::cerr << FecRingRegisters::decodeFrame (e.getFAck()) << std::endl ;

    std::cout << "ERROR IN: fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, value:"
	      <<(int)fecAddress<<" "<<(int)ringAddress<<std::hex<<" "<<(int)ccuAddress<<" "
	      <<(int)channelAddress<<" "<<(int)deviceAddress<<std::dec<<" "<<value<<std::endl;

    fecAccess_->removei2cAccess(index); //to be tested
    if(flag==1) throw;
  }

  //  cout<<"CCU I2C CRA = 0x"<<hex<<  int(fecAccess_->geti2cChannelCRA(index))<<dec<<endl; //for testing faulty portcard

  fecAccess_->removei2cAccess(index);

  if (value == -1) XCEPT_RAISE(xdaq::exception::Exception,"Problem in portcardI2CDeviceRead. Value is -1");

  return value;
}

void PixelTKFECSupervisor::Default (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{

  std::string currentState=fsm_.getStateName(fsm_.getCurrentState());
        
  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  xgi::Utils::getPageHeader(*out, "Pixel Tracker FEC Supervisor", fsm_.getStateName(fsm_.getCurrentState()));

  // Rendering the State Machine GUI
  
  std::set<std::string> allInputs=fsm_.getInputs();
  std::set<std::string> clickableInputs=fsm_.getInputs(fsm_.getCurrentState());
  std::set<std::string>::iterator i;

  *out<<"<h2> Finite State Machine </h2>";
  
  std::string urn="/"+getApplicationDescriptor()->getURN();
  *out<<"<form name=\"input\" method=\"get\" action=\""<<urn<<"/XgiHandler"<<"\" enctype=\"multipart/form-data\">";
  
  *out<<"<table border cellpadding=10 cellspacing=0>";
  *out<<"<tr>";
  *out<<"<td> <b>Current State</b> <br/>"<<fsm_.getStateName(fsm_.getCurrentState())<<"</td>";
  *out<<"<td colspan=4>";
        
  // dialog to input value (configuration No.) for GlobalKey
  if (currentState=="Halted") {
    *out<<"Global Key <input type=\"text\" name=\"GlobalKey\"/>"<<std::endl;
  }
        
  *out<<"</td>"<<endl;
  *out<<"</tr>"<<endl;
  *out<<"<tr>"<<endl;

  for (i=allInputs.begin();i!=allInputs.end();i++) {
    *out<<"<td>";
    if (clickableInputs.find(*i)!=clickableInputs.end())
      *out<<"<input type=\"submit\" name=\"Command\" value=\""<<(*i)<<"\"/>";
    else
      *out<<"<input type=\"submit\" disabled=\"true\" name=\"Command\" value=\""<<(*i)<<"\"/>";
    *out<<"</td>"<<endl;
  }
  
  *out<<"</tr>";
  *out<<"</table>";

//=========================================== PIA Reset (only VME Mode Considered)
	*out <<"<hr/>"<<std::endl;
	*out <<"<h2>Pia Reset</h2>"<<std::endl;
	if (theGlobalKey_ != 0) { //need the global key to issue resets
	*out <<"<table border=1><tr><td width=100>"<<std::endl;
	*out <<"  <select name=\"resetParam\">"<<std::endl;
	*out <<"   <option value=\"all\" selected>all"<<std::endl;
	*out <<"   <option value=\"roc\">roc"<<std::endl;
	*out <<"   <option value=\"aoh\">aoh"<<std::endl;
	*out <<"   <option value=\"doh\">doh"<<std::endl;
	*out <<"   <option value=\"res1\">res1"<<std::endl;
	*out <<"   <option value=\"res2\">res2"<<std::endl;
	*out <<"   <option value=\"fpixroc\">fpixroc"<<std::endl; 
	*out <<"   <option value=\"fpixdevice\">fpixdevice"<<std::endl; 
	*out <<"  </select>"<<std::endl;
	*out <<" </td><td width=100>"<<std::endl;
	*out <<"<input type=\"submit\" name=\"Command\" value=\"Pia Reset\">"<<std::endl;
	*out <<"</td></tr></table>"<<std::endl;
	*out<<"</table>"; //should this be here?
	}
	else {
	  *out<<"PIA Reset functions are not available without a global key</br>"<<endl;
	}
//=============================================

  *out<<"</form>"<<endl;
  
  *out<<"<hr/>"<<endl;
  
  // DiagSystem GUI
  std::string urlDiag_ = "/"; \
  urlDiag_ += getApplicationDescriptor()->getURN(); \
  urlDiag_ += "/callDiagSystemPage"; \
  *out << "<h2> Error Dispatcher </h2> "<<std::endl;
  *out << "<a href=" << urlDiag_ << ">Configure DiagSystem</a>" <<std::endl;
  *out << " <hr/> " << std::endl;
  
  // Rendering Low Level GUI
  
  *out<<"<h2>Low Level Commands</h2>"<<std::endl;
  *out<<"<table border=1 bgcolor=gold>"<<std::endl;
  *out<<" Tracker FEC Board"<<std::endl;
  for (unsigned int mfec=1; mfec<8; ++mfec) {
    *out<<" <tr>"<<std::endl;
    *out<<"  <td rowspan=2>mFEC "<<mfec<<"</td>"<<std::endl;
    *out<<" </tr>"<<std::endl;
    *out<<" <tr>"<<std::endl;
    *out<<"  <td bgcolor=aqua><a href=\""<<urn<<"/CCUBoardGUI?mFEC="<<mfec<<"&mFECChannel=A\" target=\"_blank\">A</a></td>"<<std::endl;
    *out<<"  <td bgcolor=aqua><a href=\""<<urn<<"/CCUBoardGUI?mFEC="<<mfec<<"&mFECChannel=B\" target=\"_blank\">B</a></td>"<<std::endl;
    *out<<" </tr>"<<std::endl;
  }
  *out<<"</table>"<<std::endl;
  
  *out<<"</body>"<<std::endl;
  *out<<"</html>"<<std::endl;

}

void PixelTKFECSupervisor::CCUBoardGUI (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{

  cgicc::Cgicc cgi(in);
  // From cgi now you can get information passed down regarding which CCU board this is.
  unsigned int mFEC=atoi(cgi.getElement("mFEC")->getValue().c_str());
  std::string mFECChannel=cgi.getElement("mFECChannel")->getValue();

  // Calculate the FEC Address, Ring Address and CCU Address that points to this CCU Board now.
  // tscType8 FecAddress = ...

  std::string urn="/"+getApplicationDescriptor()->getURN();

  *out<<"<html>"<<std::endl;
  *out<<" <body bgcolor=olive>"<<std::endl;
  *out<<"  <h2 align=center> CCU Board GUI </h2>"<<std::endl;
  *out<<"  <h5 align=center> CCU Board attached to mFEC "<<mFEC<<", mFECChannel "<<mFECChannel<<" </h5>"<<std::endl;
  *out<<"  <hr width=50%>"<<std::endl;
  *out<<"  <p align=center>"<<std::endl;
  *out<<"   Reset DOH 1 and all CCU chips <button onClick=ResetA(10,10,10)>ResetA</button><br/>"<<std::endl;
  *out<<"   Reset DOH 2 and all CCU chips <button onClick=ResetB(10,10,10)>ResetB</button><br/>"<<std::endl;
  *out<<"  </p>"<<std::endl;
  *out<<"  <table border=1 align=center>"<<std::endl;
  *out<<"   <tr>"<<std::endl;
  for (unsigned int ccu=1; ccu<=4; ++ccu) {
    *out<<"     <td bgcolor=slategray>CCU Chip</td>"<<std::endl;
  }
  *out<<"   </tr>"<<std::endl;
  *out<<"  </table>"<<std::endl;
  *out<<" </body>"<<std::endl;
  *out<<"</html>"<<std::endl;

}
  

void PixelTKFECSupervisor::XgiHandler (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{
  cgicc::Cgicc cgi(in);
  
  std::string Command=cgi.getElement("Command")->getValue();
  
  if (Command=="Initialize")  {

    xoap::MessageReference msg = MakeSOAPMessageReference("Initialize");
    xoap::MessageReference reply = Initialize(msg);
    if (Receive(reply)!="InitializeDone") std::cout<<"The TKFEC could not be Initialized!"<<std::endl;

  } else if (Command=="Configure") {

    Attribute_Vector parametersXgi(1);
    parametersXgi.at(0).name_="GlobalKey";	parametersXgi.at(0).value_=cgi.getElement("GlobalKey")->getValue();

    xoap::MessageReference msg = MakeSOAPMessageReference("Configure", parametersXgi);
    xoap::MessageReference reply = Configure(msg);
    if (Receive(reply)!="ConfigureDone") cout<<"The FEC could not be Configured!"<<endl;

  } else if (Command=="Start") {

    xoap::MessageReference msg = MakeSOAPMessageReference("Start");
    xoap::MessageReference reply = Start(msg);
    if (Receive(reply)!="StartDone") cout<<"The FEC could not be Started!"<<endl;

  } else if (Command=="Pause") {

    xoap::MessageReference msg = MakeSOAPMessageReference("Pause");
    xoap::MessageReference reply = Pause(msg);
    if (Receive(reply)!="PauseDone") cout<<"The FEC could not be Paused!"<<endl;

  } else if (Command=="Resume") {

    xoap::MessageReference msg = MakeSOAPMessageReference("Resume");
    xoap::MessageReference reply = Resume(msg);
    if (Receive(reply)!="ResumeDone") cout<<"The FEC could not be Resumed!"<<endl;

  } else if (Command=="Halt") {

    xoap::MessageReference msg = MakeSOAPMessageReference("Halt");
    xoap::MessageReference reply = Halt(msg);
    if (Receive(reply)!="HaltDone") cout<<"The FEC could not be Halted!"<<endl;

  } else if (Command=="AOH") {
    
    Attribute_Vector parametersXgi(3);
    parametersXgi.at(0).name_="AOH";	parametersXgi.at(0).value_=cgi.getElement("AOH")->getValue();
    parametersXgi.at(1).name_="Gain";       parametersXgi.at(1).value_=cgi.getElement("Gain")->getValue();
    parametersXgi.at(2).name_="Bias";       parametersXgi.at(2).value_=cgi.getElement("Bias")->getValue();
    
    xoap::MessageReference msg = MakeSOAPMessageReference("AOH", parametersXgi);
    xoap::MessageReference reply = AOH(msg);
    if (Receive(reply)!="AOHDone") cout<<"AOH command could not be executed!"<<endl;

//=============================================================
  } else if (Command == "Pia Reset") {
    Attribute_Vector parametersXgi(1);
    parametersXgi.at(0).name_="resetParam";	
    parametersXgi.at(0).value_=cgi.getElement("resetParam")->getValue();
    xoap::MessageReference msg = MakeSOAPMessageReference("PIAReset", parametersXgi);
    xoap::MessageReference reply = PIAReset(msg);
    if (Receive(reply)!="PIAResetDone") cout<<"PIAReset command could not be executed!"<<endl;
  }

  this->Default(in, out);

}

void PixelTKFECSupervisor::CCUBoardGUI_XgiHandler (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{

  cgicc::Cgicc cgi(in);

  std::string command=cgi.getElement("Command")->getValue();
  
  if (command=="ResetA") {

    std::string fecAddress_string=cgi.getElement("FECAddress")->getValue();
    std::string ringAddress_string=cgi.getElement("RingAddress")->getValue();
    std::string ccuAddress_string=cgi.getElement("CCUAddress")->getValue();

    // Do a Reset A with this information now
    std::cout<<"PixelTKFECSupervisor::CCUBoardGUI_XgiHandler - We are all the way here now and got a ResetA command!"<<std::endl;
  }

  this->CCUBoardGUI(in, out);

}
  
//---------------------------------------------------------------------------------------------------------------------
xoap::MessageReference PixelTKFECSupervisor::Initialize (xoap::MessageReference msg) //throw (xoap::exception::Exception)
{

  std::cout<<"PixelTKFECSupervisor::Initialize - Entering function"<<std::endl;
  diagService_->reportError("--- INITIALIZE ---",DIAGINFO);

  //needed for delay25 calibration
  try {
     std::set<xdaq::ApplicationDescriptor*> set_PixelFECSupervisors = getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->g\
etApplicationDescriptors("PixelFECSupervisor");
     for (std::set<xdaq::ApplicationDescriptor*>::iterator i_set_PixelFECSupervisor=set_PixelFECSupervisors.begin();
          i_set_PixelFECSupervisor!=set_PixelFECSupervisors.end();
          ++i_set_PixelFECSupervisor) {
        PixelFECSupervisors_.insert(make_pair((*i_set_PixelFECSupervisor)->getInstance(), *(i_set_PixelFECSupervisor)));
     }
     //diagService_->reportError(stringF(PixelFECSupervisors_.size()) + " PixelFECSupervisor(s) found in the \"daq\" group.",DIAGINFO);
     //cout<<PixelFECSupervisors_.size()<<" PixelFECSupervisor(s) found in the \"daq\" group."<<std::endl;

  } catch (xdaq::exception::Exception& e) {
    //diagService_->reportError("No PixelFECSupervisor(s) found in the \"daq\" group.",DIAGWARN);
    std::cout<<"No PixelFECSupervisor(s) found in the \"daq\" group."<<std::endl;
  }
  
  // Detect PixelSupervisor
  try {
    PixelSupervisor_=getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor("PixelSupervisor", 0);
    std::cout<<"PixelTKFECSupervisor::Initialize - Instance 0 of PixelSupervisor found."<<std::endl;
  } catch (xdaq::exception::Exception& e) {
    std::cout<<"PixelTKFECSupervisor::Initialize - Instance 0 of PixelSupervisor not found!"<<std::endl;    
  }

  // Detect PixelDCStoTKFECDpInterface
  try {
    PixelDCStoTKFECDpInterface_ = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor("PixelDCStoTrkFECDpInterface", 0);
    std::cout<<"PixelTKFECSupervisor::Initialize - Instance 0 of PixelDCStoTKFECInterface found."<<std::endl;
  } catch (xdaq::exception::Exception& e) {
    std::cout<<"PixelTKFECSupervisor::Initialize - Instance 0 of PixelDCStoTKFECInterface not found!"<<std::endl;
  }

  // Detect PixelDCSFSMInterface
  try {
    PixelDCSFSMInterface_=getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor("PixelDCSFSMInterface", 0);
    std::cout<<"PixelTKFECSupervisor::Initialize - Instance 0 of PixelDCSFSMInterface found."<<std::endl;
  } catch (xdaq::exception::Exception& e) {
    std::cout<<"PixelTKFECSupervisor::Initialize - Instance 0 of PixelDCSFSMInterface not found."
	     <<"Automatic Detector Startup procedure will not be followed. The detector must be powered on manually."<<std::endl;    
  }
  
  // Query PixelDCSFSMInterface with a SOAP message. Update member data after parsing reply.
  if (PixelDCSFSMInterface_!=0){

    Attribute_Vector parametersToSend(3);
    parametersToSend[0].name_="name"; parametersToSend[0].value_="PixelTKFECSupervisor";
    parametersToSend[1].name_="type"; parametersToSend[1].value_="TrkFEC";
    parametersToSend[2].name_="instance"; parametersToSend[2].value_=itoa(crate_);
    std::cout<<" PixelTKFECSupervisor::Initialize - about to send SOAP to PixelDCSFSMInterface"<<std::endl;
    xoap::MessageReference fsmStateResponse=SendWithSOAPReply(PixelDCSFSMInterface_, "fsmStateRequest", parametersToSend);
    std::cout<<" PixelTKFECSupervisor::Initialize - received SOAP from PixelDCSFSMInterface"<<std::endl;
    fsmStateResponse->writeTo(std::cout);
  
    xoap::SOAPEnvelope envelope=fsmStateResponse->getSOAPPart().getEnvelope();
    xoap::SOAPName notificationName=envelope.createName("fsmStateResponse");
    std::vector<xoap::SOAPElement> notifications=envelope.getBody().getChildElements(notificationName);
    for (unsigned int j=0; j<notifications.size(); ++j) {          
      xoap::SOAPName stateName=envelope.createName("state");
      xoap::SOAPName partitionName=envelope.createName("partition");
      std::vector<xoap::SOAPElement> stateList=notifications.at(j).getChildElements(stateName);
      for (unsigned int i=0; i<stateList.size(); ++i) {
	std::string powerCoordinate=stateList.at(i).getAttributeValue(partitionName);
	std::string fsmState=stateList.at(i).getValue();
	std::cout<<"PixelTKFECSupervisor:Initialize - powerCoordinate = "<<powerCoordinate<<", fsmState = "<<fsmState<<std::endl;
	if (fsmState=="LV_OFF") powerMap_.setVoltage(powerCoordinate, LV_OFF, std::cout);	  
	else if (fsmState=="LV_ON")  powerMap_.setVoltage(powerCoordinate, LV_ON,  std::cout);
	else {
	  std::cout<<"PixelTKFECSupervisor::Initialize - "<<fsmState<<" not recognized!"<<std::endl;
	}
      }
    }

  }

  try
    {
      toolbox::Event::Reference e(new toolbox::Event("Initialize", this));
      fsm_.fireEvent(e);
    }
  catch (toolbox::fsm::exception::Exception & e)
    {
      XCEPT_RETHROW(xoap::exception::Exception, "Invalid Command.", e);
    }

  xoap::MessageReference reply=MakeSOAPMessageReference ("InitializeDone");
  //diagService_->reportError("PixelTKFECSupervisor:: --- Initialising Done ---",DIAGINFO);

  diagService_->reportError("--- INITIALIZATION DONE ---",DIAGINFO);
  return reply;

}

xoap::MessageReference PixelTKFECSupervisor::Configure (xoap::MessageReference msg) //throw (xoap::exception::Exception)
{

  if (extratimers_)  GlobalTimer_.start("entering Configure");

  Attribute_Vector parameters(1);
  parameters.at(0).name_="GlobalKey";
  Receive(msg, parameters);
  std::cout<<"[PixelTKFECSupervisor::Configure] The Global Key was received as "<<parameters.at(0).value_<<std::endl;
  theGlobalKey_ = new PixelConfigKey (atoi(parameters.at(0).value_.c_str()));
  if (theGlobalKey_==0) {
    diagService_->reportError("Failure to create GlobalKey",DIAGERROR);
    return MakeSOAPMessageReference("ConfigureFailed");
  }
  
  xoap::MessageReference reply = MakeSOAPMessageReference("ConfigureDone");
  
  // That's it! Step to the Configuring state, and
  // relegate all further configuring to the stateConfiguring method.
  try {
    toolbox::Event::Reference e(new toolbox::Event("Configure", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    diagService_->reportError("[PixelTKFECSupervisor::Configure] Configure is an invalid command for the current state."+state_.toString(), DIAGERROR);
    reply=MakeSOAPMessageReference("ConfigureFailed");
  }
  
  if (extratimers_) {
    GlobalTimer_.printTime("Configure -- Exit");

    //GlobalTimer_.stop("done with Configure");
    //cout<<"TKFEC configuration took "<<GlobalTimer_.tottime()<<endl;
  }
  return reply;
}

xoap::MessageReference PixelTKFECSupervisor::Start (xoap::MessageReference msg) //throw (xoap::exception::Exception)
{

  diagService_->reportError("--- START",DIAGINFO);
  Attribute_Vector parameter(1);
  parameter[0].name_="RUN_NUMBER";
  Receive(msg, parameter);
  runNumber_=parameter[0].value_;

  setupOutputDir();

  diagService_->reportError("Start Run "+runNumber_,DIAGINFO);
  suppressHardwareError_=false;

  // Start workloops
  if (theCalibObject_==0) {

    if(doDCULoop_) {
      dculock_->take(); workloopContinue_=true; dculock_->give();
      workloop_->activate();
      std::cout<<"PixelTKFECSupervisor::Start - Activated workloop."<<std::endl;
    }


    if (doResetCheck_) {
      //try {
 	rclock_->take(); workloopContinueRC_=true; rclock_->give();
 	//resetCheckWorkloop_->activate();
	if ( !resetCheckWorkloop_->isActive() ) resetCheckWorkloop_->activate();
 	std::cout<<"PixelTKFECSupervisor::START:  Activated resetCheck workloop."<<std::endl;
	//} catch (xcept::Exception & e) {
 	//diagService_->reportError("PixelTKFECSupervisor::START: Caught exception activating reset check workloop: "+string(e.what()),DIAGERROR);
	//} // try
    }  // if doResetCheck

  }

  try {
    toolbox::Event::Reference e(new toolbox::Event("Start", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    XCEPT_RETHROW(xoap::exception::Exception, "Invalid State Machine Input.", e);
  }
  
  xoap::MessageReference reply = MakeSOAPMessageReference("StartDone");
  return reply;

}

xoap::MessageReference PixelTKFECSupervisor::Stop (xoap::MessageReference msg) //throw (xoap::exception::Exception)
{
  diagService_->reportError("-- STOP --",DIAGDEBUG);

  // Stop workloops
  if (theCalibObject_==0) {

    if(doDCULoop_) {

      if (fsm_.getStateName(fsm_.getCurrentState())=="Running") {
	dculock_->take(); workloopContinue_=false; dculock_->give();
	if( workloop_->isActive() ) workloop_->cancel();
	std::cout<<"PixelTKFECSupervisor::Stop - Cancelled workloop."<<std::endl;
      } else if (fsm_.getStateName(fsm_.getCurrentState())=="Paused") {
	std::cout<<"PixelTKFECSupervisor::Stop - Nothing to do."<<std::endl;
      }
    }

     if (doResetCheck_) {
       //       try {
       rclock_->take(); workloopContinueRC_=false; rclock_->give();
       if (resetCheckWorkloop_->isActive()) resetCheckWorkloop_->cancel();
       std::cout<<"PixelTKFECSupervisor::Stop - Cancelled resetCheck workloop."<<std::endl;
	 // 	  diagService_->reportError("PixelTKFECSupervisor::STOP:  Cancelled resetCheck workloop",DIAGTRACE);
	 // 	}
	 //       } catch (xcept::Exception & e) {
	 // 	diagService_->reportError("PixelTKFECSupervisor::STOP: Caught exception canceling reset check workloop: "+string(e.what()),DIAGERROR);
	 //       }
     } // if doResetCheck
     
  }

  try {
    toolbox::Event::Reference e(new toolbox::Event("Stop", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    XCEPT_RETHROW(xoap::exception::Exception, "Invalid State Machine Input.", e);
  }
    
  xoap::MessageReference reply = MakeSOAPMessageReference("StopDone");
  return reply;

}

xoap::MessageReference PixelTKFECSupervisor::Pause (xoap::MessageReference msg) //throw (xoap::exception::Exception)
{
  PixelTimer pausetimer;
  pausetimer.start();

  if (theCalibObject_==0) {

    if(doDCULoop_) {

      //    if (dcudebug_) diagService_->reportError("[Pause] about to set DCU workloopContinue_ to false",DIAGTRACE);
      dculock_->take(); workloopContinue_=false; dculock_->give();
      //    if (dcudebug_) diagService_->reportError("[Pause] about to cancel DCU workloop",DIAGTRACE);
      if( workloop_->isActive() ) workloop_->cancel();
      diagService_->reportError("[Pause] Cancelled DCU workloop",DIAGTRACE);
    }

    if (doResetCheck_) {
      //try {
      rclock_->take(); workloopContinueRC_=false; rclock_->give();
      if (resetCheckWorkloop_->isActive()) resetCheckWorkloop_->cancel();
      cout<<"PixelTKFECSupervisor::PAUSE:  Cancelled resetCheck workloop"<<endl;
      //diagService_->reportError("PixelTKFECSupervisor::PAUSE:  Cancelled resetCheck workloop",DIAGTRACE);      
      //} catch (xcept::Exception & e) {
      //diagService_->reportError("PixelTKFECSupervisor::PAUSE: Caught exception canceling reset check workloop: "+string(e.what()),DIAGERROR);
      //}
    } // if doResetCheck
 
 } // if calibObject


  try {
    toolbox::Event::Reference e(new toolbox::Event("Pause", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    XCEPT_RETHROW(xoap::exception::Exception, "Invalid Command.", e);
  }

  pausetimer.stop();
  diagService_->reportError("-- Exit PAUSE -- "+stringF(pausetimer.tottime()),DIAGDEBUG);
  xoap::MessageReference reply = MakeSOAPMessageReference("PauseDone");
  return reply;

}

xoap::MessageReference PixelTKFECSupervisor::Resume (xoap::MessageReference msg) //throw (xoap::exception::Exception) 
{
  cout<<" Enter RESUME "<<endl;

  // First try to reprogram portcards (added in 01/12) 
  // Do we need a try/catch here?
#ifdef DO_PORTCARD_RECOVERY
  try  {
    // do reporgamming
    cout<<" RESUME: Will reprogram portcrads "<<endl; 
    bool status = programPortcards(false);  
    if(status) diagService_->reportError("Resume - Error in portcard programming : status = ",DIAGERROR);
  } catch (...)  { //exceptions might be thrown by startupHVCheck
    diagService_->reportError("Resume - Error in portcard programming ",DIAGERROR);
    //reply = MakeSOAPMessageReference("ResumeFailed");
    //diagService_->reportError("Failed to Resume run with exception: "+string(e.what()),DIAGERROR);
    //FIXME maybe we should transition to the Error state in case this happens?
  } 
#endif    
  suppressHardwareError_=false;     

  if (theCalibObject_==0) {

    if(doDCULoop_) {

      dculock_->take(); workloopContinue_=true; dculock_->give();
      workloop_->activate();
      std::cout<<"PixelTKFECSupervisor::Resume - Activated workloop."<<std::endl;
    }

    if (doResetCheck_) {
      //try {
      rclock_->take(); workloopContinueRC_=true; rclock_->give();
      if ( !resetCheckWorkloop_->isActive() ) resetCheckWorkloop_->activate();
      std::cout<<"PixelTKFECSupervisor::RESUME:  Activated resetCheck workloop."<<std::endl;
      //} catch (xcept::Exception & e) {
      //diagService_->reportError("PixelTKFECSupervisor::RESUME: Caught exception activating reset check workloop: "+string(e.what()),DIAGERROR);
      //} // try
    }  // if doResetCheck

  } // if calibObject

  try {
    toolbox::Event::Reference e(new toolbox::Event("Resume", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    XCEPT_RETHROW(xoap::exception::Exception, "Invalid Command.", e);
  }

  xoap::MessageReference reply = MakeSOAPMessageReference("ResumeDone");
  return reply;

}

xoap::MessageReference PixelTKFECSupervisor::Halt (xoap::MessageReference msg) //throw (xoap::exception::Exception)
{
  diagService_->reportError("-- HALT --",DIAGDEBUG);

  if (theCalibObject_==0) { // do only for global runs

    //cancel the reset check workloop
    if (doResetCheck_) {
      //try {
      rclock_->take(); workloopContinueRC_=false; rclock_->give();
      //diagService_->reportError("Canceling reset check workloop",DIAGDEBUG);
      if (resetCheckWorkloop_->isActive()) resetCheckWorkloop_->cancel();
      resetCheckWorkloop_->remove(resetCheck_);
      //} catch (xcept::Exception & e) {
      //diagService_->reportError("Caught exception canceling reset check workloop: "+string(e.what()),DIAGDEBUG);
      //}
      //diagService_->reportError("Reset check workloop canceled",DIAGDEBUG);
      cout<<"Reset check workloop canceled"<<endl;
    }  // doResetCheck

    
    if(doDCULoop_) {
      if (fsm_.getStateName(fsm_.getCurrentState())=="Configured") {
	workloop_->remove(readDCU_);
	std::cout<<"PixelTKFECSupervisor::Halt - Removed readDCU_ job from workloop."<<std::endl;
      } else if (fsm_.getStateName(fsm_.getCurrentState())=="Running") {
	dculock_->take(); workloopContinue_=false; dculock_->give();
	workloop_->cancel();
	workloop_->remove(readDCU_);
	std::cout<<"PixelTKFECSupervisor::Halt - Cancelled workloop and removed readDCU_ job from it."<<std::endl;
      } else if (fsm_.getStateName(fsm_.getCurrentState())=="Paused") {
	workloop_->remove(readDCU_);
	std::cout<<"PixelTKFECSupervisor::Halt - Removed readDCU_ job from workloop."<<std::endl;
      }
    }

  } // theCalibObject

  /*
    Delete and clear all your objects here.
  */

  cleanupGlobalConfigData(); 

  try {
    toolbox::Event::Reference e(new toolbox::Event("Halt", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    XCEPT_RETHROW(xoap::exception::Exception, "Invalid Command.", e);
  }
  
  xoap::MessageReference reply = MakeSOAPMessageReference("HaltDone");
  return reply;

}

xoap::MessageReference PixelTKFECSupervisor::Recover (xoap::MessageReference msg) 
{
  diagService_->reportError("-- Enter RECOVER --", DIAGINFO);

  if (state_!="Error") return MakeSOAPMessageReference("RecoverFailed"); //sanity

  //if there is a failure, the workloop may or may not be active
  if (theCalibObject_==0) {

    if(doDCULoop_) {
      
      if (workloop_!=0) {
	if (workloop_->isActive()) {
	  dculock_->take(); workloopContinue_=false; dculock_->give();
	  workloop_->cancel(); //this really ought to be exception safe
	}
	try {
	  workloop_->remove(readDCU_);
	}
	catch (xcept::Exception & e) { //will happen if the task was never submitted
	  diagService_->reportError("Failed to remove DCU workloop: "+string(e.what()), DIAGDEBUG);
	}
      }
    }


    //cancel the reset check workloop
    if (doResetCheck_) {
      try {
	rclock_->take(); workloopContinueRC_=false; rclock_->give();
	if (resetCheckWorkloop_->isActive()) resetCheckWorkloop_->cancel();
	resetCheckWorkloop_->remove(resetCheck_);
      } catch (xcept::Exception & e) {
	diagService_->reportError("Caught exception canceling or removing reset check workloop: "+string(e.what()),DIAGDEBUG);
      }
    }
  }

  //we should cleanup the fecAccess_ here, in case creating it was the source of failure
  delete fecAccess_;
  fecAccess_=0;

  cleanupGlobalConfigData(); 

  xoap::MessageReference reply = MakeSOAPMessageReference("RecoverDone");
  try {
    toolbox::Event::Reference e(new toolbox::Event("Halt", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    diagService_->reportError("Failed to transition to Halted state! message: "+string(e.what()), DIAGERROR);
    reply = MakeSOAPMessageReference("RecoverFailed");
  }
  
  diagService_->reportError("-- Exit RECOVER --", DIAGINFO);
  return reply;
  
}

void PixelTKFECSupervisor::cleanupGlobalConfigData() {

  //normally called from ::Halt
  //Everything here should be safe to run at any time

  ///////////////////////////////////////////

  //Will not delete the fecAccess!
  //delete fecAccess_;
  //fecAccess_=0;

  map<string, PixelPortCardConfig*>::iterator iter=
    mapNamePortCard_.begin();

  for (;iter!=mapNamePortCard_.end();++iter){
    delete iter->second;
  }
  
  mapNamePortCard_.clear();
  

  delete thePortcardMap_; thePortcardMap_=0;
  delete theGlobalKey_; theGlobalKey_=0;
  delete theTKFECConfiguration_; theTKFECConfiguration_=0;
  delete theCalibObject_; theCalibObject_=0;
  delete theNameTranslation_; theNameTranslation_=0;
  delete theFECConfiguration_; theFECConfiguration_=0;
  delete theGlobalDelay25_; theGlobalDelay25_=0;

}

xoap::MessageReference PixelTKFECSupervisor::Reconfigure (xoap::MessageReference msg) {

  diagService_->reportError("-- Enter Reconfigure --",DIAGINFO);
  xoap::MessageReference reply = MakeSOAPMessageReference("ReconfigureDone");

  //cancel the reset check workloop
  if (theCalibObject_==0) {

    if (doResetCheck_) {
      //try {
      rclock_->take(); workloopContinueRC_=false; rclock_->give();
      if (resetCheckWorkloop_->isActive()) resetCheckWorkloop_->cancel();
      //} catch (xcept::Exception & e) {
      //diagService_->reportError("Caught exception canceling reset check workloop: "+string(e.what()),DIAGERROR);
      //}
    } // doresetcheck

    if(doDCULoop_) {      
      //    if (dcudebug_) diagService_->reportError("[Reconfigure] about to set DCU workloopContinue_ to false",DIAGTRACE);
      dculock_->take(); workloopContinue_=false; dculock_->give();
      //    if (dcudebug_) diagService_->reportError("[Reconfigure] about to cancel DCU workloop",DIAGTRACE);
      // Add IF ACTIVE
      workloop_->cancel();
      cout<<"PixelTKFECSupervisor::Reconfigure:  Cancelled DCU workloop"<<endl;
    }
    
  } // if global



  try {
    
    Attribute_Vector parameters(1);
    parameters.at(0).name_="GlobalKey";
    Receive(msg, parameters);
    diagService_->reportError("Will reconfigure TKFEC with global key = "+parameters.at(0).value_,DIAGDEBUG);
    PixelConfigKey* newGlobalKey = new PixelConfigKey (atoi(parameters.at(0).value_.c_str()));
    if (newGlobalKey==0) {
      diagService_->reportError("Failure to create GlobalKey",DIAGERROR);
      return MakeSOAPMessageReference("ReconfigureFailed");
    }
    
    PixelGlobalDelay25* newGlobalDelay25=0;
    PixelConfigInterface::get(newGlobalDelay25, "pixel/globaldelay25/", *newGlobalKey);
    if (newGlobalDelay25==0) XCEPT_RAISE(xdaq::exception::Exception,"The globalDelay25 object is null!");
    
    enumDeviceType modeType = PHILIPS ;
    //loop over portcards
    map<string, PixelPortCardConfig*>::const_iterator iportcard=mapNamePortCard_.begin();
    for ( ; iportcard != mapNamePortCard_.end() ; ++iportcard) {
      
      const string TKFECID = iportcard->second->getTKFECID();
      
      //since we've already built mapNamePortCard to only have
      //the portcards on this tkfec, then this better not fail!
      assert ( theTKFECConfiguration_->crateFromTKFECID(TKFECID) == crate_ );
      const unsigned int TKFECAddress = theTKFECConfiguration_->addressFromTKFECID(TKFECID);
      
      //need to set SCL, TRG, SDA
      vector<pair<unsigned int, unsigned int> > toProgram;
      unsigned int deviceAddress = iportcard->second->getdeviceAddressForSetting(PortCardSettingNames::k_Delay25_SCL);
      unsigned int value = iportcard->second->getdeviceValuesForSetting(PortCardSettingNames::k_Delay25_SCL);
      value = newGlobalDelay25->getDelay(value);
      toProgram.push_back(make_pair(deviceAddress,value));
      
      deviceAddress = iportcard->second->getdeviceAddressForSetting(PortCardSettingNames::k_Delay25_TRG);
      value = iportcard->second->getdeviceValuesForSetting(PortCardSettingNames::k_Delay25_TRG);
      value = newGlobalDelay25->getDelay(value);
      toProgram.push_back(make_pair(deviceAddress,value));
      
      deviceAddress = iportcard->second->getdeviceAddressForSetting(PortCardSettingNames::k_Delay25_SDA);
      value = iportcard->second->getdeviceValuesForSetting(PortCardSettingNames::k_Delay25_SDA);
      value=newGlobalDelay25->getCyclicDelay(value);
      toProgram.push_back(make_pair(deviceAddress,value));
      
      for ( vector<pair<unsigned int, unsigned int> >::const_iterator i=toProgram.begin() ;
	    i != toProgram.end() ; ++i) {
	portcardI2CDevice(fecAccess_, 
			  TKFECAddress, 
			  iportcard->second->getringAddress(), 
			  iportcard->second->getccuAddress(), 
			  iportcard->second->getchannelAddress(), 
			  i->first, 
			  modeType, 
			  i->second, 
			  1); //as far as i can tell, this value is not used (!)
	
	unsigned int readBack = portcardI2CDeviceRead(fecAccess_, 
						      TKFECAddress, 
						      iportcard->second->getringAddress(), 
						      iportcard->second->getccuAddress(), 
						      iportcard->second->getchannelAddress(), 
						      i->first,
						      modeType, 
						      1); //again, not used
	if ((i->second)!=readBack) {
	  diagService_->reportError("Programming Error in Reconfigure! Read back != value",DIAGERROR);
	  cout << "ccu address=0x" << std::hex << iportcard->second->getccuAddress() 
	       << " chan=0x" << iportcard->second->getchannelAddress() << std::dec ;
	  cout << " deviceAddress = 0x" << std::hex << i->first << ", value = 0x" << i->second << std::dec << ", flag = " << 1 << "\n";
	  cout << "read back = 0x " << hex << readBack << dec << endl;
	}
	
      }
    }
    //if we get this far, then we've successfully reprogrammed the hardware
    delete theGlobalKey_;
    delete theGlobalDelay25_;
    theGlobalKey_=newGlobalKey;
    theGlobalDelay25_=newGlobalDelay25;
  }
  catch (exception & e) {
    diagService_->reportError("Reconfigure failed with exception: "+string(e.what()),DIAGERROR);
    reply = MakeSOAPMessageReference("ReconfigureFailed");
  }

  //here we want to restart the reset check workloop
  // SKIP THIS will be done at RESUME
  if (theCalibObject_==0) {
    if (doResetCheck_) {
      rclock_->take(); workloopContinueRC_=true; rclock_->give();
      //try { //no reason to think this will fail, but can't be too careful
      if ( !resetCheckWorkloop_->isActive() ) resetCheckWorkloop_->activate();
      //}
      //catch (xcept::Exception & e) {
      //diagService_->reportError("Exception restarting the reset check workloop: "+string(e.what()),DIAGERROR);
      //}
    }
    if(doDCULoop_) {
      dculock_->take(); workloopContinue_=true; dculock_->give();
      workloop_->activate();
      std::cout<<"PixelTKFECSupervisor::Reconfigure:  Activated DCU workloop."<<std::endl;
    }
 }

  diagService_->reportError("-- Exit Reconfigure --",DIAGINFO);
  return reply;
}

//////////////////////////////////////////////////////////////////////
//////////////////////// FSM State Entry Actions /////////////////////
//////////////////////////////////////////////////////////////////////

void PixelTKFECSupervisor::stateChanged(toolbox::fsm::FiniteStateMachine &fsm) //throw (toolbox::fsm::exception::Exception)
{
 try {
  state_=fsm.getStateName(fsm.getCurrentState());
  if (PixelSupervisor_!=0) {
    Attribute_Vector parameters(3);
    parameters[0].name_="Supervisor"; parameters[0].value_="PixelTKFECSupervisor";
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

void PixelTKFECSupervisor::stateConfigured(toolbox::fsm::FiniteStateMachine &fsm) //throw (toolbox::fsm::exception::Exception)
{

  if (extratimers_)     GlobalTimer_.printTime("stateConfigured -- Enter");

  //test for theCalibObject in order to decide if we are in physics mode
  if (state_=="Configuring" && theCalibObject_==0) { 

    if (doResetCheck_ ) { 
      //activate the reset check workloop
      rclock_->take(); workloopContinueRC_=true; rclock_->give();
      //try {
      resetCheckWorkloop_->submit(resetCheck_);
      //resetCheckWorkloop_->activate();  // Do not strat it yet
      diagService_->reportError("Activated reset check workloop",DIAGDEBUG);
      //}
      //catch (xcept::Exception & e) {
      //diagService_->reportError("Problem activating resetCheck workloop: "+string(e.what()),DIAGWARN);
      //}
    }

    if(doDCULoop_) {
      dculock_->take(); workloopContinue_=true; dculock_->give();
      workloop_->submit(readDCU_);
      std::cout<<"PixelTKFECSupervisor::Configure - Submitted readDCU_ job to the workloop."<<std::endl;
    }

  }

  stateChanged(fsm);

  if (extratimers_) {
    GlobalTimer_.stop("In Configured: done with Configure");
    cout<<"TKFEC configuration took "<<GlobalTimer_.tottime()<<endl;
  }

}
//==========================================================================================================================
bool PixelTKFECSupervisor::pixDCDCCommand(tscType8 fecAddress,
					  tscType8 ringAddress,
					  tscType8 ccuAddressEnable,
					  tscType8 ccuAddressPgood,
					  tscType8 piaChannelAddress,
					  bool turnOn,
					  unsigned int portNumber) {

  printf("Doing pixDCDCCommand slot %i ring %i ccuAddrEnable %x ccuAddressPgood %x piaChannelAddr %x portNumber %i turnOn? %i \n", fecAddress, ringAddress, ccuAddressEnable, ccuAddressPgood, piaChannelAddress, portNumber, turnOn);
  usleep(500000);

  bool success = true;
  
  keyType enableKey = buildCompleteKey(fecAddress, ringAddress, ccuAddressEnable, piaChannelAddress, 0);
  keyType pgoodKey  = buildCompleteKey(fecAddress, ringAddress, ccuAddressPgood,  piaChannelAddress, 0);

  try {
    fecAccess_->addPiaAccess(enableKey, MODE_SHARE); // JMTBAD use PiaChannelAccess
    fecAccess_->addPiaAccess(pgoodKey,  MODE_SHARE);

    unsigned int bits    = 0x3 << (portNumber * 2);
    unsigned int invBits = 0xFF ^ bits;

    // Set just the two pins we want to input for pgood.
    usleep(10000);
    fecAccess_->setPiaChannelDDR(pgoodKey, invBits & fecAccess_->getPiaChannelDDR(pgoodKey));

    // Read the pgood bit to check state before doing anything.
    usleep(10000);
    unsigned int initPgoodVal = fecAccess_->getPiaChannelDataReg(pgoodKey);
    bool initPgood = ((initPgoodVal >> (portNumber * 2)) & 0x3) == 0x3;
    cout << "Initial pgoodVal = 0x" << std::hex << initPgoodVal << " = " << (initPgood ? "PGOOD" : "NOT PGOOD") << endl;
    if (turnOn + initPgood != 1) {
      cout << " but asked to turn " << (turnOn ? "ON" : "OFF") << " ; bailing out!!!" << endl;
      success = false;
    }
    else {
      // Set just the two pins we want to output for enable;
      usleep(10000);
      fecAccess_->setPiaChannelDDR(enableKey, bits | fecAccess_->getPiaChannelDDR(enableKey));

      // and set the inverted bits in the data reg.
      usleep(10000);
      unsigned int initEnableVal = fecAccess_->getPiaChannelDataReg(enableKey); // JMTBAD the two lines below ere using the pgood values???

      usleep(10000);
      if (turnOn)
	fecAccess_->setPiaChannelDataReg(enableKey, invBits & initEnableVal);
      else
	fecAccess_->setPiaChannelDataReg(enableKey, bits    | initEnableVal);
    
      // Sleep 5 ms before reading back the pgood bit.
      usleep(50000);

      // Read back the pgood bit and report status. 
      unsigned pgoodVal = fecAccess_->getPiaChannelDataReg(pgoodKey);
      bool pgood = ((pgoodVal >> (portNumber * 2)) & 0x3) == 0x3;
      cout << "pgoodVal = 0x" << std::hex << pgoodVal << " = " << (pgood ? "PGOOD!" : "NOT PGOOD") << endl;
      if (turnOn + pgood == 1) {
	cout << " but turning " << (turnOn ? "ON" : "OFF") << " ; problem!!!" << endl;
	success = false;
      }
    }

    fecAccess_->removePiaAccess(enableKey);
    fecAccess_->removePiaAccess(pgoodKey);
  }
  catch (FecExceptionHandler e) {
    cout << std::string("Exception caught when doing PIA access: ") + e.what();
    success = false;
  }
    
  return success;
}

void PixelTKFECSupervisor::stateConfiguring(toolbox::fsm::FiniteStateMachine &fsm) //throw (toolbox::fsm::exception::Exception) 
{

  //we can't freely raise exceptions here. They will not be caught externally
  if (extratimers_)     GlobalTimer_.printTime("stateConfiguring -- Enter");

  // Update the state_ member data so that Infospace may publish this information
  stateChanged(fsm); //eliminate redundant code

  bool isfailure=false;
  PixelDetectorConfig* detconfig=0; 

  // Big try-catch loop here 
  try { //load configuration data and access hardware

    bool proceed=true;
    PixelConfigInterface::get(detconfig, "pixel/detconfig/", *theGlobalKey_); 
    if (detconfig==0) XCEPT_RAISE(xdaq::exception::Exception, "Failed to load detconfig!");
    
    //Use the presence of PixelDCSFSMInterface to trigger the automatic detector startup  
    if (PixelDCSFSMInterface_!=0) {
      
      diagService_->reportError("[PixelTKFECSupervisor::stateConfiguring] PixelDCSFSMInterface detected. Automatic detector startup.",DIAGDEBUG);      
      const std::set<std::string>& portcards=thePortcardMap_->portcards(detconfig);
      std::cout << "Number of portcards to configure:"<<portcards.size()<< std::endl;  
      std::set<std::string>::const_iterator iportcard=portcards.begin();
    
      for (iportcard=portcards.begin();iportcard!=portcards.end();iportcard++) {
      
	std::string powerCoordinate=iportcard->substr(0, 8);
	//      std::cout<<"PixelTKFECSupervisor::stateConfiguring - Portcard "<<*iportcard<<" has power coordinate "<<powerCoordinate<<std::endl;
	BiVoltage power=powerMap_.getVoltage(powerCoordinate, std::cout);
	
	if (power==LV_OFF) {
	  std::cout<<"[PixelTKFECSupervisor::stateConfiguring] The voltage for "<<powerCoordinate<<" is OFF!"<<std::endl;
	  proceed=false;
	}
	
	if (power==LV_ON) {
	  std::cout<<"[PixelTKFECSupervisor::stateConfiguring] The voltage for "<<powerCoordinate<<" is ON."<<std::endl;
	}
	
      }
      
    }
    
    diagService_->reportError("stateConfiguring -- configure portcards",DIAGTRACE);

    if (extratimers_)     GlobalTimer_.printTime("stateConfiguring -- Power check finished");
    
    if (proceed) {
      
      //const long loop = 1 ;
      //const unsigned long tms  = 0 ;  // wait tms microseconds
      string fecAccessType = "unset";
      bool fack=true;
      
      const std::set<std::string>& portcards=thePortcardMap_->portcards(detconfig);
      unsigned int slot=9999; 
      unsigned int ring =9999;
      static bool ringInit[8] = {false,false,false,false,false,false,false,false}; // has the ring been reset
      int np=0;
      
      //this first loop loads the portcard information from the database
      //it also creates the fecAccess and resets the control rings
      map<unsigned int, set<pair<unsigned int,bool> > > ccuRingMap;
      set<std::string>::const_iterator iiportcard=portcards.begin(); 
      
      for ( ;iiportcard!=portcards.end();iiportcard++) {      // Cycle over portcards
	
	cout<<"config portcrad "<<*iiportcard<<endl;
	
	PixelPortCardConfig* tempPortCard=0;
	PixelConfigInterface::get(tempPortCard,"pixel/portcard/"+*iiportcard, *theGlobalKey_);
	if (extratimers_)     GlobalTimer_.printTime("stateConfiguring -- after get");
	
	if (tempPortCard==0) XCEPT_RAISE(xdaq::exception::Exception, "Failed to load portcard!");
	
	const std::string TKFECID = tempPortCard->getTKFECID();
	if ( theTKFECConfiguration_->crateFromTKFECID(TKFECID) != crate_ ) continue;
	
	
	slot = theTKFECConfiguration_->addressFromTKFECID(TKFECID);
	ring = tempPortCard->getringAddress();
	
	mapNamePortCard_[*iiportcard]=tempPortCard;
	
	// Init VME Interface, DO ONLY ONCE 
	np++;
	const std::string type = theTKFECConfiguration_->typeFromTKFECID(TKFECID);
	assert( type == fecAccessType || fecAccessType == "unset" );
	//creates the fecAccess object
	if (np==1) {
	  
	  cout << "[PixelTKFECSupervisor::stateConfiguring] Will initialize slot:"<<slot<<endl;
	  
	  if (fecAccess_==0) {
	    
	    if (extratimers_) GlobalTimer_.printTime("stateConfiguring -- Will create FEC access");
	    
	    std::cout << "First device, will create Fec Access" << std::endl;
	    
	    // Create the FEC Access
	    int argc=2;
	    int cnt ;
	    try {
	      if(type=="PCI"){
        
          //char * argv[]={"portcard.txt","-psi"};
        
          char* argv[2];
  
          std::string argv_0 = "portcard.exe";
          std::string argv_1 = "-pci";

          char* argv_0c = new char[argv_0.size()+1];
          char* argv_1c = new char[argv_1.size()+1];
  
          std::copy(argv_0.begin(), argv_0.end(), argv_0c);
          std::copy(argv_1.begin(), argv_1.end(), argv_1c);
  
          argv_0c[argv_0.size()] = '\0';
          argv_1c[argv_1.size()] = '\0';

          argv[0]=argv_0c;
          argv[1]=argv_1c;

          createFecAccess ( argc, argv, &cnt, slot );
	      }
	      else if (type=="VME"){
		argc=5;
		std::string addressTablePath_string;
		if (getenv("BUILD_HOME")==0) {
		  addressTablePath_string=getenv("XDAQ_ROOT");
		  addressTablePath_string+="/dat/FecSoftwareV3_0/FecAddressTable.dat";
		}
		else {
		  addressTablePath_string=getenv("ENV_CMS_TK_ONLINE_ROOT");
		  addressTablePath_string+="/config/FecAddressTable.dat";
		}
		
		if (extratimers_) GlobalTimer_.printTime("stateConfiguring -- After addressTable");
		
		char *addressTablePath=(char *)addressTablePath_string.c_str();
		//char * argv2[]={"portcard.exe","-vmecaenpci", "-fec", const_cast<char*>(itoa(slot).c_str()), addressTablePath};
    char *argv2[5];

    std::string argv2_0 = "portcard.exe";
    std::string argv2_1 = "-vmecaenpci";
    std::string argv2_2 = "-fec";
    
    char* argv2_0c = new char[argv2_0.size()+1];
    char* argv2_1c = new char[argv2_1.size()+1];
    char* argv2_2c = new char[argv2_2.size()+1];

    std::copy(argv2_0.begin(), argv2_0.end(), argv2_0c);
    std::copy(argv2_1.begin(), argv2_1.end(), argv2_1c);
    std::copy(argv2_2.begin(), argv2_2.end(), argv2_2c);

    argv2_0c[argv2_0.size()] = '\0';
    argv2_1c[argv2_1.size()] = '\0';
    argv2_2c[argv2_2.size()] = '\0';

    argv2[0]=argv2_0c;
    argv2[1]=argv2_1c;
    argv2[2]=argv2_2c;
    argv2[3]=const_cast<char*>(itoa(slot).c_str());
    argv2[4]=addressTablePath; 
    
    // char * argv2[]={"portcard.exe","-vmecaenpci", "-fec", const_cast<char*>(itoa(slot).c_str()), addressTablePath};
		createFecAccess ( argc, argv2, &cnt, false ) ;
		
		if (extratimers_)     GlobalTimer_.printTime("stateConfiguring -- After createFecAccess");
		
		// reset the VME crate (yes, the whole crate)
		// seems to propogate reset down to all mFECs and all CCUs
		// overkill, but probably safe
		if(fecAccess_) {
#ifndef NO_PILOT_RESET
		  crateReset( fecAccess_, false , 42, 42 );  //FIXME this is overkill! (but works)
#endif
		}
		
		if (extratimers_)     GlobalTimer_.printTime("stateConfiguring -- After carteReset");
		
	      }
	      else assert(0);
	      fecAccessType = type;
	    }
	    catch (FecExceptionHandler e) {
	      cout << "------------ Exception ----------" << std::endl ;
	      cout << e.what()  << std::endl ;
	      cout << "---------------------------------" << std::endl ;
	      XCEPT_RAISE(xdaq::exception::Exception,"Failed to create FEC access");
	    }
	  }
	  else
	    {
	      std::cout << "First device, Fec Access already exists" << std::endl;
	    }
	  // Set the options for the FecAccess
	  fecAccess_->setForceAcknowledge (fack) ;
	  fecAccess_->seti2cChannelSpeed (tempPortCard->geti2cSpeed()) ;
	  
	  
	  if (extratimers_)     GlobalTimer_.printTime("stateConfiguring -- After speed and ack programming");
	  
	  // Enable the device driver counters
	  if (getErrorCounterFlag()) {
	    fprintf (getStdchan(), "------------------------------------ Error counting start\ntimestamp=%ld\n", time(NULL)) ;
	  }
	  
	} // if np
	
	
	//send reset, check if ring has been reset alerady 
	if( !ringInit[(ring-1)]) {  // rings go from 1-8
	  cout<<" Reset slot/mfec "<<slot<<"/"<<ring<<endl;
#ifndef NO_PILOT_RESET
	  resetPlxFec ( fecAccess_, slot, ring, loop, tms ) ;
#endif
	  ringInit[(ring-1)]=true;
	}
	else
	  cout << "NOT RESETTING slot/mfec " << slot << "/" << ring << " since it was already done" << endl;
	
	if (extratimers_)     GlobalTimer_.printTime("stateConfiguring -- After resetPlxFec");
	
	// this would be where we pull a STATUS flag for the CCU out of the configuration data
	//for now insert true for everything
	//for testing, we could arbitrarily insert 'false' for one ccu
	ccuRingMap[ tempPortCard->getringAddress() ].insert( make_pair(tempPortCard->getccuAddress(),true));
	//using a std::set should make a list of 8 CCU addresses for the BPix and 4 for the fpix
	
	if (extratimers_)   GlobalTimer_.printTime("stateConfiguring -- After ccuRingMap");
	
      } //end of loop over portcards
      
      //Each TKFEC controls several CCU rings (4 for FPix, 4 for BPix)
      //Each CCU ring has either 4 (FPix) or 8 (BPix) CCUs plus the dummy
      //the topology of the CCU rings happens to be in decreasing address order
      //so we will put them in a set and iterate from the end
      
      //There was previously some logic here to figure out the dummy address from the regular CCU addresses.
      //This logic doesn't work! if we even actually try to implement this, we need to either hard-code the dummy addresses
      //(0x7a for fpix and 0x76 for bpix) or we need to put them in a new piece of configuration data
      
      
      if (extratimers_)     GlobalTimer_.printTime("stateConfiguring -- Finish FecAccess part");
      
      
      ////////////Configure CCU Ring///////////////
      /* This is for configuring the B ring...leave commented out for now
	 
      //loop over our map
      for( map<unsigned int, set<pair<unsigned int,bool> > >::const_iterator ringiter = ccuRingMap.begin(); ringiter != ccuRingMap.end(); ++ringiter ) {

      keyType indexFecRing = buildFecRingKey(slot,ringiter->first) ;
      
      //copied from the fec code
      try {
	unsigned int timeout = 10 ;
	do {
	  // Make a reset
	  fecAccess_->fecHardReset (indexFecRing) ;
	  fecAccess_->fecRingReset (indexFecRing) ;
	  fecAccess_->fecRingResetB ( indexFecRing ) ;
	  if (! isFecSR0Correct(fecAccess_->getFecRingSR0(indexFecRing))) {
	    cout << "PARASITIC: After reset => SR0 = 0x" << hex << (int)fecAccess_->getFecRingSR0(indexFecRing)<<endl ;
	    emptyFIFOs ( fecAccess_, indexFecRing, false ) ;
	  }	  
	  // Log message
	  cout << "FEC reset Performed (SR0 = 0x" << hex << (int)fecAccess_->getFecRingSR0(indexFecRing) << ")" << endl ;
	  timeout -- ;
	  if (! isFecSR0Correct(fecAccess_->getFecRingSR0(indexFecRing))) usleep (tms) ;
	}
	while (! isFecSR0Correct(fecAccess_->getFecRingSR0(indexFecRing)) && timeout > 0) ;
      }
      catch (FecExceptionHandler & e) {
	cout << "Problem during the reset of the ring: " << e.getMessage() << endl ;
      }
      
      
      set<pair<unsigned int,bool> >::const_reverse_iterator ccuiterprev = ringiter->second.rbegin();
      set<pair<unsigned int,bool> >::const_reverse_iterator ccuiter = ringiter->second.rbegin();
      // comment out redundancy ring stuff (still in testing stage)
      unsigned int ccucounter = 1;
      bool previousCcuBad=false;
      for( ; ccuiter != ringiter->second.rend(); ++ccuiter ) { //ccu loop
	if (ccuiter == ringiter->second.rbegin() ) { //first ccu in the ring
	  cout<<ccucounter<<"; First CCU: 0x"<<hex<<ccuiter->first<<dec; //DEBUG
	  tscType32 fecCR0 = fecAccess_->getFecRingCR0(indexFecRing) ; //get current fec cr0 values
	  if ( ccuiter->second == false ) { //ccu is flagged as bad
	    cout<<" is bad!"<<endl; //DEBUG
	    //tell TKFEC to use ring B (control reg 0, bit 3 =1)
	    //the fec software does something a bit fancier, but i'm only going to implement what i understand
	    fecCR0 |= 8 ; //enable bit 3-->output on the b ring from the FEC
	    previousCcuBad=true;
	  }
	  else { //ccu is flagged as good
	    cout<<" is good!"<<endl; //DEBUG
	    //similar, except we want to take fecCR0 and set bit 3 to 0
	    fecCR0 &= 0xFFE7; //disable bits 3 and 4, otherwise leave enabled bits alone
	    previousCcuBad=false;
	  }
	  cout<<"Set FEC CR0 to 0x"<<hex<<fecCR0<<dec<<endl;
	  fecAccess_->setFecRingCR0 (indexFecRing, fecCR0) ;
	}
	else if (ccucounter != ringiter->second.size()) { //not the last ccu in the ring
	  cout<<ccucounter<<"; CCU: 0x"<<hex<<ccuiter->first<<dec; //DEBUG
	  if ( ccuiter->second == false ) { //this ccu is bad
	    cout<<" is bad!"<<endl; //DEBUG
	    assert ( !previousCcuBad ); //we can't have 2 bad CCUs in a row
	    //we need to operate on the previous ccu in the ring
	    keyType index = buildCompleteKey(slot,ringiter->first,ccuiterprev->first,0,0) ; //from fec code
	    //set bit 1 of CRC
	    fecAccess_->setCcuCRC (index, 2) ;
	    //we need to tell the next ccu to use the alternate input port
	    previousCcuBad=true;
	  }
	  else if (previousCcuBad) {
	    cout<<" is good, but previous CCU was bad!"<<endl; //DEBUG
	    //set alternate input port on current CCU
	    keyType index = buildCompleteKey(slot,ringiter->first,ccuiter->first,0,0) ; //from fec code
	    //set bit 0 of CRC
	    fecAccess_->setCcuCRC (index, 1) ;
	    fecAccess_->fecRingRelease(indexFecRing); //inspired by fec code
	    previousCcuBad=false;
	  }
	  else { //this ccu and the previous ccu are both ok
	    cout<<" is good!"<<endl; //DEBUG
	    //we could set ccu crc to 0	    
	  }
	  ++ccuiterprev; //increment the other iterator
	}
	else { //last CCU in the ring
	  //this is the dummy CCU
	  cout<<ccucounter<<"; dummy CCU: 0x"<<hex<<ccuiter->first<<dec; //DEBUG
	  tscType32 fecCR0 = fecAccess_->getFecRingCR0(indexFecRing) ; //get current fec cr0 values
	  if ( ccuiter->second == false ) { //this ccu is bad
	    cout<<" is bad!"<<endl; //DEBUG
	    assert ( !previousCcuBad ); //we can't have 2 bad CCUs in a row
	    //fec code sets the fec first, then the ccu; i will do the same
	    // tell the FEC to use the alternate input port
	    fecCR0 |= 16 ; //enable bit 4-->input on the b ring from the FEC
	    fecAccess_->setFecRingCR0 (indexFecRing, fecCR0) ;
	    //we need to operate on the previous ccu in the ring
	    keyType index = buildCompleteKey(slot,ringiter->first,ccuiterprev->first,0,0) ; //from fec code
	    //set bit 1 of CRC
	    fecAccess_->setCcuCRC (index, 2) ;
	  }
	  else if (previousCcuBad) {
	    cout<<" is good, but previous CCU was bad!"<<endl; //DEBUG
	    //set alternate input port on current CCU
	    keyType index = buildCompleteKey(slot,ringiter->first,ccuiter->first,0,0) ; //from fec code
	    //set bit 0 of CRC
	    fecAccess_->setCcuCRC (index, 1) ;
	    fecAccess_->fecRingRelease(indexFecRing); //inspired by fec code
	    previousCcuBad=false;
	  }
	  else { //this ccu and previous are ok
	    cout<<" is good!"<<endl; //DEBUG
	    fecCR0 &= 0xFFEF; //disable bit 4, otherwise leave enabled bits alone
	    cout<<"about to setFecRingCR0"<<endl;
	    fecAccess_->setFecRingCR0 (indexFecRing, fecCR0) ;
	    cout<<"done with setFecRingCR0"<<endl;
	  }
	    cout<<"about to do fecRingRelease"<<endl;
	  fecAccess_->fecRingRelease(indexFecRing); //inspired by fec code
	    cout<<"done with fecRingRelease"<<endl;
	}
	ccucounter++;
      }
      cout<<"about to do emptyFIFOs"<<endl;
      emptyFIFOs(fecAccess_,indexFecRing,false); //following fec code
      cout<<"done with emptyFIFOs"<<endl;
      cout << "End of CCU redundancy configuration for ring = 0x"<<hex<<ringiter->first<<dec<<" (SR0 = 0x" 
	   << hex << (int)fecAccess_->getFecRingSR0(indexFecRing) << ")" << endl ;

      //now we have (in principle) configured the CCU B channels and have a working CCU ring
      //so we should loop over the (working) CCUs and configure other things
      //in this case it doesn't matter in what order we traverse the CCU ring for programming
      //but we may as well do it the same way as before
      ccuiter = ringiter->second.rbegin();
      for( ; ccuiter != ringiter->second.rend(); ++ccuiter ) { //ccu loop
	if ( ccuiter->second == false) continue; //this ccu is bad, so skip it
	if ( ccuiter->first == dummyAddress ) continue; //skip dummy
	//0x30 is the channel address for the PIA channel; same for fpix and bpix
	keyType index = buildCompleteKey(slot,ringiter->first,ccuiter->first,0x30,0) ;
	PiaChannelAccess piaChannelAccess(fecAccess_,index); //create pia access
	keyType accessKey=piaChannelAccess.getKey();
	cout<<"[CCU configuration ring=0x"<<hex<<ringiter->first<<dec<<" ccu=0x"<<hex<<ccuiter->first<<dec<<"] Found Pia Channel DDR = "<<flush;
	cout<<int(fecAccess_->getPiaChannelDDR(accessKey))<<endl;
	fecAccess_->setPiaChannelDDR(accessKey,0xFF); //write lots of 1s to set the data direction register to 'output'
	//for debugging, read it back again
	cout<<"[After programming ring=0x"<<hex<<ringiter->first<<dec<<" ccu=0x"<<hex<<ccuiter->first<<dec<<"] Found Pia Channel DDR = "<<int(fecAccess_->getPiaChannelDDR(accessKey))<<endl;
      }
    }
/////////// END Configuration of CCU Ring/////// 
end of redundancy ring comment */
 
      if (extratimers_)     GlobalTimer_.printTime("stateConfiguring -- Fec access done");
      
      // Now really program portcards
      std::cout << "Number of portcards to configure:"<<portcards.size()<< std::endl;        
      bool status = programPortcards(false);
      if(status) cout<<" Error in portcard config "<<status<<endl;
      if (extratimers_)     GlobalTimer_.printTime("stateConfiguring -- configure portcards done!");
      suppressHardwareError_=false;

      
      // Submit a job to the workloop if it is a Physics Run (Actual start of thread)
      if (theCalibObject_==0) { // This must be a Physics Run
	
	// Do nothing      
	//if(doDCULoop_) {
	//workloop_->submit(readDCU_);
	//std::cout<<"PixelTKFECSupervisor::Configure - Submitted readDCU_ job to the workloop."<<std::endl;
	//}
	
      } else {
	
	//Create pointers to give to the calibration object
	PixelTKFECSupervisorConfiguration* pixTKFECSupConfPtr = dynamic_cast <PixelTKFECSupervisorConfiguration*> (this);
	SOAPCommander* soapCmdrPtr = dynamic_cast <SOAPCommander*> (this);
	theTKFECCalibrationBase_=0;      
	std::string mode=theCalibObject_->mode();      
	PixelCalibrationFactory calibFactory;     
	theTKFECCalibrationBase_ = calibFactory.getTKFECCalibration(mode, pixTKFECSupConfPtr, soapCmdrPtr);
	
      }

      //std::cout << "Disable the PIA ports "<< std::endl;        
      //program the CCU (this is to disable PIA resets in order not to have the fire by themselves)
      for( map<unsigned int, set<pair<unsigned int,bool> > >::const_iterator ringiter = ccuRingMap.begin(); 
	   ringiter != ccuRingMap.end(); ++ringiter ) { // loop over mfecs

	// JMTBAD this needs to be configurable in software. Loop over
	// ccu or some other config objects and find out whether we're
	// supposed to send PIA commands to enable DC-DC. For now just do it
	if (ringiter->first == 8) {
	  pixDCDCCommand(slot, ringiter->first, 0x7e, 0x7d, 0x30, true, 2);
	}

	set<pair<unsigned int,bool> >::const_reverse_iterator ccuiter = ringiter->second.rbegin();
	for( ; ccuiter != ringiter->second.rend(); ++ccuiter ) { //ccu loop

	  if ( ccuiter->second == false) continue; //this ccu is bad, so skip it
	  //	if ( ccuiter->first == dummyAddress ) continue; //skip dummy
#if 0      

#ifdef MYTEST
	  // This is a special test to check the PIA ports, use unused port 0x33
	  keyType index = buildCompleteKey(slot,ringiter->first,ccuiter->first,0x33,0) ;
	  PiaChannelAccess piaChannelAccess(fecAccess_,index); //create pia access
	  keyType accessKey=piaChannelAccess.getKey();

	  //printPIAinfo(slot,ringiter->first,ccuiter->first,0x33);
	  //cout<<"[ring=0x"<<hex<<ring<<dec<<" ccu=0x"<<hex<<ccu<<dec<<" PIA channel=0x"<<hex<<channel<<dec<<"]"<<endl;
	  cout<<" PIA channel=0x"<<hex<<0x33<<dec<<"]"<<endl;
	  cout<<"PIA GCR = 0x"<<hex<<int(fecAccess_->getPiaChannelGCR(accessKey))<<" ";
	  cout<<"PIA status = 0x"<<hex<<int(fecAccess_->getPiaChannelStatus(accessKey))<<" ";
	  cout<<"PIA DDR = 0x"<<hex<<int(fecAccess_->getPiaChannelDDR(accessKey))<<" ";
	  cout<<"PIA data reg = 0x"<<hex<<int(fecAccess_->getPiaChannelDataReg(accessKey))<<dec<<endl;

	  fecAccess_->setPiaChannelDDR(accessKey,0xFF); // define as output

	  //printPIAinfo(slot,ringiter->first,ccuiter->first,0x33);
	  //cout<<"[ring=0x"<<hex<<ring<<dec<<" ccu=0x"<<hex<<ccu<<dec<<" PIA channel=0x"<<hex<<channel<<dec<<"]"<<endl;
	  cout<<"PIA GCR = 0x"<<hex<<int(fecAccess_->getPiaChannelGCR(accessKey))<<" ";
	  cout<<"PIA status = 0x"<<hex<<int(fecAccess_->getPiaChannelStatus(accessKey))<<" ";
	  cout<<"PIA DDR = 0x"<<hex<<int(fecAccess_->getPiaChannelDDR(accessKey))<<" ";
	  cout<<"PIA data reg = 0x"<<hex<<int(fecAccess_->getPiaChannelDataReg(accessKey))<<dec<<endl;

	  //writePIADataReg(slot,ringiter->first,ccuiter->first,0x33,0x55); // write 01010101
	  //cout<<"[ring=0x"<<hex<<ring<<dec<<" ccu=0x"<<hex<<ccu<<dec<<" PIA channel=0x"<<hex<<channel<<dec<<"]"<<endl;
	  cout<<"Writing PIA DataReg with value = "<<hex<<0x55<<dec<<endl;
	  fecAccess_->setPiaChannelDataReg(accessKey,0x55);


	  //printPIAinfo(slot,ringiter->first,ccuiter->first,0x33);
	  //cout<<"[ring=0x"<<hex<<ring<<dec<<" ccu=0x"<<hex<<ccu<<dec<<" PIA channel=0x"<<hex<<channel<<dec<<"]"<<endl;
	  cout<<"PIA GCR = 0x"<<hex<<int(fecAccess_->getPiaChannelGCR(accessKey))<<" ";
	  cout<<"PIA status = 0x"<<hex<<int(fecAccess_->getPiaChannelStatus(accessKey))<<" ";
	  cout<<"PIA DDR = 0x"<<hex<<int(fecAccess_->getPiaChannelDDR(accessKey))<<" ";
	  cout<<"PIA data reg = 0x"<<hex<<int(fecAccess_->getPiaChannelDataReg(accessKey))<<dec<<endl;

#endif

	  //0x30 is the channel address for the PIA channel; same for fpix and bpix
	  printPIAinfo(slot,ringiter->first,ccuiter->first,0x30);
	  printCCUCRE(slot,ringiter->first,ccuiter->first);
	  disablePIAchannels(slot,ringiter->first,ccuiter->first);
	  printCCUCRE(slot,ringiter->first,ccuiter->first);

	  //	printPIAinfo(slot,ringiter->first,ccuiter->first,0x30); //throws an exception iff PIA channels have been disabled
	  //	writePIADataReg(slot,ringiter->first,ccuiter->first,0x30,0x0); //doesn't seem to do anything!
	  // 	fecAccess_->setPiaChannelDDR(accessKey,0xFF); //seems to cause a reset
	  //	printPIAinfo(slot,ringiter->first,ccuiter->first,0x30);

#endif

	}
      }
      
      // Fire a transition to Configured state
      try {
	toolbox::Event::Reference e(new toolbox::Event("ConfiguringDone", this));
	fsm_.fireEvent(e);
      } catch (toolbox::fsm::exception::Exception & e) {
	XCEPT_RETHROW(xoap::exception::Exception, "Invalid Command.", e);
      }
    }
  } //end of the BIG try-catch
  catch (FecExceptionHandler e) {
    diagService_->reportError("Failed to configure TKFEC (is the power off?); exception: "+string(e.what()),DIAGERROR);
    isfailure=true;
  }
  catch (std::exception & e) { //failure to load config data raises std::exception (not xcept::Exception)
    diagService_->reportError("Failed to configure TKFEC; exception: "+string(e.what()),DIAGERROR);
    isfailure=true;
  }


  if (isfailure) {

    try {
      //The following line should be commented for normal operation; it can be uncommented to test pxl FEC when there is no power
#ifdef SKIP_LV_CHECK
      // no power bypass, for special tests without power ON
      toolbox::Event::Reference ev(new toolbox::Event("ConfiguringDone", this)); diagService_->reportError("Bypassing Error state in TKFEC!",DIAGWARN);
#else 
      // For normal default operation with power ON 
      toolbox::Event::Reference ev(new toolbox::Event("Failure", this)); //comment this out only for testing
#endif 
      fsm_.fireEvent(ev);
    } catch (toolbox::fsm::exception::Exception & e2) {
      diagService_->reportError("Failed to transition to Failed state!",DIAGFATAL);
    }

  }
  
  delete detconfig;
  
}

//------------------------------------------------------------------------------------------
// Split the pure portcard programming from other things in stateConfiguring
bool  PixelTKFECSupervisor::programPortcards(bool errorFlag)  {
     
  //const long loop = 1 ; // number of resets 
  //const unsigned long tms  = 0 ;  // wait tms microseconds after reset
  //unsigned int slot=9999; 
  unsigned int ring =9999;
  static bool ringInit[8] = {false,false,false,false,false,false,false,false}; // has the ring been reset
  enumDeviceType modeType = PHILIPS ;
  bool problem = false;

  std::cout << "Number of portcards to configure:"<<mapNamePortCard_.size()<< std::endl;

  //loop over portcards
  map<string, PixelPortCardConfig*>::const_iterator iportcard=mapNamePortCard_.begin();

  for ( ; iportcard != mapNamePortCard_.end() ; ++iportcard) {

    //since we've already built mapNamePortCard to only have
    //the portcards on this tkfec, then this better not fail!

    PixelPortCardConfig* tempPortCard = iportcard->second;
    const std::string TKFECID = tempPortCard->getTKFECID();

    if ( theTKFECConfiguration_->crateFromTKFECID(TKFECID) != crate_ ) assert(0); //continue;
    assert ( theTKFECConfiguration_->crateFromTKFECID(TKFECID) == crate_ );

    const unsigned int TKFECAddress = theTKFECConfiguration_->addressFromTKFECID(TKFECID);
    
#ifdef USELESS    
    std::vector< unsigned int > index_check(tempPortCard->getdevicesize()+5); // +5 to make room for the 5 initialization settings
    for(unsigned int i=0;i<tempPortCard->getdevicesize()+5;i++)index_check[i]=0;
    int indexNo=0;
#endif

    // Try the PLX reset , do it only once per ring (or should we do it for each portcard?)
     ring = tempPortCard->getringAddress();
     if( !ringInit[(ring-1)]) {  // rings go from 1-8
       cout<<" Reset slot/mfec "<<TKFECAddress<<"/"<<ring<<endl;
#ifndef NO_PILOT_RESET
       resetPlxFec ( fecAccess_, TKFECAddress, ring, loop, tms ) ;
#endif
       ringInit[(ring-1)]=true;
     }

    std::cout <<"Will now configure portcard:"<<std::endl;
    for(int i=-5;i<(signed int)(tempPortCard->getdevicesize());i++){  // num of devices

      unsigned int deviceAddress;
      unsigned int value;
      
      // 5 initialization settings, and then the settings from the configuration file.
      if (i==-4) {
	deviceAddress = tempPortCard->getdeviceAddressForSetting(PortCardSettingNames::k_PLL_CTR1);
	value         = 0x8;
      } else if (i==-3) {
	deviceAddress = tempPortCard->getdeviceAddressForSetting(PortCardSettingNames::k_PLL_CTR1);
	value         = 0x0;
      } else if (i==-2) {
	deviceAddress = tempPortCard->getdeviceAddressForSetting(PortCardSettingNames::k_PLL_CTR2); // set PLL_CTR4or5 to PLL_CTR5
	value         = 0x20;
      } else if (i==-1) {
	deviceAddress = tempPortCard->getdeviceAddressForSetting(PortCardSettingNames::k_PLL_CTR4or5);
	value         = 0x0;
      } else if (i==-5) {  // Make this first 
	deviceAddress = tempPortCard->getdeviceAddressForSetting(PortCardSettingNames::k_DOH_Dummy); // turn off unused laser
	value         = 0x0;
      } else {
	deviceAddress=tempPortCard->getdeviceAddress(i);
	value=tempPortCard->getdeviceValues(i);
	
	if (theGlobalDelay25_!=0 && theCalibObject_==0) { // This must be a Physics Run
	  // SCL and TRG do not wrap around, if the desired value is out of range, no global delay is applied
	  // i.e. getDelay(value) just returns the original 'value'
	  if ( (deviceAddress==tempPortCard->getdeviceAddressForSetting(PortCardSettingNames::k_Delay25_SCL)) ||
	       (deviceAddress==tempPortCard->getdeviceAddressForSetting(PortCardSettingNames::k_Delay25_TRG)) ){
	    value=theGlobalDelay25_->getDelay(value);
	  }
	  // SDA wraps around, it is always applied regardless if SCL/TRG were changed or not
	  if (deviceAddress==tempPortCard->getdeviceAddressForSetting(PortCardSettingNames::k_Delay25_SDA)) {
	    value=theGlobalDelay25_->getCyclicDelay(value);
	  }
	  
	}
      } // if

#ifdef USELESS
        // Set flag to 0 if this deviceAddress was not set before, or 1 if it was set before.
        int flag = 0;
        for(int j=0;j<indexNo+1;j++) {
	  if(deviceAddress==index_check[j]){
	    flag=1;break;
	  }
	}
        if(!flag){index_check[indexNo]=deviceAddress;indexNo++;}
#endif

        //write value to the I2C device
        //        std::cout<<std::endl;
        //        printf("%2d, the i2c address is : %x, flag = %10d\n",i,tempPortCard->getdeviceAddress(i),flag);

        //this is all for debugging
        //== start debugging block
//      try {
//        cout << "fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, value: "
//             <<(int)TKFECAddress<<" "<<(int)tempPortCard->getringAddress()<<std::hex<<" "<<(int)tempPortCard->getccuAddress()<<" "           
//             <<(int)tempPortCard->getchannelAddress()<<" "<<(int)deviceAddress<<std::dec<<" "<<endl;
//        keyType myindex = buildCompleteKey(TKFECAddress,tempPortCard->getringAddress(),tempPortCard->getccuAddress(),tempPortCard->getchannelAddress(),deviceAddress) ;
//        fecAccess_->addi2cAccess (myindex,   modeType,MODE_SHARE) ;
//        cout<<"I2C SRA = "<<hex<<(int) fecAccess_->geti2cChannelSRA( myindex)<<dec<<endl;
//        cout<<"I2C SRB = "<<hex<<(int) fecAccess_->geti2cChannelSRB( myindex)<<dec<<endl;
//        cout<<"I2C SRC = "<<hex<<(int) fecAccess_->geti2cChannelSRC( myindex)<<dec<<endl;
//        cout<<"I2C SRD = "<<hex<<(int) fecAccess_->geti2cChannelSRD( myindex)<<dec<<endl;
//        fecAccess_->removei2cAccess (myindex);
//      }
//      catch (FecExceptionHandler theError) {
//        cout<<"Caught an exception: "<<theError.what()<<endl;

//      }
//      catch (...) {
//                cout<<"Caught an unknown exception!"<<endl;
//      }


        // WRITE 
        try {
	  portcardI2CDevice(fecAccess_,
			    TKFECAddress,
			    tempPortCard->getringAddress(),
			    tempPortCard->getccuAddress(),
			    tempPortCard->getchannelAddress(),
			    deviceAddress,
			    modeType,
			    value,
			    1);
        } catch (FecExceptionHandler& e) {
	  cout<<" --------------------------------------------------"<<endl;
	  cout<<" -- Exception caught during portcardI2CDevice() --"<<hex<<endl
	      <<"ring address = "<<tempPortCard->getringAddress()<<endl
	      <<"CCU address  = "<<tempPortCard->getccuAddress()<<endl
	      <<"Channel address = "<<tempPortCard->getchannelAddress()<<endl
	      <<"device address  = "<<deviceAddress<<endl
	      <<"value = "<<value<<dec<<endl;
	  
	  //keyType myindex = buildCompleteKey(TKFECAddress,tempPortCard->getringAddress(),tempPortCard->getccuAddress(),tempPortCard->getchannelAddress(),deviceAddress) ;
	  //fecAccess_->addi2cAccess (myindex,modeType,MODE_SHARE);
	  //cout<<"I2C SRA = "<<hex<<(int) fecAccess_->geti2cChannelSRA( myindex)<<dec<<endl;
	  //cout<<"I2C SRB = "<<hex<<(int) fecAccess_->geti2cChannelSRB( myindex)<<dec<<endl;
	  //cout<<"I2C SRC = "<<hex<<(int) fecAccess_->geti2cChannelSRC( myindex)<<dec<<endl;
	  //cout<<"I2C SRD = "<<hex<<(int) fecAccess_->geti2cChannelSRD( myindex)<<dec<<endl;
	  //fecAccess_->removei2cAccess (myindex);

	  //throw; // DO NOT RE-THROW
	} catch (...) {
	  cout<<"Caught an unknown exception in PixelTKFECSupervisor::programPortcard!"<<endl;
	}

	// Now readback
        unsigned int readBack=0;
        try {
	  readBack = portcardI2CDeviceRead(fecAccess_,
					   TKFECAddress,
					   tempPortCard->getringAddress(),
					   tempPortCard->getccuAddress(),
					   tempPortCard->getchannelAddress(),
					   deviceAddress,
					   modeType,
					   1);
	  
	} catch (FecExceptionHandler& e) {
	  cout<<" --------------------------------------------------"<<endl;
	  cout<<" -- Exception caught during portcardI2CDeviceRead() --"<<hex<<endl
	      <<"ring address = "<<tempPortCard->getringAddress()<<endl
	      <<" CCU address = "<<tempPortCard->getccuAddress()<<endl
	      <<"Channel address = "<<tempPortCard->getchannelAddress()<<endl
	      <<"device address = "<<deviceAddress<<dec<<endl
	      <<"value = "<<value<<endl;
	  // throw;
	} catch (...) {
	  cout<<"Caught an unknown exception in PixelTKFECSupervisor::programPortcard!"<<endl;
	}


        // check readback value to see it matches
        problem = ( value!=readBack );

        // here we ignore PLL CTR1 because some bits are "write-only" and will always read 0
	// and DELAY25 GCR where the reset bit reads always 0 
        if ( deviceAddress == tempPortCard->getdeviceAddressForSetting("PLL_CTR1") ||
             deviceAddress == tempPortCard->getdeviceAddressForSetting("Delay25_GCR") ) problem = false;

        if ( printReadBack_ ||  problem ) {
	  cout << "[PixelTKFECSupervisor::Configure]" << endl;
	  if (problem) {
	    cout << " Programming ERROR! Data read back != desired value."<< endl;
	  }
	  cout << "ring address=0x"<<hex<<tempPortCard->getringAddress()<<dec<<" ";
	  cout << "ccu address=0x" << std::hex << tempPortCard->getccuAddress()
	       << " chan=0x" << tempPortCard->getchannelAddress() << std::dec ;
	  cout << " deviceAddress = 0x" << std::hex << deviceAddress << ", value = 0x" << value << std::dec << "\n";
	  cout << "read back = 0x " << hex << readBack << dec << endl;
	}  //end of problem or printout

    } // loop over devices

    if (extratimers_)     GlobalTimer_.printTime("stateConfiguring -- portcard done");

  } // portcard loop 

  return problem;

}
////////////////////////////////////////////////////////////////////////////////////////////////////////

void PixelTKFECSupervisor::disablePIAchannels(int slot,int ring,int ccu)  {
  keyType index = buildCompleteKey(slot,ring,ccu,0,0) ;
  int currentval = fecAccess_->getCcuCRE(index);
#ifdef MYTEST
  int newval = currentval & 0xF8FFFF; // 0xF8FFFF to keep 0x33 enabled
#else
  int newval = currentval & 0xF0FFFF; // diable all PIA channels
#endif
  // Disable the PIA channel
  fecAccess_->setCcuCRE (index, newval);

}


void PixelTKFECSupervisor::enablePIAchannels(int slot,int ring,int ccu)  {
  keyType index = buildCompleteKey(slot,ring,ccu,0,0) ;
  int currentval = fecAccess_->getCcuCRE(index);
  //to enable all pia channels would mean 0x0F0000 here

#ifdef MYTEST
  int newval = currentval | 0x090000; // use 0x090000 to enable also the 0x33 channel
#else 
  int newval = currentval | 0x010000; //we only use the first pia channel (0x30). 
#endif

  // Disable the PIA channel
  fecAccess_->setCcuCRE (index, newval);

}

void PixelTKFECSupervisor::printCCUCRE(int slot,int ring,int ccu) {
  keyType index = buildCompleteKey(slot,ring,ccu,0,0) ;

  cout<<"[ring=0x"<<hex<<ring<<dec<<" ccu=0x"<<hex<<ccu<<dec<<"]"<<endl;
  cout<<"CCU CRE = 0x"<<hex<<  int(fecAccess_->getCcuCRE(index))<<dec<<endl;

  // Add other registers 
  cout<<"CCU CRA, SRA = 0x"<<hex<<  int(fecAccess_->getCcuCRA(index))<<" "
      <<  int(fecAccess_->getCcuSRA(index))<<dec<<endl;

  cout<<"CCU SRF = 0x"<<hex<<  int(fecAccess_->getCcuSRF(index))<<dec<<endl;  // added 19.09.12 d.k.

}

void PixelTKFECSupervisor::printI2Cinfo(int slot,int ring,int ccu,int channel) {
  keyType index = buildCompleteKey(slot,ring,ccu,channel,0) ;

  cout<<"[ring=0x"<<hex<<ring<<dec<<" ccu=0x"<<hex<<ccu<<dec<<" I2C channel=0x"<<hex<<channel<<dec<<"]"<<endl;

  // Registers 
  cout<<"I2C CRA, SRA = 0x"<<hex<<  int(fecAccess_->geti2cChannelCRA(index))<<" "
      <<  int(fecAccess_->geti2cChannelSRA(index))<<dec<<endl;

}

void PixelTKFECSupervisor::printPIAinfo(int slot,int ring,int ccu,int channel) {
  keyType index = buildCompleteKey(slot,ring,ccu,channel,0) ;
  PiaChannelAccess piaChannelAccess(fecAccess_,index); //create pia access
  keyType accessKey=piaChannelAccess.getKey();

  cout<<"[ring=0x"<<hex<<ring<<dec<<" ccu=0x"<<hex<<ccu<<dec<<" PIA channel=0x"<<hex<<channel<<dec<<"]"<<endl;
  cout<<"PIA GCR = 0x"<<hex<<int(fecAccess_->getPiaChannelGCR(accessKey))<<" ";
  cout<<"PIA status = 0x"<<hex<<int(fecAccess_->getPiaChannelStatus(accessKey))<<" ";
  cout<<"PIA DDR = 0x"<<hex<<int(fecAccess_->getPiaChannelDDR(accessKey))<<" ";
  cout<<"PIA data reg = 0x"<<hex<<int(fecAccess_->getPiaChannelDataReg(accessKey))<<dec<<endl;


}
void PixelTKFECSupervisor::writePIADataReg(int slot,int ring,int ccu,int channel,tscType8 value) {
  keyType index = buildCompleteKey(slot,ring,ccu,channel,0) ;
  PiaChannelAccess piaChannelAccess(fecAccess_,index); //create pia access
  keyType accessKey=piaChannelAccess.getKey();

  cout<<"[ring=0x"<<hex<<ring<<dec<<" ccu=0x"<<hex<<ccu<<dec<<" PIA channel=0x"<<hex<<channel<<dec<<"]"<<endl;
  cout<<"Writing PIA DataReg with value = "<<hex<<int(value)<<dec<<endl;

  fecAccess_->setPiaChannelDataReg(accessKey,value);

}

//////////////////////////////////////////////////////////////////////////
/////////////////////////// FSM State Transition Actions /////////////////
//////////////////////////////////////////////////////////////////////////

void PixelTKFECSupervisor::transitionHaltedToConfiguring (toolbox::Event::Reference e) //throw (toolbox::fsm::exception::Exception)
{

  if (DEBUG && extratimers_)   GlobalTimer_.printTime("transitionHaltedToConfiguring -- Enter");
  //instead of assert
  if ( mapNamePortCard_.size()!=0 || thePortcardMap_!=0 ) cleanupGlobalConfigData();

  if (extratimers_)   GlobalTimer_.printTime("transitionHaltedToConfiguring -- get calib");
  
  try { //access configuration data
  PixelConfigInterface::get(theCalibObject_, "pixel/calib/", *theGlobalKey_);
  
  if (extratimers_)   GlobalTimer_.printTime("transitionHaltedToConfiguring -- build ROC and module lists");
  // Build ROC and module lists.
  PixelConfigInterface::get(theNameTranslation_, "pixel/nametranslation/", *theGlobalKey_);

  if(dynamic_cast <PixelCalibConfiguration*> (theCalibObject_)!=0){
    PixelDetectorConfig* detconfig=0; // temporary, used only to build ROC and module lists
    PixelConfigInterface::get(detconfig, "pixel/detconfig/", *theGlobalKey_);
    if (detconfig==0) 
      XCEPT_RAISE (toolbox::fsm::exception::Exception, "[PixelTKFECSupervisor::transitionHaltedToConfiguring] The detconfig could not be loaded");
    (dynamic_cast <PixelCalibConfiguration*> (theCalibObject_))->buildROCAndModuleLists(theNameTranslation_, detconfig);
    delete detconfig;
  }
  
  if (extratimers_)   GlobalTimer_.printTime("transitionHaltedToConfiguring -- read other files");
  
  //needed for delay25 calibration
  PixelConfigInterface::get(theFECConfiguration_, "pixel/fecconfig/", *theGlobalKey_);

  PixelConfigInterface::get(theTKFECConfiguration_, "pixel/tkfecconfig/", *theGlobalKey_);
  
  PixelConfigInterface::get(thePortcardMap_,"pixel/portcardmap/", *theGlobalKey_);

  PixelConfigInterface::get(theGlobalDelay25_, "pixel/globaldelay25/", *theGlobalKey_);
  if (theGlobalDelay25_==0) cout<<"Global delay in Delay25 is not specified. Using only the default Delay25 settings."<<endl;
  }
  catch (toolbox::fsm::exception::Exception & e) { throw; }
  catch (std::exception & e) {  //translate std::exception to the correct type
    XCEPT_RAISE(toolbox::fsm::exception::Exception, string(e.what()));
  }


  if (extratimers_)   GlobalTimer_.printTime("transitionHaltedToConfiguring -- done!");

  //check for failure
  //can't test theGlobalDelay25_ or theCalibObject_ because it is allowed to be undefined
  if (theNameTranslation_==0 || theFECConfiguration_==0 || theTKFECConfiguration_==0 || thePortcardMap_==0) 
    XCEPT_RAISE (toolbox::fsm::exception::Exception, "Failure to load configuration data in PixelTKFECSupervisor::transitionHaltedToConfiguring");
  //this exception will be automatically caught and transition us to the Error state

}


void PixelTKFECSupervisor::enteringError(toolbox::Event::Reference e) //throw (toolbox::fsm::exception::Exception)
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

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------
xoap::MessageReference PixelTKFECSupervisor::PIAReset (xoap::MessageReference msg) //throw (xoap::exception::Exception)
	{
 	Attribute_Vector parameters(1);
 	parameters.at(0).name_="resetParam";
	Receive(msg,parameters);
	std::string resetParam_ = parameters.at(0).value_;
	//cout<<"globalKey = "<<theGlobalKey_<<" "<<resetParam_<<endl;

	long loop = 1;
	unsigned long tms  = 0;
	unsigned int piaResetValue = 0xFF;
	unsigned int piaChannel    = 0x30;

	PixelConfigInterface::get(theTKFECConfiguration_, "pixel/tkfecconfig/", *theGlobalKey_); assert(theTKFECConfiguration_!=0);

	PixelConfigInterface::get(thePortcardMap_,"pixel/portcardmap/", *theGlobalKey_); assert(theGlobalKey_!=0);

	PixelDetectorConfig* detconfig=0; // temporary, used only to build ROC and module lists
	PixelConfigInterface::get(detconfig, "pixel/detconfig/", *theGlobalKey_); assert(detconfig!=0);
	const std::set<std::string>& portcards=thePortcardMap_->portcards(detconfig);
	delete detconfig;

	std::set<std::string>::const_iterator iportcard=portcards.begin(); 
	for (iportcard=portcards.begin();iportcard!=portcards.end();iportcard++)
		{
		PixelPortCardConfig* tempPortCard=0;
		PixelConfigInterface::get(tempPortCard,"pixel/portcard/"+*iportcard, *theGlobalKey_);
		assert(tempPortCard!=0);

		const std::string TKFECID = tempPortCard->getTKFECID();

		const unsigned int TKFECAddress = theTKFECConfiguration_->addressFromTKFECID(TKFECID);

		unsigned int ringAddress = tempPortCard->getringAddress();
		unsigned int ccuAddress = tempPortCard->getccuAddress();

		enablePIAchannels(TKFECAddress,ringAddress,ccuAddress); //since we now disable these channels at Configure

//		Do Pia Resets
		cout <<"fecaccess = "<<fecAccess_<<"; PORTCADR = "<<*iportcard<<"; TKFECAddress = "<<TKFECAddress<<"; ringAddress = "<<ringAddress<<"; ccuAddress = "<<ccuAddress<<endl;
		cout <<"resetParam_ = "<<resetParam_<<endl;
		if ( resetParam_ =="all" )
			{
			piaResetValue = 0xFF ;
			testPIAResetfunctions ( fecAccess_, TKFECAddress, ringAddress, ccuAddress, piaChannel, piaResetValue, 1, 10000, loop, tms );
			}
		if ( resetParam_ =="roc" )  
			{
			  piaResetValue = 0x1 ;  // BPIX
			  //			  piaResetValue = 0x2A ;  //FPIX
			testPIAResetfunctions ( fecAccess_, TKFECAddress, ringAddress, ccuAddress, piaChannel, piaResetValue, 1, 10000, loop, tms );
			}
		if ( resetParam_ =="aoh" )  //FPIX should be 0x15 bits 0,2,4  all portcard devices AOH, DOH, Delay 25, DCU 
                        {
			  piaResetValue = 0x2 ;  //BPIX
			  //			  piaResetValue = 0x15 ;  //FPIX
			testPIAResetfunctions ( fecAccess_, TKFECAddress, ringAddress, ccuAddress, piaChannel, piaResetValue, 1, 10000, loop, tms );
			}
		if ( resetParam_ =="doh" )
			{
			piaResetValue = 0x4 ;
			testPIAResetfunctions ( fecAccess_, TKFECAddress, ringAddress, ccuAddress, piaChannel, piaResetValue, 1, 10000, loop, tms );
			}
		if ( resetParam_ =="res1" )
			{
			piaResetValue = 0x8 ;
			testPIAResetfunctions ( fecAccess_, TKFECAddress, ringAddress, ccuAddress, piaChannel, piaResetValue, 1, 10000, loop, tms );
			}
		if ( resetParam_ =="res2" )
			{
			piaResetValue = 0x10 ;
			testPIAResetfunctions ( fecAccess_, TKFECAddress, ringAddress, ccuAddress, piaChannel, piaResetValue, 1, 10000, loop, tms );
			}
		if ( resetParam_ =="fpixroc" ) //FPIX 0x2A bits 1, 3, 5
		        {
			 piaResetValue = 0x2A ;  
			 testPIAResetfunctions ( fecAccess_, TKFECAddress, ringAddress, ccuAddress, piaChannel, piaResetValue, 1, 10000, loop, tms );
		        }
		if ( resetParam_ =="fpixdevice" ) //FPIX 0x15 bits 0, 2, 4 : to AOH, DOH, Delay25, DCU
		        {
			 piaResetValue = 0x15;  
			 testPIAResetfunctions ( fecAccess_, TKFECAddress, ringAddress, ccuAddress, piaChannel, piaResetValue, 1, 10000, loop, tms );
		        }

		disablePIAchannels(TKFECAddress,ringAddress,ccuAddress);
		
		
		}
	xoap::MessageReference reply=MakeSOAPMessageReference("PIAResetDone");
	return reply;
	}

//==========================================================================================

xoap::MessageReference PixelTKFECSupervisor::FSMStateRequest (xoap::MessageReference msg) //throw (xoap::exception::Exception)
{
  return MakeSOAPMessageReference(state_);
}

xoap::MessageReference PixelTKFECSupervisor::Reset (xoap::MessageReference msg) //throw (xoap::exception::Exception)
{
  std::cout<<"PixelTKFECSupervisor::reset - New state before reset is: "<<fsm_.getStateName(fsm_.getCurrentState());
  fsm_.reset();
  std::cout<<"PixelTKFECSupervisor::reset - New state after reset is: "<<fsm_.getStateName(fsm_.getCurrentState());
  xoap::MessageReference reply=MakeSOAPMessageReference("resetDone");
  return reply;
}

xoap::MessageReference PixelTKFECSupervisor::AOH (xoap::MessageReference msg) //throw (xoap::exception::Exception)
{
        Attribute_Vector parametersReceived(3);
        parametersReceived.at(0).name_="Module";
        parametersReceived.at(1).name_="Bias";
	parametersReceived.at(2).name_="Gain";
        Receive(msg, parametersReceived);

	//Use the Tracker FEC interface to change bias, gains here. 
	
        xoap::MessageReference reply = MakeSOAPMessageReference("TBMCommandDone");
        return reply;
}

xoap::MessageReference PixelTKFECSupervisor::SetDelay (xoap::MessageReference msg) //throw (xoap::exception::Exception)
{
        Attribute_Vector parametersReceived(5);
	parametersReceived.at(0).name_="PortCardName";
        parametersReceived.at(1).name_="Delay";
        parametersReceived.at(2).name_="Value";
	parametersReceived.at(3).name_="Update";
	parametersReceived.at(4).name_="Write";

	Receive(msg, parametersReceived);

	/****************************************
	 *            Set the Delays            *
	 ****************************************/
	
	std::string portcardname = parametersReceived.at(0).value_;
	map<string, PixelPortCardConfig*>::iterator iter=mapNamePortCard_.find(portcardname);
	

        if (iter==mapNamePortCard_.end()){
          std::cout << "Looking for portcard:"<<portcardname<<std::endl;
          std::cout << "mapNamePortCard_ size="<<mapNamePortCard_.size()<<std::endl;
	  map<string, PixelPortCardConfig*>::iterator i=mapNamePortCard_.begin();
	  while(i!=mapNamePortCard_.end()){
	    std::cout << "mapNamePortCards:"<<i->first <<std::endl;
	    ++i;
	  }
        }
   
	assert(iter!=mapNamePortCard_.end());

	std::string TKFECID = iter->second->getTKFECID();
	assert( theTKFECConfiguration_->crateFromTKFECID(TKFECID) == crate_ );
	unsigned int fecAddress=theTKFECConfiguration_->addressFromTKFECID(TKFECID);
	unsigned int ringAddress=iter->second->getringAddress();
	unsigned int ccuAddress=iter->second->getccuAddress();
	unsigned int channelAddress=iter->second->getchannelAddress();
	unsigned int deviceAddress;

	//std::cout << "fecAddress:"<<fecAddress<<std::endl;
	//std::cout << "ringAddress:"<<ringAddress<<std::endl;

	string SDA, RDA, SCL, RCL, TRG;
	SDA="SDATA";RDA="RDATA";SCL="SCLK";RCL="RCLK";TRG="TRIG";
	
	if     (parametersReceived.at(1).value_.c_str()==SDA){ deviceAddress=iter->second->getdeviceAddressForSetting(k_Delay25_SDA); }
	else if(parametersReceived.at(1).value_.c_str()==RDA){ deviceAddress=iter->second->getdeviceAddressForSetting(k_Delay25_RDA); }
	else if(parametersReceived.at(1).value_.c_str()==SCL){ deviceAddress=iter->second->getdeviceAddressForSetting(k_Delay25_SCL); }
	else if(parametersReceived.at(1).value_.c_str()==RCL){ deviceAddress=iter->second->getdeviceAddressForSetting(k_Delay25_RCL); }
	else if(parametersReceived.at(1).value_.c_str()==TRG){ deviceAddress=iter->second->getdeviceAddressForSetting(k_Delay25_TRG); }
	else{
	  std::cout << "Register:"<<parametersReceived.at(1).value_.c_str()<<std::endl;
	  assert(0);
	}

	
	unsigned int deviceValues=atoi(parametersReceived.at(2).value_.c_str());
	
	//	std::cout<<fecAddress<<", "<<ringAddress<<", "<<ccuAddress<<", "<<channelAddress<<", "<<deviceAddress<<", "<<deviceValues<<std::endl;

	/** Download all
	 */

	assert(fecAccess_!=0);
	
	//int  flag=1;
	enumDeviceType modeType = PHILIPS ;


	portcardI2CDevice(fecAccess_, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, modeType, deviceValues, 1);
	
	/****************************************
	 *            Set the Delays            *
	 ****************************************/
	
        bool update=atoi(parametersReceived.at(3).value_.c_str());
        if(update)
          {
            iter->second->setdeviceValues(deviceAddress, deviceValues);
	    //cout << "PixelTKFECSupervisor:  updated " << parametersReceived.at(1).value_ << " value." << endl;
          }

        bool write=atoi(parametersReceived.at(4).value_.c_str());
        if(write)
          {
            iter->second->writeASCII(outputDir_+"/");
            cout << "PixelTKFECSupervisor:  new portcard_"<<portcardname<<".dat file written." << endl;
          }

	xoap::MessageReference reply = MakeSOAPMessageReference("SetDelayDone");
	return reply;
}

xoap::MessageReference PixelTKFECSupervisor::SetDelayEnMass(xoap::MessageReference msg)
{
  Attribute_Vector parametersReceived(4);
    parametersReceived.at(0).name_ = "Delay25Setting";
    parametersReceived.at(1).name_ = "DelayType";
    parametersReceived.at(2).name_ = "Update";
    parametersReceived.at(3).name_ = "Write";
  Receive(msg, parametersReceived);

  std::string TKFECID, SDA = "SDATA", RDA = "RDATA", SCL = "SCLK", RCL = "RCLK", TRG = "TRIG", delayType = parametersReceived.at(1).value_.c_str();
  unsigned int fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, delaySetting = atoi(parametersReceived.at(0).value_.c_str());
  int update = atoi(parametersReceived.at(2).value_.c_str()), write = atoi(parametersReceived.at(3).value_.c_str()); //  flag = 1;
  enumDeviceType modeType = PHILIPS;
  map<std::string, PixelPortCardConfig*>::iterator iter;

  for(iter = mapNamePortCard_.begin(); iter != mapNamePortCard_.end(); iter++){
    TKFECID = iter->second->getTKFECID();
      assert(theTKFECConfiguration_->crateFromTKFECID(TKFECID) == crate_);
    fecAddress = theTKFECConfiguration_->addressFromTKFECID(TKFECID);
    ringAddress = iter->second->getringAddress();
    ccuAddress = iter->second->getccuAddress();
    channelAddress = iter->second->getchannelAddress();

    if(delayType == SDA){
      deviceAddress = iter->second->getdeviceAddressForSetting(k_Delay25_SDA);
    } else if(delayType == RDA){
      deviceAddress = iter->second->getdeviceAddressForSetting(k_Delay25_RDA);
    } else if(delayType == SCL){
      deviceAddress = iter->second->getdeviceAddressForSetting(k_Delay25_SCL);
    } else if(delayType == RCL){
      deviceAddress = iter->second->getdeviceAddressForSetting(k_Delay25_RCL);
    } else if(delayType == TRG){
      deviceAddress= iter->second->getdeviceAddressForSetting(k_Delay25_TRG);
    } else{
      std::cout << "Register:" << delayType << std::endl;
      assert(0);
    }

    assert(fecAccess_ != 0);
    portcardI2CDevice(fecAccess_, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, modeType, delaySetting, 1);

    if(update){
      iter->second->setdeviceValues(deviceAddress, delaySetting);
    }
    if(write){
      iter->second->writeASCII(outputDir_ + "/");
    }
  }

  xoap::MessageReference reply = MakeSOAPMessageReference("SetDelayEnMassDone");
  return reply;
}

xoap::MessageReference PixelTKFECSupervisor::SetAOHGainEnMass (xoap::MessageReference msg) //throw (xoap::exception::Exception)
{
	Attribute_Vector parameters(1);
	parameters[0].name_="AOHGain";
	Receive(msg, parameters);
	const unsigned int AOHGain = atoi(parameters[0].value_.c_str());
	
	//std::cout << "Will set all AOHBias values for modules in the calib object to AOHBias = "<<AOHBias<<".\n";
	
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
        assert(tempCalibObject!=0);

	// List of port cards and addresses to write.  The new values are stored in the PixelPortCardConfig objects.
	std::vector< std::pair< PixelPortCardConfig, std::set<unsigned int> > > newSettings;
	//                      port card                     address

	// Fill the list of port cards and addresses to write.
	const std::set<PixelChannel>& channelsToCalibrate = tempCalibObject->channelList();
	for ( std::set<PixelChannel>::const_iterator channelsToCalibrate_itr = channelsToCalibrate.begin(); channelsToCalibrate_itr != channelsToCalibrate.end(); channelsToCalibrate_itr++ )
	{
		const std::pair< std::string, int > portCardAndAOH = thePortcardMap_->PortCardAndAOH(*channelsToCalibrate_itr);
		const std::string portCardName = portCardAndAOH.first; assert(portCardName!="none");
		
		//ben debugging
		std::cout << "portCardName: " << portCardName << std::endl;

		PixelPortCardConfig* tempPortCard=mapNamePortCard_[portCardName];
		assert(tempPortCard!=0);
		const std::string TKFECID = tempPortCard->getTKFECID();
		if ( theTKFECConfiguration_->crateFromTKFECID(TKFECID) != crate_ ) continue;

		const int AOHNumber = portCardAndAOH.second;
		
		std::map<std::string,PixelPortCardConfig*>::const_iterator mapNamePortCard_itr = mapNamePortCard_.find(portCardName);
		assert( mapNamePortCard_itr != mapNamePortCard_.end() );
		const PixelPortCardConfig* thisPortCardConfig = mapNamePortCard_itr->second;
		
		// Look for this port card in the list of new settings, or add it to the list.
		std::vector< std::pair< PixelPortCardConfig, std::set<unsigned int> > >::iterator portCardToChange;
		bool foundConfig = false;
		for ( std::vector< std::pair< PixelPortCardConfig, std::set<unsigned int> > >::iterator portcard_itr = newSettings.begin(); portcard_itr != newSettings.end(); portcard_itr++ )
		{
			if ( portcard_itr->first.getPortCardName() == thisPortCardConfig->getPortCardName() )
			{
				portCardToChange = portcard_itr;
				foundConfig = true;
				break;
			}
		}
		if ( !foundConfig )
		{
			portCardToChange = newSettings.insert( newSettings.end(), std::make_pair(*thisPortCardConfig, std::set<unsigned int>() ) );
			portCardToChange->first.setPortCardName(portCardName); // ensure that there will be no problems if the name stored in the configuration file doesn't match
		}
		
		// Change the AOH gain and record the address for this AOH.
		portCardToChange->first.setAOHGain(AOHNumber, AOHGain);
		const unsigned int addressToAdd = portCardToChange->first.AOHGainAddressFromAOHNumber(AOHNumber);
		portCardToChange->second.insert(addressToAdd);
	}

	// Write the list of new settings to the hardware.
	for ( std::vector< std::pair< PixelPortCardConfig, std::set<unsigned int> > >::const_iterator portcard_itr = newSettings.begin(); portcard_itr != newSettings.end(); portcard_itr++ )
	{
		const PixelPortCardConfig& portcard = portcard_itr->first;
		const std::set<unsigned int>& addressesOnPortcard = portcard_itr->second;
		for ( std::set<unsigned int>::const_iterator address_itr = addressesOnPortcard.begin(); address_itr != addressesOnPortcard.end(); address_itr++ )
		{
			const unsigned int address = *address_itr;
			const unsigned int value = portcard.getdeviceValuesForAddress(address);
			SetPortCardSetting( &portcard, address, value );
		}
	}

	xoap::MessageReference reply = MakeSOAPMessageReference("SetAOHGainEnMassDone");
	return reply;
}

xoap::MessageReference PixelTKFECSupervisor::SetAOHBiasEnMass (xoap::MessageReference msg) //throw (xoap::exception::Exception)
{
	Attribute_Vector parameters(1);
	parameters[0].name_="AOHBias";
	Receive(msg, parameters);
	const unsigned int AOHBias = atoi(parameters[0].value_.c_str());
	
	//std::cout << "Will set all AOHBias values for modules in the calib object to AOHBias = "<<AOHBias<<".\n";
	
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
        assert(tempCalibObject!=0);

	const set<PixelModuleName> &aModule_string=tempCalibObject->moduleList();

	for (set<PixelModuleName>::iterator aModule_itr = aModule_string.begin(); aModule_itr != aModule_string.end(); ++aModule_itr)
	{


		const std::set< std::pair< std::string, int > > portCardAndAOHsOnThisModule = thePortcardMap_->PortCardAndAOHs(*aModule_itr);
		// Loop over all portcard-AOH pairs attached to this module (either 1 or 2)
		for ( std::set< std::pair< std::string, int > >::const_iterator portCardAndAOH_itr = portCardAndAOHsOnThisModule.begin(); portCardAndAOH_itr != portCardAndAOHsOnThisModule.end(); ++portCardAndAOH_itr )
		{
			std::string portCardName = portCardAndAOH_itr->first;
			const int AOHNumber = portCardAndAOH_itr->second;

			PixelPortCardConfig* tempPortCard=mapNamePortCard_[portCardName];
			if (tempPortCard==0) continue;
			const std::string TKFECID = tempPortCard->getTKFECID();
			if ( theTKFECConfiguration_->crateFromTKFECID(TKFECID) != crate_ ) continue;

			SetAOHBias(portCardName, AOHNumber, AOHBias);
		}
	}

	xoap::MessageReference reply = MakeSOAPMessageReference("SetAOHBiasEnMassDone");
	return reply;
}

xoap::MessageReference PixelTKFECSupervisor::SetAOHBiasOneChannel (xoap::MessageReference msg) 
//throw (xoap::exception::Exception)
{
	Attribute_Vector parameters(3);
	parameters[0].name_="PortCardName";
	parameters[1].name_="AOHNumber";
	parameters[2].name_="AOHBias";
	Receive(msg, parameters);
	const std::string portCardName = parameters[0].value_;
	const unsigned int AOHNumber = atoi(parameters[1].value_.c_str());
	const unsigned int AOHBias   = atoi(parameters[2].value_.c_str());
	
	SetAOHBias(portCardName, AOHNumber, AOHBias);
	
	xoap::MessageReference reply = MakeSOAPMessageReference("SetAOHBiasOneChannelDone");
	return reply;
}

xoap::MessageReference PixelTKFECSupervisor::TKFECCalibrations (xoap::MessageReference msg) //throw (xoap::exception::Exception)
{
   return theTKFECCalibrationBase_->execute(msg);
}


xoap::MessageReference PixelTKFECSupervisor::beginCalibration(xoap::MessageReference msg) //throw (xoap::exception::Exception)
{
  if (theTKFECCalibrationBase_!=0){
    
    return theTKFECCalibrationBase_->beginCalibration(msg);
  }
  return MakeSOAPMessageReference("PixelTKFECSupervisor::beginCalibration Default");
}

xoap::MessageReference PixelTKFECSupervisor::endCalibration(xoap::MessageReference msg) //throw (xoap::exception::Exception)
{
  if (theTKFECCalibrationBase_!=0){  
    return theTKFECCalibrationBase_->endCalibration(msg);
  }
  return MakeSOAPMessageReference("PixelTKFECSupervisor::endCalibration Default");
}


bool PixelTKFECSupervisor::SetAOHBias(std::string portCardName, unsigned int AOHNumber, unsigned int AOHBiasValue)
{
	std::map<std::string,PixelPortCardConfig*>::const_iterator mapNamePortCard_itr = mapNamePortCard_.find(portCardName);
	assert( mapNamePortCard_itr != mapNamePortCard_.end() );
	const PixelPortCardConfig* thisPortCardConfig = mapNamePortCard_itr->second;
	
	unsigned int AOHBiasAddress = thisPortCardConfig->AOHBiasAddressFromAOHNumber(AOHNumber);
	
	return SetPortCardSetting(thisPortCardConfig, AOHBiasAddress, AOHBiasValue);
}

bool PixelTKFECSupervisor::SetPortCardSetting(const PixelPortCardConfig* portCardConfig, unsigned int deviceAddress, unsigned int settingValue)
{
	assert(fecAccess_ != 0);
	const std::string TKFECID = portCardConfig->getTKFECID();
	if ( theTKFECConfiguration_->crateFromTKFECID(TKFECID) != crate_ ) return false;
	const unsigned int TKFECAddress = theTKFECConfiguration_->addressFromTKFECID(TKFECID);
	enumDeviceType modeType = PHILIPS ;
	//int flag = 1;
	
	portcardI2CDevice(fecAccess_, 
			      TKFECAddress, 
			      portCardConfig->getringAddress(), 
			      portCardConfig->getccuAddress(), 
			      portCardConfig->getchannelAddress(), 
			      deviceAddress, 
			      modeType, 
			      settingValue, 
			      1);
	return true;
}

bool PixelTKFECSupervisor::SetPLL_CTR4(const PixelPortCardConfig* portCardConfig, unsigned int settingValue, unsigned int last_PLL_CTR2_value)
{
	SetPortCardSetting( portCardConfig, portCardConfig->getdeviceAddressForSetting(PortCardSettingNames::k_PLL_CTR2), portCardConfig->new_PLL_CTR2_value(PortCardSettingNames::k_PLL_CTR4, last_PLL_CTR2_value) );
	return SetPortCardSetting( portCardConfig, portCardConfig->getdeviceAddressForSetting(PortCardSettingNames::k_PLL_CTR4or5), settingValue );
}

bool PixelTKFECSupervisor::SetPLL_CTR5(const PixelPortCardConfig* portCardConfig, unsigned int settingValue, unsigned int last_PLL_CTR2_value)
{
	SetPortCardSetting( portCardConfig, portCardConfig->getdeviceAddressForSetting(PortCardSettingNames::k_PLL_CTR2), portCardConfig->new_PLL_CTR2_value(PortCardSettingNames::k_PLL_CTR5, last_PLL_CTR2_value) );
	return SetPortCardSetting( portCardConfig, portCardConfig->getdeviceAddressForSetting(PortCardSettingNames::k_PLL_CTR4or5), settingValue );
}

//---------------------------------------------------------------------------

bool PixelTKFECSupervisor::checkForResets(toolbox::task::WorkLoop *w1) {
  static bool printStatus = true;
  static bool performReset = false;

/* jmt -- new code to be tested; to slow down execution of this loop
  for (unsigned int ii=0; ii<60; ii++) {
    //exit quickly if desired
    rclock_->take(); if (workloopContinueRC_) rclock_->give();  else {rclock_->give(); return true;}
    ::sleep(1);
  }
*/

// Just slow the readout frequence
  static int wait = 0;
  wait++;
  if(wait!=100) {::sleep(1); return true;}
  else {wait=0;}

  printStatus = false;

  //exit quickly if desired
  rclock_->take(); if (workloopContinueRC_) rclock_->give();  else {rclock_->give(); return true;}

  ::sleep(1); //maybe want to make this shorter b/c we could be wasting up to 5 sec here

  //in case cancel came while we were sleeping
  rclock_->take(); if (workloopContinueRC_) rclock_->give();  else {rclock_->give(); return true;}
  //return true so that cancel() never fails

  //#ifdef PRINT_MORE
  cout<<"Running [checkForResets] "<<wait<<endl;
  //  return true;
  //#endif

  try {

    hardwarelock_->take(); //concurrent threads accessing both hardware _and_ mapNamePortCard
    //loop over portcards
    map<string, PixelPortCardConfig*>::const_iterator iter=mapNamePortCard_.begin();
    
    //const int  flag=1;
    enumDeviceType modeType = PHILIPS ;
    bool ringInit[8] = {false,false,false,false,false,false,false,false}; // has the ring been reset
    
    //loop over portcards
    for ( ; iter != mapNamePortCard_.end() ; ++iter ) {
      
      bool status = true;
      const string TKFECID = iter->second->getTKFECID();
      if ( theTKFECConfiguration_->crateFromTKFECID(TKFECID) != crate_ ) continue;
      
      const unsigned int tkfecAddress=theTKFECConfiguration_->addressFromTKFECID(TKFECID);
      const unsigned int ringAddress=iter->second->getringAddress();
      const unsigned int ccuAddress=iter->second->getccuAddress();
      const unsigned int channelAddress=iter->second->getchannelAddress();

#ifdef DO_READCCU
      //send reset, check if ring has been reset alerady 
      try {

	// read ccu register
	keyType index = buildCompleteKey(tkfecAddress,ringAddress,ccuAddress,0,0) ;
	int tmp = int(fecAccess_->getCcuSRF(index));  // added 19.09.12 d.k.
	if(tmp!=0) {suppressHardwareError_ = false; cout<<"CCU SRF = 0x"<<hex<<tmp<<dec<<endl;}

	//if( printStatus || !suppressHardwareError_ ) {
	if( !suppressHardwareError_ || do_force_ccu_readout ) {

	  cout<<"mfec/ccu/channel "<<hex<<ringAddress<<"/"<<ccuAddress<<"/"
	      <<channelAddress<<dec<<endl;
	  
	  if( ringAddress<=8 && ringAddress>=1 && !ringInit[(ringAddress-1)] ) {  // do only once per ring  go from 1-8
	    
#ifdef DO_RESET
	    if(performReset) {
	      cout<<" After error reset slot/mfec "<<tkfecAddress<<"/"<<ringAddress<<endl;
	      resetPlxFec ( fecAccess_, tkfecAddress, ringAddress, 1, 0 ) ;
	      performReset=false;
	    }
#endif // DO_RESET	    

	    //readFecRegisterSR0 ( fecAccess_, tkfecAddress, ringAddress, 1, 0 ) ; // does all rings, fec&rinf address ignored
	    keyType indexFEC = buildFecRingKey(tkfecAddress,ringAddress) ;
	    //fecAccess_->getFecRingSR0(indexFEC) ;

	    tscType16 fecSR0 = fecAccess_->getFecRingSR0(indexFEC);
	    tscType16 fecSR1 = fecAccess_->getFecRingSR1(indexFEC);
	    cout<<" FEC SR0 "<<hex<<fecSR0<<" SR1 "<<fecSR1<<dec<<endl;

	  } // once per ring
	  	  
	  printCCUCRE(tkfecAddress, ringAddress,ccuAddress);

#ifdef DO_READPIA
	  // Read the PIA registers 
	  //try {
	    enablePIAchannels(tkfecAddress,ringAddress,ccuAddress);
	    printPIAinfo(tkfecAddress,ringAddress,ccuAddress,0x30);
#ifdef MYTEST
	    // check the onused port 0x33
	    cout<<" PIA info: ring "<<ringAddress<<hex<<" ccu "<<ccuAddress<<" I2C-channel "<<channelAddress<<" PIA-channel 0x33"
		<<dec<<endl;
	    printPIAinfo(tkfecAddress,ringAddress,ccuAddress,0x33);
#endif // MYTEST
	    disablePIAchannels(tkfecAddress,ringAddress,ccuAddress);
	    //} catch (FecExceptionHandler & e) {
	    //cout<<" Error in PIA readout: slot "<<tkfecAddress<<" ring "<<ringAddress<<" ccu "<<ccuAddress<<endl;
	    //}
#endif // DO_READPIA


	  // Read the I2C control register (reset will disable it), gives error after reset
#ifndef DO_RESET
	  printI2Cinfo(tkfecAddress,ringAddress,ccuAddress,channelAddress);
#endif //DO_RESET

	  ringInit[(ringAddress-1)]=true;
	  
	} // is supppress
	
      } catch (FecExceptionHandler & e) {
	  
	diagService_->reportError("Caught hardware exception in resetCheck loop: "+string(e.what()),DIAGWARN);
	
	suppressHardwareError_=false;
	printStatus=true;	
	//if (hardwarelock_->value()==toolbox::BSem::EMPTY) hardwarelock_->give();
	
      } // end catch


#endif // DO_READCCU
      
      unsigned int deviceAddress=0;
      unsigned int readvalue =  0;

      // Read back the DELAY25
      if(0) {
	deviceAddress=iter->second->getdeviceAddressForSetting(PortCardSettingNames::k_Delay25_SDA);

	try {
	  readvalue =  portcardI2CDeviceRead(fecAccess_, tkfecAddress, ringAddress,ccuAddress,channelAddress,
					   deviceAddress,  modeType,  1);

	} catch (FecExceptionHandler & e) {
	  
	  diagService_->reportError("Caught hardware exception in resetCheck loop: "+string(e.what()),DIAGWARN);
	
	  suppressHardwareError_=false;
	  printStatus=true;	  
	  //if (hardwarelock_->value()==toolbox::BSem::EMPTY) hardwarelock_->give();
	
	}
	
	//ideally we will add flags so that output only comes once
	if (!( readvalue & 0x40 )) {
	  ostringstream errmsg;
	  errmsg<<"Problem with Delay25_SDA in tkfec=0x"<<hex<<tkfecAddress<<dec<<" ring=0x"<<hex<<ringAddress<<dec
		<<" ccu=0x"<<hex<<ccuAddress<<dec<<" channel=0x"<<hex<<channelAddress<<dec<<" value=0x"<<hex<<readvalue<<dec;
	  diagService_->reportError(errmsg.str(),DIAGERROR);
	  status = false;
	}
	//uncomment for debugging
	//      else { cout<<"Everything is ok! Delay25_SDA=0x"<<hex<<readvalue<<dec<<endl;    }
      }  // READ DELAY25


      // Readback the PLL
      //FIXME -- danek says:
      /*
	Coming back to the PLL testing, the register/bit you are looking at is always on I think.
	I would be better to look at register  CTR2 and look at bit 5 so mask 0x20.
	It should be on after configure and I think it goes to 0 after a reset. 
      */

      bool readoutError = false;
      readvalue = 0;

      //this is easy to implement, but should be tested at the TIF before actually implementing it
      deviceAddress=iter->second->getdeviceAddressForSetting(PortCardSettingNames::k_PLL_CTR1);

      try {
	readvalue =  portcardI2CDeviceRead(fecAccess_, tkfecAddress, ringAddress,ccuAddress,channelAddress,
					   deviceAddress,  modeType,  1);
      } catch (FecExceptionHandler & e) {
	
	// uncomment the following line if you want the hardware exception to be printed. md 17-07-2012
	// diagService_->reportError("Caught hardware exception in resetCheck loop: "+string(e.what()),DIAGWARN);
	
	suppressHardwareError_=false;
	printStatus=true;	  
	readoutError = true;
	performReset = true;
	//if (hardwarelock_->value()==toolbox::BSem::EMPTY) hardwarelock_->give();
	
      }

#ifdef PRINT_MORE
      cout<<"1 ";
      if (readvalue != 0x1 ) cout<<"For PLL CTR1 in tkfec=0x"<<hex<<tkfecAddress<<dec<<" ring=0x"<<hex<<ringAddress<<dec
				 <<" ccu=0x"<<hex<<ccuAddress<<dec<<" channel=0x"<<hex<<channelAddress<<" value "
				 <<readvalue<<dec<<endl;
#endif

      // If readout OK check the PLL values 
      if (!readoutError) {
	if( !( readvalue & 0x1 ) ) {

	  // skip the bad fpix channel
	  if(tkfecAddress==0x12 && ringAddress==0x6 && ccuAddress==0x7e && channelAddress==0x11 ) {
	    cout<<"Problem with PLL CTR1 in tkfec=0x"<<hex<<tkfecAddress<<" ring=0x"<<ringAddress
		<<" ccu=0x"<<ccuAddress<<" channel=0x"<<channelAddress<<" value "<<readvalue<<dec<<endl;
	  } else {
	    ostringstream errmsg;
	    errmsg<<"Problem with PLL CTR1 in tkfec=0x"<<hex<<tkfecAddress<<dec<<" ring=0x"<<hex<<ringAddress<<dec
		  <<" ccu=0x"<<hex<<ccuAddress<<dec<<" channel=0x"<<hex<<channelAddress<<" value "<<readvalue<<dec;
	    diagService_->reportError(errmsg.str(),DIAGWARN);
	    
	    status = false;
	  } // end if 

	}
	
	if ( ( readvalue & 0x8 ) ) {
	  ostringstream errmsg;
	  errmsg<<"Maybe? a SEU in  PLL in tkfec=0x"<<hex<<tkfecAddress<<dec<<" ring=0x"<<hex<<ringAddress<<dec
		<<" ccu=0x"<<hex<<ccuAddress<<dec<<" channel=0x"<<hex<<channelAddress<<" value "<<readvalue<<dec;
	  diagService_->reportError(errmsg.str(),DIAGWARN);
	  status = false;
	}
      } // if readout error 



#ifdef DO_READCCU  // READ CCU I2C registers 
      // Check CCU registers in case of error
      if(!status || readoutError || do_force_ccu_readout) {
	cout<<" After error (or forced) "<<endl;
	keyType indexFEC = buildFecRingKey(tkfecAddress,ringAddress) ;
	//fecAccess_->getFecRingSR0(indexFEC) ;

	tscType16 fecSR0 = fecAccess_->getFecRingSR0(indexFEC);
	tscType16 fecSR1 = fecAccess_->getFecRingSR1(indexFEC);
	cout<<" FEC SR0 "<<hex<<fecSR0<<" SR1 "<<fecSR1<<dec<<endl;

	//tscType32 fecSR0 = fecAccess_->getFecRingSR0(indexFEC);
	//cout<<" FEC SR0 "<<hex<<fecSR0<<endl;      

	printCCUCRE(tkfecAddress, ringAddress,ccuAddress);
	printI2Cinfo(tkfecAddress, ringAddress,ccuAddress,channelAddress);
	printStatus = true;	
      }  // if status
#endif  // DO_READCCU

      //uncomment for debugging
      //      else {cout<<"Everything is ok! PLL CTR1=0x"<<hex<<readvalue<<dec<<endl;  }

    } //  portcard loop 

#ifdef PRINT_MORE
    cout<<endl;
#endif

    if(printStatus) suppressHardwareError_= false;
    else suppressHardwareError_= true;

    //printStatus = false;
    hardwarelock_->give();

  } catch (FecExceptionHandler & e) {

    diagService_->reportError("Caught hardware exception in resetCheck loop: "+string(e.what()),DIAGWARN);

    suppressHardwareError_=false;
    //printStatus=true;
    if (hardwarelock_->value()==toolbox::BSem::EMPTY) hardwarelock_->give();

  }  // big try-catch 

  return true;
}

//=============================================================================================

void PixelTKFECSupervisor::readTemp ()
{
  cout<<endl<< "//--> PixelTKFECSupervisor::readTemp" <<endl;
  
  bool toreturn = false;
  //bool toreturn = true;
  if (toreturn) {
    cout<< "--> return from PixelTKFECSupervisor::readTemp" <<endl;
    return;
  }

  map<string, PixelPortCardConfig*>::iterator iter=mapNamePortCard_.begin();
	   
  assert(iter!=mapNamePortCard_.end());

  std::string TKFECID = iter->second->getTKFECID();
  assert( theTKFECConfiguration_->crateFromTKFECID(TKFECID) == crate_ );
  unsigned int fecAddress=theTKFECConfiguration_->addressFromTKFECID(TKFECID);
  unsigned int ringAddress=iter->second->getringAddress();
  unsigned int ccuAddress=iter->second->getccuAddress();
  unsigned int channelAddress=iter->second->getchannelAddress();
  unsigned int deviceAddress;
  
  cout<< "fecAddress      = " << fecAddress <<endl;
  cout<< "ringAddress     = " << ringAddress <<endl;
  cout<< "ccuAddress      = " << ccuAddress <<endl;
  cout<< "channelAddress  = " << channelAddress <<endl;
  
  assert(fecAccess_!=0);
	
  //int  flag=1;
  enumDeviceType modeType = PHILIPS ;

  for(unsigned int repeat=0;repeat<10;repeat++){

    for(unsigned int channel=0;channel<8;channel++){
      
      deviceAddress=0x50;

      unsigned int deviceValues=0x88|channel;
      
      portcardI2CDevice(fecAccess_, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, modeType, deviceValues, 1);
      
      deviceAddress=0x53;
      unsigned int lowbits=portcardI2CDeviceRead(fecAccess_, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, modeType, 1);
      
      deviceAddress=0x51;
      unsigned int highbits=portcardI2CDeviceRead(fecAccess_, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, modeType, 1);
      
      unsigned int temp=lowbits+((highbits&0xf)<<8);
  
      std::cout << "lowbits :"<<std::hex<<lowbits<<std::dec<<std::endl;
      std::cout << "highbits:"<<std::hex<<highbits<<std::dec<<std::endl;
      std::cout << "Channel Temp    :"<<channel<<" "<<temp<<std::endl;
  
    }
  }

  cout<<endl<< "--> try dcuAccess" <<endl;
  
  cout<< "mapNamePortCard_.size() = " << mapNamePortCard_.size() <<endl;
  for (map<string, PixelPortCardConfig*>::iterator portcard=mapNamePortCard_.begin(); portcard!=mapNamePortCard_.end(); ++portcard)
  {
    unsigned iPortCard = std::distance(mapNamePortCard_.begin(), portcard);
    
    std::string TKFECID = iter->second->getTKFECID();
    assert( theTKFECConfiguration_->crateFromTKFECID(TKFECID) == crate_ );
    unsigned int fecAddress=theTKFECConfiguration_->addressFromTKFECID(TKFECID);
    unsigned int ringAddress=portcard->second->getringAddress();
    unsigned int ccuAddress=portcard->second->getccuAddress();
    unsigned int channelAddress=portcard->second->getchannelAddress();
    
    //   dcuAccess (FecAccess *fec,
    // 	     tscType16 fecSlot,
    // 	     tscType16 ringSlot,
    // 	     tscType16 ccuAddress,
    // 	     tscType16 i2cChannel,
    // 	     tscType16 i2cAddress,
    // 	     std::string dcuType = DCUFEH ) ;
    dcuAccess dcu(fecAccess_, fecAddress, ringAddress, ccuAddress, channelAddress, 0x50);
    
    unsigned cregLIR = 0x88;
    unsigned cregHIR = 0x80;
    
    cout<< "LIR" <<endl;
    for (unsigned channel=0; channel<8; ++channel) {
      dcu.setDcuCREG(cregLIR | channel);
      unsigned highbits = dcu.getDcuSHREG();
      unsigned lowbits  = dcu.getDcuLREG();

      unsigned temp = lowbits + ((highbits & 0xf) << 8);
      cout << "--> iPortCard = " << iPortCard << " dcuAccess: Channel Temp    :" << channel << " " <<temp <<endl;
    }
    
    cout<< "HIR" <<endl;
    for (unsigned channel=0; channel<8; ++channel) {
      dcu.setDcuCREG(cregHIR | channel);
      unsigned highbits = dcu.getDcuSHREG();
      unsigned lowbits  = dcu.getDcuLREG();

      unsigned temp = lowbits + ((highbits & 0xf) << 8);
      cout << "--> iPortCard = " << iPortCard << " dcuAccess: Channel Temp    :" << channel << " " <<temp <<endl;
    }
  }

  bool test_file = true;
  if (test_file)
  {
    ofstream ofile_dcu("dcu.dat");
     
    cout<<endl<< "--> start filling test file" <<endl;
    tscType16 Vaa_ch    = 0;
    tscType16 Vdd_ch    = 1;
    tscType16 rtd2_ch   = 2;
    tscType16 rtd3_ch   = 3;
    tscType16 aoh_ch    = 4;     // AOH RTD
    tscType16 Vpc_ch    = 5;     // Port card Vpower
    tscType16 ch6_ch    = 6;     // unknown
    tscType16 ts_ch     = 7;     // DCU diode-based Temperature Sensor
    
    // We are interested in rtd2 and rtd3. The Vpc will be used for normalizing. So sequence of readout channels will be:
    // rtd2_ch
    // Vpc_ch
    // rtd3_ch
    // Vaa_ch
    // ts_ch
    // Vdd_ch
    // aoh_ch
    // ch6_ch
    
    map<string, PixelPortCardConfig*>::iterator portcard=mapNamePortCard_.begin();
    // For current setup of the Pilot Run Detector only the second Port Card has rtd2 and rtd3 switched ON
    ++portcard;
    assert(portcard != mapNamePortCard_.end());
    

    std::string TKFECID = iter->second->getTKFECID();
    assert( theTKFECConfiguration_->crateFromTKFECID(TKFECID) == crate_ );
    unsigned int fecAddress=theTKFECConfiguration_->addressFromTKFECID(TKFECID);
    tscType16 ringAddress=portcard->second->getringAddress();
    tscType16 ccuAddress=portcard->second->getccuAddress();
    tscType16 channelAddress=portcard->second->getchannelAddress();
    tscType16 i2cAddress = 0x50;                 // for DCU on the PortCard
    //   dcuAccess (FecAccess *fec,
    // 	     tscType16 fecSlot,
    // 	     tscType16 ringSlot,
    // 	     tscType16 ccuAddress,
    // 	     tscType16 i2cChannel,
    // 	     tscType16 i2cAddress,
    // 	     std::string dcuType = DCUFEH ) ;
    dcuAccess dcu(fecAccess_, fecAddress, ringAddress, ccuAddress, channelAddress, i2cAddress);
    
    // to set CREG for A to C conversion
    // bits 0..2(CHANNEL):        channel
    // bit 3 (POLARITY):          1 for Low Input Range, 0 for High Input Range. Looks, we will use LIR for real data.
    // bits 4(TSON) and 5(HIRES): should be 0
    // bit 6(RESET):              0 for our purpose
    // bit 7(START):              1: start A to D conversion
    tscType16 creg_base = 0x80;
    tscType16 bit_HIR = 0x0;
    tscType16 bit_LIR = 0x8;
    
    cout<< "sizeof(unsigned) = " << sizeof(unsigned) <<endl;
    
    tscType16 creg_LIR = creg_base | bit_LIR;
    tscType16 creg_HIR = creg_base | bit_HIR;
    cout<< hex << "creg_LIR = 0x" << creg_LIR << " creg_HIR = 0x" << creg_HIR << dec <<endl;
    
    tscType16 creg;
    
    //--int npoints = 1000000;
    int npoints = 1000;
    for (int ipoint=0; ipoint<npoints; ++ipoint)
    {
      if (ipoint % 10000 == 0) cout<< "point " << ipoint <<endl;
      // LIR mode
      creg = creg_LIR;    // need to be ORed with the channel
      
      dcu.setDcuCREG(creg | rtd2_ch);
      tscType16 rtd2_LIR = dcu.getDcuLREG() + ((dcu.getDcuSHREG() & 0xf) << 8);
      dcu.setDcuCREG(creg | Vpc_ch);
      tscType16 Vpc_LIR = dcu.getDcuLREG() + ((dcu.getDcuSHREG() & 0xf) << 8);
      dcu.setDcuCREG(creg | rtd3_ch);
      tscType16 rtd3_LIR = dcu.getDcuLREG() + ((dcu.getDcuSHREG() & 0xf) << 8);
      dcu.setDcuCREG(creg | Vaa_ch);
      tscType16 Vaa_LIR = dcu.getDcuLREG() + ((dcu.getDcuSHREG() & 0xf) << 8);
      dcu.setDcuCREG(creg | ts_ch);
      tscType16 ts_LIR = dcu.getDcuLREG() + ((dcu.getDcuSHREG() & 0xf) << 8);
      dcu.setDcuCREG(creg | Vdd_ch);
      tscType16 Vdd_LIR = dcu.getDcuLREG() + ((dcu.getDcuSHREG() & 0xf) << 8);
      dcu.setDcuCREG(creg | aoh_ch);
      tscType16 aoh_LIR = dcu.getDcuLREG() + ((dcu.getDcuSHREG() & 0xf) << 8);
      dcu.setDcuCREG(creg | ch6_ch);
      tscType16 ch6_LIR = dcu.getDcuLREG() + ((dcu.getDcuSHREG() & 0xf) << 8);

      // HIR mode
      creg = creg_HIR;    // need to be ORed with the channel

      dcu.setDcuCREG(creg | rtd2_ch);
      tscType16 rtd2_HIR = dcu.getDcuLREG() + ((dcu.getDcuSHREG() & 0xf) << 8);
      dcu.setDcuCREG(creg | Vpc_ch);
      tscType16 Vpc_HIR = dcu.getDcuLREG() + ((dcu.getDcuSHREG() & 0xf) << 8);
      dcu.setDcuCREG(creg | rtd3_ch);
      tscType16 rtd3_HIR = dcu.getDcuLREG() + ((dcu.getDcuSHREG() & 0xf) << 8);
      dcu.setDcuCREG(creg | Vaa_ch);
      tscType16 Vaa_HIR = dcu.getDcuLREG() + ((dcu.getDcuSHREG() & 0xf) << 8);
      dcu.setDcuCREG(creg | ts_ch);
      tscType16 ts_HIR = dcu.getDcuLREG() + ((dcu.getDcuSHREG() & 0xf) << 8);
      dcu.setDcuCREG(creg | Vdd_ch);
      tscType16 Vdd_HIR = dcu.getDcuLREG() + ((dcu.getDcuSHREG() & 0xf) << 8);
      dcu.setDcuCREG(creg | aoh_ch);
      tscType16 aoh_HIR = dcu.getDcuLREG() + ((dcu.getDcuSHREG() & 0xf) << 8);
      dcu.setDcuCREG(creg | ch6_ch);
      tscType16 ch6_HIR = dcu.getDcuLREG() + ((dcu.getDcuSHREG() & 0xf) << 8);

      ofile_dcu
        <<setw(8)<< Vaa_LIR <<setw(8)<< Vaa_HIR
        <<setw(8)<< Vdd_LIR <<setw(8)<< Vdd_HIR
        <<setw(8)<< rtd2_LIR <<setw(8)<< rtd2_HIR
        <<setw(8)<< rtd3_LIR <<setw(8)<< rtd3_HIR
        <<setw(8)<< aoh_LIR <<setw(8)<< aoh_HIR
        <<setw(8)<< Vpc_LIR <<setw(8)<< Vpc_HIR
        <<setw(8)<< ch6_LIR <<setw(8)<< ch6_HIR
        <<setw(8)<< ts_LIR <<setw(8)<< ts_HIR
      <<"\n";
      //<<endl;
    }
    ofile_dcu.close();
    
    cout<< "--> finish filling test file" <<endl<<endl;
  }
}

xoap::MessageReference PixelTKFECSupervisor::ReadDCU_workloop_fakeSOAP (xoap::MessageReference msg)	// method to test exception handling
{

  cout<< "// PixelTKFECSupervisor::ReadDCU_workloop_fakeSOAP" <<endl;
  
  usleep(readDCU_workloop_usleep_);
  
  std::ostringstream diagMessage;
  diagMessage << "<PixelTKFECSupervisor " << this->getApplicationDescriptor()->getInstance() << " ::ReadDCU_workloop_SOAP_exception>:";
  //diagService_->reportError(diagMessage.str(), DIAGINFO);

	//
	// This section has been taken from PixelTKFECSupervisor::Initialize
	//
  // Detect PixelDCStoTKFECDpInterface
  try {
    PixelDCStoTKFECDpInterface_ = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor("PixelDCStoTrkFECDpInterface", 0);
    std::cout<<"// PixelTKFECSupervisor::ReadDCU_workloop_fakeSOAP: while doing the same as PixelTKFECSupervisor::Initialize - Instance 0 of PixelDCStoTKFECInterface found."<<std::endl;
  } catch (xdaq::exception::Exception& e) {
    std::cout<<"// PixelTKFECSupervisor::ReadDCU_workloop_fakeSOAP: while doing the same as PixelTKFECSupervisor::Initialize - Instance 0 of PixelDCStoTKFECInterface not found!"<<std::endl;
  }

  if (PixelDCStoTKFECDpInterface_!=0) {
  	// call test version of the readDCU
    xoap::MessageReference soapRequest = readDCU_fakeSOAP(MakeSOAPMessageReference("updateDpValue"));  
    std::string soapResponse = "";
    try {
      soapResponse = Send(PixelDCStoTKFECDpInterface_, soapRequest);		// successfully works
      // soapResponse = Send(0, soapRequest);		// intentially set pointer to 0 to test exception handling
    }
    catch  (xdaq::exception::Exception& e) {
      diagService_->reportError("//-- PixelTKFECSupervisor::ReadDCU_workloop_fakeSOAP: Caught exception -- Failed to post SOAP to PixelDCStoTKFECDpInterface",DIAGWARN);
    }
    if ( soapResponse != "updateDpValueDone" ) {
      std::cerr << "<PixelTKFECSupervisor " << this->getApplicationDescriptor()->getInstance() << " ::ReadDCU_workloop_fakeSOAP>:"
		<< " Failed to send DCU temperatures to PixelDCStoTKFECDpInterface" << std::endl;
      //diagService_->reportError("Failed to send DCU temperatures to PixelDCStoTKFECDpInterface", DIAGDEBUG);
    }
		else {
			cout<< "PixelTKFECSupervisor::ReadDCU_workloop_fakeSOAP: successfully sent SOAP mess to PixelDCStoTKFECDpInterface" <<endl;
		}
  }
	else {
		cout<< "PixelTKFECSupervisor::ReadDCU_workloop_fakeSOAP: PixelDCStoTKFECDpInterface_ == 0" <<endl;
	}
  
  return MakeSOAPMessageReference("ReadDCU_workloop_fakeSOAP_done");
}

bool PixelTKFECSupervisor::ReadDCU_workloop (toolbox::task::WorkLoop *w1)
{
  // ENABLE-DISABLE DCU READOUT
#ifndef READ_DCU
  usleep(readDCU_workloop_usleep_);  // aad sleep to prevent 100% cpu, d.k. 19/8/11
  return true; //remove this line to enable dcu readout - BEN 2011 05 26
#endif

  //std::ostringstream diagMessage;
  //diagMessage << "<PixelTKFECSupervisor " << this->getApplicationDescriptor()->getInstance() << " ::ReadDCU_workloop>:";
  //diagService_->reportError(diagMessage.str(), DIAGINFO);

  if (PixelDCStoTKFECDpInterface_!=0) {
    //exit quickly if desired ; return true because we don't want cancel() to fail
    //i'm not sure that returning true is the correct strategy. but it is certainly safer
    dculock_->take(); if (workloopContinue_) dculock_->give();  else {dculock_->give(); return true;}

    //    if (dcudebug_) diagService_->reportError("[ReadDCU_workloop] about to do enter readDCU()",DIAGTRACE);

    //this step can take a lot of time!
    xoap::MessageReference soapRequest = readDCU(MakeSOAPMessageReference("updateDpValue"));  

    //    if (dcudebug_) diagService_->reportError("[ReadDCU_workloop] done with readDCU()",DIAGTRACE);

    dculock_->take(); if (workloopContinue_) dculock_->give();  else {dculock_->give(); return true;}

    //    if (dcudebug_) diagService_->reportError("[ReadDCU_workloop] about to send SOAP to PixelDCStoTKFECDpInterface",DIAGTRACE);
    std::string soapResponse = "";
    try {
      soapResponse = Send(PixelDCStoTKFECDpInterface_, soapRequest);
    }
    catch  (xdaq::exception::Exception& e) {
      diagService_->reportError("Caught exception -- Failed to post SOAP to PixelDCStoTKFECDpInterface",DIAGWARN);
    }
    if ( soapResponse != "updateDpValueDone" ) {
      std::cerr << "<PixelTKFECSupervisor " << this->getApplicationDescriptor()->getInstance() << " ::readDCU>:"
		<< " Failed to send DCU temperatures to PixelDCStoTKFECDpInterface" << std::endl;
      //diagService_->reportError("Failed to send DCU temperatures to PixelDCStoTKFECDpInterface", DIAGDEBUG);
    }
    //    if (dcudebug_) diagService_->reportError("[ReadDCU_workloop] done sending SOAP to PixelDCStoTKFECDpInterface",DIAGTRACE);
  }

  usleep(readDCU_workloop_usleep_);

  return true; // ensures that this goes on forever unless the WorkLoop is cancelled!

}

xoap::MessageReference PixelTKFECSupervisor::ResetCCU(xoap::MessageReference msg) //throw (xoap::exception::Exception) 
{
  diagService_->reportError("Entering ResetCCU",DIAGDEBUG);

  if (0==fecAccess_) {
    cout<<"[PixelTKFECSupervisor] Cannot get pointer to FEC"<<endl;
    return MakeSOAPMessageReference("ResetCCUFailed");
  }

  try {
    for (map<string, PixelPortCardConfig*>::iterator portcard=mapNamePortCard_.begin(); portcard!=mapNamePortCard_.end(); ++portcard)
      {
	
	string TKFECID = portcard->second->getTKFECID();
	if (theTKFECConfiguration_->crateFromTKFECID(TKFECID) != crate_ )  continue;
	const unsigned int fecAddress=theTKFECConfiguration_->addressFromTKFECID(TKFECID);
	const unsigned int ringAddress=portcard->second->getringAddress();
	resetPlxFec ( fecAccess_, fecAddress, ringAddress, 1, 0 ) ;
      }
  }
  catch (...) {
      diagService_->reportError("Caught an exception while trying to reset the CCU!",DIAGERROR);
  }
  
  diagService_->reportError("Exiting ResetCCU",DIAGDEBUG);
  return MakeSOAPMessageReference("ResetCCUResponse");
}

xoap::MessageReference PixelTKFECSupervisor::readDCU(xoap::MessageReference msg) //throw (xoap::exception::Exception)
{
  if (debug_) cout<<endl<< "PixelTKFECSupervisor::readDCU" <<endl;
  xoap::MessageReference soapMessage;

  try { //hardware access
    //prevent simultaneous accesses to hardware
    hardwarelock_->take();

  PortCard::DCU::Mode mode = PortCard::DCU::LIR;
  
  // extract DCU mode from the SOAP message
  
  xoap::SOAPEnvelope envelope = msg->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName modeName = envelope.createName(PortCard::DCU::name_modeDCU);
  
  std::vector<xoap::SOAPElement> bodyElements = body.getChildElements(modeName);
  //cout << "// PixelTKFECSupervisor::readDCU: Number of BodyElements = " << bodyElements.size() << endl;
  
  if (bodyElements.size() > 0) {
    xoap::SOAPName valueName = envelope.createName(PortCard::DCU::name_modeValue);
    std::string modeStr = bodyElements.at(0).getAttributeValue(valueName);
    if (modeStr == PortCard::DCU::name_modeLIR) mode = PortCard::DCU::LIR;
    else                                        mode = PortCard::DCU::HIR;
    //cout<< "// PixelTKFECSupervisor::readDCU: modeStr = " << modeStr << " mode = " << mode <<endl;
  }
  //   for (std::vector<xoap::SOAPElement>::iterator p_bodyElement = bodyElements.begin(); p_bodyElement != bodyElements.end(); ++p_bodyElement)
  //   {
  //     cout<< "// PixelTKFECSupervisor::readDCU: vector element " << std::distance(bodyElements.begin(), p_bodyElement) << " BodyElement name = " << p_bodyElement->getElementName().getQualifiedName() <<endl;
  //     //cout<< "// PixelTKFECSupervisor::readDCU: BodyElement name = " << p_bodyElement->getElementName().getQualifiedName() <<endl;
  //     
  //     xoap::SOAPName valueName = envelope.createName("modeValue");
  //     std::string modeStr = p_bodyElement->getAttributeValue(valueName);
  //     cout<< "// PixelTKFECSupervisor::readDCU: modeStr = " << modeStr <<endl;
  //   }
    
  std::vector<PortCard::AddressDCU> vdcu;
  std::string strBPix = "BPix";

  if (debug_) {
    time_t rawtime;                   // time_t: the number of seconds elapsed since 00:00 hours, Jan 1, 1970 UTC
    struct tm* timeinfo;

    time(&rawtime);                   // Get the current calendar time as a time_t object
    timeinfo = localtime(&rawtime);   // represents time_t value as a local time

    cout<< " Time: " << asctime(timeinfo);   // format: Wed Oct  7 19:09:34 2009 NB: asciitime inserts carrige return ('\n') into the string!
  }
  
  //cout<< "PixelTKFECSupervisor::readDCU: mapNamePortCard_.size() = " << mapNamePortCard_.size() <<endl;
  for (map<string, PixelPortCardConfig*>::iterator portcard=mapNamePortCard_.begin(); portcard!=mapNamePortCard_.end(); ++portcard)
  {
    //exit quickly if desired
    dculock_->take();
    if (workloopContinue_) dculock_->give();  else {dculock_->give(); hardwarelock_->give();  
      //      if (dcudebug_) diagService_->reportError("[readDCU] got DCU workloop cancel inside of readDCU()",DIAGTRACE);
      return MakeSOAPMessageReference("DCUWorkloopCancel");}

    std::string portcard_name = portcard->first;
    
    unsigned iPortCard = std::distance(mapNamePortCard_.begin(), portcard);
    if (debug_) cout<< "PixelTKFECSupervisor::readDCU: reading Port Card #" << iPortCard << " with name " << portcard->first <<endl;
    
    // make sure this is not BPix
    if (portcard->first.find(strBPix,0) != std::string::npos) continue;
    
    std::string TKFECID = portcard->second->getTKFECID();
    assert( theTKFECConfiguration_->crateFromTKFECID(TKFECID) == crate_ );
    unsigned int fecSlot=theTKFECConfiguration_->addressFromTKFECID(TKFECID);
    unsigned int ringSlot = portcard->second->getringAddress();
    unsigned int ccuAddress = portcard->second->getccuAddress();
    unsigned int i2cChannel = portcard->second->getchannelAddress();
    unsigned int i2cAddress = 0x50;           // hardwared by design: address of control register CREG
    
    if (debug_) cout<< "PixelTKFECSupervisor::readDCU: before dcuAccess: TKFECID = " << TKFECID << " fecSlot = " << fecSlot << " ringSlot = " << ringSlot << " ccuAddress = " << ccuAddress << " i2cChannel = " << i2cChannel <<endl;
    
    //   dcuAccess (FecAccess *fec,
    // 	     tscType16 fecSlot,
    // 	     tscType16 ringSlot,
    // 	     tscType16 ccuAddress,
    // 	     tscType16 i2cChannel,
    // 	     tscType16 i2cAddress,
    // 	     std::string dcuType = DCUFEH ) ;
    dcuAccess dcuaccess(fecAccess_, fecSlot, ringSlot, ccuAddress, i2cChannel, i2cAddress);
    
    if (debug_) cout<< "PixelTKFECSupervisor::readDCU:  after dcuAccess" <<endl;
    
    //
    //  data to be returned through soap message
    //
    unsigned adc[8];
    
    unsigned cregLIR = 0x88;
    unsigned cregHIR = 0x80;
    
    for (unsigned channel=0; channel<8; ++channel) {
      //if (mode == PortCard::DCU::Mode::LIR) {
      if (mode == PortCard::DCU::LIR) {
        //cout<< "LIR" <<endl;
        dcuaccess.setDcuCREG(cregLIR | channel);
      }
      else {
        //cout<< "HIR" <<endl;
        dcuaccess.setDcuCREG(cregHIR | channel);
      }
      unsigned highbits = dcuaccess.getDcuSHREG();
      unsigned lowbits  = dcuaccess.getDcuLREG();
      adc[channel] = lowbits + ((highbits & 0xf) << 8);
    }

    // read DCU ID
    unsigned dcuId = dcuaccess.getDcuHardId();
    if (debug_) cout<< "// portcard_name = " << portcard_name << " dcuId = " << dcuId <<endl;

    PortCard::AddressDCU addressDCU;
    addressDCU.address_.portcardName_   = portcard_name;
    addressDCU.address_.dcuId_          = dcuId;
    addressDCU.address_.fecBoardId_     = fecSlot;
    addressDCU.address_.mfecId_         = ringSlot;
    addressDCU.address_.ccuId_          = ccuAddress;
    addressDCU.address_.ccuChannelId_   = i2cChannel;
    addressDCU.dcu_.mode_  = mode;
    addressDCU.dcu_.Vaa_   = adc[0];
    addressDCU.dcu_.Vdd_   = adc[1];
    addressDCU.dcu_.rtd2_  = adc[2];
    addressDCU.dcu_.rtd3_  = adc[3];
    addressDCU.dcu_.aoh_   = adc[4];
    addressDCU.dcu_.Vpc_   = adc[5];
    addressDCU.dcu_.Vbg_   = adc[6];
    addressDCU.dcu_.ts_    = adc[7];
    
    if (debug_) addressDCU.print(std::cout);
    
    vdcu.push_back(addressDCU);
  } // loop over Port Cards
  
  // make SOAP message with tag PortCard::SOAP_ReadAll::name_command = "updateDpValueTrkFEC"
  soapMessage = PortCard::SOAP_ReadAll::Make(PortCard::SOAP_ReadAll::name_command_, vdcu);

  hardwarelock_->give();

  } catch (FecExceptionHandler & e) {
    if (hardwarelock_->value()==toolbox::BSem::EMPTY) hardwarelock_->give();
    diagService_->reportError("Caught hardware access exception in readDCU: "+string(e.what()),DIAGWARN);
    soapMessage = MakeSOAPMessageReference("readDCUFailed");
  }
  catch (...) {
    if (hardwarelock_->value()==toolbox::BSem::EMPTY) hardwarelock_->give();
    diagService_->reportError("Caught unknown exception in readDCU",DIAGWARN);
    soapMessage = MakeSOAPMessageReference("readDCUFailed");
  }
  
  if (debug_) soapMessage->writeTo(cout);
  if (debug_) cout<<endl;
  
  return soapMessage;
}

//////////////////////////////////////////////////
//
//  test routine to debug decoding of SOAP message
//
//////////////////////////////////////////////////

xoap::MessageReference PixelTKFECSupervisor::readDCU_fakeSOAP(xoap::MessageReference msg) //throw (xoap::exception::Exception)
{
  cout<<endl<< "PixelTKFECSupervisor::readDCU_fakeSOAP" <<endl;
  
  //
  //  LIR or HIR: from config
  //
  PortCard::DCU::Mode mode = PortCard::DCU::LIR;
  
  std::vector<PortCard::AddressDCU> vdcu;
  
  cout<< "PixelTKFECSupervisor::readDCU_fakeSOAP: mapNamePortCard_.size() = " << mapNamePortCard_.size() <<endl;
  //for (map<string, PixelPortCardConfig*>::iterator portcard=mapNamePortCard_.begin(); portcard!=mapNamePortCard_.end(); ++portcard)
  for (unsigned iPortCard=0; iPortCard<2; ++iPortCard)
  {
    //cout<<endl<< "PixelTKFECSupervisor::readDCU_fakeSOAP: \"reading\" Port Card #" << iPortCard << " with name " << portcard->first <<endl;
    cout<<endl<< "PixelTKFECSupervisor::readDCU_fakeSOAP: \"reading\" Port Card #" << iPortCard <<endl;
    
    std::stringstream ss_portcard_name;
    ss_portcard_name << "fake_name_portcard_" << iPortCard;
    std::string portcard_name = ss_portcard_name.str();
    
    unsigned int fecSlot = (iPortCard+1)*1000 + 1;
    unsigned int ringSlot = (iPortCard+1)*1000 + 2;
    unsigned int ccuAddress = (iPortCard+1)*1000 + 3;
    unsigned int i2cChannel = (iPortCard+1)*1000 + 4;
    //unsigned int i2cAddress = 0x50;           // hardwared by design
    
    cout<< "PixelTKFECSupervisor::readDCU_fakeSOAP: fecSlot = " << fecSlot << " ringSlot = " << ringSlot << " ccuAddress = " << ccuAddress << " i2cChannel = " << i2cChannel <<endl;
    
    //
    //  data to be returned through soap message
    //
    unsigned adc[8];
    
    //unsigned cregLIR = 0x88;
    //unsigned cregHIR = 0x80;
    
    for (unsigned channel=0; channel<8; ++channel) {
      //if (mode == PortCard::DCU::Mode::LIR) {
      if (mode == PortCard::DCU::LIR) {
        //cout<< "LIR" <<endl;
        //dcu.setDcuCREG(cregLIR | channel);
      }
      else {
        //cout<< "HIR" <<endl;
        //dcu.setDcuCREG(cregHIR | channel);
      }
      //unsigned highbits = dcu.getDcuSHREG();
      //unsigned lowbits  = dcu.getDcuLREG();
      //adc[channel] = lowbits + ((highbits & 0xf) << 8);
      adc[channel] = (iPortCard+1)*100 + channel;
    }

    PortCard::AddressDCU addressDCU;
    addressDCU.address_.portcardName_  = portcard_name;
    addressDCU.address_.dcuId_         = 0;
    addressDCU.address_.fecBoardId_    = fecSlot;
    addressDCU.address_.mfecId_        = ringSlot;
    addressDCU.address_.ccuId_         = ccuAddress;
    addressDCU.address_.ccuChannelId_  = i2cChannel;
    addressDCU.dcu_.mode_  = mode;
    addressDCU.dcu_.Vaa_   = adc[0];
    addressDCU.dcu_.Vdd_   = adc[1];
    addressDCU.dcu_.rtd2_  = adc[2];
    addressDCU.dcu_.rtd3_  = adc[3];
    addressDCU.dcu_.aoh_   = adc[4];
    addressDCU.dcu_.Vpc_   = adc[5];
    addressDCU.dcu_.Vbg_   = adc[6];
    addressDCU.dcu_.ts_    = adc[7];
    
    addressDCU.print(std::cout);
    
    vdcu.push_back(addressDCU);
  } // loop over Port Cards
  
  // make SOAP message with tag PortCard::SOAP_ReadAll::name_command = "updateDpValueTrkFEC"
  xoap::MessageReference soapMessage = PortCard::SOAP_ReadAll::Make(PortCard::SOAP_ReadAll::name_command_, vdcu);
  
  //std::cout << "Christian v1.0" << std::endl;
  soapMessage->writeTo(cout);
  cout<<endl;
  
  bool decode_SOAP = true;
  if (decode_SOAP)
  {
    // decode SOAP using PortCardDCU_SOAP::DecodeSOAP
    cout<< "// PixelTKFECSupervisor::readDCU_fakeSOAP: decode SOAP using PortCardDCU_SOAP::DecodeSOAP" <<endl;
    std::vector<PortCard::AddressDCU> vdcu = PortCard::SOAP_ReadAll::Decode(soapMessage);

    cout<< "PixelTKFECSupervisor::readDCU_fakeSOAP: The number of Port Cards is " << vdcu.size() <<endl;
    for (std::vector<PortCard::AddressDCU>::iterator addressDCU=vdcu.begin(); addressDCU!=vdcu.end(); ++addressDCU)
    {
      cout<< "PixelTKFECSupervisor::readDCU_fakeSOAP: Port Card #" << std::distance(vdcu.begin(), addressDCU) <<endl;
      addressDCU->print(std::cout);
    }
  }
  
  //return MakeSOAPMessageReference("fake_soap_mess");
  return soapMessage;
}

xoap::MessageReference PixelTKFECSupervisor::fsmStateNotification(xoap::MessageReference msg) //throw (xoap::exception::Exception)
{
  if (extratimers_)   {
    fsmStateNotificationTimer_.start();
    if(DEBUG) std::cout<<"PixelTKFECSupervisor::fsmStateNotification - state "<< fsm_.getStateName(fsm_.getCurrentState()) <<std::endl;
  }

  xoap::MessageReference response=MakeSOAPMessageReference("fsmStateNotificationDone");
  
  xoap::SOAPEnvelope envelope=msg->getSOAPPart().getEnvelope();
  xoap::SOAPName notificationName=envelope.createName("fsmStateNotification");
  std::vector<xoap::SOAPElement> notifications=envelope.getBody().getChildElements(notificationName);
  for (unsigned int j=0; j<notifications.size(); ++j) {
    xoap::SOAPName stateName=envelope.createName("state");
    xoap::SOAPName partitionName=envelope.createName("partition");
    std::vector<xoap::SOAPElement> stateList=notifications.at(j).getChildElements(stateName);
    for (unsigned int i=0; i<stateList.size(); ++i) {
      std::string powerCoordinate=stateList.at(i).getAttributeValue(partitionName);
      std::string fsmState=stateList.at(i).getValue();
      std::cout<<"PixelTKFECSupervisor::fsmStateNotification - powerCoordinate = "<<powerCoordinate<<", fsmState = "<<fsmState<<std::endl;
      if (fsmState=="LV_OFF") powerMap_.setVoltage(powerCoordinate, LV_OFF, std::cout);
      else if (fsmState=="LV_ON")  powerMap_.setVoltage(powerCoordinate, LV_ON,  std::cout);
      else {
        std::cout<<"PixelTKFECSupervisor::fsmStateNotification - "<<fsmState<<" not recognized!"<<std::endl;
        response=MakeSOAPMessageReference("fsmStateNotificationFailed");
      }
    }
  }
  
  if (extratimers_)  {
    fsmStateNotificationTimer_.stop();
    cout<<"[PixelTKFECSupervisor::fsmStateNotification] call # "<<fsmStateNotificationTimer_.ntimes()
	<<"; total time="<<fsmStateNotificationTimer_.tottime()<<endl;
    if(DEBUG) std::cout<<"PixelTKFECSupervisor::fsmStateNotification - state "<< fsm_.getStateName(fsm_.getCurrentState()) <<std::endl;
  }

  // If we're in the Configuring state,
  // Try to re-enter the Configuring state!
  if (fsm_.getStateName(fsm_.getCurrentState())=="Configuring") {
    
    try {
      toolbox::Event::Reference e(new toolbox::Event("Configure", this));
      fsm_.fireEvent(e);
    } catch (toolbox::fsm::exception::Exception & e) {
      XCEPT_RETHROW(xoap::exception::Exception, "Invalid State Machine Input.", e);
    }

  }

  if(DEBUG) std::cout<<"PixelTKFECSupervisor::fsmStateNotification - state "<< fsm_.getStateName(fsm_.getCurrentState()) <<std::endl;
  return response;

}

xoap::MessageReference PixelTKFECSupervisor::FixSoftError (xoap::MessageReference msg)
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
  diagService_->reportError("The global key is " + stringF(theGlobalKey_->key()),DIAGDEBUG);
  diagService_->reportError("PixelTKFECSupervisor::FixSoftError: The Global Key is " + stringF(theGlobalKey_->key()),DIAGDEBUG);
  //*console_<<"PixelTKFECSupervisor::FixSoftError: The Global Key is " + stringF(theGlobalKey_->key())<<std::endl;
  
  xoap::MessageReference reply=MakeSOAPMessageReference("FixSoftErrorDone");

  // That's it! Step to the FixingSoftError state, and
  // relegate all further fixing to the stateFixingSoftError method.
  try {
    toolbox::Event::Reference e(new toolbox::Event("FixSoftError", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    //*console_<<"[PixelTKFECSupervisor::FixSoftError] FixSoftError is an invalid command for the "<<state_.toString()<<" state."<<std::endl;
    diagService_->reportError("[PixelTKFECSupervisor::FixSoftError] FixSoftError is an invalid command for the current state."+state_.toString(), DIAGERROR);
    reply=MakeSOAPMessageReference("FixSoftErrorFailed");
  }
  
  diagService_->reportError("--- FixSoftError DONE ---",DIAGINFO);
  diagService_->reportError("PixelTKFECSupervisor::FixSoftError: A prompt SOAP reply is sent back before exiting function",DIAGINFO);
  
  return reply;
  
}

void PixelTKFECSupervisor::stateFixingSoftError (toolbox::fsm::FiniteStateMachine &fsm) {
  // Update the state_ member data so that Infospace may publish this information
  stateChanged(fsm);
  
  PixelTimer FixingSoftErrorTimer;
  FixingSoftErrorTimer.start();
  bool isfailure = false;
  try {
    //Code from Pause inserted here. 
    
    if (theCalibObject_==0) {

      if(doDCULoop_) {

	//    if (dcudebug_) diagService_->reportError("[Pause] about to set DCU workloopContinue_ to false",DIAGTRACE);
	dculock_->take(); workloopContinue_=false; dculock_->give();
	//    if (dcudebug_) diagService_->reportError("[Pause] about to cancel DCU workloop",DIAGTRACE);
	workloop_->cancel();
	//diagService_->reportError("PixelTKFECSupervisor::stateFixingSoftError:  Cancelled DCU workloop",DIAGTRACE);
      }

      if (doResetCheck_) {
	//try {
	rclock_->take(); workloopContinueRC_=false; rclock_->give();
	if (resetCheckWorkloop_->isActive()) resetCheckWorkloop_->cancel();
	//diagService_->reportError("PixelTKFECSupervisor::stateFixingSoftError:  Cancelled resetCheck workloop",DIAGTRACE);
	std::cout<<"PixelTKFECSupervisor::stateFixingSoftError:  Canselled resetCheck workloop."<<std::endl;
	//}
	//} catch (xcept::Exception & e) {
	//diagService_->reportError("PixelTKFECSupervisor::stateFixingSoftError: Caught exception canceling reset check workloop: "+string(e.what()),DIAGERROR);
	//}
      }
    }

#ifdef DO_PORTCARD_RECOVERY
    //Programming of port cards
    try  {
      // do reporgamming
      cout<<" stateFixingSodtError: Will reprogram portcrads "<<endl; 
      bool status = programPortcards(false);  
      if(status) diagService_->reportError("stateFixingSoftError - Error in portcard programming ",DIAGERROR);
    } catch (...)  { //exceptions
      diagService_->reportError("stateFixingSoftError - Error in portcard programming ",DIAGERROR);
      //reply = MakeSOAPMessageReference("ResumeFailed");
      //diagService_->reportError("Failed to Resume run with exception: "+string(e.what()),DIAGERROR);
      //FIXME maybe we should transition to the Error state in case this happens?
    }      
#endif    
    suppressHardwareError_=false;

    //Code from Resume inserted here. 
    if (theCalibObject_==0) {

      if(doDCULoop_) {
	dculock_->take(); workloopContinue_=true; dculock_->give();
	workloop_->activate();
	//std::cout<<"PixelTKFECSupervisor::stateFixingSoftError:  Activated DCU workloop."<<std::endl;
      }

      if (doResetCheck_) {
	//try {
	rclock_->take(); workloopContinueRC_=true; rclock_->give();
	//resetCheckWorkloop_->activate();
	if ( !resetCheckWorkloop_->isActive() ) resetCheckWorkloop_->activate();

	std::cout<<"PixelTKFECSupervisor::stateFixingSoftError:  Activated resetCheck workloop."<<std::endl;
	//} catch (xcept::Exception & e) {
	//diagService_->reportError("PixelTKFECSupervisor::stateFixingSoftError: Caught exception activating reset check workloop: "+string(e.what()),DIAGERROR);
	//}
      }

    }
    

    toolbox::Event::Reference e(new toolbox::Event("FixingSoftErrorDone", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {

    diagService_->reportError("[PixelTKFECSupervisor::FixingSoftError] FixingSoftErrorDone is an invalid command for the current state."+state_.toString(), DIAGERROR);

    diagService_->reportError("PixelTKFECSupervisor::stateFixingSoftError: Detected Error: "+string(e.what()),DIAGERROR);
    try {
      toolbox::Event::Reference ev(new toolbox::Event("Failure", this)); //comment this out only for testing
      fsm_.fireEvent(ev);
    } catch (toolbox::fsm::exception::Exception & e2) {
      diagService_->reportError("PixelTKFECSupervisor::stateFixingSoftError: Failed to transition to Failed state!",DIAGFATAL);
    }
    return;

  } catch (FecExceptionHandler e) {
    diagService_->reportError("Failed to configure TKFEC (is the power off?); exception: "+string(e.what()),DIAGERROR);
    isfailure=true;
  }
  catch (std::exception & e) { //failure to load config data raises std::exception (not xcept::Exception)
    diagService_->reportError("Failed to configure TKFEC; exception: "+string(e.what()),DIAGERROR);
    isfailure=true;
  }
  if (isfailure) {
    try {
      //The following line should be commented for normal operation; it can be uncommented to test pxl FEC when there is no power
#ifdef SKIP_LV_CHECK
      // no power bypass, for special tests without power ON
      toolbox::Event::Reference ev(new toolbox::Event("FixingSoftErrorDone", this)); diagService_->reportError("Bypassing Error state in TKFEC!",DIAGWARN);
#else 
      // For normal default operation with power ON 
      toolbox::Event::Reference ev(new toolbox::Event("Failure", this)); //comment this out only for testing
#endif 
      fsm_.fireEvent(ev);
    } catch (toolbox::fsm::exception::Exception & e2) {
      diagService_->reportError("Failed to transition to Failed state!",DIAGFATAL);
    }
  }

  FixingSoftErrorTimer.stop();

  diagService_->reportError("--- Exit PixelTKFECSupervisor::stateFixingSoftError --- "+stringF(FixingSoftErrorTimer.tottime()),DIAGINFO);

}


xoap::MessageReference PixelTKFECSupervisor::ResumeFromSoftError (xoap::MessageReference msg)
{
  diagService_->reportError("--- RESUMEFROMSOFTERROR ---",DIAGINFO);
  //*console_<<"--- Resuming From Soft Error ---"<<std::endl;

  xoap::MessageReference reply = MakeSOAPMessageReference("ResumeFromSoftErrorDone");

  try {

    toolbox::Event::Reference e(new toolbox::Event("ResumeFromSoftError", this));
    fsm_.fireEvent(e);

  } catch (toolbox::fsm::exception::Exception & e) {

    //*console_<<"[PixelTKFECSupervisor::ResumeFromSoftError] ResumeFromSoftError is an invalid command for the "<<state_.toString()<<" state."<<std::endl;
    diagService_->reportError("[PixelTKFECSupervisor::ResumeFromSoftError] ResumeFromSoftError is an invalid command for the current state."+state_.toString(), DIAGERROR);

    reply = MakeSOAPMessageReference("ResumeFromSoftErrorFailed");

  }

  diagService_->reportError("-- Exit stateFixingSoftError --",DIAGINFO);
  return reply;
}
