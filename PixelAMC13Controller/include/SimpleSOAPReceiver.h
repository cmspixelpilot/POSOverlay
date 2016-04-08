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

class SimpleSOAPReceiver: public xdaq::Application  
{
	
	public:
	
	XDAQ_INSTANTIATOR();
	
	SimpleSOAPReceiver(xdaq::ApplicationStub * s): xdaq::Application(s) 
	{	
		//
		// Bind SOAP callback
		//
		xoap::bind(this, &SimpleSOAPReceiver::onMessage, "onMessage", XDAQ_NS_URI );
		
	
	}
	
	
	//
	// SOAP Callback  
	//
	xoap::MessageReference onMessage (xoap::MessageReference msg) throw (xoap::exception::Exception)
	{
                printf("HELLO DAN\n");

		//XCEPT_DECLARE(xoap::exception::Exception, e1, "andrea e' innamorato di clara gaspar");
		//XCEPT_DECLARE_NESTED(xoap::exception::Exception, q, "quante volte viene lei" , e1);
		//XCEPT_RETHROW(xoap::exception::Exception, "pirlotto guarda il mio fault se e' giusto", q);

		// reply to caller
		
		xoap::MessageReference reply = xoap::createMessage();
		xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
		xoap::SOAPName responseName = envelope.createName( "onMessageResponse", "xdaq", XDAQ_NS_URI);
		xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
		return reply;
		
	}


	
	
};

#endif
