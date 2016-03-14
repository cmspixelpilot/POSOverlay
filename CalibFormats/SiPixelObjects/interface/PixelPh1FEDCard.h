#ifndef TP_PIXELPH1FEDCARD_H
#define TP_PIXELPH1FEDCARD_H
/**
*   \file CalibFormats/SiPixelObjects/interface/PixelPh1FEDCard.h
*   \brief This class implements..
*
*   A longer explanation will be placed here later
*/

#include "CalibFormats/SiPixelObjects/interface/PixelConfigBase.h"

#include <vector>
#include <string>
#include <stdint.h>

namespace pos{
/*!  \ingroup ConfigurationObjects "Configuration Objects"
*    
*  @{
*
*  \class PixelPh1FEDCard PixelPh1FEDCard.h
*  \brief This is the documentation about PixelPh1FEDCard...
*
*  The structure which holds all the informations needed to setup 
*  a pixel FED. Danek Kotlinski 18/4/06
*/
  class PixelPh1FEDCard : public PixelConfigBase{

  public:
    PixelPh1FEDCard(); // empty
    PixelPh1FEDCard(std::string filename); // create from files
    PixelPh1FEDCard(std::vector<std::vector<std::string> > & tab); // create from DB
    ~PixelPh1FEDCard() {};

    void writeASCII(std::string dir="") const; // write to files

    uint64_t enabledChannels();  // returns 64-bit integer mask
    bool useChannel(unsigned int iChannel);
    void setChannel(unsigned int iChannel, bool mode);
    void restoreChannelMasks();
    void restoreControlAndModeRegister();

    //VME base address  JMTBAD just a unique id now
    unsigned long FEDBASE_0, fedNumber;

    //Number of ROCS per FED channel
    int NRocs[48];

    // enbable bits
    uint32_t cntrl_1;
    uint32_t cntrl_2;
    uint32_t cntrl_3;
    uint32_t cntrl_1_original;
    uint32_t cntrl_2_original;
    uint32_t cntrl_3_original;

    // Control register and delays for the TTCrx
    int CoarseDel,FineDes2Del,FineDes1Del;
    unsigned int ClkDes2; 

    uint32_t Ccntrl;
    uint32_t Ccntrl_original;
    uint32_t modeRegister;
    uint32_t modeRegister_original;

    // ADC gain/range settings 1Vpp(0), 2Vpp(1)
    uint64_t adcg;

    // trailer mask
    uint64_t TBMmask;

    // Private fill/gap word
    uint32_t Pword;

    // channel you want transparent/scope fifo for
    unsigned TransScopeCh;

    // # consecutive out of syncs, empty events until tts error
    int Ooslvl,Errlvl;

    //data Regs adjustable fifo Almost Full level (JMTBAD still a thing?)
    int fifo1Bzlvl;

    // almost full level for fifo3
    int fifo3Wrnlvl;

    //Master delay for FED TTC signals 
    int FedTTCDelay;

    //data Regs adjustable hit limits in fifo1 by fpga
    int hitlimit;
    
    // was roc skips, what now?
    uint32_t testreg;

    // for setting the ddr data size expectation -- will we need this
    uint32_t packet_nb;

    int BusyHoldMin       ;
    int BusyWhenBehind    ;
    uint32_t FeatureRegister   ;
    int FIFO2Limit    	  ;
    int SimHitsPerRoc 	  ;
    int TimeoutOROOSLimit ;
    int TriggerHoldoff    ;
    int SPARE1  	  ;
    int SPARE2  	  ;
    int SPARE3  	  ;
    int SPARE4  	  ;
    int SPARE5  	  ;
    int SPARE6  	  ;
    int SPARE7  	  ;
    int SPARE8  	  ;
    int SPARE9  	  ;
    int SPARE10 	  ;
    
 private: 
    void clear();

  };
}
/* @} */

#endif
