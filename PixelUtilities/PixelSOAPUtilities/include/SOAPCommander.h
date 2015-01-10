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
		xoap::SOAPName command = envelope.createName(cmd, "xdaq", XDAQ_NS_URI);
		xoap::SOAPBody body= envelope.getBody();
		body.addBodyElement(command);
		return msg;
	}

	xoap::MessageReference MakeSOAPMessageReference(std::string cmd, Attribute_Vector attrib)
	{
		xoap::MessageReference msg = xoap::createMessage();
		xoap::SOAPEnvelope envelope = msg->getSOAPPart().getEnvelope();
		xoap::SOAPName command = envelope.createName(cmd, "xdaq", XDAQ_NS_URI);
		xoap::SOAPBody body = envelope.getBody();
		xoap::SOAPElement bodyCommand = body.addBodyElement(command);
		xoap::SOAPName attributeName = envelope.createName("Null");
		for (unsigned int i=0;i<attrib.size();++i)
		{
			attributeName = envelope.createName(attrib[i].name_);
			bodyCommand.addAttribute(attributeName,attrib[i].value_);
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
		  XCEPT_RETHROW(xdaq::exception::Exception,"Failed to send SOAP command.",e);
		}
	}

	xoap::MessageReference SendWithSOAPReply(xdaq::ApplicationDescriptor *d, std::string cmd) throw (xdaq::exception::Exception)
	{
  	try
  	{
  		xoap::MessageReference msg = MakeSOAPMessageReference(cmd);
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
			XCEPT_RETHROW(xdaq::exception::Exception,"Failed to send SOAP command.",e);
		}
		
	}

	xoap::MessageReference SendWithSOAPReply(xdaq::ApplicationDescriptor* d, std::string cmd, Attribute_Vector attrib) throw (xdaq::exception::Exception)
  {
  	try
  	{
  		xoap::MessageReference msg = MakeSOAPMessageReference(cmd, attrib);
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
