#include "Pixelb2inCommander.h"
#include "b2in/utils/MessengerCache.h"
#include "b2in/nub/Method.h"

#include "b2in/nub/exception/Exception.h"
#include "b2in/utils/exception/Exception.h"

#include "xdata/String.h"
#include "xdaq/Application.h"
#include "xdaq/Endpoint.h"
#include "xdaq/Network.h"
#include "pt/Address.h"
#include "xdata/UnsignedInteger.h"

#include <iostream>


using namespace std;

Pixelb2inCommander:: Pixelb2inCommander(xdaq::Application* app)
{

  
  ownerPointer_=true;

  lock_=new toolbox::BSem(toolbox::BSem::FULL,true);
  
  /*lock_ is used to obtain and grant access to the memory, in our case, the multithreaded processes may access messageID at the same time, it will bring confusion. So when one process accesses messageID, it first "takes" the memory to make sure other process cannot access the memory, after increasing messageID, it "gives" the memory and other process can access it.*/
  
  /*the set containter is used to store the messageID, returnMsgValue_ stores the return information that is needed for BaselineCalibration*/
  msgID_=new std::set<int>;
  returnMsgValue_=new std::map<int, std::string>;
  
  /*"daq" is the string appearing in ConfigurationNoRun.xml, <xc:Application class="PixelSupervisor" id="51" instance="0" network="local" group="daq" service="supervisor" />*/
  brokerGroup_="daq"; 

  /*used to initialized brokerProxy_, in our case, it can be either PixelSupervisor, PixelFEDSupervisor, PixelFECSupervisor ...*/
  application_=app;
    

  try{
    
    /*! Discover services in the provided list of groups (brokerGroup_). For each discovered service a messenger is created and put into the messenger cache */

    brokerProxy_ = new b2in::utils::ServiceProxy(application_, "b2in-eventing", brokerGroup_.toString(), this);
    
  }
  catch(b2in::utils::exception::Exception & e)
    {
      cout<<"brokerProxy_ cannot be initialized"<<endl;
      return;
    }


} 

/*copy constructor*/ 

Pixelb2inCommander::Pixelb2inCommander(const Pixelb2inCommander& p){

  ownerPointer_=false;
  msgID_=p.msgID_;
  lock_=p.lock_;
  application_=p.application_;
  brokerProxy_=p.brokerProxy_;
  brokerGroup_=p.brokerGroup_;
  returnMsgValue_=p.returnMsgValue_;
  
  
} 
/*deconstructor*/
Pixelb2inCommander::~Pixelb2inCommander(){

  if(ownerPointer_) 
    { 
      delete msgID_;
      delete lock_;
      delete returnMsgValue_;
      
    }  
}

int Pixelb2inCommander::send(xdaq::ApplicationDescriptor* targetApplicationDescriptor, std::string& action, bool& hasReturnValue, Attribute_Vector parameters){

  static int messageID=0;
  messageID++;

  lock_->take();
  msgID_->insert(messageID);
  if(hasReturnValue){
    (*returnMsgValue_)[messageID]="";
  }
  lock_->give();

  xdaq::Network* network=application_->getApplicationContext()->getNetGroup()->getNetwork("xmas");

  std::string senderURL=network->getAddress(application_->getApplicationDescriptor()->getContextDescriptor())->toString();

  xdata::UnsignedIntegerT sendId=application_->getApplicationDescriptor()->getLocalId();
  char buffer[50];
  sprintf(buffer,"%d",sendId);
  std::string senderId(buffer);  

  std::string targetURL=network->getAddress(targetApplicationDescriptor->getContextDescriptor())->toString();

  xdata::UnsignedIntegerT lid=targetApplicationDescriptor->getLocalId();
  sprintf(buffer,"%d",lid);
  std::string id(buffer);  

  brokerProxy_->addURL(targetURL);

   
  b2in::utils::MessengerCache * messengerCache  = 0;
   
  try
    {

       messengerCache = brokerProxy_->getMessengerCache();
       

     }
  catch(b2in::utils::exception::Exception & e)
     {
       cout<<"messengerCache error"<<endl;
       return messageID;
     }


   std::list<std::string> destinations = messengerCache->getDestinations();
	
   xdata::Properties myData;

   myData.setProperty("action",action);

   if ( !destinations.empty() )
     {
      
       myData.setProperty ("urn:b2in-protocol:lid", id);

       if(parameters.size()!=0){
	 for(unsigned int i=0; i<parameters.size(); i++){
	   myData.setProperty (parameters[i].name_, parameters[i].value_);
	 }
       }
       std::string messageid =SOAPCommander::itoa(messageID);
       myData.setProperty ("messageID",messageid);
       myData.setProperty ("return:url", senderURL);
       myData.setProperty ("return:id", senderId);
    
     }

   try
     {
      
       messengerCache->send(targetURL,0,myData);


       return  messageID;
     }
   catch (b2in::nub::exception::InternalError & e)
     {
       std::stringstream msg;
       
       cout << "Failed 1"<<endl;
      
       msg << "Failed to send to url " <<(targetURL);
      
       application_->notifyQualified("fatal",e);
       return messageID;
     }
   catch ( b2in::nub::exception::QueueFull & e )
     {
       std::stringstream msg;
       msg << "Failed to send to url " <<(targetURL);
       
       cout << "Failed 2"<<endl;
      
       application_->notifyQualified("error",e);
       return messageID;
     }
   catch ( b2in::nub::exception::OverThreshold& e)
     {
       
       cout << "Failed 3"<<endl;
       return messageID;
     }
   

   return messageID;
   
}


void Pixelb2inCommander::sendReply(xdata::Properties inputProperties){

  std::string returnURL=inputProperties.getProperty("return:url");
  std::string returnId=inputProperties.getProperty("return:id");

  brokerProxy_->addURL(returnURL);

  b2in::utils::MessengerCache * messengerCache  = 0;


  try
    {
      messengerCache = brokerProxy_->getMessengerCache();
    }
  catch(b2in::utils::exception::Exception & e)
    {
      std::cout<<"messengerCache error"<<std::endl;
      return;
    }

  std::list<std::string> destinations = messengerCache->getDestinations();

  if ( !destinations.empty() )
    {
      inputProperties.setProperty ("b2in-eventing:action", "return");
      inputProperties.setProperty ("urn:b2in-protocol:lid", returnId);
    }

  try
    {
      messengerCache->send(returnURL,0,inputProperties);
      return;
    }
  catch (b2in::nub::exception::InternalError & e)
    {
      std::stringstream msg;
      
      cout << "[b2inEvent] Failed 1"<<endl;

      msg << "Failed to send to url " <<returnURL;

      application_->notifyQualified("fatal",e);
      return;
    }
  catch ( b2in::nub::exception::QueueFull & e )
    {
      std::stringstream msg;
      msg << "Failed to send to url " <<returnURL;
	    
      cout << "[b2inEvent] Failed 2"<<endl;

      application_->notifyQualified("error",e);
      return;
    }
  catch ( b2in::nub::exception::OverThreshold& e)
    {
      cout << "[b2inEvent] Failed 3"<<endl;

      return;
    }
  
}
  
  
bool Pixelb2inCommander::waitForReply(int messageID){

  bool more=false;

  do{

    lock_->take();

    more = (msgID_->find(messageID)!=msgID_->end());
    
    lock_->give();
    
  }while(more);

  return true;    
}

std::string Pixelb2inCommander::waitForReplyWithReturn(int messageID){

 bool more=false;
 std::string returnValue="";

  do{

    lock_->take();

    more = (msgID_->find(messageID)!=msgID_->end());
    
    lock_->give();
    
  }while(more);

  lock_->take();
  returnValue=returnMsgValue_->find(messageID)->second;
  returnMsgValue_->erase(messageID);
  lock_->give();

  return returnValue;
  



}

void Pixelb2inCommander::removeMsgID(int messageID, std::string returnValue){
  
  lock_->take();
  
  if(returnMsgValue_->find(messageID)!=returnMsgValue_->end()){

    returnMsgValue_->find(messageID)->second=returnValue;

  }
  
  msgID_->erase(messageID);
  lock_->give();

}



