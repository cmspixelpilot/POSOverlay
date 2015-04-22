// $Id: PixelDCSTestApplication.h,v 1.11 2012/01/20 19:15:07 kreis Exp $

/*************************************************************************
 * Test class for commissioning of PixelDCSSupervisor,                   *
 * PixelDCStoFECDpInterface and PixelDCStoFEDDpInterface classes         *
 * (the latter two are contained in the PixelDCSInterface package);      *
 * sends dummy SOAP messages to the classes to be tested                 *
 * upon click of a button in the XDAQ webpage GUI                        *
 *                                                                       *
 * Author: Christian Veelken, UC Davis					 *
 *                                                                       *
 * Last update: $Date: 2012/01/20 19:15:07 $ (UTC)                       *
 *          by: $Author: kreis $                                       *
 *************************************************************************/

#ifndef _PixelDCSTestApplication_h_
#define _PixelDCSTestApplication_h_

#include <string>

#include "xdaq/Application.h"
#include "xdaq/exception/Exception.h"
#include "xoap/MessageReference.h"
#include "xgi/Method.h"
#include "toolbox/fsm/FiniteStateMachine.h"
#include "xdata/String.h"

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"

#include "PixelUtilities/PixelTKFECDataTools/include/PortCardDCU.h"

//class xdaq::ApplicationStub;

class PixelDCSTestApplication : public xdaq::Application, public SOAPCommander
{
 public:

  XDAQ_INSTANTIATOR();

  PixelDCSTestApplication(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);
  ~PixelDCSTestApplication();
  
  void Default(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  void XgiHandler(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  
  xoap::MessageReference TestDCStoFEDDpInterface(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference TestDCStoTrkFECDpInterface(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference TestDCStoTrkFECDpInterfaceConfigure(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference TestTrkFECSupervisor_DCUreadout(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference TestTrkFECSupervisor_DCUreadout_fakeSOAP(xoap::MessageReference msg) throw (xoap::exception::Exception);
  
 protected:
  
  xoap::MessageReference composeTestMessageFED();
  xoap::MessageReference composeTestMessageTrkFEC();

  std::string request_;
  std::string response_;
  
 private:
  
  bool debug_;

  xdata::String fedDpInterface_;
  xdata::String trkfecDpInterface_;
  xdata::String dcsSupervisor_;
  
  std::string XDAQ_ROOT;
  std::string htmlbase_,xmlbase_;
  
  std::map<PortCard::Address, PortCard::DCU> dcu_map_;
};

#endif
