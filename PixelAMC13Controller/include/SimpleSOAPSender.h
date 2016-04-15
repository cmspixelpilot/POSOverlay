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

struct Attribute { std::string name_; std::string value_; };
typedef std::vector<Attribute> Attribute_Vector;


class SimpleSOAPSender: public xdaq::Application
{
	
	public:
	
	XDAQ_INSTANTIATOR();
	
        SimpleSOAPSender(xdaq::ApplicationStub * s): xdaq::Application(s)
	{	
		// A simple web control interface
		xgi::bind(this,&SimpleSOAPSender::Default, "Default");
		xgi::bind(this,&SimpleSOAPSender::sendCalSync, "sendCalSync");
		xgi::bind(this,&SimpleSOAPSender::sendResetROC, "sendResetROC");
		xgi::bind(this,&SimpleSOAPSender::sendResetTBM, "sendResetTBM");
	}
	
	void Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
	{
		*out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
		*out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
		*out << cgicc::title("Send SOAP Message") << std::endl;

		std::string url = "/";
		url += getApplicationDescriptor()->getURN();
		url += "/sendCalSync";	
		*out << cgicc::form().set("method","get").set("action", url).set("enctype","multipart/form-data") << std::endl;
		*out << cgicc::input().set("type", "submit").set("name", "send").set("value", "CalSync");
		*out << cgicc::p() << std::endl;
		*out << cgicc::form() << std::endl;

		std::string urlwa = "/";
                urlwa += getApplicationDescriptor()->getURN();
                urlwa += "/sendResetROC";
                *out << cgicc::form().set("method","get").set("action", urlwa).set("enctype","multipart/form-data") << std::endl;
                *out << cgicc::input().set("type", "submit").set("name", "send").set("value", "ResetROC");
                *out << cgicc::p() << std::endl;
                *out << cgicc::form() << std::endl;

		std::string urltbm = "/";
                urltbm += getApplicationDescriptor()->getURN();
                urltbm += "/sendResetTBM";
                *out << cgicc::form().set("method","get").set("action", urltbm).set("enctype","multipart/form-data") << std::endl;
                *out << cgicc::input().set("type", "submit").set("name", "send").set("value", "ResetTBM");
                *out << cgicc::p() << std::endl;
                *out << cgicc::form() << std::endl;
	}

	void sendSomething(xgi::Input * in, xgi::Output * out, std::string something) throw (xgi::exception::Exception)
	{
          printf("something is %s\n", something.c_str());

	  	xoap::MessageReference msg = xoap::createMessage();
                xoap::SOAPPart soap = msg->getSOAPPart();
                xoap::SOAPEnvelope envelope = soap.getEnvelope();
                xoap::SOAPBody body = envelope.getBody();
                xoap::SOAPName command = envelope.createName("userCommand","xdaq", "urn:xdaq-soap:3.0");
                xoap::SOAPElement bodyCommand = body.addBodyElement(command);

                Attribute_Vector attrib(2);
                attrib[0].name_="xdaq:CommandPar";
                attrib[0].value_="Execute Sequence";
                attrib[1].name_="xdaq:sequence_name";
                attrib[1].value_=something;

                for (unsigned int i=0;i<attrib.size();++i) {
                  xoap::SOAPName attributeName = envelope.createName(attrib[i].name_);
                  bodyCommand.addAttribute(attributeName,attrib[i].value_);
                }

		try
		{	
			xdaq::ApplicationDescriptor * d = getApplicationContext()->getDefaultZone()->getApplicationDescriptor("PixelAMC13Controller", 0);
			xdaq::ApplicationDescriptor * o = this->getApplicationDescriptor();
			xoap::MessageReference reply = getApplicationContext()->postSOAP(msg, *o,  *d);
                        std::vector<xoap::SOAPElement> bodyList = reply->getSOAPPart().getEnvelope().getBody().getChildElements();
                        std::string command = bodyList[0].getElementName().getLocalName();
                        printf("command response for %s was %s\n", something.c_str(), command.c_str());
		} 
		catch (xdaq::exception::Exception& e)
		{
			XCEPT_RETHROW (xgi::exception::Exception, "Cannot send message", e);
		}
				
		this->Default(in,out);

	}

        void sendCalSync(xgi::Input * in, xgi::Output * out) throw (xgi::exception::Exception) {
          sendSomething(in, out, "CalSync");
        }

        void sendResetROC(xgi::Input * in, xgi::Output * out) throw (xgi::exception::Exception) {
          sendSomething(in, out, "ResetROC");
        }

        void sendResetTBM(xgi::Input * in, xgi::Output * out) throw (xgi::exception::Exception) {
          sendSomething(in, out, "ResetTBM");
        }

};

#endif
	
