// $Id: SimpleSOAPReceiver.h,v 1.7 2008/07/18 15:26:44 gutleber Exp $

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2009, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _SimpleSOAPReceiver_h_
#define _SimpleSOAPReceiver_h_

#include "xdaq/Application.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/exception/Exception.h"

#include "xdaq/NamespaceURI.h"

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPBody.h"
#include "xoap/Method.h"

struct Attribute { std::string name_; std::string value_; };
typedef std::vector<Attribute> Attribute_Vector;


class SimpleSOAPReceiver: public xdaq::Application  
{
	
	public:
	
	XDAQ_INSTANTIATOR();
	
	SimpleSOAPReceiver(xdaq::ApplicationStub * s): xdaq::Application(s) 
	{	
		//
		// Bind SOAP callback
		//
          xoap::bind(this, &SimpleSOAPReceiver::userCommand, "userCommand", XDAQ_NS_URI );
		
	
	}
	
	
	//
	// SOAP Callback  
	//
	xoap::MessageReference userCommand (xoap::MessageReference msg) throw (xoap::exception::Exception)
	{
                printf("HELLO DAN\n");

                Attribute_Vector parameters(2);
                parameters[0].name_ = "xdaq:CommandPar";
                parameters[1].name_ = "xdaq:sequence_name";

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
                        std::cout<<" Complaint ";
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

                printf("command was %s\n", commandName.c_str());
                printf("parameters are:\n");
                printf("0: %s\n", parameters[0].value_.c_str());
                printf("1: %s\n", parameters[1].value_.c_str());

                {
		xoap::MessageReference reply = xoap::createMessage();
		xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
		xoap::SOAPName responseName = envelope.createName( "userTTCciControlResponse", "xdaq", XDAQ_NS_URI);
		xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
		return reply;
                }
		
	}


	
	
};

#endif
