// $Id: PixelDCSCreateDataPoints.h,v 1.4 2007/08/13 10:07:50 veelken Exp $

/*************************************************************************
 * Base class for PixelDCStoFECCreateDataPoints and PixelDCStoFEDCreateDataPoints; *
 * implements access to Oracle DataBase                                  *
 *                                                                       *
 * Author: Christian Veelken, UC Davis				         *
 *                                                                       *
 * Last update: $Date: 2007/08/13 10:07:50 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#ifndef _PixelDCSCreateDataPoints_h_
#define _PixelDCSCreateDataPoints_h_

#include <string>

#include "xdaq/Application.h"
#include "xdaq/exception/Exception.h"
#include "xoap/MessageReference.h"
#include "xgi/Method.h"
#include "toolbox/fsm/FiniteStateMachine.h"
#include "toolbox/lang/Class.h"
#include "toolbox/task/WaitingWorkLoop.h"
#include "xdata/Table.h"
#include "xdata/String.h"
#include "xdata/Integer.h"

#include "PixelDCSCreateDataPointsDpNameTable.h"

//class xdaq::ApplicationStub;
class SOAPCommander;
class PixelDCSPVSSCommander;

class PixelDCSCreateDataPoints : public xdaq::Application
{
 public:
  
  XDAQ_INSTANTIATOR();
  
  PixelDCSCreateDataPoints(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);
  virtual ~PixelDCSCreateDataPoints();
  
  void Default(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  void XgiHandler(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  
  xoap::MessageReference createDataPointsFED(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference createDataPointsTrkFEC_Temperatures(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference createDataPointsTrkFEC_Voltages(xoap::MessageReference msg) throw (xoap::exception::Exception);
  
 protected:
  
  void createDataPoints();
  bool createDataPoints_workloop(toolbox::task::WorkLoop* workloop);
  
  std::string httpPageHeader_;
  
  PixelDCSCreateDataPointsDpNameTable* dpNameTable_; // list of data-points to be created
  
  std::string workloopStatus_;
  
  std::list<std::string> textfieldEntries_;

  SOAPCommander* soapCommander_; // pointer to auxiliary class used to generate SOAP messages sent to other XDAQ applications
  PixelDCSPVSSCommander* pvssCommander_; // pointer to auxiliary class used to generate SOAP messages sent to PSX server, 
                                         // in order to get or set values of PVSS data-points 
  
  std::string XDAQ_ROOT;
};

#endif
