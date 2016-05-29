#include "PixelAMC13Controller/include/PixelAMC13Controller.h"

XDAQ_INSTANTIATOR_IMPL(PixelAMC13Controller)

namespace {
  const bool PRINT = true;

  // stolen from stack overflow yay
  std::string commaify(unsigned value) {
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << value;
    return ss.str();
  }

  // another code theft without attribution
  void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
      subject.replace(pos, search.length(), replace);
      pos += replace.length();
    }
  }
}

PixelAMC13Controller::PixelAMC13Controller(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception)
  : xdaq::Application(s),
    SOAPCommander(this),
    amc13(0)
{
  xgi::bind(this, &PixelAMC13Controller::Default, "Default");
  xgi::bind(this, &PixelAMC13Controller::StateMachineXgiHandler, "StateMachineXgiHandler");
  xgi::bind(this, &PixelAMC13Controller::AllAMC13Tables, "AllAMC13Tables");

  xoap::bind(this, &PixelAMC13Controller::Reset, "reset", XDAQ_NS_URI);
  xoap::bind(this, &PixelAMC13Controller::Configuration, "ParameterSet", XDAQ_NS_URI);
  xoap::bind(this, &PixelAMC13Controller::Configure, "configure", XDAQ_NS_URI);
  xoap::bind(this, &PixelAMC13Controller::userCommand, "userCommand", XDAQ_NS_URI);
  xoap::bind(this, &PixelAMC13Controller::Enable, "enable", XDAQ_NS_URI);
  xoap::bind(this, &PixelAMC13Controller::Stop, "stop", XDAQ_NS_URI);
  xoap::bind(this, &PixelAMC13Controller::Suspend, "suspend", XDAQ_NS_URI);  
 
  getApplicationInfoSpace()->fireItemAvailable("DoNothing", &doNothing);
  getApplicationInfoSpace()->fireItemAvailable("Uri1", &uri1);
  getApplicationInfoSpace()->fireItemAvailable("Uri2", &uri2);
  getApplicationInfoSpace()->fireItemAvailable("AddressT1", &addressT1);
  getApplicationInfoSpace()->fireItemAvailable("AddressT2", &addressT2);
  getApplicationInfoSpace()->fireItemAvailable("Mask", &mask);
  getApplicationInfoSpace()->fireItemAvailable("CalBX", &calBX);
}

void PixelAMC13Controller::InitAMC13() {
  amc13 = new PixelAMC13Interface(uri1, addressT1, uri2, addressT2);
  amc13->SetMask(mask);
  amc13->SetDebugPrints(PRINT);
  amc13->SetCalBX(calBX);
  //amc13->SetL1ABurstDelay(10);
}

void PixelAMC13Controller::Default(xgi::Input* in, xgi::Output* out ) throw (xgi::exception::Exception) {
  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  *out << cgicc::html().set("lang", "en").set("dir", "ltr") << std::endl;
  *out << cgicc::title("Pixel AMC13 Controller") << std::endl;

  std::vector<std::string> sends = {"reset", "CalSync", "LevelOne", "ResetROC", "ResetTBM", "ResetCounters" };

  if (!amc13)
    *out << "not yet initialized!<br>";

  *out << "<h3>Commands:</h3>\n"
       << "<table><tr>\n";
  for (std::vector<std::string>::const_iterator it = sends.begin(), ite = sends.end(); it != ite; ++it) {
    *out << "<td>";
    std::string url = "/";
    url += getApplicationDescriptor()->getURN();
    url += "/StateMachineXgiHandler";
    *out << cgicc::form().set("method","get").set("action", url).set("enctype","multipart/form-data") << std::endl;
    *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", *it);
    *out << cgicc::p() << std::endl;
    *out << cgicc::form() << std::endl;
    *out << "</td>\n";
    if (!amc13) break;
  }
  *out << "</tr></table><br>\n";

  if (!amc13) return;
  
  //*out << "<table><tr>\n"; 
  *out << "L1A count: "
       << commaify(amc13->GetL1ACount()) << "<br>\n";

  *out << "L1A rate: "
       << commaify(amc13->GetL1ARate()) << " Hz<br>\n";

  *out << "CalSync count: "
       << commaify(amc13->GetCalSyncCount()) << "<br>\n";

  *out << "LevelOne count: "
       << commaify(amc13->GetLevelOneCount()) << "<br>\n";

  *out << "ResetROC count: "
       << commaify(amc13->GetResetROCCount()) << "<br>\n";

  *out << "ResetTBM count: "
       << commaify(amc13->GetResetTBMCount()) << "<br>\n";

  *out << "Input clock frequency measurement: "
       << commaify(amc13->GetClockFreq())
       << " &plusmn; 50 Hz<br>\n";


  amc13->Get()->getStatus()->SetHTML();
  *out << "T1 Firmware Version: "
       << "0x" << std::hex << amc13->Get()->read(amc13::AMC13Simple::T1, "STATUS.FIRMWARE_VERS") << std::dec << "<br>\n";
  *out << "T2 Firmware Version: "
       << "0x" << std::hex << amc13->Get()->read(amc13::AMC13Simple::T2, "STATUS.FIRMWARE_VERS") << std::dec << "<br>\n";
  *out << "TTCLOOP: "
       << amc13->Get()->read(amc13::AMC13Simple::T1, "CONF.DIAG.FAKE_TTC_ENABLE") << "<br>\n";
  *out << "FAKE: "
       << amc13->Get()->read(amc13::AMC13Simple::T1, "CONF.LOCAL_TRIG.FAKE_DATA_ENABLE") << "<br>\n";

  std::stringstream ss;
  //*out << amc13->Get()->getStatus()->ReportHeader() << "\n";
  //*out << amc13->Get()->getStatus()->ReportStyle() << "\n";
  
  amc13->Get()->getStatus()->Report(99, ss, "TTC_BGO");

  //amc13->Get()->getStatus()->Report(99, ss, "TTC_History");
  //amc13->Get()->getStatus()->Report(99, ss, "TTC_History_conf");

  amc13->Get()->getStatus()->Report(99, ss, "Temps_Voltages");

  *out << ss.str();
  amc13->Get()->getStatus()->UnsetHTML();
  std::string urlAllAMC13Tables = "/";
  urlAllAMC13Tables += getApplicationDescriptor()->getURN();
  urlAllAMC13Tables += "/AllAMC13Tables";
  *out << "<a href=\"" << urlAllAMC13Tables << "\"><h3>All AMC13 status tables</h3></a><br>\n";
}

void PixelAMC13Controller::StateMachineXgiHandler(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception) {
  cgicc::Cgicc cgi(in);

  Attribute_Vector attrib(2);
  attrib[0].name_="xdaq:CommandPar";
  attrib[0].value_="Execute Sequence";
  attrib[1].name_="xdaq:sequence_name";
  attrib[1].value_=cgi.getElement("StateInput")->getValue();

  if (!amc13 && attrib[1].value_ != "reset") {
    *out << "<h1>won't do anything before reset</h1>\n";
    this->Default(in, out);
    return;
  }

  if (attrib[1].value_ == "reset") {
    xoap::MessageReference msg = MakeSOAPMessageReference("reset");
    Reset(msg);
  }
  else {
    xoap::MessageReference msg = MakeSOAPMessageReference("userCommand", attrib);
    userCommand(msg);
  }

  this->Default(in, out);
}

void PixelAMC13Controller::AllAMC13Tables(xgi::Input* in, xgi::Output* out ) throw (xgi::exception::Exception) {
  if (!amc13) {
    *out << "<h1>won't do anything before reset</h1>\n";
    return;
  }

  *out << "<h3>Dump of AMC13 tables:</h3>\n";
  std::stringstream ss;
  amc13->Get()->getStatus()->SetHTML();
  try {
    amc13->Get()->getStatus()->Report(99, ss, "");
  } catch (std::exception e) {
    *out << "<br><br><h1>SOME PROBLEM WITH STATUS</h1>\n";
  }
  for (int i = 0; i < amc13->Get()->getStatus()->GetTableColumns("0_Board").size(); ++i) {
    *out << amc13->Get()->getStatus()->GetTableColumns("0_Board")[i] << "\n";
  }
  *out << amc13->Get()->getStatus()->GetCell("0_Board", "INFO", "T1_VER")->Print(-1, true) << "\n";
  *out << ss.str();
  amc13->Get()->getStatus()->UnsetHTML(); 
}

xoap::MessageReference PixelAMC13Controller::Reset(xoap::MessageReference msg) throw (xoap::exception::Exception) {
  if (PRINT) std::cout << "PixelAMC13Controller::Reset" << std::endl;
  if (doNothing)
    std::cout << "PixelAMC13Controller: DO NOTHING" << std::endl;
  else {
    if (!amc13) InitAMC13();
    amc13->Configure();
  }
  return MakeSOAPMessageReference("TTCciControlFSMReset");
}

xoap::MessageReference PixelAMC13Controller::Configuration (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  if (PRINT) std::cout << "PixelAMC13Controller::Configuration(IMPLEMENT ME)" << std::endl;
  if (doNothing)
    std::cout << "PixelAMC13Controller: DO NOTHING" << std::endl;
  else {
  }
  return MakeSOAPMessageReference("ParameterSetResponse");
}

xoap::MessageReference PixelAMC13Controller::Configure (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  if (PRINT) std::cout << "PixelAMC13Controller::Configure(MAGIC HAPPENS IN RESET)" << std::endl;
  if (doNothing)
    std::cout << "PixelAMC13Controller: DO NOTHING" << std::endl;
  else {
  }
  return MakeSOAPMessageReference("configureResponse");
}

xoap::MessageReference PixelAMC13Controller::userCommand (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  Attribute_Vector parameters(2);
  parameters[0].name_ = "xdaq:CommandPar";
  parameters[1].name_ = "xdaq:sequence_name";
  Receive(msg, parameters);

  if (PRINT) std::cout << "PixelAMC13Controller::userCommand(" << parameters[0].value_ << ", " << parameters[1].value_ << ")" << std::endl;

  if (doNothing)
    std::cout << "PixelAMC13Controller: DO NOTHING" << std::endl;
  else {
    if (parameters[1].value_ == "CalSync")
      amc13->CalSync();
    else if (parameters[1].value_ == "LevelOne")
      amc13->LevelOne();
    else if (parameters[1].value_ == "ResetROC")
      amc13->ResetROC();
    else if (parameters[1].value_ == "ResetTBM")
      amc13->ResetTBM();
    else if (parameters[1].value_ == "ResetCounters")
      amc13->ResetCounters();
    else
      XCEPT_RAISE(xoap::exception::Exception, "Don't know anything about command " + parameters[1].value_);
  }
  
  return MakeSOAPMessageReference("userTTCciControlResponse");
}

xoap::MessageReference PixelAMC13Controller::Enable(xoap::MessageReference msg) throw (xoap::exception::Exception) {
  if (PRINT) std::cout << "PixelAMC13Controller::Enable(IMPLEMENT ME)" << std::endl;
  if (doNothing)
    std::cout << "PixelAMC13Controller: DO NOTHING" << std::endl;
  else {
  }
  return MakeSOAPMessageReference("enableResponse");
}

xoap::MessageReference PixelAMC13Controller::Stop(xoap::MessageReference msg) throw (xoap::exception::Exception) {
  if (PRINT) std::cout << "PixelAMC13Controller::Stop(IMPLEMENT ME)" << std::endl;
  if (doNothing)
    std::cout << "PixelAMC13Controller: DO NOTHING" << std::endl;
  else {
  }
  return MakeSOAPMessageReference("stopResponse");
}

xoap::MessageReference PixelAMC13Controller::Suspend (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  if (PRINT) std::cout << "PixelAMC13Controller::Suspend(IMPLEMENT ME)" << std::endl;
  if (doNothing)
    std::cout << "PixelAMC13Controller: DO NOTHING" << std::endl;
  else {
  }
  return MakeSOAPMessageReference("suspendResponse");
}
