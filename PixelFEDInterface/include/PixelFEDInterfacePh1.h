#ifndef PixelFEDInterface_PixelFEDInterfacePh1_h
#define PixelFEDInterface_PixelFEDInterfacePh1_h

#include <bitset>
#include <map>
#include <vector>

#include "PixelFEDInterface/include/PixelFEDInterfaceBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFEDCard.h"
#include "PixelUtilities/PixeluTCAUtilities/include/RegManager.h"

class PixelFEDInterfacePh1 : public PixelFEDInterfaceBase {
 public:
  struct FitelRegItem {
    uint8_t fAddress;
    uint8_t fDefValue;
    uint8_t fValue;
    char fPermission;
  };

  typedef std::map < std::string, FitelRegItem > FitelRegMap;

  typedef std::bitset<48> enbable_t;

  PixelFEDInterfacePh1(RegManager*, const std::string&);
  ~PixelFEDInterfacePh1();

  int setup(const std::string& fileName); 
  int setup(pos::PixelFEDCard pfc); 
  int setup();  // run the setup 

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
  bool WriteFitelBlockReg(std::vector<uint32_t>& pVecReq);
  bool ReadFitelBlockReg(std::vector<uint32_t>& pVecReq);
  std::pair<bool, std::vector<double> > ReadADC( const uint8_t pFMCId, const uint8_t pFitelId);

  void loadFPGA(); // (re)Loads the FPGA with the program in the EEPROM
  int reset(); // resets everything
  void resetFED(); // reset FED (LRES + CLRES + fake event,center OOS counters + error fifos)
  void sendResets(unsigned which); // LRES + CLRES on old FED

  // arm OSD readback from roc; wait 31 triggers and call read
  void armOSDFifo(int channel, int rochi, int roclo);
  uint32_t readOSDFifo(int channel);

  void readPhases(bool verbose, bool override_timeout);

  uint32_t last_calib_mode_nevents;
  void prepareCalibrationMode(unsigned nevents);

  // JMTBAD get rid of this dumb shit
struct encfifo1hit {
  unsigned ch;
  unsigned roc;
  unsigned dcol;
  unsigned pxl;
  unsigned ph;
};
struct encfifo1 {
  unsigned event;
  bool found;
  std::vector<encfifo1hit> hits;
  encfifo1() : found(false) {}
};
struct digfifo1 {
  std::vector<uint32_t> cFifo1A;
  std::vector<uint32_t> cFifo1B;
  std::vector<uint32_t> cMarkerA;
  std::vector<uint32_t> cMarkerB;
  encfifo1 a;
  encfifo1 b;
};

  std::vector<uint32_t> readTransparentFIFO();
  int drainTransparentFifo(uint32_t* data);
  std::vector<uint32_t> readSpyFIFO();
  int drainSpyFifo(uint32_t* data);
  digfifo1 readFIFO1();
  encfifo1 prettyprintFIFO1( const std::vector<uint32_t>& pFifoVec, const std::vector<uint32_t>& pMarkerVec, std::ostream& os);
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
  void LoadFitelRegMap(int cFMCId, int cFitelId);
  void ConfigureFitel(int cFMCId, int cFitelId, bool pVerifLoop);

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

  int slink64calls;
};

#endif
