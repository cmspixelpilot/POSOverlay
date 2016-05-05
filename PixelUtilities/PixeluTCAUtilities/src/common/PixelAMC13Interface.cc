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
                               const std::string& addressT1,
                               const std::string& uriT2,
                               const std::string& addressT2,
                               const std::string& amcMask)
  : fAMC13(new amc13::AMC13(uriT1, addressT1, uriT2, addressT2)),
    fAMCMask(fAMC13->parseInputEnableList(amcMask, true)),
    fDebugPrints(false),
    fSimulate(true)
{
}

PixelAMC13Interface::~PixelAMC13Interface() {
  delete fAMC13;
}

void PixelAMC13Interface::SetBGO(size_t i, BGO b) {
  if (i > 3)
    throw_simple(amc13::Exception::UnexpectedRange(), "addBGO() - only 4 bgos allowed");
  fBGOs[i] = b;
}

PixelAMC13Interface::BGO PixelAMC13Interface::GetBGO(size_t i) {
  if (i > 3)
    throw_simple(amc13::Exception::UnexpectedRange(), "addBGO() - only 4 bgos allowed");
  std::map<size_t, BGO>::iterator it = fBGOs.find(i);
  if (it == fBGOs.end())
    return BGO();
  return it->second;
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


void PixelAMC13Interface::DoResets() {
  if (fDebugPrints) std::cout << "Resetting T1, T2, counters, DAQ" << std::endl;
  fAMC13->reset(amc13::AMC13Simple::T1);
  fAMC13->reset(amc13::AMC13Simple::T2);
  fAMC13->resetCounters();
  fAMC13->resetDAQ();
  fAMC13->sendLocalEvnOrnReset(true, true);

  fAMC13->setTTCHistoryFilter(0, 0x10101); // filter out BC0
  fAMC13->setTTCFilterEna(true);
  fAMC13->setTTCHistoryEna(false);

  // JMTBAD these two don't work with fw T1 0x23D T2 0x2E (but they are both on the T2 and this is the latest fw?)
  //fAMC13->clearTTCHistory(); 
  //fAMC13->clearTTCHistoryFilter();
  ClearL1AHistory();
  ClearTTCHistory();
  ClearTTCHistoryFilter();

  fAMC13->setTTCHistoryFilter(0, 0x10101);
  fAMC13->setTTCFilterEna(true);
  fAMC13->setTTCHistoryEna(true);

  for (int i = 0; i < 4; ++i) 
    fAMC13->configureBGOShort(i, 0, 0, 0, 0);
}

void PixelAMC13Interface::Configure() {
  DoResets();

  ClearL1AHistory();

  fAMC13->AMCInputEnable(fAMCMask);
  if (fDebugPrints) {
    std::cout << "Enabled TTC links for the following AMCs: ";
    for (int i = 0; i < 12; ++i)
      if (fAMCMask & (1 << i)) std::cout << i+1 << " ";
    std::cout << std::endl;
  }

  assert(fBGOs.size() <= 4);
  for (BGOs::const_iterator it = fBGOs.begin(), ite = fBGOs.end(); it != ite; ++it) {
    const size_t i = it->first;
    const BGO& bgo = it->second;
    if (bgo.fUse) {
      ConfigureBGO(i, bgo);
      if (fDebugPrints) std::cout << "Configured BGO Channel " << i << " :"
                                  << " Command: " << bgo.fCommand
                                  << " BX: " << bgo.fBX
                                  << " Prescale: " << bgo.fPrescale
                                  << " Repetitive: " << bgo.fRepeat << std::endl;
    }
  }

  fAMC13->configureLocalL1A(fTrigger.fEnabled, fTrigger.fMode, uint32_t(fTrigger.fBurst), uint32_t(fTrigger.fRate), fTrigger.fRules );
  //fAMC13->fakeDataEnable(1); // fAMC13->write(amc13::AMC13Simple::T1, "CONF.LOCAL_TRIG.FAKE_DATA_ENABLE", 1); // JMTBAD ???
  if (fDebugPrints) std::cout << "Configuring local L1A:"
                                << " Enabled: " << fTrigger.fEnabled
                                << " Mode: " << fTrigger.fMode
                                << " Rate: " << fTrigger.fRate
                                << " Burst: " << fTrigger.fBurst
                                << " Rules: " << fTrigger.fRules << std::endl;

  fAMC13->localTtcSignalEnable(fSimulate);
  if (fSimulate && fDebugPrints) std::cout << "AMC13 configured to use local TTC simulator" << std::endl;

  //ClearTTCHistory();
  //ClearTTCHistoryFilter();

  if (fDebugPrints) std::cout << "AMC13 successfully configured!" << std::endl;
}

void PixelAMC13Interface::Halt() {
  if (fDebugPrints) std::cout << "Halt: ";
  DoResets();
}

void PixelAMC13Interface::Reset() {
  if (fDebugPrints) std::cout << "Reset: " << std::endl;
  DoResets();
}

void PixelAMC13Interface::StartL1A()
{
  fAMC13->startContinuousL1A();
}

void PixelAMC13Interface::StopL1A()
{
  fAMC13->stopContinuousL1A();
}

void PixelAMC13Interface::BurstL1A()
{
  fAMC13->sendL1ABurst();
}

uint32_t PixelAMC13Interface::getTTCHistoryItemAddress( int item) {
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

void PixelAMC13Interface::ConfigureBGO(size_t i, BGO bgo) {
  if (i > 3)
    throw_simple(amc13::Exception::UnexpectedRange(), "AMC13::ConfigureBGO() - channel must be in range 0 to 3");

  if (bgo.fBX > 3563)
    throw_simple(amc13::Exception::UnexpectedRange(), "AMC13::ConfigureBGO() - bx must be in range 0 to 3563");

  char tmp[32];

  snprintf(tmp, sizeof(tmp), "CONF.TTC.BGO%lu.COMMAND", i);
  fAMC13->write( amc13::AMC13Simple::T1, tmp, bgo.fCommand);

  snprintf(tmp, sizeof(tmp), "CONF.TTC.BGO%lu.LONG_CMD", i);
  fAMC13->write( amc13::AMC13Simple::T1, tmp, (bgo.fCommand & 0xFFFFFF00) ? 1 : 0);

  snprintf(tmp, sizeof(tmp), "CONF.TTC.BGO%lu.BX", i);
  fAMC13->write( amc13::AMC13Simple::T1, tmp, bgo.fBX);

  snprintf(tmp, sizeof(tmp), "CONF.TTC.BGO%lu.ORBIT_PRESCALE", i);
  fAMC13->write( amc13::AMC13Simple::T1, tmp, bgo.fPrescale);
}

void PixelAMC13Interface::FireBGOs(unsigned which) {
  bool at_least_one = false;
  char tmp[64];

  for (size_t i = 0; i < 4; ++i) {
    BGO bgo = GetBGO(i);
    if (bgo.fUse) {
      at_least_one = true;

      const bool enable = which & (1 << i);
      snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%lu.ENABLE_SINGLE", i);
      fAMC13->write(amc13::AMC13Simple::T1, tmp, enable && !bgo.fRepeat);
      snprintf( tmp, sizeof(tmp), "CONF.TTC.BGO%lu.ENABLE", i);
      fAMC13->write(amc13::AMC13Simple::T1, tmp, enable && bgo.fRepeat);
    }
  }

  if (!at_least_one)
    throw_simple(amc13::Exception::UnexpectedRange(), "FireBGO() - none enabled?");

  fAMC13->write(amc13::AMC13Simple::T1, "CONF.TTC.ENABLE_BGO", 1);
  fAMC13->write(amc13::AMC13Simple::T1, "ACTION.TTC.SINGLE_COMMAND", 1);
  fAMC13->write(amc13::AMC13Simple::T1, "CONF.TTC.ENABLE_BGO", 0);
}

