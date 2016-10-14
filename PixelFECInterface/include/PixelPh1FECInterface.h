#ifndef PIXELPH1FECINTERFACE_H
#define PIXELPH1FECINTERFACE_H

// The PixelPh1FECInterface class for VME access to the pixel FEC.
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
#include "PixelUtilities/PixeluTCAUtilities/include/RegManager.h"

class PixelPh1FECInterface: public pos::PixelFECConfigInterface {
    
public:
  PixelPh1FECInterface(RegManager* const RegManagerPtr, const char* boardid);

  bool hasclock();
  bool clocklost();
  void resetttc();
  void resetclocklost();

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
    int getStatus(void);
    unsigned getGeneral();

    int writeCSregister(int mfec, int fecchannel, int cscommand);
    
    void mfecbusy(int mfec, int fecchannel, unsigned int *cs1,unsigned int *cs2);
    void outputblock(const int mfec, const int fecchannel,std::vector<uint32_t> wordcont);
    void outputwordhal(const char *halname, unsigned int data);
    
    int getfecctrlstatus(const int mfec, unsigned long *data);
    int resetdoh(const int mfec, const int fecchannel); // reset the digital opto hybrid
    
    int readback(const int mfec, int channel); // added tbm readout d.k.11/07
    std::vector<uint32_t> readreturn(const int mfec, const int channel, uint32_t size);
    int getByteHubCount(const int mfec, const int channel, const int byte, int *data);
    
    
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
    int rocinit(int NCOLS, int mfec, int mfecchannel, int hubaddress, int portaddress,
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

    unsigned int flipByte(unsigned int input);
	    
private:
    typedef uhal::ValWord<uint32_t> valword;
    typedef uhal::ValVector<uint32_t> valvec;
    RegManager * const pRegManager;

    const std::string board_id;
    
    // ordering mfecs to match designations on vme card
    unsigned long mfecaddress[9];
    
    // cache of mfec CS registers from previous mfecbusy
    unsigned long mfecCSregister[9];
    
    //These are for collecting statistics could be removed (ryd)
    int qbufnsend[9][3];
    int qbufnerr[9][3];
    
    enum {maxbuffersize = 1000, qbufsize = 16384}; // JMTBAD some float needed for 0xFFs etc.
    int qbufn[9][3];
    unsigned char qbuf[9][3][qbufsize];
    
    int qbufn_old[9][3];
    unsigned char qbuf_old[9][3][qbufsize];
    
    //int qbuflasthub[9][3];
    //int qbuflastport[9][3];
    
    int fecdebug;

    std::vector<unsigned char> d25_trimloadtest;
};

#endif
