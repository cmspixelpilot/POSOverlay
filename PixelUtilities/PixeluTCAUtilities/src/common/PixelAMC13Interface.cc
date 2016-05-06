#include <iomanip>
#include <iostream>
#include <string>
#include "PixelUtilities/PixeluTCAUtilities/include/PixelAMC13Interface.h"

namespace {
  template <typename T>
  void throw_simple(T t, std::string extra) {
    t.Append(extra);
    throw t;
  }
}

PixelAMC13Interface::PixelAMC13Interface(const std::string& uriT1,
                                         const std::string& uriT2)

  : fAMC13(new amc13::AMC13(uriT1, "/opt/cactus/etc/amc13/AMC13XG_T1.xml",
                            uriT2, "/opt/cactus/etc/amc13/AMC13XG_T2.xml")),
    fMask(0),
    fDebugPrints(false),
    fCalBX(381),
    fL1ABurstDelay(10)
{
}

PixelAMC13Interface::PixelAMC13Interface(const std::string& uriT1,
                                         const std::string& addressT1,
                                         const std::string& uriT2,
                                         const std::string& addressT2)

  : fAMC13(new amc13::AMC13(uriT1, addressT1, uriT2, addressT2)),
    fMask(0),
    fDebugPrints(false),
    fCalBX(381),
    fL1ABurstDelay(10)
{
}

PixelAMC13Interface::~PixelAMC13Interface() {
  delete fAMC13;
}

void PixelAMC13Interface::DoResets() {
  if (fDebugPrints) std::cout << "Resetting T1, T2, counters, DAQ" << std::endl;
  fAMC13->reset(amc13::AMC13Simple::T1);
  fAMC13->reset(amc13::AMC13Simple::T2);
  fAMC13->resetCounters();
  fAMC13->resetDAQ();
  fAMC13->sendLocalEvnOrnReset(true, true);

  // JMTBAD these two don't work with fw T1 0x23D T2 0x2E (but they are both on the T2 and this is the latest fw?)
  //fAMC13->clearTTCHistory(); 
  //fAMC13->clearTTCHistoryFilter();
  ClearL1AHistory();
  ClearTTCHistory();
  ClearTTCHistoryFilter();

  fAMC13->setTTCHistoryFilter(0, 0x10101); // filter BC0
  fAMC13->setTTCFilterEna(true);
  fAMC13->setTTCHistoryEna(true);

  for (int i = 0; i < 4; ++i)
    fAMC13->configureBGOShort(i, 0, 0, 0, true);
}

void PixelAMC13Interface::Configure() {
  DoResets();

  fAMC13->localTtcSignalEnable(true);

  fAMC13->AMCInputEnable(fMask);
  if (fDebugPrints) {
    std::cout << "Enabled TTC links for these AMCs: ";
    for (int i = 0; i < 12; ++i)
      if (fMask & (1 << i)) std::cout << i+1 << " ";
    std::cout << std::endl;
  }

  ConfigureBGO(0, BGO(0x2c, false,  false,  0, fCalBX)); // CAL
  ConfigureBGO(1, BGO(0x14, false, false,  0, 100));    // RESET TBM
  ConfigureBGO(2, BGO(0x1c, false, false,  0, 100));    // RESET ROC

  //fAMC13->fakeDataEnable(1); // JMTBAD needed to send triggers ???
  fAMC13->configureLocalL1A(true, 0, 1, 1, 0); // trigger burst 1 after 1 orbit = 
}

void PixelAMC13Interface::Halt() {
  if (fDebugPrints) std::cout << "Halt";
  DoResets();
}

void PixelAMC13Interface::Reset() {
  if (fDebugPrints) std::cout << "Reset" << std::endl;
  DoResets();
}

void PixelAMC13Interface::CalSync() {
  if (fDebugPrints) std::cout << "CalSync" << std::endl;
  fAMC13->write(amc13::AMC13Simple::T1, "CONF.TTC.BGO0.ENABLE", 1);
  usleep(40000);
  fAMC13->sendL1ABurst();
  usleep(20000);
  fAMC13->write(amc13::AMC13Simple::T1, "CONF.TTC.BGO0.ENABLE", 0);
  usleep(10000);
}

void PixelAMC13Interface::LevelOne() {
  if (fDebugPrints) std::cout << "LevelOne" << std::endl;
  fAMC13->sendL1ABurst();
}

void PixelAMC13Interface::ResetTBM() {
  if (fDebugPrints) std::cout << "ResetTBM" << std::endl;
  FireBGO(1);
}

void PixelAMC13Interface::ResetROC() {
  if (fDebugPrints) std::cout << "ResetROC" << std::endl;
  FireBGO(2);
}

uint32_t PixelAMC13Interface::GetClockFreq() {
  uint32_t v = fAMC13->read(amc13::AMC13Simple::T2, "STATUS.TTC.CLK_FREQ") * 50;
  if (fDebugPrints) std::cout << "ClockFreq " << v << std::endl;
  return v;
}

uint64_t PixelAMC13Interface::GetL1ACount() {
  uint64_t v = (uint64_t(fAMC13->read(amc13::AMC13Simple::T1, "STATUS.GENERAL.L1A_COUNT_HI")) << 32) | fAMC13->read(amc13::AMC13Simple::T1, "STATUS.GENERAL.L1A_COUNT_LO");
  if (fDebugPrints) std::cout << "L1ACount " << v << std::endl;
  return v;
}

uint32_t PixelAMC13Interface::GetL1ARate() {
  uint32_t v = fAMC13->read(amc13::AMC13Simple::T1, "STATUS.GENERAL.L1A_RATE_HZ");
  if (fDebugPrints) std::cout << "L1ARate " << v << std::endl;
  return v;
}

void PixelAMC13Interface::ClearL1AHistory() {
  for (int i = 0; i < 512; ++i)
    fAMC13->write(amc13::AMC13Simple::T1, 0x200+i, 0);
}

void PixelAMC13Interface::ClearTTCHistory() {
  const uint32_t base = fAMC13->getT2()->getNode("STATUS.TTC_HISTORY.BUFFER.BASE").getAddress();
  for (int i = 0; i < 2048; ++i)
    fAMC13->write(amc13::AMC13Simple::T2, base+i, 0);
}

void PixelAMC13Interface::ClearTTCHistoryFilter() {
  const uint32_t base = fAMC13->getT2()->getNode("CONF.TTC_HISTORY.FILTER_LIST").getAddress();
  for (int i = 0; i < 16; ++i)
    fAMC13->write(amc13::AMC13Simple::T2, base+i, 0);
}

uint32_t PixelAMC13Interface::getTTCHistoryItemAddress(int item) {
  if (item > -1 || item < -512)
    throw_simple(amc13::Exception::UnexpectedRange(), "TTC history item offset out of range");

  const uint32_t base = fAMC13->getT2()->getNode("STATUS.TTC_HISTORY.BUFFER.BASE").getAddress();
  const uint32_t wp = fAMC13->read(amc13::AMC13Simple::T2, "STATUS.TTC_HISTORY.COUNT");
  uint32_t a = base + 4*(wp+item);
  if (a < base)
    a += 0x800;
  return a;
}

void PixelAMC13Interface::DumpHistory() {
  std::vector<uint32_t> cVec;
  const uint32_t base = fAMC13->getT2()->getNode("STATUS.TTC_HISTORY.BUFFER.BASE").getAddress();
  const int nhist = fAMC13->getTTCHistoryCount();
  if (nhist > 0) {
    uint32_t adr = getTTCHistoryItemAddress(-nhist);
    for( int i=0; i<nhist; i++) {
      for( int k=0; k<4; k++)
        cVec.push_back( fAMC13->read( amc13::AMC13Simple::T2, adr+k));
      adr = base + ((adr + 4) % 0x800);
    }
  }

  std::cout << "TTC History:\n"
            << "Index  Cmd         Orbit    BX     Event" << std::endl;
  for (size_t index = 0; index < cVec.size() / 4; ++index) {
    const uint32_t command = cVec.at(index * 4 + 0);
    const uint32_t orbit   = cVec.at(index * 4 + 1);
    const uint32_t bx      = cVec.at(index * 4 + 2);
    const uint32_t event   = cVec.at(index * 4 + 3);

    if (command || orbit || bx || event)
      std::cout << std::setfill(' ')
                << std::setw(5) << index
                << std::setw(5) << std::hex << command << std::dec
                << std::setw(14) << orbit
                << std::setw(6) << bx
                << std::setw(10) << event << std::endl;
  }
}

void PixelAMC13Interface::DumpTriggers()
{
  std::vector<uint32_t> cVec(512, 0);
  for (int i = 0; i < 512; ++i)
    cVec[i] = fAMC13->read(amc13::AMC13Simple::T1, 0x200+i);

  //now decode the Info in here!
  std::cout << "L1A History:\n"
            << "Index         Orbit    BX     Event     Flags" << std::endl;
  for (size_t index = 0; index < cVec.size() / 4; ++index) {
    uint32_t orbit = cVec.at(index * 4 + 0);
    uint32_t bx    = cVec.at(index * 4 + 1);
    uint32_t event = cVec.at(index * 4 + 2);
    uint32_t flags = cVec.at(index * 4 + 3);

    if (orbit || bx || event || flags)
      std::cout << std::setfill(' ')
                << std::setw(5) << index
                << std::setw(14) << orbit
                << std::setw(6) << bx
                << std::setw(10) << event
                << std::setw(10) << std::hex << flags << std::dec << std::endl;
  }
}

void PixelAMC13Interface::ConfigureBGO(unsigned i, BGO bgo) {
  if (i > 3)
    throw_simple(amc13::Exception::UnexpectedRange(), "AMC13::ConfigureBGO() - channel must be in range 0 to 3");

  if (bgo.fBX > 3563)
    throw_simple(amc13::Exception::UnexpectedRange(), "AMC13::ConfigureBGO() - bx must be in range 0 to 3563");

  static const std::string cmds[4] = {
    "CONF.TTC.BGO0.",
    "CONF.TTC.BGO1.",
    "CONF.TTC.BGO2.",
    "CONF.TTC.BGO3."
  };

  const std::string& bgo_base = cmds[i];

  fAMC13->write(amc13::AMC13Simple::T1, bgo_base + "COMMAND",        bgo.fCommand);
  fAMC13->write(amc13::AMC13Simple::T1, bgo_base + "LONG_CMD",       bgo.isLong);
  fAMC13->write(amc13::AMC13Simple::T1, bgo_base + "ENABLE",         bgo.fEnable);
  fAMC13->write(amc13::AMC13Simple::T1, bgo_base + "ENABLE_SINGLE",  bgo.fEnableSingle);
  fAMC13->write(amc13::AMC13Simple::T1, bgo_base + "BX",             bgo.fBX);
  fAMC13->write(amc13::AMC13Simple::T1, bgo_base + "ORBIT_PRESCALE", bgo.fPrescale);
}

void PixelAMC13Interface::FireBGO(unsigned i) {
  if (i > 3)
    throw_simple(amc13::Exception::UnexpectedRange(), "AMC13::FireBGO() - channel must be in range 0 to 3");

  static const std::string cmds[4] = {
    "CONF.TTC.BGO0.ENABLE_SINGLE",
    "CONF.TTC.BGO1.ENABLE_SINGLE",
    "CONF.TTC.BGO2.ENABLE_SINGLE",
    "CONF.TTC.BGO3.ENABLE_SINGLE"
  };

  fAMC13->write(amc13::AMC13Simple::T1, cmds[i], 1); 
  fAMC13->write(amc13::AMC13Simple::T1, "ACTION.TTC.SINGLE_COMMAND", 1);
  fAMC13->write(amc13::AMC13Simple::T1, cmds[i], 0);
}

