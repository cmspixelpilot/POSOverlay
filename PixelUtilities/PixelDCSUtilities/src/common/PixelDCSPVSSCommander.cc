
#include "PixelDCSPVSSCommander.h"

/*************************************************************************
 * Auxiliary class to get and set values of PVSS data-points             *
 * via PSX Server interface                                              *
 *                                                                       *
 * Author: Christian Veelken, UC Davis				         *
 *                                                                       *
 * Last update: $Date: 2009/09/14 11:43:39 $ (UTC)                       *
 *          by: $Author: joshmt $                                       *
 *************************************************************************/

#include <iomanip>

#define PSX_NS_URI "http://xdaq.cern.ch/xdaq/xsd/2006/psx-pvss-10.xsd"

const std::string dpeSuffix_set = ":_original.._value";
const std::string dpeSuffix_get = ":_online.._value";

PixelDCSPVSSCommander::PixelDCSPVSSCommander(xdaq::Application* xdaqApplication, xdaq::ApplicationDescriptor* psxDescriptor)
  : PixelDCSPSXCommander(xdaqApplication, psxDescriptor)
{
//--- nothing to be done yet...
}

PixelDCSPVSSCommander::~PixelDCSPVSSCommander()
{
//--- nothing to be done yet...
}

//
//---------------------------------------------------------------------------------------------------
//

std::list<PixelDCSPVSSDpe> PixelDCSPVSSCommander::getDpeValues(const std::list<PixelDCSPVSSDpe>& dpes) //throw (xdaq::exception::Exception)
{
  std::list<PixelDCSPVSSDpe> dpeValues;

//--- skip sending SOAP message in case list of data-point elements is empty
  if ( dpes.begin() != dpes.end() ) {
    xoap::MessageReference psxRequest = MakeSOAPMessageReference_dpGet(dpes);

    try {
      xoap::MessageReference psxResponse = postSOAP(psxDescriptor_, psxRequest);
      
      xoap::SOAPEnvelope envelope = psxResponse->getSOAPPart().getEnvelope();
      xoap::SOAPBody body = envelope.getBody();
      xoap::SOAPName commandElement = envelope.createName("dpGetResponse");
      std::vector<xoap::SOAPElement> commandElements = body.getChildElements(commandElement);          	  
      for ( std::vector<xoap::SOAPElement>::iterator p_commandElement = commandElements.begin();
	    p_commandElement != commandElements.end(); ++p_commandElement ) {
	xoap::SOAPName dpElement = envelope.createName("dp");
	std::vector<xoap::SOAPElement> dpElements = p_commandElement->getChildElements(dpElement);          	  
	
	for ( std::vector<xoap::SOAPElement>::iterator dpElement = dpElements.begin();
	      dpElement != dpElements.end(); ++dpElement ) {
	  xoap::SOAPName nameElement = envelope.createName("name");
	  std::string dpeName = dpElement->getAttributeValue(nameElement);
	            
//--- find alias associated with dpeName                    
	  std::string dpeAlias;
          unsigned int numAssociations = 0;
	  for ( std::list<PixelDCSPVSSDpe>::const_iterator dpe = dpes.begin();
		dpe != dpes.end(); ++dpe ) {             
                
//--- dpeName contains system name prefix and ":_online.._value" suffix,
//    so do not use exact string comparisson, 
//    but just check that dpe->getName is contained within dpeName string;
//    print warning if association is not unique
	    if ( dpeName.find(dpe->getName()) != std::string::npos ) {
              dpeAlias = dpe->getAlias();
              ++numAssociations;
            }
	  }

          if ( numAssociations != 1 ) {
            std::cerr << "no or no unique Association of dpeName = " << dpeName << " to Alias !!!" << std::endl;
          }

	  std::string dpeValue = dpElement->getValue();
	  
	  PixelDCSPVSSDpe dpe(dpeName, dpeValue);
	  dpe.setAlias(dpeAlias);
	  
	  dpeValues.push_back(dpe);	  
	}
      }
    } catch ( xdaq::exception::Exception& e ) {
      std::cout << "Caught Exception" << std::endl;
      std::cout << " " << e.what() << std::endl;
      XCEPT_RETHROW(xdaq::exception::Exception, "Failed to getDpValue --> " + std::string(e.what()), e);
    }
  } else {
    XCEPT_RAISE(xdaq::exception::Exception, "Empty data-point element List");
  }

  return dpeValues;
}

std::string PixelDCSPVSSCommander::getDpeValue(const std::string& dpeName) //throw (xoap::exception::Exception)
{
  std::string dpeValue = "";

  xoap::MessageReference psxRequest = MakeSOAPMessageReference_dpGet(dpeName);
  
  try {
    xoap::MessageReference psxResponse = postSOAP(psxDescriptor_, psxRequest);
    
    xoap::SOAPEnvelope envelope = psxResponse->getSOAPPart().getEnvelope();
    xoap::SOAPBody body = envelope.getBody();
    xoap::SOAPName commandElement = envelope.createName("dpGetResponse");
    std::vector<xoap::SOAPElement> commandElements = body.getChildElements(commandElement);          	  
    for ( std::vector<xoap::SOAPElement>::iterator p_commandElement = commandElements.begin();
	  p_commandElement != commandElements.end(); ++p_commandElement ) {
      xoap::SOAPName dpElement = envelope.createName("dp");
      std::vector<xoap::SOAPElement> dpElements = p_commandElement->getChildElements(dpElement);          	  
      
      if ( dpElements.size() == 1 ) {
	dpeValue = dpElements[0].getValue();
      }
    }
  } catch ( xdaq::exception::Exception& e ) {
    std::cout << "Caught Exception" << std::endl;
    std::cout << " " << e.what() << std::endl;
    XCEPT_RETHROW(xdaq::exception::Exception, "Failed to getDpValue --> " + std::string(e.what()), e);
  }

  return dpeValue;
}

bool PixelDCSPVSSCommander::getDpeValue_bool(const std::string& dpeName) //throw (xdaq::exception::Exception) 
{
/*
  std::string dpeValue_string = getDpeValue(dpeName);
  if ( dpeValue_string == "true" || 
       dpeValue_string == "TRUE" ) {
    return true;
  } else if ( dpeValue_string == "false" || 
	      dpeValue_string == "FALSE" ) {
    return false;
  } else {
    XCEPT_RAISE (xdaq::exception::Exception, "Undefined dpeValue");
  }
*/
//--- boolean data-points not yet implemented 
//    in PSX server/PVSS API
  std::string dpeValue_string = getDpeValue(dpeName);
  if ( dpeValue_string == "1" ) {
    return true;
  } else if ( dpeValue_string == "0" ) {
    return false;
  } else {
    XCEPT_RAISE (xdaq::exception::Exception, "Undefined dpeValue");
  }
}

float PixelDCSPVSSCommander::getDpeValue_float(const std::string& dpeName) 
{
  std::string dpeValue_string = getDpeValue(dpeName);
  float dpeValue_float = atof(dpeValue_string.data());
  return dpeValue_float;
}

int PixelDCSPVSSCommander::getDpeValue_int(const std::string& dpeName)
{
  std::string dpeValue_string = getDpeValue(dpeName);
  int dpeValue_int = atoi(dpeValue_string.data());
  return dpeValue_int;
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelDCSPVSSCommander::setDpeValues(const std::list<PixelDCSPVSSDpe>& dpes) //throw (xdaq::exception::Exception)
{
//--- skip sending SOAP message in case list of data-point elements is empty
  if ( dpes.begin() != dpes.end() ) {
    xoap::MessageReference psxRequest = MakeSOAPMessageReference_dpSet(dpes);

    try {
      postSOAP(psxDescriptor_, psxRequest);
    } catch ( xdaq::exception::Exception& e ) {
      std::cout << "Caught Exception" << std::endl;
      std::cout << " " << e.what() << std::endl;
      XCEPT_RETHROW(xdaq::exception::Exception, "Failed to setDpeValues --> " + std::string(e.what()), e);
    }
  } else {
    XCEPT_RAISE(xdaq::exception::Exception, "Empty data-point element List");
  }
}

void PixelDCSPVSSCommander::setDpeValue(const std::string& dpeName, const::std::string& dpeValue) //throw (xdaq::exception::Exception) 
{
  xoap::MessageReference psxRequest = MakeSOAPMessageReference_dpSet(dpeName, dpeValue);
 
  try {
    postSOAP(psxDescriptor_, psxRequest);
  } catch ( xdaq::exception::Exception& e ) {
    std::cout << "Caught Exception" << std::endl;
    std::cout << " " << e.what() << std::endl;
    XCEPT_RETHROW(xdaq::exception::Exception, "Failed to setDpValue --> " + std::string(e.what()), e);
  }
}

void PixelDCSPVSSCommander::setDpeValue_bool(const std::string& dpeName, bool dpeValue)
{
/*
  std::string dpeValue_string;
  if ( dpeValue == true ) {
    dpeValue_string = "TRUE";
  } else {
    dpeValue_string = "FALSE";
  }
  setDpeValue(dpeName, dpeValue_string);
 */
//--- boolean data-points not yet implemented 
//    in PSX server/PVSS API
  if ( dpeValue == true ) {  
    setDpeValue_int(dpeName, 1);
  } else {
    setDpeValue_int(dpeName, 0);
  }
}

void PixelDCSPVSSCommander::setDpeValue_float(const std::string& dpeName, float dpeValue)
{
  std::ostringstream dpeValue_string;
  dpeValue_string.setf(std::ios::fixed);
  dpeValue_string << std::setprecision(1) << dpeValue;
  setDpeValue(dpeName, dpeValue_string.str());
}

void PixelDCSPVSSCommander::setDpeValue_int(const std::string& dpeName, int dpeValue)
{
  std::ostringstream dpeValue_string;
  dpeValue_string << dpeValue;
  setDpeValue(dpeName, dpeValue_string.str());
}

//---------------------------------------------------------------------------------------------------
//                    SOAP MESSAGES TO GET AND SET DATA-POINT VALUES
//---------------------------------------------------------------------------------------------------

xoap::MessageReference PixelDCSPVSSCommander::MakeSOAPMessageReference_dpGet(const std::list<PixelDCSPVSSDpe>& dpes)
{
  xoap::MessageReference psxRequest = xoap::createMessage();
  xoap::SOAPEnvelope envelope = psxRequest->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName commandElement = envelope.createName("dpGet", "psx", PSX_NS_URI);
  xoap::SOAPBodyElement bodyElement = body.addBodyElement(commandElement);

  for ( std::list<PixelDCSPVSSDpe>::const_iterator dpe = dpes.begin();
	dpe != dpes.end(); ++dpe ) {
    const std::string& dpeName = dpe->getName();

    xoap::SOAPName dpElement = envelope.createName("dp");
    xoap::SOAPElement childElement = bodyElement.addChildElement(dpElement);
    xoap::SOAPName nameElement = envelope.createName("name");

    childElement.addAttribute(nameElement, dpeName + dpeSuffix_get);
  }
  
  return psxRequest;
}

xoap::MessageReference PixelDCSPVSSCommander::MakeSOAPMessageReference_dpGet(const std::string& dpeName)
{
  xoap::MessageReference psxRequest = xoap::createMessage();
  xoap::SOAPEnvelope envelope = psxRequest->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName commandElement = envelope.createName("dpGet", "psx", PSX_NS_URI);
  xoap::SOAPBodyElement bodyElement = body.addBodyElement(commandElement);
  xoap::SOAPName dpElement = envelope.createName("dp");
  xoap::SOAPElement childElement = bodyElement.addChildElement(dpElement);
  xoap::SOAPName nameElement = envelope.createName("name");
  childElement.addAttribute(nameElement, dpeName + dpeSuffix_get);

  return psxRequest;
}

xoap::MessageReference PixelDCSPVSSCommander::MakeSOAPMessageReference_dpSet(const std::string& dpeName, const::std::string& dpeValue)
{
  xoap::MessageReference psxRequest = xoap::createMessage();
  xoap::SOAPEnvelope envelope = psxRequest->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName commandElement = envelope.createName("dpSet", "psx", PSX_NS_URI);
  xoap::SOAPBodyElement bodyElement = body.addBodyElement(commandElement);
  xoap::SOAPName dpElement = envelope.createName("dp");
  xoap::SOAPElement childElement = bodyElement.addChildElement(dpElement);
  xoap::SOAPName nameElement = envelope.createName("name");
  childElement.addAttribute(nameElement, dpeName + dpeSuffix_set);
  childElement.addTextNode(dpeValue);

  return psxRequest;
}

xoap::MessageReference PixelDCSPVSSCommander::MakeSOAPMessageReference_dpSet(const std::list<PixelDCSPVSSDpe>& dpes)
{
  xoap::MessageReference psxRequest = xoap::createMessage();
  xoap::SOAPEnvelope envelope = psxRequest->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName commandElement = envelope.createName("dpSet", "psx", PSX_NS_URI);
  xoap::SOAPBodyElement bodyElement = body.addBodyElement(commandElement);

  for ( std::list<PixelDCSPVSSDpe>::const_iterator dpe = dpes.begin();
	dpe != dpes.end(); ++dpe ) {
    const std::string& dpeName = dpe->getName();
    const::std::string& dpeValue = dpe->getValue();

    xoap::SOAPName dpElement = envelope.createName("dp");
    xoap::SOAPElement childElement = bodyElement.addChildElement(dpElement);
    xoap::SOAPName nameElement = envelope.createName("name");

    childElement.addAttribute(nameElement, dpeName + dpeSuffix_set);
    childElement.addTextNode(dpeValue);
  }
  
  return psxRequest;
}
