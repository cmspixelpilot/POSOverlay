// $Id: PixelDCSDpInterface.h,v 1.8 2010/04/26 08:27:00 joshmt Exp $

/*************************************************************************
 * Base class for PixelDCStoFECDpInterface and PixelDCStoFEDDpInterface; *
 * implements access to Oracle DataBase                                  *
 *                                                                       *
 * Author: Christian Veelken, UC Davis				         *
 *                                                                       *
 * Last update: $Date: 2010/04/26 08:27:00 $ (UTC)                       *
 *          by: $Author: joshmt $                                       *
 *************************************************************************/

#ifndef _PixelDCSDpInterface_h_
#define _PixelDCSDpInterface_h_

#include <string>
#include <sstream>

#include "xdaq/Application.h"
#include "xdaq/exception/Exception.h"
#include "xoap/MessageReference.h"
#include "xgi/Method.h"
#include "toolbox/fsm/AsynchronousFiniteStateMachine.h"

#include "xdata/Table.h"
#include "xdata/String.h"
#include "xdata/Integer.h"

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSPVSSDpe.h"

//class xdaq::ApplicationStub;

struct PixelDCSDpInterfaceStatus
{
  float dpRate_;
  float dpLoad_;
  bool heartbeat_;
  bool busy_;
  bool error_;
};

void addStatus(const PixelDCSDpInterfaceStatus& status, const std::string& dpName_status,
	       std::list<PixelDCSPVSSDpe>& dpeList);

class PixelDCSDpInterface : public xdaq::Application, public SOAPCommander
{
 public:
  
  XDAQ_INSTANTIATOR();
  
  PixelDCSDpInterface(xdaq::ApplicationStub * s) throw (xcept::Exception);
  virtual ~PixelDCSDpInterface();
  
  virtual void Default(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  virtual void XgiHandler(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  
  virtual xoap::MessageReference Configure(xoap::MessageReference msg)  { return MakeSOAPMessageReference("DummyImplementation"); }
  virtual xoap::MessageReference Halt(xoap::MessageReference msg) throw (xoap::exception::Exception);

  virtual void stateConfiguring(toolbox::fsm::FiniteStateMachine & fsm) {std::cout<<"virtual void!"<<std::endl;}
  void stateChanged(toolbox::fsm::FiniteStateMachine & fsm);
  
 protected:
  
  void connectOracleDB(const std::string& oracleViewName, const std::string& oraclePassword) throw (xcept::Exception);
  xdata::Table getOracleTable(const std::string& oracleViewName, const std::string& oracleTableName) throw (xcept::Exception);
  void disconnectOracleDB(const std::string& oracleViewName) throw (xcept::Exception);
  
  toolbox::fsm::AsynchronousFiniteStateMachine fsm_;
  
  std::string httpPageHeader_;
  
  std::string oracleConnectionId_;

  xdata::String oracleUserName_;
  xdata::String oraclePassword_;
  xdata::String oracleViewName_dpNames_;
  xdata::String oracleTableName_dpNames_;
  std::string oraclePassword_dpNames_;
  xdata::String oracleViewName_dpFilter_;
  xdata::String oracleTableName_dpFilter_;
  std::string oraclePassword_dpFilter_;
  xdata::String oracleViewName_dpCalibration_;
  xdata::String oracleTableName_dpCalibration_;
  std::string oraclePassword_dpCalibration_;
  xdata::String version_dcu_calib_filter_;                    // version of filter table to be used
  
  xdata::Integer dpValueUpdate_maxLength_;
  
  PixelDCSDpInterfaceStatus status_;
  xdata::String dpName_status_;

  // fault strings
  std::stringstream fault_message_db_connection_;
  std::stringstream fault_string_db_get_table_;
  
  std::string XDAQ_ROOT;

  bool debug_;
};

#endif
