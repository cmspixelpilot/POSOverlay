#include "PixelDCSSupervisor/include/PixelDCSSupervisor.h"

/*************************************************************************
 * Supervisor class for integration of Run-Control                       *
 * and Detector Control systems, as described in CMS IN 2005/015;        *
 * sends SOAP messages containing the current state of the topmost       *
 * PVSS FSM node to the PixelSupervisor                                  *
 *                                                                       *
 * Author: Christian Veelken, UC Davis					 *
 *                                                                       *
 * Last update: $Date: 2009/10/20 12:04:17 $ (UTC)                       *
 *          by: $Author: aryd $                                       *
 *************************************************************************/

#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationStub.h"
#include "xgi/Utils.h"
#include "cgicc/HTMLClasses.h"
#include "toolbox/fsm/FailedEvent.h"
#include "xoap/DOMParserFactory.h"
#include "xercesc/dom/DOMDocument.hpp"
#include "xercesc/dom/DOMNodeList.hpp"
#include "xercesc/dom/DOMNode.hpp"

#include "PixelUtilities/PixelGUIUtilities/include/HTML2XGI.h"
#include "PixelUtilities/PixelDCSUtilities/include/readDCSFSMDeviceDefinition.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSSMICommander.h"
#include "PixelUtilities/PixelGUIUtilities/include/PixelAJAXCommander.h"

#define PSX_NS_URI "http://xdaq.cern.ch/xdaq/xsd/2006/psx-pvss-10.xsd"
#define PSX_SMI_NS_URI "http://xdaq.web.cern.ch/xdaq/xsd/2006/psx-smi-10.xsd"

const std::string fsmNodeOwner = "PixelDCSSupervisor";
const bool fsmNodeExclusiveFlag = true;

XDAQ_INSTANTIATOR_IMPL(PixelDCSSupervisor)

PixelDCSSupervisor::PixelDCSSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception) 
  : xdaq::Application(s), SOAPCommander(this)
{
//--- bind SOAP call-back functions 
//    to State Machine Commmands
  xoap::bind(this, &PixelDCSSupervisor::Initialize, "Initialize", XDAQ_NS_URI);
  xoap::bind(this, &PixelDCSSupervisor::Configure, "Configure", XDAQ_NS_URI);
  xoap::bind(this, &PixelDCSSupervisor::Start, "Start", XDAQ_NS_URI);
  xoap::bind(this, &PixelDCSSupervisor::Stop, "Stop", XDAQ_NS_URI);
  xoap::bind(this, &PixelDCSSupervisor::Pause, "Pause", XDAQ_NS_URI);
  xoap::bind(this, &PixelDCSSupervisor::Resume, "Resume", XDAQ_NS_URI);
  xoap::bind(this, &PixelDCSSupervisor::Halt, "Halt", XDAQ_NS_URI);

//--- bind XGI call-back functions 
//    to messages originating from the browser
  xgi::bind(this, &PixelDCSSupervisor::Default, "Default");
  xgi::bind(this, &PixelDCSSupervisor::XgiHandler, "XgiHandler");

//--- bind AJAX handler 
//    to "server" requests originating from the browser
//    (automatically triggered periodically by some Javascript code
//     contained in webpage created in PixelDCSSupervisor::Default method)
  xgi::bind(this, &PixelDCSSupervisor::AjaxHandler, "AjaxHandler");

//--- bind SOAP call-back functions
//    to PVSS FSM state transitions
  xoap::bind(this, &PixelDCSSupervisor::updateFSMState, "notify", PSX_SMI_NS_URI);

//--- define the states of the Finite State Machine
  fsm_.addState('I', "Initial");
  fsm_.addState('H', "Halted");
  fsm_.addState('c', "Configuring");
  fsm_.addState('C', "Configured");
  fsm_.addState('R', "Running");
  fsm_.addState('P', "Paused");
  fsm_.setStateName('F',"Error");

//--- define the transitions of the Finite State Machine
  fsm_.addStateTransition('I', 'H', "Initialize");
  fsm_.addStateTransition('I', 'C', "Initialize");
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
//--- add transitions into Error state
//    (neccessary in case a XDAQ action fails)
  //fsm_.addStateTransition('I', 'F', "invisibleGotoError");
  //fsm_.addStateTransition('H', 'F', "invisibleGotoError");
  //fsm_.addStateTransition('c', 'F', "invisibleGotoError");
  //fsm_.addStateTransition('C', 'F', "invisibleGotoError");
  //fsm_.addStateTransition('R', 'F', "invisibleGotoError");
  //fsm_.addStateTransition('P', 'F', "invisibleGotoError");

//--- set initial state 
//    and reset Finite State Machine
  fsm_.setInitialState('I');
  fsm_.reset();

//--- initialize parameters 
//    defined by environment variables
  XDAQ_ROOT = getenv("XDAQ_ROOT");

//--- initialize parameters 
//    defined by .xml configuration file
  this->getApplicationInfoSpace()->fireItemAvailable("pvssFSMNodeName", &pvssFSMNodeName_);
  this->getApplicationInfoSpace()->fireItemAvailable("pvssFSMNodeType", &pvssFSMNodeType_);
  this->getApplicationInfoSpace()->fireItemAvailable("pvssFSMNodeOwnership", &pvssFSMNodeOwnership_);
  this->getApplicationInfoSpace()->fireItemAvailable("configuration", &configFileName_);

  // WARNING: parameters defined by .xml configuration file
  //          are not initialized yet in constructor;
  //          therefore need to postpone loading of configuration file
  //          (to retrieve FSM state and command Translation Tables 
  //           between XDAQ and PVSS)
  //
  configFileLoaded_ = false;

  xdaq::ApplicationDescriptor* psxDescriptor = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor("psx", 0);

  smiCommander_ = new PixelDCSSMICommander(this, psxDescriptor);

  pvssCurrentFSMState_ = "UNDEFINED";
  pvssCurrentFSMNodeOwnership_ = "manual";
  psxConnectionStatus_ = "disconnected";
}

PixelDCSSupervisor::~PixelDCSSupervisor()
{
  std::cout << "<PixelDCSSupervisor::~PixelDCSSupervisor>:" << std::endl;

//--- release ownership of FSM node 
//    if neccessary
  if ( pvssCurrentFSMNodeOwnership_ == "included" ) {
    releaseOwnershipOfFSM(pvssFSMNodeName_.toString()); 
  }
  
//--- unsubscribe to state changes of FSM node
//    given in .xml configuration file 
  if ( psxConnectionStatus_ == "connected" ) { 
    disconnectFromFSM(pvssFSMNodeName_.toString());
  }

//--- delete PixelDCSFSMDeviceDefinition objects
//    initialized from xml configuration file
  for ( std::map<std::string, PixelDCSFSMNodeDefinition*>::const_iterator fsmNodeDefinition = fsmNodeDefinitionMap_.begin();
	fsmNodeDefinition != fsmNodeDefinitionMap_.end(); ++fsmNodeDefinition ) {
    delete fsmNodeDefinition->second;
  }

  delete smiCommander_;
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelDCSSupervisor::loadConfigFile() throw (xdaq::exception::Exception)
{
//--- open xml configuration file
//    and retrieve Translation Tables between
//   o PVSS and XDAQ FSM States
//   o PVSS and XDAQ FSM Commands
//
//    (NOTE: the actual translation of XDAQ to PVSS FSM Commands 
//           depends upon the current XDAQ FSM State)
//
  std::cout << "<PixelDCSSupervisor::loadConfigFile>:" << std::endl;
  std::cout << " opening config file = " << configFileName_.toString() << std::endl;

  try {
    std::auto_ptr<DOMDocument> configDocument(xoap::getDOMParserFactory()->get("configure")->loadXML(configFileName_.toString()));

    if ( configDocument.get() != NULL ) {
      DOMNodeList* configTagList = configDocument->getElementsByTagName(xoap::XStr("dcssupervisor:configuration"));
      if ( configTagList->getLength() == 1 ) {
	DOMNode* configTag = configTagList->item(0);

	DOMNodeList* configNodes = configTag->getChildNodes();
	unsigned int numConfigNodes = configNodes->getLength();
	for ( unsigned int iNode = 0; iNode < numConfigNodes; ++iNode ) {
	  DOMNode* configNode = configNodes->item(iNode);

//--- skip empty lines
//    (not removed by DOM parser)
	  if ( xoap::XMLCh2String(configNode->getLocalName()) == "" ) continue;

	  if ( xoap::XMLCh2String(configNode->getLocalName()) == "node" ) {
//--- initialize mapping between PVSS and XDAQ states and commands
//    for given FSM device type

	    std::string deviceType = xoap::getNodeAttribute(configNode, "type");

	    try {
	      fsmNodeDefinitionMap_[deviceType] = new PixelDCSFSMNodeDefinition(readFSMNodeDefinition(configNode));
	    } catch ( xcept::Exception& e ) {
	      std::cout << "Caught Exception thrown by readFSMDeviceType" << std::endl;
	      std::cout << " " << e.what() << std::endl;
	      XCEPT_RETHROW(xoap::exception::Exception, "Error parsing config File", e);
	    }
	  } else {
	    XCEPT_RAISE (xcept::Exception, "Error parsing config File");
	  }
	}
      } else {
	XCEPT_RAISE (xcept::Exception, "Error parsing config File");
      }
    } else {
      XCEPT_RAISE (xcept::Exception, "Could not open config File");
    }
  } catch ( xcept::Exception& e ) {
    std::cout << "Caught Exception thrown by loadXML" << std::endl;
    std::cout << " " << e.what() << std::endl;
    XCEPT_RETHROW(xcept::Exception, "Could not parse config File. " + std::string(e.what()), e);
  }

  std::cout << " finished parsing config file." << std::endl;
  configFileLoaded_ = true;
}

//
//---------------------------------------------------------------------------------------------------
//

std::string PixelDCSSupervisor::getPVSSFSMNodeColor(const std::string& pvssFSMNodeState)
{
  std::string pvssFSMNodeColor = "#C0C0C0";
  for ( std::map<std::string, PixelDCSFSMNodeDefinition*>::const_iterator fsmNodeDefinition = fsmNodeDefinitionMap_.begin();
	fsmNodeDefinition != fsmNodeDefinitionMap_.end(); ++fsmNodeDefinition ) {
    if ( fsmNodeDefinition->second->getType() == pvssFSMNodeType_.toString() ) {
      if ( !(pvssFSMNodeState == "" || pvssFSMNodeState == "UNDEFINED" || pvssFSMNodeState == "DEAD") ) {
	pvssFSMNodeColor = fsmNodeDefinition->second->getColor(pvssFSMNodeState);
      }
    }
  }
  
  return pvssFSMNodeColor;
}

void PixelDCSSupervisor::Default(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{
  std::cout << "<PixelDCSSupervisor::Default>:" << std::endl;

  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  xgi::Utils::getPageHeader(*out, "Pixel DCS Supervisor", fsm_.getStateName(fsm_.getCurrentState()));

  *out << "<body onload=\"ajaxFunction()\">" << std::endl;

//--- add Javascript code for AJAX functionality
//    (automatic update of webpage whenever state of PVSS FSM node changes)
  std::string urlAjaxHandler = "/" + getApplicationDescriptor()->getURN() + "/AjaxHandler";

  PixelAJAXCommander::printJavascriptHeader(*out);

  *out << "  xmlHttp.onreadystatechange = function() {" << std::endl;
//--- check that transmission of data from "server" process is complete
  *out << "    if ( xmlHttp.readyState == 4 ) {" << std::endl; 
  *out << "      var xmlDoc = xmlHttp.responseXML.documentElement;" << std::endl;
  *out << "      document.getElementById('pvssCurrentFSMState').innerHTML = xmlDoc.getElementsByTagName(\"pvssCurrentState\")[0].childNodes[0].nodeValue;" << std::endl;
  *out << "      document.getElementById('pvssCurrentFSMState').style.backgroundColor = xmlDoc.getElementsByTagName(\"pvssColor\")[0].childNodes[0].nodeValue;" << std::endl;
  *out << "      document.getElementById('xdaqCurrentFSMState').innerHTML = xmlDoc.getElementsByTagName(\"xdaqCurrentState\")[0].childNodes[0].nodeValue;" << std::endl;
//--- WARNING: operator ++var not defined in JavaScript, need to use operator var++ instead
//             (loop will not work otherwise !!!)
  *out << "      for ( iStateTransition = 0; iStateTransition < xmlDoc.getElementsByTagName(\"xdaqStateTransition\").length; iStateTransition++ ) {" << std::endl;
  *out << "        var buttonName = xmlDoc.getElementsByTagName(\"xdaqStateTransition\")[iStateTransition].getAttribute(\"name\");" << std::endl;
  *out << "        var buttonStatus = xmlDoc.getElementsByTagName(\"xdaqStateTransition\")[iStateTransition].childNodes[0].nodeValue;" << std::endl;
  *out << "        if ( buttonStatus == 'enabled' ) {" << std::endl;
  *out << "          document.getElementById(buttonName).disabled = false;" << std::endl;
  *out << "        } else if ( buttonStatus == 'disabled' ) {" << std::endl;
  *out << "          document.getElementById(buttonName).disabled = true;" << std::endl;
  *out << "        }" << std::endl;
  *out << "      }" << std::endl;
  *out << "    }" << std::endl;
  *out << "  }" << std::endl;

  PixelAJAXCommander::printJavascriptServerRequest(urlAjaxHandler, *out);
  *out << std::endl;
  *out << "  var today = new Date();" << std::endl;
  *out << "  var h = today.getHours();" << std::endl;
  *out << "  var m = today.getMinutes();" << std::endl;
  *out << "  var s = today.getSeconds();" << std::endl;
//--- add a zero in front of numbers<10
  *out << "  m = checkTime(m);" << std::endl;
  *out << "  s = checkTime(s);" << std::endl;
  *out << "  document.getElementById('clock').innerHTML = h + \":\" + m + \":\" + s;" << std::endl;
  *out << std::endl;
  *out << "  t = setTimeout('ajaxFunction()', 500);" << std::endl;
  PixelAJAXCommander::printJavascriptTrailer(*out); 

//--- render the State Machine GUI
  
  std::set<std::string> allInputs = fsm_.getInputs();
  std::set<std::string> clickableInputs = fsm_.getInputs(fsm_.getCurrentState());
  
  *out<<"<h2> Finite State Machine </h2>";
  
  std::string urlXgiHandler = "/" + getApplicationDescriptor()->getURN() + "/XgiHandler";
  *out << "<form name=\"input\" method=\"get\" action=\"" << urlXgiHandler << "\" enctype=\"multipart/form-data\">";
  
  *out << "<table border cellpadding=10 cellspacing=0>";
  *out << "<tr>";
  *out << "<td> <b>Current State</b> <br/>" << "<div id=\"xdaqCurrentFSMState\">" << fsm_.getStateName(fsm_.getCurrentState()) << "</div>" << "</td>";
  *out << "<td colspan=7>";
  for ( std::set<std::string>::iterator i = allInputs.begin();
	i != allInputs.end(); ++i ) {

//--- skip "invisible" (internally triggered)
//    state transitions
    if ( i->find("invisible") != std::string::npos ) continue;

    if ( clickableInputs.find(*i) != clickableInputs.end() ) {
      HTML2XGI(out, XDAQ_ROOT + "/pixel/PixelDCSSupervisor/html/" + (*i) + ".htm");
    }
  }
  *out << "</td>" << std::endl;
  *out << "</tr>" << std::endl;
  *out << "<tr>" << std::endl;
  
  for ( std::set<std::string>::iterator input = allInputs.begin();
	input != allInputs.end(); ++input ) {

//--- skip "invisible" (internally triggered)
//    state transitions
    if ( input->find("invisible") != std::string::npos ) continue;

    *out << "<td>";
    if ( clickableInputs.find(*input) != clickableInputs.end() ) {
      *out << "<input type=\"submit\" name=\"Command\" id=\"" << (*input) << "\" value=\"" << (*input) << "\"/>";
    } else {
      *out << "<input type=\"submit\" disabled=\"true\" name=\"Command\" id=\"" << (*input) << "\" value=\"" << (*input) << "\"/>";
    }
    *out << "</td>" << std::endl;
  }
  
  *out << "</tr>";
  *out << "</table>";
  *out << "</form>" << std::endl;
  
  *out << "<hr/>" << std::endl;
  
//--- render low-level custom GUI
  
  *out << "<h2>Ownership of PVSS FSM Node</h2>" << std::endl;
  
  *out << cgicc::table().set("border cellpadding","10").set("cellspacing","0") << std::endl;
  *out << cgicc::tr() 
       << cgicc::td() << "<b>Current Setting</b>" << "<br/>" << pvssCurrentFSMNodeOwnership_ << cgicc::td()
       << cgicc::td().set("rowspan", "2").set("align", "center") << "<form name=\"input\" method=\"get\" action=\"" << urlXgiHandler << "\" enctype=\"multipart/form-data\">";
//--- show either "Take" or "Release" button,
//    depending on whether PVSS FSM is operated by XDAQ or solely monitored
  if ( pvssCurrentFSMNodeOwnership_ == "included" ) {
    *out << "<input type=\"submit\" ";
    if ( psxConnectionStatus_ != "connected" ) *out << "disabled=\"true\" ";
    *out << "name=\"Command\" id=\"ReleaseOwnership\" value=\"Release Ownership\"/>" << std::endl;
  } else {
    *out << "<input type=\"submit\" ";
    if ( psxConnectionStatus_ != "connected" ) *out << "disabled=\"true\" ";
    *out << "name=\"Command\" id=\"TakeOwnership\" value=\"Take Ownership\"/>" << std::endl;
  }
  *out << "</form>" << cgicc::td() << std::endl;
  *out << cgicc::table() << std::endl;
    
  *out << cgicc::hr() << std::endl;

  *out << "<h2>PVSS FSM Commands</h2>" << std::endl;
  
  *out << "<form name=\"input\" method=\"get\" action=\"" << urlXgiHandler << "\" enctype=\"multipart/form-data\">";
  *out << cgicc::table().set("border cellpadding","10").set("cellspacing","0") << std::endl;
  *out << cgicc::tr();
  *out << cgicc::td().set("align", "center") << "<input type=\"submit\" ";
  if ( psxConnectionStatus_ != "connected" || pvssCurrentFSMNodeOwnership_ != "included" ) *out << "disabled=\"true\" ";
  *out << "name=\"Command\" id=\"STANDBY\" value=\"STANDBY\"/>" << cgicc::td() << std::endl;  
  *out << cgicc::td().set("align", "center") << "<input type=\"submit\" ";
  if ( psxConnectionStatus_ != "connected" || pvssCurrentFSMNodeOwnership_ != "included" ) *out << "disabled=\"true\" ";
  *out << "name=\"Command\" id=\"ON\" value=\"ON\"/>" << cgicc::td() << std::endl;
  *out << cgicc::td().set("align", "center") << "<input type=\"submit\" ";
  if ( psxConnectionStatus_ != "connected" || pvssCurrentFSMNodeOwnership_ != "included" ) *out << "disabled=\"true\" ";
  *out << "name=\"Command\" id=\"OFF\" value=\"OFF\"/>" << cgicc::td() << std::endl;
  *out << cgicc::td().set("align", "center") << "<input type=\"submit\" ";
  if ( psxConnectionStatus_ != "connected" || pvssCurrentFSMNodeOwnership_ != "included" ) *out << "disabled=\"true\" ";
  *out << "name=\"Command\" id=\"RECOVER\" value=\"RECOVER\"/>" << cgicc::td() << std::endl;
  *out << cgicc::table() << std::endl;
  *out << "</form>" << std::endl;
    
  *out << cgicc::hr() << std::endl;

  *out << "<h2>Status Information</h2>" << std::endl;
  
  *out << cgicc::table().set("border","2") << std::endl;
  //*out << cgicc::table().set("border cellpadding","10").set("cellspacing","0") << std::endl;
  *out << cgicc::tr() 
       << cgicc::th().set("colspan", "2").set("align", "center") << "PVSS FSM Node" << cgicc::th()
       << cgicc::th() << "Connection" << cgicc::th()
       << cgicc::th() << "Ownership" << cgicc::th() << std::endl;
  *out << cgicc::tr() 
       << cgicc::td().set("align", "center") << "Name" << cgicc::td() 
       << cgicc::td().set("align", "center") << "Current State" << cgicc::td() 
       << cgicc::td().set("align", "center") << "Status" << cgicc::td() 
       << cgicc::td().set("align", "center") << "Status" << cgicc::td() << std::endl;

  std::string pvssFSMNodeColor = getPVSSFSMNodeColor("UNDEFINED");

  *out << cgicc::tr() 
       << cgicc::td() << pvssFSMNodeName_.toString() << cgicc::td() 
       << cgicc::td().set("align", "center").set("bgcolor", pvssFSMNodeColor) << "<div id=\"pvssCurrentFSMState\">" << pvssCurrentFSMState_ << "</div>" << cgicc::td() 
       << cgicc::td().set("align", "center") << psxConnectionStatus_ << cgicc::td()
       << cgicc::td().set("align", "center") << pvssCurrentFSMNodeOwnership_ << cgicc::td() << std::endl;
  *out << cgicc::table() << std::endl;

  *out << cgicc::hr() << std::endl;

  *out << "last updated: <text id=\"clock\">UNINITIALIZED</text>" << std::endl;
  //*out << "<div id=\"DEBUG\">debug output here !!!</div>" << std::endl;
  
  *out << "</body>" << std::endl;
  *out << "</html>";
}

void PixelDCSSupervisor::XgiHandler(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception)
{
  cgicc::Cgicc cgi(in);

  std::string command = cgi.getElement("Command")->getValue();
  
  if ( command == "Initialize" ) {
    xoap::MessageReference msg = MakeSOAPMessageReference("Initialize");
    xoap::MessageReference reply = Initialize(msg);
    if ( Receive(reply) != "InitializeDone" ) std::cout << "Failed to execute \"Initialize\" command" << std::endl;
  } else if ( command == "Configure" ) {
    xoap::MessageReference msg = MakeSOAPMessageReference("Configure");
    xoap::MessageReference reply = Configure(msg);
    if ( Receive(reply) != "ConfigureDone" ) std::cout << "Failed to execute \"Configure\" command" << std::endl;
  } else if ( command == "Start") {
    xoap::MessageReference msg = MakeSOAPMessageReference("Start");
    xoap::MessageReference reply = Start(msg);
    if ( Receive(reply) != "StartDone" ) std::cout << "Failed to execute \"Start\" command" << std::endl;
  } else if ( command == "Pause") {
    xoap::MessageReference msg = MakeSOAPMessageReference("Pause");
    xoap::MessageReference reply = Pause(msg);
    if ( Receive(reply) != "PauseDone" ) std::cout << "Failed to execute \"Pause\" command" << std::endl;
  } else if ( command == "Resume") {
    xoap::MessageReference msg = MakeSOAPMessageReference("Resume");
    xoap::MessageReference reply = Resume(msg);
    if ( Receive(reply) != "ResumeDone" ) std::cout << "Failed to execute \"Resume\" command" << std::endl;
  } else if ( command == "Halt" ) {
    xoap::MessageReference msg = MakeSOAPMessageReference("Halt");
    xoap::MessageReference reply = Halt(msg);
    if ( Receive(reply) != "HaltDone" ) std::cout << "Failed to execute \"Halt\" command" << std::endl;
  } else if ( command == "Stop" ) {
    xoap::MessageReference msg = MakeSOAPMessageReference("Stop");
    xoap::MessageReference reply = Stop(msg);
    if ( Receive(reply) != "StopDone" ) std::cout << "Failed to execute \"Stop\" command" << std::endl;
  } else if ( command == "Take Ownership" ) {
    takeOwnershipOfFSM(pvssFSMNodeName_);
  } else if ( command == "Release Ownership" ) {
    releaseOwnershipOfFSM(pvssFSMNodeName_);
  } else if ( command == "STANDBY" ) {
    if ( pvssCurrentFSMNodeOwnership_ == "included" ) {
      std::cout << "sending FSM command..." << std::endl;
      smiCommander_->sendCommandToFSM(pvssFSMNodeName_, fsmNodeOwner, "STANDBY");
    }
  } else if ( command == "ON" ) {
    if ( pvssCurrentFSMNodeOwnership_ == "included" ) {
      std::cout << "sending FSM command..." << std::endl;
      smiCommander_->sendCommandToFSM(pvssFSMNodeName_, fsmNodeOwner, "ON");
    }
  } else if ( command == "OFF" ) {
    if ( pvssCurrentFSMNodeOwnership_ == "included" ) {
      std::cout << "sending FSM command..." << std::endl;
      smiCommander_->sendCommandToFSM(pvssFSMNodeName_, fsmNodeOwner, "OFF");
    }
  } else if ( command == "RECOVER" ) {
    if ( pvssCurrentFSMNodeOwnership_ == "included" ) {
      std::cout << "sending FSM command..." << std::endl;
      smiCommander_->sendCommandToFSM(pvssFSMNodeName_, fsmNodeOwner, "RECOVER");
    }
  }

  this->Default(in, out);
}

void PixelDCSSupervisor::AjaxHandler(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception) 
{
//--- compose XML message containing current state of PVSS FSM node

  //std::cout << "<PixelDCSSupervisor::AjaxHandler>:" << std::endl;

//--- create header
  cgicc::HTTPResponseHeader response("HTTP/1.1", 200, "OK");
  response.addHeader("Content-Type", "text/xml");

//--- add XML content
  *out << "<AjaxHandler>" << std::endl;
  *out << "<pvssFSMNode name=\"" << pvssFSMNodeName_.toString() << "\">" << std::endl;
  std::string pvssCurrentState = pvssCurrentFSMState_;
  std::string tag_pvssCurrentState = std::string("<pvssCurrentState>") + pvssCurrentState + "</pvssCurrentState>";
  *out << tag_pvssCurrentState << std::endl;
  std::string pvssColor = getPVSSFSMNodeColor(pvssCurrentState);
  std::string tag_pvssColor = std::string("<pvssColor>") + pvssColor + "</pvssColor>";
  *out << tag_pvssColor << std::endl;
  *out << "</pvssFSMNode>" << std::endl;

  *out << "<xdaqFSM>" << std::endl;
  std::string tag_xdaqCurrentState = std::string("<xdaqCurrentState>") + fsm_.getStateName(fsm_.getCurrentState()) + "</xdaqCurrentState>";
  *out << tag_xdaqCurrentState << std::endl;

  std::set<std::string> allInputs = fsm_.getInputs();
  std::set<std::string> clickableInputs = fsm_.getInputs(fsm_.getCurrentState());

  for ( std::set<std::string>::iterator input = allInputs.begin();
	input != allInputs.end(); ++input ) {

//--- skip "invisible" (internally triggered)
//    state transitions
    if ( input->find("invisible") != std::string::npos ) continue;

    std::string status_xdaqStateTransition;
    if ( clickableInputs.find(*input) != clickableInputs.end() ) {
      status_xdaqStateTransition = "enabled";
    } else {
      status_xdaqStateTransition = "disabled";
    }
    
    std::string tag_xdaqCurrentStateTransition = std::string("<xdaqStateTransition name=\"") + (*input) + "\">" + status_xdaqStateTransition  + "</xdaqStateTransition>";
    *out << tag_xdaqCurrentStateTransition << std::endl;
  }
  
  *out << "</xdaqFSM>" << std::endl;
  *out << "</AjaxHandler>" << std::endl;

//--- send XML message 
//    (to be processed by Javascript code in PixelDCSSupervisor::Default method)
  out->setHTTPResponseHeader(response);  
}

//
//---------------------------------------------------------------------------------------------------
//

xoap::MessageReference PixelDCSSupervisor::Initialize(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  std::cout << "<PixelDCSSupervisor::Initialize>:" << std::endl;

//--- do not attempt to "Initialize"
//    if already "Halted"
  if ( fsm_.getStateName(fsm_.getCurrentState()) == "Halted" ) {
    std::cout << " PixelDCSSupervisor is already \"Halted\"" << std::endl;
    return MakeSOAPMessageReference("InitializeFailed");
  }

//--- establish connection to PSX server
//    if neccessary
  if ( psxConnectionStatus_ != "connected" ) {
    try {
      std::cout << "connecting to PSX server..." << std::endl;
      connectToFSM(pvssFSMNodeName_.toString());
    } catch ( xdaq::exception::Exception& e ) {
      std::cout << "Caught Exception thrown by PixelDCSSupervisor::connectPSX" << std::endl;
      std::cout << " " << e.what() << std::endl;
      //XCEPT_RETHROW(xoap::exception::Exception, e.what(), e);
      return MakeSOAPMessageReference("InitializeFailed");
    }
  }

//--- take control of PVSS FSM node
//    if neccessary
  if ( pvssFSMNodeOwnership_.toString() == "included" &&
       pvssCurrentFSMNodeOwnership_ != "included" ) {
    try {
      std::cout << "taking ownership of FSM..." << std::endl;
      takeOwnershipOfFSM(pvssFSMNodeName_.toString());
    } catch ( xdaq::exception::Exception& e ) {
      std::cout << "Caught Exception thrown by PixelDCSSupervisor::takeOwnershipPSX" << std::endl;
      std::cout << " " << e.what() << std::endl;
      //XCEPT_RETHROW(xoap::exception::Exception, e.what(), e);
      return MakeSOAPMessageReference("InitializeFailed");
    }
  }

//--- send command corresponding to XDAQ command "Initialize"
//    to PVSS
  if ( pvssCurrentFSMNodeOwnership_ == "included" ) {
    try {
      std::cout << "sending FSM command..." << std::endl;
      sendCommandToFSM(pvssFSMNodeName_.toString(), pvssFSMNodeType_.toString(), "Initialize");
    } catch ( xdaq::exception::Exception& e ) {
      std::cout << "Caught Exception thrown by PixelDCSSupervisor::sendCommandToFSM" << std::endl;
      std::cout << " " << e.what() << std::endl;
      //XCEPT_RETHROW(xoap::exception::Exception, e.what(), e);
      return MakeSOAPMessageReference("InitializeFailed");
    }
  }

  return MakeSOAPMessageReference("InitializeDone");
}

xoap::MessageReference PixelDCSSupervisor::Configure(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  std::cout << "<PixelDCSSupervisor::Configure>:" << std::endl;

//--- do not attempt to "Configure"
//    if already "Configured" or "Configuring"
  if ( fsm_.getStateName(fsm_.getCurrentState()) == "Configuring" ||
       fsm_.getStateName(fsm_.getCurrentState()) == "Configured" ) {
    std::cout << " PixelDCSSupervisor is already \"" << fsm_.getStateName(fsm_.getCurrentState()) << "\"" << std::endl;
    return MakeSOAPMessageReference("ConfigureFailed");
  }

//--- send command corresponding to XDAQ command "Configure"
//    to PVSS
  if ( pvssCurrentFSMNodeOwnership_ == "included" ) {
    try {
      std::cout << "sending FSM command..." << std::endl;
      sendCommandToFSM(pvssFSMNodeName_.toString(), pvssFSMNodeType_.toString(), "Configure");
    } catch ( xdaq::exception::Exception& e ) {
      std::cout << "Caught Exception thrown by PixelDCSSupervisor::sendCommandToFSM" << std::endl;
      std::cout << " " << e.what() << std::endl;
      //XCEPT_RETHROW(xoap::exception::Exception, e.what(), e);
      return MakeSOAPMessageReference("ConfigureFailed");
    }
  }

  toolbox::Event::Reference e(new toolbox::Event("Configure", this));
  fsm_.fireEvent(e);

  return MakeSOAPMessageReference("ConfigureDone");
}

xoap::MessageReference PixelDCSSupervisor::Start(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  std::cout << "<PixelDCSSupervisor::Start>:" << std::endl;

//--- do not attempt to "Start"
//    if already "Running"
  if ( fsm_.getStateName(fsm_.getCurrentState()) == "Running" ) {
    std::cout << " PixelDCSSupervisor is already \"Running\"" << std::endl;
    return MakeSOAPMessageReference("StartFailed");
  }

  toolbox::Event::Reference e(new toolbox::Event("Start", this));
  fsm_.fireEvent(e);

  return MakeSOAPMessageReference("StartDone");
}

xoap::MessageReference PixelDCSSupervisor::Halt(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  std::cout << "<PixelDCSSupervisor::Halt>:" << std::endl;

//--- do not attempt to "Halt"
//    if already "Halted"
  if ( fsm_.getStateName(fsm_.getCurrentState()) == "Halted" ) {
    std::cout << " PixelDCSSupervisor is already \"Halted\"" << std::endl;
    return MakeSOAPMessageReference("HaltFailed");
  }

//--- send command corresponding to XDAQ command "Halt"
//    to PVSS
  if ( pvssCurrentFSMNodeOwnership_ == "included" ) {
    try {
      std::cout << "sending FSM command..." << std::endl;
      sendCommandToFSM(pvssFSMNodeName_.toString(), pvssFSMNodeType_.toString(), "Halt");
    } catch ( xdaq::exception::Exception& e ) {
      std::cout << "Caught Exception thrown by PixelDCSSupervisor::sendCommandToFSM" << std::endl;
      std::cout << " " << e.what() << std::endl;
      //XCEPT_RETHROW(xoap::exception::Exception, e.what(), e);
      return MakeSOAPMessageReference("HaltFailed");
    }
  }
  
  return MakeSOAPMessageReference("HaltDone");
}

xoap::MessageReference PixelDCSSupervisor::Pause(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  std::cout << "<PixelDCSSupervisor::Pause>:" << std::endl;

//--- do not attempt to "Pause"
//    if already "Paused"
  if ( fsm_.getStateName(fsm_.getCurrentState()) == "Paused" ) {
    std::cout << " PixelDCSSupervisor is already \"Paused\"" << std::endl;
    return MakeSOAPMessageReference("PauseFailed");
  }

  toolbox::Event::Reference e(new toolbox::Event("Pause", this));
  fsm_.fireEvent(e);

  return MakeSOAPMessageReference("PauseDone");
}

xoap::MessageReference PixelDCSSupervisor::Resume(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  std::cout << "<PixelDCSSupervisor::Resume>:" << std::endl;

//--- do not attempt to "Resume"
//    if already "Running"
  if ( fsm_.getStateName(fsm_.getCurrentState()) == "Running" ) {
    std::cout << " PixelDCSSupervisor is already \"Running\"" << std::endl;
    return MakeSOAPMessageReference("ResumeFailed");
  }

  toolbox::Event::Reference e(new toolbox::Event("Resume", this));
  fsm_.fireEvent(e);

  return MakeSOAPMessageReference("ResumeDone");
}

xoap::MessageReference PixelDCSSupervisor::Stop(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  std::cout << "<PixelDCSSupervisor::Stop>:" << std::endl;

//--- do not attempt to "Stop"
//    if already "Configured"
  if ( fsm_.getStateName(fsm_.getCurrentState()) == "Configured" ) {
    std::cout << " PixelDCSSupervisor is already \"Configured\"" << std::endl;
    return MakeSOAPMessageReference("StopFailed");
  }

  toolbox::Event::Reference e(new toolbox::Event("Stop", this));
  fsm_.fireEvent(e);
  
  return MakeSOAPMessageReference("StopDone");
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelDCSSupervisor::connectToFSM(const std::string& fsmNodeName) throw (xdaq::exception::Exception)
{
//--- compose and send SOAP message to the PSX server
//    to subscribe to the FSM state 
//    of the PVSS FSM node given as function argument

  std::cout << "<PixelDCSSupervisor::connectToFSM>:" << std::endl;
  std::cout << " fsmNodeName = " << fsmNodeName << std::endl;

  if ( !configFileLoaded_ ) {
    try {
//--- initialize FSM state and command Translation Tables 
//    between XDAQ and PVSS
      std::cout << "loading configuration file" << std::endl;
      this->loadConfigFile();
    } catch ( xdaq::exception::Exception& e ) {
      std::cout << "Caught Exception thrown by PixelDCSSupervisor::loadConfigFile" << std::endl;
      std::cout << " " << e.what() << std::endl;
      XCEPT_RETHROW(xdaq::exception::Exception, "Could parse config File. " + std::string(e.what()), e);
    }    
  }

//--- do not attempt to connect to PSX server
//    if already connected
  if ( psxConnectionStatus_ == "connected" ) return;

  std::pair<std::string, std::string> psxResponse = smiCommander_->connectToFSM(fsmNodeName, "", fsmNodeOwner);

  psxConnectionStatus_ = psxResponse.first;
  psxConnectionId_ = psxResponse.second;
}

void PixelDCSSupervisor::takeOwnershipOfFSM(const std::string& fsmNodeName) throw (xdaq::exception::Exception)
{
//--- compose and send SOAP message to the PSX server
//    to take ownership of the PVSS FSM node 
//    given as function argument

  std::cout << "<PixelDCSSupervisor::takeOwnershipOfFSM>:" << std::endl;
  std::cout << " fsmNodeName = " << fsmNodeName << std::endl;

//--- do not attempt to take ownership twice
  if ( pvssCurrentFSMNodeOwnership_ == "included" ) return;

  smiCommander_->takeOwnershipOfFSM(fsmNodeName, fsmNodeOwner, fsmNodeExclusiveFlag);

  pvssCurrentFSMNodeOwnership_ = "included";
}

void PixelDCSSupervisor::sendCommandToFSM(const std::string& fsmNodeName, const std::string& fsmNodeType, 
					  const std::string& command) throw (xdaq::exception::Exception)
{
//--- compose and send SOAP message to the PSX server
//    to take ownership of the PVSS FSM node 
//    given as function argument

  std::cout << "<PixelDCSSupervisor::sendCommandToFSM>:" << std::endl;
  std::cout << " fsmNodeName = " << fsmNodeName << std::endl;
  std::cout << " fsmNodeType = " << fsmNodeType << std::endl;

//--- check availability of device definition
//    for type of PVSS FSM node
  if ( fsmNodeDefinitionMap_[fsmNodeType] == NULL ) {
    XCEPT_RAISE(xdaq::exception::Exception, "Undefined device Type");
  }

//--- find PVSS command 
//    corresponding to XDAQ command given as function argument
//
//    NOTE: mapping between PVSS and XDAQ commands
//          depends on the current XDAQ state
//          (e.g. XDAQ command "Halt" corresponds to PVSS command 
//               o "OFF" if current XDAQ state is "Configured"
//               o "RECOVER" if current XDAQ state is "Error")
//
  const std::string& xdaqCommand = command;
  const std::string xdaqState = fsm_.getStateName(fsm_.getCurrentState());
  std::cout << "xdaqCommand = " << xdaqCommand << std::endl;
  std::cout << "xdaqState = " << xdaqState << std::endl;

  const std::string& pvssCommand = fsmNodeDefinitionMap_[fsmNodeType]->getCommandDefinition(xdaqCommand, xdaqState);
  if ( pvssCommand == "" ) {
    XCEPT_RAISE(xdaq::exception::Exception, "Could not map XDAQ to PVSS command");
  }

  smiCommander_->sendCommandToFSM(fsmNodeName, fsmNodeOwner, pvssCommand);
}

void PixelDCSSupervisor::releaseOwnershipOfFSM(const std::string& fsmNodeName) throw (xdaq::exception::Exception)
{
//--- compose and send SOAP message to the PSX server
//    to release ownership of the PVSS FSM node 
//    given as function argument

  std::cout << "<PixelDCSSupervisor::releaseOwnershipOfFSM>:" << std::endl;
  std::cout << " fsmNodeName = " << fsmNodeName << std::endl;

//--- do not attempt to take ownership twice
  if ( pvssCurrentFSMNodeOwnership_ == "manual" ) return;

  smiCommander_->releaseOwnershipOfFSM(fsmNodeName, fsmNodeOwner);

  pvssCurrentFSMNodeOwnership_ = "manual";
}

void PixelDCSSupervisor::disconnectFromFSM(const std::string& fsmNodeName) throw (xdaq::exception::Exception)
{
//--- compose and send SOAP message to the PSX server
//    to unsubscribe to the FSM state 
//    of the PVSS FSM node given as function argument

  std::cout << "<PixelDCSSupervisor::disconnectFromFSM>:" << std::endl;
  std::cout << " fsmNodeName = " << fsmNodeName << std::endl;

//--- do not attempt to disconnect to PSX server
//    if already disconnected
  if ( psxConnectionStatus_ == "disconnected" ) return;

  smiCommander_->disconnectFromFSM(psxConnectionId_);

  psxConnectionStatus_ = "disconnected";
}

//
//---------------------------------------------------------------------------------------------------
//

xoap::MessageReference PixelDCSSupervisor::updateFSMState(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  std::cout << "<PixelDCSSupervisor::updateFSMState>:" << std::endl;

  std::cout << " Notification : ------------------------------------ " << std::endl;
  msg->writeTo(std::cout);
  std::cout << std::endl;
  std::cout << " ---------------------------------------------- " << std::endl;

//--- acknowledge receipt of FSM state update notification;
//    send SOAP response to PSX server
/*
  try { 
    smiCommander_->sendNotifyResponseToFSM();
  } catch ( xcept::Exception& e ) {
    std::cout << "Caught Exception thrown by PixelDCSSMICommander::sendConnectResponseToFSM" << std::endl;
    std::cout << " " << e.what() << std::endl;
    XCEPT_RETHROW(xoap::exception::Exception, "Could not send SOAP message", e);
  }
 */

//--- begin unpacking SOAP message
//    received from PSX server
  xoap::SOAPEnvelope envelope = msg->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();

//--- create SOAPName objects 
//    for keywords searched for in SOAP message
  xoap::SOAPName command = envelope.createName("notify");
  xoap::SOAPName objectAttribute = envelope.createName("object");
  xoap::SOAPName transactionIdAttribute = envelope.createName("id");
  xoap::SOAPName contextAttribute = envelope.createName("context");

//--- find within body 
//    "notify" command
  std::vector< xoap::SOAPElement > bodyElements = body.getChildElements(command);          	  
  for ( std::vector< xoap::SOAPElement >::iterator bodyElement = bodyElements.begin();
	bodyElement != bodyElements.end(); ++bodyElement ) {

//--- unpack FSM node name and state,
//    together with transaction identifier and context information
    std::string fsmNodeName = bodyElement->getAttributeValue(objectAttribute).data();
    std::string transactionId = bodyElement->getAttributeValue(transactionIdAttribute).data();
    std::string context = bodyElement->getAttributeValue(contextAttribute).data();
    std::string fsmNodeState = bodyElement->getValue().data();

    pvssCurrentFSMState_ = fsmNodeState;

//--- check that fsmNodeName decoded from SOAP message
//    matches expected value
    if ( fsmNodeName == pvssFSMNodeName_.toString() ) {

//--- check availability of device definition
//    for type of PVSS FSM node
      if ( fsmNodeDefinitionMap_[pvssFSMNodeType_.toString()] == NULL ) {
	XCEPT_RAISE(xoap::exception::Exception, "Undefined device Type");
	//return MakeSOAPMessageReference("updateFSMStateFailed");
      }

//--- find XDAQ state corresponding to
//    fsmNodeState
      const std::string& xdaqFSMState = fsmNodeDefinitionMap_[pvssFSMNodeType_.toString()]->getXdaqState(fsmNodeState);

      if ( xdaqFSMState == "" ) {
	XCEPT_RAISE(xoap::exception::Exception, "Could not map PVSS to XDAQ state");
	//return MakeSOAPMessageReference("updateFSMStateFailed");
      }

//--- check if XDAQ state needs to be changed
      if ( fsm_.getStateName(fsm_.getCurrentState()) != xdaqFSMState ) {

//--- find XDAQ state transition 
//    leading from current XDAQ state 
//    to state corresponding to fsmNodeState
	std::map<std::string, toolbox::fsm::State> xdaqFSMTransitions = fsm_.getTransitions(fsm_.getCurrentState());
	bool executedStateTransition = false;
	for ( std::map<std::string, toolbox::fsm::State>::const_iterator xdaqFSMTransition = xdaqFSMTransitions.begin();
	      xdaqFSMTransition != xdaqFSMTransitions.end(); ++xdaqFSMTransition ) {
	  if ( fsm_.getStateName(xdaqFSMTransition->second) == xdaqFSMState ) {
//--- execute XDAQ state transition
	    std::cout << "going into " << xdaqFSMState << " state" << std::endl;
	    toolbox::Event::Reference e(new toolbox::Event(xdaqFSMTransition->first, this));
	    fsm_.fireEvent(e);
	    executedStateTransition = true;
	  }
	}
	
	if ( !executedStateTransition ) {
	  XCEPT_RAISE(xoap::exception::Exception, "Could not execute transition to XDAQ state");
	  //return MakeSOAPMessageReference("updateFSMStateFailed");
	}
      }
    }
  }
  
  //return MakeSOAPMessageReference("updateFSMStateDone");
  return smiCommander_->MakeSOAPMessageReference_fsmNotifyResponse();
}

