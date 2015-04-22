
#include "PixelDCSPSXCommander.h"

/*************************************************************************
 * Base class for                                                        *
 *  PixelPVSSCommander                                                   *
 *  PixelSMICommander                                                    *
 * (auxiliary classes for XDAQ-DCS communication)                        *
 *                                                                       *
 * Author: Christian Veelken, UC Davis				         *
 *                                                                       *
 * Last update: $Date: 2009/07/29 17:02:40 $ (UTC)                       *
 *          by: $Author: joshmt $                                       *
 *************************************************************************/

#include <iomanip>
#include <iostream>
using namespace std;


PixelDCSPSXCommander::PixelDCSPSXCommander(xdaq::Application* xdaqApplication, xdaq::ApplicationDescriptor* psxDescriptor)
  : SOAPCommander(xdaqApplication)
{
  psxDescriptor_ = psxDescriptor;
}

PixelDCSPSXCommander::~PixelDCSPSXCommander()
{
//--- nothing to be done yet...
}

//
//---------------------------------------------------------------------------------------------------
//

xoap::MessageReference PixelDCSPSXCommander::postSOAP(xdaq::ApplicationDescriptor* psxDescriptor, xoap::MessageReference psxRequest,
						      const char* expectedResponse) throw (xdaq::exception::Exception)
{
//   std::cout << "<PixelDCSPSXCommander::postSOAP>:" << std::endl;

//   std::cout << "sending SOAP message to " << psxDescriptor_->getContextDescriptor()->getURL() << std::endl;
//   std::cout << " Request : ------------------------------------ "<< std::endl;
//   psxRequest->writeTo(std::cout);
//   std::cout << std::endl;
//   std::cout << " ---------------------------------------------- "<< std::endl;

  xoap::MessageReference psxResponse;
  try {
    psxResponse = app_->getApplicationContext()->postSOAP(psxRequest, *app_->getApplicationDescriptor(), *psxDescriptor);
  } catch ( xdaq::exception::Exception& e ) {
    std::cout << "[PixelDCSPSXCommander::postSOAP] Caught Exception: " << e.what() << std::endl;
    XCEPT_RETHROW(xdaq::exception::Exception, "Failed to send SOAP message --> " + std::string(e.what()), e);
  }

//   std::cout <<" Reply : -------------------------------------- "<< std::endl;
//   psxResponse->writeTo(std::cout);
//   std::cout << std::endl;
//   std::cout <<" ---------------------------------------------- "<< std::endl;
  
  xoap::SOAPEnvelope envelope = psxResponse->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  if ( !body.hasFault() ) {
    //if ( expectedResponse != "" ) {
    if ( strcmp(expectedResponse,"") != 0 ) {
      xoap::SOAPName responseElement = envelope.createName(expectedResponse);
      std::vector<xoap::SOAPElement> bodyElements = body.getChildElements(responseElement);
      if ( bodyElements.size() != 1 ) {
	XCEPT_RAISE (xdaq::exception::Exception, "Failed to receive expected SOAP response");
	return MakeSOAPMessageReference("SendFailed");
      }
    }
    
//--- actual response matches expected response
//    or response not required to be checked
      return psxResponse;
  } else {
    xoap::SOAPFault fault = body.getFault();
    XCEPT_RAISE (xdaq::exception::Exception, "Failure in sending SOAP message --> " + std::string(fault.getFaultString()));
    return MakeSOAPMessageReference("SendFailed");
  }

  return psxResponse;
}
