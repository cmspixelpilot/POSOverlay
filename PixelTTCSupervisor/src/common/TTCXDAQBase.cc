#include "TTCXDAQBase.hh"

std::string htmlencode(const std::string & s) {
  std::string ret;
  for (size_t i = 0; i < s.size(); ++i) {
    switch(s[i]) {
    case '<': ret += "&lt;"; break;
    case '>': ret += "&gt;"; break;
    case '&': ret += "&amp;"; break;
    case '"': ret += "&quot;"; break;
    default: ret += s[i];
    }
  }
  return ret;
}

TTCXDAQBase::TTCXDAQBase(xdaq::ApplicationStub* stub, uint32_t BTimeCorrection) :
  xdaq::WebApplication(stub),
  localid_(getApplicationDescriptor()->getLocalId()),
  instance_(getApplicationDescriptor()->hasInstanceNumber() ?
            getApplicationDescriptor()->getInstance() :
            (uint32_t)-1),
  busAdapter_(0),
  busadaptername_("$TTCBUSADAPTER"),
  //asciConfigurationFilePath_("dummy.txt"),
  ReloadAtEveryConfigure_(0),
  Is64XCompatible_(true),
  Location_(0),
  CtrlLvl_(0),
  BTimeCorrection_(BTimeCorrection),
  any_errors_(0), 
  ReadConfigFromFile_(false),
  ConfigModified_(true),
  oldfrequency_(-99.0),
  neverSOAPed_(true)
{
  time(&tnow_);
  tprevious_ = tnow_;

  getApplicationInfoSpace()->fireItemAvailable("name", &name_);
  getApplicationInfoSpace()->fireItemAvailable("BusAdapter", &busadaptername_);
  getApplicationInfoSpace()->fireItemAvailable("Location", &Location_ );
  getApplicationInfoSpace()->fireItemAvailable("HyperdaqCtrlLevel", &CtrlLvl_ );
  getApplicationInfoSpace()->fireItemAvailable("BTimeCorrection", &BTimeCorrection_ );
  getApplicationInfoSpace()->fireItemAvailable("ReloadAtEveryConfigure", 
                                               &ReloadAtEveryConfigure_ );
  getApplicationInfoSpace()->fireItemAvailable("Is64XCompatible", &Is64XCompatible_ );
  //getApplicationInfoSpace()->fireItemAvailable("Configuration", &ConfigurationString_);
  getApplicationInfoSpace()->fireItemAvailable("StateName", &StateName_);
  getApplicationInfoSpace()->fireItemAvailable("stateName", &StateName_);

}

TTCXDAQBase::~TTCXDAQBase() {
}
