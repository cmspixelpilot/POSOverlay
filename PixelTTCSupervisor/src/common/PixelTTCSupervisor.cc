#include "PixelTTCSupervisor/include/PixelTTCSupervisor.hh"
#include "TTCciAddresses.hh"
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

//gio
#include <diagbag/DiagBagWizard.h>
#include "DiagCompileOptions.h"
#include <toolbox/convertstring.h>

using namespace ttc;
using namespace std;
using namespace pos;

// HTML Macros:
#define BLACK "<span style=\"color: rgb(0, 0, 0);\">"
#define PURPLE "<span style=\"color: rgb(102, 0, 204);\">"
#define DARKBLUE "<span style=\"color: rgb(0, 0, 153);\">"
#define RED "<span style=\"color: rgb(255, 0, 0);\">"
#define GREEN "<span style=\"color: rgb(51, 204, 0);\">"
#define BLUE "<span style=\"color: rgb(51, 51, 255);\">"
#define YELLOW "<span style=\"color: rgb(255, 255, 0);\">"
#define ORANGE "<span style=\"color: rgb(255, 153, 0);\">"
#define GREY "<span style=\"color: rgb(153, 153, 153);\">"
#define BROWN "<span style=\"color: rgb(153, 51, 0);\">"
#define NOCOL "</span>"
#define MONO "<span style=\"font-family: monospace;\">"
#define NOMONO "</span>"
#define BOLD "<b>"
#define NOBOLD "</b>"
#define ITALIC "<i>"
#define NOITALIC "</i>"
#define SMALLER "<font size=\"-1\">"
#define NOSMALLER "</font>"
#define BIGGER "<font size=\"+1\">"
#define NOBIGGER "</font>"
#define UNDERL "<span style=\"text-decoration: underline;\">"
#define NOUNDERL "</span>"
#define BGDCOL_YELLOW "background-color: rgb(255, 255, 153);"
#define BGDCOL_GREEN "background-color: rgb(153, 255, 153);"   // light green
#define BGDCOL_GREY "background-color: rgb(153, 153, 153);"
#define BGDCOL_PINK "background-color: rgb(255, 153, 255);"
#define BGDCOL_ORANGE "background-color: rgb(255, 204, 102);"

XDAQ_INSTANTIATOR_IMPL(PixelTTCSupervisor);

PixelTTCSupervisor::PixelTTCSupervisor(xdaq::ApplicationStub* stub) :
  TTCXDAQBase(stub, DEFAULT_TTCCI_BTC),
  SOAPCommander(this),
  executeReconfMethodMutex(toolbox::BSem::FULL),
  asciConfigurationFilePath_("dummy.txt"),
  DelayT2Correction_(DEFAULT_TTCCI_DT2),
  ClockSource_("Undefined"), 
  ClockFrequency_(0), 
  OrbitSource_("Undefined"), 
  TriggerSource_("Undefined"), 
  BGOSource_("Undefined"), 
  AutoRefresh_(true)
{

  //gio

  diagService_ = new DiagBagWizard(
                                   ("ReconfigurationModule") ,
                                   this->getApplicationLogger(),
                                   getApplicationDescriptor()->getClassName(),
                                   getApplicationDescriptor()->getInstance(),
                                   getApplicationDescriptor()->getLocalId(),
                                   (xdaq::WebApplication *)this,
                                   "MYSYSTEM",
                                   "MYSUBSYTSTEM"
                                   );

  DIAG_DECLARE_USER_APP

    diagService_->reportError("The DiagSystem is installed --- this is a bogus error message",DIAGUSERINFO);

  //

  classname_ = "PixelTTCSupervisor";

  LOG4CPLUS_DEBUG(this->getApplicationLogger(), "constructing PixelTTCSupervisor");

  // export paramater to run control, direct binding
  // run control can read and write all exported variables
  // directly (no need for get and put from the user).

  getApplicationInfoSpace()->fireItemAvailable("DelayT2Correction", &DelayT2Correction_ );

  // THE REAL TTCci STUFF:

  // 
  getApplicationInfoSpace()->fireItemAvailable("EventCounter", &EventCounter_);
  getApplicationInfoSpace()->fireItemAvailable("OrbitCounter", &OrbitCounter_);
  getApplicationInfoSpace()->fireItemAvailable("StrobeCounter", &StrobeCounter_);
  getApplicationInfoSpace()->fireItemAvailable("BoardStatus", &BoardStatus_);

  getApplicationInfoSpace()->fireItemAvailable("ClockSource", &ClockSource_); 
  getApplicationInfoSpace()->fireItemAvailable("ClockFrequency", &ClockFrequency_); 
  getApplicationInfoSpace()->fireItemAvailable("OrbitSource", &OrbitSource_); 
  getApplicationInfoSpace()->fireItemAvailable("TriggerSource", &TriggerSource_); 
  getApplicationInfoSpace()->fireItemAvailable("BGOSource", &BGOSource_); 

  getApplicationInfoSpace()->addItemRetrieveListener ("ClockSource", this);
  getApplicationInfoSpace()->addItemRetrieveListener ("ClockFrequency", this);
  getApplicationInfoSpace()->addItemRetrieveListener ("OrbitSource", this);
  getApplicationInfoSpace()->addItemRetrieveListener ("TriggerSource", this);
  getApplicationInfoSpace()->addItemRetrieveListener ("BGOSource", this);

  getApplicationInfoSpace()->addItemChangedListener ("ClockSource", this);
  getApplicationInfoSpace()->addItemChangedListener ("ClockFrequency", this);
  getApplicationInfoSpace()->addItemChangedListener ("OrbitSource", this);
  getApplicationInfoSpace()->addItemChangedListener ("TriggerSource", this);
  getApplicationInfoSpace()->addItemChangedListener ("BGOSource", this);
	
  // Define FSM
  fsm_.addState ('H', "Halted");
  fsm_.addState ('R', "Ready");
  fsm_.addState ('E', "Enabled");
  fsm_.addState ('S', "Suspended");

  fsm_.addStateTransition ('H','R', "Configure", this, &PixelTTCSupervisor::ConfigureAction);
  fsm_.addStateTransition ('R','E', "Enable", this, &PixelTTCSupervisor::EnableAction);
  fsm_.addStateTransition ('E','R', "Stop", this, &PixelTTCSupervisor::StopAction);
  fsm_.addStateTransition ('E','H', "Halt", this, &PixelTTCSupervisor::HaltAction);
  fsm_.addStateTransition ('R','H', "Halt", this, &PixelTTCSupervisor::HaltAction);
  fsm_.addStateTransition ('S','H', "Halt", this, &PixelTTCSupervisor::HaltAction);
  fsm_.addStateTransition ('H','H', "Halt");
  fsm_.addStateTransition ('E','S', "Suspend", this, &PixelTTCSupervisor::SuspendAction);
  fsm_.addStateTransition ('S','E', "Resume", this, &PixelTTCSupervisor::ResumeAction);
  fsm_.addStateTransition ('S','R', "Stop", this, &PixelTTCSupervisor::StopAction);
  fsm_.setInitialState('H');
  fsm_.reset();
  StateName_ = fsm_.getStateName(fsm_.getCurrentState());

  // Bind SOAP callbacks for control messages
  xoap::bind(this, &PixelTTCSupervisor::changeState, "Configure", XDAQ_NS_URI);
  xoap::bind(this, &PixelTTCSupervisor::changeState, "Enable", XDAQ_NS_URI);
  xoap::bind(this, &PixelTTCSupervisor::changeState, "Stop", XDAQ_NS_URI);
  xoap::bind(this, &PixelTTCSupervisor::changeState, "Suspend", XDAQ_NS_URI);
  xoap::bind(this, &PixelTTCSupervisor::changeState, "Resume", XDAQ_NS_URI);
  xoap::bind(this, &PixelTTCSupervisor::changeState, "Halt", XDAQ_NS_URI);

  xoap::bind(this, &PixelTTCSupervisor::SendPeriodic, "PeriodicL1A", XDAQ_NS_URI);
  xoap::bind(this, &PixelTTCSupervisor::SendPeriodic, "PeriodicCalSync", XDAQ_NS_URI);
  xoap::bind(this, &PixelTTCSupervisor::SendPeriodic, "PeriodicReset", XDAQ_NS_URI);
  xoap::bind(this, &PixelTTCSupervisor::StopPeriodic, "StopPeriodic", XDAQ_NS_URI);

  xoap::bind(this, &PixelTTCSupervisor::getStateRCMS, "getState", "urn:rcms-soap:2.0");

  xoap::bind(this, &PixelTTCSupervisor::UserAction, "User", XDAQ_NS_URI);	
  xoap::bind(this, &PixelTTCSupervisor::userPixelTTCSupervisor, "userPixelTTCSupervisor", XDAQ_NS_URI);  
  xoap::bind(this, &PixelTTCSupervisor::userCommand, "userCommand", XDAQ_NS_URI);
  xoap::bind(this, &PixelTTCSupervisor::userCommand, "ExecuteSequence", XDAQ_NS_URI);

  xoap::bind(this, &PixelTTCSupervisor::LevelOneAction, "LevelOne", XDAQ_NS_URI);	
  xoap::bind(this, &PixelTTCSupervisor::CalSyncAction, "CalSync", XDAQ_NS_URI);	
  xoap::bind(this, &PixelTTCSupervisor::ResetROCAction, "ResetROC", XDAQ_NS_URI);	
  xoap::bind(this, &PixelTTCSupervisor::ResetTBMAction, "ResetTBM", XDAQ_NS_URI);	

  TTCciPtr_ = 0;
  theTTCciConfig_ = 0;
  theGlobalKey_ = 0;

  // XGI Stuff: 
  xgi::bind(this,&PixelTTCSupervisor::Default, "Default");
  xgi::bind(this, &PixelTTCSupervisor::dispatch, "dispatch");
  xgi::bind(this, &PixelTTCSupervisor::NewConfiguration, "NewConfiguration");
  xgi::bind(this, &PixelTTCSupervisor::NewConfigurationFile, "NewConfigurationFile");
  xgi::bind(this, &PixelTTCSupervisor::WriteConfigurationFile, "WriteConfigurationFile");
  xgi::bind(this, &PixelTTCSupervisor::SendCommand, "Command");
  xgi::bind(this, &PixelTTCSupervisor::MainConfigCommand, "MainConfigCommand");
  xgi::bind(this, &PixelTTCSupervisor::TriggerRules, "TriggerRules");
  xgi::bind(this, &PixelTTCSupervisor::BGOConfigCommand, "BGOConfigCommand");
  xgi::bind(this, &PixelTTCSupervisor::BGOSelectCommand, "BGOSelectCommand");
  xgi::bind(this, &PixelTTCSupervisor::CyclicConfigCommand, "CyclicConfigCommand");
  xgi::bind(this, &PixelTTCSupervisor::SequenceSelectCommand, "SequenceSelectCommand");
  xgi::bind(this, &PixelTTCSupervisor::SequenceEditCommand, "SequenceEditCommand");
  xgi::bind(this, &PixelTTCSupervisor::RegisterAccessCommand, 
            "RegisterAccessCommand");
  xgi::bind(this, &PixelTTCSupervisor::VMEBData,"VMEBData");
  //xgi::bind(this, &PixelTTCSupervisor::ConfigurePage, "ConfigurePage");
  xgi::bind(this, &PixelTTCSupervisor::MainConfiguration, "MainConfiguration");
  xgi::bind(this, &PixelTTCSupervisor::BGOConfiguration, "BGOConfiguration");
  xgi::bind(this, &PixelTTCSupervisor::CyclicGenerators, "CyclicGenerators");
  xgi::bind(this, &PixelTTCSupervisor::SummaryPage, "SummaryPage");
  xgi::bind(this, &PixelTTCSupervisor::RegisterAccess, "RegisterAccess");
  xgi::bind(this, &PixelTTCSupervisor::Sequences, "Sequences");
  xgi::bind(this, &PixelTTCSupervisor::ReadWriteParameters, "ReadWriteParameters");
  xgi::bind(this, &PixelTTCSupervisor::DebugTest, "DebugTest");
  
  //DIAGNOSTIC REQUESTED CALLBACK
  xgi::bind(this,&PixelTTCSupervisor::configureDiagSystem, "configureDiagSystem");
  xgi::bind(this,&PixelTTCSupervisor::applyConfigureDiagSystem, "applyConfigureDiagSystem");
  xgi::bind(this,&PixelTTCSupervisor::callDiagSystemPage, "callDiagSystemPage");

  ConfigureIcon();

  std::stringstream timerName;
  timerName << getApplicationDescriptor()->getContextDescriptor()->getURL() << ":";
  timerName << getApplicationDescriptor()->getClassName() << ":" << getApplicationDescriptor()->getLocalId() <<
    ":" << getApplicationDescriptor()->getInstance();
  toolbox::task::Timer * timer = toolbox::task::getTimerFactory()->createTimer(timerName.str());
  toolbox::TimeInterval interval(AUTO_UP_CONFIGURE_DELAY,0);
  toolbox::TimeVal start;
  start = toolbox::TimeVal::gettimeofday() + interval;
  timer->schedule( this, start,  0, "" );

  PixelSupervisor_=getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor("PixelSupervisor", 0);

}

PixelTTCSupervisor::~PixelTTCSupervisor() 
{
  deleteHardware();
}	

//gio
void PixelTTCSupervisor::timeExpired (toolbox::task::TimerEvent& e)
{
  DIAG_EXEC_FSM_INIT_TRANS
    }
//
//DIAG ADDED
void PixelTTCSupervisor::callDiagSystemPage(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  diagService_->getDiagSystemHtmlPage(in, out,getApplicationDescriptor()->getURN());
}



/**
 * Callback function for registered xdata properties.
 */
void PixelTTCSupervisor::actionPerformed( xdata::Event& e )
{
  if (xdata::ItemRetrieveEvent * ev = dynamic_cast<xdata::ItemRetrieveEvent *>(&e)) {

    LOG4CPLUS_INFO(getApplicationLogger(),
                   toolbox::toString("PixelTTCSupervisor::actionPerformed(retrieve %s)",
                                     ev->itemName().c_str()));
    if (TTCciPtr_){
      std::string item = ev->itemName();
      if (item == "EventCounter"){
	ReadTTCciCounters();
      }else if (item == "BoardStatus"){
	BoardStatus_ = TTCciPtr_->BoardStatus(false);
      }else if (item == "ClockSource"){
        ClockSource_=INAME(TTCciPtr_->CheckClock());
      }else if (item == "ClockFrequency"){
        ClockFrequency_=TTCciPtr_->GetQPLLFrequencyBits();
      }else if (item == "OrbitSource"){
        OrbitSource_=INAME(TTCciPtr_->CheckOrbitSource());
      }else if (item == "BGOSource"){
        std::vector<ExternalInterface> vinf=TTCciPtr_->CheckBGOSource();
        BGOSource_="UNDEFINED";
        if (vinf.size()==1) BGOSource_=INAME(vinf[0]);
        else{
          for (size_t i=0; i<vinf.size(); ++i){
            BGOSource_=INAME(vinf[i]);
          }
        }
      }else if (item == "TriggerSource"){
        std::vector<ExternalInterface> vinf=TTCciPtr_->CheckTrigger();
        TriggerSource_="UNDEFINED";
        if (vinf.size()==1) TriggerSource_=INAME(vinf[0]);
        else{
          for (size_t i=0; i<vinf.size(); ++i){
            TriggerSource_=INAME(vinf[i]);
          }
        }
      }else{
        LOG4CPLUS_ERROR(getApplicationLogger(), toolbox::toString("PixelTTCSupervisor::actionPerformed(): Invalid 'retreive' parameter item='%s'",item.c_str()));
      }
    }

  } else if (xdata::ItemChangedEvent * ev = dynamic_cast<xdata::ItemChangedEvent *>(&e)) {  
        
    std::string item = ev->itemName();

    LOG4CPLUS_INFO(getApplicationLogger(), toolbox::toString("PixelTTCSupervisor::actionPerformed(change %s)", item.c_str()));
    if (item == "ClockSource"){
      if (TTCciPtr_){
        ttc::ExternalInterface inf = INTERFACE(ClockSource_);
        if (inf != ttc::UNDEFINED){
          TTCciPtr_->SelectClock(inf);
        }else{
          LOG4CPLUS_ERROR(getApplicationLogger(), toolbox::toString("PixelTTCSupervisor::actionPerformed(): Invalid value for %s='%s'",item.c_str(),ClockSource_.toString().c_str()));
          ClockSource_=INAME(TTCciPtr_->CheckClock());
        }
      }
    }else if (item == "ClockFrequency"){
      if (TTCciPtr_){
        TTCciPtr_->SetQPLLFrequencyBits(ClockFrequency_);
        ClockFrequency_ = TTCciPtr_->GetQPLLFrequencyBits();
      }
    }else if (item == "OrbitSource"){
      if (TTCciPtr_){
        ttc::ExternalInterface inf = INTERFACE(OrbitSource_);
        if (inf != ttc::UNDEFINED){
          TTCciPtr_->SelectOrbitSource(inf);
        }else{
          LOG4CPLUS_ERROR(getApplicationLogger(), toolbox::toString("PixelTTCSupervisor::actionPerformed(): Invalid value for %s='%s'",item.c_str(),OrbitSource_.toString().c_str()));
          OrbitSource_=INAME(TTCciPtr_->CheckOrbitSource());
        }
      }
    }else if (item == "TriggerSource"){
      if (TTCciPtr_){
        ttc::ExternalInterface inf = INTERFACE(TriggerSource_);
        if (inf != ttc::UNDEFINED){
          std::vector<ExternalInterface> vinf; vinf.push_back(inf);
          TTCciPtr_->SelectTrigger(vinf);
        }else{
          LOG4CPLUS_ERROR(getApplicationLogger(), toolbox::toString("PixelTTCSupervisor::actionPerformed(): Invalid value for %s='%s'",item.c_str(),TriggerSource_.toString().c_str()));
          std::vector<ExternalInterface> vinf = TTCciPtr_->CheckTrigger();
          TriggerSource_="UNDEFINED";
          if (vinf.size()==1) TriggerSource_=INAME(vinf[0]);
          else{
            for (size_t i=0; i<vinf.size(); ++i){
              TriggerSource_=INAME(vinf[i]);
            }
          }
        }
      }
    }else if (item == "BGOSource"){
      if (TTCciPtr_){
        ttc::ExternalInterface inf = INTERFACE(BGOSource_);
        if (inf != ttc::UNDEFINED){
          std::vector<ExternalInterface> vinf; vinf.push_back(inf);
          TTCciPtr_->SelectBGOSource(vinf);
        }else{
          LOG4CPLUS_ERROR(getApplicationLogger(), toolbox::toString("PixelTTCSupervisor::actionPerformed(): Invalid value for %s='%s'",item.c_str(),BGOSource_.toString().c_str()));
          std::vector<ExternalInterface> vinf = TTCciPtr_->CheckBGOSource();
          BGOSource_="UNDEFINED";
          if (vinf.size()==1) BGOSource_=INAME(vinf[0]);
          else{
            for (size_t i=0; i<vinf.size(); ++i){
              BGOSource_=INAME(vinf[i]);
            }
          }
        }
      }
    }else{
      LOG4CPLUS_ERROR(getApplicationLogger(), toolbox::toString("PixelTTCSupervisor::actionPerformed(): Invalid 'changed' parameter '%s'",item.c_str()));
    }
    
  }else{
    LOG4CPLUS_WARN(getApplicationLogger(), toolbox::toString("PixelTTCSupervisor::actionPerformed(): Unknown command e.type()='%s'", e.type().c_str()));
  }
  
}

//
// SOAP Callback trigger state change 
//
xoap::MessageReference PixelTTCSupervisor::changeState (xoap::MessageReference msg) 
  throw (xoap::exception::Exception)
{
  string message = Receive(msg);
  if(message == "Configure")
    {
      xoap::SOAPEnvelope envelope = msg->getSOAPPart().getEnvelope();
      std::vector<xoap::SOAPElement> bodyList = envelope.getBody().getChildElements();
      xoap::SOAPElement command = bodyList[0];
      xoap::SOAPName name = envelope.createName("GlobalKey");
      string globalKeyValue = command.getAttributeValue(name);
      if (globalKeyValue=="")
	{
	  cout << "PixelTTCSupervisor:  No global key received!  Using default value 0." << endl;
	  theGlobalKey_ = new PixelConfigKey(0);
	} else {
	  theGlobalKey_ = new PixelConfigKey(atoi(globalKeyValue.c_str()));
	}
    }
  if(message == "Halt")
    {
      delete theGlobalKey_;
      theGlobalKey_ = 0;
      delete theTTCciConfig_;
      theTTCciConfig_ = 0;
    }
  CheckSizeOfHTMLOutput();
  soapReceived_ = msg;
  //cout << "[TIM-INFO]: PixelTTCSupervisor::changeState() called."<<endl;
  xoap::SOAPPart part = msg->getSOAPPart();
  xoap::SOAPEnvelope env = part.getEnvelope();
  xoap::SOAPBody body = env.getBody();
  DOMNode* node = body.getDOMNode();
  DOMNodeList* bodyList = node->getChildNodes();
  for (unsigned int i = 0; i < bodyList->getLength(); i++) 
    {
      DOMNode* command = bodyList->item(i);

      if (command->getNodeType() == DOMNode::ELEMENT_NODE)
        {
          //cout<<"command->getLocalName() = '"<<command->getLocalName()<<"'"<<endl;
          std::string commandName = xoap::XMLCh2String (command->getLocalName());
          //cout << "commandName="<<commandName<<endl;
          try 
            {
              toolbox::Event::Reference e(new toolbox::Event(commandName, this));
              fsm_.fireEvent(e);
              if (!TTCciPtr_) fsm_.reset();
              StateName_ = fsm_.getStateName(fsm_.getCurrentState());
              // Synchronize Web state machine
              // wsm_.setInitialState(fsm_.getCurrentState());
            }
          catch (toolbox::fsm::exception::Exception & e)
            {
              LOG4CPLUS_ERROR(this->getApplicationLogger(),  toolbox::toString("Command not allowed : %s", e.what()));
              XCEPT_RETHROW(xoap::exception::Exception, "Command not allowed", e);		
              //XCEPT_RETHROW(xcept::Exception, "Command not allowed", e);
            }

          xoap::MessageReference reply = xoap::createMessage();
          xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
          xoap::SOAPName responseName = envelope.createName( commandName +"Response", "xdaq", XDAQ_NS_URI);
          envelope.getBody().addBodyElement ( responseName );
          soapResponse_ = reply;
          return reply;
        }
    }

  LOG4CPLUS_ERROR(this->getApplicationLogger(), "No command found");
  XCEPT_RAISE(xoap::exception::Exception, "No command found");		
}

xoap::MessageReference PixelTTCSupervisor::getStateRCMS(xoap::MessageReference /*msg*/)
  throw (xoap::exception::Exception)
{
  xoap::MessageReference responseMsg = xoap::createMessage();
  xoap::SOAPEnvelope envelope = responseMsg->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName responseName =
    envelope.createName("getStateResponse","rcms","urn:rcms-soap:2.0");
  xoap::SOAPBodyElement responseElement =
    body.addBodyElement(responseName);
  responseElement.addNamespaceDeclaration("xsi",
                                          "http://www.w3.org/2001/XMLSchema-instance");
  responseElement.addNamespaceDeclaration("xsd",
                                          "http://www.w3.org/2001/XMLSchema");
  responseElement.addNamespaceDeclaration("soapenc",
                                          "http://schemas.xmlsoap.org/soap/encoding/");
  xoap::SOAPName stateName =
    envelope.createName("state","rcms","urn:rcms-soap:2.0");
  xoap::SOAPElement stateElement =
    responseElement.addChildElement(stateName);
  xoap::SOAPName typeName = envelope.createName("type", "xsi",
                                                "http://www.w3.org/2001/XMLSchema-instance");
  stateElement.addAttribute(typeName, "xsd:string");
  stateElement.addTextNode(fsm_.getStateName(fsm_.getCurrentState()));
  
  return responseMsg;
}

// manual trigger
xoap::MessageReference PixelTTCSupervisor::userPixelTTCSupervisor(xoap::MessageReference msg)
  throw( xoap::exception::Exception) {

  soapReceived_ = msg; 

  try{
    if ( fsm_.getCurrentState() == 'S' ) { 
      LOG4CPLUS_DEBUG(this->getApplicationLogger(), "Sending manual trigger through TTCci.");
      // no need to check SW state, the HW ignores this if not selected
      // (could give a warning, though)
      //TTCciPtr_->sendManual();
      {
        LOG4CPLUS_INFO(this->getApplicationLogger(), 
                       "Function \"sendManual()\" not yet implemented!");
        html<<"WARNING: PixelTTCSupervisor::changeState(): "
            <<"Function &quot;sendManual()&quot; not yet implemented!"<<endl;
      }
    } else {
      LOG4CPLUS_ERROR(this->getApplicationLogger(), "NOT IN STATE ENABLED: CANNOT SEND USERTRIGGER");
    }
    // send back a reply:
    xoap::MessageReference reply = xoap::createMessage();
    xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
    xoap::SOAPName responseName = envelope.createName( "userPixelTTCSupervisorResponse", "xdaq", XDAQ_NS_URI);
    envelope.getBody().addBodyElement ( responseName );
    soapResponse_ = reply;
    return reply;                        
  } catch ( HardwareAccessException& e) {
    LOG4CPLUS_ERROR(this->getApplicationLogger(),  e.what() );
    // delete everything which was created so far 
    deleteHardware(); 
    XCEPT_RAISE(xcept::Exception, "Configure failed, going to state Halted");
    CreateTTCciObject();
  }
}

// user command
xoap::MessageReference PixelTTCSupervisor::userCommand(xoap::MessageReference msg)
  throw( xoap::exception::Exception) {

  soapReceived_ = msg;

  try{
    xoap::SOAPPart soap = msg->getSOAPPart();
    xoap::SOAPEnvelope envelope = soap.getEnvelope();
    xoap::SOAPBody body = envelope.getBody();
    //xoap::SOAPName commandName = body.getElementName();
    xoap::SOAPName parameter("CommandPar","xdaq","urn:xdaq-soap:3.0");
    string parameter_value = body.getAttributeValue(parameter);
    //cout << "PixelTTCSupervisor::userCommand(): value = '"
    //     <<parameter_value<<"'"<<endl;
    if (parameter_value == "ResetCounters"){
      if ( TTCciPtr_ && fsm_.getCurrentState() != 'E' ) { 
        LOG4CPLUS_DEBUG(this->getApplicationLogger(), 
                        toolbox::toString("Resetting TTCci Counters..."));
        TTCciPtr_->ResetCounters();
        // read the counters:
        ReadTTCciCounters();
      } else {
        LOG4CPLUS_ERROR(this->getApplicationLogger(), 
                        toolbox::toString("NOT IN PROPER STATE TO RESET COUNTERS; status=%s",fsm_.getStateName(fsm_.getCurrentState()).c_str()));
      }      
    }else if (parameter_value == "ReadCounters"){
      LOG4CPLUS_DEBUG(this->getApplicationLogger(), 
                      toolbox::toString("Reading TTCci Counters..."));
      //ReadTTCciCounters(); // already done when reloading the html page
    }else if (parameter_value == "DumpVMEHistory-All"){
      if ( TTCciPtr_ ) { 
        TTCciPtr_->PrintVMEHistory();//
      } else {
        LOG4CPLUS_ERROR(this->getApplicationLogger(), toolbox::toString("NOT IN PROPER STATE TO '%s'",parameter_value.c_str()));
      }      
    }else if (parameter_value == "DumpVMEHistory-WriteOnly"){
      if ( TTCciPtr_ ) { 
        TTCciPtr_->PrintVMEHistory("",1);//
      } else {
        LOG4CPLUS_ERROR(this->getApplicationLogger(), toolbox::toString("NOT IN PROPER STATE TO '%s'",parameter_value.c_str()));
      }      
    }else if (parameter_value == "Send VME-BGO"){
      if ( TTCciPtr_ ) { 
        TTCciPtr_->ExecuteVMEBGO(CurrentBGO);//
      } else {
        LOG4CPLUS_ERROR(this->getApplicationLogger(), "NOT IN STATE ENABLED: CANNOT RESET COUNTERS");
      }      
    }else if (parameter_value == "Send BGO-Start" || parameter_value == "Send BGO-Stop"){
      if ( TTCciPtr_ ) { 
        TTCciPtr_->ExecuteVMEBGO((parameter_value == "Send BGO-Start")? 9 : 10 );//
      } else {
        LOG4CPLUS_ERROR(this->getApplicationLogger(), "NOT IN STATE ENABLED: CANNOT RESET COUNTERS");
      }      
    }else if (parameter_value == "Execute Sequence"){
      if ( TTCciPtr_ ) { 
        html << "Executing &quot;"<<SequenceSelector.GetDefaultTitle()
             <<"&quot; Sequence... "<<endl;
        if (!TTCciPtr_->ExecuteSequence(SequenceSelector.GetDefaultTitle())){
          any_errors_ = 1;
          err_message_ <<RED<<"ERROR(S) or WARNING(S) while executing sequence \""
                        <<SequenceSelector.GetDefaultTitle()<<"\"!<br>"<<endl
                        <<"&nbsp;&nbsp;&nbsp; --> Read output (below) for more details."
                        <<NOCOL<<"<br>"<<endl;
        }
      } else {
        LOG4CPLUS_ERROR(this->getApplicationLogger(), "NOT IN STATE ENABLED: CANNOT RESET COUNTERS");
      }      
    }else if (parameter_value == "ReadBanksFromTTCci"){
      if ( 1 ) { 
        ReadTTCciContent();//
      } else {
        LOG4CPLUS_ERROR(this->getApplicationLogger(), toolbox::toString("NOT IN PROPER STATE TO '%s'",parameter_value.c_str()));
      }      
    }else if (parameter_value == "Reset request cntr"){
      if ( TTCciPtr_ ) { 
        //TTCciPtr_->Write(TTCciAdd::CHRESET,CurrentBGO,4);//
        stringstream my;
        my << "WARNING: PixelTTCSupervisor::userCommand(..): \""
           <<parameter_value<<"\" not implemented!";
        cout << my.str()<<endl;
        html << htmlencode(my.str())<<endl;
      } else {
        LOG4CPLUS_ERROR(this->getApplicationLogger(),toolbox::toString("TTCciPtr_ NOT INITIALIZED! CANNOT PERFORM '%s'",parameter_value.c_str()));
      }      
    }else if (parameter_value == "Reset prescale cntr"){
      if ( TTCciPtr_ ) { 
        //TTCciPtr_->Write(TTCciAdd::CHRESET,CurrentBGO,2);//
        stringstream my;
        my << "WARNING: PixelTTCSupervisor::userCommand(..): \""
           <<parameter_value<<"\" not implemented!";
        cout << my.str()<<endl;
        html << htmlencode(my.str())<<endl;
      } else {
        LOG4CPLUS_ERROR(this->getApplicationLogger(),toolbox::toString("TTCciPtr_ NOT INITIALIZED! CANNOT PERFORM '%s'",parameter_value.c_str()));
      }      
    }else if (parameter_value == "Reset cancel cntr"){
      if ( TTCciPtr_ ) { 
        //TTCciPtr_->Write(TTCciAdd::CHRESET,CurrentBGO,1);//
        stringstream my;
        my << "WARNING: PixelTTCSupervisor::userCommand(..): \""
           <<parameter_value<<"\" not implemented!";
        cout << my.str()<<endl;
        html << htmlencode(my.str())<<endl;
      } else {
        LOG4CPLUS_ERROR(this->getApplicationLogger(),toolbox::toString("TTCciPtr_ NOT INITIALIZED! CANNOT PERFORM '%s'",parameter_value.c_str()));
      }      
    }else if (parameter_value == "Reset all 3 counters"){
      if ( TTCciPtr_ ) { 
        //TTCciPtr_->Write(TTCciAdd::CHRESET,CurrentBGO,7);//
        stringstream my;
        my << "WARNING: PixelTTCSupervisor::userCommand(..): \""
           <<parameter_value<<"\" not implemented!";
        cout << my.str()<<endl;
        html << htmlencode(my.str())<<endl;
      } else {
        LOG4CPLUS_ERROR(this->getApplicationLogger(),toolbox::toString("TTCciPtr_ NOT INITIALIZED! CANNOT PERFORM '%s'",parameter_value.c_str()));
      }      
    }else if (parameter_value == "Reset all on all channels"){
      if ( TTCciPtr_ ) { 
        //for (size_t i=0; i<TTCciPtr_->NChannels(); ++i){
        //  TTCciPtr_->Write(TTCciAdd::CHRESET,i,7);//
        //}
        stringstream my;
        my << "WARNING: PixelTTCSupervisor::userCommand(..): \""
           <<parameter_value<<"\" not implemented!";
        cout << my.str()<<endl;
        html << htmlencode(my.str())<<endl;
      } else {
        LOG4CPLUS_ERROR(this->getApplicationLogger(),toolbox::toString("TTCciPtr_ NOT INITIALIZED! CANNOT PERFORM '%s'",parameter_value.c_str()));
      }      
    }else if (parameter_value == "Read BGO configuration from TTCci"){
      if ( TTCciPtr_ ) { 
        TTCciPtr_->ReadBGODataFromTTCci(100);
      } else {
        LOG4CPLUS_ERROR(this->getApplicationLogger(),toolbox::toString("TTCciPtr_ NOT INITIALIZED! CANNOT PERFORM '%s'",parameter_value.c_str()));
      }      
    }else{
      bool seqfound=false;
      if ( TTCciPtr_ ) { 
        vector<string> seqnames = TTCciPtr_->GetSequenceNames();
        for (size_t m=0; m<seqnames.size(); ++m){
          if (parameter_value == seqnames[m]){
            seqfound=true;
            TTCciPtr_->ExecuteSequence(parameter_value);
            break;
          }
        }
      } else {
        LOG4CPLUS_ERROR(this->getApplicationLogger(),toolbox::toString("TTCciPtr_ NOT INITIALIZED! CANNOT PERFORM 'ExecuteSequence %s'",parameter_value.c_str()));
      }      
      if (!seqfound){
        LOG4CPLUS_ERROR(this->getApplicationLogger(),toolbox::toString("PixelTTCSupervisor::userCommand(): Unknown Parameter QualifiedName=%s value=%s",parameter.getQualifiedName().c_str(),parameter_value.c_str()));
      }
    }
    // send back a reply:
    xoap::MessageReference reply = xoap::createMessage();
    xoap::SOAPEnvelope env = reply->getSOAPPart().getEnvelope();
    xoap::SOAPName responseName = env.createName( "userPixelTTCSupervisorResponse", "xdaq", XDAQ_NS_URI);
    env.getBody().addBodyElement ( responseName );
    soapResponse_ = reply;
    return reply;                        
  } catch ( HardwareAccessException& e) {
    LOG4CPLUS_ERROR(this->getApplicationLogger(),  e.what() );
    // delete everything which was created so far 
    deleteHardware(); 
    XCEPT_RAISE(xcept::Exception, "userCommand() failed, going to state Halted");
  }
}

// we use selectManual() instead of disableTrigger()
// so that we can send triggers by hand (VME access)
void PixelTTCSupervisor::ConfigureAction(toolbox::Event::Reference /*e*/) throw(xcept::Exception) {
  
  //cout << "[TIM-INFO]: PixelTTCSupervisor::ConfigureAction() called."<<endl;
  html << "PixelTTCSupervisor::ConfigureAction(): State Change \"Configure\" ..."
       <<endl;

  //cout << "PixelTTCSupervisor::ConfigureAction():  getting theTTCciConfig_" << endl;
  //cout << "Using global key " << theGlobalKey_->key() << endl;
  PixelConfigInterface::get(theTTCciConfig_, "pixel/ttcciconfig/", *theGlobalKey_);
  assert(theTTCciConfig_!=0);

  // create the TTCci
  try {
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "Configure: creating TTCci object");

    try {
      if (!TTCciPtr_) CreateTTCciObject(false);
      //TTCciPtr_->DisableL1A();   
    } catch (const std::exception & e) {
      cout << "Exception!!! " << e.what() << endl;
      deleteHardware();
      err_message_ << RED << BOLD <<"ERROR: "<<NOBOLD<<e.what()<<NOCOL;
      any_errors_ = 1;
      return;
      //XCEPT_RAISE(xcept::Exception, string("Error during configure: ") + e.what());		
    }
    
    // set up the ttcvi with some default settings:
    LOG4CPLUS_DEBUG(this->getApplicationLogger(), "Configure: setting defaults in TTCci");
    SendSOAPMessageToPixelTTCSupervisor("userCommand","CommandPar",
                                  "ReadCounters");

    //LOG4CPLUS_INFO(this->getApplicationLogger(), toolbox::toString("Loading config file '%s' into TTCci", asciConfigurationFilePath_.value_.c_str()));
    //TTCciPtr_->InitBanks(asciConfigurationFilePath_);
    
    if (int(ReloadAtEveryConfigure_)==1 || ConfigModified_){
      ConfigureTTCcifromASCI();
    }
    
    if (!TTCciPtr_->ExecuteSequence("Configure")){
      any_errors_ = 1;
      err_message_ <<RED<<"ERROR(S) or WARNING(S) while executing sequence \""
                    <<"Configure\"!<br>"<<endl
                    <<"&nbsp;&nbsp;&nbsp; --> Read output (below) for more details."
                    <<NOCOL<<"<br>"<<endl;
    }
    if (TTCciPtr_->IsL1AEnabled()){
      any_errors_ = 1;
      err_message_ << RED<<"WARNING: L1A is enabled after \"Configure\"!"<<NOCOL<<"<br>"
                    <<endl
                    << "(Please consider to enable L1A only in the \"Enable\" and \"Resume\" sequences)<br>"
                    <<endl;
    }
    
    //ReadTTCciContent(); // Read back what is in the banks etc...

  } catch ( const std::exception & e) {
    LOG4CPLUS_ERROR(this->getApplicationLogger(),  e.what() );
    // delete everything which was created so far 
    deleteHardware(); 
    XCEPT_RAISE(xcept::Exception, "ConfigureAction() failed, going to state Halted");
    
  }
  LOG4CPLUS_DEBUG(this->getApplicationLogger(), "Configure: end");

}

// could add a parameter to select the trigger source
// no manual triggers should be possible while running.
void PixelTTCSupervisor::EnableAction(toolbox::Event::Reference /*e*/) throw(xcept::Exception) {
  //cout << "[TIM-INFO]: PixelTTCSupervisor::EnableAction() called."<<endl;
  html << "PixelTTCSupervisor::EnableAction(): State Change \"Enable\" ..."
        <<endl;
  try{
    if (!TTCciPtr_->ExecuteSequence("Enable")){
      any_errors_ = 1;
      err_message_ <<RED<<"ERROR(S) or WARNING(S) while executing sequence \""
                    <<"Enable\"!<br>"<<endl
                    <<"&nbsp;&nbsp;&nbsp; --> Read output (below) for more details."
                    <<NOCOL<<"<br>"<<endl;
    }
    //TTCciPtr_->EnableL1A();
    if (TTCciPtr_->IsL1ADisabled()){
      any_errors_ = 1;
      err_message_ << RED<<"WARNING: L1A is disabled after \"Enable\"!"<<NOCOL<<"<br>"
                    <<endl
                    << "(Please consider to enable L1A in the \"Enable\" and \"Resume\" sequences)"
                    <<endl;
      
    }
  } catch ( HardwareAccessException& e) {
    LOG4CPLUS_ERROR(this->getApplicationLogger(),  e.what() );
    // delete everything which was created so far 
    deleteHardware(); 
    XCEPT_RAISE(xcept::Exception, "Enable failed, going to state Halted");
  }
}

// can send manual triggers while stopped
void PixelTTCSupervisor::StopAction(toolbox::Event::Reference /*e*/) throw(xcept::Exception) {
  //cout << "[TIM-INFO]: PixelTTCSupervisor::StopAction() called."<<endl;
  html << "PixelTTCSupervisor::StopAction(): State Change \"Stop\" ..."
        <<endl;
  try{ 
	  
    if (!TTCciPtr_->ExecuteSequence("Suspend")){
      any_errors_ = 1;
      err_message_ <<RED<<"ERROR(S) or WARNING(S) while executing sequence \""
                    <<"Stop\"!<br>"<<endl
                    <<"&nbsp;&nbsp;&nbsp; --> Read output (below) for more details."
                    <<NOCOL<<"<br>"<<endl;
    }
    if (TTCciPtr_->IsL1AEnabled()){
      any_errors_ = 1;
      err_message_ << RED<<"WARNING: L1A is enabled after \"Stop\"!"<<NOCOL<<"<br>"
                    <<endl
                    << "(Please consider to enable L1A only in the \"Enable\" and \"Resume\" sequences)"
                    <<endl;      
    }
    //TTCciPtr_->DisableL1A();
  } catch ( HardwareAccessException& e) {
    LOG4CPLUS_ERROR(this->getApplicationLogger(),  e.what() );
    // delete everything which was created so far 
    deleteHardware(); 
    XCEPT_RAISE(xcept::Exception, "Stop failed, going to state Halted");
  }
}

// can send manual triggers while stopped
void PixelTTCSupervisor::SuspendAction(toolbox::Event::Reference /*e*/) throw(xcept::Exception) {
  //cout << "[TIM-INFO]: PixelTTCSupervisor::SuspendAction() called."<<endl;
  html << "PixelTTCSupervisor::SuspendAction(): State Change \"Suspend\" ..."
        <<endl;
  try{ 
	  
    if (!TTCciPtr_->ExecuteSequence("Suspend")){
      any_errors_ = 1;
      err_message_ <<RED<<"ERROR(S) or WARNING(S) while executing sequence \""
                    <<"Suspend\"!<br>"<<endl
                    <<"&nbsp;&nbsp;&nbsp; --> Read output (below) for more details."
                    <<NOCOL<<"<br>"<<endl;
    }
    if (TTCciPtr_->IsL1AEnabled()){
      any_errors_ = 1;
      err_message_ << RED<<"WARNING: L1A is enabled after \"Suspend\"!"<<NOCOL<<"<br>"
                    <<endl
                    << "(Please consider to enable L1A only in the \"Enable\" and \"Resume\" sequences)"
                    <<endl;      
    }
    //TTCciPtr_->DisableL1A();
  } catch ( HardwareAccessException& e) {
    LOG4CPLUS_ERROR(this->getApplicationLogger(),  e.what() );
    // delete everything which was created so far 
    deleteHardware(); 
    XCEPT_RAISE(xcept::Exception, "Suspend failed, going to state Halted");
  }
}

void PixelTTCSupervisor::ResumeAction(toolbox::Event::Reference /*e*/)  throw(xcept::Exception){
  //cout << "[TIM-INFO]: PixelTTCSupervisor::ResumeAction() called."<<endl;  
  html << "PixelTTCSupervisor::ResumeAction(): State Change \"Resume\" ..."
        <<endl;
  try{
    //TTCciPtr_->selectTriggerSource(triggerSource_);
    if (!TTCciPtr_->ExecuteSequence("Resume")){
      any_errors_ = 1;
      err_message_ <<RED<<"ERROR(S) or WARNING(S) while executing sequence \""
                    <<"Resume\"!<br>"<<endl
                    <<"&nbsp;&nbsp;&nbsp; --> Read output (below) for more details."
                    <<NOCOL<<"<br>"<<endl;
    }
    if (TTCciPtr_->IsL1ADisabled()){
      any_errors_ = 1;
      err_message_ << RED<<"WARNING: L1A is enabled after \"Configure\"!"<<NOCOL<<"<br>"
                    <<endl
                    << "(Please consider to enable L1A only in the \"Enable\" and \"Resume\" sequences)"
                    <<endl;
    }
    //TTCciPtr_->EnableL1A();
  } catch ( HardwareAccessException& e) {
    LOG4CPLUS_ERROR(this->getApplicationLogger(),  e.what() );
    // delete everything which was created so far 
    deleteHardware(); 
    XCEPT_RAISE(xcept::Exception, "Resume failed, going to state Halted");
  }
}

// No more manual triggers are possible
void PixelTTCSupervisor::HaltAction(toolbox::Event::Reference /*e*/) throw(xcept::Exception) {
  html << "PixelTTCSupervisor::HaltAction(): State Change \"Halt\" ..."
        <<endl;

  try{
    //TTCciPtr_->DisableL1A();
    if (!TTCciPtr_->ExecuteSequence("Halt")){
      any_errors_ = 1;
      err_message_ <<RED<<"ERROR(S) or WARNING(S) while executing sequence \""
                    <<"Halt\"!<br>"<<endl
                    <<"&nbsp;&nbsp;&nbsp; --> Read output (below) for more details."
                    <<NOCOL<<"<br>"<<endl;
    }
    if (TTCciPtr_->IsL1AEnabled()){
      any_errors_ = 1;
      err_message_ << RED<<"WARNING: L1A is enabled after \"Halt\"!"<<NOCOL<<"<br>"
                    <<endl
                    << "(Please consider to disable L1A in the \"Halt\" and \"Suspend\" sequences)"
                    <<endl;
    }
    //triggerCount_ = TTCciPtr_->getTriggerCount();
    SendSOAPMessageToPixelTTCSupervisor("userCommand","CommandPar","ReadCounters");
    //deleteHardware();
    //TTCciPtr_->MainReset();
  } catch ( HardwareAccessException& e) {
    LOG4CPLUS_ERROR(this->getApplicationLogger(),  e.what() );
    // delete everything which was created so far 
    deleteHardware(); 
    XCEPT_RAISE(xcept::Exception, "PixelTTCSupervisor::HaltAction() failed, going to state Halted");
  }
}

// SOAP call to the User Sequence
//void PixelTTCSupervisor::UserAction(toolbox::Event::Reference /*e*/) throw(xcept::Exception) {
xoap::MessageReference  PixelTTCSupervisor::UserAction(xoap::MessageReference msg) throw(xoap::exception::Exception) {

  html << "PixelTTCSupervisor::UserAction(): Sequence \"User\" called ..."
        <<endl;
  soapReceived_ = msg; 
  
  try{
    //TTCciPtr_->DisableL1A();
    if (!TTCciPtr_ || !TTCciPtr_->ExecuteSequence("User")){
      any_errors_ = 1;
      err_message_ <<RED<<"ERROR(S) or WARNING(S) while executing sequence \""
                    <<"User\"!<br>"<<endl
                    <<"&nbsp;&nbsp;&nbsp; --> Read output (below) for more details."
                    <<NOCOL<<"<br>"<<endl;
    }
    //SendSOAPMessageToPixelTTCSupervisor("userCommand","CommandPar","ReadCounters");
    xoap::MessageReference reply = xoap::createMessage();
    xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
    xoap::SOAPName responseName = envelope.createName( string("User")+string("Response"), "xdaq", XDAQ_NS_URI);
    envelope.getBody().addBodyElement ( responseName );
    soapResponse_ = reply;
    return reply;                        
  } catch ( HardwareAccessException& e) {
    LOG4CPLUS_ERROR(this->getApplicationLogger(),  e.what() );
    // delete everything which was created so far 
    deleteHardware(); 
    XCEPT_RAISE(xcept::Exception, "PixelTTCSupervisor::UserAction() failed, going to state Halted");
  }
}

// SOAP call to the LevelOne Sequence
xoap::MessageReference  PixelTTCSupervisor::LevelOneAction(xoap::MessageReference msg) throw(xoap::exception::Exception) {

  html << "PixelTTCSupervisor::LevelOneAction(): Sequence \"LevelOne\" called ..."
        <<endl;
  soapReceived_ = msg; 
  
  try{
    //TTCciPtr_->DisableL1A();
    if (!TTCciPtr_ || !TTCciPtr_->ExecuteSequence("LevelOne")){
      any_errors_ = 1;
      err_message_ <<RED<<"ERROR(S) or WARNING(S) while executing sequence \""
                    <<"LevelOne\"!<br>"<<endl
                    <<"&nbsp;&nbsp;&nbsp; --> Read output (below) for more details."
                    <<NOCOL<<"<br>"<<endl;
    }
    //SendSOAPMessageToPixelTTCSupervisor("userCommand","CommandPar","ReadCounters");
    xoap::MessageReference reply = xoap::createMessage();
    xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
    xoap::SOAPName responseName = envelope.createName( string("LevelOne")+string("Response"), "xdaq", XDAQ_NS_URI);
    envelope.getBody().addBodyElement ( responseName );
    soapResponse_ = reply;
    return reply;                        
  } catch ( HardwareAccessException& e) {
    LOG4CPLUS_ERROR(this->getApplicationLogger(),  e.what() );
    // delete everything which was created so far 
    deleteHardware(); 
    XCEPT_RAISE(xcept::Exception, "PixelTTCSupervisor::LevelOneAction() failed, going to state Halted");
  }
}

// SOAP call to the CalSync Sequence
xoap::MessageReference  PixelTTCSupervisor::CalSyncAction(xoap::MessageReference msg) throw(xoap::exception::Exception) {

  html << "PixelTTCSupervisor::CalSyncAction(): Sequence \"CalSync\" called ..."
        <<endl;
  soapReceived_ = msg; 
  
  try{
    //TTCciPtr_->DisableL1A();
    if (!TTCciPtr_ || !TTCciPtr_->ExecuteSequence("CalSync")){
      any_errors_ = 1;
      err_message_ <<RED<<"ERROR(S) or WARNING(S) while executing sequence \""
                    <<"CalSync\"!<br>"<<endl
                    <<"&nbsp;&nbsp;&nbsp; --> Read output (below) for more details."
                    <<NOCOL<<"<br>"<<endl;
    }
    //SendSOAPMessageToPixelTTCSupervisor("userCommand","CommandPar","ReadCounters");
    xoap::MessageReference reply = xoap::createMessage();
    xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
    xoap::SOAPName responseName = envelope.createName( string("CalSync")+string("Response"), "xdaq", XDAQ_NS_URI);
    envelope.getBody().addBodyElement ( responseName );
    soapResponse_ = reply;
    return reply;                        
  } catch ( HardwareAccessException& e) {
    LOG4CPLUS_ERROR(this->getApplicationLogger(),  e.what() );
    // delete everything which was created so far 
    deleteHardware(); 
    XCEPT_RAISE(xcept::Exception, "PixelTTCSupervisor::CalSyncAction() failed, going to state Halted");
  }
}

// SOAP call to the ResetROC Sequence
xoap::MessageReference  PixelTTCSupervisor::ResetROCAction(xoap::MessageReference msg) throw(xoap::exception::Exception) {

  html << "PixelTTCSupervisor::ResetROCAction(): Sequence \"ResetROC\" called ..."
        <<endl;
  soapReceived_ = msg; 
  
  try{
    //TTCciPtr_->DisableL1A();
    if (!TTCciPtr_ || !TTCciPtr_->ExecuteSequence("ResetROC")){
      std::cout << "[PixelTTCSupervisor] ResetROC has error!"<<std::endl;
      any_errors_ = 1;
      err_message_ <<RED<<"ERROR(S) or WARNING(S) while executing sequence \""
                    <<"ResetROC\"!<br>"<<endl
                    <<"&nbsp;&nbsp;&nbsp; --> Read output (below) for more details."
                    <<NOCOL<<"<br>"<<endl;
    }
    //SendSOAPMessageToPixelTTCSupervisor("userCommand","CommandPar","ReadCounters");
    xoap::MessageReference reply = xoap::createMessage();
    xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
    xoap::SOAPName responseName = envelope.createName( string("ResetROC")+string("Response"), "xdaq", XDAQ_NS_URI);
    envelope.getBody().addBodyElement ( responseName );
    soapResponse_ = reply;
    return reply;                        
  } catch ( HardwareAccessException& e) {
    LOG4CPLUS_ERROR(this->getApplicationLogger(),  e.what() );
    // delete everything which was created so far 
    deleteHardware(); 
    XCEPT_RAISE(xcept::Exception, "PixelTTCSupervisor::ResetROCAction() failed, going to state Halted");
  }
}

// SOAP call to the ResetTBM Sequence
xoap::MessageReference  PixelTTCSupervisor::ResetTBMAction(xoap::MessageReference msg) throw(xoap::exception::Exception) {

  html << "PixelTTCSupervisor::ResetTBMAction(): Sequence \"ResetTBM\" called ..."
        <<endl;
  soapReceived_ = msg; 
  
  try{
    //TTCciPtr_->DisableL1A();
    if (!TTCciPtr_ || !TTCciPtr_->ExecuteSequence("ResetTBM")){
      std::cout << "[PixelTTCSupervisor] ResetTBM has error!"<<std::endl;
      any_errors_ = 1;
      err_message_ <<RED<<"ERROR(S) or WARNING(S) while executing sequence \""
                    <<"ResetTBM\"!<br>"<<endl
                    <<"&nbsp;&nbsp;&nbsp; --> Read output (below) for more details."
                    <<NOCOL<<"<br>"<<endl;
    }
    //SendSOAPMessageToPixelTTCSupervisor("userCommand","CommandPar","ReadCounters");
    xoap::MessageReference reply = xoap::createMessage();
    xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
    xoap::SOAPName responseName = envelope.createName( string("ResetTBM")+string("Response"), "xdaq", XDAQ_NS_URI);
    envelope.getBody().addBodyElement ( responseName );
    soapResponse_ = reply;
    return reply;                        
  } catch ( HardwareAccessException& e) {
    LOG4CPLUS_ERROR(this->getApplicationLogger(),  e.what() );
    // delete everything which was created so far 
    deleteHardware(); 
    XCEPT_RAISE(xcept::Exception, "PixelTTCSupervisor::ResetTBMAction() failed, going to state Halted");
  }
}


//SOAP call to assorted periodic sequences
xoap::MessageReference PixelTTCSupervisor::SendPeriodic(xoap::MessageReference msg) throw(xoap::exception::Exception) {

  string ttcmode = Receive(msg);
  cout << "PixelTTCSupervisor::SendPeriodic():  the requested TTC mode is " << ttcmode << endl;

  oldState_ = fsm_.getStateName(fsm_.getCurrentState());

  if(oldState_=="Enabled") {
    cout << "The TTC is already Enabled." << endl;
    cout << "Cannot reconfigure for periodic signals." << endl;
    string reply = ttcmode+"Failed";
    xoap::MessageReference response = MakeSOAPMessageReference(reply);
    return response;
  }

  unsigned int newGlobalKey=PixelConfigInterface::getAliases_map().find("TTC"+ttcmode)->second;
  string reply;
  
  if(oldState_!="Halted") {
    oldkey_ = theGlobalKey_->key();
    cout << "PixelTTCSupervisor::SendPeriodic():  the global key starts as " << oldkey_ << endl;
    reply = Send(getApplicationDescriptor(),"Halt");
    if(reply=="HaltResponse") {
      cout << "PixelTTCSupervisor::SendPeriodic():  The TTC has been halted." << endl;
    } else {
      cout << "PixelTTCSupervisor::SendPeriodic():  The TTC has not been halted." << endl;
      reply = "HaltFailed";
      xoap::MessageReference response = MakeSOAPMessageReference(reply);
      return response;
    }
  } else {
    cout << "PixelTTCSupervisor::SendPeriodic():  This is the first time the TTC is configured." << endl;
  }
  theGlobalKey_ = new PixelConfigKey(newGlobalKey);
  cout << "Set global key to " << newGlobalKey << endl;

  Attribute_Vector parameters(1);
  parameters.at(0).name_="GlobalKey";  parameters.at(0).value_=itoa(newGlobalKey);
  reply = Send(getApplicationDescriptor(),"Configure",parameters);
  if(reply=="ConfigureResponse") {
    cout << "PixelTTCSupervisor::SendPeriodic():  The TTC has been configured." << endl;
  } else {
    cout << "PixelTTCSupervisor::SendPeriodic():  The TTC has not been configured." << endl;
    reply = "ConfigureFailed";
    xoap::MessageReference response = MakeSOAPMessageReference(reply);
    return response;
  }
  reply = Send(getApplicationDescriptor(),"Enable");
  if(reply=="EnableResponse") {
    cout << "PixelTTCSupervisor::SendPeriodic():  The TTC has been enabled." << endl;
    reply = ttcmode+"Response";
    xoap::MessageReference response = MakeSOAPMessageReference(reply);
    return response;
  } else {
    cout << "PixelTTCSupervisor::SendPeriodic():  The TTC has not been enabled." << endl;
    reply = "EnableFailed";
    xoap::MessageReference response = MakeSOAPMessageReference(reply);
    return response;
  }
}


//SOAP call to stop periodic sequences
xoap::MessageReference PixelTTCSupervisor::StopPeriodic(xoap::MessageReference msg) throw(xoap::exception::Exception) {

  string currentState = fsm_.getStateName(fsm_.getCurrentState());
  string reply;

  //Halted, Configured, or Suspended

  reply = Send(getApplicationDescriptor(),"Halt");
  if(reply=="HaltResponse") {
    cout << "PixelTTCSupervisor::StopPeriodic():  The TTC has been halted." << endl;
  } else {
    cout << "PixelTTCSupervisor::StopPeriodic():  The TTC has not been halted." << endl;
    reply = "HaltFailed";
    xoap::MessageReference response = MakeSOAPMessageReference(reply);
    return response;
  }
  if(oldState_=="Halted") {
    reply="StopPeriodicResponse";
    xoap::MessageReference response = MakeSOAPMessageReference(reply);
    return response;
  }

  theGlobalKey_ = new PixelConfigKey(oldkey_);
  cout << "Reset global key to " << oldkey_ << endl;
  Attribute_Vector parameters(1);
  parameters.at(0).name_="GlobalKey";  parameters.at(0).value_=itoa(oldkey_);
  
  reply = Send(getApplicationDescriptor(),"Configure",parameters);
  if(reply=="ConfigureResponse") {
    cout << "PixelTTCSupervisor::StopPeriodic():  The TTC has been configured." << endl;
  } else {
    cout << "PixelTTCSupervisor::StopPeriodic():  The TTC has not been configured." << endl;
    reply = "ConfigureFailed";
    xoap::MessageReference response = MakeSOAPMessageReference(reply);
    return response;
  }
  if(oldState_=="Ready") {
    reply="SendPeriodicResponse";
    xoap::MessageReference response = MakeSOAPMessageReference(reply);
    return response;
  }

  reply = Send(getApplicationDescriptor(),"Enable");
  if(reply=="EnableResponse") {
    cout << "PixelTTCSupervisor::StopPeriodic():  The TTC has been enabled." << endl;
  } else {
    cout << "PixelTTCSupervisor::StopPeriodic():  The TTC has not been enabled." << endl;
    reply = "EnableFailed";
    xoap::MessageReference response = MakeSOAPMessageReference(reply);
    return response;
  }
  reply = Send(getApplicationDescriptor(),"Suspend");
  if(reply=="SuspendResponse") {
    cout << "PixelTTCSupervisor::StopPeriodic():  The TTC has been suspended." << endl;
  } else {
    cout << "PixelTTCSupervisor::StopPeriodic():  The TTC has not been suspended." << endl;
    reply = "SuspendFailed";
    xoap::MessageReference response = MakeSOAPMessageReference(reply);
    return response;
  }
  reply = "StopPeriodicResponse";
  xoap::MessageReference response = MakeSOAPMessageReference(reply);
  return response;

}


// Helper function: clean up the hardware components
void PixelTTCSupervisor::deleteHardware() {
  if (TTCciPtr_) delete TTCciPtr_;
  TTCciPtr_ = 0;
#ifdef SIDET
  releaseBusAdapter();
#else
  if (busAdapter_) delete busAdapter_;
#endif
  busAdapter_ = 0;
}

// Create a SOAP Message to PixelTTCSupervisor:
xoap::MessageReference PixelTTCSupervisor::CreateSOAPMessage4PixelTTCSupervisor(const std::string &commandname, const std::string &varName, const std::string &var) const {
  xoap::MessageReference msg = xoap::createMessage();
  xoap::SOAPPart soap = msg->getSOAPPart();
  xoap::SOAPEnvelope envelope = soap.getEnvelope();

  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName command = envelope.createName(commandname,"xdaq",
                                               "urn:xdaq-soap:3.0");
  body.addBodyElement(command);
  if (varName.size()>0){
    xoap::SOAPName name = xoap::SOAPName(varName,"xdaq","urn:xdaq-soap:3.0");
    body.addAttribute(name, var);
  }else{ 
    ;
  }
  cout << "============================================================"<<endl;
  msg->writeTo(cout);
  cout << endl
       << "============================================================"<<endl;
  return msg;
}

// Send a SOAP Message to PixelTTCSupervisor:
void PixelTTCSupervisor::SendSOAPMessageToPixelTTCSupervisor(const std::string &commandname, 
                                             const std::string &varName, 
                                             const std::string &var)  {
  xoap::MessageReference mymsg = CreateSOAPMessage4PixelTTCSupervisor(commandname,
                                                                varName,var);
  
  //xdaq::ApplicationDescriptor * d = 
  //  getApplicationContext()->getApplicationGroup()
  //  ->getApplicationDescriptor(classname_, instance_);//0
  xoap::MessageReference reply=getApplicationContext()->postSOAP(mymsg,getApplicationDescriptor());
}


// The TTCci Web Page in XDAQ
void PixelTTCSupervisor::Default(xgi::Input * /*in*/, xgi::Output * out ) throw (xgi::exception::Exception){

  LastPage_ = "Default";
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  //*out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  *out << cgicc::title("TTCci \""+name_.toString()+"\"") << std::endl;
  
  if (TTCciPtr_ && AutoRefresh_){
    *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"3; URL="
         <<LastPage_<<"\">" <<endl;
  }
  
  PrintHTMLHeader(out);

  *out << cgicc::h1(toolbox::toString("TTCci Control (%s)",
                                      GetName(1).c_str()))<<std::endl;


  ReadTTCciCounters();

  ErrorStatement(out);

  // The State Machine "Table"
  std::string url = "/";
  url += getApplicationDescriptor()->getURN();
  url += "/dispatch";	
  HTMLTable tab(*out,1,10,0,"","center");
  tab.NewCell();
  *out << "TTCci State Machine";
  
  tab.NewCell();
  PrintSMState(out);
  tab.NewCell();
  if (TTCciPtr_){
    if (TTCciPtr_->IsLaserOn()){
      *out << GREEN << "LASER on" << NOCOL;
    }else{
      *out << RED << "LASER off" << NOCOL;
    }
  }else{
    *out << GREY << "LASER" << NOCOL;
  }
  tab.NewCell();
  *out << cgicc::form().set("method","get").set("action", string("/"+getApplicationDescriptor()->getURN()+"/Command")).set("enctype","multipart/form-data") << std::endl;
  if (TTCciPtr_){
  *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "CreateTTCciObject" ).set("disabled", "true");;
  }else{
  *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "CreateTTCciObject" );
  }
  *out << cgicc::form();       
  // Periodic Sequence Status:
  tab.NewCell();
  if (TTCciPtr_){
    if (!TTCciPtr_->PeriodicSequenceEnabled() || TTCciPtr_->Periodicity()<0.0){
      *out << GREY <<"Periodic Seq.<br>Off"<<endl;
      if (TTCciPtr_->Periodicity()>0.0){
        *out << "("<<TTCciPtr_->Periodicity()<<" s)" << endl;
      }
      *out << NOCOL<<endl;
    }else{
      *out << RED << "Periodic Seq.<br>On ("
           <<TTCciPtr_->Periodicity()<<" s)" << NOCOL;
    }
  }else{
    *out << GREY << "Periodic Seq." << NOCOL;
  }
  
  // The Configure Button
  tab.NewRow();
  tab.NewCell();
  *out << cgicc::form().set("method","get").set("action", url).set("enctype","multipart/form-data") << std::endl;
  if (fsm_.getCurrentState() != 'H'){
  *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", "Configure" ).set("disabled", "true");;
  }else{
  *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", "Configure" );
  }
  if (int(ReloadAtEveryConfigure_)==1){
    *out << RED<<"*)"<<NOCOL<<endl;
  }
  *out << cgicc::form();     
  // The Enable Button
  tab.NewCell();
  *out << cgicc::form().set("method","get").set("action", url).set("enctype","multipart/form-data") << std::endl;
  if (fsm_.getCurrentState() != 'R'){
  *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", "Enable" ).set("disabled", "true");;
  }else{
  *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", "Enable" );
  }
  *out << cgicc::form();       
  // The Suspend Button
  tab.NewCell();
  *out << cgicc::form().set("method","get").set("action", url).set("enctype","multipart/form-data") << std::endl;
  if (fsm_.getCurrentState() != 'E'){
  *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", "Suspend" ).set("disabled", "true");;
  }else{
  *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", "Suspend" );
  }
  *out << cgicc::form();       
  *out << "</th>"<<endl;
  // The Resume Button
  *out << "<th>";
  *out << cgicc::form().set("method","get").set("action", url).set("enctype","multipart/form-data") << std::endl;
  if (fsm_.getCurrentState() != 'S'){
  *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", "Resume" ).set("disabled", "true");;
  }else{
  *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", "Resume" );
  }
  *out << cgicc::form();       
  // The halt Button
  tab.NewCell();
  *out << cgicc::form().set("method","get").set("action", url).set("enctype","multipart/form-data") << std::endl;
  if (fsm_.getCurrentState() != 'E' && 
      fsm_.getCurrentState() != 'R' &&
      fsm_.getCurrentState() != 'S' ){
  *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", "Halt" ).set("disabled", "true");;
  }else{
  *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", "Halt" );
  }
  *out << cgicc::form(); 

  tab.NewRow();
  // The User Sequence Button
  tab.NewCell("",2);
  *out << "Exec. Sequence " << endl;
  *out << cgicc::form().set("method","get").set("action", url).set("enctype","multipart/form-data") << std::endl;
  if ( !TTCciPtr_ ){
  *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", "User" ).set("disabled", "true");;
  }else{
  *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", "User" );
  }
  *out << cgicc::form(); 
      
  tab.NewCell();
  // The LevelOne Sequence Button
  *out << cgicc::form().set("method","get").set("action", url).set("enctype","multipart/form-data") << std::endl;
  if ( !TTCciPtr_ ){
  *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", "LevelOne" ).set("disabled", "true");;
  }else{
  *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", "LevelOne" );
  }
  *out << cgicc::form(); 

  tab.NewCell();
  // The CalSync Sequence Button
  *out << cgicc::form().set("method","get").set("action", url).set("enctype","multipart/form-data") << std::endl;
  if ( !TTCciPtr_ ){
  *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", "CalSync" ).set("disabled", "true");;
  }else{
  *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", "CalSync" );
  }
  *out << cgicc::form(); 

  tab.NewCell();
  // The ResetROC Sequence Button
  *out << cgicc::form().set("method","get").set("action", url).set("enctype","multipart/form-data") << std::endl;
  if ( !TTCciPtr_ ){
    *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", "ResetROC" ).set("disabled", "true");;
  }else{
    *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", "ResetROC" );
  }
  *out << cgicc::form();

  //tab.NewCell();
  tab.Close();
  
  // DiagSystem GUI
  std::string urlDiag_ = "/"; \
  urlDiag_ += getApplicationDescriptor()->getURN(); \
  urlDiag_ += "/callDiagSystemPage"; \
  *out << "<hr/> " << std::endl;
  *out << "<h2> Error Dispatcher </h2> "<<std::endl;
  *out << "<a href=" << urlDiag_ << ">Configure DiagSystem</a>" <<std::endl;
  *out << " <hr/> " << std::endl;
  
  // Basic Monitoring & Counters
  if (1){
    std::string command_url = "/";
    command_url += getApplicationDescriptor()->getURN();
    command_url += "/Command";	
    tab.Set(1,3,2,"","left");
    tab.Open();
    
    tab.NewRow();
    tab.NewCell();
    *out << BOLD<<"<center> Counters <br> &amp; Status </center>"<<NOBOLD;

    // Print all values read
    tab.NewCell("",1,2);
    *out << (!TTCciPtr_ ? GREY : "");
    *out << UNDERL <<"Counters" << NOUNDERL << endl;
    *out << "<br>" << "Evt-ID: " << EventCounter_ <<endl;
    *out << "<br>" << "Orbit: " << OrbitCounter_ <<endl;
    *out << "<br>" << "BGO-Req.Cntr: " << StrobeCounter_ <<endl;
    *out << (!TTCciPtr_ ? NOCOL : "" );
    if (TTCciPtr_){
      const double dt = difftime(tnow_, tprevious_);
      *out << "<br><br>"<<UNDERL<<"L1A rate (after "<<dt<<" s):"
           <<NOUNDERL<<"<br>"<<endl;
      double diff = double(EventCounter_-previousEventCounter_);
      double odiff = double(OrbitCounter_-previousOrbitCounter_);
      *out << int(diff)<<" L1As in "<<odiff<<" orbs.<br>"<<endl;
      if (odiff>0.0 && dt>0.0) 
        *out << RED<<(diff/(odiff*3564*25.0e-9))<<" Hz"<<NOCOL
             <<"<br>"<<endl;
      else 
        *out << RED<<"nan"<<" Hz"<<NOCOL<<"<br>"<<endl;

      if (odiff>0.0) *out << (diff/odiff) << " L1As/orbit <br>"<<endl;
      else *out << "nan" << " L1As/orbit <br>"<<endl;
      if (diff>0.0) *out << (odiff/diff) << " orbits/L1A <br>"<<endl;
      else *out << "nan" << " orbits/L1A <br>"<<endl;
      diff = double(StrobeCounter_-previousStrobeCounter_);
      *out << UNDERL<<"BGO-req.rate:"<<NOUNDERL<<"<br>"<<endl;
      *out << int(diff)<<" BGOs<br>"<<endl;
      if (dt>0.0) *out << RED<<(diff/(odiff*3564*25.0e-9))
                       <<" Hz"<<NOCOL<<"<br>"<<endl;
      else *out << RED<<"nan"<<" Hz"<<NOCOL<<"<br>"<<endl;
      if (odiff>0.0) *out << (diff/odiff) << " BGOs/orbit <br>"<<endl;
      else *out << "nan" << " BGOs/orbit <br>"<<endl;
      if (diff>0.0) *out << (odiff/diff) << " orbits/BGO <br>"<<endl;
      else *out << "nan" << " orbits/BGO <br>"<<endl;
    }
    // Print second lot of values
    tab.NewCell("",1,2);
    *out << ( !TTCciPtr_ ? GREY : "" );
    *out << UNDERL<<"Board Status:"<<NOUNDERL<<MONO<<BLUE<<hex<<" 0x"
         << BoardStatus_ <<NOCOL<<NOMONO<<dec<<endl;
    *out << ( !TTCciPtr_ ? NOCOL : "" );
    string defaultcol = GREEN, alertcol = RED;
    if (TTCciPtr_){
      { // BDataCancelled:
        bool isok  = !TTCciPtr_->IsBDataCancelled();
        bool isok2 = !TTCciPtr_->IsBDataCancelled_Latched(false);
        string col = (isok?defaultcol:alertcol);
        *out << "<br>" << "BData Cancelled? " <<col
             << (isok?"no":"yes")<<NOCOL;
        col = (isok2?defaultcol:alertcol);
        *out << " ("<<col<<(isok2?"no":"yes")<<NOCOL<<")"<<endl;
      }
      
      { // DataClkError:
        bool isok  = !TTCciPtr_->IsDataClkError(false);
        bool isok2 = !TTCciPtr_->IsDataClkError_Latched(false);
        string col = (isok?defaultcol:alertcol);
        *out << "<br>" << "Dat/Clk sync error? " <<col
             << (isok?"no":"yes")<<NOCOL;
        col = (isok2?defaultcol:alertcol);
        *out << " ("<<col<<(isok2?"no":"yes")<<NOCOL<<")"<<endl;
      }
      { // IsClkSingleEvtUpset:
        bool isok  = !TTCciPtr_->IsClkSingleEvtUpset(false);
        bool isok2 = !TTCciPtr_->IsClkSingleEvtUpset_Latched(false);
        string col = (isok?defaultcol:alertcol);
        *out << "<br>" << "Clk. single-evt. upset? " <<col
             << (isok?"no":"yes")<<NOCOL;
        col = (isok2?defaultcol:alertcol);
        *out << " ("<<col<<(isok2?"no":"yes")<<NOCOL<<")"<<endl;
      }
      { // MissedL1A:
        bool isok  = !TTCciPtr_->MissedL1A(false);
        bool isok2 = !TTCciPtr_->MissedL1A_Latched(false);
        string col = (isok?defaultcol:alertcol);
        *out << "<br>" << "L1A missed @ 40MHz? " <<col
             << (isok?"no":"yes")<<NOCOL;
        col = (isok2?defaultcol:alertcol);
        *out << " ("<<col<<(isok2?"no":"yes")<<NOCOL<<")"<<endl;
      }
      { // DoubleL1Aat40MHz:
        bool isok  = !TTCciPtr_->DoubleL1Aat40MHz(false);
        bool isok2 = !TTCciPtr_->DoubleL1Aat40MHz_Latched(false);
        string col = (isok?defaultcol:alertcol);
        *out << "<br>" << "Double-L1A @ 40MHz? " <<col
             << (isok?"no":"yes")<<NOCOL;
        col = (isok2?defaultcol:alertcol);
        *out << " ("<<col<<(isok2?"no":"yes")<<NOCOL<<")"<<endl;
      }
      *out << SMALLER<<"<br> &nbsp;&nbsp;&nbsp;&nbsp;(..) = latched"
           <<NOSMALLER<<endl; 
      { // Clock inverted?
        bool inv  = TTCciPtr_->ClockInverted(false);
        string col = (inv?defaultcol:defaultcol);
        *out << "<br>" << "Clock inverted? " 
             << (inv?"yes":"no")<<endl;
      }
      { // Clock locked?
        bool lock  = TTCciPtr_->ClockLocked(false);
        string col = (lock?defaultcol:alertcol);
        *out << "<br>" << "Clock locked? " <<col
             << (lock?"yes":"no")<<NOCOL<<endl;
      }
    }
    
    *out << NOMONO;
    // Read all Counters
    tab.NewRow();
    tab.NewCell();
    *out << cgicc::form().set("method","get").set("action", command_url).set("enctype","multipart/form-data") << std::endl;
    //if (fsm_.getCurrentState() == 'H'){
    if (!TTCciPtr_){
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "ReadCounters").set("disabled", "true");;
    }else{
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "ReadCounters");
    }
    *out << cgicc::form();
    // Reset Counters
    *out << cgicc::form().set("method","get").set("action", command_url).set("enctype","multipart/form-data") << std::endl;
    if ( !TTCciPtr_ || fsm_.getCurrentState() == 'E' ){
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "ResetCounters").set("disabled", "true");;
    }else{
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "ResetCounters");
    }
    *out << cgicc::form();  
    tab.Close();

    *out << "<p>" << std::endl;
  } 
  if (int(ReloadAtEveryConfigure_)==1){
    *out << RED<<"*) "<<SMALLER<<"Configuration will be reloaded from "
         << (ReadConfigFromFile_?"file":"conf. stream")
         <<" at every \"Configure\" state change."
         <<NOSMALLER<<NOCOL<<"<br>"<<endl;
  }


  {// General INFO
    *out << BOLD<<"General Info"<<NOBOLD<<"<br>"<<endl;
    *out << "Software: "<<TTCciPtr_->GetVersion()<<"<br>"<<endl;
    if (TTCciPtr_){
      *out << "Firmware: "<<TTCciPtr_->GetFirmwareVersionString()
           <<"<br>"<<endl;
    }
    *out << "TTCciLocation: "<<Location_;
    if (TTCciPtr_) *out<<" (Actual slot = " << TTCciPtr_->GetInfo().location() << ")";
    *out << "<br>"<<endl;
    *out << "BTimeCorrection: "<<int32_t(BTimeCorrection_)<<" BX, "<<endl; 
    *out << "DelayT2Correction: "<<int32_t(DelayT2Correction_)<<" BX"<<endl; 
  }

  
  {// Configration file display & change
    std::string confurl = "/";
    confurl += getApplicationDescriptor()->getURN();
    confurl += "/NewConfigurationFile";
  
    *out << cgicc::form().set("method","post")
      .set("action", confurl)
      .set("enctype","multipart/form-data") << std::endl;
  
    GetFileList();
    *out << cgicc::fieldset() << std::endl;
    *out << cgicc::legend("Configure TTCci FROM file") << std::endl;
    *out << cgicc::label("Current configuration source: ");
    if (ReadConfigFromFile_){
      *out <<"File ("<<MONO
           <<asciConfigurationFilePath_<< NOMONO<<")" << std::endl;
    }else{
      *out <<"NOT from file but directly from the xml input-string \"Configuration\""<<endl;
    }
    *out << "<br>" << endl;
    InputFileList.Write(*out);
    if (1 ){
      *out << cgicc::label("&nbsp;&nbsp;&nbsp;&nbsp;(enter absolute path): ") << std::endl;
      *out << cgicc::input().set("type","text")
        .set("name","ConfigurationFile")
        .set("size", "60").set("value","") << cgicc::p() << std::endl;
      *out << "<p>" << std::endl;
    }
    if (!TTCciPtr_){
      *out << SMALLER
           <<"N.B.: 'Submit' will only change path to the configuration file, the "
           <<"configuration itself (i.e. opening the file and reading in the "
           <<"configuration) will happen when requesting 'Configure' in the "
           <<"State Machine."<<NOSMALLER<<"<br>"<<endl;
    }else{
      *out << SMALLER
           <<"N.B.: Since the TTCci object has already been created, 'Submit' "
           <<"will change the path to the configuration file AND then load "
           <<"this new configuration."<<NOSMALLER<<"<br>"<<endl;
    }
    *out << cgicc::fieldset() << std::endl;
    if ( 0 && !TTCciPtr_ ){
      *out << cgicc::input().set("type", "submit")
        .set("name", "submit")
        .set("value", "Submit").set("disabled", "true");
    }else{
      *out << cgicc::input().set("type", "submit")
        .set("name", "submit")
        .set("value", "Submit");
    }
    *out << "<p>" << std::endl;
    *out << cgicc::form() << std::endl;

    // Write Configration to new file
    confurl = "/";
    confurl += getApplicationDescriptor()->getURN();
    confurl += "/WriteConfigurationFile";
  
    *out << cgicc::form().set("method","post")
      .set("action", confurl)
      .set("enctype","multipart/form-data") << std::endl;
  
    *out << cgicc::fieldset() << std::endl;
    char s[1024];
    gethostname(s, sizeof s);
    string machine = s;
    size_t pos = machine.find_first_of(".");
    if (pos<machine.size())
      machine.erase(machine.begin()+pos, machine.end());
    machine = string(" (on ")+MONO+machine+NOMONO+")";
    *out << cgicc::legend((string("Write current configuration TO file")+machine).c_str()) << std::endl;
    *out << cgicc::label("(Default) File path: ") <<MONO
         <<asciConfigurationFilePath_<<NOMONO
         << "<p>" << std::endl;
    *out << cgicc::label("New file (enter absolute path): ") 
         << std::endl;
    *out << cgicc::input().set("type","text")
      .set("name","ConfigurationFile")
      .set("size", "60").set("value",(ReadConfigFromFile_?asciConfigurationFilePath_:"")) << cgicc::p() << std::endl;
    *out << "<p>" << std::endl;
  
    *out << cgicc::input().set("type", "checkbox")
      .set("name","overwrite") << std::endl;
    *out << "force overwrite (if file exists)"<<endl; 

    *out << cgicc::fieldset() << std::endl;
    if ( !TTCciPtr_ ){
      *out << cgicc::input().set("type", "submit")
        .set("name", "submit")
        .set("value", "Write").set("disabled", "true");
    }else{
      *out << cgicc::input().set("type", "submit")
        .set("name", "submit")
        .set("value", "Write");
    }
    *out << "<p>" << std::endl;
    *out << cgicc::form() << std::endl;
  }

  WriteDebugTextArray(out, true);
  PrintHTMLFooter(out);
}


// Dispatcher function
void PixelTTCSupervisor::dispatch (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception)
{
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  cgicc::Cgicc cgi(in);
  //const cgicc::CgiEnvironment& env = cgi.getEnvironment();
  cgicc::const_form_iterator stateInputElement = cgi.getElement("StateInput");
  std::string stateInput = (*stateInputElement).getValue();
  SendSOAPMessageToPixelTTCSupervisor(stateInput);
  
  std::string url = LastPage_;

  //*out<<"<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL="<<url<<"\">" <<endl;
  *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL="<<url<<"\">" <<endl;
  
  //*out << stateInput << " sent ... "<<endl;
  *out << cgicc::h1(stateInput+" command sent ...")<<std::endl;
  RedirectMessage(out,url);
}

void PixelTTCSupervisor::RedirectMessage(xgi::Output * out, 
                                   const std::string &url){
  string myurl=url;
  if (myurl.length()==0){
    myurl = string("/")+getApplicationDescriptor()->getURN()+"/"+LastPage_;
  }
  *out << "<br>"<<BIGGER
       <<RED<<"NOTE:"<<NOCOL<<endl
       <<" If your browser (e.g. some safari versions) does "<<BOLD<<"not "
       <<NOBOLD<<"automatically redirect you to the previous page, "<<endl
       <<"<a href=\""<<myurl<<"\">click here</a>."<<NOBIGGER<<"<br>"<<endl;
}

// Dispatcher function
void PixelTTCSupervisor::NewConfiguration (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception)
{
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  std::string url = LastPage_;

  *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"4; URL="<<url<<"\">" <<endl;
    //*out << stateInput << " sent ... "<<endl;
  *out << cgicc::h1("New Configuration")<<std::endl;
  
  *out << RED <<cgicc::h1("FUNCTIONALITY NOT YET IMLPEMENTED!")
       <<NOCOL<<std::endl;

  try 
    {
      // Create a new Cgicc object containing all the CGI data
      cgicc::Cgicc cgi(in);

      // Get a pointer to the environment
      //const cgicc::CgiEnvironment& env = cgi.getEnvironment();

      //cgicc::const_form_iterator e;

      std::string parName = cgi["ParName"]->getValue();
      uint32_t myValue = cgi["Value"]->getIntegerValue(0);
		
      *out << "Parameter Name = "<< parName<<"<br>"<<endl;
      *out << "Value = "<< myValue << "<br> "<<endl;
      bool hasChecked = cgi.queryCheckbox("boxchecked");
      if (hasChecked){
        *out << "Checkbox was checked."<<endl;
      }else{
        *out << "But check box was not checked!" <<endl;
      }
      
    }
  catch(const std::exception& e) 
    {
      *out << "<pre>" << e.what() << "</pre>" << std::endl;

    }
  RedirectMessage(out,url);
}

// Read from / Write to TTCci Adresses
void PixelTTCSupervisor::ReadWriteParameters (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception)
{ 
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  std::string url = "/";
  url += getApplicationDescriptor()->getURN()+"/"+LastPage_;

  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  // *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  *out << cgicc::title("TTCci \""+name_.toString()+"\"") << std::endl;

  //*out<<"<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"4; URL="<<url<<"\">" <<endl;

  PrintHTMLHeader(out);

  *out << cgicc::h1("Read/Write TTCci Parameters")<<std::endl;

  //*out << RED <<cgicc::h1("FUNCTIONALITY NOT YET IMLPEMENTED!")
  //    <<NOCOL<<std::endl;

  try 
    {
      // Create a new Cgicc object containing all the CGI data
      cgicc::Cgicc cgi(in);

      // Get a pointer to the environment
      //const cgicc::CgiEnvironment& env = cgi.getEnvironment();

      //cgicc::const_form_iterator e;

      const std::string parName = cgi["ParName"]->getValue();
      const uint32_t myValue = std::strtol(cgi["Value"]->getValue().c_str(), 0, 0);
		
      *out << "Parameter Name = '"<<MONO<< parName<<NOMONO<<"'<br>"<<endl;
      *out << "Value = "<<MONO<< myValue <<NOMONO<< "<br> "<<endl;
      bool read = cgi.queryCheckbox("readchecked");
      bool write = cgi.queryCheckbox("writechecked");

      *out << "Read "<< (read?"checked":"unchecked")<<".<br>"<<endl;
      *out << "Write "<< (read?"checked":"unchecked")<<".<br>"<<endl;

      if (read || write) *out << "<ul>"<<endl;
      if (read){
        *out << "<li>"<<endl;
        *out << "Reading from VME address with name '"<<MONO<<parName
             <<NOMONO<<"': <br>"<<endl;
        uint32_t output = TTCciPtr_->Read(parName);
        *out << MONO<<BLUE<<"0x"<<hex<<output<<dec<<" = "<<output
             <<NOCOL<<NOMONO<<endl;
        *out << "</li>"<<endl;
      }
      if (write){
        *out << "<li>"<<endl;
        *out << "Writing 0x"<<hex<<myValue<<dec<<" = "<<myValue
             <<" to VME address with name '"<<MONO<<parName<<NOMONO
             <<"'"<<endl;
        TTCciPtr_->Write(parName,myValue);
        *out << "</li>"<<endl;
        if (read){
        *out << "<li>"<<endl;
          *out << "Reading (again) from VME address with name '"
               <<MONO<<parName<<NOMONO
               <<"': <br> "<<endl;
          uint32_t output = TTCciPtr_->Read(parName);
          *out << MONO<<BLUE<<"0x"<<hex<<output<<dec<<" = "<<output
               <<NOMONO<<NOCOL<<endl;
        *out << "</li>"<<endl;
        }
      }
      if (read || write) *out << "</ul>"<<endl;
      
    }
  catch(const std::exception& e) 
    {
      *out << "<pre>" << e.what() << "</pre>" << std::endl;

    }
  WriteDebugTextArray(out);
  PrintHTMLFooter(out);
}

// Debug Test (to be removed later... )
void PixelTTCSupervisor::DebugTest (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception)
{ 
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  std::string url = "/";
  url += getApplicationDescriptor()->getURN()+"/"+LastPage_;

  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  //*out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  *out << cgicc::title("TTCci \""+name_.toString()+"\"") << std::endl;

  //*out<<"<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"4; URL="<<url<<"\">" <<endl;

  PrintHTMLHeader(out);

  *out << cgicc::h1("Debug Test")<<std::endl;

  //*out << RED <<cgicc::h1("FUNCTIONALITY NOT YET IMLPEMENTED!")
  //    <<NOCOL<<std::endl;

  try 
    {
      // Create a new Cgicc object containing all the CGI data
      cgicc::Cgicc cgi(in);

      // Get a pointer to the environment
      //const cgicc::CgiEnvironment& env = cgi.getEnvironment();

      //cgicc::const_form_iterator e;

      const std::string parName = cgi["ParName"]->getValue();
      const uint32_t myValue = std::strtol(cgi["Value"]->getValue().c_str(), 0, 0);
		
      *out << "Parameter Name = '"<<MONO<< parName<<NOMONO<<"'<br>"<<endl;
      *out << "Value = "<<MONO<< myValue <<NOMONO<< "<br> "<<endl;
      bool read = cgi.queryCheckbox("readchecked");
      bool write = cgi.queryCheckbox("writechecked");

      cgicc::const_form_iterator radio1 = cgi.getElement("debugchecked");
      cout << "Name = "<<radio1->getName()<<endl
           << "Value = "<<radio1->getValue()
           <<endl;

      *out << "Read "<< (read?"checked":"unchecked")<<".<br>"<<endl;
      *out << "Write "<< (read?"checked":"unchecked")<<".<br>"<<endl;

      cgicc::const_form_iterator list1 = cgi.getElement("cars");
      cout << "List Name = "<<list1->getName()<<endl
           << "List Value = "<<list1->getValue()
           <<endl;
      

      if (read || write) *out << "<ul>"<<endl;
      if (read){
        *out << "<li>"<<endl;
        *out << "Reading from VME address with name '"<<MONO<<parName
             <<NOMONO<<"': <br>"<<endl;
        uint32_t output = TTCciPtr_->Read(parName);
        *out << MONO<<BLUE<<"0x"<<hex<<output<<dec<<" = "<<output
             <<NOCOL<<NOMONO<<endl;
        *out << "</li>"<<endl;
      }
      if (write){
        *out << "<li>"<<endl;
        *out << "Writing 0x"<<hex<<myValue<<dec<<" = "<<myValue
             <<" to VME address with name '"<<MONO<<parName<<NOMONO
             <<"'"<<endl;
        TTCciPtr_->Write(parName,myValue);
        *out << "</li>"<<endl;
        if (read){
        *out << "<li>"<<endl;
          *out << "Reading (again) from VME address with name '"
               <<MONO<<parName<<NOMONO
               <<"': <br> "<<endl;
          uint32_t output = TTCciPtr_->Read(parName);
          *out << MONO<<BLUE<<"0x"<<hex<<output<<dec<<" = "<<output
               <<NOMONO<<NOCOL<<endl;
        *out << "</li>"<<endl;
        }
      }
      if (read || write) *out << "</ul>"<<endl;
      
    }
  catch(const std::exception& e) 
    {
      *out << "<pre>" << e.what() << "</pre>" << std::endl;

    }
  WriteDebugTextArray(out);
  PrintHTMLFooter(out);
}

void PixelTTCSupervisor::NewConfigurationFile (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception)
{ 
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  cout << "Called fctn. PixelTTCSupervisor::NewConfigurationFile() ..."<<endl;
  std::string url = LastPage_;

  *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"1; URL="<<url<<"\">" <<endl;
  *out << cgicc::h1("Configuring TTCci from File ...")<<std::endl;

  try 
    {
      // Create a new Cgicc object containing all the CGI data
      cgicc::Cgicc cgi(in);

      // Get a pointer to the environment
      //const cgicc::CgiEnvironment& env = cgi.getEnvironment();

      //cgicc::const_form_iterator e;

      std::string fileName = cgi["ConfigurationFile"]->getValue();
      //uint32_t myValue = cgi["Value"]->getIntegerValue(0);
      //cgicc::const_file_iterator file=cgi.getFile("ConfigurationFile");
		
      std::string file_radiobutton = cgi["inputfiles"]->getValue();

      string oldpath = asciConfigurationFilePath_;
      bool oldbool = ReadConfigFromFile_;
      if (file_radiobutton == "other"){
        if (fileName.size()>0){
          asciConfigurationFilePath_ = fileName;
          ReadConfigFromFile_=true;
          *out << "New path to configuration file = "
               << asciConfigurationFilePath_ <<"<br>"<<endl;
        }
      }else{
        asciConfigurationFilePath_ = file_radiobutton;
        ReadConfigFromFile_=true;
        *out << "New path to configuration file = "
             << asciConfigurationFilePath_ <<"<br>"<<endl;
      }
      FILE *fp=fopen(asciConfigurationFilePath_.c_str(),"r");
      if (fp==NULL){
        *out << "<big style=\"font-weight: bold;\">"<<RED;
        *out << "<br> ERROR! <br> Unable to open file! File may not exist at this location! <br>"<<endl;
        *out << NOCOL<<"</big>";
        any_errors_ = 1;
        err_message_ << RED << BOLD <<"ERROR:"<<NOBOLD
                      <<" Unable to open file \""<<MONO
                      <<asciConfigurationFilePath_<<NOMONO
                      <<"\"!"<<NOCOL<<"<br>"<<endl;
        err_message_ <<RED<<"&nbsp;&nbsp;&nbsp; (check path!)"
                      <<NOCOL<<"<br>"<<endl;     
        ReadConfigFromFile_=oldbool;
        asciConfigurationFilePath_ = oldpath;
      }else{
        fclose (fp);
        if (TTCciPtr_) {
          ConfigureTTCcifromASCI();
        }
      }
      cout << "done."<<endl;
    }
  catch(const std::exception& e) 
    {
      *out << "<pre>" << e.what() << "</pre>" << std::endl;

    }
  RedirectMessage(out,url);
}

void PixelTTCSupervisor::WriteConfigurationFile (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception)
{ 
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  cout << "Called fctn. PixelTTCSupervisor::WriteConfigurationFile() ..."<<endl;
  std::string url = LastPage_;

  *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"2; URL="<<url<<"\">" <<endl;
  *out << cgicc::h1("Writing Configuration to File ...")<<std::endl;

  try 
    {
      // Create a new Cgicc object containing all the CGI data
      cgicc::Cgicc cgi(in);

      // Get a pointer to the environment
      //const cgicc::CgiEnvironment& env = cgi.getEnvironment();

      //cgicc::const_form_iterator e;

      std::string fileName = cgi["ConfigurationFile"]->getValue();
      //uint32_t myValue = cgi["Value"]->getIntegerValue(0);
      //cgicc::const_file_iterator file=cgi.getFile("ConfigurationFile");
		
      if (fileName.size()>0){
        asciConfigurationFilePath_ = fileName;
        ReadConfigFromFile_=true;
        html << "Path to new Configuration file = "
              << asciConfigurationFilePath_ <<endl;
        *out << "Path to new Configuration file = "
              << asciConfigurationFilePath_ <<"<br>"<<endl;
      }
      
      bool overwrite = cgi.queryCheckbox("overwrite");
      if (!overwrite){
        FILE* fp=fopen(asciConfigurationFilePath_.c_str(),"r");
        if (fp!=NULL){
          *out << RED << "FILE '"<<asciConfigurationFilePath_
               <<"' already exists, but 'allow overwriting' was not checked!"
               <<NOCOL<<"<br>"<<endl;
          html<< "FILE '"<<asciConfigurationFilePath_
               <<"' already exists, but 'allow replacing' was not checked!"
               <<endl;
          return;
        }
      }
      
      ofstream oo(asciConfigurationFilePath_.c_str(), 
                   ios_base::out);
      if (!oo){
        html << "   Unable to open file '"
             <<asciConfigurationFilePath_<<"'!"<<endl;
        *out << "   Unable to open file '"
             <<asciConfigurationFilePath_<<"'!<br>"<<endl;
        return;
      }
      TTCciPtr_->WriteConfiguration(oo,
                                    string("(from PixelTTCSupervisor::")+
                                    string("WriteConfigurationFile())"));
      html << "Configuration written to file '"
            << asciConfigurationFilePath_<<"'."<<endl;
    }
  catch(const std::exception& e) 
    {
      *out << "<pre>" << e.what() << "</pre>" << std::endl;

    }
  RedirectMessage(out,url);
}


// The TTCci Main Configuration Page
void PixelTTCSupervisor::MainConfiguration(xgi::Input * /*in*/, xgi::Output * out ) throw (xgi::exception::Exception){
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  LastPage_ = "MainConfiguration";
  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  //*out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  *out << cgicc::title("TTCci \""+name_.toString()+"\"") << std::endl;

  PrintHTMLHeader(out);

  *out << cgicc::h1(toolbox::toString("TTCci Main Configuration (%s)",
                                      GetName(1).c_str()))
       <<std::endl;

    ErrorStatement(out);

  if (!TTCciPtr_) {
    *out << RED<<BIGGER<<"TTCci Object not yet created.<br>"
         <<NOBIGGER<<NOCOL<<endl; 
    *out << "--> Go to the <a href=\"/"<<getApplicationDescriptor()->getURN()
         <<"/\">Main Page (TTCci Control)</a> and create the TTCci object "
         <<"(or 'Configure') first.<br>"<<endl;
    //return;
  }


  *out << "State Machine: ";
  PrintSMState(out);
  *out << cgicc::p()<<cgicc::p()<<endl;

  std::string command_url = "/";
  command_url += getApplicationDescriptor()->getURN();
  command_url += "/MainConfigCommand";	
  
  if (TTCciPtr_){
    *out << cgicc::form().set("method","post")
      .set("action", command_url)
      .set("enctype","multipart/form-data") << std::endl;
    *out << cgicc::fieldset() << std::endl;
    {
      stringstream my; 
      my<<"Input Sources (0x"
        <<hex<<TTCciPtr_->Read(TTCciAdd::INSELECT,"(INSELECT)")<<dec
        <<") & QPLL";
      *out << cgicc::legend(my.str().c_str()) << std::endl;
    }

  
    // Table
    HTMLTable tab(*out,0,2,2,"","",100);
    tab.SetCellStyle("top");
    tab.NewCell();
  
    string sel = UNDERL, unsel = NOUNDERL;
    *out << ""<<BIGGER<<BOLD<<"Clock Source: "<<NOBOLD<<NOBIGGER << endl;
    string dums=INAME(TTCciPtr_->CheckClock());
    ClockSourceList.SetDefault(dums);
    ClockSourceList.Write(*out);
    *out << " ("<<MONO<<dums<<NOMONO<<")";
    *out << "<br>"<<endl;
  
    *out << BIGGER<<"QPLLCTRL Bit 5"<<NOBIGGER
         << SMALLER<<" (if external clock)"<<NOSMALLER
         <<BIGGER<<":<br> "<<NOBIGGER<<endl;
    if (TTCciPtr_->Is_ResetQPLL(true) &&
        TTCciPtr_->CheckClock()!=INTERNAL){
      *out << cgicc::input().set("type", "radio")
        .set("name","QPLLReset")
        .set("value","YES").set("checked","checked");
      *out << sel<<"Reset (0)"<<unsel;
      *out << cgicc::input().set("type", "radio")
        .set("name","QPLLReset")
        .set("value","NO");
      *out << "No Reset (1)"<<endl;
    }else{
      *out << cgicc::input().set("type", "radio")
        .set("name","QPLLReset")
        .set("value","YES");
      *out << "Reset (0)";
      *out << cgicc::input().set("type", "radio")
        .set("name","QPLLReset")
        .set("value","NO").set("checked","checked");
      *out << sel<<"No Reset (1)"<<unsel<<endl;
    }
    *out << "<br>"<<endl;

    *out << BIGGER<<"QPLLCTRL Bit 4"<<NOBIGGER
         << SMALLER<<" (if external clock)"<<NOSMALLER
         << BIGGER<<":<br> "<<NOBIGGER<<endl;
    if (TTCciPtr_->CheckClock()!=INTERNAL && 
        TTCciPtr_->Is_AutoRestartQPLL(true) ){
      *out << GREEN<<"AutoRestart"<<NOCOL<<": "<<endl;
      *out << cgicc::input().set("type", "radio")
        .set("name","QPLLAutoRestart")
        .set("value","ON").set("checked","checked");
      *out << sel<<"ON (1)"<<unsel;
      *out << cgicc::input().set("type", "radio")
        .set("name","QPLLAutoRestart")
        .set("value","OFF");
      *out << "OFF (0)"<<endl;
    }else{
      *out << RED<<"AutoRestart"<<NOCOL<<": "<<endl;
      *out << cgicc::input().set("type", "radio")
        .set("name","QPLLAutoRestart")
        .set("value","ON");
      *out << "ON (1) ";
      *out << cgicc::input().set("type", "radio")
        .set("name","QPLLAutoRestart")
        .set("value","OFF").set("checked","checked");
      *out << sel<<"OFF (0)"<<unsel<<endl;
    }
    *out << "<br>"<<endl;
    {// QPLL Frequency bits
      //char dum[30]; 
      //sprintf(dum,"%lx",TTCciPtr_->GetQPLLFrequencyBits(false,(ClockSourceList.GetDefault()!=INAME(ttc::INTERNAL))));
      *out << "QPLL Freq. bits: <br>"<<MONO<<"0x"<<NOMONO;
      *out << cgicc::input().set("type","text")
        .set("name","freqbits")
        .set("size","3")
        .set("value",Long2HexString(TTCciPtr_->GetQPLLFrequencyBits(false,(ClockSourceList.GetDefault()!=INAME(ttc::INTERNAL)))).c_str())
           << SMALLER<<" (6 bits for INTERNAL clock, 4 otherwise)"<<NOSMALLER
           << std::endl;
    }
    *out << "<br><br>"<<BOLD<<"BGO Source<br>" << NOBOLD<<endl;
    vector<ttc::ExternalInterface> itf = TTCciPtr_->CheckBGOSource();
    BGOSourceList_other.UnCheckAll();
    if (find(itf.begin(), itf.end(), ttc::CTC)!=itf.end())
      BGOSourceList.SetDefault(INAME(ttc::CTC));
    else if (find(itf.begin(), itf.end(), ttc::LTCIN)!=itf.end())
      BGOSourceList.SetDefault(INAME(ttc::LTCIN));
    else {
      BGOSourceList.SetDefault("other");
      BGOSourceList_other.UnCheckAll();
      for (size_t i=0; i<itf.size(); ++i){
        BGOSourceList_other.Check(INAME(itf[i]));
      }
    }
    BGOSourceList.Write(*out);
    HTMLTable tab2(*out,0,2,0,"");
    tab2.NewCell();
    *out << "&nbsp;&nbsp;&nbsp;&nbsp&nbsp;&nbsp;&nbsp;&nbsp"<<endl;
    tab2.NewCell();
    BGOSourceList_other.Write(*out);
    tab2.Close();
  
    // next table cell:
    tab.NewCell();
  
    *out << BOLD<<"Orbit Source" <<NOBOLD<< endl;
    OrbitSourceList.SetDefault((dums=INAME(TTCciPtr_->CheckOrbitSource())));
    OrbitSourceList.Write(*out);
    *out << " ("<<MONO<<dums<<NOMONO<<")<br>";

    *out << "<br>"<<endl;
    *out << BOLD<<"Trigger Source (";
    if (TTCciPtr_->IsL1AEnabled())
      *out << GREEN << "Enabled"<< NOCOL;
    else
      *out << RED << "Disabled"<< NOCOL;
    *out << ")<br>" <<NOBOLD<< endl;
    itf = TTCciPtr_->CheckTrigger();
    TriggerSourceList_other.UnCheckAll();
    if (find(itf.begin(), itf.end(), ttc::CTC)!=itf.end())
      TriggerSourceList.SetDefault(INAME(ttc::CTC));
    else if (find(itf.begin(), itf.end(), ttc::LTCIN)!=itf.end())
      TriggerSourceList.SetDefault(INAME(ttc::LTCIN));
    else {
      TriggerSourceList.SetDefault("other");
      for (size_t i=0; i<itf.size(); ++i){
        TriggerSourceList_other.Check(INAME(itf[i]));
      }
    }
    TriggerSourceList.Write(*out);
    tab2.Open();
    tab2.NewCell();
    tab2.NewCell();
    *out << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"<<endl;
    tab2.NewCell();
    TriggerSourceList_other.Write(*out);
  
    if (TTCciPtr_->CanSetInternalTriggerFrequency()){// Trigger Frequency
      *out << cgicc::label("&nbsp;&nbsp;&nbsp;&nbsp;Freq.: ");
      char freq[20];
      double trigfreq = TTCciPtr_->GetInternalTriggerFrequency();
      if ( trigfreq<0 ) sprintf(freq,"1");
      else if (trigfreq == double(int(trigfreq))) sprintf(freq,"%.0f",trigfreq);
      else sprintf(freq,"%.2f",trigfreq);
      *out << cgicc::input().set("type","text")
        .set("name","TriggerFrequency")
        .set("size","6")
        .set("value",string(freq)) <<  " Hz\n"<<endl;
        if (TTCciPtr_->GetInternalTriggerRandom()) {
          *out << cgicc::input().set("type", "checkbox").set("name", "TriggerRandom").set("checked","checked")
               << GREEN << " Random" << NOCOL << endl;
        }else{
          *out << cgicc::input().set("type", "checkbox").set("name", "TriggerRandom")
               << " Random" << endl;
        }
        *out << "<br>&nbsp;&nbsp;&nbsp;&nbsp;"
             << SMALLER<<"(if Trigger Source = "<<MONO<<"INTERNAL"<<NOMONO
             << ")"<<NOSMALLER<< std::endl;
    }else{
      *out << GREY<<"&nbsp;&nbsp;&nbsp;&nbsp;Freq.: "
           <<"not applicable.<br>";
      *out << "&nbsp;&nbsp;&nbsp;&nbsp;(\""
           <<MONO<<"TRIGGER_INTERVAL x"<<NOMONO<<"\" in config. file)"
           <<NOCOL<<endl;
    }
    tab2.Close();
    //*out << "</td></tr></tbody></table>"<<endl;
    // End of table:
    tab.Close();
 
    *out << cgicc::fieldset() << std::endl;// End Fieldset 
    if ( TTCciPtr_ ){
      *out << cgicc::input().set("type", "submit")
        .set("name", "submit")
        .set("value", "Apply");
    }else{
      *out << cgicc::input().set("type", "submit")
        .set("name", "submit")
        .set("value", "Apply").set("disabled", "true");
    }
    *out << cgicc::form() << std::endl;
  }

  if (TTCciPtr_){
    // TRIGGER RULES:
    std::string myurl = "/";
    myurl += getApplicationDescriptor()->getURN();
    myurl += "/TriggerRules";	
    *out << cgicc::form().set("method","post")
      .set("action", myurl)
      .set("enctype","multipart/form-data") << std::endl;
    *out << cgicc::fieldset() << std::endl;
    *out << cgicc::legend("Trigger Rules (apply to front-panel triggers only!)") 
         << std::endl;
    for (size_t i=TTCciPtr_->FirstTriggerRule(); 
         i<TTCciPtr_->TriggerRuleSize(); ++i){
      if (i>TTCciPtr_->FirstTriggerRule()) *out << "<br> ";
      *out << "No more than "<<i<<" triggers in ";
      //char dum1[50], dum2[20];
      //sprintf(dum1,"trigrule  %u",i);
      //sprintf(dum2,"%lu",TTCciPtr_->GetTriggerRule(i));
      *out << cgicc::input().set("type","text")
        .set("name",("trigrule  "+UnsignedLong2String(i)).c_str())
        .set("size","6")
        .set("value",UnsignedLong2String(TTCciPtr_->GetTriggerRule(i)).c_str())<<endl;      
      *out << " consecutive BXs"<<endl;
    }
    *out << cgicc::fieldset() << std::endl;// End Fieldset 
    if ( TTCciPtr_ ){
      *out << cgicc::input().set("type", "submit")
        .set("name", "submit")
        .set("value", "Apply");
    }else{
      *out << cgicc::input().set("type", "submit")
        .set("name", "submit")
        .set("value", "Apply").set("disabled", "true");
    }
    *out << cgicc::form() << std::endl;
  }

  WriteDebugTextArray(out);
  PrintHTMLFooter(out);

}

// The TTCci Register Access
void PixelTTCSupervisor::RegisterAccess(xgi::Input * /*in*/, xgi::Output * out ) throw (xgi::exception::Exception){
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  LastPage_ = "RegisterAccess";
  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  //*out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  *out << cgicc::title("TTCci \""+name_.toString()+"\"") << std::endl;

  PrintHTMLHeader(out);

  *out << cgicc::h1(toolbox::toString("TTCci Register Access (for Debugging) (%s)",
                                      GetName(1).c_str()))<<std::endl;

  ErrorStatement(out);

  if (!TTCciPtr_) {
    *out << RED<<BIGGER<<"TTCci Object not yet created. <br>"
         <<NOBIGGER<<NOCOL<<endl; 
    *out << "--> Go to the <a href=\"/"<<getApplicationDescriptor()->getURN()
         <<"/\">Main Page (TTCci Control)</a> and create the TTCci object "
         <<"(or 'configure') first.<br>"<<endl;
    return;
  }
  

  *out << "State Machine: ";
  PrintSMState(out);
  *out << cgicc::p()<<cgicc::p()<<endl;

  {
    HTMLTable tab(*out,0,2,2,"","",100);
    tab.NewCell();
    // DumpVME commands
    std::string cmd_url = "/";
    cmd_url += getApplicationDescriptor()->getURN();
    cmd_url += "/Command";	
    *out << cgicc::form().set("method","get").set("action", cmd_url).set("enctype","multipart/form-data") << std::endl;
    if ( !TTCciPtr_ ){
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "DumpVMEHistory-All").set("disabled", "true");
    }else{
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "DumpVMEHistory-All");
    }
    *out << cgicc::form();       
    //
    *out << cgicc::form().set("method","get").set("action", cmd_url).set("enctype","multipart/form-data") << std::endl;
    if ( !TTCciPtr_ ){
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "DumpVMEHistory-WriteOnly").set("disabled", "true");
    }else{
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "DumpVMEHistory-WriteOnly");
    }
    *out << cgicc::form(); 
    tab.NewCell();
    {// THE VMEDATS/L stuff
      string sel = UNDERL, unsel = NOUNDERL;
      *out << cgicc::form().set("method","post")
        .set("action", string("/"+getApplicationDescriptor()->getURN()
                              +"/VMEBData"))
        .set("enctype","multipart/form-data") << std::endl;
      *out << cgicc::fieldset() << std::endl;
      *out << cgicc::legend("Send B-Data directly through VME") << std::endl;
      //*out << BOLD<<"VME BDATA (for debugging):"<<NOBOLD<<"<br>"<<endl;
      *out << "BData "<<MONO<<"0x"<<NOMONO<<endl;
      //char cdebugbdata[30]; sprintf(cdebugbdata,"%lx",_debugbdata);
      *out << cgicc::input().set("type","text")
        .set("name","vmebdata")
        .set("size","4")
        .set("value",Long2HexString(_debugbdata).c_str()) <<"  "<<std::endl;
     if ( _debugb_short ){
        *out << cgicc::input().set("type", "radio")
          .set("name","debugshort")
          .set("value","yes").set("checked","checked");
        *out << sel<<"SHORT"<<unsel<<" ";
        *out << cgicc::input().set("type", "radio")
          .set("name","debugshort")
          .set("value","no");
        *out << "LONG ";
      }else{
        *out << cgicc::input().set("type", "radio")
          .set("name","debugshort")
          .set("value","yes");
        *out << "SHORT"<<" ";
        *out << cgicc::input().set("type", "radio")
          .set("name","debugshort")
          .set("value","no").set("checked","checked");
        *out << sel<<"LONG "<<unsel;
      }
      *out << cgicc::fieldset() << std::endl;// End Fieldset 
      if ( TTCciPtr_ ){
        *out << cgicc::input().set("type", "submit")
          .set("name", "submit")
          .set("value", "Send");
      }else{
        *out << cgicc::input().set("type", "submit")
          .set("name", "submit")
          .set("value", "Send").set("disabled", "true");
      }
      *out << cgicc::form() << std::endl;
    }    
    tab.Close();
  }



  std::string command_url = "/";
  command_url += getApplicationDescriptor()->getURN();
  command_url += "/RegisterAccessCommand";	

  if (!TTCciPtr_){
    return;
  }
  
  *out << cgicc::form().set("method","post")
    .set("action", command_url)
    .set("enctype","multipart/form-data") << std::endl;
  *out << cgicc::fieldset() << std::endl;
  *out << cgicc::legend("Read from or write to TTCci Registers") << std::endl;

  //*out << BOLD<<"Addresses"<<NOBOLD<<"<br>"<<endl;
  *out << cgicc::label("Address Name ") << std::endl;
  VMEAddrList1.Write(*out);

  *out << cgicc::label("Index ");
  *out << cgicc::input().set("type","text")
    .set("name","index")
    .set("size","4")
    .set("value","0") <<  cgicc::p() << std::endl;

  *out << "Value "<<MONO<<"0x"<<NOMONO<<endl;
  *out << cgicc::input().set("type","text")
    .set("name","Value")
    .set("size","4")
    .set("value","0") <<  cgicc::p() << std::endl;
  *out << std::endl; 

  *out << cgicc::label("Read ");
  *out << cgicc::input().set("type", "checkbox")
    .set("name","readchecked") << std::endl;
  *out << cgicc::label("Write ");
  *out << cgicc::input().set("type", "checkbox")
    .set("name","writechecked") << std::endl;
   
  *out << cgicc::fieldset() << std::endl;// End Fieldset 
  if ( TTCciPtr_ ){
    *out << cgicc::input().set("type", "submit")
      .set("name", "submit")
      .set("value", "Apply");
  }else{
    *out << cgicc::input().set("type", "submit")
      .set("name", "submit")
      .set("value", "Apply").set("disabled", "true");
  }
  *out << cgicc::form() << std::endl;
  

  WriteDebugTextArray(out);
  PrintHTMLFooter(out);

}

// The TTCci Register Access
void PixelTTCSupervisor::Sequences(xgi::Input * /*in*/, xgi::Output * out ) throw (xgi::exception::Exception){
  LastPage_ = "Sequences";
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  //*out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  *out << cgicc::title("TTCci \""+name_.toString()+"\"") << std::endl;

  PrintHTMLHeader(out);

  *out << cgicc::h1(toolbox::toString("TTCci Sequences (%s)",
                                      GetName(1).c_str()))<<std::endl;

  ErrorStatement(out);

  if (!TTCciPtr_) {
    *out << RED<<BIGGER<<"TTCci Object not yet created. <br>"
         <<NOBIGGER<<NOCOL<<endl; 
    *out << "--> Go to the <a href=\"/"<<getApplicationDescriptor()->getURN()
         <<"/\">Main Page (TTCci Control)</a> and create the TTCci object "
         <<"(or 'configure') first.<br>"<<endl;
    return;
  }
  

  *out << "State Machine: ";
  PrintSMState(out);
  *out << cgicc::p()<<cgicc::p()<<endl;

  
  string SeqTitle = SequenceSelector.GetDefaultTitle();
  const vector<string> newseqvalues = TTCciPtr_->GetSequenceNames();
  SequenceSelector.Set(HTMLFieldElement::DROPDOWNMENU, 0,
                       "sequenceselect", newseqvalues, newseqvalues);
  if (!SequenceSelector.HasValue(SeqTitle))
    SeqTitle = newseqvalues.size() ? newseqvalues[0] : "";
  SequenceSelector.SetDefault(SeqTitle);
  

  *out << BIGGER<<BOLD<<"TTCci Sequence \""
       << BLUE<<SeqTitle<<NOCOL <<"\""<<NOBOLD<<NOBIGGER
       <<"<br>"<<std::endl;
  {
    
    // The First Table with 2 cells:
    HTMLTable tab(*out,0,0,0,"","",100);
    tab.NewCell();

    // Select Sequence to be displayed:
    std::string select_url = "/";
    select_url += getApplicationDescriptor()->getURN();
    select_url += "/SequenceSelectCommand";	
    *out << cgicc::form().set("method","post")
      .set("action", select_url)
      .set("enctype","multipart/form-data") << std::endl;
    *out << cgicc::fieldset() << std::endl;// Begin Fieldset 
    *out << cgicc::legend(SeqTitle + " Sequence") << std::endl;
    *out << "Select sequence: "<<endl;
    SequenceSelector.Write(*out);
    vector<string> seqchce, seqchce_tit;
    seqchce.push_back("display");seqchce_tit.push_back("display");
    seqchce.push_back("remove"); seqchce_tit.push_back("remove");
    seqchce.push_back("add"); seqchce_tit.push_back("add new:");
    HTMLFieldElement seqchoice(HTMLFieldElement::RADIOBUTTON, 0, 
                               "seqchoice",
                               seqchce,seqchce_tit /*,"vertical green"*/);
    seqchoice.Write(*out);
    *out << " " << cgicc::input().set("type","text")
      .set("name","newsequence")
      .set("size","10")
      .set("value","")<<std::endl;

    
    *out << "<p>"<<endl;
    *out << cgicc::fieldset() << std::endl;// End Fieldset 
    if ( TTCciPtr_ ){
      *out << cgicc::input().set("type", "submit")
        .set("name", "submit")
        .set("value", "Select");
    }else{
      *out << cgicc::input().set("type", "submit")
        .set("name", "submit")
        .set("value", "Select").set("disabled", "true");
    }
    *out << cgicc::form() << std::endl;
 
    tab.NewCell();
    {// Execute the "User" Sequence:
      std::string cmd_url = "/";
      cmd_url += getApplicationDescriptor()->getURN();
      cmd_url += "/Command";	
      *out << cgicc::form().set("method","get").set("action", cmd_url).set("enctype","multipart/form-data") << std::endl;      
      if ( !TTCciPtr_ ){
        *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", string(string("Exec. ")+SequenceSelector.GetDefaultTitle()+string(" Sequence")).c_str()).set("disabled", "true");
      }else{
        *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", string(string("Exec. ")+SequenceSelector.GetDefaultTitle()+string(" Sequence")).c_str());
      }
      *out << cgicc::form();       
    }
    tab.Close();
  }

  const Sequence* myseq = TTCciPtr_->GetSequence(SeqTitle);
  if (!myseq){
    *out << RED<<"ERROR! No sequence found with name '"
         <<SeqTitle<<"'<br>"<<NOCOL<<endl;
    return;
  }
  { // Edit current sequence
    std::string select_url = "/";
    select_url += getApplicationDescriptor()->getURN();
    select_url += "/SequenceEditCommand";	
    *out << cgicc::form().set("method","post")
      .set("action", select_url)
      .set("enctype","multipart/form-data") << std::endl;
    *out << cgicc::fieldset() << std::endl;// Begin Fieldset 
    *out << cgicc::legend("Edit "+SeqTitle+" Sequence") 
         << std::endl;
    // The Edit Table with 2 cells:
    HTMLTable tab(*out,1,1,0,"","",100);
    tab.NewCell();
    // Show the currecnt content of this sequence as list of radio buttons:
    vector<string> seqsel = myseq->Get();
    seqsel.push_back("< Append >");
    HTMLFieldElement sequence(HTMLFieldElement::RADIOBUTTON, 0, 
                              "sequenceline",seqsel,seqsel,"vertical green");
    sequence.SetDefault(seqsel[seqsel.size()-1]);
    if (seqsel.size()<=1) *out << "empty sequence.<br>"<<endl;
    sequence.Write(*out);

    tab.NewCell(); // 2nd cell

    *out <<"Edit action:<br>"<<endl;
    vector<string> edit, edit_tit;
    edit.push_back("new"); edit_tit.push_back("New line (insert above)");
    edit.push_back("modify"); edit_tit.push_back("Modify line");
    edit.push_back("delete"); edit_tit.push_back("Delete line");
    HTMLFieldElement edittab(HTMLFieldElement::RADIOBUTTON, 0, 
                             "edit",edit,edit_tit,"vertical");
    edittab.SetDefault(edit[0]);
    edittab.Write(*out);
    *out << endl;
    *out << "Command: "<<endl;
    vector<string> edit2, edit2_tit;
    edit2.push_back("Choose"); edit2_tit.push_back("< Choose >");
    edit2.push_back("EnableL1A"); edit2_tit.push_back("EnableL1A");
    edit2.push_back("DisableL1A"); edit2_tit.push_back("DisableL1A");
    edit2.push_back("Sleep"); edit2_tit.push_back("Sleep x seconds");
    edit2.push_back("mSleep"); edit2_tit.push_back("mSleep x millissec.");
    edit2.push_back("uSleep"); edit2_tit.push_back("uSleep x microsec.");
    edit2.push_back("BGO"); edit2_tit.push_back("BGO x (x=Channel)");
    edit2.push_back("ResetCounters"); edit2_tit.push_back("ResetCounters");
    edit2.push_back("BGOSource LTC"); edit2_tit.push_back("Set BGO-source to LTC");
    edit2.push_back("BGOSource CTC"); edit2_tit.push_back("Set BGO-source to CTC");
    edit2.push_back("BGOSource VME"); edit2_tit.push_back("Set BGO-source to VME (i.e. OFF)");
    edit2.push_back("BGOSource CYCLIC"); edit2_tit.push_back("Set BGO-source to CYCLIC");
    edit2.push_back("Periodic On"); edit2_tit.push_back("Periodic On");
    edit2.push_back("Periodic Off"); edit2_tit.push_back("Periodic Off");
    edit2.push_back("Periodic"); edit2_tit.push_back("Periodic interval x [sec]");
    edit2.push_back("ResumeCyclicAtStart yes"); edit2_tit.push_back("ResumeCyclicAtStart yes");
    edit2.push_back("ResumeCyclicAtStart no"); edit2_tit.push_back("ResumeCyclicAtStart no");
    edit2.push_back("SendShortBDATA"); edit2_tit.push_back("SendShortBDATA x");
    edit2.push_back("SendLongBDATA"); edit2_tit.push_back("SendLongBDATA x");
    edit2.push_back("SendBST"); edit2_tit.push_back("SendBST");
    HTMLFieldElement edittab2(HTMLFieldElement::DROPDOWNMENU, 0, 
                             "edit2",edit2,edit2_tit);
    edittab2.SetDefault(edit2[0]);
    edittab2.Write(*out);
    *out << endl <<" x = ";
    *out << cgicc::input().set("type","text")
      .set("name","x")
      .set("size","4")
      .set("value","") <<"<br>"<<std::endl;
    edit2.clear(); edit2_tit.clear();
    edit2.push_back("Choose"); edit2_tit.push_back("< Choose >");
    for (size_t k=0; k<TTCciPtr_->NChannels(); ++k){
      string val = "BGO ", tit;
      string dum = TTCciPtr_->GetBChannel(k)->GetName();
      if (dum[0]=='C' && dum[1]=='h' && dum[2]=='a'){
        char si[3];
        sprintf(si,"%u",k);
        val += string(si);
      }else{
        val += dum;
      }
      tit = val;
      edit2.push_back(val); edit2_tit.push_back(tit);
    }
    *out << "&nbsp;&nbsp; or BGO: ";
    HTMLFieldElement edittab3(HTMLFieldElement::DROPDOWNMENU, 0, 
                             "edit3",edit2,edit2_tit);
    edittab3.SetDefault(edit2[0]);
    edittab3.Write(*out);
    
    tab.Close();
    *out << "<p>"<<endl;
    *out << cgicc::fieldset() << std::endl;// End Fieldset 
    if ( TTCciPtr_ ){
      *out << cgicc::input().set("type", "submit")
        .set("name", "submit")
        .set("value", "Apply");
    }else{
      *out << cgicc::input().set("type", "submit")
        .set("name", "submit")
        .set("value", "Apply").set("disabled", "true");
    }
    *out << cgicc::form() << std::endl;
  }

  WriteDebugTextArray(out);
  PrintHTMLFooter(out);

}



// The TTCci Main Configuration Page
void PixelTTCSupervisor::BGOConfiguration(xgi::Input * /*in*/, xgi::Output * out ) throw (xgi::exception::Exception){
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  LastPage_ = "BGOConfiguration";
  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  //*out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  *out << cgicc::title("TTCci \""+name_.toString()+"\"") << std::endl;

  if (TTCciPtr_ && AutoRefresh_){
    *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"5; URL="
         <<LastPage_<<"\">" <<endl;
  }

  PrintHTMLHeader(out);

  *out << cgicc::h1(toolbox::toString("TTCci BGO %s (%s)",
                                      CurrentBGOList.GetDefaultTitle().c_str(),
                                      GetName(1).c_str()))<<std::endl;

  ErrorStatement(out);

  if (!TTCciPtr_) {
    *out << RED<<BIGGER<<"TTCci Object not yet created.<br>"
         <<NOBIGGER<<NOCOL<<endl; 
    *out << "--> Go to the <a href=\"/"<<getApplicationDescriptor()->getURN()
         <<"/\">Main Page (TTCci Control)</a> and create the TTCci object "
         <<"(or 'configure') first.<br>"<<endl;
    return;
  }

  if (CurrentBGOList.GetDefaultInt()!=int(CurrentBGO)){
    html << "WARNING: PixelTTCSupervisor::BGOConfiguration(): "
          <<"CurrentBGOList.GetDefaultInt() != int(CurrentBGO) ("
          <<CurrentBGOList.GetDefaultInt()<<" != "<<int(CurrentBGO)<<")"<<endl;
    CurrentBGOList.SetDefault(CurrentBGO);
  }
  const size_t ich = CurrentBGO;

  *out << "State Machine: ";
  PrintSMState(out);
  *out << cgicc::p()<<cgicc::p()<<endl;

  if ( !TTCciPtr_ ){
    // error
    *out << BIGGER<<BOLD<<RED
         <<"ERROR!!! TTCciPtr_ not yet initialized!"
         <<NOCOL<<NOBOLD<<NOBIGGER<<endl;
    return;
  }

  {// BGO Channel Select
    HTMLTable tab(*out,0,0,1,"","",100);
    tab.NewCell();
    std::string select_url = "/";
    select_url += getApplicationDescriptor()->getURN();
    select_url += "/BGOSelectCommand";	
    *out << cgicc::form().set("method","post")
      .set("action", select_url)
      .set("enctype","multipart/form-data") << std::endl;
    *out << cgicc::fieldset() << std::endl;// Begin Fieldset 
    *out << cgicc::legend("Current No. "+
                          CurrentBGOList.GetDefault()) << std::endl;
    *out << "Switch to other BGO Channel: "<<endl;
    CurrentBGOList.Write(*out);
    *out << "<p>"<<endl;
    *out << cgicc::fieldset() << std::endl;// End Fieldset 
    if ( TTCciPtr_ ){
      *out << cgicc::input().set("type", "submit")
        .set("name", "submit")
        .set("value", "Show channel");
    }else{
      *out << cgicc::input().set("type", "submit")
        .set("name", "submit")
        .set("value", "Show channel").set("disabled", "true");
    }
    *out << cgicc::form() << std::endl;
    tab.NewCell();
    *out << "VME BGO-"<<(ich>9?"":"0")<<ich<<":"<<endl;

    {// VME BGO Button
      std::string cmd_url = "/";
      cmd_url += getApplicationDescriptor()->getURN();
      cmd_url += "/Command";	
      *out << cgicc::form().set("method","get").set("action", cmd_url).set("enctype","multipart/form-data") << std::endl;
      if ( !TTCciPtr_ || !TTCciPtr_->IsSourceSelected("BGO",ttc::VME)){
        *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "Send VME-BGO").set("disabled", "true");
      }else{
        *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "Send VME-BGO");
      }
      *out << cgicc::form();       
    }
    tab.Close();
  }

  std::string command_url = "/";
  command_url += getApplicationDescriptor()->getURN();
  command_url += "/BGOConfigCommand";	

  *out << cgicc::form().set("method","post")
    .set("action", command_url)
    .set("enctype","multipart/form-data") << std::endl;
  *out << cgicc::fieldset() << std::endl;// Begin Fieldset 
  *out << cgicc::legend("Configuration of BGO "+
                        CurrentBGOList.GetDefaultTitle()) << std::endl;

  // THE TABLE
  HTMLTable tab(*out,0,0,1,"","",100);
  tab.NewCell("top");
  // the left big cell for the main BGO config of that channel
  const ttc::BChannel *bch = TTCciPtr_->GetBChannel(ich);
  string sel = string(UNDERL)+string(GREEN), 
    unsel = string(NOCOL)+string(NOUNDERL);
  *out << BOLD<<BIGGER<<"Data Properties<br>"<<NOBIGGER<<NOBOLD<<endl;
  *out << "Command Length:<br>"<<endl;
  if (bch->IsSingleCommand()){
    *out << cgicc::input().set("type", "radio")
      .set("name","commandlength")
      .set("value","single").set("checked","checked");
    *out << sel<<"SINGLE"<<unsel<<" ";
  }else{
    *out << cgicc::input().set("type", "radio")
      .set("name","commandlength")
      .set("value","single");
    *out << "SINGLE ";
  }
  *out << "<br>"<<endl;
  if (bch->IsDoubleCommand()){
    *out << cgicc::input().set("type", "radio")
      .set("name","commandlength")
      .set("value","double").set("checked","checked");
    *out << sel<<"DOUBLE"<<unsel<<" ";
  }else{
    *out << cgicc::input().set("type", "radio")
      .set("name","commandlength")
      .set("value","double");
    *out << "DOUBLE ";
  }
  *out << "<br>"<<endl;
  if (bch->IsBlockCommand()){
    *out << cgicc::input().set("type", "radio")
      .set("name","commandlength")
      .set("value","block").set("checked","checked");
    *out << sel<<"BLOCK"<<unsel;
  }else{
    *out << cgicc::input().set("type", "radio")
      .set("name","commandlength")
      .set("value","block");
    *out << "BLOCK";
  }
  *out << endl;
  *out << "<br>"<<endl;

  if (bch->IsRepetitive()){
    *out << GREEN<<BOLD;
    *out << cgicc::input().set("type", "checkbox")
      .set("name","repetitive").set("checked","checked") << std::endl;
  }else{
    *out << cgicc::input().set("type", "checkbox")
      .set("name","repetitive") << std::endl;
  }
  *out << cgicc::label("Repetitive");
  if (bch->IsRepetitive()) *out << NOBOLD<<NOCOL;
  *out << "<br>"<<endl;
 
//   char sdel1[30], sdel2[30];
//   sprintf(sdel1,"%lu", bch->GetDelayTime1());
//   sprintf(sdel2,"%lu", bch->GetDelayTime2());
  *out << "Delay "<<ITALIC<<"t1"<<NOITALIC<<": ";
  *out << cgicc::input().set("type","text")
    .set("name","delay1")
    .set("size","6")
    .set("value",UnsignedLong2String(bch->GetDelayTime1()).c_str()) <<  " &times; BX"<<endl;
  //*out << "<br>Delay "<<ITALIC<<"t<sub>2</sub>"<<NOITALIC<<": ";
  *out << "<br>Delay "<<ITALIC<<"t2"<<NOITALIC<<": ";
  *out << cgicc::input().set("type","text")
    .set("name","delay2")
    .set("size","6")
    .set("value",UnsignedLong2String(bch->GetDelayTime2()).c_str()) <<  " &times; BX"<<endl;

//   char spre[30], spost[30], soffset[30];
//   sprintf(spre, "%lu", bch->GetPrescale());
//   sprintf(soffset, "%lu", bch->GetInitialPrescale());
//   sprintf(spost,"%lu", bch->GetPostscale());
  *out << "<br>Prescale: ";
  *out << cgicc::input().set("type","text")
    .set("name","prescale")
    .set("size","6")
    .set("value",UnsignedLong2String(bch->GetPrescale()).c_str()) << endl;
  *out << SMALLER<<" [0,65535]"<<NOSMALLER<<"<br>Postscale: ";
  *out << cgicc::input().set("type","text")
    .set("name","postscale")
    .set("size","6")
    .set("value",UnsignedLong2String(bch->GetPostscale()).c_str()) <<  SMALLER<<" [0,65535]"
       <<NOSMALLER<<"<br>Init.offset: "<<endl;
  *out << cgicc::input().set("type","text")
    .set("name","offset")
    .set("size","6")
    .set("value",UnsignedLong2String(bch->GetInitialPrescale()).c_str())<<" requests" <<endl;

  // begin inner right table: 
  tab.NewCell();
  *out << BOLD<<BIGGER<< CurrentBGOList.GetDefaultTitle() <<dec
       <<" BGO Commands:<br>"<<NOBIGGER<<NOBOLD<<endl;
  HTMLTable tab2(*out,1,2,2,"","",100);
  tab2.NewRow();
  tab2.NewCell("top");
  *out << "No." << endl;
  tab2.NewCell();
  *out << "Data" << endl;
  tab2.NewCell();
  *out << "Type" << endl;
  tab2.NewCell();
  *out << MONO<<"CH"<<(ich>9?"":"0")<<ich<<"D/P"<<NOMONO<<endl;
  tab2.NewCell();
  *out << "Remove" << endl;
  char dummy[30];
  vector<string> values, titles;
  titles.push_back("New"); 
  values.push_back("New"); 
  for (size_t n=0; n<bch->NWords(); ++n){
    tab2.NewRow();
    const bool LAST=bch->IsLastWord(n);
    if (!LAST){
      sprintf(dummy,"%d)",n);
      titles.push_back(string(dummy));
      sprintf(dummy,"%d",n);
      values.push_back(string(dummy));
    }
    //string bgdcol = (LAST ? BGDCOL_GREY : 
    //                 (bch->IsACommand(n) ? BGDCOL_ORANGE : BGDCOL_YELLOW));
    string bgdcol = (LAST?"grey":(bch->IsACommand(n)?"orange":"yellow"));
    *out << BIGGER<<endl;
    tab2.NewCell(bgdcol);
    if (0 && LAST){
      *out <<"Last";
    }else{
      *out <<dec<<n<<")";
    }
    *out << endl;
    tab2.NewCell(bgdcol);
    if (LAST){
      *out << ">>LastWord<<";
    }else{
      *out << MONO<<"0x"<<hex<<bch->DataWord_D(n)<<dec<<NOMONO;
    }
    *out<<endl;
    tab2.NewCell(bgdcol);
    if (bch->IsACommand(n)){
      *out << "A-Cmnd./L1A";
    }else{
      *out << (bch->IsShortWord(n)?"Short":"Long");
    }
    *out <<endl;
    tab2.NewCell(bgdcol);
    *out << SMALLER<<MONO<<BLUE<<"0x"<<hex<<GetBGOWord(ich,n,true)<<NOCOL
         <<dec<<" / "<<BLUE
         <<"0x"<<hex<<GetBGOWord(ich,n,false)<<NOCOL<<dec<<NOMONO<<NOSMALLER
         <<endl;
    *out << "<br>";
    tab2.NewCell(bgdcol);
    sprintf(dummy,"delete%d",n);
    if (!LAST){
      *out << cgicc::input().set("type", "checkbox")
        .set("name",dummy) << endl;
    }
    *out << NOBIGGER<<endl;
  }
  tab2.NewRow();
  tab2.NewCell("lightgreen");
  HTMLFieldElement DataList(HTMLFieldElement::DROPDOWNMENU, 0,
                            "datawordselect", values, titles);
  DataList.SetDefault(values[0]);
  DataList.Write(*out);
  tab2.NewCell();
  *out << MONO<<"0x"<<MONO;
  *out << cgicc::input().set("type","text")
    .set("name","newdata").set("size","8").set("value","") <<endl;;  
  tab2.NewCell();
  DataTypeSelector.Write(*out);
  tab2.NewCell();
  tab2.NewCell();
  *out << "append"<<endl;
  tab2.Close();
  *out << "<br>" << endl;
  *out << MONO<<"CHIHB #"<<dec<<ich<<" = "
       <<BLUE<<"0x"<<hex<<TTCciPtr_->Read(TTCciAdd::CHIHB,ich)
       <<dec<<NOCOL<<NOMONO  //<< "<br>"
       << MONO<<" CHPRESC #"<<dec<<ich<<" = "
       <<BLUE<<"0x"<<hex<<TTCciPtr_->Read(TTCciAdd::CHPRESC,ich)
       <<dec<<NOCOL<<NOMONO
       <<dec<<"<br>"<<endl;
  // End inner right table
  tab.Close();

  *out << cgicc::fieldset() << std::endl;// End Fieldset 
  if ( TTCciPtr_ ){
    *out << cgicc::input().set("type", "submit")
      .set("name", "submit")
      .set("value", "Apply");
  }else{
    *out << cgicc::input().set("type", "submit")
      .set("name", "submit")
      .set("value", "Apply").set("disabled", "true");
  }
  *out << cgicc::form() << std::endl;

  {// RST & RMC
    std::string cmd_url = "/";
    cmd_url += getApplicationDescriptor()->getURN();
    cmd_url += "/Command";	
    const uint32_t rmc = (TTCciPtr_ ? 
                               TTCciPtr_->Read(TTCciAdd::CHRMC,CurrentBGO) : 0 );
    HTMLTable tab2(*out,0,2,1,"","",100);
    tab2.NewCell("top");
    *out << BOLD<<UNDERL<<"Counters for BGO Channel "<<dec
         << CurrentBGO
         <<NOUNDERL<<NOBOLD<<"<br>"<<endl;
    *out << "Last cancelled data: "<<endl;
    if ((rmc>>27)&1){
      *out << RED <<"yes (= error)"<<NOCOL;
    }else{
      *out << "no (= o.k.)";
    }
    *out <<"<br>"<<endl;
    *out << "RAM: Buffer " << ((rmc>>26)&1 ? 
                               string(UNDERL)+"reached"+string(NOUNDERL) : 
                               string(UNDERL)+"not"+string(NOUNDERL)+" at")
         <<" end<br>"<<endl;
    *out << "Request Counter: " <<dec<< ((rmc>>0)&0xffff);
    *out << "<br>"<<endl;
    const uint32_t cancelcntr = ((rmc>>16)&0x3ff);
    *out << "Cancel Counter: "<<dec; 
    if (cancelcntr>0) *out << RED << BOLD << cancelcntr << NOBOLD << NOCOL;
    else *out << cancelcntr ;
    *out << endl;
    tab2.NewCell();
   // Reset Request Counter
#if 0
    *out << cgicc::form().set("method","get").set("action", cmd_url).set("enctype","multipart/form-data") << std::endl;
    if ( !TTCciPtr_ ){
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "Reset request cntr").set("disabled", "true");
    }else{
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "Reset request cntr");
    }
    *out << cgicc::form();       
    // Reset Prescale Counter 
    *out << cgicc::form().set("method","get").set("action", cmd_url).set("enctype","multipart/form-data") << std::endl;
    if ( !TTCciPtr_ ){
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "Reset prescale cntr").set("disabled", "true");
    }else{
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "Reset prescale cntr");
    }
    *out << cgicc::form();       
    //*out << "<br>"<<endl;
    // Reset Cancel Counter 
    *out << cgicc::form().set("method","get").set("action", cmd_url).set("enctype","multipart/form-data") << std::endl;
    if ( !TTCciPtr_ ){
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "Reset cancel cntr").set("disabled", "true");
    }else{
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "Reset cancel cntr");
    }
    *out << cgicc::form();       
    tab2.NewCell();
    // Reset All Counters 
    *out << cgicc::form().set("method","get").set("action", cmd_url).set("enctype","multipart/form-data") << std::endl;
    if ( !TTCciPtr_ ){
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "Reset all 3 counters").set("disabled", "true");
    }else{
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "Reset all 3 counters");
    }
    *out << cgicc::form();       
    // Reset All Counters on ALL channels
    *out << cgicc::form().set("method","get").set("action", cmd_url).set("enctype","multipart/form-data") << std::endl;
    if ( !TTCciPtr_ ){
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "Reset all on all channels").set("disabled", "true");
    }else{
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "Reset all on all channels");
    }
    *out << cgicc::form();           
#endif
    // Read BGO Config from TTCci
    *out << cgicc::form().set("method","get").set("action", cmd_url).set("enctype","multipart/form-data") << std::endl;
    if ( !TTCciPtr_ ){
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "Read BGO configuration from TTCci").set("disabled", "true");
    }else{
      *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "Read BGO configuration from TTCci");
    }
    *out << cgicc::form();   
    tab2.Close();
  }
 
  WriteDebugTextArray(out);
  PrintHTMLFooter(out); 
}


// The Cyclic Generators Page
void PixelTTCSupervisor::CyclicGenerators(xgi::Input * /*in*/, xgi::Output * out ) throw (xgi::exception::Exception){
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  LastPage_ = "CyclicGenerators";
  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  //*out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  *out << cgicc::title("TTCci \""+name_.toString()+"\"") << std::endl;

  if (TTCciPtr_ && AutoRefresh_){
    *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"5; URL="
         <<LastPage_<<"\">" <<endl;
  }

  PrintHTMLHeader(out);

  *out << cgicc::h1(toolbox::toString("TTCci Cyclic Generators (%s)",
                                      GetName(1).c_str()))<<std::endl;

  ErrorStatement(out);

  if (!TTCciPtr_) {
    *out << RED<<BIGGER<<"TTCci Object not yet created.<br>"
         <<NOBIGGER<<NOCOL<<endl; 
    *out << "--> Go to the <a href=\"/"<<getApplicationDescriptor()->getURN()
         <<"/\">Main Page (TTCci Control)</a> and create the TTCci object "
         <<"(or 'configure') first.<br>"<<endl;
    return;
  }

  {// Start & Stop Buttons
    {
      std::string cmd_url = "/";
      cmd_url += getApplicationDescriptor()->getURN();
      cmd_url += "/Command";
      HTMLTable buttontab(*out,0,2,2);
      buttontab.NewCell();
      // VME BGO Start Button:	
      *out << cgicc::form().set("method","get").set("action", cmd_url).set("enctype","multipart/form-data") << std::endl;
      if ( !TTCciPtr_ || !TTCciPtr_->IsSourceSelected("BGO",ttc::VME)){
        *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "Send BGO-Start").set("disabled", "true")<<endl;
      }else{
        *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "Send BGO-Start")<<endl;
      }
      *out << cgicc::form(); 
      buttontab.NewCell();
      // Stop Button:
      *out << cgicc::form().set("method","get").set("action", cmd_url).set("enctype","multipart/form-data") << std::endl;
      if ( !TTCciPtr_ || !TTCciPtr_->IsSourceSelected("BGO",ttc::VME)){
        *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "Send BGO-Stop").set("disabled", "true")<<endl;
        *out << "("<<INAME(VME)<<" is NOT selected as BGO source)"<<endl;
      }else{
        *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "Send BGO-Stop")<<endl;
      }
      *out << cgicc::form(); 
      buttontab.Close();
    }
    
  }

  bool triggersource=false, bgosource=false; 
  {
    std::vector<ExternalInterface> myitf = TTCciPtr_->CheckSource("Trigger");
    if (find(myitf.begin(), myitf.end(), CYCLIC) != myitf.end())
      triggersource=true;
    myitf = TTCciPtr_->CheckSource("BGO");
    if (find(myitf.begin(), myitf.end(), CYCLIC) != myitf.end())
      bgosource=true;
  }

  {
    *out << "State Machine: ";
    PrintSMState(out);
    *out << " - Source Selection: "<<BOLD<<"BGO=" 
         << (bgosource?GREEN:RED)<<(bgosource?"enabled":"disabled")<<NOCOL 
         <<" Trigger="
         <<(triggersource?GREEN:RED)
         <<(triggersource?"enabled":"disabled")<<NOCOL<<NOBOLD
         <<endl;
    *out << cgicc::p()<<cgicc::p()<<endl;
  }

  if ( !TTCciPtr_ ){
    *out << BIGGER<<BOLD<<RED
         <<"ERROR!!! TTCciPtr_ not yet initialized!"
         <<NOCOL<<NOBOLD<<NOBIGGER<<endl;
    return;
  }


  std::string command_url = "/";
  command_url += getApplicationDescriptor()->getURN();
  command_url += "/CyclicConfigCommand";	

  *out << cgicc::form().set("method","post")
    .set("action", command_url)
    .set("enctype","multipart/form-data") << std::endl;
  *out << cgicc::fieldset() << std::endl;// Begin Fieldset 
  *out << cgicc::legend("Cyclic Generator Configuration") << std::endl;

  { // The Table
    HTMLTable maintab(*out,0);
    // First row:
    maintab.NewRow();
    maintab.NewCell();
    *out << "Name";
    maintab.NewCell();
    *out << "Configuration";
    maintab.CloseRow();
    
    vector<string> enable_val, enable_tit;
    enable_val.push_back("enable");  enable_tit.push_back("enable");
    enable_val.push_back("disable"); enable_tit.push_back("disable");

    for (size_t ik=0; ik < (TTCciPtr_->NCyclicBGO()+
                            TTCciPtr_->NCyclicTrigger()); ++ik){
      char genc[10];sprintf(genc,"%u__",ik);const string sgen=string(genc);
      bool trigger = (ik >= TTCciPtr_->NCyclicBGO());
      const size_t i = (!trigger ? ik : ik-TTCciPtr_->NCyclicBGO());
      const CyclicTriggerOrBGO *cycl = TTCciPtr_->GetCyclic(trigger, i);
      // New row
      // First cell:
      maintab.NewRow();
      maintab.NewCell();
      const bool enabled = cycl->IsEnabled();
      *out << cycl->GetName()<<"<br>"<<endl;
      HTMLFieldElement enable(HTMLFieldElement::RADIOBUTTON, 0, sgen+"enable",
                              enable_val, enable_tit,"vertical bold blue");
      enable.SetDefault((enabled?0:1));
      enable.Write(*out);

      // Second Cell:
      maintab.NewCell();
      {// Configuration Cell  
        string bgdcol="";
        if (enabled){
          if ((trigger&&triggersource)||(!trigger&&bgosource))
            bgdcol = "lightgreen";
          else
            bgdcol = "green";
        }else if (!enabled){
          if((trigger&&triggersource)||(!trigger&&bgosource))
            bgdcol = "lightgrey";
          else
            bgdcol = "grey";
        }
        HTMLTable inner(*out,1,3,0,bgdcol);
        // first config row:
        inner.NewRow();
        // StartBX
        inner.NewCell();
        *out << "StartBX: "<<endl;
        //char dum[100];
        //sprintf(dum,"%ld",cycl->GetStartBX());
        *out << cgicc::input().set("type","text")
          .set("name",sgen+"startbx")
          .set("size","3")
          .set("value",Long2String(cycl->GetStartBX()).c_str())<<endl;
        // Prescale
        inner.NewCell();
        *out << "Prescale: "<<endl;
        //sprintf(dum,"%ld",cycl->GetPrescale());
        *out << cgicc::input().set("type","text")
          .set("name",sgen+"prescale")
          .set("size","3")
          .set("value",Long2String(cycl->GetPrescale()).c_str())<<endl;
        // InitPrescale
        inner.NewCell();
        *out << "InitWait [Orb]: "<<endl;
        //sprintf(dum,"%ld",cycl->GetInitialPrescale());
        *out << cgicc::input().set("type","text")
          .set("name",sgen+"initprescale")
          .set("size","3")
          .set("value",Long2String(cycl->GetInitialPrescale()).c_str())<<endl;
        // B Channels:
        inner.NewCell();
        if (!trigger){
          HTMLFieldElement BGOList = CurrentBGOList;
          BGOList.SetName(sgen+"channelno");
          BGOList.SetDefault(cycl->GetBChannel());
          BGOList.Write(*out);
        }
        
        // next config line
        inner.NewRow();

        // Postscale
        inner.NewCell();
        //sprintf(dum,"%ld",cycl->GetPostscale());
        *out << cgicc::input().set("type","text")
          .set("name",sgen+"postscale")
          .set("size","3")
          .set("value",Long2String(cycl->GetPostscale()).c_str())<<endl;
        *out << " times"<<endl;
        // Pause
        inner.NewCell();
        *out << "Pause [Orb]: "<<endl;
        //sprintf(dum,"%ld",cycl->GetPause());
        *out << cgicc::input().set("type","text")
          .set("name",sgen+"pause")
          .set("size","3")
          .set("value",Long2String(cycl->GetPause()).c_str())<<endl;
        // IsRepetitive(); 
        inner.NewCell();
        if (cycl->IsRepetitive()){
          *out << cgicc::input().set("type", "checkbox")
            .set("name",sgen+"repetitive").set("checked","checked");
        }else{
          *out << cgicc::input().set("type", "checkbox")
            .set("name",sgen+"repetitive");
        }
        *out<<  "repetitive" << std::endl;
        // IsPermanent();
        inner.NewCell();
        if (cycl->IsPermanent()){
          *out << cgicc::input().set("type", "checkbox")
            .set("name",sgen+"permanent").set("checked","checked");
        }else{
          *out << cgicc::input().set("type", "checkbox")
            .set("name",sgen+"permanent");
        }
        *out<<  "permanent" << std::endl;

        inner.Close();
      }
    }
    // END MAIN table
    maintab.Close();
  }
  *out << cgicc::fieldset() << std::endl;// Begin Fieldset 
  if ( TTCciPtr_ ){
    *out << cgicc::input().set("type", "submit")
      .set("name", "submit")
      .set("value", "Apply");
  }else{
    *out << cgicc::input().set("type", "submit")
      .set("name", "submit")
      .set("value", "Apply").set("disabled", "true");
  }
  *out << cgicc::form() << std::endl;

 
  WriteDebugTextArray(out);
  PrintHTMLFooter(out);
}

// The Summary Page
void PixelTTCSupervisor::SummaryPage(xgi::Input * /*in*/, xgi::Output * out ) throw (xgi::exception::Exception){
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  LastPage_ = "SummaryPage";
  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  //*out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  *out << cgicc::title("TTCci \""+name_.toString()+"\"") << std::endl;

  
  if (TTCciPtr_ && AutoRefresh_){
    *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"5; URL="
         <<LastPage_<<"\">" <<endl;
  }
  

  PrintHTMLHeader(out);

  *out << cgicc::h1(toolbox::toString("TTCci Summary (%s)",
                                      GetName(1).c_str()))<<std::endl;

  ErrorStatement(out);

  if (!TTCciPtr_) {
    *out << RED<<BIGGER<<"TTCci Object not yet created.<br>"
         <<NOBIGGER<<NOCOL<<endl; 
    *out << "--> Go to the <a href=\"/"<<getApplicationDescriptor()->getURN()
         <<"/\">Main Page (TTCci Control)</a> and create the TTCci object "
         <<"(or 'configure') first.<br>"<<endl;
    return;
  }

  
  if (TTCciPtr_){
    const string emph=DARKBLUE, deflt=BLACK, alert=RED;
    // Status Summary
    const string url = "/"+getApplicationDescriptor()->getURN()+"/";
    ReadTTCciCounters();
    HTMLTable stab(*out,1,2,0,"","",100);
    stab.NewCell("top",1,1);
    *out << "State: ";
    PrintSMState(out);
    stab.NewCell("top",1,1);
    *out << "L1A: "<<BOLD<<(TTCciPtr_->IsL1ADisabled()?RED:GREEN)
         <<(TTCciPtr_->IsL1ADisabled()?"Disabled":"Enabled")
         << NOCOL<<NOBOLD<< endl;
    stab.NewCell("top",1,1);
    *out << "Laser: ";
    if (TTCciPtr_->IsLaserOn()){
      *out << GREEN << "on" << NOCOL;
    }else{
      *out << RED << "off" << NOCOL;
    }
    stab.NewCell("top",1,1);
    stab.NewCell("top",1,1);
    { // Clock locked?
      string defaultcol = GREEN, alertcol = RED;
      bool lock  = TTCciPtr_->ClockLocked();
      string col = (lock?defaultcol:alertcol);
      *out << "Clock: " <<col
           << (lock?"locked":"not locked")<<NOCOL<<endl;
    }
    { // Clock inverted?
      bool inv  = TTCciPtr_->ClockInverted();
      *out << (inv?", inverted":", not inverted")<<endl;
    }
    // Status & Counters
    stab.NewRow();
    stab.NewCell("top lightyellow");
    *out << UNDERL<<"Counters / Status"<<NOUNDERL<<endl;
    stab.NewCell();
    *out << UNDERL <<"Counters" << NOUNDERL << endl;
    *out << "<br>" << "Evt-ID: " << EventCounter_ <<endl;
    *out << "<br>" << "Orbit: " << OrbitCounter_ <<endl;
    *out << "<br>" << "BGO-Req.Cntr: " << StrobeCounter_ <<endl;
    stab.NewCell();
    {
      const double dt = difftime(tnow_, tprevious_);
      *out << UNDERL<<"L1A rate (after "<<dt<<" s):"
           <<NOUNDERL<<"<br>"<<endl;
      double diff = double(EventCounter_-previousEventCounter_);
      double odiff = double(OrbitCounter_-previousOrbitCounter_);
      *out << int(diff)<<" L1As in "<<odiff<<" orbs.<br>"<<endl;
      if (odiff>0.0 && dt>0.0) 
        *out << RED<<(diff/(odiff*3564*25.0e-9))<<" Hz"<<NOCOL
             <<"<br>"<<endl;
      else 
        *out << RED<<"nan"<<" Hz"<<NOCOL<<"<br>"<<endl;

      if (odiff>0.0) *out << (diff/odiff) << " L1As/orbit <br>"<<endl;
      else *out << "nan" << " L1As/orbit <br>"<<endl;
      if (diff>0.0) *out << (odiff/diff) << " orbits/L1A <br>"<<endl;
      else *out << "nan" << " orbits/L1A"<<endl;
      stab.NewCell();
      diff = double(StrobeCounter_-previousStrobeCounter_);
      *out << UNDERL<<"BGO-req. rate:"<<NOUNDERL<<"<br>"<<endl;
      *out << int(diff)<<" BGOs<br>"<<endl;
      if (dt>0.0) *out << RED<<(diff/(odiff*3564*25.0e-9))
                       <<" Hz"<<NOCOL<<"<br>"<<endl;
      else *out << RED<<"nan"<<" Hz"<<NOCOL<<"<br>"<<endl;
      if (odiff>0.0) *out << (diff/odiff) << " BGOs/orbit <br>"<<endl;
      else *out << "nan" << " BGOs/orbit <br>"<<endl;
      if (diff>0.0) *out << (odiff/diff) << " orbits/BGO <br>"<<endl;
      else *out << "nan" << " orbits/BGO"<<endl;
    }
    stab.NewCell();
    {
      *out << UNDERL<<"Board Status:"<<NOUNDERL<<MONO<<BLUE<<hex<<" 0x"
           << BoardStatus_ <<NOCOL<<NOMONO<<dec<<endl;
      *out << SMALLER<<"&nbsp;(..) = latched"
           <<NOSMALLER<<endl; 
      string defaultcol = GREEN, alertcol = RED;
      { // BDataCancelled:
        bool isok  = !TTCciPtr_->IsBDataCancelled();
        bool isok2 = !TTCciPtr_->IsBDataCancelled_Latched(false);
        string col = (isok?defaultcol:alertcol);
        *out << "<br>" << "BData Cancelled? " <<col
             << (isok?"no":"yes")<<NOCOL;
        col = (isok2?defaultcol:alertcol);
        *out << " ("<<col<<(isok2?"no":"yes")<<NOCOL<<")"<<endl;
      }
      { // DataClkError:
        bool isok  = !TTCciPtr_->IsDataClkError(false);
        bool isok2 = !TTCciPtr_->IsDataClkError_Latched(false);
        string col = (isok?defaultcol:alertcol);
        *out << "<br>" << "Dat/Clk sync error? " <<col
             << (isok?"no":"yes")<<NOCOL;
        col = (isok2?defaultcol:alertcol);
        *out << " ("<<col<<(isok2?"no":"yes")<<NOCOL<<")"<<endl;
      }
      { // IsClkSingleEvtUpset:
        bool isok  = !TTCciPtr_->IsClkSingleEvtUpset(false);
        bool isok2 = !TTCciPtr_->IsClkSingleEvtUpset_Latched(false);
        string col = (isok?defaultcol:alertcol);
        *out << "<br>" << "Clk. single-evt. upset? " <<col
             << (isok?"no":"yes")<<NOCOL;
        col = (isok2?defaultcol:alertcol);
        *out << " ("<<col<<(isok2?"no":"yes")<<NOCOL<<")"<<endl;
      }
      { // MissedL1A:
        bool isok  = !TTCciPtr_->MissedL1A(false);
        bool isok2 = !TTCciPtr_->MissedL1A_Latched(false);
        string col = (isok?defaultcol:alertcol);
        *out << "<br>" << "L1A missed @ 40MHz? " <<col
             << (isok?"no":"yes")<<NOCOL;
        col = (isok2?defaultcol:alertcol);
        *out << " ("<<col<<(isok2?"no":"yes")<<NOCOL<<")"<<endl;
      }
      { // DoubleL1Aat40MHz:
        bool isok  = !TTCciPtr_->DoubleL1Aat40MHz(false);
        bool isok2 = !TTCciPtr_->DoubleL1Aat40MHz_Latched(false);
        string col = (isok?defaultcol:alertcol);
        *out << "<br>" << "Double-L1A @ 40MHz? " <<col
             << (isok?"no":"yes")<<NOCOL;
        col = (isok2?defaultcol:alertcol);
        *out << " ("<<col<<(isok2?"no":"yes")<<NOCOL<<")"<<endl;
      }
    
    }
    // Sources:
    stab.NewRow();
    stab.NewCell("top lightblue");
    bool cycltrig=false, cyclbgo=false;
    *out << "<a href=\""<<url<<"MainConfiguration\">";
    *out << "Input Sources"<<"</a>"<<endl;
    stab.NewCell();
    *out << BOLD<<"Clock:"<<NOBOLD<<"<br>- "
         <<INAME(TTCciPtr_->CheckClock())<<endl;
    stab.NewCell();
    *out << BOLD<<"Orbit:"<<NOBOLD<<"<br>- "
         <<INAME(TTCciPtr_->CheckOrbitSource())<<endl;
    stab.NewCell();
    *out << BOLD<<"BGO:"<<NOBOLD<<endl;
    std::vector<ExternalInterface> bsource = TTCciPtr_->CheckBGOSource();
    for (size_t i=0; i<bsource.size(); ++i){
      if (bsource[i] == CYCLIC) cyclbgo=true;
      *out<<"<br>- "<<INAME(bsource[i])<<endl;
    }
    stab.NewCell();
    *out << BOLD<<"Trigger:"<<NOBOLD<<endl;
    std::vector<ExternalInterface> tsource = TTCciPtr_->CheckTrigger();
    for (size_t i=0; i<tsource.size(); ++i){
      if (tsource[i] == CYCLIC) cycltrig=true;
      *out<<"<br>- "<<INAME(tsource[i])<<endl;
      if (TTCciPtr_->CanSetInternalTriggerFrequency() && 
          tsource[i]==ttc::INTERNAL){
        *out << "("<<dec<<TTCciPtr_->GetInternalTriggerFrequency()<<" Hz)";
      }
    }
    // cyclic triggers:
    if (cycltrig){
      for (size_t i=0; i<TTCciPtr_->NCyclicTrigger(); ++i){
        CyclicTriggerOrBGO *cycl = TTCciPtr_->GetCyclic(true,i);
        if (!cycl->IsEnabled()) continue;
        stab.NewRow();
        stab.NewCell("lightred");
        *out << "<a href=\""<<url<<"CyclicGenerators\">";
        *out << "Cycl. Trig "<<cycl->GetID()<<"</a>"<<endl;
        stab.NewCell("",4,1);
        *out<<(cycl->GetStartBX()>0?emph:deflt)
            <<"StartBX: "<<cycl->GetStartBX()<<NOCOL
            <<"&nbsp;&nbsp;&nbsp;&nbsp;"
            <<(cycl->GetPrescale()>0?emph:deflt)
            <<"Prescale: "<<cycl->GetPrescale()<<NOCOL
            <<"&nbsp;&nbsp;&nbsp;&nbsp;"
            <<(cycl->GetInitialPrescale()>0?emph:deflt)
            <<"InitWait: "<<cycl->GetInitialPrescale()<<NOCOL
            <<" orbits"<<"&nbsp;&nbsp;&nbsp;&nbsp;"<<endl;
        *out <<"<br>"<<endl;
        *out <<(cycl->GetPostscale()>0?emph:deflt)
             <<cycl->GetPostscale()<<" &times;"<<NOCOL
             <<"&nbsp;&nbsp;&nbsp;&nbsp;"
             <<(cycl->GetPause()>0?emph:deflt)
             <<"Pause: "<<cycl->GetPause()<<" orbits"<<NOCOL
             <<"&nbsp;&nbsp;&nbsp;&nbsp;"
             <<(cycl->IsRepetitive()?deflt:emph)
             <<(cycl->IsRepetitive()?"(repetitive)":"(non-repetitive)")
             <<"&nbsp;&nbsp;&nbsp;&nbsp;"
             <<(cycl->IsPermanent()?alert:deflt)
             <<(cycl->IsPermanent()?"(permanent)":"(non-permanent)")
             <<NOCOL<<endl;
      }
    }
    // cyclic BGOs:
    if (cycltrig){
      for (size_t i=0; i<TTCciPtr_->NCyclicBGO(); ++i){
        CyclicTriggerOrBGO *cycl = TTCciPtr_->GetCyclic(false,i);
        if (!cycl->IsEnabled()) continue;
        stab.NewRow();
        stab.NewCell("lightorange");
        *out << "<a href=\""<<url<<"CyclicGenerators\">";
        *out << "Cycl. BGO "<<cycl->GetID()<<"</a>"<<endl;
        stab.NewCell("",4,1);
        *out<<(cycl->GetStartBX()>0?emph:deflt)
            <<"StartBX: "<<cycl->GetStartBX()<<NOCOL
            <<"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
            <<(cycl->GetPrescale()>0?emph:deflt)
            <<"Prescale: "<<cycl->GetPrescale()<<NOCOL
            <<"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
            <<(cycl->GetInitialPrescale()>0?emph:deflt)
            <<"InitWait: "<<cycl->GetInitialPrescale()
            <<" orbits"<<NOCOL
            <<"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"<<endl;
        *out << "Channel: "<<emph<<cycl->GetBChannel()<<NOCOL<<" (\""
             <<emph
             <<TTCciPtr_->GetBChannel(cycl->GetBChannel())->GetName()
             <<NOCOL<<"\")";
        *out <<"<br>"<<endl;
        *out <<(cycl->GetPostscale()>0?emph:deflt)
             <<cycl->GetPostscale()<<" &times;"<<NOCOL
             <<"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
             <<(cycl->GetPause()>0?emph:deflt)
             <<"Pause: "<<cycl->GetPause()<<" orbits"<<NOCOL
             <<"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
             <<(cycl->IsRepetitive()?deflt:emph)
             <<(cycl->IsRepetitive()?"(repetitive)":"(non-repetitive)")
             <<"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
             <<(cycl->IsPermanent()?(cycl->GetBChannel()!=1?alert:emph):deflt)
             <<(cycl->IsPermanent()?"(permanent)":"(non-permanent)")
             <<NOCOL<<endl;
      }
    }
    
    // BGO Channels:
    size_t nch=0;
    for (size_t i=0; i<TTCciPtr_->NChannels(); ++i){
      BChannel *bch = TTCciPtr_->GetBChannel(i);
      for (size_t j=0; j<bch->NWords(); ++j){
        if (!bch->IsLastWord(j)){
          ++nch;
          break;
        }
      }
    }
    if (nch>0){
      stab.NewRow();
      stab.NewCell("middle lightgreen",1,nch+1);
        *out << "<a href=\""<<url<<"BGOConfiguration\">";
      *out << "BGO</a>"<<endl;
      
      for (size_t i=0; i<TTCciPtr_->NChannels(); ++i){
        BChannel *bch = TTCciPtr_->GetBChannel(i);
        if (bch->NWords()<=1) continue;
        stab.NewRow();
        stab.NewCell("top",1,1);
        *out << BOLD<<"Ch "<<emph<<i<<NOCOL<<" - "<<emph
             <<bch->GetName()<<NOCOL
             <<NOBOLD<<endl;
        const uint32_t rmc = 
          TTCciPtr_->Read(TTCciAdd::CHRMC,i,"(PixelTTCSupervisor)");
        *out << "<br>N<sub>requested</sub> = "<<((rmc>>0)&0xffff)<<endl;
        const uint32_t cancelcntr = ((rmc>>16)&0x3ff);
        *out << "<br>"<<endl;
        if (cancelcntr>0) *out << RED;
        *out << "N<sub>cancelled</sub> = "<<cancelcntr<<endl;
        if (cancelcntr>0) *out << NOCOL;
        stab.NewCell();
        *out << UNDERL<<"DATA:"<<NOUNDERL<<endl;
        for (size_t j=0; j<bch->NWords(); ++j){
          if (bch->IsLastWord(j)) continue;
          *out << "<br>"<<MONO<<emph<<"0x"<<hex<<bch->DataWord_D(j)
               <<dec<<NOCOL<<" ";
          *out << (bch->IsACommand(j)?alert+"(A-command)"+NOCOL:
                   (bch->IsShortWord(j)?"(short)":"(long)"))<<endl;
        }
        stab.NewCell();
        *out << (bch->IsRepetitive()?"repetitive":emph+"non-repetitive"+NOCOL)
             <<"<br>"<<endl;
        *out<<(bch->IsSingleCommand()?"SINGLE":
               (bch->IsDoubleCommand()?emph+"DOUBLE"+NOCOL:emph+"BLOCK"+NOCOL))
            <<" cmnd."<<endl;
        //*out << "<br>Delay t<sub>1</sub>: "<<int(bch->GetDelayTime1())<<" BX"<<endl;
        *out <<(bch->GetDelayTime1()>0?emph:deflt)
             << "<br>Delay t1: "
             <<int(bch->GetDelayTime1())<<" BX"<<NOCOL<<endl;
        if (!bch->IsSingleCommand()){
          //*out << "<br>Delay t<sub>2</sub>: "<<int(bch->GetDelayTime2())
          //     <<" BX"<<endl;
          *out <<(bch->GetDelayTime2()>0?emph:deflt)
               << "<br>Delay t2: "<<int(bch->GetDelayTime2())
               <<" BX"<<NOCOL<<endl;
        }
        stab.NewCell();
        *out << (bch->GetPrescale()>0?emph:deflt)
             << "Prescale: "<<bch->GetPrescale()<<NOCOL<<"<br>"<<endl;
        *out << (bch->GetPostscale()>0?emph:deflt)
             << "Postscale: "<<bch->GetPostscale()<<" times<br>"<<NOCOL<<endl;
        *out << (bch->GetInitialPrescale()>0?emph:deflt)
             << "Init. offset: "<<bch->GetInitialPrescale()<<" req."
             <<NOCOL<<endl;
        // B Counters:
        {
          const uint32_t rmc = (TTCciPtr_ ? 
                                     TTCciPtr_->Read(TTCciAdd::CHRMC,i):0);
          //*out << "<br>"<<UNDERL<<"Counters/Status:"<<NOUNDERL<<endl;
          *out << "<br>Last cancelled data: "<<endl;
          if ((rmc>>27)&1){
            *out << RED <<"yes (error!)"<<NOCOL;
          }else{
            *out << "no (o.k.)";
          }
          *out << "<br>RAM: Buffer " 
               << ((rmc>>26)&1 ? 
                   string(UNDERL)+"reached"+string(NOUNDERL) : 
                   string(UNDERL)+"not"+string(NOUNDERL)+" at end")
               <<" end<br>"<<endl;
       }
      }
    }
    
    // Sequences:
    stab.NewRow();
    stab.NewCell("center lightyellow",5,1);
    *out << BOLD<<"<a href=\""<<url<<"Sequences\">"
         <<"Sequences</a>"<<NOBOLD<<endl;
    stab.NewRow();
    for (size_t i=0; i<5; ++i){
      bool enabled=false;
      bool disabled=false;
      stab.NewCell("left",1,1);
      string myseq;
      if (i==1) myseq = "Enable";
      else if (i==2) myseq = "Suspend";
      else if (i==3) myseq = "Resume";
      else if (i==4) myseq = "Halt";
      else myseq = "Configure";
      const Sequence* seq = TTCciPtr_->GetSequence(myseq);
      *out << UNDERL<<BOLD<<"\""<<seq->GetName()<<"\""<<NOBOLD<<NOUNDERL<<endl;
      for (size_t j=0; j<seq->N(); ++j){
        *out << "<br>- "<<MONO<<seq->Get(j)<<NOMONO<<endl;
        // Check if this command makes sense in this particular sequence:
        if (seq->Get(j)=="DisableL1A") {disabled=true;}
        if (seq->Get(j)=="EnableL1A") {enabled=true;}
        if (myseq=="Enable" || myseq=="Resume"){
          if (seq->Get(j)=="DisableL1A"){
            *out << alert<<"<br>&nbsp;&nbsp;disable L1A here?!?"<<NOCOL<<endl;
          }else if(seq->Get(j)=="ResetCounters"){
            *out << alert<<"<br>&nbsp;&nbsp;reset counters here?!?"
                 <<NOCOL<<endl;
          }
        }else if (myseq=="Suspend" || myseq=="Halt"){
          if (seq->Get(j)=="EnableL1A"){
            *out << alert<<"<br>&nbsp;&nbsp;enable L1A here?!?"<<NOCOL<<endl;
          }          
        }
      }
      if(myseq=="Enable" || myseq=="Resume"){
        if (!enabled){
          *out << alert<<"<br>No "<<MONO<<"EnableL1A"<<NOMONO
               <<" here?!?"<<NOCOL<<endl;
        }
      }else if(myseq=="Halt" || myseq=="Suspend"){
        if (!disabled){
          *out << alert<<"<br>No "<<MONO<<"DisableL1A"<<NOMONO
               <<" here?!?"<<NOCOL<<endl;
        }
      }
    }
  
    stab.Close();
  }
 
  {// General INFO
    *out << "<br>"<<BOLD<<"General Info"<<NOBOLD<<"<br>"<<endl;
    *out << "Software: "<<TTCciPtr_->GetVersion()<<"<br>"<<endl;
    if (TTCciPtr_){
      *out << "Firmware: "<<TTCciPtr_->GetFirmwareVersionString()
           <<"<br>"<<endl;
    }
    *out << "TTCciLocation: "<<Location_;
    if (TTCciPtr_) *out<<" (Actual slot = " << TTCciPtr_->GetInfo().location() << ")";
    *out << "<br>"<<endl;
    *out << "BTimeCorrection: "<<int32_t(BTimeCorrection_)<<" BX, "<<endl; 
    *out << "DelayT2Correction: "<<int32_t(DelayT2Correction_)<<" BX"<<endl; 
  }

  WriteDebugTextArray(out);
  PrintHTMLFooter(out);
}



void PixelTTCSupervisor::SendCommand (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception)
{
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  std::string url = LastPage_;

  *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL="<<url<<"\">" <<endl;
  *out << cgicc::h1("New TTCci Configuration File")<<std::endl;

  try 
    {
      // Create a new Cgicc object containing all the CGI data
      cgicc::Cgicc cgi(in);

      // Get a pointer to the environment
      //const cgicc::CgiEnvironment& env = cgi.getEnvironment();

      //cgicc::const_form_iterator e;

      std::string command = cgi["Command"]->getValue();
      //uint32_t myValue = cgi["Value"]->getIntegerValue(0);
      //cgicc::const_file_iterator file=cgi.getFile("ConfigurationFile");

      bool found = false;
      if (command == "CreateTTCciObject"){
        found=true;
        if (!TTCciPtr_){
          CreateTTCciObject(false);
          TTCciPtr_->ReadBGODataFromTTCci(100);
        }else{
          LOG4CPLUS_ERROR(getApplicationLogger(),toolbox::toString("PixelTTCSupervisor::SendCommand(): Unable to execute \"%s\" since TTCciPtr_ is already initialized.", command.c_str()));
        }
      }
      if (command == "ReadCounters"){
        found=true;
        if (1 || fsm_.getCurrentState() == 'E'){
          SendSOAPMessageToPixelTTCSupervisor("userCommand","CommandPar",command);
        }
      }
      if (command == "ResetCounters"){
        found=true;
        if (1 || (fsm_.getCurrentState() == 'R' ||
                  fsm_.getCurrentState() == 'S' )){
          SendSOAPMessageToPixelTTCSupervisor("userCommand","CommandPar",
                                        command);
        }
      }
      if (command == "DumpVMEHistory-All"){
        found=true;
        if ( TTCciPtr_ ){
          SendSOAPMessageToPixelTTCSupervisor("userCommand","CommandPar",
                                      command);
        }else{
          LOG4CPLUS_ERROR(getApplicationLogger(),toolbox::toString("PixelTTCSupervisor::SendCommand(): Unable to execute \"%s\" since TTCciPtr_ not initialized.", command.c_str()));
        }
      }
      if (command == "DumpVMEHistory-WriteOnly"){
        found=true;
        if ( TTCciPtr_ ){
          SendSOAPMessageToPixelTTCSupervisor("userCommand","CommandPar",
                                        command);
        }else{
          LOG4CPLUS_ERROR(getApplicationLogger(),toolbox::toString("PixelTTCSupervisor::SendCommand(): Unable to execute \"%s\" since TTCciPtr_ not initialized.", command.c_str()));
        }
      }
      if (command == "Send VME-BGO"){
        found=true;
        if ( TTCciPtr_ ){
          SendSOAPMessageToPixelTTCSupervisor("userCommand","CommandPar",
                                        command);
        }else{
          LOG4CPLUS_ERROR(getApplicationLogger(),toolbox::toString("PixelTTCSupervisor::SendCommand(): Unable to execute \"%s\" since TTCciPtr_ not initialized.", command.c_str()));
        }
      }
      if (command == "Send BGO-Start" || command == "Send BGO-Stop"){
        found=true;
        if ( TTCciPtr_ ){
          SendSOAPMessageToPixelTTCSupervisor("userCommand","CommandPar",
                                        command);
        }else{
          LOG4CPLUS_ERROR(getApplicationLogger(),toolbox::toString("PixelTTCSupervisor::SendCommand(): Unable to execute \"%s\" since TTCciPtr_ not initialized.", command.c_str()));
        }
      }
      string sdummy;
      if (ttc::FindString(command,"Exec.",sdummy) && 
          ttc::FindString(command,"Sequence",sdummy)){
        found=true;
        if ( TTCciPtr_ ){
          SendSOAPMessageToPixelTTCSupervisor("userCommand","CommandPar",
                                        "Execute Sequence");
        }else{
          LOG4CPLUS_ERROR(getApplicationLogger(),toolbox::toString("PixelTTCSupervisor::SendCommand(): Unable to execute \"%s\" since TTCciPtr_ not initialized.", command.c_str()));
        }
      }
      if (command == "AutoRefresh OFF" || command == "AutoRefresh"){
        found=true;
        AutoRefresh_ = !AutoRefresh_;
      }
      if (command == "ReadBanksFromTTCci"){
        found=true;
        if ( TTCciPtr_ ){
          SendSOAPMessageToPixelTTCSupervisor("userCommand","CommandPar",
                                        command);
        }else{
          LOG4CPLUS_ERROR(getApplicationLogger(),toolbox::toString("PixelTTCSupervisor::SendCommand(): Unable to execute \"%s\" since TTCciPtr_ not initialized.", command.c_str()));
        }
      }
      if (command == "Reset request cntr" || 
          command == "Reset prescale cntr" || 
          command == "Reset cancel cntr" || 
          command == "Reset all 3 counters" || 
          command == "Reset all on all channels"  ){
        found=true;
        if ( TTCciPtr_ ){
          SendSOAPMessageToPixelTTCSupervisor("userCommand","CommandPar",
                                        command);
        }else{
          LOG4CPLUS_ERROR(getApplicationLogger(),toolbox::toString("PixelTTCSupervisor::SendCommand(): Unable to execute \"%s\" since TTCciPtr_ not initialized.", command.c_str()));
        }
      }
      if (command == "Read BGO configuration from TTCci" ){
        found=true;
        if ( TTCciPtr_ ){
          SendSOAPMessageToPixelTTCSupervisor("userCommand","CommandPar",
                                        command);
        }else{
          LOG4CPLUS_ERROR(getApplicationLogger(),toolbox::toString("PixelTTCSupervisor::SendCommand(): Unable to execute \"%s\" since TTCciPtr_ not initialized.", command.c_str()));
        }
      }
      if (!found){
        LOG4CPLUS_ERROR(getApplicationLogger(),toolbox::toString("PixelTTCSupervisor::SendCommand(): Unknown command \"%s\"", command.c_str()));
      }
    }
  catch(const std::exception& e) 
    {
      *out << "<pre>" << e.what() << "</pre>" << std::endl;

    }
  RedirectMessage(out,url);
}

void PixelTTCSupervisor::MainConfigCommand (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception)
{
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  std::string url = LastPage_;

  *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL="<<url<<"\">" <<endl;
  *out << cgicc::h1("New TTCci Config Page")<<std::endl;

  try 
    {
      // Create a new Cgicc object containing all the CGI data
      cgicc::Cgicc cgi(in);

      // Get a pointer to the environment
      //const cgicc::CgiEnvironment& env = cgi.getEnvironment();

      //cgicc::const_form_iterator e;

      //std::string command = cgi["Command"]->getValue();
      //uint32_t myValue = cgi["Value"]->getIntegerValue(0);
      //double myValue = cgi["Value"]->getDoubleValue();
      
      // Trigger Sources
      std::string value = cgi["triggersource"]->getValue();
      if (value.size()==0){
          LOG4CPLUS_ERROR(getApplicationLogger(),
                          toolbox::toString("PixelTTCSupervisor::MainConfigCommand(): Unable to find \"%s\"!", "triggersource"));
      }else{
        TriggerSourceList.SetDefault(value);
        if (value=="other"){
          std::vector<string> all = TriggerSourceList_other.AllVector();
          for (size_t jk=0; jk<all.size(); ++jk){
            if (cgi.queryCheckbox((string("triggersource2__")+all[jk]).c_str())){
              TriggerSourceList_other.Check(all[jk]);
            }else{
              TriggerSourceList_other.UnCheck(all[jk]);
            }
          }
          if (TTCciPtr_->CanSetInternalTriggerFrequency()){
            double myValue = cgi["TriggerFrequency"]->getDoubleValue(0.0);
            bool myRandom = cgi.queryCheckbox("TriggerRandom");
            if (myValue > 0.0 && myValue<40000000.0){
              if (oldfrequency_ != myValue || myRandom != TTCciPtr_->GetInternalTriggerRandom()){
                TTCciPtr_->SetInternalTriggerFrequency(myValue, myRandom);
                TTCciPtr_->WriteTriggerDelaysToTTCci();
                oldfrequency_ = myValue;
              }
            }else{
              LOG4CPLUS_ERROR(getApplicationLogger(),
                              toolbox::toString("PixelTTCSupervisor::MainConfigCommand(): Unable to extract trigger frequency from \"%s\"!", cgi["TriggerFrequency"]->getValue().c_str()));
            }
          }
        }
      }

      // Clock Sources
      value = cgi["clocksource"]->getValue();
      if (value.size()==0){
          LOG4CPLUS_ERROR(getApplicationLogger(),
                          toolbox::toString("PixelTTCSupervisor::MainConfigCommand(): Unable to find \"%s\"!", "clocksource"));
      }else{
        ClockSourceList.SetDefault(value);
        html << "value="<<value<<" --> Changed clock source to "
              <<ClockSourceList.GetDefault()<<endl;
      }

      // BGO Sources
      value = cgi["bgosource"]->getValue();
      if (value.size()==0){
          LOG4CPLUS_ERROR(getApplicationLogger(),
                          toolbox::toString("PixelTTCSupervisor::MainConfigCommand(): Unable to find \"%s\"!", "bgosource"));
      }else{
        BGOSourceList.SetDefault(value);
        html << "value="<<value<<" --> Changed BGO source to "
              <<BGOSourceList.GetDefault()<<endl;
        if (value=="other"){
          std::vector<string> all = BGOSourceList_other.AllVector();
          for (size_t jk=0; jk<all.size(); ++jk){
            if (cgi.queryCheckbox((string("bgosource2__")+all[jk]).c_str())){
              BGOSourceList_other.Check(all[jk]);
            }else{
              BGOSourceList_other.UnCheck(all[jk]);
            }
          }
        }
      }

      // Orbit Sources
      value = cgi["orbitsource"]->getValue();
      if (value.size()==0){
          LOG4CPLUS_ERROR(getApplicationLogger(),
                          toolbox::toString("PixelTTCSupervisor::MainConfigCommand(): Unable to find \"%s\"!", "orbitsource"));
      }else{
        OrbitSourceList.SetDefault(value);
        html << "value="<<value<<" --> Changed orbit source to "
              <<OrbitSourceList.GetDefault()<<endl;
      }

      // QPLL
      /*
      cgicc::const_form_iterator qpllEnableRadio=cgi.getElement("QPLLEnable");
      if (qpllEnableRadio->getValue() == "ON"){
        TTCciPtr_->EnableQPLL();
      }else{
        TTCciPtr_->DisableQPLL();
      }
      */
      {
        vector<string> tvec=TriggerSourceList_other.CheckedVector();
        vector<string> bvec=BGOSourceList_other.CheckedVector();
        if (INTERFACE(TriggerSourceList.GetDefault()) == ttc::CTC ||
            INTERFACE(TriggerSourceList.GetDefault()) == ttc::LTCIN){
          tvec.clear();
          tvec.push_back(TriggerSourceList.GetDefault());
        }
        if (INTERFACE(BGOSourceList.GetDefault()) == ttc::CTC ||
            INTERFACE(BGOSourceList.GetDefault()) == ttc::LTCIN){
          bvec.clear();
          bvec.push_back(BGOSourceList.GetDefault());
        }
        TTCciPtr_->SelectInputs(ClockSourceList.GetDefault(),
                                OrbitSourceList.GetDefault(),
                                tvec, bvec);
      }

      bool internalclock=ClockSourceList.GetDefault()==INAME(ttc::INTERNAL);
      html << "internalclock="
            <<(internalclock?"TRUE":"FALSE (i.e. external clock)")<<endl;
      if (internalclock){
        TTCciPtr_->DisableQPLL();
      }else{
        TTCciPtr_->EnableQPLL();
      }
      if (!internalclock){
        cgicc::const_form_iterator qpllEnableRadio = 
          cgi.getElement("QPLLReset");
        if (qpllEnableRadio->getValue() == "YES"){
          TTCciPtr_->ResetQPLL(true);
        }else{
          TTCciPtr_->ResetQPLL(false);
        }
        qpllEnableRadio = cgi.getElement("QPLLAutoRestart");
        if (qpllEnableRadio->getValue() == "ON"){
          TTCciPtr_->AutoRestartQPLL(true);
        }else{
          TTCciPtr_->AutoRestartQPLL(false);
        }
      }
      uint32_t freqbits;
      stringstream g(cgi["freqbits"]->getValue()); g>>hex>>freqbits;
      //sscanf(cgi["freqbits"]->getValue().c_str(),"%lx",&freqbits);
      TTCciPtr_->SetQPLLFrequencyBits(freqbits,!internalclock);
    }
  catch(const std::exception& e) 
    {
      *out << "<pre>" << e.what() << "</pre>" << std::endl;

    }
  RedirectMessage(out,url);
}

void PixelTTCSupervisor::TriggerRules (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception)
{
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  std::string url = LastPage_;

  *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL="<<url<<"\">" <<endl;

  try 
    {
      // Create a new Cgicc object containing all the CGI data
      cgicc::Cgicc cgi(in);

      // Get a pointer to the environment
      //const cgicc::CgiEnvironment& env = cgi.getEnvironment();

      //cgicc::const_form_iterator e;

      //std::string command = cgi["Command"]->getValue();
      //uint32_t myValue = cgi["Value"]->getIntegerValue(0);
      //double myValue = cgi["Value"]->getDoubleValue();
      
      if (!TTCciPtr_){
        return;
      }
      
      // Trigger Rules
      for (size_t i=TTCciPtr_->FirstTriggerRule(); 
           i<TTCciPtr_->TriggerRuleSize(); ++i){
        //char dum1[50];
        //sprintf(dum1,"trigrule  %u",i);
        uint32_t myValue = cgi[("trigrule  "+UnsignedLong2String(i)).c_str()]->getIntegerValue(0);
        TTCciPtr_->SetTriggerRule(i,myValue);
      }
    }
  catch(const std::exception& e) 
    {
      *out << "<pre>" << e.what() << "</pre>" << std::endl;

    }
  RedirectMessage(out,url);
}

void PixelTTCSupervisor::RegisterAccessCommand (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception)
{
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  std::string url = "/";
  url += getApplicationDescriptor()->getURN()+"/"+LastPage_;

  //*out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL="<<url<<"\">"<<endl;
  *out << cgicc::h1("VME Read/Write Results")<<std::endl;


  string oldurl = "/"+getApplicationDescriptor()->getURN()+"/";
  *out << BOLD<<BIGGER;
  *out << "<a href=\""<<oldurl<<"RegisterAccess\"> Back to previous page </a>";
  *out << NOBIGGER<<NOBOLD<<"<br>"<<endl;

  try 
    {
      // Create a new Cgicc object containing all the CGI data
      cgicc::Cgicc cgi(in);

      // Get a pointer to the environment
      //const cgicc::CgiEnvironment& env = cgi.getEnvironment();

      //cgicc::const_form_iterator e;

      //std::string command = cgi["Command"]->getValue();
      //uint32_t myValue = cgi["Value"]->getIntegerValue(0);
      //double myValue = cgi["Value"]->getDoubleValue();
      
      const std::string parName = cgi["Addresses"]->getValue();
      //const std::string par2 = cgi["arrayAddresses"]->getValue();
      if (parName != "none"){
        //const std::string parName = par1;
        bool read = cgi.queryCheckbox("readchecked");
        bool write = cgi.queryCheckbox("writechecked");
        const uint32_t index = cgi["index"]->getIntegerValue(0);
        uint32_t myValue;
        stringstream g(cgi["Value"]->getValue());
        g>>hex>>myValue;
        //sscanf(cgi["Value"]->getValue().c_str(),"%lx",&myValue);
        if (read || write) *out << "<ul>"<<endl;
        if (read){
          *out << "<li>"<<endl;
          *out << "Reading from VME address with name '"<<MONO<<parName
               <<NOMONO<<"', index="<<dec<<index<<": <br>"<<endl;
          uint32_t output = TTCciPtr_->Read(parName, index);
          *out << MONO<<BLUE<<"0x"<<hex<<output<<dec<<" = "<<output
               <<NOCOL<<NOMONO<<endl;
          if (write){
            *out << BLUE << " (before)."<<NOCOL<<endl;
          }
          *out << "</li>"<<endl;
        }
        if (write){
          *out << "<li>"<<endl;
          *out << "Writing 0x"<<hex<<myValue<<dec<<" = "<<myValue
               <<" to VME address with name '"<<MONO<<parName<<NOMONO
               <<"', index="<<dec<<index<<endl;
          TTCciPtr_->Write(parName,index,myValue);
          *out << "</li>"<<endl;
          if (read){
            *out << "<li>"<<endl;
            *out << "Reading (again) from VME address with name '"
                 <<MONO<<parName<<NOMONO
                 <<"', index="<<dec<<index<<": <br> "<<endl;
            uint32_t output = TTCciPtr_->Read(parName,index);
            *out << MONO<<BLUE<<"0x"<<hex<<output<<dec<<" = "<<output
                 <<NOMONO<<NOCOL<<endl
                 << BLUE << " (after)."<<NOCOL<<endl;
            *out << "</li>"<<endl;
          }
        }
        if (read || write) *out << "</ul>"<<endl;
        
      }
//       if (par2 != "none"){
//         const std::string parName = par2;
//         ttc::AddressArray myadd=ttc::AddressArray::fromName(parName);
//         bool read = cgi.queryCheckbox("read2checked");
//         bool write = cgi.queryCheckbox("write2checked");
//         const uint32_t index = cgi["index"]->getIntegerValue(0);
//         uint32_t myValue;
//         sscanf(cgi["Value2"]->getValue().c_str(),"%lx",&myValue);
//         if (read || write) *out << "<ul>"<<endl;
//         if (read){
//           *out << "<li>"<<endl;
//           *out << "Reading from VME address with name '"<<MONO<<parName
//                <<NOMONO<<"', index="<<dec<<index<<": <br>"<<endl;
//           //uint32_t output = TTCciPtr_->Read(parName,index);
//           uint32_t output = TTCciPtr_->Read(myadd,index);
//           *out << MONO<<BLUE<<"0x"<<hex<<output<<dec<<" = "<<output
//                <<NOCOL<<NOMONO<<endl;
//           if (write){
//             *out << BLUE << " (before)."<<NOCOL<<endl;
//           }
//           *out << "</li>"<<endl;
//         }
//         if (write){
//           *out << "<li>"<<endl;
//           *out << "Writing 0x"<<hex<<myValue<<dec<<" = "<<myValue
//                <<" to VME address with name '"<<MONO<<parName<<NOMONO
//                <<"', index="<<dec<<index<<endl;
//           TTCciPtr_->Write(myadd,index,myValue);
//           *out << "</li>"<<endl;
//           if (read){
//             *out << "<li>"<<endl;
//             *out << "Reading (again) from VME address with name '"
//                  <<MONO<<parName<<NOMONO
//                  <<"': <br> "<<endl;
//             uint32_t output = TTCciPtr_->Read(myadd,index);
//             *out << MONO<<BLUE<<"0x"<<hex<<output<<dec<<" = "<<output
//                  <<NOMONO<<NOCOL<<endl;
//             if (write){
//               *out << BLUE << " (after)."<<NOCOL<<endl;
//             }
//             *out << "</li>"<<endl;
//           }
//         }
//         if (read || write) *out << "</ul>"<<endl;
        
//       }

    }
  catch(const std::exception& e) 
    {
      *out << "<pre>" << e.what() << "</pre>" << std::endl;

    }

  *out << BOLD<<BIGGER;
  *out << "<a href=\""<<oldurl<<"RegisterAccess\"> Back to previous page </a>";
  *out << NOBIGGER<<NOBOLD<<"<br>"<<endl;
}

void PixelTTCSupervisor::VMEBData (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception)
{
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  std::string url = LastPage_;

  *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL="<<url<<"\">"<<endl;
  //*out << cgicc::h1("VME Read/Write Results")<<std::endl;

  cout << "[INFO] : PixelTTCSupervisor::VMEBData() called;"<<endl;

  try 
    {
      // Create a new Cgicc object containing all the CGI data
      cgicc::Cgicc cgi(in);

      // Get a pointer to the environment
      //const cgicc::CgiEnvironment& env = cgi.getEnvironment();

      //cgicc::const_form_iterator e;

      //std::string command = cgi["Command"]->getValue();
      //uint32_t myValue = cgi["Value"]->getIntegerValue(0);
      //double myValue = cgi["Value"]->getDoubleValue();

      cgicc::const_form_iterator debugBdataRadio=cgi.getElement("debugshort");
      _debugb_short = (debugBdataRadio->getValue() == "yes" ? true : false);
      
      //sscanf(cgi["vmebdata"]->getValue().c_str(),"%lx",&_debugbdata);
      stringstream g(cgi["vmebdata"]->getValue()); g>>hex>>_debugbdata;
      if (_debugb_short){
        TTCciPtr_->Write(TTCciAdd::VMEDATS, _debugbdata, "(VMEDATS)");
      }else{
        TTCciPtr_->Write(TTCciAdd::VMEDATL, _debugbdata, "(VMEDATL)");
      }      
    }
  catch(const std::exception& e) 
    {
      *out << "<pre>" << e.what() << "</pre>" << std::endl;
    }
  RedirectMessage(out,url);
}


void PixelTTCSupervisor::BGOConfigCommand (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception)
{
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  std::string url = LastPage_;

  *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL="<<url<<"\">" <<endl;
  *out << cgicc::h1("New TTCci Config Page")<<std::endl;

  try 
    {
      // Create a new Cgicc object containing all the CGI data
      cgicc::Cgicc cgi(in);

      // Get a pointer to the environment
      //const cgicc::CgiEnvironment& env = cgi.getEnvironment();

      //cgicc::const_form_iterator e;

      //std::string command = cgi["Command"]->getValue();
      //uint32_t myValue = cgi["Value"]->getIntegerValue(0);
      //double myValue = cgi["Value"]->getDoubleValue();
      //bool repetitive = cgi.queryCheckbox("repetitive"); // checkbox
      //cgicc::const_form_iterator dum=cgi.getElement("myname"); // radio b.
      //   dum->getValue(); // radio button value

      ttc::BChannel *bch = TTCciPtr_->GetBChannel(CurrentBGO);

      int err = 0;
      
      // BGO Command Stuff: 
      for (size_t n=0; n<bch->NWords(); ++n){
        char dummy[30];
        sprintf(dummy,"delete%d",n);
        bool deletethis = cgi.queryCheckbox(dummy);
        if (deletethis){
          bch->DeleteWord(n);
          TTCciPtr_->WriteBGODataToTTCci(CurrentBGO);
        }
      }
      string cnewdata = cgi["newdata"]->getValue();
      //"datawordselect"
      string iword = cgi["datawordselect"]->getValue();
      string wdtype = cgi["datatypeselect"]->getValue();
      if (cnewdata.size()>0 && iword != " "){
        uint32_t newdata=0;// = cgi["newdata"]->getIntegerValue(0);
        //if (1 != sscanf(cnewdata.c_str(),"%lx",&newdata)){
        stringstream g(cnewdata);
        if (!(g>>hex>>newdata)){
          html << "ERROR: PixelTTCSupervisor::BGOConfigCommand(): Invalid new data '"
                << newdata<<"' (hex) ! "<<endl;
          LOG4CPLUS_ERROR(getApplicationLogger(),toolbox::toString("PixelTTCSupervisor::BGOConfigCommand(): Invalid new data '%s'!", cnewdata.c_str()));
        }else{
          const bool isACommand=(wdtype=="A-Command" ? true : false);
          const bool islong = ((!isACommand && wdtype=="Long") ? true : false);
          const bool isLast = false, notransmit = false;
          cout << "INFO: wdtype='"<<wdtype<<"' --> isACommand="<<isACommand<<" islong="<<islong<<endl;
          if (iword == "New"){
            bch->PushBackData(newdata, !islong, isACommand, 
                              isLast, notransmit);
            TTCciPtr_->WriteBGODataToTTCci(CurrentBGO);
          }else{
            size_t iw=0;
            if (sscanf(iword.c_str(),"%u",&iw)==1 && iw<bch->NWords()){
              bch->SetData(size_t(iw),newdata, !islong, isACommand,
                           isLast, notransmit);
              TTCciPtr_->WriteBGODataToTTCci(CurrentBGO);
            }else{
              html << "ERROR: PixelTTCSupervisor::BGOConfigCommand(): Invalid word index "<< iw <<" ('"<<iword<<"')! Nword = "<<bch->NWords()<<endl;
              LOG4CPLUS_ERROR(getApplicationLogger(),toolbox::toString("PixelTTCSupervisor::BGOConfigCommand(): Invalid word index '%s'!", iword.c_str()));              
            }
          }
        }
      }

      // The main config: 
      bool repetitive = cgi.queryCheckbox("repetitive");
      bool freerunning = cgi.queryCheckbox("freerunning");
      bool timewrtorbit = cgi.queryCheckbox("orbittiming");
      cgicc::const_form_iterator commandlength=cgi.getElement("commandlength");
      ttc::BGODataLength L = ttc::SINGLE;
      if (commandlength->getValue() == "single"){
        L = ttc::SINGLE;
      }else if(commandlength->getValue() == "double"){
        L = ttc::DOUBLE;
      }else if (commandlength->getValue() == "block"){
        L = ttc::BLOCK;
      }else{
        ++err;
        html << "ERROR: PixelTTCSupervisor::BGOConfigCommand(): "
              <<"Unknown data length '"
              << commandlength->getValue()<<"'!"<<endl;
        LOG4CPLUS_ERROR(getApplicationLogger(),toolbox::toString("PixelTTCSupervisor::BGOConfigCommand(): Unknown data length '%s'", commandlength->getValue().c_str()));
      }
      uint32_t delayTime1 = cgi["delay1"]->getIntegerValue(0);
      uint32_t delayTime2 = cgi["delay2"]->getIntegerValue(0);
      uint32_t prescale  = cgi["prescale"]->getIntegerValue(0);
      uint32_t postscale = cgi["postscale"]->getIntegerValue(0);
      uint32_t initprescale = cgi["offset"]->getIntegerValue(0);
      if (err == 0){
        html << "PixelTTCSupervisor::BGOConfigCommand(): New config for channel"
              <<CurrentBGO<<":"<<endl
              << "   L="<<commandlength->getValue()
              << ", "<< (repetitive?"repetitive":"non-repetitive")
              << ", "<< (freerunning?"free running":"strobe running")
              << ", timing w.r.t "<< (!timewrtorbit?"STROBE":"ORBIT")<<endl;
        bch->Set(L, repetitive, /* freerunning, !timewrtorbit,*/ 
                 delayTime1,delayTime2);
        html << "   -> CHIHB-"<<CurrentBGO<<" = 0x"
              <<hex<< bch->InhibitDelayWord()<<dec<<endl;
        TTCciPtr_->Write(TTCciAdd::CHIHB, CurrentBGO, bch->InhibitDelayWord_corr(),
                         "(inhib/del word)");
        bch->SetPreAndInitPrescale(prescale,initprescale);
        TTCciPtr_->Write(TTCciAdd::CHPRESC, CurrentBGO, 
                         bch->PrescaleWord(),"(prescale)");
        bch->SetPostscale(postscale);
        TTCciPtr_->Write(TTCciAdd::CHPOSTSC, CurrentBGO, 
                         bch->PostscaleWord(),"(postscale)");
      }else{
        html << "ERROR: PixelTTCSupervisor::BGOConfigCommand(): Unable to change "
              << "configuration for channel "<<dec<<CurrentBGO;
        LOG4CPLUS_ERROR(getApplicationLogger(),toolbox::toString("PixelTTCSupervisor::BGOConfigCommand(): Unable to change config for channel %u", CurrentBGO));
        return;
      }
    }
  catch(const std::exception& e) 
    {
      *out << "<pre>" << e.what() << "</pre>" << std::endl;

    }
  RedirectMessage(out,url);
}

void PixelTTCSupervisor::CyclicConfigCommand (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception)
{
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  std::string url = LastPage_;

  *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL="<<url<<"\">" <<endl;

  try 
    {
      // Create a new Cgicc object containing all the CGI data
      cgicc::Cgicc cgi(in);

      // Get a pointer to the environment
      //const cgicc::CgiEnvironment& env = cgi.getEnvironment();

      //cgicc::const_form_iterator e;

      //std::string command = cgi["Command"]->getValue();
      //uint32_t myValue = cgi["Value"]->getIntegerValue(0);
      //double myValue = cgi["Value"]->getDoubleValue();
      //bool repetitive = cgi.queryCheckbox("repetitive"); // checkbox
      //cgicc::const_form_iterator dum=cgi.getElement("myname"); // radio b.
      //   dum->getValue(); // radio button value

      // loop over all cycl generators:
      for (size_t ik=0; ik < (TTCciPtr_->NCyclicBGO()+
                              TTCciPtr_->NCyclicTrigger()); ++ik){
        char genc[10]; sprintf(genc,"%u__",ik);
        const string sgen=string(genc);
        bool trigger = (ik >= TTCciPtr_->NCyclicBGO());
        const size_t i = (!trigger ? ik : ik-TTCciPtr_->NCyclicBGO());
        CyclicTriggerOrBGO *cycl = TTCciPtr_->GetCyclic(trigger, i);
        
        cycl->Print();
        
        // start-BX
        uint32_t val=cgi[(sgen+"startbx").c_str()]->getIntegerValue(0);
        cycl->SetStartBX(val);
        // prescale
        val=cgi[(sgen+"prescale").c_str()]->getIntegerValue(0);
        cycl->SetPrescale(val);
        // initprescale
        val=cgi[(sgen+"initprescale").c_str()]->getIntegerValue(0);
        cycl->SetInitialPrescale(val);
        // Postscale
        val=cgi[(sgen+"postscale").c_str()]->getIntegerValue(0);
        cycl->SetPostscale(val);
        // Pause
        val=cgi[(sgen+"pause").c_str()]->getIntegerValue(0);
        cycl->SetPause(val);
        // Select the B-Channel:
        if (!trigger){
          string channel = cgi[(sgen+"channelno").c_str()]->getValue();
          val = cgi[(sgen+"channelno").c_str()]->getIntegerValue();
          cycl->SetBChannel(val);
        }
        // repetitive
        bool boolval = cgi.queryCheckbox((sgen+"repetitive").c_str());
        cycl->SetRepetitive(boolval);
        // repetitive
        boolval = cgi.queryCheckbox((sgen+"permanent").c_str());
        cycl->SetPermanent(boolval);
        // enable
        cgicc::const_form_iterator EnableRadio = 
          cgi.getElement((sgen+"enable").c_str());
        cycl->SetEnable((EnableRadio->getValue() == "enable"));

        TTCciPtr_->WriteCyclicGeneratorToTTCci(trigger, i);
      }
    }
  catch(const std::exception& e) 
    {
      *out << "<pre>" << e.what() << "</pre>" << std::endl;

    }
  RedirectMessage(out,url);
}

void PixelTTCSupervisor::BGOSelectCommand (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception)
{
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  std::string url = LastPage_;

  *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL="<<url<<"\">" <<endl;
  *out << cgicc::h1("New TTCci Config Page")<<std::endl;

  try 
    {
      // Create a new Cgicc object containing all the CGI data
      cgicc::Cgicc cgi(in);

      // Get a pointer to the environment
      //const cgicc::CgiEnvironment& env = cgi.getEnvironment();

      //cgicc::const_form_iterator e;

      //std::string command = cgi["Command"]->getValue();
      //uint32_t myValue = cgi["Value"]->getIntegerValue(0);
      //double myValue = cgi["Value"]->getDoubleValue();
      

      // The BGO Selection: which channel? 
      uint32_t mybgo = cgi["BGOselect"]->getIntegerValue(0);
      if (mybgo < TTCciPtr_->NChannels()){
        if (mybgo != CurrentBGO){
          CurrentBGO = mybgo;
          CurrentBGOList.SetDefault(CurrentBGO);
        }
      }else{
        ; // error
      }
    }
  catch(const std::exception& e) 
    {
      *out << "<pre>" << e.what() << "</pre>" << std::endl;

    }
  RedirectMessage(out,url);
}

void PixelTTCSupervisor::SequenceSelectCommand (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception)
{
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  std::string url = LastPage_;

  *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL="<<url<<"\">" <<endl;

  try 
    {
      // Create a new Cgicc object containing all the CGI data
      cgicc::Cgicc cgi(in);

     // The Sequence Selection: which sequence? 
      string seqname = cgi["sequenceselect"]->getValue();
      string seqchoice = cgi["seqchoice"]->getValue();
      if (seqchoice=="remove"){
        if (!TTCciPtr_->DeleteSequence(seqname)){
          any_errors_ = 1;
          err_message_ << RED << BOLD <<"ERROR:"<<NOBOLD
                        <<" Errors/warnings detected during attempt to delete sequence \""
                        <<seqname<<"\"!"<<NOCOL<<"<br>"<<endl;
          err_message_ << RED <<"&nbsp;&nbsp;&nbsp; --> Read output (below) for more details."
                        <<NOCOL<<"<br>"<<endl;
        }
      }else if (seqchoice=="add"){
        string newname = cgi["newsequence"]->getValue();
        if (newname.size()>0){
          TTCciPtr_->AddSequence(newname);
          SequenceSelector.push_back(newname);
          SequenceSelector.SetDefault(newname);
        }
      }else{
        SequenceSelector.SetDefault(seqname);
      }
    }
  catch(const std::exception& e) 
    {
      *out << "<pre>" << e.what() << "</pre>" << std::endl;

    }
  RedirectMessage(out,url);
}

void PixelTTCSupervisor::SequenceEditCommand (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception)
{
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  std::string url = LastPage_;

  *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL="<<url<<"\">" <<endl;

  try 
    {
      // Create a new Cgicc object containing all the CGI data
      cgicc::Cgicc cgi(in);

      // which sequence line (or eppend)? 
      string line = cgi["sequenceline"]->getValue();
      
      // which action to take? 
      string edit = cgi["edit"]->getValue();

      // which action to take? 
      string command = cgi["edit2"]->getValue();
      string command2 = cgi["edit3"]->getValue();

      // which action to take? 
      string xstring = cgi["x"]->getValue();
      int32_t x = -99;
      //if (sscanf(xstring.c_str(),"%ld",&x)!=1) x=-99;
      stringstream g(xstring); 
      if (!(g>>x)) x=-99;

      const string SeqTitle = SequenceSelector.GetDefaultTitle();
      Sequence* myseq = TTCciPtr_->GetSequence(SeqTitle);  
      if (!myseq){
        html << "ERROR! PixelTTCSupervisor::SequenceEditCommand():No sequence "
              <<"found with name '"<<SeqTitle<<"'<br>"<<endl;
        any_errors_ = 1;
        return;
      }
      if (edit=="delete"){
        if (line == "< Append >"){
          html << "ERROR: PixelTTCSupervisor::SequenceEditCommand(): To delete, "
                << "you have to choose a different line other than "
                <<"'< Append >'!" <<endl;
          any_errors_ = 1;
          return;
        }else{
          int index = myseq->GetIndex(line);
          if (index<0) {
            html << "ERROR: PixelTTCSupervisor::SequenceEditCommand(): "
                  << " Unable to find line '"<<line<<"' in sequence '"
                  << SeqTitle <<"'!"<<endl;
            any_errors_ = 1;
            return; 
          }
          myseq->Delete(size_t(index));
        }
      }else if(edit=="modify" || edit=="new"){
        if (command=="Choose" && command2=="Choose"){
          html << "ERROR: PixelTTCSupervisor::SequenceEditCommand(): To '"
                << edit << "' you have to "<<endl
                << "       select a command other than '"<<command<<"'!"<<endl;
          any_errors_ = 1;
          return;
        }
        string newline = command;
        if (command=="Choose"){
          newline=command2;
        }else{
          if (command == "Sleep" || command == "mSleep"  || 
              command == "uSleep" ||
              command == "BGO" ||  command == "Periodic" ||
              command == "SendShortBDATA" ||
              command == "SendLongBDATA"){
            if (x==(-99) || 
                (command=="Sleep" && x<0) || 
                (command=="BGO" && (x<0 || x>int(TTCciPtr_->NChannels())))){
              html << "ERROR: PixelTTCSupervisor::SequenceEditCommand(): "
                    << " Invalid value of x="<<x<<" for command "
                    <<command<<"!"<<endl;
              any_errors_ = 1;
              return;
            }
            newline = newline+" "+xstring;
          }
        }
        if (line=="< Append >"){
          myseq->PushBack(newline);
        }else {
          int index = myseq->GetIndex(line);
          if (index<0) {
            html << "ERROR: PixelTTCSupervisor::SequenceEditCommand(): "
                  << " Unable to find line '"<<line<<"' in sequence '"
                  << SeqTitle <<"'!"<<endl;
            any_errors_ = 1;
            return; 
          }
          if (edit=="new"){
            myseq->Insert((size_t)index, newline);
          }else if (edit=="modify"){
            myseq->Set((size_t)index, newline);
          }else{
            html << "ERROR: PixelTTCSupervisor::SequenceEditCommand(): "
                  << "Unknown action '"<<edit<<"'!"<<endl;
            any_errors_ = 1;
            return;
          }
        }
      }
    }
  catch(const std::exception& e) 
    {
      *out << "<pre>" << e.what() << "</pre>" << std::endl;

    }
  RedirectMessage(out,url);
}

void PixelTTCSupervisor::PrintHTMLHeader(xgi::Output *out, 
                                   const std::string &title) {
  char instance[20], lid[20], slot[20];
  sprintf(instance," : %d",instance_);
  sprintf(lid,"%d",localid_);
  sprintf(slot,"%d",TTCciPtr_?TTCciPtr_->GetInfo().location():(int)Location_);
  xgi::Utils::getPageHeader
    (out, 
     title+" \""+name_.toString()+"\" (Slot="+slot+", lid="+lid+")",
     getApplicationDescriptor()->getContextDescriptor()->getURL(),
     getApplicationDescriptor()->getURN(),
     "/" + getApplicationDescriptor()->getAttribute("icon")
     );
  string url = "/"+getApplicationDescriptor()->getURN()+"/";
  *out << "<table style=\"text-align: left;\" border=\"0\" cellpadding=\"0\"cellspacing=\"2\">" << std::endl;
  *out << "<tbody>"<<endl;
  *out << "<tr>" << std::endl;
  *out << "<td style=\"vertical-align: top;\">"<<endl;
  *out << "[<a href=\""<<url<<"\">TTCci Control</a>] "
    //<< "[<a href=\""<<url<<"ConfigurePage\">Configuration</a>] "
       << "[<a href=\""<<url<<"MainConfiguration\">Main Config</a>] " 
       << "[<a href=\""<<url<<"BGOConfiguration\">BGO Config</a>] " 
       << "[<a href=\""<<url<<"Sequences\">Sequences</a>] " 
       << "[<a href=\""<<url<<"CyclicGenerators\">Cyclic Gen.</a>] " 
       << "[<a href=\""<<url<<"SummaryPage\">Summary</a>] " 
       << "[<a href=\""<<url<<"RegisterAccess\">Registers</a>] " 
       << endl;
  *out << "</td>"<<endl;
  *out << "<td style=\"vertical-align: top;\">"<<endl;
  {// AUTO REFRESH BUTTON:
      std::string cmd_url = "/";
      cmd_url += getApplicationDescriptor()->getURN();
      cmd_url += "/Command";	
      *out << cgicc::form().set("method","get").set("action", cmd_url).set("enctype","multipart/form-data") << std::endl;
      if ( AutoRefresh_ ){
        *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "AutoRefresh OFF");
      }else{
        *out << cgicc::input().set("type", "submit").set("name", "Command").set("value", "AutoRefresh");
      }
      *out << cgicc::form();       
  }
  *out << "</td>"<<endl;
  *out << "</tr>"<<endl;
  *out << "</tbody>"<<endl;
  *out << "</table>"<<endl;

  //*out << "<div style=\"text-align: right;\">Time: " 
  //     << GetCurrentTime()<<"</div>"<<endl;;
}

void PixelTTCSupervisor::PrintHTMLFooter(xgi::Output *out){
  // Footer of Page:
  *out << "<hr style=\"width: 100%; height: 2px;\">"<<endl;
  *out << "<center>"<<endl;
  *out << "<small style=\"font-family: arial;\"> ";
  *out << "TTCci Library "<<TTCci::GetVersion()<<"; <br>"<<endl;
  *out << "Authors: "
       << cgicc::a("Tim Christiansen").set("href","mailto:Tim.Christiansen@cern.ch") << " and "
       <<cgicc::a("Emlyn Corrin").set("href","mailto:Emlyn.Corrin@cern.ch") << "</small>"<<endl;
  *out << "<br>"<<endl;
  *out << cgicc::a("TTCci Manual").set("href","http://cmsdoc.cern.ch/cms/TRIDAS/ttc/modules/ttcci/index.html") << "  "<<endl;
  *out << cgicc::a("XDAQ Web site").set("href","http://xdaqwiki.cern.ch") << endl;
  *out << "</center>"<<endl;

}

void PixelTTCSupervisor::ReadTTCciContent(){
  {
    LOG4CPLUS_INFO(this->getApplicationLogger(), 
                   "Function \"PixelTTCSupervisor::ReadTTCciContent()\" not yet implemented!");
    html<<"WARNING: PixelTTCSupervisor::changeState(): "
         <<"Function \"PixelTTCSupervisor::ReadTTCciContent()\" not yet implemented!"
         <<endl;
  }
  if (TTCciPtr_){
    ;
  }else{
    ;
  }
}

void PixelTTCSupervisor::ConfigureTTCcifromASCI(){
  if (!TTCciPtr_){
    stringstream my;
    my<<"ERROR: PixelTTCSupervisor::ConfigureTTCcifromASCI(): TTCciPtr_ not "
      <<"initialized!";
    LOG4CPLUS_ERROR(getApplicationLogger(), 
                    toolbox::toString("%s", my.str().c_str()));
    throw std::invalid_argument(my.str());
  }

  TTCciPtr_->MainReset(); // reset of counters etc.
  //asciConfigurationFilePath_ = theTTCciConfig_->getTTCConfigPath();
  
  //html << "PixelTTCSupervisor::ConfigureTTCcifromASCI(): Input File ='"<<asciConfigurationFilePath_<<"'."<<endl;
  //ifstream in(asciConfigurationFilePath_.c_str(), ios_base::in);
  stringstream &in = theTTCciConfig_->getTTCConfigStream();
  if (!in){
    stringstream my;
    my<<"ERROR: PixelTTCSupervisor::ConfigureTTCcifromASCI(): unable to "
      <<"open file '"<<asciConfigurationFilePath_<<"'.";
    LOG4CPLUS_ERROR(getApplicationLogger(), 
		    toolbox::toString("%s", my.str().c_str()));
    //throw std::invalid_argument(my.str());
    html << my.str() <<endl;
  }
  any_errors_ = TTCciPtr_->Configure(in);
  if (any_errors_!=0){
    err_message_ << RED << BOLD <<"WARNING:"<<NOBOLD<<" Errors/warnings detected during configuration!"
		 << " Level: \""<<MONO<<(any_errors_<0?"fatal error":"error or warning")
		 <<NOMONO<<"\"<br>"<<endl;
    err_message_ << "&nbsp;&nbsp;&nbsp; --> Read output (below) for more details."
		 <<NOCOL<<"<br>"<<endl;
  }
  ConfigModified_=false;
}

void PixelTTCSupervisor::PrintSMState(xgi::Output *out){
  *out << "<big style=\"font-weight: bold;\">";
  if (fsm_.getCurrentState()=='H') *out << RED;
  else if (fsm_.getCurrentState()=='H') *out << RED;
  else if (fsm_.getCurrentState()=='S'||
           fsm_.getCurrentState()=='R') *out << ORANGE;
  else if (fsm_.getCurrentState()=='E') *out << GREEN;
  else *out << RED;
  *out << fsm_.getStateName(fsm_.getCurrentState());
  //*out << "</span></big>";
  *out << NOCOL <<"</big>"<<endl;
  if (TTCciPtr_ && TTCciPtr_->IsL1ADisabled() && 
      fsm_.getCurrentState()=='E'){
    *out <<RED<< "<br>*) L1A="<<(TTCciPtr_->IsL1AEnabled()?"On":"Off")
         <<" (see below)" <<NOCOL<<endl;
    html << "*) WARNING: PixelTTCSupervisor::PrintSMState(): State="
          <<fsm_.getStateName(fsm_.getCurrentState())<<", but "
          << "EnableL1A not issued!" <<endl
          << "            Have you added 'EnableL1A' to your 'Enable' & "
          << "'Resume' Sequences?"<<endl;
    //any_errors_ = 1;
  }else if (TTCciPtr_ && TTCciPtr_->IsL1AEnabled() && 
            fsm_.getCurrentState()!='E'){
    *out <<RED<< "<br>*) L1A="<<(TTCciPtr_->IsL1AEnabled()?"On":"0ff")
         <<" (see below)" <<NOCOL<<endl;
    html << "*) WARNING: PixelTTCSupervisor::PrintSMState(): State="
          <<fsm_.getStateName(fsm_.getCurrentState())<<", but "
          << "DisableL1A not issued!" <<endl
          << "            Have you added 'DisableL1A' to your 'Suspend' & "
          << "'Halt' Sequences?"<<endl;
    //any_errors_ = 1;
  }
}

void PixelTTCSupervisor::WriteOutputToStream(std::ostream *out){
  size_t hsize=html.str().size();
  *out << "[INFO: Size of HTML-output stream: "<<hsize
       <<", time: "<<GetCurrentTime()<<"]"<<endl<<endl;
  *out << html.str();
  const size_t max_ = 30000;
  if (hsize > max_){
    //html->clear();
    html.str("");
    *out <<endl<<"[Size of HTML-output stream = "<<html.str().size()
         <<" is bigger than max="<<max_<<" --> clearing stream.]"<<endl;
  }
}

void PixelTTCSupervisor::CheckSizeOfHTMLOutput(){
  if (html.str().size() > 100000){
    //html.clear();
    html.str("");
  }  
}

void PixelTTCSupervisor::WriteDebugTextArray(std::ostream *out, bool printSOAP){
  *out << cgicc::h3("(Debug) Output")<<endl;
  *out << "<textarea width=\"100%\" height=\"300px\" style=\"width:100%;height:300px\" name=\"code\" wrap=\"logical\" rows=\"12\" cols=\"42\">"<<endl;
  if (fsm_.getCurrentState() != 'H'){
    neverSOAPed_ = false;
  }
  if (printSOAP && !neverSOAPed_){
    *out << "#################################################################"<<endl;
    *out << "RECORD OF MOST RECENT SOAP MESSAGE RECEIVED:"<<endl;
    soapReceived_->writeTo(*out);
    *out << endl<<"RECORD OF SOAP RESPONSE:"<<endl;
    soapResponse_->writeTo(*out);
    *out << endl;
    *out << "#################################################################"<<endl<<endl;;
  }    
  WriteOutputToStream(out);
  *out << "</textarea>"<<endl;
  
}

void PixelTTCSupervisor::ReadTTCciCounters(){
  if (!TTCciPtr_){
    html << "PixelTTCSupervisor::ReadTTCciCounters(): TTCci uninitialized (TTCciPtr_=0)!"
          << endl;
    LOG4CPLUS_WARN(this->getApplicationLogger(), 
                   toolbox::toString("PixelTTCSupervisor::ReadTTCciCounters(): TTCci uninitialized (TTCciPtr_=0)!"));
    EventCounter_=previousEventCounter_=0;
    OrbitCounter_=previousOrbitCounter_=0;
    StrobeCounter_=previousStrobeCounter_=0;
    BoardStatus_=0;
    tprevious_ = tnow_;
    time(&tnow_);
    return;
  }
  tprevious_ = tnow_;
  time(&tnow_);
  previousEventCounter_=EventCounter_;
  EventCounter_=TTCciPtr_->ReadEventCounter();
  previousOrbitCounter_=OrbitCounter_;
  OrbitCounter_=TTCciPtr_->ReadOrbitCounter();
  previousStrobeCounter_=StrobeCounter_;
  StrobeCounter_=TTCciPtr_->ReadStrobeCounter();
  BoardStatus_=TTCciPtr_->BoardStatus(false);
}

std::string PixelTTCSupervisor::GetCurrentTime(){
  static time_t tnow_;
  tnow_= time(0);
  std::string mytime(ctime(&tnow_));
  if (mytime[mytime.size()-1]=='\n'){
    //cout <<"Cut away the endl... "<<endl;
    mytime[mytime.size()-1]=' '; // better remove it!
  }
  return mytime;
}

void PixelTTCSupervisor::CreateTTCciObject(const bool LoadConfiguration){
  LOG4CPLUS_DEBUG(this->getApplicationLogger(),
                  "PixelTTCSupervisor::CreateTTCciObject(): Creating TTCci ...");
  if (TTCciPtr_){
    LOG4CPLUS_WARN(this->getApplicationLogger(),
                    "PixelTTCSupervisor::CreateTTCciObject(): TTCciPtr_ already exists!");
    //deleteHardware();
    return;
  }
  try
    {
#ifdef SIDET
      busAdapter_ = getCAENLinuxBusAdapter();
#else
      busAdapter_ = new HAL::CAENLinuxBusAdapter(HAL::CAENLinuxBusAdapter::V2718);
      //busAdapter_ = new HAL::CAENLinuxBusAdapter(HAL::CAENLinuxBusAdapter::V1718);
#endif
      TTCciPtr_ = new ttc::TTCci(*busAdapter_,Location_,Is64XCompatible_ , 
                                 BTimeCorrection_, DelayT2Correction_, &html);
      LOG4CPLUS_INFO(this->getApplicationLogger(),
                     toolbox::toString("PixelTTCSupervisor::CreateTTCciObject(): %s (Board-ID %d)",
                                       TTCciPtr_->GetFirmwareVersionString().c_str(),
                                       TTCciPtr_->GetBoardID()));
      LOG4CPLUS_INFO(this->getApplicationLogger(),
                     toolbox::toString("PixelTTCSupervisor::CreateTTCciObject(): Software %s (%.1f)",TTCciPtr_->GetVersion().c_str(),TTCciPtr_->GetVersionNo()));
      if ((TTCciPtr_->GetVersionNo()>=3.0 && 
           TTCciPtr_->GetFirmwareVersion() < 2) || 
          (TTCciPtr_->GetVersionNo()<3.0 && 
           TTCciPtr_->GetFirmwareVersion() >= 2)){
        cout << "*******************************************************"
             <<"*********"<<endl;
        cout <<"WARNING!!! WARNING!!! WARNING!!! WARNING!!! \n"
             <<"Potential of incompatibility between TTCci software and firmware!\n"
             <<"Firmware: "<<TTCciPtr_->GetFirmwareVersionString()<<"\n"
             <<"Software: "<<TTCciPtr_->GetVersion()<<"\n"
             <<"(for firmware version 2 or higher you need software 3.x, \n"
             <<"for all other firmware, you need software 2.x)\n"
             <<"*******************************************************"
             <<"*********"<<endl;
        
      }
      InitHTMLFields();
    }
  catch (toolbox::fsm::exception::Exception & e)
    {
      LOG4CPLUS_ERROR(this->getApplicationLogger(), toolbox::toString("EXCEPTION : %s", e.what()));
      XCEPT_RETHROW(xoap::exception::Exception, "ERROR ", e);
    }

  if (!LoadConfiguration) return;

  if (int(ReloadAtEveryConfigure_)!=1 || ConfigModified_) {
    ConfigureTTCcifromASCI();
  }

}

void PixelTTCSupervisor::InitHTMLFields(){
  { // Trigger Sources:
    vector<string> values_all = TTCciPtr_->GetSourceListNames("Trigger");
    vector<string> values, values_int;
    for (size_t i=0; i<values_all.size(); ++i){
      if (values_all[i]=="CTC" || values_all[i]=="LTC")
        values.push_back(values_all[i]);
      else
        values_int.push_back(values_all[i]);
    }
    vector<string> titles = values;
    values.push_back("other");
    titles.push_back("other / choose from:");
    TriggerSourceList.Set(HTMLFieldElement::RADIOBUTTON, 0,
                          "triggersource", values, titles, 
                          "vertical green bold");
    TriggerSourceList.SetDefault(values[0]);
    TriggerSourceList_other.Set(HTMLFieldElement::CHECKBOX, 0,
                                "triggersource2", values_int, values_int, 
                                "vertical green bold");
  }
  { // Clock Sources:
    vector<string> values = TTCciPtr_->GetSourceListNames("Clock");
    ClockSourceList.Set(HTMLFieldElement::DROPDOWNMENU, 0,
                          "clocksource", values);
    ClockSourceList.SetDefault(values[0]);
  }
  { // BGO Sources:
    vector<string> values_all = TTCciPtr_->GetSourceListNames("BGO");
    vector<string> values, values_int;
    for (size_t i=0; i<values_all.size(); ++i){
      if (values_all[i]=="CTC" || values_all[i]=="LTC")
        values.push_back(values_all[i]);
      else
        values_int.push_back(values_all[i]);
    }
    vector<string> titles = values;
    values.push_back("other");
    titles.push_back("other / choose from:");
    BGOSourceList.Set(HTMLFieldElement::RADIOBUTTON, 0,
                      "bgosource", values, titles, 
                      "vertical green bold");
    BGOSourceList.SetDefault(values[0]);
    //BGOSourceList_other = values_int;
    BGOSourceList_other.Set(HTMLFieldElement::CHECKBOX, 0,
                            "bgosource2", values_int, values_int, 
                            "vertical green bold");
  }
  { // Orbit Sources:
    vector<string> values = TTCciPtr_->GetSourceListNames("Orbit");
    OrbitSourceList.Set(HTMLFieldElement::DROPDOWNMENU, 0,
                          "orbitsource", values);
    OrbitSourceList.SetDefault(values[0]);
  }
  
  { // The 16 BGO Channels
    vector<string> values, titles;
    for (size_t i=0; i<TTCciPtr_->NChannels(); ++i){
      char si[100];
      sprintf(si,"%d",i);
      values.push_back(si);
      string name = TTCciPtr_->GetBChannel(i)->GetName();
      if (name[0]!='C'|| name[1]!='h' || name[2]!='a'){
        name += (string(" (")+string(si)+string(")"));
      }
      titles.push_back(name);
    }
    CurrentBGOList.Set(HTMLFieldElement::DROPDOWNMENU, 0,
                       "BGOselect", values, titles);
    CurrentBGO = 0;
    CurrentBGOList.SetDefault(CurrentBGO);
  }
  { // The Cyclic Generators
    vector<string> values, titles;
    int jj=0;
    for (size_t i=0; i<TTCciPtr_->NCyclicTrigger(); ++i){
      titles.push_back(TTCciPtr_->GetCyclic(true, i)->GetName());
      char tit[10];
      sprintf(tit,"%d",jj++);
      values.push_back(tit);
    }
    for (size_t i=0; i<TTCciPtr_->NCyclicBGO(); ++i){
      titles.push_back(TTCciPtr_->GetCyclic(false, i)->GetName());
      char tit[10];
      sprintf(tit,"%d",jj++);
      values.push_back(tit);
    }
    CurrentCyclicList.Set(HTMLFieldElement::DROPDOWNMENU, 0,
                          "cyclicselect", values, titles);
    CurrentCyclicList.SetDefault(values[0]);
  }
  {// VME registers
    vector<string> add1, add1_tit; //, add2, add2_tit;
    TTCciPtr_->GetVMEAddresses(add1, add1_tit); //, add2, add2_tit);
    add1.insert(add1.begin(), string("none"));
    add1_tit.insert(add1_tit.begin(), string("Choose parameter"));
    //add2.insert(add2.begin(), string("none"));
    //add2_tit.insert(add2_tit.begin(), string("Choose parameter"));
    VMEAddrList1.Set(HTMLFieldElement::DROPDOWNMENU, 0,
                     "Addresses", add1, add1_tit);
    VMEAddrList1.SetDefault("none");
    //VMEAddrList2.Set(HTMLFieldElement::DROPDOWNMENU, 0,
    //                 "arrayAddresses", add2, add2_tit);
    //VMEAddrList2.SetDefault("none");
  }

  {
    vector<string> values, titles;
    values.push_back("Short");     titles.push_back("Short");
    values.push_back("Long");      titles.push_back("Long");
    values.push_back("A-Command"); titles.push_back("A-Cmnd/L1A");
    DataTypeSelector.Set(HTMLFieldElement::DROPDOWNMENU, 0,
                         "datatypeselect", values, titles);
    DataTypeSelector.SetDefault(values[0]);
  }

  {
    const vector<string> values = TTCciPtr_->GetSequenceNames();
    SequenceSelector.Set(HTMLFieldElement::DROPDOWNMENU, 0,
                         "sequenceselect", values, values);
    SequenceSelector.SetDefault(values[0]);
  }

  {
    //toolbox::Event::Reference e(new toolbox::Event(string("Init"), this));
    //fireEvent(e);
    ;
  }

  _debugb_short = true;
  _debugbdata = 0;
}

uint32_t PixelTTCSupervisor::GetBGOWord(size_t channel, size_t iword, 
                                       bool Dword){
  string Name = "CH";
  if (channel<10) Name+="0";
  char cchan[3];
  sprintf(cchan,"%u",channel);
  Name += string(cchan)+(Dword?"D":"P");
  return TTCciPtr_->Read(Name,iword);
}

void PixelTTCSupervisor::ErrorStatement(xgi::Output * out){
  if (TTCciPtr_ && TTCciPtr_->CheckClock()!=INTERNAL && !TTCciPtr_->ClockLocked()){
    any_errors_ = 1;
    err_message_ << RED << BOLD <<"ERROR: "<<NOBOLD
                 << "TTCci Clock NOT locked!!!"
                 <<NOCOL<<"<br>"<<endl;
  }

  if (any_errors_!=0 && err_message_.str().size()==0){
    *out << RED << BOLD <<"WARNING:"<<NOBOLD<<" Errors/warnings detected!"
         << " Level: \""<<MONO<<(any_errors_<0?"fatal error":"error or warning")
         <<NOMONO<<"\"<br>"<<endl;
    *out << "&nbsp;&nbsp;&nbsp; --> Read output (below) for more details."
         <<NOCOL<<"<br>"<<endl;
  }else{
    *out << err_message_.str()<<endl;
  }
  err_message_.str("");
  any_errors_ = 0;
}

std::string PixelTTCSupervisor::GetName(const int opt) const {
  if (opt==0)
    return name_.value_;
  else
    return string(ITALIC)+string(BROWN)+name_.value_+string(NOCOL)+
      string(NOITALIC);
}

void PixelTTCSupervisor::GetFileList(){
  char s[1024];
  gethostname(s, sizeof s);
  string machine = s;
  size_t pos = machine.find_first_of(".");
  if (pos<machine.size())
    machine.erase(machine.begin()+pos, machine.end());
  machine = string(" (on ")+MONO+machine+NOMONO+")";

  string path = "", suffix="";
  std::vector<std::string> filelist, filelistvar;
  if (ReadConfigFromFile_){
    path = asciConfigurationFilePath_;
    filelist.push_back(string(MONO)+path+string(NOMONO)+machine);
    filelistvar.push_back(path);
    size_t pos = path.find_last_of("/");
    if (pos<path.size()){
      suffix = path;
      path.erase(path.begin()+pos, path.end());
      // find the ending
      suffix.erase(suffix.begin(), suffix.begin()+pos+1);
      pos = suffix.find_last_of(".");
      if (pos==0) suffix="";
      else if (pos<suffix.size()) 
        suffix.erase(suffix.begin(), suffix.begin()+pos);
      else{
        cout << "No file ending found in '"<<suffix<<"'"<<endl;
        suffix="";
      }
    }
  }
  std::vector<std::string> addlist = ttc::filelist(path,"",(suffix=".dat"));
  for (size_t i=0; i<addlist.size(); ++i){
    if (filelistvar.size()>0 && addlist[i] == filelistvar[0]) continue;
    filelist.push_back(string(MONO)+addlist[i]+string(NOMONO)+machine);
    filelistvar.push_back(addlist[i]);
  }
  addlist = ttc::filelist(path,"",(suffix=".txt"));
  for (size_t i=0; i<addlist.size(); ++i){
    if (filelistvar.size()>0 && addlist[i] == filelistvar[0]) continue;
    filelist.push_back(string(MONO)+addlist[i]+string(NOMONO)+machine);
    filelistvar.push_back(addlist[i]);
  }
  filelist.push_back("other"+machine+":");
  filelistvar.push_back("other");
  InputFileList.Set(HTMLFieldElement::RADIOBUTTON, 0,
                    "inputfiles", filelistvar, filelist,"vertical bold blue");
  InputFileList.SetDefault(filelistvar[0]);

  //cout << "filelist.size()="<<filelist.size()<<endl;
  //for (size_t i=0; i<filelist.size(); ++i){
  //  cout << i<<")\t'"<<filelist[i]<<"'"<<endl;
  //}
  
}

void PixelTTCSupervisor::ConfigureIcon() {
  string iconbase = std::getenv("BUILD_HOME") + string("/");
  string myicon = STRINGIZE(IMAGE_DIR) "/TTCciIcon.gif";
  string mynewicon = STRINGIZE(IMAGE_DIR) "/TTCciIcon";
  char id[50];
  sprintf(id,"_lid%d_inst%d.gif",localid_,instance_);
  mynewicon += id;
  string FullName = name_.toString();
  vector<string>picname;
  for (size_t i=0; i<FullName.size(); ){
    const size_t begin=i;
    string dum;
    for (size_t j=begin; (j<(begin+5) && j<FullName.size()); ++j){
      if (FullName[j]!=' '){
        dum.push_back(FullName[j]);
      }
      ++i;
      if (j>(begin+1) && (FullName[j]==' ' || FullName[j]=='-' ||
                          FullName[j]=='+' || FullName[j]=='_')) {break;}
    }
    picname.push_back(dum);
  }
  if (picname.size() > 0) {
    const int Size=90; // font size
    stringstream command;
    command << "convert -font helvetica-bold -pointsize "<<Size;
    for (size_t i=0; (i<picname.size()&&i<3); ++i){
      const int xi = 10;
      const int yi = 190+(Size*(3-picname.size()))+Size*i;
      command <<" -fill black -draw 'text "<<(xi+5)<<","<<(yi+5)
              <<" \""<<picname[i]<<"\"' "
              <<" -fill red -draw 'text "<<(xi)<<","<<(yi)
              <<" \""<<picname[i]<<"\"' ";
    }
    command << (iconbase+myicon) << " " << (iconbase+mynewicon);
    system(command.str().c_str());
  }
  if (picname.size() > 0 && ifstream((iconbase+mynewicon).c_str())){
    cout << "Setting custom icon... "<<endl;
    getApplicationDescriptor()->setAttribute("icon",mynewicon);
  }else if (ifstream((iconbase+myicon).c_str())){
    cout << "Setting standard TTCci icon... "<<endl;
    getApplicationDescriptor()->setAttribute("icon",myicon);
  } else {
    cout << "Error finding TTC icon --> using standard XDAQ icon... "<<endl;
  }
}

