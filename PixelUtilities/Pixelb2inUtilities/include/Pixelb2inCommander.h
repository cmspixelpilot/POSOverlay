/*****************************************************************************************
 *This class is used to send b2in message, for each message, there is a message ID. After*
 *the message is received, this message ID is delted.                                    *
 *****************************************************************************************/

#ifndef _Pixelb2inCommander_h
#define _Pixelb2inCommander_h

#include "b2in/utils/MessengerCache.h"
#include "b2in/utils/MessengerCacheListener.h"
#include "b2in/nub/Method.h"
#include "b2in/utils/ServiceProxy.h"

#include "b2in/nub/exception/Exception.h"
#include "b2in/utils/exception/Exception.h"

#include "xdata/String.h"
#include "xdata/Properties.h"
#include "xdaq/Application.h"
#include "xdaq/ApplicationDescriptor.h"
#include "toolbox/BSem.h"


#include <iostream>

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"


using namespace std;

class Pixelb2inCommander : public b2in::utils::MessengerCacheListener{

 public:
  
  Pixelb2inCommander(xdaq::Application* app);
  
  Pixelb2inCommander(const Pixelb2inCommander& p); 

  virtual ~Pixelb2inCommander();

  int send(xdaq::ApplicationDescriptor* targetApplicationDescriptor, std::string& action, bool& hasReturnValue, Attribute_Vector parameters = Attribute_Vector(0));

  void sendReply(xdata::Properties inputProperties);
  
  bool waitForReply(int messageID);
  std::string waitForReplyWithReturn(int messageID);

  void removeMsgID(int messageID, std::string returnValue);

 protected:
  
  xdaq::Application* application_;
  void asynchronousExceptionNotification(xcept::Exception& e){}
  

 private:

  Pixelb2inCommander(); 
  xdata::String brokerGroup_; 
  b2in::utils::ServiceProxy* brokerProxy_;
  toolbox::BSem* lock_;
  bool ownerPointer_;
  std::set<int>* msgID_;
  std::map<int, std::string>* returnMsgValue_;
  
  
};

#endif
