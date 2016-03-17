// -*- C++ -*-
// $Id: PixelFECSupervisor.cc,v 1.149 2012/06/16 14:13:21 mdunser Exp $
/**************************************************************************
 * XDAQ Components for Pixel Online Software                              *
 * Copyright (C) 2009, Cornell University		                  *
 * All rights reserved.                                                   *
 * Authors: Souvik Das, Anders Ryd, Karl Ecklund, Joshua Thompson         *
  *************************************************************************/

#include "PixelFECSupervisor/include/PixelFECSupervisor.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFECConfig.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"

#include "PixelFECSupervisor/include/exception/Exception.h"
#include "log4cplus/logger.h"
#include "log4cplus/loggingmacros.h"
//gio
// #include <diagbag/DiagBagWizard.h>
// #include "DiagCompileOptions.h"
// #include <toolbox/convertstring.h>

using namespace pos;

//#define USE_SEU_DETECT

XDAQ_INSTANTIATOR_IMPL(PixelFECSupervisor)

enum  { kProg_DACs_set, kProg_DACs_increase, kProg_DACs_decrease };

PixelFECSupervisor::PixelFECSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception) : xdaq::Application(s), SOAPCommander(this), Pixelb2inCommander(this), executeReconfMethodMutex(toolbox::BSem::FULL),fsm_("urn:toolbox-task-workloop:PixelFECSupervisor")
,phlock_(new toolbox::BSem(toolbox::BSem::FULL,true))
,workloopContinue_(false)
,sv_logger_(getApplicationLogger())
{

  //gio
  // diagService_ = new DiagBagWizard(
  //                                  ("ReconfigurationModule") ,
  //                                  this->getApplicationLogger(),
  //                                  getApplicationDescriptor()->getClassName(),
  //                                  getApplicationDescriptor()->getInstance(),
  //                                  getApplicationDescriptor()->getLocalId(),
  //                                  (xdaq::WebApplication *)this,
  //                                  "Pixel",
  //                                  "FECSupervisor"
  //                                  );

//std::string const msg_info_uir = "The DiagSystem is installed --- this is a bogus error message";
// LOG4CPLUS_INFO(sv_logger_,msg_info_uir);


  //Binding SOAP Callbacks to State Machine Commmands
  xoap::bind(this, &PixelFECSupervisor::Initialize, "Initialize", XDAQ_NS_URI);        
  xoap::bind(this, &PixelFECSupervisor::Configure, "Configure", XDAQ_NS_URI );
  xoap::bind(this, &PixelFECSupervisor::Start, "Start", XDAQ_NS_URI );
  xoap::bind(this, &PixelFECSupervisor::Stop, "Stop", XDAQ_NS_URI);
  xoap::bind(this, &PixelFECSupervisor::Pause, "Pause", XDAQ_NS_URI );
  xoap::bind(this, &PixelFECSupervisor::Resume, "Resume", XDAQ_NS_URI );
  xoap::bind(this, &PixelFECSupervisor::Halt, "Halt", XDAQ_NS_URI );
  xoap::bind(this, &PixelFECSupervisor::Recover, "Recover", XDAQ_NS_URI );

  // Soft Error Stuff
  xoap::bind(this, &PixelFECSupervisor::FixSoftError, "FixSoftError", XDAQ_NS_URI );
  xoap::bind(this, &PixelFECSupervisor::ResumeFromSoftError, "ResumeFromSoftError", XDAQ_NS_URI);
  
  //a pseudo-FSM transition
  xoap::bind(this, &PixelFECSupervisor::preConfigure, "preConfigure", XDAQ_NS_URI );
  //for quick fine delay scan reprogramming
  xoap::bind(this, &PixelFECSupervisor::Reconfigure, "Reconfigure", XDAQ_NS_URI );

  // A SOAP callback used for generic handshaking by retrieving the FSM state
  xoap::bind(this, &PixelFECSupervisor::FSMStateRequest, "FSMStateRequest", XDAQ_NS_URI);

  //Binding SOAP Callbacks to Low Level Commands
  xoap::bind(this, &PixelFECSupervisor::TBMCommand, "TBMCommand", XDAQ_NS_URI);
  xoap::bind(this, &PixelFECSupervisor::TBMSpeed2, "TBMSpeed2", XDAQ_NS_URI);
  xoap::bind(this, &PixelFECSupervisor::Delay25Test, "Delay25Test", XDAQ_NS_URI);
  xoap::bind(this, &PixelFECSupervisor::Prog_DAC, "Prog_DAC", XDAQ_NS_URI);
  xoap::bind(this, &PixelFECSupervisor::Prog_DACs, "Prog_DACs", XDAQ_NS_URI);
  xoap::bind(this, &PixelFECSupervisor::SetROCDACsEnMass, "SetROCDACsEnMass", XDAQ_NS_URI);
  xoap::bind(this, &PixelFECSupervisor::Prog_Pix, "Prog_Pix", XDAQ_NS_URI);
  xoap::bind(this, &PixelFECSupervisor::Cal_Pix, "Cal_Pix", XDAQ_NS_URI);
  xoap::bind(this, &PixelFECSupervisor::ClrCal, "ClrCal", XDAQ_NS_URI);
  xoap::bind(this, &PixelFECSupervisor::CalibRunning, "CalibRunning", XDAQ_NS_URI);
  xoap::bind(this, &PixelFECSupervisor::CalibRunningThreshold, "CalibRunningThreshold", XDAQ_NS_URI);
  xoap::bind(this, &PixelFECSupervisor::ClrCalEnMass, "ClrCalEnMass", XDAQ_NS_URI);
  xoap::bind(this, &PixelFECSupervisor::DisableHitsEnMass, "DisableHitsEnMass", XDAQ_NS_URI);
  xoap::bind(this, &PixelFECSupervisor::EnableFullBufferCheck, "EnableFullBufferCheck", XDAQ_NS_URI);
  xoap::bind(this, &PixelFECSupervisor::SetTBMDACsEnMass, "SetTBMDACsEnMass", XDAQ_NS_URI);
  xoap::bind(this, &PixelFECSupervisor::ResetROCs, "ResetROCs", XDAQ_NS_URI);
  xoap::bind(this, &PixelFECSupervisor::ResetTBM, "ResetTBM", XDAQ_NS_URI);
  xoap::bind(this, &PixelFECSupervisor::fsmStateNotification, "fsmStateNotification", XDAQ_NS_URI);
  xoap::bind(this, &PixelFECSupervisor::Null, "Null", XDAQ_NS_URI);
  
  // Binding XGI Callbacks for messages from the browser
  xgi::bind(this, &PixelFECSupervisor::Default, "Default");
  xgi::bind(this, &PixelFECSupervisor::StateMachineXgiHandler, "StateMachineXgiHandler");
  xgi::bind(this, &PixelFECSupervisor::mFECsHubs, "mFECsHubs");
  xgi::bind(this, &PixelFECSupervisor::mFECsHubs_XgiHandler, "mFECsHubs_XgiHandler");
  xgi::bind(this, &PixelFECSupervisor::Panel, "Panel");
  xgi::bind(this, &PixelFECSupervisor::Panel_XgiHandler, "Panel_XgiHandler");
  xgi::bind(this, &PixelFECSupervisor::ROC, "ROC");
  xgi::bind(this, &PixelFECSupervisor::ROC_XgiHandler, "ROC_XgiHandler");

  //DIAGNOSTIC REQUESTED CALLBACK
  // xgi::bind(this,&PixelFECSupervisor::configureDiagSystem, "configureDiagSystem");
  // xgi::bind(this,&PixelFECSupervisor::applyConfigureDiagSystem, "applyConfigureDiagSystem");
  // xgi::bind(this,&PixelFECSupervisor::callDiagSystemPage, "callDiagSystemPage");

  b2in::nub::bind(this, &PixelFECSupervisor::b2inEvent);

//DIAGNOSTIC REQUESTED DECLARATION
  // DIAG_DECLARE_USER_APP

  // Defining the state of the State Machine
  fsm_.addState('I', "Initial", this, &PixelFECSupervisor::stateChanged);
  fsm_.addState('H', "Halted", this, &PixelFECSupervisor::stateChanged);
  fsm_.addState('c', "Configuring", this, &PixelFECSupervisor::stateConfiguring);
  fsm_.addState('C', "Configured", this, &PixelFECSupervisor::stateChanged);
  fsm_.addState('R', "Running", this, &PixelFECSupervisor::stateChanged);
  fsm_.addState('P', "Paused", this, &PixelFECSupervisor::stateChanged);

  //Adding Soft Error Detection Stuff

  fsm_.addState('s', "FixingSoftError", this, &PixelFECSupervisor::stateFixingSoftError);
  fsm_.addState('S', "FixedSoftError", this, &PixelFECSupervisor::stateChanged);

  fsm_.setStateName('F',"Error");
  fsm_.setFailedStateTransitionAction(this, &PixelFECSupervisor::enteringError);
  fsm_.setFailedStateTransitionChanged(this, &PixelFECSupervisor::stateChanged);

  // Defining the transitions of the State Machine
  fsm_.addStateTransition('I', 'H', "Initialize");
  fsm_.addStateTransition('H', 'c', "Configure", this, &PixelFECSupervisor::transitionHaltedToConfiguring);
  fsm_.addStateTransition('c', 'c', "Configure");
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

  //Adding Soft Error Detection Stuff
  
  fsm_.addStateTransition('R', 's', "FixSoftError");
  fsm_.addStateTransition('s', 'S', "FixingSoftErrorDone");
  fsm_.addStateTransition('S', 'R', "ResumeFromSoftError");

  //error transitions
  fsm_.addStateTransition('H', 'F', "Failure"); //preConfigure could have an error
  fsm_.addStateTransition('c', 'F', "Failure");
  fsm_.addStateTransition('F', 'F', "Failure");

  //Adding Soft Error Detection Stuff

  fsm_.addStateTransition('s','F',"Failure");
  fsm_.addStateTransition('S','F',"Failure");

  fsm_.setInitialState('I');
  fsm_.reset();


  // Miscellaneous variables
  if (getenv("BUILD_HOME")==0){
    htmlbase_=std::string(getenv("XDAQ_ROOT"))+"/htdocs/PixelFECSupervisor/html/";
    datbase_=std::string(getenv("XDAQ_ROOT"))+"/dat/PixelFECInterface/dat/";
  }
  else{
    htmlbase_=std::string(getenv("BUILD_HOME"))+"/pixel/PixelFECSupervisor/html/";
    datbase_=std::string(getenv("BUILD_HOME"))+"/pixel/PixelFECInterface/dat/";  
  }


  crate_=this->getApplicationDescriptor()->getInstance();

  // Change for HAL d.k. 19/12/07
#ifdef USE_HAL
  busAdapter_ = (HAL::CAENLinuxBusAdapter*) 0;
  addressTablePtr_ = (HAL::VMEAddressTable*) 0;
#else
  Device=0;
  aBHandle=0;
  Link=0;
#endif // USE_HAL
  
  theGlobalKey_=0;
  theLastGlobalKey_=-1;  // change from 0, -1 is a valid key 
  theNameTranslation_=0;
  theDetectorConfiguration_=0;
  theFECConfiguration_=0;
  theCalibObject_=0;
  calibStateCounter_=0;
  PixelDCSFSMInterface_=0;
  PixelPSXServer_=0;

  preconfigureWorkloop_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("preconfigureWorkloop", "waiting");
  preconfigureTask_ = toolbox::task::bind(this, &PixelFECSupervisor::preconfigure_workloop, "preconfigure_workloop");
  preConfigureDone_=false;
  detConfigLoaded_=false;
  pclock_=new toolbox::BSem(toolbox::BSem::FULL,true);
  totalTimer_.setName("PixelFECSupervisorConfigurationTimer");

  // Initialize the Physics WorkLoop
  doTBMReadoutLoop_ = false;  // disable 17/6/15 dk 
  if(doTBMReadoutLoop_) {
    workloop_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("PixelFECSupervisorWorkLoop", "waiting");
    physicsRunning_=toolbox::task::bind (this, &PixelFECSupervisor::PhysicsRunning, "PhysicsRunning");
  }

  // Exporting the FSM state to this application's default InfoSpace
  state_=fsm_.getStateName(fsm_.getCurrentState());
  getApplicationInfoSpace()->fireItemAvailable("stateName", &state_);

  powerMap_.init();
  powerMapLast_.init();

  //QPLL workloop
  doQPLLLoop_ = true;
  qpllWorkloop_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("FecQpllWorkLoop", "waiting");
  qpllCheck_=toolbox::task::bind (this, &PixelFECSupervisor::qpllCheck, "qpllCheck");
  num_qpll_locked_ = 0;
  num_qpll_unlocked_ = 0;
  qplllock_ = new toolbox::BSem(toolbox::BSem::FULL,true);

//DIAGNOSTIC REQUESTED AUTOCONF TIMER

  // std::stringstream timerName;
  // timerName << getApplicationDescriptor()->getContextDescriptor()->getURL() << ":";
  // timerName << getApplicationDescriptor()->getClassName() << ":" << getApplicationDescriptor()->getLocalId() << ":" << getApplicationDescriptor()->getInstance();
  // toolbox::task::Timer * timer = toolbox::task::getTimerFactory()->createTimer(timerName.str());
  // toolbox::TimeInterval interval(AUTO_UP_CONFIGURE_DELAY,0);
  // toolbox::TimeVal start;
  // start = toolbox::TimeVal::gettimeofday() + interval;
  // timer->schedule( this, start,  0, "" );

}

PixelFECSupervisor::~PixelFECSupervisor()
{
  delete phlock_;
  delete pclock_;
  deleteHardware();
  delete qplllock_;
}


//gio
// void PixelFECSupervisor::timeExpired (toolbox::task::TimerEvent& e)
// {
//   DIAG_EXEC_FSM_INIT_TRANS
// }

//DIAG ADDED
// void PixelFECSupervisor::callDiagSystemPage(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
// {
//   diagService_->getDiagSystemHtmlPage(in, out,getApplicationDescriptor()->getURN());
// }

void PixelFECSupervisor::Default (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{

  //gio
//std::string const msg_error_yfv = "The DiagSystem is installed --- this is a bogus error message";
// LOG4CPLUS_ERROR(sv_logger_,msg_error_yfv);
  //diagService_->reportError("Access PixelFECSupervisor",DIAGERROR);

  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  // xgi::Utils::getPageHeader(*out, "Pixel Front End Controller Supervisor", fsm_.getStateName(fsm_.getCurrentState()));

  // Rendering the State Machine GUI
  
  std::set<std::string> allInputs=fsm_.getInputs();
  std::set<std::string> clickableInputs=fsm_.getInputs(fsm_.getCurrentState());
  std::set<std::string>::iterator i;
  
  *out<<"<h2> Finite State Machine </h2>";
  
  std::string url="/"+getApplicationDescriptor()->getURN();
  *out<<"If in doubt, click <a href=\""<<url<<"\">here</a> to refresh"<<std::endl;
  *out<<"<form name=\"input\" method=\"get\" action=\""<<url+"/StateMachineXgiHandler"<<"\" enctype=\"multipart/form-data\">";
  
  *out<<"<table border cellpadding=10 cellspacing=0>";
  *out<<"<tr>";
  *out<<"<td>";
  *out<<"<b>Current State</b>:"<<fsm_.getStateName(fsm_.getCurrentState())<<"<br/>"<<std::endl;
  if (theGlobalKey_!=0) {
    *out<<"<b>Configuration Key: </b>"<<theGlobalKey_->key()<<"<br/>"<<std::endl;
  }
  *out<<"</td>";
  *out<<"<td colspan=5>";
  for (i=allInputs.begin();i!=allInputs.end();i++)
  {
    if (clickableInputs.find(*i)!=clickableInputs.end())
    {
      HTML2XGI(out, htmlbase_+(*i)+".htm");
    }
  }
  *out<<"</td>"<<endl;
  *out<<"</tr>"<<endl;
  *out<<"<tr>"<<endl;
  
  for (i=allInputs.begin();i!=allInputs.end();i++)
    {
      *out<<"<td>";
      if (clickableInputs.find(*i)!=clickableInputs.end())
	{
	  *out<<"<input type=\"submit\" name=\"Command\" value=\""<<(*i)<<"\"/>";
	}
      else
	{
	  *out<<"<input type=\"submit\" disabled=\"true\" name=\"Command\" value=\""<<(*i)<<"\"/>";
	}
      *out<<"</td>"<<endl;
    }
  
  *out<<"</tr>";
  *out<<"</table>";
  *out<<"</form>"<<endl;
  
  *out<<"<hr/>"<<endl;

  *out << "<h2>DCS-FSM interaction</h2> "<<std::endl;
  *out << "<form name=\"input2\" method=\"get\" action=\"" << url + "/StateMachineXgiHandler" <<"\" enctype=\"multipart/form-data\">";
  *out << "<input type=\"submit\" name=\"Command\" id=\"DumpPowerMap\" value=\"DumpPowerMap\"/>";
  *out << "</form>" << std::endl;

  *out<<"<hr/>"<<endl;

  *out << "<h2>QPLL Status</h2> "<<std::endl;
  if (!FECInterface.empty() ) {
    qplllock_->take();
    *out <<"Number of   locked QPLLs = "<<num_qpll_locked_<<"<br>"<<endl;
    *out <<"Number of unlocked QPLLs = "<<num_qpll_unlocked_<<"<br>"<<endl;
    qplllock_->give();
  }
  *out << " <hr/> " << std::endl;

  // DiagSystem GUI
  // std::string urlDiag_ = "/"; 
  // urlDiag_ += getApplicationDescriptor()->getURN(); 
  // urlDiag_ += "/callDiagSystemPage"; 
  // *out << "<h2> Error Dispatcher </h2> "<<std::endl;
  // *out << "<a href=" << urlDiag_ << ">Configure DiagSystem</a>" <<std::endl;
  // *out << " <hr/> " << std::endl;
  
  // Rendering Low Level GUI  
  *out<<"<h2>Low Level Commands</h2>"<<endl;
  
  for (FECInterfaceMap::iterator iFEC=FECInterface.begin();iFEC!=FECInterface.end();++iFEC)
    {
      std::string vmeBaseAddress_string=itoa(iFEC->first);
      *out<<"<a href=\""<<url+"/mFECsHubs?FECBaseAddress="<<vmeBaseAddress_string<<"\" target=\"_blank\">"<<"FEC with Base Address 0x"<<hex<<atoi(vmeBaseAddress_string.c_str())<<dec<<"</a>"<<endl;
    }
 
  *out<<"<p align=right>"<<std::endl;
  *out<<"Please send bug reports and suggestions to "<<std::endl;
  *out<<"<a href=\"mailto:sd259@cornell.edu?Subject=PixelFECSupervisor\">Souvik Das</a>"<<std::endl;
  *out<<"</p>"<<std::endl;
  
  *out<<"</body>"<<endl;
  
  *out<<"</html>"<<endl;
}

void PixelFECSupervisor::mFECsHubs (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{
        cgicc::Cgicc cgi(in);

	std::string vmeBaseAddress_string=cgi.getElement("FECBaseAddress")->getValue();
	unsigned long vmeBaseAddress=atol(vmeBaseAddress_string.c_str());

	std::string url="/"+getApplicationDescriptor()->getURN();

	*out<<"<html>"<<endl;
	*out<<"<body>"<<endl;

	*out<<"<h1> FEC with Base Address 0x"<<hex<<vmeBaseAddress<<dec<<"</h1>"<<endl;

	*out<<"<table border=2>";
	*out<<"<tr><td colspan=2>";
	*out<<"<form name=\"input\" method=\"get\" action=\""<<url+"/mFECsHubs_XgiHandler"<<"\" enctype=\"multipart/form-data\">";
		HTML2XGI(out, htmlbase_+"TBMCommand.htm");
		*out<<"<input type=\"hidden\" name=\"FECBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<endl;
	*out<<"</form>";
	*out<<"</td></tr><tr><td>";
	*out<<"<form name=\"input\" method=\"get\" action=\""<<url+"/mFECsHubs_XgiHandler"<<"\" enctype=\"multipart/form-data\">";
		HTML2XGI(out, htmlbase_+"ProgDAC.htm");
		*out<<"<input type=\"hidden\" name=\"FECBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<endl;
	*out<<"</form>";
	*out<<"</td><td>";
	*out<<"<form name=\"input\" method=\"get\" action=\""<<url+"/mFECsHubs_XgiHandler"<<"\" enctype=\"multipart/form-data\">";
		HTML2XGI(out, htmlbase_+"ProgPix.htm");
		*out<<"<input type=\"hidden\" name=\"FECBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<endl;
	*out<<"</form>"<<endl;
	*out<<"</td></tr><tr><td>";
	*out<<"<form name=\"input\" method=\"get\" action=\""<<url+"/mFECsHubs_XgiHandler"<<"\" enctype=\"multipart/form-data\">";
		HTML2XGI(out, htmlbase_+"CalPix.htm");
		*out<<"<input type=\"hidden\" name=\"FECBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<endl;
	*out<<"</form>"<<endl;
	*out<<"</td><td>";
	*out<<"<form name=\"input\" method=\"get\" action=\""<<url+"/mFECsHubs_XgiHandler"<<"\" enctype=\"multipart/form-data\">";
		HTML2XGI(out, htmlbase_+"ClrCal.htm");
		*out<<"<input type=\"hidden\" name=\"FECBaseAddress\" value=\""<<vmeBaseAddress_string<<"\"/>"<<endl;
	*out<<"</form>"<<endl;
	*out<<"</td></tr>"<<endl;
	*out<<"</table>"<<endl;

	///////////////////////////////////////////////////////////////
	// New Interface begins here!
	///////////////////////////////////////////////////////////////

	*out<<"New Interface begins here --- "<<endl;

	std::map <unsigned int, std::vector < std::map <unsigned int, std::string> > > mFECChannelsHubs;
	//           mFEC       channel                     hub       module name
	std::vector <PixelModuleName> module_list = theDetectorConfiguration_->getModuleList();
	std::vector <PixelModuleName>::iterator module_name = module_list.begin();
	for (;module_name!=module_list.end();++module_name)
	{
		const PixelHdwAddress& module_firstHdwAddress = theNameTranslation_->firstHdwAddress( *module_name );
		unsigned int fecnumber=module_firstHdwAddress.fecnumber();
		unsigned int feccrate=theFECConfiguration_->crateFromFECNumber(fecnumber);
		unsigned int fecVMEBaseAddress=theFECConfiguration_->VMEBaseAddressFromFECNumber(fecnumber);
		if (feccrate==crate_ && fecVMEBaseAddress==vmeBaseAddress)
		  {
		    unsigned int mFEC=module_firstHdwAddress.mfec();
		    unsigned int mFECChannel=module_firstHdwAddress.mfecchannel();
		    unsigned int hub=module_firstHdwAddress.hubaddress();
		    if (mFECChannelsHubs.find(mFEC)==mFECChannelsHubs.end())
		      {
			std::map <unsigned int, std::string> hubs;
			hubs.insert(make_pair(hub, module_name->modulename()));
			std::vector < std::map <unsigned int, std::string> > hubsChannels(2);
			hubsChannels.at(mFECChannel-1)=hubs;
			mFECChannelsHubs.insert(make_pair(mFEC, hubsChannels));
		      }
		    else
		      {
			mFECChannelsHubs[mFEC].at(mFECChannel-1).insert(make_pair(hub, module_name->modulename()));
		      }
		  }
	}

	*out<<"<table border=1>"<<endl;
	*out<<" <tr>"<<endl;
	for (unsigned int mFEC=1;mFEC<=8;++mFEC)
	  {
	    if (mFECChannelsHubs.find(mFEC)!=mFECChannelsHubs.end())
	      {
		*out<<"  <td colspan=2 align=center bgcolor=cyan>"<<endl;
		*out<<"   mFEC "<<mFEC<<endl;
		*out<<"  </td>"<<endl;
	      }
	  }
	*out<<" </tr>"<<endl;
	*out<<" <tr>"<<endl;
	for (unsigned int mFEC=1;mFEC<=8;++mFEC){
		if (mFECChannelsHubs.find(mFEC)!=mFECChannelsHubs.end()){
			*out<<"  <td align=center bgcolor=pink>"<<endl;
			*out<<"   A"<<endl;
			*out<<"   <table border=1>"<<endl;
			std::map <unsigned int, std::string>::iterator i_hub;
			for (i_hub=mFECChannelsHubs[mFEC].at(0).begin();i_hub!=mFECChannelsHubs[mFEC].at(0).end();++i_hub){
				*out<<"    <tr><td align=center bgcolor=lightgreen>";
				*out<<"     <a onClick=\"window.open('"<<url+"/Panel?FECBaseAddress="<<vmeBaseAddress_string<<"&ModuleName="<<(i_hub->second)<<"','Panel="<<(i_hub->second)<<"','menubar=no, toolbar=no, width=700, height=400')\">";
				*out<<"     Hub "<<(i_hub->first)<<"</a>"<<endl;
				*out<<"    </td></tr>"<<endl;
			}
			*out<<"   </table>"<<endl;
			*out<<"  </td>"<<endl;
			*out<<"  <td align=center bgcolor=pink>"<<endl;
			*out<<"   B"<<endl;
			*out<<"   <table border=1>"<<endl;
			for (i_hub=mFECChannelsHubs[mFEC].at(1).begin();i_hub!=mFECChannelsHubs[mFEC].at(1).end();++i_hub){
                                *out<<"    <tr><td align=center bgcolor=lightgreen>";
                                *out<<"     <a onClick=\"window.open('"<<url+"/Panel?FECBaseAddress="<<vmeBaseAddress_string<<"&ModuleName="<<(i_hub->second)<<"','Panel="<<(i_hub->second)<<"','menubar=no, toolbar=no, width=700, height=400')\">";
                                *out<<"     Hub "<<(i_hub->first)<<"</a>"<<endl;
                                *out<<"    </td></tr>"<<endl;
                        }
			*out<<"   </table>"<<endl;
                	*out<<"  </td>"<<endl;
		}
	}
	*out<<" </tr>"<<endl;
	*out<<"</table>"<<endl;

	*out<<"<p align=right>"<<std::endl;
	*out<<"Please send bug reports and suggestions to "<<std::endl;
	*out<<"<a href=\"mailto:sd259@cornell.edu?Subject=PixelFECSupervisor Panel Low Level GUI\">Souvik Das</a>"<<std::endl;
	*out<<"</p>"<<std::endl;

	*out<<"</body>"<<endl;
	*out<<"</html>"<<endl;
}

void PixelFECSupervisor::Panel (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{
	cgicc::Cgicc cgi(in);

  std::string vmeBaseAddress_string=cgi.getElement("FECBaseAddress")->getValue();
	std::string moduleName_string=cgi.getElement("ModuleName")->getValue();

	PixelModuleName moduleName(moduleName_string);

	const PixelHdwAddress& module_firstHdwAddress = theNameTranslation_->firstHdwAddress( moduleName );
        unsigned int mFEC=module_firstHdwAddress.mfec();
        unsigned int mFECChannel=module_firstHdwAddress.mfecchannel();
        unsigned int hub=module_firstHdwAddress.hubaddress();

        std::string url="/"+getApplicationDescriptor()->getURN();

	std::map <unsigned int, std::map <std::string, unsigned int> > portsRocs;
	//              port            roc string name, roc id
	// Pass module name through name translation to find the ROC hardware addresses
        // FIXME This might not do the right thing...
	std::vector <PixelROCName>roc_vector=theNameTranslation_->getROCsFromModule(moduleName);
	std::vector <PixelROCName>::iterator iroc=roc_vector.begin();
	for (;iroc!=roc_vector.end();++iroc){
		const PixelHdwAddress* roc_hdwaddress=theNameTranslation_->getHdwAddress(*iroc);
		unsigned int port=roc_hdwaddress->portaddress();
		unsigned int rocid=roc_hdwaddress->rocid();
		if (portsRocs.find(port)==portsRocs.end()){
			std::map <std::string, unsigned int> rocs;
			rocs.insert(make_pair(iroc->rocname(), rocid));
			portsRocs.insert(make_pair(port, rocs));
		} else {
			portsRocs[port].insert(make_pair(iroc->rocname(), rocid));
		}
	}


  *out<<"<html>"<<endl;
	*out<<"<head>"<<endl;
	*out<<" <script language=\"JavaScript\" src=\"../pixel/PixelFECSupervisor/html/PanelGUIAJAX.js\"></script>"<<std::endl;
	*out<<"</head>"<<endl;
        *out<<"<body>"<<endl;

        *out<<"<h1> FEC "<<hex<<vmeBaseAddress_string<<dec<<", mFEC "<<mFEC<<" "<<mFECChannel<<", Hub "<<hub<<"</h1>"<<endl;

	*out<<"<table border=1>"<<endl;
	*out<<" <tr><td>TBM</td></tr>"<<endl;
	*out<<"</table><br/>"<<endl;

	for (unsigned int port=0;port<=3;++port){
		if (portsRocs.find(port)!=portsRocs.end()){
			*out<<"Port "<<port<<endl;
			*out<<"<table border=1>"<<endl;
			*out<<" <tr>"<<endl;
			std::map <std::string, unsigned int>::iterator iroc=portsRocs[port].begin();
			for (;iroc!=portsRocs[port].end();++iroc){
				*out<<"  <td align=center>"<<endl;
				*out<<"   <button onClick=\"window.open('"<<url+"/ROC?FECBaseAddress="<<vmeBaseAddress_string<<"&ROCName="<<(iroc->first)<<"', 'ROC', 'menubar=no, toolbar=no, width=1050, height=950, scrollbars=yes')\">";
				*out<<"   ROC "<<(iroc->second)<<"</button>"<<endl;;
				*out<<"  </td>"<<endl;
			}
			*out<<" </tr>"<<endl;
			*out<<"</table><br/>"<<endl;
		}
	}

	*out<<"   <button onclick=\"ResetTBM_AJAX('"<<url<<"', '"<<vmeBaseAddress_string<<"', "<<mFEC<<", "<<mFECChannel<<", 14, "<<hub<<")\">Reset TBM</button><br/>"<<std::endl;
	*out<<"   <button onclick=\"ResetROCs_AJAX('"<<url<<"', '"<<vmeBaseAddress_string<<"', "<<mFEC<<", "<<mFECChannel<<", 14, "<<hub<<")\">Reset All ROCs</button><br/>"<<std::endl;

	*out<<"<p align=right>"<<std::endl;
	*out<<"Please send bug reports and suggestions to "<<std::endl;
	*out<<"<a href=\"mailto:sd259@cornell.edu?Subject=PixelFECSupervisor Panel Low Level GUI\">Souvik Das</a>"<<std::endl;
	*out<<"</p>"<<std::endl;

	*out<<"</body>"<<endl;
	*out<<"</html>"<<endl;
}

void PixelFECSupervisor::ROC (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{
	cgicc::Cgicc cgi(in);

        std::string vmeBaseAddress_string=cgi.getElement("FECBaseAddress")->getValue();
	std::string rocName_string=cgi.getElement("ROCName")->getValue();

        PixelROCName rocName(rocName_string);
	PixelModuleName moduleName(rocName_string);

        const PixelHdwAddress* roc_hdwaddress=theNameTranslation_->getHdwAddress(rocName);
        unsigned int mFEC=roc_hdwaddress->mfec();
        unsigned int mFECChannel=roc_hdwaddress->mfecchannel();
        unsigned int hub=roc_hdwaddress->hubaddress();
	unsigned int port=roc_hdwaddress->portaddress();
	unsigned int rocid=roc_hdwaddress->rocid();
        unsigned long vmeBaseAddress=atol(vmeBaseAddress_string.c_str());

	PixelROCDACSettings *dacSettings=theDACs_[moduleName]->getDACSettings(rocName);
	assert(dacSettings!=0);

	PixelROCMaskBits *maskSettings=theMasks_[moduleName]->getMaskBits(rocName);
	assert(maskSettings!=0);

	PixelROCTrimBits *trimSettings=theTrims_[moduleName]->getTrimBits(rocName);
	assert(trimSettings!=0);

	std::string url="/"+getApplicationDescriptor()->getURN();

        *out<<"<html>"<<endl;
	*out<<"<head>"<<std::endl;
	*out<<"	<script language=\"JavaScript\" src=\"/pixel/PixelFECSupervisor/html/slider.js\"></script>"<<std::endl;
	*out<<" <script language=\"JavaScript\" src=\"/pixel/PixelFECSupervisor/html/ROCGUIAJAX.js\"></script>"<<std::endl;

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
	*out<<"		's_imgControl' : '/pixel/PixelFECSupervisor/html/DAC_255_bg.gif',"<<std::endl;
	*out<<"		's_imgSlider' : '/pixel/PixelFECSupervisor/html/DAC_sl.gif',"<<std::endl;
	*out<<"		'n_zIndex' : 1 "<<std::endl;
	*out<<"	}"<<std::endl;

	*out<<"	var B_TPL = {"<<std::endl;
	*out<<"		'b_vertical' : false, "<<std::endl;
	*out<<"		'b_watch' : true, "<<std::endl;
	*out<<"		'n_controlWidth': 15, "<<std::endl;
	*out<<"		'n_controlHeight': 16, "<<std::endl;
	*out<<"		'n_sliderWidth' : 19, "<<std::endl;
	*out<<"		'n_sliderHeight' : 16, "<<std::endl;
	*out<<"		'n_pathLeft' : 1, "<<std::endl;
	*out<<"		'n_pathTop' : 0, "<<std::endl;
	*out<<"		'n_pathLength' : 15, "<<std::endl;
	*out<<"		's_imgControl' : '/pixel/PixelFECSupervisor/html/DAC_255_bg.gif',"<<std::endl;
	*out<<"		's_imgSlider' : '/pixel/PixelFECSupervisor/html/DAC_sl.gif',"<<std::endl;
	*out<<"		'n_zIndex' : 1 "<<std::endl;
	*out<<"	}"<<std::endl<<std::endl;

	*out<<"    function returnValue(DACAddress, value){"<<endl;
        *out<<"     DAC_AJAX('"<<url<<"', '"<<vmeBaseAddress<<"', '"<<mFEC<<"', '"<<mFECChannel<<"', '"<<hub<<"', '"<<port<<"', '"<<rocid<<"', '"<<rocName_string<<"', DACAddress, value);"<<endl;
        *out<<"    }"<<endl;

	*out<<"	function pixelAction(pixelid, row, column, mask, trim){"<<std::endl;
	*out<<"		if (Command=='Cal_Pix_Capacitor') {"<<std::endl;
	*out<<"			document.getElementById(pixelid).bgColor=\"green\";"<<std::endl;
	*out<<"			Cal_Pix_AJAX('"<<url<<"', '"<<vmeBaseAddress<<"', '"<<mFEC<<"', '"<<mFECChannel<<"', '"<<hub<<"', '"<<port<<"', '"<<rocid<<"', '"<<rocName_string<<"', row, column, '1');"<<std::endl;
	*out<<"		} else if (Command=='Cal_Pix_Sensor') {"<<std::endl;
	*out<<"			document.getElementById(pixelid).bgColor=\"red\";"<<std::endl;
	*out<<"			Cal_Pix_AJAX('"<<url<<"', '"<<vmeBaseAddress<<"', '"<<mFEC<<"', '"<<mFECChannel<<"', '"<<hub<<"', '"<<port<<"', '"<<rocid<<"', '"<<rocName_string<<"', row, column, '2');"<<std::endl;
	*out<<"		} else if (Command=='Prog_Pix') {"<<std::endl;
	std::string tempString;
	tempString="Mask <input type=text size=\\\"1\\\" id=\\\"mask\\\" value=\"+mask+\"><br>";
	//tempString="Mask <select id=\\\"mask\\\"> <option id=\\\"mask0\\\" value=\\\"0\\\">0 <option id=\\\"mask1\\\" value=\\\"1\\\">1 </select>";
	//tempString+="<script type=\\\"text/javascript\\\"> if (mask==0) {document.getElementById('mask0').selected=\\\"selected\\\";} else {document.getElementById('mask1').selected=\\\"selected\\\";}</script>";
	tempString+="Trim <input type=text size=\\\"1\\\" id=\\\"trim\\\" value=\"+trim+\"><br>";
	tempString+="<button onclick=\\\"Prog_Pix_Entered('\"+pixelid+\"', \"+row+\", \"+column+\")\\\">Click</button>";
	*out<<"			document.getElementById(pixelid).innerHTML=\""<<tempString<<"\";"<<std::endl;
	*out<<"		}"<<std::endl;
	*out<<"	}"<<endl;

	*out<<"	function Prog_Pix_Entered(pixelid, row, column){"<<std::endl;
	*out<<"		var mask=document.getElementById('mask').value;"<<std::endl;
	*out<<"         var trim=document.getElementById('trim').value;"<<std::endl;
	std::string tempString1="<a title=\\\"row='\"+row+\"' column='\"+column+\"'\\\" onclick=\\\"pixelAction('\"+pixelid+\"', \"+row+\", \"+column+\", \"+mask+\", \"+trim+\")\\\"><i>\"+parseInt(trim)+\"</i></a>";
	std::string tempString2="<a title=\\\"row='\"+row+\"' column='\"+column+\"'\\\" onclick=\\\"pixelAction('\"+pixelid+\"', \"+row+\", \"+column+\", \"+mask+\", \"+trim+\")\\\"><b>\"+parseInt(trim)+\"</b></a>";
	*out<<"		if (mask==0) {"<<std::endl;
	*out<<"		 document.getElementById(pixelid).innerHTML=\""<<tempString1<<"\";"<<std::endl;
	*out<<"         } else if (mask==1) {"<<std::endl;
	*out<<"          document.getElementById(pixelid).innerHTML=\""<<tempString2<<"\";"<<std::endl;
        *out<<"         }"<<std::endl;
	*out<<"		Prog_Pix_AJAX('"<<url<<"', '"<<vmeBaseAddress<<"', '"<<mFEC<<"', '"<<mFECChannel<<"', '"<<hub<<"', '"<<port<<"', '"<<rocid<<"', '"<<rocName_string<<"',row, column, mask, trim);"<<std::endl;
	*out<<"	}"<<std::endl<<std::endl;

	*out<<"	function ClrCal() {"<<std::endl;
	*out<<"		ClrCal_AJAX('"<<url<<"', '"<<vmeBaseAddress<<"', '"<<mFEC<<"', '"<<mFECChannel<<"', '"<<hub<<"', '"<<port<<"', '"<<rocid<<"', '"<<rocName_string<<"');"<<std::endl;
	*out<<"		var pixelid_string;"<<std::endl;
	*out<<"		for (var pixelid=0;pixelid<4159;++pixelid) {"<<std::endl;
	*out<<"			pixelid_string=\"pixel\"+pixelid;"<<std::endl;
	*out<<"			document.getElementById(pixelid_string).bgColor=\"lightblue\";"<<std::endl;
	*out<<"		}"<<std::endl;
	*out<<"	}"<<std::endl<<std::endl;

	*out<<"	</script>"<<std::endl;

	*out<<"</head>"<<std::endl;

	*out<<"<body>"<<endl;

	*out<<"<h2> ROC: "<<rocName_string<<" </h2>"<<std::endl;
        *out<<"<h3> FEC "<<hex<<vmeBaseAddress<<dec<<", mFEC "<<mFEC<<" "<<mFECChannel<<", Hub "<<hub<<", Port "<<port<<", ROC "<<rocid<<" </h3>"<<endl;

	*out<<"<table border=1 width=100%>"<<endl;
	*out<<" <tr>"<<endl;
	*out<<"  <td align=center rowspan=28 colspan=3>"<<std::endl;
	*out<<"   <table border=0>"<<endl;
	for (unsigned int row=0; row<80; ++row){
		*out<<"    <tr>"<<std::endl;
		for (unsigned int column=0; column<=51; ++column){
			unsigned int pixelid=row*52+column;
			unsigned int mask=maskSettings->mask(column, row);
			unsigned int trim=trimSettings->trim(column, row);
			*out<<"     <td id=\"pixel"<<pixelid<<"\" align=center bgColor=lightblue>"<<endl;
			*out<<"      <a title=\"row='"<<row<<"' column='"<<column<<"'\" onclick=\"pixelAction('pixel"<<pixelid<<"', '"<<row<<"', '"<<column<<"', '"<<mask<<"', '"<<trim<<"')\">";
			if (mask==0) *out<<"<i>"; else if (mask==1) *out<<"<b>";
			*out<<trim;
			if (mask==0) *out<<"</i>"; else if (mask==1) *out<<"</b>";
			*out<<"</a>"<<std::endl;
			*out<<"     </td>"<<endl;
		}
		*out<<"    </tr>"<<endl;
	}
	*out<<"   </table>"<<endl;
	*out<<"  </td>"<<endl;

#if 0
	*out<<"  <td>"<<endl;
	ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "Vdd", "1", dacSettings->getVdd(), 4);
	*out<<"  </td>"<<endl;
	*out<<" </tr>"<<endl;

	*out<<" <tr><td>"<<endl;
	ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "Vana", "2", dacSettings->getVana(), 8);
	*out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "Vsf", "3", dacSettings->getVsf(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "Vcomp", "4", dacSettings->getVcomp(), 4);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "Vleak", "5", dacSettings->getVleak(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "VrgPr", "6", dacSettings->getVrgPr(), 4);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "VwllPr", "7", dacSettings->getVwllPr(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "VrgSh", "8", dacSettings->getVrgSh(), 4);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "VwllSh", "9", dacSettings->getVwllSh(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "VHldDel", "10", dacSettings->getVHldDel(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "Vtrim", "11", dacSettings->getVtrim(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "VcThr", "12", dacSettings->getVcThr(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "VIbias_bus", "13", dacSettings->getVIbias_bus(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "VIbias_sf", "14", dacSettings->getVIbias_sf(), 4);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "VOffsetOp", "15", dacSettings->getVOffsetOp(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "VbiasOp", "16", dacSettings->getVbiasOp(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "VOffsetRO", "17", dacSettings->getVOffsetRO(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "VIon", "18", dacSettings->getVIon(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "VIbias_PH", "19", dacSettings->getVIbias_PH(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "VIbias_DAC", "20", dacSettings->getVIbias_DAC(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "VIbias_roc", "21", dacSettings->getVIbias_roc(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "VIColOr", "22", dacSettings->getVIColOr(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "Vnpix", "23", dacSettings->getVnpix(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "VsumCol", "24", dacSettings->getVsumCol(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "Vcal", "25", dacSettings->getVcal(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "CalDel", "26", dacSettings->getCalDel(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "WBC", "254", dacSettings->getWBC(), 8);
        *out<<" </td></tr>"<<endl;

	*out<<" <tr><td>"<<endl;
        ROCDACs(out, url, vmeBaseAddress_string, mFEC, mFECChannel, hub, port, rocid, "ChipControlRegister", "253", dacSettings->getControlRegister(), 8);
        *out<<" </td></tr>"<<endl;
#endif

	*out<<"  <td align=center>"<<endl;
	*out<<"    Program Pixels"<<endl;
	*out<<"    <input name=\"Command\" type=\"radio\" value=\"Prog_Pix\" onclick=\"Command='Prog_Pix'\">"<<endl;
	*out<<"  </td>"<<endl;
	*out<<"  <td align=center>"<<endl;
	*out<<"   Calibration Pulses to Pixels using:<br>"<<endl;
	*out<<"   <input name=\"Command\" type=\"radio\" onclick=\"Command='Cal_Pix_Capacitor'\">Calibration Capacitor"<<std::endl;
	*out<<"   <input name=\"Command\" type=\"radio\" onclick=\"Command='Cal_Pix_Sensor'\">Sensor Bumps"<<std::endl;
	*out<<"  </td>"<<endl;
	*out<<"  <td align=center>"<<endl;
	*out<<"   <input name=\"Command\" type=\"button\" value=\"ClrCal\" onclick=\"ClrCal()\">"<<std::endl;
	*out<<"  </td>"<<endl;
	*out<<"  <td align=center>"<<endl;
	*out<<"   <button onclick=\"FileDACs('"<<url<<"', '"<<rocName_string<<"')\">File Current DAC Settings</button><br/>"<<std::endl;
	*out<<"   <button onclick=\"FileMasks('"<<url<<"', '"<<rocName_string<<"')\">File Current Mask Settings</button><br/>"<<std::endl;
	*out<<"   <button onclick=\"FileTrims('"<<url<<"', '"<<rocName_string<<"')\">File Current Trim Settings</button>"<<std::endl;
	*out<<"  </td>"<<endl;
	*out<<" </tr>"<<endl;
	*out<<"</table>"<<endl;

	*out<<"<p align=right>"<<std::endl;
	*out<<"Please send bug reports and suggestions to "<<std::endl;
	*out<<"<a href=\"mailto:sd259@cornell.edu?Subject=PixelFECSupervisor ROC Low Level GUI\">Souvik Das</a>"<<std::endl;
	*out<<"</p>"<<std::endl;

	*out<<"</body>"<<std::endl;
	*out<<"</html>"<<std::endl;

}

void PixelFECSupervisor::ROCDACs (xgi::Output *out,
					std::string url,
					std::string FECBaseAddress,
					unsigned int mFEC,
					unsigned int mFECChannel,
					unsigned int hub,
					unsigned int port,
					unsigned int rocid,
					std::string dacname,
					std::string dacaddress,
					unsigned int dacvalue,
					unsigned int bits){

        //*out<<"  <form name=\"input"<<dacaddress<<"\" method=\"get\" action=\""<<url+"/ROC_XgiHandler"<<"\" enctype=\"multipart/form-data\">"<<std::endl;
        *out<<"  <form name=\"input"<<dacaddress<<"\">"<<std::endl;
        *out<<"   "<<dacname<<endl;
        *out<<"   <input type=\"text\" name=\""<<dacaddress<<"\" id=\"DACAddress"<<dacaddress<<"\" size=\"3\" onchange=\"returnValue('"<<dacaddress<<"', this.value)\"/>"<<endl;
        *out<<"   <script language=\"JavaScript\">"<<endl;
        *out<<"    var INIT_"<<dacname<<"={"<<endl;
        *out<<"     's_form' : 'input"<<dacaddress<<"', "<<std::endl;
        *out<<"     's_name' : 'DACAddress"<<dacaddress<<"',"<<std::endl;
        *out<<"     'n_minValue' : 0, "<<std::endl;
        if (bits==4) *out<<"     'n_maxValue' : 15, "<<std::endl;
	else if (bits==8) *out<<"     'n_maxValue' : 255, "<<std::endl;
        *out<<"     'n_value' : "<<dacvalue<<","<<std::endl;
        *out<<"     'n_step' : 1 "<<std::endl;
        *out<<"    }"<<std::endl;
	if (bits==4) *out<<"    new slider(INIT_"<<dacname<<", B_TPL);"<<endl;
	else if (bits==8) *out<<"    new slider(INIT_"<<dacname<<", A_TPL);"<<endl;
        *out<<"   </script>"<<std::endl;
       /* *out<<"   <input type=\"hidden\" name=\"FECBaseAddress\" value=\""<<FECBaseAddress<<"\"/>"<<endl;
        *out<<"   <input type=\"hidden\" name=\"mFEC\" value=\""<<mFEC<<"\"/>"<<endl;
        *out<<"   <input type=\"hidden\" name=\"mFECChannel\" value=\""<<mFECChannel<<"\"/>"<<endl;
        *out<<"   <input type=\"hidden\" name=\"hub\" value=\""<<hub<<"\"/>"<<endl;
        *out<<"   <input type=\"hidden\" name=\"port\" value=\""<<port<<"\"/>"<<endl;
        *out<<"   <input type=\"hidden\" name=\"rocid\" value=\""<<rocid<<"\"/>"<<endl;*/
        *out<<"  </form>"<<endl;
}

void PixelFECSupervisor::StateMachineXgiHandler (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{
	cgicc::Cgicc cgi(in);

	std::string Command=cgi.getElement("Command")->getValue();

	if (Command=="Initialize")
	{
	  xoap::MessageReference msg=MakeSOAPMessageReference("Initialize");
	  xoap::MessageReference reply=Initialize(msg);
	  if (Receive(reply)!="InitializeDone")
	  {
std::string const msg_error_qtg = "PixelFECSupervisor::StateMachineXgiHandler - PixelFECSupervisor for crate #"+stringF(crate_)+" could not be initialized!";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_qtg);
	 
	  }
        }	  
	else if (Command=="Configure")
	{
		Attribute_Vector parametersXgi(1);
		parametersXgi.at(0).name_="GlobalKey"; parametersXgi.at(0).value_=cgi.getElement("GlobalKey")->getValue();

		xoap::MessageReference msg = MakeSOAPMessageReference("Configure", parametersXgi);
		xoap::MessageReference reply = Configure(msg);
		if (Receive(reply)!="ConfigureDone")
		  {
std::string const msg_error_fta = "The FEC could not be Configured!";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_fta);
	     
		  }
	}
	else if (Command=="Start")
	{
		xoap::MessageReference msg = MakeSOAPMessageReference("Start");
		xoap::MessageReference reply = Start(msg);
		if (Receive(reply)!="StartDone")
		  {
std::string const msg_error_gza = "The FEC could not be Started!";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_gza);
		    
		  }
	}
	else if (Command=="Pause")
	{
		xoap::MessageReference msg = MakeSOAPMessageReference("Pause");
		xoap::MessageReference reply = Pause(msg);
		if (Receive(reply)!="PauseDone")
		  {
std::string const msg_error_ynt = "The FEC could not be Paused!";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_ynt);
		    
		  }
	}
	else if (Command=="Resume")
	{
		xoap::MessageReference msg = MakeSOAPMessageReference("Resume");
		xoap::MessageReference reply = Resume(msg);
		if (Receive(reply)!="ResumeDone")
		  {
std::string const msg_error_las = "The FEC could not be Resumed!";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_las);
		
		  }
	}
	else if (Command=="Halt")
	{
		xoap::MessageReference msg = MakeSOAPMessageReference("Halt");
		xoap::MessageReference reply = Halt(msg);
		if (Receive(reply)!="HaltDone")
		  {
std::string const msg_error_wjx = "The FEC could not be Halted!";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_wjx);
		    
		  }
	}
	else if (Command=="DumpPowerMap") {
	  for (int l=0; l < 2; ++l)
	    for (int i=0; i<2; ++i)
	      for (int j=0; j<2; ++j)
		for (int k=0; k<2; ++k) {
		  if (l == 0)
		    std::cout << "LV: i=" << i << " j=" << j << " k=" << k << "  -> " << powerMap_.getVoltage(i,j,k) << std::endl;
		  else 
		    std::cout << "HV: i=" << i << " j=" << j << " k=" << k << "  -> " << powerMap_.getHVoltage(i,j,k) << std::endl;
		}
	}

	this->Default(in, out);
}

void PixelFECSupervisor::mFECsHubs_XgiHandler (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{
	cgicc::Cgicc cgi(in);

	std::string Command=cgi.getElement("Command")->getValue();
	std::string vmeBaseAddress_string=cgi.getElement("FECBaseAddress")->getValue();

	if (Command=="TBMCommand")
	{
		Attribute_Vector parametersXgi(9);
		parametersXgi.at(0).name_="mFEC";               parametersXgi.at(0).value_=cgi.getElement("mFEC")->getValue();
                parametersXgi.at(1).name_="mFECChannel";        parametersXgi.at(1).value_=cgi.getElement("mFECChannel")->getValue();
		parametersXgi.at(2).name_="TBMChannel";		parametersXgi.at(2).value_=cgi.getElement("TBMChannel")->getValue();
                parametersXgi.at(3).name_="HubAddress";         parametersXgi.at(3).value_=cgi.getElement("HubAddress")->getValue();
                parametersXgi.at(4).name_="PortAddress";        parametersXgi.at(4).value_=cgi.getElement("PortAddress")->getValue();
                parametersXgi.at(5).name_="Offset";             parametersXgi.at(5).value_=cgi.getElement("Offset")->getValue();
                parametersXgi.at(6).name_="DataByte";           parametersXgi.at(6).value_=cgi.getElement("DataByte")->getValue();
                parametersXgi.at(7).name_="Direction";          parametersXgi.at(7).value_=cgi.getElement("Direction")->getValue();
		parametersXgi.at(8).name_="VMEBaseAddress";	parametersXgi.at(8).value_=vmeBaseAddress_string;

		xoap::MessageReference msg = MakeSOAPMessageReference("TBMCommand", parametersXgi);
		xoap::MessageReference reply = TBMCommand(msg);
		if (Receive(reply)!="TBMCommandDone")
		  {
std::string const msg_error_jta = "TBMCommand could not be executed!";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_jta);
		    
		  }
	}
	else if (Command=="Prog_Pix")
	{
		Attribute_Vector parametersXgi(10);
		parametersXgi.at(0).name_="mFEC";		parametersXgi.at(0).value_=cgi.getElement("mFEC")->getValue();
		parametersXgi.at(1).name_="mFECChannel";	parametersXgi.at(1).value_=cgi.getElement("mFECChannel")->getValue();
		parametersXgi.at(2).name_="HubAddress";         parametersXgi.at(2).value_=cgi.getElement("HubAddress")->getValue();
		parametersXgi.at(3).name_="PortAddress";        parametersXgi.at(3).value_=cgi.getElement("PortAddress")->getValue();
		parametersXgi.at(4).name_="ROCId";              parametersXgi.at(4).value_=cgi.getElement("ROCId")->getValue();
		parametersXgi.at(5).name_="Column";             parametersXgi.at(5).value_=cgi.getElement("Column")->getValue();
		parametersXgi.at(6).name_="Row";                parametersXgi.at(6).value_=cgi.getElement("Row")->getValue();
		parametersXgi.at(7).name_="Mask";               parametersXgi.at(7).value_=cgi.getElement("Mask")->getValue();
		parametersXgi.at(8).name_="Trim";               parametersXgi.at(8).value_=cgi.getElement("Trim")->getValue();
		parametersXgi.at(9).name_="VMEBaseAddress";	parametersXgi.at(9).value_=vmeBaseAddress_string;

		xoap::MessageReference msg = MakeSOAPMessageReference ("Prog_Pix", parametersXgi);
		xoap::MessageReference reply = Prog_Pix(msg);
		if (Receive(reply)!="Prog_PixDone")
		  {
std::string const msg_error_arq = "Prog_Pix could not be executed!";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_arq);
		   
		  }
	}
	else if (Command=="Prog_DAC")
	{
		Attribute_Vector parametersXgi(8);
                parametersXgi.at(0).name_="mFEC";               parametersXgi.at(0).value_=cgi.getElement("mFEC")->getValue();
                parametersXgi.at(1).name_="mFECChannel";        parametersXgi.at(1).value_=cgi.getElement("mFECChannel")->getValue();
                parametersXgi.at(2).name_="HubAddress";         parametersXgi.at(2).value_=cgi.getElement("HubAddress")->getValue();
                parametersXgi.at(3).name_="PortAddress";        parametersXgi.at(3).value_=cgi.getElement("PortAddress")->getValue();
                parametersXgi.at(4).name_="ROCId";              parametersXgi.at(4).value_=cgi.getElement("ROCId")->getValue();
                parametersXgi.at(5).name_="DACAddress";         parametersXgi.at(5).value_=cgi.getElement("DACAddress")->getValue();
                parametersXgi.at(6).name_="DACValue";           parametersXgi.at(6).value_=cgi.getElement("DACValue")->getValue();
		parametersXgi.at(7).name_="VMEBaseAddress";	parametersXgi.at(7).value_=vmeBaseAddress_string;

                xoap::MessageReference msg = MakeSOAPMessageReference ("Prog_DAC", parametersXgi);
                xoap::MessageReference reply = Prog_DAC(msg);
                if (Receive(reply)!="Prog_DACDone")
		  {
std::string const msg_error_jli = "Prog_DAC could not be executed!";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_jli);
		    
		  }
	}
	else if (Command=="Cal_Pix")
	{
		Attribute_Vector parametersXgi(9);
                parametersXgi.at(0).name_="mFEC";               parametersXgi.at(0).value_=cgi.getElement("mFEC")->getValue();
                parametersXgi.at(1).name_="mFECChannel";        parametersXgi.at(1).value_=cgi.getElement("mFECChannel")->getValue();
                parametersXgi.at(2).name_="HubAddress";         parametersXgi.at(2).value_=cgi.getElement("HubAddress")->getValue();
                parametersXgi.at(3).name_="PortAddress";        parametersXgi.at(3).value_=cgi.getElement("PortAddress")->getValue();
                parametersXgi.at(4).name_="ROCId";              parametersXgi.at(4).value_=cgi.getElement("ROCId")->getValue();
                parametersXgi.at(5).name_="Column";             parametersXgi.at(5).value_=cgi.getElement("Column")->getValue();
                parametersXgi.at(6).name_="Row";                parametersXgi.at(6).value_=cgi.getElement("Row")->getValue();
                parametersXgi.at(7).name_="CalData";            parametersXgi.at(7).value_=cgi.getElement("CalData")->getValue();
		parametersXgi.at(8).name_="VMEBaseAddress";	parametersXgi.at(8).value_=vmeBaseAddress_string;

                xoap::MessageReference msg = MakeSOAPMessageReference ("Cal_Pix", parametersXgi);
                xoap::MessageReference reply = Cal_Pix(msg);
                if (Receive(reply)!="Cal_PixDone")
		  {
std::string const msg_error_okn = "Cal_Pix could not be executed!";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_okn);
		   
		  }
	}
	else if (Command=="ClrCal")
	{
		Attribute_Vector parametersXgi(6);
                parametersXgi.at(0).name_="mFEC";               parametersXgi.at(0).value_=cgi.getElement("mFEC")->getValue();
                parametersXgi.at(1).name_="mFECChannel";        parametersXgi.at(1).value_=cgi.getElement("mFECChannel")->getValue();
                parametersXgi.at(2).name_="HubAddress";         parametersXgi.at(2).value_=cgi.getElement("HubAddress")->getValue();
                parametersXgi.at(3).name_="PortAddress";        parametersXgi.at(3).value_=cgi.getElement("PortAddress")->getValue();
                parametersXgi.at(4).name_="ROCId";              parametersXgi.at(4).value_=cgi.getElement("ROCId")->getValue();
		parametersXgi.at(5).name_="VMEBaseAddress";	parametersXgi.at(5).value_=vmeBaseAddress_string;

                xoap::MessageReference msg = MakeSOAPMessageReference ("ClrCal", parametersXgi);
                xoap::MessageReference reply = ClrCal(msg);
                if (Receive(reply)!="ClrCalDone")
		  {
std::string const msg_error_guc = "ClrCal could not be executed!";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_guc);
		    
		  }
	}
	this->mFECsHubs(in, out);
}

void PixelFECSupervisor::Panel_XgiHandler (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{
	cgicc::Cgicc cgi(in);

        std::string Command=cgi.getElement("Command")->getValue();
	std::string vmeBaseAddress_string=cgi.getElement("FECBaseAddress")->getValue();
	std::string mFEC_string=cgi.getElement("mFEC")->getValue();
	std::string mFECChannel_string=cgi.getElement("mFECChannel")->getValue();

	if (Command=="ResetROCs") {
		Attribute_Vector parametersXgi(5);
		parametersXgi.at(0).name_="mFEC";		parametersXgi.at(0).value_=mFEC_string;
		parametersXgi.at(1).name_="mFECChannel";	parametersXgi.at(1).value_=mFECChannel_string;
		parametersXgi.at(2).name_="TBMChannel";		parametersXgi.at(2).value_=cgi.getElement("TBMChannel")->getValue();
		parametersXgi.at(3).name_="HubAddress";		parametersXgi.at(3).value_=cgi.getElement("HubAddress")->getValue();
		parametersXgi.at(4).name_="VMEBaseAddress";	parametersXgi.at(4).value_=vmeBaseAddress_string;

		xoap::MessageReference msg=MakeSOAPMessageReference("ResetROCs", parametersXgi);
		xoap::MessageReference reply = ResetROCs(msg);
		if (Receive(reply)!="ResetROCsDone")
		  {
std::string const msg_error_rfh = "Reset All ROCs could not be done.";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_rfh);
		    
		  }
	} else if (Command=="ResetTBM") {
		Attribute_Vector parametersXgi(5);
		parametersXgi.at(0).name_="mFEC";		parametersXgi.at(0).value_=mFEC_string;
		parametersXgi.at(1).name_="mFECChannel";	parametersXgi.at(1).value_=mFECChannel_string;
		parametersXgi.at(2).name_="TBMChannel";		parametersXgi.at(2).value_=cgi.getElement("TBMChannel")->getValue();
		parametersXgi.at(3).name_="HubAddress";		parametersXgi.at(3).value_=cgi.getElement("HubAddress")->getValue();
		parametersXgi.at(4).name_="VMEBaseAddress";	parametersXgi.at(4).value_=vmeBaseAddress_string;

		xoap::MessageReference msg=MakeSOAPMessageReference("ResetTBM", parametersXgi);
		xoap::MessageReference reply = ResetTBM(msg);
		if (Receive(reply)!="ResetTBMDone")
		  {
std::string const msg_error_ipt = "Reset TBM could not be done.";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_ipt);
		   
		  }
	}
}

void PixelFECSupervisor::ROC_XgiHandler (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{
	cgicc::Cgicc cgi(in);

        std::string Command=cgi.getElement("Command")->getValue();
	std::string rocName_string=cgi.getElement("PixelROCName")->getValue();

	if (Command=="Prog_DAC") {

		std::string dacAddress_string=cgi.getElement("DACAddress")->getValue();
                std::string dacValue_string=cgi.getElement("DACValue")->getValue();
                unsigned int dacAddress=atoi(dacAddress_string.c_str());
                unsigned int dacValue=atoi(dacValue_string.c_str());

	        Attribute_Vector parametersXgi(8);
                parametersXgi.at(0).name_="mFEC";               parametersXgi.at(0).value_=cgi.getElement("mFEC")->getValue();
                parametersXgi.at(1).name_="mFECChannel";        parametersXgi.at(1).value_=cgi.getElement("mFECChannel")->getValue();
                parametersXgi.at(2).name_="HubAddress";         parametersXgi.at(2).value_=cgi.getElement("HubAddress")->getValue();
                parametersXgi.at(3).name_="PortAddress";        parametersXgi.at(3).value_=cgi.getElement("PortAddress")->getValue();
                parametersXgi.at(4).name_="ROCId";              parametersXgi.at(4).value_=cgi.getElement("ROCId")->getValue();
                parametersXgi.at(5).name_="DACAddress";         parametersXgi.at(5).value_=dacAddress_string;
                parametersXgi.at(6).name_="DACValue";           parametersXgi.at(6).value_=dacValue_string;
                parametersXgi.at(7).name_="VMEBaseAddress";     parametersXgi.at(7).value_=cgi.getElement("FECBaseAddress")->getValue();

                xoap::MessageReference msg = MakeSOAPMessageReference ("Prog_DAC", parametersXgi);
                xoap::MessageReference reply = Prog_DAC(msg);
                if (Receive(reply)!="Prog_DACDone")
		  {
std::string const msg_error_uod = "Prog_DAC could not be executed!";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_uod);
		    //out<<"Prog_DAC could not be executed!"<<endl;
		  }
		PixelModuleName moduleName(rocName_string);
                PixelROCName rocName(rocName_string);
                PixelROCDACSettings *dacSettings=theDACs_[moduleName]->getDACSettings(rocName);
                assert(dacSettings!=0);
                dacSettings->setDAC(dacAddress, dacValue);

        } else if (Command=="Cal_Pix") {

		Attribute_Vector parametersXgi(9);
		parametersXgi.at(0).name_="mFEC";		parametersXgi.at(0).value_=cgi.getElement("mFEC")->getValue();
		parametersXgi.at(1).name_="mFECChannel";	parametersXgi.at(1).value_=cgi.getElement("mFECChannel")->getValue();
		parametersXgi.at(2).name_="HubAddress";		parametersXgi.at(2).value_=cgi.getElement("HubAddress")->getValue();
		parametersXgi.at(3).name_="PortAddress";	parametersXgi.at(3).value_=cgi.getElement("PortAddress")->getValue();
		parametersXgi.at(4).name_="ROCId";		parametersXgi.at(4).value_=cgi.getElement("ROCId")->getValue();
		parametersXgi.at(5).name_="Column";		parametersXgi.at(5).value_=cgi.getElement("Column")->getValue();
		parametersXgi.at(6).name_="Row";		parametersXgi.at(6).value_=cgi.getElement("Row")->getValue();
		parametersXgi.at(7).name_="CalData";		parametersXgi.at(7).value_=cgi.getElement("CalData")->getValue();
		parametersXgi.at(8).name_="VMEBaseAddress";	parametersXgi.at(8).value_=cgi.getElement("FECBaseAddress")->getValue();

		xoap::MessageReference msg = MakeSOAPMessageReference ("Cal_Pix", parametersXgi);
                xoap::MessageReference reply = Cal_Pix(msg);
                if (Receive(reply)!="Cal_PixDone")
		  {
std::string const msg_error_oul = "Cal_Pix could not be executed!";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_oul);
		    
		  }
	} else if (Command=="Prog_Pix") {

		std::string column_string=cgi.getElement("Column")->getValue();
		std::string row_string=cgi.getElement("Row")->getValue();
		std::string mask_string=cgi.getElement("Mask")->getValue();
		std::string trim_string=cgi.getElement("Trim")->getValue();
		unsigned int column=atoi(column_string.c_str());
		unsigned int row=atoi(row_string.c_str());
		unsigned int mask=atoi(mask_string.c_str());
		unsigned int trim=atoi(trim_string.c_str());

		Attribute_Vector parametersXgi(10);
		parametersXgi.at(0).name_="mFEC";		parametersXgi.at(0).value_=cgi.getElement("mFEC")->getValue();
		parametersXgi.at(1).name_="mFECChannel";	parametersXgi.at(1).value_=cgi.getElement("mFECChannel")->getValue();
		parametersXgi.at(2).name_="HubAddress";		parametersXgi.at(2).value_=cgi.getElement("HubAddress")->getValue();
		parametersXgi.at(3).name_="PortAddress";	parametersXgi.at(3).value_=cgi.getElement("PortAddress")->getValue();
		parametersXgi.at(4).name_="ROCId";		parametersXgi.at(4).value_=cgi.getElement("ROCId")->getValue();
		parametersXgi.at(5).name_="Column";		parametersXgi.at(5).value_=column_string;
		parametersXgi.at(6).name_="Row";		parametersXgi.at(6).value_=row_string;
		parametersXgi.at(7).name_="Mask";		parametersXgi.at(7).value_=mask_string;
		parametersXgi.at(8).name_="Trim";		parametersXgi.at(8).value_=trim_string;
		parametersXgi.at(9).name_="VMEBaseAddress";	parametersXgi.at(9).value_=cgi.getElement("FECBaseAddress")->getValue();

		xoap::MessageReference msg = MakeSOAPMessageReference ("Prog_Pix", parametersXgi);
                xoap::MessageReference reply = Prog_Pix(msg);
                if (Receive(reply)!="Prog_PixDone")
		  {
std::string const msg_error_tag = "Prog_Pix could not be executed!";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_tag);
		    
		  }
		PixelModuleName moduleName(rocName_string);
                PixelROCName rocName(rocName_string);
                PixelROCMaskBits *maskSettings=theMasks_[moduleName]->getMaskBits(rocName);
		PixelROCTrimBits *trimSettings=theTrims_[moduleName]->getTrimBits(rocName);
		assert(maskSettings!=0);
		assert(trimSettings!=0);
		maskSettings->setMask(column, row, mask);
		trimSettings->setTrim(column, row, trim);

	} else if (Command=="ClrCal") {

		Attribute_Vector parametersXgi(6);
                parametersXgi.at(0).name_="mFEC";               parametersXgi.at(0).value_=cgi.getElement("mFEC")->getValue();
                parametersXgi.at(1).name_="mFECChannel";        parametersXgi.at(1).value_=cgi.getElement("mFECChannel")->getValue();
                parametersXgi.at(2).name_="HubAddress";         parametersXgi.at(2).value_=cgi.getElement("HubAddress")->getValue();
                parametersXgi.at(3).name_="PortAddress";        parametersXgi.at(3).value_=cgi.getElement("PortAddress")->getValue();
                parametersXgi.at(4).name_="ROCId";              parametersXgi.at(4).value_=cgi.getElement("ROCId")->getValue();
		parametersXgi.at(5).name_="VMEBaseAddress";	parametersXgi.at(5).value_=cgi.getElement("FECBaseAddress")->getValue();

                xoap::MessageReference msg = MakeSOAPMessageReference ("ClrCal", parametersXgi);
                xoap::MessageReference reply = ClrCal(msg);
                if (Receive(reply)!="ClrCalDone")
		  {
std::string const msg_error_fkv = "ClrCal could not be executed!";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_fkv);
		   
		  }
	} else if (Command=="ResetROC") {

                Attribute_Vector parametersXgi(6);
                parametersXgi.at(0).name_="mFEC";               parametersXgi.at(0).value_=cgi.getElement("mFEC")->getValue();
                parametersXgi.at(1).name_="mFECChannel";        parametersXgi.at(1).value_=cgi.getElement("mFECChannel")->getValue();
                parametersXgi.at(2).name_="HubAddress";         parametersXgi.at(2).value_=cgi.getElement("HubAddress")->getValue();
                parametersXgi.at(3).name_="PortAddress";        parametersXgi.at(3).value_=cgi.getElement("PortAddress")->getValue();
                parametersXgi.at(4).name_="ROCId";              parametersXgi.at(4).value_=cgi.getElement("ROCId")->getValue();
                parametersXgi.at(5).name_="VMEBaseAddress";     parametersXgi.at(5).value_=cgi.getElement("FECBaseAddress")->getValue();

                xoap::MessageReference msg = MakeSOAPMessageReference ("ResetROC", parametersXgi);
                xoap::MessageReference reply = ResetROCs(msg);
                if (Receive(reply)!="ResetROCsDone")
		  {
std::string const msg_error_yjz = "ROC could not be reset!";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_yjz);
		    
		  }
        } else if (Command=="FileDACs") {

		PixelModuleName moduleName(rocName_string);
		PixelDACSettings *dacSettings=theDACs_[moduleName];
                assert(dacSettings!=0);
		std::string filename="ROC_DAC_module_"+moduleName.modulename()+".dat";
		dacSettings->writeASCII(filename);

	} else if (Command=="FileMasks") {

		PixelModuleName moduleName(rocName_string);
                PixelMaskBase *maskSettings=theMasks_[moduleName];
		assert(maskSettings!=0);
		std::string filename="ROC_Masks_module_"+moduleName.modulename()+".dat";
		maskSettings->writeASCII(filename);

	} else if (Command=="FileTrims") {

		PixelModuleName moduleName(rocName_string);
                PixelTrimBase *trimSettings=theTrims_[moduleName];
		assert(trimSettings!=0);
		std::string filename="ROC_Trims_module_"+moduleName.modulename()+".dat";
		trimSettings->writeASCII(filename);

	}
}

bool PixelFECSupervisor::qpllCheck(toolbox::task::WorkLoop * w1) {
  
  //make local versions of global variables so that we can compare the two
  unsigned int num_qpll_unlocked=0;
  unsigned int num_qpll_locked=0;
  map<unsigned long, int> qpllStatus;


  try {
    //fetch the current status
    for (FECInterfaceMap::const_iterator iFEC=FECInterface.begin();iFEC!=FECInterface.end();++iFEC) {
      const int status=iFEC->second->getStatus();
      qpllStatus[iFEC->first] = status;
      if ((status & 0x2) && !(status & 0x4)) ++num_qpll_locked;
      else                                   ++num_qpll_unlocked;
    }
  }
  catch (HAL::BusAdapterException & e) {
std::string const msg_error_mhm = "QPLL check caught hardware exception: "+string(e.what());
LOG4CPLUS_ERROR(sv_logger_,msg_error_mhm);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_mhm, e);
this->notifyQualified("fatal",f);
    ::sleep(5);
    return true;
    //could return false here and cancel the workloop. depends on what we want in the rare case that this fails
  }
  catch (...) {
std::string const msg_error_mfx = "QPLL check caught unknown exception!";
LOG4CPLUS_ERROR(sv_logger_,msg_error_mfx);
pixel::PixelFECSupervisorException trivial_exception("pixel::PixelFECSupervisorException","module",msg_error_mfx,1318,"PixelFECSupervisor::qpllCheck(toolbox::task::WorkLoop*)");
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_mfx,trivial_exception);
this->notifyQualified("fatal",f);
    ::sleep(5);
    return true; //as above
  }
  
  qplllock_->take();
  if ( num_qpll_unlocked_>0 && num_qpll_unlocked==0 ) {
std::string const msg_info_fwd = "All QPLLs are once again locked";
 LOG4CPLUS_INFO(sv_logger_,msg_info_fwd);
  }else{
  if ( num_qpll_unlocked>0 ) { //there is now a QPLL unlocked
	
    for (map<unsigned long, int>::const_iterator i=qpllStatus.begin(); i!=qpllStatus.end(); ++i) {
      if ( !((i->second & 0x2) && !(i->second & 0x4)) ) {
	//this qpll is unlocked, so compare to the old status
	bool wantoutput = false;
	if ( qpllStatus_.find(i->first) != qpllStatus_.end() ) { //does the old status exist?
	  if ( qpllStatus_[i->first] != i->second ) wantoutput = true; //the status has changed
	}
	else wantoutput=true; //the old status does not exist
	if (wantoutput) { //only print when something is new/changed
	  ostringstream msg;

	  msg<<"FEC at address 0x"<<hex<<i->first<<dec
	     <<" has unlocked QPLL (status=0x"<<hex<<i->second<<dec<<")";
	  std::string const msg_error_djk = msg.str();
	  LOG4CPLUS_ERROR(sv_logger_,msg_error_djk);
	  // we should go to error state here 
	  //::abort(); // added to make it clear, d.k. 8/1/15, disable 25/6/15
	}
      }
    }
  }
}
  //copy the local map into the global one
  for (map<unsigned long, int>::const_iterator i=qpllStatus.begin(); i!=qpllStatus.end(); ++i) 
    qpllStatus_[i->first] = i->second;
  //copy totals into the global totals  
  num_qpll_unlocked_ = num_qpll_unlocked;
  num_qpll_locked_ = num_qpll_locked;
  qplllock_->give();

  ::sleep(3);

  return true;
}

xoap::MessageReference PixelFECSupervisor::Initialize (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
std::string const msg_info_hli = "--- INITIALIZE ---";
 LOG4CPLUS_INFO(sv_logger_,msg_info_hli);

  // Detect PixelSupervisor
  try {
    PixelSupervisor_=getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor("PixelSupervisor", 0);

std::string const msg_info_kel = "PixelFECSupervisor::Initialize - Instance 0 of PixelSupervisor found.";
 LOG4CPLUS_INFO(sv_logger_,msg_info_kel);
  } catch (xdaq::exception::Exception& e) {
std::string const msg_fatal_unu = "PixelFECSupervisor::Initialize - Instance 0 of PixelSupervisor not found!";
LOG4CPLUS_FATAL(sv_logger_,msg_fatal_unu);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_fatal_unu, e);
this->notifyQualified("fatal",f);
    
  }

  // Detect PixelDCSFSMInterface
  try {
    PixelDCSFSMInterface_=getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor("PixelDCSFSMInterface", 0);
    
std::string const msg_info_smz = "PixelFECSupervisor::Initialize - Instance 0 of PixelDCSFSMInterface found.";
 LOG4CPLUS_INFO(sv_logger_,msg_info_smz);
  } catch (xdaq::exception::Exception& e) {
std::string const msg_warn_wbj = "PixelFECSupervisor::Initialize - Instance 0 of PixelDCSFSMInterface not found. Automatic Detector Startup procedure will not be followed. The detector must be powered on manually.";
LOG4CPLUS_WARN(sv_logger_,msg_warn_wbj);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_warn_wbj, e);
this->notifyQualified("fatal",f);
  }
  
  // Detect the PSX Server
  try {
    PixelPSXServer_=getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor("psx", 0);
std::string const msg_info_nzo = "PixelFECSupervisor::Initialize - Instance 0 of PSX server found.";
 LOG4CPLUS_INFO(sv_logger_,msg_info_nzo);
   
  } catch (xdaq::exception::Exception& e) {
std::string const msg_warn_aab = "PixelFECSupervisor::Initialize - Instance 0 of PSX server not found. Automatic Detector Startup procedure will not check for changing currents due to ROC configuration";
LOG4CPLUS_WARN(sv_logger_,msg_warn_aab);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_warn_aab, e);
this->notifyQualified("fatal",f);
  }
  
  // Query PixelDCSFSMInterface with a SOAP message. Update member data after parsing the response.
  if (PixelDCSFSMInterface_!=0) {
    
    Attribute_Vector parametersToSend(3);
    parametersToSend[0].name_="name"; parametersToSend[0].value_="PixelFECSupervisor";
    parametersToSend[1].name_="type"; parametersToSend[1].value_="PxlFEC";
    parametersToSend[2].name_="instance"; parametersToSend[2].value_=itoa(crate_);
    xoap::MessageReference fsmStateResponse=SendWithSOAPReply(PixelDCSFSMInterface_, "fsmStateRequest", parametersToSend);
  
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
		if (fsmState.find("LV_OFF")!=string::npos) powerMap_.setVoltage(powerCoordinate, LV_OFF, std::cout);
	else if (fsmState.find("LV_ON_REDUCED")!=string::npos) powerMap_.setVoltage(powerCoordinate, LV_ON_REDUCED, std::cout);
	else if (fsmState.find("LV_ON")!=string::npos)  powerMap_.setVoltage(powerCoordinate, LV_ON,  std::cout);
	else {
std::string const msg_fatal_uar = "PixelFECSupervisor::Initialize - "+fsmState+" not recognized! (LV)";
 LOG4CPLUS_FATAL(sv_logger_,msg_fatal_uar);
	  assert(0);
	}
	//also look at HV state
	if (fsmState.find("HV_OFF")!=string::npos) powerMap_.setHVoltage(powerCoordinate, HV_OFF, std::cout);
	else if (fsmState.find("HV_ON")!=string::npos)  powerMap_.setHVoltage(powerCoordinate, HV_ON,  std::cout);
	else {
std::string const msg_fatal_ddm = "PixelFECSupervisor::Initialize - "+fsmState+" not recognized! (HV)";
 LOG4CPLUS_FATAL(sv_logger_,msg_fatal_ddm);
	  assert(0);
	}
      }
    }
        
  }

  // Update Finite State Machine
  try {
    toolbox::Event::Reference e(new toolbox::Event("Initialize", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    XCEPT_RETHROW(xoap::exception::Exception, "Invalid State Machine Input.", e);
  }
  
std::string const msg_info_fqd = "PixelFECSupervisor::Intitialize -- Exiting function";
 LOG4CPLUS_INFO(sv_logger_,msg_info_fqd);
  xoap::MessageReference reply = MakeSOAPMessageReference("InitializeDone");

std::string const msg_info_uxd = "--- INITIALIZATION DONE ---";
 LOG4CPLUS_INFO(sv_logger_,msg_info_uxd);
  return reply;
}

xoap::MessageReference PixelFECSupervisor::Configure (xoap::MessageReference msg) throw (xoap::exception::Exception)
{

  std::string const msg_info_zym = "--- CONFIGURE ---";
  LOG4CPLUS_INFO(sv_logger_,msg_info_zym);
  if (state_ != "Halted") return MakeSOAPMessageReference("ConfigureFailed");  
  xoap::MessageReference reply = MakeSOAPMessageReference("ConfigureDone");  
  
  // That's it! Step to the Configuring state, and
  // relegate all further configuring to the stateConfiguring method.
  try {
    toolbox::Event::Reference e(new toolbox::Event("Configure", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    std::string const msg_error_uei = "Failure to fire Configure transition: "+string(e.what());
    LOG4CPLUS_ERROR(sv_logger_,msg_error_uei);
    XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_uei, e);
    this->notifyQualified("fatal",f);
    reply = MakeSOAPMessageReference("ConfigureFailed");
  }

  std::string const m1 = "--- CONFIGURE, exit ---";
  LOG4CPLUS_INFO(sv_logger_,m1);

  return reply;
}

xoap::MessageReference PixelFECSupervisor::Start (xoap::MessageReference msg) throw (xoap::exception::Exception)
{

  std::string const msg_info_pca = "--- START ---";
  LOG4CPLUS_INFO(sv_logger_,msg_info_pca);
  
  xoap::MessageReference reply = MakeSOAPMessageReference("StartDone");
  
  try
	{

	  Attribute_Vector parameter(1);
	  parameter[0].name_="RUN_NUMBER";
	  Receive(msg, parameter);
	  runNumber_=parameter[0].value_;
	  
std::string const msg_info_bhm = "Start Run "+runNumber_;
 LOG4CPLUS_INFO(sv_logger_,msg_info_bhm);


	  //check HV status and prog dacs if necessary
	  startupHVCheck(true);
	  
	  toolbox::Event::Reference e(new toolbox::Event("Start", this));
	  fsm_.fireEvent(e);
	}
  catch (toolbox::fsm::exception::Exception & e)
	{
std::string const msg_error_blm = "Failure to fire Start transition: "+string(e.what());
LOG4CPLUS_ERROR(sv_logger_,msg_error_blm);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_blm, e);
this->notifyQualified("fatal",f);
	  reply = MakeSOAPMessageReference("StartFailed");
        }
  catch (std::exception & e)
	{ 
std::string const msg_error_gle = "Failed to Start run with exception: "+string(e.what());
LOG4CPLUS_ERROR(sv_logger_,msg_error_gle);
std::exception * error_ptr = &e;
pixel::PixelFECSupervisorException *new_exception = dynamic_cast<pixel::PixelFECSupervisorException *> (error_ptr);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_gle, *new_exception);
this->notifyQualified("fatal",f);
	 //exceptions might be thrown by startupHVCheck
	  reply = MakeSOAPMessageReference("StartFailed");
	  //FIXME maybe we should transition to the Error state in case this happens?
	}

  if (theCalibObject_==0) {
    
    if(doTBMReadoutLoop_) {

      phlock_->take(); workloopContinue_=true; phlock_->give();
      workloop_->activate();
std::string const msg_info_ncz = "PixelFECSupervisor::Start. Calib object == 0. Physics data taking workloop activated.";
 LOG4CPLUS_INFO(sv_logger_,msg_info_ncz);
    }
  }


  
	return reply;
}

xoap::MessageReference PixelFECSupervisor::Stop (xoap::MessageReference msg) throw (xoap::exception::Exception)
{


  std::string const msg_info_nyi = "--- STOP ---";
  LOG4CPLUS_INFO(sv_logger_,msg_info_nyi);
  xoap::MessageReference reply = MakeSOAPMessageReference("StopDone");

  if (theCalibObject_==0) {

    if(doTBMReadoutLoop_) {

      if (fsm_.getStateName(fsm_.getCurrentState())=="Running") {
	phlock_->take(); workloopContinue_=false; phlock_->give();
	workloop_->cancel();
	std::string const msg_info_tdp = "PixelFECSupervisor::Stop. Calib object == 0, physics workloop is cancelled.";
	LOG4CPLUS_INFO(sv_logger_,msg_info_tdp);
      } else if (fsm_.getStateName(fsm_.getCurrentState())=="Paused") {
	phlock_->take(); workloopContinue_=false; phlock_->give();
	//workloop_->cancel();
	std::string const msg_info_jhn = "PixelFECSupervisor::Stop. Calib object == 0, physics workloop is cancelled.";
	LOG4CPLUS_INFO(sv_logger_,msg_info_jhn);
      }
    }

  }  // theCalibObject

  try
    {
      startupHVCheck(false); //disable ROCs
      
      toolbox::Event::Reference e(new toolbox::Event("Stop", this));
      fsm_.fireEvent(e);
    }
catch (toolbox::fsm::exception::Exception & e)    {
      XCEPT_RETHROW(xoap::exception::Exception, "Invalid State Machine Input.", e); //FIXME
    }
  catch (std::exception & e)
	{ 
std::string const msg_error_wtc = "Failed to Stop run with exception: "+string(e.what());
LOG4CPLUS_ERROR(sv_logger_,msg_error_wtc);
std::exception * error_ptr = &e;
pixel::PixelFECSupervisorException *new_exception = dynamic_cast<pixel::PixelFECSupervisorException *> (error_ptr);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_wtc, *new_exception);
this->notifyQualified("fatal",f);
     //exceptions might be thrown by startupHVCheck
      reply = MakeSOAPMessageReference("StopFailed");
      //FIXME maybe we should transition to the Error state in case this happens?
    }
  
  calibStateCounter_=0; // dangerous klugde
  return reply;
}

xoap::MessageReference PixelFECSupervisor::Pause (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
std::string const msg_info_erx = "--- PAUSE ---";
 LOG4CPLUS_INFO(sv_logger_,msg_info_erx);

  xoap::MessageReference reply = MakeSOAPMessageReference("PauseDone");

  if (theCalibObject_==0) {

    if(doTBMReadoutLoop_) {
      
      phlock_->take(); workloopContinue_=false; phlock_->give();
      workloop_->cancel();
std::string const msg_info_rtx = "PixelFECSupervisor::Pause. Calib object == 0, physics workloop is cancelled.";
 LOG4CPLUS_INFO(sv_logger_,msg_info_rtx);
    }
  }

	try
	{
	  startupHVCheck(false); //disable ROCs

	  toolbox::Event::Reference e(new toolbox::Event("Pause", this));
	  fsm_.fireEvent(e);
	}
catch (toolbox::fsm::exception::Exception & e)        {
                XCEPT_RETHROW(xoap::exception::Exception, "Invalid Command.", e);
        }
	catch (std::exception & e)
	{ 
std::string const msg_error_pvo = "Failed to Pause run with exception: "+string(e.what());
LOG4CPLUS_ERROR(sv_logger_,msg_error_pvo);
std::exception * error_ptr = &e;
pixel::PixelFECSupervisorException *new_exception = dynamic_cast<pixel::PixelFECSupervisorException *> (error_ptr);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_pvo, *new_exception);
this->notifyQualified("fatal",f);
	 //exceptions might be thrown by startupHVCheck
	  reply = MakeSOAPMessageReference("PauseFailed");
	  //FIXME maybe we should transition to the Error state in case this happens?
	}



	//calibStateCounter_=0; // dangerous klugde
	return reply;
}

xoap::MessageReference PixelFECSupervisor::Resume (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
std::string const msg_info_bcr = "--- RESUME ---";
 LOG4CPLUS_INFO(sv_logger_,msg_info_bcr);
  xoap::MessageReference reply = MakeSOAPMessageReference("ResumeDone");


  try
    {
      // First do the reporgramming 
      //check HV status and prog dacs if necessary
      startupHVCheck(true);

      // Second restart the workloops
      if (theCalibObject_==0) {
	if(doTBMReadoutLoop_) {
	  
	  phlock_->take(); workloopContinue_=true; phlock_->give();
	  workloop_->activate();
std::string const msg_info_ksp = "Resume. Physics data taking workloop activated.";
 LOG4CPLUS_INFO(sv_logger_,msg_info_ksp);
	}
      }
      
      // 3rd fire the state change
      toolbox::Event::Reference e(new toolbox::Event("Resume", this));
      fsm_.fireEvent(e);

    }
catch (toolbox::fsm::exception::Exception & e)    {
      XCEPT_RETHROW(xoap::exception::Exception, "Invalid Command.", e);
    }
  catch (std::exception & e){ 
std::string const msg_error_dve = "Failed to Resume run with exception: "+string(e.what());
LOG4CPLUS_ERROR(sv_logger_,msg_error_dve);
std::exception * error_ptr = &e;
pixel::PixelFECSupervisorException *new_exception = dynamic_cast<pixel::PixelFECSupervisorException *> (error_ptr);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_dve, *new_exception);
this->notifyQualified("fatal",f);
     //exceptions might be thrown by startupHVCheck
      reply = MakeSOAPMessageReference("ResumeFailed");
      //FIXME maybe we should transition to the Error state in case this happens?
    }


std::string const msg_info_jua = "-- PixelFECSupervisor Resumed --";
 LOG4CPLUS_INFO(sv_logger_,msg_info_jua);
  
  return reply;
}

xoap::MessageReference PixelFECSupervisor::Halt (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
std::string const msg_info_ozm = "--- Enter HALT ---";
 LOG4CPLUS_INFO(sv_logger_,msg_info_ozm);

  if (theCalibObject_==0) {

    if(doTBMReadoutLoop_) {

      if (fsm_.getStateName(fsm_.getCurrentState())=="Configured") {
	workloop_->remove(physicsRunning_);
std::string const msg_info_ulj = "PixelFECSupervisor::Halt from Configured. Removed Physics data taking job from workloop.";
 LOG4CPLUS_INFO(sv_logger_,msg_info_ulj);
	
      } else if (fsm_.getStateName(fsm_.getCurrentState())=="Running") {
	
std::string const msg_debug_fbt = "About to cancel workloop";
 LOG4CPLUS_DEBUG(sv_logger_,msg_debug_fbt);
	phlock_->take(); workloopContinue_=false; phlock_->give();
	workloop_->cancel();
	workloop_->remove(physicsRunning_);
std::string const msg_info_snz = "PixelFECSupervisor::Halt. Physics workloop is cancelled.";
 LOG4CPLUS_INFO(sv_logger_,msg_info_snz);
      } else if (fsm_.getStateName(fsm_.getCurrentState())=="Paused") {
std::string const msg_debug_wng = "About to cancel workloop";
 LOG4CPLUS_DEBUG(sv_logger_,msg_debug_wng);
	phlock_->take(); workloopContinue_=false; phlock_->give();
	//workloop_->cancel(); // bug that caused error 9-4-2012 Nic
	workloop_->remove(physicsRunning_);
std::string const msg_info_yrw = "PixelFECSupervisor::Halt. Physics workloop is cancelled.";
 LOG4CPLUS_INFO(sv_logger_,msg_info_yrw);
      }
    }
  }

  try {
    //disable ROCs
    startupHVCheck(false);
  } catch (std::exception & e) {
std::string const msg_error_vnc = "Failed to Halt run with exception: "+string(e.what());
LOG4CPLUS_ERROR(sv_logger_,msg_error_vnc);
std::exception * error_ptr = &e;
pixel::PixelFECSupervisorException *new_exception = dynamic_cast<pixel::PixelFECSupervisorException *> (error_ptr);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_vnc,*new_exception);
this->notifyQualified("fatal",f);
    //FIXME maybe we should transition to the Error state in case this happens?
    return MakeSOAPMessageReference("HaltFailed");
  }

  HaltAction();
  xoap::MessageReference reply=MakeSOAPMessageReference( HaltFSM() );
std::string const msg_info_mvb = "--- Exit HALT ---";
 LOG4CPLUS_INFO(sv_logger_,msg_info_mvb);
  return reply;
}

void PixelFECSupervisor::HaltAction() {

  try {
    if (qpllWorkloop_->isActive()) qpllWorkloop_->cancel();
    qpllWorkloop_->remove(qpllCheck_);
  }
  catch (xcept::Exception & e) {
std::string const msg_warn_xac = "While cancelling QPLL workloop, caught exception: "+string(e.what());
LOG4CPLUS_WARN(sv_logger_,msg_warn_xac);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_warn_xac, e);
this->notifyQualified("fatal",f);
  }
  qplllock_->take(); //we already cancelled the workloop, but it shouldn't hurt
  qpllStatus_.clear();
  num_qpll_locked_ = 0;
  num_qpll_unlocked_ = 0;  
  qplllock_->give();
  

  pclock_->take();
  detConfigLoaded_=false;
  pclock_->give();

  //preserve the value of the current global key
  theLastGlobalKey_ = theGlobalKey_->key();
  delete theGlobalKey_;		theGlobalKey_=0;
  //no longer want to delete other configuration data here except the calib object
  //it is possible that we can get away with not deleting theCalibObject_, but I'm not sure
  delete theCalibObject_;		theCalibObject_=0;


}

void PixelFECSupervisor::CleanupConfigurationData() {

  //this used to be part of HaltAction()
  pclock_->take(); //let's be careful about global data!

	std::map<PixelModuleName, PixelMaskBase*>::iterator i_Masks;
        std::map<PixelModuleName, PixelTrimBase*>::iterator i_Trims;
        std::map<PixelModuleName, PixelDACSettings*>::iterator i_DACs;
        std::map<PixelModuleName, PixelTBMSettings*>::iterator i_TBMs;

	for (i_Masks=theMasks_.begin();i_Masks!=theMasks_.end();++i_Masks) delete i_Masks->second;
	theMasks_.clear();

	for (i_Trims=theTrims_.begin();i_Trims!=theTrims_.end();++i_Trims) delete i_Trims->second;
	theTrims_.clear();

	for (i_DACs=theDACs_.begin();i_DACs!=theDACs_.end();++i_DACs) delete i_DACs->second;
	theDACs_.clear();

	for (i_TBMs=theTBMs_.begin();i_TBMs!=theTBMs_.end();++i_TBMs) delete i_TBMs->second;
	theTBMs_.clear();

	delete theNameTranslation_;	theNameTranslation_=0;
	delete theDetectorConfiguration_;theDetectorConfiguration_=0;
	delete theFECConfiguration_;	theFECConfiguration_=0;
	//note that theCalibObject_ and theGlobalKey_ are still deleted in HaltAction_

  pclock_->give();

}

string PixelFECSupervisor::HaltFSM() {
  //deleteHardware and make FSM transition
  deleteHardware(); // possibly not needed here
  string reply = "HaltDone";
  try
    {
      toolbox::Event::Reference e(new toolbox::Event("Halt", this));
      fsm_.fireEvent(e);
    }
  catch (toolbox::fsm::exception::Exception & e)
	{
std::string const msg_error_mkz = "Invalid FSM command: "+string(e.what());
LOG4CPLUS_ERROR(sv_logger_,msg_error_mkz);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_mkz, e);
this->notifyQualified("fatal",f);
    
      reply="HaltFailed";
    }
  return reply;
}

xoap::MessageReference PixelFECSupervisor::Recover (xoap::MessageReference msg) {

  //as far as I can tell, there is nothing special that needs to be done other than a regular Halt
  //except for canceling any preconfigure workloop
std::string const msg_info_ycr = "--- Enter RECOVER ---";
 LOG4CPLUS_INFO(sv_logger_,msg_info_ycr);
  if (state_!="Error") {
std::string const msg_error_liv = "Can't do Recover from state = "+string(state_);
 LOG4CPLUS_ERROR(sv_logger_,msg_error_liv);
    return MakeSOAPMessageReference("RecoverFailed"); //sanity
  }


  pclock_->take();
  try {
    if (preconfigureWorkloop_->isActive()) preconfigureWorkloop_->cancel(); 
    preconfigureWorkloop_->remove(preconfigureTask_); 
  } 
  catch ( xcept::Exception & e) {
std::string const msg_error_rta = "Exception caught while killing preconfigure workloop: "+string(e.what());
LOG4CPLUS_DEBUG(sv_logger_,msg_error_rta);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_rta,e);
this->notifyQualified("fatal",f);
    //odds are the remove() will fail
  }
  pclock_->give();

  if(doTBMReadoutLoop_) {

    if (workloop_!=0) {
      try {
	if (workloop_->isActive()) {
	  phlock_->take(); workloopContinue_=false; phlock_->give();
	  workloop_->cancel(); //hopefully exception safe 
	}
	workloop_->remove(physicsRunning_); //will throw if the task was not submitted
      }
      catch (xcept::Exception & e) {
std::string const msg_error_ouz = "Failed to remove FED physics workloop (probably ok): "+string(e.what());
LOG4CPLUS_DEBUG(sv_logger_,msg_error_ouz);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_ouz, e);
this->notifyQualified("fatal",f);
      }
    }
  }


  HaltAction();
  xoap::MessageReference replySOAP=MakeSOAPMessageReference("RecoverDone");
  string reply= HaltFSM();
  if (reply!="HaltDone") replySOAP=MakeSOAPMessageReference("RecoverFailed");
std::string const msg_info_dqt = "--- Exit RECOVER ---";
 LOG4CPLUS_INFO(sv_logger_,msg_info_dqt);
  return replySOAP;
}


xoap::MessageReference PixelFECSupervisor::FSMStateRequest (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  return MakeSOAPMessageReference(state_);
}

void PixelFECSupervisor::stateChanged(toolbox::fsm::FiniteStateMachine &fsm) //throw (toolbox::fsm::exception::Exception)
{
 try {
  state_=fsm.getStateName(fsm.getCurrentState());
  if (PixelSupervisor_!=0) {
    Attribute_Vector parameters(3);
    parameters[0].name_="Supervisor"; parameters[0].value_="PixelFECSupervisor";
    parameters[1].name_="Instance";   parameters[1].value_=itoa(crate_);
    parameters[2].name_="FSMState";   parameters[2].value_=state_;
    Send(PixelSupervisor_, "FSMStateNotification", parameters);
  }    
std::string const msg_trace_wbh = "New state is:" +std::string(state_);
 LOG4CPLUS_TRACE(sv_logger_,msg_trace_wbh);
 }
 catch (xcept::Exception & ex) {
   ostringstream err;
   err<<"Failed to report FSM state "<<state_.toString()<<" to PixelSupervisor. Exception: "<<ex.what();
std::string const msg_error_ozk = err.str();
LOG4CPLUS_ERROR(sv_logger_,msg_error_ozk);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_ozk, ex);
this->notifyQualified("fatal",f);

 }
}

xoap::MessageReference PixelFECSupervisor::preConfigure (xoap::MessageReference msg) {
  /*
This is a "pseudo-FSM" command. As soon as this SOAP command is received,
we will start loading configuration data using the global key.
However, we will not change the FSM state (it will remain in Halted).

Configure will not be allowed to proceed until this preConfigure process is completed.

This SOAP function must return promptly. So it will start a workloop that will actually
perform the preconfiguration actions described above.

In the workloop we will prefetch all of the most important configuration data from the
database. We will not allow hardware to be touched until all data is loaded.
It would be faster to allow hardware corresponding to already loaded data to be programmed while
we simultaneously loaded newer data. But that is a more complicated programming problem.
  */

  std::string const msg_info_sso = "--- Enter preConfigure ---";
  LOG4CPLUS_INFO(sv_logger_,msg_info_sso);
  if (state_!="Halted") return MakeSOAPMessageReference("preConfigureFailed"); //sanity
  
  try {
    if (preconfigureWorkloop_->isActive() ) { //if the workloop is still active, it is because something went wrong
      std::string const msg_info_alu = "Found preConfigure workloop active at beginning of preconfigure. This can be normal after a Recover transition.";
      LOG4CPLUS_INFO(sv_logger_,msg_info_alu);
      preconfigureWorkloop_->cancel(); 
      HaltAction(); //this stops the qpll workloop and clears configuration data
      preconfigureWorkloop_->remove(preconfigureTask_);  //this will probably fail with an exception
    }
  } catch (xcept::Exception & e) {
    std::string const msg_error_jni = "(probably normal) Exception caught while killing preconfigure workloop: "+string(e.what());
    LOG4CPLUS_DEBUG(sv_logger_,msg_error_jni);
    XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_jni, e);
    this->notifyQualified("fatal",f);
  }
  
  preConfigureDone_=false;
  // Extract the Global Key from the SOAP message
  // Update the Global Key member data
  // Advertize the Global Key
  Attribute_Vector parameters(1);
  parameters.at(0).name_="GlobalKey";
  Receive(msg, parameters);  
  theGlobalKey_ = new PixelConfigKey (atoi(parameters.at(0).value_.c_str()));
  if (theGlobalKey_==0) {
    std::string const msg_error_mrk = "Failure to create GlobalKey";
    LOG4CPLUS_ERROR(sv_logger_,msg_error_mrk);
    return MakeSOAPMessageReference("preConfigureFailed");
  }
  std::string const msg_debug_phw = "The global key is " + stringF(theGlobalKey_->key());
  LOG4CPLUS_DEBUG(sv_logger_,msg_debug_phw);
  std::string const msg_info_wew = "PixelFECSupervisor::preConfigure - The Global Key was received as "+parameters.at(0).value_;
  LOG4CPLUS_INFO(sv_logger_,msg_info_wew);
  
  
  xoap::MessageReference replySOAP=MakeSOAPMessageReference("preConfigureDone");
  try {
    preconfigureWorkloop_->submit(preconfigureTask_);
    preconfigureWorkloop_->activate();
  }
  catch (xcept::Exception & e) {
    std::string const msg_error_cjd = "Creation of preconfiguration workloop failed with exception: "+string(e.what());
    LOG4CPLUS_ERROR(sv_logger_,msg_error_cjd);
    XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_cjd, e);
    this->notifyQualified("fatal",f);
    //this is a very bad error in that it means Configuration will fail as well
    //I guess at this point PixelSupervisor should really complain
    replySOAP=MakeSOAPMessageReference("preConfigureFailed");
  }
  
  std::string const msg_info_xhn = "--- Exit preConfigure ---";
  LOG4CPLUS_INFO(sv_logger_,msg_info_xhn);
  return replySOAP;
}

void PixelFECSupervisor::loadCalibObject() {

  // Retrieve the Calibration Object from database
  //note -- theCalibObject_ is not used in transitionHaltedToConfiguring, so no lock is needed
  PixelConfigInterface::get(theCalibObject_, "pixel/calib/", *theGlobalKey_);
  if(dynamic_cast <PixelCalibConfiguration*> (theCalibObject_)!=0) {
    pclock_->take(); //locked needed because theDetectorConfiguration_ and theNameTranslation_ are used in parallel
    (dynamic_cast <PixelCalibConfiguration*> (theCalibObject_))->buildROCAndModuleLists(theNameTranslation_, theDetectorConfiguration_);
    pclock_->give();
  }
  calibStateCounter_=0;

}

bool PixelFECSupervisor::preconfigure_workloop(toolbox::task::WorkLoop * w1) {

  //here we test whether theGlobalKey_->key() is == theLastGlobalKey_
  //if yes, then we can skip all of this code!
  //if no, then just run the code as normal

  //in principle we could be more fancy, and check the version numbers of lots of the underlying kinds of
  //configuration data (dac settings, trims, masks, etc). this way we could save time even in the case
  //where only other data (e.g. the fed data) has been updated.
  //But for now we take the simple approach.

  std::string const m1 = "--- Enter preConfigure-workloop ---";
  LOG4CPLUS_INFO(sv_logger_,m1);
  
  try {     
    pclock_->take();
    unsigned int theNewGlobalKey = theGlobalKey_->key();
    pclock_->give();
    if (theNewGlobalKey != theLastGlobalKey_ ) { //new global key means go ahead and load new config data
      
      CleanupConfigurationData(); //delete old configuration data from memory
      
      //moved from transitionHaltedToConfiguring
      
      //would prefer to use a scoped_lock here
      pclock_->take(); //lock access to global objects
      // Retrieve the Pixel Name Translation from database

      std::string const m2 = "--- In preConfigure-workloop, will load nametranslation ---";
      LOG4CPLUS_INFO(sv_logger_,m2);

      PixelConfigInterface::get(theNameTranslation_, "pixel/nametranslation/", *theGlobalKey_);
      if (theNameTranslation_==0) {
	pclock_->give();
	XCEPT_RAISE(toolbox::fsm::exception::Exception,"Failed to load the nametranslation");
      }
      
      // Retrieve the Pixel FEC Configuration from database
      PixelConfigInterface::get(theFECConfiguration_, "pixel/fecconfig/", *theGlobalKey_);
      if (theFECConfiguration_==0) {
	pclock_->give();
	XCEPT_RAISE(toolbox::fsm::exception::Exception,"Failed to load the FEC configuration");
      }
      
      //note -- it is critical to preserve this order: first load name translation and fec config,
      //then load detconfig last
      // Retrieve the Pixel Detector Configuration from database
      PixelConfigInterface::get(theDetectorConfiguration_, "pixel/detconfig/", *theGlobalKey_);
      if (theDetectorConfiguration_==0) {
	pclock_->give();
	XCEPT_RAISE(toolbox::fsm::exception::Exception,"Failed to load the detector configuration");
      }
      detConfigLoaded_=true; //this is the signal for transitionHaltedToConfiguring to proceed
      pclock_->give();
      
      std::string const m3 = "--- In preConfigure-workloop, detconfig loaded ---";
      LOG4CPLUS_INFO(sv_logger_,m3);

      loadCalibObject();
      
      //moved from stateConfiguring
      PixelMaskBase *tempMask=0;
      PixelTrimBase *tempTrims=0;
      PixelDACSettings *tempDACs=0;
      PixelTBMSettings *tempTBMs=0;
    
    PixelTimer getDACTimer, getMaskTimer, getTrimTimer, getTBMTimer;
    
    // Loop over all modules in the configuration
    pclock_->take();
    std::vector <PixelModuleName> modules=theDetectorConfiguration_->getModuleList();
    pclock_->give();
    std::vector <PixelModuleName>::iterator module_name;

    std::string const m4 = "--- In preConfigure-workloop, load module data ---";
    LOG4CPLUS_INFO(sv_logger_,m4);

    for (module_name=modules.begin(); module_name!=modules.end(); ++module_name) {
      
      pclock_->take();
      const PixelHdwAddress& module_firstHdwAddress = theNameTranslation_->firstHdwAddress( *module_name );
      unsigned int fecnumber=module_firstHdwAddress.fecnumber();
      unsigned int feccrate=theFECConfiguration_->crateFromFECNumber(fecnumber);
      pclock_->give();      


      if (feccrate==crate_) {
	
        totalTimer_.start();
        
        std::string modulePath=(module_name->modulename());

	//this stuff is pretty much orthogonal to what goes on in parallel in transitionHaltedToConfiguring
	//so locks are not needed
        getDACTimer.start();
        PixelConfigInterface::get(tempDACs, "pixel/dac/"+modulePath, *theGlobalKey_);
        getDACTimer.stop();
        if (tempDACs==0) XCEPT_RAISE(xdaq::exception::Exception,"Failed to load dac settings!");
        theDACs_.insert(make_pair(*module_name, tempDACs));

        getMaskTimer.start();
        PixelConfigInterface::get(tempMask, "pixel/mask/"+modulePath, *theGlobalKey_);
        getMaskTimer.stop();
        if (tempMask==0)  XCEPT_RAISE(xdaq::exception::Exception,"Failed to load mask settings!");
        theMasks_.insert(make_pair(*module_name, tempMask));

        getTrimTimer.start();
        PixelConfigInterface::get(tempTrims, "pixel/trim/"+modulePath, *theGlobalKey_);
        getTrimTimer.stop();
        if (tempTrims==0) XCEPT_RAISE(xdaq::exception::Exception,"Failed to load trim settings!");
        theTrims_.insert(make_pair(*module_name, tempTrims));	

        getTBMTimer.start();
        PixelConfigInterface::get(tempTBMs, "pixel/tbm/"+modulePath, *theGlobalKey_);
        getTBMTimer.stop();
        if (tempTBMs==0) XCEPT_RAISE(xdaq::exception::Exception,"Failed to load TBM settings!");
        theTBMs_.insert(make_pair(*module_name, tempTBMs));

        totalTimer_.stop();
        
      } // End of if this is the right crate
      
    } // End of loop over all modules


    std::string const msg_debug_dgj = "FEC get Trims total calls: "+stringF(getTrimTimer.ntimes())+" total time: "+stringF(getTrimTimer.tottime())+" avg time: "+stringF(getTrimTimer.avgtime());
    LOG4CPLUS_INFO(sv_logger_,msg_debug_dgj);
    std::string const msg_debug_uqw = "FEC get Masks total calls: "+stringF(getMaskTimer.ntimes())+" total time: "+stringF(getMaskTimer.tottime())+" avg time: "+stringF(getMaskTimer.avgtime());
    LOG4CPLUS_INFO(sv_logger_,msg_debug_uqw);
    std::string const msg_debug_fec = "FEC get DACs total calls : "+stringF(getDACTimer.ntimes())+" total time: "+stringF(getDACTimer.tottime())+" avg time: "+stringF(getDACTimer.avgtime());
    LOG4CPLUS_INFO(sv_logger_,msg_debug_fec);
    std::string const msg_debug_asl = "FEC get TBMs total calls : "+stringF(getTBMTimer.ntimes())+" total time: "+stringF(getTBMTimer.tottime())+" avg time: "+stringF(getTBMTimer.avgtime());
    LOG4CPLUS_INFO(sv_logger_,msg_debug_asl);
    
    } else {
      std::string const msg_info_rnc = "Global key is the same as last configuration. Skipping loading of FEC data from database.";
      LOG4CPLUS_INFO(sv_logger_,msg_info_rnc);
      pclock_->take();
      detConfigLoaded_=true; //this is the signal for transitionHaltedToConfiguring to proceed
      pclock_->give();
      
      //we still delete theCalibObject during Halt, so we need to reload it here
      loadCalibObject();
    }
  } catch (std::exception & ex) {
    std::string const msg_error_wec = "Caught exception while loading FEC configuration data: "+string(ex.what());
    LOG4CPLUS_ERROR(sv_logger_,msg_error_wec);
    std::exception * error_ptr = &ex;
    pixel::PixelFECSupervisorException *new_exception = dynamic_cast<pixel::PixelFECSupervisorException *> (error_ptr);
    XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_wec, *new_exception);
    this->notifyQualified("fatal",f);
    try {
      toolbox::Event::Reference e(new toolbox::Event("Failure", this));
      fsm_.fireEvent(e);
    } catch (toolbox::fsm::exception::Exception & e) {
      std::string const msg_fatal_xvo = "Failed to transition to Error state";
      LOG4CPLUS_FATAL(sv_logger_,msg_fatal_xvo);
      XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_fatal_xvo, e);
      this->notifyQualified("fatal",f);
    }
  }
  
  pclock_->take();
  preConfigureDone_=true;
  pclock_->give();

  std::string const m5 = "--- In preConfigure-workloop, finished ---";
  LOG4CPLUS_INFO(sv_logger_,m5);

  return false;
}

void PixelFECSupervisor::stateConfiguring(toolbox::fsm::FiniteStateMachine &fsm) //throw (toolbox::fsm::exception::Exception)
{

  std::string const m1 = "--- stateCOnfiguring, entered ---";
  LOG4CPLUS_INFO(sv_logger_,m1);

  while (true) {  //don't proceed until preconfiguration is done
    pclock_->take();
    if (preConfigureDone_) { 
      pclock_->give(); 
      try {
	if (preconfigureWorkloop_->isActive())   preconfigureWorkloop_->cancel(); 
	//preconfigureWorkloop_->remove(preconfigureTask_); //not needed because the workloop always returns false
      }
      catch ( xcept::Exception & e) {
	std::string const msg_error_epd = "Exception caught while killing preconfigure workloop: "+string(e.what());
	LOG4CPLUS_DEBUG(sv_logger_,msg_error_epd);
	XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_epd, e);
	this->notifyQualified("fatal",f);
      }
      break; 
    }
    pclock_->give();
    ::sleep(1); //can't make this too big or we'll be giving back the time that we're trying to save!
  }

  
  if (state_=="Error") { //preconfigure may have raised an error state
    std::string const msg_info_huh = "In error state at the beginning of stateConfiguring. Stopping configuration.";
    LOG4CPLUS_INFO(sv_logger_,msg_info_huh);
    return;
  } else {
    std::string const msg_info_ivb = "Preconfiguration is finished. Starting stateConfiguring.";
    LOG4CPLUS_INFO(sv_logger_,msg_info_ivb);
  }
  
  // Update the state_ member data so that Infospace may publish this information
  stateChanged(fsm); //should this go here or above the preConfigureDone check?

  bool proceed=true;
  

  try { //for hardware and configuration data access

  //Use the presence of PixelDCSFSMInterface to trigger the automatic detector startup  
  if (PixelDCSFSMInterface_!=0) {
    
    std::string const msg_info_hkq = 
      "[PixelFECSupervisor::stateConfiguring] PixelDCSFSMInterface detected. Automatic detector startup.";
    LOG4CPLUS_INFO(sv_logger_,msg_info_hkq);
    
    // Loop over all modules in the configuration
    std::vector <PixelModuleName> modules=theDetectorConfiguration_->getModuleList();
    std::vector <PixelModuleName>::iterator module_name;
    map <string, bool> printedDigitalLV;
    for (module_name=modules.begin(); module_name!=modules.end(); ++module_name) {
      
      const PixelHdwAddress& module_firstHdwAddress = 
	theNameTranslation_->firstHdwAddress( *module_name );
      unsigned int fecnumber=module_firstHdwAddress.fecnumber();
      unsigned int feccrate=theFECConfiguration_->crateFromFECNumber(fecnumber);
      unsigned int fecVMEBaseAddress=theFECConfiguration_->VMEBaseAddressFromFECNumber(fecnumber);
      
      //Only work with this module if it belongs to this PixelFECSupervisor's crate
      if (feccrate==crate_) {

        std::string modulePath=module_name->modulename();
        std::string powerCoordinate = "Pilt_BmI";  //Only one partition for pilot because of the bs connect we have to do  //modulePath.substr(0, 8);

        TriVoltage power=powerMap_.getVoltage(powerCoordinate, std::cout);
	BiVoltage powerHV=powerMap_.getHVoltage(powerCoordinate, std::cout); //get HV status
	//in Configure step, we are now going to *ignore* HV status, unless it is a calibration
	if (theCalibObject_==0) //physics run
	  powerMapLast_.setHVoltage(powerCoordinate, HV_OFF, std::cout); //we are acting as if HV is off
	else //this must be a calibration
	  powerMapLast_.setHVoltage(powerCoordinate, powerHV, std::cout); //store 'last' state of HV

	//	cout<<"[PixelFECSupervisor::stateConfiguring] powerCoordinate = "<<powerCoordinate<<" ; with power,powerHV = "<<power<<","<<powerHV<<endl;

        if (power==LV_OFF) {
	  bool doprint=false;
	  map<string, bool>::iterator iprinted = printedDigitalLV.find(powerCoordinate);
	  if (iprinted == printedDigitalLV.end() || iprinted->second==false) doprint=true;
	  if (doprint)  {
	    std::string const msg_debug_zay = 
	      "[PixelFECSupervisor::stateConfiguring] The Digital Voltage for "+powerCoordinate+" is OFF!";
	    LOG4CPLUS_DEBUG(sv_logger_,msg_debug_zay);
	    printedDigitalLV[powerCoordinate] = true;
	  }
          proceed=false;
        }

        if (power==LV_ON_REDUCED) {

          if (modulesProgDACced_.find(modulePath)==modulesProgDACced_.end()) {

	    std::string const msg_trace_bji = 
	      "[PixelFECSupervisor::stateConfiguring] Programming DACs (LV_ON_REDUCED) for module "+modulePath;
	    LOG4CPLUS_TRACE(sv_logger_,msg_trace_bji);
	    
            // Configure the DACs of this module's ROCs.
            // If a PSX server is visible, check to make sure that the currents drawn changed.
            // If it didn't then yell loudly
            PixelDACSettings *tempDACs=0;
            PixelConfigInterface::get(tempDACs, "pixel/dac/"+modulePath, *theGlobalKey_);
            if (tempDACs==0) XCEPT_RAISE(xdaq::exception::Exception,"Failed to load dac settings!");
            if (false && PixelPSXServer_!=0) { //disable this as long as readCurrent is not implemented
              float current1=0, current2=0;
              current1=readCurrent(*module_name);
	      //                                                                                                           use HV OFF settings

	      //cout<<" CALL DACS ======================================================= 1"<<endl;
              tempDACs->generateConfiguration(FECInterface[fecVMEBaseAddress], theNameTranslation_, theDetectorConfiguration_, false);
              ::sleep(1); // why sleep?
              current2=readCurrent(*module_name);
              if (current1==current2) {
		std::string const msg_error_vvt = 
		  "[PixelFECSupervisor::stateConfiguring] Currents in module "+modulePath+" didn't change on configuring DACs!";
		LOG4CPLUS_ERROR(sv_logger_,msg_error_vvt);
                //XCEPT_RAISE(toolbox::fsm::exception::Exception, "Currents didn't change. Transitioning to Error state.");
              }
            } else {
	      //                                                                                                           use HV OFF settings
	      //cout<<" CALL DACS ======================================================= 2"<<endl;
              tempDACs->generateConfiguration(FECInterface[fecVMEBaseAddress], theNameTranslation_, theDetectorConfiguration_, false);
            }

            modulesProgDACced_.insert(modulePath);
          }

          // Only try to ramp up the voltage of this power coordinate
          // From LV_ON_REDUCED to LV_ON if the command hasn't already
          // Been communicated to PixelDCSFSMInterface_
          if (powerCoordinatesRampingUp_.find(powerCoordinate)==powerCoordinatesRampingUp_.end()) {

            // If all modules in this power coordinate have been Prog DACced,
            // Then send an "INITIALIZED" SOAP message to PixelDCSFSMInterface_
            // That will ramp up the voltage of this power coordinate from
            // LV_ON_REDUCED to LV_ON
            bool rampUp=true;
            std::vector <PixelModuleName>::iterator i_module;
            for (i_module=modules.begin(); i_module!=modules.end(); ++i_module) {
              std::string i_modulename=i_module->modulename();
              if (i_modulename.substr(0, 8) == powerCoordinate) {
                if (modulesProgDACced_.find(i_modulename)==modulesProgDACced_.end()) {
                  rampUp=false;
                }
              }
            }
            if (rampUp) {
	      
              xoap::MessageReference message = xoap::createMessage();
              xoap::SOAPEnvelope envelope = message->getSOAPPart().getEnvelope();
              xoap::SOAPName command = envelope.createName("fsmStateNotification", "xdaq", XDAQ_NS_URI);
              xoap::SOAPBody body = envelope.getBody();
              xoap::SOAPBodyElement commandElement = body.addBodyElement(command);
              xoap::SOAPName state = envelope.createName("state");
              xoap::SOAPElement stateElement = commandElement.addChildElement(state);
              xoap::SOAPName partitionElement = envelope.createName("partition");
              stateElement.addAttribute(partitionElement, powerCoordinate);
              stateElement.addTextNode("INITIALIZED");

              xoap::MessageReference response = SendWithSOAPReply(PixelDCSFSMInterface_, message);
              powerCoordinatesRampingUp_.insert(powerCoordinate);

            }

          }

          proceed=false;

        }

        if (power==LV_ON) {
	  bool doprint=false;
	  map<string, bool>::iterator iprinted = printedDigitalLV.find(powerCoordinate);
	  if (iprinted == printedDigitalLV.end() || iprinted->second==false) doprint=true;
	  if (doprint)  {
	    std::string const msg_debug_vdq = 
	      "[PixelFECSupervisor::stateConfiguring] The Digital Voltage for "+powerCoordinate+" is ON.";
	    LOG4CPLUS_DEBUG(sv_logger_,msg_debug_vdq);
	    printedDigitalLV[powerCoordinate] = true;
	  }
        }
        
      } // End of if this is the right crate
      
    } // End of loop over all modules
          
  } // End of if PixelDCSFSMInterface_ exists

  
  if (proceed) {

    std::string const msg_debug_hov = 
      "[PixelFECSupervisor::stateConfiguring] Finally we program all the TBMs, DACs, Masks and Trims.";
    LOG4CPLUS_INFO(sv_logger_,msg_debug_hov);
    
    // Clear all std::sets
    modulesProgDACced_.clear();
    powerCoordinatesRampingUp_.clear();    
    
    // Loop over all modules,
    // To program their DACs,
    // Their Masks,
    // And their trims.    
    // We will also time the configuration
    
    PixelMaskBase *tempMask=0;
    PixelTrimBase *tempTrims=0;
    PixelDACSettings *tempDACs=0;
    PixelTBMSettings *tempTBMs=0;

    PixelTimer configMaskTrimTimer, configDACTimer, configTBMTimer;
    
    int modulecount=0;
    // Loop over all modules in the configuration
    std::vector <PixelModuleName> modules=theDetectorConfiguration_->getModuleList();
    std::vector <PixelModuleName>::iterator module_name;
    for (module_name=modules.begin(); module_name!=modules.end(); ++module_name) {
      
      const PixelHdwAddress& module_firstHdwAddress = theNameTranslation_->firstHdwAddress( *module_name );
      unsigned int fecnumber=module_firstHdwAddress.fecnumber();
      unsigned int feccrate=theFECConfiguration_->crateFromFECNumber(fecnumber);
      unsigned int fecVMEBaseAddress=theFECConfiguration_->VMEBaseAddressFromFECNumber(fecnumber);
      
      if (feccrate==crate_) {
    
        totalTimer_.start();
        
        std::string modulePath=(module_name->modulename());
	modulecount++;
	if (modulecount%48 == 0){ //prescale this message
	  std::string const msg_trace_jee = 
	    "[PixelFECSupervisor::stateConfiguring] Now configuring module "+modulePath;
	  LOG4CPLUS_TRACE(sv_logger_,msg_trace_jee);
        }
	tempDACs = theDACs_.find(*module_name)->second;
        if (tempDACs==0) XCEPT_RAISE(xdaq::exception::Exception,"Failed to retrieve dac settings!");

	tempMask = theMasks_.find(*module_name)->second;
        if (tempMask==0)  XCEPT_RAISE(xdaq::exception::Exception,"Failed to retrieve mask settings!");

	tempTrims = theTrims_.find(*module_name)->second;
        if (tempTrims==0) XCEPT_RAISE(xdaq::exception::Exception,"Failed to retrieve trim settings!");

	tempTBMs = theTBMs_.find(*module_name)->second;
        if (tempTBMs==0) XCEPT_RAISE(xdaq::exception::Exception,"Failed to retrieve TBM settings!");
#if 0
  	analogInputBiasLast_ .insert(pair<string, int>(module_name->modulename(), tempTBMs->getAnalogInputBias() ) );
	analogOutputBiasLast_.insert(pair<string, int>(module_name->modulename(), tempTBMs->getAnalogOutputBias() ) );
  	analogOutputGainLast_.insert(pair<string, int>(module_name->modulename(), tempTBMs->getAnalogOutputGain() ) );
#endif

        configTBMTimer.start();
        bool physics=(theCalibObject_==0);
	//note -- it appears that this 'physics' argument is not used
        tempTBMs->generateConfiguration(FECInterface[fecVMEBaseAddress], theNameTranslation_,physics);
        configTBMTimer.stop();

	if (PixelDCSFSMInterface_!=0) {
	  std::string powerCoordinate= "Pilt_BmI"; //Only one partition for pilot because of the bs connect we have to do // modulePath.substr(0, 8);
	  BiVoltage powerHV=powerMap_.getHVoltage(powerCoordinate, std::cout); //get HV status
	  configDACTimer.start();
	  if (physics) {
	    powerMapLast_.setHVoltage(powerCoordinate, HV_OFF, std::cout); //we are programming as if HV is off
	    //                                                                                                   use HV OFF settings
	    //cout<<" CALL DACS ======================================================= 3"<<endl;

	    tempDACs->generateConfiguration(FECInterface[fecVMEBaseAddress], theNameTranslation_, theDetectorConfiguration_, false);
	  }
	  else { //calibration
	    powerMapLast_.setHVoltage(powerCoordinate, powerHV, std::cout); //store 'last' state of HV
	    //cout<<" CALL DACS ======================================================= 4"<<endl;

	    tempDACs->generateConfiguration(FECInterface[fecVMEBaseAddress], theNameTranslation_, theDetectorConfiguration_, powerHV==HV_ON);
	  }
	  configDACTimer.stop();
	}
	else {
	  configDACTimer.start();
	  //assume HV is ON
	  //cout<<" CALL DACS ======================================================= 5"<<endl;

	  tempDACs->generateConfiguration(FECInterface[fecVMEBaseAddress], theNameTranslation_, theDetectorConfiguration_, true);
	  configDACTimer.stop();
	}
	
        configMaskTrimTimer.start();
        tempTrims->generateConfiguration(FECInterface[fecVMEBaseAddress], theNameTranslation_, *tempMask);
        configMaskTrimTimer.stop();

	// First waite for the last command to finish. Added 3/9/09 d.k.
	unsigned int stat1=0, stat2=0;
	FECInterface[fecVMEBaseAddress]->mfecbusy(module_firstHdwAddress.mfec(),
						  module_firstHdwAddress.mfecchannel(),
						  &stat1,&stat2);
	// Send a reset after all configs are done
	FECInterface[fecVMEBaseAddress]->injectrstroc(module_firstHdwAddress.mfec(),1); // ResetROC

        totalTimer_.stop();
        
      } // End of if this is the right crate
      
    } // End of loop over all modules
    
    std::string const msg_info_hfn = "Total FEC time (preconfigure + configure) ="+stringF(totalTimer_.tottime());
    LOG4CPLUS_INFO(sv_logger_,msg_info_hfn);
    totalTimer_.reset();
    std::string const msg_debug_qmv = "PixelFECSupervisor::stateConfiguring: FEC config Trim+Mask total calls :"+stringF(configMaskTrimTimer.ntimes())+" total time:"+stringF(configMaskTrimTimer.tottime())+" avg time:"+stringF(configMaskTrimTimer.avgtime());
    LOG4CPLUS_INFO(sv_logger_,msg_debug_qmv);
    std::string const msg_debug_aoa = "PixelFECSupervisor::stateConfiguring: FEC config DAC total calls :"+stringF(configDACTimer.ntimes())+" total time:"+stringF(configDACTimer.tottime())+" avg time:"+stringF(configDACTimer.avgtime());
    LOG4CPLUS_INFO(sv_logger_,msg_debug_aoa);
    std::string const msg_debug_lua = "PixelFECSupervisor::stateConfiguring: FEC config TBM total calls :"+stringF(configTBMTimer.ntimes())+" total time:"+stringF(configTBMTimer.tottime())+" avg time:"+stringF(configTBMTimer.avgtime());
    LOG4CPLUS_INFO(sv_logger_,msg_debug_lua);
    
    if (theCalibObject_ == 0) {
      if(doTBMReadoutLoop_) {
	workloop_->submit(physicsRunning_);
	std::string const msg_info_mez = "Physics data taking job submitted to the workloop";
	LOG4CPLUS_INFO(sv_logger_,msg_info_mez);
      }
    }

    // Fire a transition to Configured state
    try {
      toolbox::Event::Reference e(new toolbox::Event("ConfiguringDone", this));
      fsm_.fireEvent(e);
    } catch (toolbox::fsm::exception::Exception & e) {
      std::string const msg_warn_jbj = 
	"Caught exception: FSM transition error: Invalid Command: "+string(e.what());
      LOG4CPLUS_WARN(sv_logger_,msg_warn_jbj);
      XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_warn_jbj, e);
      this->notifyQualified("fatal",f);
    }
  } //end of if proceed
  } catch (std::exception & ex) {
    std::string const msg_error_zsx = "Caught exception while configuring FEC: "+string(ex.what());
    LOG4CPLUS_ERROR(sv_logger_,msg_error_zsx);
    std::exception * error_ptr = &ex;
    pixel::PixelFECSupervisorException *new_exception = dynamic_cast<pixel::PixelFECSupervisorException *> (error_ptr);
    XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_zsx, *new_exception);
    this->notifyQualified("fatal",f);
    try {
      toolbox::Event::Reference e(new toolbox::Event("Failure", this));
      fsm_.fireEvent(e);
    } catch (toolbox::fsm::exception::Exception & e) {
      std::string const msg_fatal_hyl = "Failed to transition to Error state";
      LOG4CPLUS_FATAL(sv_logger_,msg_fatal_hyl);
      XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_fatal_hyl, e);
      this->notifyQualified("fatal",f);
    }
  }
  
  
  //readBackTBMs(); // call this function for reading the TBMs. it basically just produces couts
  std::string const m2 = "--- stateCOnfiguring, exit ---";
  LOG4CPLUS_INFO(sv_logger_,m2);

}

void PixelFECSupervisor::readBackTBMs() {
  // marc's function for reading back the TBM registries

  bool verbose = true;
  //bool verbose = false;
  if (verbose) cout << " =================================== " << endl;
  // try to figure out how to get the output dir without including everything or creating the string yourself
  //std::string outputDir;
  //setupOutputDir();
  //cout << runNumber_ << outputDir() << endl;
  if (verbose) cout << "[info] start the readout of the TBMs " << endl;
  // get all modules
  std::vector <PixelModuleName> modules=theDetectorConfiguration_->getModuleList();
  std::vector <PixelModuleName>::iterator module_name;

  // get the fed number (this could be used to get the FPix vs. BPix information. FIXME this is an unnecessary functionality, really
  std::map < unsigned int, std::set < unsigned int > >::const_iterator it;
  std::map < unsigned int, std::set < unsigned int > > fedsAndChannels = theDetectorConfiguration_->getFEDsAndChannels(theNameTranslation_);
  std::set < unsigned int >::const_iterator ch_it;
  //for (it = fedsAndChannels.begin() ; it != fedsAndChannels.end() ; it++) {
  //  cout << "[info] printout of the map: FED number "<< it->first << " :";
  //  for (ch_it = it->second.begin() ; ch_it != it->second.end() ; ch_it++) {
  //    cout << " channel " << *ch_it << " , ";
  //  }
  //  cout << endl;
  //}
  // loop over all modules of the FEC at the TIF there is only one FEC with 7 modules, one is broken
  for (module_name=modules.begin(); module_name!=modules.end(); ++module_name) {
    cout << "[info] ================================ new module =========================" << endl;
    const PixelHdwAddress& module_firstHdwAddress = theNameTranslation_->firstHdwAddress( *module_name );
    unsigned int fecnumber=module_firstHdwAddress.fecnumber();
    unsigned int feccrate=theFECConfiguration_->crateFromFECNumber(fecnumber);

    if (feccrate==crate_) {
      cout << "[info] this is the fecnumber: " << fecnumber << endl;
      cout << "[info] this is the feccratenumber: " << feccrate << endl;
      unsigned int fecVMEBaseAddress=theFECConfiguration_->VMEBaseAddressFromFECNumber(fecnumber);

      cout << "[info] at module: " << module_name->modulename() << endl;
      if ( (module_name->modulename()).find("BPix") != string::npos ) {   // not a very nice way to skip bpix modules
        cout << "[info] THIS IS A BPIX MODULE!!" << endl;                // not a very nice way to skip bpix modules
      //  continue;                                                         // not a very nice way to skip bpix modules
      }                                                                   // not a very nice way to skip bpix modules
      //cout << "[info] at channel: " << theNameTranslation_->getChannelFromHdwAddress(module_firstHdwAddress) << endl;
      const unsigned int mfec = module_firstHdwAddress.mfec();
      const unsigned int mfecchannel = module_firstHdwAddress.mfecchannel();
      const unsigned int hubaddress = module_firstHdwAddress.hubaddress();
      unsigned int port = 4; // port 4 communicates with the TBM itself
      unsigned int tbmchannel = 14; // TBM A, TBM B would be 15 (0xE0 and 0xF0)
      cout << "[info] mfec: " << mfec << " mfecchannel: " << mfecchannel << " hubaddress: " << hubaddress << " port: " << port << endl;
      cout << "[info] start loop over the 8 readable registries" << endl;
      //cout << "[test] set the analog input bias to 33 first: " << endl;
      //int bla = FECInterface[fecVMEBaseAddress]->tbmcmd(mfec, mfecchannel, tbmchannel, hubaddress, port, 5, 33, 0);
      for(int i=0;i<8;++i) {
        int outtbm=FECInterface[fecVMEBaseAddress]->tbmread(mfec, mfecchannel,tbmchannel, hubaddress, port, i); // read back
        //cout<<i<<" A "<<hex<<" out = "<<outtbm<<" "<<dec<<outtbm<<" ";
        // read out only the decimal value
        cout<<"offset: " << i<<" | A |"<<hex<<" out = "<<dec<<outtbm<<" ";

        if(i==1) cout<<" stack = "<< (outtbm&0x3F) <<" mode "<<(outtbm>>6);
        else if(i==2) cout<<" event = "<< (outtbm&0xFF);
        else if(i==3) cout<<" status bits = "<< hex<<(outtbm&0xFF)<<dec;
        cout<<endl;
      }
    } // end if feccrate == crate_
    else {
      cout << "[info] at module: " << module_name->modulename() << endl;
      cout << "[info] this module is not part of the right crate!" << endl;
      continue;
    }
    ////Only work with this module if it belongs to this PixelFECSupervisor's crate
        //} // End of if PixelDCSFSMInterface_ exists
  } // End of loop over all modules
} // end readBackTBMs function



xoap::MessageReference PixelFECSupervisor::Reconfigure (xoap::MessageReference msg) 
{

std::string const msg_info_afq = "--- Enter RECONFIGURE ---";
 LOG4CPLUS_INFO(sv_logger_,msg_info_afq);
  xoap::MessageReference reply = MakeSOAPMessageReference("ReconfigureDone");  

  PixelTimer reconfigureTimer,dacLoadTimer,dacProgTimer;
  reconfigureTimer.start();

  try {
    // Extract the Global Key from the SOAP message
    // Update the Global Key member data
    // Advertize the Global Key
    Attribute_Vector parameters(1);
    parameters.at(0).name_="GlobalKey";
    Receive(msg, parameters);  
    PixelConfigKey* newGlobalKey = new PixelConfigKey (atoi(parameters.at(0).value_.c_str()));
    if (newGlobalKey==0) {
std::string const msg_error_opr = "Reconfigure failed to create GlobalKey";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_opr);
      return MakeSOAPMessageReference("ReconfigureFailed");
    }
std::string const msg_debug_wxp = "Will reconfigure with global key = " + stringF(newGlobalKey->key());
 LOG4CPLUS_DEBUG(sv_logger_,msg_debug_wxp);

    //now get the dac settings
    PixelDACSettings *tempDACs=0;    
    //copy loop structure from stateConfiguring
    vector <PixelModuleName> modules=theDetectorConfiguration_->getModuleList();
    vector <PixelModuleName>::iterator module_name;
    for (module_name=modules.begin(); module_name!=modules.end(); ++module_name) {
      
      const PixelHdwAddress& module_firstHdwAddress = theNameTranslation_->firstHdwAddress( *module_name );
      const unsigned int fecnumber=module_firstHdwAddress.fecnumber();
      const unsigned int feccrate=theFECConfiguration_->crateFromFECNumber(fecnumber);
      const unsigned int fecVMEBaseAddress=theFECConfiguration_->VMEBaseAddressFromFECNumber(fecnumber);
      
      if (feccrate==crate_) {
	string modulePath=(module_name->modulename());
	dacLoadTimer.start();
        PixelConfigInterface::get(tempDACs, "pixel/dac/"+modulePath, *newGlobalKey);
        if (tempDACs==0) XCEPT_RAISE(xdaq::exception::Exception,"Reconfigure failed to load dac settings!");
	dacLoadTimer.stop();
	//delete the old dac object and replace it
	delete theDACs_[*module_name];
	theDACs_[*module_name] = tempDACs;

	dacProgTimer.start();
	if (PixelDCSFSMInterface_!=0) {
	  string powerCoordinate="Pilt_BmI";  //Only one partition for pilot because of the bs connect we have to do // modulePath.substr(0, 8);
	  powerMapLast_.setHVoltage(powerCoordinate, HV_OFF, std::cout); //act like HV is off no matter what
	  //cout<<" CALL DACS ======================================================= 7"<<endl;

	  tempDACs->generateConfiguration(FECInterface[fecVMEBaseAddress], theNameTranslation_, theDetectorConfiguration_, false);
	}
	else {	  //assume HV is ON
	  //cout<<" CALL DACS ======================================================= 8"<<endl;

	  tempDACs->generateConfiguration(FECInterface[fecVMEBaseAddress], theNameTranslation_, theDetectorConfiguration_, true);
	}
	dacProgTimer.stop();

	//wait for programming to finish
	unsigned int stat1=0, stat2=0;
	FECInterface[fecVMEBaseAddress]->mfecbusy(module_firstHdwAddress.mfec(),
						  module_firstHdwAddress.mfecchannel(),
						  &stat1,&stat2);
	// Send a reset after all configs are done
	FECInterface[fecVMEBaseAddress]->injectrstroc(module_firstHdwAddress.mfec(),1); // ResetROC
      }
    }
    //if no exceptions have been thrown yet, then update the global key
    delete theGlobalKey_;
    theGlobalKey_=newGlobalKey;
  } catch (exception & e) {
std::string const msg_error_ojp = "Reconfiguration of DACs failed with exception: "+string(e.what());
LOG4CPLUS_ERROR(sv_logger_,msg_error_ojp);
std::exception * error_ptr = &e;
pixel::PixelFECSupervisorException *new_exception = dynamic_cast<pixel::PixelFECSupervisorException *> (error_ptr);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_ojp, *new_exception);
this->notifyQualified("fatal",f);
    reply =  MakeSOAPMessageReference("ReconfigureFailed");
  }

  reconfigureTimer.stop();
  cout<<"DACs were reloaded, reprogrammed for "<<dacLoadTimer.ntimes()<<" , "<<dacProgTimer.ntimes()<<" ROCs"<<endl;
  cout<<"Total database loading time               = "<<dacLoadTimer.tottime()<<endl;
  cout<<"Total ROC programming time                = "<<dacProgTimer.tottime()<<endl;
  cout<<"Total time for Reconfiguration of the FEC = "<<reconfigureTimer.tottime()<<endl;
std::string const msg_info_pqc = "--- Exit RECONFIGURE ---";
 LOG4CPLUS_INFO(sv_logger_,msg_info_pqc);

  return reply;
}

///////////////////////////////////////////////////////////////
//////////////////  FSM State Transition Functions ////////////
///////////////////////////////////////////////////////////////

void PixelFECSupervisor::transitionHaltedToConfiguring (toolbox::Event::Reference e)// throw (toolbox::fsm::exception::Exception)
{
  //exception of type toolbox::fsm::exception::Exception will automatically trigger a transition to the Error state
  
  cout << " transitionHaltedToConfiguring  " << endl;
  
  try { //hardware access
  // Get the VME Bus Adapter
  #ifdef USE_HAL
    // Change for HAL
    //busAdapter_ = new HAL::CAENLinuxBusAdapter(HAL::CAENLinuxBusAdapter::V2718); //optical
    cout << " busAdapter before   " << endl;
    busAdapter_  = new HAL::CAENLinuxBusAdapter(HAL::CAENLinuxBusAdapter::V2718,
						0,0, HAL::CAENLinuxBusAdapter::A3818);
    cout << " busAdapter after   " << endl;
    //busAdapter_ = new HAL::CAENLinuxBusAdapter(HAL::CAENLinuxBusAdapter::V1718); //usb d.k. 3/07
std::string const msg_trace_gfe = "Got a CAEN Linux Bus Adapter for the FEC";
 LOG4CPLUS_TRACE(sv_logger_,msg_trace_gfe);
    // ASCII address table
    HAL::VMEAddressTableASCIIReader reader(datbase_+"PFECAddressMap.dat");
    addressTablePtr_=new HAL::VMEAddressTable( "PFEC address table", reader );
  #else // USE_HAL
std::string const msg_trace_gea = "PixelFECSupervisor::Configure - VMEBoard="+VMEBoard+" Device="+Device+" Link="+Link+" aBHandle="+aBHandle+" cvSuccess="+cvSuccess ;
 LOG4CPLUS_TRACE(sv_logger_,msg_trace_gea);
  #endif // USE_HAL
  }
catch (...) { //FIXME maybe we should catch the actual type of exception that is thrown....    
	XCEPT_RAISE(toolbox::fsm::exception::Exception,"Caught exception when getting FEC HAL BusAdapter");
  }

  //we need the detconfig next. But it is now loaded in the preconfigure step.
  //we need to ensure that it has been loaded
  while (true) {
    pclock_->take();
    if (detConfigLoaded_ == true) {pclock_->give();  break;}
    pclock_->give();
    cout<<"[PixelFECSupervisor::transitionHaltedToConfiguring] Waiting for detconfig to load!"<<endl;
    ::sleep(1);
  }
  //note -- we load the detconfig last, so the other global objects will be loaded once it is loaded

  // Create the FECInterface_ objects
  pclock_->take(); //be careful about global object access
  std::vector <PixelModuleName> modules=theDetectorConfiguration_->getModuleList();
  pclock_->give();
  std::vector <PixelModuleName>::iterator module_name;
  
  try { //hardware access
  for (module_name=modules.begin(); module_name!=modules.end(); ++module_name) {
   
    pclock_->take(); 
    const PixelHdwAddress& module_firstHdwAddress = theNameTranslation_->firstHdwAddress( *module_name );
    unsigned int fecnumber=module_firstHdwAddress.fecnumber();
    unsigned int feccrate=theFECConfiguration_->crateFromFECNumber(fecnumber);
    unsigned int fecVMEBaseAddress=theFECConfiguration_->VMEBaseAddressFromFECNumber(fecnumber);
    unsigned int fecSlot=theFECConfiguration_->FECSlotFromFECNumber(fecnumber);
    pclock_->give();    

    if (feccrate==crate_){

      //there are globals used here, but they aren't used in preconfigure, so it is ok to leave them unlocked

      if(FECInterface.find(fecVMEBaseAddress)==FECInterface.end()) {           
#ifdef USE_HAL
	VMEPtr_[fecVMEBaseAddress] =new HAL::VMEDevice(*addressTablePtr_, *busAdapter_, fecVMEBaseAddress);
        int dummy=0;
	PixelFECInterface* tempFECInterface=new PixelFECInterface(VMEPtr_[fecVMEBaseAddress],dummy,feccrate,fecSlot);
#else
	PixelFECInterface* tempFECInterface=new PixelFECInterface(fecVMEBaseAddress, aBHandle,feccrate,fecSlot);
#endif // USE_HAL

	if (tempFECInterface==0) XCEPT_RAISE(toolbox::fsm::exception::Exception,"Could not create FECInterface");
        tempFECInterface->setssid(4);
	
        //save in the FECInterface map for later use
        FECInterface[fecVMEBaseAddress]=tempFECInterface;
	FECInterfaceByFECNumber_[fecnumber]=tempFECInterface;

	cout<<"FEC Crate="+itoa(crate_)+" VMEAdd=0x"+htoa(fecVMEBaseAddress)<<endl; // dk     

        unsigned long version = 0;
        tempFECInterface->getversion(&version);

	std::string const msg_info_oaw = 
	  "[PixelFECSupervisor::transitionHaltedToConfiguring] FEC Crate="+itoa(crate_)+" VMEAdd=0x"+htoa(fecVMEBaseAddress)+" read mFEC firmware version="+itoa(version);
	LOG4CPLUS_INFO(sv_logger_,msg_info_oaw);
      }        

      // If this is a BPix module, do this
      // FIXME hack!!!!
#if defined SETUP_TIF
      if (true) { //module_name->detsub()=='B') {  //BPIX
#else
      if (module_name->detsub()=='B') {  //BPIX
#endif
	// set the bit to ignore the fullbuffRDa 
        FECInterface[fecVMEBaseAddress]->FullBufRDaDisable(module_firstHdwAddress.mfec(),1);
	// disable the debug check 
        FECInterface[fecVMEBaseAddress]->fecDebug(0);
      } else { // FPIX
	// enable the debug check for fpix 
        FECInterface[fecVMEBaseAddress]->fecDebug(1);
      }

      // Waiting not needed since we do not do any block transfers before
      // Reset the ROCs and TBMs before doing anything
      FECInterface[fecVMEBaseAddress]->injectrsttbm(module_firstHdwAddress.mfec(),1);  // ResetTBM (includes ResetROC)
      //cout<<"vme access 14"<<endl;
      
    }
    
  }

  //start the QPLL check, now that we have created the FECInterfaces
  cout << " transitionHaltedToConfiguring - start qpll loop " << endl;
  qpllWorkloop_->submit(qpllCheck_);
  qpllWorkloop_->activate();
  
  cout << " transitionHaltedToConfiguring - exit " << endl;

  }
catch (toolbox::fsm::exception::Exception & e) { throw; }
catch (std::exception & e) { //translate std::exception to the correct type    XCEPT_RAISE(toolbox::fsm::exception::Exception, string(e.what()));
  }
catch (...) {    
	XCEPT_RAISE(toolbox::fsm::exception::Exception,"Caught unknown exception while accessing FEC via VME");
  }

}

void PixelFECSupervisor::enteringError(toolbox::Event::Reference e)
{

  toolbox::fsm::FailedEvent & fe = dynamic_cast<toolbox::fsm::FailedEvent&>(*e);
  ostringstream errstr;
  errstr<<"Failure performing transition from: "  
	<< fe.getFromState() 
	<<  " to: " 
	<< fe.getToState() 
	<< "; exception: " << fe.getException().what();
std::string const msg_error_ybb = errstr.str();
 LOG4CPLUS_ERROR(sv_logger_,msg_error_ybb);

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

xoap::MessageReference PixelFECSupervisor::TBMCommand (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
        Attribute_Vector parametersReceived(9);
	parametersReceived.at(0).name_="VMEBaseAddress";
        parametersReceived.at(1).name_="mFEC";
        parametersReceived.at(2).name_="mFECChannel";
	parametersReceived.at(3).name_="TBMChannel";
        parametersReceived.at(4).name_="HubAddress";
        parametersReceived.at(5).name_="PortAddress";
        parametersReceived.at(6).name_="Offset";
        parametersReceived.at(7).name_="DataByte";
        parametersReceived.at(8).name_="Direction";
        Receive(msg, parametersReceived);

        FECInterface[atoi(parametersReceived.at(0).value_.c_str())]->tbmcmd(atoi(parametersReceived.at(1).value_.c_str()),
									atoi(parametersReceived.at(2).value_.c_str()),
									atoi(parametersReceived.at(3).value_.c_str()),
									atoi(parametersReceived.at(4).value_.c_str()),
									atoi(parametersReceived.at(5).value_.c_str()),
									atoi(parametersReceived.at(6).value_.c_str()),
									atoi(parametersReceived.at(7).value_.c_str()),
									atoi(parametersReceived.at(8).value_.c_str()));

        xoap::MessageReference reply = MakeSOAPMessageReference("TBMCommandDone");
        return reply;
}

xoap::MessageReference PixelFECSupervisor::TBMSpeed2 (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
        Attribute_Vector parametersReceived(7);
	parametersReceived.at(0).name_="VMEBaseAddress";
        parametersReceived.at(1).name_="mFEC";
	parametersReceived.at(2).name_="mFECChannel";
	parametersReceived.at(3).name_="TBMChannel";
	parametersReceived.at(4).name_="HubAddress";
	parametersReceived.at(5).name_="PortAddress";
	parametersReceived.at(6).name_="Repetitions";
        Receive(msg, parametersReceived);

	int successes = 0;
	int repeat = atoi(parametersReceived.at(6).value_.c_str());

	for(int test=0; test<repeat; test++)
	  {
	    int setSpeedFailed = FECInterface[atoi(parametersReceived.at(0).value_.c_str())]->tbmspeed2(atoi(parametersReceived.at(1).value_.c_str()),atoi(parametersReceived.at(2).value_.c_str()),atoi(parametersReceived.at(3).value_.c_str()),atoi(parametersReceived.at(4).value_.c_str()),atoi(parametersReceived.at(1).value_.c_str()));
	    if(!setSpeedFailed)
	      {
		successes++;
	      }
	  }
	std::string numberSuccesses = itoa(successes);
	std::string replySuccesses = "TBMSpeed2Done"+numberSuccesses;
	xoap::MessageReference reply = MakeSOAPMessageReference(replySuccesses);
	return reply;
}

xoap::MessageReference PixelFECSupervisor::Delay25Test (xoap::MessageReference msg) throw (xoap::exception::Exception)
{

        Attribute_Vector parametersReceived(8);
	parametersReceived.at(0).name_="VMEBaseAddress";
        parametersReceived.at(1).name_="mFEC";
	parametersReceived.at(2).name_="mFECChannel";
	parametersReceived.at(3).name_="TBMChannel";
	parametersReceived.at(4).name_="HubAddress";
	parametersReceived.at(5).name_="PortAddress";
	parametersReceived.at(6).name_="Repetitions";
	parametersReceived.at(7).name_="Commands";
        Receive(msg, parametersReceived);

  // turn off debug mode (we know there will be returned data errors and we do not want to print them!
        FECInterface[atoi(parametersReceived.at(0).value_.c_str())]->fecDebug(0);

	//int successes = 0;
	int repeat = atoi(parametersReceived.at(6).value_.c_str());
	int commands = atoi(parametersReceived.at(7).value_.c_str());

        int nSuccess0=0;
        int nSuccess1=0;
        int nSuccess2=0;
        int nSuccess3=0;
	int nSuccess4=0;


        FECInterface[atoi(parametersReceived.at(0).value_.c_str())]->delay25Test(atoi(parametersReceived.at(1).value_.c_str()),
                                                                                                      atoi(parametersReceived.at(2).value_.c_str()),
                                                                                                      atoi(parametersReceived.at(4).value_.c_str()),
                                                                                                      atoi(parametersReceived.at(3).value_.c_str()),
                                                                                                      atoi(parametersReceived.at(5).value_.c_str()),
                                                                                                      0,
                                                                                                      0,
                                                                                                      0,
                                                                                                      repeat,
										                      commands,
                                                                                                      nSuccess0,
                                                                                                      nSuccess1,
                                                                                                      nSuccess2,
										                      nSuccess3,
                                                                                                      nSuccess4
										                      );

        Attribute_Vector parametersToReturn(5);
        parametersToReturn[0].name_="nSuccess0"; parametersToReturn[0].value_=itoa(nSuccess0);
        parametersToReturn[1].name_="nSuccess1"; parametersToReturn[1].value_=itoa(nSuccess1);
        parametersToReturn[2].name_="nSuccess2"; parametersToReturn[2].value_=itoa(nSuccess2);
        parametersToReturn[3].name_="nSuccess3"; parametersToReturn[3].value_=itoa(nSuccess3);
        parametersToReturn[4].name_="nSuccess4"; parametersToReturn[4].value_=itoa(nSuccess4);

        xoap::MessageReference reply=MakeSOAPMessageReference("Delay25TestDone", parametersToReturn);
        return reply;

}

xoap::MessageReference PixelFECSupervisor::Prog_DAC (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
	Attribute_Vector parametersReceived(8);
        parametersReceived.at(0).name_="mFEC";
        parametersReceived.at(1).name_="mFECChannel";
        parametersReceived.at(2).name_="HubAddress";
        parametersReceived.at(3).name_="PortAddress";
        parametersReceived.at(4).name_="ROCId";
        parametersReceived.at(5).name_="DACAddress";
        parametersReceived.at(6).name_="DACValue";
	parametersReceived.at(7).name_="VMEBaseAddress";

        Receive(msg, parametersReceived);

        FECInterface[atoi(parametersReceived.at(7).value_.c_str())]->progdac(atoi(parametersReceived.at(0).value_.c_str()),
								atoi(parametersReceived.at(1).value_.c_str()),
								atoi(parametersReceived.at(2).value_.c_str()),
								atoi(parametersReceived.at(3).value_.c_str()),
								atoi(parametersReceived.at(4).value_.c_str()),
								atoi(parametersReceived.at(5).value_.c_str()),
								atoi(parametersReceived.at(6).value_.c_str()));

        xoap::MessageReference reply = MakeSOAPMessageReference("Prog_DACDone");

        return reply;
}

xoap::MessageReference PixelFECSupervisor::Prog_DACs (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
//--- begin unpacking SOAP message
//    received from FED Supervisor
        xoap::SOAPEnvelope envelope = msg->getSOAPPart().getEnvelope();
	xoap::SOAPBody body = envelope.getBody();

//--- create SOAPName objects
//    for keywords searched for in SOAP message
        xoap::SOAPName command = envelope.createName("Prog_DACs");

        xoap::SOAPName section_set = envelope.createName("set");
        xoap::SOAPName section_increase = envelope.createName("increase");
	xoap::SOAPName section_decrease = envelope.createName("decrease");

	xoap::SOAPName dacAddressName = envelope.createName("dacAddress");

//--- find within body
//    "Prog_DACs" command
	std::vector< xoap::SOAPElement > bodyElements = body.getChildElements(command);
	for ( std::vector< xoap::SOAPElement >::iterator bodyElement = bodyElements.begin();
	      bodyElement != bodyElements.end(); ++bodyElement ) {
//--- unpack DAC "address"
//    (equivalent to name of DAC;
//     cf. "PSI46 Pixel Chip - External Specification", DocDB #52)
	  int dacAddress = atoi(bodyElement->getAttributeValue(dacAddressName).data());

//--- find within "Prog_DACs" command
//    "set" DAC section
	  std::vector< xoap::SOAPElement > setElements = bodyElement->getChildElements(section_set);
	  for ( std::vector< xoap::SOAPElement >::iterator setElement = setElements.begin();
		setElement != setElements.end(); ++setElement ) {
	    try {
	      decodeProg_DACs(envelope, *setElement, dacAddress, kProg_DACs_set);
} catch ( xoap::exception::Exception& e ) {	      
	XCEPT_RETHROW(xoap::exception::Exception, std::string("Failed to decode Prog_DACs command: ") + e.what(), e);
	    }
	  }

//    "increase" DAC section
	  std::vector< xoap::SOAPElement > increaseElements = bodyElement->getChildElements(section_increase);
	  for ( std::vector< xoap::SOAPElement >::iterator increaseElement = increaseElements.begin();
		increaseElement != increaseElements.end(); ++increaseElement ) {
	    try {
	      decodeProg_DACs(envelope, *increaseElement, dacAddress, kProg_DACs_increase);
} catch ( xoap::exception::Exception& e ) {	      
	XCEPT_RETHROW(xoap::exception::Exception, std::string("Failed to decode Prog_DACs command: ") + e.what(), e);
	    }
	  }

//    "decrease" DAC section
	  std::vector< xoap::SOAPElement > decreaseElements = bodyElement->getChildElements(section_decrease);
	  for ( std::vector< xoap::SOAPElement >::iterator decreaseElement = decreaseElements.begin();
		decreaseElement != decreaseElements.end(); ++decreaseElement ) {
	    try {
	      decodeProg_DACs(envelope, *decreaseElement, dacAddress, kProg_DACs_decrease);
} catch ( xoap::exception::Exception& e ) {	      
	XCEPT_RETHROW(xoap::exception::Exception, std::string("Failed to decode Prog_DACs command: ") + e.what(), e);
	    }
	  }
	}

//--- send SOAP reply
	xoap::MessageReference reply = MakeSOAPMessageReference("Prog_DACsDone");

        return reply;
}

xoap::MessageReference PixelFECSupervisor::SetROCDACsEnMass(xoap::MessageReference msg)
{
  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
    assert(tempCalibObject != 0);

  const vector<PixelROCName> &aROC_string = tempCalibObject->rocList();

  Attribute_Vector parametersReceived(2);
    parametersReceived.at(0).name_ = "DACAddress";
    parametersReceived.at(1).name_ = "DACValue";
  Receive(msg, parametersReceived);

  for (unsigned int iROC = 0; iROC < aROC_string.size(); iROC++){
    const PixelHdwAddress *aROC_hardware = theNameTranslation_->getHdwAddress(aROC_string.at(iROC));
    unsigned int crate = theFECConfiguration_->crateFromFECNumber(aROC_hardware->fecnumber());
    if (crate != crate_){
      continue;
    }
    unsigned int VMEBaseAddress = theFECConfiguration_->VMEBaseAddressFromFECNumber(aROC_hardware->fecnumber());
/*
    cout << "MFEC:  " << aROC_hardware->mfec() 
         << ", MFEC Channel:  " << aROC_hardware->mfecchannel()
         << ", Hub Adress:  " << aROC_hardware->hubaddress()
         << ", Port Address:  " << aROC_hardware->portaddress()
         << ", ROCID:  " << aROC_hardware->rocid()
         << ", DAC Address:  " << atoi(parametersReceived.at(0).value_.c_str())
         << ", DAC Value:  " << atoi(parametersReceived.at(1).value_.c_str()) << endl;
*/
    FECInterface[VMEBaseAddress]->progdac(aROC_hardware->mfec(),
                                          aROC_hardware->mfecchannel(),
                                          aROC_hardware->hubaddress(),
                                          aROC_hardware->portaddress(),
                                          aROC_hardware->rocid(),
                                          atoi(parametersReceived.at(0).value_.c_str()),
                                          atoi(parametersReceived.at(1).value_.c_str()),
                                          true /* to turn the buffer mode on*/);
  }

  std::map <unsigned long, PixelFECInterface*>::iterator iPixelFEC;
  for(iPixelFEC = FECInterface.begin(); iPixelFEC != FECInterface.end(); ++iPixelFEC){
    iPixelFEC->second->qbufsend();
  }

  xoap::MessageReference reply = MakeSOAPMessageReference("SetROCDACsEnMassDone");
  return reply;
}

void PixelFECSupervisor::decodeProg_DACs (xoap::SOAPEnvelope& soapEnvelope, xoap::SOAPElement& soapElement, int dacAddress, int mode) throw (xoap::exception::Exception)
{
//--- create SOAPName objects
//    for keywords searched for in SOAP message
        xoap::SOAPName fecBoard = soapEnvelope.createName("fecBoard");

	xoap::SOAPName vmeBaseAddressName = soapEnvelope.createName("vmeBaseAddress");
	xoap::SOAPName mfecNumberName = soapEnvelope.createName("mfecNumber");
	xoap::SOAPName mfecChannelName = soapEnvelope.createName("mfecChannel");
	xoap::SOAPName hubAddressName = soapEnvelope.createName("hubAddress");
	xoap::SOAPName portAddressName = soapEnvelope.createName("portAddress");
	xoap::SOAPName rocIdName = soapEnvelope.createName("rocId");

//--- find within SOAP element
//    "fecBoard" section
          std::vector< xoap::SOAPElement > fecBoardElements = soapElement.getChildElements(fecBoard);
	  for ( std::vector< xoap::SOAPElement >::iterator fecBoardElement = fecBoardElements.begin();
		fecBoardElement != fecBoardElements.end(); ++fecBoardElement ) {
//--- unpack fecBoardId
//    associated to "fecBoard" identifier
	    int vmeBaseAddress = atoi(fecBoardElement->getAttributeValue(vmeBaseAddressName).data());

//--- loop over all child elements
//    of "fecBoard" section
//    (each child element corresponds to a DAC setting for one read-out chip)
	    std::vector< xoap::SOAPElement > rocElements = fecBoardElement->getChildElements();
	    for ( std::vector< xoap::SOAPElement >::iterator rocElement = rocElements.begin();
		  rocElement != rocElements.end(); ++rocElement ) {
//--- unpack mfec number,
//           mfec channel,
//           hub address,
//           port address and
//           rocId
	      int mfecNumber = atoi(rocElement->getAttributeValue(mfecNumberName).data());
	      int mfecChannel = atoi(rocElement->getAttributeValue(mfecChannelName).data());
	      int hubAddress = atoi(rocElement->getAttributeValue(hubAddressName).data());
	      int portAddress = atoi(rocElement->getAttributeValue(portAddressName).data());
	      int rocId = atoi(rocElement->getAttributeValue(rocIdName).data());

//--- unpack DAC value
	      int dacValue = -1;
	      int dacValueMin = 0; // FIXME
	      int dacValueMax = 0; // FIXME
	      int dacValueCurrent = 0; // FIXME
	      switch ( mode ){
	      case kProg_DACs_set :
		dacValue = atoi(rocElement->getValue().data());
/*
		if ( dacValue < dacValueMin || dacValue > dacValueMax ) {
		  XCEPT_RAISE (xoap::exception::Exception, "DAC value out of bounds");
		}
 */
		break;
	      case kProg_DACs_increase :
		dacValue = dacValueCurrent + 1;
		if ( dacValue > dacValueMax ) {
		  dacValue = dacValueMax;
		  std::cerr << "Warning in <PixelFECSupervisor::decodeProg_DACs>: trying to increase DAC value beyond upper bound !" << std::endl;
		}
		break;
	      case kProg_DACs_decrease :
		dacValue = dacValueCurrent - 1;
		if ( dacValue < dacValueMin ) {
		  dacValue = dacValueMin;
		  std::cerr << "Warning in <PixelFECSupervisor::decodeProg_DACs>: trying to decrease DAC value beyond lower bound !" << std::endl;
		}
		break;
	      default:
		XCEPT_RAISE (xoap::exception::Exception, "Undefined mode");
	      }

//--- set DAC value in read-out chip
	      FECInterface[vmeBaseAddress]->progdac(mfecNumber, mfecChannel, hubAddress, portAddress, rocId, dacAddress, dacValue);
	    }
	  }
}

xoap::MessageReference PixelFECSupervisor::Cal_Pix (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
        Attribute_Vector parametersReceived(9);
        parametersReceived.at(0).name_="mFEC";
        parametersReceived.at(1).name_="mFECChannel";
        parametersReceived.at(2).name_="HubAddress";
        parametersReceived.at(3).name_="PortAddress";
        parametersReceived.at(4).name_="ROCId";
        parametersReceived.at(5).name_="Column";
	parametersReceived.at(6).name_="Row";
        parametersReceived.at(7).name_="CalData";
	parametersReceived.at(8).name_="VMEBaseAddress";
        Receive(msg, parametersReceived);

	FECInterface[atoi(parametersReceived.at(8).value_.c_str())]->calpix(atoi(parametersReceived.at(0).value_.c_str()),
								atoi(parametersReceived.at(1).value_.c_str()),
								atoi(parametersReceived.at(2).value_.c_str()),
								atoi(parametersReceived.at(3).value_.c_str()),
								atoi(parametersReceived.at(4).value_.c_str()),
								atoi(parametersReceived.at(5).value_.c_str()),
								atoi(parametersReceived.at(6).value_.c_str()),
								atoi(parametersReceived.at(7).value_.c_str()));

        xoap::MessageReference reply = MakeSOAPMessageReference("Cal_PixDone");
        return reply;
}


xoap::MessageReference PixelFECSupervisor::Prog_Pix (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
	Attribute_Vector parametersReceived(10);
	parametersReceived.at(0).name_="mFEC";
	parametersReceived.at(1).name_="mFECChannel";
	parametersReceived.at(2).name_="HubAddress";
	parametersReceived.at(3).name_="PortAddress";
	parametersReceived.at(4).name_="ROCId";
	parametersReceived.at(5).name_="Column";
	parametersReceived.at(6).name_="Row";
	parametersReceived.at(7).name_="Mask";
	parametersReceived.at(8).name_="Trim";
	parametersReceived.at(9).name_="VMEBaseAddress";
	Receive(msg, parametersReceived);

	// Change to progpix1(), d.k. 23/11/07
	//FECInterface[atoi(parametersReceived.at(9).value_.c_str())]->progpix(atoi(parametersReceived.at(0).value_.c_str()),
	FECInterface[atoi(parametersReceived.at(9).value_.c_str())]->progpix1(atoi(parametersReceived.at(0).value_.c_str()),
								atoi(parametersReceived.at(1).value_.c_str()),
								atoi(parametersReceived.at(2).value_.c_str()),
								atoi(parametersReceived.at(3).value_.c_str()),
								atoi(parametersReceived.at(4).value_.c_str()),
								atoi(parametersReceived.at(5).value_.c_str()),
								atoi(parametersReceived.at(6).value_.c_str()),
								atoi(parametersReceived.at(7).value_.c_str()),
								atoi(parametersReceived.at(8).value_.c_str()));

	xoap::MessageReference reply = MakeSOAPMessageReference("Prog_PixDone");
	return reply;
}

xoap::MessageReference PixelFECSupervisor::ClrCal (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
	Attribute_Vector parametersReceived(6);
	parametersReceived.at(0).name_="VMEBaseAddress";
	parametersReceived.at(1).name_="mFEC";
	parametersReceived.at(2).name_="mFECChannel";
	parametersReceived.at(3).name_="HubAddress";
	parametersReceived.at(4).name_="PortAddress";
	parametersReceived.at(5).name_="ROCId";
	Receive(msg, parametersReceived);

	FECInterface[atoi(parametersReceived.at(0).value_.c_str())]->clrcal(atoi(parametersReceived.at(1).value_.c_str()),
				atoi(parametersReceived.at(2).value_.c_str()),
				atoi(parametersReceived.at(3).value_.c_str()),
				atoi(parametersReceived.at(4).value_.c_str()),
				atoi(parametersReceived.at(5).value_.c_str()));

	xoap::MessageReference reply=MakeSOAPMessageReference("ClrCalDone");
	return reply;
}

xoap::MessageReference PixelFECSupervisor::ResetROCs (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
        Attribute_Vector parametersReceived(5);
        parametersReceived.at(0).name_="VMEBaseAddress";
        parametersReceived.at(1).name_="mFEC";
        parametersReceived.at(2).name_="mFECChannel";
	parametersReceived.at(3).name_="TBMChannel";
        parametersReceived.at(4).name_="HubAddress";
        Receive(msg, parametersReceived);

        FECInterface[atoi(parametersReceived.at(0).value_.c_str())]->rocreset(atoi(parametersReceived.at(1).value_.c_str()),
                                atoi(parametersReceived.at(2).value_.c_str()),
                                atoi(parametersReceived.at(3).value_.c_str()),
                                atoi(parametersReceived.at(4).value_.c_str()));

        xoap::MessageReference reply=MakeSOAPMessageReference("ResetROCsDone");
        return reply;
}

xoap::MessageReference PixelFECSupervisor::ResetTBM (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
        Attribute_Vector parametersReceived(5);
        parametersReceived.at(0).name_="VMEBaseAddress";
        parametersReceived.at(1).name_="mFEC";
        parametersReceived.at(2).name_="mFECChannel";
	parametersReceived.at(3).name_="TBMChannel";
        parametersReceived.at(4).name_="HubAddress";
        Receive(msg, parametersReceived);

        FECInterface[atoi(parametersReceived.at(0).value_.c_str())]->tbmreset(atoi(parametersReceived.at(1).value_.c_str()),
                                atoi(parametersReceived.at(2).value_.c_str()),
                                atoi(parametersReceived.at(3).value_.c_str()),
                                atoi(parametersReceived.at(4).value_.c_str()));

        xoap::MessageReference reply=MakeSOAPMessageReference("ResetTBMDone");
        return reply;
}

xoap::MessageReference PixelFECSupervisor::ClrCalEnMass (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);
	const vector<PixelROCName> &aROC_string=tempCalibObject->rocList();

	for (unsigned int iROC=0; iROC<aROC_string.size();++iROC)
	{
		const PixelHdwAddress *aROC_hardware=theNameTranslation_->getHdwAddress(aROC_string.at(iROC));
		//std::cout << "PixelHdwAddress for iROC=:"<<iROC<<std::endl<<*aROC_hardware<<std::endl;
		unsigned int crate=theFECConfiguration_->crateFromFECNumber( aROC_hardware->fecnumber() );
		if (crate!=crate_) continue;
		unsigned int VMEBaseAddress=theFECConfiguration_->VMEBaseAddressFromFECNumber(aROC_hardware->fecnumber());
		unsigned int mFEC=aROC_hardware->mfec();
		unsigned int mFECChannel=aROC_hardware->mfecchannel();
		unsigned int HubAddress=aROC_hardware->hubaddress();
		unsigned int PortAddress=aROC_hardware->portaddress();
		unsigned int ROCId=aROC_hardware->rocid();
		FECInterface[VMEBaseAddress]->clrcal(mFEC, mFECChannel, HubAddress, PortAddress, ROCId);
	}

	xoap::MessageReference reply=MakeSOAPMessageReference("ClrCalEnMassDone");
	return reply;
}

xoap::MessageReference PixelFECSupervisor::DisableHitsEnMass (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);
	const set<PixelChannel> channelsToCalibrate = tempCalibObject->channelList();
	for (set<PixelChannel>::const_iterator channelsToCalibrate_itr = channelsToCalibrate.begin(); channelsToCalibrate_itr != channelsToCalibrate.end(); ++channelsToCalibrate_itr)
	{
		vector<PixelROCName> ROCsOnThisChannel = theNameTranslation_->getROCsFromChannel(*channelsToCalibrate_itr);
		for (vector<PixelROCName>::const_iterator ROCsOnThisChannel_itr = ROCsOnThisChannel.begin(); ROCsOnThisChannel_itr != ROCsOnThisChannel.end(); ROCsOnThisChannel_itr++)
		{
			// Get info about this ROC.
			const PixelHdwAddress *thisROCHdwAddress=theNameTranslation_->getHdwAddress(*ROCsOnThisChannel_itr);
			unsigned int crate = theFECConfiguration_->crateFromFECNumber( thisROCHdwAddress->fecnumber() );
			if (crate!=crate_) continue;
			unsigned int VMEBaseAddress = theFECConfiguration_->VMEBaseAddressFromFECNumber(thisROCHdwAddress->fecnumber());
			unsigned int mFEC = thisROCHdwAddress->mfec();
			unsigned int mFECChannel = thisROCHdwAddress->mfecchannel();
			unsigned int HubAddress = thisROCHdwAddress->hubaddress();
			unsigned int PortAddress = thisROCHdwAddress->portaddress();
			unsigned int ROCId = thisROCHdwAddress->rocid();

			// Get the current value of the control register.
			std::map<pos::PixelModuleName, pos::PixelDACSettings*>::const_iterator thisModuleDACSettings = theDACs_.find(theNameTranslation_->getChannelForROC(*ROCsOnThisChannel_itr).module());
			if (thisModuleDACSettings == theDACs_.end()) {
			  std::string module=theNameTranslation_->getChannelForROC(*ROCsOnThisChannel_itr).module().modulename();
std::string const msg_error_pix = "Could not find DAC settings for module:"+module;
 LOG4CPLUS_ERROR(sv_logger_,msg_error_pix);
			  assert(0);
			}
			PixelROCDACSettings* thisROCDACSettings = thisModuleDACSettings->second->getDACSettings(*ROCsOnThisChannel_itr);
			assert( thisROCDACSettings != 0 );
			int ChipContReg_value = thisROCDACSettings->getControlRegister();

			// Program the control register
			ChipContReg_value = ChipContReg_value | 0x2; // enable bit 1
			FECInterface[VMEBaseAddress]->progdac(mFEC, mFECChannel, HubAddress, PortAddress, ROCId, k_DACAddress_ChipContReg, ChipContReg_value);
		}
	}

	xoap::MessageReference reply=MakeSOAPMessageReference("DisableHitsEnMassDone");
	return reply;
}

xoap::MessageReference PixelFECSupervisor::EnableFullBufferCheck (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  
  std::vector <PixelModuleName> modules=theDetectorConfiguration_->getModuleList();

  std::vector <PixelModuleName>::iterator module_name;

  for (module_name=modules.begin(); module_name!=modules.end(); ++module_name) {
    const PixelHdwAddress& module_firstHdwAddress = theNameTranslation_->firstHdwAddress( *module_name );

    unsigned int fecnumber=module_firstHdwAddress.fecnumber();
    unsigned int feccrate=theFECConfiguration_->crateFromFECNumber(fecnumber);
    unsigned int fecVMEBaseAddress=theFECConfiguration_->VMEBaseAddressFromFECNumber(fecnumber);

    if (feccrate==crate_){
      FECInterface[fecVMEBaseAddress]->FullBufRDaDisable(module_firstHdwAddress.mfec(),0);
    }
  }
  
  xoap::MessageReference reply=MakeSOAPMessageReference("EnableFullBufferCheckDone");
	return reply;
}

xoap::MessageReference PixelFECSupervisor::SetTBMDACsEnMass (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
	enum {kAnalogInputBias = 0, kAnalogOutputBias = 1, kAnalogOutputGain = 2};

	Attribute_Vector inputParameters(3);
	inputParameters[kAnalogInputBias].name_ ="AnalogInputBias";
	inputParameters[kAnalogOutputBias].name_="AnalogOutputBias";
	inputParameters[kAnalogOutputGain].name_="AnalogOutputGain";
	Receive(msg, inputParameters);

	PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
	assert(tempCalibObject!=0);
	const set<PixelModuleName> &modulesToCalibrate=tempCalibObject->moduleList();

	for (set<PixelModuleName>::const_iterator modulesToCalibrate_itr = modulesToCalibrate.begin(); modulesToCalibrate_itr != modulesToCalibrate.end(); ++modulesToCalibrate_itr)
	{
		// Use the first hardware address associated with this module.
		const PixelHdwAddress& module_firstHdwAddress = theNameTranslation_->firstHdwAddress( *modulesToCalibrate_itr );
		unsigned int crate=theFECConfiguration_->crateFromFECNumber( module_firstHdwAddress.fecnumber() );
		if (crate!=crate_) continue;
		unsigned int VMEBaseAddress=theFECConfiguration_->VMEBaseAddressFromFECNumber(module_firstHdwAddress.fecnumber());
		unsigned int mFEC=module_firstHdwAddress.mfec();
		unsigned int mFECChannel=module_firstHdwAddress.mfecchannel();
		unsigned int HubAddress=module_firstHdwAddress.hubaddress();

		const int tbmchannel = 14; // = 0xE
		// commands based on PixelConfigDataFormats/src/common/PixelTBMSettings
		//Analog input bias
		if (inputParameters[kAnalogInputBias].value_  != "unchanged")
			FECInterface[VMEBaseAddress]->tbmcmd(mFEC, mFECChannel, tbmchannel, HubAddress, 4, 5, atoi(inputParameters[kAnalogInputBias].value_.c_str()),  0);
		//Analog output bias
		if (inputParameters[kAnalogOutputBias].value_ != "unchanged")
			FECInterface[VMEBaseAddress]->tbmcmd(mFEC, mFECChannel, tbmchannel, HubAddress, 4, 6, atoi(inputParameters[kAnalogOutputBias].value_.c_str()), 0);
		//Analog output gain
		if (inputParameters[kAnalogOutputGain].value_ != "unchanged")
			FECInterface[VMEBaseAddress]->tbmcmd(mFEC, mFECChannel, tbmchannel, HubAddress, 4, 7, atoi(inputParameters[kAnalogOutputGain].value_.c_str()), 0);
	}

	xoap::MessageReference reply=MakeSOAPMessageReference("SetTBMDACsEnMassDone");
	return reply;
}

xoap::MessageReference PixelFECSupervisor::CalibRunning (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  xoap::MessageReference reply=MakeSOAPMessageReference("CalibRunningDone");

  if (fsm_.getStateName(fsm_.getCurrentState())!="Running")
    {
      reply=MakeSOAPMessageReference("CalibRunningNotRunInRunningState");
      return reply;
    }

  if (theCalibObject_==0)
    {
      reply=MakeSOAPMessageReference("CalibObjectNonExistent");
      return reply;
    }

  if (theDetectorConfiguration_==0)
    {
      reply=MakeSOAPMessageReference("DetectorConfigNonExistent");
      return reply;
    }

  if (theNameTranslation_==0)
    {
      reply=MakeSOAPMessageReference("NameTranslationNonExistent");
      return reply;
    }

  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  tempCalibObject->nextFECState(FECInterfaceByFECNumber_, 
				theDetectorConfiguration_, 
				theNameTranslation_, 
				&theMasks_, 
				&theTrims_, 
				&theDACs_, 
				&theTBMs_,
				calibStateCounter_);

  calibStateCounter_++;

  return reply;
}

xoap::MessageReference PixelFECSupervisor::CalibRunningThreshold (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  xoap::MessageReference reply=MakeSOAPMessageReference("CalibRunningThresholdDone");

  Attribute_Vector parametersReceived(1);
  parametersReceived.at(0).name_="Event";
  Receive(msg, parametersReceived);


  if (fsm_.getStateName(fsm_.getCurrentState())!="Running")
    {
      reply=MakeSOAPMessageReference("CalibRunningThresholdNotRunInRunningState");
      return reply;
    }

  if (theCalibObject_==0)
    {
      reply=MakeSOAPMessageReference("CalibObjectNonExistent");
      return reply;
    }

  if (theDetectorConfiguration_==0)
    {
      reply=MakeSOAPMessageReference("DetectorConfigNonExistent");
      return reply;
    }

  if (theNameTranslation_==0)
    {
      reply=MakeSOAPMessageReference("NameTranslationNonExistent");
      return reply;
    }

  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  unsigned int event=atoi(parametersReceived.at(0).value_.c_str());
  assert((unsigned int)event==calibStateCounter_*tempCalibObject->nTriggersPerPattern());

  tempCalibObject->nextFECState(FECInterfaceByFECNumber_, 
				theDetectorConfiguration_, 
				theNameTranslation_, 
				&theMasks_, 
				&theTrims_, 
				&theDACs_, 
				&theTBMs_,
				calibStateCounter_);
   
  calibStateCounter_++;

  return reply;
}


xoap::MessageReference PixelFECSupervisor::fsmStateNotification(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
std::string const msg_trace_qfl = "[PixelFECSupervisor::fsmStateNotification] Entered.";
 LOG4CPLUS_TRACE(sv_logger_,msg_trace_qfl);
  xoap::MessageReference response=MakeSOAPMessageReference("fsmStateNotificationDone");
  
  // Update the tri-state voltage for each partition
  // From the contents of the SOAP message
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
std::string const msg_info_tra = "PixelFECSupervisor::fsmStateNotification - powerCoordinate = "+powerCoordinate+", fsmState = "+fsmState;
 LOG4CPLUS_INFO(sv_logger_,msg_info_tra);
      if (fsmState.find("LV_OFF")!=string::npos) {
	powerMap_.setVoltage(powerCoordinate, LV_OFF, std::cout);
  if ( state_ == "Running" ){
std::string const msg_error_eep = "We are Running, but the LV is OFF for "+powerCoordinate;
 LOG4CPLUS_ERROR(sv_logger_,msg_error_eep);
	}
      }
      else if (fsmState.find("LV_ON_REDUCED")!=string::npos) powerMap_.setVoltage(powerCoordinate, LV_ON_REDUCED,  std::cout);
      else if (fsmState.find("LV_ON")!=string::npos) powerMap_.setVoltage(powerCoordinate, LV_ON, std::cout);
      else {
std::string const msg_error_wuy = "PixelFECSupervisor::fsmStateNotification - "+fsmState+" not recognized! (LV)";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_wuy);
        response=MakeSOAPMessageReference("fsmStateNotificationFailed");
      }
      if (fsmState.find("HV_OFF")!=string::npos) {
	powerMap_.setHVoltage(powerCoordinate, HV_OFF, std::cout);
  if ( state_ == "Running" ){
std::string const msg_warn_scl = "We are Running, but the HV is OFF for "+powerCoordinate;
 LOG4CPLUS_WARN(sv_logger_,msg_warn_scl);
	}
      }
      else if (fsmState.find("HV_ON")!=string::npos) powerMap_.setHVoltage(powerCoordinate, HV_ON,  std::cout);
      else {
std::string const msg_error_zee = "PixelFECSupervisor::fsmStateNotification - "+fsmState+" not recognized! (HV)";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_zee);
        response=MakeSOAPMessageReference("fsmStateNotificationFailed");
      }
    }
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

  return response;

}

float PixelFECSupervisor::readCurrent (pos::PixelModuleName moduleName)
{
  float current=0;
  //   current = current relevant to this module
  return current;
}

xoap::MessageReference PixelFECSupervisor::Null (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
	xoap::MessageReference reply=MakeSOAPMessageReference("NullDone");
	return reply;
}

void PixelFECSupervisor::deleteHardware()
{
  for (FECInterfaceMap::iterator fecs=FECInterface.begin(); fecs!=FECInterface.end(); fecs++) 
    {
      // cout << "PixelFECSupervisor::deleteHardware() deleting FECInterface in crate="<< crate_ 
      //   << " at VMEAddress=0x" << hex << fecs->first
      //   << dec << endl;
      delete fecs->second;  //delete FECInterfaces
    }
  FECInterface.clear();  //clear the map of FECInterface pointers
  FECInterfaceByFECNumber_.clear();

#ifdef USE_HAL

  for (VMEPtrMap::iterator i=VMEPtr_.begin();i!=VMEPtr_.end(); i++)
    {
      delete i->second;  // delete VMEDevices
    }
  VMEPtr_.clear();  // clear the map of vme device pointers (used by FECInterface)
  if(busAdapter_==0) {
std::string const msg_debug_cch = "PixelFECSupervisor::deleteHardware() called when busAdapter_==0";
 LOG4CPLUS_DEBUG(sv_logger_,msg_debug_cch);
  }
  else {
    delete busAdapter_;
  }
  if(addressTablePtr_==0) {
std::string const msg_debug_nvw = "PixelFECSupervisor::deleteHardware() called when addressTablePtr_==0";
 LOG4CPLUS_DEBUG(sv_logger_,msg_debug_nvw);
  }
  else {
    delete addressTablePtr_;
  }
#endif //USE_HAL
}


void PixelFECSupervisor::startupHVCheck(bool startingRun, bool doReset) {

  //if startingRun is true, then we do HV-dependent programming of the ROCs
  //if it is false, then we always disable the ROCs (if they were previously enabled)
  bool printedEnable(false);
  //here we need to loop over the detector etc
  PixelTimer enableTimer;
  PixelTimer disableTimer;
  PixelTimer totalTimer; totalTimer.start();
  if (PixelDCSFSMInterface_!=0) {
    
    // Loop over all modules in the configuration
    std::vector <PixelModuleName> modules=theDetectorConfiguration_->getModuleList();
    std::vector <PixelModuleName>::iterator module_name;
    for (module_name=modules.begin(); module_name!=modules.end(); ++module_name) {
      
      const PixelHdwAddress& module_firstHdwAddress = theNameTranslation_->firstHdwAddress( *module_name );
      unsigned int fecnumber=module_firstHdwAddress.fecnumber();
      unsigned int feccrate=theFECConfiguration_->crateFromFECNumber(fecnumber);
      unsigned int fecVMEBaseAddress=theFECConfiguration_->VMEBaseAddressFromFECNumber(fecnumber);
      
      //Only work with this module if it belongs to this PixelFECSupervisor's crate
      if (feccrate==crate_) {

        std::string modulePath=module_name->modulename();
        std::string powerCoordinate= "Pilt_BmI";//Only one partition for pilot because of the bs connect we have to do //modulePath.substr(0, 8);
	BiVoltage powerHV=powerMap_.getHVoltage(powerCoordinate, std::cout); //get HV status
	//compare this to powerMapLast_
//update -- NO LONGER USE lastHVstate. Just always program
//	BiVoltage lastHVstate=powerMapLast_.getHVoltage(powerCoordinate, std::cout);
//	if (startingRun && powerHV==HV_ON) {
//we need to talk to PixelDACSettings and tell it to:
//-- set Vcthr to the nominal values (get this from theDACS_[*module_name])

	//-- enable and reset the ROCs
	enableTimer.start();
	//also reprogram TBMs
	PixelTBMSettings * tempTBMs = theTBMs_.find(*module_name)->second;
	if (tempTBMs==0) XCEPT_RAISE(xdaq::exception::Exception,"Failed to retrieve TBM settings!");
	tempTBMs->generateConfiguration(FECInterface[fecVMEBaseAddress], theNameTranslation_,theCalibObject_==0,doReset);
	
	bool enableRocs = startingRun && (powerHV==HV_ON);
	//change to generateConfiguration
	theDACs_[*module_name]->generateConfiguration(FECInterface[fecVMEBaseAddress], theNameTranslation_, theDetectorConfiguration_,enableRocs);
	// First wait for the last command to finish. Added 3/9/09 d.k.
	unsigned int stat1=0, stat2=0;
	FECInterface[fecVMEBaseAddress]->mfecbusy(module_firstHdwAddress.mfec(),
						  module_firstHdwAddress.mfecchannel(),
						  &stat1,&stat2);
	FECInterface[fecVMEBaseAddress]->injectrstroc(module_firstHdwAddress.mfec(),1); // ResetROC
	
	enableTimer.stop();
	if (!printedEnable) {
	  string msg;
	  if (startingRun && enableRocs) msg = "While entering Running state, found HV is ON. Enabling ROCs.";
	  else if (startingRun && !enableRocs) msg="While entering Running state, found HV is OFF. Disabling ROCs.";
	  else if (!startingRun && !enableRocs) msg="Disabling ROCs because we are not running.";
	  else {msg = enableRocs ? "Unexpected logic error in startupHVcheck (rocs enabled)" : "Unexpected logic error in startupHVcheck (rocs disabled)";}
	  std::string const msg_info_hel = msg;
	  LOG4CPLUS_INFO(sv_logger_,msg_info_hel);
	  printedEnable=true;
	} //if !printedEnable
	//	}
      } //if feccrate
    } //for (loop over modules)

    //update 'last' state to equal the current state
    if (startingRun) powerMapLast_ = powerMap_;
    else powerMapLast_.setHVoff(); //if startingRun is false, then we have treated it as if HV is off
  } //if
  totalTimer.stop();
  std::cout<<"[PixelFECSupervisor::startupHVCheck]  total DCSFSM time = "<<totalTimer.tottime()<<endl;
  if (enableTimer.ntimes()>0)
    std::cout<<"                             total enable time = "<<enableTimer.tottime()
	     <<" ; ntimes="<<enableTimer.ntimes()<<" ; avg time="<<enableTimer.avgtime()<<std::endl;
  if (disableTimer.ntimes()>0)
    std::cout<<"                            total disable time = "<<disableTimer.tottime()
	     <<" ; ntimes="<<disableTimer.ntimes()<<" ; avg time="<<disableTimer.avgtime()<<std::endl;
  
  
}


void PixelFECSupervisor::b2inEvent(toolbox::mem::Reference* msg, xdata::Properties& plist) throw (b2in::nub::exception::Exception){

  std::string action=plist.getProperty("action");
  xdata::UnsignedIntegerT returnedLid=PixelSupervisor_->getLocalId();
  char buffer[50];
  sprintf(buffer,"%d", returnedLid);
  std::string returnedId(buffer);
  std::string receiveMsg;

  plist.setProperty("urn:b2in-protocol:lid",returnedId);

  std::map<std::string, std::string, std::less<std::string> >& propertiesMap = plist.getProperties();
  Attribute_Vector attrib;
  
  for(std::map<std::string, std::string, std::less<std::string> >::iterator itr=propertiesMap.begin(), itr_end=propertiesMap.end(); itr!=itr_end; ++itr){
    
    Attribute attribute; 
    attribute.name_=itr->first;
    attribute.value_=itr->second;
    attrib.push_back(attribute);
  }
  
  if(action=="CalibRunning"){
    
    xoap::MessageReference soapMsg=this->MakeSOAPMessageReference("CalibRunning", attrib);

    receiveMsg = Receive(this->CalibRunning(soapMsg));
   
  }
  else if(action=="ClrCalEnMass"){

    xoap::MessageReference soapMsg=this->MakeSOAPMessageReference("ClrCalEnMass", attrib);

    receiveMsg = Receive(this->ClrCalEnMass(soapMsg));

  }
  else if(action=="DisableHitsEnMass"){
    
    xoap::MessageReference soapMsg=this->MakeSOAPMessageReference("DisableHitsEnMass", attrib);
 
    receiveMsg = Receive(this->DisableHitsEnMass(soapMsg));
  }
  else if(action=="SetTBMDACsEnMass"){

    xoap::MessageReference soapMsg=this->MakeSOAPMessageReference("SetTBMDACsEnMass", attrib);
 
    receiveMsg = Receive(this->SetTBMDACsEnMass(soapMsg));

  }
  else if(action=="CalibRunningThreshold"){

    xoap::MessageReference soapMsg=this->MakeSOAPMessageReference("CalibRunningThreshold", attrib);
 
    receiveMsg = Receive(this->CalibRunningThreshold(soapMsg));

  }
  else if(action=="SetROCDACsEnMass"){

    xoap::MessageReference soapMsg=this->MakeSOAPMessageReference("SetROCDACsEnMass", attrib);

    receiveMsg = Receive(this->SetROCDACsEnMass(soapMsg));

  }

  plist.setProperty("returnValue", receiveMsg);

  this->sendReply(plist);




}



xoap::MessageReference PixelFECSupervisor::FixSoftError (xoap::MessageReference msg)
{

std::string const msg_info_gfe = "--- FixSoftError ---";
 LOG4CPLUS_INFO(sv_logger_,msg_info_gfe);
  
  // Extract the Global Key from the SOAP message
  // Update the Global Key member data
  // Advertize the Global Key
  // Attribute_Vector parameters(1);
  // parameters[0].name_="GlobalKey";
  // Receive(msg, parameters);
  // if(theGlobalKey_ != 0) delete theGlobalKey_;
  // theGlobalKey_ = new PixelConfigKey(atoi(parameters[0].value_.c_str()));
  if (theGlobalKey_==0) {
std::string const msg_error_boi = "GlobalKey does not exist";
 LOG4CPLUS_ERROR(sv_logger_,msg_error_boi);
    return MakeSOAPMessageReference("FixSoftErrorFailed");
  }
std::string const msg_debug_ymf = "The global key is " + stringF(theGlobalKey_->key());
 LOG4CPLUS_DEBUG(sv_logger_,msg_debug_ymf);
std::string const msg_debug_ugj = "PixelFECSupervisor::FixSoftError: The Global Key is " + stringF(theGlobalKey_->key());
 LOG4CPLUS_DEBUG(sv_logger_,msg_debug_ugj);
  //*console_<<"PixelFECSupervisor::FixSoftError: The Global Key is " + stringF(theGlobalKey_->key())<<std::endl;
  
  xoap::MessageReference reply=MakeSOAPMessageReference("FixSoftErrorDone");

  // That's it! Step to the FixingSoftError state, and
  // relegate all further fixing to the stateFixingSoftError method.
  try {
    toolbox::Event::Reference e(new toolbox::Event("FixSoftError", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
std::string const msg_error_eiy = "[PixelFECSupervisor::FixSoftError] FixSoftError is an invalid command for the current state."+state_.toString();
LOG4CPLUS_ERROR(sv_logger_,msg_error_eiy);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_eiy, e);
this->notifyQualified("fatal",f);
    //*console_<<"[PixelFECSupervisor::FixSoftError] FixSoftError is an invalid command for the "<<state_.toString()<<" state."<<std::endl;
    reply=MakeSOAPMessageReference("FixSoftErrorFailed");
  }
  
std::string const msg_info_gee = "--- FixSoftError DONE ---";
 LOG4CPLUS_INFO(sv_logger_,msg_info_gee);
std::string const msg_info_tte = "PixelFECSupervisor::FixSoftError: A prompt SOAP reply is sent back before exiting function";
 LOG4CPLUS_INFO(sv_logger_,msg_info_tte);
  
  return reply;
  
}

void PixelFECSupervisor::stateFixingSoftError(toolbox::fsm::FiniteStateMachine &fsm) //throw (toolbox::fsm::exception::Exception)
{

  // Update the state_ member data so that Infospace may publish this information
  stateChanged(fsm);

  PixelTimer FixingSoftErrorTimer;
  FixingSoftErrorTimer.start();
  std::string const msg_info_log = "--- FIXINGSOFTERROR ---";
  LOG4CPLUS_INFO(sv_logger_,msg_info_log);
  //*console_<<"--- FIXINGSOFTERROR ---"<<std::endl;
  
  // Stop the physicsRunning Workloop
  if (theCalibObject_==0) {
    
    if(doTBMReadoutLoop_) {   
      phlock_->take(); workloopContinue_=false; phlock_->give();
      workloop_->cancel();
      std::string const msg_info_azx = "PixelFECSupervisor::stateFixingSoftError. Calib object == 0, physics workloop is cancelled.";
      LOG4CPLUS_INFO(sv_logger_,msg_info_azx);
    }
  }
  
  try {
    try {
	//check HV status and prog dacs if necessary
	//startupHVCheck(true);
	startupHVCheck(true, true); // add the reset to the soft-error recovery, d.k. 20.11.12
    } catch (toolbox::fsm::exception::Exception & e)      {
      XCEPT_RETHROW(xoap::exception::Exception, "Invalid Command.", e);
    } catch (std::exception & e){ 
      std::string const msg_error_vjb = "PixelFECSupervisor::stateFixingSoftError: Detected Error: "+string(e.what());
      LOG4CPLUS_ERROR(sv_logger_,msg_error_vjb);
      std::exception * error_ptr = &e;
      pixel::PixelFECSupervisorException *new_exception = dynamic_cast<pixel::PixelFECSupervisorException *> (error_ptr);
      XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_vjb, *new_exception);
      this->notifyQualified("fatal",f);
      //exceptions might be thrown by startupHVCheck
      try {
	toolbox::Event::Reference ev(new toolbox::Event("Failure", this)); //comment this out only for testing
	fsm_.fireEvent(ev);
      } catch (toolbox::fsm::exception::Exception & e2) {
	std::string const msg_fatal_srm = "PixelFECSupervisor::stateFixingSoftError: Failed to transition to Failed state!";
	LOG4CPLUS_FATAL(sv_logger_,msg_fatal_srm);
	XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_fatal_srm, e2);
	this->notifyQualified("fatal",f);
      } // try-catch 
      return;
    } // try-catch

    // deal with the FEC seu detector
    if(doTBMReadoutLoop_) {   // do only if required

#if 0
      // Reset the list of bad channels for TBM readout
      tbmReadbackBadChannels_.clear();
#endif
    
      // Reset all the AnalogLasts to the nominal values
      // This way we'll immediately re-throw DetectSoftError if the problem wasn't actually fixed
      std::vector <PixelModuleName> modules=theDetectorConfiguration_->getModuleList();
      std::vector <PixelModuleName>::iterator module_name;
      
      for (module_name=modules.begin(); module_name!=modules.end(); ++module_name) {
	
	const PixelHdwAddress& module_firstHdwAddress = theNameTranslation_->firstHdwAddress( *module_name );
	unsigned int fecnumber=module_firstHdwAddress.fecnumber();
	unsigned int feccrate=theFECConfiguration_->crateFromFECNumber(fecnumber);
	//unsigned int fecVMEBaseAddress=theFECConfiguration_->VMEBaseAddressFromFECNumber(fecnumber);
	
	//Only work with this module if it belongs to this PixelFECSupervisor's crate
	if (feccrate==crate_) {
	  //cout<<module_name->modulename()<<" "<<theTBMs_.size()<<endl;
	  PixelTBMSettings* tempTBMs = theTBMs_.find(*module_name)->second;
	  if (tempTBMs==0) XCEPT_RAISE(xdaq::exception::Exception,"Failed to retrieve TBM settings!");      

#if 0	  
	  analogInputBiasLast_[module_name->modulename()] = tempTBMs->getAnalogInputBias();
	  analogOutputBiasLast_[module_name->modulename()] = tempTBMs->getAnalogOutputBias();
	  analogOutputGainLast_[module_name->modulename()] = tempTBMs->getAnalogOutputBias();
#endif
	} // if right crate
      } // for loop 
    } // if do 

    toolbox::Event::Reference e(new toolbox::Event("FixingSoftErrorDone", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    std::string const msg_error_pxm = "PixelFECSupervisor::stateFixingSoftError: Detected Error: "+string(e.what());
    LOG4CPLUS_ERROR(sv_logger_,msg_error_pxm);
    XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_pxm, e);
    this->notifyQualified("fatal",f);
    try {
      toolbox::Event::Reference ev(new toolbox::Event("Failure", this)); //comment this out only for testing
      fsm_.fireEvent(ev);
    } catch (toolbox::fsm::exception::Exception & e2) {
      std::string const msg_fatal_xmf = "PixelFECSupervisor::stateFixingSoftError: Failed to transition to Failed state!";
      LOG4CPLUS_FATAL(sv_logger_,msg_fatal_xmf);
      XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_fatal_xmf, e2);
      this->notifyQualified("fatal",f);
    }
    return;
  }
  
  // Re-start the physicsRunning workloop
  if (theCalibObject_==0) {    
    if(doTBMReadoutLoop_) {
      phlock_->take(); workloopContinue_=true; phlock_->give();
      workloop_->activate();
      std::string const msg_info_ndq = "stateFixingSoftError. Physics data taking workloop activated.";
      LOG4CPLUS_INFO(sv_logger_,msg_info_ndq);
    }
  }
  
  FixingSoftErrorTimer.stop();
  std::string const msg_info_tui = "--- Exit PixelFECSupervisor::stateFixingSoftError --- "+stringF(FixingSoftErrorTimer.tottime());
  LOG4CPLUS_INFO(sv_logger_,msg_info_tui);
  
}


xoap::MessageReference PixelFECSupervisor::ResumeFromSoftError (xoap::MessageReference msg)
{
std::string const msg_info_vhq = "--- RESUMEFROMSOFTERROR ---";
 LOG4CPLUS_INFO(sv_logger_,msg_info_vhq);
  //*console_<<"--- Resuming From Soft Error ---"<<std::endl;


  xoap::MessageReference reply = MakeSOAPMessageReference("ResumeFromSoftErrorDone");

  try {

    toolbox::Event::Reference e(new toolbox::Event("ResumeFromSoftError", this));
    fsm_.fireEvent(e);

  } catch (toolbox::fsm::exception::Exception & e) {
std::string const msg_error_ksb = "[PixelFECSupervisor::ResumeFromSoftError] ResumeFromSoftError is an invalid command for the current state."+state_.toString();
LOG4CPLUS_ERROR(sv_logger_,msg_error_ksb);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_ksb,e);
this->notifyQualified("fatal",f);

    //*console_<<"[PixelFECSupervisor::ResumeFromSoftError] ResumeFromSoftError is an invalid command for the "<<state_.toString()<<" state."<<std::endl;

    reply = MakeSOAPMessageReference("ResumeFromSoftErrorFailed");

  }

  return reply;
}

bool PixelFECSupervisor::PhysicsRunning(toolbox::task::WorkLoop *w1) {

  // This workloop monitors the TBM settings registers and sends DetectSoftError to PixelSupervisor
  // if they change from the database values
  
  // For pilot it does nothing.

#if 0
  // get all modules
  std::vector <PixelModuleName> modules=theDetectorConfiguration_->getModuleList();
  std::vector <PixelModuleName>::iterator module_name;

  for (module_name=modules.begin(); module_name!=modules.end(); ++module_name) {

    if (find(tbmReadbackBadChannels_.begin(), tbmReadbackBadChannels_.end(), module_name->modulename()) != tbmReadbackBadChannels_.end())
	{
		continue;
	}
    
    phlock_->take(); if (workloopContinue_)  phlock_->give(); else {phlock_->give(); return true;}

    const PixelHdwAddress& module_firstHdwAddress = theNameTranslation_->firstHdwAddress( *module_name );
    unsigned int fecnumber=module_firstHdwAddress.fecnumber();
    unsigned int feccrate=theFECConfiguration_->crateFromFECNumber(fecnumber);

    if ( (module_name->modulename()).find("BPix") != string::npos ) {   // not a very nice way to skip bpix modules
      //cout << "[info] THIS IS A BPIX MODULE!!!" << endl;                // not a very nice way to skip bpix modules
      continue;                                                         // not a very nice way to skip bpix modules
    }                                                                   // not a very nice way to skip bpix modules

    if (feccrate==crate_) { // Make sure we're on the right computer
      // Get the nominal settings
      PixelTBMSettings* tempTBMs = theTBMs_.find(*module_name)->second;
      if (tempTBMs==0) XCEPT_RAISE(xdaq::exception::Exception,"Failed to retrieve TBM settings!");
      int analog_input_bias = tempTBMs->getAnalogInputBias();
      int analog_output_bias = tempTBMs->getAnalogOutputBias();
      int analog_output_gain = tempTBMs->getAnalogOutputGain();

      unsigned int fecVMEBaseAddress=theFECConfiguration_->VMEBaseAddressFromFECNumber(fecnumber);

      // get all the information to read a register
      const unsigned int mfec = module_firstHdwAddress.mfec();
      const unsigned int mfecchannel = module_firstHdwAddress.mfecchannel();
      const unsigned int hubaddress = module_firstHdwAddress.hubaddress();
      unsigned int port = 4; // port 4 communicates with the TBM itself
      unsigned int tbmchannel = 14; // TBM A, TBM B would be 15 (0xE0 and 0xF0)

      // Read out the three registers
      int read_analog_input_bias = -1;
      int read_analog_output_bias = -1;
      int read_analog_output_gain = -1;
      bool match=true;

	try {	
	      read_analog_input_bias=FECInterface[fecVMEBaseAddress]->tbmread(mfec, mfecchannel,tbmchannel, hubaddress, port, 5); // read back
	      read_analog_output_bias=FECInterface[fecVMEBaseAddress]->tbmread(mfec, mfecchannel,tbmchannel, hubaddress, port, 6); // read back
	      read_analog_output_gain=FECInterface[fecVMEBaseAddress]->tbmread(mfec, mfecchannel,tbmchannel, hubaddress, port, 7); // read back

	      for (int i = 0; i < 3; i++) {
		      match = match & (read_analog_input_bias==FECInterface[fecVMEBaseAddress]->tbmread(mfec, mfecchannel,tbmchannel, hubaddress, port, 5));
		      match = match & (read_analog_output_bias==FECInterface[fecVMEBaseAddress]->tbmread(mfec, mfecchannel,tbmchannel, hubaddress, port, 6));
		      match = match & (read_analog_output_gain==FECInterface[fecVMEBaseAddress]->tbmread(mfec, mfecchannel,tbmchannel, hubaddress, port, 7));
	      }
	}
catch (TBMReadException e) {		// Add this to the list of bad channels and go to the next channel
		tbmReadbackBadChannels_.push_back(module_name->modulename());
		continue;
	}

      // if readings weren't the same every time
      if (!match) {
	//tbmReadbackBadChannels_.push_back(module_name->modulename());
	//std::stringstream error;
	//error << "Number of masked TBMs " << tbmReadbackBadChannels_.size();
	//diagService_->reportError(error.str().c_str(), DIAGWARN);
	continue;
      }

      // Check to see if they match the nominal settings
      if ((read_analog_input_bias != analog_input_bias) || (read_analog_output_bias != analog_output_bias) ||
          (read_analog_output_gain != analog_output_gain)) {
        // If they don't match the nominal settings, check to see if this is a new error, or one we've already seen
        if ((read_analog_input_bias != analogInputBiasLast_[module_name->modulename()]) ||
            (read_analog_output_bias != analogOutputBiasLast_[module_name->modulename()]) ||
            (read_analog_output_gain != analogOutputGainLast_[module_name->modulename()]) )
        {
          // Send a message to PixelSuperVisor
	 std::stringstream warn;
	 warn << "Detected soft error using TBM readback in module " << module_name->modulename().c_str();
std::string const msg_warn_vja = warn.str().c_str();
 LOG4CPLUS_WARN(sv_logger_,msg_warn_vja);

	 cout<<" Detected TBM error "<<module_name->modulename()<<" mfec "<<mfec<<" chan"<<" "<<mfecchannel<<" tbm "
	     <<tbmchannel<<" hub "<<hubaddress<<" port "<<port
	     <<" reg5 "<<read_analog_input_bias<<"/"<<analog_input_bias
	     <<" reg6 "<<read_analog_output_bias<<"/"<<analog_output_bias
	     <<" reg7 "<<read_analog_output_gain<<"/"<<analog_output_gain<<endl;

#ifdef USE_SEU_DETECT
         //DetectSoftError();
#endif
        }
      }
      // Keep track of what the registers were last time
      analogInputBiasLast_[module_name->modulename()] = read_analog_input_bias;
      analogOutputBiasLast_[module_name->modulename()] = read_analog_output_bias;
      analogOutputGainLast_[module_name->modulename()] = read_analog_output_gain;
    } // end if feccrate == crate_
    ////Only work with this module if it belongs to this PixelFECSupervisor's crate
  } // End of loop over all modules
#endif

  phlock_->take(); if (workloopContinue_)  phlock_->give(); else {phlock_->give(); return true;}
  usleep(1000000);

  return true;
}

void PixelFECSupervisor::DetectSoftError() {
    // Do the actual work of sending DetectSoftError to PixelSupervisor
    try {
     if (PixelSupervisor_!=0) {
       Attribute_Vector parameters(2);
       parameters[0].name_="Supervisor"; parameters[0].value_="PixelFECSupervisor";
       parameters[1].name_="Instance";   parameters[1].value_=itoa(crate_);
       Send(PixelSupervisor_, "DetectSoftError", parameters);
     }
    }
    catch (xcept::Exception & ex) {
      ostringstream err;
      err<<"Failed to send DetectSoftError to PixelSupervisor. Exception: "<<ex.what();
std::string const msg_error_jse = err.str();
LOG4CPLUS_ERROR(sv_logger_,msg_error_jse);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_error_jse,ex);
this->notifyQualified("fatal",f);

      try {
	toolbox::Event::Reference ev(new toolbox::Event("Failure", this)); //comment this out only for testing
	fsm_.fireEvent(ev);
      } catch (toolbox::fsm::exception::Exception & e2) {
std::string const msg_fatal_ppg = "PixelFECSupervisor::stateFixingSoftError: Failed to transition to Failed state!";
LOG4CPLUS_FATAL(sv_logger_,msg_fatal_ppg);
XCEPT_DECLARE_NESTED(pixel::PixelFECSupervisorException,f,msg_fatal_ppg, e2);
this->notifyQualified("fatal",f);
      }
    }
    return;
}

