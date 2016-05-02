#include "PixelAMC13Controller.h"

XDAQ_INSTANTIATOR_IMPL(PixelAMC13Controller)

PixelAMC13Controller::PixelAMC13Controller(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception) :xdaq::Application(s) 
{
  xoap::bind(this, &PixelAMC13Controller::userCommand, "userCommand", XDAQ_NS_URI );
  xoap::bind(this, &PixelAMC13Controller::Reset, "Reset", XDAQ_NS_URI );
  xoap::bind(this, &PixelAMC13Controller::Enable, "Enable", XDAQ_NS_URI );
  xoap::bind(this, &PixelAMC13Controller::Stop, "Stop", XDAQ_NS_URI );
  xoap::bind(this, &PixelAMC13Controller::Suspend, "Suspend", XDAQ_NS_URI );  
 
  getApplicationInfoSpace()->fireItemAvailable("Uri1", &cUri1);
  getApplicationInfoSpace()->fireItemAvailable("AddressT1", &cAddressT1);
  getApplicationInfoSpace()->fireItemAvailable("Uri2", &cUri2);
  getApplicationInfoSpace()->fireItemAvailable("AddressT2", &cAddressT2);
  getApplicationInfoSpace()->fireItemAvailable("AMCMask", &amcMaskStr);
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
      
  for (unsigned int i = 0;i < parameters.size(); ++i)
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
  
  amc13Description = new Amc13Description();
  amc13Description->setAMCMask(parseAMCMask(amcMaskStr));

  int calsyncCom = 0x2c;
  int resetROCCom = 0x1c;
  int resetTBMCom = 0x14;
  amc13Description->addBGO(calsyncCom, bgoRepeat, bgoPrescale, bgoBX);
  amc13Description->addBGO(resetROCCom, bgoRepeat, bgoPrescale, bgoBX);
  amc13Description->addBGO(resetTBMCom, bgoRepeat, bgoPrescale, bgoBX);  
  
  amc13Interface = new Amc13Interface(cUri1, cAddressT1, cUri2, cAddressT2);
  amc13Interface->setAmc13Description(amc13Description);
  amc13Interface->ConfigureAmc13();

  if (parameters[1].value_ == "CalSync") {
    amc13Interface->DisableBGO(1);
    amc13Interface->DisableBGO(2);
  }
  if (parameters[1].value_ == "ResetROC") {
    amc13Interface->DisableBGO(0);
    amc13Interface->DisableBGO(2);
  }
  if (parameters[1].value_ == "ResetTBM") {
    amc13Interface->DisableBGO(0);
    amc13Interface->DisableBGO(1);
  }

  amc13Interface->FireBGO();

  xoap::MessageReference reply = xoap::createMessage();
  xoap::SOAPEnvelope replyEnvelope = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName responseName = replyEnvelope.createName( "userTTCciControlResponse", "xdaq", XDAQ_NS_URI);
  xoap::SOAPBodyElement e = replyEnvelope.getBody().addBodyElement ( responseName );
  return reply;
}

xoap::MessageReference PixelAMC13Controller::Reset (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  xoap::MessageReference reply = xoap::createMessage();
  xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName responseName = envelope.createName( "TTCciControlFSMReset", "xdaq", XDAQ_NS_URI);
  xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
  return reply;
}

xoap::MessageReference PixelAMC13Controller::Enable (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  xoap::MessageReference reply = xoap::createMessage();
  xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName responseName = envelope.createName( "enableResponse", "xdaq", XDAQ_NS_URI);
  xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
  return reply;
}

xoap::MessageReference PixelAMC13Controller::Stop (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  xoap::MessageReference reply = xoap::createMessage();
  xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName responseName = envelope.createName( "stopResponse", "xdaq", XDAQ_NS_URI);
  xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
  return reply;
}

xoap::MessageReference PixelAMC13Controller::Suspend (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  xoap::MessageReference reply = xoap::createMessage();
  xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName responseName = envelope.createName( "suspendResponse", "xdaq", XDAQ_NS_URI);
  xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
  return reply;
}

xoap::MessageReference PixelAMC13Controller::Configuration (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  xoap::MessageReference reply = xoap::createMessage();
  xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName responseName = envelope.createName( "Fault", "xdaq", XDAQ_NS_URI);
  xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
  return reply;
}

std::vector<int> PixelAMC13Controller::parseAMCMask(xdata::String maskStr) {
  std::vector<int> amcVec;

  std::string cList = maskStr;
  std::string ctoken;
  std::stringstream cStr(cList);

  while (std::getline(cStr, ctoken, ',')) {
    amcVec.push_back(convertAnyInt( ctoken.c_str() ));
  }
  return amcVec;
}
