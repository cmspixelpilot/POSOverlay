#include "PixelAMC13Controller/include/PixelAMC13Controller.h"

struct Attribute { std::string name_; std::string value_; };
typedef std::vector<Attribute> Attribute_Vector;

XDAQ_INSTANTIATOR_IMPL(PixelAMC13Controller)

// TODO: use SoapCommander

namespace {
  const bool PRINT = true;
}

PixelAMC13Controller::PixelAMC13Controller(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception)
  : xdaq::Application(s),
    amc13(0)
{
  xoap::bind(this, &PixelAMC13Controller::Reset, "reset", XDAQ_NS_URI);
  xoap::bind(this, &PixelAMC13Controller::Configuration, "ParameterSet", XDAQ_NS_URI);
  xoap::bind(this, &PixelAMC13Controller::Configure, "configure", XDAQ_NS_URI);
  xoap::bind(this, &PixelAMC13Controller::userCommand, "userCommand", XDAQ_NS_URI);
  xoap::bind(this, &PixelAMC13Controller::Enable, "enable", XDAQ_NS_URI);
  xoap::bind(this, &PixelAMC13Controller::Stop, "stop", XDAQ_NS_URI);
  xoap::bind(this, &PixelAMC13Controller::Suspend, "suspend", XDAQ_NS_URI);  
 
  getApplicationInfoSpace()->fireItemAvailable("Uri1", &uri1);
  getApplicationInfoSpace()->fireItemAvailable("Uri2", &uri2);
  getApplicationInfoSpace()->fireItemAvailable("AddressT1", &addressT1);
  getApplicationInfoSpace()->fireItemAvailable("AddressT2", &addressT2);
  getApplicationInfoSpace()->fireItemAvailable("Mask", &mask);
  getApplicationInfoSpace()->fireItemAvailable("CalBX", &calBX);
}

xoap::MessageReference PixelAMC13Controller::Reset(xoap::MessageReference msg) throw (xoap::exception::Exception) {
  if (PRINT) std::cout << "PixelAMC13Controller::Reset" << std::endl;

  amc13 = new PixelAMC13Interface(uri1, addressT1,
                                  uri2, addressT2);
  amc13->SetMask(mask);
  amc13->SetDebugPrints(PRINT);
  amc13->SetCalBX(calBX);
  //amc13->SetL1ABurstDelay(10);

  amc13->Configure();

  xoap::MessageReference reply = xoap::createMessage();
  xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName responseName = envelope.createName( "TTCciControlFSMReset", "xdaq", XDAQ_NS_URI);
  xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
  return reply;
}

xoap::MessageReference PixelAMC13Controller::Configuration (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  if (PRINT) std::cout << "PixelAMC13Controller::Configuration(IMPLEMENT ME)" << std::endl;
  xoap::MessageReference reply = xoap::createMessage();
  xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName responseName = envelope.createName("ParameterSetResponse", "xdaq", XDAQ_NS_URI);
  xoap::SOAPBodyElement e = envelope.getBody().addBodyElement (responseName);
  return reply;
}

xoap::MessageReference PixelAMC13Controller::Configure (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  if (PRINT) std::cout << "PixelAMC13Controller::Configure(MAGIC HAPPENS IN RESET)" << std::endl;
  xoap::MessageReference reply = xoap::createMessage();
  xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName responseName = envelope.createName("configureResponse", "xdaq", XDAQ_NS_URI);
  xoap::SOAPBodyElement e = envelope.getBody().addBodyElement (responseName);
  return reply;
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

  for (unsigned int i = 0;i < parameters.size(); ++i) {
    name=envelope.createName(parameters[i].name_);

    try {
      parameters[i].value_ = command.getAttributeValue(name);
      if (parameters[i].value_ == "" && parameters[i].name_ != "")
        std::cout <<" Complaint "
                  <<" : Parameter "<<parameters[i].name_
                  <<" ("<<i <<") does not exist in the list of incoming parameters!"
                  <<"It could also be because you passed an empty string"<<std::endl;
    }
    catch (xoap::exception::Exception& e) {
      std::cout<<"Parameter "<<parameters[i].name_<<" does not exist in the list of incoming parameters!"<<std::endl;
      XCEPT_RETHROW(xoap::exception::Exception,"Looking for parameter that does not exist!",e);
    }
  }

  if (PRINT) std::cout << "PixelAMC13Controller::userCommand(" << commandName << ", " << parameters[0].value_ << ", " << parameters[1].value_ << ")" << std::endl;

  if (parameters[1].value_ == "CalSync")
    amc13->CalSync();
  else if (parameters[1].value_ == "LevelOne")
    amc13->LevelOne();
  else if (parameters[1].value_ == "ResetROC")
    amc13->ResetROC();
  else if (parameters[1].value_ == "ResetTBM")
    amc13->ResetTBM();
  else
    XCEPT_RAISE(xoap::exception::Exception, "Don't know anything about command " + parameters[1].value_);
  
  xoap::MessageReference reply = xoap::createMessage();
  xoap::SOAPEnvelope replyEnvelope = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName responseName = replyEnvelope.createName( "userTTCciControlResponse", "xdaq", XDAQ_NS_URI);
  xoap::SOAPBodyElement e = replyEnvelope.getBody().addBodyElement ( responseName );
  return reply;
}

xoap::MessageReference PixelAMC13Controller::Enable(xoap::MessageReference msg) throw (xoap::exception::Exception) {
  if (PRINT) std::cout << "PixelAMC13Controller::Enable(IMPLEMENT ME)" << std::endl;
  xoap::MessageReference reply = xoap::createMessage();
  xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName responseName = envelope.createName( "enableResponse", "xdaq", XDAQ_NS_URI);
  xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
  return reply;
}

xoap::MessageReference PixelAMC13Controller::Stop(xoap::MessageReference msg) throw (xoap::exception::Exception) {
  if (PRINT) std::cout << "PixelAMC13Controller::Stop(IMPLEMENT ME)" << std::endl;
  xoap::MessageReference reply = xoap::createMessage();
  xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName responseName = envelope.createName( "stopResponse", "xdaq", XDAQ_NS_URI);
  xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
  return reply;
}

xoap::MessageReference PixelAMC13Controller::Suspend (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  if (PRINT) std::cout << "PixelAMC13Controller::Suspend(IMPLEMENT ME)" << std::endl;
  xoap::MessageReference reply = xoap::createMessage();
  xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName responseName = envelope.createName( "suspendResponse", "xdaq", XDAQ_NS_URI);
  xoap::SOAPBodyElement e = envelope.getBody().addBodyElement ( responseName );
  return reply;
}
