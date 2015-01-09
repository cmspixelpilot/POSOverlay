#ifndef TP_PIXELFEDINTERFACE_H
#define TP_PIXELFEDINTERFACE_H

// The PixelFEDInterface class for VME access to the pixel FED..
// Uses HAL calls for VME, the direct CAEN access can be still used..
// Will Johns & Danek Kotlinski, 3/06.
//
// Mod. take out the decoding methods 14/4/06 d.k.
// Add the external testDAC oading. 2/5/06 d.k.
// Update fifo readouts to remember the last entry. 11/5/06. d.k.
// Add method to return a copy of PixelFEDCard
    
#include <string>
#include <bitset>
using namespace std;

#define USE_HAL 

#ifdef USE_HAL // Access VME with HALa
#include "VMEDevice.hh"
#else // direct CAEN
#include "CAENVMElib.h"  // CAEN library prototypes
#endif //USE_HAL

#include "CalibFormats/SiPixelObjects/interface/PixelFEDCard.h"


class PixelFEDInterface {

 public:


#ifdef USE_HAL // Access VME with HAL
  
  PixelFEDInterface(const HAL::VMEDevice * const);
  ~PixelFEDInterface();

  // Generic FIFO2 access  
  int drainFifo2(string item, uint32_t offset, 
		 uint32_t *data); // generic

  void test(void);

#else // direct CAEN

  PixelFEDInterface(const uint32_t fedBase, long BHandle);
  ~PixelFEDInterface();
  // Generic FIFO2 access  
  int drainFifo2(uint32_t VmeAddress, uint32_t *data); // generic

#endif // USE_HAL


  // These do not use VME access
  int configFile(string fileName); // readin the DB parameters
  int setupFromDB(string fileName); 
  int setupFromDB(pos::PixelFEDCard pfc); 
  int setup(void);  // run the setup 

  pos::PixelFEDCard& getPixelFEDCard() {return pixelFEDCard;} // return (a reference) to the current pixelFEDCard settings (private data)
  void setPixelFEDCard(pos::PixelFEDCard aPixelFEDCard) {pixelFEDCard=aPixelFEDCard;}

  // Use VME access
  int reset(void); // reset FED (LRES)
  void loadFPGA(); // (re)Loads the FPGA with the program in the EEPROM
//int loadEEPROM(void);// Re-programs the EEPROMs via VME

  int TTCRX_I2C_REG_READ( int Register_Nr); 
  int TTCRX_I2C_REG_WRITE( int Register_Nr, int Value); 

  void set_opto_params(); // setup opto-receiver parameters
  void set_offset_dacs();  // setup ADC offsets
  // Setup clock phase for a specific channel to delay
  int setPhases(const int channel, int delay);  
  int setPhases(void);  // Setup all clock phases according to the stored value
  // Set adc clock delay and phase for a single channel
  int setClockDelayAndPhase( int chan, int delay, int phase);

  void set_clock_phases(); //
  void set_chnl_delays(); //
  void set_chnl_nrocs();//Set the proper number of Rocs for a fed
  void set_blk_ublk_thold(); // Load UB and B limits
  void set_blk_ublk_trans_thold(); // Load UB and B limits for safe transparent mode
  void set_data_levels();  // Load address levels
  void set_chnls_onoff(); // Set Enable channles
  void set_chnls_onoff(int mode); // Enable/Disable channles
  void set_TBMmask(uint32_t mask); //Set common 8bit mask for TBM trlr
  void set_PrivateWord(uint32_t pword); //Set common gap/fill 8bit word
  void set_SpecialDac(uint32_t mode); // Set Special Random DAC mode for RNDM Trigs 1on 0off
  void set_MODE_front(); // Set Mode REGs for front(N,NC,SC,S) FPGAs
  void get_MODE_front(); // Print Mode REGs+ for front(N,NC,SC,S) FPGAs
  void set_adc_1v2v(); // Set Adc Gain from Memory
  void set_adc_1v2v(int mode); // Set ADC 1Vpp or 2Vpp
  void set_adc_1v2v(int mode,int chnl); //Set ADC 1Vpp or 2Vpp by ADC/Channel
  int  get_adc_1v2v(int chnl); // Get ADC 1Vpp (return value 0) or 2Vpp (return value 1) by channel
  void BaselineCorr_on(); // Turn on Baseline correction (whole fed)
  void BaselineCorr_off(); // Turn off Baseline correction (whole fed)
  uint32_t get_BaselineCorrVal(int chnl);//retrieves PixelFEDCard value
  void BaselineCorr_on(int chnl); // Turn on Baseline correction (single channel)
  void BaselineCorr_off(int chnl); // Turn off Baseline correction (single channel)
  void set_BaselineCorr(int chip,uint32_t value); // Set the baseline correction value for an FPGA (1-4)
  void set_BaselineCorr(); //Set the baseline corection values from the database
  void dump_BaselineCorr(); //dumps the baseline corection values from the database and the FED
  void get_BaselineCorr(int * values); //gets the current 36 baseline corection values from the FED
  int get_BaselineCorr(int chnl); //gets the current baseline corection value from the FED for a channel
   uint32_t get_FirmwareDate(int chip);
   uint32_t get_VMEFirmwareDate(void);
  // Cable Test Methods
  void testTTSbits(uint32_t data,int enable);
  int testSlink();

  //set the number of simulated hits/ROC
  void set_SimulatedHits(void) ;
  void set_SimulatedHits(uint32_t data) ;
  void set_Hold(void) ;
  void set_Hold(uint32_t data) ;

  // Fills the three test DACchannels  with three given pulse trains
  void fillDACRegister(vector <uint32_t> pulseTrain_R, vector <uint32_t> pulseTrain_G, 
		       vector <uint32_t> pulseTrain_B) const; 

  void fillDACRegister2(void) const; // Fills the test DAC with a linearly increasing ramp from 100 to 1124
  void fillDACRegister(void) const; // Fill the test DAC 
  void fillDACRegister(const int *const dac1, const int *const dac2, 
		       const int *const dac3) const; // Fill the test DAC
		        
// Fill the test DAC with a finite length pulse train
  void fillDACRegisterLength(const int *const dac1, const int *const dac2, 
                       const int *const dac3,int mlength) const;  
 
  void setup_testDAC(int pedestal);//pre-load the testDAC with a pedestal value
  void stop_testDAC();//send one VME trigger with finite testDAC lengthand LRES/CLRES 


  int setControlRegister(const int value);
  int loadControlRegister(void);
  int getControlRegister(void);

  int setFedIDRegister(const int value);
  int loadFedIDRegister(void);
  int getFedIDRegister(void);

  int loadModeRegister();    // Load Mode Register from DB
  int setModeRegister(int value); // set the mode register to value.
  int getModeRegister(void) const {return pixelFEDCard.modeRegister;} // return ths mode register value
  int enableSpyMemory(int enable); // enable=1, enable spy memory, 0=disable
  void resetSlink();   // change name?

  int generateVMETrigger(void);
  int readBXCounter(); 
  int readEventCounter(); 
  int setVMEevntnmbr(const int value);  // ?

  // For the histohramming memory
  int enableHisMemory(int enable); // enable=1, enable histogramming memories, 0=disable
  void clear_hismem(); 
  int selectTripple(const int trip);
  int drainHisRoc(const int trip,const int Roc,uint32_t *data); 
  int drainHisMemory(uint32_t *data);
  int drainTripple(const int trip,uint32_t *data);
   
  // Read fifos for each channel
  int drainFifo1(int chan,uint32_t *data); 
  int drain_transBuffer(int chan, uint32_t *data); 
  int drainFifo1(int chnl, uint32_t *data, 
		 const uint32_t length); // readout length in bytes 
  int drainDataFifo2(const int chip,uint32_t *data);
  int drainErrorFifo(const int chip,uint32_t *data);
  int drainTemperatureFifo(const int chip, uint32_t *data);

  // Read fifos for all channel (overloded methods)
  int drainFifo1(uint32_t *data); 
  int drain_transBuffer(uint32_t *data); 
  int drainDataFifo2(uint32_t *data);
  int drainErrorFifo(uint32_t *data);
  int drainTemperatureFifo(uint32_t *data);
  int drainLastDACFifo(uint32_t *data);
  int drainTTSFifo(uint32_t *data);
  int drainDataFifo3(uint32_t *data);
  int drainSpyFifo1up(uint32_t *data);
  int drainSpyFifo1dn(uint32_t *data);
  // Unload the spy fifo-3's into a slink type word
  int spySlink64(uint64_t *data);
  //Set CRCchk=true, CRC is checked, mismatch sets count negative, prints warning
  int spySlink64(uint64_t *data,bool CRCchk);
//get all possible 8 bit private words in slink buff, return one 32bit word
//bits order follow channel (bits 0-3 ch 1-9), status = bit on for each good word  
  int PwordSlink64(uint64_t *data,const int length, uint32_t &totword);
  uint32_t getFifoStatus(void);
  void dump_FifoStatus(uint32_t fword);
  void set_Printlevel(int level);//0=critical only, 1=all error,2& =info, 4&param file info
  void set_TTslevels(void);//Sets adjustable TTs consecutive levels for OOS and ERR from Fedcard
  void set_TTslevels(int inmoos,int inmerr);//Sets adjustable TTs consecutive levels for OOS and ERR
                                        //puts values ioos and ierr into Fedcard
  void set_Fifolevels(void);//Sets adjustable fifo-1 Almost full levels from Fedcard
//Sets adjustable fifo-1 Almost full levels,puts values Nfif1,NCfif1,SCfif1,Sfif1 intoFedcard
  void set_Fifo1levels(int Nfif1,int NCfif1,int SCfif1,int Sfif1);
//Sets adjustable fifo-3 Almost full levels,puts values Cfif3 intoFedcard
  void set_Fifo3levels(int Cfif3);
//Sets adjustable fifo-1 hit limits
  void set_HitLimits(void);//Sets adjustable fifo-1 hit limits from Fedcard
//Sets adjustable fifo-1 hit limits,puts values Nlimt,NClimt,SClimt,Slimt intoFedcard
  void set_HitLimits(int Nlimt,int NClimt,int SClimt,int Slimt);
//Sets ROC skip for bbb problem, 1 skip possible per fpga
   void set_ROCskip(void);
//Sets ROC skip for bbb problem, 1 skip possible per fpga, returns -1 for > 1/fpga requested
   int set_ROCskip(int chnl, int roc);     
//Set master delay for TTC signals to FED
	void set_FEDTTCDelay(void);
	int get_FEDTTCDelay(void);
// Feature Register
	void set_FeatureReg(void);
	int get_FeatureReg(void);
//bbb skiped replacement ub
int FixBBB(int chan,uint32_t *data);
	
	
	
//return very last TTS read 
  uint32_t getlastTTS() {return lastTTS;}

  // Is there a new/whole event in FIFO 3? Disable writing to FIFO 3 and poll Centre Chip for given #times
  bool isWholeEvent (uint32_t nTries=100000);
  bool isNewEvent (uint32_t nTries=100000);
  
  // Return internal status of the control registerd 
  int getCcntrl(void) const {return pixelFEDCard.Ccntrl;}

// 2 very stupid bit finding functions for the CRC checker

int lbitval(int ibit,uint64_t wrd)
{uint64_t valb=((0x1ULL<<ibit)&wrd)>>ibit;
int val=(int)valb;
return val; }

int sbitval(int ibit,int wrd)
{int valb=((0x1<<ibit)&wrd)>>ibit;
return valb; }

// XY Mechanism
void setXY( int X, int Y);
int getXYCount();
void resetXYCount();

// Fake event mechanism
int getNumFakeEvents();
void resetNumFakeEvents();

// Check for channels that don't match FEDCard
// Passes bitsets which indicate which channel(s) didn't match
// If something doesn't match, it usually indicates an SEU
bool checkFEDChannelSEU();
void incrementSEUCountersFromEnbableBits(vector<int>&, bitset<9>, bitset<9>);
void resetEnbableBits();
// Check to see if any channel as too many SEUs
bool checkSEUCounters(int);
bool runDegraded;
void storeEnbableBits();

 private:

  // Private methods
#ifdef USE_HAL // Access VME with HAL

  // Generic FIFO2 access  
  int drainFifo2(string item, uint32_t offset, const uint32_t length, 
		 uint32_t *data); // generic
#else // direct CAEN

  int drainFifo2(uint32_t VmeAddress, uint32_t *data); // generic

#endif // end USE_HAL

  
 private:

  // Private variables
  // Channel access 
  uint32_t CHIP[5],CH_SubAddr[9];
  pos::PixelFEDCard pixelFEDCard; // the FED settings
  
  // Keep track of the expected status of the FED channels
  bitset<9> N_enbable_expected, NC_enbable_expected, SC_enbable_expected, S_enbable_expected;
  // Keep track of the status of the FED channels last time we checked
  bitset<9> N_enbable_last, NC_enbable_last, SC_enbable_last, S_enbable_last;

  // Keep track of the number of SEUs in each channel
  vector<int> N_num_SEU, NC_num_SEU, SC_num_SEU, S_num_SEU;

  uint32_t lastErrorValue[8];
  uint32_t lastDACValue[8];
  uint32_t lastTTS;

  //printing
  int Printlevel; //0=critical only, 1=all error,2& =info, 4&param file info
  
#ifdef USE_HAL  // Access VME with HAL
  const HAL::VMEDevice *const vmeDevicePtr;

  string FPGAName[5];  // to hold the VME register name for HAL 

  // Fifo length in BYTES for block readouts
  static const int spyFifo1Length = 4096; // 1024 words
  static const int transFifo1Length = 4096; //1024 words-protects against stale
  static const int spyFifo2Length = 512; // 128 words
  static const int spyFifo3Length = 4096; // 1024 words
  static const int errorFifoLength = 1024; // 256 words
  static const int lastDACFifoLength = 1024; // 256 words
  static const int TTSFifoLength = 512; // 128 words


#else  // Direct with CANE

  // VME Addresses 
  uint32_t FEDBASE; //Fed Base Address
  uint32_t LAD_N; //
  uint32_t LAD_NC;
  uint32_t LAD_SC;
  uint32_t LAD_S;
  uint32_t LAD_BRCST;

  uint32_t LAD_C;

  uint32_t READ_GA;

  uint32_t RES_TTCrx;

  uint32_t LRES;
  uint32_t CLRES;
  uint32_t RESET_SPY_FIFOS;

  uint32_t OPTOPAR_RECEIVERS1;
  uint32_t OPTOPAR_RECEIVERS2;
  uint32_t OPTOPAR_RECEIVERS3;

  uint32_t OFFSET_DAC;
  
  uint32_t DELAY;
  uint32_t PHASE[4];

  uint32_t TEST_DAC;

  uint32_t CTRL;
  uint32_t MODE;
  uint32_t EVENT_NUM;
  uint32_t BX_NUM;
  uint32_t EVENT_TRIG;

  uint32_t WrEventNum;
  uint32_t ClearHist;
  uint32_t EnabHisto;
  uint32_t TripleSelUp;
  uint32_t TripleSelDn;
  uint32_t RocReadUp;
  uint32_t RocReadDn;

  uint32_t I2C_RES;
  uint32_t I2C_LOAD;
  uint32_t I2C_ADDR_RW;
  uint32_t I2C_RD_DATA;
  uint32_t I2C_RD_STAT;

  // For the CAEN VME 
  long BHandle; // pointer to the device
  CVDataWidth dw; // data width (see CAENVMEtypes.h )
  CVAddressModifier am;  // VME address modifier
  CVErrorCodes    ret; // return code

#endif // USE_HAL 

};

#endif // ifdef declare 
