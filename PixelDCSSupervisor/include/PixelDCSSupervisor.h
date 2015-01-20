// $Id: PixelDCSSupervisor.h,v 1.8 2012/01/20 19:49:49 kreis Exp $

/*************************************************************************
 * Supervisor class for integration of Run-Control                       *
 * and Detector Control systems, as described in CMS IN 2005/015;        *
 * sends SOAP messages containing the current state of the topmost       *
 * PVSS FSM node to the PixelSupervisor                                  *
 *                                                                       *
 * Author: Christian Veelken, UC Davis				         *
 *                                                                       *
 * Last update: $Date: 2012/01/20 19:49:49 $ (UTC)                       *
 *          by: $Author: kreis $                                       *
 *************************************************************************/

#ifndef _PixelDCSSupervisor_h_
#define _PixelDCSSupervisor_h_

#include <string>

#include "xdaq/Application.h"
#include "xdaq/exception/Exception.h"
#include "xoap/MessageReference.h"
#include "xgi/Method.h"
#include "toolbox/fsm/FiniteStateMachine.h"
#include "xdata/String.h"

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"

#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMNodeDefinition.h"

class xdaq::ApplicationStub;
class PixelDCSSMICommander;

class PixelDCSSupervisor : public xdaq::Application, public SOAPCommander
{
 public:
  XDAQ_INSTANTIATOR();
  
  PixelDCSSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);
  ~PixelDCSSupervisor();
  
  void Default(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  void XgiHandler(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  void AjaxHandler(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception);

  xoap::MessageReference Initialize(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference Configure(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference Start(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference Pause(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference Resume(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference Halt(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference Stop(xoap::MessageReference msg) throw (xoap::exception::Exception);
  
 protected:
  xoap::MessageReference Connect(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference Disconnect(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference Take(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference Release(xoap::MessageReference msg) throw (xoap::exception::Exception);
  
  std::string getPVSSFSMNodeColor(const std::string& pvssFSMNodeState);

  void loadConfigFile() throw (xdaq::exception::Exception);
  
  void connectToFSM(const std::string& fsmNodeName) throw (xdaq::exception::Exception);
  void takeOwnershipOfFSM(const std::string& fsmNodeName) throw (xdaq::exception::Exception);
  void sendCommandToFSM(const std::string& fsmNodeName, const std::string& fsmNodeType, 
					    const std::string& command) throw (xdaq::exception::Exception);
  void releaseOwnershipOfFSM(const std::string& fsmNodeName) throw (xdaq::exception::Exception);
  void disconnectFromFSM(const std::string& fsmNodeName) throw (xdaq::exception::Exception);
  
  xoap::MessageReference updateFSMState (xoap::MessageReference msg) throw (xoap::exception::Exception);
  
  toolbox::fsm::FiniteStateMachine fsm_;
  
  xdata::String pvssFSMNodeName_;
  xdata::String pvssFSMNodeType_;
  xdata::String pvssFSMNodeOwnership_;
  std::string pvssCurrentFSMNodeOwnership_;
  std::string pvssCurrentFSMState_;
  
  xdata::String configFileName_;
  bool configFileLoaded_;
  
  std::map<std::string, PixelDCSFSMNodeDefinition*> fsmNodeDefinitionMap_;
  
  PixelDCSSMICommander* smiCommander_; // pointer to auxiliary class used to generate SOAP messages sent to PSX server, 
                                       // in order to communicate with PVSS Finite State Machine (FSM)

  std::string psxConnectionStatus_;
  std::string psxConnectionId_;
  
 private:
  std::string XDAQ_ROOT;
};

#endif
