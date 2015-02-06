//--*-C++-*--
// $Id: PixelDCSFSMInterface.h,v 1.25 2012/02/17 15:42:17 mdunser Exp $
/*************************************************************************
 * Interface class for sending states of CAEN A4602 and A4603            *
 * power supplies from PVSS to XDAQ                                      *
 *                                                                       *
 * Author: Christian Veelken, UC Davis				         *
 *                                                                       *
 * Last update: $Date: 2012/02/17 15:42:17 $ (UTC)                       *
 *          by: $Author: mdunser $                                       *
 *************************************************************************/

#ifndef _PixelDCSFSMInterface_h_
#define _PixelDCSFSMInterface_h_

#include <string>
#include <list>
#include <map>

#include "xdaq/Application.h"
#include "xdaq/exception/Exception.h"
#include "xoap/MessageReference.h"
#include "xgi/Method.h"
#include "toolbox/fsm/AsynchronousFiniteStateMachine.h"
#include "toolbox/lang/Class.h"
#include "toolbox/task/WaitingWorkLoop.h"
#include "toolbox/BSem.h"
#include "xdata/String.h"

//diagsystem
#include "toolbox/task/TimerListener.h"
#include <diagbag/DiagBagWizard.h>

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMNodeDefinition.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMDeviceDefinition.h"

#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"

#include "CalibFormats/SiPixelObjects/interface/PixelConfigKey.h"

#include "PixelDCSInterface/include/PixelDCSFSMPartition.h"

class xdaq::ApplicationStub;
class PixelDCSFSMNode;
class SOAPCommander;
class PixelDCSPVSSCommander;
class PixelDCSSMICommander;
class PixelDCSSMIConnectionManager;

class PixelDCSFSMInterface : public xdaq::Application, public SOAPCommander, public toolbox::task::TimerListener
{
 public:
  XDAQ_INSTANTIATOR();

  enum nodeType { kA4602, kA4603 };

  PixelDCSFSMInterface(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);
  ~PixelDCSFSMInterface();

  void Default(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  void XgiHandler(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  void AjaxHandler(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);

  xoap::MessageReference Initialize(xoap::MessageReference soapMessage) throw (xoap::exception::Exception);
  xoap::MessageReference Configure(xoap::MessageReference soapMessage) throw (xoap::exception::Exception);
  xoap::MessageReference Halt(xoap::MessageReference soapMessage) throw (xoap::exception::Exception);

  void stateConfiguring(toolbox::fsm::FiniteStateMachine &fsm) throw (toolbox::fsm::exception::Exception);
  void stateChanged(toolbox::fsm::FiniteStateMachine &fsm) throw (toolbox::fsm::exception::Exception);

  xoap::MessageReference FSMStateRequest(xoap::MessageReference soapMessage) throw (xoap::exception::Exception);

  xoap::MessageReference getPartitionState_Power(xoap::MessageReference soapRequest) throw (xoap::exception::Exception);
  xoap::MessageReference updatePartitionState_Power(xoap::MessageReference soapMessage) throw (xoap::exception::Exception);
  xoap::MessageReference updatePartitionState_ReadoutChips(xoap::MessageReference soapMessage) throw (xoap::exception::Exception);
  
  //for diagsystem
  void timeExpired (toolbox::task::TimerEvent& e);
  toolbox::BSem executeReconfMethodMutex;
  DiagBagWizard * diagService_;
  
 protected:
  template <class T> std::string getPVSSFSMNodeColor(const T* fsmDeviceDefinition, const std::string& fsmNodeState);

  void loadConfigFile() ;//throw (xdaq::exception::Exception);

  template <class T> void displayNodeState(xgi::Output *out,
					   const std::string& fsmPartitionName, const std::list<const T*> fsmNodeList, unsigned nodeType,
					   const std::string& fsmSummarizedState);
  template <class T> void updateNodeState(xgi::Output* out,
					  const std::string& fsmPartitionName, const std::list<const T*> fsmNodeList, unsigned nodeType);
  void displaySummarizedStates(xgi::Output *out, const PixelDCSFSMPartition& fsmPartition);
  void updateSummarizedStates(xgi::Output *out, const PixelDCSFSMPartition& fsmPartition);
    
  xoap::MessageReference Connect(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference Disconnect(xoap::MessageReference msg) throw (xoap::exception::Exception);
  
  bool connectToFSM_workloop(toolbox::task::WorkLoop* workloop);
  bool disconnectFromFSM_workloop(toolbox::task::WorkLoop* workloop);
  void connectToFSM(const std::string& fsmNode, const std::string& fsmNodeDomain, const std::string& workloopName) throw (xdaq::exception::Exception);
  void disconnectFromFSM(const std::string& fsmNode) throw (xdaq::exception::Exception);

  void decodePartitionState_Power(xoap::MessageReference soapMessage, const char* tagName) throw (xoap::exception::Exception);
  void determineSummarizedStateA4602(PixelDCSFSMPartition& fsmPartition);
  void determineSummarizedStateA4603(PixelDCSFSMPartition& fsmPartition);
  std::string getSummarizedState(const std::string& soapConnectionType, 
				 const std::string& stateA4602, const std::string& stateA4603) throw (xoap::exception::Exception);
  xoap::MessageReference composeFSMStateNotification_synchronous(const std::string& supervisorName, const std::string& supervisorType, unsigned int supervisorInstance);
  xoap::MessageReference composeFSMStateNotification_asynchronous(const PixelDCSFSMPartition& fsmPartition, const std::string& summarizedState);
  void updateSupervisors(std::string overrideA4602="",std::string overrideA4603="" );

  toolbox::fsm::AsynchronousFiniteStateMachine fsm_;
  
  xdata::String configFileName_;
  bool configFileLoaded_;

  xdata::String state_;
  xdaq::ApplicationDescriptor* PixelSupervisor_;


//--- connection status and Id information of PSX server
//    for states of PVSS FSM nodes
  std::map<std::string, std::string> psxConnectionStatus_;
  std::map<std::string, std::string> psxConnectionId_;

//--- map between PVSS FSM device types (first parameter)
//    and list of states associated to device type (second parameter),
//    together with information whether state means "On" or "Off"
//    (e.g. "RAMPING" means "Off")
  std::map<std::string, PixelDCSFSMNodeDefinition*> fsmNodeDefinitionMap_;
  std::map<std::string, PixelDCSFSMDeviceDefinition*> fsmDeviceDefinitionMap_;

//--- list of TTC partitions handled by this instance of PixelDCSFSMInterface
  std::list<PixelDCSFSMPartition> fsmPartitionList_; 
  std::map<std::string, std::string> fsmPartitionPreviousStateA4603_;
  std::map<std::string, std::string> fsmPartitionPreviousStateA4602_;

//--- temporary information needed for execution of connectToFSM member-function within work-loop
//    (signature of member-function registered for work-loop execution does not allow to pass function arguments 
//     other then the work-loop in which the member-function is executed)
  std::map<std::string, std::list<const PixelDCSFSMNode*> > workloopData_;
  std::map<std::string, std::string> workloopStatus_;
  
 private:

  std::string getUsedDescription(bool nodeIsUsed);
  std::string getUsedColor(bool nodeIsUsed);

  pos::PixelConfigKey *theGlobalKey_;
  toolbox::BSem* lock_;

  PixelDCSSMIConnectionManager *psxConnectionManager_;
  toolbox::task::WorkLoop * disconnectWorkloop_;
  bool readyToConnect_;
  bool readyToConfigure_;
  PixelTimer psxTimer_;

  SOAPCommander* soapCommander_; // pointer to auxiliary class used to generate SOAP messages sent to other XDAQ applications
  PixelDCSPVSSCommander* pvssCommander_; // pointer to auxiliary classes used to generate SOAP messages sent to PSX server,
                                         //  in order to send status of read-out chip initialization from XDAQ to PVSS 
  PixelDCSSMICommander* smiCommander_; //  in order to query state of CAEN A4602 and A4603 power supplies in PVSS FSM

  std::string XDAQ_ROOT;
};

#endif
