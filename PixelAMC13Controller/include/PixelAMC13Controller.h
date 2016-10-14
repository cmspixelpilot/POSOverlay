#ifndef _PixelAMC13Controller_h_
#define _PixelAMC13Controller_h_

#include "xdaq/Application.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/exception/Exception.h"

#include "xdaq/NamespaceURI.h"

#include "xgi/Utils.h"
#include "xgi/Method.h"
#include "cgicc/CgiDefs.h"
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPBody.h"
#include "xoap/Method.h"

#include "xdata/Boolean.h"
#include "xdata/UnsignedInteger.h"
#include "xdata/String.h"

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"
#include "CalibFormats/SiPixelObjects/interface/PixelAMC13Parameters.h"
#include "PixelUtilities/PixeluTCAUtilities/include/PixelAMC13Interface.h"

class PixelAMC13Controller : public xdaq::Application, public SOAPCommander {
 public:
  XDAQ_INSTANTIATOR();

  xdata::Boolean doNothing;
  xdata::String uriT1;
  xdata::String uriT2;
  xdata::String addressT1;
  xdata::String addressT2;
  xdata::String mask;
  xdata::UnsignedInteger calBX;
  xdata::UnsignedInteger L1ADelay;
  xdata::Boolean newWay;
  xdata::Boolean verifyL1A;
  xdata::Boolean simTTC;

  PixelAMC13Controller(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);
  ~PixelAMC13Controller();

  void Default(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
  void StateMachineXgiHandler(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);
  void AllAMC13Tables(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);

  xoap::MessageReference Reset(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference Configuration(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference Configure(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference ConfigureFromXML(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference userCommand(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference Enable(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference Stop(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference Suspend(xoap::MessageReference msg) throw (xoap::exception::Exception);

 private:
  std::vector<pos::PixelAMC13Parameters> amc13_cfgs;
  std::vector<PixelAMC13Interface*> amc13s;
  void AddAMC13(const pos::PixelAMC13Parameters& p);
  void InitAMC13s();
  bool AMC13sReady() { return bool(amc13s.size()); }
  void DelAMC13s();
};

#endif
