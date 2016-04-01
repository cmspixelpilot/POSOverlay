// $Id: SimpleSOAPSender.h,v 1.4 2008/07/18 15:26:44 gutleber Exp $

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2009, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _SimpleSOAPSender_h_
#define _SimpleSOAPSender_h_

#include "xdaq/Application.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/exception/Exception.h"

#include "xdaq/NamespaceURI.h"

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPConstants.h"
#include "xoap/SOAPBody.h"
#include "xoap/Method.h"
#include "xoap/AttachmentPart.h"

#include "xgi/Utils.h"
#include "xgi/Method.h"
#include "cgicc/CgiDefs.h"
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

#include "xdata/String.h"
#include "xdata/UnsignedLong.h"

// This is a simple example for sending a SOAP message. It can be used in combination
// with the SimpleSOAPReceiver application.

class SimpleSOAPSender: public xdaq::Application  
{
	
	public:
	
	XDAQ_INSTANTIATOR();
	
	SimpleSOAPSender(xdaq::ApplicationStub * s): xdaq::Application(s) 
	{	
		// A simple web control interface
		xgi::bind(this,&SimpleSOAPSender::Default, "Default");
		xgi::bind(this,&SimpleSOAPSender::sendMessage, "sendMessage");
		xgi::bind(this,&SimpleSOAPSender::sendMessageWithAttachments, "sendMessageWithAttachments");
		xgi::bind(this,&SimpleSOAPSender::sendMessage1_2, "sendMessage1_2");
		xgi::bind(this,&SimpleSOAPSender::sendMessageWithAttachments1_2, "sendMessageWithAttachments1_2");

	}
	
	void Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
	{
		*out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
		*out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
		*out << cgicc::title("Send SOAP Message") << std::endl;

//		xgi::Utils::getPageHeader
//			(out, 
//			"Simple SOAP Sender", 
//			getApplicationDescriptor()->getContextDescriptor()->getURL(),
//			getApplicationDescriptor()->getURN(),
//			"/daq/xgi/images/Application.jpg"
//			);

		std::string url = "/";
		url += getApplicationDescriptor()->getURN();
		url += "/sendMessage";	
		*out << cgicc::form().set("method","get").set("action", url).set("enctype","multipart/form-data") << std::endl;
		*out << cgicc::input().set("type", "submit").set("name", "send").set("value", "Send");
		*out << cgicc::p() << std::endl;
		*out << cgicc::form() << std::endl;

		std::string urlwa = "/";
                urlwa += getApplicationDescriptor()->getURN();
                urlwa += "/sendMessageWithAttachments";
                *out << cgicc::form().set("method","get").set("action", urlwa).set("enctype","multipart/form-data") << std::endl;
                *out << cgicc::input().set("type", "submit").set("name", "send").set("value", "SendWithAttachments");
                *out << cgicc::p() << std::endl;
                *out << cgicc::form() << std::endl;

		std::string url1_2 = "/";
                url1_2 += getApplicationDescriptor()->getURN();
                url1_2 += "/sendMessage1_2";
                *out << cgicc::form().set("method","get").set("action", url1_2).set("enctype","multipart/form-data") << std::endl;
                *out << cgicc::input().set("type", "submit").set("name", "send").set("value", "Send1_2");
                *out << cgicc::p() << std::endl;
                *out << cgicc::form() << std::endl;

                std::string urlwa1_2 = "/";
                urlwa1_2 += getApplicationDescriptor()->getURN();
                urlwa1_2 += "/sendMessageWithAttachments1_2";
                *out << cgicc::form().set("method","get").set("action", urlwa1_2).set("enctype","multipart/form-data") << std::endl;
                *out << cgicc::input().set("type", "submit").set("name", "send").set("value", "SendWithAttachments1_2");
                *out << cgicc::p() << std::endl;
                *out << cgicc::form() << std::endl;


		//xgi::Utils::getPageFooter(*out);	
	}
	void sendMessage(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
	{
	  	xoap::MessageReference msg = xoap::createMessage();
                xoap::SOAPPart soap = msg->getSOAPPart();
                xoap::SOAPEnvelope envelope = soap.getEnvelope();
                xoap::SOAPBody body = envelope.getBody();
                xoap::SOAPName command = envelope.createName("onMessage","xdaq", "urn:xdaq-soap:3.0");
                body.addBodyElement(command);

		try
		{	
			xdaq::ApplicationDescriptor * d = getApplicationContext()->getDefaultZone()->getApplicationDescriptor("SimpleSOAPReceiver", 0);
			xdaq::ApplicationDescriptor * o = this->getApplicationDescriptor();
			xoap::MessageReference reply = getApplicationContext()->postSOAP(msg, *o,  *d);
		} 
		catch (xdaq::exception::Exception& e)
		{
			XCEPT_RETHROW (xgi::exception::Exception, "Cannot send message", e);
		
		
		
		}
				
		this->Default(in,out);

	}

	void sendMessageWithAttachments(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
        {               
                xoap::MessageReference msg = xoap::createMessage();
                xoap::SOAPPart soap = msg->getSOAPPart();
                xoap::SOAPEnvelope envelope = soap.getEnvelope();
                xoap::SOAPBody body = envelope.getBody();
                xoap::SOAPName command = envelope.createName("onMessage","xdaq", "urn:xdaq-soap:3.0");
                body.addBodyElement(command);

		//
        	// Create attachment
		//
        	xoap::AttachmentPart * attachment = msg->createAttachmentPart();

       		char * buf = new char [256];
		
		// Need to initialize buffer, otherwise
		// SOAP leftovers may corrupt the message by
		// '--' mime boundary marks that appear in
		// uninitialized memory areas.
		//
		memset (buf, 'x', 256);
		
       		attachment->setContent(buf, 256, "content/unknown");

       		msg->addAttachmentPart(attachment);

       		//
       		// Write message to screen
		//
       		//string s;
       		//msg.writeTo(s);
       		//cout << s << endl;

       		delete buf;
                        
                try     
                {       
                        xdaq::ApplicationDescriptor * d = getApplicationContext()->getDefaultZone()->getApplicationDescriptor("SimpleSOAPReceiver", 0);
			xdaq::ApplicationDescriptor * o = this->getApplicationDescriptor();
                        xoap::MessageReference reply = getApplicationContext()->postSOAP(msg, *o, *d);
                }       
                catch (xdaq::exception::Exception& e)
                {       
                        XCEPT_RETHROW (xgi::exception::Exception, "Cannot send message", e);
                        
                        
                        
                }       
                                
                this->Default(in,out);
                        
        }  
	void sendMessage1_2(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
	{
		xoap::MessageFactory * factory = xoap::MessageFactory::getInstance(xoap::SOAPConstants::SOAP_1_2_PROTOCOL);
	  	xoap::MessageReference msg = factory->createMessage();
                xoap::SOAPPart soap = msg->getSOAPPart();
                xoap::SOAPEnvelope envelope = soap.getEnvelope();
                xoap::SOAPBody body = envelope.getBody();
                xoap::SOAPName command = envelope.createName("onMessage","xdaq", "urn:xdaq-soap:3.0");
                body.addBodyElement(command);

		try
		{	
			xdaq::ApplicationDescriptor * d = getApplicationContext()->getDefaultZone()->getApplicationDescriptor("SimpleSOAPReceiver", 0);
			xdaq::ApplicationDescriptor * o = this->getApplicationDescriptor();
			xoap::MessageReference reply = getApplicationContext()->postSOAP(msg, *o, *d);
		} 
		catch (xdaq::exception::Exception& e)
		{
			XCEPT_RETHROW (xgi::exception::Exception, "Cannot send message", e);
		
		
		
		}
				
		this->Default(in,out);

	}

	void sendMessageWithAttachments1_2(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
        {               
		xoap::MessageFactory * factory = xoap::MessageFactory::getInstance(xoap::SOAPConstants::SOAP_1_2_PROTOCOL);
	  	xoap::MessageReference msg = factory->createMessage();
                xoap::SOAPPart soap = msg->getSOAPPart();
                xoap::SOAPEnvelope envelope = soap.getEnvelope();
                xoap::SOAPBody body = envelope.getBody();
                xoap::SOAPName command = envelope.createName("onMessage","xdaq", "urn:xdaq-soap:3.0");
                body.addBodyElement(command);

		//
        	// Create attachment
		//
        	xoap::AttachmentPart * attachment = msg->createAttachmentPart();

       		char * buf = new char [256];
		
		// Need to initialize buffer, otherwise
		// SOAP leftovers may corrupt the message by
		// '--' mime boundary marks that appear in
		// uninitialized memory areas.
		//
		memset (buf, 'x', 256);
		
       		attachment->setContent(buf, 256, "content/unknown");

       		msg->addAttachmentPart(attachment);

       		//
       		// Write message to screen
		//
       		//string s;
       		//msg.writeTo(s);
       		//cout << s << endl;

       		delete buf;
                        
                try     
                {       
                        xdaq::ApplicationDescriptor * d = getApplicationContext()->getDefaultZone()->getApplicationDescriptor("SimpleSOAPReceiver", 0);
			xdaq::ApplicationDescriptor * o = this->getApplicationDescriptor();
                        xoap::MessageReference reply = getApplicationContext()->postSOAP(msg, *o, *d);
                }       
                catch (xdaq::exception::Exception& e)
                {       
                        XCEPT_RETHROW (xgi::exception::Exception, "Cannot send message", e);
                        
                        
                        
                }       
                                
                this->Default(in,out);
                        
        }  

	
	
};

#endif
	
