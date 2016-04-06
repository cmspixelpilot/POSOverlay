#ifndef PixelFEDInterface_PixelFEDInterfaceBase_h
#define PixelFEDInterface_PixelFEDInterfaceBase_h

class PixelFEDInterfaceBase {
 public:
  PixelFEDInterfaceBase();
  virtual ~PixelFEDInterfaceBase();

  virtual void setPrintlevel(int level) = 0;
  virtual int getPrintlevel() const = 0;

  virtual int setup(const std::string& fileName) = 0; 
  virtual int setup() = 0;

  virtual void printBoardInfo() = 0; // fedid, firmware dates, network config, etc.

  virtual void loadFPGA() = 0; // (re)Loads the FPGA with the program in the EEPROM
  virtual void reset() = 0; // resets everything
  virtual void resetFED() = 0; // reset FED (LRES + CLRES + fake event,center OOS counters + error fifos)

  virtual void armOSDFifo(int channel, int rochi, int roclo) = 0;
  virtual uint32_t readOSDFifo(int channel) = 0;

  virtual void readPhases(bool verbose, bool override_timeout) = 0;

  virtual int drainTransparentFifo(uint32_t* data) = 0;
  virtual int drainSpyFifo(uint32_t* data) = 0;
  virtual int drainFifo1(uint32_t* data) = 0;
  virtual int drainTBMFifo(uint32_t* data) = 0;
  virtual int drainErrorFifo(uint32_t* data) = 0;
  virtual int drainTemperatureFifo(uint32_t* data) = 0;
  virtual int drainTTSFifo(uint32_t *data) = 0;
  virtual int spySlink64(uint64_t* data) = 0;

  virtual bool isWholeEvent(uint32_t nTries) = 0;
  virtual bool isNewEvent(uint32_t nTries) = 0;
  virtual int enableSpyMemory(const int enable) = 0;

  virtual bool setFedIDRegister(uint32_t value) = 0;
  virtual uint32_t getFedIDRegister() = 0;

  virtual bool loadControlRegister() = 0;
  virtual bool setControlRegister(uint32_t value) = 0;
  virtual uint32_t getControlRegister() = 0;

  virtual bool loadModeRegister() = 0;
  virtual bool setModeRegister(uint32_t value) = 0;
  virtual uint32_t getModeRegister() = 0;

  virtual void setPrivateWord(uint32_t pword) = 0;

  virtual void resetSlink() = 0;

  virtual bool checkFEDChannelSEU() = 0;
  virtual void resetEnbableBits() = 0;
  virtual bool checkSEUCounters(int) = 0;
  virtual void storeEnbableBits() = 0;  
  virtual void resetSEUCountAndDegradeState(void) = 0;
  virtual bool runDegraded() = 0;

  virtual void testTTSbits(uint32_t data, int enable) = 0;

  virtual void setXY(int X, int Y) = 0;
  virtual int getXYCount() = 0;
  virtual void resetXYCount() = 0;

  virtual int getNumFakeEvents() = 0;
  virtual void resetNumFakeEvents() = 0;

  virtual uint32_t readEventCounter() = 0;
  virtual uint32_t getFifoStatus() = 0;
  virtual uint32_t linkFullFlag() = 0;
  virtual uint32_t numPLLLocks() = 0;
  virtual uint32_t getFifoFillLevel() = 0;
  virtual uint64_t getSkippedChannels() = 0;
  virtual uint32_t getErrorReport(int ch) = 0;
  virtual uint32_t getTimeoutReport(int ch) = 0;

  virtual int TTCRX_I2C_REG_READ(int Register_Nr) = 0;
  virtual int TTCRX_I2C_REG_WRITE(int Register_Nr, int Value) = 0;
};

#endif
