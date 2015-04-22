#ifndef _SOAPCommander_h
#define _SOAPCommander_h
#include "xdaq/Application.h"
#include "xoap/Method.h"
#include "xdaq/NamespaceURI.h"
#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPPart.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPBody.h"
#include "xoap/domutils.h"
#include "xoap/AttachmentPart.h"

#include "xcept/tools.h"

#include "PixelSOAP.h"

#include <iostream>
#include <sys/time.h>

class SOAPCommander : public virtual toolbox::lang::Class
{
 public:
  
  SOAPCommander(xdaq::Application* app)
    {app_=app;}
  
  SOAPCommander(const SOAPCommander& aSOAPCommander)
    {app_=aSOAPCommander.app_;}
  
  static std::string itoa (int value)
  {
    char buffer[50];
    sprintf(buffer,"%d",value);
    std::string str(buffer);
    return str;
  }
  
  static std::string htoa(int value)
  {
    char buffer[50];
    sprintf(buffer,"%x",value);
    std::string str(buffer);
    return str;
    
  }
  
  std::string Receive(xoap::MessageReference msg)
    {
      std::vector<xoap::SOAPElement> bodyList = msg->getSOAPPart().getEnvelope().getBody().getChildElements();
      std::string command = bodyList[0].getElementName().getLocalName();
      return command;
    }
  
  std::string Receive(xoap::MessageReference msg, Attribute_Vector &parameters)
    {
      xoap::SOAPEnvelope envelope=msg->getSOAPPart().getEnvelope();
      std::vector<xoap::SOAPElement> bodyList=envelope.getBody().getChildElements();
      xoap::SOAPElement command=bodyList[0];
      std::string commandName=command.getElementName().getLocalName();
      xoap::SOAPName name=envelope.createName("Key");
      
      for (unsigned int i=0;i<parameters.size();++i)
        {
          name=envelope.createName(parameters[i].name_);
          
          try{
            parameters[i].value_=command.getAttributeValue(name);
            if (parameters[i].value_==""&&parameters[i].name_!=""){
              std::cout<<" Complaint from "<<(app_->getApplicationDescriptor()->getClassName());
              std::cout <<" : Parameter "<<parameters[i].name_
                        <<" ("<<i
                        <<") does not exist in the list of incoming parameters!"<<std::endl;
              std::cout<<"It could also be because you passed an empty string"<<std::endl;
              //assert(0);
            };
          }
          catch (xoap::exception::Exception& e)
            {
              std::cout<<"Parameter "<<parameters[i].name_<<" does not exist in the list of incoming parameters!"<<std::endl;
              XCEPT_RETHROW(xoap::exception::Exception,"Looking for parameter that does not exist!",e);
            }
          
        }
      
      return commandName;
    }
  
  xoap::MessageReference MakeSOAPMessageReference(std::string cmd)
    {
      xoap::MessageReference msg = xoap::createMessage();
      xoap::SOAPEnvelope envelope = msg->getSOAPPart().getEnvelope();
      //envelope.addNamespaceDeclaration("xsi", "http://www.w3.org/2001/XMLSchema-instance");
      //envelope.addNamespaceDeclaration("xsd", "http://www.w3.org/2001/XMLSchema");
      xoap::SOAPName command = envelope.createName(cmd, "xdaq", XDAQ_NS_URI);
      xoap::SOAPBody body= envelope.getBody();
      body.addBodyElement(command);
      return msg;
    }

  xoap::MessageReference MakeSOAPMessageReference(std::string cmd, Attribute_Vector attrib)
    {
      xoap::MessageReference msg = xoap::createMessage();
      xoap::SOAPEnvelope envelope = msg->getSOAPPart().getEnvelope();
      //envelope.addNamespaceDeclaration("xsi", "http://www.w3.org/2001/XMLSchema-instance");
      //envelope.addNamespaceDeclaration("xsd", "http://www.w3.org/2001/XMLSchema");
      xoap::SOAPName command = envelope.createName(cmd, "xdaq", XDAQ_NS_URI);
      xoap::SOAPBody body = envelope.getBody();
      xoap::SOAPElement bodyCommand = body.addBodyElement(command);
      //xoap::SOAPName attributeName = envelope.createName("Null");
      for (unsigned int i=0;i<attrib.size();++i)
        {
          xoap::SOAPName attributeName = envelope.createName(attrib[i].name_);
          bodyCommand.addAttribute(attributeName,attrib[i].value_);
        }

      return msg;
    }
  
  xoap::MessageReference MakeSOAPMessageReference(std::string cmd, Attribute_Vector attrib, Variable_Vector vars)
    {
      xoap::MessageReference msg = xoap::createMessage();
      xoap::SOAPEnvelope envelope = msg->getSOAPPart().getEnvelope();
      envelope.addNamespaceDeclaration("xsi", "http://www.w3.org/2001/XMLSchema-instance");
      envelope.addNamespaceDeclaration("xsd", "http://www.w3.org/2001/XMLSchema");
      xoap::SOAPName command = envelope.createName(cmd, "xdaq", XDAQ_NS_URI);
      xoap::SOAPBody body = envelope.getBody();
      xoap::SOAPElement bodyCommand = body.addBodyElement(command);

      for (unsigned int i=0;i<attrib.size();++i)
        {
          xoap::SOAPName attributeName = envelope.createName(attrib[i].name_,"xdaq", XDAQ_NS_URI);
          bodyCommand.addAttribute(attributeName,attrib[i].value_);
        }
      
      for (unsigned int i=0; i<vars.size(); ++i)
        {
          xoap::SOAPName subelementName = envelope.createName(vars[i].name_,"xdaq", XDAQ_NS_URI);
          xoap::SOAPElement subelementElement = bodyCommand.addChildElement(subelementName);
          xoap::SOAPName attributeName = envelope.createName("type","xsi","http://www.w3.org/2001/XMLSchema-instance");
          subelementElement.addAttribute(attributeName,"xsd:"+vars[i].type_);
          subelementElement.setTextContent(vars[i].payload_);
        }
      
      return msg;
    }
  
  xoap::MessageReference MakeSOAPMessageReference(std::string cmd, std::string filepath)
    {
      std::cout << "SOAP XML file path : " << filepath << std::endl;
      xoap::MessageReference msg = xoap::createMessage();
      xoap::SOAPPart soap = msg->getSOAPPart();
      xoap::SOAPEnvelope envelope = soap.getEnvelope();
      xoap::AttachmentPart * attachment;
      attachment = msg->createAttachmentPart();
      attachment->setContent(filepath);
      attachment->setContentId("SOAPTEST1");
      attachment->addMimeHeader("Content-Description", "This is a SOAP message with attachments");
      msg->addAttachmentPart(attachment);
      xoap::SOAPName command = envelope.createName(cmd, "xdaq", XDAQ_NS_URI);
      xoap::SOAPBody body= envelope.getBody();
      body.addBodyElement(command);
      return msg;
    }
  
  
  xoap::MessageReference
    createSimpleSOAPCommand(std::string const& command, std::string const& actionRequestorId="")
    {
      xoap::MessageReference msg = xoap::createMessage();
      xoap::SOAPEnvelope envelope = msg->getSOAPPart().getEnvelope();
      envelope.addNamespaceDeclaration("xsi", "http://www.w3.org/2001/XMLSchema-instance");
      envelope.addNamespaceDeclaration("xsd", "http://www.w3.org/2001/XMLSchema");
      xoap::SOAPName commandName = envelope.createName(command, "xdaq", XDAQ_NS_URI);
      xoap::SOAPBody body = envelope.getBody();
      xoap::SOAPElement commandElement = body.addBodyElement(commandName);
      if (actionRequestorId.size() > 0){
        xoap::SOAPName reqIdName = envelope.createName("actionRequestorId", "xdaq", "dummy");
        commandElement.addAttribute(reqIdName, actionRequestorId);
      }
      return(msg);
    }
  
  xoap::MessageReference
    createComplexSOAPCommand(std::string const& command, std::string const& actionRequestorId="",
                             std::string const& subElementName="", unsigned int const subElementValue=0)
    {
      xoap::MessageReference msg = createSimpleSOAPCommand(command, actionRequestorId);
      if (subElementName.size() > 0)
        {
          xoap::SOAPEnvelope envelope = msg->getSOAPPart().getEnvelope();
          xoap::SOAPBody body = envelope.getBody();
          xoap::SOAPElement commandElement = body.getChildElements().at(0);
          xoap::SOAPName runNumberName = envelope.createName(subElementName, "xdaq", "dummy");
          xoap::SOAPElement runNumberElement = commandElement.addChildElement(runNumberName);
          xoap::SOAPName typeName = envelope.createName("type", "xsi", "http://www.w3.org/2001/XMLSchema-instance");
          runNumberElement.addAttribute(typeName, "xsd:unsignedInt");
          runNumberElement.setTextContent(toolbox::toString("%d", subElementValue));
        }
      return(msg);
    }
  
  xoap::MessageReference
    createComplexSOAPCommand(std::string const& command, std::string const& actionRequestorId="",
                             std::string const& subElementName="", std::string const& subElementValue="")
    {
      xoap::MessageReference msg = createSimpleSOAPCommand(command, actionRequestorId);
      if (subElementName.size() > 0)
        {
          xoap::SOAPEnvelope envelope = msg->getSOAPPart().getEnvelope();
          xoap::SOAPBody body = envelope.getBody();
          xoap::SOAPElement commandElement = body.getChildElements().at(0);
          xoap::SOAPName runNumberName = envelope.createName(subElementName, "xdaq", "dummy");
          xoap::SOAPElement runNumberElement = commandElement.addChildElement(runNumberName);
          xoap::SOAPName typeName = envelope.createName("type", "xsi", "http://www.w3.org/2001/XMLSchema-instance");
          runNumberElement.addAttribute(typeName, "xsd:string");
          runNumberElement.setTextContent(subElementValue);
        }
      return(msg);
    }
  
  xoap::MessageReference
    createSimpleSOAPParameterGet(std::string const& className, std::string const& parName, std::string const& parType)
    {
      std::string const applicationNameSpace = toolbox::toString("urn:xdaq-application:%s", className.c_str());
      
      xoap::MessageReference msg = createSimpleSOAPCommand("ParameterGet", "");
      xoap::SOAPEnvelope envelope = msg->getSOAPPart().getEnvelope();
      envelope.addNamespaceDeclaration("soapenc", "http://schemas.xmlsoap.org/soap/encoding/");
      xoap::SOAPBody body = envelope.getBody();
      xoap::SOAPElement commandElement = body.getChildElements().at(0);

      xoap::SOAPName propertiesName = envelope.createName("properties", "p", applicationNameSpace);
      xoap::SOAPElement propertiesElement = commandElement.addChildElement(propertiesName);
      xoap::SOAPName propertiesTypeName = envelope.createName("type", "xsi", "http://www.w3.org/2001/XMLSchema-instance");
      propertiesElement.addAttribute(propertiesTypeName, "soapenc:Struct");

      xoap::SOAPName propertyName = envelope.createName(parName, "p", applicationNameSpace);
      xoap::SOAPElement propertyElement = propertiesElement.addChildElement(propertyName);
      xoap::SOAPName propertyTypeName = envelope.createName("type", "xsi", "http://www.w3.org/2001/XMLSchema-instance");
      propertyElement.addAttribute(propertyTypeName, parType);
      return(msg);
    }
  

  xoap::MessageReference
    postSOAPCommand(xoap::MessageReference cmd, xdaq::ApplicationDescriptor* destination)
    {
      try
        {  
          xoap::MessageReference const reply = app_->getApplicationContext()->postSOAP(cmd, *(app_->getApplicationDescriptor()), *destination);
          if (hasFault(reply))
            {
              std::string msg("");
              std::string const faultString = extractFaultString(reply);
              if (hasFaultDetail(reply))
                {
                  std::string const faultDetail = extractFaultDetail(reply);
                  msg = toolbox::toString("Received a SOAP fault as reply: '%s: %s'.", faultString.c_str(), faultDetail.c_str());
                }
              else
                {
                  msg = toolbox::toString("Received a SOAP fault as reply: '%s'.", faultString.c_str());
                }
              XCEPT_RAISE(xdaq::exception::Exception, msg);
            }
          else
            {
              return(reply);
            }
        }
      catch (xdaq::exception::Exception& err)
        {
          std::string const msg = toolbox::toString("Problem executing SOAP command: '%s'.", err.what());
          XCEPT_RETHROW(xdaq::exception::Exception, msg, err);
        }
    }
  
  
  
  bool
    hasFault(xoap::MessageReference const& msg)
  {
    return(msg->getSOAPPart().getEnvelope().getBody().hasFault());
  }
  
  bool
    hasFaultDetail(xoap::MessageReference const& msg)
  {
    bool res = false;
    if (hasFault(msg))
      {
        res = msg->getSOAPPart().getEnvelope().getBody().getFault().hasDetail();
      }
    return(res);
  }
  
  
  std::string
    extractFaultString(xoap::MessageReference const& msg)
    {
      std::string res("");
      if (hasFault(msg))
        {
          xoap::SOAPFault fault = msg->getSOAPPart().getEnvelope().getBody().getFault();
          res = fault.getFaultString();
        }
      return(res);
    }

  
  std::string
    extractFaultDetail(xoap::MessageReference const& msg)
    {
      std::string res("");
      if (hasFaultDetail(msg))
        {
          xoap::SOAPFault fault = msg->getSOAPPart().getEnvelope().getBody().getFault();
          res = fault.getDetail().getTextContent();
        }
      return(res);
    }
  
  
  
  std::string Send(xdaq::ApplicationDescriptor* d, std::string cmd) throw (xdaq::exception::Exception)
    {
      xoap::MessageReference msg;
      try
        {
          msg = MakeSOAPMessageReference(cmd);
          msg->getMimeHeaders()->setHeader("Content-Location", d->getURN());
          xoap::MessageReference reply = app_->getApplicationContext()->postSOAP(msg, *(app_->getApplicationDescriptor()), *d);
          std::string replyString = Receive(reply);
          return replyString;
        }
      catch (xdaq::exception::Exception& e)
        {
          std::cout << "SOAPCommander::Send: This application failed to send a SOAP message to "							
                    << d->getClassName() << " instance " << d->getInstance()
                    << " with command = " << cmd
                    << " re-throwing exception = " << xcept::stdformat_exception_history(e)<<std::endl;
          msg->writeTo(std::cout);
          std::cout<<std::endl;
          XCEPT_RETHROW(xdaq::exception::Exception,"Failed to send SOAP command.",e);
        }
    }
  
  xoap::MessageReference SendWithSOAPReply(xdaq::ApplicationDescriptor *d, std::string cmd) throw (xdaq::exception::Exception)
    {
      xoap::MessageReference msg;
      try
  	{
          msg = MakeSOAPMessageReference(cmd);
          msg->getMimeHeaders()->setHeader("Content-Location", d->getURN());
          xoap::MessageReference reply = app_->getApplicationContext()->postSOAP(msg, *(app_->getApplicationDescriptor()), *d);
          return reply;
  	}
      catch (xdaq::exception::Exception& e)
  	{
          std::cout << "This application failed to send a SOAP message to "
                    << d->getClassName() << " instance " << d->getInstance()
                    << " with command = " << cmd
                    << " re-throwing exception = " << xcept::stdformat_exception_history(e)<<std::endl;
          msg->writeTo(std::cout);
          std::cout<<std::endl;
          XCEPT_RETHROW(xdaq::exception::Exception,"Failed to send SOAP command.",e);
  	}
      
    }
  
  std::string Send(xdaq::ApplicationDescriptor *d, xoap::MessageReference msg) throw (xdaq::exception::Exception)
    {	
      try
        {
          xoap::MessageReference reply = app_->getApplicationContext()->postSOAP(msg, *(app_->getApplicationDescriptor()), *d);
          msg->getMimeHeaders()->setHeader("Content-Location", d->getURN());
          std::string replyString = Receive(reply);
          return replyString;
        }
      catch (xdaq::exception::Exception& e)
        {
          std::cout << "This application failed to send a SOAP message to "							
                    << d->getClassName() << " instance " << d->getInstance()
                    << " re-throwing exception = " << xcept::stdformat_exception_history(e);
          msg->writeTo(std::cout);
          std::cout<<std::endl;
          XCEPT_RETHROW(xdaq::exception::Exception,"Failed to send SOAP command.",e);
        }
    }
  
  xoap::MessageReference SendWithSOAPReply(xdaq::ApplicationDescriptor *d, xoap::MessageReference msg) throw (xdaq::exception::Exception)
    {
      try
  	{
          xoap::MessageReference reply = app_->getApplicationContext()->postSOAP(msg, *(app_->getApplicationDescriptor()), *d);
          msg->getMimeHeaders()->setHeader("Content-Location", d->getURN());
          return reply;
  	}
      catch (xdaq::exception::Exception& e)
  	{
          std::cout << "This application failed to send a SOAP message to "
                    << d->getClassName() << " instance " << d->getInstance()								
                    << " re-throwing exception = " << xcept::stdformat_exception_history(e);
          msg->writeTo(std::cout);
          std::cout<<std::endl;
          XCEPT_RETHROW(xdaq::exception::Exception,"Failed to send SOAP command.",e);
  	}
    }
  
  std::string Send(xdaq::ApplicationDescriptor* d, std::string cmd, Attribute_Vector attrib) throw (xdaq::exception::Exception)
    {
      xoap::MessageReference msg;
      try
        {
          msg = MakeSOAPMessageReference(cmd, attrib);
          msg->getMimeHeaders()->setHeader("Content-Location", d->getURN());
          xoap::MessageReference reply = app_->getApplicationContext()->postSOAP(msg, *(app_->getApplicationDescriptor()), *d);
          std::string replyString = Receive(reply);
          return replyString;
        }
      catch (xdaq::exception::Exception& e)
        {
          std::cout << "This application failed to send a SOAP message to "							
                    << d->getClassName() << " instance " << d->getInstance()
                    << " with command = " << cmd
                    << " re-throwing exception = " << xcept::stdformat_exception_history(e)<<std::endl;
          msg->writeTo(std::cout);
          std::cout<<std::endl;
          XCEPT_RETHROW(xdaq::exception::Exception,"Failed to send SOAP command.",e);
        }
      
    }
  
  xoap::MessageReference SendWithSOAPReply(xdaq::ApplicationDescriptor* d, std::string cmd, Attribute_Vector attrib) throw (xdaq::exception::Exception)
    {
      xoap::MessageReference msg;
      try
  	{
          msg = MakeSOAPMessageReference(cmd, attrib);
          msg->getMimeHeaders()->setHeader("Content-Location", d->getURN());
          xoap::MessageReference reply = app_->getApplicationContext()->postSOAP(msg, *(app_->getApplicationDescriptor()), *d);
          return reply;
  	}
      catch (xdaq::exception::Exception& e)
  	{
          std::cout << "This application failed to send a SOAP message to "
                    << d->getClassName() << " instance " << d->getInstance()
                    << " with command = " << cmd
                    << " re-throwing exception = " << xcept::stdformat_exception_history(e);
          msg->writeTo(std::cout);
          std::cout<<std::endl;
          XCEPT_RETHROW(xdaq::exception::Exception,"Failed to send SOAP command.",e);
  	}
      
    }

  std::string Send(xdaq::ApplicationDescriptor* d, std::string cmd, Attribute_Vector attrib, Variable_Vector vars) throw (xdaq::exception::Exception)
    {
      xoap::MessageReference msg;
      try
        {
          msg = MakeSOAPMessageReference(cmd, attrib, vars);
          msg->getMimeHeaders()->setHeader("Content-Location", d->getURN());
          xoap::MessageReference reply = app_->getApplicationContext()->postSOAP(msg, *(app_->getApplicationDescriptor()), *d);
          std::string replyString = Receive(reply);
          return replyString;
        }
      catch (xdaq::exception::Exception& e)
        {
          std::cout << "This application failed to send a SOAP message to "							
                    << d->getClassName() << " instance " << d->getInstance()
                    << " with command = " << cmd
                    << " re-throwing exception = " << xcept::stdformat_exception_history(e)<<std::endl;
          msg->writeTo(std::cout);
          std::cout<<std::endl;
          XCEPT_RETHROW(xdaq::exception::Exception,"Failed to send SOAP command.",e);
        }
      
    }
  
  xoap::MessageReference SendWithSOAPReply(xdaq::ApplicationDescriptor* d, std::string cmd, Attribute_Vector attrib, Variable_Vector vars) throw (xdaq::exception::Exception)
    {
      try
  	{
          xoap::MessageReference msg = MakeSOAPMessageReference(cmd, attrib, vars);
          msg->getMimeHeaders()->setHeader("Content-Location", d->getURN());
          xoap::MessageReference reply = app_->getApplicationContext()->postSOAP(msg, *(app_->getApplicationDescriptor()), *d);
          return reply;
  	}
      catch (xdaq::exception::Exception& e)
  	{
          std::cout << "This application failed to send a SOAP message to "
                    << d->getClassName() << " instance " << d->getInstance()
                    << " with command = " << cmd
                    << " re-throwing exception = " << xcept::stdformat_exception_history(e);
          XCEPT_RETHROW(xdaq::exception::Exception,"Failed to send SOAP command.",e);
  	}
      
    }

  
  std::string Send(xdaq::ApplicationDescriptor* d, std::string cmd, std::string filepath) throw (xdaq::exception::Exception)
    {
      try
        {
          std::cout << "SOAP XML file path : " << filepath << std::endl;
          xoap::MessageReference msg = xoap::createMessage();
          xoap::SOAPPart soap = msg->getSOAPPart();
          xoap::SOAPEnvelope envelope = soap.getEnvelope();
          xoap::AttachmentPart * attachment;
          attachment = msg->createAttachmentPart();
          attachment->setContent(filepath);
          attachment->setContentId("SOAPTEST1");
          attachment->addMimeHeader("Content-Description", "This is a SOAP message with attachments");
          msg->addAttachmentPart(attachment);
          xoap::SOAPName command = envelope.createName(cmd, "xdaq", XDAQ_NS_URI);
          xoap::SOAPBody body= envelope.getBody();
          body.addBodyElement(command);
#ifdef DEBUGMSG
          string mystring;
          msg->writeTo(mystring);
          std::cout << "SOAP Message : "<< mystring <<std::endl;
#endif
          msg->getMimeHeaders()->setHeader("Content-Location", d->getURN());
          xoap::MessageReference reply=app_->getApplicationContext()->postSOAP(msg, *(app_->getApplicationDescriptor()), *d);
          std::string replyString = Receive(reply);
          return replyString;
        }
      catch (xdaq::exception::Exception& e)
        {
          XCEPT_RETHROW(xdaq::exception::Exception,"Failed to send SOAP command.",e);
        }
    }
  
  std::string SendStatus(xdaq::ApplicationDescriptor* d, std::string msg) throw (xdaq::exception::Exception) 
    {	  
      std::string cmd = "StatusNotification";
      try
        {
          timeval tv;	    //keep track of when the message comes
          gettimeofday(&tv,NULL);
          
          Attribute_Vector parameters(3);
          parameters[0].name_="Description"; parameters[0].value_=msg;
          parameters[1].name_="Time"; parameters[1].value_=itoa(tv.tv_sec);
          parameters[2].name_="usec"; parameters[2].value_=itoa(tv.tv_usec);
          std::string replyString = Send(d, cmd, parameters);
          return replyString;
        }
      catch (xdaq::exception::Exception& e)
        {
          std::cout << "This application failed to send a SOAP error message to "							
                    << d->getClassName() << " instance " << d->getInstance()
                    << " with command = " << cmd
                    << " re-throwing exception = " << xcept::stdformat_exception_history(e)<<std::endl;
          XCEPT_RETHROW(xdaq::exception::Exception,"Failed to send SOAP command.",e);
        }
    }
  
 protected:
  xdaq::Application* app_;
  
};
#endif
