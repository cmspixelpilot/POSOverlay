#ifndef TP_PIXELFEDCARD_H
#define TP_PIXELFEDCARD_H
/**
*   \file CalibFormats/SiPixelObjects/interface/PixelFEDCard.h
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
*  \class PixelFEDCard PixelFEDCard.h
*  \brief This is the documentation about PixelFEDCard...
*
*  The structure which holds all the informations needed to setup 
*  a pixel FED. Danek Kotlinski 18/4/06
*
*  This class can describe both the VME and uTCA FED. Making a separate class
*  or a class hierarchy seemed too cumbersome when looking at how many
*  places it is used w/o encapsulation besides PixelFEDInterface.
*  VME-only params are ignored by PixelFEDInterfacePh1 and vice-versa.
*  Arrays of constants per channel can now go up to 48. The last 16 are just ignored for VME.
*  Where the arrays of constants are not applicable to uTCA, they remain 36 long... let's make it maximally confusing.
*  JMTBAD this isn't great. Please make only the Interfaces know about the different fed cards, then have all the access go through them.
*/
  class PixelFEDCard : public PixelConfigBase{

  public:
    
    //Return true or false depending on if iChannel is used
    //iChannel=1..48 if type == CTA else 1..36
    bool useChannel(unsigned int iChannel);

    //Set iChannel enable to mode
    //iChannel=1..48 if type == CTA else 1..36
    void setChannel(unsigned int iChannel, bool mode);

    void restoreBaselinAndChannelMasks();
    void restoreControlAndModeRegister();


    // Constructor and destructor
    PixelFEDCard(); // empty
    PixelFEDCard(std::string filename); // create from files
    PixelFEDCard(std::vector<std::vector<std::string> > & tab); // create from DB
    ~PixelFEDCard() {};

    void readDBTBMLevels(std::vector<std::vector<std::string> > &tableMat, int first, int last) ;
    void readDBROCLevels(std::vector<std::vector<std::string> > &tableMat, int first, int last) ;
    void writeASCII(std::string dir="") const; // write to files
    void 	 writeXML(      pos::PixelConfigKey key, int version, std::string path)                     const ;
    virtual void writeXMLHeader(pos::PixelConfigKey key, int version, std::string path, std::ofstream *out) const ;
    virtual void writeXML(                                                              std::ofstream *out) const ;
    virtual void writeXMLTrailer(                                                       std::ofstream *out) const ;
    virtual void writeXMLHeader(pos::PixelConfigKey key, 
				int version, 
				std::string path, 
				std::ofstream *fedstream, 
				std::ofstream *roclvlstream, 
				std::ofstream *tbmlvlsteram) const ;
    virtual void writeXML(std::ofstream *fedstream,
			  std::ofstream *rocstream,
			  std::ofstream *tbmstream) const ;
    virtual void writeXMLTrailer(std::ofstream *fedstream,
				 std::ofstream *recostream,
				 std::ofstream *tbmstream) const ;
    uint64_t enabledChannels();  // returns 64-bit integer mask 47..0 for Ph1 FED or 35..0 for Ph0

    enum { VME, VMEPiggy, CTA };
    int type;
    
    //Settable optical input parameters (one for each 12-receiver)
    int opt_cap[3];   // Capacitor adjust
    int opt_inadj[3]; // DC-input offset
    int opt_ouadj[3]; // DC-output offset
  
    //input offset dac (one for each channel)
    //Ph1 FED: these can all be 36-arrays because we don't use them in the code
    int offs_dac[36];
  
    //clock phases, use bits 0-8, select the clock edge
    unsigned int clkphs1_9,clkphs10_18,clkphs19_27,clkphs28_36;

    //Channel delays, one for each channel, 0=15
    int DelayCh[36];
  
    //Blacks and Ultra-blacks, 3 limit per channel
    int BlackHi[36];
    int BlackLo[36];
    int Ublack[36];
    
    //Signal levels for the TBM, one per channel
    int TBM_L0[36],TBM_L1[36],TBM_L2[36],TBM_L3[36],TBM_L4[36];
    int TRL_L0[36],TRL_L1[36],TRL_L2[36],TRL_L3[36],TRL_L4[36];
    // Address levels 1 per channel (36) per roc(max=26)
    int ROC_L0[36][26],ROC_L1[36][26],ROC_L2[36][26],ROC_L3[36][26],
      ROC_L4[36][26];

    //These bits turn off(1) and on(0) channels
    //For VME, lower 9 bits of each are used.
    unsigned int Ncntrl,NCcntrl,SCcntrl,Scntrl;

    // Ph1 FED: same as N/NC/SC/Scntrl, but all in one word. Assumes we won't do 96-ch fed.
    uint64_t cntrl_utca;
    uint64_t cntrl_utca_original;
    int cntrl_utca_override; // use _original instead of respecting the PixelConfigurationVerifier choice

    //The values as read from file so that they can be restored after
    //calibration
    unsigned int Ncntrl_original,NCcntrl_original,SCcntrl_original,Scntrl_original;

     //Bits (1st 8) used to mask TBM trailer bits
    unsigned int N_TBMmask,NC_TBMmask,SC_TBMmask,S_TBMmask;
    
    //Bits (1st 8) used to set the Private Word in the gap and filler words
    unsigned int N_Pword,NC_Pword,SC_Pword,S_Pword;

    //Bits (1st 4) used to set the channel you want to read in spy scope (piggy)
    unsigned int N_ScopeCh, NC_ScopeCh, SC_ScopeCh, S_ScopeCh;

    // 1 = Special Random trigger DAC mode on, 0=off
    unsigned int SpecialDac;
 
    // Control register and delays for the TTCrx
    int CoarseDel,FineDes2Del,FineDes1Del;
    unsigned int ClkDes2; 

    //Main control reg for determining the DAQ mode
    unsigned int Ccntrl; // "CtrlReg" in LAD_C
 
    //Mode register
    int modeRegister; // "ModeReg" in LAD_C
  
    //Number of ROCS per FED channel -- 48 channels for Ph1 FED
    int NRocs[48];

    //Control Regs for setting ADC 1Vpp and 2Vpp
    unsigned int Nadcg,NCadcg,SCadcg,Sadcg;

    //Control and data Regs for setting Baseline Adjustment
    unsigned int Nbaseln,NCbaseln,SCbaseln,Sbaseln;

    //data Regs for TTs adjustable levels
    int Ooslvl,Errlvl;

    //data Regs adjustable fifo Almost Full levels
    int Nfifo1Bzlvl,NCfifo1Bzlvl,SCfifo1Bzlvl,Sfifo1Bzlvl,fifo3Wrnlvl;
		
		//Master delay for FED TTC signals 
		int FedTTCDelay;

    //data Regs adjustable hit limits in fifo1s by fpga
    int N_hitlimit,NC_hitlimit,SC_hitlimit,S_hitlimit;
    
    // data Regs to skip bad ROCs by fpga in old fed, testregs in fed with piggy
    unsigned int N_testreg,NC_testreg,SC_testreg,S_testreg;

    // Ph1 FED: channel you want transparent/scope fifo for
    unsigned TransScopeCh;
    
    //The values as read from file so that they can be restored after
    //calibration
    int Nbaseln_original,NCbaseln_original,SCbaseln_original,
        Sbaseln_original;

    int Ccntrl_original;
    int modeRegister_original;

    //VME base address  = a unique id in the case of uTCA  JMTBAD redundant with fedNumber...
    unsigned long FEDBASE_0, fedNumber;

    // Ph1 FED: something for the DDRs (this will go away)
    unsigned PACKET_NB;

    // Ph1 FED: What position the FMC is in
    int which_FMC;

    // Ph1 FED: whether to swap channel order for fitel
    int swap_Fitel_order;

    // Ph1 FED: whether timeout checking enabled, where to start the counter, and how many timeouts in a row before going OOS 
    int timeout_checking_enabled;
    int timeout_counter_start;
    int timeout_number_oos_threshold;

    // Ph1 FED: disable data from front end to back end
    int frontend_disable_backend;

    // Ph1 FED: acquisition mode: 1: TBM fifo, 2: Slink FIFO, 4: FEROL
    int acq_mode;

    // Ph1 FED: calibration mode on? (JMTBAD can we just keep it on?)
    int calib_mode;

    // Ph1 FED: number of events per ipbus read in calibration mode (JMTBAD only 1 works with the rest of the POS the moment)
    int calib_mode_num_events;

    // Ph1 FED: data type: 0: real data, 1: constants after TBM fifo, 2: pattern before TBM fifo
    int data_type;

    // Ph1 FED: tbm trailer mask: bit = 1 = unmasked. Bit7-0: NoTokenPass - ResetTBM - ResetROC - SyncError - SyncTrigger - ClrTrigCnt - CalTrig - Stackful
    int tbm_trailer_mask;

    // Ph1 FED: tbm trailer mask 2: bit = 1 = unmasked. Bit2-0: WrongNbOfROCs - AutoResetSent - PKAMReseSent
    int tbm_trailer_mask_2;

    // Ph1 FED: private event number
    int private_event_number;

    // Ph1 FED: whether event number error checking is enabled
    int event_count_checking_enabled;

    // Ph1 FED: how many event number errors lead to OOS
    int event_count_num_err_oos;

    // Most recent additions requested by Will and Danek (Dario)
    int BusyHoldMin       ;
    int BusyWhenBehind    ;
    int FeatureRegister   ;
    int FIFO2Limit    	  ;
    int LastDacOff    	  ;
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
 
    // Added by Dario (March 26th 2008)
    void clear(void) ;

  }; // end class PixelFEDCard
}
/* @} */
#endif // ifdef include
