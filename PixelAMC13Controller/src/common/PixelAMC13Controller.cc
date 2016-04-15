/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2009, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelAMC13Controller.h"

XDAQ_INSTANTIATOR_IMPL(PixelAMC13Controller)

PixelAMC13Controller::PixelAMC13Controller(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception) :xdaq::Application(s) 
{
  xoap::bind(this, &PixelAMC13Controller::userCommand, "userCommand", XDAQ_NS_URI );
  
  getApplicationInfoSpace()->fireItemAvailable("Uri1", &cUri1);
  getApplicationInfoSpace()->fireItemAvailable("AddressT1", &cAddressT1);
  getApplicationInfoSpace()->fireItemAvailable("Uri2", &cUri2);
  getApplicationInfoSpace()->fireItemAvailable("AddressT2", &cAddressT2);
  getApplicationInfoSpace()->fireItemAvailable("BGORepeat", &bgoRepeat);
  getApplicationInfoSpace()->fireItemAvailable("BGOPrescale", &bgoPrescale);
  getApplicationInfoSpace()->fireItemAvailable("BGOBX", &bgoBX);
}

xoap::MessageReference PixelAMC13Controller::userCommand (xoap::MessageReference msg) throw (xoap::exception::Exception) {
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

  printf("Command was %s\n", commandName.c_str());
  printf("Parameters are:\n");
  printf("0: %s\n", parameters[0].value_.c_str());
  printf("1: %s\n", parameters[1].value_.c_str());

  bgoCommand = 0;
  if (parameters[1].value_ == "CalSync")
    bgoCommand = 44;
  if (parameters[1].value_ == "ResetROC")
    bgoCommand = 28;
  if (parameters[1].value_ == "ResetTBM")
    bgoCommand = 20;

  Amc13Description* fAmc13 = new Amc13Description();
  fAmc13->setAMCMask({3});
  fAmc13->addBGO(bgoCommand, bgoRepeat, bgoPrescale, bgoBX);

  Amc13Interface* fAmc13Interface = new Amc13Interface(cUri1, cAddressT1, cUri2, cAddressT2);

  fAmc13Interface->setAmc13Description(fAmc13);
  fAmc13Interface->ConfigureAmc13();  
  fAmc13Interface->FireBGO();

  {
    xoap::MessageReference reply = xoap::createMessage();
    xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
    xoap::SOAPName responseName = envelope.createName( "userTTCciControlResponse", "xdaq", XDAQ_NS_URI);
    xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
    return reply;
  }	       
}
