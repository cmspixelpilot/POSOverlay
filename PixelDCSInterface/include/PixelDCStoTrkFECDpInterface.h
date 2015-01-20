// $Id: PixelDCStoTrkFECDpInterface.h,v 1.22 2010/04/28 20:46:55 zatserkl Exp $

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.                                        *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini                                    *
 *                                                                       *
 * For the licensing terms see LICENSE.                                  *
 * For the list of contributors see CREDITS.                             *
 *************************************************************************/

#ifndef _PixelDCStoTrkFECDpInterface_h_
#define _PixelDCStoTrkFECDpInterface_h_

#include "xdaq/Application.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/ApplicationStubImpl.h"
#include "xdaq/exception/Exception.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/NamespaceURI.h"

#include "xdata/UnsignedLong.h"
#include "xdata/String.h"

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/AttachmentPart.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPMessage.h"
#include "xoap/SOAPBody.h"
#include "xoap/SOAPPart.h"
#include "xoap/DOMParser.h"
#include "xoap/SOAPName.h"
#include "xoap/SOAPElement.h"
#include "xoap/domutils.h"

#include "xoap/Method.h"

// Includes for the GUI
#include "xdaq/WebApplication.h"
#include "xgi/Method.h"
#include "xgi/Utils.h"
#include "cgicc/HTMLClasses.h"
#include "toolbox/fsm/FiniteStateMachine.h"
#include "toolbox/fsm/FailedEvent.h"
#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"

#include "PixelDCSInterface/include/PixelDCSDpInterface.h"

#include "PixelUtilities/PixelTKFECDataTools/include/PortCardDCU.h"

#include <string>
#include <sstream>

class PixelDCStoTrkFECDpInterface : public PixelDCSDpInterface
{
public:

  XDAQ_INSTANTIATOR();

  PixelDCStoTrkFECDpInterface(xdaq::ApplicationStub * s) throw (xcept::Exception);
  ~PixelDCStoTrkFECDpInterface();

  void Default(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  void XgiHandler(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);

  xoap::MessageReference Configure (xoap::MessageReference msg);// throw (xoap::exception::Exception, xcept::Exception);
  virtual void stateConfiguring(toolbox::fsm::FiniteStateMachine & fsm);

  xoap::MessageReference Halt (xoap::MessageReference msg) throw (xoap::exception::Exception);

  xoap::MessageReference updateDpValueTrkFEC (xoap::MessageReference msg) throw (xoap::exception::Exception);
  
protected:

  std::map<unsigned int         // fecBoardId
    , std::map< unsigned int    // mfecId
    , std::map< unsigned int    // ccuId
    , std::map< unsigned int    // ccuChannelId
    , std::map< unsigned int    // dcuChannelId
    , float
  > > > > > dpValueTable_;
  std::string request_;
  std::string response_;
  float lastTable_;

private:

  void HaltFSM();

  xdata::String psx_system_name_;     // prefix for datapoint name. Defined in xml configuration file (e.g. XDAQConfiguration/ConfigurationNoRU.xml)
  double weight_;                     // weight of the current value in computing of the average
  bool debug_;                        // debug printout
  int nsent_;                         // the number of message sent to PSX server
  int nreport_;                       // report every nreport_ events
  bool send_to_PSX_;                  // for test using DCU emulator 
  bool dcu_printout_on_;              // turn on printout (to cout) of DCU readout

  time_t lastTime_;
  std::string XDAQ_ROOT;

  std::stringstream fault_message_psx_;
  
  // Tora_dpTable_dpName_: map to obtain dpName
  std::map<unsigned int         // fecBoardId
    , std::map<unsigned int     // mfecId
    , std::map<unsigned int     // ccuId
    , std::map<unsigned int     // ccuChannelId
    , std::map<unsigned int     // dcuChannelId
    , std::string
  > > > > > Tora_dpTable_dpName_;
  
  // Tora_dpTable_dpAlias_: map to obtain dpAlias
  std::map<unsigned int         // fecBoardId
    , std::map<unsigned int     // mfecId
    , std::map<unsigned int     // ccuId
    , std::map<unsigned int     // ccuChannelId
    , std::map<unsigned int     // dcuChannelId
    , std::string
  > > > > > Tora_dpTable_dpAlias_;
  
  // deadband_: map to obtain deadband value for RTD2, RTD3 and AOH temperatures
  std::map<unsigned int         // fecBoardId
    , std::map<unsigned int     // mfecId
    , std::map<unsigned int     // ccuId
    , std::map<unsigned int     // ccuChannelId
    , std::map<unsigned int     // dcuChannelId
    , double
  > > > > > deadband_;
  
  // average value
  std::map<unsigned int         // fecBoardId
    , std::map<unsigned int     // mfecId
    , std::map<unsigned int     // ccuId
    , std::map<unsigned int     // ccuChannelId
    , std::map<unsigned int     // dcuChannelId
    , double
  > > > > > average_;
  
  // last sent to db value
  std::map<unsigned int         // fecBoardId
    , std::map<unsigned int     // mfecId
    , std::map<unsigned int     // ccuId
    , std::map<unsigned int     // ccuChannelId
    , std::map<unsigned int     // dcuChannelId
    , double
  > > > > > last_sent_;
  
  std::map<PortCard::Address, PortCard::DCU> dcu_map_;
};
#endif
