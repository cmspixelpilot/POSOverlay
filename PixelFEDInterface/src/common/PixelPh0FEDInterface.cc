// PixelPh0FEDInterface class to access VME funtions of the PixelFED
// Uses HAL or direct calls to the CAEN interface.
// Will Johns & Danek Kotlinski, 3/06.
// 
// Correct the bug in address level loading. 19/04/06, d.k.
// Change to the event counter. 26/04/06, d.k.
// Add Fifo readouts for all channels. 28/04/06. d.k.
// Add brackets.

#include <iostream>
#include <time.h>
#include <assert.h>
#include <unistd.h> // for usleep()

#include "PixelFEDInterface/include/PixelPh0FEDInterface.h"
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"
#include "CAENVMElib.h"
using namespace std;
using namespace pos;

namespace {
  //const bool PRINT = false;
  //const bool PRINT = true;
}

#define PILOT_FED

#ifdef USE_HAL // Access VME with HAL

//// Constructor //////////////////////////////////////////////////
PixelPh0FEDInterface::PixelPh0FEDInterface(const HAL::VMEDevice * const vmeDeviceP ) : 
  runDegraded_(false), vmeDevicePtr(vmeDeviceP)  {

  cout<<" PixelPh0FEDInterface constructor "<<endl;
  Printlevel=1;
  printIfSlinkHeaderMessedup = true;
  // Initilize the FPGA register names for HAL, there is probably a better way of doing it
  FPGAName[0] = "LAD_N"; //
  FPGAName[1] = "LAD_NC"; //
  FPGAName[2] = "LAD_SC"; //
  FPGAName[3] = "LAD_S"; //
  FPGAName[4] = "LAD_C"; //

  CHIP[0]=0x0;      // LAD_N  ch 1 -9
  CHIP[1]=0x200000; // LAD_NC ch 10-18
  CHIP[2]=0x400000; // LAD_SC ch 19-27
  CHIP[3]=0x600000; // LAD_S  ch 28-36
  CHIP[4]=0x800000; // LAD_S  ch 28-36
  
  // Channel addresses within one FPGA 
  // Beware the definition of these parameters is different from the direct CAEN 
  // version.
  CH_SubAddr[0]= 0x20000;  // ch1
  CH_SubAddr[1]= 0x40000;  // ch2
  CH_SubAddr[2]= 0x60000;  // ch3
  CH_SubAddr[3]= 0x80000;  // ch4
  CH_SubAddr[4]= 0xa0000;  // ch5
  CH_SubAddr[5]= 0xc0000;  // ch6
  CH_SubAddr[6]= 0xe0000;  // ch7
  CH_SubAddr[7]= 0x100000; // ch8
  CH_SubAddr[8]= 0x120000; // ch9

  // Init the stored fifo values
  lastTTS=0xffffffff;
  for(int i=0;i<8;++i) {
    lastErrorValue[i]=0x0;
    lastDACValue[i]  =0x0;
  }

  // Initialize the SEU counters
  for(int i=0; i < 9; i++) {
    N_num_SEU.push_back(0);
    NC_num_SEU.push_back(0);
    SC_num_SEU.push_back(0);
    S_num_SEU.push_back(0);
  }

  assert(N_num_SEU.size()==9);

  DauCards_lastStatusPoll = 0;
}
//////////////////////////////////////////////////////////////////////
PixelPh0FEDInterface::~PixelPh0FEDInterface(void) {
  cout<<" PixelPh0FEDInterface destructor "<<endl;
}
//////////////////////////////////////////////////////////////////////
// Testing only
void PixelPh0FEDInterface::test(void) {
  cout<<"PixelPh0FEDInterface::test "<<endl;
  uint32_t value;
  vmeDevicePtr->read("READ_GA", &value );
  cout<<" GA = 0x"<<hex<<value<<dec<<endl;
}
/////////////////////////////////////////////////////////////////////////

#else // Direct CAEN VME


PixelPh0FEDInterface::PixelPh0FEDInterface(const uint32_t fedBase, long aBHandle) {
  // For the CAEN interface
  BHandle = aBHandle;  // store the VME pointer 
  dw = cvD32; // data width (see CAENVMEtypes.h )
  am = cvA32_U_DATA;

  FEDBASE = fedBase; // FED base address

  cout<<" PixelPh0FEDInterface constructor "<<hex<<FEDBASE<<" "<<BHandle<<dec<<endl;

  // Define the FED registers
  LAD_N       = (FEDBASE);          // N-Altera
  LAD_NC      = (FEDBASE+0x200000); // NC-Altera
  LAD_SC      = (FEDBASE+0x400000); // SC-Altera
  LAD_S       = (FEDBASE+0x600000); // S-Altera
  LAD_BRCST   = (FEDBASE+0x700000); // Broadcast address
  LAD_C       = (FEDBASE+0x800000); // C-Altera

  READ_GA     = (FEDBASE+0xa00000); //

  I2C_RD_STAT = (FEDBASE+0xa00014);
  I2C_RES     = (FEDBASE+0xa00008);
  I2C_LOAD    = (FEDBASE+0xa0000c);
  I2C_ADDR_RW = (FEDBASE+0xa00010);
  I2C_RD_DATA = (FEDBASE+0xa00010);

  RES_TTCrx   = (FEDBASE+0xa00038); // Access TTCrx chip

  LRES        = (FEDBASE+0xa00000);
  CLRES       = (FEDBASE+0xa00004);
  nCONFIG     = (FEDBASE+0xa00018);
  RESET_SPY_FIFOS = (LAD_BRCST+0x1c8000);

  OPTOPAR_RECEIVERS1 = (LAD_S+0x180000);
  OPTOPAR_RECEIVERS2 = (LAD_S+0x188000);
  OPTOPAR_RECEIVERS3 = (LAD_S+0x190000);

  OFFSET_DAC = (LAD_N+0x190000);
  
  DELAY = (LAD_C+0x198000);
  PHASE[0] = (LAD_N +0x1b0000);
  PHASE[1] = (LAD_NC+0x1b0000);
  PHASE[2] = (LAD_SC+0x1b0000);
  PHASE[3] = (LAD_S +0x1b0000);

  TEST_DAC = (LAD_N+0x180000);

  CTRL       = (LAD_C+0x1a0000);
  MODE       = (LAD_C+0x1c0000);
  EVENT_NUM  = (LAD_C+0x190000);
  BX_NUM     = (LAD_C+0x188000);
  EVENT_TRIG = (LAD_C+0x180000);

  WrEventNum  = (LAD_C+0x180000);
  ClearHist   = (LAD_C+0x20000);
  EnabHisto   = (LAD_C+0x28000);
  TripleSelUp = (LAD_C+0x30000);
  TripleSelDn = (LAD_C+0x38000);
  RocReadUp   = (LAD_C+0x40000);
  RocReadDn   = (LAD_C+0x48000);

  CHIP[0]=LAD_N;  //ch 1-9
  CHIP[1]=LAD_NC; //ch 10-18
  CHIP[2]=LAD_SC; //ch 19-27
  CHIP[3]=LAD_S;  //ch 28-36
  
  CH_SubAddr[0]= 0x38000;  //FifoI ch1
  CH_SubAddr[1]= 0x58000;  //FifoI ch2
  CH_SubAddr[2]= 0x78000;  //FifoI ch3
  CH_SubAddr[3]= 0x98000;  //FifoI ch4
  CH_SubAddr[4]= 0xb8000;  //FifoI ch5
  CH_SubAddr[5]= 0xd8000;  //FifoI ch6
  CH_SubAddr[6]= 0xf8000;  //FifoI ch7
  CH_SubAddr[7]= 0x118000; //FifoI ch8
  CH_SubAddr[8]= 0x138000; //FifoI ch9
  
  // New Addresses For Daughter Cards
  TopDauCard_nConfig         = LAD_N+0x1b8000;
  BottomDauCard_nConfig      = LAD_S+0x1b8000;
  TopDauCard_pll             = LAD_NC+0x1c0000;
  BottomDauCard_pll          = LAD_S+0x1c0000;
  TopDauCard_com             = LAD_N+0x1a8000;
  BottomDauCard_com          = LAD_S+0x1a8000;
  TopDauCard_UpStatus        = LAD_N+0x158000;
  TopDauCard_DownStatus      = LAD_N+0x178000;
  BottomDauCard_UpStatus     = LAD_S+0x158000;
  BottomDauCard_DnStatus     = LAD_S+0x178000;
  TopDauCard_UpTempFifo      = LAD_NC+0x148000;
  TopDauCard_DownTempFifo    = LAD_NC+0x168000;
  BottomDauCard_UpTmpFifo    = LAD_S+0x148000;
  BottomDauCard_DnTmpFifo    = LAD_S+0x168000;
  DauCards_lastStatusPoll = 0;
}
//////////////////////////////////////////////////////////////////////
PixelPh0FEDInterface::~PixelPh0FEDInterface(void) {
  cout<<" PixelPh0FEDInterface destructor "<<endl;
}

extern void analyzeError(CVErrorCodes ret); // Wills VME error analyzer.

# endif // USE_HAL


////////////////////////////////////////////////////////////////////////////
// Read the configuration parameters from file
int PixelPh0FEDInterface::configFile(string fileName) {
  assert(0);

  int i;
  int DEBUG=0;
  int ijx,ijy;
  if(Printlevel&4)DEBUG=1;
  cout<<" Get setup parameters from file "<<fileName<<endl;
  FILE *infile = fopen((fileName.c_str()),"r");
  if (infile == NULL) {
    cout<<"No parameter file!"<<fileName<<endl; 
    return(-1);
  }
  
  //Fed Base Address
  unsigned long FEDBASE_0,fedNumber;//******************these need unsigned long**************
  fscanf(infile,"FED Base address                         :%lx\n",
	 &FEDBASE_0);
  fscanf(infile,"FEDID Number                             :%lx\n",
	 &fedNumber);
  printf("FED Base address     :%lx\n",FEDBASE_0);
  printf("FED ID number        :%ld\n",fedNumber);

  //if(FEDBASE != FEDBASE_0) cout<<" Inconsistent FED base address?"<<endl;
 
  // Number of ROCs
  for(i=0;i<36;i++){
    fscanf(infile,"Number of ROCs Chnl %d:%d \n",&ijx,&pixelFEDCard.NRocs[i]);
    if(DEBUG==1)printf("Number of ROCs per Chnl %d:%d \n",ijx,pixelFEDCard.NRocs[i]);
  }


  //Settable optical input parameters
  fscanf(infile,"Optical reciever 1  Capacitor Adjust(0-3):%d\n",&pixelFEDCard.opt_cap[0]);
  fscanf(infile,"Optical reciever 2  Capacitor Adjust(0-3):%d\n",&pixelFEDCard.opt_cap[1]);
  fscanf(infile,"Optical reciever 3  Capacitor Adjust(0-3):%d\n",&pixelFEDCard.opt_cap[2]);
  fscanf(infile,"Optical reciever 1  Input Offset (0-15)  :%d\n",&pixelFEDCard.opt_inadj[0]);
  fscanf(infile,"Optical reciever 2  Input Offset (0-15)  :%d\n",&pixelFEDCard.opt_inadj[1]);
  fscanf(infile,"Optical reciever 3  Input Offset (0-15)  :%d\n",&pixelFEDCard.opt_inadj[2]);
  fscanf(infile,"Optical reciever 1 Output Offset (0-3)   :%d\n",&pixelFEDCard.opt_ouadj[0]);
  fscanf(infile,"Optical reciever 2 Output Offset (0-3)   :%d\n",&pixelFEDCard.opt_ouadj[1]);
  fscanf(infile,"Optical reciever 3 Output Offset (0-3)   :%d\n",&pixelFEDCard.opt_ouadj[2]);
  
  if(DEBUG==1)printf("Optical reciever 1  Capacitor Adjust(0-3):%d\n",pixelFEDCard.opt_cap[0]);
  if(DEBUG==1)printf("Optical reciever 2  Capacitor Adjust(0-3):%d\n",pixelFEDCard.opt_cap[1]);
  if(DEBUG==1)printf("Optical reciever 3  Capacitor Adjust(0-3):%d\n",pixelFEDCard.opt_cap[2]);
  if(DEBUG==1)printf("Optical reciever 1  Input Offset (0-15)   :%d\n",pixelFEDCard.opt_inadj[0]);
  if(DEBUG==1)printf("Optical reciever 2  Input Offset (0-15)   :%d\n",pixelFEDCard.opt_inadj[1]);
  if(DEBUG==1)printf("Optical reciever 3  Input Offset (0-15)   :%d\n",pixelFEDCard.opt_inadj[2]);
  if(DEBUG==1)printf("Optical reciever 1 Output Offset (0-3)  :%d\n",pixelFEDCard.opt_ouadj[0]);
  if(DEBUG==1)printf("Optical reciever 2 Output Offset (0-3)  :%d\n",pixelFEDCard.opt_ouadj[1]);
  if(DEBUG==1)printf("Optical reciever 3 Output Offset (0-3)  :%d\n",pixelFEDCard.opt_ouadj[2]);
  
  //input offset dac
  for(int i=0;i<36;i++)fscanf(infile,"Offset DAC channel %d:%d\n",&ijx,&pixelFEDCard.offs_dac[i]);
  if(DEBUG==1){for(int i=0;i<36;i++)printf("Offset DAC channel %d:%d\n",i+1,pixelFEDCard.offs_dac[i]);}
  
  //clock phases
  fscanf(infile,"Clock Phase Bits ch   1-9:%x\n",& pixelFEDCard.clkphs1_9 );
  fscanf(infile,"Clock Phase Bits ch 10-18:%x\n",&pixelFEDCard.clkphs10_18);
  fscanf(infile,"Clock Phase Bits ch 19-27:%x\n",&pixelFEDCard.clkphs19_27);
  fscanf(infile,"Clock Phase Bits ch 28-36:%x\n",&pixelFEDCard.clkphs28_36);
  if(DEBUG==1)printf("Clock Phase Bits ch    1-9:%x\n",pixelFEDCard.clkphs1_9 );
  if(DEBUG==1)printf("Clock Phase Bits ch  10-18:%x\n",pixelFEDCard.clkphs10_18 );
  if(DEBUG==1)printf("Clock Phase Bits ch  19-27:%x\n",pixelFEDCard.clkphs19_27 );
  if(DEBUG==1)printf("Clock Phase Bits ch  28-36:%x\n",pixelFEDCard.clkphs28_36 );
  
  //Blacks 
  for(i=0;i<36;i++){
    fscanf(infile,"Black HiThold ch %d:%d \n",&ijx,&pixelFEDCard.BlackHi[i]);
    fscanf(infile,"Black LoThold ch %d:%d \n",&ijx,&pixelFEDCard.BlackLo[i]);
    fscanf(infile,"ULblack Thold ch %d:%d \n",&ijx, &pixelFEDCard.Ublack[i]);
    if(DEBUG==1)printf("Black HiThold ch %d:%d\n",ijx,pixelFEDCard.BlackHi[i]);
    if(DEBUG==1)printf("Black LoThold ch %d:%d\n",ijx,pixelFEDCard.BlackLo[i]);
    if(DEBUG==1)printf("ULblack Thold ch %d:%d\n",ijx, pixelFEDCard.Ublack[i]);
  }
  
  //Channel delays
  for(i=0;i<36;i++) {fscanf(infile,"Delay channel %d(0-15):%d\n",&ijx,&pixelFEDCard.DelayCh[i]);}
  if(DEBUG==1){for(i=0;i<36;i++){printf("Delay channel %d(0-15):%d\n",i+1,pixelFEDCard.DelayCh[i]);}}
  
  //Signal levels
  for(i=0;i<36;i++) {
    fscanf(infile,"TBM level 0 Channel  %d:%d\n",&ijx,&pixelFEDCard.TBM_L0[i]);
    fscanf(infile,"TBM level 1 Channel  %d:%d\n",&ijx,&pixelFEDCard.TBM_L1[i]);
    fscanf(infile,"TBM level 2 Channel  %d:%d\n",&ijx,&pixelFEDCard.TBM_L2[i]);
    fscanf(infile,"TBM level 3 Channel  %d:%d\n",&ijx,&pixelFEDCard.TBM_L3[i]);
    fscanf(infile,"TBM level 4 Channel  %d:%d\n",&ijx,&pixelFEDCard.TBM_L4[i]);
    if(DEBUG==1)printf("TBM level 0 Channel  %d:%d\n",ijx,pixelFEDCard.TBM_L0[i]);
    if(DEBUG==1)printf("TBM level 1 Channel  %d:%d\n",ijx,pixelFEDCard.TBM_L1[i]);
    if(DEBUG==1)printf("TBM level 2 Channel  %d:%d\n",ijx,pixelFEDCard.TBM_L2[i]);
    if(DEBUG==1)printf("TBM level 3 Channel  %d:%d\n",ijx,pixelFEDCard.TBM_L3[i]);
    if(DEBUG==1)printf("TBM level 4 Channel  %d:%d\n",ijx,pixelFEDCard.TBM_L4[i]);
    
    for(int j=0;j<pixelFEDCard.NRocs[i];j++) {
      fscanf(infile,"ROC%d level 0 Channel  %d :%d\n",&ijy,&ijx,&pixelFEDCard.ROC_L0[i][j]);
      fscanf(infile,"ROC%d level 1 Channel  %d :%d\n",&ijy,&ijx,&pixelFEDCard.ROC_L1[i][j]);
      fscanf(infile,"ROC%d level 2 Channel  %d :%d\n",&ijy,&ijx,&pixelFEDCard.ROC_L2[i][j]);
      fscanf(infile,"ROC%d level 3 Channel  %d :%d\n",&ijy,&ijx,&pixelFEDCard.ROC_L3[i][j]);
      fscanf(infile,"ROC%d level 4 Channel  %d :%d\n",&ijy,&ijx,&pixelFEDCard.ROC_L4[i][j]);
      if(DEBUG==1)printf("ROC%d level 0 Channel  %d :%d\n",ijy,ijx,pixelFEDCard.ROC_L0[i][j]);
      if(DEBUG==1)printf("ROC%d level 1 Channel  %d :%d\n",ijy,ijx,pixelFEDCard.ROC_L1[i][j]);
      if(DEBUG==1)printf("ROC%d level 2 Channel  %d :%d\n",ijy,ijx,pixelFEDCard.ROC_L2[i][j]);
      if(DEBUG==1)printf("ROC%d level 3 Channel  %d :%d\n",ijy,ijx,pixelFEDCard.ROC_L3[i][j]);
      if(DEBUG==1)printf("ROC%d level 4 Channel  %d :%d\n",ijy,ijx,pixelFEDCard.ROC_L4[i][j]);
    }
      
    fscanf(infile,"TRLR level 0 Channel %d:%d\n",&ijx,&pixelFEDCard.TRL_L0[i]);
    fscanf(infile,"TRLR level 1 Channel %d:%d\n",&ijx,&pixelFEDCard.TRL_L1[i]);
    fscanf(infile,"TRLR level 2 Channel %d:%d\n",&ijx,&pixelFEDCard.TRL_L2[i]);
    fscanf(infile,"TRLR level 3 Channel %d:%d\n",&ijx,&pixelFEDCard.TRL_L3[i]);
    fscanf(infile,"TRLR level 4 Channel %d:%d\n",&ijx,&pixelFEDCard.TRL_L4[i]);
    if(DEBUG==1)printf("TRLR level 0 Channel %d:%d\n",ijx,pixelFEDCard.TRL_L0[i]);
    if(DEBUG==1)printf("TRLR level 1 Channel %d:%d\n",ijx,pixelFEDCard.TRL_L1[i]);
    if(DEBUG==1)printf("TRLR level 2 Channel %d:%d\n",ijx,pixelFEDCard.TRL_L2[i]);
    if(DEBUG==1)printf("TRLR level 3 Channel %d:%d\n",ijx,pixelFEDCard.TRL_L3[i]);
    if(DEBUG==1)printf("TRLR level 4 Channel %d:%d\n",ijx,pixelFEDCard.TRL_L4[i]);
  }
  
  
  //These bits turn off(1) and on(0) channels
  fscanf(infile,"Channel Enbable bits chnls 1-9  (on = 0):%x\n",
	 &pixelFEDCard.Ncntrl);
  fscanf(infile,"Channel Enbable bits chnls 10-18(on = 0):%x\n",
	 &pixelFEDCard.NCcntrl);
  fscanf(infile,"Channel Enbable bits chnls 19-27(on = 0):%x\n",
	 &pixelFEDCard.SCcntrl);
  fscanf(infile,"Channel Enbable bits chnls 28-36(on = 0):%x\n",
	 &pixelFEDCard.Scntrl);
  if(DEBUG==1)
    printf("Channel Enbable bits chnls 1-9  (on = 0):%x\n",pixelFEDCard.Ncntrl);
  if(DEBUG==1)
    printf("Channel Enbable bits chnls 10-18(on = 0):%x\n",pixelFEDCard.NCcntrl);
  if(DEBUG==1)
    printf("Channel Enbable bits chnls 19-27(on = 0):%x\n",pixelFEDCard.SCcntrl);
  if(DEBUG==1)
    printf("Channel Enbable bits chnls 28-36(on = 0):%x\n",pixelFEDCard.Scntrl);
  
  //These are delays to the TTCrx
  fscanf(infile,"TTCrx Coarse Delay Register 2:%d\n",&pixelFEDCard.CoarseDel);
  fscanf(infile,"TTCrc      ClkDes2 Register 3:%x\n",&pixelFEDCard.ClkDes2);
  fscanf(infile,"TTCrc Fine Dlay ClkDes2 Reg 1:%d\n",&pixelFEDCard.FineDes2Del);

  if(DEBUG==1)printf("TTCrx Coarse Delay Register 2:%d\n",pixelFEDCard.CoarseDel);
  if(DEBUG==1)printf("TTCrc	   ClkDes2 Register 3:%x\n",pixelFEDCard.ClkDes2);
  if(DEBUG==1)printf("TTCrc Fine Dlay ClkDes2 Reg 1:%d\n",pixelFEDCard.FineDes2Del);


  // Control register
  fscanf(infile,"Center Chip Control Reg:%x\n",&pixelFEDCard.Ccntrl);
  printf("Control Reg:0x%x\n",pixelFEDCard.Ccntrl);
  fscanf(infile,"Initial Slink DAQ mode:%d\n",&pixelFEDCard.modeRegister);
  printf("Mode Reg:%d\n",pixelFEDCard.modeRegister);

   //These bits set ADC Gain/Range 1Vpp(0) and 2Vpp(1) for channels
  fscanf(infile,"Channel ADC Gain bits chnls  1-12(1Vpp = 0):%x\n",
         &pixelFEDCard.Nadcg);
  fscanf(infile,"Channel ADC Gain bits chnls 13-20(1Vpp = 0):%x\n",
         &pixelFEDCard.NCadcg);
  fscanf(infile,"Channel ADC Gain bits chnls 21-28(1Vpp = 0):%x\n",
         &pixelFEDCard.SCadcg);
  fscanf(infile,"Channel ADC Gain bits chnls 29-36(1Vpp = 0):%x\n",
         &pixelFEDCard.Sadcg);
  if(DEBUG)
    printf("Channel ADC Gain bits chnls  1-12(1Vpp = 0):%x\n",pixelFEDCard.Nadcg);
  if(DEBUG)
    printf("Channel ADC Gain bits chnls 13-20(1Vpp = 0):%x\n",pixelFEDCard.NCadcg);
  if(DEBUG)
    printf("Channel ADC Gain bits chnls 21-28(1Vpp = 0):%x\n",pixelFEDCard.SCadcg);
  if(DEBUG)
    printf("Channel ADC Gain bits chnls 29-36(1Vpp = 0):%x\n",pixelFEDCard.Sadcg);
    
       //These bits set Baseline adjustment value (common by FPGA)//can turn on by channel
  fscanf(infile,"Channel Baseline Enbable chnls 1-9  (on = (0x1ff<<16)+):%x\n",
         &pixelFEDCard.Nbaseln);
  fscanf(infile,"Channel Baseline Enbable chnls 10-18(on = (0x1ff<<16)+):%x\n",
         &pixelFEDCard.NCbaseln);
  fscanf(infile,"Channel Baseline Enbable chnls 19-27(on = (0x1ff<<16)+):%x\n",
         &pixelFEDCard.SCbaseln);
  fscanf(infile,"Channel Baseline Enbable chnls 28-36(on = (0x1ff<<16)+):%x\n",
         &pixelFEDCard.Sbaseln);
  if(DEBUG)
    printf("Channel Baseline Enbable chnls 1-9  (on = (0x1ff<<16)+):%x\n",pixelFEDCard.Nbaseln);
  if(DEBUG)
    printf("Channel Baseline Enbable chnls 10-18(on = (0x1ff<<16)+):%x\n",pixelFEDCard.NCbaseln);
  if(DEBUG)
    printf("Channel Baseline Enbable chnls 19-27(on = (0x1ff<<16)+):%x\n",pixelFEDCard.SCbaseln);
  if(DEBUG)
    printf("Channel Baseline Enbable chnls 28-36(on = (0x1ff<<16)+):%x\n",pixelFEDCard.Sbaseln);

       //These bits set TBM trailer mask (common by FPGA) 
  fscanf(infile,"TBM trailer mask chnls 1-9  (0xff = all masked):%x\n",
         &pixelFEDCard.N_TBMmask);
  fscanf(infile,"TBM trailer mask chnls 10-18(0xff = all masked):%x\n",
         &pixelFEDCard.NC_TBMmask);
  fscanf(infile,"TBM trailer mask chnls 19-27(0xff = all masked):%x\n",
         &pixelFEDCard.SC_TBMmask);
  fscanf(infile,"TBM trailer mask chnls 28-36(0xff = all masked):%x\n",
         &pixelFEDCard.S_TBMmask);
  if(DEBUG)
    printf("TBM trailer mask chnls 1-9  (0xff = all masked):%x\n",pixelFEDCard.N_TBMmask);
  if(DEBUG)
    printf("TBM trailer mask chnls 10-18(0xff = all masked):%x\n",pixelFEDCard.NC_TBMmask);
  if(DEBUG)
    printf("TBM trailer mask chnls 19-27(0xff = all masked):%x\n",pixelFEDCard.SC_TBMmask);
  if(DEBUG)
    printf("TBM trailer mask chnls 28-36(0xff = all masked):%x\n",pixelFEDCard.S_TBMmask);

       //These bits set the Private fill/gap word value (common by FPGA) 
  fscanf(infile,"Private 8 bit word chnls 1-9  :%x\n",
         &pixelFEDCard.N_Pword);
  fscanf(infile,"Private 8 bit word chnls 10-18:%x\n",
         &pixelFEDCard.NC_Pword);
  fscanf(infile,"Private 8 bit word chnls 19-27:%x\n",
         &pixelFEDCard.SC_Pword);
  fscanf(infile,"Private 8 bit word chnls 28-36:%x\n",
         &pixelFEDCard.S_Pword);
  if(DEBUG)
    printf("Private 8 bit word chnls 1-9  :%x\n",pixelFEDCard.N_Pword);
  if(DEBUG)
    printf("Private 8 bit word chnls 10-18:%x\n",pixelFEDCard.NC_Pword);
  if(DEBUG)
    printf("Private 8 bit word chnls 19-27:%x\n",pixelFEDCard.SC_Pword);
  if(DEBUG)
    printf("Private 8 bit word chnls 28-36:%x\n",pixelFEDCard.S_Pword);

      //Bits (1st 4) used to set the channel you want to read in spy fifo2
  fscanf(infile,"N  Scope channel(0-8):%x\n",
         &pixelFEDCard.N_ScopeCh);
  fscanf(infile,"NC Scope channel(0-8):%x\n",
         &pixelFEDCard.NC_ScopeCh);
  fscanf(infile,"SC Scope channel(0-8):%x\n",
         &pixelFEDCard.SC_ScopeCh);
  fscanf(infile,"S  Scope channel(0-8):%x\n",
         &pixelFEDCard.S_ScopeCh);
  if(DEBUG)
    printf("N  Scope channel(0-8):%x\n",pixelFEDCard.N_ScopeCh);
  if(DEBUG)
    printf("NC Scope channel(0-8):%x\n",pixelFEDCard.NC_ScopeCh);
  if(DEBUG)
    printf("SC Scope channel(0-8):%x\n",pixelFEDCard.SC_ScopeCh);
  if(DEBUG)
    printf("S  Scope channel(0-8):%x\n",pixelFEDCard.S_ScopeCh);

       //These bit sets the special dac mode for random triggers 
  fscanf(infile,"Special Random testDAC mode (on = 0x1, off=0x0):%x\n",
         &pixelFEDCard.SpecialDac);
  if(DEBUG)
    printf("Special Random testDAC mode (on = 0x1, off=0x0):%x\n",pixelFEDCard.SpecialDac);

      //These bits set the number of Out of consecutive out of sync events until a TTs OOs 
  fscanf(infile,"Number of Consecutive (max 1023) Out of Syncs till TTs OOS set:%d\n",
         &pixelFEDCard.Ooslvl);
  if(DEBUG)
    printf("Number of Consecutive (max 1023) Out of Syncs till TTs OOS set:%d\n",pixelFEDCard.Ooslvl);

      //These bits set the number of Empty events until a TTs Error 
  fscanf(infile,"Number of Consecutive (max 1023) Empty events till TTs ERR set:%d\n",
         &pixelFEDCard.Errlvl);
  if(DEBUG)
    printf("Number of Consecutive (max 1023) Empty events till TTs ERR set:%d\n",pixelFEDCard.Errlvl);

      //These bits set the Almost Full level in fifo-1, Almost full = TTs BUSY in fifo-1 N
  fscanf(infile,"N Fifo-1 almost full level,sets TTs BUSY (max 1023):%d\n",
         &pixelFEDCard.Nfifo1Bzlvl);
  if(DEBUG)
    printf("N Fifo-1 almost full level,sets TTs BUSY (max 1023):%d\n",pixelFEDCard.Nfifo1Bzlvl);

      //These bits set the Almost Full level in fifo-1, Almost full = TTs BUSY in fifo-1 NC
  fscanf(infile,"NC Fifo-1 almost full level,sets TTs BUSY (max 1023):%d\n",
         &pixelFEDCard.NCfifo1Bzlvl);
  if(DEBUG)
    printf("NC Fifo-1 almost full level,sets TTs BUSY (max 1023):%d\n",pixelFEDCard.NCfifo1Bzlvl);

      //These bits set the Almost Full level in fifo-1, Almost full = TTs BUSY in fifo-1 SC
  fscanf(infile,"SC Fifo-1 almost full level,sets TTs BUSY (max 1023):%d\n",
         &pixelFEDCard.SCfifo1Bzlvl);
  if(DEBUG)
    printf("SC Fifo-1 almost full level,sets TTs BUSY (max 1023):%d\n",pixelFEDCard.SCfifo1Bzlvl);

      //These bits set the Almost Full level in fifo-1, Almost full = TTs BUSY in fifo-1 S
  fscanf(infile,"S Fifo-1 almost full level,sets TTs BUSY (max 1023):%d\n",
         &pixelFEDCard.Sfifo1Bzlvl);
  if(DEBUG)
    printf("S Fifo-1 almost full level,sets TTs BUSY (max 1023):%d\n",pixelFEDCard.Sfifo1Bzlvl);

      //These bits set the Almost Full level in fifo-3, Almost full = TTs WARN in fifo-3
  fscanf(infile,"Fifo-3 almost full level,sets TTs WARN (max 8191):%d\n",
         &pixelFEDCard.fifo3Wrnlvl);
  if(DEBUG)
    printf("Fifo-3 almost full level,sets TTs WARN (max 8191):%d\n",pixelFEDCard.fifo3Wrnlvl);

  fscanf(infile,"FED Master delay 0=0,1=32,2=48,3=64:%d\n",&pixelFEDCard.FedTTCDelay);
  if(DEBUG)
    printf("FED Master delay 0=0,1=32,2=48,3=64:%d\n",pixelFEDCard.FedTTCDelay);

  fscanf(infile,"TTCrx Register 0 fine delay ClkDes1:%d\n",&pixelFEDCard.FineDes1Del);
  if(DEBUG)
    printf("TTCrx Register 0 fine delay ClkDes1:%d\n",pixelFEDCard.FineDes1Del);

  int checkword=0;
  fscanf(infile,"Params FED file check word:%d\n",
	 &checkword);
  if(checkword!=90508&&checkword!=91509&&checkword!=20211) cout <<  "FEDID: "                   << fedNumber 
								<< " Params FED File read error. Checkword read " << checkword
								<<" check word expected 090508 or 91509 or 20211"          << endl;
  assert((checkword==90508)|(checkword==91509)|(checkword==20211));
  
  
  //These bits set the hit limit in fifo-1 for an event
				
				if(checkword==20211){


  //These bits set the hit limit in fifo-1 for an event
  fscanf(infile,"N fifo-1 hit limit (max 1023 (hard) 900 (soft):%d\n",&pixelFEDCard.N_hitlimit);
  if(DEBUG)
    printf("N fifo-1 hit limit (max 1023 (hard) 900 (soft):%d\n",pixelFEDCard.N_hitlimit);    
  fscanf(infile,"NC fifo-1 hit limit (max 1023 (hard) 900 (soft):%d\n",&pixelFEDCard.NC_hitlimit);
  if(DEBUG)
    printf("NC fifo-1 hit limit (max 1023 (hard) 900 (soft):%d\n",pixelFEDCard.NC_hitlimit);
  fscanf(infile,"SC fifo-1 hit limit (max 1023 (hard) 900 (soft):%d\n",&pixelFEDCard.SC_hitlimit);
  if(DEBUG)
    printf("SC fifo-1 hit limit (max 1023 (hard) 900 (soft):%d\n",pixelFEDCard.SC_hitlimit);
  fscanf(infile,"S fifo-1 hit limit (max 1023 (hard) 900 (soft):%d\n",&pixelFEDCard.S_hitlimit);
  if(DEBUG)
    printf("S fifo-1 hit limit (max 1023 (hard) 900 (soft):%d\n",pixelFEDCard.S_hitlimit);
      //These bits allow a ROC to be skipped (1/fpga)
      
  fscanf(infile,"N  testreg:%x\n",&pixelFEDCard.N_testreg);
  if(DEBUG)
    printf("N  testreg:%x\n",pixelFEDCard.N_testreg);
  fscanf(infile,"NC testreg:%x\n",&pixelFEDCard.NC_testreg);
  if(DEBUG)
    printf("NC testreg:%x\n",pixelFEDCard.NC_testreg);
  fscanf(infile,"SC testreg:%x\n",&pixelFEDCard.SC_testreg);
  if(DEBUG)
    printf("SC testreg:%x\n",pixelFEDCard.SC_testreg);
  fscanf(infile,"S  testreg:%x\n",&pixelFEDCard.S_testreg);
  if(DEBUG)
    printf("S  testreg:%x\n",pixelFEDCard.S_testreg);

  fscanf(infile,"Set BUSYWHENBEHIND by this many triggers with timeouts:%d\n",&pixelFEDCard.BusyWhenBehind);
  if(DEBUG)
    printf("Set BUSYWHENBEHIND by this many triggers with timeouts:%d\n",pixelFEDCard.BusyWhenBehind);
				
 fscanf(infile,"D[0]=1 enable fed-stuck reset D[1]=1 disable ev# protect(dont):%x\n",&pixelFEDCard.FeatureRegister);
	  if(DEBUG)
    printf("D[0]=1 enable fed-stuck reset D[1]=1 disable ev# protect(dont):%x\n",pixelFEDCard.FeatureRegister);	 
		 
 fscanf(infile,"Limit for fifo-2 almost full (point for the TTS flag):%x\n",&pixelFEDCard.FIFO2Limit);
	  if(DEBUG)
    printf("Limit for fifo-2 almost full (point for the TTS flag):%x\n",pixelFEDCard.FIFO2Limit);	 
		 
 fscanf(infile,"Limit for consecutive timeout OR OOSs:%d\n",&pixelFEDCard.TimeoutOROOSLimit);
	  if(DEBUG)
    printf("Limit for consecutive timeout OR OOSs:%d\n",pixelFEDCard.TimeoutOROOSLimit);	 
		 
 fscanf(infile,"Turn off filling of lastdac fifos(exc 1st ROC):%d\n",&pixelFEDCard.LastDacOff);
	  if(DEBUG)
    printf("Turn off filling of lastdac fifos(exc 1st ROC):%d\n",pixelFEDCard.LastDacOff);	 
		 
 fscanf(infile,"Number of simulated hits per ROC for internal generator:%d\n",&pixelFEDCard.SimHitsPerRoc);
	  if(DEBUG)
    printf("Number of simulated hits per ROC for internal generator:%d\n",pixelFEDCard.SimHitsPerRoc);	 

 fscanf(infile,"Miniumum hold time for busy (changing definition):%d\n",&pixelFEDCard.BusyHoldMin);
	  if(DEBUG)
    printf("Miniumum hold time for busy (changing definition):%d\n",pixelFEDCard.BusyHoldMin);	 
		 
 fscanf(infile,"Trigger Holdoff in units of 25us(0=none):%d\n",&pixelFEDCard.TriggerHoldoff);
	  if(DEBUG)
    printf("Trigger Holdoff in units of 25us(0=none):%d\n",pixelFEDCard.TriggerHoldoff);	 
		 
 fscanf(infile,"Spare fedcard input 1:%d\n",&pixelFEDCard.SPARE1);
	  if(DEBUG)
    printf("Spare fedcard input 1:%d\n",pixelFEDCard.SPARE1);	 
 fscanf(infile,"Spare fedcard input 2:%d\n",&pixelFEDCard.SPARE2);
	  if(DEBUG)
    printf("Spare fedcard input 2:%d\n",pixelFEDCard.SPARE2);	 
 fscanf(infile,"Spare fedcard input 3:%d\n",&pixelFEDCard.SPARE3);
	  if(DEBUG)
    printf("Spare fedcard input 3:%d\n",pixelFEDCard.SPARE3);	 
 fscanf(infile,"Spare fedcard input 4:%d\n",&pixelFEDCard.SPARE4);
	  if(DEBUG)
    printf("Spare fedcard input 4:%d\n",pixelFEDCard.SPARE4);	 
 fscanf(infile,"Spare fedcard input 5:%d\n",&pixelFEDCard.SPARE5);
	  if(DEBUG)
    printf("Spare fedcard input 5:%d\n",pixelFEDCard.SPARE5);	 
 fscanf(infile,"Spare fedcard input 6:%d\n",&pixelFEDCard.SPARE6);
	  if(DEBUG)
    printf("Spare fedcard input 6:%d\n",pixelFEDCard.SPARE6);	 
 fscanf(infile,"Spare fedcard input 7:%d\n",&pixelFEDCard.SPARE7);
	  if(DEBUG)
    printf("Spare fedcard input 7:%d\n",pixelFEDCard.SPARE7);	 
 fscanf(infile,"Spare fedcard input 8:%d\n",&pixelFEDCard.SPARE8);
	  if(DEBUG)
    printf("Spare fedcard input 8:%d\n",pixelFEDCard.SPARE8);	 
 fscanf(infile,"Spare fedcard input 9:%d\n",&pixelFEDCard.SPARE9);
	  if(DEBUG)
    printf("Spare fedcard input 9:%d\n",pixelFEDCard.SPARE9);	 
 fscanf(infile,"Spare fedcard input 10:%d\n",&pixelFEDCard.SPARE10);
	  if(DEBUG)
    printf("Spare fedcard input 10:%d\n",pixelFEDCard.SPARE10);
			 
               }else if(checkword==91509){
  
    //These bits set the hit limit in fifo-1 for an event
  fscanf(infile,"N fifo-1 hit limit (max 1023 (hard) 900 (soft):%d\n",&pixelFEDCard.N_hitlimit);
  if(DEBUG)
    printf("N fifo-1 hit limit (max 1023 (hard) 900 (soft):%d\n",pixelFEDCard.N_hitlimit);    
  fscanf(infile,"NC fifo-1 hit limit (max 1023 (hard) 900 (soft):%d\n",&pixelFEDCard.NC_hitlimit);
  if(DEBUG)
    printf("NC fifo-1 hit limit (max 1023 (hard) 900 (soft):%d\n",pixelFEDCard.NC_hitlimit);
  fscanf(infile,"SC fifo-1 hit limit (max 1023 (hard) 900 (soft):%d\n",&pixelFEDCard.SC_hitlimit);
  if(DEBUG)
    printf("SC fifo-1 hit limit (max 1023 (hard) 900 (soft):%d\n",pixelFEDCard.SC_hitlimit);
  fscanf(infile,"S fifo-1 hit limit (max 1023 (hard) 900 (soft):%d\n",&pixelFEDCard.S_hitlimit);
  if(DEBUG)
    printf("S fifo-1 hit limit (max 1023 (hard) 900 (soft):%d\n",pixelFEDCard.S_hitlimit);
      //These bits allow a ROC to be skipped (1/fpga)
      
  fscanf(infile,"N  testreg:%x\n",&pixelFEDCard.N_testreg);
  if(DEBUG)
    printf("N  testreg:%x\n",pixelFEDCard.N_testreg);
  fscanf(infile,"NC testreg:%x\n",&pixelFEDCard.NC_testreg);
  if(DEBUG)
    printf("NC testreg:%x\n",pixelFEDCard.NC_testreg);
  fscanf(infile,"SC testreg:%x\n",&pixelFEDCard.SC_testreg);
  if(DEBUG)
    printf("SC testreg:%x\n",pixelFEDCard.SC_testreg);
  fscanf(infile,"S  testreg:%x\n",&pixelFEDCard.S_testreg);
  if(DEBUG)
    printf("S  testreg:%x\n",pixelFEDCard.S_testreg);

  pixelFEDCard.BusyWhenBehind=8;
  pixelFEDCard.FeatureRegister=0x1;    
  pixelFEDCard.FIFO2Limit=0x1c00;         
  pixelFEDCard.TimeoutOROOSLimit=200;   
  pixelFEDCard.LastDacOff=0;           
  pixelFEDCard.SimHitsPerRoc=0;        
  pixelFEDCard.BusyHoldMin=0;
  pixelFEDCard.TriggerHoldoff=0;           
  pixelFEDCard.SPARE1=0;                
  pixelFEDCard.SPARE2=0;                
  pixelFEDCard.SPARE3=0;             
  pixelFEDCard.SPARE4=0;                
  pixelFEDCard.SPARE5=0;                
  pixelFEDCard.SPARE6=0;                
  pixelFEDCard.SPARE7=0;                
  pixelFEDCard.SPARE8=0;                
  pixelFEDCard.SPARE9=0; 	        	   
  pixelFEDCard.SPARE10=0;     

				         } else {
    
    pixelFEDCard.N_hitlimit=192;	
    pixelFEDCard.NC_hitlimit=192;
    pixelFEDCard.SC_hitlimit=192;
    pixelFEDCard.S_hitlimit=192;

    pixelFEDCard.N_testreg=0;
    pixelFEDCard.NC_testreg=0;
    pixelFEDCard.SC_testreg=0;
    pixelFEDCard.S_testreg=0;

    pixelFEDCard.BusyWhenBehind=8;
    pixelFEDCard.FeatureRegister=0x1;    
    pixelFEDCard.FIFO2Limit=0x1c00;         
    pixelFEDCard.TimeoutOROOSLimit=200;   
    pixelFEDCard.LastDacOff=0;           
    pixelFEDCard.SimHitsPerRoc=0;        
    pixelFEDCard.BusyHoldMin=0;
    pixelFEDCard.TriggerHoldoff=0;           
    pixelFEDCard.SPARE1=0;                
    pixelFEDCard.SPARE2=0;                
    pixelFEDCard.SPARE3=0;             
    pixelFEDCard.SPARE4=0;                
    pixelFEDCard.SPARE5=0;                
    pixelFEDCard.SPARE6=0;                
    pixelFEDCard.SPARE7=0;                
    pixelFEDCard.SPARE8=0;                
    pixelFEDCard.SPARE9=0; 	        	   
    pixelFEDCard.SPARE10=0;     }

  fclose(infile);
  return(0);
}

// Methods which use VME access

// Test Method for Piggy Board pll Reset
int PixelPh0FEDInterface::resetDigFEDpll(void) {
  // This code is written for Pilot FED
  // which has 2 daughter boards on it  
  // with two adresses for pll reset
  // LAD_S+0x1c0000 and LAD_NC+0x1c0000
  // uses only bit=1
  unsigned long data = 0x1; // data for reset
#ifdef USE_HAL // Use HAL
  
  cout<<" --- resetDigFEDPll --- "<< endl;
  //For North Piggy
  vmeDevicePtr->write("TopDauCard_pll", data );
  usleep(200);
  data=0x0;
  vmeDevicePtr->write("TopDauCard_pll", data );
  
  //For South Piggy
  data=0x1;
  vmeDevicePtr->write("BottomDauCard_pll", data );
  usleep(200);
  data=0x0;
  vmeDevicePtr->write("BottomDauCard_pll", data );
    
#else  // Use direct CAEN 
  //For North Piggy
  ret = CAENVME_WriteCycle(BHandle,TopDauCard_pll,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<" Error in write for North Daughter Board Pll "<<hex<<TopDauCard_pll<<" "<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret);
    return -1;
  }
  usleep(200);
  data=0x0;
  ret = CAENVME_WriteCycle(BHandle,TopDauCard_pll,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<" Error in write for North Daughter Board Pll "<<hex<<TopDauCard_pll<<" "<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret);
    return -1;
  }

  //For South Piggy
  data=0x1;
  ret = CAENVME_WriteCycle(BHandle,BottomDauCard_pll,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<" Error in write for South Daughter Board Pll "<<hex<<BottomDauCard_pll<<" "<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret);
    return -1;
  }
  usleep(200);
  data=0x0;
  ret = CAENVME_WriteCycle(BHandle,BottomDauCard_pll,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<" Error in write for South Daughter Board Pll "<<hex<<BottomDauCard_pll<<" "<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret);
    return -1;
  }
#endif // USE_HAL
  
  return(0);
}

// Test Method for Piggy Board register Reset
int PixelPh0FEDInterface::resetDigFEDreg(void) {
  // This code is written for Pilot FED
  // which has 2 daughter boards on it  
  // with two adresses for pll reset
  // LAD_S+0x1c0000 and LAD_NC+0x1c0000
  // uses only bit=2
  unsigned long data = 0x2; // data for reset
#ifdef USE_HAL // Use HAL
  
  cout<<" --- resetDigFEDreg --- "<< endl;
  //For North Piggy
  vmeDevicePtr->write("TopDauCard_pll", data );
  usleep(200);
  data=0x0;
  vmeDevicePtr->write("TopDauCard_pll", data );
  
  //For South Piggy
  data=0x2;
  vmeDevicePtr->write("BottomDauCard_pll", data );
  usleep(200);
  data=0x0;
  vmeDevicePtr->write("BottomDauCard_pll", data );
  
#else  // Use direct CAEN 
  //For North Piggy
  ret = CAENVME_WriteCycle(BHandle,TopDauCard_pll,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<" Error in write for North Daughter Board Pll register "<<hex<<TopDauCard_pll<<" "<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret);
    return -1;
  }
  usleep(200);
  data=0x0;
  ret = CAENVME_WriteCycle(BHandle,TopDauCard_pll,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<" Error in write for North Daughter Board Pll register "<<hex<<TopDauCard_pll<<" "<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret);
    return -1;
  }

  //For South Piggy
  data=0x2;
  ret = CAENVME_WriteCycle(BHandle,BottomDauCard_pll,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<" Error in write for South Daughter Board Pll register "<<hex<<BottomDauCard_pll<<" "<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret);
    return -1;
  }
  usleep(200);
  data=0x0;
  ret = CAENVME_WriteCycle(BHandle,BottomDauCard_pll,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<" Error in write for South Daughter Board Pll register "<<hex<<BottomDauCard_pll<<" "<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret);
    return -1;
  }
#endif // USE_HAL
  
  return(0);
}


///////////////////////////////////////////////////////////////////////
// General reset, resets everything there is to reset.
void PixelPh0FEDInterface::reset(void) {
  uint32_t data = 0x0; // data for reseta 

  cout<<" In reset() "<<endl;
#ifndef PILOT_FED
  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Shutting off Baseline Correction"<<endl;
  BaselineCorr_off();
#endif
  // do the reset 
  //data=0x0;    
  //vmeDevicePtr->write("ResTTCrx", data );
  //usleep(20000);
  //cout<<" After reset TTCrx, sleep for 20ms  "<<endl;


  // Reset TTCrx (to reset the event and bx counters)
  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Check / Reset TTCrx"<<endl;

  //get TTCrx status
  
  int ttcrx_stat = TTCRX_I2C_REG_READ( 22);
  int resetTTCrx=0;
  
  cout<<"TTCrx status should be 0xe0 read = 0x"<<hex<<ttcrx_stat<<dec<<endl;
  
  if(ttcrx_stat&0x80) {cout<<" Pll Ready"<<endl;} else{resetTTCrx++;cout<<"Pll Not Ready!!!"<<endl;}
  if(ttcrx_stat&0x40) {cout<<" dll Ready"<<endl;} else{resetTTCrx++;cout<<"dll Not Ready!!!"<<endl;}
  if(ttcrx_stat&0x20) {cout<<" Frame Synced"<<endl;} else{resetTTCrx++;cout<<"Frame not Synced!!!"<<endl;}
  if(ttcrx_stat&0x10) { // added to clear the bit after TTCrx autoreset  1/9/15 dk
    cout<<"Autoreset detected! Try to clear it "<<endl;
    TTCRX_I2C_REG_WRITE(22,0); //reset watch dog
    usleep(20000);
    int ttcrx_stat1 = TTCRX_I2C_REG_READ( 22);
    if(ttcrx_stat1 & 0x10) {
      cout<<" Autoreset bit did not disappear, try resting the TTCrx "<<endl;
      resetTTCrx++;
    } else {
      cout<<" Autoreset cleared, all OK "<<hex<<ttcrx_stat1<<dec<<endl;
    }    
  }

  if(resetTTCrx>0){
    cout<<"TTCrx problem...resetting"<<endl;
    
    data=0x0;
    
    vmeDevicePtr->write("ResTTCrx", data );
    
    usleep(20000);
    cout<<" After reset TTCrx, sleep for 20ms,  and programming of the TTCrx "<<endl;
    
    //re-check status	
    int ttcrx_stat2 = TTCRX_I2C_REG_READ( 22);	
    cout<<"After Reset TTCrx status = 0x"<<hex<<ttcrx_stat2<<dec<<endl;	
    
  } else {cout<<" TTCrx OK"<<endl;}
 
  if(Printlevel&1) {
    cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Enable the Clock40Des2 signal from the TTCrx "<<endl;
  }
  // Need to enable the Clock40Des2 signal, otherwise the event counting does not work!
  TTCRX_I2C_REG_WRITE( 2, 0 );   // COARSE DELAY REG
  TTCRX_I2C_REG_WRITE( 1, 5 );   // Fine Delay ClockDes2 (this never changes)
  TTCRX_I2C_REG_WRITE( 0, 0xe);   // Fine Delay ClockDes1
  TTCRX_I2C_REG_WRITE( 3, 0x9b );// ControlReg  enable ClockDes2

  // Another way to reset the TTCrx counters is to write to them.
  //TTCRX_I2C_REG_WRITE(26, 0x0);// write to the event counter resets it to 0.
  //TTCRX_I2C_REG_WRITE(24, 0x0);// write to the bx counter resets it to 0.

  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Reset Plls"<<endl;
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Reset PLLs"<<endl;
  data = 0x20000000; 

  vmeDevicePtr->write("NWrResetPls", data );
  usleep(10);
  vmeDevicePtr->write("NCWrResetPls", data );
  usleep(10);
  vmeDevicePtr->write("SCWrResetPls", data );
  usleep(10);
  vmeDevicePtr->write("SWrResetPls", data );
  usleep(10);
  vmeDevicePtr->write("ResetPls", data );
  usleep(10);
  
  // Make Sure to execute for pilotFED only!!!!
#ifdef PILOT_FED
    cout << " pilotFED resets  "  << endl;
    resetDigFEDpll();
    usleep(200000);
    resetDigFEDreg();
    usleep(200000);
#endif
  
  // Reset LRES
  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Setting LRES"<<endl;
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Setting LRES"<<endl;
  data = 0x80000000; // data for reset bit
  //LRES resets pxl state machine,
  //clears the pipeline,
  //flushes FIFoI,II,Err,
  //ChOfsett DACS and Temperature fifos

#ifdef USE_HAL // Use HAL

  vmeDevicePtr->write("LRES",data);

#else  // Use direct CAEN 
  ret = CAENVME_WriteCycle(BHandle,LRES,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<" Error in write for LRES "<<hex<<LRES<<" "<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret);
    return -1;
  }
#endif // USE_HAL


  // reset CLRES
  //CLRES flushes the header and fifoIII
  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Setting CLRES"<<endl;
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Setting CLRES"<<endl;

#ifdef USE_HAL // Use HAL

  vmeDevicePtr->write("CLRES",data );

#else  // Use direct CAEN 
  ret = CAENVME_WriteCycle(BHandle,CLRES,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<" Error in write for CLRES "<<hex<<CLRES<<" "<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret);
    return -1;
  }
#endif // USE_HAL

  // Reset the Spy FIFOs and the TestDACRAMcounter. Do with a broadcast.
  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Flush SPY FIFOs N and DAC RAM Counter "<<endl;
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Flush SPY FIFOs N and DAC RAM Counter "<<endl;
  data = 0x80000000; 

#ifdef USE_HAL // Use HAL

  //vmeDevicePtr->write("BrcstResetPls", data ); maybe replace by individual 
  vmeDevicePtr->write("NWrResetPls", data );
  vmeDevicePtr->write("NCWrResetPls", data );
  vmeDevicePtr->write("SCWrResetPls", data );
  vmeDevicePtr->write("SWrResetPls", data );

#else  // Use direct CAEN 
  ret = CAENVME_WriteCycle(BHandle,RESET_SPY_FIFOS,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
      analyzeError(ret); 
  }
#endif // USE_HAL


  //Make sure TTS test enable is off
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Reset the TTS bits"<<endl;
  testTTSbits(0,0);

  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Drain FIFOs"<<endl;
  //drain all the fifo's for good measure
  uint32_t buffer[(36*1024)];
  int wdcnt=drainErrorFifo(buffer);
  wdcnt=drainTemperatureFifo(buffer);
  wdcnt=drainTTSFifo(buffer);
  wdcnt=drainFifo1(buffer);
  for(int i=1;i<9;i+=6)drainDigTransFifo(i,buffer);
  for(int i=1;i<9;i++)wdcnt=drainDataFifo2(i,buffer);
  wdcnt=drainDataFifo3(buffer);
}

void PixelPh0FEDInterface::armDigFEDOSDFifo(int channel, int rochi, int roclo) {
  const int chip = (channel - 1)/9;
  const unsigned offset = (channel % 9) * 0x20000 + 0x8000;
  const uint32_t data = ((rochi & 0x1F) << 5) | (roclo & 0x1F);
  //std::cout << "armDigFEDOSDFifo chip = " << chip << " offset = 0x" << std::hex << offset << std::dec << " data = 0x" << std::hex << data << std::dec << std::endl;
#ifdef USE_HAL
  vmeDevicePtr->write(FPGAName[chip], data, HAL::HAL_NO_VERIFY, offset);
#else
#error armDigFEDOSDFifo not implemented for direct CAEN VME access
#endif
}

uint32_t PixelPh0FEDInterface::readDigFEDOSDFifo(int channel) {
  const int chip = (channel - 1)/9;
  const unsigned offset = (channel % 9) * 0x20000 + 0x8000;
  uint32_t data;
  //std::cout << "readDigFEDOSDFifo chip = " << chip << " offset = 0x" << std::hex << offset << std::dec;
#ifdef USE_HAL
  vmeDevicePtr->read(FPGAName[chip], &data, offset);
#else
#error readDigFEDOSDFifo not implemented for direct CAEN VME access
#endif
  //std::cout << " data = 0x" << std::hex << data << std::dec << std::endl;
  return data;
}

void PixelPh0FEDInterface::readDigFEDTempFifo(){
  //uint32_t data = 0x80000000;
  uint32_t d, i;
#ifdef USE_HAL // Use HAL
    
  printf("\n\n\nTEMP FIFO Sup\n\n");
  for(i=0;i<256;i++)  { 
    vmeDevicePtr->read("BottomDauCard_UpTmpFifo",&d);
    if(d) { 
      printf("%x\n",d); 
      printf("CH#:%2d",((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",((d>>1)&0xff00)+(d&0xff));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",((d>>4)&0xff00)+(d&0xff));    
    }
  }	
  
  printf("\n\n\nTEMP FIFO Sdown\n\n");
  for(i=0;i<256;i++)  { 
    vmeDevicePtr->read("BottomDauCard_DnTmpFifo",&d);
    if(d) { 
      printf("%x\n",d); 
      printf("CH#:%2d",((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",((d>>1)&0xff00)+(d&0xff));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",((d>>4)&0xff00)+(d&0xff));    
    }
  }	
  
  printf("\n\n\nTEMP FIFO NCup\n\n");
  for(i=0;i<256;i++)  { 
    vmeDevicePtr->read("TopDauCard_UpTempFifo",&d);
    if(d) { 
      printf("%x\n",d); 
      printf("CH#:%2d",((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",((d>>1)&0xff00)+(d&0xff));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",((d>>4)&0xff00)+(d&0xff));    
    }
  }	
  
  printf("\n\n\nTEMP FIFO NCdown\n\n");
  for(i=0;i<256;i++)  { 
    vmeDevicePtr->read("TopDauCard_DownTempFifo",&d);
    if(d) { 
      printf("%x\n",d); 
      printf("CH#:%2d",((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",((d>>1)&0xff00)+(d&0xff));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",((d>>4)&0xff00)+(d&0xff));    
    }
  }	
  
#else // Use direct CAEN
  
  printf("\n\n\nTEMP FIFO Sup\n\n");
  for(i=0;i<256;i++)  { 
    ret = CAENVME_ReadCycle(BHandle,BottomDauCard_UpTmpFifo,&d,am,dw);
    if(ret != cvSuccess) {
      cout<<"Error in read "<<hex<<ret<<" "<<d<<dec<<endl;
      analyzeError(ret);   
    }
    if(d) { 
      printf("%x\n",d); 
      printf("CH#:%2d",((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",((d>>1)&0xff00)+(d&0xff));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",((d>>4)&0xff00)+(d&0xff));    
    }
  }	
  
  printf("\n\n\nTEMP FIFO Sdown\n\n");
  for(i=0;i<256;i++)  { 
    ret = CAENVME_ReadCycle(BHandle,BottomDauCard_DnTmpFifo,&d,am,dw);
    if(ret != cvSuccess) {
      cout<<"Error in read "<<hex<<ret<<" "<<d<<dec<<endl;
      analyzeError(ret);   
    }
    if(d) { 
      printf("%x\n",d); 
      printf("CH#:%2d",((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",((d>>1)&0xff00)+(d&0xff));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",((d>>4)&0xff00)+(d&0xff));    
    }
  }	
  
  printf("\n\n\nTEMP FIFO NCup\n\n");
  for(i=0;i<256;i++)  { 
    ret = CAENVME_ReadCycle(BHandle,TopDauCard_UpTempFifo,&d,am,dw);
    if(ret != cvSuccess) {
      cout<<"Error in read "<<hex<<ret<<" "<<d<<dec<<endl;
      analyzeError(ret);   
    }
    if(d) { 
      printf("%x\n",d); 
      printf("CH#:%2d",((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",((d>>1)&0xff00)+(d&0xff));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",((d>>4)&0xff00)+(d&0xff));    
    }
  }	
  
  printf("\n\n\nTEMP FIFO NCdown\n\n");
  for(i=0;i<256;i++)  { 
    ret = CAENVME_ReadCycle(BHandle,TopDauCard_DownTempFifo,&d,am,dw);
    if(ret != cvSuccess) {
      cout<<"Error in read "<<hex<<ret<<" "<<d<<dec<<endl;
      analyzeError(ret);   
    }
    if(d) { 
      printf("%x\n",d); 
      printf("CH#:%2d",((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",((d>>1)&0xff00)+(d&0xff));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",((d>>4)&0xff00)+(d&0xff));    
    }
  }	
  
#endif // Use HAL  
  
}

void PixelPh0FEDInterface::readDigFEDStatus(bool verbose, bool override_timeout) {
  if (!override_timeout) {
    timeval t;
    int s = gettimeofday(&t, 0);
    if (s != 0) {
      perror("problem w gettimeofday in readDigFEDStatus");
      return;
    }
    long long tt = t.tv_sec*1000000 + t.tv_usec;
    if (tt - DauCards_lastStatusPoll < 12000000) // 24 * 0.5 sec
      return;
    DauCards_lastStatusPoll = tt;
  }

  uint32_t d;
  size_t i;
  const size_t Npoll = 48;
  const size_t Nkeep = 24;

  int lock[19] = {0};
  std::vector<int> phases[19];
  double means[19] = {0.};
  double rmses[19] = {0.};
  for (int j = 1; j <= 18; ++j)
    phases[j].assign(Npoll, 0);

#ifdef USE_HAL
  if (verbose) printf("\n\n\nPIGGYstatus NORTHup    CH#1 / 2  CH#3 / 4  CH#5 / 6  \n\n") ;
  for(i=0;i<Npoll;i++)  {
    vmeDevicePtr->read("LAD_N", &d, 0x158000);
    if (i >= Nkeep)
      continue;

    int a = phases[1][i] = d&0x7;
    int b = phases[2][i] = (d>>8)&0x7;
    int c = phases[3][i] = (d>>16)&0x7;
    char va = 118-(d&0x8);
    char vb = 118-((d>>8)&0x8);
    char vc = 118-((d>>16)&0x8);
    if (verbose) {
      printf("                       ");printf("%c %1d",va,a);
      printf("        ");printf("%c %1d",vb,b);
      printf("        ");printf("%c %1d",vc,c);
    }
    means[1] += a;
    means[2] += b;
    means[3] += c;
    if (va == 'v') ++lock[1];
    if (vb == 'v') ++lock[2];
    if (vc == 'v') ++lock[3];
  }

  if (verbose) printf("\n\n\nPIGGYstatus NORTHdown  CH#7 / 8  CH#9 /10  CH#11/12    \n\n");
  for(i=0;i<Npoll;i++)  {
    vmeDevicePtr->read("LAD_N", &d, 0x178000);
    if (i >= Nkeep)
      continue;

    int a = phases[4][i] = d&0x7;
    int b = phases[5][i] = (d>>8)&0x7;
    int c = phases[6][i] = (d>>16)&0x7;
    char va = 118-(d&0x8);
    char vb = 118-((d>>8)&0x8);
    char vc = 118-((d>>16)&0x8);
    if (verbose) {
      printf("                       ");printf("%c %1d",va,a);
      printf("        ");printf("%c %1d",vb,b);
      printf("        ");printf("%c %1d",vc,c);
    }
    means[4] += a;
    means[5] += b;
    means[6] += c;
    if (va == 'v') ++lock[4];
    if (vb == 'v') ++lock[5];
    if (vc == 'v') ++lock[6];
  }

  if (verbose) printf("\n\n\nPIGGYstatus SOUTHup    CH#25/26  CH#27/28  CH#29/30      \n\n") ;
  for(i=0;i<Npoll;i++)  {
    vmeDevicePtr->read("LAD_S", &d, 0x158000);
    if (i >= Nkeep)
      continue;

    int a = phases[13][i] = d&0x7;
    int b = phases[14][i] = (d>>8)&0x7;
    int c = phases[15][i] = (d>>16)&0x7;
    char va = 118-(d&0x8);
    char vb = 118-((d>>8)&0x8);
    char vc = 118-((d>>16)&0x8);
    if (verbose) {
      printf("                       ");printf("%c %1d",va,a);
      printf("        ");printf("%c %1d",vb,b);
      printf("        ");printf("%c %1d",vc,c);
    }
    means[13] += a;
    means[14] += b;
    means[15] += c;
    if (va == 'v') ++lock[13];
    if (vb == 'v') ++lock[14];
    if (vc == 'v') ++lock[15];
  }

  if (verbose) printf("\n\n\nPIGGYstatus SOUTHdown  CH#31/32  CH#33/34  CH#35/36      \n\n") ;
  for(i=0;i<Npoll;i++)  {
    vmeDevicePtr->read("LAD_S", &d, 0x178000);
    if (i >= Nkeep)
      continue;

    int a = phases[16][i] = d&0x7;
    int b = phases[17][i] = (d>>8)&0x7;
    int c = phases[18][i] = (d>>16)&0x7;
    char va = 118-(d&0x8);
    char vb = 118-((d>>8)&0x8);
    char vc = 118-((d>>16)&0x8);
    if (verbose) {
      printf("                       ");printf("%c %1d",va,a);
      printf("        ");printf("%c %1d",vb,b);
      printf("        ");printf("%c %1d",vc,c);
    }
    means[16] += a;
    means[17] += b;
    means[18] += c;
    if (va == 'v') ++lock[16];
    if (vb == 'v') ++lock[17];
    if (vc == 'v') ++lock[18];
  }
#else
  assert(0);
#endif

  printf("FEDID:%lu phase stats:\n", pixelFEDCard.fedNumber);
  for (int j = 1; j <= 18; ++j) {
    if (j == 3 || (j >= 6 && j <= 12) || j == 15 || j == 18)
      continue;
    means[j] /= Npoll;
    for (int k = 0; k < Npoll; ++k)
      rmses[j] += pow(phases[j][k] - means[j], 2);
    rmses[j] /= (Npoll - 1);
    rmses[j] = sqrt(rmses[j]);
    printf("ch %2i/%2i: #locks: %2i/%2i  mean %4.1f rms %6.4f\n", j*2-1, j*2, lock[j], int(Nkeep), means[j], rmses[j]);
  }

  fflush(stdout);
}

void PixelPh0FEDInterface::loadFPGADigFED(){
  uint32_t data = 0x0; // data for reseta 
  
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Loading FPGA's from Program EEPROMs"<<endl;
  data=0x80;
#ifdef USE_HAL // Use HAL
  
  cout<< " --- loadFPGADigFED --- " << endl;
  vmeDevicePtr->write("TopDauCard_nConfig", data );
  usleep(10000);
  data=0x0;
  vmeDevicePtr->write("TopDauCard_nConfig", data );
  usleep(1000);
  
  data=0x10;
  vmeDevicePtr->write("BottomDauCard_nConfig", data );
  usleep(10000);
  data=0x0;
  vmeDevicePtr->write("BottomDauCard_nConfig", data );
  
    
#else // Use direct CAEN
  ret = CAENVME_WriteCycle(BHandle,TopDauCard_nConfig,&data,am,dw);
  usleep(10000); 
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
      analyzeError(ret); 
  }
  data=0x0;
  ret = CAENVME_WriteCycle(BHandle,TopDauCard_nConfig,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
      analyzeError(ret); 
  }
  
  data=0x10;
  ret = CAENVME_WriteCycle(BHandle,BottomDauCard_nConfig,&data,am,dw);
  usleep(10000); //min is 40us in manual to initiate
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
      analyzeError(ret); 
  }
  data=0x0;
  ret = CAENVME_WriteCycle(BHandle,BottomDauCard_nConfig,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
      analyzeError(ret); 
  }
#endif // Use HAL
}//end



/////////////////////////////////////////////////////////////////
// Load the FPGAs -----------------------------------------------
// Takes the programs in the EEPROM memory and loads it in FPGAs
// This also SHOULD occur upon power up reset
  void PixelPh0FEDInterface::loadFPGA(){
  uint32_t data = 0x0; // data for reseta 

  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Loading FPGA's from Program EEPROMs"<<endl;
  data=0x1;
  vmeDevicePtr->write("nCONFIG", data );
  usleep(1000); //min is 40us in manual to initiate
  data=0x0;
  vmeDevicePtr->write("nCONFIG", data );
  usleep(1000000); //10ms in example programs, extra time now for clock
  data=0x2;
  vmeDevicePtr->write("nCONFIG", data );
  usleep(1000); //min is 40us in manual to initiate
  data=0x0;
  vmeDevicePtr->write("nCONFIG", data );
  usleep(1000000); //10ms in example programs, extra time now for clock

#ifdef PILOT_FED  
    cout << " pilotFED load fpga  "<< endl;
    loadFPGADigFED();
#endif

#ifndef PILOT_FED
//new sequence for v4
// load test constants
// setup for VME trigger and testDAC
// load testDAC with a constant level
// reset the Slink
// shut off the slink
// shut off the channels
// generate a vme trigger
// drain the fifo-I's

std::string filnam(getenv("BUILD_HOME"));
filnam+="/pixel/PixelPh0FEDInterface/dat/";
filnam+="params_fed.dat";
setupFromDB(filnam);

uint32_t value = 0x0e;
int status = setControlRegister(value);
if(status<0)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Error Setting Control Word! status ="<<status<<endl;
int dac0[256];
for(int i=0;i<256;i++) dac0[i]=340;
dac0[255]=0;//make sure any leftover testDAC setting is 0 
int mlength=256;//overkill-currently shuts off setDAC after stop bit
fillDACRegisterLength(dac0,dac0,dac0,mlength);
usleep(100);
#endif

resetSlink();
setModeRegister(0x1);

generateVMETrigger();

#ifndef PILOT_FED
uint32_t fbufr[1024];
drainFifo1(fbufr);
#endif

data=0x80000000;
 vmeDevicePtr->write("LRES",data);
data=0x80000000;
 vmeDevicePtr->write("CLRES",data);


if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Be sure to call reset() and reload (setup())all constants before proceeding"<<endl;
}//end
/////////////////////////////////////////////////////////////////
// TTCrx acces --------------------------------------------------
// Read an TTCrx internal register through I2C.
int PixelPh0FEDInterface::TTCRX_I2C_REG_READ( int Register_Nr)  { 
  uint32_t d;
  int  i2c_addr,i2c_nbytes;

  //  RESET I2C STATE MACHINE : Reset procedure :  set 0x2 then set 0x0
  uint32_t ds =  0x2;
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("I2C_RES",ds);
#else  // Use direct CAEN 
  ret = CAENVME_WriteCycle(BHandle,I2C_RES,&ds,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<" Error in write TTCRX_I2C_RES"<<hex<<ret<<" "<<ds<<dec<<endl;
    analyzeError(ret); 
  }
#endif // USE_HAL

  usleep(80);
  ds =  0x0;
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("I2C_RES",ds );
#else  // Use direct CAEN 
  ret = CAENVME_WriteCycle(BHandle,I2C_RES,&ds,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<" Error in write TTCRX_I2C_RES"<<hex<<ret<<" "<<ds<<dec<<endl;
    analyzeError(ret); }
#endif // USE_HAL
  usleep(80);//
  
  // I2C PAYLOAD
  ds =  Register_Nr;
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("I2C_LOAD",ds);
#else  // Use direct CAEN 
  ret = CAENVME_WriteCycle(BHandle,I2C_LOAD,&ds,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<" Error in write TTCRX_I2C_LOAD"<<hex<<ret<<" "<<ds<<dec<<endl;
    analyzeError(ret); }
#endif // USE_HAL
  usleep(80);//
  
  i2c_addr=7*2; 
  i2c_nbytes=1;
  //VXIout (0x3,I2C_ADDR_RW, 2, (i2c_nbytes<<8)+(i2c_addr<<1)+0/*WRITE*/);
  ds =  (i2c_nbytes<<8)+(i2c_addr<<1)+0;
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("I2C_ADDR_RW",ds);
#else  // Use direct CAEN 
  ret = CAENVME_WriteCycle(BHandle,I2C_ADDR_RW,&ds,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<" Error in write TTCRX_I2C_ADDR_RW"<<hex<<ret<<" "<<ds<<dec<<endl;
    analyzeError(ret); }
#endif // USE_HAL
  usleep(80);//
  
  //usleep(300);
  
  //Check Status
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->read("I2C_RD_STAT",&d);
#else  // Use direct CAEN 
  ret = CAENVME_ReadCycle(BHandle,I2C_RD_STAT,&d,am,dw);
  if(ret != cvSuccess) {
    cout<<" Error in read TTCRX_I2C_REG_READ"<<hex<<ret<<" "<<ds<<dec<<endl;
    analyzeError(ret);   }
#endif // USE_HAL
  usleep(80);//
  
  //Report Status 
  if((d&0xff)==1)printf("ERROR: BUS BUSY !! \n");
  if((d&0xff)==2)printf("ERROR: I2C_ADDR  NOT ACKNOWLEDGED !! \n");
  if((d&0xff)==4)printf("ERROR: I2C_WBYTE NOT ACKNOWLEDGED !! \n");
  if((d&0xff)==8)printf("ERROR: I2C_LBYTE NOT ACKNOWLEDGED !! \n");
  
  //  RESET I2C STATE MACHINE
  ds =  0x2;
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("I2C_RES",ds);
#else  // Use direct CAEN 
  ret = CAENVME_WriteCycle(BHandle,I2C_RES,&ds,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<" Error in write TTCRX_I2C_RES"<<hex<<ret<<" "<<ds<<dec<<endl;
    analyzeError(ret); }
#endif // USE_HAL
  usleep(80);
  ds =  0x0;
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("I2C_RES",ds);
#else  // Use direct CAEN 
  ret = CAENVME_WriteCycle(BHandle,I2C_RES,&ds,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<" Error in write TTCRX_I2C_RES"<<hex<<ret<<" "<<ds<<dec<<endl;
    analyzeError(ret); }
#endif // USE_HAL
  usleep(80);
    
  /////////////////////////////  I2C READ  //////////////////////////////////
 
    i2c_addr=7*2 +1 ; i2c_nbytes=1;
    //VXIout (0x3,I2C_ADDR_RW, 2, (i2c_nbytes<<8)+(i2c_addr<<1)+0/*WRITE*/);
    ds =  (i2c_nbytes<<8)+(i2c_addr<<1)+1;
#ifdef USE_HAL // Use HAL
    vmeDevicePtr->write("I2C_ADDR_RW",ds);
#else  // Use direct CAEN 
    ret = CAENVME_WriteCycle(BHandle,I2C_ADDR_RW,&ds,am,dw);
    if(ret != cvSuccess) {  // Error
      cout<<"Error in write TTCRX_I2C_ADDR_RW"<<hex<<ret<<" "<<ds<<dec<<endl;
      analyzeError(ret); }
#endif // USE_HAL
    usleep(80);//
    //Check Status
#ifdef USE_HAL // Use HAL
    vmeDevicePtr->read("I2C_RD_STAT",&d);
#else  // Use direct CAEN 
    ret = CAENVME_ReadCycle(BHandle,I2C_RD_STAT,&d,am,dw);
    if(ret != cvSuccess) {
      cout<<"Error in read TTCRX_I2C_RD_STAT"<<hex<<ret<<" "<<ds<<dec<<endl;
      analyzeError(ret);   }
#endif // USE_HAL
    usleep(80);//
    //Report Status 
    if((d&0xff)==1)printf("ERROR: BUS BUSY !! \n");
    if((d&0xff)==2)printf("ERROR: I2C_ADDR NOT ACKNOWLEDGED !! \n");
    
    //Get Payload
#ifdef USE_HAL // Use HAL
    vmeDevicePtr->read("I2C_RD_DATA",&d);
#else  // Use direct CAEN 
    ret = CAENVME_ReadCycle(BHandle,I2C_RD_DATA,&d,am,dw);
    if(ret != cvSuccess) {
      cout<<"Error in read TTCRX_I2C_RD_DATA"<<hex<<ret<<" "<<ds<<dec<<endl;
      analyzeError(ret);   }
#endif // USE_HAL
    
    usleep(80);//
    return(d&0xff);
}
////////////////////////////////////////////////////////////////////////////////// 
// Write an TTCrx internal register through I2C.
int PixelPh0FEDInterface::TTCRX_I2C_REG_WRITE( int Register_Nr, int Value) { 
  uint32_t d;
  int  i2c_addr,i2c_nbytes;

  if(Printlevel&1) cout<<" 1 "<<endl;

  uint32_t ds =  0x2;
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("I2C_RES",ds);
#else  // Use direct CAEN 
  ret = CAENVME_WriteCycle(BHandle,I2C_RES,&ds,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write TTCRX_I2C_REG_WRITE"<<hex<<ret<<" "<<ds<<dec<<endl;
    analyzeError(ret); }
#endif // USE_HAL
  usleep(1000);
  
  if(Printlevel&1) cout<<" 2 "<<endl;

  ds =  0x0;
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("I2C_RES",ds);
#else  // Use direct CAEN 
  ret = CAENVME_WriteCycle(BHandle,I2C_RES,&ds,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write TTCRX_I2C_REG_WRITE"<<hex<<ret<<" "<<ds<<dec<<endl;
    analyzeError(ret); }
#endif // USE_HAL
  usleep(1000);
  
  if(Printlevel&1) cout<<" 3 "<<endl;

  //I2C PAYLOAD 
  ds =  Register_Nr;
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("I2C_LOAD",ds);
#else  // Use direct CAEN 
  ret = CAENVME_WriteCycle(BHandle,I2C_LOAD,&ds,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write TTCRX_I2C_LOAD"<<hex<<ret<<" "<<ds<<dec<<endl;
    analyzeError(ret); }
#endif // USE_HAL
  usleep(1000);
  
  if(Printlevel&1) cout<<" 4 "<<endl;

  i2c_addr=7*2; 
  i2c_nbytes=1;
  ds =  (i2c_nbytes<<8)+(i2c_addr<<1)+0;
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("I2C_ADDR_RW",ds);
#else  // Use direct CAEN 
  ret = CAENVME_WriteCycle(BHandle,I2C_ADDR_RW,&ds,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write TTCRX_I2C_REG_WRITE"<<hex<<ret<<" "<<ds<<dec<<endl;
    analyzeError(ret); 
  }
#endif // USE_HAL
  usleep(1000);
  
  if(Printlevel&1) cout<<" 5 "<<endl;

#ifdef USE_HAL // Use HAL
    vmeDevicePtr->read("I2C_RD_STAT",&d);
#else  // Use direct CAEN 
  ret = CAENVME_ReadCycle(BHandle,I2C_RD_STAT,&d,am,dw);
  if(ret != cvSuccess) {
    cout<<"Error in read TTCRX_I2C_REG_WRITE"<<hex<<ret<<" "<<ds<<dec<<endl;
    analyzeError(ret);   
  }
#endif // USE_HAL

  usleep(1000);//
    
  if((d&0xff)==1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" ERROR: BUS BUSY !!"<<endl;
  if((d&0xff)==2)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" ERROR: I2C_ADDR NOT ACKNOWLEDGED !!"<<endl; 
  if((d&0xff)==4)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" ERROR: I2C_WBYTE NOT ACKNOWLEDGED !!"<<endl;
  if((d&0xff)==8)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" ERROR: I2C_LBYTE NOT ACKNOWLEDGED !!"<<endl;
 
  if(Printlevel&1) cout<<" 6 "<<endl;

  //RESET I2C STATE MACHINE
  ds =  0x2;
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("I2C_RES",ds);
#else  // Use direct CAEN 
  ret = CAENVME_WriteCycle(BHandle,I2C_RES,&ds,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write TTCRX_I2C_RES"<<hex<<ret<<" "<<ds<<dec<<endl;
    analyzeError(ret); 
  }
#endif // USE_HAL
  usleep(1000);
  
  if(Printlevel&1) cout<<" 7 "<<endl;

  ds =  0x0;
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("I2C_RES",ds);
#else  // Use direct CAEN 
  ret = CAENVME_WriteCycle(BHandle,I2C_RES,&ds,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write TTCRX_I2C_RES"<<hex<<ret<<" "<<ds<<dec<<endl;
    analyzeError(ret); 
  }
#endif // USE_HAL
  usleep(1000);
 
  if(Printlevel&1) cout<<" 8 "<<endl;

  ds =  Value;
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("I2C_LOAD",ds);
#else  // Use direct CAEN 
  ret = CAENVME_WriteCycle(BHandle,I2C_LOAD,&ds,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write TTCRX_I2C_LOAD"<<hex<<ret<<" "<<ds<<dec<<endl;
    analyzeError(ret); 
  }
#endif // USE_HAL
  usleep(1000);//
   
  if(Printlevel&1) cout<<" 9 "<<endl;

   i2c_addr=7*2+1; i2c_nbytes=1;
  ds =  (i2c_nbytes<<8)+(i2c_addr<<1)+0;
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("I2C_ADDR_RW",ds);
#else  // Use direct CAEN 
  ret = CAENVME_WriteCycle(BHandle,I2C_ADDR_RW,&ds,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write TTCRX_I2C_ADDR_RW"<<hex<<ret<<" "<<ds<<dec<<endl;
    analyzeError(ret); 
  }
#endif // USE_HAL
  usleep(1000);//
 
  //printf("%x\n",(i2c_nbytes<<8)+(i2c_addr<<1)+0 );
  usleep(300);
 
  if(Printlevel&1) cout<<" 10 "<<endl;

#ifdef USE_HAL // Use HAL
  vmeDevicePtr->read("I2C_RD_STAT",&d);
#else  // Use direct CAEN 
  ret = CAENVME_ReadCycle(BHandle,I2C_RD_STAT,&d,am,dw);
  if(ret != cvSuccess) {
    cout<<"Error in read TTCRX_I2C_RD_STAT"<<hex<<ret<<" "<<ds<<dec<<endl;
    analyzeError(ret);   
  }
#endif // USE_HAL
  usleep(1000);//
 
  if(Printlevel&1) cout<<" 11 "<<endl;

  
  if((d&0xff)==1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" ERROR: BUS BUSY !!"<<endl;
  if((d&0xff)==2)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" ERROR: I2C_ADDR NOT ACKNOWLEDGED !!"<<endl ;
  if((d&0xff)==4)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" ERROR: I2C_WBYTE NOT ACKNOWLEDGED !!"<<endl;
  if((d&0xff)==8)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" ERROR: I2C_LBYTE NOT ACKNOWLEDGED !!"<<endl;
  return(0);
}
// END TTC 
///////////////////////////////////////////////////////////////////////
// Read the file with the FED setup parameters.
// Download these parameters to the FED. 
int PixelPh0FEDInterface::setupFromDB(string fileName) {
  assert(0);
  int status = 0;
  cout<<" read setup parameters from file "<<fileName<<endl;
  status = configFile(fileName);
  if(status!=0) return(-1);

  status = setup();
 
  return status;
}
/////////////////////////////////////////////////////////////////////////
// Read the file with the FED setup parameters.
// Download these parameters to the FED. 
//int PixelPh0FEDInterface::setupFromDB(PixelFEDCard pfc) : pixelFEDCard(pfc) {
int PixelPh0FEDInterface::setupFromDB(PixelFEDCard pfc) {
  pixelFEDCard = pfc;
  cout<<" Setup from the parameter structure "<< endl;
  int status = setup();
  return status;
}
/////////////////////////////////////////////////////////////////////////
// Download these parameters to the FED. 
int PixelPh0FEDInterface::setup(void) {
  int status = 0;

  if(Printlevel&2) cout<<"Setting "<<"FEDID:"<<pixelFEDCard.fedNumber<<endl;
  cout<<"Setting "<<"FEDID:"<<pixelFEDCard.fedNumber<<endl;

  int ttcrx_stat = TTCRX_I2C_REG_READ( 22);
  cout<<"TTCrx status should be 0xe0 read = 0x"<<hex<<ttcrx_stat<<dec<<endl;
  
  // Reprogram the TTCrx registers, onlu now we know the values from DB
  TTCRX_I2C_REG_WRITE( 2, pixelFEDCard.CoarseDel); //COARSE DELAY REG
  if(Printlevel&1) cout<<"Setting "<<"TTCRX 2: "<<endl;
  //ttcrx_stat = TTCRX_I2C_REG_READ( 22);
  //cout<<"TTCrx status should be 0xe0 read = 0x"<<hex<<ttcrx_stat<<dec<<endl;

  TTCRX_I2C_REG_WRITE( 1, pixelFEDCard.FineDes2Del); // Fine Delay ClockDes2
  if(Printlevel&1) cout<<"Setting "<<"TTCRX 1: "<<endl;
  //ttcrx_stat = TTCRX_I2C_REG_READ( 22);
  //cout<<"TTCrx status should be 0xe0 read = 0x"<<hex<<ttcrx_stat<<dec<<endl;

  TTCRX_I2C_REG_WRITE( 0, pixelFEDCard.FineDes1Del); // Fine Delay ClockDes1
  if(Printlevel&1) cout<<"Setting "<<"TTCRX 0: "<<endl;
  //ttcrx_stat = TTCRX_I2C_REG_READ( 22);
  //cout<<"TTCrx status should be 0xe0 read = 0x"<<hex<<ttcrx_stat<<dec<<endl;

  TTCRX_I2C_REG_WRITE( 3, pixelFEDCard.ClkDes2);// ControlReg  enable ClockDes2 !need
  if(Printlevel&1) cout<<"Setting "<<"TTCRX 3: "<<endl;
  //ttcrx_stat = TTCRX_I2C_REG_READ( 22);
  //cout<<"TTCrx status should be 0xe0 read = 0x"<<hex<<ttcrx_stat<<dec<<endl;

  setFedIDRegister(pixelFEDCard.fedNumber);

#ifndef PILOT_FED
  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Setting Optical reciever parameters"<<endl;
  set_opto_params();

  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Setting Clock Phases"<<endl;
  setPhases();  // Set all phases from DB
#endif

  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Setting Number of Rocs for each Channel"<<endl;
  set_chnl_nrocs();  // Set #Rocs from DB

#ifndef PILOT_FED 
  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Setting black, ultra-black Thresholds"<<endl;
  set_blk_ublk_thold();
     
  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Setting ROC, Header, Trailer level thresholds"<<endl;
  set_data_levels();
#endif

  // This is controls if a channel is on or off
  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Turning on all all channels for Normal Running"<<endl;
  set_chnls_onoff(); //transfer control now handled by VME-trigger

#ifndef PILOT_FED 
  //Offset DAC V2:
  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Setting the Offset DACs"<<endl;
  set_offset_dacs();
  
  //ADC Gain Registers
  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Setting ADC GAIN values"<<endl;
  set_adc_1v2v();
#endif

  //make sure testDAC is not sending extra data
  stop_testDAC();//This also loads Control and Mode registers in the central chip!

#ifndef PILOT_FED
  //Baseline Restoration Registers
  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Setting Baseline values"<<endl;
  set_BaselineCorr();
#endif

  //TTs levels for warning and busy
  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Setting Values for TTs Warn and Busy Levels"<<endl;
  set_TTslevels();

  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Setting Almost Full Values for fifo-1 and fifo-3"<<endl;
  set_Fifolevels();

#ifndef PILOT_FED  
  //Baseline Restoration Registers
  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Setting Baseline values"<<endl;
  set_BaselineCorr();
#endif

  //FED Master Delay
  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.FedTTCDelay<<" Setting FED Master Delay"<<endl;
  set_FEDTTCDelay();

  //Hit Limits
  if(Printlevel&2) 
  {cout<<"FEDID:"<<pixelFEDCard.N_hitlimit<<"N Hit Limit"<<endl;
  cout<<"FEDID:"<<pixelFEDCard.NC_hitlimit<<"NCHit Limit"<<endl;
  cout<<"FEDID:"<<pixelFEDCard.SC_hitlimit<<"SC Hit Limit"<<endl;
  cout<<"FEDID:"<<pixelFEDCard.S_hitlimit<<"S Hit Limit"<<endl;}
  set_HitLimits();


  
  // testregs
  if(Printlevel&2) 
  {cout<<"FEDID:"<<pixelFEDCard.N_testreg<<" N  testreg"<<endl;
  cout<<"FEDID:"<<pixelFEDCard.NC_testreg<<" NC testreg"<<endl;
  cout<<"FEDID:"<<pixelFEDCard.SC_testreg<<" SC testreg"<<endl;
  cout<<"FEDID:"<<pixelFEDCard.S_testreg<<" S  testreg"<<endl;}
  set_ROCskip();


  //  status = loadControlRegister();  // setup Control register from DB
  //cout<<" skip the control register load from DB"<<endl;
  //  loadModeRegister();  // Load the Mode register from DB

  set_MODE_front();//load front FPGA load registers

  //make sure TTS test enable is off
  testTTSbits(0,0);

  // Do some custom configurations which are not defined in the DB
  // Mostly to erase parameters uesed in special tests (HI)

  //make sure that simulated hit generation is off
  set_SimulatedHits();

  // make sure that the L1A holdoff for HI is OFF
  set_Hold();  // added 16/09/10

  //enable auto-clear of fifos when stuck in busy
  if (Printlevel&2) {
     if(pixelFEDCard.FeatureRegister&0x1) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Enabling auto-clear of FIFOs when stuck in BUSY"<<endl;
     if(pixelFEDCard.FeatureRegister&0x2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Warning!!! fifo transfer protection disabled!!!"<<endl;
   }

  // set the feature register
  set_FeatureReg();
	  
  // Do additional settings
  // Set the timeout for the UP/DN counter (from Wills setUPDNBusy
   cout<<"Program the UP/DOWN counter timeout to 3 (1.6ms)"<<endl;
   vmeDevicePtr->write("LAD_N", 0x3,HAL::HAL_NO_VERIFY,0x198000);	
   vmeDevicePtr->write("LAD_NC",0x3,HAL::HAL_NO_VERIFY,0x198000);	
   vmeDevicePtr->write("LAD_SC",0x3,HAL::HAL_NO_VERIFY,0x198000);	
   vmeDevicePtr->write("LAD_S", 0x3,HAL::HAL_NO_VERIFY,0x198000);	


  return status;
}

/////////////////////////////////////////////////////////////////////////
// Load the test DAC from 3 arrays in the argument list/
// Test DAC is 256 words long.
void PixelPh0FEDInterface::fillDACRegister(const int *const dac1, const int *const dac2,
					const int *const dac3) const {
assert(0);
  uint32_t TestData[256];

  // Compose DACs
  for(int i=0;i<256;i++) {
    uint32_t tmp1 = (dac1[i] & 0x3ff);
    uint32_t tmp2 = (dac2[i] & 0x3ff);
    uint32_t tmp3 = (dac3[i] & 0x3ff);
    TestData[i] =  tmp1 + (tmp2<<10) + (tmp3<<20);
  }

  // Load DACs
#ifdef USE_HAL // Use HAL

  for(int i=0;i<256;i++) {
    uint32_t offset = i*4;
    //cout<<i<<" "<<offset<<" "<<hex<<TestData[i]<<dec<<endl;
    vmeDevicePtr->write("TestDAC0",TestData[i],HAL::HAL_NO_VERIFY,offset);
  }
  
  // block write does not work??
//   TestData[150] = V_OFFSET;   // test pules
//   uint32_t length = 1024; // in bytes
//   char * buffer = (char *) TestData;
//   uint32_t offset = 0;
//   vmeDevicePtr->writeBlock("TestDAC",length,buffer,HAL::HAL_NO_VERIFY,
// 			   HAL::HAL_DO_INCREMENT,offset);

#else // direct CAEN VME access
  for(int i=0;i<256;i++) {
    CVErrorCodes ret = CAENVME_WriteCycle(BHandle,TEST_DAC+i*4,&TestData[i],
					  am,dw);
    if(ret != cvSuccess) {  // Error
      cout<<"Error in write to DAC TEST_DAC+i*4"<<hex<<ret<<" "<<data
	  <<dec<<endl;
      analyzeError(ret);
    }
  }
#endif // USE_HAL
         
  usleep(100); // Time for DAC

}
///////////////////////////////////////////////////////////////////////////////////////////
// Fill the DAC test array and load it
void PixelPh0FEDInterface::fillDACRegister() const {
assert(0);
  const uint32_t V_OFFSET=100; // 100; 
  const uint32_t UB =50;
  //const uint32_t B = 300;
  int TestData[256];

  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Filling the dac registers"<<endl;

  // DAC TEST DATA //////////////////////////////////// 
  for(int i=0;i<256;i++) 
    TestData[i]=V_OFFSET+UB; //+B

  // Test the phase 
  TestData[10] = V_OFFSET+UB;   //UB  TBM-header
  TestData[11] = V_OFFSET+UB;   //UB  TBM-header
  TestData[12] = V_OFFSET+800; // 800;   // test pules
  TestData[13] = V_OFFSET+UB;   //UB  TBM-header
  TestData[14] = V_OFFSET+UB;   //UB  TBM-header

//   const int START=64;  // define the start of the data packet

//   // Limits are 
//   // The levels are 200,300,400,500 
//   TestData[START+0] = V_OFFSET+UB;   //UB  TBM-header
//   TestData[START+1] = V_OFFSET+UB;   //UB
//   TestData[START+2] = V_OFFSET+UB;   //UB
//   TestData[START+3] = V_OFFSET+B;    //B
//   TestData[START+4] = V_OFFSET+200;  // event number
//   TestData[START+5] = V_OFFSET+200;
//   TestData[START+6] = V_OFFSET+200;
//   TestData[START+7] = V_OFFSET+300;

//   TestData[START+8] = V_OFFSET+UB;   //UB  ROC#0
//   TestData[START+9] = V_OFFSET+B;    //B
//   TestData[START+10]= V_OFFSET+800;  //  LastDac

//   TestData[START+11]= V_OFFSET+650;  // dcol MSB PIXEL 1
//   TestData[START+12]= V_OFFSET+300;  // dcol LSB     
//   TestData[START+13]= V_OFFSET+450;  // pix 
//   TestData[START+14]= V_OFFSET+200;  // pix 
//   TestData[START+15]= V_OFFSET+200;  // pix   
//   TestData[START+16]= V_OFFSET+100;  // PH

//   TestData[START+17]= V_OFFSET+UB;   // UB  TBM trailer
//   TestData[START+18]= V_OFFSET+UB;   // UB
//   TestData[START+19]= V_OFFSET+B;    // B
//   TestData[START+20]= V_OFFSET+B;    // B
//   TestData[START+21]= V_OFFSET+200;  // Status
//   TestData[START+22]= V_OFFSET+200;
//   TestData[START+23]= V_OFFSET+200;
//   TestData[START+24]= V_OFFSET+200;
    
  // Send data to the DAC, use all 3 DACs the same. 
  fillDACRegister(TestData, TestData, TestData); 
  return;

} // end
//////////////////////////////////////////////////////////////////////
// DAC routine from Souvik
//
void PixelPh0FEDInterface::fillDACRegister(vector <uint32_t> pulseTrain_R, 
					vector <uint32_t> pulseTrain_G, 
					vector <uint32_t> pulseTrain_B) const {
assert(0);
  uint32_t compositePulseTrain[256];
  
  for (int i=0;i<256;++i) {
    compositePulseTrain[i]=
      (pulseTrain_R[i]&0x3ff)+((pulseTrain_G[i]&0x3ff)<<10)+((pulseTrain_B[i]&0x3ff)<<20);
  }
  
#ifdef USE_HAL // Use HAL
  
  for(int i=0;i<256;i++) {
    uint32_t offset = i*4;
    //cout<<i<<" "<<offset<<" "<<hex<<TestData[i]<<dec<<endl;
    vmeDevicePtr->write("TestDAC0",compositePulseTrain[i],HAL::HAL_NO_VERIFY,offset);
  }
 
#else // direct CAEN VME access
   
  for(int i=0;i<256;i++) {
    CVErrorCodes ret = CAENVME_WriteCycle(BHandle,TEST_DAC+i*4,&compositePulseTrain[i],am,dw);
    if(ret != cvSuccess) {
      // Error
      cout<<"Error in write to DAC TEST_DAC+i*4"<<hex<<ret<<" "<<data<<dec<<endl;
      analyzeError(ret);
    }
  }
 
#endif
 
  usleep(100); // Time for DAC
}
///////////////////////////////////////////////////////////////////////////////////////////////
void PixelPh0FEDInterface::fillDACRegister2(void) const {
assert(0);
  //const uint32_t V_OFFSET=100; // 100;
  //const uint32_t UB =50;
  //const uint32_t B = 300;
  int TestData[256];
 
  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Filling the dac registers"<<endl;
 
  // DAC TEST DATA ////////////////////////////////////
  for(int i=0;i<256;i++)
    TestData[i]=i*4; //+B
 
  TestData[0]=800;
  TestData[1]=900;
  TestData[2]=1023;
  TestData[3]=1030;
  TestData[4]=1050;
  TestData[5]=1100;
 
  // Send data to the DAC, use all 3 DACs the same. 
  fillDACRegister(TestData, TestData, TestData); 

//   // Load DACs
//   for(int i=0;i<256;i++) {
//     uint32_t tmp = (TestData[i] & 0x3ff);
//     TestData[i] =  tmp + (tmp<<10) + (tmp<<20);
//   }
 
// #ifdef USE_HAL // Use HAL
 
//   for(int i=0;i<256;i++) {
//     uint32_t offset = i*4;
//     //cout<<i<<" "<<offset<<" "<<hex<<TestData[i]<<dec<<endl;
//     vmeDevicePtr->write("TestDAC0",TestData[i],HAL::HAL_NO_VERIFY,offset);
//   }
                                                                                
//   // block write does not work??
// //   TestData[150] = V_OFFSET;   // test pules
// //   uint32_t length = 1024; // in bytes
// //   char * buffer = (char *) TestData;
// //   uint32_t offset = 0;
// //   vmeDevicePtr->writeBlock("TestDAC",length,buffer,HAL::HAL_NO_VERIFY,
// //                         HAL::HAL_DO_INCREMENT,offset);
 
// #else // direct CAEN VME access
//   for(int i=0;i<256;i++) {
//     CVErrorCodes ret = CAENVME_WriteCycle(BHandle,TEST_DAC+i*4,&TestData[i],
//                                           am,dw);
//     if(ret != cvSuccess) {  // Error
//       cout<<"Error in write to DAC TEST_DAC+i*4"<<hex<<ret<<" "<<data
//           <<dec<<endl;
//       analyzeError(ret);
//     }
//   }
// #endif // USE_HAL
 
 
//   usleep(100); // Time for DAC
 
}
//
/////////////////////////////////////////////////////////////////////////
// Load the test DAC from 3 arrays in the argument list/
// Test DAC is "length" words long. Stop bit is set for last word
void PixelPh0FEDInterface::fillDACRegisterLength(const int *const dac1, const int *const dac2,
					const int *const dac3, int length) const {
  if(length<1) length=1;
  if(length>256) length=256;
  uint32_t TestData[256];
  for(int i=0;i<length;i++) {TestData[i]=0;}

  // Compose DACs
  for(int i=0;i<length;i++) {
    uint32_t tmp1 = (dac1[i] & 0x3ff);
    uint32_t tmp2 = (dac2[i] & 0x3ff);
    uint32_t tmp3 = (dac3[i] & 0x3ff);
    TestData[i] =  tmp1 + (tmp2<<10) + (tmp3<<20);
  }
    TestData[length-1]=0x80000000&TestData[length-1];


  // Load DACs
#ifdef USE_HAL // Use HAL

  for(int i=0;i<256;i++) {
    uint32_t offset = i*4;
    //cout<<i<<" "<<offset<<" "<<hex<<TestData[i]<<dec<<endl;
    vmeDevicePtr->write("TestDAC0",TestData[i],HAL::HAL_NO_VERIFY,offset);
  }
  
  // block write does not work??
//   TestData[150] = V_OFFSET;   // test pules
//   uint32_t length = 1024; // in bytes
//   char * buffer = (char *) TestData;
//   uint32_t offset = 0;
//   vmeDevicePtr->writeBlock("TestDAC",length,buffer,HAL::HAL_NO_VERIFY,
// 			   HAL::HAL_DO_INCREMENT,offset);

#else // direct CAEN VME access
  for(int i=0;i<256;i++) {
    CVErrorCodes ret = CAENVME_WriteCycle(BHandle,TEST_DAC+i*4,&TestData[i],
					  am,dw);
    if(ret != cvSuccess) {  // Error
      cout<<"Error in write to DAC TEST_DAC+i*4"<<hex<<ret<<" "<<data
	  <<dec<<endl;
      analyzeError(ret);
    }
  }
#endif // USE_HAL
   
      
  usleep(100); // Time for DAC

}
// Read FIFOs
/////////////////////////////////////////////////////////////////////////////////
  void PixelPh0FEDInterface::setup_testDAC(int pedestal){//pre-load the testDAC with a pedestal value
assert(0);
uint32_t value = 0x0e;
vmeDevicePtr->write("CtrlReg",value);

int dac0[256];
for(int i=0;i<256;i++) dac0[i]=pedestal&0x3ff;
fillDACRegister(dac0,dac0,dac0);
usleep(100);

resetSlink();
value=0x1;
vmeDevicePtr->write("ModeReg",value);
generateVMETrigger();

uint32_t data=0x80000000;
 vmeDevicePtr->write("LRES",data);
data=0x80000000;
 vmeDevicePtr->write("CLRES",data);

int status = loadControlRegister();
if(status<0)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Error Setting Control Word! status ="<<status<<endl;

status = loadModeRegister();
if(status<0)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Error Setting Mode Word! status ="<<status<<endl;

}
/////////////////////////////////////////////////////////////////////////////////
  void PixelPh0FEDInterface::stop_testDAC(){//load the testDAC with finite length

uint32_t value = 0x0e;
vmeDevicePtr->write("CtrlReg",value);

int dac0[256];
for(int i=0;i<256;i++) dac0[i]=0;
int length=256;
fillDACRegisterLength(dac0,dac0,dac0,length);
usleep(100);

resetSlink();
value=0x1;
vmeDevicePtr->write("ModeReg",value);


generateVMETrigger();

uint32_t data=0x80000000;
 vmeDevicePtr->write("LRES",data);
data=0x80000000;
 vmeDevicePtr->write("CLRES",data);
 
int status = loadControlRegister();
if(status<0)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Error Setting Control Word! status ="<<status<<endl;

status = loadModeRegister();
if(status<0)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Error Setting Mode Word! status ="<<status<<endl;

}

void PixelPh0FEDInterface::drainDigTransFifo(const int chip, uint32_t* data) {
  std::string chipname;
  if      (chip == 1) chipname = "BLAD_N";
  else if (chip == 3) chipname = "BLAD_NC";
  else if (chip == 5) chipname = "BLAD_SC";
  else if (chip == 7) chipname = "BLAD_S";
  else
    return;

  vmeDevicePtr->readBlock(chipname, 1024, (char*)data, HAL::HAL_NO_INCREMENT, 0x20000);
}

void PixelPh0FEDInterface::drainTimestamp(const int chip, uint32_t* data) {
  std::string chipname;
  if      (chip == 1) chipname = "BLAD_N";
  else if (chip == 3) chipname = "BLAD_NC";
  else if (chip == 5) chipname = "BLAD_SC";
  else if (chip == 7) chipname = "BLAD_S";
  else
    return;

  vmeDevicePtr->readBlock(chipname, 1024, (char*)data, HAL::HAL_NO_INCREMENT, 0x40000);
}

//////////////////////////////////////////////////////////////////////
// Read transparent data from FIFO1 for all channels.
int PixelPh0FEDInterface::drain_transBuffer(uint32_t *data) {

//
//transFifo1Length is 1024*4 nothing fancy, just read everything out 
//
  const uint32_t length = transFifo1Length;  // fifo1 length in bytes

  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Read transparent Buffer from FIFO1 for all channels "<<dec<<endl;
  //cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Read transparent Buffer from FIFO1 for all channels "<<dec<<endl;

  int count=0, status=0;
  // Loop over all channels
  for(int chan=1;chan<37;chan++) {
    status = drainFifo1(chan,&data[count],length);
    //cout<<chan<<" "<<count<<" "<<status<<endl;
    if(status>0) count += transFifo1Length/4;
  }

  return count;
}
//////////////////////////////////////////////////////////////////////
// Read transparent data from FIFO1 for one channel.
int PixelPh0FEDInterface::drain_transBuffer(int chnl, uint32_t *data) {
//
///transFifo1Length is 1024*4 then we look at the buffer and assign it
// a length of 0 or 960
//
  uint32_t offset=0;
  string item;
  string item0;

  //cout<<"drain_transBuffer "<<chnl<<endl;

  if(chnl<1) return(-1);
  else if(chnl>36) return(-1);
  else if(chnl<10) {   // Channels in LAD_N
    item = "BLAD_N";
    item0 = "LAD_N";   // old single IO non-block access for V2 version
    offset = CH_SubAddr[chnl-1]  + 0x18000;
  } else if((chnl>9)&(chnl<19)) {
    item = "BLAD_NC";
    item0 = "LAD_NC";
    offset = CH_SubAddr[chnl-10] + 0x18000;
  } else if((chnl>18)&(chnl<28)) {
    item = "BLAD_SC";
    item0 = "LAD_SC";
    offset = CH_SubAddr[chnl-19] + 0x18000;
  } else if((chnl>27)&(chnl<37)) {
    item = "BLAD_S";
    item0 = "LAD_S";
    offset = CH_SubAddr[chnl-28] + 0x18000;
  }

  const uint32_t length = transFifo1Length;  // transparent fifo payload
  //cout<<"drain_transBuffer "<<length<<" "<<item0<<" "<<item<<" "<<offset<<endl;

  // block transfer from fifo1 
  char * buffer = (char *) data;
  //cout<<" readout fifo1 "<<length<<endl;

  uint32_t datadum;
  vmeDevicePtr->read(item0,&datadum,offset);//dummy read to align buffer
  vmeDevicePtr->readBlock(item,length,buffer,HAL::HAL_NO_INCREMENT,offset);

  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Read transparent buffer from FIFO1 for channel "<<dec<<chnl<<endl;
  //cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Read transparent buffer from FIFO1 for channel "<<dec<<chnl<<endl;

  int count=0;

//transparent buffer expected structure
//this is hardwired
//buffer[0-960]
//check buffers [1]-[51], if same, length=0
uint32_t olddata=data[1];
int ibadd=0;
//check 1st 50 buffers, if same, length=0
for(int i=2;i<52;i++){if(data[i]==olddata)ibadd++;}
//if(ibadd==50)return 0;

if(((data[960]&0xff)==0xff)&&((data[959]&0xff)!=0xff)){count+=961;} else
{cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Warning:Transparent buffer Bad or MisAligned! channel ="<<chnl<<endl;
return 1000;}

#ifndef PILOT_FED
//cout<<" call fixBBB "<<chnl<<endl; 
//int newstatus = FixBBB(chnl,data);
 FixBBB(chnl,data);
#endif

  return count;
}
//////////////////////////////////////////////////////////////////////
// Read data from FIFO1 for all channels.
// This needs the recognition of ed of valid data for all channels.
// For normal readout it works OK but fails for the transparent readout.
int PixelPh0FEDInterface::drainFifo1(uint32_t *data) {

  const uint32_t length = spyFifo1Length;  // fifo1 length in bytes

  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Read from FIFO1 for all channels "<<dec<<endl;

  int count=0, status=0;
  // Loop over all channels
  for(int chan=1;chan<37;chan++) {
    status = drainFifo1(chan,&data[count],length);
    //cout<<chan<<" "<<count<<" "<<status<<endl;
    if(status>0) count += status;
  }

  return count;
}
//////////////////////////////////////////////////////////////////////
// Read data from FIFO1 for a single channel.
// This needs the recognition of ed of valid data for all channels.
// For normal readout it works OK but fails for the transparent readout.
int PixelPh0FEDInterface::drainFifo1(int chan, uint32_t *data) {

  const uint32_t length = spyFifo1Length;  // fifo1 length in bytes

  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Read from FIFO1 for channel "<<dec<<chan<<endl;

  int count=0, status=0;
    status = drainFifo1(chan,&data[count],length);
    //cout<<chan<<" "<<count<<" "<<status<<endl;
    if(status>0) count += status;

  return count;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Read data from FIFO1 for one channel.
// What length should we read? The whole FIFO1 1024words?
// The method returns the length of the valid data by checking word repetiotions.
// This works OK for normal data but not for transparent data.
// For transparent data it is best to read and analyze the whole buffer (maximum fifo length).
int PixelPh0FEDInterface::drainFifo1(int chnl, uint32_t *data,
				  const uint32_t length) {
  int wordCount=0;
  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Read from FIFO1 Channel "<<dec<<chnl<<" "<<length<<endl;

#ifdef USE_HAL // Use HAL
  uint32_t offset=0;
  string item;
  string item0;
  if(chnl<1) return(-1);
  else if(chnl>36) return(-1);
  else if(chnl<10) {   // Channels in LAD_N
    item = "BLAD_N";
    item0 = "LAD_N";   // old single IO non-block access for V2 version
    offset = CH_SubAddr[chnl-1]  + 0x18000;
  } else if((chnl>9)&(chnl<19)) {
    item = "BLAD_NC";
    item0 = "LAD_NC";
    offset = CH_SubAddr[chnl-10] + 0x18000;
  } else if((chnl>18)&(chnl<28)) {
    item = "BLAD_SC";
    item0 = "LAD_SC";
    offset = CH_SubAddr[chnl-19] + 0x18000;
  } else if((chnl>27)&(chnl<37)) {
    item = "BLAD_S";
    item0 = "LAD_S";
    offset = CH_SubAddr[chnl-28] + 0x18000;
  }

#else // direct CAEN VME access

  uint32_t vmeAddress;
  if(chnl<1) return(-1);
  else if(chnl>36) return(-1);
  else if(chnl<10)             vmeAddress=CHIP[0]+CH_SubAddr[chnl-1];
  else if((chnl>9)&(chnl<19))  vmeAddress=CHIP[1]+CH_SubAddr[chnl-10];
  else if((chnl>18)&(chnl<28)) vmeAddress=CHIP[2]+CH_SubAddr[chnl-19];
  else if((chnl>27)&(chnl<37)) vmeAddress=CHIP[3]+CH_SubAddr[chnl-28];

#endif // USE_HAL

#ifdef USE_HAL // Use HAL

  // block transfer from fifo1 works with V3 but not with V2?
  char * buffer = (char *) data;
  //cout<<" readout fifo1 "<<length<<endl;
  uint32_t datadum;
  vmeDevicePtr->read(item0,&datadum,offset);//dummy read to align buffer
  vmeDevicePtr->readBlock(item,length,buffer,HAL::HAL_NO_INCREMENT,offset);
  //for(int icx=0;icx<256;icx++) { // READ FIFO1 for V2
  //vmeDevicePtr->read(item0,&data[icx],offset);
  //}

  // Find the real length
  uint32_t olddata=0;
  int count=0;
  // How to find end of data?
  // For the moment I loop over data and check is they are the same.
  // After a sequence od same data I stop. This does not work for transparent data.
  for(uint32_t i = 0; i<(length/4);i++) {
    //if(chnl==1) cout<<i<<" "<<count<<" "<<hex<<data[i]<<dec<<endl;
    if(i>0 && data[i]==olddata) { // check if data the same
      count++;
      if(count>1) {  // exit if too many same data
	wordCount=i;
	break;
      }
    } else {
      olddata = data[i];
      count=0;
    }
  }

#else // direct CAEN VME access

  for(int icx=0;icx<256;icx++) { // READ FIFO1
    ret = CAENVME_ReadCycle(BHandle,vmeAddress,&data[icx],am,dw);
    if(ret != cvSuccess) {  // Error
      cout<<"Error in read "<<hex<<ret<<" "<<data[icx]<<dec<<endl;
      analyzeError(ret);
      return -1;
    }
  }

#endif // USE_HAL

  return wordCount;
} // end
//////////////////////////////////////////////////////////////////////
// Read the data spy-FIFO3.
// Change the e-o-i recognitions from 0xffff to word repetition
int PixelPh0FEDInterface::drainDataFifo3(uint32_t *data) {
  int wordCount = 0;

#ifdef USE_HAL // Use HAL

  int count=0;
  uint32_t offset = 0;
  string item = "RdSpyFifoUp";
  const uint32_t length = spyFifo3Length; // size of SPY-FIFO3 in bytes, is it 128?
  char * buffer = (char *) data;
  vmeDevicePtr->readBlock(item,length,buffer,HAL::HAL_NO_INCREMENT,offset);

  uint32_t olddata = 99999;
  for(uint32_t i=0; i<(length/4);i++) {
    //cout<<i<<" "<<hex<<data[i]<<dec<<endl;
    //if((data[i]&0xffff) == 0xffff) count++;  // how do I find the end?????
    if(i>0 && data[i] == olddata && data[i]!=1) {
      count++;  // count same word
      if(count>0) {
	//cout<<" end "<<i<<" "<<hex<<data[i]<<dec<<endl;
	wordCount=i;
	break;
      }
    }
    olddata=data[i];
  }
  //cout<<length<<" " <<wordCount<<" "<<count<<" "<<hex<<data[0]<<" "<<data[wordCount-1]<<endl;

  //int halfMarker = wordCount;  // split between the up and dw fifos

  item = "RdSpyFifoDn";
  buffer = (char *) &data[wordCount];
  vmeDevicePtr->readBlock(item,length,buffer,HAL::HAL_NO_INCREMENT,offset);
  count=0;
  for(uint32_t i=0; i<(length/4);i++) {
    //cout<<i<<" "<<hex<<data[i+wordCount]<<dec<<endl;
    if(i>0 && data[i+wordCount] == olddata && data[i+wordCount]!=1) {
      count++;  // count same word
      if(count>0) {
	//cout<<" end "<<i<<" "<<hex<<data[i+wordCount]<<dec<<endl;
	wordCount += i;
	break;
      }
    }
    olddata=data[i+wordCount];
  }

  //cout<<length<<" " <<wordCount<<" "<<count<<" "<<hex<<data[halfMarker]<<" "<<data[wordCount-1]
  //  <<dec<<endl;
  
#else // direct CAEN VME access
  cout<<" Not implemented"<<endl;
#endif // USE_HAL

  return wordCount;
}
//////////////////////////////////////////////////////////////////////
// Read the TTS FIFO
int PixelPh0FEDInterface::drainTTSFifo(uint32_t *data) {
  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Read from TTS FIFO "<<endl;
  int wdcnt = 0;

#ifdef USE_HAL // Use HAL
  uint32_t offset = 0;
  string item = "RdTTSFifo";

  // Should it be a block transfer? Probably not, we hope that there are few errors only?
  //wordCount = drainFifo2(item,offset,TTSFifoLength,data); // use the generic FIFO2 routine

  uint32_t data0=0;
  int count=0;
//  int countSame=0;
//  const int countSameMax = 0;
 
  // The error and last-dac fifos akways keep the last value from the previous event.
  // So if for the new event there is no new data (e.g. no errors) the fifos still will give 
  // the last enbtry from the previous event.
  // I should skip this entry either already in the PixelPh0FEDInterface or later in the decoders?
 
  while( count < (TTSFifoLength/4) ) {  // assume error and lastdac fifo same size
    count++;
    vmeDevicePtr->read(item,&data0,offset);  // replace with block transfer?
    
    if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<"TTS "<<wdcnt<<" "<<data0<<" "<<lastTTS<<endl;
    //cout<<count<<" "<<wdcnt<<" "<<hex<<data0<<" "<<lastTTS<<dec<<endl;
    if( data0 == 0 ) { break;
     } else {
      lastTTS=data0;
 	*data=data0;
	if( count < (TTSFifoLength/4) )data++;
	wdcnt++;
    }
    //always return last state of TTS
  } // end while 


#else // direct CAEN VME access
  cout<<" Not implemented"<<endl;
#endif // USE_HAL

    // if(wdcnt==0){wdcnt=1;*data=lastTTS;data++;}//no change, return last state


  return wdcnt;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Read the error FIFO (FIFO2) for all channels
int PixelPh0FEDInterface::drainErrorFifo(uint32_t *data) {
  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Read from Error FIFO for all channels "<<endl;
  int count=0, status=0;
  // Loop over all channels
  for(int chan=1;chan<9;chan++) {  // loop over 8 groups
    status = drainErrorFifo(chan,&data[count]);
    //cout<<chan<<" "<<count<<" "<<status<<endl;
    if(status>0) count += status;
  }
  return count;
}
//////////////////////////////////////////////////////////////////////////////////////////////
// Read the error FIFO (FIFO2) for one group (chip/channel)
// Group = 1,2 Chip=North up, down, 3,4 NorthCenter up,down, 5,6 SouthCenter up,down, 7,8 South
// Modify to a local readout in this method instead of the generic method.
int PixelPh0FEDInterface::drainErrorFifo(const int chip, uint32_t *data) {
  int wordCount = 0;

#ifdef USE_HAL // Use HAL
  uint32_t offset;
  string item;

  if(chip==1)      {
    offset = 0x140000;  // North, UP
    item = "BLAD_N";
  } else if(chip==2) {
    offset = 0x160000;  // North, DOWN
    item = "BLAD_N";
  } else if(chip==3) {
    item = "BLAD_NC";
     offset = 0x140000; // NorthCenter, UP
  } else if(chip==4) {
    item = "BLAD_NC";
    offset = 0x160000; // NorthCenter DOWN
  } else if(chip==5) {
    item = "BLAD_SC";
    offset = 0x140000; // SouthCenter, UP
  } else if(chip==6) {
    item = "BLAD_SC";
    offset = 0x160000; // SouthCenter DOWN
  } else if(chip==7) {
    item = "BLAD_S";
    offset = 0x140000;  // South UP
  } else if(chip==8) {
    item = "BLAD_S";
    offset = 0x160000;  // South DOWN
  } else return(0); // invalid

  // Should it be a block transfer? Probably not, we hope that there are few errors only?
  //wordCount = drainFifo2(item,offset,errorFifoLength,data); // call the generic routine

  //cout<<item<<" "<<hex<<offset<<dec<<" "<<length<<endl;

  uint32_t datanew=0;
  uint32_t dataold=lastErrorValue[chip-1];
  int count=0;
  int countSame=0;
  // This define how many times more the same value sould appear before we exit the fifo reading
  const int countSameMax=1;  // >1 means 2 times, has to be 2 since the real error can be by
  // a coincidence equal to the fake (last stored) error. So we have to be able to skip over it.
  // This also mean that once the error might be missed.

  // The error and last-dac fifos akways keep the last value from the previous event.
  // So if for the new event there is no new data (e.g. no errors) the fifos still will give
  // the last enbtry from the previous event.
  // I should skip this entry either already in the PixelPh0FEDInterface or later in the decoders?

 uint32_t tbuf[256];
  char * buffer = (char *) tbuf;
  //cout<<" readout fifo1 "<<length<<endl;
  uint32_t length=errorFifoLength;
  vmeDevicePtr->readBlock(item,length,buffer,HAL::HAL_NO_INCREMENT,offset);

  while( count < (errorFifoLength/4) ) {  // assume error and lastdac fifo same size
datanew=tbuf[count];
    count++;
//    vmeDevicePtr->read(item,&datanew,offset);  // replace with block transfer?

    if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" "<<wordCount<<" "<<hex <<datanew<<" "<<hex<<dataold<<endl;
    //cout<<count<<" "<<wordCount<<" "<<hex<<datanew<<" "<<dataold<<dec<<endl;

    if( dataold == datanew ) {
      countSame++; // to skip the spy-fifo2 invalid data
      //cout<<"same "<<countSame<<" "<<hex<<datanew<<" "<<dataold<<dec<<endl;
      if(countSame>countSameMax) break; // set threshold for buffer termination
    } else {
      countSame=0;
      dataold=datanew;
      *data=datanew;
      if( count < (errorFifoLength/4) )data++;
      wordCount++;
    }
  } // end while

  // Store the last value
  if(wordCount>0) lastErrorValue[chip-1] = datanew;  // remember the last value for this group (fifo)

#else // direct CAEN VME access

  uint32_t vmeAddress=0;
  if(chip==1)      vmeAddress = LAD_N+0x140000;  // North, UP
  else if(chip==2) vmeAddress = LAD_N+0x160000;  // North DOWN
  else if(chip==3) vmeAddress = LAD_NC+0x140000; // NorthCenter, UP
  else if(chip==4) vmeAddress = LAD_NC+0x160000; // NorthCenter DOWN
  else if(chip==5) vmeAddress = LAD_SC+0x140000; // SouthCenter, UP
  else if(chip==6) vmeAddress = LAD_SC+0x160000; // SouthCenter DOWN
  else if(chip==7) vmeAddress = LAD_S+0x140000;  // South UP
  else if(chip==8) vmeAddress = LAD_S+0x160000;  // South DOWN
  else return(0); // invalid

  wordCount = drainFifo2(vmeAddress, data);

#endif // USE_HAL
   

  if(Printlevel&2)
    cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Drain Error FIFO for channel "<<chip<<" count = "<<wordCount<<endl;

  return wordCount;
}
/////////////////////////////////////////////////////////////////////
// Read the temperature FIFO (FIFO2) for all channels
int PixelPh0FEDInterface::drainTemperatureFifo(uint32_t *data) {
  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Read from Temperature FIFO for all channels "<<endl;
  int count=0, status=0;
  // Loop over all channels
  for(int chan=1;chan<9;chan++) {
    status = drainTemperatureFifo(chan,&data[count]);
    //cout<<chan<<" "<<count<<" "<<status<<endl;
    if(status>0) count += status;
  }

  return count;
}

/////////////////////////////////////////////////////////////////////
// Read the Last DAC FIFO (FIFO2) for all channels
int PixelPh0FEDInterface::drainLastDACFifo(uint32_t *data){
  if(Printlevel&2){
    cout << "FEDID: " << pixelFEDCard.fedNumber << " Read from Last DAC FIFO for all channels " << endl;
  }
  int count = 0, status = 0;

  for(int chan = 1; chan < 9; chan++){  //loop over all channels
    status = drainTemperatureFifo(chan, &data[count]);
    if(status > 0){
      count += status;
    }
  }

  return count;
}

///////////////////////////////////////////////////////////////////////////////////////
// Read the temperature FIFO (FIFO2) for one channel
// Return number of words
int PixelPh0FEDInterface::drainTemperatureFifo(const int chip, uint32_t *data) {
  int wordCount = 0;

#ifdef USE_HAL // Use HAL

  uint32_t offset;
  string item;

  if(chip==1)      {
    offset = 0x148000;  // North, UP
    item = "BLAD_N";
  } else if(chip==2) {
    offset = 0x168000;  // North, DOWN
    item = "BLAD_N";
  } else if(chip==3) { 
    item = "BLAD_NC";
     offset = 0x148000; // NorthCenter, UP
  } else if(chip==4) { 
    item = "BLAD_NC";
    offset = 0x168000; // NorthCenter DOWN
  } else if(chip==5) { 
    item = "BLAD_SC";
    offset = 0x148000; // SouthCenter, UP
  } else if(chip==6) { 
    item = "BLAD_SC";
    offset = 0x168000; // SouthCenter DOWN
  } else if(chip==7) { 
    item = "BLAD_S";
    offset = 0x148000;  // South UP
  } else if(chip==8) { 
    item = "BLAD_S";
    offset = 0x168000;  // South DOWN
  } else return(0); // invalid

  // I assume that for  a few values only blocktransfer does not help. TBD?
  //wordCount = drainFifo2(item,offset,lastDACFifoLength,data); // call the generic routine


  uint32_t datanew=0;
  uint32_t dataold=lastDACValue[chip-1];
  int count=0;
  int countSame=0;
  // This define how many times more the same value sould appear before we exit the fifo reading
  const int countSameMax=1;  // >1 means 2 times, has to be 2 since the real error can be by
  // a coincidence equal to the fake (last stored) error. So we have to be able to skip over it.
  // This also mean that once the error might be missed. 

  // The error and last-dac fifos akways keep the last value from the previous event.
  // So if for the new event there is no new data (e.g. no errors) the fifos still will give 
  // the last enbtry from the previous event.
  // I should skip this entry either already in the PixelPh0FEDInterface or later in the decoders?
 uint32_t tbuf[256];
  char * buffer = (char *) tbuf;
  //cout<<" readout fifo1 "<<length<<endl;
  uint32_t length=lastDACFifoLength;
  vmeDevicePtr->readBlock(item,length,buffer,HAL::HAL_NO_INCREMENT,offset);
 
  while( count < (lastDACFifoLength/4) ) {  // assume error and lastdac fifo same size
   datanew=tbuf[count];
    count++;
//    vmeDevicePtr->read(item,&datanew,offset);  // replace with block transfer?

    if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" "<<wordCount<<" "<<datanew<<" "<<dataold<<endl;
    //cout<<count<<" "<<wordCount<<" "<<hex<<datanew<<" "<<dataold<<dec<<endl;

    if( dataold == datanew ) {
      countSame++; // to skip the spy-fifo2 invalid data
      //cout<<"same "<<countSame<<" "<<hex<<datanew<<" "<<dataold<<dec<<endl;
      if(countSame>countSameMax) break; // set threshold for buffer termination
    } else {
      countSame=0;
      dataold=datanew;
      *data=datanew;
      if( count < (lastDACFifoLength/4) )data++;
      wordCount++;
    }
  } // end while

  // Store the last value
  if(wordCount>0) lastDACValue[chip-1] = datanew;  // remember the last value for this group (fifo)


#else // direct CAEN VME access

  uint32_t vmeAddress=0;
  if(chip==1)      vmeAddress = LAD_N+ 0x148000; // North, UP
  else if(chip==2) vmeAddress = LAD_N+ 0x168000; // North DOWN
  else if(chip==3) vmeAddress = LAD_NC+0x148000; // NorthCenter, UP
  else if(chip==4) vmeAddress = LAD_NC+0x168000;// NorthCenter DOWN
  else if(chip==5) vmeAddress = LAD_SC+0x148000; // SouthCenter, UP
  else if(chip==6) vmeAddress = LAD_SC+0x168000; // SouthCenter DOWN
  else if(chip==7) vmeAddress = LAD_S+ 0x148000; // South UP
  else if(chip==8) vmeAddress = LAD_S+ 0x168000; // South DOWN
  else return(0); // invalid
  int wordCount = drainFifo2(vmeAddress, data);
#endif // USE_HAL

  if(Printlevel&2)
    cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Drain Temperatute FIFO for channel "<<chip<<" count = "<<wordCount<<endl;
  return wordCount;
}
////////////////////////////////////////////////////////////////////
// Read the data FIFO (FIFO2) for all channels
int PixelPh0FEDInterface::drainDataFifo2(uint32_t *data) {
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Read from Data FIFO for all channels "<<endl;
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Does not work yet "<<endl;
  return 0;
}
// Read the data FIFOs (FIFO2) for one channel
int PixelPh0FEDInterface::drainDataFifo2(const int chip, uint32_t *data) {

  int wordCount = 0;

#ifdef USE_HAL // Use HAL

  uint32_t offset;
  string item;

  if(chip==1)      {
    offset = 0x150000;  // North, UP
    item = "BLAD_N";
  } else if(chip==2) {
    offset = 0x170000;  // North, DOWN
    item = "BLAD_N";
  } else if(chip==3) {
    item = "BLAD_NC";
    offset = 0x150000; // NorthCenter, UP
  } else if(chip==4) {
    item = "BLAD_NC";
    offset = 0x170000; // NorthCenter DOWN
  } else if(chip==5) { 
    item = "BLAD_SC";
    offset = 0x150000; // SouthCenter, UP
  } else if(chip==6) { 
    item = "BLAD_SC";
    offset = 0x170000; // SouthCenter DOWN
  } else if(chip==7) { 
    item = "BLAD_S";
    offset = 0x150000;  // South UP
  } else if(chip==8) { 
    item = "BLAD_S";
    offset = 0x170000;  // South DOWN
  } else return(0); // invalid


  // Single transfer readout
  //wordCount = drainFifo2("LAD_N",offset,data); // call the generic routine

  // Chanege to block read at some point
  const uint32_t length = 4096; //spyFifo2Length; // size of SPY-FIFO2 in bytes, is it 128? 
  char * buffer = (char *) data;
  vmeDevicePtr->readBlock(item,length,buffer,HAL::HAL_NO_INCREMENT,offset);
  //find the wordCount?
  for(uint32_t i=0; i<(length/4);i++) {
    //cout<<i<<" "<<hex<<data[i]<<dec<<endl;
    if(i>0 && data[i]==0) {  // Assume taht after data=0 there is nothing more? 
      wordCount=i;          // Problem, sometomes 0s are already at the beginning.
      break;
    }
    wordCount=i;
  }


#else // direct CAEN VME access

  uint32_t vmeAddress=0;  
  if(chip==1) vmeAddress =      LAD_N+ 0x150000; // North, UP
  else if(chip==2) vmeAddress = LAD_N+ 0x170000; // North DOWN
  else if(chip==3) vmeAddress = LAD_NC+0x150000; // NorthCenter, UP
  else if(chip==4) vmeAddress = LAD_NC+0x170000; // NorthCenter DOWN
  else if(chip==5) vmeAddress = LAD_SC+0x150000; // SouthCenter, UP
  else if(chip==6) vmeAddress = LAD_SC+0x170000; // SouthCenter DOWN
  else if(chip==7) vmeAddress = LAD_S+ 0x150000; // South UP
  else if(chip==8) vmeAddress = LAD_S+ 0x170000; // South DOWN
  else return(0); // invalid
  
  int wordCount = drainFifo2(vmeAddress, data);

#endif // USE_HAL

  if(Printlevel&2)
    cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Drain FIFO2 for channel "<<chip<<" count = "<<wordCount<<endl;
  return wordCount;
}
//////////////////////////////////////////////////////////////////////
// // drains a fifo till word comes back the same (generic for all FIFO2 accesses)
// // returns 0 (empty) or # of words and a pointer to 1st word
#ifdef USE_HAL // Use HAL
int PixelPh0FEDInterface::drainFifo2(string item, uint32_t offset, const uint32_t length, 
			 uint32_t *pnt) {
#else // direct CAEN VME access
int PixelPh0FEDInterface::drainFifo2(uint32_t VmeAddress, uint32_t *pnt) {
#endif // USE_HAL
  
  //cout<<item<<" "<<hex<<offset<<dec<<" "<<length<<endl;

  uint32_t data=0;
  uint32_t dataold=999999;
  int wdcnt=0;
  int count=0;
  int countSame=0;
 
  // The error and last-dac fifos akways keep the last value from the previous event.
  // So if for the new event there is no new data (e.g. no errors) the fifos still will give 
  // the last enbtry from the previous event.
  // I should skip this entry either already in the PixelPh0FEDInterface or later in the decoders?
 
  while( count < (errorFifoLength/4) ) {  // assume error and lastdac fifo same size
    count++;

#ifdef USE_HAL // Use HAL

    vmeDevicePtr->read(item,&data,offset);  // replace with block transfer?

#else // direct CAEN VME access

    CVErrorCodes ret = CAENVME_ReadCycle(BHandle, VmeAddress,&data,am,dw);
    if(ret != cvSuccess) {  // Error
      cout<<"Error in Draining the Fifo "<<hex<<ret<<" "<<data<<dec<<endl;
      analyzeError(ret);
    }

#endif // USE_HAL

    if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" "<<wdcnt<<" "<<data<<" "<<dataold<<endl;
    //cout<<count<<" "<<wdcnt<<" "<<hex<<data<<" "<<dataold<<dec<<endl;

    if( dataold == data ) {
      if(data!=1) countSame++; // to skip the spy-fifo2 invalid data
      //cout<<"same "<<countSame<<" "<<data<<" "<<dataold<<endl;
      if(countSame>1) break; // set threshold for buffer termination
    } else {
      countSame=0;
      dataold=data;
      if(data>0) {
	*pnt=data;
	if( count < (errorFifoLength/4) )pnt++;
	wdcnt++;
      }
    }
  } // end while 

  return(wdcnt);
 } //end

////////////////////////////////////////////////////////////////////////////
// Load the MODE register from the internaly stored value (modeRegister)
int PixelPh0FEDInterface::loadModeRegister(void) {
  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Load Mode register "<<hex<<pixelFEDCard.modeRegister<<dec<<endl;
  int status = setModeRegister(pixelFEDCard.modeRegister);
  return status;
} // end
////////////////////////////////////////////////////////////////////////////
// Set the MODE register to mode.
int PixelPh0FEDInterface::setModeRegister(int mode) {
  //verbose cout<<"Set Mode register "<<hex<<mode<<dec<<endl;
  uint32_t data = mode; // take from the argument
  pixelFEDCard.modeRegister = data;  // Store it

#ifdef USE_HAL // Use HAL

  vmeDevicePtr->write("ModeReg",data);

#else // direct CAEN VME access
  ret = CAENVME_WriteCycle(BHandle,MODE,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret);
    return -1;
  }
#endif // USE_HAL

  return 0;
} // end
////////////////////////////////////////////////////////////////////////////
// 
// I am not sure what this does?
int PixelPh0FEDInterface::enableSpyMemory(const int enable) {
  if(enable==1) {

    // set the mode to enable write to spy memory
    //    cout<<"Enable spy memory "<<endl;
    pixelFEDCard.modeRegister = pixelFEDCard.modeRegister & 0xfffffffd; // put 0 in bit 1, keep rest the same  
  } else {

    // set the mode to disable write to spy memory
    //    cout<<"Disable spy memory "<<endl;
    pixelFEDCard.modeRegister = pixelFEDCard.modeRegister | 0x2; // put 1 in bit 1, keep rest the same  
  }

  int status = setModeRegister(pixelFEDCard.modeRegister);

  return status;
} // end
////////////////////////////////////////////////////////////////////////////
// Sent an event via vme
int PixelPh0FEDInterface::generateVMETrigger(void) {
  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" PixelPh0FEDInterface::generateVMETrigger(void)"<<endl;
  uint32_t data = 0x200; // Event 1 (the event number seems to be always 0 unless you
                              // set it first using the routine below CAUTION: do not
			      // set the event number(bit 8) and trigger (bit 9) at the same time.)
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("WrEventNum",data);

#else // direct CAEN VME access
  ret = CAENVME_WriteCycle(BHandle,EVENT_TRIG,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret);
  }
#endif // USE_HAL
  usleep(100);
  return 0;
} // end
////////////////////////////////////////////////////////////////////////////
int PixelPh0FEDInterface::setVMEevntnmbr(const int value) {
  //cout<<"Set VME event number = "<<dec<<value<<endl;
  uint32_t data = 0x100+(value&0xff); // lower 8 bits(7-0) for event number, next bit(8) to write 
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("WrEventNum",data);

#else // direct CAEN VME access
  ret = CAENVME_WriteCycle(BHandle,WrEventNum,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret); 
    return -1;
  }
#endif  
  return 0;
} // end
////////////////////////////////////////////////////////////////////////////
// Load the Control Register with the value stored internall (Ccntrl)
int PixelPh0FEDInterface::loadControlRegister(void) {
  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Load Control register from DB 0x"<<hex<<pixelFEDCard.Ccntrl<<dec<<endl;
  int data = pixelFEDCard.Ccntrl; // take from DB
  int status = setControlRegister(data);
  return status;
}
////////////////////////////////////////////////////////////////////////////
// Set the Control Register to value
 int PixelPh0FEDInterface::setControlRegister(const int value) {
   if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Set Control register "<<hex<<value<<dec<<endl;
   uint32_t data = value;
#ifdef USE_HAL // Use HAL
   vmeDevicePtr->write("CtrlReg",data);

#else // direct CAEN VME access
   ret = CAENVME_WriteCycle(BHandle,CTRL,&data,am,dw);
   if(ret != cvSuccess) {  // Error
     cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
     analyzeError(ret); 
     return -1;
   }
#endif // USE_HAL
   pixelFEDCard.Ccntrl=data; // stored this value   
   return 0;
 }
 ////////////////////////////////////////////////////////////////////////////
 // Read the control register. This does not seem to work?
 int PixelPh0FEDInterface::getControlRegister(void) {
  uint32_t data = 0; 
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->read("CtrlReg",&data);
  
#else // direct CAEN VME access
  ret = CAENVME_ReadCycle(BHandle,CTRL,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in read "<<hex<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret); 
  }
#endif // USE_HAL
  //cout<<hex<<data<<dec<<endl;
  return data;
}
////////////////////////////////////////////////////////////////////////////
// Set the fedid to value
bool PixelPh0FEDInterface::setFedIDRegister(const int value) {

   cout<<"Set FEDID register "<<hex<<value<<dec<<endl;
   uint32_t data = value;
   vmeDevicePtr->write("SetFedID",data);
   uint32_t read = getFedIDRegister();
   bool ok = data == read;
   if(!ok) cout<<"soft FEDID = "<<data<<" doesn't match hard board FEDID = "<<status<<endl;
   return ok;
 }
 ////////////////////////////////////////////////////////////////////////////
 // Read the permanent hardware register.
uint32_t PixelPh0FEDInterface::getFedIDRegister() { 
  uint32_t data = 0; 
  vmeDevicePtr->read("READ_GA",&data);
  return data;
}


  ////////////////////////////////////////////////////////////////////////////
void PixelPh0FEDInterface::set_Hold(void) {
  //set the holdoff from the database
	uint32_t data = (uint32_t) pixelFEDCard.TriggerHoldoff;
	
  vmeDevicePtr->write("LAD_C",data,HAL::HAL_NO_VERIFY,0x0e8000);

}
  ////////////////////////////////////////////////////////////////////////////
void PixelPh0FEDInterface::set_Hold(uint32_t data) {
  //set the holdoff, over-ride the database
  cout<<"Over-riding data base value for the trigger holdoff!!!!! Value set to "<<dec<<data<<endl;

  vmeDevicePtr->write("LAD_C",data,HAL::HAL_NO_VERIFY,0x0e8000);

}
  ////////////////////////////////////////////////////////////////////////////
void PixelPh0FEDInterface::set_SimulatedHits(void) {
  // set the number of hits/ROC from the database
  
  //copied out of setHitsHold.cpp
  int SubAddr_Sim[9];
  SubAddr_Sim[0] = 0x3c000; 
  SubAddr_Sim[1] = 0x5c000; 
  SubAddr_Sim[2] = 0x7c000;
  SubAddr_Sim[3] = 0x9c000; 
  SubAddr_Sim[4] = 0xbc000; 
  SubAddr_Sim[5] = 0xdc000;
  SubAddr_Sim[6] = 0xfc000; 
  SubAddr_Sim[7] = 0x11c000; 
  SubAddr_Sim[8] = 0x13c000;
  uint32_t data = (uint32_t) pixelFEDCard.SimHitsPerRoc;
  if (data > 50) data=50;

  if(Printlevel&2) 
    cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Setting number of simulated hits/ROC to "<<data<<endl;

  for(int jk=0;jk<9;jk++){ //loop through all 9 channels in each FPGA
    
    vmeDevicePtr->write("LAD_N",data,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);

    vmeDevicePtr->write("LAD_NC",data,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);	

    vmeDevicePtr->write("LAD_SC",data,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);	

    vmeDevicePtr->write("LAD_S",data,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);
  }

}

  ////////////////////////////////////////////////////////////////////////////
void PixelPh0FEDInterface::set_SimulatedHits(uint32_t data) {
  // set the number of hits/ROC over-ride the database
  
  //copied out of setHitsHold.cpp
  int SubAddr_Sim[9];
  SubAddr_Sim[0] = 0x3c000; 
  SubAddr_Sim[1] = 0x5c000; 
  SubAddr_Sim[2] = 0x7c000;
  SubAddr_Sim[3] = 0x9c000; 
  SubAddr_Sim[4] = 0xbc000; 
  SubAddr_Sim[5] = 0xdc000;
  SubAddr_Sim[6] = 0xfc000; 
  SubAddr_Sim[7] = 0x11c000; 
  SubAddr_Sim[8] = 0x13c000;
  if (data > 50) data=50;
  cout<<"Over-riding data base value for Simulated hits/roc!!!!! Value set to "<<dec<<data<<endl;

  if(Printlevel&2) 
    cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Setting number of simulated hits/ROC to "<<data<<endl;

  for(int jk=0;jk<9;jk++){ //loop through all 9 channels in each FPGA
    
    vmeDevicePtr->write("LAD_N",data,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);

    vmeDevicePtr->write("LAD_NC",data,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);	

    vmeDevicePtr->write("LAD_SC",data,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);	

    vmeDevicePtr->write("LAD_S",data,HAL::HAL_NO_VERIFY,SubAddr_Sim[jk]);
  }

}

/////////////////////////////////////////////////////////////////////////////
// Read VME event couinter
int PixelPh0FEDInterface::readEventCounter() {
  //Check VME event counter
  uint32_t data; 

#ifdef USE_HAL // Use HAL
  //vmeDevicePtr->read("RdEventCntr",&data);
  vmeDevicePtr->read("RdEventCntrVME",&data); // switch to the new counter

#else // direct CAEN VME access
  CVErrorCodes ret = CAENVME_ReadCycle(BHandle,EVENT_NUM,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in read "<<hex<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret);
  }
#endif // USE_HAL

  data = data & 0xffffff;  // mask the upper 8 bits, used for lock counter

  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Event "<<hex<<data<<dec<<endl;
  return(data);
} // end
///////////////////////////////////////////////////////////////////
// Just flip the bit to reset the Slink. Return to the previous setting.
void PixelPh0FEDInterface::resetSlink() {

  uint32_t data = 0x4+pixelFEDCard.modeRegister; // reset S-Link

#ifdef USE_HAL // Use HAL

  vmeDevicePtr->write("ModeReg",data);

#else // direct CAEN VME access
  CVErrorCodes ret = CAENVME_WriteCycle(BHandle,MODE,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret); 
  }
#endif // USE_HAL

  usleep(10);		    
  
  data = pixelFEDCard.modeRegister; // back to usual

#ifdef USE_HAL // Use HAL

  vmeDevicePtr->write("ModeReg",data);

#else // direct CAEN VME access
  ret = CAENVME_WriteCycle(BHandle,MODE,&data,am,dw);	
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret); 
  }
#endif // USE_HAL

  usleep(10);		    

} // end
////////////////////////////////////////////////////////////////////////////
void PixelPh0FEDInterface::set_opto_params() {
assert(0);
  //form opto word from params
  uint32_t data = (pixelFEDCard.opt_cap[0]<<6)+pixelFEDCard.opt_inadj[0]+(pixelFEDCard.opt_ouadj[0]<<4);
  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Write OptoRec1Par :"<<hex<<pixelFEDCard.opt_cap[0]<<" "<<pixelFEDCard.opt_ouadj[0]<<" "
      <<pixelFEDCard.opt_inadj[0]<<dec<<endl;

#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("OptoRec1Par",data);

#else // direct CAEN VME access
  ret = CAENVME_WriteCycle(BHandle,OPTOPAR_RECEIVERS1,&data,am,dw);	
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<OPTOPAR_RECEIVERS1<<" "
	<<data<<dec<<endl;
    analyzeError(ret); }
#endif // USE_HAL

  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Write OptoRec2Par :"<<hex<<pixelFEDCard.opt_cap[1]<<" "<<pixelFEDCard.opt_ouadj[1]<<" "
      <<pixelFEDCard.opt_inadj[1]<<dec<<endl;
  data = (pixelFEDCard.opt_cap[1]<<6)+pixelFEDCard.opt_inadj[1]+(pixelFEDCard.opt_ouadj[1]<<4); 

#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("OptoRec2Par",data);
#else // direct CAEN VME access
  ret = CAENVME_WriteCycle(BHandle,OPTOPAR_RECEIVERS2,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<OPTOPAR_RECEIVERS2<<" "
	<<data<<dec<<endl;
    analyzeError(ret); 
  } 
#endif // USE_HAL

  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Write OptoRec3Par :"<<hex<<pixelFEDCard.opt_cap[2]<<" "<<pixelFEDCard.opt_ouadj[2]<<" "
		<<pixelFEDCard.opt_inadj[2]<<dec<<endl;
  data =(pixelFEDCard.opt_cap[2]<<6)+pixelFEDCard.opt_inadj[2]+(pixelFEDCard.opt_ouadj[2]<<4);  

#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("OptoRec3Par",data);
#else // direct CAEN VME access
  ret = CAENVME_WriteCycle(BHandle,OPTOPAR_RECEIVERS3,&data,am,dw);	
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<OPTOPAR_RECEIVERS3<<" "
	<<data<<dec<<endl;
    analyzeError(ret); 
  }
#endif // USE_HAL
} // end
//////////////////////////////////////////////////////////////////////////
// Set the OFFSET DACs
// Offsets are 8bit so the range is 0-255.
void PixelPh0FEDInterface::set_offset_dacs() {

  for(int ix=0;ix<12;ix++){
    // data for Offset
    //        ch 1-12^  Ch#^  Offs^
    uint32_t data = 0x1000 + (ix<<8) + (pixelFEDCard.offs_dac[ix] & 0xff); 
    if(Printlevel&2) 
      cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Input 1, channel "<<ix<<" "<<hex<<pixelFEDCard.offs_dac[ix]<<dec<<endl;

#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("DacChanOffset",data);

#else // direct CAEN VME access
    CVErrorCodes ret = CAENVME_WriteCycle(BHandle,OFFSET_DAC,&data,am,dw);
    if(ret != cvSuccess) {  // Error
      cout<<"Error in write Offset DAC ch 1-12 "<<hex<<ret<<" "<<data<<dec
	  <<endl;
      analyzeError(ret); 
    }		    
#endif // USE_HAL
    
    //  ch 13-24^  Ch#^  Offs^
    data = 0x2000+(ix<<8)+(pixelFEDCard.offs_dac[ix+12]& 0xff); // data for Offset
    if(Printlevel&2) 
      cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Input 2, channel "<<(ix+12)<<" "<<hex<<pixelFEDCard.offs_dac[ix+12]<<dec<<endl;
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("DacChanOffset",data);

#else // direct CAEN VME access
    ret = CAENVME_WriteCycle(BHandle,OFFSET_DAC,&data,am,dw);
    if(ret != cvSuccess) {  // Error
      cout<<"Error in write Offset DAC ch 13-24"<<hex<<ret<<" "<<data<<dec
	  <<endl;
      analyzeError(ret); 
    }
#endif // USE_HAL
  
    //  ch 25-36^  Ch#^  Offs^
    data = 0x4000+(ix<<8)+(pixelFEDCard.offs_dac[ix+24]& 0xff); // data for Offset
    if(Printlevel&2) 
      cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Input 3, channel "<<(ix+24)<<" "<<hex<<pixelFEDCard.offs_dac[ix+24]<<dec<<endl;

#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("DacChanOffset",data);

#else // direct CAEN VME access
    ret = CAENVME_WriteCycle(BHandle,OFFSET_DAC,&data,am,dw);
    if(ret != cvSuccess) {  // Error
      cout<<"Error in write Offset DAC ch 25-36"<<hex<<ret<<" "<<data<<dec<<dec
	  <<endl;
      analyzeError(ret); 
    }		    
#endif // USE_HAL
  } 
		    
  usleep(10000); // Time for offset DAC to settle
}

///////////////////////////////////////////////////////////////////////////
  int PixelPh0FEDInterface::setPhases(const int channel, int delay) {
assert(0);
    // Select the +- clock phase 
    //uint32_t data = 0; //

    // Check min and max value
    if(delay<0) delay=0;
    //else if(delay>15) delay=15;
    else if(delay>31) delay=31;

    if(delay>15) {
      delay = delay - 16;
      pixelFEDCard.clkphs1_9   = 0x0; // 
      pixelFEDCard.clkphs10_18 = 0x0; // 
      pixelFEDCard.clkphs19_27 = 0x0; // 
      pixelFEDCard.clkphs28_36 = 0x0; // 
    } else {
      pixelFEDCard.clkphs1_9   = 0x1ff; // use this 
      pixelFEDCard.clkphs10_18 = 0x1ff; // 
      pixelFEDCard.clkphs19_27 = 0x1ff; // 
      pixelFEDCard.clkphs28_36 = 0x1ff; // 
    }

//     // Ignore channel (TESTING ONLY)
//     if(delay<8) { // use N phase, bit set to 1 
//       pixelFEDCard.clkphs1_9   = 0x1ff; // use this 
//       pixelFEDCard.clkphs10_18 = 0x1ff; // 
//       pixelFEDCard.clkphs19_27 = 0x1ff; // 
//       pixelFEDCard.clkphs28_36 = 0x1ff; //

// //       pixelFEDCard.clkphs1_9   = 0x0; // 
// //       pixelFEDCard.clkphs10_18 = 0x0; // 
// //       pixelFEDCard.clkphs19_27 = 0x0; // 
// //       pixelFEDCard.clkphs28_36 = 0x0; // 

//     } else {   // P phase, bit set to 0

//       pixelFEDCard.clkphs1_9   = 0x0; // use this 
//       pixelFEDCard.clkphs10_18 = 0x0; // 
//       pixelFEDCard.clkphs19_27 = 0x0; // 
//       pixelFEDCard.clkphs28_36 = 0x0; // 

// //       pixelFEDCard.clkphs1_9   = 0x1ff; // 
// //       pixelFEDCard.clkphs10_18 = 0x1ff; // 
// //       pixelFEDCard.clkphs19_27 = 0x1ff; // 
// //       pixelFEDCard.clkphs28_36 = 0x1ff; // 

//     } // TESTING


//     // Adjust the N/P phase window
//     if(channel<1) {  // invald
//       return(-1);

//     } else if(channel<10) {  // channels 1-9, in N
//       data = pixelFEDCard.clkphs1_9; // 
//       if(delay<8) // use N phase, bit set to 1 
// 	data = data | (0x1 << (channel-1)); //set to 1
//       else
// 	data = data & ~(0x1 << (channel-1)); //set to 0
//       pixelFEDCard.clkphs1_9=data;

//     } else if(channel<19) {  // channels 10-18, in NC
//       data = pixelFEDCard.clkphs10_18; // 
//       if(delay<8) // use N phase, bit set to 1 
// 	data = data | (0x1 << (channel-10)); //set to 1
//       else
// 	data = data & ~(0x1 << (channel-10)); //set to 0
//       pixelFEDCard.clkphs10_18=data;

//     } else if(channel<28) {  // channels 19-27, in SC
//       data = pixelFEDCard.clkphs19_27; // 
//       if(delay<8) // use N phase, bit set to 1 
// 	data = data | (0x1 << (channel-19)); //set to 1
//       else
// 	data = data & ~(0x1 << (channel-19)); //set to 0
//       pixelFEDCard.clkphs19_27=data;

//     } else if(channel<37) {  // channels 28-36, in S
//       data = pixelFEDCard.clkphs28_36; // 
//       if(delay<8) // use N phase, bit set to 1 
// 	data = data | (0x1 << (channel-28)); //set to 1
//       else
// 	data = data & ~(0x1 << (channel-28)); //set to 0
//       pixelFEDCard.clkphs28_36=data;

//     } else return(-1);

    set_clock_phases(); // Load the edge selection

    //cout<<" Set phase for channel "<<channel<<" to "<<delay<<endl;
    // Redefie the phase value
    pixelFEDCard.DelayCh[channel-1]=delay;  // Adjust the delat for channel 
    //for(int i =1; i<37; i++)  pixelFEDCard.DelayCh[i-1]=delay; // testing only
    set_chnl_delays(); // Load delay setting

    return 0;
  }
///////////////////////////////////////////////////////////////////////////////////
int PixelPh0FEDInterface::setPhases(void) {
assert(0);
  // Select the +- clock phase 
  set_clock_phases();
  // Select the phase values
  set_chnl_delays();
  return 0;
}
///////////////////////////////////////////////////////////////////////////
  int PixelPh0FEDInterface::setClockDelayAndPhase(int chan, int delay, int phase) {

    assert(chan>0);
    assert(chan<=36);
    assert(delay>=0);
    assert(delay<16);
    assert(phase==0||phase==1);

    // Check min and max value
    if(delay<0) {
      delay=0;
    }
    else if(delay>15) {
      delay=15;
    }
    if(phase<0) {
      phase=0;
    }
    else if(phase>1) {
      phase=1;
    }

    // retrieve the phase data and mask according to channel

    if(chan<1) {  // invalid
      return(-1);
    } 
    else if(chan<10) {  // channels 1-9, in N
      int data = pixelFEDCard.clkphs1_9;
      //cout << "set phase of channel " << chan << " to " << phase << endl;
      //cout << "OLD clkphs1_9 = " << data << endl;
      data = (data & ~(1 << (chan-1))) | (phase << (chan-1)); //set to 0 or 1
      //cout << "NEW clkphs1_9 = " << data << endl;
      pixelFEDCard.clkphs1_9=data;
     } 
    else if(chan<19) {  // channels 10-18, in NC
       int data = pixelFEDCard.clkphs10_18; 
       data = (data & ~(1 << (chan-10))) | (phase << (chan-10)); //set to 0 or 1
       pixelFEDCard.clkphs10_18=data;
    } 
    else if(chan<28) {  // channels 19-27, in SC
      int data = pixelFEDCard.clkphs19_27; 
      data = (data & ~(1 << (chan-19))) | (phase << (chan-19)); //set to 0 or 1
      pixelFEDCard.clkphs19_27=data;
    } 
    else if(chan<37) {  // channels 28-36, in S
       int data = pixelFEDCard.clkphs28_36; // 
       data = (data & ~(1 << (chan-28))) | (phase << (chan-28)); //set to 0 or 1
       pixelFEDCard.clkphs28_36=data;
    }
else return(-1);

    set_clock_phases(); // Load the phases

    //cout<<" Set delay for channel "<<chan<<" to "<<delay<<endl;
    pixelFEDCard.DelayCh[chan-1]=delay;  // Adjust the delay for channel 
    set_chnl_delays(); // Load delay setting

    return 0;
  }

///////////////////////////////////////////////////////////////////////////////////
void PixelPh0FEDInterface::set_clock_phases() {
assert(0);
/* a 9 bit number, 1 bit for each of 9 channels in each of 4 chips
1=on   means use negative clock edge    
0=off        use positive clock edge
for now all set to negative clock edge
*/
  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Set phase to "<<hex<<pixelFEDCard.clkphs1_9<<" "<<pixelFEDCard.clkphs10_18<<" "
		<<pixelFEDCard.clkphs19_27<<" "<<pixelFEDCard.clkphs28_36<<dec<<endl;
  //cout<<" Set clock edges to "<<hex<<pixelFEDCard.clkphs1_9<<" "<<pixelFEDCard.clkphs10_18<<" "
  //  <<pixelFEDCard.clkphs19_27<<" "<<pixelFEDCard.clkphs28_36<<dec<<endl;

 uint32_t data = pixelFEDCard.clkphs1_9; // data all negative clock phase

#ifdef USE_HAL // Use HAL

  vmeDevicePtr->write("ClockEdgeN",data);

#else // direct CAEN VME access
  CVErrorCodes ret = CAENVME_WriteCycle(BHandle,PHASE[0],&data,am,dw);	
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret); 
  }		    
#endif // USE_HAL

  data = pixelFEDCard.clkphs10_18; // data all negative clock phase

#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("ClockEdgeNC",data);

#else // direct CAEN VME access
  ret = CAENVME_WriteCycle(BHandle,PHASE[1],&data,am,dw);	
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret); 
  }		    
#endif // USE_HAL

  data = pixelFEDCard.clkphs19_27; // data all negative clock phase

#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("ClockEdgeSC",data);

#else // direct CAEN VME access
  ret = CAENVME_WriteCycle(BHandle,PHASE[2],&data,am,dw);	
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret);
  }		    
#endif // USE_HAL

  data = pixelFEDCard.clkphs28_36; // data all negative clock phase

#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("ClockEdgeS",data);

#else // direct CAEN VME access
  ret = CAENVME_WriteCycle(BHandle,PHASE[3],&data,am,dw);	
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret); 
  }
#endif // USE_HAL

} // end
////////////////////////////////////////////////////////////////////////////// 
//   This one is a little different as the clock distribution is
//   controlled on the center chip, and we are setting delays
//   for each input channel 1-36 		    
 void PixelPh0FEDInterface::set_chnl_delays() {
assert(0);
   for(int channel=1;channel<37;channel++) {

     //      enable channel(6 bits) delay(4 bits)
     uint32_t data = 
       0x800+((channel&0x3f)<<4) + (pixelFEDCard.DelayCh[channel-1]&0xf); // data for delay
     
     if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" set delay for channel "<<channel<<" "<<pixelFEDCard.DelayCh[channel-1]<<endl;
     //cout<<" Set phase for channel "<<channel<<" to "<<pixelFEDCard.DelayCh[channel-1]<<endl;

#ifdef USE_HAL // Use HAL

     vmeDevicePtr->write("ClockPhase",data);

#else // direct CAEN VME access
    CVErrorCodes ret = CAENVME_WriteCycle(BHandle,DELAY,&data,am,dw);	
    if(ret != cvSuccess) {  // Error
      cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
      analyzeError(ret); 
    }		
#endif // USE_HAL
  }
} // end
//////////////////////////////////////////////////////////////////////////////////
//Load UB and B thresholds for each channel & ROC
void PixelPh0FEDInterface::set_blk_ublk_thold() {
  
  for(int chip_nr=0;chip_nr<4;chip_nr++) {
    for(int channel=1;channel<10;channel++) {

      /*UB_B_ThreshAddr is the chip +bit[20-17]=channel + bit[16-14] set to = 2	
	black and Ultra-balck are assumed to be the same for all ROC's
      */	

      if(Printlevel&2) 
	cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Load group, channel "<<chip_nr<<" "<<channel<<" "
	    <<(chip_nr*9+channel)<<"-"
	    <<pixelFEDCard.Ublack[chip_nr*9+channel-1]<<" "
	    <<pixelFEDCard.BlackLo[chip_nr*9+channel-1]<<" " 
	    <<pixelFEDCard.BlackHi[chip_nr*9+channel-1]<<endl;

      uint32_t data = pixelFEDCard.Ublack[chip_nr*9+channel-1]+ 
	((pixelFEDCard.BlackLo[chip_nr*9+channel-1])<<10) + 
	((pixelFEDCard.BlackHi[chip_nr*9+channel-1])<<20); // data UB BL BH
      //      UBLvl  BlackLowLvl   BlackHighLvl       

#ifdef USE_HAL // Use HAL
      uint32_t offset = (channel<<17)+(2<<14);
      vmeDevicePtr->write(FPGAName[chip_nr],data,HAL::HAL_NO_VERIFY,offset);
      
#else // direct CAEN VME access
      uint32_t UB_B_ThreshAddr=CHIP[chip_nr]+(channel<<17)+(2<<14);
      CVErrorCodes ret = CAENVME_WriteCycle(BHandle,UB_B_ThreshAddr,&data,am,dw);
      if(ret != cvSuccess) {  // Error
	cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
	analyzeError(ret); 
      }		    
#endif // USE_HAL
    }
  }
  
} // end
//////////////////////////////////////////////////////////////////////////////////
//Load B thresholds to 1000 and 999, UB to 300 for each channel
void PixelPh0FEDInterface::set_blk_ublk_trans_thold() {
assert(0);
	//cout<<"FEDID:"<<pixelFEDCard.fedNumber<<"Seeting Black for safe transparent mode"<<endl; 
  for(int chip_nr=0;chip_nr<4;chip_nr++) {
    for(int channel=1;channel<10;channel++) {

      /*UB_B_ThreshAddr is the chip +bit[20-17]=channel + bit[16-14] set to = 2	
	black and Ultra-balck are assumed to be the same for all ROC's
      */	

      if(Printlevel&2) 
	cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Load group, channel "<<chip_nr<<" "<<channel<<" "
	    <<(chip_nr*9+channel)<<"-"
	    <<pixelFEDCard.Ublack[chip_nr*9+channel-1]<<" "
	    <<"999"<<" " 
	    <<"1000"<<endl;

      uint32_t data = pixelFEDCard.Ublack[chip_nr*9+channel-1] + (999<<10) + (1000<<20); // data UB BL BH
      //      UBLvl  BlackLowLvl   BlackHighLvl       

      uint32_t offset = (channel<<17)+(2<<14);
      vmeDevicePtr->write(FPGAName[chip_nr],data,HAL::HAL_NO_VERIFY,offset);
    }
  }
  
} // end

//////////////////////////////////////////////////////////////////////////////////
//Load the correct number of ROCS for each channel
void PixelPh0FEDInterface::set_chnl_nrocs() {
  
  for(int chip_nr=0;chip_nr<4;chip_nr++) {
    for(int channel=1;channel<10;channel++) {

      /*
      The number of ROCS is the first 5 bits in the data word. #ROC>25 is
      reserved for error info!
      */	

      if(Printlevel&2) 
	cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Load group, channel - NRocs "<<chip_nr<<" "<<channel<<" "
	    <<(chip_nr*9+channel)<<" - "
	    <<pixelFEDCard.NRocs[chip_nr*9+channel-1]<<endl;

      uint32_t data = pixelFEDCard.NRocs[chip_nr*9+channel-1];
      if(data>24) {
      cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Number of ROCS requested "<<data<<" exceeds 24, setting to 24"<<endl; 
      data=24;
      }
#ifdef USE_HAL // Use HAL
      uint32_t offset = (channel<<17);
      vmeDevicePtr->write(FPGAName[chip_nr],data,HAL::HAL_NO_VERIFY,offset);
      
#else // direct CAEN VME access
      uint32_t NrocAddr=CHIP[chip_nr]+(channel<<17);
      CVErrorCodes ret = CAENVME_WriteCycle(BHandle,NrocAddr,&data,am,dw);
      if(ret != cvSuccess) {  // Error
	cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
	analyzeError(ret); 
      }		    
#endif // USE_HAL
    }
  }
  
} // end

/////////////////////////////////////////////////////////////////////////////
// Set the address levels for TBM and all ROCs
void PixelPh0FEDInterface::set_data_levels() {
assert(0);
  for(int chip_nr=0;chip_nr<4;chip_nr++) {  // loop over FPGs
    for(int channel=1;channel<10;channel++) {   // loop over channels

      //This is a little complicated. We actually have 2 "extra" ROC's in each
      //input channel. 
      //TBM's levels for the input header acts like 1st ROC 
      //Roc 0-(#of readout chips-1) has the levels for the actual pixel readout chips
      //TRL has the levels for the TBM trailer acts like last ROC. 
           
      //  TBM levels header levels 0,1,2
      uint32_t data = pixelFEDCard.TBM_L0[chip_nr*9+channel-1] +
	((pixelFEDCard.TBM_L1[chip_nr*9+channel-1])<<10)+
	((pixelFEDCard.TBM_L2[chip_nr*9+channel-1])<<20);
      //        Lvl 0     Lvl 1         Lvl 2     

      //if(PRINT || (channel==1 && chip_nr==0) ) 
      if(Printlevel&2) 
	cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Load TBM levels group,channel (seq#)"<<chip_nr<<" "<<channel<<" "
	    <<"0   "<<hex<<data<<dec<<" "
	    <<pixelFEDCard.TBM_L0[chip_nr*9+channel-1]<<" "
	    <<pixelFEDCard.TBM_L1[chip_nr*9+channel-1]<<" "
	    <<pixelFEDCard.TBM_L2[chip_nr*9+channel-1]<<" ";


#ifdef USE_HAL // Use HAL

      uint32_t offset = (channel<<17)+(4<<14)+(0<<2);
      vmeDevicePtr->write(FPGAName[chip_nr],data,HAL::HAL_NO_VERIFY,offset);
      //cout<<FPGAName[chip_nr]<<" "<<hex<<offset<<dec;

#else // direct CAEN VME access

      uint32_t L012_ThreshAddr=CHIP[chip_nr]+(channel<<17)+(4<<14);
      uint32_t vmeAddress = L012_ThreshAddr+((0)<<2);
      CVErrorCodes ret = CAENVME_WriteCycle(BHandle,vmeAddress,&data,am,dw);	
      if(ret != cvSuccess) {  // Error
	cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
	analyzeError(ret); 
      }		

#endif // USE_HAL
      
      // TBM header levels 3,4
      data =pixelFEDCard.TBM_L3[chip_nr*9+channel-1]+ 
	  ((pixelFEDCard.TBM_L4[chip_nr*9+channel-1])<<10);
      //        Lvl 3    Lvl 4        

      //if(PRINT || (channel==1 && chip_nr==0) ) 
      if(Printlevel&2) 
	cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" "<<pixelFEDCard.TBM_L3[chip_nr*9+channel-1]<<" "
	    <<pixelFEDCard.TBM_L4[chip_nr*9+channel-1]<<" "
	    <<hex<<data<<dec<<endl;

#ifdef USE_HAL // Use HAL

      offset = (channel<<17)+(5<<14)+(0<<2);
      vmeDevicePtr->write(FPGAName[chip_nr],data,HAL::HAL_NO_VERIFY,offset);

#else // direct CAEN VME access

      uint32_t L34_ThreshAddr=CHIP[chip_nr]+(channel<<17)+(5<<14);
      vmeAddress = L34_ThreshAddr+((0)<<2);
      ret = CAENVME_WriteCycle(BHandle,vmeAddress,&data,am,dw);	
      if(ret != cvSuccess) {  // Error
	cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
	analyzeError(ret); 
      }		

#endif // USE_HAL
      

      // Load ROC levels
      for(int roc=0;roc<pixelFEDCard.NRocs[chip_nr*9+channel-1];roc++) {  // loop over  ROCs
	
	// ROC levels 0,1,2
	data = pixelFEDCard.ROC_L0[chip_nr*9+channel-1][roc]+
          ((pixelFEDCard.ROC_L1[chip_nr*9+channel-1][roc])<<10)+
	  ((pixelFEDCard.ROC_L2[chip_nr*9+channel-1][roc])<<20);
	//        Lvl 0     Lvl 1         Lvl 2     

	//if(PRINT || (channel==1 && chip_nr==0) ) 
	if(Printlevel&2) 
	  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Load ROC levels group,channel (seq #)"<<chip_nr<<" "<<channel<<" "
	      <<(roc+1)<<" "<<chip_nr*9+channel-1<<" "<<hex<<data<<dec<<" "
	      <<pixelFEDCard.ROC_L0[chip_nr*9+channel-1][roc]<<" " 
	      <<pixelFEDCard.ROC_L1[chip_nr*9+channel-1][roc]<<" "
	      <<pixelFEDCard.ROC_L2[chip_nr*9+channel-1][roc]<<" ";
	

#ifdef USE_HAL // Use HAL

	offset = (channel<<17)+(4<<14)+((roc+1)<<2);
	vmeDevicePtr->write(FPGAName[chip_nr],data,HAL::HAL_NO_VERIFY,offset);
      
#else // direct CAEN VME access
	vmeAddress = L012_ThreshAddr+((roc+1)<<2);
	ret = CAENVME_WriteCycle(BHandle,vmeAddress,&data,am,dw);	
	if(ret != cvSuccess) {  // Error
	  cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
	  analyzeError(ret); }		
#endif // USE_HAL
	
	// ROC levels 3,4
	data =pixelFEDCard.ROC_L3[chip_nr*9+channel-1][roc]+ 
	  ((pixelFEDCard.ROC_L4[chip_nr*9+channel-1][roc])<<10);
	//        Lvl 3    Lvl 4        
	
	//if(PRINT || (channel==1 && chip_nr==0) ) 
	if(Printlevel&2) 
	  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" "<<pixelFEDCard.ROC_L3[chip_nr*9+channel-1][roc]<<" " 
	      <<pixelFEDCard.ROC_L4[chip_nr*9+channel-1][roc]<<" "
	      <<hex<<data<<dec<<endl;
	
#ifdef USE_HAL // Use HAL
	
	offset = (channel<<17)+(5<<14)+((roc+1)<<2);
	vmeDevicePtr->write(FPGAName[chip_nr],data,HAL::HAL_NO_VERIFY,offset);
	
#else // direct CAEN VME access
	
	vmeAddress = L34_ThreshAddr+((roc+1)<<2);
	ret = CAENVME_WriteCycle(BHandle,vmeAddress,&data,am,dw);	
	if(ret != cvSuccess) {  // Error
	  cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
	  analyzeError(ret); 
	}		
	
#endif // USE_HAL
	
      } // end loop roc
      
      //if(PRINT || (channel==1 && chip_nr==0) ) 
      if(Printlevel&2) 
	cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Load TRL levels group,channel (seq #)"<<chip_nr<<" "<<channel<<" "
	    <<(pixelFEDCard.NRocs[chip_nr*9+channel-1]+1)<<" "
	    <<pixelFEDCard.TRL_L0[chip_nr*9+channel-1]<<" "
	    <<pixelFEDCard.TRL_L1[chip_nr*9+channel-1]<<" "
	    <<pixelFEDCard.TRL_L2[chip_nr*9+channel-1]<<" ";
	
      data = pixelFEDCard.TRL_L0[chip_nr*9+channel-1]+
	((pixelFEDCard.TRL_L1[chip_nr*9+channel-1])<<10)+
	((pixelFEDCard.TRL_L2[chip_nr*9+channel-1])<<20);
      //        Lvl 0     Lvl 1         Lvl 2     
      
      
#ifdef USE_HAL // Use HAL
      
      offset = (channel<<17)+(4<<14)+((pixelFEDCard.NRocs[chip_nr*9+channel-1]+1)<<2);
      vmeDevicePtr->write(FPGAName[chip_nr],data,HAL::HAL_NO_VERIFY,offset);
      
#else // direct CAEN VME access
      
      vmeAddress = L012_ThreshAddr+((pixelFEDCard.NRocs[chip_nr*9+channel-1]+1)<<2);
      ret = CAENVME_WriteCycle(BHandle,vmeAddress,&data,am,dw);	
      if(ret != cvSuccess) {  // Error
	cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
	analyzeError(ret); 
      }		
      
#endif // USE_HAL
      
      data =pixelFEDCard.TRL_L3[chip_nr*9+channel-1]+ 
	((pixelFEDCard.TRL_L4[chip_nr*9+channel-1])<<10);
      //        Lvl 3    Lvl 4        
      
      //if(PRINT || (channel==1 && chip_nr==0) ) 
      if(Printlevel&2) 
	cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" "<<pixelFEDCard.TRL_L3[chip_nr*9+channel-1]<<" "
	    <<pixelFEDCard.TRL_L4[chip_nr*9+channel-1]<<endl;
      
#ifdef USE_HAL // Use HAL
      
      offset = (channel<<17)+(5<<14)+((pixelFEDCard.NRocs[chip_nr*9+channel-1]+1)<<2);
      vmeDevicePtr->write(FPGAName[chip_nr],data,HAL::HAL_NO_VERIFY,offset);
      
#else // direct CAEN VME access
      
      vmeAddress = L34_ThreshAddr+((pixelFEDCard.NRocs[chip_nr*9+channel-1])<<2);
      ret = CAENVME_WriteCycle(BHandle,vmeAddress,&data,am,dw);	
      if(ret != cvSuccess) {  // Error
	cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
	analyzeError(ret); 
      }		
      
#endif // USE_HAL
          
    } // end channel loop
  } // end group loop

  //int dummy;
  //cout<<" enter :";
  //cin>>dummy;
  
} // end

void PixelPh0FEDInterface::toggle_chnls_offon() {
  const uint32_t save_Ncntrl  = pixelFEDCard.Ncntrl;
  const uint32_t save_NCcntrl = pixelFEDCard.NCcntrl;
  const uint32_t save_SCcntrl = pixelFEDCard.SCcntrl;
  const uint32_t save_Scntrl  = pixelFEDCard.Scntrl;

  pixelFEDCard.Ncntrl  = pixelFEDCard.Ncntrl  | 0x1ff;
  pixelFEDCard.NCcntrl = pixelFEDCard.NCcntrl | 0x1ff;
  pixelFEDCard.SCcntrl = pixelFEDCard.SCcntrl | 0x1ff;
  pixelFEDCard.Scntrl  = pixelFEDCard.Scntrl  | 0x1ff;
  set_chnls_onoff();
  usleep(5000);

  pixelFEDCard.Ncntrl  = save_Ncntrl;
  pixelFEDCard.NCcntrl = save_NCcntrl;
  pixelFEDCard.SCcntrl = save_SCcntrl;
  pixelFEDCard.Scntrl  = save_Scntrl;
  set_chnls_onoff();
  usleep(5000);
}

/////////////////////////////////////////////////////////////////////////
// This bits control if the data is trasfered from FIFO1 to FIFO2
// 0 - means data is transfered, 1 - data is not transfered.
void PixelPh0FEDInterface::set_chnls_onoff(int mode) {

  if(mode==1) {  // keep data in fifo1

    // Make sure we leave bits above 9 alone
    pixelFEDCard.Ncntrl  = 0x1ff | pixelFEDCard.Ncntrl;
    pixelFEDCard.NCcntrl = 0x1ff | pixelFEDCard.NCcntrl;
    pixelFEDCard.SCcntrl = 0x1ff | pixelFEDCard.SCcntrl;
    pixelFEDCard.Scntrl  = 0x1ff | pixelFEDCard.Scntrl;

  } else if(mode==0) {

    // Make sure we leave bits above 9 alone
    pixelFEDCard.Ncntrl  = 0x000 | (pixelFEDCard.Ncntrl & 0xffff0000LL);
    pixelFEDCard.NCcntrl = 0x000 | (pixelFEDCard.NCcntrl & 0xffff0000LL);
    pixelFEDCard.SCcntrl = 0x000 | (pixelFEDCard.SCcntrl & 0xffff0000LL);
    pixelFEDCard.Scntrl  = 0x000 | (pixelFEDCard.Scntrl & 0xffff0000LL);

  } else {

    cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" wrong mode for onoff"<<endl;
    return;

  }

  set_chnls_onoff(); // call the method below

 }
/////////////////////////////////////////////////////////////////////////
// This bits control if the data is trasfered from FIFO1 to FIFO2
// 0 - means data is transfered, 1 - data is not transfered.
void PixelPh0FEDInterface::set_chnls_onoff() {
  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" FIFO2/3 enable "<<hex<<pixelFEDCard.Ncntrl<<" "<<pixelFEDCard.NCcntrl
      <<" "<<pixelFEDCard.SCcntrl<<" "<<pixelFEDCard.Scntrl<<dec<<endl;

  // bits 0-8 on = channels 1-9 off
  uint32_t data = pixelFEDCard.Ncntrl; 
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("NWrRdCntrReg",data);

#else // direct CAEN VME access
  uint32_t vmeAddress = LAD_N+0x1a0000;
  CVErrorCodes ret = CAENVME_WriteCycle(BHandle,vmeAddress,&data,am,dw);	
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret); 
  }		    
#endif // USE_HAL

  data = pixelFEDCard.NCcntrl; // bits 0-8 on = channels 10-18 on

#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("NCWrRdCntrReg",data);

#else // direct CAEN VME access
  vmeAddress = LAD_NC+0x1a0000;
  ret = CAENVME_WriteCycle(BHandle,vmeAddress,&data,am,dw);	
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret); 
  }		    
#endif // USE_HAL

  data = pixelFEDCard.SCcntrl; // bits 0-8 on = channels 19-27 on

#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("SCWrRdCntrReg",data);

#else // direct CAEN VME access
  vmeAddress = LAD_SC+0x1a0000;
  ret = CAENVME_WriteCycle(BHandle,vmeAddress,&data,am,dw);	
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret); 
  }		    
#endif // USE_HAL

  data = pixelFEDCard.Scntrl; // bits 0-8 on = channels 28-36 on

#ifdef USE_HAL // Use HAL
  vmeDevicePtr->write("SWrRdCntrReg",data);

#else // direct CAEN VME access
  vmeAddress = LAD_S+0x1a0000;
  ret = CAENVME_WriteCycle(BHandle,vmeAddress,&data,am,dw);	
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret); 
  }		    
#endif // USE_HAL
} // end
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// The 1st 8 bits control if the 8 bits in the TBM trailer set an error
// bit n on means bit n is masked in the TBM
// command is common to the fed and updates the fedcard
void PixelPh0FEDInterface::set_TBMmask(uint32_t mask) {

    pixelFEDCard.N_TBMmask  = (mask&0xff);
    pixelFEDCard.NC_TBMmask = (mask&0xff);
    pixelFEDCard.SC_TBMmask = (mask&0xff);
    pixelFEDCard.S_TBMmask  = (mask&0xff);

  set_MODE_front(); // call the method below

 }
/////////////////////////////////////////////////////////////////////////
// The 1st 8 bits control the word written to the 1st 8 bits of the 
// gap and filler words
// command is common to the fed and updates the fedcard
void PixelPh0FEDInterface::set_PrivateWord(uint32_t pword) {

    pixelFEDCard.N_Pword  = (pword&0xff);
    pixelFEDCard.NC_Pword = (pword&0xff);
    pixelFEDCard.SC_Pword = (pword&0xff);
    pixelFEDCard.S_Pword  = (pword&0xff);

  set_MODE_front(); // call the method below

 }

// The 1st 4 bits set the channel used in the spy scope (Piggy card)
 void PixelPh0FEDInterface::set_ScopeChannel(int which, uint32_t ch) {
   if      (which == 0) pixelFEDCard.N_ScopeCh  = ch & 0xF;
   else if (which == 1) pixelFEDCard.NC_ScopeCh = ch & 0xF;
   else if (which == 2) pixelFEDCard.SC_ScopeCh = ch & 0xF;
   else if (which == 3) pixelFEDCard.S_ScopeCh  = ch & 0xF;
   else
     assert(0);
   set_MODE_front();
 }

// The 1st 4 bits set the channel used in the spy scope (Piggy card)
 void PixelPh0FEDInterface::set_ScopeChannels(uint32_t N_ch, uint32_t NC_ch, uint32_t SC_ch, uint32_t S_ch) {
   pixelFEDCard.N_ScopeCh  =  N_ch & 0xF;
   pixelFEDCard.NC_ScopeCh = NC_ch & 0xF;
   pixelFEDCard.SC_ScopeCh = SC_ch & 0xF;
   pixelFEDCard.S_ScopeCh  =  S_ch & 0xF;
   set_MODE_front();
 }

/////////////////////////////////////////////////////////////////////////
// The 1st 8 bits control the word written to the 1st 8 bits of the 
// gap and filler words
// command is common to the fed and updates the fedcard
void PixelPh0FEDInterface::set_SpecialDac(uint32_t mode) {

    pixelFEDCard.SpecialDac = (mode&0x1);

  set_MODE_front(); // call the method below

 }
/////////////////////////////////////////////////////////////////////////
// This updates the FED with the latest value of the mode registers
// for the front FPGA's
//Bits 32-24 used to mask TBM trialer bits
//Bits 23-16 the private word that gets put in gap and fill words
//Bits 11-8 the channel for spy scope
//1st bit in N FPGA is used for a special DAC mode for random trigs  

void PixelPh0FEDInterface::set_MODE_front() {
  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Setting Mode REG in Front FPGA's "<<endl;

  uint32_t data = 
    (pixelFEDCard.SpecialDac&0x1)|((pixelFEDCard.N_ScopeCh&0xF)<<8)|((pixelFEDCard.N_Pword&0xff)<<16)|((pixelFEDCard.N_TBMmask&0xff)<<24);

  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" North FPGA Mode REG 0x"<<hex<<data<<dec<<endl;
  vmeDevicePtr->write("NWrModeReg",data);


  data = ((pixelFEDCard.NC_ScopeCh&0xF)<<8)|((pixelFEDCard.NC_Pword&0xff)<<16)|((pixelFEDCard.NC_TBMmask&0xff)<<24);

  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" North Center FPGA Mode REG 0x"<<hex<<data<<dec<<endl;
  vmeDevicePtr->write("NCWrModeReg",data);


  data = ((pixelFEDCard.SC_ScopeCh&0xF)<<8)|((pixelFEDCard.SC_Pword&0xff)<<16)|((pixelFEDCard.SC_TBMmask&0xff)<<24);

  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" South Center FPGA Mode REG 0x"<<hex<<data<<dec<<endl;
  vmeDevicePtr->write("SCWrModeReg",data);


  data = ((pixelFEDCard.S_ScopeCh&0xF)<<8)|((pixelFEDCard.S_Pword&0xff)<<16)|((pixelFEDCard.S_TBMmask&0xff)<<24);

  if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" South FPGA Mode REG 0x"<<hex<<data<<dec<<endl;
  vmeDevicePtr->write("SWrModeReg",data);

} // end
/////////////////////////////////////////////////////////////////////////
// This prints the latest value of the mode registers for the front FPGA's
//Bits 32-24 used to mask TBM trialer bits
//Bits 23-16 the private word that gets put in gap and fill words
//1st bit in N FPGA is used for a special DAC mode for random trigs  

void PixelPh0FEDInterface::get_MODE_front() {
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Printing Mode REG in Front FPGA's "<<endl;

  uint32_t data = 
    (pixelFEDCard.SpecialDac&0x1)|((pixelFEDCard.N_ScopeCh&0xF)<<8)|((pixelFEDCard.N_Pword&0xff)<<16)|((pixelFEDCard.N_TBMmask&0xff)<<24);

  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" North FPGA Mode REG 0x"<<hex<<data<<dec<<endl;
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" North Special Dac   0x"<<hex<<(pixelFEDCard.SpecialDac&0x1)<<dec<<endl;
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" North Scope Ch 0x"<<hex<<(pixelFEDCard.N_ScopeCh&0xf)<<dec<<endl;
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" North Private word  0x"<<hex<<(pixelFEDCard.N_Pword&0xff)<<dec<<endl;
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" North TBM trlr mask 0x"<<hex<<(pixelFEDCard.N_TBMmask&0xff)<<dec<<endl;

  data = ((pixelFEDCard.NC_ScopeCh&0xF)<<8)|((pixelFEDCard.NC_Pword&0xff)<<16)|((pixelFEDCard.NC_TBMmask&0xff)<<24);

  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" North Center FPGA Mode REG 0x"<<hex<<data<<dec<<endl;
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" North Center Scope Ch 0x"<<hex<<(pixelFEDCard.NC_ScopeCh&0xf)<<dec<<endl;
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" North Center Private word  0x"<<hex<<(pixelFEDCard.NC_Pword&0xff)<<dec<<endl;
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" North Center TBM trlr mask 0x"<<hex<<(pixelFEDCard.NC_TBMmask&0xff)<<dec<<endl;

  data = ((pixelFEDCard.SC_ScopeCh&0xF)<<8)|((pixelFEDCard.SC_Pword&0xff)<<16)|((pixelFEDCard.SC_TBMmask&0xff)<<24);

  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" South Center FPGA Mode REG 0x"<<hex<<data<<dec<<endl;
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" South Center Scope Ch 0x"<<hex<<(pixelFEDCard.SC_ScopeCh&0xf)<<dec<<endl;
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" South Center Private word  0x"<<hex<<(pixelFEDCard.SC_Pword&0xff)<<dec<<endl;
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" South Center TBM trlr mask 0x"<<hex<<(pixelFEDCard.SC_TBMmask&0xff)<<dec<<endl;

  data = ((pixelFEDCard.S_ScopeCh&0xF)<<8)|((pixelFEDCard.S_Pword&0xff)<<16)|((pixelFEDCard.S_TBMmask&0xff)<<24);

  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" South FPGA Mode REG 0x"<<hex<<data<<dec<<endl;
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" South Scope Ch 0x"<<hex<<(pixelFEDCard.S_ScopeCh&0xf)<<dec<<endl;
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" South Private word  0x"<<hex<<(pixelFEDCard.S_Pword&0xff)<<dec<<endl;
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" South TBM trlr mask 0x"<<hex<<(pixelFEDCard.S_TBMmask&0xff)<<dec<<endl;

} // end
/////////////////////////////////////////////////////////////////////////
// This bits control the range of the ADC for a single channel
// 0 - means adc is set 1Vpp, 1 - adc set 2Vpp
// IMPORTANT!!! each adc has 2 channels
int PixelPh0FEDInterface::get_adc_1v2v(int chnl) {
  assert(0);
  // bracket logic:
  // (1<<((((chnl%2)+chnl)/2)-1))
  // each bit controls 2 adc channels. E.g.
  // 1,2 bit 0
  // 3,4 bit 1
  // 5,6 bit 2

	if((chnl<1) | (chnl>36)){
		cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Channel out of range "<<endl;
		assert(0);}
	
	if(chnl<13){
	
		if ( (int)(pixelFEDCard.Nadcg&(1<<((((chnl%2)+chnl)/2)-1))) != 0 ) return 1;
		else                                                           return 0;
	
	} else if((chnl>12) & (chnl <21)) {
	
		if ( (int)(pixelFEDCard.NCadcg&(1<<((((chnl-12)%2+(chnl-12))/2) -1))) != 0 ) return 1;
		else                                                                       return 0;

	} else if((chnl>20) & (chnl <29)) {
	
		if ( (int)(pixelFEDCard.SCadcg&(1<<((((chnl-20)%2+(chnl-20))/2)-1))) != 0 ) return 1;
		else                                                                      return 0;
	
	} else {
	
		if ( (int)(pixelFEDCard.Sadcg&(1<<((((chnl-28)%2+(chnl-28))/2)-1))) != 0 ) return 1;
		else                                                                     return 0;

	}
	
	assert(0); // should never get here

}
/////////////////////////////////////////////////////////////////////////
// This bits control the range of the ADC for a single channel
// 0 - means adc is set 1Vpp, 1 - adc set 2Vpp
// IMPORTANT!!! each adc has 2 channels
void PixelPh0FEDInterface::set_adc_1v2v(int mode,int chnl) {
assert(0);
if((chnl<1) | (chnl>36)){
    cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Channel out of range "<<endl;
    return;}

if(chnl<13){

if(mode==1)pixelFEDCard.Nadcg = pixelFEDCard.Nadcg|(1<<(((chnl%2+chnl)/2)-1));
if(mode==0)pixelFEDCard.Nadcg = pixelFEDCard.Nadcg&(0x3f ^ (1<<(((chnl%2+chnl)/2)-1)));

} else if((chnl>12) & (chnl <21)) {

if(mode==1)pixelFEDCard.NCadcg =pixelFEDCard.NCadcg|(1<<((((chnl-12)%2+(chnl-12))/2) -1));
if(mode==0)pixelFEDCard.NCadcg =pixelFEDCard.NCadcg&(0xf ^ (1<<((((chnl-12)%2+(chnl-12))/2)-1)));

} else if((chnl>20) & (chnl <29)) {

if(mode==1)pixelFEDCard.SCadcg =pixelFEDCard.SCadcg|(1<<((((chnl-20)%2+(chnl-20))/2)-1));
if(mode==0)pixelFEDCard.SCadcg =pixelFEDCard.SCadcg&(0xf ^ (1<<((((chnl-20)%2+(chnl-20))/2)-1)));

} else {

if(mode==1)pixelFEDCard.Sadcg  =pixelFEDCard.Sadcg|(1<<((((chnl-28)%2+(chnl-28))/2)-1));
if(mode==0)pixelFEDCard.Sadcg  =pixelFEDCard.Sadcg&(0xf ^ (1<<((((chnl-28)%2+(chnl-28))/2)-1)));

}


   set_adc_1v2v(); // call the method below

 }
/////////////////////////////////////////////////////////////////////////
// This bits control the range of the ADC
// 0 - means adc is set 1Vpp, 1 - adc set 2Vpp
void PixelPh0FEDInterface::set_adc_1v2v(int mode) {
assert(0);
  if(mode==1) {  // keep data in fifo1

    pixelFEDCard.Nadcg  = 0x3f;
    pixelFEDCard.NCadcg = 0xf;
    pixelFEDCard.SCadcg = 0xf;
    pixelFEDCard.Sadcg  = 0xf;

  } else if(mode==0) {

    pixelFEDCard.Nadcg  = 0x000;
    pixelFEDCard.NCadcg = 0x000;
    pixelFEDCard.SCadcg = 0x000;
    pixelFEDCard.Sadcg  = 0x000;

  } else {

    cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" wrong mode 1v2v"<<endl;
    return;

  }

  set_adc_1v2v(); // call the method below

 }
/////////////////////////////////////////////////////////////////////////
// This bits control if the data is trasfered from FIFO1 to FIFO2
// 0 - means data is transfered, 1 - data is not transfered.
void PixelPh0FEDInterface::set_adc_1v2v() {
assert(0);
  //cout<<" ADC GAIN Set "<<hex<<pixelFEDCard.Nadcg<<" "<<pixelFEDCard.NCadcg
  //    <<" "<<pixelFEDCard.SCadcg<<" "<<pixelFEDCard.Sadcg<<dec<<endl;

  // bits 0-5 on(off) = channels 1-12 set 2Vpp(1Vpp)
  uint32_t data = pixelFEDCard.Nadcg;
  vmeDevicePtr->write("NWrGainReg",data);

  data = pixelFEDCard.NCadcg; // bits 0-3 on(off) = channels 13-20 set 2Vpp(1Vpp)
  vmeDevicePtr->write("NCWrGainReg",data);

  data = pixelFEDCard.SCadcg; // bits 0-8 on(off) = channels 21-28 set 2Vpp(1Vpp)
  vmeDevicePtr->write("SCWrGainReg",data);

  data = pixelFEDCard.Sadcg; // bits 0-8 on(off) = channels 29-36 set 2Vpp(1Vpp)
  vmeDevicePtr->write("SWrGainReg",data);

} // end

/////////////////////////////////////////////////////////////////////////
//This Method turns on the Baseline Adjustment for a whole fed
  void PixelPh0FEDInterface::BaselineCorr_on(){
assert(0);
//cout<<"PixelPh0FEDInterface::BaselineCorr_on() ENTERED!!!"<<endl;

uint32_t data=(0x1ff<<16)+(pixelFEDCard.Nbaseln&0x3ff);
set_BaselineCorr(1,data);

data=(0x1ff<<16)+(pixelFEDCard.NCbaseln&0x3ff);
set_BaselineCorr(2,data);

data=(0x1ff<<16)+(pixelFEDCard.SCbaseln&0x3ff);
set_BaselineCorr(3,data);

data=(0x1ff<<16)+(pixelFEDCard.Sbaseln&0x3ff);
set_BaselineCorr(4,data);

  }//end

/////////////////////////////////////////////////////////////////////////
//Turn off Baseline correction (whole fed)
  void PixelPh0FEDInterface::BaselineCorr_off(){
assert(0);
if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" PixelPh0FEDInterface::BaselineCorr_off() ENTERED!!!"<<endl;

uint32_t data=(pixelFEDCard.Nbaseln&0x3ff);
set_BaselineCorr(1,data); 

data=(pixelFEDCard.NCbaseln&0x3ff);
set_BaselineCorr(2,data); 

data=(pixelFEDCard.SCbaseln&0x3ff);
set_BaselineCorr(3,data); 

data=(pixelFEDCard.Sbaseln&0x3ff);
set_BaselineCorr(4,data); 

data=0x80000000;
 vmeDevicePtr->write("LRES",data);

//cout<<"A local reset has been issued to shut off baseline correction"<<endl;

  }//end
/////////////////////////////////////////////////////////////////////////
//get Baseline correction (single channel, but bits are common!)
uint32_t PixelPh0FEDInterface::get_BaselineCorrVal(int chnl){
assert(0);
uint32_t data=0xffffffff;

if((chnl>0)&(chnl<10)){

data = pixelFEDCard.Nbaseln;

} else if((chnl>9)&(chnl<19)){

data = pixelFEDCard.NCbaseln;

} else if((chnl>18)&(chnl<28)){

data = pixelFEDCard.SCbaseln;

} else if((chnl>27)&(chnl<37)){

data = pixelFEDCard.Sbaseln;

} else {
cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Channel out of (1-36)range!! Passed channel="<<chnl<<endl;

}

return data;

  }//end
/////////////////////////////////////////////////////////////////////////
//Turn on Baseline correction (single channel)
  void PixelPh0FEDInterface::BaselineCorr_on(int chnl){
assert(0);
//cout<<"PixelPh0FEDInterface::BaselineCorr_on(int "<<chnl<<") ENTERED!!!"<<endl;

if((chnl>0)&(chnl<10)){

uint32_t data=(0x1<<(15+chnl))|pixelFEDCard.Nbaseln;
set_BaselineCorr(1,data); 

} else if((chnl>9)&(chnl<19)){

uint32_t data=(0x1<<(15+chnl-9))|pixelFEDCard.NCbaseln;
set_BaselineCorr(2,data); 

} else if((chnl>18)&(chnl<28)){

uint32_t data=(0x1<<(15+chnl-18))|pixelFEDCard.SCbaseln;
set_BaselineCorr(3,data); 

} else if((chnl>27)&(chnl<37)){

uint32_t data=(0x1<<(15+chnl-27))|pixelFEDCard.Sbaseln;
set_BaselineCorr(4,data); 

} else {
cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Channel out of (1-36)range!! Passed channel="<<chnl<<endl;

}

  }//end
/////////////////////////////////////////////////////////////////////////
//Turn off Baseline correction (single channel)
  void PixelPh0FEDInterface::BaselineCorr_off(int chnl){
assert(0);
//cout<<"PixelPh0FEDInterface::BaselineCorr_off(int "<<chnl<<") ENTERED!!!"<<endl;

 if((chnl>0)&(chnl<10)){

uint32_t data=(~(0x1<<(15+chnl)))&pixelFEDCard.Nbaseln;
set_BaselineCorr(1,data); 

} else if((chnl>9)&(chnl<19)){

uint32_t data=(~(0x1<<(15+chnl-9)))&pixelFEDCard.NCbaseln;
set_BaselineCorr(2,data); 

} else if((chnl>18)&(chnl<28)){

uint32_t data=(~(0x1<<(15+chnl-18)))&pixelFEDCard.SCbaseln;
set_BaselineCorr(3,data); 

} else if((chnl>27)&(chnl<37)){

uint32_t data=(~(0x1<<(15+chnl-27)))&pixelFEDCard.Sbaseln;
set_BaselineCorr(4,data); 

} else {
cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Channel out of (1-36)range!!"<<endl;
}

uint32_t data=0x80000000;
 vmeDevicePtr->write("LRES",data);

//cout<<"A local reset has been issued to shut off baseline correction"<<endl;

  }//end
/////////////////////////////////////////////////////////////////////////
//Set the baseline correction value for an FPGA (1-4)  10 bits max!
  void PixelPh0FEDInterface::set_BaselineCorr(int chip,uint32_t value){
assert(0);
//    cout<<"PixelPh0FEDInterface::set_BaselineCorr(int "<<chip<<", uint32_t 0x"<<hex <<value<< dec << ") ENTERED!!!"<<endl;


if(chip==1){
pixelFEDCard.Nbaseln=value;
 vmeDevicePtr->write("NWrBaseLAdj",value);
} else if(chip==2) {
pixelFEDCard.NCbaseln=value;
 vmeDevicePtr->write("NCWrBaseLAdj",value);
} else if(chip==3) {
pixelFEDCard.SCbaseln=value;
 vmeDevicePtr->write("SCWrBaseLAdj",value);
} else if(chip==4) {
pixelFEDCard.Sbaseln=value;
 vmeDevicePtr->write("SWrBaseLAdj",value);
} else {
cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Chip must be 1(NORTH),  2(NORTH CENTER),  3(SOUTH CENTER), or 4(SOUTH)"<<endl;
}

  }//end
/////////////////////////////////////////////////////////////////////////
//Set the baseline corection values from the database
  void PixelPh0FEDInterface::set_BaselineCorr(){
assert(0);
//cout<<"PixelPh0FEDInterface::set_BaselineCorr() ENTERED!!"<<endl;

uint32_t value = pixelFEDCard.Nbaseln;
 vmeDevicePtr->write("NWrBaseLAdj",value);

value = pixelFEDCard.NCbaseln;
 vmeDevicePtr->write("NCWrBaseLAdj",value);

value = pixelFEDCard.SCbaseln;
 vmeDevicePtr->write("SCWrBaseLAdj",value);

value = pixelFEDCard.Sbaseln;
 //cout<<"PixelPh0FEDInterface::set_BaselineCorr().. SW =0x"<<hex<<value<<dec<<endl;
 vmeDevicePtr->write("SWrBaseLAdj",value);

  }//end
/////////////////////////////////////////////////////////////////////////////
//This method dumps the current value of the baseline adjustment
void PixelPh0FEDInterface::dump_BaselineCorr() {
assert(0);
//cout<<"PixelPh0FEDInterface::dump_BaselineCorr() ENTERED!!"<<endl;

uint32_t iwrdat;
int blstat[36];
vmeDevicePtr->read("NRdBaseL321",&iwrdat);
blstat[0]=(iwrdat&0x3ff);
blstat[1]=(iwrdat&0xffc00)>>10;
blstat[2]=(iwrdat&0x3ff00000)>>20;
vmeDevicePtr->read("NRdBaseL654",&iwrdat);
blstat[3]=(iwrdat&0x3ff);
blstat[4]=(iwrdat&0xffc00)>>10;
blstat[5]=(iwrdat&0x3ff00000)>>20;
vmeDevicePtr->read("NRdBaseL987",&iwrdat);
blstat[6]=(iwrdat&0x3ff);
blstat[7]=(iwrdat&0xffc00)>>10;
blstat[8]=(iwrdat&0x3ff00000)>>20;

vmeDevicePtr->read("NCRdBaseL321",&iwrdat);
blstat[9]=(iwrdat&0x3ff);
blstat[10]=(iwrdat&0xffc00)>>10;
blstat[11]=(iwrdat&0x3ff00000)>>20;
vmeDevicePtr->read("NCRdBaseL654",&iwrdat);
blstat[12]=(iwrdat&0x3ff);
blstat[13]=(iwrdat&0xffc00)>>10;
blstat[14]=(iwrdat&0x3ff00000)>>20;
vmeDevicePtr->read("NCRdBaseL987",&iwrdat);
blstat[15]=(iwrdat&0x3ff);
blstat[16]=(iwrdat&0xffc00)>>10;
blstat[17]=(iwrdat&0x3ff00000)>>20;

vmeDevicePtr->read("SCRdBaseL321",&iwrdat);
blstat[18]=(iwrdat&0x3ff);
blstat[19]=(iwrdat&0xffc00)>>10;
blstat[20]=(iwrdat&0x3ff00000)>>20;
vmeDevicePtr->read("SCRdBaseL654",&iwrdat);
blstat[21]=(iwrdat&0x3ff);
blstat[22]=(iwrdat&0xffc00)>>10;
blstat[23]=(iwrdat&0x3ff00000)>>20;
vmeDevicePtr->read("SCRdBaseL987",&iwrdat);
blstat[24]=(iwrdat&0x3ff);
blstat[25]=(iwrdat&0xffc00)>>10;
blstat[26]=(iwrdat&0x3ff00000)>>20;

vmeDevicePtr->read("SRdBaseL321",&iwrdat);
blstat[27]=(iwrdat&0x3ff);
blstat[28]=(iwrdat&0xffc00)>>10;
blstat[29]=(iwrdat&0x3ff00000)>>20;
vmeDevicePtr->read("SRdBaseL654",&iwrdat);
blstat[30]=(iwrdat&0x3ff);
blstat[31]=(iwrdat&0xffc00)>>10;
blstat[32]=(iwrdat&0x3ff00000)>>20;
vmeDevicePtr->read("SRdBaseL987",&iwrdat);
blstat[33]=(iwrdat&0x3ff);
blstat[34]=(iwrdat&0xffc00)>>10;
blstat[35]=(iwrdat&0x3ff00000)>>20;

for(int i=0;i<36;i++){
if(i==0){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Baseline Value for Channels 1-9 is:"<<pixelFEDCard.Nbaseln<<endl;}
if(i==9){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Baseline Value for Channels 10-18 is:"<<pixelFEDCard.NCbaseln<<endl;}
if(i==18){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Baseline Value for Channels 19-27 is:"<<pixelFEDCard.SCbaseln<<endl;}
if(i==27){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Baseline Value for Channels 28-36 is:"<<pixelFEDCard.Sbaseln<<endl;}
if(blstat[i]&0x200){
cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Baseline Adjust for Channel "<<dec<<i+1<<" is -"<<((~blstat[i])&0x1ff)+1<<endl;
} else {
cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Baseline Adjust for Channel "<<dec<<i+1<<" is "<<blstat[i]<<endl;
}
}

}//end
/////////////////////////////////////////////////////////////////////////////
//This method gets all the current values of the baseline adjustment
//10 bit words are in 2's compliment for negative, see dump_BaselineCorr
void PixelPh0FEDInterface::get_BaselineCorr(int * blstat) {
assert(0);
uint32_t iwrdat;
vmeDevicePtr->read("NRdBaseL321",&iwrdat);
blstat[0]=(iwrdat&0x3ff);
blstat[1]=(iwrdat&0xffc00)>>10;
blstat[2]=(iwrdat&0x3ff00000)>>20;
vmeDevicePtr->read("NRdBaseL654",&iwrdat);
blstat[3]=(iwrdat&0x3ff);
blstat[4]=(iwrdat&0xffc00)>>10;
blstat[5]=(iwrdat&0x3ff00000)>>20;
vmeDevicePtr->read("NRdBaseL987",&iwrdat);
blstat[6]=(iwrdat&0x3ff);
blstat[7]=(iwrdat&0xffc00)>>10;
blstat[8]=(iwrdat&0x3ff00000)>>20;

vmeDevicePtr->read("NCRdBaseL321",&iwrdat);
blstat[9]=(iwrdat&0x3ff);
blstat[10]=(iwrdat&0xffc00)>>10;
blstat[11]=(iwrdat&0x3ff00000)>>20;
vmeDevicePtr->read("NCRdBaseL654",&iwrdat);
blstat[12]=(iwrdat&0x3ff);
blstat[13]=(iwrdat&0xffc00)>>10;
blstat[14]=(iwrdat&0x3ff00000)>>20;
vmeDevicePtr->read("NCRdBaseL987",&iwrdat);
blstat[15]=(iwrdat&0x3ff);
blstat[16]=(iwrdat&0xffc00)>>10;
blstat[17]=(iwrdat&0x3ff00000)>>20;

vmeDevicePtr->read("SCRdBaseL321",&iwrdat);
blstat[18]=(iwrdat&0x3ff);
blstat[19]=(iwrdat&0xffc00)>>10;
blstat[20]=(iwrdat&0x3ff00000)>>20;
vmeDevicePtr->read("SCRdBaseL654",&iwrdat);
blstat[21]=(iwrdat&0x3ff);
blstat[22]=(iwrdat&0xffc00)>>10;
blstat[23]=(iwrdat&0x3ff00000)>>20;
vmeDevicePtr->read("SCRdBaseL987",&iwrdat);
blstat[24]=(iwrdat&0x3ff);
blstat[25]=(iwrdat&0xffc00)>>10;
blstat[26]=(iwrdat&0x3ff00000)>>20;

vmeDevicePtr->read("SRdBaseL321",&iwrdat);
blstat[27]=(iwrdat&0x3ff);
blstat[28]=(iwrdat&0xffc00)>>10;
blstat[29]=(iwrdat&0x3ff00000)>>20;
vmeDevicePtr->read("SRdBaseL654",&iwrdat);
blstat[30]=(iwrdat&0x3ff);
blstat[31]=(iwrdat&0xffc00)>>10;
blstat[32]=(iwrdat&0x3ff00000)>>20;
vmeDevicePtr->read("SRdBaseL987",&iwrdat);
blstat[33]=(iwrdat&0x3ff);
blstat[34]=(iwrdat&0xffc00)>>10;
blstat[35]=(iwrdat&0x3ff00000)>>20;

for(int ij=0;ij<36;ij++){
if(blstat[ij]&0x200)blstat[ij]=-1*(((~blstat[ij])&0x1ff)+1);
}

  }//end
/////////////////////////////////////////////////////////////////////////////
//This method gets the current value of the baseline adjustment for a single channel
//10 bit words are in 2's compliment for negative, see dump_BaselineCorr
int PixelPh0FEDInterface::get_BaselineCorr(int chnl) {
assert(0);
uint32_t iwrdat;
int blstat=0;
if((chnl==1)|(chnl==2)|(chnl==3)){
vmeDevicePtr->read("NRdBaseL321",&iwrdat);
if(chnl==1)blstat=(iwrdat&0x3ff);
if(chnl==2)blstat=(iwrdat&0xffc00)>>10;
if(chnl==3)blstat=(iwrdat&0x3ff00000)>>20;
} else if ((chnl==4)|(chnl==5)|(chnl==6)){
vmeDevicePtr->read("NRdBaseL654",&iwrdat);
if(chnl==4)blstat=(iwrdat&0x3ff);
if(chnl==5)blstat=(iwrdat&0xffc00)>>10;
if(chnl==6)blstat=(iwrdat&0x3ff00000)>>20;
} else if ((chnl==7)|(chnl==8)|(chnl==9)){
vmeDevicePtr->read("NRdBaseL987",&iwrdat);
if(chnl==7)blstat=(iwrdat&0x3ff);
if(chnl==8)blstat=(iwrdat&0xffc00)>>10;
if(chnl==9)blstat=(iwrdat&0x3ff00000)>>20;
} else if ((chnl==10)|(chnl==11)|(chnl==12)){
vmeDevicePtr->read("NCRdBaseL321",&iwrdat);
if(chnl==10)blstat=(iwrdat&0x3ff);
if(chnl==11)blstat=(iwrdat&0xffc00)>>10;
if(chnl==12)blstat=(iwrdat&0x3ff00000)>>20;
} else if ((chnl==13)|(chnl==14)|(chnl==15)){
vmeDevicePtr->read("NCRdBaseL654",&iwrdat);
if(chnl==13)blstat=(iwrdat&0x3ff);
if(chnl==14)blstat=(iwrdat&0xffc00)>>10;
if(chnl==15)blstat=(iwrdat&0x3ff00000)>>20;
} else if ((chnl==16)|(chnl==17)|(chnl==18)){
vmeDevicePtr->read("NCRdBaseL987",&iwrdat);
if(chnl==16)blstat=(iwrdat&0x3ff);
if(chnl==17)blstat=(iwrdat&0xffc00)>>10;
if(chnl==18)blstat=(iwrdat&0x3ff00000)>>20;
} else if ((chnl==19)|(chnl==20)|(chnl==21)){
vmeDevicePtr->read("SCRdBaseL321",&iwrdat);
if(chnl==19)blstat=(iwrdat&0x3ff);
if(chnl==20)blstat=(iwrdat&0xffc00)>>10;
if(chnl==21)blstat=(iwrdat&0x3ff00000)>>20;
} else if ((chnl==22)|(chnl==23)|(chnl==24)){
vmeDevicePtr->read("SCRdBaseL654",&iwrdat);
if(chnl==22)blstat=(iwrdat&0x3ff);
if(chnl==23)blstat=(iwrdat&0xffc00)>>10;
if(chnl==24)blstat=(iwrdat&0x3ff00000)>>20;
} else if ((chnl==25)|(chnl==26)|(chnl==27)){
vmeDevicePtr->read("SCRdBaseL987",&iwrdat);
if(chnl==25)blstat=(iwrdat&0x3ff);
if(chnl==26)blstat=(iwrdat&0xffc00)>>10;
if(chnl==27)blstat=(iwrdat&0x3ff00000)>>20;
} else if ((chnl==28)|(chnl==29)|(chnl==30)){
vmeDevicePtr->read("SRdBaseL321",&iwrdat);
if(chnl==28)blstat=(iwrdat&0x3ff);
if(chnl==29)blstat=(iwrdat&0xffc00)>>10;
if(chnl==30)blstat=(iwrdat&0x3ff00000)>>20;
} else if ((chnl==31)|(chnl==32)|(chnl==33)){
vmeDevicePtr->read("SRdBaseL654",&iwrdat);
if(chnl==31)blstat=(iwrdat&0x3ff);
if(chnl==32)blstat=(iwrdat&0xffc00)>>10;
if(chnl==33)blstat=(iwrdat&0x3ff00000)>>20;
} else if ((chnl==34)|(chnl==35)|(chnl==36)){
vmeDevicePtr->read("SRdBaseL987",&iwrdat);
if(chnl==34)blstat=(iwrdat&0x3ff);
if(chnl==35)blstat=(iwrdat&0xffc00)>>10;
if(chnl==36)blstat=(iwrdat&0x3ff00000)>>20;
} else {
cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" no data for channel "<<chnl<<" Channels 1-36 only"<<endl;
return -1;
}
if(blstat&0x200)blstat=-1*(((~blstat)&0x1ff)+1);
return blstat;
  }//end
//This method gets the firmware date for a single chip
//0=North, 1=NorthCenter, 2=SouthCenter, 3=South, 4=Center 
uint32_t PixelPh0FEDInterface::get_FirmwareDate(int chip) {

uint32_t iwrdat=0;
if(chip<0) return(0);
else if(chip>4) return(0);
uint32_t offset=0x1f0000;
vmeDevicePtr->read(FPGAName[chip],&iwrdat,offset);
cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" "<<FPGAName[chip]<<" FPGA firmware date d/m/y "
<<dec<<((iwrdat&0xff000000)>>24)<<"/"
<<dec<<((iwrdat&0xff0000)>>16)<<"/"
<<dec<<((((iwrdat&0xff00)>>8)*100)+(iwrdat&0xff))<<endl;

return iwrdat;
}//end
/////////////////////////////////////////////////////////////////////////////
//This method gets the firmware date for the VME interface chip 
uint32_t PixelPh0FEDInterface::get_VMEFirmwareDate(void) {

uint32_t iwrdat=0;
uint32_t offset=0xa0003c;
vmeDevicePtr->read("LAD_N",&iwrdat,offset);
cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" VME FPGA (update via jtag pins only) firmware date d/m/y "
<<dec<<((iwrdat&0xff000000)>>24)<<"/"
<<dec<<((iwrdat&0xff0000)>>16)<<"/"
<<dec<<((((iwrdat&0xff00)>>8)*100)+(iwrdat&0xff))<<endl;

return iwrdat;
}//end

void PixelPh0FEDInterface::get_PiggyFirmwareVer() {
  uint32_t du, dd;
  vmeDevicePtr->read("LAD_N", &du, 0x158000);
  vmeDevicePtr->read("LAD_N", &dd, 0x178000);
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" N Piggy version: ";
  printf("%1x%1x%1x%1x%1x%1x\n", (dd>>20)&0xf, (dd>>12)&0xf, (dd>>4)&0xf, (du>>20)&0xf, (du>>12)&0xf, (du>>4)&0xf);
  vmeDevicePtr->read("LAD_S", &du, 0x158000);
  vmeDevicePtr->read("LAD_S", &dd, 0x178000);
  cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" S Piggy version: ";
  printf("%1x%1x%1x%1x%1x%1x\n", (dd>>20)&0xf, (dd>>12)&0xf, (dd>>4)&0xf, (du>>20)&0xf, (du>>12)&0xf, (du>>4)&0xf);
}

/////////////////////////////////////////////////////////////////////////////
// Read BX counter
int PixelPh0FEDInterface::readBXCounter() {
  uint32_t data; 
#ifdef USE_HAL // Use HAL
  vmeDevicePtr->read("RdBunchCntr",&data);

#else // direct CAEN VME access
  CVErrorCodes ret = CAENVME_ReadCycle(BHandle,BX_NUM,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in read "<<hex<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret);
  }
#endif // USE_HAL
  if(Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" BX "<<hex<<data<<dec<<endl;
  return(data);
} // end


//////////////////////////////////////////////////////////////////////
// Histogram access routines.
//This routine selects which histogram memory to read at the channel level
//there are 3 channels for every address as the (8 bit) counters are compressed
//in to a single long word:
//There are Up and Dn memories with the histograms, we define
// // triplets are arranged as: to read the Up
// // 1  2  3  4  5  6 <-triplet number
// // 1  4  7 10 13 16 <-channel of lowest 8 bit counter in word for triplet number
// // 2  5  8 11 14 17 <-channel of middle 8 bit counter in word for triplet number
// // 3  6  9 12 15 18 <-channel of highest 8 bit counter in word for triplet number
//  To read the down Dn (note, Up and Dn are just different addresses, we do the
//  right thing internally, which is why we defined tripplet number this way
// // 7  8  9  10 11 12 <-triplet number
// // 19 22 25 28 31 34 <-channel of lowest 8 bit counter in word for triplet number
// // 20 23 26 29 32 35 <-channel of middle 8 bit counter in word for triplet number
// // 21 24 27 30 33 36 <-channel of highest 8 bit counter in word for triplet number
 int PixelPh0FEDInterface::selectTripple(const int trip) {
assert(0);
   uint32_t loctrip;                                                                                                                            
   if((trip<1)||(trip>12)){
     cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Error: Tripplet request "<<dec<<trip<<" out of bounds, must be 1-12"
	 <<endl;
     return(-1);
   }
                                                                                                                            
   if(trip<7){ //looking at channels 1-18
     loctrip=trip;

#ifdef USE_HAL // Use HAL
     vmeDevicePtr->write("TripleSelUp",loctrip);

#else // direct CAEN VME access
     ret = CAENVME_WriteCycle(BHandle,TripleSelUp,&loctrip,am,dw);
     if(ret != cvSuccess) {  // Error
       cout<<"Error in Selecting histogram tripple "<<hex<<ret<<" "<<dec<<datum<<endl;
       analyzeError(ret);
     }
#endif                                                                                                                            
   } else {  //looking at channels 19-36
     loctrip=trip-6;

#ifdef USE_HAL // Use HAL
     vmeDevicePtr->write("TripleSelDn",loctrip);

#else // direct CAEN VME access
     ret = CAENVME_WriteCycle(BHandle,TripleSelDn,&loctrip,am,dw);
     if(ret != cvSuccess) {  // Error
       cout<<"Error in Selecting histogram tripple "<<hex<<ret<<" "<<dec<<datum
	   <<endl;
       analyzeError(ret);
     }
#endif                                                                                                                            
   }
   return(0);
   
 } //end
//////////////////////////////////////////////////////////////////////
// Read expects 1st Roc to be #1
 int PixelPh0FEDInterface::drainHisRoc(const int trip,const int Roc, uint32_t *data){
assert(0);
   uint32_t offset = (Roc-1)*4;
   //const uint32_t length = 104; //number of double columns(26) *bytes(4) for block reads - wait for v4!
   uint32_t locdata;  
   //char * buffer = (char *) data;//for block reads - wait for v4!

   if((Roc<1)||(Roc>24)){
     cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Error: Roc request "<<dec<<Roc<<" out of bounds, must be 1-24"
	 <<endl;
     return(-1);
   }
   if((trip<1)||(trip>12)){
     cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Error: Tripplet request "<<dec<<trip<<" out of bounds, must be 1-12"
	 <<endl;
     return(-1);
   }

   if(trip<7){ //looking at channels 1-18

# ifdef USE_HAL // Use HAL
// Need to be careful!! The full 5 bits must be used in the Double Column
// memory counter to reset the next read = 26 real reads + 6 dummy reads


for(int j=1;j<27;j++){vmeDevicePtr->read("ROCHisMemUp",&locdata,offset);*data=locdata;data++;}
for(int j=27;j<33;j++){vmeDevicePtr->read("ROCHisMemUp",&locdata);} //DC 27 - 31 dummy

#else // direct CAEN VME access

     for(int j=1;j<27;j++){
       ret = CAENVME_WriteCycle(BHandle,RocSelUp+Roc*4,&loctrip,am,dw);
       if(ret != cvSuccess) {  // Error
	 cout<<"Error in Selecting histogram tripple "<<hex<<ret<<" "<<dec<<datum
	     <<endl;
	 analyzeError(ret);
       }
       *data=locdata;
       data++;
     }   
#endif                                                                                                                            
   } else {  //looking at channels 19-36

# ifdef USE_HAL // Use HAL                                                                                                                           
for(int j=1;j<27;j++){vmeDevicePtr->read("ROCHisMemDn",&locdata,offset);*data=locdata;data++;}
for(int j=27;j<33;j++){vmeDevicePtr->read("ROCHisMemDn",&locdata);}//DC 27 - 31 dummy 


#else // direct CAEN VME access
     for(int j=1;j<27;j++){
       ret = CAENVME_WriteCycle(BHandle,RocSelDn+Roc*4,&loctrip,am,dw);
       if(ret != cvSuccess) {  // Error
	 cout<<"Error in Selecting histogram tripple "<<hex<<ret<<" "<<dec<<datum
	     <<endl;
	 analyzeError(ret);
       }
       *data=locdata;
       data++;
     }
#endif                                                                                                                            
   }
   return(0);
                
 } //end
//////////////////////////////////////////////////////////////////////
// // drain all the Double Columns in all rocs  for a particular tripple
// // 
 int PixelPh0FEDInterface::drainTripple(const int trip, uint32_t *pnt) {
assert(0);
   int status=0;
   
   if((trip<1)||(trip>12)){
     cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Error: Tripplet request "<<dec<<trip<<" out of bounds, must be 1-12"
	 <<endl;
     return(-1);
   }
//see if all channels in a tripple are on, if not, don't read
/////////////////////////////////////////////////////////////////////////////////////
// // triplets are arranged as: to read the Up
// // 1  2  3  4  5  6 <-triplet number
// // 1  4  7 10 13 16 <-channel of lowest 8 bit counter in word for triplet number
// // 2  5  8 11 14 17 <-channel of middle 8 bit counter in word for triplet number
// // 3  6  9 12 15 18 <-channel of highest 8 bit counter in word for triplet number
//  To read the down Dn (note, Up and Dn are just different addresses, we do the
//  right thing internally, which is why we defined tripplet number this way
// // 7  8  9  10 11 12 <-triplet number
// // 19 22 25 28 31 34 <-channel of lowest 8 bit counter in word for triplet number
// // 20 23 26 29 32 35 <-channel of middle 8 bit counter in word for triplet number
// // 21 24 27 30 33 36 <-channel of highest 8 bit counter in word for triplet number
////////////////////////////////////////////////////////////////////////////////////   
//chnls = (trip-1)*3 +1,2,3
//Get max # Rocs for all on channels   
int maxroc=0;
//cout<<hex<<"0x "<<!pixelFEDCard.Ncntrl<<" 0x"<<(0x1<<((trip-1)*3))<<endl;

if(trip<4){
 if(!(pixelFEDCard.Ncntrl&(0x1<<((trip-1)*3))))maxroc=pixelFEDCard.NRocs[((trip-1)*3)];   
 if(!(pixelFEDCard.Ncntrl&(0x1<<((trip-1)*3+1))))maxroc=max(maxroc,pixelFEDCard.NRocs[((trip-1)*3+1)]);   
 if(!(pixelFEDCard.Ncntrl&(0x1<<((trip-1)*3+2))))maxroc=max(maxroc,pixelFEDCard.NRocs[((trip-1)*3+2)]);   
  if(maxroc<1) return 0;}
else if((trip>3)&&(trip<7)){
 if(!(pixelFEDCard.NCcntrl&(0x1<<((trip-4)*3))))maxroc=pixelFEDCard.NRocs[((trip-1)*3)];   
 if(!(pixelFEDCard.NCcntrl&(0x1<<((trip-4)*3+1))))maxroc=max(maxroc,pixelFEDCard.NRocs[((trip-1)*3+1)]);   
 if(!(pixelFEDCard.NCcntrl&(0x1<<((trip-4)*3+2))))maxroc=max(maxroc,pixelFEDCard.NRocs[((trip-1)*3+2)]);   
//cout<<hex<<"0x"<<pixelFEDCard.NCcntrl<<" 0x"<<(0x1<<((trip-4)*3))<<dec<<endl;

  if(maxroc<1) return 0;}
else if((trip>6)&&(trip<10)){
 if(!(pixelFEDCard.NCcntrl&(0x1<<((trip-7)*3))))maxroc=pixelFEDCard.NRocs[((trip-1)*3)];   
 if(!(pixelFEDCard.NCcntrl&(0x1<<((trip-7)*3+1))))maxroc=max(maxroc,pixelFEDCard.NRocs[((trip-1)*3+1)]);   
 if(!(pixelFEDCard.NCcntrl&(0x1<<((trip-7)*3+2))))maxroc=max(maxroc,pixelFEDCard.NRocs[((trip-1)*3+2)]);   
  if(maxroc<1) return 0;}
else if(trip>9){
 if(!(pixelFEDCard.NCcntrl&(0x1<<((trip-10)*3))))maxroc=pixelFEDCard.NRocs[((trip-1)*3)];   
 if(!(pixelFEDCard.NCcntrl&(0x1<<((trip-10)*3+1))))maxroc=max(maxroc,pixelFEDCard.NRocs[((trip-1)*3+1)]);   
 if(!(pixelFEDCard.NCcntrl&(0x1<<((trip-10)*3+2))))maxroc=max(maxroc,pixelFEDCard.NRocs[((trip-1)*3+2)]);   
  if(maxroc<1) return 0;}
   status = selectTripple(trip);
   if(status!=0) return 0;
  // cout<<trip<<" "<<maxroc<<endl;
//set up memory locations

CVErrorCodes ret;
 CVDataWidth dw[24*32];   // data width (see CAENVMEtypes.h )
 CVAddressModifier am[24*32]; 
 CVErrorCodes errs[24*32];
 uint32_t cycles=0;
 uint32_t addrses[24*32];
 uint32_t data[24*32];

if(maxroc>24){
cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Max # rocs limited to 24 in triplet read "<<endl;
maxroc=24;}

for(int i=0;i<(maxroc*32);i++){
dw[i]=cvD32;
am[i]=cvA32_U_DATA;}



if(trip<7){
for(int i=0;i<maxroc;i++){//loop over ROCs
for(int j=0;j<32;j++){//loop over DC's

addrses[i*32+j]=pixelFEDCard.FEDBASE_0+CHIP[4]+0x40000+(i)*4;
cycles++;
}}

} else {

for(int i=0;i<maxroc;i++){//loop over ROCs
for(int j=0;j<32;j++){//loop over DC's

addrses[i*32+j]=pixelFEDCard.FEDBASE_0+CHIP[4]+0x48000+(i)*4;
cycles++;
}}

}
  ret = CAENVME_MultiRead(0,addrses,data,cycles,am,dw,errs);
//  cout<<"triplet "<<trip<<" cycles "<<cycles<<" maxroc "<<maxroc<<endl;	
  if(ret != cvSuccess) {  // Error 
   cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Error in CAEN Multi read "<<hex<<ret<<" "<<data[0]<<dec;
  switch (ret) {
  case cvGenericError   : cout<<" Generic error !!!"<<endl;
    break ;
  case cvBusError  : cout<<" Bus Error !!!"<<endl;
    break ;                                                                    
  case cvCommError : cout<<" Communication Error !!!"<<endl;
    break ;
  case cvInvalidParam : cout<<" Invalid Param Error !!!"<<endl;
    break ;
  default          : cout<<" Unknown Error !!!"<<endl;
    break ;
  }
  return 0;
}
//cout<<"maxroc"<<maxroc<<endl;

*pnt=(uint32_t) trip;pnt++;
*pnt=(uint32_t) maxroc;pnt++;
 
for(int j=0;j<(int)cycles;j++){*pnt=data[j];pnt++;}


   return(cycles+2);
 } //end
//////////////////////////////////////////////////////////////////////
// Drain all the Double Columns in all Rocs of all channels for a 
// particular fed
// 
 int PixelPh0FEDInterface::drainHisMemory(uint32_t *data) {
assert(0);
   int status=0;
   int count=0;
   for(int trip=1;trip<13;trip++){
     //cout<<"draining histogram memories for tripplet"<<dec<<trip<<endl;
     status=drainTripple(trip,data);
     data+=status;//increment for number of double columns in a tripple
     count+=status;
     //cout<<"triple "<<trip<<" count "<<count<<endl;
   }
   return count;
 } //end
////////////////////////////////////////////////////////////////////////////
int PixelPh0FEDInterface::enableHisMemory(int enable) {
  int data; // should probably be uint32_t?
  if(enable==1) { // make an explicit check
    data=1;
    if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Histogram Memories Enabled"<<endl;
  } else {
    data=0;
    if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Histogram Memories Disabled "<<endl;
  } 

#ifdef USE_HAL // Use HAL

  vmeDevicePtr->write("EnabHisto",data);

#else // direct CAEN VME access
  ret = CAENVME_WriteCycle(BHandle,EnabHisto,&data,am,dw);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
    analyzeError(ret); 
    return -1;
  }
#endif  
  return 0;
} // end
////////////////////////////////////////////////////////////////////////////
// Clear the histogrammming memories
 void PixelPh0FEDInterface::clear_hismem(void) {
assert(0);
   uint32_t data = 0x1; // Toggle???
   
#ifdef USE_HAL // Use HAL
   vmeDevicePtr->write("ClearHist",data);
#else  
   CVErrorCodes  ret = CAENVME_WriteCycle(BHandle,ClearHist,&data,am,dw);
   if(ret != cvSuccess) {  // Error
     cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
     analyzeError(ret);
   }
#endif  
   if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Clearing Histogram Memories"<<endl;
   usleep(13);
 } // end
////////////////////////////////////////////////////////////////////////
// // gets an event from the spy fifo 3's and forms an 64 bit slink data
// // packet. Uses the header and trailer for checking data integrity
// // returns 2 (empty) or # of words and a pointer to 1st word if ok
// // returns a negative number with an error if there's a problem
// //

 int PixelPh0FEDInterface::spySlink64(uint64_t *data) {
   //cout<<item<<" "<<hex<<offset<<dec<<" "<<length<<endl;
   
   //drain the spy fifo 3up
   //look through the words
   //find header and trailer
   //check data length
   //drain spy fifo 3dn the correct number of words
   //form data words
   
   uint32_t mlength = 1024*4; // in bytes
   uint32_t mbuffer[1024];
   uint32_t moffset=0;
   int mwdcnt=-1;
   
   //drain whole spy fifo
   vmeDevicePtr->readBlock("RdSpyFifoUp",mlength,(char *) mbuffer,HAL::HAL_NO_INCREMENT,moffset);
   
   if(((mbuffer[0]&0xf0000000)>>28)!=0x5) {
     if (printIfSlinkHeaderMessedup) {
       cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Slink Header Messed up!"<<endl;
       if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Will dump the first 50 words in the buffer:"<<endl;
       for(uint32_t i=0;i<50;i++) {
	 if(Printlevel&1)cout<<"mbuffer["<<i<<"]="<<hex<<mbuffer[i]<<dec<<endl;
       }
     }
     return mwdcnt;
   }
   
   data[0]=(uint64_t)(mbuffer[0])<<32;
   int pwdcnt=1;
   
   while((mwdcnt<0)&(pwdcnt<1024)) {
     data[pwdcnt]=(uint64_t)(mbuffer[pwdcnt])<<32;
     if(((mbuffer[pwdcnt]&0xf0000000)>>28)!=0xa) {pwdcnt++;}
     else {mwdcnt=pwdcnt;}
   }
   
   if(mwdcnt<0)
     {cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" No trailer in 1024 words, Dumping diagnostics:"<<endl;

   //drain whole other half of spy fifo
   vmeDevicePtr->readBlock("RdSpyFifoDn",mlength,(char *) mbuffer,HAL::HAL_NO_INCREMENT,moffset);

   cout<<"Dumping spy fifo-3 buffer"<<endl;
    for(int ij=0;ij<1024;ij++)cout<<hex<<(data[ij]+ (uint64_t)(mbuffer[ij]))<<dec<<endl;
   cout<<"Dumping fifo state"<<endl;
    dump_FifoStatus(getFifoStatus());
   cout<<"Dumping TTS fifo"<<endl;
       // Read TTS FIFO 
    pwdcnt=drainTTSFifo(mbuffer); // Read TTS FIFOs
     for(int ij=0;ij<pwdcnt;ij++)cout<<hex<<mbuffer[ij]<<dec<<endl;
   cout<<"Dumping spy fifo II's"<<endl;
    for(int ix=1;ix<9;ix++){
       pwdcnt=drainDataFifo2(ix,mbuffer);
      cout<<" Spy fifo2, "<<ix<<" count = "<<pwdcnt<<endl; 
     for(int ij=0;ij<pwdcnt;ij++)cout<<hex<<mbuffer[ij]<<dec<<endl;}
    cout<<"Dumping mini spy fifo I"<<endl;   
     pwdcnt=drainSpyFifo1up(mbuffer);
     cout<<"looking at spy fifo 1 up words= "<<pwdcnt<<endl;
     for(int ij=0;ij<pwdcnt;ij++)cout<<hex<<mbuffer[ij]<<dec<<endl;

     pwdcnt=drainSpyFifo1dn(mbuffer);
     cout<<"looking at spy fifo 1 dn words= "<<pwdcnt<<endl;
     for(int ij=0;ij<pwdcnt;ij++)cout<<hex<<mbuffer[ij]<<dec<<endl;
     
     return mwdcnt;}
   
   if(((mbuffer[mwdcnt]&0x00ffffff)-1)!=(uint32_t) mwdcnt) {
     cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Unpacked word count does not match actual";
     cout<<"FEDID:"<<pixelFEDCard.fedNumber<<".. mbuffer[mwdcnt]="<<dec<<(mbuffer[mwdcnt]&0x00ffffff)<<" and mwdcnt="<<(uint32_t)mwdcnt<<endl;
     if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Will dump the first 50 words in the buffer:"<<endl;
     for(uint32_t i=0;i<50;i++) {
       if(Printlevel&1)cout<<"mbuffer["<<i<<"]="<<hex<<mbuffer[i]<<dec<<endl;
     }
     return -2;
   }
   
   //unpack just as many words as we need
   vmeDevicePtr->readBlock("RdSpyFifoDn",(uint32_t) (mwdcnt+1)*4,(char *) mbuffer,HAL::HAL_NO_INCREMENT,moffset);
   
   for(int i=0;i<mwdcnt+1;i++) {//	cout<<"data = "<<hex<<data[i]<<" mbuffer= "<<mbuffer[i]<<dec<<endl;
     data[i]=(data[i] + mbuffer[i]);
   }
   
   if(mwdcnt>0) {
     return(mwdcnt+1);
   } else {
     return(mwdcnt);
   }
   
 } //end
////////////////////////////////////////////////////////////////////////
// // gets an event from the spy fifo 3's and forms an 64 bit slink data
// // packet. Uses the header and trailer for checking data integrity
// // returns -2 (empty) or # of words and a pointer to 1st word if ok
// // returns a negative number with an error if there's a problem
// // if CRCchk is true performs a CRC check
// //

 int PixelPh0FEDInterface::spySlink64(uint64_t *data,bool CRCchk) {
   //cout<<item<<" "<<hex<<offset<<dec<<" "<<length<<endl;
   
   //drain the spy fifo 3up
   //look through the words
   //find header and trailer
   //check data length
   //drain spy fifo 3dn the correct number of words
   //form data words
   
   uint32_t mlength = 1024*4; // in bytes
   uint32_t mbuffer[1024];
   uint32_t moffset=0;
   int mwdcnt=-1;
   
   //drain whole spy fifo
   vmeDevicePtr->readBlock("RdSpyFifoUp",mlength,(char *) mbuffer,HAL::HAL_NO_INCREMENT,moffset);
   
   if(((mbuffer[0]&0xf0000000)>>28)!=0x5) {
     cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Slink Header Messed up!"<<endl;
     if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Will dump the first 50 words in the buffer:"<<endl;
     for(uint32_t i=0;i<50;i++) {
       if(Printlevel&1)cout<<"mbuffer["<<i<<"]="<<hex<<mbuffer[i]<<dec<<endl;
     }
     return mwdcnt;
   }
   
   data[0]=(uint64_t)(mbuffer[0])<<32;
   int pwdcnt=1;
   
   while((mwdcnt<0)&(pwdcnt<1024)) {
     data[pwdcnt]=(uint64_t)(mbuffer[pwdcnt])<<32;
     if(((mbuffer[pwdcnt]&0xf0000000)>>28)!=0xa) {pwdcnt++;}
     else {mwdcnt=pwdcnt;}
   }
   
   if(mwdcnt<0)
     {cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" No trailer in 1024 words Dumping Diagnostics"<<endl;
     
     
     
   //drain whole other half of spy fifo
   vmeDevicePtr->readBlock("RdSpyFifoDn",mlength,(char *) mbuffer,HAL::HAL_NO_INCREMENT,moffset);

   cout<<"Dumping spy fifo-3 buffer"<<endl;
    for(int ij=0;ij<1024;ij++)cout<<hex<<(data[ij]+ (uint64_t)(mbuffer[ij]))<<dec<<endl;
   cout<<"Dumping fifo state"<<endl;
    dump_FifoStatus(getFifoStatus());
   cout<<"Dumping TTS fifo"<<endl;
       // Read TTS FIFO 
    pwdcnt=drainTTSFifo(mbuffer); // Read TTS FIFOs
     for(int ij=0;ij<pwdcnt;ij++)cout<<hex<<mbuffer[ij]<<dec<<endl;
   cout<<"Dumping spy fifo II's"<<endl;
    for(int ix=1;ix<9;ix++){
       pwdcnt=drainDataFifo2(ix,mbuffer);
      cout<<" Spy fifo2, "<<ix<<" count = "<<pwdcnt<<endl; 
     for(int ij=0;ij<pwdcnt;ij++)cout<<hex<<mbuffer[ij]<<dec<<endl;}
    cout<<"Dumping mini spy fifo I"<<endl;   
     pwdcnt=drainSpyFifo1up(mbuffer);
     cout<<"looking at spy fifo 1 up words= "<<pwdcnt<<endl;
     for(int ij=0;ij<pwdcnt;ij++)cout<<hex<<mbuffer[ij]<<dec<<endl;

     pwdcnt=drainSpyFifo1dn(mbuffer);
     cout<<"looking at spy fifo 1 dn words= "<<pwdcnt<<endl;
     for(int ij=0;ij<pwdcnt;ij++)cout<<hex<<mbuffer[ij]<<dec<<endl;

     
     
     
     return mwdcnt;}
   
   if(((mbuffer[mwdcnt]&0x00ffffff)-1)!=(uint32_t) mwdcnt) {
     cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Unpacked word count does not match actual";
     cout<<"FEDID:"<<pixelFEDCard.fedNumber<<".. mbuffer[mwdcnt]="<<dec<<(mbuffer[mwdcnt]&0x00ffffff)<<" and mwdcnt="<<(uint32_t)mwdcnt<<endl;
     if(Printlevel&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Will dump the first 50 words in the buffer:"<<endl;
     for(uint32_t i=0;i<50;i++) {
       if(Printlevel&1)cout<<"mbuffer["<<i<<"]="<<hex<<mbuffer[i]<<dec<<endl;
     }
     return -2;
   }
   
   //unpack just as many words as we need
   vmeDevicePtr->readBlock("RdSpyFifoDn",(uint32_t) (mwdcnt+1)*4,(char *) mbuffer,HAL::HAL_NO_INCREMENT,moffset);
   
   for(int i=0;i<mwdcnt+1;i++) {//	cout<<"data = "<<hex<<data[i]<<" mbuffer= "<<mbuffer[i]<<dec<<endl;
     data[i]=(data[i] + mbuffer[i]);
   }
   
   if(mwdcnt>0) {
if(CRCchk){  
//cout<<"***********************************CHECKING CRC********************************"<<endl;
 uint64_t tmpdata;
 int C,NewCRC,OldCRC=0;
        C = 0xFFFF;  
 for(int i=0;i<mwdcnt+1;i++){

tmpdata=data[i];

if(i==mwdcnt){tmpdata=data[i]&0xffffffff0000fffbULL;OldCRC=((mbuffer[i]&0xFFFF0000)>>16);}        

  NewCRC = 0;
  
  NewCRC = lbitval(63,tmpdata) ^ lbitval(62,tmpdata) ^ lbitval(61,tmpdata) ^ lbitval(60,tmpdata) ^ lbitval(55,tmpdata) ^ lbitval(54,tmpdata) ^
           lbitval(53,tmpdata) ^ lbitval(52,tmpdata) ^ lbitval(51,tmpdata) ^ lbitval(50,tmpdata) ^ lbitval(49,tmpdata) ^ lbitval(48,tmpdata) ^
           lbitval(47,tmpdata) ^ lbitval(46,tmpdata) ^ lbitval(45,tmpdata) ^ lbitval(43,tmpdata) ^ lbitval(41,tmpdata) ^ lbitval(40,tmpdata) ^
           lbitval(39,tmpdata) ^ lbitval(38,tmpdata) ^ lbitval(37,tmpdata) ^ lbitval(36,tmpdata) ^ lbitval(35,tmpdata) ^ lbitval(34,tmpdata) ^
           lbitval(33,tmpdata) ^ lbitval(32,tmpdata) ^ lbitval(31,tmpdata) ^ lbitval(30,tmpdata) ^ lbitval(27,tmpdata) ^ lbitval(26,tmpdata) ^
           lbitval(25,tmpdata) ^ lbitval(24,tmpdata) ^ lbitval(23,tmpdata) ^ lbitval(22,tmpdata) ^ lbitval(21,tmpdata) ^ lbitval(20,tmpdata) ^
           lbitval(19,tmpdata) ^ lbitval(18,tmpdata) ^ lbitval(17,tmpdata) ^ lbitval(16,tmpdata) ^ lbitval(15,tmpdata) ^ lbitval(13,tmpdata) ^
           lbitval(12,tmpdata) ^ lbitval(11,tmpdata) ^ lbitval(10,tmpdata) ^ lbitval(9,tmpdata) ^ lbitval(8,tmpdata) ^ lbitval(7,tmpdata) ^
           lbitval(6,tmpdata) ^ lbitval(5,tmpdata) ^ lbitval(4,tmpdata) ^ lbitval(3,tmpdata) ^ lbitval(2,tmpdata) ^ lbitval(1,tmpdata) ^
           lbitval(0,tmpdata) ^ sbitval(0,C) ^ sbitval(1,C) ^ sbitval(2,C) ^ sbitval(3,C) ^ sbitval(4,C) ^
           sbitval(5,C) ^ sbitval(6,C) ^ sbitval(7,C) ^ sbitval(12,C) ^ sbitval(13,C) ^ sbitval(14,C) ^
           sbitval(15,C);

  NewCRC = NewCRC +
           ((lbitval(63,tmpdata) ^ lbitval(62,tmpdata) ^ lbitval(61,tmpdata) ^ lbitval(56,tmpdata) ^ lbitval(55,tmpdata) ^ lbitval(54,tmpdata) ^
           lbitval(53,tmpdata) ^ lbitval(52,tmpdata) ^ lbitval(51,tmpdata) ^ lbitval(50,tmpdata) ^ lbitval(49,tmpdata) ^ lbitval(48,tmpdata) ^
           lbitval(47,tmpdata) ^ lbitval(46,tmpdata) ^ lbitval(44,tmpdata) ^ lbitval(42,tmpdata) ^ lbitval(41,tmpdata) ^ lbitval(40,tmpdata) ^
           lbitval(39,tmpdata) ^ lbitval(38,tmpdata) ^ lbitval(37,tmpdata) ^ lbitval(36,tmpdata) ^ lbitval(35,tmpdata) ^ lbitval(34,tmpdata) ^
           lbitval(33,tmpdata) ^ lbitval(32,tmpdata) ^ lbitval(31,tmpdata) ^ lbitval(28,tmpdata) ^ lbitval(27,tmpdata) ^ lbitval(26,tmpdata) ^
           lbitval(25,tmpdata) ^ lbitval(24,tmpdata) ^ lbitval(23,tmpdata) ^ lbitval(22,tmpdata) ^ lbitval(21,tmpdata) ^ lbitval(20,tmpdata) ^
           lbitval(19,tmpdata) ^ lbitval(18,tmpdata) ^ lbitval(17,tmpdata) ^ lbitval(16,tmpdata) ^ lbitval(14,tmpdata) ^ lbitval(13,tmpdata) ^
           lbitval(12,tmpdata) ^ lbitval(11,tmpdata) ^ lbitval(10,tmpdata) ^ lbitval(9,tmpdata) ^ lbitval(8,tmpdata) ^ lbitval(7,tmpdata) ^
           lbitval(6,tmpdata) ^ lbitval(5,tmpdata) ^ lbitval(4,tmpdata) ^ lbitval(3,tmpdata) ^ lbitval(2,tmpdata) ^ lbitval(1,tmpdata) ^
           sbitval(0,C) ^ sbitval(1,C) ^ sbitval(2,C) ^ sbitval(3,C) ^ sbitval(4,C) ^ sbitval(5,C) ^
           sbitval(6,C) ^ sbitval(7,C) ^ sbitval(8,C) ^ sbitval(13,C) ^ sbitval(14,C) ^ sbitval(15,C))<<1);

  NewCRC = NewCRC +
           ((lbitval(61,tmpdata) ^ lbitval(60,tmpdata) ^ lbitval(57,tmpdata) ^ lbitval(56,tmpdata) ^ lbitval(46,tmpdata) ^ lbitval(42,tmpdata) ^
           lbitval(31,tmpdata) ^ lbitval(30,tmpdata) ^ lbitval(29,tmpdata) ^ lbitval(28,tmpdata) ^ lbitval(16,tmpdata) ^ lbitval(14,tmpdata) ^
           lbitval(1,tmpdata) ^ lbitval(0,tmpdata) ^ sbitval(8,C) ^ sbitval(9,C) ^ sbitval(12,C) ^ sbitval(13,C))<<2);
  NewCRC = NewCRC +
           ((lbitval(62,tmpdata) ^ lbitval(61,tmpdata) ^ lbitval(58,tmpdata) ^ lbitval(57,tmpdata) ^ lbitval(47,tmpdata) ^ lbitval(43,tmpdata) ^
           lbitval(32,tmpdata) ^ lbitval(31,tmpdata) ^ lbitval(30,tmpdata) ^ lbitval(29,tmpdata) ^ lbitval(17,tmpdata) ^ lbitval(15,tmpdata) ^
           lbitval(2,tmpdata) ^ lbitval(1,tmpdata) ^ sbitval(9,C) ^ sbitval(10,C) ^ sbitval(13,C) ^ sbitval(14,C))<<3);
  NewCRC = NewCRC +
           ((lbitval(63,tmpdata) ^ lbitval(62,tmpdata) ^ lbitval(59,tmpdata) ^ lbitval(58,tmpdata) ^ lbitval(48,tmpdata) ^ lbitval(44,tmpdata) ^
           lbitval(33,tmpdata) ^ lbitval(32,tmpdata) ^ lbitval(31,tmpdata) ^ lbitval(30,tmpdata) ^ lbitval(18,tmpdata) ^ lbitval(16,tmpdata) ^
           lbitval(3,tmpdata) ^ lbitval(2,tmpdata) ^ sbitval(0,C) ^ sbitval(10,C) ^ sbitval(11,C) ^ sbitval(14,C) ^
           sbitval(15,C))<<4);
  NewCRC = NewCRC +
           ((lbitval(63,tmpdata) ^ lbitval(60,tmpdata) ^ lbitval(59,tmpdata) ^ lbitval(49,tmpdata) ^ lbitval(45,tmpdata) ^ lbitval(34,tmpdata) ^
           lbitval(33,tmpdata) ^ lbitval(32,tmpdata) ^ lbitval(31,tmpdata) ^ lbitval(19,tmpdata) ^ lbitval(17,tmpdata) ^ lbitval(4,tmpdata) ^
           lbitval(3,tmpdata) ^ sbitval(1,C) ^ sbitval(11,C) ^ sbitval(12,C) ^ sbitval(15,C))<<5);
  NewCRC = NewCRC +
  ((lbitval(61,tmpdata) ^ lbitval(60,tmpdata) ^ lbitval(50,tmpdata) ^ lbitval(46,tmpdata) ^ lbitval(35,tmpdata) ^ lbitval(34,tmpdata) ^
           lbitval(33,tmpdata) ^ lbitval(32,tmpdata) ^ lbitval(20,tmpdata) ^ lbitval(18,tmpdata) ^ lbitval(5,tmpdata) ^ lbitval(4,tmpdata) ^
           sbitval(2,C) ^ sbitval(12,C) ^ sbitval(13,C))<<6);
  NewCRC = NewCRC +
           ((lbitval(62,tmpdata) ^ lbitval(61,tmpdata) ^ lbitval(51,tmpdata) ^ lbitval(47,tmpdata) ^ lbitval(36,tmpdata) ^ lbitval(35,tmpdata) ^
           lbitval(34,tmpdata) ^ lbitval(33,tmpdata) ^ lbitval(21,tmpdata) ^ lbitval(19,tmpdata) ^ lbitval(6,tmpdata) ^ lbitval(5,tmpdata) ^
           sbitval(3,C) ^ sbitval(13,C) ^ sbitval(14,C))<<7);
   NewCRC= NewCRC +
           ((lbitval(63,tmpdata) ^ lbitval(62,tmpdata) ^ lbitval(52,tmpdata) ^ lbitval(48,tmpdata) ^ lbitval(37,tmpdata) ^ lbitval(36,tmpdata) ^
           lbitval(35,tmpdata) ^ lbitval(34,tmpdata) ^ lbitval(22,tmpdata) ^ lbitval(20,tmpdata) ^ lbitval(7,tmpdata) ^ lbitval(6,tmpdata) ^
           sbitval(0,C) ^ sbitval(4,C) ^ sbitval(14,C) ^ sbitval(15,C))<<8);
   NewCRC= NewCRC +
           ((lbitval(63,tmpdata) ^ lbitval(53,tmpdata) ^ lbitval(49,tmpdata) ^ lbitval(38,tmpdata) ^ lbitval(37,tmpdata) ^ lbitval(36,tmpdata) ^
           lbitval(35,tmpdata) ^ lbitval(23,tmpdata) ^ lbitval(21,tmpdata) ^ lbitval(8,tmpdata) ^ lbitval(7,tmpdata) ^ sbitval(1,C) ^
           sbitval(5,C) ^ sbitval(15,C))<<9);
   NewCRC= NewCRC +
           ((lbitval(54,tmpdata) ^ lbitval(50,tmpdata) ^ lbitval(39,tmpdata) ^ lbitval(38,tmpdata) ^ lbitval(37,tmpdata) ^ lbitval(36,tmpdata) ^
           lbitval(24,tmpdata) ^ lbitval(22,tmpdata) ^ lbitval(9,tmpdata) ^ lbitval(8,tmpdata) ^ sbitval(2,C) ^ sbitval(6,C))<<10);
   NewCRC= NewCRC +
           ((lbitval(55,tmpdata) ^ lbitval(51,tmpdata) ^ lbitval(40,tmpdata) ^ lbitval(39,tmpdata) ^ lbitval(38,tmpdata) ^ lbitval(37,tmpdata) ^
           lbitval(25,tmpdata) ^ lbitval(23,tmpdata) ^ lbitval(10,tmpdata) ^ lbitval(9,tmpdata) ^ sbitval(3,C) ^ sbitval(7,C))<<11);
   NewCRC= NewCRC +
           ((lbitval(56,tmpdata) ^ lbitval(52,tmpdata) ^ lbitval(41,tmpdata) ^ lbitval(40,tmpdata) ^ lbitval(39,tmpdata) ^ lbitval(38,tmpdata) ^
           lbitval(26,tmpdata) ^ lbitval(24,tmpdata) ^ lbitval(11,tmpdata) ^ lbitval(10,tmpdata) ^ sbitval(4,C) ^ sbitval(8,C))<<12);
   NewCRC= NewCRC +
           ((lbitval(57,tmpdata) ^ lbitval(53,tmpdata) ^ lbitval(42,tmpdata) ^ lbitval(41,tmpdata) ^ lbitval(40,tmpdata) ^ lbitval(39,tmpdata) ^
           lbitval(27,tmpdata) ^ lbitval(25,tmpdata) ^ lbitval(12,tmpdata) ^ lbitval(11,tmpdata) ^ sbitval(5,C) ^ sbitval(9,C))<<13);
   NewCRC= NewCRC +
           ((lbitval(58,tmpdata) ^ lbitval(54,tmpdata) ^ lbitval(43,tmpdata) ^ lbitval(42,tmpdata) ^ lbitval(41,tmpdata) ^ lbitval(40,tmpdata) ^
           lbitval(28,tmpdata) ^ lbitval(26,tmpdata) ^ lbitval(13,tmpdata) ^ lbitval(12,tmpdata) ^ sbitval(6,C) ^ sbitval(10,C))<<14);
   NewCRC= NewCRC +
           ((lbitval(63,tmpdata) ^ lbitval(62,tmpdata) ^ lbitval(61,tmpdata) ^ lbitval(60,tmpdata) ^ lbitval(59,tmpdata) ^ lbitval(54,tmpdata) ^
           lbitval(53,tmpdata) ^ lbitval(52,tmpdata) ^ lbitval(51,tmpdata) ^ lbitval(50,tmpdata) ^ lbitval(49,tmpdata) ^ lbitval(48,tmpdata) ^
           lbitval(47,tmpdata) ^ lbitval(46,tmpdata) ^ lbitval(45,tmpdata) ^ lbitval(44,tmpdata) ^ lbitval(42,tmpdata) ^ lbitval(40,tmpdata) ^
           lbitval(39,tmpdata) ^ lbitval(38,tmpdata) ^ lbitval(37,tmpdata) ^ lbitval(36,tmpdata) ^ lbitval(35,tmpdata) ^ lbitval(34,tmpdata) ^
           lbitval(33,tmpdata) ^ lbitval(32,tmpdata) ^ lbitval(31,tmpdata) ^ lbitval(30,tmpdata) ^ lbitval(29,tmpdata) ^ lbitval(26,tmpdata) ^
           lbitval(25,tmpdata) ^ lbitval(24,tmpdata) ^ lbitval(23,tmpdata) ^ lbitval(22,tmpdata) ^ lbitval(21,tmpdata) ^ lbitval(20,tmpdata) ^
           lbitval(19,tmpdata) ^ lbitval(18,tmpdata) ^ lbitval(17,tmpdata) ^ lbitval(16,tmpdata) ^ lbitval(15,tmpdata) ^ lbitval(14,tmpdata) ^
           lbitval(12,tmpdata) ^ lbitval(11,tmpdata) ^ lbitval(10,tmpdata) ^ lbitval(9,tmpdata) ^ lbitval(8,tmpdata) ^ lbitval(7,tmpdata) ^
           lbitval(6,tmpdata) ^ lbitval(5,tmpdata) ^ lbitval(4,tmpdata) ^ lbitval(3,tmpdata) ^ lbitval(2,tmpdata) ^ lbitval(1,tmpdata) ^
           lbitval(0,tmpdata) ^ sbitval(0,C) ^ sbitval(1,C) ^ sbitval(2,C) ^ sbitval(3,C) ^ sbitval(4,C) ^
           sbitval(5,C) ^ sbitval(6,C) ^ sbitval(11,C) ^ sbitval(12,C) ^ sbitval(13,C) ^ sbitval(14,C) ^
           sbitval(15,C))<<15);

C=NewCRC;

   if((i==mwdcnt)&&(NewCRC!=OldCRC)){
   cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" "<<hex<<NewCRC<<" CRC MisMatch! "<<OldCRC<<dec<<endl;
   return(-mwdcnt-1);}
 }  
}//CRCchk  
     return(mwdcnt+1);
   } else {
     return(mwdcnt);
   }
   
 } //end
////////////////////////////////////////////////////////////////////////////
// Decode Slink Data
int PixelPh0FEDInterface::PwordSlink64(uint64_t * ldata, const int length, uint32_t &totword) {
 
  // The header-trailer looks OK  
  if( (ldata[0]&0xf000000000000000LL) != 0x5000000000000000LL ) 
    {cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" error on data header "<<hex<<ldata[0]<<dec<<endl;return 0;}
  if( (ldata[length-1]&0xf000000000000000LL) != 0xa000000000000000LL ) 
    {cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" error on data trailer "<<hex<<ldata[length-1]<<dec<<endl;return 0;}
  
//analyze the data buffer to find private words

int fif2cnt=0;
int dumcnt=0;
int gapcnt=0;
uint32_t gap[8];
uint32_t dum[8];
uint32_t word1=0;
uint32_t word2=0;
uint32_t chan=0;
uint32_t roc=0;

const uint32_t rocmsk = 0x3e00000;
const uint32_t chnlmsk = 0xfc000000;

for(int jk=0;jk<8;jk++)gap[jk]=0;
for(int jk=0;jk<8;jk++)dum[jk]=0;

int fifcnt=1;

for(int kk=1;kk<length-1;kk++) 
{

word2 = (uint32_t) ldata[kk];
word1 = (uint32_t) (ldata[kk]>>32);

//1st word

chan= ((word1&chnlmsk)>>26);
roc= ((word1&rocmsk)>>21);

//count non-error words
if(roc<25){if(dumcnt>0){dumcnt=0;cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" ***Stale dummy!"<<endl;}//stale dummy!
if((chan<5)&&(fifcnt!=1))cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" error in fifo counting!"<<endl;
if((chan>4)&&(chan<10)&&(fifcnt!=2)) {fif2cnt=0;fifcnt=2;}
if((chan>9)&&(chan<14)&&(fifcnt!=3)) {fif2cnt=0;fifcnt=3;}
if((chan>13)&&(chan<19)&&(fifcnt!=4)){fif2cnt=0;fifcnt=4;}
if((chan>18)&&(chan<23)&&(fifcnt!=5)){fif2cnt=0;fifcnt=5;}
if((chan>22)&&(chan<28)&&(fifcnt!=6)){fif2cnt=0;fifcnt=6;}
if((chan>27)&&(chan<32)&&(fifcnt!=7)){fif2cnt=0;fifcnt=7;}
if((chan>31)&&(fifcnt!=8)){fif2cnt=0;fifcnt=8;} fif2cnt++;}

if(roc==26){gap[fifcnt-1]=(0x1000+(word1&0xff));gapcnt++;}

if((roc==27)&&((fif2cnt+dumcnt)<6)){dumcnt++;dum[fifcnt-1]=(0x1000+(word1&0xff));}
else if((roc==27)&&((fif2cnt+dumcnt)>6)){dumcnt=1;fif2cnt=0;fifcnt++;}

//2nd word

chan= ((word2&chnlmsk)>>26);
roc= ((word2&rocmsk)>>21);

if(roc<25){if(dumcnt>0){dumcnt=0;cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" ***Stale dummy!"<<endl;}//stale dummy!
if((chan<5)&&(fifcnt!=1))cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" error in fifo counting!"<<endl;
if((chan>4)&&(chan<10)&&(fifcnt!=2)) {fif2cnt=0;fifcnt=2;}
if((chan>9)&&(chan<14)&&(fifcnt!=3)) {fif2cnt=0;fifcnt=3;}
if((chan>13)&&(chan<19)&&(fifcnt!=4)){fif2cnt=0;fifcnt=4;}
if((chan>18)&&(chan<23)&&(fifcnt!=5)){fif2cnt=0;fifcnt=5;}
if((chan>22)&&(chan<28)&&(fifcnt!=6)){fif2cnt=0;fifcnt=6;}
if((chan>27)&&(chan<32)&&(fifcnt!=7)){fif2cnt=0;fifcnt=7;}
if((chan>31)&&(fifcnt!=8)){fif2cnt=0;fifcnt=8;} fif2cnt++;}

if(roc==26){gap[fifcnt-1]=(0x1000+(word2&0xff));gapcnt++;}

if ((roc==27)&&((fif2cnt+dumcnt)<6)) {dumcnt++;dum[fifcnt-1]=(0x1000+(word1&0xff));}
else if((roc==27)&&((fif2cnt+dumcnt)>6)){dumcnt=1;fif2cnt=0;fifcnt++;}


//word check complete
if(((fif2cnt+dumcnt)==6)&&(dumcnt>0)) //done with this fifo 
{dumcnt=0;fif2cnt=0;fifcnt++;}
if((gapcnt>0)&&((dumcnt+fif2cnt)>5))//done with this fifo
{gapcnt=0;fifcnt++;fif2cnt=0;dumcnt=0;}
else if((gapcnt>0)&&((dumcnt+fif2cnt)<6)) gapcnt=0;

}//end of fifo-3 word loop-see what we got!

int status=0;

if(gap[0]>0) {totword=(gap[0]&0xff);status=1;}
else if(gap[1]>0){totword=(gap[1]&0xff);status=1;}
else if(dum[0]>0){totword=(dum[0]&0xff);status=1;}
else if(dum[1]>0){totword=(dum[1]&0xff);status=1;}

if(gap[2]>0) {totword=totword|((gap[2]&0xff)<<8);status=status|0x2;}
else if(gap[3]>0){totword=totword|((gap[3]&0xff)<<8);status=status|0x2;}
else if(dum[2]>0){totword=totword|((dum[2]&0xff)<<8);status=status|0x2;}
else if(dum[3]>0){totword=totword|((dum[3]&0xff)<<8);status=status|0x2;}

if(gap[4]>0) {totword=totword|((gap[4]&0xff)<<16);status=status|0x4;}
else if(gap[5]>0){totword=totword|((gap[5]&0xff)<<16);status=status|0x4;}
else if(dum[4]>0){totword=totword|((dum[4]&0xff)<<16);status=status|0x4;}
else if(dum[5]>0){totword=totword|((dum[5]&0xff)<<16);status=status|0x4;}

if(gap[6]>0){totword=totword|((gap[6]&0xff)<<24);status=status|0x8;}
else if(gap[7]>0){totword=totword|((gap[7]&0xff)<<24);status=status|0x8;}
else if(dum[6]>0){totword=totword|((dum[6]&0xff)<<24);status=status|0x8;}
else if(dum[7]>0){totword=totword|((dum[7]&0xff)<<24);status=status|0x8;}
return(status);

}
////////////////////////////////////////////////////////////////////////
// // gets an event from the spy fifo 1 for ch 1-4 and forms a data buffer. 
// //

 int PixelPh0FEDInterface::drainSpyFifo1up(uint32_t *data) {
int wordCount=0;
//drain the spy fifo 1up 
uint32_t offset = 0;
  // Chanege to block read at some point
  const uint32_t length = spyFifo2Length; // size of SPY-FIFO2 in bytes, is it 128? 
  char * buffer = (char *) data;
  vmeDevicePtr->readBlock("NRdSpyIUp",length,buffer,HAL::HAL_NO_INCREMENT,offset);
  //find the wordCount?
  for(uint32_t i=0; i<(length/4);i++) {
    //cout<<dec<<i<<" "<<hex<<data[i]<<endl;
    if(i>0 && data[i]==0) {  // Assume taht after data=0 there is nothing more? 
      wordCount=i;          // Problem, sometomes 0s are already at the beginning.
      break;
    }
    wordCount=i;
  }

  return(wordCount);
 } //end
////////////////////////////////////////////////////////////////////////
// // gets an event from the spy fifo 1 for ch 32-36 and forms a data buffer. 
// //

 int PixelPh0FEDInterface::drainSpyFifo1dn(uint32_t *data) {
int wordCount=0;
//drain the spy fifo 1up 
uint32_t offset = 0;
  // Chanege to block read at some point
  const uint32_t length = spyFifo2Length; // size of SPY-FIFO2 in bytes, is it 128? 
  char * buffer = (char *) data;
  vmeDevicePtr->readBlock("NRdSpyIDn",length,buffer,HAL::HAL_NO_INCREMENT,offset);
  //find the wordCount?
  for(uint32_t i=0; i<(length/4);i++) {
    //cout<<dec<<i<<" "<<hex<<data[i]<<endl;
    if(i>0 && data[i]==0) {  // Assume taht after data=0 there is nothing more? 
      wordCount=i;          // Problem, sometomes 0s are already at the beginning.
      break;
    }
    wordCount=i;
  }

  return(wordCount);

 } //end

void PixelPh0FEDInterface::testTTSbits(uint32_t data,int enable) {
//will turn on the test bits indicate in bits 0...3 as long as bit 31 is set
//As of this writing, the bits indicated are: 0(Warn), 1(OOS), 2(Busy), 4(Ready)
//Use a 1 or any >1 to enable, a 0 or <0 to disable

if(enable>0){data=((data|0x80000000)&0x8000000f);} 
else 
{data=data&0x0000000f;}

  vmeDevicePtr->write("TTStest",data);
} //end
/////////////////////////////////////////////////////////////////////////
int PixelPh0FEDInterface::testSlink() {
//sends a test word on the slink, then resets the mode register to the database value

uint32_t data =pixelFEDCard.modeRegister | 0x80000000;
  vmeDevicePtr->write("ModeReg",data);

usleep(10);

int status = setModeRegister(pixelFEDCard.modeRegister);
return status; 
} //end
/////////////////////////////////////////////////////////////////////////
uint32_t PixelPh0FEDInterface::getFifoStatus(void) {
//gets the word containg status of fifoI,II,III

uint32_t data = 0;
  vmeDevicePtr->read("CtrlReg",&data);

return data; 
} //end
/////////////////////////////////////////////////////////////////////////
void PixelPh0FEDInterface::dump_FifoStatus(uint32_t fword) {
/*from an email by Helmut Steininger-decode word retuned by getFifoStatus
[0] .. AlmostFull FIFO I  North ( all 9 InputChannels ored )
[1] .. NearlyFull FIFO II  North ( or of both FIFO II )
[2] .. AlmostFull FIFO I  NorthCenter
[3] .. NearlyFull FIFO II  NorthCenter
[4] .. AlmostFull FIFO I  SouthCenter
[5] .. NearlyFull FIFO II  SouthCenter
[6] .. AlmostFull FIFO I  South
[7] .. NearlyFull FIFO II  South
[8] .. AlmostFull FIFO III UP
[9] .. AlmostFUll FIFO III DOWN
*/
if(fword&1)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" [0] .. AlmostFull FIFO I  North ( all 9 InputChannels ored )"<<endl;
if(fword&2)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" [1] .. NearlyFull FIFO II  North ( or of both FIFO II )      "<<endl;
if(fword&4)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" [2] .. AlmostFull FIFO I  NorthCenter                       "<<endl;
if(fword&8)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" [3] .. NearlyFull FIFO II  NorthCenter                       "<<endl;
if(fword&0x10)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" [4] .. AlmostFull FIFO I  SouthCenter                       "<<endl;
if(fword&0x20)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" [5] .. NearlyFull FIFO II  SouthCenter                       "<<endl;
if(fword&0x40)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" [6] .. AlmostFull FIFO I  South                             "<<endl;
if(fword&0x80)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" [7] .. NearlyFull FIFO II  South                             "<<endl;
if(fword&0x100)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" [8] .. AlmostFull FIFO III UP                               "<<endl;
if(fword&0x200)cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" [9] .. AlmostFUll FIFO III DOWN                             "<<endl;

}

/////////////////////////////////////////////////////////////////////////
// Polling 31st bit of Central Chip's register - Souvik Das
bool PixelPh0FEDInterface::isWholeEvent(uint32_t nTries)
{

  bool eventExists=false;
  uint32_t itry=0;
  uint32_t data=0x80000000;
  PixelTimer timer;
   do {
     timer.start();
     vmeDevicePtr->read("RdEventCntr", &data);
     usleep(10);
     eventExists=((data & 0x80000000)!=0x0);
     ++itry;
     timer.stop();
    if (itry%10000==0) cout << "[PixelPh0FEDInterface::isWholeEvent] itry="<<itry<<endl;
  } while ( (!eventExists) && (timer.tottime()<1.0*nTries));

  return eventExists;
}
/////////////////////////////////////////////////////////////////////////
// Polling 31st bit of Central Chip's register - Souvik Das
bool PixelPh0FEDInterface::isNewEvent(uint32_t nTries)
{

  bool eventExists=false;
  uint32_t itry=0;
  uint32_t data=0x80000000;
  PixelTimer timer;
  timer.start();
  do {
     timer.start();
     usleep(10);
     vmeDevicePtr->read("RdEventCntr", &data);
     eventExists=((data & 0x80000000)==0x0);
     ++itry;
     timer.stop();
    if (itry%10000==0) cout << "[PixelPh0FEDInterface::isNewEvent] itry="<<itry<<endl;
  } while ( (!eventExists) && (timer.tottime()<1.0*nTries) );

  return eventExists;
}
////////////////////////////////////////////////////////////////////////
void PixelPh0FEDInterface::set_Printlevel(int level)
{Printlevel=level;
cout<<"FEDID:"<<pixelFEDCard.fedNumber<<"Setting Print level ="<<Printlevel<<endl;}

void PixelPh0FEDInterface::set_Printlevel_silent(int level)
{Printlevel=level;}

////////////////////////////////////////////////////////////////////////
void PixelPh0FEDInterface::set_TTslevels(void)
{

if(pixelFEDCard.Ooslvl>1023){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" number of TTs OOS exceeds max, setting to 1023 "<<endl;pixelFEDCard.Ooslvl=1023;}
if(pixelFEDCard.Errlvl>1023){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" number of TTs EER exceeds max, setting to 1023 "<<endl;pixelFEDCard.Errlvl=1023;}
if(pixelFEDCard.Ooslvl<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" number of TTs OOS <1, setting to 1 "<<endl;pixelFEDCard.Ooslvl=640;}
if(pixelFEDCard.Errlvl<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" number of TTs EER <1, setting to 1 "<<endl;pixelFEDCard.Errlvl=640;}

  // Introduce the new OR of ERROR and OOS states. Make it 1 less than ERROR. d.k. 2/6/10
  uint32_t oosErrOr = (uint32_t) pixelFEDCard.TimeoutOROOSLimit;
  uint32_t data = (pixelFEDCard.Ooslvl&0x3ff) + ((pixelFEDCard.Errlvl&0x3ff)<<10) + ((oosErrOr&0x3ff)<<20);

  vmeDevicePtr->write("TTsErrOoslvl", data );  // program it

  cout<<"TTS-OOS, TTS-ERROT, TTS-OR limits:"<<pixelFEDCard.Ooslvl<<" "<<pixelFEDCard.Errlvl<<" "<<oosErrOr<<endl;

}
////////////////////////////////////////////////////////////////////////
void PixelPh0FEDInterface::set_TTslevels(int inmoos,int inmerr)//Sets adjustable TTs consecutive levels for OOS and ERR
{
if(inmoos>1023){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" number of TTs OOS exceeds max, setting to 1023 "<<endl;pixelFEDCard.Ooslvl=1023;}
if(inmerr>1023){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" number of TTs EER exceeds max, setting to 1023 "<<endl;pixelFEDCard.Errlvl=1023;}
if(inmoos<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" number of TTs OOS <1, setting to 1 "<<endl;pixelFEDCard.Ooslvl=640;}
if(inmerr<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" number of TTs EER <1, setting to 1 "<<endl;pixelFEDCard.Errlvl=640;}

  // Introduce the new OR of ERROR and OOS states. Make it 1 less than ERROR. d.k. 2/6/10
  uint32_t oosErrOr = pixelFEDCard.TimeoutOROOSLimit;
  uint32_t data = (pixelFEDCard.Ooslvl&0x3ff) + ((pixelFEDCard.Errlvl&0x3ff)<<10) + ((oosErrOr&0x3ff)<<20);
//uint32_t data = (pixelFEDCard.Ooslvl&0x3ff) + ((pixelFEDCard.Errlvl&0x3ff)<<10);

  vmeDevicePtr->write("TTsErrOoslvl", data );  // program it

  cout<<"TTS-OOS, TTS-ERROT, TTS-OR limits:"<<pixelFEDCard.Ooslvl<<" "<<pixelFEDCard.Errlvl<<" "<<oosErrOr<<endl;

}
////////////////////////////////////////////////////////////////////////
void PixelPh0FEDInterface::set_Fifolevels(void)
{
if(pixelFEDCard.Nfifo1Bzlvl>1023){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" N fifo-1 almost full level exceeds max, setting to 1023 "<<endl;pixelFEDCard.Nfifo1Bzlvl=1023;}
if(pixelFEDCard.Nfifo1Bzlvl<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" N fifo-1 almost full level<1! setting to 1 "<<endl;pixelFEDCard.Nfifo1Bzlvl=1;}
if(pixelFEDCard.NCfifo1Bzlvl>1023){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" NC fifo-1 almost full level exceeds max, setting to 1023 "<<endl;pixelFEDCard.NCfifo1Bzlvl=1023;}
if(pixelFEDCard.NCfifo1Bzlvl<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" NC fifo-1 almost full level<1! setting to 1 "<<endl;pixelFEDCard.NCfifo1Bzlvl=1;}
if(pixelFEDCard.SCfifo1Bzlvl>1023){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" SC fifo-1 almost full level exceeds max, setting to 1023 "<<endl;pixelFEDCard.SCfifo1Bzlvl=1023;}
if(pixelFEDCard.SCfifo1Bzlvl<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" SC fifo-1 almost full level<1! setting to 1 "<<endl;pixelFEDCard.SCfifo1Bzlvl=1;}
if(pixelFEDCard.Sfifo1Bzlvl>1023){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" S fifo-1 almost full level exceeds max, setting to 1023 "<<endl;pixelFEDCard.Sfifo1Bzlvl=1023;}
if(pixelFEDCard.Sfifo1Bzlvl<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" S fifo-1 almost full level<1! setting to 1 "<<endl;pixelFEDCard.Sfifo1Bzlvl=1;}

if(pixelFEDCard.fifo3Wrnlvl>8191){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" fifo-3 almost full level exceeds max, setting to 8191 "<<endl;pixelFEDCard.fifo3Wrnlvl=8191;}
if(pixelFEDCard.fifo3Wrnlvl<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" fifo-3 almost full level<1! setting to 1 "<<endl;pixelFEDCard.fifo3Wrnlvl=1;}


//   uint32_t data = (pixelFEDCard.Nfifo1Bzlvl&0x3ff);
//   vmeDevicePtr->write("NFifo1bzlv", data );
//   data = (pixelFEDCard.Nfifo1Bzlvl&0x3ff);
//   vmeDevicePtr->write("NCFifo1bzlv", data );
//   data = (pixelFEDCard.Nfifo1Bzlvl&0x3ff);
//   vmeDevicePtr->write("SCFifo1bzlv", data );
//   data = (pixelFEDCard.Nfifo1Bzlvl&0x3ff);
//   vmeDevicePtr->write("SFifo1bzlv", data );
//   data =(pixelFEDCard.fifo3Wrnlvl&0x1fff);
//   vmeDevicePtr->write("Fifo3Warnlvl", data );

// new code to set the Warining limit for fifo2
  const uint32_t fifo2_limit = (uint32_t) pixelFEDCard.FIFO2Limit;
  cout<<" The limit on fifi-2 is "<<pixelFEDCard.FIFO2Limit<<endl;
  uint32_t data = ((pixelFEDCard.Nfifo1Bzlvl&0x3ff)+fifo2_limit)<<10;
  vmeDevicePtr->write("NFifo1bzlv", data );
  data = ((pixelFEDCard.Nfifo1Bzlvl&0x3ff)+fifo2_limit)<<10;
  vmeDevicePtr->write("NCFifo1bzlv", data );
  data = ((pixelFEDCard.Nfifo1Bzlvl&0x3ff)+fifo2_limit)<<10;
  vmeDevicePtr->write("SCFifo1bzlv", data );
  data = ((pixelFEDCard.Nfifo1Bzlvl&0x3ff)+fifo2_limit)<<10;
  vmeDevicePtr->write("SFifo1bzlv", data );





}
////////////////////////////////////////////////////////////////////////
void PixelPh0FEDInterface::set_Fifo1levels(int Nfif1,int NCfif1,int SCfif1,int Sfif1)
{

pixelFEDCard.Nfifo1Bzlvl=Nfif1;
pixelFEDCard.NCfifo1Bzlvl=NCfif1;
pixelFEDCard.SCfifo1Bzlvl=SCfif1;
pixelFEDCard.Sfifo1Bzlvl=Sfif1;

if(Nfif1>1023){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" N fifo-1 almost full level exceeds max, setting to 1023 "<<endl;pixelFEDCard.Nfifo1Bzlvl=1023;}
if(Nfif1<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" N fifo-1 almost full level<1! setting to 1 "<<endl;pixelFEDCard.Nfifo1Bzlvl=1;}
if(NCfif1>1023){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" NC fifo-1 almost full level exceeds max, setting to 1023 "<<endl;pixelFEDCard.NCfifo1Bzlvl=1023;}
if(NCfif1<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" NC fifo-1 almost full level<1! setting to 1 "<<endl;pixelFEDCard.NCfifo1Bzlvl=1;}
if(SCfif1>1023){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" SC fifo-1 almost full level exceeds max, setting to 1023 "<<endl;pixelFEDCard.SCfifo1Bzlvl=1023;}
if(SCfif1<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" SC fifo-1 almost full level<1! setting to 1 "<<endl;pixelFEDCard.SCfifo1Bzlvl=1;}
if(Sfif1>1023){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" S fifo-1 almost full level exceeds max, setting to 1023 "<<endl;pixelFEDCard.Sfifo1Bzlvl=1023;}
if(Sfif1<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" S fifo-1 almost full level<1! setting to 1 "<<endl;pixelFEDCard.Sfifo1Bzlvl=1;}

uint32_t data = (pixelFEDCard.Nfifo1Bzlvl&0x3ff);
  vmeDevicePtr->write("NFifo1bzlv", data );
data = (pixelFEDCard.NCfifo1Bzlvl&0x3ff);
  vmeDevicePtr->write("NCFifo1bzlv", data );
data = (pixelFEDCard.SCfifo1Bzlvl&0x3ff);
  vmeDevicePtr->write("SCFifo1bzlv", data );
data = (pixelFEDCard.Sfifo1Bzlvl&0x3ff);
  vmeDevicePtr->write("SFifo1bzlv", data );
}
////////////////////////////////////////////////////////////////////////
void PixelPh0FEDInterface::set_Fifo3levels(int Cfif3)
{
if(Cfif3>8191){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" fifo-3 almost full level exceeds max, setting to 8191 "<<endl;pixelFEDCard.fifo3Wrnlvl=8191;}
if(Cfif3<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" fifo-3 almost full level<1! setting to 1 "<<endl;pixelFEDCard.fifo3Wrnlvl=1;}

uint32_t data =(pixelFEDCard.fifo3Wrnlvl&0x1fff);
  vmeDevicePtr->write("Fifo3Warnlvl", data );
}
////////////////////////////////////////////////////////////////////////
//These bits set the limit on the number of hits/event on the channels
//of the fpga in question
//
void PixelPh0FEDInterface::set_HitLimits(void)
{     
 
if(pixelFEDCard.N_hitlimit>900){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" N fifo-1 hit limit exceeds max, setting to 900 "<<endl;pixelFEDCard.N_hitlimit=900;}
//if(pixelFEDCard.N_hitlimit<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" N fifo-1 hit limit<1! setting to 1 "<<endl;pixelFEDCard.N_hitlimit=1;}
if(pixelFEDCard.NC_hitlimit>900){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" NC fifo-1 hit limit exceeds max, setting to 900 "<<endl;pixelFEDCard.NC_hitlimit=900;}
//if(pixelFEDCard.NC_hitlimit<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" NC fifo-1 hit limit<1! setting to 1 "<<endl;pixelFEDCard.NC_hitlimit=1;}
if(pixelFEDCard.SC_hitlimit>900){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" SC fifo-1 hit limit exceeds max, setting to 900 "<<endl;pixelFEDCard.SC_hitlimit=900;}
//if(pixelFEDCard.SC_hitlimit<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" SC fifo-1 hit limit<1! setting to 1 "<<endl;pixelFEDCard.SC_hitlimit=1;}
if(pixelFEDCard.S_hitlimit>900){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" S fifo-1 hit limit exceeds max, setting to 900 "<<endl;pixelFEDCard.S_hitlimit=900;}
//if(pixelFEDCard.S_hitlimit<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" S fifo-1 hit limit<1! setting to 1 "<<endl;pixelFEDCard.S_hitlimit=1;}

 uint32_t offset = 0x1e8000;
 
 /*
   busyWhenBehind must be set = 0 for very old (front042810) firmware
   for later firmware (front070810 for example), 4<<16 is our nominal setting
   For later firmware, a setting of = 0 is equivalent to = 200.
 */
 
 // edit here to change the busy threshold for channels in timeout recovery
 uint32_t busyWhenBehind=(uint32_t) pixelFEDCard.BusyWhenBehind;
 busyWhenBehind = busyWhenBehind<<16;  //number of events behind to assert BUSY

 // Add the Minimum Busy programming d.k. 18/8/11
 uint32_t minW = (uint32_t) pixelFEDCard.BusyHoldMin;
 if(minW>3||minW<0) {cout<<" Illegal minimum width setting, setting to 0!"<<endl;minW=0;}

 cout <<"FEDID:"<<pixelFEDCard.fedNumber<<" busy generation enabled when behind " 
      << (busyWhenBehind>>16) << " events.";
 if(minW==0)      cout<<" Busy Minimum width for busy when behind is: 100 us"<<endl;
 else if(minW==1) cout<<" Busy Minimum width for busy when behind is: 50 us"<<endl;
 else if(minW==2) cout<<" Busy Minimum width for busy when behind is: 25 us"<<endl;
 else if(minW==3) cout<<" Busy Minimum width for busy when behind is: 12.5 us"<<endl;
   
 minW = minW<<24;   // change from 23 d.k. 24/8/11
      
 uint32_t data = (pixelFEDCard.N_hitlimit&0x3ff)+busyWhenBehind+minW;
 vmeDevicePtr->write(FPGAName[0],data,HAL::HAL_NO_VERIFY,offset);
 data = (pixelFEDCard.NC_hitlimit&0x3ff)+busyWhenBehind+minW;
 vmeDevicePtr->write(FPGAName[1],data,HAL::HAL_NO_VERIFY,offset);
 data = (pixelFEDCard.SC_hitlimit&0x3ff)+busyWhenBehind+minW;
 vmeDevicePtr->write(FPGAName[2],data,HAL::HAL_NO_VERIFY,offset);
 data = (pixelFEDCard.S_hitlimit&0x3ff)+busyWhenBehind+minW;
 vmeDevicePtr->write(FPGAName[3],data,HAL::HAL_NO_VERIFY,offset);

}
////////////////////////////////////////////////////////////////////////
//Tese bits set the limit on the number of hits/event on the channels
//of the fpga in question
//
void PixelPh0FEDInterface::set_HitLimits(int Nlimt,int NClimt,int SClimt,int Slimt)
{
pixelFEDCard.N_hitlimit=Nlimt;
pixelFEDCard.NC_hitlimit=NClimt;
pixelFEDCard.SC_hitlimit=SClimt;
pixelFEDCard.S_hitlimit=Slimt;

if(Nlimt>1023){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" N fifo-1 hit limit exceeds max, setting to 900 "<<endl;pixelFEDCard.N_hitlimit=900;}
//if(Nlimt<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" N fifo-1 hit limit<1! setting to 1 "<<endl;pixelFEDCard.N_hitlimit=1;}
if(NClimt>1023){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" NC fifo-1 hit limit exceeds max, setting to 900 "<<endl;pixelFEDCard.NC_hitlimit=900;}
//if(NClimt<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" NC fifo-1 hit limit<1! setting to 1 "<<endl;pixelFEDCard.NC_hitlimit=1;}
if(SClimt>1023){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" SC fifo-1 hit limit exceeds max, setting to 900 "<<endl;pixelFEDCard.SC_hitlimit=900;}
//if(SClimt<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" SC fifo-1 hit limit<1! setting to 1 "<<endl;pixelFEDCard.SC_hitlimit=1;}
if(Slimt>1023){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" S fifo-1 hit limit exceeds max, setting to 900 "<<endl;pixelFEDCard.S_hitlimit=900;}
//if(Slimt<1){cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" S fifo-1 hit limit<1! setting to 1 "<<endl;pixelFEDCard.S_hitlimit=1;}

      uint32_t offset = 0x1e8000;
      
uint32_t data = (pixelFEDCard.N_hitlimit&0x3ff)+(12<<16);
 vmeDevicePtr->write(FPGAName[0],data,HAL::HAL_NO_VERIFY,offset);
data = (pixelFEDCard.NC_hitlimit&0x3ff)+(12<<16);
 vmeDevicePtr->write(FPGAName[1],data,HAL::HAL_NO_VERIFY,offset);
data = (pixelFEDCard.SC_hitlimit&0x3ff)+(12<<16);
 vmeDevicePtr->write(FPGAName[2],data,HAL::HAL_NO_VERIFY,offset);
data = (pixelFEDCard.S_hitlimit&0x3ff)+(12<<16);
 vmeDevicePtr->write(FPGAName[3],data,HAL::HAL_NO_VERIFY,offset);
}
////////////////////////////////////////////////////////////////////////
//These bits allow a ROC to be skipped (1/fpga) if the ROC puts out
//BBB instead of UlB B LD
//
void PixelPh0FEDInterface::set_ROCskip(void)
{
#ifndef PILOT_FED
//Check data words
 if((pixelFEDCard.N_testreg&0x7e0)>0){
if((pixelFEDCard.N_testreg&0x7e0)>(0x120))
{pixelFEDCard.N_testreg=0;cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" N BBB skip exceeds ch 9, set to 0"<<endl;}
else
{int chanl=(pixelFEDCard.N_testreg&0x7e0)>>5;
if(((pixelFEDCard.N_testreg&0x1f)>=pixelFEDCard.NRocs[chanl-1])|((pixelFEDCard.N_testreg&0x1f)==0))
{pixelFEDCard.N_testreg=0;cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" N BBB skip exceeds #ROCS-1 or = 0, set to 0"<<endl;}}
 } else {pixelFEDCard.N_testreg=0;}

  if((pixelFEDCard.NC_testreg&0x7e0)>0){
if(((pixelFEDCard.NC_testreg&0x7e0)>(0x240))|((pixelFEDCard.NC_testreg&0x7e0)<(0x140)))
{pixelFEDCard.NC_testreg=0;cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" NC BBB skip ch must be 10-18, set to 0"<<endl;}
else
{int chanl=(pixelFEDCard.NC_testreg&0x7e0)>>5;
if(((pixelFEDCard.NC_testreg&0x1f)>=pixelFEDCard.NRocs[chanl-1])|((pixelFEDCard.NC_testreg&0x1f)==0))
{pixelFEDCard.NC_testreg=0;cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" NC BBB skip exceeds #ROCS-1 or = 0, set to 0"<<endl;}}
  } else {pixelFEDCard.NC_testreg=0;}

 if((pixelFEDCard.S_testreg&0x7e0)>0){
if(((pixelFEDCard.S_testreg&0x7e0)>(0x480))|((pixelFEDCard.S_testreg&0x7e0)<(0x380)))
{pixelFEDCard.S_testreg=0;cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" S BBB skip ch must be 28-36, set to 0"<<endl;}
else 
{int chanl=(pixelFEDCard.S_testreg&0x7e0)>>5;
if(((pixelFEDCard.S_testreg&0x1f)>=pixelFEDCard.NRocs[chanl-1])|((pixelFEDCard.S_testreg&0x1f)==0))
{pixelFEDCard.S_testreg=0;cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" S BBB skip exceeds #ROCS-1 or = 0, set to 0"<<endl;}}
 } else {pixelFEDCard.S_testreg=0;}

  if((pixelFEDCard.SC_testreg&0x7e0)>0){
if(((pixelFEDCard.SC_testreg&0x7e0)>(0x360))|((pixelFEDCard.SC_testreg&0x7e0)<(0x260))) 
{pixelFEDCard.SC_testreg=0;cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" SC BBB skip ch must be 19-27, set to 0"<<endl;}
else
{int chanl=(pixelFEDCard.SC_testreg&0x7e0)>>5;if((pixelFEDCard.SC_testreg&0x1f)>=pixelFEDCard.NRocs[chanl-1])
{pixelFEDCard.SC_testreg=0;cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" SC BBB skip exceeds #ROCS-1 or = 0, set to 0"<<endl;}}
  } else {pixelFEDCard.SC_testreg=0;}
#else
uint32_t offset = 0x1a8000;
std::cout << "set_ROCskip " << std::hex << pixelFEDCard.N_testreg << " " << pixelFEDCard.NC_testreg << " " << pixelFEDCard.SC_testreg << " " << pixelFEDCard.S_testreg << std::dec << std::endl;
vmeDevicePtr->write(FPGAName[0], pixelFEDCard.N_testreg,  HAL::HAL_NO_VERIFY, offset);
vmeDevicePtr->write(FPGAName[1], pixelFEDCard.NC_testreg, HAL::HAL_NO_VERIFY, offset);
vmeDevicePtr->write(FPGAName[2], pixelFEDCard.SC_testreg, HAL::HAL_NO_VERIFY, offset);
vmeDevicePtr->write(FPGAName[3], pixelFEDCard.S_testreg,  HAL::HAL_NO_VERIFY, offset);
#endif

#ifndef PILOT_FED
  // test BBB
  // if(pixelFEDCard.N_testreg>0){cout<<" skip BBB ROC "<<hex
  //                                   <<pixelFEDCard.N_testreg
  //                                   <<" for fed "<<dec<<pixelFEDCard.fedNumber
  //                                   <<endl;}
  // if(pixelFEDCard.NC_testreg>0){cout<<" skip BBB ROC "<<hex
  //                                    <<pixelFEDCard.NC_testreg
  //                                    <<" for fed "<<dec<<pixelFEDCard.fedNumber
  //                                   <<endl;}
  // if(pixelFEDCard.SC_testreg>0){cout<<" skip BBB ROC "<<hex
  //                                    <<pixelFEDCard.SC_testreg
  //                                    <<" for fed "<<dec<<pixelFEDCard.fedNumber
  //                                    <<endl;}
  // if(pixelFEDCard.S_testreg>0){cout<<" skip BBB ROC "<<hex
  //                                   <<pixelFEDCard.S_testreg
  //                                   <<" for fed "<<dec<<pixelFEDCard.fedNumber
  //                                   <<endl;}


uint32_t offset = 0x1a8000;
uint32_t data = (pixelFEDCard.N_testreg&0xfff);
 vmeDevicePtr->write(FPGAName[0],data,HAL::HAL_NO_VERIFY,offset);
data = (pixelFEDCard.NC_testreg&0xfff);
 vmeDevicePtr->write(FPGAName[1],data,HAL::HAL_NO_VERIFY,offset);
data = (pixelFEDCard.SC_testreg&0xfff);
 vmeDevicePtr->write(FPGAName[2],data,HAL::HAL_NO_VERIFY,offset);
data = (pixelFEDCard.S_testreg&0xfff);
 vmeDevicePtr->write(FPGAName[3],data,HAL::HAL_NO_VERIFY,offset);
#endif
}

////////////////////////////////////////////////////////////////////////
int PixelPh0FEDInterface::set_ROCskip(int chnl, int roc){
  assert(0);
////////////////////////////////////////////////////////////////////////
//These bits allow a ROC to be skipped (1/fpga) if the ROC puts out
//BBB instead of UlB B LD, this method checks the input and updates
//the FEDCard
//
if((chnl>36)|(chnl<1))
{cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Channel for BBB skip out of bounds "<<endl;
return -1;}
if((roc>=pixelFEDCard.NRocs[chnl-1])|(roc<1))
{cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Roc for BBB skip out of bounds "<<endl;
return -1;}

if((chnl>0)&(chnl<10)){
pixelFEDCard.N_testreg=(chnl<<5)+(roc-1);
set_ROCskip();
return 0;
 
} else if((chnl>9)&(chnl<19)){

pixelFEDCard.NC_testreg=(chnl<<5)+(roc-1);
set_ROCskip();
return 0;

} else if((chnl>18)&(chnl<28)){

pixelFEDCard.SC_testreg=(chnl<<5)+(roc-1);
set_ROCskip();
return 0;

} else if((chnl>27)&(chnl<37)){

  //pixelFEDCard.S_testreg=chnl<<5+(roc-1);
  pixelFEDCard.S_testreg=(chnl<<5)+(roc-1);
set_ROCskip();
return 0;

}



return -1;}
////////////////////////////////////////////////////////////////////////
//sets the L1A delay to FED 0=0,1=32,2=48,3=64 clocks
void PixelPh0FEDInterface::set_FEDTTCDelay(void)
{
uint32_t data =(pixelFEDCard.FedTTCDelay&0x3);
  vmeDevicePtr->write("TTCBigDelay", data );
}
////////////////////////////////////////////////////////////////////////
int PixelPh0FEDInterface::get_FEDTTCDelay(void)
{return((int)(pixelFEDCard.FedTTCDelay&0x3));}
////////////////////////////////////////////////////////////////////////
void PixelPh0FEDInterface::set_FeatureReg(void)
{
  if (Printlevel&2) cout<<"FEDID:"<<pixelFEDCard.fedNumber<<" Setting Feature Register 0x"<<hex<<pixelFEDCard.FeatureRegister<<dec<<endl;
vmeDevicePtr->write("LAD_C",(uint32_t)pixelFEDCard.FeatureRegister,HAL::HAL_NO_VERIFY,0x1e0000);	
}
///////////////////////////////////////////////////////////////////////				
int PixelPh0FEDInterface::get_FeatureReg(void)
{
return pixelFEDCard.FeatureRegister;	
}
///////////////////////////////////////////////////////////////////////				
int PixelPh0FEDInterface::FixBBB(int chan,uint32_t *data)
{
  assert(0);
 //bbb
//if((pixelFEDCard.fedNumber==38)&&chan==4){
//cout<<"BBB entered channel "<<dec<<((pixelFEDCard.N_testreg&0x7e0)>>5)<<" trans channel "<<chan<<hex<<" testreg 0x"<<pixelFEDCard.N_testreg<<dec<<endl;}

  const bool debug = false;

  // // test BBB
  // if(pixelFEDCard.N_testreg>0){cout<<" skip BBB ROC "<<hex
  // 				   <<pixelFEDCard.N_testreg
  // 				   <<" for fed "<<dec<<pixelFEDCard.fedNumber
  // 				   <<" " <<chan<<endl;}
  // if(pixelFEDCard.NC_testreg>0){cout<<" skip BBB ROC "<<hex
  // 				    <<pixelFEDCard.NC_testreg
  // 				    <<" for fed "<<dec<<pixelFEDCard.fedNumber
  // 				    <<" " <<chan<<endl;}
  // if(pixelFEDCard.SC_testreg>0){cout<<" skip BBB ROC "<<hex
  // 				    <<pixelFEDCard.SC_testreg
  // 				    <<" for fed "<<dec<<pixelFEDCard.fedNumber
  // 				    <<" " <<chan<<endl;}
  // if(pixelFEDCard.S_testreg>0){cout<<" skip BBB ROC "<<hex
  // 				   <<pixelFEDCard.S_testreg
  // 				   <<" for fed "<<dec<<pixelFEDCard.fedNumber
  // 				   <<" " <<chan<<endl;}


  if(debug) cout<<"BBB entered channel "<<chan<<dec<<((pixelFEDCard.N_testreg&0x7e0)>>5)<<" trans channel "<<chan<<hex<<" testreg 0x"<<pixelFEDCard.N_testreg<<dec<<endl;


  int iroc=0;
  if(chan<10){if(chan!=((pixelFEDCard.N_testreg&0x7e0)>>5)){ return -1;}else{iroc=(pixelFEDCard.N_testreg&0x1f);}}
  if(chan>9&&chan<19){if(chan!=((pixelFEDCard.NC_testreg&0x7e0)>>5)){ return -1;}else{iroc=(pixelFEDCard.NC_testreg&0x1f);}}
  if(chan>18&&chan<28){if(chan!=((pixelFEDCard.SC_testreg&0x7e0)>>5)){ return -1;}else{iroc=(pixelFEDCard.SC_testreg&0x1f);}}
  if(chan>27){if(chan!=((pixelFEDCard.S_testreg&0x7e0)>>5)){ return -1;}else{iroc=(pixelFEDCard.SC_testreg&0x1f);}}
  

 if(debug) cout<<" attempting ub insertion for channel "<<chan<<" roc "
	       <<iroc<< " "<<  pixelFEDCard.Ublack[chan-1] <<endl;

 int ubpos[32];int nubs=0;int ublvls[32];
 for(int ij=0;ij<100;ij++) { //header
   //cout<<data[ij]<<" "<< ((data[ij]&0xffc00000)>>22) <<endl;
   if( (int((data[ij]  &0xffc00000)>>22)<pixelFEDCard.Ublack[chan-1])&&
       (int((data[ij+1]&0xffc00000)>>22)<pixelFEDCard.Ublack[chan-1])&&
       (int((data[ij+2]&0xffc00000)>>22)<pixelFEDCard.Ublack[chan-1])&&
       (nubs==0) ) {  //found UB
     nubs=3;ubpos[0]=ij;ubpos[1]=ij+1;ubpos[2]=ij+2;
     ublvls[0]=((data[ij]   & 0xffc00000)>>22);
     ublvls[1]=((data[ij+1] & 0xffc00000)>>22);
     ublvls[2]=((data[ij+2] & 0xffc00000)>>22);

     if(debug) cout<<" found "<<ublvls[0]<<" "<<ublvls[1]<<" "<<ublvls[2]<<endl;
     break;
   }
   //cout<<"if no ultrablack header...don't change the buffer "<<nubs<<endl;
   //return 0;
 }//header
 
 if(nubs==0) { // stop here if the UBUBUB was not found 
   cout<<"no ultrablack header found ...don't change the buffer, exit "
       <<nubs<<endl;
   return 0;
 }

 if(debug) cout<<" BBB header found at "<<ubpos[0]<<" level "<<endl;


for(int ij=ubpos[2]+6;ij<960;ij+=3){

  if(nubs>29) return 0;//Everything is an UB!

if(int((data[ij] & 0xffc00000)>>22)<pixelFEDCard.Ublack[chan-1]){
ubpos[nubs]=ij;ublvls[nubs]=((data[ij] & 0xffc00000)>>22);nubs++;
 }


if((nubs==(pixelFEDCard.NRocs[chan-1]+3))&&(int((data[ij+1] & 0xffc00000)>>22)<pixelFEDCard.Ublack[chan-1])){//confirms trailer

  //cout<<" nubs "<<nubs<<" iroc "<<iroc<<endl;
  //cout<<((data[ubpos[3+iroc]-3] & 0xffc00000)>>22)<<" "<<((data[ubpos[3+iroc]-2] & 0xffc00000)>>22)<<" "<<
  //((data[ubpos[3+iroc]-1] & 0xffc00000)>>22)<<" "<<((data[ubpos[3+iroc]] & 0xffc00000)>>22)<<" "<<endl;

if( (int((data[ubpos[3+iroc]-1] & 0xffc00000)>>22)<pixelFEDCard.BlackHi[chan-1])&&(int((data[ubpos[3+iroc]-1] & 0xffc00000)>>22)>pixelFEDCard.BlackLo[chan-1])&&
(int((data[ubpos[3+iroc]-2] & 0xffc00000)>>22)<pixelFEDCard.BlackHi[chan-1])&&(int((data[ubpos[3+iroc]-2] & 0xffc00000)>>22)>pixelFEDCard.BlackLo[chan-1])&&
(int((data[ubpos[3+iroc]-3] & 0xffc00000)>>22)<pixelFEDCard.BlackHi[chan-1])&&(int((data[ubpos[3+iroc]-3] & 0xffc00000)>>22)>pixelFEDCard.BlackLo[chan-1]) )

{data[ubpos[3+iroc]-3]=data[ubpos[2]];

//cout<<"confirmed bbb "<<data[ubpos[3+iroc]-3]<<endl; //set to same level as tast header ub
}

 }
}

return 0;

}

void PixelPh0FEDInterface::setXY( int X, int Y) {
  /*
  Configure the X in Y mechanism by setting X and Y.

  This mechanism increments a counter if a FED raises X out-of-syncs in Y triggers.
  See also the methods getXYCount and resetXYCount.
  */
  uint32_t numOOS=X;
  uint32_t numEvents=Y;
  uint32_t XY=(numOOS&0xff) + ((numEvents&0xffffff)<<8); //LAD_C+0x080000 bits[7...0] number of OOS - found in bits[31...8]triggers
  vmeDevicePtr->write("LAD_C",XY,HAL::HAL_NO_VERIFY,0x080000);
  return;
}

int PixelPh0FEDInterface::getXYCount() {
  /*
  Get the value of the counter that tracks how many times the FED had X out-of-syncs
  in Y triggers. X and Y are set by the setXY method.
  */
  uint32_t output;
  vmeDevicePtr->read("LAD_C",&output,0x090000);
  return (int)output;
}

void PixelPh0FEDInterface::resetXYCount() {
  /*
  Reset the counter that is read out by getXYCount method to 0
  */
  uint32_t zero=0;
  vmeDevicePtr->write("LAD_C",zero,HAL::HAL_NO_VERIFY,0x090000);
  return;
}

int PixelPh0FEDInterface::getNumFakeEvents() {
  /*
  Read out the counter which keeps track of how many fake events the FED has sent.
  */
  uint32_t output=0;
  vmeDevicePtr->read("LAD_C",&output,0x098000);
  return (int)output;
}

void PixelPh0FEDInterface::resetNumFakeEvents() {
  /*
  Reset the counter which keeps track of how many fake events the FED has sent
  */
  uint32_t zero=0;
  vmeDevicePtr->write("LAD_C",zero,HAL::HAL_NO_VERIFY,0x098000);
  return;
}

bool PixelPh0FEDInterface::checkFEDChannelSEU() {
  /*
  Check to see if the channels that are currently on match what we expect. If not
  increment the counter and return true. Note that this assumes that the method won't
  be called again until the SEU is fixed. Otherwise, the counter will be incremented multiple
  times for the same SEU.
  */
  bool foundSEU = false;
  uint32_t N_enbable_current_i = 0, NC_enbable_current_i=0, SC_enbable_current_i=0, S_enbable_current_i;
  vmeDevicePtr->read("NWrRdCntrReg", &N_enbable_current_i);
  vmeDevicePtr->read("NCWrRdCntrReg", &NC_enbable_current_i);
  vmeDevicePtr->read("SCWrRdCntrReg", &SC_enbable_current_i);
  vmeDevicePtr->read("SWrRdCntrReg", &S_enbable_current_i);
  bitset<9> N_enbable_current(N_enbable_current_i);
  bitset<9> NC_enbable_current(NC_enbable_current_i);
  bitset<9> SC_enbable_current(SC_enbable_current_i);
  bitset<9> S_enbable_current(S_enbable_current_i);

  // Note: since N_enbable_expected is bitset<9>, this only compares the first 9 bits
  if ((N_enbable_current != N_enbable_expected) && (N_enbable_current != N_enbable_last)) {
    foundSEU = true;
    cout << "Detected FEDChannel SEU in FED " << pixelFEDCard.fedNumber << " North" << endl;
    cout << "Expected " << N_enbable_expected << " Found " << N_enbable_current << " Last " << N_enbable_last << endl;
    incrementSEUCountersFromEnbableBits(N_num_SEU, N_enbable_current, N_enbable_last);
  }
  if ((NC_enbable_current != NC_enbable_expected) && (NC_enbable_current != NC_enbable_last)) {
    foundSEU = true;
    cout << "Detected FEDChannel SEU in FED " << pixelFEDCard.fedNumber << " North Central" << endl;
    cout << "Expected " << NC_enbable_expected << " Found " << NC_enbable_current << " Last " << NC_enbable_last << endl;
    incrementSEUCountersFromEnbableBits(NC_num_SEU, NC_enbable_current, NC_enbable_last);
  }
  if ((SC_enbable_current != SC_enbable_expected) && (SC_enbable_current != SC_enbable_last)) {
    foundSEU = true;
    cout << "Detected FEDChannel SEU in FED " << pixelFEDCard.fedNumber << " South Central" << endl;
    cout << "Expected " << SC_enbable_expected << " Found " << SC_enbable_current << " Last " << SC_enbable_last << endl;
    incrementSEUCountersFromEnbableBits(SC_num_SEU, SC_enbable_current, SC_enbable_last);
  }
  if ((S_enbable_current != S_enbable_expected) && (S_enbable_current != S_enbable_last)) {
    foundSEU = true;
    cout << "Detected FEDChannel SEU in FED " << pixelFEDCard.fedNumber << " South" << endl;
    cout << "Expected " << S_enbable_expected << " Found " << S_enbable_current << " Last " << S_enbable_last << endl;
    incrementSEUCountersFromEnbableBits(S_num_SEU, S_enbable_current, S_enbable_last);
  }


  N_enbable_last = N_enbable_current;
  NC_enbable_last = NC_enbable_current;
  SC_enbable_last = SC_enbable_current;
  S_enbable_last = S_enbable_current;

  return foundSEU;
}

void PixelPh0FEDInterface::incrementSEUCountersFromEnbableBits(vector<int> &counter, bitset<9> current, bitset<9> last) {
  for(size_t i = 0; i < current.size(); i++) {
    if (current[i] != last[i]) {
      counter[i]++;
    }
  }
}

bool PixelPh0FEDInterface::checkSEUCounters(int threshold) {
  /*
  Check to see if any of the channels have more than threshold SEUs.
  If so, return true and set expected enbable bit for that channel
  to off.
  Otherwise, return false
  */
  bool return_val = false;
  cout << "Checking for more than " << threshold << " SEUs in FED " << pixelFEDCard.fedNumber << endl;
  cout << "Channels with too many SEUs: ";
  int channel_base = 1;
  for (size_t i=0; i<9; i++)
  {
    channel_base = 1;
    if (N_num_SEU[i] >= threshold) {
      N_enbable_expected[i] = 1; // 1 is off
      cout << " " << channel_base+i << "(" << N_num_SEU[i] << ")";
      return_val = true;
    }
    channel_base += 9;
    if (NC_num_SEU[i] >= threshold) {
      NC_enbable_expected[i] = 1; // 1 is off
      cout << " " << channel_base+i << "(" << NC_num_SEU[i] << ")";
      return_val = true;
    }
    channel_base += 9;
    if (SC_num_SEU[i] >= threshold) {
      SC_enbable_expected[i] = 1; // 1 is off
      cout << " " << channel_base+i << "(" << SC_num_SEU[i] << ")";
      return_val = true;
    }
    channel_base += 9;
    if (S_num_SEU[i] >= threshold) {
      S_enbable_expected[i] = 1; // 1 is off
      cout << " " << channel_base+i << "(" << S_num_SEU[i] << ")";
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

void PixelPh0FEDInterface::resetEnbableBits() {
  // Get the current values of higher bits in these registers, so we can leave them alone
  // This is also the time when the runDegraded flag gets set if appropriate
  uint32_t nOtherConfigBits = 0;
  uint32_t ncOtherConfigBits = 0;
  uint32_t sOtherConfigBits = 0;
  uint32_t scOtherConfigBits = 0;
  uint32_t otherBitsMask = 0xFFFFFE00;
  vmeDevicePtr->read("SWrRdCntrReg", &sOtherConfigBits);
  sOtherConfigBits &= otherBitsMask;
  vmeDevicePtr->read("SCWrRdCntrReg", &scOtherConfigBits);
  scOtherConfigBits &= otherBitsMask;
  vmeDevicePtr->read("NWrRdCntrReg", &nOtherConfigBits);
  nOtherConfigBits &= otherBitsMask;
  vmeDevicePtr->read("NCWrRdCntrReg", &ncOtherConfigBits);
  ncOtherConfigBits &= otherBitsMask;
  
  uint32_t N_write = (nOtherConfigBits | (uint32_t)N_enbable_expected.to_ulong());
  uint32_t NC_write = (ncOtherConfigBits | (uint32_t)NC_enbable_expected.to_ulong());
  uint32_t SC_write = (scOtherConfigBits | (uint32_t)SC_enbable_expected.to_ulong());
  uint32_t S_write = (sOtherConfigBits | (uint32_t)S_enbable_expected.to_ulong());
  
  // Set channels on/off as they were originally configured
  vmeDevicePtr->write("SWrRdCntrReg", S_write);
  vmeDevicePtr->write("SCWrRdCntrReg", SC_write);
  vmeDevicePtr->write("NWrRdCntrReg", N_write);
  vmeDevicePtr->write("NCWrRdCntrReg", NC_write);
}

void PixelPh0FEDInterface::storeEnbableBits() {
  // Save state of first 9 enbable bits
  N_enbable_expected = pixelFEDCard.Ncntrl;
  NC_enbable_expected = pixelFEDCard.NCcntrl;
  SC_enbable_expected = pixelFEDCard.SCcntrl;
  S_enbable_expected = pixelFEDCard.Scntrl;
  N_enbable_last = N_enbable_expected;
  NC_enbable_last = N_enbable_expected;
  SC_enbable_last = SC_enbable_expected;
  S_enbable_last = S_enbable_expected;
}

void PixelPh0FEDInterface::sendResets() {
  const uint32_t data = 0x80000000;
  vmeDevicePtr->write("LRES",data);
  usleep(10);
  vmeDevicePtr->write("CLRES",data);
  usleep(10);
}

uint32_t PixelPh0FEDInterface::testReg(uint32_t data) {
  uint32_t ret = 0;
  uint32_t d;
  const char* regs[5] = {
    "TestReg",
    "NWrRdTestReg", 
    "NCWrRdTestReg", 
    "SCWrRdTestReg", 
    "SWrRdTestReg"
  };
  for (int i = 0; i < 5; ++i) {
    vmeDevicePtr->write(regs[i], data);
    usleep(10000);
    vmeDevicePtr->read(regs[i], &d);
    usleep(10000);
    ret |= int(data == d) << i;
  }
  return ret;
}

void PixelPh0FEDInterface::resetSEUCountAndDegradeState(void) {
  cout << "reset SEU counters and the runDegrade flag " << endl;
  // reset the state back to running 
  runDegraded_ = false;
  // clear the count flag
  for (size_t i=0; i<9; i++) {
    N_num_SEU[i]  = 0;
    NC_num_SEU[i] = 0;
    SC_num_SEU[i] = 0;
    S_num_SEU[i]  = 0;
  }
  // reset the expected state to default
  storeEnbableBits();

  return;
}

void PixelPh0FEDInterface::resetFED(void) {
  // maine reset
  vmeDevicePtr->write("LRES",0x80000000);
  vmeDevicePtr->write("CLRES",0x80000000);
  
  //reset fake event counter
  uint32_t resword=(1<<23);
  vmeDevicePtr->write("LAD_C",resword,HAL::HAL_NO_VERIFY,0x1c8000);
  //reset center OOS counter
  resword=(1<<15);
  vmeDevicePtr->write("LAD_C",resword,HAL::HAL_NO_VERIFY,0x1c8000);
  
  // reset the error-fifo
  vmeDevicePtr->write("NWrResetPls", 0x80000000 );
  vmeDevicePtr->write("NCWrResetPls",0x80000000 );
  vmeDevicePtr->write("SCWrResetPls",0x80000000 );
  vmeDevicePtr->write("SWrResetPls", 0x80000000 );

}
