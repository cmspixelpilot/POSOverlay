#include <iostream>
#include <time.h>
#include <assert.h>
#include <unistd.h>

#include "PixelFEDInterface/include/PixelFEDInterface.h"
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"

using namespace std;

PixelFEDInterface::PixelFEDInterface(Ph2_HwInterface::RegManager * const rm)
  : Printlevel(1),
    regManager(rm)
{
  num_SEU.assign(48, 0);
}

PixelFEDInterface::~PixelFEDInterface() {
}

int PixelFEDInterface::setup(const string& fileName) {
  pixelFEDCard = pos::PixelPh1FEDCard(fileName);
  return setup();
}

int PixelFEDInterface::setup(pos::PixelPh1FEDCard& pfc) {
  pixelFEDCard = pfc;
  return setup();
}

int PixelFEDInterface::setup() {
  return 0;
}

void PixelFEDInterface::loadFPGA() {
}

void PixelFEDInterface::reset() {
}

void PixelFEDInterface::resetFED() {
}

void PixelFEDInterface::armOSDFifo(int channel, int rochi, int roclo) {
}

uint32_t PixelFEDInterface::readOSDFifo(int channel) {
  return 0;
}

void PixelFEDInterface::readPhases(bool verbose, bool override_timeout) {
}

int PixelFEDInterface::drainTransparentFifo(uint32_t* data) {
  return 0;
}

int PixelFEDInterface::drainSpyFifo(uint32_t* data) {
  return 0;
}

int PixelFEDInterface::drainFifo1(uint32_t *data) {
  return 0;
}

int PixelFEDInterface::drainErrorFifo(uint32_t *data) {
  return 0;
}

int PixelFEDInterface::drainTemperatureFifo(uint32_t* data) {
  return 0;
}

int PixelFEDInterface::drainTTSFifo(uint32_t *data) {
  return 0;
}

int PixelFEDInterface::spySlink64(uint64_t *data) {
  return 0;
}

bool PixelFEDInterface::isWholeEvent(uint32_t nTries) {
  return false;
}

bool PixelFEDInterface::isNewEvent(uint32_t nTries) {
  return false;
}

int PixelFEDInterface::enableSpyMemory(const int enable) {
  // pixelFEDCard.modeRegister ?
  return setModeRegister(pixelFEDCard.modeRegister);
}

uint32_t PixelFEDInterface::get_VMEFirmwareDate() {
  uint32_t iwrdat=0;
  // read here
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" VME FPGA (update via jtag pins only) firmware date d/m/y "
      <<dec<<((iwrdat&0xff000000)>>24)<<"/"
      <<dec<<((iwrdat&0xff0000)>>16)<<"/"
      <<dec<<((((iwrdat&0xff00)>>8)*100)+(iwrdat&0xff))<<endl;
  return iwrdat;
}

uint32_t PixelFEDInterface::get_FirmwareDate(int chip) {
  uint32_t iwrdat=0;
  if(chip != 1) return 0;
  //read here
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" FPGA firmware date d/m/y "
      <<dec<<((iwrdat&0xff000000)>>24)<<"/"
      <<dec<<((iwrdat&0xff0000)>>16)<<"/"
      <<dec<<((((iwrdat&0xff00)>>8)*100)+(iwrdat&0xff))<<endl;
  return iwrdat;
}

bool PixelFEDInterface::loadFedIDRegister() {
  if(Printlevel&1)cout<<"Load FEDID register from DB 0x"<<hex<<pixelFEDCard.fedNumber<<dec<<endl;
  return setFedIDRegister(pixelFEDCard.fedNumber);
}

bool PixelFEDInterface::setFedIDRegister(const uint32_t value) {
  cout<<"Set FEDID register "<<hex<<value<<dec<<endl;
  // write here
  uint32_t got = getFedIDRegister();
  if (value != got) cout<<"soft FEDID = "<<value<<" doesn't match hard board FEDID = "<<got<<endl;
  return value == got;
}

uint32_t PixelFEDInterface::getFedIDRegister() {
  return 0;
}

bool PixelFEDInterface::loadControlRegister() {
  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Load Control register from DB 0x"<<hex<<pixelFEDCard.Ccntrl<<dec<<endl;
  return setControlRegister(pixelFEDCard.Ccntrl);
}

bool PixelFEDInterface::setControlRegister(uint32_t value) {
  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Set Control register "<<hex<<value<<dec<<endl;
  // write here
  pixelFEDCard.Ccntrl=value; // stored this value   
  return false;
}

uint32_t PixelFEDInterface::getControlRegister() {
  return 0;
}

bool PixelFEDInterface::loadModeRegister() {
  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Load Mode register from DB 0x"<<hex<<pixelFEDCard.Ccntrl<<dec<<endl;
  return setModeRegister(pixelFEDCard.modeRegister);
}

bool PixelFEDInterface::setModeRegister(uint32_t value) {
  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Set Mode register "<<hex<<value<<dec<<endl;
  // write here
  pixelFEDCard.modeRegister=value; // stored this value   
  return false;
}

uint32_t PixelFEDInterface::getModeRegister() {
  return 0;
}

void PixelFEDInterface::set_PrivateWord(uint32_t pword) {
}

void PixelFEDInterface::resetSlink() {
}

bool PixelFEDInterface::checkFEDChannelSEU() {
  /*
  Check to see if the channels that are currently on match what we expect. If not
  increment the counter and return true. Note that this assumes that the method won't
  be called again until the SEU is fixed. Otherwise, the counter will be incremented multiple
  times for the same SEU.
  */
  bool foundSEU = false;
  uint64_t enbable_current_i = 0;
  // read here
  enbable_t enbable_current(enbable_current_i);

  // Note: since N_enbable_expected is bitset<9>, this only compares the first 9 bits
  if (enbable_current != enbable_expected && enbable_current != enbable_last) {
    foundSEU = true;
    cout << "Detected FEDChannel SEU in FED " << pixelFEDCard.fedNumber << endl;
    cout << "Expected " << enbable_expected << " Found " << enbable_current << " Last " << enbable_last << endl;
    incrementSEUCountersFromEnbableBits(num_SEU, enbable_current, enbable_last);
  }

  enbable_last = enbable_current;

  return foundSEU;
}

void PixelFEDInterface::incrementSEUCountersFromEnbableBits(vector<int> &counter, bitset<48> current, bitset<48> last) {
  for(size_t i = 0; i < current.size(); i++)
    if (current[i] != last[i])
      counter[i]++;
}

bool PixelFEDInterface::checkSEUCounters(int threshold) {
  /*
  Check to see if any of the channels have more than threshold SEUs.
  If so, return true and set expected enbable bit for that channel
  to off.
  Otherwise, return false
  */
  bool return_val = false;
  cout << "Checking for more than " << threshold << " SEUs in FED " << pixelFEDCard.fedNumber << endl;
  cout << "Channels with too many SEUs: ";
  for (size_t i=0; i<48; i++)
  {
    if (num_SEU[i] >= threshold) {
      enbable_expected[i] = 1; // 1 is off
      cout << " " << 1+i << "(" << num_SEU[i] << ")";
      return_val = true;
    }
  }
  if (return_val) {
    cout << ". Disabling." << endl;
    cout << "Setting runDegraded flag for FED " << pixelFEDCard.fedNumber << endl;
    runDegraded_ = true;
  } else cout << endl;
  return return_val;
}

void PixelFEDInterface::resetEnbableBits() {
  // Get the current values of higher bits in these registers, so we can leave them alone

  //uint64_t OtherConfigBits = 0;
  //uint64_t enbable_exp = 0; //enbable_expected.to_ullong(); 
  //for (int i = 0; i < 48; ++i)
  //  if (enbable_expected[i])
  //    enbable_exp |= 1<<i;
  //uint64_t write = OtherConfigBits | enbable_exp;
  //  write;
}

void PixelFEDInterface::storeEnbableBits() {
  enbable_expected = pixelFEDCard.cntrl;
  enbable_last = enbable_expected;
}

void PixelFEDInterface::resetSEUCountAndDegradeState(void) {
  cout << "reset SEU counters and the runDegrade flag " << endl;
  // reset the state back to running 
  runDegraded_ = false;
  // clear the count flag
  num_SEU.assign(48, 0);
  // reset the expected state to default
  storeEnbableBits();
}

void PixelFEDInterface::testTTSbits(uint32_t data, int enable) {
  //will turn on the test bits indicate in bits 0...3 as long as bit 31 is set
  //As of this writing, the bits indicated are: 0(Warn), 1(OOS), 2(Busy), 4(Ready)
  //Use a 1 or any >1 to enable, a 0 or <0 to disable
  if (enable>0)
    data = (data | 0x80000000) & 0x8000000f;
  else
    data = data & 0xf;
  // write here
}

void PixelFEDInterface::setXY(int X, int Y) {
}

int PixelFEDInterface::getXYCount() {
  return 0;
}

void PixelFEDInterface::resetXYCount() {
}

int PixelFEDInterface::getNumFakeEvents() {
  return 0;
}

void PixelFEDInterface::resetNumFakeEvents() {
}

uint32_t PixelFEDInterface::readEventCounter() {
  return 0;
}

uint32_t PixelFEDInterface::getFifoStatus() {
  return 0;
}

uint32_t PixelFEDInterface::linkFullFlag() {
  return 0;
}

uint32_t PixelFEDInterface::numPLLLocks() {
  return 0;
}

uint32_t PixelFEDInterface::getFifoFillLevel() {
  return 0;
}

uint64_t PixelFEDInterface::getSkippedChannels() {
  return 0;
}

uint32_t PixelFEDInterface::getErrorReport(int ch) {
  return 0;
}

uint32_t PixelFEDInterface::getTimeoutReport(int ch) {
  return 0;
}

int PixelFEDInterface::TTCRX_I2C_REG_READ(int Register_Nr) {
  return 0;
}

int PixelFEDInterface::TTCRX_I2C_REG_WRITE(int Register_Nr, int Value) {
  return 0;
}
