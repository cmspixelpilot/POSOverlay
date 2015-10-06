
#include "PixelDCSSMICommander.h"

/*************************************************************************
 * Auxiliary class to query states of PVSS FSM nodes                     *
 * and send commands to PVSS FSM                                         *
 *                                                                       *
 * Author: Christian Veelken, UC Davis				         *
 *                                                                       *
 * Last update: $Date: 2009/09/14 11:43:39 $ (UTC)                       *
 *          by: $Author: joshmt $                                       *
 *************************************************************************/

#include <iomanip>
#include <vector>
#include <string>
using namespace std;

#define PSX_SMI_NS_URI "http://xdaq.web.cern.ch/xdaq/xsd/2006/psx-smi-10.xsd"

PixelDCSSMICommander::PixelDCSSMICommander(xdaq::Application* xdaqApplication, xdaq::ApplicationDescriptor* psxDescriptor)
  : PixelDCSPSXCommander(xdaqApplication, psxDescriptor)
{
//--- nothing to be done yet...
}

PixelDCSSMICommander::~PixelDCSSMICommander()
{
//--- nothing to be done yet...
}

//
//---------------------------------------------------------------------------------------------------
//

std::string PixelDCSSMICommander::getStateOfFSM(const std::string& fsmNodeName, 
						const std::string& fsmNodeDomain, 
						const std::string& fsmNodeOwner) //throw (xdaq::exception::Exception)
{
//--- request PSX server to send state of PVSS FSM node 
//    given as function argument

  xoap::MessageReference psxRequest = MakeSOAPMessageReference_fsmGetState(fsmNodeName, fsmNodeDomain, fsmNodeOwner);

  std::cout << "<PixelDCSSMICommander::getStateOfFSM>:" << std::endl;
  std::cout << "sending SOAP message to " << psxDescriptor_->getContextDescriptor()->getURL() << std::endl;
  std::cout <<" Request : ------------------------------------ "<< std::endl;
  psxRequest->writeTo(std::cout);
  std::cout << std::endl;
  std::cout <<" ---------------------------------------------- "<< std::endl;
  xoap::MessageReference psxResponse;
  
  bool posted=false;
  while (!posted) {
    posted=true;
    try {
      psxResponse = app_->getApplicationContext()->postSOAP(psxRequest, *app_->getApplicationDescriptor(), *psxDescriptor_);
    } catch ( xdaq::exception::Exception& e ) {
      cout<<"Failed to post SOAP to the PSX server! I will keep trying!"<<endl;
      posted=false;
      ::sleep(3);
    }
  }
  std::cout <<" Response : ----------------------------------- "<< std::endl;
  psxResponse->writeTo(std::cout);
  std::cout << std::endl;
  std::cout <<" ---------------------------------------------- "<< std::endl;

  xoap::SOAPEnvelope responseEnvelope = psxResponse->getEnvelope();
  xoap::SOAPBody responseBody = responseEnvelope.getBody();
  
  if ( !responseBody.hasFault() ) {
    xoap::SOAPName commandElement = responseEnvelope.createName("getStateResponse");
    std::vector<xoap::SOAPElement> bodyElements = responseBody.getChildElements(commandElement);
    for ( vector< xoap::SOAPElement >::iterator bodyElement = bodyElements.begin();
	  bodyElement != bodyElements.end(); ++bodyElement ) {
      std::string fsmState = bodyElement->getValue().data();
      return fsmState;
    }
  } else {
    std::string fail = "Failed to query State of FSM: " + responseBody.getFault().getFaultString();
    XCEPT_RAISE (xdaq::exception::Exception, fail);
  }

//--- program flow should never come here...
  return std::string("Undefined");
}

std::pair<std::string, std::string> PixelDCSSMICommander::connectToFSM(const std::string& fsmNodeName,
								       const std::string& fsmNodeDomain, 
								       const std::string& fsmNodeOwner) //throw (xdaq::exception::Exception)
{
//--- request PSX server to connect to PVSS FSM hierarchy; 
//    return value of function is a std::pair<std::string, std::string>(connectionStatus, connectionId)
//    (if connection has been established successfully)

  std::cout << "<PixelDCSSMICommander::connectToFSM>:" << std::endl;
  std::cout << "sending SOAP message to " << psxDescriptor_->getContextDescriptor()->getURL() << std::endl;

  xoap::MessageReference psxRequest = MakeSOAPMessageReference_fsmConnect(fsmNodeName, fsmNodeDomain, fsmNodeOwner);
  
  xoap::MessageReference psxResponse;
  std::cout <<" Request : ------------------------------------ "<< std::endl;
  psxRequest->writeTo(std::cout);
  std::cout << std::endl;
  std::cout <<" ---------------------------------------------- "<< std::endl;

  try {
    psxResponse = app_->getApplicationContext()->postSOAP(psxRequest, *app_->getApplicationDescriptor(), *psxDescriptor_);
  }
  catch ( xdaq::exception::Exception& e ) {
    std::cout <<"[PixelDCSSMICommander::connectToFSM] Failed SOAP request : ------------------------------------ "<< std::endl;
    psxRequest->writeTo(std::cout);
    std::cout << std::endl;
    std::cout <<" ---------------------------------------------- "<< std::endl;
    XCEPT_RETHROW(xdaq::exception::Exception, "Failed to post SOAP to PSX server!",e);
  }
  
  std::cout <<" Response : ----------------------------------- "<< std::endl;
  psxResponse->writeTo(std::cout);
  std::cout << std::endl;
  std::cout <<" ---------------------------------------------- "<< std::endl;

  xoap::SOAPEnvelope responseEnvelope = psxResponse->getEnvelope();
  xoap::SOAPBody responseBody = responseEnvelope.getBody();
  if ( !responseBody.hasFault() ) {
    xoap::SOAPName commandElement = responseEnvelope.createName("connectResponse");
    std::vector<xoap::SOAPElement> bodyElements = responseBody.getChildElements(commandElement);
    for ( vector< xoap::SOAPElement >::iterator bodyElement = bodyElements.begin();
	  bodyElement != bodyElements.end(); ++bodyElement ) {
      xoap::SOAPName connectionIdAttribute("id", "", "");

      std::string psxConnectionStatus = "connected";
      std::string psxConnectionId = bodyElement->getAttributeValue(connectionIdAttribute);

      //      std::cout << "psxConnectionId set to " << psxConnectionId << "." << std::endl;

      return std::pair<std::string, std::string>(psxConnectionStatus, psxConnectionId);
    }
  } else {
    XCEPT_RAISE (xdaq::exception::Exception, "Could not connect to PSX server");
  }

//--- program flow should never come here...
  return std::pair<std::string, std::string>("Undefined", "Undefined");
}

void PixelDCSSMICommander::takeOwnershipOfFSM(const std::string& fsmNodeName,
					      const std::string& fsmNodeOwner,
					      bool fsmNodeExclusiveFlag) //throw (xdaq::exception::Exception)
{
//--- request PSX server to take ownership of the PVSS FSM node 
//    given as function argument
  std::cout << "<PixelDCSSMICommander::takeOwnershipOfFSM>:" << std::endl;

  xoap::MessageReference psxRequest = MakeSOAPMessageReference_fsmTakeOwnership(fsmNodeName, fsmNodeOwner, fsmNodeExclusiveFlag);
  
  try {
    postSOAP(psxDescriptor_, psxRequest, "takeResponse");
  } catch ( xdaq::exception::Exception& e ) {
    XCEPT_RETHROW(xcept::Exception, "Failed to take Ownership of FSM", e);
  }
}

void PixelDCSSMICommander::sendCommandToFSM(const std::string& fsmNodeName,
					    const std::string& fsmNodeOwner,
					    const std::string& fsmCommand) //throw (xdaq::exception::Exception)
{
//--- send command given as function argument
//    to FSM node

  std::cout << "<PixelDCSSMICommander::sendCommandToFSM>:" << std::endl;
  xoap::MessageReference psxRequest = MakeSOAPMessageReference_fsmSendCommand(fsmNodeName, fsmNodeOwner, fsmCommand);

  try {
    postSOAP(psxDescriptor_, psxRequest, "sendResponse");
  } catch ( xdaq::exception::Exception& e ) {
    XCEPT_RETHROW(xcept::Exception, "Failed to send Command to FSM", e);
  }
}

void PixelDCSSMICommander::releaseOwnershipOfFSM(const std::string& fsmNodeName,
						 const std::string& fsmNodeOwner) //throw (xdaq::exception::Exception)
{
//--- request PSX server to release ownership of PVSS FSM node 
//    given as function argument

  std::cout << "<PixelDCSSMICommander::releaseOwnershipOfFSM>:" << std::endl;
  xoap::MessageReference psxRequest = MakeSOAPMessageReference_fsmReleaseOwnership(fsmNodeName, fsmNodeOwner);

  try {
    postSOAP(psxDescriptor_, psxRequest, "releaseResponse");
  } catch ( xdaq::exception::Exception& e ) {
    XCEPT_RETHROW(xcept::Exception, "Failed to release Ownership of FSM", e);
  }
}

std::string PixelDCSSMICommander::disconnectFromFSM(const std::string& psxConnectionId) //throw (xdaq::exception::Exception)
{
//--- request PSX server to disconnect from PVSS FSM hierarchy;
//    return value of function is a std::string connectionStatus
  
  std::cout << "<PixelDCSSMICommander::disconnectFromFSM>:" << std::endl;
  xoap::MessageReference psxRequest = MakeSOAPMessageReference_fsmDisconnect(psxConnectionId);

  try {
    postSOAP(psxDescriptor_, psxRequest, "disconnectResponse");
  } catch ( xdaq::exception::Exception& e ) {
    std::cout<<"[PixelDCSSMICommander::disconnectFromFSM] Caught exception. Failed to post SOAP to PSX server!"<<std::endl;
    XCEPT_RETHROW(xdaq::exception::Exception, "Failed to disconnect from FSM", e);
  }

  return std::string("disconnected");
}

//
//---------------------------------------------------------------------------------------------------
//

xoap::MessageReference PixelDCSSMICommander::MakeSOAPMessageReference_fsmNotifyResponse()
{
  xoap::MessageReference psxResponse = xoap::createMessage();
  xoap::SOAPEnvelope responseEnvelope = psxResponse->getEnvelope();
  xoap::SOAPBody responseBody = responseEnvelope.getBody();
  xoap::SOAPName commandElement = responseEnvelope.createName("notifyResponse", "smi", PSX_SMI_NS_URI);
  responseBody.addBodyElement(commandElement);

  return psxResponse;
}

//---------------------------------------------------------------------------------------------------
//                    SOAP MESSAGES TO CONNECT TO FSM HIERARCHY, TAKE CONTROL, 
//                    QUERY ITS STATE AND SEND COMMANDS TO IT
//---------------------------------------------------------------------------------------------------

xoap::MessageReference PixelDCSSMICommander::MakeSOAPMessageReference_fsmGetState(const std::string& fsmNodeName,
										  const std::string& fsmNodeDomain, 
										  const std::string& fsmNodeOwner)
{
//--- compose SOAP message requesting PSX server to send State of PVSS FSM node

  xoap::MessageReference psxRequest = xoap::createMessage();
  xoap::SOAPEnvelope envelope = psxRequest->getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName commandElement = envelope.createName("getState", "smi", PSX_SMI_NS_URI);
  xoap::SOAPBodyElement bodyElement = body.addBodyElement(commandElement);
  xoap::SOAPName nodeAttribute = envelope.createName("object", "", "");
  bodyElement.addAttribute(nodeAttribute, fsmNodeName);
  if ( fsmNodeDomain != "" ) {
    xoap::SOAPName domainAttribute = envelope.createName("domain", "", "");
    bodyElement.addAttribute(domainAttribute, fsmNodeDomain);
  }
  xoap::SOAPName ownerAttribute = envelope.createName("owner", "", "");
  bodyElement.addAttribute(ownerAttribute, fsmNodeOwner);

  return psxRequest;
}

xoap::MessageReference PixelDCSSMICommander::MakeSOAPMessageReference_fsmConnect(const std::string& fsmNodeName,
										 const std::string& fsmNodeDomain, 
										 const std::string& fsmNodeOwner)
{
//--- compose SOAP message requesting PSX server to connect to PVSS FSM hierarchy 

  xoap::MessageReference psxRequest = xoap::createMessage();
  xoap::SOAPEnvelope envelope = psxRequest->getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName commandElement = envelope.createName("connect", "smi", PSX_SMI_NS_URI);
  xoap::SOAPBodyElement bodyElement = body.addBodyElement(commandElement);
  xoap::SOAPName nodeAttribute = envelope.createName("object", "", "");
  bodyElement.addAttribute(nodeAttribute, fsmNodeName);
  if ( fsmNodeDomain != "" ) {
    xoap::SOAPName domainAttribute = envelope.createName("domain", "", "");
    bodyElement.addAttribute(domainAttribute, fsmNodeDomain);
  }
  xoap::SOAPName ownerAttribute = envelope.createName("owner", "", "");
  bodyElement.addAttribute(ownerAttribute, fsmNodeOwner);
  xoap::SOAPName urlAttribute = envelope.createName("url", "", "");
  bodyElement.addAttribute(urlAttribute, app_->getApplicationContext()->getContextDescriptor()->getURL());
  //bodyElement.addAttribute(urlAttribute, "");
  xoap::SOAPName actionAttribute = envelope.createName("action", "", "");
  bodyElement.addAttribute(actionAttribute, app_->getApplicationDescriptor()->getURN());
  //bodyElement.addAttribute(actionAttribute, "");
  xoap::SOAPName contextAttribute = envelope.createName("context", "", "");
  bodyElement.addAttribute(contextAttribute, "none");     

  return psxRequest;
}

xoap::MessageReference PixelDCSSMICommander::MakeSOAPMessageReference_fsmTakeOwnership(const std::string& fsmNodeName,
										       const std::string& fsmNodeOwner,
										       bool fsmNodeExclusiveFlag)
{
//--- compose SOAP message requesting Ownership of PVSS FSM hierarchy 

  xoap::MessageReference psxRequest = xoap::createMessage();
  xoap::SOAPEnvelope envelope = psxRequest->getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName commandElement = envelope.createName("take", "smi", PSX_SMI_NS_URI);
  xoap::SOAPBodyElement bodyElement = body.addBodyElement(commandElement);
  xoap::SOAPName objectAttribute = envelope.createName("object", "", "");
  bodyElement.addAttribute(objectAttribute, fsmNodeName);
  xoap::SOAPName ownerAttribute = envelope.createName("owner", "", "");
  bodyElement.addAttribute(ownerAttribute, fsmNodeOwner);
  xoap::SOAPName exclusiveAttribute = envelope.createName("exclusive", "", "");
  std::string exclusiveValue = (fsmNodeExclusiveFlag) ? "true" : "false";
  bodyElement.addAttribute(exclusiveAttribute, exclusiveValue);

  return psxRequest;
}

xoap::MessageReference PixelDCSSMICommander::MakeSOAPMessageReference_fsmSendCommand(const std::string& fsmNodeName,
										     const std::string& fsmNodeOwner,
										     const std::string& fsmCommand)
{
//--- compose SOAP message for sending Command to PVSS FSM node

  xoap::MessageReference psxRequest = xoap::createMessage();
  xoap::SOAPEnvelope envelope = psxRequest->getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName commandElement = envelope.createName("send", "smi", PSX_SMI_NS_URI);
  xoap::SOAPBodyElement bodyElement = body.addBodyElement(commandElement);
  xoap::SOAPName objectAttribute = envelope.createName("object", "", "");
  bodyElement.addAttribute(objectAttribute, fsmNodeName);
  xoap::SOAPName commandAttribute = envelope.createName("command", "", "");
  bodyElement.addAttribute(commandAttribute, fsmCommand);
  xoap::SOAPName ownerAttribute = envelope.createName("owner", "", "");
  bodyElement.addAttribute(ownerAttribute, fsmNodeOwner);

  return psxRequest;
}

xoap::MessageReference PixelDCSSMICommander::MakeSOAPMessageReference_fsmReleaseOwnership(const std::string& fsmNodeName,
											  const std::string& fsmNodeOwner)
{
//--- compose SOAP message to release Ownership of PVSS FSM hierarchy 

  xoap::MessageReference psxRequest = xoap::createMessage();
  xoap::SOAPEnvelope envelope = psxRequest->getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName commandElement = envelope.createName("release", "smi", PSX_SMI_NS_URI);
  xoap::SOAPBodyElement bodyElement = body.addBodyElement(commandElement);
  xoap::SOAPName objectAttribute = envelope.createName("object", "", "");
  bodyElement.addAttribute(objectAttribute, fsmNodeName);
  xoap::SOAPName ownerAttribute = envelope.createName("owner", "", "");
  bodyElement.addAttribute(ownerAttribute, fsmNodeOwner);
  xoap::SOAPName allAttribute = envelope.createName("all", "", "");
  std::string allValue = "true";
  bodyElement.addAttribute(allAttribute, allValue);

  return psxRequest;
}

xoap::MessageReference PixelDCSSMICommander::MakeSOAPMessageReference_fsmDisconnect(const std::string& psxConnectionId)
{
//--- compose SOAP message requesting PSX server to disconnect from PVSS FSM hierarchy 

  xoap::MessageReference psxRequest = xoap::createMessage();
  xoap::SOAPEnvelope envelope = psxRequest->getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName commandElement = envelope.createName("disconnect", "smi", PSX_SMI_NS_URI);
  xoap::SOAPBodyElement bodyElement = body.addBodyElement(commandElement);
  xoap::SOAPName connectionIdAttribute("id", "", "");
  bodyElement.addAttribute(connectionIdAttribute, psxConnectionId);

  return psxRequest;
}



