#ifndef PixelFEDInterface_PixelPh1FEDInterface_h
#define PixelFEDInterface_PixelPh1FEDInterface_h

#include <bitset>
#include <map>
#include <vector>

#include "PixelFEDInterface/include/PixelFEDInterfaceBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFEDCard.h"
#include "PixelUtilities/PixeluTCAUtilities/include/RegManager.h"

class PixelPh1FEDInterface : public PixelFEDInterfaceBase {
 public:
  struct FitelRegItem {
    uint8_t fAddress;
    uint8_t fDefValue;
    uint8_t fValue;
    char fPermission;
  };

  typedef std::map < std::string, FitelRegItem > FitelRegMap;

  typedef std::bitset<48> enbable_t;

  PixelPh1FEDInterface(RegManager*, const std::string&);
  ~PixelPh1FEDInterface();

  int setup(const std::string& fileName); 
  int setup(pos::PixelFEDCard pfc); 
  int setup();  // run the setup 

  void setChannelOfInterest(int ch);
  void setFIFO1(int ch);
  void disableBE(bool disable);
  void setPixelForScore(int dc, int pxl);
  uint32_t getScore(int channel);
  void getBoardInfo();
  void disableFMCs();
  void enableFMCs();
  void FlashProm( const std::string & strConfig, const char* pstrFile );
  void JumpToFpgaConfig( const std::string & strConfig );
  std::vector<std::string> getFpgaConfigList();
  void DeleteFpgaConfig( const std::string & strId );
  void DownloadFpgaConfig( const std::string& strConfig, const std::string& strDest);
  void checkIfUploading();
  void EncodeFitelReg( const FitelRegItem& pRegItem, uint8_t pFMCId, uint8_t pFitelId , std::vector<uint32_t>& pVecReq );
  void DecodeFitelReg( FitelRegItem& pRegItem, uint8_t pFMCId, uint8_t pFitelId, uint32_t pWord );
  void i2cRelease(uint32_t pTries);
  bool polli2cAcknowledge(uint32_t pTries);
  bool WriteFitelReg (const std::string& pRegNode, int cFMCId, int cFitelId, uint8_t pValue, bool pVerifLoop);
  bool WriteFitelBlockReg(std::vector<uint32_t>& pVecReq);
  bool ReadFitelBlockReg(std::vector<uint32_t>& pVecReq);
  double ReadRSSI(int fiber);

  void loadFPGA(); // (re)Loads the FPGA with the program in the EEPROM
  int reset(); // resets everything
  void resetFED(); // reset FED (LRES + CLRES + fake event,center OOS counters + error fifos)
  void sendResets(unsigned which); // LRES + CLRES on old FED

  // arm OSD readback from roc; wait 31 triggers and call read
  void armOSDFifo(int channel, int rochi, int roclo);
  uint32_t readOSDFifo(int channel);

  struct decoded_phases {
    decoded_phases(int fiber_, uint32_t a0, uint32_t a1, uint32_t a2);

    int fiber;

    bool idelay_ctrl_ready;
    int idelay_tap_set;
    int idelay_tap_read;
    uint32_t idelay_tap_scan;

    bool sampling_clock_swapped;
    bool init_swap_finished;
    bool init_init_finished;

    int first_zeros_lo;
    int first_zeros_hi;
    int second_zeros_lo;
    int second_zeros_hi;
    int num_windows;

    int delay_tap_used;

    static void print_header(std::ostream& o);
  };

  std::vector<decoded_phases> autoPhases();
  std::vector<decoded_phases> manualPhases();
  std::vector<decoded_phases> readPhases();
  void readPhases(bool verbose, bool override_timeout) { readPhases(); }

  uint8_t getTTSState();
  void printTTSState();

  void getSFPStatus(uint8_t pFMCId);

  void PrintSlinkStatus();

  uint32_t last_calib_mode_nevents;
  void prepareCalibrationMode(unsigned nevents);

  // JMTBAD get rid of this dumb shit
struct encfifo1hit {
  unsigned ch;
  unsigned roc;
  unsigned dcol;
  unsigned pxl;
  unsigned ph;
  unsigned col;
  unsigned row;
};
struct encfifo1roc {
  unsigned ch;
  unsigned roc;
  unsigned rb;
};
struct encfifo1 {
  bool found;
  unsigned ch; // from tbm header
  unsigned ch_tbm_t;
  unsigned ch_evt_t;
  unsigned tbm_h;
  unsigned event;
  unsigned id_tbm_h;
  unsigned id_tbm_t;
  unsigned id_evt_t;
  unsigned tbm_t1;
  unsigned tbm_t2;
  unsigned marker;
  std::vector<encfifo1roc> rocs;
  std::vector<encfifo1hit> hits;
  encfifo1() : found(false), n_tbm_h(0), n_tbm_t(0), n_roc_h(0) {}
  int n_tbm_h;
  int n_tbm_t;
  int n_roc_h;
  bool ok() { return n_tbm_h == 1 && n_tbm_t == 1 && n_roc_h == 8; }
};
struct digfifo1 {
  std::vector<uint32_t> cFifo1A;
  std::vector<uint32_t> cFifo1B;
  std::vector<uint32_t> cMarkerA;
  std::vector<uint32_t> cMarkerB;
  encfifo1 a;
  encfifo1 b;
  int nonzerowords(int which) {
    int c = 0;
    const std::vector<uint32_t>& f = which == 0 ? cFifo1A : cFifo1B;
    const std::vector<uint32_t>& m = which == 0 ? cMarkerA : cMarkerB;
    assert(f.size() == m.size());
    for (size_t i = 0, ie = f.size(); i < ie; ++i)
      if (f[i] || m[i])
        ++c;
    return c;
  }
  encfifo1& aorb(int i) { return i ? b : a; }
};

  std::vector<uint32_t> readTransparentFIFO();
  int drainTransparentFifo(uint32_t* data);
  void decodeTransparentSymbols (const std::vector<uint32_t>& pInData, std::vector<uint8_t>& p5bSymbol, std::vector<uint8_t>& p5bNRZI, std::vector<uint8_t>& p4bNRZI);
  void prettyPrintTransparentFIFO (const std::vector<uint32_t>& pFifoVec, const std::vector<uint8_t>& p5bSymbol, const std::vector<uint8_t>& p5bNRZI, const std::vector<uint8_t>& p4bNRZI);

  std::vector<uint32_t> readSpyFIFO();
  int drainSpyFifo(uint32_t* data);
  encfifo1 decodeFIFO1Stream(const std::vector<uint32_t>& fifo, const std::vector<uint32_t>& markers);
  void prettyprintFIFO1Stream(const std::vector<uint32_t>& fifo, const std::vector<uint32_t>& markers);
  digfifo1 readFIFO1(bool print);
  int drainFifo1(uint32_t* data);
  int drainTBMFifo(uint32_t* data);
  int drainErrorFifo(uint32_t* data);
  int drainTemperatureFifo(uint32_t* data);
  int drainTTSFifo(uint32_t *data);
  void SelectDaqDDR( uint32_t pNthAcq );
  std::vector<uint32_t> ReadData(uint32_t pBlockSize );
  int spySlink64(uint64_t* data);

  bool isWholeEvent(uint32_t nTries=100000);
  bool isNewEvent(uint32_t nTries=100000);
  int enableSpyMemory(const int enable);

  void printBoardInfo();
  
  int setControlRegister(const int value);
  int loadControlRegister();
  int getControlRegister();

  int setFedIDRegister(const int value);
  int loadFedIDRegister();
  int getFedIDRegister();

  int loadModeRegister();
  int setModeRegister(int value);
  int getModeRegister();

  void set_PrivateWord(uint32_t pword);

  void resetSlink();

  // Check for channels that don't match FEDCard
  // Passes bitsets which indicate which channel(s) didn't match
  // If something doesn't match, it usually indicates an SEU
  bool checkFEDChannelSEU();
  void incrementSEUCountersFromEnbableBits(std::vector<int>&, enbable_t, enbable_t);
  void resetEnbableBits();
  // Check to see if any channel as too many SEUs
  bool checkSEUCounters(int);
  void storeEnbableBits();  
  void resetSEUCountAndDegradeState(void);
  bool runDegraded(void) {return runDegraded_;}

  void testTTSbits(uint32_t data, int enable);

  void setXY(int X, int Y);
  int getXYCount();
  void resetXYCount();

  uint32_t getNumFakeEvents();
  void resetNumFakeEvents();

  int readEventCounter();
  uint32_t getFifoStatus();
  uint32_t linkFullFlag();
  uint32_t numPLLLocks();
  uint32_t getFifoFillLevel();
  uint64_t getSkippedChannels();
  uint32_t getErrorReport(int ch);
  uint32_t getTimeoutReport(int ch);

  int TTCRX_I2C_REG_READ(int Register_Nr);
  int TTCRX_I2C_REG_WRITE(int Register_Nr, int Value);

 private:
  RegManager* const regManager;

  enum { FMC0_Fitel0, FMC0_Fitel1, FMC1_Fitel0, FMC1_Fitel1, nFitels};
  FitelRegMap fRegMap[nFitels];
  int FitelMapNum( int cFMCId, int cFitelId ) const { return 2*cFMCId + cFitelId; }
  std::string fitel_fn_base;
  std::string fRegMapFilename[nFitels];
  FitelRegItem& GetFitelRegItem(const std::string& node, int cFMCId, int cFitelId);
  void LoadFitelRegMap(int cFMCId, int cFitelId);
  void ConfigureFitel(int cFMCId, int cFitelId, bool pVerifLoop);

  int maybeSwapFitelChannels(int ch);
  std::string fitelChannelName(int ch);

  unsigned long long fNthAcq; // for keeping track of reading from ddr0/1
  std::string fStrDDR;
  std::string fStrDDRControl;
  std::string fStrFull;
  std::string fStrReadout;

  // Keep track of the expected status of the FED channels
  // and the status of the FED channels last time we checked
  enbable_t enbable_expected, enbable_last;

  // Keep track of the number of SEUs in each channel
  std::vector<int> num_SEU;

  // keep track of degraded state
  bool runDegraded_;

  uint64_t slink64calls;

  std::vector<uint32_t> bxs;

  std::vector<bool> fibers_in_use;
  void phaseStabilityTest();

  void DumpFitelRegs(int fitel);

  std::vector<std::pair<int, uint64_t> > baddies[5];
};

std::ostream& operator<<(std::ostream& o, const PixelPh1FEDInterface::decoded_phases& p);

#endif
