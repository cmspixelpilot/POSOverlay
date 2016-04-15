/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2009, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelAMC13Controller_h_
#define _PixelAMC13Controller_h_

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

#include "xdata/Boolean.h"
#include "xdata/Integer.h"
#include "xdata/String.h"

#include "PixelUtilities/PixeluTCAUtilities/include/Amc13Interface.h"
#include "PixelUtilities/PixeluTCAUtilities/include/Amc13Description.h"


struct Attribute { std::string name_; std::string value_; };
typedef std::vector<Attribute> Attribute_Vector;


class PixelAMC13Controller: public xdaq::Application  
{
	
	public:
	
	XDAQ_INSTANTIATOR();

	xdata::String cUri1;
	xdata::String cAddressT1;
	xdata::String cUri2;
	xdata::String cAddressT2;
	
	int bgoCommand;
	xdata::Boolean bgoRepeat;
	xdata::Integer bgoPrescale;
	xdata::Integer bgoBX;

	PixelAMC13Controller(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);
	
	xoap::MessageReference userCommand (xoap::MessageReference msg) throw (xoap::exception::Exception);	
};

#endif
