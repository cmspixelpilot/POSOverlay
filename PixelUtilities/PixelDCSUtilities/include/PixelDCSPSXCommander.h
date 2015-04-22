// $Id: PixelDCSPSXCommander.h,v 1.2 2007/10/25 09:37:28 veelken Exp $

/*************************************************************************
 * Base class for                                                        *
 *  PixelPVSSCommander                                                   *
 *  PixelSMICommander                                                    *
 * (auxiliary classes for XDAQ-DCS communication)                        *
 *                                                                       *
 * Author: Christian Veelken, UC Davis				         *
 *                                                                       *
 * Last update: $Date: 2007/10/25 09:37:28 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#ifndef _PixelDCSPSXCommander_h_
#define _PixelDCSPSXCommander_h_

#include <string>

#include "xdaq/Application.h"
#include "xdaq/exception/Exception.h"
#include "xoap/MessageReference.h"

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"

//class xdaq::Application;

class PixelDCSPSXCommander : public SOAPCommander
{
 public:
  PixelDCSPSXCommander(xdaq::Application* xdaqApplication, xdaq::ApplicationDescriptor* psxDescriptor);
  virtual ~PixelDCSPSXCommander();

 protected:
  xoap::MessageReference postSOAP(xdaq::ApplicationDescriptor* psxDescriptor, xoap::MessageReference psxRequest,
				  const char* expectedResponse = "") throw (xdaq::exception::Exception);
  
  xdaq::ApplicationDescriptor* psxDescriptor_;
};

#endif
