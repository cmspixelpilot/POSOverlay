// $Id: PixelDCSSMICommander.h,v 1.6 2009/09/14 11:43:38 joshmt Exp $

/*************************************************************************
 * Auxiliary class to query states of PVSS FSM nodes                     *
 * and send commands to PVSS FSM                                         *
 *                                                                       *
 * Author: Christian Veelken, UC Davis				         *
 *                                                                       *
 * Last update: $Date: 2009/09/14 11:43:38 $ (UTC)                       *
 *          by: $Author: joshmt $                                       *
 *************************************************************************/

#ifndef _PixelDCSSMICommander_h_
#define _PixelDCSSMICommander_h_

#include <string>

#include "xdaq/Application.h"
#include "xdaq/exception/Exception.h"
#include "xoap/MessageReference.h"

#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSPSXCommander.h"

//class xdaq::Application;

class PixelDCSSMICommander : public PixelDCSPSXCommander
{
 public:
  PixelDCSSMICommander(xdaq::Application* xdaqApplication, xdaq::ApplicationDescriptor* psxDescriptor);
  virtual ~PixelDCSSMICommander();

  // functions to query states of PVSS FSM nodes and send commands to PVSS FSM
  std::string getStateOfFSM(const std::string& fsmNodeName, const std::string& fsmNodeDomain, const std::string& fsmNodeOwner) ;
  std::pair<std::string, std::string> connectToFSM(const std::string& fsmNodeName, const std::string& fsmNodeDomain, 
						   const std::string& fsmNodeOwner) ;
  void takeOwnershipOfFSM(const std::string& fsmNodeName, const std::string& fsmNodeOwner,
			  bool fsmNodeExclusiveFlag) ;
  void sendCommandToFSM(const std::string& fsmNodeName, const std::string& fsmNodeOwner,
			const std::string& fsmCommand) ;
  void releaseOwnershipOfFSM(const std::string& fsmNodeName, const std::string& fsmNodeOwner) ;
  std::string disconnectFromFSM(const std::string& psxConnectionId) ;

  xoap::MessageReference MakeSOAPMessageReference_fsmNotifyResponse();

 protected:
  xoap::MessageReference MakeSOAPMessageReference_fsmGetState(const std::string& fsmNodeName, const std::string& fsmNodeDomain, const std::string& fsmNodeOwner);
  xoap::MessageReference MakeSOAPMessageReference_fsmConnect(const std::string& fsmNodeName, const std::string& fsmNodeDomain, const std::string& fsmNodeOwner);
  xoap::MessageReference MakeSOAPMessageReference_fsmTakeOwnership(const std::string& fsmNodeName, const std::string& fsmNodeOwner, 
								   bool fsmNodeExclusiveFlag);
  xoap::MessageReference MakeSOAPMessageReference_fsmSendCommand(const std::string& fsmNodeName, const std::string& fsmNodeOwner,
								 const std::string& fsmCommand);
  xoap::MessageReference MakeSOAPMessageReference_fsmReleaseOwnership(const std::string& fsmNodeName, const std::string& fsmNodeOwner);
  xoap::MessageReference MakeSOAPMessageReference_fsmDisconnect(const std::string& psxConnectionId);
};

#endif
