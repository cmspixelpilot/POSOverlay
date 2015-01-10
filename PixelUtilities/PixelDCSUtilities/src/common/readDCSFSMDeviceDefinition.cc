#include "PixelUtilities/PixelDCSUtilities/include/readDCSFSMDeviceDefinition.h"

/*************************************************************************
 * Auxiliary function to parse information neccessary                    *
 * for translation between XDAQ FSM states and commands on the one hand  *
 * and PVSS FSM states and commands on the other hand                    *
 * for a single FSM node (device or logical/control unit)                *
 * from an xml file and initialize PixelDCSFSMDeviceDefinitionClass      *
 *                                                                       *
 * Author: Christian Veelken, UC Davis			 	         *
 *                                                                       *
 * Last update: $Date: 2007/08/22 00:07:46 $ (UTC)                       *
 *          by: $Author: aryd $                                       *
 *************************************************************************/

#include "xercesc/dom/DOMNodeList.hpp"
#include "xercesc/dom/DOMNode.hpp"
#include "xoap/domutils.h"

PixelDCSFSMNodeDefinition readFSMNodeDefinition(xercesc::DOMNode* configNode) throw (xdaq::exception::Exception)
{
//--- read mapping between PVSS and XDAQ states and commands
//    for given FSM node type (logical or control unit)

  std::string type = xoap::getNodeAttribute(configNode, "type");
  
  std::cout << "reading definition of FSM node, type = " << type << std::endl;

//--- read mapping between PVSS and XDAQ states
  std::list<std::pair<PixelDCSFSMStateTranslation, std::string> > stateList;
  std::list<PixelDCSFSMCommandTranslation> commandList;

  xercesc::DOMNodeList* definitions = configNode->getChildNodes();
  unsigned int numDefinitions = definitions->getLength();
  for ( unsigned int iDefinition = 0; iDefinition < numDefinitions; ++iDefinition ) {
    xercesc::DOMNode* definition = definitions->item(iDefinition);

//--- skip empty lines
//    (not removed by DOM parser)
    if ( xoap::XMLCh2String(definition->getLocalName()) == "" ) continue;

    if ( xoap::XMLCh2String(definition->getLocalName()) == "state" ) {
      std::string pvssName = xoap::getNodeAttribute(definition, "pvssName");
      std::string xdaqName = xoap::getNodeAttribute(definition, "xdaqName");
      std::string color = xoap::getNodeAttribute(definition, "color");
      if ( color == "" ) color = "#C0C0C0";
      
      std::cout << " PVSS State = " << pvssName << " maps to XDAQ State = " << xdaqName << " (color = " << color << ")" << std::endl;

      stateList.push_back(std::pair<PixelDCSFSMStateTranslation, std::string>(PixelDCSFSMStateTranslation(pvssName, xdaqName), color)); 
    } else if ( xoap::XMLCh2String(definition->getLocalName()) == "command" ) {
      std::string xdaqName = xoap::getNodeAttribute(definition, "xdaqName");
      std::string pvssName = xoap::getNodeAttribute(definition, "pvssName");
      std::string xdaqState = xoap::getNodeAttribute(definition, "xdaqState");

      std::cout << "PVSS Command = \"" << pvssName << "\" maps to XDAQ Command = \"" << xdaqName << "\" for XDAQ State = \"" << xdaqState << "\"" << std::endl;

      commandList.push_back(PixelDCSFSMCommandTranslation(xdaqName, xdaqState, pvssName));
    } else {
      XCEPT_RAISE (xcept::Exception, "Error parsing config File");
    }
  }

//--- add translation for "UNDEFINED" state,
//    in which all PixelDCSFSMNode objects are initialized 
//    (PixelDCSFSMNode are in "UNDEFINED" state until
//     the connection to the PSX server is established and actual FSM states are received from PVSS)
  stateList.push_back(std::pair<PixelDCSFSMStateTranslation, std::string>(PixelDCSFSMStateTranslation("UNDEFINED", "UNDEFINED"), "#999999")); 

  return PixelDCSFSMNodeDefinition(type, stateList, commandList);
}

PixelDCSFSMDeviceDefinition readFSMDeviceDefinition(xercesc::DOMNode* configNode) throw (xdaq::exception::Exception)
{
//--- read mapping between PVSS and XDAQ states and commands
//    for given FSM device type (device unit)

  std::string type = xoap::getNodeAttribute(configNode, "type");
  
  std::cout << "reading definition of device, type = " << type << std::endl;

//--- read mapping between XDAQ states and corresponding PVSS data-point values
  std::list<std::pair<PixelDCSFSMStateToDpValueTranslation, std::string> > stateList;

  xercesc::DOMNodeList* definitions = configNode->getChildNodes();
  unsigned int numDefinitions = definitions->getLength();
  for ( unsigned int iDefinition = 0; iDefinition < numDefinitions; ++iDefinition ) {
    xercesc::DOMNode* definition = definitions->item(iDefinition);

//--- skip empty lines
//    (not removed by DOM parser)
    if ( xoap::XMLCh2String(definition->getLocalName()) == "" ) continue;

    if ( xoap::XMLCh2String(definition->getLocalName()) == "state" ) {
      std::string xdaqState = xoap::getNodeAttribute(definition, "xdaqName");
      std::string pvssDpValue = xoap::getNodeAttribute(definition, "pvssDpValue");
      std::string color = xoap::getNodeAttribute(definition, "color");
      if ( color == "" ) color = "#C0C0C0";
      
      std::cout << " XDAQ State = " << xdaqState << " maps to PVSS data-point Value = " << pvssDpValue << " (color = " << color << ")" << std::endl;

      stateList.push_back(std::pair<PixelDCSFSMStateToDpValueTranslation, std::string>(PixelDCSFSMStateToDpValueTranslation(xdaqState, pvssDpValue), color));
    } else {
      XCEPT_RAISE (xcept::Exception, "Error parsing config File");
    }
  }

  return PixelDCSFSMDeviceDefinition(type, stateList);
}
