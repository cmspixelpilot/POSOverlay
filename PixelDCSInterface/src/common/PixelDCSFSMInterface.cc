//--*-C++-*--
#include "PixelDCSInterface/include/PixelDCSFSMInterface.h"

/*************************************************************************
 * Interface class for sending states of CAEN A4602 and A4603            *
 * power supplies from PVSS to XDAQ                                      *
 *                                                                       *
 * Author: Christian Veelken, UC Davis				         *
 * Josh Thompson, Cornell -- add A4603 HV, Configure transition          *
 *                                                                       *
 * Last update: $Date: 2012/06/16 14:13:20 $ (UTC)                       *
 *          by: $Author: mdunser $                                       *
 *************************************************************************/

#include <memory>

#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationStub.h"
#include "xgi/Utils.h"
#include "cgicc/HTMLClasses.h"
#include "toolbox/fsm/FailedEvent.h"
#include "toolbox/task/WorkLoopFactory.h"
#include "toolbox/task/Action.h"

//diagsystem
#include "toolbox/task/Timer.h"
#include "toolbox/task/TimerFactory.h"
#include "toolbox/task/TimerListener.h"
#include "toolbox/TimeInterval.h"

#include "xoap/DOMParserFactory.h"
#include "xercesc/dom/DOMDocument.hpp"
#include "xercesc/dom/DOMNodeList.hpp"
#include "xercesc/dom/DOMNode.hpp"

#include <diagbag/DiagBagWizard.h>

#include "PixelUtilities/PixelGUIUtilities/include/HTML2XGI.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSPVSSDpe.h"
#include "PixelUtilities/PixelDCSUtilities/include/readDCSFSMDeviceDefinition.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSSOAPConnection.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSPVSSCommander.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSSMICommander.h"
#include "PixelUtilities/PixelGUIUtilities/include/PixelAJAXCommander.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSSMIConnectionManager.h"

#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDetectorConfig.h"

#define PSX_NS_URI "http://xdaq.cern.ch/xdaq/xsd/2006/psx-pvss-10.xsd"
#define PSX_SMI_NS_URI "http://xdaq.web.cern.ch/xdaq/xsd/2006/psx-smi-10.xsd"

using namespace std;
using namespace pos;

XDAQ_INSTANTIATOR_IMPL(PixelDCSFSMInterface)

const std::string fsmNodeOwner = "PixelDCSFSMInterface";

const std::string xdaqState_LV_ON = "LV_ON";
const std::string xdaqState_LV_ON_REDUCED = "LV_ON_REDUCED";
const std::string xdaqState_LV_OFF = "LV_OFF";
const std::string xdaqState_UNDEFINED = "UNDEFINED";

const std::string xdaqState_HV_ON = "HV_ON";
const std::string xdaqState_HV_OFF = "HV_OFF";

const std::string defaultColor = "#C0C0C0";

// declare global auxiliary functions
std::list<const PixelDCSFSMNode*> getNodeList(const PixelDCSFSMPartition& fsmPartition);

std::string getTagNameA4602_state(const std::string& fsmPartitionName, const std::string& fsmNodeName);
std::string getTagNameA4602_color(const std::string& fsmPartitionName, const std::string& fsmNodeName);
std::string getTagNameA4602_state_used(const std::string& fsmPartitionName, const std::string& fsmNodeName);
std::string getTagNameA4602_color_used(const std::string& fsmPartitionName, const std::string& fsmNodeName);

std::string getTagNameA4603_state_power(const std::string& fsmPartitionName, const std::string& fsmNodeName);
std::string getTagNameA4603_color_power(const std::string& fsmPartitionName, const std::string& fsmNodeName);
std::string getTagNameA4603_state_readoutChipInitialization(const std::string& fsmPartitionName, const std::string& fsmNodeName);
std::string getTagNameA4603_color_readoutChipInitialization(const std::string& fsmPartitionName, const std::string& fsmNodeName);
std::string getTagNameA4603_state_used(const std::string& fsmPartitionName, const std::string& fsmNodeName);
std::string getTagNameA4603_color_used(const std::string& fsmPartitionName, const std::string& fsmNodeName);

std::string getTagNamePartitionSummary_state(const std::string& fsmPartitionName, const std::string& soapConnectionName);
std::string getTagNamePartitionSummary_color(const std::string& fsmPartitionName, const std::string& soapConnectionName);

//
//---------------------------------------------------------------------------------------------------
//

PixelDCSFSMInterface::PixelDCSFSMInterface(xdaq::ApplicationStub* s) throw (xdaq::exception::Exception) 
  : xdaq::Application(s), SOAPCommander(this), executeReconfMethodMutex(toolbox::BSem::FULL),fsm_("urn:toolbox-task-workloop:PixelDCSFSMInterface")
{

  diagService_ = new DiagBagWizard(("ReconfigurationModule") ,
				   this->getApplicationLogger(),
				   getApplicationDescriptor()->getClassName(),
				   getApplicationDescriptor()->getInstance(),
				   getApplicationDescriptor()->getLocalId(),
                                   (xdaq::WebApplication *)this,
				   "MYSYSTEM",
				   "MYSUBSYTSTEM"
				   );
  
  diagService_->reportError("The DiagSystem is installed --- this is a bogus error message",DIAGUSERINFO);

  // A SOAP callback used for generic handshaking by retrieving the FSM state
  xoap::bind(this, &PixelDCSFSMInterface::FSMStateRequest, "FSMStateRequest", XDAQ_NS_URI);

//--- bind SOAP call-back functions 
//    to Finite State Machine commmands
  xoap::bind(this, &PixelDCSFSMInterface::Initialize, "Initialize", XDAQ_NS_URI);
  xoap::bind(this, &PixelDCSFSMInterface::Configure, "Configure", XDAQ_NS_URI);
  xoap::bind(this, &PixelDCSFSMInterface::Halt, "Halt", XDAQ_NS_URI);

//--- binde XGI call-back functions 
//    to messages from the browser
  xgi::bind(this, &PixelDCSFSMInterface::Default, "Default");
  xgi::bind(this, &PixelDCSFSMInterface::XgiHandler, "XgiHandler");

//--- bind AJAX handler 
//    to "server" requests originating from the browser
//    (automatically triggered periodically by some Javascript code
//     contained in webpage created in PixelDCSFSMInterface::Default method)
  xgi::bind(this, &PixelDCSFSMInterface::AjaxHandler, "AjaxHandler");

//--- define the states 
//    of the Finite State Machine
//    (call-back functions need to be initialized in derived classes)
  fsm_.addState('I', "Initial", this, &PixelDCSFSMInterface::stateChanged);
  fsm_.addState('H', "Halted", this, &PixelDCSFSMInterface::stateChanged);
  fsm_.addState('c', "Configuring", this, &PixelDCSFSMInterface::stateConfiguring);
  fsm_.addState('C', "Configured", this, &PixelDCSFSMInterface::stateChanged);
  fsm_.setStateName('F',"Error");
  //fsm_.setFailedStateTransitionAction(this, &PixelDCSDpInterface::enteringError);

//--- define the state transitions
//    of the Finite State Machine
  fsm_.addStateTransition('I', 'H', "Initialize");
  fsm_.addStateTransition('H', 'c', "Configure");
  fsm_.addStateTransition('c', 'C', "ConfiguringDone");
  fsm_.addStateTransition('C', 'H', "Halt");
  fsm_.setInitialState('I');
  fsm_.reset();

//--- bind XGI call-back functions 
//    to messages originating from the browser
  xgi::bind(this, &PixelDCSFSMInterface::Default, "Default");
  xgi::bind(this, &PixelDCSFSMInterface::XgiHandler, "XgiHandler");

//--- bind SOAP call-back functions
//    to PVSS FSM state transitions
  xoap::bind(this, &PixelDCSFSMInterface::getPartitionState_Power, "fsmStateRequest", XDAQ_NS_URI);//PSX_SMI_NS_URI);
  xoap::bind(this, &PixelDCSFSMInterface::updatePartitionState_Power, "notify", PSX_SMI_NS_URI);
  xoap::bind(this, &PixelDCSFSMInterface::updatePartitionState_ReadoutChips, "fsmStateNotification", XDAQ_NS_URI);

//--- initialize parameters 
//    defined by environment variables
  XDAQ_ROOT = getenv("XDAQ_ROOT");

//--- initialize parameters 
//    defined by .xml configuration file
  this->getApplicationInfoSpace()->fireItemAvailable("configuration", &configFileName_);
  //const std::string& BUILD_HOME = getenv("BUILD_HOME");
  //configFileName_ = BUILD_HOME + "/pixel/PixelDCSInterface/xml/" + "interface.xml";

  // WARNING: parameters defined by .xml configuration file
  //          are not initialized yet in constructor;
  //          therefore need to postpone loading of configuration file
  //          (to retrieve FSM state and command Translation Tables 
  //           between XDAQ and PVSS)
  //
  configFileLoaded_ = false; 
  
  // Exporting the FSM state to this application's default InfoSpace
  state_=fsm_.getStateName(fsm_.getCurrentState());
  getApplicationInfoSpace()->fireItemAvailable("stateName", &state_);

  theGlobalKey_=0;
  lock_=new toolbox::BSem(toolbox::BSem::FULL,true);
  psxConnectionManager_=new PixelDCSSMIConnectionManager();
  disconnectWorkloop_=0;
  readyToConnect_=false;
  readyToConfigure_=false;

  xdaq::ApplicationDescriptor* pvssDescriptor = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor("psx", 0);
  pvssCommander_ = new PixelDCSPVSSCommander(this, pvssDescriptor);

  xdaq::ApplicationDescriptor* smiDescriptor = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor("psx", 0);
  smiCommander_ = new PixelDCSSMICommander(this, smiDescriptor);

   //diagsystem
  DIAG_DECLARE_USER_APP
   std::stringstream timerName;
   timerName << getApplicationDescriptor()->getContextDescriptor()->getURL() << ":";
   timerName << getApplicationDescriptor()->getClassName() << ":" << getApplicationDescriptor()->getLocalId() << ":" << getApplicationDescriptor()->getInstance();
   toolbox::task::Timer * timer = toolbox::task::getTimerFactory()->createTimer(timerName.str());
   toolbox::TimeInterval interval(AUTO_UP_CONFIGURE_DELAY,0);
   toolbox::TimeVal start;
   start = toolbox::TimeVal::gettimeofday() + interval;
   timer->schedule( this, start,  0, "" );
   
}

PixelDCSFSMInterface::~PixelDCSFSMInterface()
{
  delete lock_;

  delete pvssCommander_;
  delete smiCommander_;
  delete psxConnectionManager_;
  delete disconnectWorkloop_;
}

//diagsystem
void PixelDCSFSMInterface::timeExpired (toolbox::task::TimerEvent& e)
{
  DIAG_EXEC_FSM_INIT_TRANS
}

xoap::MessageReference PixelDCSFSMInterface::FSMStateRequest (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  return MakeSOAPMessageReference(state_);
}

void PixelDCSFSMInterface::stateChanged(toolbox::fsm::FiniteStateMachine &fsm) throw (toolbox::fsm::exception::Exception)
{
  //  cout<<"[PixelDCSFSMInterface::stateChanged] *** enter ***"<<endl;

  state_=fsm.getStateName(fsm.getCurrentState());
  if (PixelSupervisor_!=0) {
    Attribute_Vector parameters(3);
    parameters[0].name_="Supervisor"; parameters[0].value_="PixelDCSFSMInterface";
    parameters[1].name_="Instance";   parameters[1].value_=itoa(0); //FIXME hardcoded instance
    parameters[2].name_="FSMState";   parameters[2].value_=state_;
    Send(PixelSupervisor_, "FSMStateNotification", parameters);
  }
  cout<< "[PixelDCSFSMInterface::stateChanged] New state is:" <<std::string(state_)<<endl;;
}

void PixelDCSFSMInterface::loadConfigFile() //throw (xdaq::exception::Exception)
{

//--- open xml configuration file
//    and retrieve Translation Tables between
//   o PVSS and XDAQ FSM States
//   o PVSS and XDAQ FSM Commands
//
//    (NOTE: the actual translation of XDAQ to PVSS FSM Commands 
//           depends upon the current XDAQ FSM State)
//
  std::cout << "<PixelDCSFSMInterface::loadConfigFile>:" << std::endl;
  std::cout << " opening config file = " << configFileName_.toString() << std::endl;

  try {
    std::auto_ptr<DOMDocument> configDocument(xoap::getDOMParserFactory()->get("configure")->loadXML(configFileName_.toString()));

    if ( configDocument.get() != NULL ) {
      DOMNodeList* configTagList = configDocument->getElementsByTagName(xoap::XStr("dcsinterface:configuration"));
      if ( configTagList->getLength() == 1 ) {
	DOMNode* configTag = configTagList->item(0);
	
	DOMNodeList* configNodes = configTag->getChildNodes();
	unsigned int numConfigNodes = (configNodes != 0) ? configNodes->getLength() : 0;
	for ( unsigned int iNode = 0; iNode < numConfigNodes; ++iNode ) {
	  DOMNode* configNode = configNodes->item(iNode);
	  
//--- skip empty lines
//    (not removed by DOM parser)
	  if ( xoap::XMLCh2String(configNode->getLocalName()) == "" ) continue;

	  if ( xoap::XMLCh2String(configNode->getLocalName()) == "node" ) {
//--- initialize mapping between PVSS and XDAQ states and commands
//    for given FSM node type (control and logical units)

	    std::string deviceType = xoap::getNodeAttribute(configNode, "type");

	    try {
//--- use std::auto_ptr here
//    to avoid memory leak
//    in case exception is thrown
	      std::auto_ptr<PixelDCSFSMNodeDefinition> nodeDefinition(new PixelDCSFSMNodeDefinition(readFSMNodeDefinition(configNode)));

	      std::list<std::string> xdaqStateList = nodeDefinition->getXdaqStateList();
	      for ( std::list<std::string>::const_iterator xdaqState = xdaqStateList.begin();
		    xdaqState != xdaqStateList.end(); ++xdaqState ) {
		if ( !((*xdaqState) == "LV_ON" || (*xdaqState) == "LV_ON_REDUCED" || (*xdaqState) == "LV_OFF" || (*xdaqState) == "UNDEFINED"
		       || (*xdaqState) == "HV_ON" ) ) {
		  XCEPT_RAISE (xdaq::exception::Exception, "Undefined XDAQ state");
		}
	      }

	      fsmNodeDefinitionMap_[deviceType] = nodeDefinition.release();
	    } catch ( xdaq::exception::Exception& e ) {
	      std::cout << "Caught Exception thrown by readFSMNodeType" << std::endl;
	      std::cout << " " << e.what() << std::endl;
	      XCEPT_RETHROW(xdaq::exception::Exception, "Error parsing config File", e);
	    }
	  } else if ( xoap::XMLCh2String(configNode->getLocalName()) == "device" ) {
//--- initialize mapping between XDAQ states and corresponding PVSS data-point values
//    for given FSM device type (device unit)

	    std::string deviceType = xoap::getNodeAttribute(configNode, "type");
	    std::string dpType = xoap::getNodeAttribute(configNode, "pvssDpType");

	    try {
//--- use std::auto_ptr here
//    to avoid memory leak
//    in case exception is thrown
	      std::auto_ptr<PixelDCSFSMDeviceDefinition> deviceDefinition(new PixelDCSFSMDeviceDefinition(readFSMDeviceDefinition(configNode)));

	      fsmDeviceDefinitionMap_[deviceType] = deviceDefinition.release();
	    } catch ( xdaq::exception::Exception& e ) {
	      std::cout << "Caught Exception thrown by readFSMDeviceType" << std::endl;
	      std::cout << " " << e.what() << std::endl;
	      XCEPT_RETHROW(xdaq::exception::Exception, "Error parsing config File", e);
	    }
	  } else if ( xoap::XMLCh2String(configNode->getLocalName()) == "partition" ) {
//--- initialize list of PVSS FSM nodes 
//    associated to given TTC partition
	    std::string fsmPartitionName = xoap::getNodeAttribute(configNode, "name");

	    std::cout << "reading definition of fsmPartition = " << fsmPartitionName << std::endl;

	    std::list<std::pair<std::string, PixelDCSFSMNodeA4602> > fsmNodeListA4602;
	    std::list<std::pair<std::string, PixelDCSFSMNodeA4603> > fsmNodeListA4603;
	    std::list<PixelDCSSOAPConnection> soapConnections;
	    
	    DOMNodeList* fsmNodes = configNode->getChildNodes();
	    unsigned int numFSMNodes = (fsmNodes != 0) ? fsmNodes->getLength() : 0;
	    for ( unsigned int iNode = 0; iNode < numFSMNodes; ++iNode ) {
	      DOMNode* fsmNode = fsmNodes->item(iNode);
	      
//--- skip empty lines
//    (not removed by DOM parser)
	      if ( xoap::XMLCh2String(fsmNode->getLocalName()) == "" ) continue;

	      if ( xoap::XMLCh2String(fsmNode->getLocalName()) == "node" ) {
		std::string fsmNodeType = xoap::getNodeAttribute(fsmNode, "type");
		std::string fsmNodeName = xoap::getNodeAttribute(fsmNode, "name");
		std::string fsmNodeDomain = xoap::getNodeAttribute(fsmNode, "domain");

                bool isEndcap = (fsmNodeDomain.find("EndCap",0) != std::string::npos);
                bool isBarrel = (!isEndcap); 

		//		std::cout<<"For "<<fsmNodeName << " isBarrel is " << isBarrel << std::endl;
		
		const PixelDCSFSMNodeDefinition* fsmNodeDefinition = NULL;
		
		for ( std::map<std::string, PixelDCSFSMNodeDefinition*>::const_iterator fsmNodeDefinitionEntry = fsmNodeDefinitionMap_.begin();
		      fsmNodeDefinitionEntry != fsmNodeDefinitionMap_.end(); ++fsmNodeDefinitionEntry ) {
		  if ( fsmNodeDefinitionEntry->first == fsmNodeType ) {
		    fsmNodeDefinition = fsmNodeDefinitionEntry->second;
		  }
		}

		if ( fsmNodeDefinition != NULL ) {
		  std::cout << " adding FSM node, type = " << fsmNodeType << " with name = " << fsmNodeName << "," 
			    << " domain = " << fsmNodeDomain << std::endl;
		  if ( fsmNodeType == "TkPowerGroup" ) {
//--- retrieve definition of read-out chip initialization status device 
//    associated to A4603 power supply node
		    std::string fsmDeviceType_ReadoutChipInitializationStatus = "";
		    std::string dpName_ReadoutChipInitializationStatus = "";

		    DOMNodeList* fsmNode_attributes = fsmNode->getChildNodes();
		    unsigned int numAttributes = (fsmNode_attributes != 0) ? fsmNode_attributes->getLength() : 0;
		    for ( unsigned int iAttribute = 0; iAttribute < numAttributes; ++iAttribute ) {
		      DOMNode* fsmNode_attribute = fsmNode_attributes->item(iAttribute);

//--- skip empty lines
//    (not removed by DOM parser)
		      if ( xoap::XMLCh2String(fsmNode_attribute->getLocalName()) == "" ) continue;

		      if ( xoap::XMLCh2String(fsmNode_attribute->getLocalName()) == "status" ) {
			if ( fsmDeviceType_ReadoutChipInitializationStatus == "" && dpName_ReadoutChipInitializationStatus == "" ) {
			  fsmDeviceType_ReadoutChipInitializationStatus = xoap::getNodeAttribute(fsmNode_attribute, "fsmType");
			  dpName_ReadoutChipInitializationStatus = xoap::getNodeAttribute(fsmNode_attribute, "dpName");			
			} else {
			  XCEPT_RAISE (xdaq::exception::Exception, "Multiple Read-out Chip Initialization Status devices associated to A4603 node");
			}
		      }
		    }
		    
		    if ( (fsmDeviceType_ReadoutChipInitializationStatus == "" || dpName_ReadoutChipInitializationStatus == "") && isEndcap ) {
		      XCEPT_RAISE (xdaq::exception::Exception, "No Read-out Chip Initialization Status device associated to A4603 node");
		    }

		    const PixelDCSFSMDeviceDefinition* fsmDeviceDefinition_ReadoutChipInitializationStatus = NULL;
		    
		    for ( std::map<std::string, PixelDCSFSMDeviceDefinition*>::const_iterator fsmDeviceDefinitionEntry = fsmDeviceDefinitionMap_.begin();
			  fsmDeviceDefinitionEntry != fsmDeviceDefinitionMap_.end(); ++fsmDeviceDefinitionEntry ) {
		      if ( fsmDeviceDefinitionEntry->first == fsmDeviceType_ReadoutChipInitializationStatus ) {
			fsmDeviceDefinition_ReadoutChipInitializationStatus = fsmDeviceDefinitionEntry->second;
		      }
		    }

		    if ( (fsmDeviceDefinition_ReadoutChipInitializationStatus != NULL) || isBarrel ) {
                      //if ( fsmDeviceDefinition_ReadoutChipInitializationStatus != NULL ) {
		      //  std::cout << "fsmDeviceDefinition_ReadoutChipInitializationStatus:" << std::endl;
		      //  fsmDeviceDefinition_ReadoutChipInitializationStatus->writeTo(std::cout);
		      //}
		      if ( dpName_ReadoutChipInitializationStatus != "" || isBarrel ) {
			PixelDCSFSMNodeA4603 fsmNode(fsmNodeName, *fsmNodeDefinition, 
						       fsmDeviceDefinition_ReadoutChipInitializationStatus, dpName_ReadoutChipInitializationStatus);
			fsmNode.setDomain(fsmNodeDomain);
                        //if ( fsmNode.getDeviceDefinition_ReadoutChipInitializationStatus() != NULL ) { 
			//  fsmNode.getDeviceDefinition_ReadoutChipInitializationStatus()->writeTo(std::cout);
			//}
			fsmNodeListA4603.push_back(std::pair<std::string, PixelDCSFSMNodeA4603>(fsmNodeName, fsmNode));
		      } else {
			XCEPT_RAISE (xdaq::exception::Exception, "No Read-out Chip Initialization Status devices associated to A4603 node");
		      }
		    } else {
		      XCEPT_RAISE (xdaq::exception::Exception, "Undefined Device Type");
		    }
		  } else if ( fsmNodeType == "FwCaenChannelCtrl" ) {
		    PixelDCSFSMNodeA4602 fsmNode(fsmNodeName, *fsmNodeDefinition);
		    fsmNode.setDomain(fsmNodeDomain);
		    fsmNodeListA4602.push_back(std::pair<std::string, PixelDCSFSMNodeA4602>(fsmNodeName, fsmNode));
		  } else {
		    XCEPT_RAISE (xdaq::exception::Exception, "Undefined Device Type");
		  }
		} else {
		  XCEPT_RAISE (xdaq::exception::Exception, "Undefined Device Type");
		}
	      } else if ( xoap::XMLCh2String(fsmNode->getLocalName()) == "connection" ) {
		std::string name = xoap::getNodeAttribute(fsmNode, "name");
		std::string type = xoap::getNodeAttribute(fsmNode, "type");
		unsigned int instance = atoi(xoap::getNodeAttribute(fsmNode, "instance").data());
		soapConnections.push_back(PixelDCSSOAPConnection(name, type, instance));
	      } else {
		XCEPT_RAISE (xdaq::exception::Exception, "Error parsing config File, id = #0000");
	      }
	    }
	    
	    if ( numFSMNodes > 0 && soapConnections.begin() != soapConnections.end() ) {
	      std::cout << "adding FSM partition = " << fsmPartitionName << std::endl;
	      std::cout << " list of associated nodes of type CAEN A4602:" << std::endl;
	      for ( std::list<std::pair<std::string, PixelDCSFSMNodeA4602> >::const_iterator fsmNode = fsmNodeListA4602.begin();
		    fsmNode != fsmNodeListA4602.end(); ++fsmNode ) {
		std::cout << "  " << fsmNode->second.getName() << std::endl;
	      }
	      std::cout << " list of associated nodes of type CAEN A4603:" << std::endl;
	      for ( std::list<std::pair<std::string, PixelDCSFSMNodeA4603> >::const_iterator fsmNode = fsmNodeListA4603.begin();
		    fsmNode != fsmNodeListA4603.end(); ++fsmNode ) {
		std::cout << "  " << fsmNode->second.getName() << std::endl;
	      }
	      std::cout << " SOAP application to be notified in case of state changes:" << std::endl;
	      for ( std::list<PixelDCSSOAPConnection>::const_iterator soapConnection = soapConnections.begin();
		    soapConnection != soapConnections.end(); ++soapConnection ) {
		soapConnection->writeTo(std::cout);
	      }
	      fsmPartitionList_.push_back(PixelDCSFSMPartition(fsmPartitionName, fsmNodeListA4602, fsmNodeListA4603, soapConnections));
	    } else {
	      if ( numFSMNodes == 0 ){
		XCEPT_RAISE (xdaq::exception::Exception, "No FSM Nodes associated to Partition");
	      }
	    
	      if ( soapConnections.begin() == soapConnections.end() ){
		XCEPT_RAISE (xdaq::exception::Exception, "No SOAP Connections defined");
	      }
	    }
	  } else {
	    XCEPT_RAISE (xdaq::exception::Exception, "Error parsing config File, id = #0001");
	  }
	}
      } else {
	XCEPT_RAISE (xdaq::exception::Exception, "Error parsing config File, id = #0002");
      }
    } else {
      XCEPT_RAISE (xdaq::exception::Exception, "Could not open config File");
    }
  } catch ( xcept::Exception& e ) {
    std::cout << "Caught Exception thrown by loadXML" << std::endl;
    std::cout << " " << e.what() << std::endl;
    XCEPT_RETHROW(xdaq::exception::Exception, "Failed to parse config File. " + std::string(e.what()), e);
  }

  std::cout << " finished parsing config file." << std::endl;
  configFileLoaded_ = true;
}

//
//---------------------------------------------------------------------------------------------------
//

template <class T> std::string PixelDCSFSMInterface::getPVSSFSMNodeColor(const T* fsmDeviceDefinition, const std::string& fsmNodeState)
{
//--- set FSM node type and color to some default value
//    used in case no device definition is available 
  std::string fsmNodeType = "Undefined";
  std::string fsmNodeColor = defaultColor;
  if ( fsmDeviceDefinition != NULL ) {
    if ( !(fsmNodeState == "" || fsmNodeState == "UNDEFINED" || fsmNodeState == "DEAD") ) {
      fsmNodeColor = fsmDeviceDefinition->getColor(fsmNodeState);
      if ( fsmNodeColor == "" ) std::cout << "No color defined for FSM node state " << fsmNodeState << std::endl;
    }
  }
  
  return fsmNodeColor;
}

std::string PixelDCSFSMInterface::getUsedDescription(bool nodeIsUsed) {
  std::string usedString = nodeIsUsed ? "Included" : "Ignored";
  return usedString;
}

std::string PixelDCSFSMInterface::getUsedColor(bool nodeIsUsed) {
  std::string usedColor = nodeIsUsed ? "#33FF33" : "#FF99CC";
  return usedColor;
}

void PixelDCSFSMInterface::Default(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception)
{
  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;

//--- instruct browser to automatically reload/update webpage every five seconds
  //*out << "<head>" << std::endl;
  //*out << "<meta http-equiv=\"refresh\" content=\"5\" >" << std::endl;
  //*out << "</head>" << std::endl;

  xgi::Utils::getPageHeader(*out, "Pixel DCS FSM Interface", "");

  *out << "<body onload=\"ajaxFunction()\">" << std::endl;

//--- add Javascript code for AJAX functionality
//    (automatic update of webpage whenever state of PVSS FSM node changes)
  std::string urlAjaxHandler = "/" + getApplicationDescriptor()->getURN() + "/AjaxHandler";
  std::cout << "urlAjaxHandler = " << urlAjaxHandler << std::endl;

  PixelAJAXCommander::printJavascriptHeader(*out);
 
  *out << "  xmlHttp.onreadystatechange = function() {" << std::endl;
//--- check that transmission of data from "server" process is complete
  *out << "    if ( xmlHttp.readyState == 4 ) {" << std::endl;  
  *out << "      var xmlDoc = xmlHttp.responseXML.documentElement;" << std::endl;
  for ( std::list<PixelDCSFSMPartition>::const_iterator fsmPartition = fsmPartitionList_.begin();
	fsmPartition != fsmPartitionList_.end(); ++fsmPartition ) {
    const std::string& fsmPartitionName = fsmPartition->getName();

    updateNodeState<PixelDCSFSMNodeA4603>(out, fsmPartitionName, fsmPartition->getNodeListA4603(), kA4603);
    updateNodeState<PixelDCSFSMNodeA4602>(out, fsmPartitionName, fsmPartition->getNodeListA4602(), kA4602);

    updateSummarizedStates(out, *fsmPartition);
  }
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
  *out << std::endl;

  PixelAJAXCommander::printJavascriptServerRequest(urlAjaxHandler, *out);

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
  
  //Manual Override Button
  const string url = "/" + getApplicationDescriptor()->getURN() + "/XgiHandler";
  
  if ( fsm_.getStateName(fsm_.getCurrentState()) != "Initial" ) {
    *out<<"<h2> Manual Override </h2>";
    *out<<"For use in case the power is 100% ON and the PSX server is unresponsive<br>"<<endl;
    *out << "<form name=\"input\" method=\"get\" action=\"" << url << "\" enctype=\"multipart/form-data\">";
    //i don't think the id= is needed, but why not
    *out << "<input type=\"submit\" name=\"Command\" id=\"ManualOverrideHVON\" value=\"ManualOverrideHVON\"/>";
    *out << "<input type=\"submit\" name=\"Command\" id=\"ManualOverrideHVOFF\" value=\"ManualOverrideHVOFF\"/>";
    *out << "</form>" << std::endl;
    *out<<"<hr/>"<< std::endl;
  }

//--- render Finite State Machine GUI
//    (only neccessary in case PixelDCSFSMInterface has not yet been initialized)
  if ( fsm_.getStateName(fsm_.getCurrentState()) == "Initial" ) {
    std::set<std::string> allInputs = fsm_.getInputs();
    std::set<std::string> clickableInputs = fsm_.getInputs(fsm_.getCurrentState());
  
    *out<<"<h2> Finite State Machine </h2>";

    *out << "<form name=\"input\" method=\"get\" action=\"" << url << "\" enctype=\"multipart/form-data\">";
  
    *out << "<table border cellpadding=10 cellspacing=0>";
    *out << "<tr>";
    *out << "<td> <b>Current State</b> <br/>" << "<div id=\"xdaqCurrentFSMState\">" << fsm_.getStateName(fsm_.getCurrentState()) << "</div>" << "</td>";
    *out << "<td colspan=2>";
    for ( std::set<std::string>::iterator i = allInputs.begin();
	  i != allInputs.end(); ++i ) {

//--- skip "invisible" (internally triggered)
//    state transitions
      if ( i->find("invisible") != std::string::npos ) continue;

      if ( clickableInputs.find(*i) != clickableInputs.end() ) {
        HTML2XGI(out, XDAQ_ROOT + "/pixel/PixelDCSInterface/html/" + (*i) + ".htm");
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
  
    *out<<"<hr/>"<< std::endl;
  }

//--- render low-level custom GUI;
//    loop over partitions  
  for ( std::list<PixelDCSFSMPartition>::const_iterator fsmPartition = fsmPartitionList_.begin();
	fsmPartition != fsmPartitionList_.end(); ++fsmPartition ) {
    const std::string& fsmPartitionName = fsmPartition->getName();

    *out << "<h2>Partition " << fsmPartitionName << "</h2>" << std::endl;

//--- display state information 
//    for individual PVSS FSM nodes
    displayNodeState<PixelDCSFSMNodeA4603>(out, fsmPartitionName, fsmPartition->getNodeListA4603(), kA4603, fsmPartition->getSummarizedStateA4603());
    displayNodeState<PixelDCSFSMNodeA4602>(out, fsmPartitionName, fsmPartition->getNodeListA4602(), kA4602, fsmPartition->getSummarizedStateA4602());

    displaySummarizedStates(out, *fsmPartition);

    *out << cgicc::hr() << std::endl;
  }

  *out << "last updated: <text id=\"clock\">UNINITIALIZED</text>" << std::endl;
  //*out << "<div id=\"DEBUG\">debug output here !!!</div>" << std::endl;

  *out << "</body>" << std::endl;
  *out << "</html>";
}

template <class T> void PixelDCSFSMInterface::displayNodeState(xgi::Output* out,
							       const std::string& fsmPartitionName, const std::list<const T*> fsmNodeList, unsigned nodeType,
							       const std::string& fsmSummarizedState)
{
  if ( !(nodeType == kA4602 || nodeType == kA4603 ) ) {
    XCEPT_RAISE (xdaq::exception::Exception, "Undefined node Type");
  }

  unsigned numColumns_state = 0;
  switch ( nodeType ) {
  case kA4602 :
    *out << "<h3>CAEN A4602 Power Supply Boards</h3>" << std::endl;
    numColumns_state = 1+1;
    break;
  case kA4603 :
    *out << "<h3>CAEN A4603 Power Supply Boards</h3>" << std::endl;
    numColumns_state = 2+1;
  }

  *out << cgicc::table().set("border", "2") << std::endl;
  //*out << cgicc::table().set("border cellpadding","10").set("cellspacing","0") << std::endl;
  *out << cgicc::tr() << cgicc::th().set("colspan", "5").set("align", "center") << "FSM Node" << cgicc::th() << cgicc::tr() << std::endl;
  char colspanBuffer[2];
  sprintf(colspanBuffer, "%d", numColumns_state);
  
  switch ( nodeType ) {
  case kA4602 :
    *out << cgicc::tr() 
	 << cgicc::td().set("align", "center") << "Name" << cgicc::td() 
	 << cgicc::td().set("align", "center") << "Type" << cgicc::td()
	 << cgicc::td().set("align", "center") << "State" << cgicc::td()
      	 << cgicc::td().set("align", "center") << "Status" << cgicc::td()
	 << cgicc::tr() << std::endl;
    break;
  case kA4603 :
    *out << cgicc::tr() 
	 << cgicc::td().set("rowspan", "2").set("align", "center") << "Name" << cgicc::td() 
	 << cgicc::td().set("rowspan", "2").set("align", "center") << "Type" << cgicc::td()
	 << cgicc::td().set("colspan", colspanBuffer).set("align", "center") << "State" << cgicc::td()
	 << cgicc::tr() << std::endl;
    *out << cgicc::tr()
       //<< cgicc::td().set("colspan", "2").set("align", "center") << "" << cgicc::td()
	 << cgicc::td().set("align", "center") << "Power" << cgicc::td()
	 << cgicc::td().set("align", "center") << "Read-out Chips" << cgicc::td()
      	 << cgicc::td().set("align", "center") << "Status" << cgicc::td()
	 << cgicc::tr() << std::endl;
  }
  
  for ( typename std::list<const T*>::const_iterator fsmNode = fsmNodeList.begin();
	fsmNode != fsmNodeList.end(); ++fsmNode ) {
    const std::string& fsmNodeName = (*fsmNode)->getName();
    const std::string& fsmNodeState = (*fsmNode)->getState();
    const bool& fsmNodeUsed = (*fsmNode)->isUsed();

//--- set FSM node type to some default value
//    used in case no device definition is available 
    std::string fsmNodeType = "Undefined";
    if ( (*fsmNode)->getDeviceDefinition() != NULL ) {
      fsmNodeType = (*fsmNode)->getDeviceDefinition()->getType();
    }

    std::string fsmNodeColor = getPVSSFSMNodeColor<PixelDCSFSMNodeDefinition>((*fsmNode)->getDeviceDefinition(), fsmNodeState);

//     std::cout << "fsmNodeType = " << fsmNodeType << std::endl;
//     std::cout << "fsmNodeColor = " << fsmNodeColor << std::endl;
//     std::cout << "fsmNodeState = " << fsmNodeState << std::endl;
//     std::cout << "fsmNodeUsed = " << fsmNodeUsed << std::endl;

    std::string tagName = "";
    std::string tagName_used ="";
    switch ( nodeType ) {
    case kA4602 :
      tagName = getTagNameA4602_state(fsmPartitionName, fsmNodeName);
      tagName_used = getTagNameA4602_state_used(fsmPartitionName, fsmNodeName);
      break;
    case kA4603 :
      tagName = getTagNameA4603_state_power(fsmPartitionName, fsmNodeName);
      tagName_used = getTagNameA4603_state_used(fsmPartitionName, fsmNodeName);
    }
    
    *out << cgicc::tr() 
	 << cgicc::td() << fsmNodeName << cgicc::td()
	 << cgicc::td() << fsmNodeType << cgicc::td()
	 << cgicc::td().set("align", "center").set("bgcolor", fsmNodeColor) << "<div id=\"" << tagName << "\">" << fsmNodeState << "</div>" << cgicc::td();
    
    switch ( nodeType ) {
    case kA4602 :
      break;
    case kA4603 :
      const PixelDCSFSMNodeA4603* fsmNodeA4603 = dynamic_cast<const PixelDCSFSMNodeA4603*>(*fsmNode);
      const std::string& state_readoutChipInitializationStatus = fsmNodeA4603->getState_ReadoutChipInitializationStatus();
      std::string color_readoutChipInitializationStatus = defaultColor;
      if ( !(state_readoutChipInitializationStatus == "" || state_readoutChipInitializationStatus == "UNDEFINED") ) {
	if ( fsmNodeA4603->getDeviceDefinition_ReadoutChipInitializationStatus() != NULL ) {
	  fsmNodeType = fsmNodeA4603->getDeviceDefinition_ReadoutChipInitializationStatus()->getType();
	
  	  color_readoutChipInitializationStatus = getPVSSFSMNodeColor<PixelDCSFSMDeviceDefinition>(fsmNodeA4603->getDeviceDefinition_ReadoutChipInitializationStatus(), state_readoutChipInitializationStatus);
        }
      }

//       std::cout << "fsmNodeType = " << fsmNodeType << std::endl;
//       std::cout << "color_readoutChipInitializationStatus = " << color_readoutChipInitializationStatus << std::endl;
//       std::cout << "state_readoutChipInitializationStatus = " << state_readoutChipInitializationStatus << std::endl;
      
      std::string tagName_readoutChipInitializationStatus = getTagNameA4603_state_readoutChipInitialization(fsmPartitionName, fsmNodeName);

      *out << cgicc::td().set("align", "center").set("bgcolor", color_readoutChipInitializationStatus) 
	   << "<div id=\"" << tagName_readoutChipInitializationStatus << "\">" << state_readoutChipInitializationStatus << "</div>" << cgicc::td();
    }
    
    *out << cgicc::td().set("align", "center").set("bgcolor", getUsedColor(fsmNodeUsed)) 
	 << "<div id=\"" << tagName_used << "\">" << getUsedDescription(fsmNodeUsed) << "</div>" << cgicc::td();
    
    *out << cgicc::tr();
  }
  
  *out << std::endl;   

  *out << cgicc::table() << std::endl;
}

template <class T> void PixelDCSFSMInterface::updateNodeState(xgi::Output* out,
							      const std::string& fsmPartitionName, const std::list<const T*> fsmNodeList, unsigned nodeType)
{
  //  std::cout << "<PixelDCSFSMInterface::updateNodeState>:" << std::endl;

  if ( !(nodeType == kA4602 || nodeType == kA4603 ) ) {
    XCEPT_RAISE (xdaq::exception::Exception, "Undefined node Type");
  }
  
  for ( typename std::list<const T*>::const_iterator fsmNode = fsmNodeList.begin();
	fsmNode != fsmNodeList.end(); ++fsmNode ) {
    const std::string& fsmNodeName = (*fsmNode)->getName();

    switch ( nodeType ) {
    case kA4602 : 
      {
	std::string xmlTagName_state = getTagNameA4602_state(fsmPartitionName, fsmNodeName);
	std::string xmlTagName_color = getTagNameA4602_color(fsmPartitionName, fsmNodeName);
	std::string xmlTagName_state_used = getTagNameA4602_state_used(fsmPartitionName, fsmNodeName);
	std::string xmlTagName_color_used = getTagNameA4602_color_used(fsmPartitionName, fsmNodeName);
	std::string httpTextfieldName = xmlTagName_state; //i don't understand the point of copying one string to another
	//	std::cout << " nodeType = A4602" << std::endl;
	//	std::cout << " httpTextfieldName = " << httpTextfieldName << std::endl;
	*out << "      document.getElementById('" + httpTextfieldName + "').innerHTML = xmlDoc.getElementsByTagName(\"" + xmlTagName_state + "\")[0].childNodes[0].nodeValue;" << std::endl;
	*out << "      document.getElementById('" + httpTextfieldName + "').style.backgroundColor = xmlDoc.getElementsByTagName(\"" + xmlTagName_color + "\")[0].childNodes[0].nodeValue;" << std::endl;
	*out << "      document.getElementById('" + xmlTagName_state_used + "').innerHTML = xmlDoc.getElementsByTagName(\"" + xmlTagName_state_used + "\")[0].childNodes[0].nodeValue;" << std::endl;
	*out << "      document.getElementById('" + xmlTagName_state_used + "').style.backgroundColor = xmlDoc.getElementsByTagName(\"" + xmlTagName_color_used + "\")[0].childNodes[0].nodeValue;" << std::endl;
      }
      break;
    case kA4603 : 
      {
	std::string xmlTagName_state_power = getTagNameA4603_state_power(fsmPartitionName, fsmNodeName);
	std::string xmlTagName_color_power = getTagNameA4603_color_power(fsmPartitionName, fsmNodeName);
	std::string httpTextfieldName_power = xmlTagName_state_power;
	std::string xmlTagName_state_readoutChipInitialization = getTagNameA4603_state_readoutChipInitialization(fsmPartitionName, fsmNodeName);
	std::string xmlTagName_color_readoutChipInitialization = getTagNameA4603_color_readoutChipInitialization(fsmPartitionName, fsmNodeName);
	std::string httpTextfieldName_readoutChipInitialization = xmlTagName_state_readoutChipInitialization;
	std::string xmlTagName_state_used = getTagNameA4603_state_used(fsmPartitionName, fsmNodeName);
	std::string xmlTagName_color_used = getTagNameA4603_color_used(fsmPartitionName, fsmNodeName);
	//	std::cout << " nodeType = A4603" << std::endl;
	//	std::cout << " httpTextfieldName_power = " << httpTextfieldName_power << std::endl;
	//	std::cout << " httpTextfieldName_readoutChipInitialization = " << httpTextfieldName_readoutChipInitialization << std::endl;
	*out << "      document.getElementById('" + httpTextfieldName_power + "').innerHTML" 
	     << " = xmlDoc.getElementsByTagName(\"" + xmlTagName_state_power + "\")[0].childNodes[0].nodeValue;" << std::endl;
	*out << "      document.getElementById('" + httpTextfieldName_power + "').style.backgroundColor"
	     << " = xmlDoc.getElementsByTagName(\"" + xmlTagName_color_power + "\")[0].childNodes[0].nodeValue;" << std::endl;
	*out << "      document.getElementById('" + httpTextfieldName_readoutChipInitialization + "').innerHTML"
	     << " = xmlDoc.getElementsByTagName(\"" + xmlTagName_state_readoutChipInitialization + "\")[0].childNodes[0].nodeValue;" << std::endl;
	*out << "      document.getElementById('" + httpTextfieldName_readoutChipInitialization + "').style.backgroundColor"
	     << " = xmlDoc.getElementsByTagName(\"" + xmlTagName_color_readoutChipInitialization + "\")[0].childNodes[0].nodeValue;" << std::endl;
	*out << "      document.getElementById('" + xmlTagName_state_used + "').innerHTML" 
	     << " = xmlDoc.getElementsByTagName(\"" + xmlTagName_state_used + "\")[0].childNodes[0].nodeValue;" << std::endl;
	*out << "      document.getElementById('" + xmlTagName_state_used + "').style.backgroundColor"
	     << " = xmlDoc.getElementsByTagName(\"" + xmlTagName_color_used + "\")[0].childNodes[0].nodeValue;" << std::endl;
      }
    }
  }
}

void PixelDCSFSMInterface::displaySummarizedStates(xgi::Output* out, const PixelDCSFSMPartition& fsmPartition)
{
//--- display summarized XDAQ state
//    send to FED and FEC Supervisors

  *out << "<h3>Summarized States</h3>" << std::endl;

  *out << cgicc::table().set("border", "2") << std::endl;
  *out << cgicc::tr() << cgicc::th().set("colspan", "2").set("align", "center") << "Connection" << cgicc::th() << cgicc::tr() << std::endl;
  *out << cgicc::tr()
       << cgicc::td().set("align", "center") << "Name" << cgicc::td() 
       << cgicc::td().set("align", "center") << "State" << cgicc::td() 
       << cgicc::tr() << std::endl;

  const std::string& fsmPartitionName = fsmPartition.getName();

  const std::string& fsmPartition_summarizedStateA4602 = fsmPartition.getSummarizedStateA4602();
  const std::string& fsmPartition_summarizedStateA4603 = fsmPartition.getSummarizedStateA4603();

  const std::list<PixelDCSSOAPConnection>& soapConnections = fsmPartition.getSOAPConnections();
  for ( std::list<PixelDCSSOAPConnection>::const_iterator soapConnection = soapConnections.begin();
	soapConnection != soapConnections.end(); ++soapConnection ) {
    const std::string& soapConnectionName = soapConnection->getName();
    const std::string& soapConnectionType = soapConnection->getType();

    std::string soapConnection_summarizedState = getSummarizedState(soapConnectionType, fsmPartition_summarizedStateA4602, 
								    fsmPartition_summarizedStateA4603);

    //the string now looks like <LV state>;<HV state> for A4603
    std::string color = defaultColor;
    if ( soapConnection_summarizedState == "LV_ON;HV_ON" || soapConnection_summarizedState =="LV_ON"  ) color = "#33FF33";
    if ( soapConnection_summarizedState.find("LV_ON_REDUCED")!=std::string::npos ||
	 soapConnection_summarizedState.find("LV_OFF")!=std::string::npos ||
	 soapConnection_summarizedState.find("HV_OFF")!=std::string::npos  ) color = "#3333FF";
    
    std::string tagName = getTagNamePartitionSummary_state(fsmPartitionName, soapConnectionName);

    *out << cgicc::tr()
	 << cgicc::td() << soapConnectionName << cgicc::td() 
	 << cgicc::td().set("bgcolor", color) << "<div id=\"" << tagName << "\">" << soapConnection_summarizedState << "</div>" << cgicc::td()
	 << cgicc::tr() << std::endl;
  }

  *out << cgicc::table() << std::endl;
}

void PixelDCSFSMInterface::updateSummarizedStates(xgi::Output* out, const PixelDCSFSMPartition& fsmPartition)
{
  const std::string& fsmPartitionName = fsmPartition.getName();
  
  const std::list<PixelDCSSOAPConnection>& soapConnections = fsmPartition.getSOAPConnections();
  for ( std::list<PixelDCSSOAPConnection>::const_iterator soapConnection = soapConnections.begin();
	soapConnection != soapConnections.end(); ++soapConnection ) {
    const std::string& soapConnectionName = soapConnection->getName();
    
    std::string xmlTagName_state = getTagNamePartitionSummary_state(fsmPartitionName, soapConnectionName);
    std::string xmlTagName_color = getTagNamePartitionSummary_color(fsmPartitionName, soapConnectionName);
    std::string httpTextfieldName = xmlTagName_state;
    
    *out << "      document.getElementById('" + httpTextfieldName + "').innerHTML = xmlDoc.getElementsByTagName(\"" + xmlTagName_state + "\")[0].childNodes[0].nodeValue;" << std::endl;
    *out << "      document.getElementById('" + httpTextfieldName + "').style.backgroundColor = xmlDoc.getElementsByTagName(\"" + xmlTagName_color + "\")[0].childNodes[0].nodeValue;" << std::endl;
  }
}

void PixelDCSFSMInterface::XgiHandler(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception)
{
  cgicc::Cgicc cgi(in);

  std::string command = cgi.getElement("Command")->getValue();
  if ( command == "Initialize" ) {
    xoap::MessageReference soapMessage = MakeSOAPMessageReference("Initialize");
    xoap::MessageReference soapResponse = Initialize(soapMessage);
    if ( Receive(soapResponse) != "InitializeDone") {
      std::cout << "The PixelDCSFSMInterface could not be Initialized" << std::endl;
    }
  }
  else if ( command=="ManualOverrideHVON" ) {
    lock_->take();
    readyToConfigure_=true;
    lock_->give();
    ::sleep(5); //this sleep is a kludge to let the Configure step finish
    //a better implementation would be to wait here for a flag that configuration has finished (!)
    updateSupervisors(xdaqState_LV_ON,xdaqState_HV_ON);
  }
  else if ( command=="ManualOverrideHVOFF" ) {
    lock_->take();
    readyToConfigure_=true;
    lock_->give();
    ::sleep(5); //this sleep is a kludge to let the Configure step finish
    //a better implementation would be to wait here for a flag that configuration has finished (!)
    updateSupervisors(xdaqState_LV_ON,xdaqState_LV_ON);
  }


  this->Default(in, out);
}

void PixelDCSFSMInterface::AjaxHandler(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception) 
{
//--- compose XML message containing current state of PVSS FSM nodes

  // std::cout << "<PixelDCSFSMInterface::AjaxHandler>:" << std::endl;

//--- create header
  cgicc::HTTPResponseHeader response("HTTP/1.1", 200, "OK"); 
  response.addHeader("Content-Type", "text/xml"); 

//--- add XML content
  *out << "<AjaxHandler>" << std::endl;

  for ( std::list<PixelDCSFSMPartition>::const_iterator fsmPartition = fsmPartitionList_.begin();
	fsmPartition != fsmPartitionList_.end(); ++fsmPartition ) {
    const std::string& fsmPartitionName = fsmPartition->getName();

    *out << "<pvssFSMPartition name=\"" << fsmPartitionName << "\">" << std::endl;

//--- add XML tags describing state of CAEN A4602 power supplies
    std::list<const PixelDCSFSMNodeA4602*> fsmNodesA4602 = fsmPartition->getNodeListA4602();
    for ( std::list<const PixelDCSFSMNodeA4602*>::const_iterator fsmNodeA4602 = fsmNodesA4602.begin();
	  fsmNodeA4602 != fsmNodesA4602.end(); ++fsmNodeA4602 ) {
      const std::string& fsmNodeName = (*fsmNodeA4602)->getName();
      const std::string& fsmNodeState = (*fsmNodeA4602)->getState();
      const bool& fsmNodeUsed = (*fsmNodeA4602)->isUsed();

      std::string fsmNodeColor = getPVSSFSMNodeColor<PixelDCSFSMNodeDefinition>((*fsmNodeA4602)->getDeviceDefinition(), fsmNodeState);

      std::string xmlTagName_state = getTagNameA4602_state(fsmPartitionName, fsmNodeName);
      std::string tag_state = std::string("<") + xmlTagName_state + ">" + fsmNodeState + "</" + xmlTagName_state + ">";
      *out << tag_state << std::endl;
      std::string xmlTagName_color = getTagNameA4602_color(fsmPartitionName, fsmNodeName);
      std::string tag_color = std::string("<") + xmlTagName_color + ">" + fsmNodeColor + "</" + xmlTagName_color + ">";
      *out << tag_color << std::endl;

      std::string xmlTagName_state_used = getTagNameA4602_state_used(fsmPartitionName, fsmNodeName);
      std::string tag_state_used = std::string("<") + xmlTagName_state_used + ">" + getUsedDescription(fsmNodeUsed) + "</" + xmlTagName_state_used + ">";
      *out << tag_state_used << std::endl;
      std::string xmlTagName_color_used = getTagNameA4602_color_used(fsmPartitionName, fsmNodeName);
      std::string tag_color_used = std::string("<") + xmlTagName_color_used + ">" + getUsedColor(fsmNodeUsed) + "</" + xmlTagName_color_used + ">";
      *out << tag_color_used << std::endl;
    }

//--- add XML tags describing state of CAEN A4603 power supplies ()
//    (and initialization status of read-out chips connected to them)
    std::list<const PixelDCSFSMNodeA4603*> fsmNodesA4603 = fsmPartition->getNodeListA4603();
    for ( std::list<const PixelDCSFSMNodeA4603*>::const_iterator fsmNodeA4603 = fsmNodesA4603.begin();
	  fsmNodeA4603 != fsmNodesA4603.end(); ++fsmNodeA4603 ) {
      const std::string& fsmNodeName = (*fsmNodeA4603)->getName();

      const std::string& fsmNodeState_power = (*fsmNodeA4603)->getState();
      std::string fsmNodeColor_power = getPVSSFSMNodeColor<PixelDCSFSMNodeDefinition>((*fsmNodeA4603)->getDeviceDefinition(), fsmNodeState_power);

      std::string xmlTagName_state_power = getTagNameA4603_state_power(fsmPartitionName, fsmNodeName);
      std::string tag_state_power = std::string("<") + xmlTagName_state_power + ">" + fsmNodeState_power + "</" + xmlTagName_state_power + ">";
      *out << tag_state_power << std::endl;
      std::string xmlTagName_color_power = getTagNameA4603_color_power(fsmPartitionName, fsmNodeName);
      std::string tag_color_power = std::string("<") + xmlTagName_color_power + ">" + fsmNodeColor_power + "</" + xmlTagName_color_power + ">";
      *out << tag_color_power << std::endl;

      if ( (*fsmNodeA4603)->getDeviceDefinition_ReadoutChipInitializationStatus() != NULL ) { 
        const std::string& fsmNodeState_readoutChipInitialization = (*fsmNodeA4603)->getState_ReadoutChipInitializationStatus();
        std::string fsmNodeColor_readoutChipInitialization = getPVSSFSMNodeColor<PixelDCSFSMDeviceDefinition>((*fsmNodeA4603)->getDeviceDefinition_ReadoutChipInitializationStatus(), fsmNodeState_readoutChipInitialization);

        std::string xmlTagName_state_readoutChipInitialization = getTagNameA4603_state_readoutChipInitialization(fsmPartitionName, fsmNodeName);
        std::string tag_state_readoutChipInitialization = 
	  std::string("<") + xmlTagName_state_readoutChipInitialization + ">" + fsmNodeState_readoutChipInitialization + "</" + xmlTagName_state_readoutChipInitialization + ">";
        *out << tag_state_readoutChipInitialization << std::endl;
        std::string xmlTagName_color_readoutChipInitialization = getTagNameA4603_color_readoutChipInitialization(fsmPartitionName, fsmNodeName);
        std::string tag_color_readoutChipInitialization = 
	  std::string("<") + xmlTagName_color_readoutChipInitialization + ">" + fsmNodeColor_readoutChipInitialization + "</" + xmlTagName_color_readoutChipInitialization + ">";
        *out << tag_color_readoutChipInitialization << std::endl;
      }

      const bool& fsmNodeUsed = (*fsmNodeA4603)->isUsed();

      std::string xmlTagName_state_used = getTagNameA4603_state_used(fsmPartitionName, fsmNodeName);
      std::string tag_state_used = std::string("<") + xmlTagName_state_used + ">" + getUsedDescription(fsmNodeUsed) + "</" + xmlTagName_state_used + ">";
      *out << tag_state_used << std::endl;
      std::string xmlTagName_color_used = getTagNameA4603_color_used(fsmPartitionName, fsmNodeName);
      std::string tag_color_used = std::string("<") + xmlTagName_color_used + ">" + getUsedColor(fsmNodeUsed) + "</" + xmlTagName_color_used + ">";
      *out << tag_color_used << std::endl;

    }

//--- add XML tags describing summarized states
//    that are send to the various connected Supervisor instances
    const std::string& fsmPartition_summarizedStateA4602 = fsmPartition->getSummarizedStateA4602();
    const std::string& fsmPartition_summarizedStateA4603 = fsmPartition->getSummarizedStateA4603();

    const std::list<PixelDCSSOAPConnection>& soapConnections = fsmPartition->getSOAPConnections();
    for ( std::list<PixelDCSSOAPConnection>::const_iterator soapConnection = soapConnections.begin();
	  soapConnection != soapConnections.end(); ++soapConnection ) {
      const std::string& soapConnectionName = soapConnection->getName();
      const std::string& soapConnectionType = soapConnection->getType();

      std::string soapConnection_summarizedState = getSummarizedState(soapConnectionType, fsmPartition_summarizedStateA4602, 
								      fsmPartition_summarizedStateA4603);

      std::string xmlTagName_state = getTagNamePartitionSummary_state(fsmPartitionName, soapConnectionName);
      std::string tag_state = std::string("<") + xmlTagName_state + ">" + soapConnection_summarizedState + "</" + xmlTagName_state + ">";
      *out << tag_state << std::endl;

      //the string now looks like <LV state>;<HV state> for the A4603
      std::string soapConnection_color = defaultColor;
      if ( soapConnection_summarizedState == "LV_ON;HV_ON" || soapConnection_summarizedState =="LV_ON"  ) soapConnection_color = "#33FF33";
      if ( soapConnection_summarizedState.find("LV_ON_REDUCED")!=std::string::npos ||
	   soapConnection_summarizedState.find("LV_OFF")!=std::string::npos ||
	   soapConnection_summarizedState.find("HV_OFF")!=std::string::npos  ) soapConnection_color = "#3333FF";
      

      std::string xmlTagName_color = getTagNamePartitionSummary_color(fsmPartitionName, soapConnectionName);
      std::string tag_color = std::string("<") + xmlTagName_color + ">" + soapConnection_color + "</" + xmlTagName_color + ">";
      *out << tag_color << std::endl;
    }

    *out << "</pvssFSMPartition>" << std::endl;
  }

  *out << "<xdaqFSM>" << std::endl;

  std::string tag_xdaqCurrentState = std::string("<xdaqCurrentState>") + fsm_.getStateName(fsm_.getCurrentState()) + "</xdaqCurrentState>";
  *out << tag_xdaqCurrentState << std::endl;

  std::set<std::string> allInputs = fsm_.getInputs();
  std::set<std::string> clickableInputs = fsm_.getInputs(fsm_.getCurrentState());

  for ( std::set<std::string>::iterator input = allInputs.begin();
	input != allInputs.end(); ++input ) {
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
//    (to be processed by Javascript code in PixelDCSFSMInterface::Default method)
  out->setHTTPResponseHeader(response);  
}

//
//---------------------------------------------------------------------------------------------------
//
  
xoap::MessageReference PixelDCSFSMInterface::Initialize(xoap::MessageReference soapMessage) throw (xoap::exception::Exception)
{
//--- establish connection to PSX server
//    and subscribe to states of FSM nodes

//FIXME assuming diagSystem gives timestamp, we can dump PixelTimer version
  PixelTimer debugTimer;
  debugTimer.setName("PixelDCSFSMInterface::Initialize");
  debugTimer.printTime(" -- start Initialize -- ");
  diagService_->reportError(" -- INITIALIZE -- ",DIAGINFO);


//--- do not attempt to "Initialized"
//    if already initialized
  if ( fsm_.getStateName(fsm_.getCurrentState()) == "Halted" ) {
    diagService_->reportError("PixelDCSFSMInterface is already initialized",DIAGERROR);
    return MakeSOAPMessageReference("InitializeFailed");
  }

  // Detect PixelSupervisor
  try {
    PixelSupervisor_=getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor("PixelSupervisor", 0);
    diagService_->reportError("PixelDCSFSMInterface::Initialize - Instance 0 of PixelSupervisor found.",DIAGTRACE);
  } catch (xdaq::exception::Exception& e) {
    diagService_->reportError("PixelDCSFSMInterface::Initialize - Instance 0 of PixelSupervisor NOT found!",DIAGFATAL);
  }

  //start Disconnect workloop
  disconnectWorkloop_ = toolbox::task::getWorkLoopFactory()->getWorkLoop("smiDisconnectWorkloop", "waiting");
  toolbox::task::ActionSignature* task = toolbox::task::bind(this, &PixelDCSFSMInterface::disconnectFromFSM_workloop, "disconnectFromFSM_workloop");
  disconnectWorkloop_->submit(task);
  disconnectWorkloop_->activate();

  xoap::MessageReference soapRequest = MakeSOAPMessageReference("Connect");
  xoap::MessageReference soapResponse = Connect(soapRequest);

  if ( Receive(soapResponse) == "ConnectDone") {
    try {
      toolbox::Event::Reference e(new toolbox::Event("Initialize", this));
      fsm_.fireEvent(e);
    } catch (toolbox::fsm::exception::Exception & e) {
      XCEPT_RETHROW(xoap::exception::Exception, "Invalid State Machine Input.", e);
    }

    return MakeSOAPMessageReference("InitializeDone");
  } else {
    return MakeSOAPMessageReference("InitializeFailed");
  }
}

//
//---------------------------------------------------------------------------------------------------
// 
xoap::MessageReference PixelDCSFSMInterface::Halt(xoap::MessageReference msg) throw (xoap::exception::Exception)
{

  diagService_->reportError(" -- HALT -- ",DIAGINFO);

  try {
    toolbox::Event::Reference e(new toolbox::Event("Halt", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    XCEPT_RETHROW(xoap::exception::Exception, "Invalid Command.", e);
  }
  
  xoap::MessageReference reply = MakeSOAPMessageReference("HaltDone");
  return reply;

}

//
//---------------------------------------------------------------------------------------------------
// 
xoap::MessageReference PixelDCSFSMInterface::Configure(xoap::MessageReference msg) throw (xoap::exception::Exception)
{

  diagService_->reportError(" -- CONFIGURE -- ",DIAGINFO);

  Attribute_Vector parameters(1);
  parameters.at(0).name_="GlobalKey";
  Receive(msg, parameters);
  diagService_->reportError("[PixelDCSFSMInterface::Configure] The Global Key was received as "+parameters.at(0).value_,DIAGDEBUG);
  theGlobalKey_ = new PixelConfigKey (atoi(parameters.at(0).value_.c_str()));
  
  try {
    toolbox::Event::Reference e(new toolbox::Event("Configure", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    XCEPT_RETHROW(xoap::exception::Exception, "Invalid State Machine Input.", e);
  }
  
  xoap::MessageReference reply=MakeSOAPMessageReference ("ConfigureDone");
  return reply;
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelDCSFSMInterface::stateConfiguring(toolbox::fsm::FiniteStateMachine &fsm) throw (toolbox::fsm::exception::Exception)
{

  diagService_->reportError(" -- stateConfiguring -- ",DIAGINFO);

  state_=fsm.getStateName(fsm.getCurrentState());
  if (PixelSupervisor_!=0) {
    Attribute_Vector parameters(3);
    parameters[0].name_="Supervisor"; parameters[0].value_="PixelDCSFSMInterface";
    parameters[1].name_="Instance";   parameters[1].value_=itoa(0); //FIXME?
    parameters[2].name_="FSMState";   parameters[2].value_=state_;
    Send(PixelSupervisor_, "FSMStateNotification", parameters);
  }
  //cout<< "[PixelDCSFSMInterface::stateConfiguring] New state is:" <<std::string(state_)<<endl;;

  while (true) {
    lock_->take();
    if (readyToConfigure_) break;
    lock_->give();
    cout<<"Waiting to configure"<<endl;
    ::sleep(1);
  }
  lock_->give();
 
  //map to hold the status (used or not used) of each node
  map<std::string, bool> nodeIsUsedA4602;
  map<std::string, bool> nodeIsUsedA4603;

  PixelDetectorConfig* detconfig=0; 
  PixelConfigInterface::get(detconfig, "pixel/detconfig/", *theGlobalKey_); 
  assert(detconfig!=0); //FIXME this should not be an assert; instead we should be catching std::exception

  //get ROC status map
  map<PixelROCName, PixelROCStatus> roclistcopy = detconfig->getROCsList();

  const string fpixbase = "PixelEndCap";
  const string bpixbase = "PixelBarrel";
  map<PixelROCName,PixelROCStatus>::iterator iroc;
  for( iroc = roclistcopy.begin(); iroc != roclistcopy.end(); ++iroc ) {
    
    ///first we need to figure out which partition this is
    string rocname = iroc->first.rocname();
    string partition=rocname.substr(0, 8); //eg FPix_BmI

    //now we need to reconstruct the node name
    ///FIXME make this translation not so hardcoded?
      string nodenameA4603= "CMS_TRACKER";
      string nodenameA4602=nodenameA4603;
      string powergroup="";
      if (partition.find("FPix")!=string::npos ) { //fpix
	string disc = rocname.substr(5,6); //eg BpI_D1
	string blade = rocname.substr(12,5); //eg BLD7_ ; BLD11
	unsigned int bladenum = atoi((blade.substr(4,1)=="_") ? blade.substr(3,1).c_str() :  blade.substr(3,2).c_str());
	assert(bladenum>=1 && bladenum <=12);
	unsigned int rognum= (bladenum-1)/3 +1; //int arithmetic trick
	ostringstream os;
	os<< fpixbase<<"_"<<disc<<"_ROG"<<rognum;
	powergroup =os.str();
	nodenameA4603+= ":" + fpixbase;
	nodenameA4602+= ":" + fpixbase;
      }
      else if (partition.find("BPix") !=string::npos) {//bpix
	string sector = rocname.substr(5,8); //eg BpI_SEC7
	unsigned int layernum = atoi(rocname.substr(17,1).c_str());
	assert(layernum>=1 && layernum <=3);
	unsigned int rognum= (layernum==3) ? 3 : 1;
	//it sure is stupid that we use LYR in roc names and LAY in DCS. C'est la vie.
	ostringstream os;
	os<<bpixbase<<"_"<<sector.substr(0,5)<<sector.substr(7,1)<<"_LAY"<<rognum;
	powergroup=os.str();
	nodenameA4603+= ":" + bpixbase;
	nodenameA4602+= ":" + bpixbase;
      }
      else {diagService_->reportError("Error reading ROC list (neither FPix nor BPix!) for ROC="+rocname,DIAGFATAL); assert(0);}
      //finally we can assemble the full nodename
      nodenameA4603 += ":"+powergroup.substr(0,15) +":"+powergroup.substr(0,18)+":"+powergroup;
      nodenameA4602 += ":"+powergroup.substr(0,15) +":"+powergroup.substr(0,18)+":ControlPowerChann";
      //cout<<"ROC = "<<rocname<<" ; nodename = "<<nodenameA4603<<endl; //DEBUG
  
      //if the power is off then the ROC should be set to noInit
      if ( iroc->second.get(PixelROCStatus::noInit) || iroc->second.get(PixelROCStatus::noAnalogSignal)) {
	//cout<<"Found noInit for ROC "<<rocname<<endl;
	//only set to false if it is not yet defined in the map
	if ( nodeIsUsedA4603.find(nodenameA4603) == nodeIsUsedA4603.end() ) nodeIsUsedA4603[nodenameA4603] = false; 
	if ( nodeIsUsedA4602.find(nodenameA4602) == nodeIsUsedA4602.end() ) nodeIsUsedA4602[nodenameA4602] = false;
      }
      else { 
	//cout<<"Found that ROC is used "<<rocname<<endl;
	nodeIsUsedA4603[nodenameA4603] = true;
	nodeIsUsedA4602[nodenameA4602] = true;
      }

  } //loop over rocs
  
  
  //DEBUG
  //   map<std::string, bool>::const_iterator i_nodeIsUsedA4602;
  //   for (i_nodeIsUsedA4602 = nodeIsUsedA4602.begin() ; i_nodeIsUsedA4602 != nodeIsUsedA4602.end(); ++i_nodeIsUsedA4602) 
  //     cout<< (*i_nodeIsUsedA4602).first <<"\t"<< (*i_nodeIsUsedA4602).second<<endl;
  
  //   map<std::string, bool>::const_iterator i_nodeIsUsedA4603;
  //  for (i_nodeIsUsedA4603 = nodeIsUsedA4603.begin() ; i_nodeIsUsedA4603 != nodeIsUsedA4603.end(); ++i_nodeIsUsedA4603) 
  //    cout<< (*i_nodeIsUsedA4603).first <<"\t"<< (*i_nodeIsUsedA4603).second<<endl;
  
  // now we need to set the Used flag for each node
  //loop over fsmPartitions
  for ( std::list<PixelDCSFSMPartition>::iterator fsmPartition = fsmPartitionList_.begin();
	fsmPartition != fsmPartitionList_.end(); ++fsmPartition ) {
    
    //loop over nodes within the partition
    std::list<const PixelDCSFSMNodeA4602*> fsmNodesA4602 = (*fsmPartition).getNodeListA4602();
    for ( std::list<const PixelDCSFSMNodeA4602*>::const_iterator fsmNodeA4602 = fsmNodesA4602.begin();
	  fsmNodeA4602 != fsmNodesA4602.end(); ++fsmNodeA4602 ) {
      
      string nodename=(*fsmNodeA4602)->getName();
      if (nodeIsUsedA4602.find(nodename)==nodeIsUsedA4602.end() ) {
 	diagService_->reportError("[PixelDCSFSMInterface::stateConfiguring] Node "+nodename+" is not in the detconfig!",DIAGINFO);
	(*fsmPartition).setNodeUsedA4602(nodename,false);
      }
      else {
	(*fsmPartition).setNodeUsedA4602(nodename,nodeIsUsedA4602[nodename]);
 	ostringstream os;
 	os<<"[PixelDCSFSMInterface::stateConfiguring] Node = "<<nodename<<" ; state = ";
 	if (nodeIsUsedA4602[nodename]) {os<<"Used";} else {os<<"NOT used";}
 	diagService_->reportError(os.str(),DIAGDEBUG);
      }
    }

    std::list<const PixelDCSFSMNodeA4603*> fsmNodesA4603 = (*fsmPartition).getNodeListA4603();
    for ( std::list<const PixelDCSFSMNodeA4603*>::const_iterator fsmNodeA4603 = fsmNodesA4603.begin();
	  fsmNodeA4603 != fsmNodesA4603.end(); ++fsmNodeA4603 ) {
      
      string nodename=(*fsmNodeA4603)->getName();
      if (nodeIsUsedA4603.find(nodename)==nodeIsUsedA4603.end() ) {
 	diagService_->reportError("[PixelDCSFSMInterface::stateConfiguring] Node "+nodename+" is not in the detconfig!",DIAGINFO);
	(*fsmPartition).setNodeUsedA4603(nodename,false);
      }
      else  {
	(*fsmPartition).setNodeUsedA4603(nodename,nodeIsUsedA4603[nodename]);
 	ostringstream os;
 	os<<"[PixelDCSFSMInterface::stateConfiguring] Node = "<<nodename<<" ; state = ";
 	if (nodeIsUsedA4603[nodename]) {os<<"Used";} else {os<<"NOT used";}
 	diagService_->reportError(os.str(),DIAGDEBUG);
      }
    }


    //update states
    determineSummarizedStateA4602(*fsmPartition);
    determineSummarizedStateA4603(*fsmPartition);  
  }

  updateSupervisors(); //tell FEC and TKFEC if this detconfig has changed whether they are allowed to configure!
  
  // Fire a transition to Configured state
  //  std::cout<<"[PixelDCSFSMInterface::stateConfiguring] about to fire event ConfiguringDone"<<std::endl;
  try {
    toolbox::Event::Reference e(new toolbox::Event("ConfiguringDone", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
      XCEPT_RETHROW(xoap::exception::Exception, "Invalid Command.", e);
  }
  
  diagService_->reportError(" --- Exiting PixelDCSFSMInterface::stateConfiguring --- ",DIAGINFO);
  
}
//
//---------------------------------------------------------------------------------------------------
//

xoap::MessageReference PixelDCSFSMInterface::Connect(xoap::MessageReference soapMessage) throw (xoap::exception::Exception)
{
//--- request notification from PSX server
//    whenever the state of one of the PVSS FSM nodes
//    associated to this PixelDCSFSMInterface instance changes  


  PixelTimer debugTimer;
  debugTimer.setName("PixelDCSFSMInterface::Connect");
  debugTimer.printTime("-- connect --");

  if ( !configFileLoaded_ ) {
    try {
//--- initialize FSM state and command Translation Tables 
//    between XDAQ and PVSS
      this->loadConfigFile();
    } catch ( xdaq::exception::Exception& e ) {
      std::cout << "Caught Exception thrown by PixelDCSFSMInterface::loadConfigFile" << std::endl;
      std::cout << " " << e.what() << std::endl;
      XCEPT_RETHROW(xoap::exception::Exception, "Failed to parse config File. " + std::string(e.what()), e);
    }    
  }
  psxTimer_.start(); //start timing of connection process

  for ( std::list<PixelDCSFSMPartition>::const_iterator fsmPartition = fsmPartitionList_.begin();
	fsmPartition != fsmPartitionList_.end(); ++fsmPartition ) {

//--- instruct PSX server to notify PixelDCSFSMInterface 
//    whenever the state of a FSM node changes;
//    as the PSX server will immediately send a "notify" SOAP message 
//    containing the current state of the FSM node
//    upon receipt of the "connect" request,
//    the connectToFSM function must be run in a work-loop
//    to ensure that the "notify" message can be received and responded to by the PixelDCSFSMInterface
//    (otherwise the SOAP communication will be blocked after the first "connect" request,
//     as the PSX server is waiting for a reply to its "notify" message,
//     while the PixelDCSFSMInterface tries to send the second "connect" request)
//

//--- create a new work-loop
//    if not already active
    std::string workloopName = std::string("PixelDCSFSMInterface_workloop") + std::string("_") + std::string(fsmPartition->getName());

    lock_->take();
    bool workloopIsActive = !(workloopStatus_[workloopName] == "" || workloopStatus_[workloopName] == "inactive");
    lock_->give();
    if ( workloopIsActive ){
      std::cout << " work-loop still active --> skipping registration of connectToFSM_workloop member-function" << std::endl;
      continue;
    }

    toolbox::task::WorkLoop* workloop = toolbox::task::getWorkLoopFactory()->getWorkLoop(workloopName, "waiting");
    
//--- register connectToFSM_workloop member-function for execution in work-loop
    std::cout << " registering member-function \"connectToFSM_workloop\" for execution in work-loop \"" << workloopName << "\"" << std::endl;
    toolbox::task::ActionSignature* task = toolbox::task::bind(this, &PixelDCSFSMInterface::connectToFSM_workloop, "connectToFSM_workloop");

    workloop->submit(task);

//--- register
//    (signature of member-function registered for work-loop execution does not allow to pass function arguments 
//     other then the work-loop in which the member-function is executed)
    std::list<const PixelDCSFSMNode*> fsmNodeList = getNodeList(*fsmPartition);

    lock_->take();
    workloopData_[workloop->getName()] = fsmNodeList;
    workloopStatus_[workloop->getName()] = "active";
    lock_->give();

//--- start execution of connectToFSM_workloop member-function in work-loop
//    (start work-loop only if not already running)
    if ( !workloop->isActive() ) workloop->activate();
  }

  return MakeSOAPMessageReference("ConnectDone");
}

xoap::MessageReference PixelDCSFSMInterface::Disconnect(xoap::MessageReference soapMessage) throw (xoap::exception::Exception)
{
//--- cancel requested notifications from PSX server
//    for state changes of PVSS FSM nodes from PSX server

  std::cout << "<PixelDCSFSMInterface::Disconnect>" << std::endl;

  for ( std::list<PixelDCSFSMPartition>::const_iterator fsmPartition = fsmPartitionList_.begin();
	fsmPartition != fsmPartitionList_.end(); ++fsmPartition ) {
    std::list<const PixelDCSFSMNode*> fsmNodeList = getNodeList(*fsmPartition);
    for ( std::list<const PixelDCSFSMNode*>::const_iterator fsmNode = fsmNodeList.begin();
	  fsmNode != fsmNodeList.end(); ++fsmNode ) {
      const std::string& fsmNodeName = (*fsmNode)->getName();

      std::cout << " unsubscribing state of FSM node = " << fsmNodeName << std::endl;
      
      disconnectFromFSM(fsmNodeName);
    }
  }
  
  return MakeSOAPMessageReference("DisconnectDone");
}


//
//---------------------------------------------------------------------------------------------------
//
bool PixelDCSFSMInterface::disconnectFromFSM_workloop(toolbox::task::WorkLoop* workloop)
{
  cout<<"-- [PixelDCSFSMInterface::disconnectFromFSM_workloop] --"<<endl;
  PixelTimer debugTimer;
  debugTimer.start();
  lock_->take();
  readyToConnect_=false;
  lock_->give();

  //disconnect any old connections
  try {
    psxConnectionManager_->disconnectOldConnections(smiCommander_);
  }
  catch ( xcept::Exception & e) {
    diagService_->reportError("[PixelDCSFSMInterface::Connect] Caught exception while closing old PSX/SMI connections!",DIAGWARN);
    cout << "Exception details = " << e.what() << endl;
  }

  debugTimer.stop();
  cout<<"-- [PixelDCSFSMInterface::disconnectFromFSM_workloop] end of job (disconnect time = "
      <<debugTimer.tottime()<<" sec) --"<<endl;
  
  lock_->take();
  readyToConnect_=true;
  lock_->give();
  return false;
  
}

bool PixelDCSFSMInterface::connectToFSM_workloop(toolbox::task::WorkLoop* workloop)
{

  lock_->take();
  if ( !readyToConnect_ ) {
    lock_->give();
    ::sleep(4); 
    return true; 
  }
  //this line caused a (rather confusing) crash. I don't think it is necessary to cancel the
  //workloop, so for now I will remove it.
  //(note -- probably the crash was caused by the multithreading...)
  //  else  if ( disconnectWorkloop_->isActive() ) disconnectWorkloop_->cancel();

  const std::string& workloopName = workloop->getName();

  if ( workloopStatus_[workloopName] == "establishing connection" ) {
    std::cout << " waiting for connection to PSX server to be established" << std::endl;
    lock_->give();
    return true;
  }

  const std::list<const PixelDCSFSMNode*> fsmNodeList = workloopData_[workloopName];

  std::cout << "workloopName = " << workloopName << std::endl;
  std::cout << " workloopStatus = " << workloopStatus_[workloopName] << std::endl;

  workloopStatus_[workloopName] = "active";

  bool allNodesConnected = true;
  std::string fsmNodeName_connectToFSM = "";
  std::string fsmNodeDomain_connectToFSM = "";
  for ( std::list<const PixelDCSFSMNode*>::const_iterator fsmNode = fsmNodeList.begin();
	fsmNode != fsmNodeList.end(); ++fsmNode ) {
    const std::string& fsmNodeName = (*fsmNode)->getName();
    const std::string& fsmNodeDomain = (*fsmNode)->getDomain();
    
 
    //    cout<<"psxConnectionStatus_[fsmNodeName] "<<psxConnectionStatus_[fsmNodeName]<<endl; //DEBUG
    if ( psxConnectionStatus_[fsmNodeName] == "connected" ) {

//--- wait for "notify" SOAP message from PSX server
//    before sending further "connect" requests
//    (otherwise the SOAP communication will be blocked after the first "connect" request,
//     as the PSX server is waiting for a reply to its "notify" message,
//     while the PixelDCSFSMInterface tries to send the second "connect" request)
      if ( (*fsmNode)->getState() == xdaqState_UNDEFINED ) workloopStatus_[workloopName] = "waiting for notify";
    } else {
      fsmNodeName_connectToFSM = fsmNodeName;
      fsmNodeDomain_connectToFSM = fsmNodeDomain;
      allNodesConnected = false;
    }
  }

  if ( allNodesConnected ) {
//--- states of all PVSS FSM nodes subscribed to;
//    terminate execution of connectToFSM member function in work-loop
    std::cout << " states of all FSM nodes subscribed to, terminating execution of work-loop" << std::endl;
    workloopStatus_[workloopName] = "inactive";

//--- update state of XDAQ FSM;
//    initiate transition into "Configured" state 
//    when **all** FSM nodes for **all** partitions have been subscribed to
    bool allPartitionsConnected = true;
    for ( std::map<std::string, std::string>::const_iterator workloopStatus = workloopStatus_.begin(); 
	  workloopStatus != workloopStatus_.end(); ++workloopStatus ) {
      std::cout << "workloopStatus->first = " << workloopStatus->first << std::endl;
      std::cout << "workloopStatus->second = " << workloopStatus->second << std::endl;
      //because of a bug (I think), there are two workloopStatus->first for each HC, one
      //called PixelDCSFSMInterface_workloop_FPix_BmI and one called
      //PixelDCSFSMInterface_workloop_FPix_BmI/waiting
      //only the one called 'waiting' becomes 'inactive'
      //the other one has status ""
      //instead of fixing this, we add a hack here:
      if ( workloopStatus->second != "inactive" && workloopStatus->second != "" ) allPartitionsConnected = false;
    }

    if ( allPartitionsConnected ) {
      psxTimer_.stop();
      cout<<"Total PSX time (connection only) = "<<psxTimer_.tottime()<<endl;
      diagService_->reportError("Connections to PSX server are complete. We are ready to Configure.",DIAGUSERINFO);
      readyToConfigure_=true;
    }
    
//--- stop execution of work-loop
//    (work-loop gets automatically stopped once member-function 
//     registered for execution in work-loop returns false)
  cout<<"[PixelDCSFSMInterface::connectToFSM_workloop] about to give lock and return false"<<endl; //jmt debug

    lock_->give();
    return false;
  }
  
  bool workloopstatus=!(workloopStatus_[workloopName] == "waiting for notify");
  lock_->give();

  if ( workloopstatus ) {
    std::cout << " subscribing to state of FSM node = " << fsmNodeName_connectToFSM << std::endl;
    lock_->take();
    workloopStatus_[workloopName] = "establishing connection";
    lock_->give();
    connectToFSM(fsmNodeName_connectToFSM, fsmNodeDomain_connectToFSM, workloopName);
  }

//--- wait one second
//    in order to reduce CPU time consumed by work-loop
  ::sleep((long unsigned int)1);

  cout<<"[PixelDCSFSMInterface::connectToFSM_workloop] about to return"<<endl; //jmt debug
  return true;
}

void PixelDCSFSMInterface::connectToFSM(const std::string& fsmNodeName, 
					const std::string& fsmNodeDomain, 
					const std::string& workloopName) throw (xdaq::exception::Exception)
{
//--- compose and send SOAP message to the PSX server
//    to subscribe to the FSM state 
//    of the PVSS FSM node given as function argument

//--- do not attempt to connect to PSX server
//    if already connected
  lock_->take();
  bool myconnectionstatus=psxConnectionStatus_[fsmNodeName] == "connected" ;
  lock_->give();
  if ( myconnectionstatus ) return;

  PixelTimer debugTimer;

  debugTimer.start();

  std::cout<<"[PixelDCSFSMInterface::connectToFSM] Trying to establish connection to PSX server"<<std::endl; //jmt debug
  try {
    //don't want this line inside the lock_, because it can take a long time if PSX is slow
    std::pair<std::string, std::string> psxResponse = smiCommander_->connectToFSM(fsmNodeName, fsmNodeDomain, fsmNodeOwner);
    debugTimer.stop();

   std::cout<<"[PixelDCSFSMInterface::connectToFSM] connection to PSX server established. about to take lock"<<std::endl; //jmt debug

    lock_->take();
    psxConnectionStatus_[fsmNodeName] = psxResponse.first;
    psxConnectionId_[fsmNodeName] = psxResponse.second;
    workloopStatus_[workloopName] = "connection established";
    psxConnectionManager_->addOpenConnection(psxResponse.second);
    lock_->give();
    
    std::cout << "psxConnectionId set to " << psxResponse.second << "." << std::endl;
  }
  catch (xdaq::exception::Exception & e) { 
    debugTimer.stop();
    diagService_->reportError("[PixelDCSFSMInterface::connectToFSM] Caught exception: "+string(e.what()),DIAGWARN);
    lock_->take();
    workloopStatus_[workloopName] = "connection failed";
    psxConnectionStatus_[fsmNodeName] = "disconnected";
    lock_->give();
  }
  cout<<"This connection took "<<debugTimer.tottime()<<" seconds"<<endl;

}

void PixelDCSFSMInterface::disconnectFromFSM(const std::string& fsmNodeName) throw (xdaq::exception::Exception)
{
//--- compose and send SOAP message to the PSX server
//    to unsubscribe to the FSM state 
//    of the PVSS FSM node given as function argument

//--- do not attempt to connect to PSX server
//    if already disconnected
  if ( psxConnectionStatus_[fsmNodeName] == "disconnected" ) return;

  std::string& psxConnectionId = psxConnectionId_[fsmNodeName];
  smiCommander_->disconnectFromFSM(psxConnectionId);

  psxConnectionStatus_[fsmNodeName] = "disconnected";
}

//
//---------------------------------------------------------------------------------------------------
//

xoap::MessageReference PixelDCSFSMInterface::getPartitionState_Power(xoap::MessageReference soapRequest) throw (xoap::exception::Exception)
{
  std::cout << "<PixelDCSFSMInterface::getPartitionState_Power>:" << std::endl;

//--- decode from SOAP message name of partition the state of which is to be queried
//    and whether the Supervisor that initiated the query is of type PixelFEC, TrkFEC or FED
  xoap::SOAPEnvelope envelope = soapRequest->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();

//--- create SOAPName objects 
//    for keywords searched for in SOAP message
  xoap::SOAPName command = envelope.createName("fsmStateRequest");
  xoap::SOAPName nameAttribute = envelope.createName("name");
  xoap::SOAPName typeAttribute = envelope.createName("type");
  xoap::SOAPName instanceAttribute = envelope.createName("instance");

//--- find within body 
//    "fsmStateRequest" command
  std::vector< xoap::SOAPElement > bodyElements = body.getChildElements(command);          	  
  for ( std::vector< xoap::SOAPElement >::iterator bodyElement = bodyElements.begin();
	bodyElement != bodyElements.end(); ++bodyElement ) {

//--- unpack type of Supervisor
    const std::string& supervisorName = bodyElement->getAttributeValue(nameAttribute).data();
    std::string supervisorType = bodyElement->getAttributeValue(typeAttribute).data();
    unsigned int supervisorInstance = atoi(bodyElement->getAttributeValue(instanceAttribute).data());

//--- check for each partition if its state is relevant for Supervisor;
//    if so, query states of all power supply boards and determine summarized state
    for ( std::list<PixelDCSFSMPartition>::iterator fsmPartition = fsmPartitionList_.begin();
	  fsmPartition != fsmPartitionList_.end(); ++fsmPartition ) {

      const std::list<PixelDCSSOAPConnection>& soapConnections = fsmPartition->getSOAPConnections();
      for ( std::list<PixelDCSSOAPConnection>::const_iterator soapConnection = soapConnections.begin();
	    soapConnection != soapConnections.end(); ++soapConnection ) {
	const std::string& soapConnectionName = soapConnection->getName();
	const std::string& soapConnectionType = soapConnection->getType();
	unsigned int soapConnectionInstance = soapConnection->getInstance();

	if ( supervisorName == soapConnectionName &&
	     supervisorType == soapConnectionType &&
	     supervisorInstance == soapConnectionInstance ) {
	  
//--- query states of all A4602 power supply boards
	  std::list<const PixelDCSFSMNodeA4602*> fsmNodeListA4602 = fsmPartition->getNodeListA4602();
	  for ( std::list<const PixelDCSFSMNodeA4602*>::const_iterator fsmNodeA4602 = fsmNodeListA4602.begin();
		fsmNodeA4602 != fsmNodeListA4602.end(); ++fsmNodeA4602 ) {
	    const std::string& fsmNodeNameA4602 = (*fsmNodeA4602)->getName();
	    const std::string& fsmNodeDomainA4602 = (*fsmNodeA4602)->getDomain();

	    std::string pvssStateNameA4602 = smiCommander_->getStateOfFSM(fsmNodeNameA4602, fsmNodeDomainA4602, fsmNodeOwner);
	    std::cout << "fsmNodeNameA4602 = " << fsmNodeNameA4602 << std::endl;
	    std::cout << " fsmNodeDomainA4602 = " << fsmNodeDomainA4602 << std::endl;
	    std::cout << " pvssStateNameA4602 = " << pvssStateNameA4602 << std::endl;
	    fsmPartition->setNodeStateA4602(fsmNodeNameA4602, pvssStateNameA4602);
	  }

//--- query states of all A4603 power supply boards ()
	  std::list<const PixelDCSFSMNodeA4603*> fsmNodeListA4603 = fsmPartition->getNodeListA4603();
	  for ( std::list<const PixelDCSFSMNodeA4603*>::const_iterator fsmNodeA4603 = fsmNodeListA4603.begin();
		fsmNodeA4603 != fsmNodeListA4603.end(); ++fsmNodeA4603 ) {
	    const std::string& fsmNodeNameA4603 = (*fsmNodeA4603)->getName();
	    const std::string& fsmNodeDomainA4603 = (*fsmNodeA4603)->getDomain();
	    
	    std::string pvssStateNameA4603 = smiCommander_->getStateOfFSM(fsmNodeNameA4603, fsmNodeDomainA4603, fsmNodeOwner);
	    std::cout << "fsmNodeNameA4603 = " << fsmNodeNameA4603 << std::endl;
	    std::cout << " fsmNodeDomainA4603 = " << fsmNodeDomainA4603 << std::endl;
	    std::cout << " pvssStateNameA4603 = " << pvssStateNameA4603 << std::endl;
	    fsmPartition->setNodeStateA4603(fsmNodeNameA4603, pvssStateNameA4603);
	  }

//--- update summarized states of A4602 and A4603 power supply boards
//    associated to partition
	  determineSummarizedStateA4602(*fsmPartition);
	  determineSummarizedStateA4603(*fsmPartition);
	}
      }
    }
    
    return composeFSMStateNotification_synchronous(supervisorName, supervisorType, supervisorInstance);
  }

//--- program flow should never come here...
  return /*soapCommander_->*/MakeSOAPMessageReference("getPartitionState_PowerFailed");
}

xoap::MessageReference PixelDCSFSMInterface::composeFSMStateNotification_synchronous(const std::string& supervisorName, const std::string& supervisorType, 
									             unsigned int supervisorInstance)
{
//--- compose the SOAP message send to PixelFEC, TrkFEC or FED Supervisor
//    to inform it about the current state of the PVSS FSM 

  std::cout << "<composeFSMStateNotification_synchronous>:" << std::endl;
  std::cout << " supervisorName = " << supervisorName << std::endl;
  std::cout << " supervisorType = " << supervisorType << std::endl;

  xoap::MessageReference message = xoap::createMessage();
  xoap::SOAPEnvelope envelope = message->getSOAPPart().getEnvelope();
  xoap::SOAPName command = envelope.createName("fsmStateResponse", "xdaq", XDAQ_NS_URI);
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPBodyElement commandElement = body.addBodyElement(command);
 
//--- check for each partition if its state is relevant for Supervisor;
//    if so, add summarized state of partition to SOAP message send to Supervisor
  for ( std::list<PixelDCSFSMPartition>::iterator fsmPartition = fsmPartitionList_.begin();
	fsmPartition != fsmPartitionList_.end(); ++fsmPartition ) {

    const std::list<PixelDCSSOAPConnection>& soapConnections = fsmPartition->getSOAPConnections();
    for ( std::list<PixelDCSSOAPConnection>::const_iterator soapConnection = soapConnections.begin();
	  soapConnection != soapConnections.end(); ++soapConnection ) {
      const std::string& soapConnectionName = soapConnection->getName();
      const std::string& soapConnectionType = soapConnection->getType();
      unsigned int soapConnectionInstance = soapConnection->getInstance();
      
      if ( supervisorName == soapConnectionName &&
	   supervisorType == soapConnectionType &&
	   supervisorInstance == soapConnectionInstance ) {

	const std::string& currentPartitionStateA4603 = fsmPartition->getSummarizedStateA4603();
	const std::string& currentPartitionStateA4602 = fsmPartition->getSummarizedStateA4602();
	std::string currentSummarizedState = getSummarizedState(soapConnectionType, currentPartitionStateA4602, currentPartitionStateA4603);
	
	std::cout << " partition: " << fsmPartition->getName() << std::endl;
	std::cout << "  summarized state A4602 = " << fsmPartition->getSummarizedStateA4602() << std::endl;
	std::cout << "                   A4603 = " << fsmPartition->getSummarizedStateA4603() << std::endl;

	xoap::SOAPName state = envelope.createName("state");
	xoap::SOAPElement stateElement = commandElement.addChildElement(state);
	xoap::SOAPName partitionElement = envelope.createName("partition");
	stateElement.addAttribute(partitionElement, fsmPartition->getName());
	stateElement.addTextNode(currentSummarizedState);
      }
    }
  }

//   std::cout << " Notification : ------------------------------------ "<< std::endl;
//   message->writeTo(std::cout);
//   std::cout << std::endl;
//   std::cout << " ---------------------------------------------- "<< std::endl;

  return message;
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelDCSFSMInterface::updateSupervisors(string overrideA4602,string overrideA4603 ) {

  //override strings allows caller to completely override the 
  //real summarized state
  if (overrideA4602 != "") diagService_->reportError("Overriding actual DCS state for A4602s with state = "+overrideA4602,DIAGWARN);
  if (overrideA4603 != "") diagService_->reportError("Overriding actual DCS state for A4603s with state = "+overrideA4603,DIAGWARN);

  cout<<"Entered updateSupervisors!!"<<endl;
//usleep(250); //jmt -- 20 Feb 2012 -- this sleep seems to fix mysterious crashes (but why???)
//--- check for each partition if the summarized state 
//    of the A4603 and A4602 power supply boards has changed
//    and notify:
//     o FED in case summarized state of either A4603 or A4602 power supplies has changed
//     o PixelFEC in case -""-
//     o TrkFEC in case summarized state of A4602 power supplies has changed

  for ( std::list<PixelDCSFSMPartition>::iterator fsmPartition = fsmPartitionList_.begin();
	fsmPartition != fsmPartitionList_.end(); ++fsmPartition ) {

    const std::string& previousPartitionStateA4603 = fsmPartitionPreviousStateA4603_[fsmPartition->getName()];
    string currentPartitionStateA4603 = fsmPartition->getSummarizedStateA4603();
    const std::string& previousPartitionStateA4602 = fsmPartitionPreviousStateA4602_[fsmPartition->getName()];
    string currentPartitionStateA4602 = fsmPartition->getSummarizedStateA4602();


    //careful! we're not doing any sanity checking on these values!
    if (overrideA4602 != "") currentPartitionStateA4602 = overrideA4602;
    if (overrideA4603 != "") currentPartitionStateA4603 = overrideA4603;
    
    const std::list<PixelDCSSOAPConnection>& soapConnections = fsmPartition->getSOAPConnections();
    for ( std::list<PixelDCSSOAPConnection>::const_iterator soapConnection = soapConnections.begin();
	  soapConnection != soapConnections.end(); ++soapConnection ) {
      const std::string& soapConnectionName = soapConnection->getName();
      const std::string& soapConnectionType = soapConnection->getType();
      unsigned int soapConnectionInstance = soapConnection->getInstance();
      
//        std::cout << "soapConnectionName = " << soapConnectionName << std::endl;
//        std::cout << "soapConnectionType = " << soapConnectionType << std::endl;
//        std::cout << " soapConnectionInstance = " << soapConnectionInstance << std::endl;   

      if ( soapConnectionName != "" ) {
	std::string previousSummarizedState = getSummarizedState(soapConnectionType, previousPartitionStateA4602, previousPartitionStateA4603);
	std::string currentSummarizedState = getSummarizedState(soapConnectionType, currentPartitionStateA4602, currentPartitionStateA4603);
//  	std::cout << "previousSummarizedState = " << previousSummarizedState << std::endl;
//  	std::cout << "currentSummarizedState = " << currentSummarizedState << std::endl;

	if ( previousSummarizedState != currentSummarizedState ) {
	  xoap::MessageReference soapRequest = composeFSMStateNotification_asynchronous(*fsmPartition, currentSummarizedState);
	  std::cout << "fsmPartition::getName = " << fsmPartition->getName() << std::endl;
    	  std::cout << "sending FSM State Notification," 
		    << " summarized State = " << currentSummarizedState << std::endl;
	  std::cout << " to: " << soapConnectionName << " ; soapConnectionInstance = " << soapConnectionInstance<< std::endl;
	  
//           std::cout << " Request : ------------------------------------ "<< std::endl;
//           soapRequest->writeTo(std::cout);
//           std::cout << std::endl;
//           std::cout << " ---------------------------------------------- "<< std::endl;
	  
	  xdaq::ApplicationDescriptor* xdaqDescriptor = 0;
	  try {
	    xdaqDescriptor =  getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor(soapConnectionName, soapConnectionInstance);
	  }
	  catch (xdaq::exception::Exception & e) {
	    xdaqDescriptor = 0;
	  }
	  if (xdaqDescriptor!=0) {
cout<<"[updateSupervisors] about to send SOAP to supervisor"<<endl; //jmt debug
	    std::string soapResponse = Send(xdaqDescriptor, soapRequest);
	    std::cout << "soapResponse = " << soapResponse << std::endl;
	    if ( soapResponse != "fsmStateNotificationDone" ) {
	    XCEPT_RAISE(xoap::exception::Exception, "Failed to send FSM State notification");
	    }
	  }
	  else {
	    cout<<"[PixelDCSFSMInterface::updateSupervisors()] WARNING: SOAPConnection does not exist! Skipping notification! "<<soapConnectionName<<" "<<soapConnectionInstance<<endl;
	  }
	}
      } else {
	XCEPT_RAISE(xoap::exception::Exception, "Undefined Connection Name");
      }
    }
    
    fsmPartitionPreviousStateA4603_[fsmPartition->getName()] = currentPartitionStateA4603;
    fsmPartitionPreviousStateA4602_[fsmPartition->getName()] = currentPartitionStateA4602;
  }

}

xoap::MessageReference PixelDCSFSMInterface::updatePartitionState_Power(xoap::MessageReference soapMessage) throw (xoap::exception::Exception)
{
//--- handle state changes of CAEN A4602 and A4603 power supplies;
//    SOAP message sent from PSX server (PVSS)


//too much verbosity
  std::cout << "<PixelDCSFSMInterface::updatePartitionState_Power>:" << std::endl; //jmt debug

  //  std::cout << " Notification : ------------------------------------ "<< std::endl;
//    soapMessage->writeTo(std::cout);
//    std::cout << std::endl;
//    std::cout << " ---------------------------------------------- "<< std::endl;

//--- unpack states of A4603 and A4602 power supply boards 
//    from SOAP message received from PSX server
  decodePartitionState_Power(soapMessage, "notify");

cout<<"<PixelDCSFSMInterface::updatePartitionState_Power>: about to update Supervisors"<<endl; //jmt debug
  updateSupervisors();

  //  std::cout << "finished processing <PixelDCSFSMInterface::updatePartitionState_Power>." << std::endl;

//--- acknowledge receipt of FSM state update notification;
//    send SOAP response to PSX server
  return smiCommander_->MakeSOAPMessageReference_fsmNotifyResponse();
}

void PixelDCSFSMInterface::decodePartitionState_Power(xoap::MessageReference soapMessage, const char* tagName) throw (xoap::exception::Exception)
{
//--- handle state changes of CAEN A4602 and A4603 power supplies;
//    SOAP message sent from PSX server (PVSS)
//
//    (tagName = "getStateResponse" for synchronous communication with PSX server or
//               "notify"           for asynchronous communication)

  PixelTimer debugTimer;
  debugTimer.setName("PixelDCSFSMInterface::decodePartitionState_Power");
  debugTimer.printTime(tagName);

//--- begin unpacking SOAP message
//    received from PSX server
  xoap::SOAPEnvelope envelope = soapMessage->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();

//--- create SOAPName objects 
//    for keywords searched for in SOAP message
  xoap::SOAPName command = envelope.createName(tagName);
  xoap::SOAPName objectAttribute = envelope.createName("object");

//--- find within body 
//    "getStateResponse" or "notify" command
  std::vector< xoap::SOAPElement > bodyElements = body.getChildElements(command);          	  
  for ( std::vector< xoap::SOAPElement >::iterator bodyElement = bodyElements.begin();
	bodyElement != bodyElements.end(); ++bodyElement ) {

//--- unpack FSM node name and state,
//    together with transaction identifier and context information
    std::string fsmNodeName = bodyElement->getAttributeValue(objectAttribute).data();
    std::string fsmNodeState = bodyElement->getValue().data();

//--- check which partitions depend 
//    on the state of the given FSM node 
    for ( std::list<PixelDCSFSMPartition>::iterator fsmPartition = fsmPartitionList_.begin();
	  fsmPartition != fsmPartitionList_.end(); ++fsmPartition ) {

//--- check states of A4602 type power supply boards
      std::list<const PixelDCSFSMNodeA4602*> fsmNodeListA4602 = fsmPartition->getNodeListA4602();
      for ( std::list<const PixelDCSFSMNodeA4602*>::const_iterator fsmNode = fsmNodeListA4602.begin();
	    fsmNode != fsmNodeListA4602.end(); ++fsmNode ) {
	if ( (*fsmNode)->getName() == fsmNodeName ) { // partition does depend on state of FSM node
	  fsmPartition->setNodeStateA4602(fsmNodeName, fsmNodeState);
	    
	  std::cout << "Number of FSM nodes of type A4602 ("<<fsmPartition->getName()<<"):" << std::endl; 
	  std::cout << " in \"LV_ON\" = " << fsmPartition->getNumNodesA4602(xdaqState_LV_ON) << std::endl;
	  std::cout << " in \"LV_OFF\" = " << fsmPartition->getNumNodesA4602(xdaqState_LV_OFF) << std::endl;
	  std::cout << " in \"UNDEFINED\" = " << fsmPartition->getNumNodesA4602(xdaqState_UNDEFINED) << std::endl;
	  std::cout << " being ignored due to detconfig = "<<fsmPartition->getNumNodesIgnoredA4602()<<std::endl;
	}
      }

//--- check states of A4603 type power supply boards ()
      std::list<const PixelDCSFSMNodeA4603*> fsmNodeListA4603 = fsmPartition->getNodeListA4603();
      for ( std::list<const PixelDCSFSMNodeA4603*>::const_iterator fsmNode = fsmNodeListA4603.begin();
	    fsmNode != fsmNodeListA4603.end(); ++fsmNode ) {
	if ( (*fsmNode)->getName() == fsmNodeName ) { // partition does depend on state of FSM node
	  fsmPartition->setNodeStateA4603(fsmNodeName, fsmNodeState);
	    
	  std::cout << "Number of FSM nodes of type A4603 ("<<fsmPartition->getName()<<"):" << std::endl; 
	  std::cout << " in \"HV_ON\" = " << fsmPartition->getNumNodesA4603(xdaqState_HV_ON) << std::endl;
	  std::cout << " in \"LV_ON\" = " << fsmPartition->getNumNodesA4603(xdaqState_LV_ON) << std::endl;
	  std::cout << " in \"LV_ON_REDUCED\" = " << fsmPartition->getNumNodesA4603(xdaqState_LV_ON_REDUCED) << std::endl;
	  std::cout << " in \"LV_OFF\" = " << fsmPartition->getNumNodesA4603(xdaqState_LV_OFF) << std::endl;
	  std::cout << " in \"UNDEFINED\" = " << fsmPartition->getNumNodesA4603(xdaqState_UNDEFINED) << std::endl;
	  std::cout << " being ignored due to detconfig = "<<fsmPartition->getNumNodesIgnoredA4603()<<std::endl;
	}
      }


//--- update summarized states of A4602 and A4603 power supply boards
//    associated to partition
      determineSummarizedStateA4602(*fsmPartition);
      determineSummarizedStateA4603(*fsmPartition);
    }
  }
}

void PixelDCSFSMInterface::determineSummarizedStateA4602(PixelDCSFSMPartition& fsmPartition)
{
//--- set current state of all A4602 power supply boards in partition;
//    make backup copy of previous state
  fsmPartitionPreviousStateA4602_[fsmPartition.getName()] = fsmPartition.getSummarizedStateA4602();
  //note -- the above line used to cite A4603. I think that was a bug
  if ( (fsmPartition.getNumNodesA4602(xdaqState_LV_OFF) 
	+ fsmPartition.getNumNodesA4602(xdaqState_UNDEFINED)) > 0 ) {
    fsmPartition.setSummarizedStateA4602(xdaqState_LV_OFF);
  } else {
    fsmPartition.setSummarizedStateA4602(xdaqState_LV_ON);
  } 
}

void PixelDCSFSMInterface::determineSummarizedStateA4603(PixelDCSFSMPartition& fsmPartition)
{
//--- set current state of all A4603 power supply boards in partition;
//    make backup copy of previous state,
//    in order to allow for PixelFEC, TrkFEC and FED Supervisors to be notified only 
//    in case summarized state does actually change
//
//    NOTE: the summarized state of partition is set to "LV_OFF"
//          if at least one low voltage channel is in state "OFF", "RAMPING", "TRIPPED" 
//          or in an "undefined" state (PVSS state not known to XDAQ)
//      
  fsmPartitionPreviousStateA4603_[fsmPartition.getName()] = fsmPartition.getSummarizedStateA4603();
  if ( (fsmPartition.getNumNodesA4603(xdaqState_LV_OFF) 
	+ fsmPartition.getNumNodesA4603(xdaqState_UNDEFINED)) > 0 ) {
    fsmPartition.setSummarizedStateA4603(xdaqState_LV_OFF);
  } else if ( fsmPartition.getNumNodesA4603(xdaqState_LV_ON_REDUCED)  > 0 ) {
    fsmPartition.setSummarizedStateA4603(xdaqState_LV_ON_REDUCED);
  } else if ( fsmPartition.getNumNodesA4603(xdaqState_LV_ON)  > 0 )
    {
      fsmPartition.setSummarizedStateA4603(xdaqState_LV_ON);
    }  else 
    {
      fsmPartition.setSummarizedStateA4603(xdaqState_HV_ON);
    } 
}

std::string PixelDCSFSMInterface::getSummarizedState(const std::string& soapConnectionType, 
						     const std::string& stateA4602, const std::string& stateA4603) throw (xoap::exception::Exception)
{

  if ( soapConnectionType == "PxlFEC" || soapConnectionType == "FED" ) {
//--- PixelFEC or FED;
//    status of A4602 and A4603 type power supply boards is important
    if ( stateA4602 == xdaqState_UNDEFINED || stateA4603 == xdaqState_UNDEFINED ) {
      return xdaqState_UNDEFINED + ";" + xdaqState_UNDEFINED; 
    } else if ( stateA4602 == xdaqState_LV_OFF || stateA4603 == xdaqState_LV_OFF ) {
      return xdaqState_LV_OFF + ";" + xdaqState_HV_OFF; 
    } else if ( stateA4603 == xdaqState_LV_ON_REDUCED ) {
      return xdaqState_LV_ON_REDUCED + ";" + xdaqState_HV_OFF;
    } else if ( stateA4602 == xdaqState_LV_ON && stateA4603 == xdaqState_LV_ON ) {
      return xdaqState_LV_ON + ";" + xdaqState_HV_OFF; 
    } else if ( stateA4602 == xdaqState_LV_ON && stateA4603 == xdaqState_HV_ON ) {
      return xdaqState_LV_ON + ";" + xdaqState_HV_ON; 
    } else {
      std::cout << "stateA4602 = " << stateA4602 << std::endl;
      std::cout << "stateA4603 = " << stateA4603 << std::endl;
      XCEPT_RAISE (xoap::exception::Exception, "Undefined  FSM State: A4602 = " + stateA4602 + ", A4603 = " + stateA4603);
    }
  } else if ( soapConnectionType == "TrkFEC" ) {
//--- TrackerFEC;
//    only status of A4602 type boards is important
    return stateA4602;
  } else {
    std::cout << "soapConnectionType = " << soapConnectionType << std::endl;
    XCEPT_RAISE(xoap::exception::Exception, "Undefined Connection Type");
  }
}

xoap::MessageReference PixelDCSFSMInterface::composeFSMStateNotification_asynchronous(const PixelDCSFSMPartition& fsmPartition, const std::string& currentSummarizedState)
{
//--- compose the SOAP message send to PixelFEC, TrkFEC or FED Supervisor
//    notifying it that the PVSS FSM state has changed

  std::cout << "<composeFSMStateNotification>:" << std::endl;
  std::cout << " partition: " << fsmPartition.getName() << std::endl;
  std::cout << "  summarized state A4602 = " << fsmPartition.getSummarizedStateA4602() << std::endl;
  std::cout << "                   A4603 = " << fsmPartition.getSummarizedStateA4603() << std::endl;
  
  xoap::MessageReference message = xoap::createMessage();
  xoap::SOAPEnvelope envelope = message->getSOAPPart().getEnvelope();
  xoap::SOAPName command = envelope.createName("fsmStateNotification", "xdaq", XDAQ_NS_URI);
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPBodyElement commandElement = body.addBodyElement(command);
 
  xoap::SOAPName state = envelope.createName("state");
  xoap::SOAPElement stateElement = commandElement.addChildElement(state);
  xoap::SOAPName partitionElement = envelope.createName("partition");
  stateElement.addAttribute(partitionElement, fsmPartition.getName());
  stateElement.addTextNode(currentSummarizedState);

//   std::cout << " Notification : ------------------------------------ "<< std::endl;
//   message->writeTo(std::cout);
//   std::cout << std::endl;
//   std::cout << " ---------------------------------------------- "<< std::endl;

  return message;
}

//
//---------------------------------------------------------------------------------------------------
//

xoap::MessageReference PixelDCSFSMInterface::updatePartitionState_ReadoutChips(xoap::MessageReference soapMessage) throw (xoap::exception::Exception)
{
//--- handle state changes of Read-out Chip initialization status;
//    SOAP message sent from PixelFECSupervisor

  std::cout << "<PixelDCSFSMInterface::updatePartitionState_ReadoutChips>:" << std::endl;

  std::cout << " Notification : ------------------------------------ "<< std::endl;
  soapMessage->writeTo(std::cout);
  std::cout << std::endl;
  std::cout << " ---------------------------------------------- "<< std::endl;

//--- begin unpacking SOAP message
//    received from PixelFECSupervisor
  xoap::SOAPEnvelope envelope = soapMessage->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();

//--- create SOAPName objects 
//    for keywords searched for in SOAP message
  xoap::SOAPName command = envelope.createName("fsmStateNotification");
  xoap::SOAPName objectName = envelope.createName("state");
  xoap::SOAPName objectAttribute = envelope.createName("partition");

  std::list<PixelDCSPVSSDpe> dpes;

//--- find within body 
//    "state" command
  std::vector< xoap::SOAPElement > bodyElements = body.getChildElements(command);  
  for ( std::vector< xoap::SOAPElement >::iterator bodyElement = bodyElements.begin();
	bodyElement != bodyElements.end(); ++bodyElement ) {
    std::vector< xoap::SOAPElement > commandElements = bodyElement->getChildElements(objectName);
    for ( std::vector< xoap::SOAPElement >::iterator commandElement = commandElements.begin();
	  commandElement != commandElements.end(); ++commandElement ) {

//--- unpack FSM partition name and state
      std::string fsmPartitionName = commandElement->getAttributeValue(objectAttribute).data();
      std::string fsmPartitionState = commandElement->getValue().data();

      std::cout << "fsmPartitionName = " << fsmPartitionName << std::endl;
      std::cout << "fsmPartitionState = " << fsmPartitionState << std::endl;

      for ( std::list<PixelDCSFSMPartition>::iterator fsmPartition = fsmPartitionList_.begin();
	    fsmPartition != fsmPartitionList_.end(); ++fsmPartition ) {
	
	std::cout << "fsmPartition::getName = " << fsmPartition->getName() << std::endl;

	if ( fsmPartition->getName() == fsmPartitionName ) {

//--- update PVSS data-points 
//    representing Read-out Chip initialization status
	  std::list<const PixelDCSFSMNodeA4603*> nodesA4603 = fsmPartition->getNodeListA4603();
	  for ( std::list<const PixelDCSFSMNodeA4603*>::iterator nodeA4603 = nodesA4603.begin();
		nodeA4603 != nodesA4603.end(); ++nodeA4603 ) {
            if ( (*nodeA4603)->getDeviceDefinition_ReadoutChipInitializationStatus() != NULL ) {
  	      PixelDCSFSMNodeA4603* nodeA4603_nonConst = const_cast<PixelDCSFSMNodeA4603*>(*nodeA4603);
	      nodeA4603_nonConst->setState_ReadoutChipInitializationStatus(fsmPartitionState);
	    
	      const PixelDCSFSMDeviceDefinition* deviceDefinition_ReadoutChipInitializationStatus =
	        (*nodeA4603)->getDeviceDefinition_ReadoutChipInitializationStatus();
	    
	      const std::string& dpeName = (*nodeA4603)->getDpName_ReadoutChipInitializationStatus();
	      const std::string& dpeValue = deviceDefinition_ReadoutChipInitializationStatus->getDpValue(fsmPartitionState);

	      std::cout << "A4603 Node = " << (*nodeA4603)->getName() << std::endl;
	      std::cout << " dpeName = " << dpeName << std::endl;
	      std::cout << " dpeValue = " << dpeValue << std::endl;
	    
	      if ( dpeName == "" || dpeValue == "" ) {
	        XCEPT_RAISE(xoap::exception::Exception, "Undefined FSM state");
	      }
	    
	      PixelDCSPVSSDpe dpe(dpeName, dpeValue);
	      dpes.push_back(dpe);
	    }
	  }
	}
      }
    }
  }

  try {
    pvssCommander_->setDpeValues(dpes);
  }
  catch (xdaq::exception::Exception & e) {
    diagService_->reportError("Probable PSX communication problem! Try configuration again if the FPix LV remains at REDUCED. Exception: "+string(e.what()),DIAGERROR);
  }

//--- acknowledge receipt of FSM state update notification;
//    send response to PixelFECSupervisor
  return MakeSOAPMessageReference("fsmStateNotificationResponse");
}

//
//---------------------------------------------------------------------------------------------------
//

std::list<const PixelDCSFSMNode*> getNodeList(const PixelDCSFSMPartition& fsmPartition)
{
  std::list<const PixelDCSFSMNode*> nodeList;

  std::list<const PixelDCSFSMNodeA4602*> nodeListA4602 = fsmPartition.getNodeListA4602();
  for ( std::list<const PixelDCSFSMNodeA4602*>::const_iterator node = nodeListA4602.begin();
	node != nodeListA4602.end(); ++node ) {
    nodeList.push_back(*node);
  }

  std::list<const PixelDCSFSMNodeA4603*> nodeListA4603 = fsmPartition.getNodeListA4603();
  for ( std::list<const PixelDCSFSMNodeA4603*>::const_iterator node = nodeListA4603.begin();
	node != nodeListA4603.end(); ++node ) {
    nodeList.push_back(*node);
  }

  return nodeList;
}

//---------------------------------------------------------------------------------------------------
//                AUXILIARY FUNCTIONS DEFINED OUTSIDE OF CLASS PIXELDCSFSMINTERFACE
//---------------------------------------------------------------------------------------------------

std::string getName_noColon(const std::string& name)
{
  std::string name_noColon = name;
  for ( unsigned int i = 0; i < name_noColon.size(); ++i ) {
    char c = name_noColon[i];
    if ( c == ':' ) name_noColon[i] = '_';
  }
  return name_noColon;
}

//
//---------------------------------------------------------------------------------------------------
//

std::string getTagNameA4602(const std::string& fsmPartitionName, const std::string& fsmNodeName)
{
  std::string tagName = fsmPartitionName + "_A4602_" + getName_noColon(fsmNodeName);
  return tagName;
} 

std::string getTagNameA4602_state(const std::string& fsmPartitionName, const std::string& fsmNodeName)
{
  std::string tagName = getTagNameA4602(fsmPartitionName, fsmNodeName) + "_state";
  return tagName;
} 

std::string getTagNameA4602_color(const std::string& fsmPartitionName, const std::string& fsmNodeName)
{
  std::string tagName = getTagNameA4602(fsmPartitionName, fsmNodeName) + "_color";
  return tagName;
}

std::string getTagNameA4602_state_used(const std::string& fsmPartitionName, const std::string& fsmNodeName)
{
  std::string tagName = getTagNameA4602(fsmPartitionName, fsmNodeName) + "_state_used";
  return tagName;
} 

std::string getTagNameA4602_color_used(const std::string& fsmPartitionName, const std::string& fsmNodeName)
{
  std::string tagName = getTagNameA4602(fsmPartitionName, fsmNodeName) + "_color_used";
  return tagName;
}
//
//---------------------------------------------------------------------------------------------------
//

std::string getTagNameA4603(const std::string& fsmPartitionName, const std::string& fsmNodeName)
{
  std::string tagName = fsmPartitionName + "_A4603_" + getName_noColon(fsmNodeName);
  return tagName;
} 

std::string getTagNameA4603_state_power(const std::string& fsmPartitionName, const std::string& fsmNodeName)
{
  std::string tagName = getTagNameA4603(fsmPartitionName, fsmNodeName) + "_state_power";
  return tagName;
} 

std::string getTagNameA4603_color_power(const std::string& fsmPartitionName, const std::string& fsmNodeName)
{
  std::string tagName = getTagNameA4603(fsmPartitionName, fsmNodeName) + "_color_power";
  return tagName;
}

std::string getTagNameA4603_state_readoutChipInitialization(const std::string& fsmPartitionName, const std::string& fsmNodeName)
{
  std::string tagName = getTagNameA4603(fsmPartitionName, fsmNodeName) + "_state_readoutChipInitialization";
  return tagName;
} 

std::string getTagNameA4603_color_readoutChipInitialization(const std::string& fsmPartitionName, const std::string& fsmNodeName)
{
  std::string tagName = getTagNameA4603(fsmPartitionName, fsmNodeName) + "_color_readoutChipInitialization";
  return tagName;
}

std::string getTagNameA4603_state_used(const std::string& fsmPartitionName, const std::string& fsmNodeName)
{
  std::string tagName = getTagNameA4603(fsmPartitionName, fsmNodeName) + "_state_used";
  return tagName;
} 

std::string getTagNameA4603_color_used(const std::string& fsmPartitionName, const std::string& fsmNodeName)
{
  std::string tagName = getTagNameA4603(fsmPartitionName, fsmNodeName) + "_color_used";
  return tagName;
}

//
//---------------------------------------------------------------------------------------------------
//

std::string getTagNamePartitionSummary(const std::string& fsmPartitionName, const std::string& soapConnectionName)
{
  std::string tagName = fsmPartitionName + "_" + soapConnectionName;
  return tagName;
} 

std::string getTagNamePartitionSummary_state(const std::string& fsmPartitionName, const std::string& soapConnectionName)
{
  std::string tagName = getTagNamePartitionSummary(fsmPartitionName, soapConnectionName) + "_state";
  return tagName;
} 

std::string getTagNamePartitionSummary_color(const std::string& fsmPartitionName, const std::string& soapConnectionName)
{
  std::string tagName = getTagNamePartitionSummary(fsmPartitionName, soapConnectionName) + "_color";
  return tagName;
}
