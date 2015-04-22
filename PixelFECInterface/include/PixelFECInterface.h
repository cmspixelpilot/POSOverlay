#ifndef PIXELFECINTERFACE_H
#define PIXELFECINTERFACE_H

// The PixelFECInterface class for VME access to the pixel FEC.
// Uses HAL calls for VME, the direct CAEN access can be still used..
/*
   This is an initial release which supports setting trims on an entire ROC,
   CLRCAL, setting Calibration and Trim bits for pixels individually, adjusting
   WBC and ROC DAC settings.
   doroshenko@physics.rutgers.edu and stone@physics.rutgers.edu 7/31/06
*/
  
#include <string>
#include <vector>
#include <assert.h>

#include "CalibFormats/SiPixelObjects/interface/PixelFECConfigInterface.h"

#define USE_HAL  // to enable HAL use
#define LINUX

#ifdef USE_HAL // Access VME with HALa

#include "VMEDevice.hh"

#else // direct CAEN

#define CSREG     12
#define CH1OUT    0x0
#define CH2OUT    0x10
#define CONTROLREG 28
/* SSID address */
#define SSID 0x304
#include "CAENVMElib.h"  // CAEN library prototypes

#endif //USE_HAL

class TBMReadException: public std::exception {
   public:
	virtual const char* what() const throw() {
		return "Failed to read from TBM";
	}
};

class PixelFECInterface: public pos::PixelFECConfigInterface {

 public:

#ifdef USE_HAL // Access VME with HAL
  
  PixelFECInterface(const HAL::VMEDevice * const, const int vmeslot,
		    unsigned int fecCrate, unsigned int fecSlot);

  ~PixelFECInterface();

  void haltest(void);

  int senddata(const int mfec, const int fecchannel);
  int injectrstroc(const int mfec, const int bitstate);
  int injecttrigger(const int mfec, const int bitstate);
  int injectrsttbm(const int mfec, const int bitstate);
  int injectrstcsr(const int mfec, const int bitstate);
  int enablecallatency(const int mfec, const int bitstate);
  int disableexttrigger(const int mfec, const int bitstate);
  int loopnormtrigger(const int mfec, const int bitstate);
  int loopcaltrigger(const int mfec, const int bitstate);
  int callatencycount(const int mfec, const int latency);
  int FullBufRDaDisable(const int mfec, const int disable);
  int AllRDaDisable(const int mfec, const int disable);

  int getversion(const int mfec, unsigned long *data);
  int getversion(unsigned long *data);
  int setssid(const int ssid);
  int getStatus(void);

  int writeCSregister(int mfec, int fecchannel, int cscommand);
  
  void mfecbusy(int mfec, int fecchannel, unsigned int *cs1,unsigned int *cs2);
  int outputbuffer(const int mfec, const int fecchannel, unsigned long data);
  int outputblock(const int mfec, const int fecchannel,
		  unsigned int *data, int ndata);
  void outputwordhalblock(const char *halname, unsigned int data);
  void outputwordhal(const char *halname, unsigned int data);

  int getfecctrlstatus(const int mfec, unsigned long *data);
  int resetdoh(const int mfec, const int fecchannel); // reset the digital opto hybrid

  int readback(const int mfec, int channel); // added tbm readout d.k.11/07
  int getByteHubCount(const int mfec, const int channel, const int byte, int *data);

#else // direct CAEN

  PixelFECInterface(const unsigned long fedBase, long BHandle,
		    unsigned int fecCrate, unsigned int fecSlot);

  virtual ~PixelFECInterface();

  // Any other function declarations that uses CAEN instead of HAL

  void caentest(void);

  // PFEC Commands, General - Operate on entire mfec module
  int outputbuffer(const int mfec, const int fecchannel, unsigned long data);
  int outputblock(const int mfec, const int fecchannel,
		  unsigned int *data, int ndata); 
  int senddata(const int mfec, const int fecchannel);
  int injectrstroc(const int mfec, const int bitstate);
  int injecttrigger(const int mfec, const int bitstate);
  int injectrsttbm(const int mfec, const int bitstate);
  int injectrstcsr(const int mfec, const int bitstate);
  int enablecallatency(const int mfec, const int bitstate);
  int disableexttrigger(const int mfec, const int bitstate);
  int loopnormtrigger(const int mfec, const int bitstate);
  int loopcaltrigger(const int mfec, const int bitstate);
  int callatencycount(const int mfec, const int latency);
  int getversion(const int mfec, unsigned long *data);
  int getversion(unsigned long *data);
  int FullBufRDaDisable(const int mfec, const int disable);
  int AllRDaDisable(const int mfec, const int disable);

  int resetdoh(const int mfec, const int fecchannel); // reset the digital opto hybrid

  void test(const int vmeslot);

  // PFEC needs to be set to PIXELS mode by setting the SSID register
  // in the TRIGGER FPGA configuration registers (should really use
  // the HAL .dat config for this single register?) 4:PIXELS;1:TRACKER
  int setssid(const int ssid);
  int getStatus(void);

  int writeCSregister(int mfec, int fecchannel, int cscommand);

  void mfecbusy(int mfec, int fecchannel, unsigned int *cs1,unsigned int *cs2);

  int getfecctrlstatus(const int mfec, unsigned long *data);
 
  int readback(const int mfec, int channel); // added tbm readout d.k.11/07
  int getByteHubCount(const int mfec, const int channel, const int byte, int *data);

  // From Wolfram
  int ClkSelect(const int mfec, const int value1){
    uint32_t value0;
    char buf[11];  snprintf(buf,10,"CLKSELM%02d",mfec);
    vmeDevicePtr->read(buf, &value0);
    vmeDevicePtr->write(buf, value1);
    return (int) (0x7fff & value0);
  };

#endif

  // Here goes all the common functions whose declarations don't depend on CAEN or VME.


  int qbufsend(int mfec, int fecchannel);
  int qbufsend(void);
  void fecDebug(int newstate);
  void analyzeFecCSRChan(int mFEC, int ichan, unsigned int ival);
  int clrcal(int mfec, int fecchannel, 
	     int hubaddress, int portaddress, int rocid,
	     bool buffermode=false);
  int tbmcmd(int mfec, int fecchannel, 
	     int tbmchannel, int hubaddress, int portaddress,
	     int offset, int databyte, int direction);
  int tbmread(int mfec, int fecchannel,
             int tbmchannel, int hubaddress, int portaddress,
             int offset);
  /* progpix: program trim and mask bit for an individual pixel */
  int progpix1(int mfec, int mfecchannel, int hubaddress, int portaddress,
	      int rocid,
	      int coladdr, int rowaddress,
	      int mask, int trim,
	      bool buffermode=false);
  int progpix(int mfec, int mfecchannel, int hubaddress, int portaddress,
	      int rocid,
	      int coladdr, int rowaddress,
	      unsigned char databyte,
	      bool buffermode=false);

  /* calpix: set calibration (cap) for an individual pixel */
  int calpix(int mfec, int mfecchannel, int hubaddress, int portaddress,
	     int rocid,
	     int coladdr, int rowaddress,
	     int caldata,
	     bool buffermode=false);

   unsigned char cinttogray(unsigned int igray);

  /* dcol: set a dcolstate (1=on;0=off) to turn on or off double column
     periphery circuitry */
  int dcolenable(int mfec, int mfecchannel,
		 int hubaddress, int portaddress, int rocid,
		 int dcol, int dcolstate,
		 bool buffermode=false);

  /* progdac: Programs dacaddress to value dacvalue. */
  int progdac(int mfec, int fecchannel, 
	      int hubaddress, int portaddress, int rocid,
	      int dacaddress, int dacvalue,
	      bool buffermode=false);

  /* progalldacs: Programs dacaddress to value dacvalue. */
  int progalldacs (int mfec, int fecchannel, 
	     int hubaddress, int portaddress, int rocid,
		   int wbc, int chipcontrol,
		   const std::vector<unsigned char>& alldacs);


  /* rocinit: Initialize mask and trim for all pixels of a roc to that
     same mask/trim value.  */
  int rocinit(int mfec, int mfecchannel, int hubaddress, int portaddress,
	      int rocid,
	      int mask, int trim);

  /* roctrimload: Programs a single ROC's (mask/trim) pixels to the contents of
     a allPixels vector.  */
  int roctrimload(int mfec, int mfecchannel,
		  int hubaddress, int portaddress,
		  int rocid,
		  const std::vector<unsigned char>& allPixels);
  /* coltrimload: Programs 1-12 cols (mask/trim) pixels to the contents of
     a allPixels vector.  */
  int coltrimload(int mfec, int mfecchannel,
		  int hubaddress, int portaddress,
		  int rocid,
		  int startcol, int numcols,
		  const std::vector<unsigned char>& allPixels);

  /* tbmspeed: Sets the tbm to 40(1) or 20(0) MHz */
  /* Note that tbm must be at or above ROC speed */
  int tbmspeed(int mfec, int fecchannel, int tbmchannel, 
	       int hubaddress, int speed);

  //Try to set speed to 40MHz, return 0 if write OK 1 if error
  int tbmspeed2(int mfec, int fecchannel,
		int tbmchannel, int hubaddress, int portaddress);


  int delay25Test(int mymfec, 
		  int myfecchannel, 
		  int myhubaddress, 
		  int mytbmchannel,
		  int myportaddress,
		  int myrocid, 
		  int mymask, 
		  int mytrim,
		  int nTry,
		  int commands,
		  int& success0, 
		  int& success1,
		  int& success2, 
		  int& success3,
		  int& success4);

  /* rocsetchipcontrolregister:  Sets the  ROC Chip Control Register
     which includes: readout speed, chip disable, calibrate signal range */
  int rocsetchipcontrolregister(int mfec, int mfecchannel,
				// int tbmchannel, 
				int hubaddress, int portaddress,
				int rocid,
				int calhighrange,
				int chipdisable,
				int halfspeed,
				const bool buffermode = false);

  /* rocdaqspeed: DELETED 4-11-07, didn't account for calibration range;
     use rocsetchipcontrolregister() instead */

  /* tbmreset: TBM reset*/
  int tbmreset(int mfec, int fecchannel, int tbmchannel, int hubaddress);		 
  /* rocreset: Generate ROC reset*/
  int rocreset(int mfec, int fecchannel, int tbmchannel, int hubaddress);		 
  /* rocsetwbc: Programs the wbc value into a ROC */
  int rocsetwbc(int mfec, int fecchannel, int tbmchannel,
	     int hubaddress, int portaddress, int rocid,
	      int wbcvalue);

  int sendcoltoroc(const int mfec, int fecchannel, int hubaddress,
		   int portaddress, int rocid,
		   int coladdr, int mask, int trim);

 // just for testing:
  int edtest(int mfec, int mfecchannel,
		  int hubaddress, int portaddress,
		  int rocid,
		  const std::vector<unsigned char>& allPixels);


  //Methods from the PixelFECConfigInterface
  void setMaskAndTrimAll(const pos::PixelHdwAddress& theROC,
  			 const std::vector<unsigned char>& allPixels,
			 const bool buffermode=false);

  //void setMaskAndTrimCommon(const pos::PixelHdwAddress& theROC,
  //		    unsigned char maskAndTrim);

  void setDcolEnableAll(const pos::PixelHdwAddress& theROC,
                        unsigned char mask,
                        const bool buffermode = false);

  void setAllDAC(const pos::PixelHdwAddress& theROC, 
		 const std::vector<unsigned int>& dacs,
		 const bool buffermode = false);

  // from wolfram
  int testFiberEnable(const int mfec, const int enable);
  int testFiber(const int mfec, const int channel, int* rda, int * rck);

 private:

#ifdef USE_HAL  // Access VME with HAL
  const HAL::VMEDevice *const vmeDevicePtr;

  int pfecvmeslot;

  //string FPGAName[4];  // to hold the VME register name for HAL 
  
  //string RSTALL1[8]; // { "RSTALL1M8", "RSTALL1M7", "RSTALL1M6", "RSTALL1M5",
  //	"RSTALL1M4", "RSTALL1M3", "RSTALL1M2", "RSTALL1M1" };

  //unsigned long int packword;
  //unsigned long int *ppackword;

#else  // Direct with CAEN

  // VME Addresses 
  unsigned long FEDBASE; //Fed Base Address
  int pfecvmeslot;

  // For the CAEN VME 
  long BHandle; // pointer to the device
  CVDataWidth dw; // data width (see CAENVMEtypes.h )
  CVAddressModifier am;  // VME address modifier
  CVErrorCodes    ret; // return code

#endif // USE_HAL 

  // ordering mfecs to match designations on vme card
  unsigned long mfecaddress[9];

  // cache of mfec CS registers from previous mfecbusy
  unsigned long mfecCSregister[9];

  //These are for collecting statistics could be removed (ryd)
  int qbufnsend[9][3];
  int qbufnerr[9][3];

  int qbufn[9][3];
  unsigned char qbuf[9][3][1024];

  int qbufn_old[9][3];
  unsigned char qbuf_old[9][3][1024];

  //int qbuflasthub[9][3];
  //int qbuflastport[9][3];

  //max buffer size; set in constructor. (has to be less than ~1000).
  int maxbuffersize_;

  int fecdebug;

  //The crate and slot number
  unsigned int fecCrate_;
  unsigned int fecSlot_;

};

#endif // ifdef declare 
