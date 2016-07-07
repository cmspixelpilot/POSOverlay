#include "CalibFormats/SiPixelObjects/interface/PixelAMC13Config.h"
#include "PixelAMC13Controller/include/PixelAMC13Controller.h"

XDAQ_INSTANTIATOR_IMPL(PixelAMC13Controller)

namespace {
  const bool PRINT = false;

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
    SOAPCommander(this)
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

  // these only used from xml to talk to one amc13 if we haven't been configured yet
  getApplicationInfoSpace()->fireItemAvailable("UriT1", &uriT1);
  getApplicationInfoSpace()->fireItemAvailable("UriT2", &uriT2);
  getApplicationInfoSpace()->fireItemAvailable("AddressT1", &addressT1);
  getApplicationInfoSpace()->fireItemAvailable("AddressT2", &addressT2);
  getApplicationInfoSpace()->fireItemAvailable("Mask", &mask);
  getApplicationInfoSpace()->fireItemAvailable("CalBX", &calBX);
  getApplicationInfoSpace()->fireItemAvailable("L1ADelay", &L1ADelay);
  getApplicationInfoSpace()->fireItemAvailable("NewWay", &newWay);
  getApplicationInfoSpace()->fireItemAvailable("VerifyL1A", &verifyL1A);
}

PixelAMC13Controller::~PixelAMC13Controller() {
  DelAMC13s();
}

void PixelAMC13Controller::DelAMC13s() {
  for (size_t i = 0; i < amc13s.size(); ++i)
    delete amc13s[i];
  amc13s.clear();
}

void PixelAMC13Controller::AddAMC13(const pos::PixelAMC13Parameters& p) {
  const std::string addrT1 = p.getAddressT1() != "" ? p.getAddressT1() : std::string(addressT1);
  const std::string addrT2 = p.getAddressT2() != "" ? p.getAddressT2() : std::string(addressT2);
  PixelAMC13Interface* amc13 = new PixelAMC13Interface(p.getUriT1(), addrT1, p.getUriT2(), addrT2);
  amc13->SetDebugPrints(PRINT);
  amc13->SetMask(p.getSlotMask());
  amc13->SetCalBX(p.getCalBX());
  amc13->SetL1ADelay(p.getL1ADelay());
  amc13->SetNewWay(p.getNewWay());
  amc13->SetVerifyL1A(p.getVerifyL1A());
  amc13s.push_back(amc13);
}

void PixelAMC13Controller::InitAMC13s() {
  DelAMC13s();

  if (amc13_cfgs.size() == 0) {
    pos::PixelAMC13Parameters p(-1, uriT1, uriT2, addressT1, addressT2, mask, calBX, L1ADelay, newWay, verifyL1A);
    amc13_cfgs.push_back(p);
  }

  for (size_t i = 0; i < amc13_cfgs.size(); ++i)
    AddAMC13(amc13_cfgs[i]);
}

void PixelAMC13Controller::Default(xgi::Input* in, xgi::Output* out ) throw (xgi::exception::Exception) {
  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  *out << cgicc::html().set("lang", "en").set("dir", "ltr") << std::endl;
  *out << cgicc::title("Pixel AMC13 Controller") << std::endl;

  const std::string& URN = getApplicationDescriptor()->getURN();
  *out <<
    "<script type=\"text/javascript\">\n"
    "window.history.pushState('Default', 'Title', '/" << URN << "');\n"
    "</script>\n";

  std::vector<std::string> sends = {"reset", "CalSync", "LevelOne", "ResetROC", "ResetTBM", "ResetCounters" };

  const size_t namc13s = amc13s.size();

  if (namc13s == 0)
    *out << "not yet initialized!<br>";

  *out << "<h3>Commands:</h3>\n"
       << "<table><tr>\n";
  for (std::vector<std::string>::const_iterator it = sends.begin(), ite = sends.end(); it != ite; ++it) {
    *out << "<td>";
    std::string url = "/";
    url += URN;
    url += "/StateMachineXgiHandler";
    *out << cgicc::form().set("method","get").set("action", url).set("enctype","multipart/form-data") << std::endl;
    *out << cgicc::input().set("type", "submit").set("name", "StateInput").set("value", *it);
    *out << cgicc::p() << std::endl;
    *out << cgicc::form() << std::endl;
    *out << "</td>\n";
    if (namc13s == 0) break;
  }
  *out << "</tr></table><br>\n";

  if (namc13s == 0) return;
  
  //*out << "<table><tr>\n"; 

  *out << "Status summary:<br>\n";
  *out << "<table><tr><td></td>\n";

  for (size_t i = 0; i < namc13s; ++i) *out << "<td>Crate " << amc13_cfgs[i].getCrate() << "</td>\n";

  *out << "<tr><td>T1 ver:</td>\n";             for (size_t i = 0; i < namc13s; ++i) *out << "<td>" << std::hex << amc13s[i]->GetT1Version() << std::dec << "</td>\n";
  *out << "<tr><td>T2 ver:</td>\n";             for (size_t i = 0; i < namc13s; ++i) *out << "<td>" << std::hex << amc13s[i]->GetT2Version() << std::dec << "</td>\n";
  *out << "<tr><td>Slot mask:</td>\n";          for (size_t i = 0; i < namc13s; ++i) *out << "<td>" << std::hex << amc13s[i]->GetSlotMask()  << std::dec << "</td>\n";
  *out << "<tr><td>TTC sim. ena.:</td>\n";      for (size_t i = 0; i < namc13s; ++i) *out << "<td>" << amc13s[i]->GetTTCSimulator()            << "</td>\n";
  *out << "<tr><td># L1A:</td>\n";              for (size_t i = 0; i < namc13s; ++i) *out << "<td>" << commaify(amc13s[i]->GetL1ACount())      << "</td>\n";
  *out << "<tr><td>L1A rate (Hz):</td>\n";      for (size_t i = 0; i < namc13s; ++i) *out << "<td>" << commaify(amc13s[i]->GetL1ARate())       << "</td>\n";
  *out << "<tr><td># CalSync:</td>\n";          for (size_t i = 0; i < namc13s; ++i) *out << "<td>" << commaify(amc13s[i]->GetCalSyncCount())  << "</td>\n";
  *out << "<tr><td># LevelOne:</td>\n";         for (size_t i = 0; i < namc13s; ++i) *out << "<td>" << commaify(amc13s[i]->GetLevelOneCount()) << "</td>\n";
  *out << "<tr><td># ResetROC:</td>\n";         for (size_t i = 0; i < namc13s; ++i) *out << "<td>" << commaify(amc13s[i]->GetResetROCCount()) << "</td>\n";
  *out << "<tr><td># ResetTBM:</td>\n";         for (size_t i = 0; i < namc13s; ++i) *out << "<td>" << commaify(amc13s[i]->GetResetTBMCount()) << "</td>\n";
  *out << "<tr><td>Input clock (Hz):</td>\n";   for (size_t i = 0; i < namc13s; ++i) *out << "<td>" << commaify(amc13s[i]->GetClockFreq())     << "</td>\n";

#if 0
  amc13->Get()->getStatus()->SetHTML();

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
  urlAllAMC13Tables += URN;
  urlAllAMC13Tables += "/AllAMC13Tables";
  *out << "<a href=\"" << urlAllAMC13Tables << "\"><h3>All AMC13 status tables</h3></a><br>\n";
#endif
}

void PixelAMC13Controller::StateMachineXgiHandler(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception) {
  cgicc::Cgicc cgi(in);

  Attribute_Vector attrib(2);
  attrib[0].name_="xdaq:CommandPar";
  attrib[0].value_="Execute Sequence";
  attrib[1].name_="xdaq:sequence_name";
  attrib[1].value_=cgi.getElement("StateInput")->getValue();

  if (!AMC13sReady() && attrib[1].value_ != "reset") {
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
#if 0
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
#endif
}

xoap::MessageReference PixelAMC13Controller::Reset(xoap::MessageReference msg) throw (xoap::exception::Exception) {
  if (PRINT) std::cout << "PixelAMC13Controller::Reset" << std::endl;
  if (doNothing)
    std::cout << "PixelAMC13Controller: DO NOTHING" << std::endl;
  else {
    if (amc13_cfgs.size() == 0) {
      InitAMC13s();
      for (size_t i = 0; i < amc13s.size(); ++i) amc13s[i]->Configure();
    }
  }
  return MakeSOAPMessageReference("TTCciControlFSMReset");
}

xoap::MessageReference PixelAMC13Controller::Configuration (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  if (PRINT) std::cout << "PixelAMC13Controller::Configuration()" << std::endl;
  if (doNothing)
    std::cout << "PixelAMC13Controller: DO NOTHING" << std::endl;
  else {
    std::stringstream ss(msg->getSOAPPart().getEnvelope().getTextContent());
    std::string line;
    bool first = true;
    if (PRINT) std::cout << "content:\n";
    while (std::getline(ss, line, '\n')) {
      if (PRINT) std::cout << line << std::endl;
      // If we're acting as both the amc13 backplane configurator and
      // the ttc, then we'll get a configuration twice. first will be
      // the amc13 config, the second will be some ttcciconfig that we
      // ignore.
      if (first) {
        first = false;

        if (line.find("AMC13 magic string") == std::string::npos) {
          if (PRINT) std::cout << "uh uh uh, you didn't say the magic words\n";
          break;
        }
        else if (PRINT) {
          std::cout << "magic words found!\n";
          amc13_cfgs.clear();
          continue;
        }
      }

      std::pair<bool, pos::PixelAMC13Parameters> p = pos::PixelAMC13Config::parse_line(line);
      if (p.first) { // if it was a blank or comment line, will be false
        amc13_cfgs.push_back(p.second);
        if (PRINT) std::cout << "successfully parsed: crate: " << amc13_cfgs.back().getCrate() << "\n";
      }
      else if (PRINT)
        std::cout << "didn't get anything out of this line!\n";
    }

    InitAMC13s();
  }

  return MakeSOAPMessageReference("ParameterSetResponse");
}

xoap::MessageReference PixelAMC13Controller::Configure (xoap::MessageReference msg) throw (xoap::exception::Exception) {
  if (PRINT) std::cout << "PixelAMC13Controller::Configure()" << std::endl;
  if (doNothing)
    std::cout << "PixelAMC13Controller: DO NOTHING" << std::endl;
  else {
    for (size_t i = 0; i < amc13s.size(); ++i) amc13s[i]->Configure();
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
      for (size_t i = 0; i < amc13s.size(); ++i) amc13s[i]->CalSync();
    else if (parameters[1].value_ == "LevelOne")
      for (size_t i = 0; i < amc13s.size(); ++i) amc13s[i]->LevelOne();
    else if (parameters[1].value_ == "ResetROC")
      for (size_t i = 0; i < amc13s.size(); ++i) amc13s[i]->ResetROC();
    else if (parameters[1].value_ == "ResetTBM")
      for (size_t i = 0; i < amc13s.size(); ++i) amc13s[i]->ResetTBM();
    else if (parameters[1].value_ == "ResetCounters")
      for (size_t i = 0; i < amc13s.size(); ++i) amc13s[i]->ResetCounters();
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
