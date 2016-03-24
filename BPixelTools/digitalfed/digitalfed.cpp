#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <bitset>
#include <vector>
#include <string>

#include "CAENVMElib.h"  // CAEN library prototypes
#include "VMELock.h"
using namespace std;

#include "SimpleCommand.h"
#include "MultiplexingServer.h"

#include "TBMdecode.cc"

//-----------------------------------------

//#define FEDBASE  0x3C000000
#define FEDBASE  0x4C000000

#define LAD_N  		(FEDBASE)
#define LAD_NC  	(FEDBASE+0x200000)
#define LAD_SC  	(FEDBASE+0x400000)
#define LAD_S  	        (FEDBASE+0x600000)
#define LAD_C   	(FEDBASE+0x800000)
#define BRDCST_FR       (FEDBASE+0xe00000) 
#define READ_MA         (FEDBASE+0xa00000)  
#define READ_GA         (FEDBASE+0xa00004)
#define I2C_RD_STAT     (FEDBASE+0xa00014)
#define I2C_RES  	(FEDBASE+0xa00008)
#define I2C_LOAD  	(FEDBASE+0xa0000c)
#define I2C_ADDR_RW     (FEDBASE+0xa00010)
#define I2C_RD_DATA     (FEDBASE+0xa00010)
#define I2C_RD_STAT     (FEDBASE+0xa00014)

#define WR_VME_TestReg  (FEDBASE+0xa08000) 
#define RD_VME_TestReg  (FEDBASE+0xa0000c) 


#define RES_TTCrx  	(FEDBASE+0xa00038)

#define LRES	  	(FEDBASE+0xa00000)
#define CLRES	  	(FEDBASE+0xa00004)
#define nCONFIG  	(FEDBASE+0xa0003c)

//-----------------------------------------

#define TTCVIBASE       0xFFFF00

#define TTCVICSR1	(TTCVIBASE+0x80)
#define TTCVICSR2	(TTCVIBASE+0x82)
#define TTCVI_sBRD	(TTCVIBASE+0xc4)
#define TTCVI_BG0_m	(TTCVIBASE+0x90)
#define TTCVI_BG0_g	(TTCVIBASE+0x96)
#define TTCVI_BG0_d	(TTCVIBASE+0xb0)
#define TTCVI_LF_h	(TTCVIBASE+0xc0)
#define TTCVI_LF_l	(TTCVIBASE+0xc2)
#define TTCVI_IN_dl	(TTCVIBASE+0x92)
#define TTCVI_IN_du	(TTCVIBASE+0x94)


 //------------------------------------------
 
#define CHa_CHb_mux	0x80000000  	//  CHa_CHb_mux=0x80000000 for use with new TBM  
                                        //  CHa_CHb_mux=0x00000000 for use with old TBM  

//------------------------------------------
  
#define write_FITELmemaddr  		0x3100
#define write_FITELdata     		0x3200 
#define FITELreadI2C        		0x2000 
#define FITELwriteI2C       		0x3000 
#define write_selectLEDs    		0x3300 
#define enable_trp_select   		0x2300 
#define write_resetFITEL	  		0x3400
#define write_select_fibers_1to6  0x3700
//------------------------------------------
 
  

CVBoardTypes  VMEBoard = cvV2718;  // define interface type
short         Link = 0;  // define device & link
short         Device = 0;
//    long          BHandle; // pointer to the device
int32_t          BHandle; // pointer to the device
CVErrorCodes  ret; // return code

bool ttcconfigured = true; // JMT must be set up using ttcci_standalone
bool firstenable = true;

unsigned long int CHIP[4],CH_SubAddr[9],CH_SubAddr_Sim[9],CH_Color[9],CLOCKsel[4],FIFO_I_ReadAddr;
int TestData[256],chip,channel,roc,gl_ch_ped[37];
unsigned int ttcaddcsr0=0x180000+0x60;//Base is slot 2 shifted over, 0x60 is BGO register

unsigned long buffer1[128],buffer2[128];
int decodePTrans(unsigned long * data1, unsigned long * data2, const int length);
int decodePTrans2(unsigned long * data1, unsigned long * data2, const int length);
void decode(unsigned long * data1, unsigned long * data2, const int length);
void decode_withROCs(unsigned long * data1, unsigned long * data2, const int length);
int readlastTTS();

int a[4]={0,0,0,0};

int a2[4]={0,0,0,0};


int fedSlot=0;
int fiber=12;
int select_fibers = 1;
bool piggyN=true;
bool isFitel=false;
bool TBMA=true;

int DecodeError(int word2decode);

void VMEout(int AddressType, unsigned long int Address, int DataSize, unsigned long int Data);
void VMEin(int AddressType, unsigned long int Address, int DataSize, unsigned long int *Data);
void VMEmove(int SRCpar, unsigned long int SRCaddr, int DSTpar, unsigned long int *DSTaddr,int length,int DataWidth);

int TTCRX_I2C_REG_READ( int Register_Nr);
int TTCRX_I2C_REG_WRITE( int Register_Nr, int Value);
void BinDisplay(unsigned long int In_Word, int Bits,int Header,int del);
std::string BinToString(unsigned long int In_Word, int Bits);

int CLOCK_DEL_V2( int CHIPnr, int CHANNELnr, int delay);

void analyzeError(CVErrorCodes ret) {
  switch (ret) {
  case cvGenericError   : cout<<" Generic error !!!"<<endl;
    break ;
  case cvBusError : cout<<" Bus Error !!!"<<endl;
    break ;
  case cvCommError : cout<<" Communication Error !!!"<<endl;
    break ;
  case cvInvalidParam : cout<<" Invalid Param Error !!!"<<endl;
    break ;
  default          : cout<<" Unknown Error !!!"<<endl;
    break ;
  }
}

ofstream file("output.txt");

// ======================================================================
void Reset() {
  VMEout(0x3,LRES,4,0x80000000);  
  VMEout(0x3,CLRES,4,0x80000000);

}

void ReadFirmwareVersion() {

  unsigned long int d,dd,du;
  VMEin(0x3,FEDBASE+0xa0003c,4,&d);	
  printf("FirmwVers  V: %02d.%02d.%02d%02d\n\n",int((d&0xff000000)>>24),int((d&0xff0000)>>16),int((d&0xff00)>>8),int(d&0xff) );
  VMEin(0x3,LAD_N+0x1f0000,4,&d);	
  printf("FirmwVers  N: %02d.%02d.%02d%02d\n",int((d&0xff000000)>>24),int((d&0xff0000)>>16),int((d&0xff00)>>8),int(d&0xff) );
  VMEin(0x3,LAD_NC+0x1f0000,4,&d);	
  printf("FirmwVers NC: %02d.%02d.%02d%02d\n",int((d&0xff000000)>>24),int((d&0xff0000)>>16),int((d&0xff00)>>8),int(d&0xff) );
  VMEin(0x3,LAD_SC+0x1f0000,4,&d);	
  printf("FirmwVers SC: %02d.%02d.%02d%02d\n",int((d&0xff000000)>>24),int((d&0xff0000)>>16),int((d&0xff00)>>8),int(d&0xff) );
  VMEin(0x3,LAD_S+0x1f0000,4,&d);	
  printf("FirmwVers  S: %02d.%02d.%02d%02d\n\n",int((d&0xff000000)>>24),int((d&0xff0000)>>16),int((d&0xff00)>>8),int(d&0xff) );
  VMEin(0x3,LAD_C+0x1f0000,4,&d);	
  printf("FirmwVers  C: %02d.%02d.%02d%02d\n\n",int((d&0xff000000)>>24),int((d&0xff0000)>>16),int((d&0xff00)>>8),int(d&0xff) );
  VMEin(0x3,LAD_N+0x158000,4,&du); VMEin(0x3,LAD_N+0x178000,4,&dd);
  printf("FirmwVers  N Piggy: %1x%1x%1x%1x%1x%1x\n",int((dd>>20)&0xf),int((dd>>12)&0xf),int((dd>>4)&0xf),int((du>>20)&0xf),int((du>>12)&0xf),int((du>>4)&0xf) );
  VMEin(0x3,LAD_S+0x158000,4,&du); VMEin(0x3,LAD_S+0x178000,4,&dd);
  printf("FirmwVers  S Piggy: %1x%1x%1x%1x%1x%1x\n",int((dd>>20)&0xf),int((dd>>12)&0xf),int((dd>>4)&0xf),int((du>>20)&0xf),int((du>>12)&0xf),int((du>>4)&0xf) );

  return;
}

void SetupTTC() {

  int caenret=0;
  
  //// TTC Settings  /////
  VMEout(0x3,RES_TTCrx,4,0x0);     
  usleep(1000); 
  VMEout (0x3,I2C_RES, 2, 0x2);   //  RESET I2C STATE MACHINE
  usleep(10);
  VMEout (0x3,I2C_RES, 2, 0x0);
  usleep(100);

  caenret = CAENVME_SetInputConf(0,cvInput0,cvDirect,cvActiveHigh);
  caenret = CAENVME_SetScalerConf(0,1023,1,cvInputSrc0,cvManualSW,cvManualSW);
  caenret = CAENVME_EnableScalerGate(0);

  TTCRX_I2C_REG_WRITE( 2, 0x0); //COARSE DELAY REG
  TTCRX_I2C_REG_WRITE( 3, 0x13);    // ControlReg  only ClockDes1 enabled  
  //TTCRX_I2C_REG_WRITE( 1, k[105]); // Fine Delay ClockDes2  ~ phasengleich mit ClockDes1
  TTCRX_I2C_REG_WRITE( 1, 5);
  TTCRX_I2C_REG_WRITE( 0, 14); // To make it similar to FEDInterface

  return;
}

void PLLResetFrontFPGA() {
  //PLLreset front FPGA
  VMEout(0x3,LAD_N  +0x1c8000,4,0x20000000);   //PLLareset FED N  	
  VMEout(0x3,LAD_NC +0x1c8000,4,0x20000000);   //PLLareset FED NC  
  VMEout(0x3,LAD_SC +0x1c8000,4,0x20000000);   //PLLareset FED SC  
  VMEout(0x3,LAD_S  +0x1c8000,4,0x20000000);   //PLLareset FED S  
  usleep(100);
  cout << "PLL reset front FPGA sent----"  << endl;
  return;
}

void PLLResetPiggy() {
  //PLLreset front Piggy
  VMEout (0x3,LAD_S+0x1c0000, 4, 0x1);   //  PLLareset  PIGGY bottom   
  usleep(10);
  VMEout (0x3,LAD_S+0x1c0000, 4, 0x0); 
  
  VMEout (0x3,LAD_NC+0x1c0000, 4, 0x1);   //  PLLareset PIGGY top 
  usleep(10);
  VMEout (0x3,LAD_NC+0x1c0000, 4, 0x0); 
  cout << "PLL reset Piggy sent----"  << endl;
  return;
}

void ConfigureAlteraFrontDevices() {

  VMEout(0x3,nCONFIG,4,0x1);
  usleep(10);
  VMEout(0x3,nCONFIG,4,0x0);
  usleep(100000); //700ms reconfigure time !!
  cout << ".... ALTERA FRONT  devices reconfigured !" << endl; 

  return;

}

void ConfigureAlteraCenterDevice() {

  VMEout(0x3,nCONFIG,4,0x2);
  usleep(10);
  VMEout(0x3,nCONFIG,4,0x0);
  usleep(100000); //700ms reconfigure time !!
  cout << ".... ALTERA CENTER  devices reconfigured !" << endl; 
  return;
}

void ConfigurePiggy() {
  VMEout (0x3,LAD_S+0x1b8000, 4, 0x10);   //  nConfig PIGGY bottom
  usleep(10000);
  VMEout (0x3,LAD_S+0x1b8000, 4, 0x00); 
  
  VMEout (0x3,LAD_N+0x1b8000, 4, 0x80);   //  nConfig PIGGY top
  usleep(10000);
  VMEout (0x3,LAD_N+0x1b8000, 4, 0x00); 
  usleep(10000);    
  VMEout (0x3,LAD_N+0x190000, 4, 0x7f00);   //  nConfig PIGGY bottom
  cout << "\n..... PIGGY reconfigured ! \n\n" << endl;
 
  return;
}

void RegResetPiggy() {

  VMEout (0x3,LAD_S+0x1c0000, 4, 0x2);   //  REGreset PIGGY bottom   
  usleep(10);
  VMEout (0x3,LAD_S+0x1c0000, 4, 0x0); 
  
  VMEout (0x3,LAD_NC+0x1c0000, 4, 0x2);   //  REGreset PIGGY top   
  usleep(10);
  VMEout (0x3,LAD_NC+0x1c0000, 4, 0x0);

}

void RegReset() {
 
  RegResetPiggy();
       
  ///////init FED //////////////
  
  VMEout(0x3,LRES,4,0x80000000);  
  VMEout(0x3,CLRES,4,0x80000000); 
  
  VMEout(0x3,LAD_C +0x1c8000,4,0x20000000);   //Reset SLink 80MHz PLL
       
  for(int j=0;j<4;j++) {  for(int i=1;i<10;i++)  CLOCK_DEL_V2( j,i,0x4); } 
  usleep(10);
  
  VMEout( 0x3,LAD_N+0x1a0000,4,0x1ff); //Skip Channel Readout N
  VMEout(0x3,LAD_NC+0x1a0000,4,0x1ff); //Skip Channel Readout NC
  VMEout(0x3,LAD_SC+0x1a0000,4,0x1ff); //Skip Channel Readout SC  0x1fe
  VMEout( 0x3,LAD_S+0x1a0000,4,0x1ff);   //Skip Channel Readout S   0x19f
  
  VMEout(0x3,LAD_N +0x1c8000,4,0x80000000); // Flush SPY FIFOs N
  VMEout(0x3,LAD_NC+0x1c8000,4,0x80000000); // Flush SPY FIFOs NC
  VMEout(0x3,LAD_SC+0x1c8000,4,0x80000000); // Flush SPY FIFOs SC
  VMEout(0x3,LAD_S +0x1c8000,4,0x80000000); // Flush SPY FIFOs S
  
  //VXIout(0x/break
  //3,LAD_C+0x1c0000,4,0xa);  //SLink disable ;Disable SpyMemWrite during readout       
  VMEout(0x3,LAD_C+0x1a0000,4,0x10014); // 0-TTCEn-0-EnableDAC-0-0			enable TTC Trigger  
  cout << "FED reset and TTC enabled " << endl;
 
  return;
}

void SendTrigger(int ntr=10) {
  unsigned long int data;
 
  
  VMEout(0x3,LAD_C+0x1a0000,4,0x14); // 0-TTCEn-0-EnableDAC-0-0			TTC Trigger  
  
  for (int i=0; i <= ntr ; i++){
    data=0xb ; // send single L1A
    //data=0xc ;                      // send CalSync + L1A
    usleep(100);
    ret = CAENVME_WriteCycle(0,ttcaddcsr0,&data,cvA24_U_DATA,cvD32);
	 
    //you can call this now when you need a trigger, it should not be called more often than 1/LHC orbit ~90us!
    if(ret != cvSuccess) { 
      cout << "Error in TTCci trigger!" << endl;
      analyzeError(ret); 
    }
    cout << "L1A " << i << endl;
    //sleep(2);
  }

  return;
}

void ReadDaughterCardStatus() {

  unsigned long int d;
  // This is for ZARLINK version
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux+0x2546);   //  NORTH PIGGYcom   
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux+0x2343);   //  SOUTH PIGGYcom   
  //  a_b_mux=0x80000000 for use with old pattern generator DTB  
  //  a_b_mux=0x00000000 for use with new module   
       
  //  bit 13  ... start PIGGYcom
  //  bit 12  ... mode bit
  //  bit 11..8   addr bits
  //  bit  7..1   data bits
  
  printf("\n\n\nPIGGYstatus NORTHup     CH#1 / 2     CH#3 / 4     CH#5 / 6   locked400 \n\n");
  for(int i=0;i<16;i++)  {
    VMEin(0x3,LAD_N+0x158000,4,&d);
    printf("                               %2x         %2x          %2x         %1d\n", int((d)&0xff), int((d>>8)&0xff), int((d>>16)&0xff), int((d>>24)&0x1));
  }
  
  printf("\n\n\nPIGGYstatus NORTHdown     CH#7 / 8     CH#9 /10     CH#11/12   locked400 \n\n");
  for(int i=0;i<16;i++)  {
    //usleep(1000000);
    VMEin(0x3,LAD_N+0x178000,4,&d);
    printf("                               %2x         %2x          %2x         %1d\n", int((d)&0xff), int((d>>8)&0xff), int((d>>16)&0xff), int((d>>24)&0x1));
  }
  
  printf("\n\n\nPIGGYstatus SOUTHup     CH#25/26     CH#27/28     CH#29/30     locked400 \n\n");
  for(int i=0;i<16;i++)  {
    VMEin(0x3,LAD_S+0x158000,4,&d);
    printf("                               %2x         %2x          %2x         %1d\n", int((d)&0xff), int((d>>8)&0xff), int((d>>16)&0xff), int((d>>24)&0x1));
  }
  
  printf("\n\n\nPIGGYstatus SOUTHdown   CH#31/32     CH#33/34     CH#35/36     locked400 \n\n");
  for(int i=0;i<16;i++)  {
    VMEin(0x3,LAD_S+0x178000,4,&d);
    printf("                               %2x         %2x          %2x         %1d\n", int((d)&0xff), int((d>>8)&0xff), int((d>>16)&0xff), int((d>>24)&0x1));
  }

  return;
  
}

void RunCasea() {
  //*****************commented to avoid conflict LC
  unsigned long int d, data, s;
  int caenret=0;
  unsigned int caendat=1;unsigned int caendat1=1;
  unsigned long int D_up[1024],D_down[1024];
  unsigned long int ErrorFIFO_down[256],Old_Data;
  unsigned long int destaddr[1024];
  
  // THIS IS REGRESET for SPYING
  RegResetPiggy();
  //////
  
  //Count number of times locked   
  int count(0);
  for(int i=0;i<10;i++)  {
    
    //VMEin(0x3,LAD_S+0x178000,4,&d);
    VMEin(0x3,LAD_N+0x178000,4,&d);
    //printf("                               %2x         %2x          %2x         %1d\n", int((d)&0xff), int((d>>8)&0xff), int((d>>16)&0xff), int((d>>24)&0x1));
    count+=(d>>24)&0x1;
  }       
  cout << "# of times locked = "  << count << endl;
       
  Reset(); 
       
  count=0;
  for(int i=0;i<10;i++)  {
	 
    //VMEin(0x3,LAD_S+0x178000,4,&d);
    VMEin(0x3,LAD_N+0x178000,4,&d);
    //printf("                               %2x         %2x          %2x         %1d\n", int((d)&0xff), int((d>>8)&0xff), int((d>>16)&0xff), int((d>>24)&0x1));
    count+=(d>>24)&0x1;
  }
  cout << "after the FED reset # of times locked = "  << count << endl;

       
  //CHANNEL ENABLE --------------
  
  VMEout( 0x3,LAD_N+0x1a0000, 4,0x0ff); //TEMP FIFO
  VMEout( 0x3,LAD_NC+0x1a0000,4,0x1fe); //ch 9&10
  VMEout( 0x3,LAD_SC+0x1a0000,4,0x1ff);
  VMEout( 0x3,LAD_S+0x1a0000, 4,0x1ff);
      
       
  //Other settings
  VMEout( 0x3,LAD_C+0x1a0000,4,0x10014);   //set the gdmf control register
  VMEout(0x3,LAD_C+0x1d8000,4,0x0); // Trigger delay in the center chip
	
  count=0;

  // Turn on the channel to spy on  bits 8-11 in the mode register for the channel
  //also msk somr trailer error bits (<<24)
  //????????????????????????????????????
  VMEout(0x3,LAD_N+0x1c0000,4,((0x2)<<24)+(0x8<<8));  //North Mode register     
  VMEout(0x3,LAD_NC+0x1c0000,4,((0x2)<<24)+(0x0<<8));  //NorthCenter Mode register     
  ///////////////////////////////////////////////////////////
  //  bit  0  1  2  3  4  5  6  7  8   N 
  //  ch   1  2  3  4  5  6  7  8  9  NC
  //  ch   10 11 12 13 14 15 16 17 18
  //  ch   
  ///////////////////////////////////////////////////////////
  // VMEout(0x3,LAD_N+0x1c0000,4,0x800); 
  // For Up Piggy mode register bit [11..8] = 5 (000 for ch1, 001 for ch2, 002 for ch3, 003 for ch4, 004 for ch5, 005 for ch6, 006 for ch7, 007 for ch8, 008 for ch9) 
  // VMEout(0x3,LAD_N+0x1c0000,4,(0x800));
  // VMEout(0x3,LAD_NC+0x1c0000,4,(0x000)); 
  // For Up Piggy mode register bit [11..8] = 5 (000 for ch10, 100 for ch11, 200 for ch12, 300 for ch13, 400 for ch14, 500 for ch15, 600 for ch16, 700 for ch17, 800 for ch18)
  // VMEout(0x3,LAD_S+0x1c0000,4,0x600);
  // For Down Piggy  mode register bit [11..8] = 5 (0 for ch28, 1 for ch29, 2 for ch30, 3 for ch31, 4 for ch32, 5 for ch33, 6 for ch34, 7 for ch35, 8 for ch36)
  ///////////////////////////////////////////////////////////
  
  ///////////////////////////////////////////////////////////

       
  //channel selection for PIGGY       
  VMEout(0x3,LAD_N+0x1a8000,4,CHa_CHb_mux+0x2304); // ch selection for transparent mode: 5-fiber#12, 4-fiber#11, 3-fiber#10, 2-fiber#9, 1-fiber# 8, 0-fiber#7
  // VMEout(0x3,LAD_S+0x1a8000,4,CHa_CHb_mux+0x2304); // ch selection for transparent mode: 5-fiber#12, 4-fiber#11, 3-fiber#10, 2-fiber#9, 1-fiber# 8, 0-fiber#7


  for(int i=0;i<10;i++)  {
    
    //VMEin(0x3,LAD_S+0x178000,4,&d);
    VMEin(0x3,LAD_N+0x178000,4,&d);
    printf("                               %2x         %2x          %2x         %1d\n", int((d)&0xff), int((d>>8)&0xff), int((d>>16)&0xff), int((d>>24)&0x1));
    count+=(d>>24)&0x1;
  }
  cout << "after disabling the unused channels  # of times locked = "  << count << endl;
 
       
  caenret = CAENVME_ReadRegister(0,cvScaler1,&caendat);
       
  // values must match with TTCciConfiguration.txt
  //Sequence for BGO 12 = 0xc   Channel #12 (CAL+L1): Private Pixel Command, Send CAL + L1A
  //data=0x9 ;	// sequence for BG0 9 =0x9  START (starts cyclic trigger and cyclic BGO calsync)
  //data=0xc ;  // send CalSync + L1A
  data=0xb;  // bgo 0xb = 11 = single l1a
  //data = 0x9; // bgo 9 = start
  ret = CAENVME_WriteCycle(0,ttcaddcsr0,&data,cvA24_U_DATA,cvD32);
  
  //you can call this now when you need a trigger, it should not be called more often than 1/LHC orbit ~90us!
  if(ret != cvSuccess) { 
    cout<<"Error in write TTCci BGO Trigger"<<hex<<ret<<" "<<d<<endl;  
    printf("Error in TTCci trigger!\n");
    analyzeError(ret); 
  }
  
  printf("Trigger sent!\n");
  usleep(10000);
  caenret = CAENVME_ReadRegister(0,cvScaler1,&caendat1);
  //cout<<"scaler"<<caendat<<endl;
  //cout<<"scaler after trigger"<<caendat1<<endl;
       
  usleep(100);
  // s=256;
  //VMEmove(0x1003,LAD_S+0x178000,0,destaddr,s,4); printf("\n\n");
  
  //printf("\n\n|||||CH|RO|DC|PXL|PH|        Data:\n");
  //for(i=0;i<60;i++)
  //	 printf("%2d:  %2d %2d %2d %3d %2x %x\n",i,(destaddr[i]>>26)&0x3f,(destaddr[i]>>21)&0x1f,(destaddr[i]>>16)&0x1f,(destaddr[i]>>8)&0xff,destaddr[i]&0xff,destaddr[i]);
  
  //s=1024;
  //VMEmove(0x1003,LAD_NC+0x150000,0,destaddr,s,4); printf("\n\n");
  //for(i=0;i<20;i++) printf("%3x",destaddr[i]); printf("\n\n");   
  ///ProcessSystemEvents ();
  
  printf("\n\n\nTEMP FIFO Sup\n\n");
  for(int i=0;i<256;i++)  { 
    VMEin(0x3,LAD_S+0x148000,4,&d);	
    ///////VMEin(0x3,LAD_N+0x168000,4,&d);
    if(d) { 
      printf("%x\n",int(d)); 
      printf("CH#:%2d",int((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",int(((d>>1)&0xff00)+(d&0xff)));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",int(((d>>4)&0xff00)+(d&0xff)));    
    }
  }	
  
  printf("\n\n\nTEMP FIFO Sdown\n\n");
  for(int i=0;i<256;i++)  { 
    VMEin(0x3,LAD_S+0x168000,4,&d);	
    ///////VMEin(0x3,LAD_N+0x168000,4,&d);
    if(d) { 
      printf("%x\n",int(d)); 
      printf("CH#:%2d",int((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",int(((d>>1)&0xff00)+(d&0xff)));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",int(((d>>4)&0xff00)+(d&0xff)));    
    }
  }	
   
  printf("\n\n\nTEMP FIFO NCup\n\n");
  for(int i=0;i<256;i++)  { 
    VMEin(0x3,LAD_NC+0x148000,4,&d);	
    ///////VMEin(0x3,LAD_N+0x168000,4,&d);
    if(d) { 
      printf("check %x\n",int(d)); 
      printf("CH#:%2d",int((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",int(((d>>1)&0xff00)+(d&0xff)));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",int(((d>>4)&0xff00)+(d&0xff)));    
    }
  }	
   
  printf("\n\n\nTEMP FIFO NCdown\n\n");
  for(int i=0;i<256;i++)  { 
    VMEin(0x3,LAD_NC+0x168000,4,&d);	
    ///////VMEin(0x3,LAD_N+0x168000,4,&d);
    if(d) { 	 //now try reading spy fifo-2
      printf("%x\n",int(d)); 
      printf("CH#:%2d",int((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",int(((d>>1)&0xff00)+(d&0xff)));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",int(((d>>4)&0xff00)+(d&0xff)));    
    }     
  }
  
  printf("\n\n\nTEMP FIFO Nup\n\n");
  for(int i=0;i<256;i++)  { 
    VMEin(0x3,LAD_N+0x148000,4,&d);	
    ///////VMEin(0x3,LAD_N+0x168000,4,&d);
    if(d) { 
      printf("%x\n",int(d)); 
      printf("CH#:%2d",int((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",int(((d>>1)&0xff00)+(d&0xff)));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",int(((d>>4)&0xff00)+(d&0xff)));    
    }
  }	
   
  printf("\n\n\nTEMP FIFO Ndown\n\n");
  for(int i=0;i<256;i++)  { 
    VMEin(0x3,LAD_N+0x168000,4,&d);	
    ///////VMEin(0x3,LAD_N+0x168000,4,&d);
    if(d) { 	 //now try reading spy fifo-2
      printf("%x\n",int(d)); 
      printf("CH#:%2d",int((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",int(((d>>1)&0xff00)+(d&0xff)));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",int(((d>>4)&0xff00)+(d&0xff)));    
    }     
  }	
  
       
  printf("\n\n\nErrorFIFO SOUTHdown\n\n");
  for(int i=0;i<256;i++) VMEin(0x3,LAD_S+0x160000,4,&ErrorFIFO_down[i]);
  //for(int i=0;i<256;i++) VMEin(0x3,LAD_N+0x160000,4,&ErrorFIFO_down[i]);
       
  Old_Data=0;int ii=0;
  while ( (Old_Data!=ErrorFIFO_down[ii]) && (ii<255) )
    {
      printf("%3d.  CH#%2d  CODE#%2d  TTC_EventNr:%2x  E_Data[hex]=%2x\n",
	     ii,int((ErrorFIFO_down[ii]>>26)&0x3f),int((ErrorFIFO_down[ii]>>21)&0x1f),int((ErrorFIFO_down[ii]>>13)&0xff),int((ErrorFIFO_down[ii])&0x3ff));
      Old_Data=ErrorFIFO_down[ii];
      ii++;	 //now try reading spy fifo-2
    }
  //printf("\nPhase:%1x\n\n",l&0x3);		
       
  VMEout(0x3,LAD_C+0x1c0000,4,0x0);  //SLink disable ;Disable SpyMemWrite during readout     
  //VMEout(0x3,LAD_C+0x1c0000,4,0x8);  //SLink ignore LFF
  usleep(10);    
       
  s=1024;
  VMEmove(0x1003,LAD_C+0x160000,0,destaddr,s,4);  //  DOWN BT - FIFO Mode
  for(int i=0;i<1024;i++) D_down[i]=destaddr[i];
  
  s=1024;
  VMEmove(0x1003,LAD_C+0x140000,0,destaddr,s,4);  //  UP BT - FIFO Mode
  for(int i=0;i<1024;i++) D_up[i]=destaddr[i];
  
  
  
  printf("\n\n");
  printf("|||||SLID|EVTY|--Event#--|--BunchC#--|--SourceID--|FOV|FH|\n");
  printf("  0:|  %1x |  %1x |  %6x  |"  ,int((D_up[0]&0xf0000000)>>28),int((D_up[0]&0xf000000)>>24),int((D_up[0]&0xffffff)) );
  printf("      %3x  |       %3x  |  %1x| %1x| %8x          %8x\n",
	 int((D_down[0]&0xfff00000)>>20),int((D_down[0]&0xfff00)>>8),int((D_down[0]&0xf0)>>4),int(D_down[0]&0xf),int(D_up[0]),int(D_down[0]));
  printf("|||||----|----|----------|-----------|------------|---|--|\n");
  
  printf("\n\n|||||CH|RO|DC|PXL|PH| S-Link |CH|RO|DC|PXL|PH|||||\n");
  
  for(int i=1;i<163;i++) {  //110
    
    printf("%3d:  %2d %2d %2d %3d %2x       ",
	   i,int((D_up[i]>>26)&0x3f),int((D_up[i]>>21)&0x1f),int((D_up[i]>>16)&0x1f),int((D_up[i]>>8)&0xff),int(D_up[i]&0xff));
    printf("  %2d %2d %2d %3d %2x              %8x          %8x\n",
	   int((D_down[i]>>26)&0x3f),int((D_down[i]>>21)&0x1f),int((D_down[i]>>16)&0x1f),int((D_down[i]>>8)&0xff),int(D_down[i]&0xff),int(D_up[i]),int(D_down[i]));
    usleep(10);  
    
    //	 printf("testing CENTER ...\n");
    //int istuff=0;
    //for(i=0;i<=900000;i++)  {
    // VMEout(0x3,LAD_C+0x1a8000,4,i+0x1fff0000); VMEin(0x3,LAD_C+0x1a8000,4,&d);
    // if(d!=i+0x1fff0000) {printf("ERROR at: WRITTEN:%6x   READ:%6x\n",i+0x1fff0000,d);std::cin>>istuff;break;}
    // if((i%100000)==0) printf("%d\n",i);
    //}
    
  }
  
  for(int i=1;i<183;i++) {  //110
    if(((D_up[i]>>21)&0x1f)>27)cout<<dec<<DecodeError(D_up[i])<<dec<<" data 0x"<<hex<<D_up[i]<<dec<<endl;
    if(((D_down[i]>>21)&0x1f)>27)cout<<dec<<DecodeError(D_down[i])<<dec<<" data 0x"<<hex<<D_up[i]<<dec<<endl;
    usleep(10);  
  }

  //try reading spy fifo
  for(int j1=0;j1<128;j1++){ 
    // VMEin(0x3,LAD_N+0x150000,4,&d);
    // buffer1[j1]=d;
    // printf("Fifo-2 North up %d %x ",j1,d);
    // VMEin(0x3,LAD_N+0x170000,4,&d);
    // printf("Fifo-2 NorthCenter up %d %x\n ",j1,d);
    // VMEin(0x3,LAD_N+0x150000,4,&d);
    // printf("Fifo-2 our N %d %x\n ",j1,d);
    // VMEin(0x3,LAD_NC+0x150000,4,&d);
    // printf("Fifo-2 our NC %d %x\n ",j1,d);
    //cout << "after buffer1[j1] " << buffer1[j1] << " d " << d << endl;
    // buffer2[j1]=d;
    
    VMEin(0x3,LAD_N+0x150000,4,&d);
    buffer1[j1]=d;
    printf("Fifo-2 North up %d %x ",j1,int(d));
    VMEin(0x3,LAD_NC+0x150000,4,&d);
    printf("Fifo-2 NorthCenter up %d %x\n ",j1,int(d));
    buffer2[j1]=d;
    
  } 
  
  // for(int j1=0;j1<128;j1++){
  // 	 VMEin(0x3,LAD_S+0x150000,4,&d);
  // 	 buffer1[j1]=d;
  // 	 printf("Spy Fifo South %d %x \n",j1,d);
  // } 
  
  cout<<decodePTrans(buffer1,buffer2,16)<<endl;
  cout<<decodePTrans2(buffer1,buffer2,16)<<endl;
  

  //decode(buffer1,buffer2,64);
  
  decode_withROCs(buffer1,buffer2,64);
  //cout << " buffer1[0] " << buffer1[0] << endl;
  //decode(buffer1,buffer2,64);
  cout << "tbmheader 1 = " << a[0]<< endl;
  cout << "tbmheader 2 = " << a2[0]<< endl;
  
  cout << "rocheader 1 = " << a[1]<< endl;
  cout << "rocheader 2 = " << a2[1]<< endl;
       
  cout << "correctdata 1 = " << a[2]<< endl;
  cout << "correctdata 2 = " << a2[2]<< endl;
  
  cout << "tbmtrailer 1 = " << a[3]<< endl;
  cout << "tbmtrailer 2 = " << a2[3]<< endl;

  //decode(buffer1,buffer2,64);
  
  //cout << " 3 " << endl;
  //cout << " buffer1[0] " << buffer1[0] << endl;

  printf("\nPIGGYup\n");
  for(int i=0;i<256;i++) {
    VMEin(0x3,LAD_N+0x20000,4,&d);
    if (i<42) {
      printf("%04x %04x              ",int((d>>16)&0xffff), int(d&0xffff));
      BinDisplay((d>>16)&0xffff, 16,0,0);
      printf("    ");
      BinDisplay(d&0xffff, 16,0,0);
      printf("\n");
      
      file<<hex<<((d>>16)&0xffff)<< " " <<hex<< (d&0xffff)<< "   ";
      int Bits(16); int i1; unsigned long mask;
      
      mask=1<<(Bits-1);  
      for (i1=0;i1<Bits;i1++) {
	if ( (((d>>16)&0xffff)<<i1) & mask) file<<"1";
	else  file<<"0";
      }
      file<<"    ";
      for (i1=0;i1<Bits;i1++) {
	if ( ((d&0xffff)<<i1) & mask) file<<"1";
	else  file<<"0";
      }
      file<<endl;
      
    }
    
  }
  
  //cout << " 2 " << endl;
  //cout << " buffer1[0] " << buffer1[0] << endl;
  //printf("\nPIGGYdown\n");
  for(int i=0;i<256;i++) {
    VMEin(0x3,LAD_S+0x20000,4,&d);
    if (i<42) {
      printf("%04x %04x              ",int((d>>16)&0xffff), int(d&0xffff));
      BinDisplay((d>>16)&0xffff, 16,0,0);
      printf("    ");
      BinDisplay(d&0xffff, 16,0,0);
      printf("\n");
    }
  }
  
  
  file.close();
  cout<<"scaler"<<caendat<<endl;
  cout<<"scaler after trigger"<<caendat1<<endl;

  return;
  
  //*****************
}

void SendTriggerAndReadAll() {

  //*********************
  //commented to avoid conflict LC
  /*
  unsigned long int d, data, s;
  int caenret=0;
  unsigned int caendat=1;unsigned int caendat1=1;
  unsigned long int D_up[1024],D_down[1024];
  unsigned long int ErrorFIFO_down[256],Old_Data;
  unsigned long int destaddr[1024];
  
  // THIS IS REGRESET for SPYING
  RegResetPiggy();
  //////
  
  //Count number of times locked   
  int count(0);
  for(int i=0;i<10;i++)  {
    
    //VMEin(0x3,LAD_S+0x178000,4,&d);
    VMEin(0x3,LAD_N+0x178000,4,&d);
    //printf("                               %2x         %2x          %2x         %1d\n", int((d)&0xff), int((d>>8)&0xff), int((d>>16)&0xff), int((d>>24)&0x1));
    count+=(d>>24)&0x1;
  }       
  cout << "# of times locked = "  << count << endl;
       
  Reset(); 
       
  count=0;
  for(int i=0;i<10;i++)  {
	 
    //VMEin(0x3,LAD_S+0x178000,4,&d);
    VMEin(0x3,LAD_N+0x178000,4,&d);
    //printf("                               %2x         %2x          %2x         %1d\n", int((d)&0xff), int((d>>8)&0xff), int((d>>16)&0xff), int((d>>24)&0x1));
    count+=(d>>24)&0x1;
  }
  cout << "after the FED reset # of times locked = "  << count << endl;
  count=0;


  for(int i=0;i<10;i++)  {
    
    //VMEin(0x3,LAD_S+0x178000,4,&d);
    VMEin(0x3,LAD_N+0x178000,4,&d);
    printf("                               %2x         %2x          %2x         %1d\n", int((d)&0xff), int((d>>8)&0xff), int((d>>16)&0xff), int((d>>24)&0x1));
    count+=(d>>24)&0x1;
  }
  cout << "after disabling the unused channels  # of times locked = "  << count << endl;
 
       
  caenret = CAENVME_ReadRegister(0,cvScaler1,&caendat);
       
  // values must match with TTCciConfiguration.txt
  //Sequence for BGO 12 = 0xc   Channel #12 (CAL+L1): Private Pixel Command, Send CAL + L1A
  //data=0x9 ;	// sequence for BG0 9 =0x9  START (starts cyclic trigger and cyclic BGO calsync)
  //data=0xc ;  // send CalSync + L1A
  data=0xb;  // bgo 0xb = 11 = single l1a
  //data = 0x9; // bgo 9 = start
  ret = CAENVME_WriteCycle(0,ttcaddcsr0,&data,cvA24_U_DATA,cvD32);
  
  //you can call this now when you need a trigger, it should not be called more often than 1/LHC orbit ~90us!
  if(ret != cvSuccess) { 
    cout<<"Error in write TTCci BGO Trigger"<<hex<<ret<<" "<<d<<endl;  
    printf("Error in TTCci trigger!\n");
    analyzeError(ret); 
  }
  
  printf("Trigger sent!\n");
  usleep(10000);
  caenret = CAENVME_ReadRegister(0,cvScaler1,&caendat1);
 
       
  usleep(100);
 
  
  printf("\n\n\nTEMP FIFO Sup\n\n");
  for(int i=0;i<256;i++)  { 
    VMEin(0x3,LAD_S+0x148000,4,&d);	
    ///////VMEin(0x3,LAD_N+0x168000,4,&d);
    if(d) { 
      printf("%x\n",int(d)); 
      printf("CH#:%2d",int((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",int(((d>>1)&0xff00)+(d&0xff)));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",int(((d>>4)&0xff00)+(d&0xff)));    
    }
  }	
  
  printf("\n\n\nTEMP FIFO Sdown\n\n");
  for(int i=0;i<256;i++)  { 
    VMEin(0x3,LAD_S+0x168000,4,&d);	
    ///////VMEin(0x3,LAD_N+0x168000,4,&d);
    if(d) { 
      printf("%x\n",int(d)); 
      printf("CH#:%2d",int((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",int(((d>>1)&0xff00)+(d&0xff)));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",int(((d>>4)&0xff00)+(d&0xff)));    
    }
  }

  printf("\n\n\nTEMP FIFO SCup\n\n");
  for(int i=0;i<256;i++)  { 
    VMEin(0x3,LAD_SC+0x148000,4,&d);	
    ///////VMEin(0x3,LAD_N+0x168000,4,&d);
    if(d) { 
      printf("%x\n",int(d)); 
      printf("CH#:%2d",int((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",int(((d>>1)&0xff00)+(d&0xff)));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",int(((d>>4)&0xff00)+(d&0xff)));    
    }
  }	
  
  printf("\n\n\nTEMP FIFO SCdown\n\n");
  for(int i=0;i<256;i++)  { 
    VMEin(0x3,LAD_SC+0x168000,4,&d);	
    ///////VMEin(0x3,LAD_N+0x168000,4,&d);
    if(d) { 
      printf("%x\n",int(d)); 
      printf("CH#:%2d",int((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",int(((d>>1)&0xff00)+(d&0xff)));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",int(((d>>4)&0xff00)+(d&0xff)));    
    }
  }	
   
  printf("\n\n\nTEMP FIFO NCup\n\n");
  for(int i=0;i<256;i++)  { 
    VMEin(0x3,LAD_NC+0x148000,4,&d);	
    ///////VMEin(0x3,LAD_N+0x168000,4,&d);
    if(d) { 
      printf("check %x\n",int(d)); 
      printf("CH#:%2d",int((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",int(((d>>1)&0xff00)+(d&0xff)));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",int(((d>>4)&0xff00)+(d&0xff)));    
    }
  }	
   
  printf("\n\n\nTEMP FIFO NCdown\n\n");
  for(int i=0;i<256;i++)  { 
    VMEin(0x3,LAD_NC+0x168000,4,&d);	
    ///////VMEin(0x3,LAD_N+0x168000,4,&d);
    if(d) { 	 //now try reading spy fifo-2
      printf("%x\n",int(d)); 
      printf("CH#:%2d",int((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",int(((d>>1)&0xff00)+(d&0xff)));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",int(((d>>4)&0xff00)+(d&0xff)));    
    }     
  }
  
  printf("\n\n\nTEMP FIFO Nup\n\n");
  for(int i=0;i<256;i++)  { 
    VMEin(0x3,LAD_N+0x148000,4,&d);	
    ///////VMEin(0x3,LAD_N+0x168000,4,&d);
    if(d) { 
      printf("%x\n",int(d)); 
      printf("CH#:%2d",int((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",int(((d>>1)&0xff00)+(d&0xff)));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",int(((d>>4)&0xff00)+(d&0xff)));    
    }
  }	
   
  printf("\n\n\nTEMP FIFO Ndown\n\n");
  for(int i=0;i<256;i++)  { 
    VMEin(0x3,LAD_N+0x168000,4,&d);	
    ///////VMEin(0x3,LAD_N+0x168000,4,&d);
    if(d) { 	 //now try reading spy fifo-2
      printf("%x\n",int(d)); 
      printf("CH#:%2d",int((d>>26)&0x3f));
      if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",int(((d>>1)&0xff00)+(d&0xff)));
      if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",int(((d>>4)&0xff00)+(d&0xff)));    
    }     
  }	
  
       
  //printf("\n\n\nErrorFIFO SOUTHdown\n\n");
  //for(int i=0;i<256;i++) VMEin(0x3,LAD_S+0x160000,4,&ErrorFIFO_down[i]);  
  printf("\n\n\nErrorFIFO NORTHdown\n\n");
  for(int i=0;i<256;i++) VMEin(0x3,LAD_N+0x160000,4,&ErrorFIFO_down[i]);
       
  Old_Data=0;int ii=0;
  while ( (Old_Data!=ErrorFIFO_down[ii]) && (ii<255) )
    {
      printf("%3d.  CH#%2d  CODE#%2d  TTC_EventNr:%2x  E_Data[hex]=%2x\n",
	     ii,int((ErrorFIFO_down[ii]>>26)&0x3f),int((ErrorFIFO_down[ii]>>21)&0x1f),int((ErrorFIFO_down[ii]>>13)&0xff),int((ErrorFIFO_down[ii])&0x3ff));
      Old_Data=ErrorFIFO_down[ii];
      ii++;	 //now try reading spy fifo-2
    }
  //printf("\nPhase:%1x\n\n",l&0x3);		
       
  VMEout(0x3,LAD_C+0x1c0000,4,0x0);  //SLink disable ;Disable SpyMemWrite during readout     
  //VMEout(0x3,LAD_C+0x1c0000,4,0x8);  //SLink ignore LFF
  usleep(10);    
       
  s=1024;
  VMEmove(0x1003,LAD_C+0x160000,0,destaddr,s,4);  //  DOWN BT - FIFO Mode
  for(int i=0;i<1024;i++) D_down[i]=destaddr[i];
  
  s=1024;
  VMEmove(0x1003,LAD_C+0x140000,0,destaddr,s,4);  //  UP BT - FIFO Mode
  for(int i=0;i<1024;i++) D_up[i]=destaddr[i];
  
  
  
  printf("\n\n");
  printf("|||||SLID|EVTY|--Event#--|--BunchC#--|--SourceID--|FOV|FH|\n");
  printf("  0:|  %1x |  %1x |  %6x  |"  ,int((D_up[0]&0xf0000000)>>28),int((D_up[0]&0xf000000)>>24),int((D_up[0]&0xffffff)) );
  printf("      %3x  |       %3x  |  %1x| %1x| %8x          %8x\n",
	 int((D_down[0]&0xfff00000)>>20),int((D_down[0]&0xfff00)>>8),int((D_down[0]&0xf0)>>4),int(D_down[0]&0xf),int(D_up[0]),int(D_down[0]));
  printf("|||||----|----|----------|-----------|------------|---|--|\n");
  
  printf("\n\n|||||CH|RO|DC|PXL|PH| S-Link |CH|RO|DC|PXL|PH|||||\n");
  
  for(int i=1;i<163;i++) {  //110
    
    printf("%3d:  %2d %2d %2d %3d %2x       ",
	   i,int((D_up[i]>>26)&0x3f),int((D_up[i]>>21)&0x1f),int((D_up[i]>>16)&0x1f),int((D_up[i]>>8)&0xff),int(D_up[i]&0xff));
    printf("  %2d %2d %2d %3d %2x              %8x          %8x\n",
	   int((D_down[i]>>26)&0x3f),int((D_down[i]>>21)&0x1f),int((D_down[i]>>16)&0x1f),int((D_down[i]>>8)&0xff),int(D_down[i]&0xff),int(D_up[i]),int(D_down[i]));
    usleep(10);  
    
    //	 printf("testing CENTER ...\n");
    //int istuff=0;
    //for(i=0;i<=900000;i++)  {
    // VMEout(0x3,LAD_C+0x1a8000,4,i+0x1fff0000); VMEin(0x3,LAD_C+0x1a8000,4,&d);
    // if(d!=i+0x1fff0000) {printf("ERROR at: WRITTEN:%6x   READ:%6x\n",i+0x1fff0000,d);std::cin>>istuff;break;}
    // if((i%100000)==0) printf("%d\n",i);
    //}
    
  }
  
  for(int i=1;i<183;i++) {  //110
    if(((D_up[i]>>21)&0x1f)>27)cout<<dec<<DecodeError(D_up[i])<<dec<<" data 0x"<<hex<<D_up[i]<<dec<<endl;
    if(((D_down[i]>>21)&0x1f)>27)cout<<dec<<DecodeError(D_down[i])<<dec<<" data 0x"<<hex<<D_up[i]<<dec<<endl;
    usleep(10);  
  }

  //try reading spy fifo
  for(int j1=0;j1<128;j1++){ 
    // VMEin(0x3,LAD_N+0x150000,4,&d);
    // buffer1[j1]=d;
    // printf("Fifo-2 North up %d %x ",j1,d);
    // VMEin(0x3,LAD_N+0x170000,4,&d);
    // printf("Fifo-2 NorthCenter up %d %x\n ",j1,d);
    // VMEin(0x3,LAD_N+0x150000,4,&d);
    // printf("Fifo-2 our N %d %x\n ",j1,d);
    // VMEin(0x3,LAD_NC+0x150000,4,&d);
    // printf("Fifo-2 our NC %d %x\n ",j1,d);
    //cout << "after buffer1[j1] " << buffer1[j1] << " d " << d << endl;
    // buffer2[j1]=d;
    
    VMEin(0x3,LAD_N+0x150000,4,&d);
    buffer1[j1]=d;
    printf("Fifo-2 North up %d %x ",j1,int(d));
    VMEin(0x3,LAD_NC+0x150000,4,&d);
    printf("Fifo-2 NorthCenter up %d %x\n ",j1,int(d));
    buffer2[j1]=d;
    
  } 
  
  // for(int j1=0;j1<128;j1++){
  // 	 VMEin(0x3,LAD_S+0x150000,4,&d);
  // 	 buffer1[j1]=d;
  // 	 printf("Spy Fifo South %d %x \n",j1,d);
  // } 
  
  cout<<decodePTrans(buffer1,buffer2,16)<<endl;
  cout<<decodePTrans2(buffer1,buffer2,16)<<endl;
  

  //decode(buffer1,buffer2,64);
  
  decode_withROCs(buffer1,buffer2,64);
  //cout << " buffer1[0] " << buffer1[0] << endl;
  //decode(buffer1,buffer2,64);
  cout << "tbmheader 1 = " << a[0]<< endl;
  cout << "tbmheader 2 = " << a2[0]<< endl;
  
  cout << "rocheader 1 = " << a[1]<< endl;
  cout << "rocheader 2 = " << a2[1]<< endl;
       
  cout << "correctdata 1 = " << a[2]<< endl;
  cout << "correctdata 2 = " << a2[2]<< endl;
  
  cout << "tbmtrailer 1 = " << a[3]<< endl;
  cout << "tbmtrailer 2 = " << a2[3]<< endl;

  //decode(buffer1,buffer2,64);
  
  //cout << " 3 " << endl;
  //cout << " buffer1[0] " << buffer1[0] << endl;

  printf("\nPIGGYup\n");
  for(int i=0;i<256;i++) {
    VMEin(0x3,LAD_N+0x20000,4,&d);
    if (i<42) {
      printf("%04x %04x              ",int((d>>16)&0xffff), int(d&0xffff));
      BinDisplay((d>>16)&0xffff, 16,0,0);
      printf("    ");
      BinDisplay(d&0xffff, 16,0,0);
      printf("\n");
      
      file<<hex<<((d>>16)&0xffff)<< " " <<hex<< (d&0xffff)<< "   ";
      int Bits(16); int i1; unsigned long mask;
      
      mask=1<<(Bits-1);  
      for (i1=0;i1<Bits;i1++) {
	if ( (((d>>16)&0xffff)<<i1) & mask) file<<"1";
	else  file<<"0";
      }
      file<<"    ";
      for (i1=0;i1<Bits;i1++) {
	if ( ((d&0xffff)<<i1) & mask) file<<"1";
	else  file<<"0";
      }
      file<<endl;
      
    }
    
  }
  
  //cout << " 2 " << endl;
  //cout << " buffer1[0] " << buffer1[0] << endl;
  //printf("\nPIGGYdown\n");
  for(int i=0;i<256;i++) {
    VMEin(0x3,LAD_S+0x20000,4,&d);
    if (i<42) {
      printf("%04x %04x              ",int((d>>16)&0xffff), int(d&0xffff));
      BinDisplay((d>>16)&0xffff, 16,0,0);
      printf("    ");
      BinDisplay(d&0xffff, 16,0,0);
      printf("\n");
    }
  }
  
  
  file.close();
  cout<<"scaler"<<caendat<<endl;
  cout<<"scaler after trigger"<<caendat1<<endl;

  return;
  */
}

void FitelUseFibers(int sel=1) {

  if (sel<0 || sel>1) 
    cout << "FITEL: Valid fiber use setting is 0 or 1" << endl;
  //printf("fibers #1 to #6  / #7 to #12     1 / 0  ? \n");
  //scanf("%d", &select_fibers);
  //printf("fiber selection: %d\n", select_fibers);
  select_fibers  = sel;
  if (piggyN) {
    VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_select_fibers_1to6 + select_fibers );
    if (sel==0) 
      std::cout << "FITEL PiggyNorth: Using Fibers 1,2,3,4,11,12" << endl;
    else 
      std::cout << "FITEL PiggyNorth: Using Fibers 5,6,7,8,9,10" << endl;
  }
  else {
    VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_select_fibers_1to6 + select_fibers );
    if (sel==0) 
      std::cout << "FITEL PiggySouth: Using Fibers 1,2,3,4,11,12" << endl;
    else 
      std::cout << "FITEL PiggySouth: Using Fibers 5,6,7,8,9,10" << endl;
  }

}

void SelectFiber(int fibernr) {
  
  if (!isFitel) {//ZARLINK FIBER SELECTION
    if (fibernr<7 || fibernr >12) {
      cout << "ZARLINK: Valid fiber numbers are 7 to 12" << endl;
      return;
    }
    
    fiber = fibernr;
    
    // Turn on the channel to spy on  bits 8-11 in the mode register for the channel
    //also msk somr trailer error bits (<<24)
    //????????????????????????????????????
    VMEout(0x3,LAD_N+0x1c0000,4,((0x2)<<24)+(0x8<<8));  //North Mode register     
    VMEout(0x3,LAD_NC+0x1c0000,4,((0x2)<<24)+(0x0<<8));  //NorthCenter Mode register 
    
    
    //channel selection for PIGGY   
    if (piggyN) {
      VMEout(0x3,LAD_N+0x1a8000,4,CHa_CHb_mux+0x2300+(fibernr-7)); // ch selection for transparent mode: 5-fiber#12, 4-fiber#11, 3-fiber#10, 2-fiber#9, 1-fiber# 8, 0-fiber#7
    }
    else {
      VMEout(0x3,LAD_S+0x1a8000,4,CHa_CHb_mux+0x2300+(fibernr-7)); // ch selection for transparent mode: 5-fiber#12, 4-fiber#11, 3-fiber#10, 2-fiber#9, 1-fiber# 8, 0-fiber#7
    }
    
    //////////////////////////////////
    //fiber to channel mapping
    //fiber    ch
    //N-7      1 2
    //N-8      3 4
    //N-9      5 6
    //N-10     7 8
    //N-11     9 10
    //N-12    11 12
    //S-7     25 26
    //S-8     27 28
    //S-9     29 30
    //S-10    31 32
    //S-11    33 34
    //S-12    35 36
    //////////////////////////////////
    
    
    //Channel selection for TEMP FIFO
    
    ///////////////////////////////////////////////////////////
    //  bit  0  1  2  3  4  5  6  7  8   
    //  ch   1  2  3  4  5  6  7  8  9  N
    //  ch   10 11 12 13 14 15 16 17 18 NC
    //  ch   19 20 21 22 23 24 25 26 27 SC
    //  ch   28 29 30 31 32 33 34 35 36 S
    ///////////////////////////////////////////////////////////
    
    // For Up Piggy mode register bit [11..8] = 5 (000 for ch1, 100 for ch2, 200 for ch3, 300 for ch4, 400 for ch5, 500 for ch6, 600 for ch7, 700 for ch8, 800 for ch9) 
    // VMEout(0x3,LAD_N+0x1c0000,4,0x800); 
    // For Up Piggy mode register bit [11..8] = 5 (000 for ch10, 100 for ch11, 200 for ch12, 300 for ch13, 400 for ch14, 500 for ch15, 600 for ch16, 700 for ch17, 800 for ch18)
    // VMEout(0x3,LAD_NC+0x1c0000,4,(0x000));  
    // For Down Piggy mode register bit [11..8] = 5 (000 for ch19, 100 for ch20, 200 for ch21, 300 for ch22, 400 for ch23, 500 for ch24, 600 for ch25, 700 for ch26, 800 for ch27)
    // VMEout(0x3,LAD_SC+0x1c0000,4,(0x000)); 
    // For Down Piggy  mode register bit [11..8] = 5 (000 for ch28, 100 for ch29, 200 for ch30, 300 for ch31, 400 for ch32, 500 for ch33, 600 for ch34, 700 for ch35, 800 for ch36)
    // VMEout(0x3,LAD_S+0x1c0000,4,0x600);
    
    ///////////////////////////////////////////////////////////
    
    if (piggyN) { //Piggy North
      int ch;
      if (TBMA)    //TBM-A
	ch = 2*fibernr - 13;
      else  //TBM-B
	ch = 2*fibernr - 12;
      
      if (ch<10) {
	int value = (ch-1)<<8;
	cout << "N channel " << std::dec << ch << " 0x" << std::hex << value << " " << std::bitset<16>(value) << std::dec << endl;
	VMEout(0x3,LAD_N+0x1c0000,4,0x800); 
      }
      else {
	int value = (ch-1-9)<<8;
	cout << "NC channel " << std::dec << ch << " 0x" << std::hex << value << " " << std::bitset<16>(value) << std::dec << endl;
	VMEout(0x3,LAD_NC+0x1c0000,4,0x800); 
      }      
    }
    
    else { //Piggy South
      int ch;
      if (TBMA)    //TBM-A
	ch = 2*fibernr + 11;
      else  //TBM-B
	ch = 2*fibernr + 12;
      
      if (ch<28) {
	int value = (ch-1-18)<<8;
	cout << "SC channel " << std::dec << ch << " 0x" << std::hex << value << " " << std::bitset<16>(value) << std::dec << endl;
	VMEout(0x3,LAD_SC+0x1c0000,4,0x800); 
      }
      else {
	int value = (ch-1-27)<<8;
	cout << "S channel " << std::dec << ch << " 0x" << std::hex << value << " " << std::bitset<16>(value) << std::dec << endl;
	VMEout(0x3,LAD_S+0x1c0000,4,0x800); 
      }      
    }
    
    
    //CHANNEL ENABLE --------------
    VMEout( 0x3,LAD_N+0x1a0000, 4,0x1ff); //Disable all channels
    VMEout( 0x3,LAD_NC+0x1a0000,4,0x1ff); 
    VMEout( 0x3,LAD_SC+0x1a0000,4,0x1ff);
    VMEout( 0x3,LAD_S+0x1a0000, 4,0x1ff);
    if (piggyN) { //Piggy North
      
      if (fibernr==7) {
	VMEout( 0x3,LAD_N+0x1a0000, 4,0x1fc); //ch 1&2
      } 
      else if (fibernr==8) {
	VMEout( 0x3,LAD_N+0x1a0000, 4,0x1f3); //ch 3&4
      } 
      else if (fibernr==9) {
	VMEout( 0x3,LAD_N+0x1a0000, 4,0x1cf); //ch 5&6
      }  
      else if (fibernr==10) {
	VMEout( 0x3,LAD_N+0x1a0000, 4,0x13f); //ch 7&8
      }  
      else if (fibernr==11) {
	VMEout( 0x3,LAD_N+0x1a0000, 4,0x0ff); //ch 9&10
	VMEout( 0x3,LAD_NC+0x1a0000, 4,0x1fe); 
      }  
      else if (fibernr==12) {
	VMEout( 0x3,LAD_NC+0x1a0000, 4,0x1f9); //ch 11&12
      }
      
    }
    else { //Piggy South
      
      if (fibernr==7) {
	VMEout( 0x3,LAD_SC+0x1a0000, 4,0x13f); //ch 25&26
      } 
      else if (fibernr==8) {
	VMEout( 0x3,LAD_SC+0x1a0000, 4,0x0ff); //ch 27&28
	VMEout( 0x3,LAD_S+0x1a0000, 4,0x1fe); 
      } 
      else if (fibernr==9) {
	VMEout( 0x3,LAD_S+0x1a0000, 4,0x1f9); //ch 29&30
      }  
      else if (fibernr==10) {
	VMEout( 0x3,LAD_S+0x1a0000, 4,0x1e7); //ch 31&32
      }  
      else if (fibernr==11) {
	VMEout( 0x3,LAD_S+0x1a0000, 4,0x1cf); //ch 33&34
      }  
      else if (fibernr==12) {
	VMEout( 0x3,LAD_S+0x1a0000, 4,0x07f); //ch 35&36
    }
      
    }
  }//END ZARLINK FIBER SELECTION
  
  else {//FITEL FIBER SELECTION
    if (fibernr<1 || fibernr >12) {
      cout << "ZARLINK: Valid fiber numbers are 1 to 12" << endl;
      return;
    }

    fiber = fibernr;

    //Select 0: Fibers 1,2,3,4,11,12
    //Select 1: Fibers 5,6,7,8,9,10
    int fibersel = 0;
    if (fibernr > 4 && fibernr < 11) 
      fibersel=1;
    FitelUseFibers(fibersel);
    
    
    // Turn on the channel to spy on  bits 8-11 in the mode register for the channel
    //also msk somr trailer error bits (<<24)
    //????????????????????????????????????
    VMEout(0x3,LAD_N+0x1c0000,4,((0x2)<<24)+(0x8<<8));  //North Mode register     
    VMEout(0x3,LAD_NC+0x1c0000,4,((0x2)<<24)+(0x0<<8));  //NorthCenter Mode register 
    
    
    //channel selection for PIGGY   
    //channel select transparent mode :  
    //  mode bit  = 0
    //  addr bits = 3
    //  data bits (2..0) = 	0 -> fiber#7 	FITEL#  4 or  5
    //                    	1 -> fiber#8    FITEL# 11 or  6
    //                    	2 -> fiber#9	FITEL# 12 or  7 
    //                    	3 -> fiber#10   FITEL#  1 or  8
    //                    	4 -> fiber#11   FITEL#  2 or  9
    //                    	5 -> fiber#12   FITEL#  3 or 10 

    if (piggyN) {
      if (fibernr == 4 || fibernr == 5) 
	VMEout(0x3,LAD_N+0x1a8000,4,CHa_CHb_mux+0x2300+0); 
      else if (fibernr == 11 || fibernr == 6) 
	VMEout(0x3,LAD_N+0x1a8000,4,CHa_CHb_mux+0x2300+1); 
      else if (fibernr == 12 || fibernr == 7) 
	VMEout(0x3,LAD_N+0x1a8000,4,CHa_CHb_mux+0x2300+2); 
      else if (fibernr == 1 || fibernr == 8) 
	VMEout(0x3,LAD_N+0x1a8000,4,CHa_CHb_mux+0x2300+3); 
      else if (fibernr == 2 || fibernr == 9) 
	VMEout(0x3,LAD_N+0x1a8000,4,CHa_CHb_mux+0x2300+4);  
      else if (fibernr == 3 || fibernr == 10) 
	VMEout(0x3,LAD_N+0x1a8000,4,CHa_CHb_mux+0x2300+5); 
    }
    else {
      if (fibernr == 4 || fibernr == 5) 
	VMEout(0x3,LAD_S+0x1a8000,4,CHa_CHb_mux+0x2300+0); 
      else if (fibernr == 11 || fibernr == 6) 
	VMEout(0x3,LAD_S+0x1a8000,4,CHa_CHb_mux+0x2300+1); 
      else if (fibernr == 12 || fibernr == 7) 
	VMEout(0x3,LAD_S+0x1a8000,4,CHa_CHb_mux+0x2300+2); 
      else if (fibernr == 1 || fibernr == 8) 
	VMEout(0x3,LAD_S+0x1a8000,4,CHa_CHb_mux+0x2300+3); 
      else if (fibernr == 2 || fibernr == 9) 
	VMEout(0x3,LAD_S+0x1a8000,4,CHa_CHb_mux+0x2300+4);  
      else if (fibernr == 3 || fibernr == 10) 
	VMEout(0x3,LAD_S+0x1a8000,4,CHa_CHb_mux+0x2300+5);
    }
    
    //////////////////////////////////
    //fiber to channel mapping
    //fiber    ch
    //N-1      7 8
    //N-2      9 10
    //N-3     11 12 
    //N-4      1 2
    //N-5      1 2 
    //N-6      3 4
    //N-7      5 6 
    //N-8      7 8 
    //N-9      9 10 
    //N-10    11 12
    //N-11     3 4
    //N-12     5 6 
    //S-1     31 32
    //S-2     33 34
    //S-3     35 36
    //S-4     25 26
    //S-5     25 26
    //S-6     27 28
    //S-7     29 30
    //S-8     31 32
    //S-9     33 34 
    //S-10    35 36
    //S-11    27 28
    //S-12    29 30
    //////////////////////////////////
    
    
    //Channel selection for TEMP FIFO
    
    ///////////////////////////////////////////////////////////
    //  bit  0  1  2  3  4  5  6  7  8   
    //  ch   1  2  3  4  5  6  7  8  9  N
    //  ch   10 11 12 13 14 15 16 17 18 NC
    //  ch   19 20 21 22 23 24 25 26 27 SC
    //  ch   28 29 30 31 32 33 34 35 36 S
    ///////////////////////////////////////////////////////////
    
    // For Up Piggy mode register bit [11..8] = 5 (000 for ch1, 100 for ch2, 200 for ch3, 300 for ch4, 400 for ch5, 500 for ch6, 600 for ch7, 700 for ch8, 800 for ch9) 
    // VMEout(0x3,LAD_N+0x1c0000,4,0x800); 
    // For Up Piggy mode register bit [11..8] = 5 (000 for ch10, 100 for ch11, 200 for ch12, 300 for ch13, 400 for ch14, 500 for ch15, 600 for ch16, 700 for ch17, 800 for ch18)
    // VMEout(0x3,LAD_NC+0x1c0000,4,(0x000));  
    // For Down Piggy mode register bit [11..8] = 5 (000 for ch19, 100 for ch20, 200 for ch21, 300 for ch22, 400 for ch23, 500 for ch24, 600 for ch25, 700 for ch26, 800 for ch27)
    // VMEout(0x3,LAD_SC+0x1c0000,4,(0x000)); 
    // For Down Piggy  mode register bit [11..8] = 5 (000 for ch28, 100 for ch29, 200 for ch30, 300 for ch31, 400 for ch32, 500 for ch33, 600 for ch34, 700 for ch35, 800 for ch36)
    // VMEout(0x3,LAD_S+0x1c0000,4,0x600);
    
    ///////////////////////////////////////////////////////////
    
    if (piggyN) { //Piggy North
      int ch=0;
      if (TBMA)    //TBM-A 
	{
	  if (fibernr == 4 || fibernr == 5)
	    ch = 1;
	  else if (fibernr == 6 || fibernr == 11)
	    ch = 3;
	  else if (fibernr == 7 || fibernr == 12)
	    ch = 5;
	  else if (fibernr == 1 || fibernr == 8)
	    ch = 7;
	  else if (fibernr == 2 || fibernr == 9)
	    ch = 9;
	  else if (fibernr == 3 || fibernr == 10)
	    ch = 11;
	}
      else  //TBM-B
	{
	  if (fibernr == 4 || fibernr == 5)
	    ch = 2;
	  else if (fibernr == 6 || fibernr == 11)
	    ch = 4;
	  else if (fibernr == 7 || fibernr == 12)
	    ch = 6;
	  else if (fibernr == 1 || fibernr == 8)
	    ch = 8;
	  else if (fibernr == 2 || fibernr == 9)
	    ch = 10;
	  else if (fibernr == 3 || fibernr == 10)
	    ch = 12;
	}
      
      if (ch<10) {
	int value = (ch-1)<<8;
	cout << "N channel " << std::dec << ch << " 0x" << std::hex << value << " " << std::bitset<16>(value) << std::dec << endl;
	VMEout(0x3,LAD_N+0x1c0000,4,0x800); 
      }
      else {
	int value = (ch-1-9)<<8;
	cout << "NC channel " << std::dec << ch << " 0x" << std::hex << value << " " << std::bitset<16>(value) << std::dec << endl;
	VMEout(0x3,LAD_NC+0x1c0000,4,0x800); 
      }      
    }
    
    else { //Piggy South
      int ch = 0;
      if (TBMA)    //TBM-A
	{
	  if (fibernr == 4 || fibernr == 5)
	    ch = 25;
	  else if (fibernr == 6 || fibernr == 11)
	    ch = 27;
	  else if (fibernr == 7 || fibernr == 12)
	    ch = 29;
	  else if (fibernr == 1 || fibernr == 8)
	    ch = 31;
	  else if (fibernr == 2 || fibernr == 9)
	    ch = 33;
	  else if (fibernr == 3 || fibernr == 10)
	    ch = 35;
	}
      else  //TBM-B
	{
	  if (fibernr == 4 || fibernr == 5)
	    ch = 26;
	  else if (fibernr == 6 || fibernr == 11)
	    ch = 28;
	  else if (fibernr == 7 || fibernr == 12)
	    ch = 30;
	  else if (fibernr == 1 || fibernr == 8)
	    ch = 32;
	  else if (fibernr == 2 || fibernr == 9)
	    ch = 34;
	  else if (fibernr == 3 || fibernr == 10)
	    ch = 36;
	}
      
      if (ch<28) {
	int value = (ch-1-18)<<8;
	cout << "SC channel " << std::dec << ch << " 0x" << std::hex << value << " " << std::bitset<16>(value) << std::dec << endl;
	VMEout(0x3,LAD_SC+0x1c0000,4,0x800); 
      }
      else {
	int value = (ch-1-27)<<8;
	cout << "S channel " << std::dec << ch << " 0x" << std::hex << value << " " << std::bitset<16>(value) << std::dec << endl;
	VMEout(0x3,LAD_S+0x1c0000,4,0x800); 
      }      
    }
    
    
    //CHANNEL ENABLE --------------
    VMEout( 0x3,LAD_N+0x1a0000, 4,0x1ff); //Disable all channels
    VMEout( 0x3,LAD_NC+0x1a0000,4,0x1ff); 
    VMEout( 0x3,LAD_SC+0x1a0000,4,0x1ff);
    VMEout( 0x3,LAD_S+0x1a0000, 4,0x1ff);
    if (piggyN) { //Piggy North
      
      if (fibernr==4 || fibernr == 5) {
	VMEout( 0x3,LAD_N+0x1a0000, 4,0x1fc); //ch 1&2
      } 
      else if (fibernr==11 || fibernr == 6) {
	VMEout( 0x3,LAD_N+0x1a0000, 4,0x1f3); //ch 3&4
      } 
      else if (fibernr==7 || fibernr ==12 ) {
	VMEout( 0x3,LAD_N+0x1a0000, 4,0x1cf); //ch 5&6
      }  
      else if (fibernr==1 || fibernr == 8) {
	VMEout( 0x3,LAD_N+0x1a0000, 4,0x13f); //ch 7&8
      }  
      else if (fibernr==2 || fibernr == 9) {
	VMEout( 0x3,LAD_N+0x1a0000, 4,0x0ff); //ch 9&10
	VMEout( 0x3,LAD_NC+0x1a0000, 4,0x1fe); 
      }  
      else if (fibernr==3 || fibernr == 10) {
	VMEout( 0x3,LAD_NC+0x1a0000, 4,0x1f9); //ch 11&12
      }
      
    }
    else { //Piggy South
      
      if (fibernr==4 || fibernr == 5) {
	VMEout( 0x3,LAD_SC+0x1a0000, 4,0x13f); //ch 25&26
      } 
      else if (fibernr==11 || fibernr == 6) {
	VMEout( 0x3,LAD_SC+0x1a0000, 4,0x0ff); //ch 27&28
	VMEout( 0x3,LAD_S+0x1a0000, 4,0x1fe); 
      } 
      else if (fibernr==7 || fibernr ==12) {
	VMEout( 0x3,LAD_S+0x1a0000, 4,0x1f9); //ch 29&30
      }  
      else if (fibernr==1 || fibernr == 8) {
	VMEout( 0x3,LAD_S+0x1a0000, 4,0x1e7); //ch 31&32
      }  
      else if (fibernr==2 || fibernr == 9) {
	VMEout( 0x3,LAD_S+0x1a0000, 4,0x1cf); //ch 33&34
      }  
      else if (fibernr==3 || fibernr == 10) {
	VMEout( 0x3,LAD_S+0x1a0000, 4,0x07f); //ch 35&36
      }
      
    }
  }//END FITEL FIBER SELECTION

  return;
}

void SendTriggerAndReadPiggy(bool debug = true) {
  unsigned long int d, data;
  int caenret=0;
  unsigned int caendat=1;
  
  // Reset
  RegResetPiggy();
  Reset(); 

  //************LC needs to be put in again
  // Send Trigger
  caenret = CAENVME_ReadRegister(0,cvScaler1,&caendat);
       
  // values must match with TTCciConfiguration.txt
  //Sequence for BGO 12 = 0xc   Channel #12 (CAL+L1): Private Pixel Command, Send CAL + L1A
  //data=0x9 ;	// sequence for BG0 9 =0x9  START (starts cyclic trigger and cyclic BGO calsync)
  //data=0xc ;  // send CalSync + L1A
  data=0xb;  // bgo 0xb = 11 = single l1a
  //data = 0x9; // bgo 9 = start
  ret = CAENVME_WriteCycle(0,ttcaddcsr0,&data,cvA24_U_DATA,cvD32);//************
  
  //you can call this now when you need a trigger, it should not be called more often than 1/LHC orbit ~90us!
  if(ret != cvSuccess) { 
    cout<<"Error in write TTCci BGO Trigger"<<hex<<ret<<" "<<d<<endl;  
    printf("Error in TTCci trigger!\n");
    analyzeError(ret); 
  }
  
  if (debug)
    printf("Trigger sent!\n");
  usleep(10000);
  //usleep(10);
  		
       
  VMEout(0x3,LAD_C+0x1c0000,4,0x0);  //SLink disable ;Disable SpyMemWrite during readout     
  //VMEout(0x3,LAD_C+0x1c0000,4,0x8);  //SLink ignore LFF
  usleep(10);    
 
  ///////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
  std::string tbm_b("") ; 
  std::string tbm_a("") ; 
 
  if (piggyN) { //Piggy North
    if (debug)
      printf("\nPIGGY North\n");
    for(int i=0;i<256;i++) {
      VMEin(0x3,LAD_N+0x20000,4,&d);
      if (i<42) {
	if (debug) {
	  printf("%04x %04x              ",int((d>>16)&0xffff), int(d&0xffff)); //show in hex
	  BinDisplay((d>>16)&0xffff, 16,0,0); //show in binary
	  printf("    ");
	  BinDisplay(d&0xffff, 16,0,0); //show in binary
	  printf("\n");
	}
	tbm_a.append(BinToString((d>>16)&0xffff, 16)); //binary to string
	tbm_b.append(BinToString(d&0xffff, 16)); //binary to string
      
      }
      
    }
  } 
  else { //Piggy South
    if (debug)
      printf("\nPIGGY South\n");
    for(int i=0;i<256;i++) {
      VMEin(0x3,LAD_S+0x20000,4,&d);
      if (i<42) {
	if (debug) {
	  printf("%04x %04x              ",int((d>>16)&0xffff), int(d&0xffff)); //show in hex
	  BinDisplay((d>>16)&0xffff, 16,0,0); //show in binary
	  printf("    ");
	  BinDisplay(d&0xffff, 16,0,0); //show in binary
	  printf("\n");
	}
	tbm_a.append(BinToString((d>>16)&0xffff, 16)); //binary to string
	tbm_b.append(BinToString(d&0xffff, 16)); //binary to string
	      
      }   
    }
  }
  
  ////////
  ////////
  ////////


  std::string a1 = "1111111111111111";
  std::string b1 = "1111111111111111";
  std::string a2 = "1111111111011111";
  std::string b2 = "1111111110111111";
  std::string a3 = "1111000000000110";
  std::string b3 = "1110000000001110";
  std::string a4 = "1110000111111111";
  std::string b4 = "0000001111111111";
  std::string a5 = "1011100010000000";
  std::string b5 = "0111000100000000";
  std::string a6 = "0111111111111111";
  std::string b6 = "1111111111111111";
  std::string a7 = "1111111111111111";
  std::string b7 = "1111111111111111";

  // tbm_b.append(b1);
  // tbm_b.append(b2); 
  // tbm_b.append(b3);
  // tbm_b.append(b4); 
  // tbm_b.append(b5);
  // tbm_b.append(b6);
  // tbm_b.append(b7);

  // tbm_a.append(a1);
  // tbm_a.append(a2); 
  // tbm_a.append(a3);
  // tbm_a.append(a4); 
  // tbm_a.append(a5);
  // tbm_a.append(a6);
  // tbm_a.append(a7);
  
 

  std::cout << "TBM-A " << std::endl;
  // std::cout << "raw   " << tbm_a << endl;
  decode_dump( tbm_a );

 
  
  std::cout << "TBM-B " << std::endl;
  //std::cout << "raw   " << tbm_b << endl;
  decode_dump( tbm_a );

  /////////
  ////////
  /////////


  return;
}

void InitFitelPiggyN() {

  unsigned long int d;  
  //debug test
  /*VMEout(0x3,LAD_S+0x1a8000,4,0xffffffff); VMEin(0x3,LAD_S+0x1a8000,4,&d);
  printf("written: 0xffffffff   read:%08x\n",d );
  VMEout(0x3,LAD_S+0x1a8000,4,0x0);  VMEin(0x3,LAD_S+0x1a8000,4,&d);
  printf("written: 0x00000000   read:%08x\n",d );
  VMEout(0x3,LAD_S+0x1a8000,4,0x55555555);  VMEin(0x3,LAD_S+0x1a8000,4,&d);
  printf("written: 0x55555555   read:%08x\n",d );
  VMEout(0x3,LAD_S+0x1a8000,4,0xaaaaaaaa);  VMEin(0x3,LAD_S+0x1a8000,4,&d);
  printf("written: 0xaaaaaaaa   read:%08x\n",d );
  *///
  
  // select which fiber is shown at the LEDs
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_selectLEDs + 0x04 );	 // select LEDS for fiber#2 (1..12)   
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " << CHa_CHb_mux + write_selectLEDs + 0x04 << endl;

           
  // reset FITEL------
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_resetFITEL + 0x01 );
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<  CHa_CHb_mux + write_resetFITEL + 0x01 << endl;	 
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_resetFITEL + 0x00 );
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<  CHa_CHb_mux + write_resetFITEL + 0x00 << endl;	 
  //------------------
                    
  //configure FITEL
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x00 );	 // read Memaddr 00
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<  CHa_CHb_mux + write_FITELmemaddr + 0x00 << endl;
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 ); 
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<  CHa_CHb_mux + write_FITELdata + 0x00 << endl;
 
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + FITELreadI2C ); 
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<  CHa_CHb_mux + FITELreadI2C << endl; 
  VMEout (0x3,LAD_N+0x1c0000, 4, 0x4);  
  usleep(100); 
  VMEin(0x3,LAD_N+0x1c0000,4,&d);
  printf("written: 0x4   read:%08x\n",d ); 
  VMEout (0x3,LAD_N+0x1c0000, 4, 0x0);     //  start I2C cycle
  VMEin(0x3,LAD_N+0x1c0000,4,&d);
  printf("written: 0x0   read:%08x\n",d );   
  usleep(1000);
  //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  4 << endl; 
  //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  0 << endl; 
  
   
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x01 );	 // read Memaddr 01
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<  CHa_CHb_mux + write_FITELmemaddr + 0x01 << endl;
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 ); 
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<  CHa_CHb_mux + write_FITELdata + 0x00 << endl;
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + FITELreadI2C );  
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + FITELreadI2C << endl;
  VMEout (0x3,LAD_N+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_N+0x1c0000, 4, 0x0);     //  start I2C cycle  
  usleep(1000);
  //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  4 << endl; 
  //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  0 << endl; 
  
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x02 );	 // read Memaddr 02 
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + write_FITELmemaddr + 0x02 << endl;
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 );
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<  CHa_CHb_mux + write_FITELdata + 0x00  << endl; 
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + FITELreadI2C );   
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<  CHa_CHb_mux + FITELreadI2C  << endl; 
  VMEout (0x3,LAD_N+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_N+0x1c0000, 4, 0x0);     //  start I2C cycle  
  usleep(1000);
  //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  4 << endl; 
  //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  0 << endl; 
  
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x03 );	 // read Memaddr 03 
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<  CHa_CHb_mux + write_FITELmemaddr + 0x03  << endl;
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 ); 
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<  CHa_CHb_mux + write_FITELdata + 0x00  << endl;
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + FITELreadI2C ); 
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<  CHa_CHb_mux + FITELreadI2C  << endl; 
  VMEout (0x3,LAD_N+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_N+0x1c0000, 4, 0x0);     //  start I2C cycle  
  usleep(1000);
  //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  4 << endl; 
  //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  0 << endl; 
  
  
  
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x13 );  // read Memaddr 13 -- device ID
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<  CHa_CHb_mux + write_FITELmemaddr + 0x13  << endl; 
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 ); 
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " << CHa_CHb_mux + write_FITELdata + 0x00  << endl;
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + FITELreadI2C );  
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " << CHa_CHb_mux + FITELreadI2C  << endl;
  VMEout (0x3,LAD_N+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_N+0x1c0000, 4, 0x0);     //  start I2C cycle  
  usleep(1000);
  //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  4 << endl; 
  //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  0 << endl; 
  
  
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x14 );  // read Memaddr 14 -- Temp index 
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<  CHa_CHb_mux + write_FITELmemaddr + 0x14  << endl; 
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 ); 
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + write_FITELdata + 0x00  << endl; 
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + FITELreadI2C );
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + FITELreadI2C  << endl;   
  VMEout (0x3,LAD_N+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_N+0x1c0000, 4, 0x0);     //  start I2C cycle  
  usleep(1000);
  //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  4 << endl; 
  //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  0 << endl; 
  
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x1c );  // ANAMUX => RSSISUM
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + write_FITELmemaddr + 0x1c  << endl; 
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 );
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + write_FITELdata + 0x00  << endl;  
  VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + FITELwriteI2C ); 
  //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + FITELwriteI2C  << endl; 
  VMEout (0x3,LAD_N+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_N+0x1c0000, 4, 0x0);     //  start I2C cycle  
  usleep(1000);
  //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  4 << endl; 
  //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  0 << endl; 
  
 
  
  for(int i=1;i<13;i++) {
    
    VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x2f );
    //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + write_FITELmemaddr + 0x2f  << endl;    
    VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + i );
    //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + write_FITELdata + i << endl;    
    VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + FITELwriteI2C ); 
    //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + FITELwriteI2C << endl;  
    VMEout (0x3,LAD_N+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_N+0x1c0000, 4, 0x0);     //  set PAGE 
    usleep(1000);
    //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  4 << endl; 
    //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  0 << endl; 
    
     VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x39 ); 
    //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + write_FITELmemaddr + 0x39  << endl;    
    VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x20 ); 
    //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + write_FITELdata + 0x20  << endl; 
    VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + FITELwriteI2C );  
    //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + FITELwriteI2C  << endl; 
    VMEout (0x3,LAD_N+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_N+0x1c0000, 4, 0x0);     //  set internalVT 
    usleep(1000);
    //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  4 << endl; 
    //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  0 << endl; 
    
    VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x3b ); 
    //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + write_FITELmemaddr + 0x3b  << endl;    
    VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 );
    //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + write_FITELdata + 0x00  << endl; 
    VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + FITELwriteI2C ); 
    //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + FITELwriteI2C  << endl;  
    VMEout (0x3,LAD_N+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_N+0x1c0000, 4, 0x0);     //  BWCTRL 
    usleep(1000);
    //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  4 << endl; 
    //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  0 << endl; 
    
    VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x30 );
    //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + write_FITELmemaddr + 0x30  << endl;     
    VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 ); 
    //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<    CHa_CHb_mux + write_FITELdata + 0x00  << endl;
    VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + FITELreadI2C ); 
    //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + FITELreadI2C  << endl;   
    VMEout (0x3,LAD_N+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_N+0x1c0000, 4, 0x0);     //  readback IRQ
    usleep(1000);
    //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  4 << endl; 
    //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  0 << endl; 
    
    VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x31 );
    //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + write_FITELmemaddr + 0x31  << endl;   
    VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 ); 
    //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + write_FITELdata + 0x00  << endl;   
   
    VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux + FITELreadI2C ); 
    //cout << "address " << std::hex << LAD_N+0x1a8000 << " data " <<   CHa_CHb_mux + FITELreadI2C  << endl;   
    VMEout (0x3,LAD_N+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_N+0x1c0000, 4, 0x0);     //  readback FAULT
    usleep(1000);
    //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  4 << endl; 
    //cout << "address " << std::hex <<  LAD_N+0x1c0000 << " data " <<  0 << endl; 
    
  }
  usleep(1000);
  printf("\n\n\nPIGGY FITEL I2C readback:      \n\n");
  for(int i=0;i<128;i++)  { // read till FIFO is empty
    VMEin(0x3,LAD_N+0x170000,4,&d);
    if(i<30) {
      //printf("%x\n",d);
      if ((d&0xff) == 0x00)printf(" INT[ 7...0]: %2x \n",int((d>>8)&0xff));
      if ((d&0xff) == 0x01)printf(" INT[15...8]: %2x \n",int((d>>8)&0xff)); 
      if ((d&0xff) == 0x02)printf(" INT[23..16]: %2x \n",int((d>>8)&0xff));
      
      if ((d&0xff) == 0x13)printf(" Device REV:  %2x \n",int((d>>8)&0xff)); 
      
      if ((d&0xff) == 0x30)printf(" fiber#:%2d     SQ:%1x LOS:%1x BSIN:%1x OOP:%1x warning:%1x\n",
				  int((i-4)/2),int((d>>12)&0x1),int((d>>13)&0x1),int((d>>14)&0x1),int((d>>15)&0x1),int((d>>16)&0x1));
      
      //if ((d&0xff) == 0x31)printf(" OOP:%1x BSIN:%1x LOS:%1x SQ:%1x CHen:%1x warning:%1x\n",
      //           (d>>11)&0x1,(d>>12)&0x1,(d>>13)&0x1,(d>>14)&0x1,(d>>15)&0x1,(d>>16)&0x1);
    }
  }
  
  
}


void InitFitelPiggyS() {
  // I2C configuration for FITEl receiver PIGGY SOUTH

  unsigned long int d;
  
  // select which fiber is shown at the LEDs
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_selectLEDs + 0x01 );	 
  
  // 	0x00..fitel#3/10   	0x01..fitel#2/9    	0x02..fitel#1/8 		0x03..fiber#12/7    	0x04..fiber#11/6     0x05..fitel#4/5  
  // 	0x00..zarlink#12  	0x01..zarlink#11   	0x02..zarlink#10 		0x03..zarlink#9  	    0x04..zarlink#8      0x05..zarlink#7  
  
  // reset FITEL------
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_resetFITEL + 0x01 );	  
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_resetFITEL + 0x00 );	  
  //------------------
  
  //configure FITEL
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x00 );	 // read Memaddr 00
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 ); 
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + FITELreadI2C );  
  VMEout (0x3,LAD_SC+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_SC+0x1c0000, 4, 0x0);     //  start I2C cycle  
  usleep(1000);
  
  
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x01 );	 // read Memaddr 01
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 ); 
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + FITELreadI2C );  
  VMEout (0x3,LAD_SC+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_SC+0x1c0000, 4, 0x0);     //  start I2C cycle  
  usleep(1000);
  
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x02 );	 // read Memaddr 02
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 ); 
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + FITELreadI2C );  
  VMEout (0x3,LAD_SC+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_SC+0x1c0000, 4, 0x0);     //  start I2C cycle  
  usleep(1000);
  
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x03 );	 // read Memaddr 03
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 ); 
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + FITELreadI2C );  
  VMEout (0x3,LAD_SC+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_SC+0x1c0000, 4, 0x0);     //  start I2C cycle  
  usleep(1000);
  
  
  
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x13 );  // read Memaddr 13 -- device ID
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 ); 
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + FITELreadI2C );  
  VMEout (0x3,LAD_SC+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_SC+0x1c0000, 4, 0x0);     //  start I2C cycle  
  usleep(1000);
  
  
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x14 );  // read Memaddr 14 -- Temp index
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 ); 
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + FITELreadI2C );  
  VMEout (0x3,LAD_SC+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_SC+0x1c0000, 4, 0x0);     //  start I2C cycle  
  usleep(1000);
  
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x1c );  // ANAMUX => RSSISUM
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 ); 
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + FITELwriteI2C );  
  VMEout (0x3,LAD_SC+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_SC+0x1c0000, 4, 0x0);     //  start I2C cycle  
  usleep(1000);
  
  
  
  
  
  
  for(int i=1;i<13;i++) {
    
    VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x2f );  
    VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + i ); 
    VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + FITELwriteI2C );  
    VMEout (0x3,LAD_SC+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_SC+0x1c0000, 4, 0x0);     //  set PAGE 
    usleep(1000);
    
    VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x39 );  
    VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x20 ); 
    VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + FITELwriteI2C );  
    VMEout (0x3,LAD_SC+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_SC+0x1c0000, 4, 0x0);     //  set internalVT 
    usleep(1000);
    
    VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x3b );  
    VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 ); 
    VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + FITELwriteI2C );  
    VMEout (0x3,LAD_SC+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_SC+0x1c0000, 4, 0x0);     //  BWCTRL 
    usleep(1000);
    
    VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x30 );  
    VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 ); 
    VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + FITELreadI2C );  
    VMEout (0x3,LAD_SC+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_SC+0x1c0000, 4, 0x0);     //  readback IRQ
    usleep(1000);
    
    VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELmemaddr + 0x31 );  
    VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + write_FITELdata + 0x00 ); 
    VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux + FITELreadI2C );  
    VMEout (0x3,LAD_SC+0x1c0000, 4, 0x4);  usleep(100);  VMEout (0x3,LAD_SC+0x1c0000, 4, 0x0);     //  readback FAULT
    usleep(1000);
    
  } 
  usleep(1000);
  printf("\n\n\nPIGGY FITEL I2C readback:      \n\n");
  for(int i=0;i<128;i++)  { // read till FIFO is empty
    VMEin(0x3,LAD_SC+0x170000,4,&d);
    if(i<30) {
      //printf("%x\n",d);
      if ((d&0xff) == 0x00)printf(" INT[ 7...0]: %2x \n",int((d>>8)&0xff));
      if ((d&0xff) == 0x01)printf(" INT[15...8]: %2x \n",int((d>>8)&0xff)); 
      if ((d&0xff) == 0x02)printf(" INT[23..16]: %2x \n",int((d>>8)&0xff));
      
      if ((d&0xff) == 0x13)printf(" Device REV:  %2x \n",int((d>>8)&0xff)); 
      
      if ((d&0xff) == 0x30)printf(" fiber#:%2d     SQ:%1x LOS:%1x BSIN:%1x OOP:%1x warning:%1x\n",
				  int((i-4)/2),int((d>>12)&0x1),int((d>>13)&0x1),int((d>>14)&0x1),int((d>>15)&0x1),int((d>>16)&0x1));
      
      //if ((d&0xff) == 0x31)printf(" OOP:%1x BSIN:%1x LOS:%1x SQ:%1x CHen:%1x warning:%1x\n",
      //           (d>>11)&0x1,(d>>12)&0x1,(d>>13)&0x1,(d>>14)&0x1,(d>>15)&0x1,(d>>16)&0x1);
    }
  }
                
                    
              
}




void exec(SimpleCommand* c){

  int value=0;
  int slot=0;
  int fibersel=0;
  if(c->Keyword("fed", slot)){
    fedSlot=slot;

  }  
   
  else if(c->Keyword("piggyN")){
    piggyN=true;

  }  
  else if(c->Keyword("piggyS")){
    piggyN=false;

  }
  else if(c->Keyword("zarlink")){
    isFitel=false;
  }
  else if(c->Keyword("fitel")){
    isFitel=true;
  }
  else if(c->Keyword("fiber",value)){
    SelectFiber(value);

  }   
  else if(c->Keyword("TBMA")){
    TBMA=true;

  }  
  else if(c->Keyword("TBMB")){
    TBMA=false;

  }  
  else if(c->Keyword("selection")){
    cout << "----------------------------" << endl;
    cout << "fedSlot " << fedSlot << endl;  
    if (piggyN)
      cout << "Piggy North" << endl; 
    else
      cout << "Piggy South" << endl;
    if (isFitel)
      cout << "FITEL receiver" << endl;
    else 
      cout << "ZARLINK receiver" << endl; 
    cout << "fiber " << fiber << endl;  
    if (TBMA)
      cout << "TBM-A" << endl; 
    else
      cout << "TBM-B" << endl;
    cout << "----------------------------" << endl;
  

  }
  else if(c->Keyword("firmware")){
    ReadFirmwareVersion();

  } 
  //**********
  else if(c->Keyword("setupTTC")){ //commented to avoid conflict LC
    SetupTTC();

  } 
  //**********
  else if(c->Keyword("reset")){
    Reset();

  } 
  else if(c->Keyword("regreset")){
    RegReset();

  }
  else if(c->Keyword("PLLreset")){
    PLLResetFrontFPGA();

  }else if(c->Keyword("PLLresetPiggy")){
    PLLResetPiggy();

  }else if(c->Keyword("cfgFront")){
    ConfigureAlteraFrontDevices();

  }else if(c->Keyword("cfgCenter")){
    ConfigureAlteraCenterDevice();

  }else if(c->Keyword("cfgPiggy")){
    ConfigurePiggy();

  }else if(c->Keyword("trigger")){
    SendTrigger(10);

  }else if(c->Keyword("trigger",value)){
    SendTrigger(value);

  }else if(c->Keyword("readStatusPiggy")){
    ReadDaughterCardStatus();

  }else if(c->Keyword("casea")){
    RunCasea();

  }else if(c->Keyword("getFromPiggy")){
    SendTriggerAndReadPiggy();

  }else if(c->Keyword("getAll")){
    SendTriggerAndReadAll();

  }else if(c->Keyword("initFitelN")){
    InitFitelPiggyN();
  }
  else if(c->Keyword("initFitelS")){
    InitFitelPiggyS();
  }
  else if(c->Keyword("usefibers",fibersel)){
    FitelUseFibers(fibersel);
  }
  else{
    cout << "what??  <" << *c << ">" << endl;
  }

  
}



int main(int argc, char *argv[]) {

  unsigned long int d;
  
  // unsigned long int d,dd,du,data,idata,i,j,k[240],index,s,df[32768],da[1024],empty,ECnt,BCnt;
  // int IP_HEADER[10],input,l,fiber;
  // unsigned long int errorcount,output_delay,type,dcol,row,ph,roc,tbm_eventnr,tbm_errorword,D_up[1024],D_down[1024];
  // unsigned long int ErrorFIFO_down[256],Old_Data,Vup[1024],Vdown[1024];
  // unsigned long int destaddr[1024],eventnum;
  // char c,reply,rplsub1;
  // char str[10];
  // FILE*Unfold_data; 
  // int caenret=0;
  // unsigned int caendat=1; unsigned int caendat1=1;

  //memset (destaddr,0,sizeof(destaddr)); 
  CLOCKsel[0]=0x0;  CLOCKsel[1]=0x0;  CLOCKsel[2]=0x0;  CLOCKsel[3]=0x0;   
  CHIP[0]=FEDBASE;  CHIP[1]=FEDBASE+0x200000;  CHIP[2]=FEDBASE+0x400000;  CHIP[3]=FEDBASE+0x600000;
  
  CH_SubAddr[0]=0x38000;  CH_SubAddr[1]= 0x58000;  CH_SubAddr[2]= 0x78000;
  CH_SubAddr[3]=0x98000;  CH_SubAddr[4]= 0xb8000;  CH_SubAddr[5]= 0xd8000;
  CH_SubAddr[6]=0xf8000;  CH_SubAddr[7]=0x118000;  CH_SubAddr[8]=0x138000;
  
  printf("\n");
  // Open interface
  if( CAENVME_Init(VMEBoard, Device, Link, &BHandle) != cvSuccess ) {
    cout<<" Error opening the device"<<cvSuccess<<endl;
    exit(1);
  }
  
  char FWrRel[3];
  ret = CAENVME_BoardFWRelease(BHandle,FWrRel);
  if(ret != cvSuccess) {  // Error
    cout<<"Error in accessing VME "<<hex<<ret<<" CAEN firmware check failed "<<endl;
    analyzeError(ret); }
  cout<<"CAEN VME Board Firmware release "<<FWrRel<<endl;
  
  printf("Digital FED -------\n");   
 
  
  //Resets
  Reset();
  VMEout(0x3,LAD_C +0x1c8000,4,0x20000000);   //Reset SLink 80MHz PLL
   
  //Set number of ROCs
  d=8;
  for(int is=1;is<10;is++){
    VMEout(0x3,(LAD_N +(is<<17)),4,d);
    VMEout(0x3,(LAD_NC +(is<<17)),4,d);
    VMEout(0x3,(LAD_S +(is<<17)),4,d);
    VMEout(0x3,(LAD_SC +(is<<17)),4,d);
    
  }

  ReadFirmwareVersion();
  
  // Read ModuleSerialNumber and pass to CenterChip
  VMEin(0x3,READ_MA,4,&d);	
  //printf("\n\nMA= %d\n",d&0x3f);
  VMEout(0x3,LAD_C+0x0e0000,4,d&0x3f);      
  
  //************commented to avoid conflit LC
  ////set TTC settings
  //SetupTTC();
  //***************

  //reset PLL
  PLLResetFrontFPGA();


  //What is this?
  VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux);   //  a_b_mux=1 for use with old pattern generator DTB     
  VMEout (0x3,LAD_NC+0x1a8000, 4, CHa_CHb_mux);

  //Other settings
  VMEout (0x3,LAD_C+0x1a0000,4,0x10014);   //set the gdmf control register
  VMEout (0x3,LAD_C+0x1d8000,4,0x0); // Trigger delay in the center chip

  
  // parse command line arguments
  int port=0; 
  for(int i=1; i<argc; i++){
    if (strcmp(argv[i],"-port")==0){
      i++;
      try{
        port=atoi(argv[i]);
      }catch(...){
        cerr << "bad port number " << argv[i] << endl;
        exit(1);
      }
    }else{
      cerr << "usage: fed [-port <port>] " << endl;
      exit(1);
    }
  }

  cout << "using port " << port << endl;
 
  
  VMELock lock(1);
  lock.acquire();


  //ADD SOCKETS
  SimpleCommand* cmd=new SimpleCommand(); 
  lock.release();
  MultiplexingServer serv(cmd, false);
  if(port){ serv.open(port); }
  cout << "done initializing server " << endl;


  // go
  while(serv.eventloop() ){
    lock.acquire();
    exec(cmd);
    lock.release();   
  }



/*start:; ; 
 printf("TEST Loops ...................... ( 1 )\n");
 printf("nCONFIG  ........................ ( 2 )\n");
 printf("nCONFIG CENTER .................. ( 3 )\n");
 printf("                                       \n");
 printf("END ............................. ( 0 )\n");

 ///reply = GetKey ();
 fgets(str,10,stdin);
 reply = str[0];
 
 
 VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux);   //  a_b_mux=1 for use with old pattern generator DTB     
 VMEout (0x3,LAD_NC+0x1a8000, 4, CHa_CHb_mux);

 switch(reply) {
   case'1':{
   sub1:; ;
   printf("\n\n\n");
   
   printf("   PIGGY nconfig ...... ............... (2)\n"); 
   printf("   PLLreset ...... .................... (3)\n"); 
   printf("   REGreset  ...... ................... (4)\n"); 
   printf("   Send Trigger ............. ......... (5)\n");
   printf("   Read Daughter Card Status w/ ZARLINK (6)\n");
   printf("   FED Reset                            (7)\n");
   printf("                                           \n");    
   printf("   ReadFED ...... ..................... (a)\n"); 
   printf("   Eventnumber loop ...... ............ (b)\n"); 
   printf("   Emulator ..........     ............ (c)\n");
   printf("   Event loop ......................... (w)\n"); 
   
   printf("                                           \n");
   printf("   BACK TO MAIN ....................... (0)\n");
  ///rplsub1 = GetKey (); 
   fgets(str,10,stdin);
   rplsub1 = str[0];
   
   switch(rplsub1) {
     case'2':{ 
       VMEout (0x3,LAD_S+0x1b8000, 4, 0x10);   //  nConfig PIGGY bottom
       usleep(10000);
       VMEout (0x3,LAD_S+0x1b8000, 4, 0x00); 
       
       VMEout (0x3,LAD_N+0x1b8000, 4, 0x80);   //  nConfig PIGGY top
       usleep(10000);
       VMEout (0x3,LAD_N+0x1b8000, 4, 0x00); 
       usleep(10000);    
       VMEout (0x3,LAD_N+0x190000, 4, 0x7f00);   //  nConfig PIGGY bottom
       cout << "written DACs" << endl;
       
     };break; 
     
     case'3':{ 
       VMEout (0x3,LAD_S+0x1c0000, 4, 0x1);   //  PLLareset  PIGGY bottom   
       usleep(10);
       VMEout (0x3,LAD_S+0x1c0000, 4, 0x0); 
       
       VMEout (0x3,LAD_NC+0x1c0000, 4, 0x1);   //  PLLareset PIGGY top 
       usleep(10);
       VMEout (0x3,LAD_NC+0x1c0000, 4, 0x0); 
       
     };break;  
     
     case'4':{ 
       VMEout (0x3,LAD_S+0x1c0000, 4, 0x2);   //  REGreset PIGGY bottom   
       usleep(10);
       VMEout (0x3,LAD_S+0x1c0000, 4, 0x0); 
       
       VMEout (0x3,LAD_NC+0x1c0000, 4, 0x2);   //  REGreset PIGGY top   
       usleep(10);
       VMEout (0x3,LAD_NC+0x1c0000, 4, 0x0);
       
       ///////init FED //////////////
       
       VMEout(0x3,LRES,4,0x80000000);  
       VMEout(0x3,CLRES,4,0x80000000); 
       
       VMEout(0x3,LAD_C +0x1c8000,4,0x20000000);   //Reset SLink 80MHz PLL
       
       for(j=0;j<4;j++) {  for(i=1;i<10;i++)  CLOCK_DEL_V2( j,i,0x4); } 
       usleep(10);
       
       VMEout( 0x3,LAD_N+0x1a0000,4,0x1ff); //Skip Channel Readout N
       VMEout(0x3,LAD_NC+0x1a0000,4,0x1ff); //Skip Channel Readout NC
       VMEout(0x3,LAD_SC+0x1a0000,4,0x1ff); //Skip Channel Readout SC  0x1fe
       VMEout( 0x3,LAD_S+0x1a0000,4,0x1ff);   //Skip Channel Readout S   0x19f
       
       VMEout(0x3,LAD_N +0x1c8000,4,0x80000000); // Flush SPY FIFOs N
       VMEout(0x3,LAD_NC+0x1c8000,4,0x80000000); // Flush SPY FIFOs NC
       VMEout(0x3,LAD_SC+0x1c8000,4,0x80000000); // Flush SPY FIFOs SC
       VMEout(0x3,LAD_S +0x1c8000,4,0x80000000); // Flush SPY FIFOs S
       
       //VXIout(0x/break
       //3,LAD_C+0x1c0000,4,0xa);  //SLink disable ;Disable SpyMemWrite during readout       
      cout<<"enabling ready?"<<endl; 
       VMEout(0x3,LAD_C+0x1a0000,4,0x10014); // 0-TTCEn-0-EnableDAC-0-0			enable TTC Trigger  
       l=0;
     };break;
     
     case'5':{
       {int ttcrxi=5;
       for(int i=0;i<100;i++){
       cout<<" input TTCrx setting "<<endl;
       cin>>ttcrxi;

        TTCRX_I2C_REG_WRITE( 1, ttcrxi);
                              }
       }
       VMEout(0x3,LAD_C+0x1a0000,4,0x14); // 0-TTCEn-0-EnableDAC-0-0			TTC Trigger  
       
       //if(!ttcconfigured) {
       //	 printf("setup TTCci\n");
       //	 //setup TTcci
       //	 system("/home/cmspixels/build/TriDAS/TTCSoftware/bin/RunTTCciControl.exe -s 2 -c ./TTCciConfiguration.txt -q &> /dev/null");
       //	 ttcconfigured=true;
       //}
       
       // values must match with TTCciConfiguration.txt
       //Sequence for BGO 12 = 0xc   Channel #12 (CAL+L1): Private Pixel Command, Send CAL + L1A
       //data=0x9 ;	// sequence for BG0 9 =0x9  START (starts cyclic trigger and cyclic BGO calsync)
       
       for ( i=0; i <= 100000 ; i++){
	 data=0xb ; // send single L1A
	 //			data=0xc ;                      // send CalSync + L1A
	 usleep(100);
	 ret = CAENVME_WriteCycle(0,ttcaddcsr0,&data,cvA24_U_DATA,cvD32);
	 
	 //you can call this now when you need a trigger, it should not be called more often than 1/LHC orbit ~90us!
	 if(ret != cvSuccess) { 
	   //cout<<"Error in write TTCci BGO Trigger"<<hex<<ret<<" "<<d<<endl;  
	   printf("Error in TTCci trigger!\n");
	   analyzeError(ret); 
	 }
	 cout << i << endl;
       }
     };break;
                     
     case'6':{ // This is for ZARLINK version
       VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux+0x2546);   //  NORTH PIGGYcom   
       VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux+0x2343);   //  SOUTH PIGGYcom   
       //  a_b_mux=0x80000000 for use with old pattern generator DTB  
       //  a_b_mux=0x00000000 for use with new module   
       
       //  bit 13  ... start PIGGYcom
       //  bit 12  ... mode bit
       //  bit 11..8   addr bits
       //  bit  7..1   data bits
       
       printf("\n\n\nPIGGYstatus NORTHup     CH#1 / 2     CH#3 / 4     CH#5 / 6   locked400 \n\n");
       for(i=0;i<16;i++)  {
	 VMEin(0x3,LAD_N+0x158000,4,&d);
	 printf("                               %2x         %2x          %2x         %1d\n", (d)&0xff, (d>>8)&0xff, (d>>16)&0xff, (d>>24)&0x1);
       }
       
       printf("\n\n\nPIGGYstatus NORTHdown     CH#7 / 8     CH#9 /10     CH#11/12   locked400 \n\n");
       for(i=0;i<16;i++)  {
	 //usleep(1000000);
	 VMEin(0x3,LAD_N+0x178000,4,&d);
	 printf("                               %2x         %2x          %2x         %1d\n", (d)&0xff, (d>>8)&0xff, (d>>16)&0xff, (d>>24)&0x1);
       }
       
       printf("\n\n\nPIGGYstatus SOUTHup     CH#25/26     CH#27/28     CH#29/30     locked400 \n\n");
       for(i=0;i<16;i++)  {
	 VMEin(0x3,LAD_S+0x158000,4,&d);
	 printf("                               %2x         %2x          %2x         %1d\n", (d)&0xff, (d>>8)&0xff, (d>>16)&0xff, (d>>24)&0x1);
       }
       
       printf("\n\n\nPIGGYstatus SOUTHdown   CH#31/32     CH#33/34     CH#35/36     locked400 \n\n");
       for(i=0;i<16;i++)  {
	 VMEin(0x3,LAD_S+0x178000,4,&d);
	 printf("                               %2x         %2x          %2x         %1d\n", (d)&0xff, (d>>8)&0xff, (d>>16)&0xff, (d>>24)&0x1);
       }
       
     };break;
     
   case'p':{ // This is for ZARLINK version
     VMEout (0x3,LAD_N+0x1a8000, 4, CHa_CHb_mux+0x2546);   //  NORTH PIGGYcom   
     VMEout (0x3,LAD_S+0x1a8000, 4, CHa_CHb_mux+0x2343);   //  SOUTH PIGGYcom   
     //  a_b_mux=0x80000000 for use with old pattern generator DTB  
     //  a_b_mux=0x00000000 for use with new module   
     
     //  bit 13  ... start PIGGYcom
     //  bit 12  ... mode bit
     //  bit 11..8   addr bits
     //  bit  7..1   data bits
     
     unsigned long int phase[160];
     int p(-9);
     int gph_5(0); int gph_4(0); int gph_3(0); int gph_2(0); int gph_1(0); 
     printf("\n\n\nPIGGYstatus NORTHup     CH#1 / 2     CH#3 / 4     CH#5 / 6   locked400 \n\n");
     for(i=0;i<16;i++)  {
       VMEin(0x3,LAD_N+0x158000,4,&d);
       printf("                               %2x         %2x          %2x         %1d\n", (d)&0xff, (d>>8)&0xff, (d>>16)&0xff, (d>>24)&0x1);
     }
     
     printf("\n\n\nPIGGYstatus NORTHdown     CH#7 / 8     CH#9 /10     CH#11/12   locked400 \n\n");
     for(i=0;i<160;i++)  {
       usleep(1000000);
       VMEin(0x3,LAD_N+0x178000,4,&d);
       printf("                               %2x         %2x          %2x         %1d\n", (d)&0xff, (d>>8)&0xff, (d>>16)&0xff, (d>>24)&0x1);
       phase[i] = (d>>8)&0xff;
     }
     printf(" phase     (phase[i-1])&(phase[i])\n");
     printf("  %2x                \n", phase[0]);
     for(i=1;i<160;i++)  {
       if ( phase[i] == 0 ) break;
       
       if ( ((phase[i])&0xff == 0x1c) || ((phase[i])&0xff == 0x0e) || ((phase[i])&0xff == 0x07) || ((phase[i])&0xff == 0x83) || ((phase[i])&0xff == 0xc1) || ((phase[i])&0xff == 0xe0) || ((phase[i])&0xff == 0x70) || ((phase[i])&0xff == 0x38)) gph_5++;
       if ( ((phase[i])&0xff == 0x1e) || ((phase[i])&0xff == 0x0f) || ((phase[i])&0xff == 0x87) || ((phase[i])&0xff == 0xc3) || ((phase[i])&0xff == 0xe1) || ((phase[i])&0xff == 0xf0) || ((phase[i])&0xff == 0x78) || ((phase[i])&0xff == 0x3c)) gph_4++;
       if ( ((phase[i])&0xff == 0x1f) || ((phase[i])&0xff == 0x8f) || ((phase[i])&0xff == 0xc7) || ((phase[i])&0xff == 0xe3) || ((phase[i])&0xff == 0xf1) || ((phase[i])&0xff == 0xf8) || ((phase[i])&0xff == 0x7c) || ((phase[i])&0xff == 0x3e)) gph_3++;
       if ( ((phase[i])&0xff == 0x3f) || ((phase[i])&0xff == 0x9f) || ((phase[i])&0xff == 0xcf) || ((phase[i])&0xff == 0xe7) || ((phase[i])&0xff == 0xf3) || ((phase[i])&0xff == 0xf9) || ((phase[i])&0xff == 0xfc) || ((phase[i])&0xff == 0x7e)) gph_2++;
       if ( ((phase[i])&0xff == 0x7f) || ((phase[i])&0xff == 0xbf) || ((phase[i])&0xff == 0xdf) || ((phase[i])&0xff == 0xef) || ((phase[i])&0xff == 0xf7) || ((phase[i])&0xff == 0xfb) || ((phase[i])&0xff == 0xfd) || ((phase[i])&0xff == 0xfe)) gph_1++;
       
       if ( (phase[i] == 28) || (phase[i] == 14) || (phase[i] == 7) || (phase[i] == 131) || (phase[i] == 193) || (phase[i] == 224) || (phase[i] == 112) || (phase[i] == 56) ) gph_5++;
       if ( (phase[i] == 30) || (phase[i] == 15) || (phase[i] == 135) || (phase[i] == 195) || (phase[i] == 225) || (phase[i] == 240) || (phase[i] == 120) || (phase[i] == 60) ) gph_4++;
       if ( (phase[i] == 31) || (phase[i] == 143) || (phase[i] == 199) || (phase[i] == 227) || (phase[i] == 241) || (phase[i] == 248) || (phase[i] == 124) || (phase[i] == 62) ) gph_3++;
       if ( (phase[i] == 63) || (phase[i] == 159) || (phase[i] == 207) || (phase[i] == 231) || (phase[i] == 243) || (phase[i] == 249) || (phase[i] == 252) || (phase[i] == 126) ) gph_2++;
       //if ( (phase[i] == 127) || (phase[i] == 191) || (phase[i] == 223) || (phase[i] == 239) || (phase[i] == 247) || (phase[i] == 251) || (phase[i] == 253) || (phase[i] == 254) ) gph_1++;
       
       
       printf("  %2x             %2x\n", phase[i], (phase[i-1])|(phase[i]));
       phase[i] = (phase[i-1])|(phase[i]);
       p=i;
     }
     printf("OR of all phases  %2x                \n", phase[p]);
     printf("gph_5  %2d                \n", gph_5); printf("gph_4  %2d                \n", gph_4);
     printf("gph_3  %2d                \n", gph_3); printf("gph_2  %2d                \n", gph_2);
     //printf("gph_1  %2d                \n", gph_1);
     
     printf("\n\n\nPIGGYstatus SOUTHup     CH#25/26     CH#27/28     CH#29/30     locked400 \n\n");
     for(i=0;i<16;i++)  {
       VMEin(0x3,LAD_S+0x158000,4,&d);
       printf("                               %2x         %2x          %2x         %1d\n", (d)&0xff, (d>>8)&0xff, (d>>16)&0xff, (d>>24)&0x1);
     }
     
     printf("\n\n\nPIGGYstatus SOUTHdown   CH#31/32     CH#33/34     CH#35/36     locked400 \n\n");
     for(i=0;i<16;i++)  {
       VMEin(0x3,LAD_S+0x178000,4,&d);
       printf("                               %2x         %2x          %2x         %1d\n", (d)&0xff, (d>>8)&0xff, (d>>16)&0xff, (d>>24)&0x1);
     }
     
   };break; 
     
   case'7':{      
     VMEout(0x3,LRES,4,0x80000000);  
     VMEout(0x3,CLRES,4,0x80000000); 
     usleep(1000);
     
   };break;    
     
     
     case'a':{   
       
        // THIS IS REGRESET for SPYING
       VMEout (0x3,LAD_S+0x1c0000, 4, 0x2);   //  REGreset PIGGY bottom   
       usleep(10);
       VMEout (0x3,LAD_S+0x1c0000, 4, 0x0); 
       
       VMEout (0x3,LAD_NC+0x1c0000, 4, 0x2);   //  REGreset PIGGY top   
       usleep(10);
       VMEout (0x3,LAD_NC+0x1c0000, 4, 0x0);
       //////
       
       
       
       int count(0);
       for(i=0;i<10;i++)  {
	 
	 //VMEin(0x3,LAD_S+0x178000,4,&d);
	 VMEin(0x3,LAD_N+0x178000,4,&d);
	 printf("                               %2x         %2x          %2x         %1d\n", (d)&0xff, (d>>8)&0xff, (d>>16)&0xff, (d>>24)&0x1);
	 count+=(d>>24)&0x1;
       }
       
       cout << "# of times locked = "  << count << endl;
       
       VMEout(0x3,LRES,4,0x80000000); 
       VMEout(0x3,CLRES,4,0x80000000); 
       
       count=0;
       for(i=0;i<10;i++)  {
	 
	 //VMEin(0x3,LAD_S+0x178000,4,&d);
	 VMEin(0x3,LAD_N+0x178000,4,&d);
	 printf("                               %2x         %2x          %2x         %1d\n", (d)&0xff, (d>>8)&0xff, (d>>16)&0xff, (d>>24)&0x1);
	 count+=(d>>24)&0x1;
       }
       cout << "after the FED reset # of times locked = "  << count << endl;

       
       //int myenable=0;
       //cout<<"input south enable word"<<endl;
       //cin>>hex>>myenable;
       
       ////VMEout( 0x3,LAD_N+0x1a0000,4,0x000); //Enable all 9 channels from North FPGA
       
       ////VMEout(0x3,LAD_NC+0x1a0000,4,0x1f8); //FOR NORTH PIGGY CARD //Enable first 3 channels from NorthCenter FPGA
       ////VMEout(0x3,LAD_NC+0x1a0000,4,0x1ff); //Disable all 9 channels from NorthCenter FPGA
       
       ////VMEout(0x3,LAD_SC+0x1a0000,4,0x1ff); //Disable all 9 channels from NorthCenter FPGA
       ////VMEout( 0x3,LAD_S+0x1a0000,4,myenable);   //Enable all 9 channels from South FPGA
       
       ////VMEout( 0x3,LAD_S+0x1a0000,4,0x7f); // FOR SOUTH PIGGY CARD  //Enable all 9 channels from South FPGA
       ////VMEout( 0x3,LAD_S+0x1a0000,4,0x1ff);   //Disable all 9 channels from South FPGA
       
       // North Piggy all chs enabled 
       //VMEout( 0x3,LAD_N+0x1a0000,4,0x030); // ch1/2, 3/4, 7/8, 9 enabled
/////	VMEout( 0x3,LAD_N+0x1a0000,4,0x1fc); // ch 1&2 enabled
       //VMEout(0x3,LAD_NC+0x1a0000,4,0x1fe); // ch10
/////       VMEout(0x3,LAD_NC+0x1a0000,4,0x1ff); //ch 10 disabled
       //VMEout(0x3,LAD_SC+0x1a0000,4,0x1ff);
       //VMEout( 0x3,LAD_S+0x1a0000,4,0x1ff); 
       
       //this is what we used when Bora and Will where here
       // North Piggy all chs enabled 
       // VMEout( 0x3,LAD_N+0x1a0000,4,0x0ff); 
       // VMEout(0x3,LAD_NC+0x1a0000,4,0x1f8); // ch11&12
       // VMEout(0x3,LAD_SC+0x1a0000,4,0x1ff);
       // VMEout( 0x3,LAD_S+0x1a0000,4,0x1ff);  
       

       // North Piggy all chs enabled 
       // VMEout( 0x3,LAD_N+0x1a0000,4,0x1cf); // ch 5&6
       // VMEout(0x3,LAD_NC+0x1a0000,4,0x1ff); 
       // VMEout(0x3,LAD_SC+0x1a0000,4,0x1ff);
       // VMEout( 0x3,LAD_S+0x1a0000,4,0x1ff);   

       // VMEout( 0x3,LAD_N+0x1a0000,4,0x1ff); 
       // VMEout( 0x3,LAD_NC+0x1a0000,4,0x1ff); //all disabled
       // VMEout( 0x3,LAD_SC+0x1a0000,4,0x1ff);
       // VMEout( 0x3,LAD_S+0x1a0000,4,0x1ff); 
       
       
       VMEout( 0x3,LAD_N+0x1a0000,4,0x0ff); //TEMP FIFO
       VMEout(0x3,LAD_NC+0x1a0000,4,0x1fe); //ch 9&10
       VMEout(0x3,LAD_SC+0x1a0000,4,0x1ff);
       VMEout( 0x3,LAD_S+0x1a0000,4,0x1ff);


       // South Piggy all chs enabled
       //VMEout( 0x3,LAD_N+0x1a0000,4,0x1ff);
       //VMEout(0x3,LAD_NC+0x1a0000,4,0x1ff);
       //VMEout(0x3,LAD_SC+0x1a0000,4,0x03f);
       //VMEout( 0x3,LAD_S+0x1a0000,4,0x186); 
       
       // South Piggy ch #11 enabled
       //VMEout( 0x3,LAD_N+0x1a0000,4,0x1ff);
       //VMEout(0x3,LAD_NC+0x1a0000,4,0x1ff);
       //VMEout(0x3,LAD_SC+0x1a0000,4,0x1ff);
       //VMEout(0x3,LAD_S+0x1a0000,4,0x19f); // ch#11 enabled
       
       
       // Both North and South Piggy enabled
       //VMEout( 0x3,LAD_N+0x1a0000,4,0x000);
       //VMEout(0x3,LAD_NC+0x1a0000,4,0x1f8);
       //VMEout(0x3,LAD_SC+0x1a0000,4,0x03f);
       //VMEout( 0x3,LAD_S+0x1a0000,4,0x000); 
       

       VMEout( 0x3,LAD_C+0x1a0000,4,0x10014);   //set the gdmf control register
       VMEout(0x3,LAD_C+0x1d8000,4,0x0); // Trigger delay in the center chip
	
       count=0;

       // Turn on the channel to spy on  bits 8-11 in the mode register for the channel
       //also msk somr trailer error bits (<<24)
       //????????????????????????????????????
       VMEout(0x3,LAD_N+0x1c0000,4,((0x2)<<24)+(0x8<<8));  //North Mode register     
       VMEout(0x3,LAD_NC+0x1c0000,4,((0x2)<<24)+(0x0<<8));  //NorthCenter Mode register     
       ///////////////////////////////////////////////////////////
       //  bit  0  1  2  3  4  5  6  7  8   N 
       //  ch   1  2  3  4  5  6  7  8  9  NC
       //  ch   10 11 12 13 14 15 16 17 18
       //  ch   
       ///////////////////////////////////////////////////////////
       // VMEout(0x3,LAD_N+0x1c0000,4,0x800); 
       // For Up Piggy mode register bit [11..8] = 5 (000 for ch1, 001 for ch2, 002 for ch3, 003 for ch4, 004 for ch5, 005 for ch6, 006 for ch7, 007 for ch8, 008 for ch9) 
       // VMEout(0x3,LAD_N+0x1c0000,4,(0x800));
       // VMEout(0x3,LAD_NC+0x1c0000,4,(0x000)); 
       // For Up Piggy mode register bit [11..8] = 5 (000 for ch10, 100 for ch11, 200 for ch12, 300 for ch13, 400 for ch14, 500 for ch15, 600 for ch16, 700 for ch17, 800 for ch18)

       // VMEout(0x3,LAD_S+0x1c0000,4,0x600);
       // For Down Piggy  mode register bit [11..8] = 5 (0 for ch28, 1 for ch29, 2 for ch30, 3 for ch31, 4 for ch32, 5 for ch33, 6 for ch34, 7 for ch35, 8 for ch36)
       ///////////////////////////////////////////////////////////
       
       ///////////////////////////////////////////////////////////

       
       //channel selection for PIGGY       
       //this choose the channel read by piggy: ex. CHa_CHb_mux+0x2304 reads channel 4
        VMEout(0x3,LAD_N+0x1a8000,4,CHa_CHb_mux+0x2304); // ch selection for transparent mode: 5-fiber#12, 4-fiber#11, 3-fiber#10, 2-fiber#9, 1-fiber# 8, 0-fiber#7
	// VMEout(0x3,LAD_S+0x1a8000,4,CHa_CHb_mux+0x2304); // ch selection for transparent mode: 5-fiber#12, 4-fiber#11, 3-fiber#10, 2-fiber#9, 1-fiber# 8, 0-fiber#7
       
       // VMEout(0x3,LAD_N+0x1a8000,4,CHa_CHb_mux+0x2302); // ch selection for transparent mode: 5-fiber#12, 4-fiber#11, 3-fiber#10, 2-fiber#9, 1-fiber# 8, 0-fiber#7
       // VMEout(0x3,LAD_NC+0x1a8000,4,CHa_CHb_mux+0x2303); // ch selection for transparent mode: 5-fiber#12, 4-fiber#11, 3-fiber#10, 2-fiber#9, 1-fiber# 8, 0-fiber#7


       for(i=0;i<10;i++)  {
	 
	 //VMEin(0x3,LAD_S+0x178000,4,&d);
	 VMEin(0x3,LAD_N+0x178000,4,&d);
	 printf("                               %2x         %2x          %2x         %1d\n", (d)&0xff, (d>>8)&0xff, (d>>16)&0xff, (d>>24)&0x1);
	 count+=(d>>24)&0x1;
       }
       cout << "after disabling the unused channels  # of times locked = "  << count << endl;

       
       
       //       VMEout (0x3, TTCVIBASE+0x86, 2, 1); // send L1A Trigger  with the TTCvi (no longer used)
       
       
       
       int mdumm=0;
       //cout<<"enter an integer to continue"<<endl;
      //cin>>mdumm; 
       //if(!ttcconfigured) {
       //	 printf("setup TTCci\n");
       //	 //setup TTcci
       //	 system("/home/cmspixp1/build/TriDAS/TTCSoftware/bin/RunTTCciControl.exe -s 2 -c ./TTCciConfiguration.txt -q &> /dev/null");
       //	 ttcconfigured=true;
       //}
       
       caenret = CAENVME_ReadRegister(0,cvScaler1,&caendat);
       
       // values must match with TTCciConfiguration.txt
       //Sequence for BGO 12 = 0xc   Channel #12 (CAL+L1): Private Pixel Command, Send CAL + L1A
       //data=0x9 ;	// sequence for BG0 9 =0x9  START (starts cyclic trigger and cyclic BGO calsync)
       //data=0xc ;  // send CalSync + L1A
       data=0xb;  // bgo 0xb = 11 = single l1a
       //data = 0x9; // bgo 9 = start
       ret = CAENVME_WriteCycle(0,ttcaddcsr0,&data,cvA24_U_DATA,cvD32);
       
       //you can call this now when you need a trigger, it should not be called more often than 1/LHC orbit ~90us!
       if(ret != cvSuccess) { 
	 cout<<"Error in write TTCci BGO Trigger"<<hex<<ret<<" "<<d<<endl;  
	 printf("Error in TTCci trigger!\n");
	 analyzeError(ret); 
       }
              
       printf("Trigger sent!\n");
       usleep(10000);
       caenret = CAENVME_ReadRegister(0,cvScaler1,&caendat1);
       //cout<<"scaler"<<caendat<<endl;
       //cout<<"scaler after trigger"<<caendat1<<endl;
       
       usleep(100);
       // s=256;
       //VMEmove(0x1003,LAD_S+0x178000,0,destaddr,s,4); printf("\n\n");
       
       //printf("\n\n|||||CH|RO|DC|PXL|PH|        Data:\n");
       //for(i=0;i<60;i++)
       //	 printf("%2d:  %2d %2d %2d %3d %2x %x\n",i,(destaddr[i]>>26)&0x3f,(destaddr[i]>>21)&0x1f,(destaddr[i]>>16)&0x1f,(destaddr[i]>>8)&0xff,destaddr[i]&0xff,destaddr[i]);
       
       //s=1024;
       //VMEmove(0x1003,LAD_NC+0x150000,0,destaddr,s,4); printf("\n\n");
       //for(i=0;i<20;i++) printf("%3x",destaddr[i]); printf("\n\n");   
       ///ProcessSystemEvents ();
       
       printf("\n\n\nTEMP FIFO Sup\n\n");
       for(i=0;i<256;i++)  { 
	 VMEin(0x3,LAD_S+0x148000,4,&d);	
	 ///////VMEin(0x3,LAD_N+0x168000,4,&d);
	 if(d) { 
	   printf("%x\n",d); 
	   printf("CH#:%2d",((d>>26)&0x3f));
	   if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",((d>>1)&0xff00)+(d&0xff));
	   if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",((d>>4)&0xff00)+(d&0xff));    
	 }
       }	
   
       printf("\n\n\nTEMP FIFO Sdown\n\n");
       for(i=0;i<256;i++)  { 
	 VMEin(0x3,LAD_S+0x168000,4,&d);	
	 ///////VMEin(0x3,LAD_N+0x168000,4,&d);
	 if(d) { 
	   printf("%x\n",d); 
	   printf("CH#:%2d",((d>>26)&0x3f));
	   if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",((d>>1)&0xff00)+(d&0xff));
	   if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",((d>>4)&0xff00)+(d&0xff));    
	 }
       }	
   
       printf("\n\n\nTEMP FIFO NCup\n\n");
       for(i=0;i<256;i++)  { 
	 VMEin(0x3,LAD_NC+0x148000,4,&d);	
	 ///////VMEin(0x3,LAD_N+0x168000,4,&d);
	 if(d) { 
	   printf("check %x\n",d); 
	   printf("CH#:%2d",((d>>26)&0x3f));
	   if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",((d>>1)&0xff00)+(d&0xff));
	   if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",((d>>4)&0xff00)+(d&0xff));    
	 }
       }	
   
       printf("\n\n\nTEMP FIFO NCdown\n\n");
       for(i=0;i<256;i++)  { 
	 VMEin(0x3,LAD_NC+0x168000,4,&d);	
	 ///////VMEin(0x3,LAD_N+0x168000,4,&d);
	 if(d) { 	 //now try reading spy fifo-2
	   printf("%x\n",d); 
	   printf("CH#:%2d",((d>>26)&0x3f));
	   if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",((d>>1)&0xff00)+(d&0xff));
	   if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",((d>>4)&0xff00)+(d&0xff));    
	 }     
       }

       printf("\n\n\nTEMP FIFO Nup\n\n");
       for(i=0;i<256;i++)  { 
	 VMEin(0x3,LAD_N+0x148000,4,&d);	
	 ///////VMEin(0x3,LAD_N+0x168000,4,&d);
	 if(d) { 
	   printf("%x\n",d); 
	   printf("CH#:%2d",((d>>26)&0x3f));
	   if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",((d>>1)&0xff00)+(d&0xff));
	   if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",((d>>4)&0xff00)+(d&0xff));    
	 }
       }	
   
       printf("\n\n\nTEMP FIFO Ndown\n\n");
       for(i=0;i<256;i++)  { 
	 VMEin(0x3,LAD_N+0x168000,4,&d);	
	 ///////VMEin(0x3,LAD_N+0x168000,4,&d);
	 if(d) { 	 //now try reading spy fifo-2
	   printf("%x\n",d); 
	   printf("CH#:%2d",((d>>26)&0x3f));
	   if(((d>>21)&0x1f)== 31) printf("  TBM_H_status:%4x\n",((d>>1)&0xff00)+(d&0xff));
	   if(((d>>21)&0x1f)== 30) printf("  TBM_T_status:%4x\n",((d>>4)&0xff00)+(d&0xff));    
	 }     
       }	

       
       printf("\n\n\nErrorFIFO SOUTHdown\n\n");
       for(i=0;i<256;i++) VMEin(0x3,LAD_S+0x160000,4,&ErrorFIFO_down[i]);
       //for(i=0;i<256;i++) VMEin(0x3,LAD_N+0x160000,4,&ErrorFIFO_down[i]);
       
       Old_Data=0;i=0;
       while ( (Old_Data!=ErrorFIFO_down[i]) && (i<255) )
	 {
	   printf("%3d.  CH#%2d  CODE#%2d  TTC_EventNr:%2x  E_Data[hex]=%2x\n",
		  i,(ErrorFIFO_down[i]>>26)&0x3f,(ErrorFIFO_down[i]>>21)&0x1f,(ErrorFIFO_down[i]>>13)&0xff,(ErrorFIFO_down[i])&0x3ff);
	   Old_Data=ErrorFIFO_down[i];
	   i++;	 //now try reading spy fifo-2
	 }
       //printf("\nPhase:%1x\n\n",l&0x3);		
       
       VMEout(0x3,LAD_C+0x1c0000,4,0x0);  //SLink disable ;Disable SpyMemWrite during readout     
       //VMEout(0x3,LAD_C+0x1c0000,4,0x8);  //SLink ignore LFF
       usleep(10);    
       
       s=1024;
       VMEmove(0x1003,LAD_C+0x160000,0,destaddr,s,4);  //  DOWN BT - FIFO Mode
       for(i=0;i<1024;i++) D_down[i]=destaddr[i];
       
       s=1024;
       VMEmove(0x1003,LAD_C+0x140000,0,destaddr,s,4);  //  UP BT - FIFO Mode
       for(i=0;i<1024;i++) D_up[i]=destaddr[i];

       
       
       printf("\n\n");
       printf("|||||SLID|EVTY|--Event#--|--BunchC#--|--SourceID--|FOV|FH|\n");
       printf("  0:|  %1x |  %1x |  %6x  |"  ,(D_up[0]&0xf0000000)>>28,(D_up[0]&0xf000000)>>24,(D_up[0]&0xffffff) );
       printf("      %3x  |       %3x  |  %1x| %1x| %8x          %8x\n",
       	      (D_down[0]&0xfff00000)>>20,(D_down[0]&0xfff00)>>8,(D_down[0]&0xf0)>>4,D_down[0]&0xf,D_up[0],D_down[0]);
       printf("|||||----|----|----------|-----------|------------|---|--|\n");
       
       printf("\n\n|||||CH|RO|DC|PXL|PH| S-Link |CH|RO|DC|PXL|PH|||||\n");
       
       for(i=1;i<163;i++) {  //110
	 
	 printf("%3d:  %2d %2d %2d %3d %2x       ",
	 	i,(D_up[i]>>26)&0x3f,(D_up[i]>>21)&0x1f,(D_up[i]>>16)&0x1f,(D_up[i]>>8)&0xff,D_up[i]&0xff);
	 printf("  %2d %2d %2d %3d %2x              %8x          %8x\n",
	 	(D_down[i]>>26)&0x3f,(D_down[i]>>21)&0x1f,(D_down[i]>>16)&0x1f,(D_down[i]>>8)&0xff,D_down[i]&0xff,D_up[i],D_down[i]);
	 usleep(10);  
	 
	 //	 printf("testing CENTER ...\n");
	 //int istuff=0;
	 //for(i=0;i<=900000;i++)  {
	 // VMEout(0x3,LAD_C+0x1a8000,4,i+0x1fff0000); VMEin(0x3,LAD_C+0x1a8000,4,&d);
	 // if(d!=i+0x1fff0000) {printf("ERROR at: WRITTEN:%6x   READ:%6x\n",i+0x1fff0000,d);std::cin>>istuff;break;}
	 // if((i%100000)==0) printf("%d\n",i);
	 //}
	 
       }
       
       for(i=1;i<183;i++) {  //110
	 if(((D_up[i]>>21)&0x1f)>27)cout<<dec<<DecodeError(D_up[i])<<dec<<" data 0x"<<hex<<D_up[i]<<dec<<endl;
	 if(((D_down[i]>>21)&0x1f)>27)cout<<dec<<DecodeError(D_down[i])<<dec<<" data 0x"<<hex<<D_up[i]<<dec<<endl;
	 usleep(10);  
       }

       //try reading spy fifo
       for(int j1=0;j1<128;j1++){ 
	 // VMEin(0x3,LAD_N+0x150000,4,&d);
	 // buffer1[j1]=d;
	 // printf("Fifo-2 North up %d %x ",j1,d);
	 // VMEin(0x3,LAD_N+0x170000,4,&d);
	 // printf("Fifo-2 NorthCenter up %d %x\n ",j1,d);
	 // VMEin(0x3,LAD_N+0x150000,4,&d);
	 // printf("Fifo-2 our N %d %x\n ",j1,d);
	 // VMEin(0x3,LAD_NC+0x150000,4,&d);
	 // printf("Fifo-2 our NC %d %x\n ",j1,d);
	 //cout << "after buffer1[j1] " << buffer1[j1] << " d " << d << endl;
	 // buffer2[j1]=d;

	 VMEin(0x3,LAD_N+0x150000,4,&d);
	 buffer1[j1]=d;
	 printf("Fifo-2 North up %d %x ",j1,d);
	 VMEin(0x3,LAD_NC+0x150000,4,&d);
	 printf("Fifo-2 NorthCenter up %d %x\n ",j1,d);
	 buffer2[j1]=d;

       } 
              
       // for(int j1=0;j1<128;j1++){
       // 	 VMEin(0x3,LAD_S+0x150000,4,&d);
       // 	 buffer1[j1]=d;
       // 	 printf("Spy Fifo South %d %x \n",j1,d);
       // } 

       cout<<decodePTrans(buffer1,buffer2,16)<<endl;
       cout<<decodePTrans2(buffer1,buffer2,16)<<endl;


       //decode(buffer1,buffer2,64);
       
       decode_withROCs(buffer1,buffer2,64);
       //cout << " buffer1[0] " << buffer1[0] << endl;
       //decode(buffer1,buffer2,64);
       cout << "tbmheader 1 = " << a[0]<< endl;
       cout << "tbmheader 2 = " << a2[0]<< endl;
       
       cout << "rocheader 1 = " << a[1]<< endl;
       cout << "rocheader 2 = " << a2[1]<< endl;
       
       cout << "correctdata 1 = " << a[2]<< endl;
       cout << "correctdata 2 = " << a2[2]<< endl;
       
       cout << "tbmtrailer 1 = " << a[3]<< endl;
       cout << "tbmtrailer 2 = " << a2[3]<< endl;

       //decode(buffer1,buffer2,64);
       
       //cout << " 3 " << endl;
       //cout << " buffer1[0] " << buffer1[0] << endl;

       printf("\nPIGGYup\n");
       for(i=0;i<256;i++) {
	 VMEin(0x3,LAD_N+0x20000,4,&d);
	 if (i<42) {
	   printf("%04x %04x              ",(d>>16)&0xffff, d&0xffff);
	   BinDisplay((d>>16)&0xffff, 16,0,0);
	   printf("    ");
	   BinDisplay(d&0xffff, 16,0,0);
	   printf("\n");
	   
	   file<<hex<<((d>>16)&0xffff)<< " " <<hex<< (d&0xffff)<< "   ";
	   int Bits(16); int i1; unsigned long mask;
	   
	   mask=1<<(Bits-1);  
	   for (i1=0;i1<Bits;i1++) {
	     if ( (((d>>16)&0xffff)<<i1) & mask) file<<"1";
	     else  file<<"0";
	   }
	   file<<"    ";
	   for (i1=0;i1<Bits;i1++) {
	     if ( ((d&0xffff)<<i1) & mask) file<<"1";
	     else  file<<"0";
	   }
	   file<<endl;
	   
	 }
       
       }

       //cout << " 2 " << endl;
       //cout << " buffer1[0] " << buffer1[0] << endl;
       //printf("\nPIGGYdown\n");
       for(i=0;i<256;i++) {
       	 VMEin(0x3,LAD_S+0x20000,4,&d);
	 if (i<42) {
       	   printf("%04x %04x              ",(d>>16)&0xffff, d&0xffff);
	   BinDisplay((d>>16)&0xffff, 16,0,0);
	   printf("    ");
	   BinDisplay(d&0xffff, 16,0,0);
       	   printf("\n");
	 }
       }
       
  
       file.close();
       cout<<"scaler"<<caendat<<endl;
       cout<<"scaler after trigger"<<caendat1<<endl;
       
     };break;    
       
     case'b':{   // event number loop  
       
       VMEout(0x3,LRES,4,0x80000000); 
       VMEout(0x3,CLRES,4,0x80000000); 

       //int myenable=0;
       //cout<<"input south enable word"<<endl;
       //cin>>hex>>myenable;
       
       ///VMEout( 0x3,LAD_N+0x1a0000,4,0x000); //Disable all 9 channels from North FPGA
       
       ///VMEout(0x3,LAD_NC+0x1a0000,4,0x1f8); //FOR NORTH PIGGY CARD //Enable first 3 channels from NorthCenter FPGA
       ///VMEout(0x3,LAD_NC+0x1a0000,4,0x1ff); //Disable all 9 channels from NorthCenter FPGA
       ///VMEout(0x3,LAD_SC+0x1a0000,4,0x1ff); //Disable all 9 channels from NorthCenter FPGA
       ///VMEout( 0x3,LAD_S+0x1a0000,4,myenable);   //Enable all 9 channels from South FPGA
       
       ///VMEout( 0x3,LAD_S+0x1a0000,4,0x7f); // FOR SOUTH PIGGY CARD  //Enable all 9 channels from South FPGA
       ///VMEout( 0x3,LAD_S+0x1a0000,4,0x1ff);   //Disable all 9 channels from South FPGA
       
       // North Piggy all chs enabled 
       VMEout( 0x3,LAD_N+0x1a0000,4,0x030);
       VMEout(0x3,LAD_NC+0x1a0000,4,0x1fe);
       VMEout(0x3,LAD_SC+0x1a0000,4,0x1ff);
       VMEout( 0x3,LAD_S+0x1a0000,4,0x1ff); 
       
       // South Piggy all chs enabled
       //VMEout( 0x3,LAD_N+0x1a0000,4,0x1ff);
       //VMEout(0x3,LAD_NC+0x1a0000,4,0x1ff);
       //VMEout(0x3,LAD_SC+0x1a0000,4,0x03f);
       //VMEout( 0x3,LAD_S+0x1a0000,4,0x000); 
       
       // Both North and South Piggy enabled
       //VMEout( 0x3,LAD_N+0x1a0000,4,0x000);
       //VMEout(0x3,LAD_NC+0x1a0000,4,0x1f8);
       //VMEout(0x3,LAD_SC+0x1a0000,4,0x03f);
       //VMEout( 0x3,LAD_S+0x1a0000,4,0x000); 

       
       VMEout( 0x3,LAD_C+0x1a0000,4,0x14);   //set the gdmf control register
       VMEout(0x3,LAD_C+0x1d8000,4,0x0); // Trigger delay in the center chip
       

       // THIS IS REGRESET for SPYING
       VMEout (0x3,LAD_S+0x1c0000, 4, 0x2);   //  REGreset PIGGY bottom   
       usleep(10);
       VMEout (0x3,LAD_S+0x1c0000, 4, 0x0); 
       
       VMEout (0x3,LAD_NC+0x1c0000, 4, 0x2);   //  REGreset PIGGY top   
       usleep(10);
       VMEout (0x3,LAD_NC+0x1c0000, 4, 0x0);
       //////
       VMEout(0x3,LAD_N+0x1c0000,4,((0x2)<<24)+(0x8<<8));  //North Mode register     
       VMEout(0x3,LAD_NC+0x1c0000,4,((0x2)<<24)+(0x0<<8));  //NorthCenter Mode register     


       
       //usleep(3000000);
       
       ///////
       //if(!ttcconfigured) {
       //	 printf("setup TTCci\n");
       //	 //setup TTcci
       //	 system("/home/cmspixels/build/TriDAS/TTCSoftware/bin/RunTTCciControl.exe -s 2 -c ./TTCciConfiguration.txt -q &> /tmp/stuff");
       //	 ttcconfigured=true;
       //}
       if ( ttcconfigured ){
	 int type0=0;
	 int type1=0;
         int type2=0;
         int type3=0;

         int ntype0=0;
         int ntype1=0;
         int ntype2=0;
         int ntype3=0;
         int ntype4=0;
         int ntype5=0;
	int ntypold2=0;
        int ntypold5=0;




	 a[0]=0;a[1]=0;a[2]=0;a[3]=0;
	 for(l=0;l<1000;l++) {
	   
	   //VMEout(0x3,LRES,4,0x80000000);
	   //VMEout(0x3,CLRES,4,0x80000000); 
	   
	   printf("  \n");
	   VMEin( 0x3,LAD_N+0x1a0000,4,&d);
	   printf("LAD_N+0x1a0000 should read 0xff and it reads = %x\n",d);
	   VMEin( 0x3,LAD_NC+0x1a0000,4,&d);
	   printf("LAD_NC+0x1a0000 should read 0x1fe and it reads = %x\n",d);
	   VMEin( 0x3,LAD_SC+0x1a0000,4,&d);
	   printf("LAD_SC+0x1a0000 should read 0x1ff and it reads = %x\n",d);
	   VMEin( 0x3,LAD_S+0x1a0000,4,&d);
	   printf("LAD_S+0x1a0000 should read 0x1ff and it reads = %x\n",d);
	   printf("  \n");
	   
	   VMEout(0x3,LAD_C+0x120000,4,0x1);  //latch fifo fill levels for this event

	   //VMEout (0x3, TTCVIBASE+0x86, 2, 1); // send L1A Trigger  
	   //usleep(100);
	   //	   data=0xb;  // bgo 0xb = 11 = single l1a

	   //data =0x2; // bgo 2 = testenable
	   //ret = CAENVME_WriteCycle(0,ttcaddcsr0,&data,cvA24_U_DATA,cvD32);
	   //usleep(100);
	   
	   caenret = CAENVME_ReadRegister(0,cvScaler1,&caendat);
	   
	   
	   //	   data=0xb;  // bgo 0xb = 11 = single l1a
	   data = 0x9; // bgo 9 = start
	   ret = CAENVME_WriteCycle(0,ttcaddcsr0,&data,cvA24_U_DATA,cvD32);
	   
	   if(ret != cvSuccess) { 
	     cout<<"Error in write TTCci BGO Trigger"<<hex<<ret<<" "<<d<<endl;  
	     printf("Error in TTCci trigger!\n");
	     analyzeError(ret); 
	   }
	   usleep(10000);
	   caenret = CAENVME_ReadRegister(0,cvScaler1,&caendat1);
	   cout<<"scaler "<<caendat<<endl;
	   cout<<"scaler after trigger "<<caendat1<<endl;
	   	   
	   VMEout(0x3,LAD_C+0x1c0000,4,0x0);  //SLink disable ;Disable SpyMemWrite during readout     
	   
	   usleep(10);    
	   //////////////////////////////////////////////////    
	   
	   s=1024;
	   VMEmove(0x1003,LAD_C+0x160000,0,destaddr,s,4);  //  DOWN BT - FIFO Mode
	   for(i=0;i<1024;i++) D_down[i]=destaddr[i];
	   
	   s=1024;
	   VMEmove(0x1003,LAD_C+0x140000,0,destaddr,s,4);  //  UP BT - FIFO Mode
	   for(i=0;i<1024;i++) D_up[i]=destaddr[i];
	   
	   printf("\n\n");
	   printf("|||||SLID|EVTY|--Event#--|--BunchC#--|--SourceID--|FOV|FH|\n");
	   printf("  0:|  %1x |  %1x |  %6x  |"  ,(D_up[0]&0xf0000000)>>28,(D_up[0]&0xf000000)>>24,(D_up[0]&0xffffff) );
	   printf("      %3x  |       %3x  |  %1x| %1x| %8x          %8x\n",
		  (D_down[0]&0xfff00000)>>20,(D_down[0]&0xfff00)>>8,(D_down[0]&0xf0)>>4,D_down[0]&0xf,D_up[0],D_down[0]);
	   printf("|||||----|----|----------|-----------|------------|---|--|\n");
	   
	   printf("\n\n|||||CH|RO|DC|PXL|PH| S-Link |CH|RO|DC|PXL|PH|||||\n");
	   
	   for(i=1;i<163;i++) {  //110
	     
	     printf("%3d:  %2d %2d %2d %3d %2x       ",
		    i,(D_up[i]>>26)&0x3f,(D_up[i]>>21)&0x1f,(D_up[i]>>16)&0x1f,(D_up[i]>>8)&0xff,D_up[i]&0xff);
	     printf("  %2d %2d %2d %3d %2x              %8x          %8x\n",
		    (D_down[i]>>26)&0x3f,(D_down[i]>>21)&0x1f,(D_down[i]>>16)&0x1f,(D_down[i]>>8)&0xff,D_down[i]&0xff,D_up[i],D_down[i]);
	     
	   }
	   
	   VMEin(0x3,LAD_C+0x198000,4,&eventnum);
	   printf("Eventnumber read from CENTER FPGA: %6x\n",eventnum& 0xffffff);


       printf("\nPIGGYup\n");
       for(i=0;i<256;i++) {
         VMEin(0x3,LAD_N+0x20000,4,&d);
         if (i<42) {
           printf("%04x %04x              ",(d>>16)&0xffff, d&0xffff);
           BinDisplay((d>>16)&0xffff, 16,0,0);
           printf("    ");
           BinDisplay(d&0xffff, 16,0,0);
           printf("\n");

           file<<hex<<((d>>16)&0xffff)<< " " <<hex<< (d&0xffff)<< "   ";
           int Bits(16); int i1; unsigned long mask;

           mask=1<<(Bits-1);
           for (i1=0;i1<Bits;i1++) {
             if ( (((d>>16)&0xffff)<<i1) & mask) file<<"1";
             else  file<<"0";
           }
           file<<"    ";
           for (i1=0;i1<Bits;i1++) {
             if ( ((d&0xffff)<<i1) & mask) file<<"1";
             else  file<<"0";
           }
           file<<endl;

         }

       }


	          	      
       //try reading spy fifo
	   for(int j1=0;j1<128;j1++){ 
	     VMEin(0x3,LAD_N+0x150000,4,&d);
	     buffer1[j1]=d;
	     printf("Fifo-2 North up %d %x ",j1,d);
	     VMEin(0x3,LAD_NC+0x150000,4,&d);
	     printf("Fifo-2 NorthCenter up %d %x\n ",j1,d);
	     buffer2[j1]=d;
	   }       	      
           
	   //	   for(int j1=0;j1<20;j1++){ 
	   //	     VMEin(0x3,LAD_S+0x170000,4,&d);
	   //	     printf("Fifo-2 South spy %x \n",d);
	   //	   }
	   
	   
	   type0=decodePTrans(buffer1,buffer2,16); //cout<<" check = "<<type0<<endl;
       type1+=(type0&0x1);
       type2+=(type0&0x2)/2;
       type3+=(type0&0x4)/4; 
       cout<<" loop = "<<l<<endl;
       cout<<"Number of buffer with unequal markers: "<<type1<<endl;
       cout<<"Number of buffer with unmatched pattern: "<<type2<<endl;
       cout<<"Number of buffer with unequal event numbers: "<<type3<<endl;

       ntype0=decodePTrans2(buffer1,buffer2,128); //cout<<" check = "<<ntype0<<endl;
       ntype1+=(ntype0&0x1);
       ntype2+=(ntype0&0x2)/2;
       ntype3+=(ntype0&0x4)/4;
       ntype4+=(ntype0&0x8)/8;
       ntype5+=(ntype0&0x10)/16;

       cout<<" loop = "<<l<<endl;
       cout<<"Number of buffer with missed trailer: "<<ntype1<<endl;
       cout<<"Number of buffer with missed roc: "<<ntype2<<endl;
       cout<<"Number of buffer with unequal event numbers: "<<ntype3<<endl;
       cout<<"Number of buffer with missed header: "<<ntype4<<endl;
       cout<<"Number of buffer with a missed hit: "<<ntype5<<endl;
       
       if(ntype2!=ntypold2) cin>>ntypold2;
       if(ntype5!=ntypold5) cin>>ntypold5;

       cout << "  " << endl;
       decode(buffer1,buffer2,64);
       cout << "tbmheader = " << a[0]<< endl;
       cout << "rocheader = " << a[1]<< endl;
       cout << "correctdata = " << a[2]<< endl;
       cout << "tbmtrailer = " << a[3]<< endl;
       
       if ( caendat != caendat1 ) break;

       	 }// endloop
       }
       
     };break;    
       
       
   case'c':{   // emulator test  
     
     CH_SubAddr_Sim[0]=0x3c000;   CH_SubAddr_Sim[1]= 0x5c000;	   CH_SubAddr_Sim[2]= 0x7c000;
     CH_SubAddr_Sim[3]=0x9c000;   CH_SubAddr_Sim[4]= 0xbc000;	   CH_SubAddr_Sim[5]= 0xdc000;
     CH_SubAddr_Sim[6]=0xfc000;   CH_SubAddr_Sim[7]=0x11c000;	   CH_SubAddr_Sim[8]=0x13c000;
     
     VMEout(0x3,LRES,4,0x80000000); 
     VMEout(0x3,CLRES,4,0x80000000); 
     
     VMEout( 0x3,LAD_N+0x1a0000,4,0x1f0); // first four channels enabled
     VMEout(0x3,LAD_NC+0x1a0000,4,0x1ff); 
     VMEout(0x3,LAD_SC+0x1a0000,4,0x1ff);  
     VMEout( 0x3,LAD_S+0x1a0000,4,0x1ff);   
                  		
     VMEout( 0x3,LAD_N+CH_SubAddr_Sim[0],4,0x2); //Number of Hits/ROC  North Channel a
     VMEout( 0x3,LAD_N+CH_SubAddr_Sim[1],4,0x2); //Number of Hits/ROC  North Channel b
     VMEout( 0x3,LAD_N+CH_SubAddr_Sim[2],4,0x1); //Number of Hits/ROC  North Channel c
     VMEout( 0x3,LAD_N+CH_SubAddr_Sim[3],4,0x1); //Number of Hits/ROC  North Channel d
     VMEout( 0x3,LAD_N+CH_SubAddr_Sim[4],4,0x1); //Number of Hits/ROC  North Channel e
     VMEout( 0x3,LAD_N+CH_SubAddr_Sim[5],4,0x1); //Number of Hits/ROC  North Channel f
     VMEout( 0x3,LAD_N+CH_SubAddr_Sim[6],4,0x1); //Number of Hits/ROC  North Channel g
     VMEout( 0x3,LAD_N+CH_SubAddr_Sim[7],4,0x1); //Number of Hits/ROC  North Channel h
     VMEout( 0x3,LAD_N+CH_SubAddr_Sim[8],4,0x1); //Number of Hits/ROC  North Channel i
     
     VMEout( 0x3,LAD_NC+CH_SubAddr_Sim[0],4,0x1); //Number of Hits/ROC  North Channel j
     VMEout( 0x3,LAD_NC+CH_SubAddr_Sim[1],4,0x1); //Number of Hits/ROC  North Channel k
     VMEout( 0x3,LAD_NC+CH_SubAddr_Sim[2],4,0x1); //Number of Hits/ROC  North Channel l
     VMEout( 0x3,LAD_NC+CH_SubAddr_Sim[3],4,0x1); //Number of Hits/ROC  North Channel m
     VMEout( 0x3,LAD_NC+CH_SubAddr_Sim[4],4,0x1); //Number of Hits/ROC  North Channel n
     VMEout( 0x3,LAD_NC+CH_SubAddr_Sim[5],4,0x1); //Number of Hits/ROC  North Channel o
     VMEout( 0x3,LAD_NC+CH_SubAddr_Sim[6],4,0x1); //Number of Hits/ROC  North Channel p
     VMEout( 0x3,LAD_NC+CH_SubAddr_Sim[7],4,0x1); //Number of Hits/ROC  North Channel q
     VMEout( 0x3,LAD_NC+CH_SubAddr_Sim[8],4,0x1); //Number of Hits/ROC  North Channel r
     
     VMEout( 0x3,LAD_SC+CH_SubAddr_Sim[0],4,0x1); //Number of Hits/ROC  North Channel s
     VMEout( 0x3,LAD_SC+CH_SubAddr_Sim[1],4,0x1); //Number of Hits/ROC  North Channel t
     VMEout( 0x3,LAD_SC+CH_SubAddr_Sim[2],4,0x1); //Number of Hits/ROC  North Channel u
     VMEout( 0x3,LAD_SC+CH_SubAddr_Sim[3],4,0x1); //Number of Hits/ROC  North Channel v
     VMEout( 0x3,LAD_SC+CH_SubAddr_Sim[4],4,0x1); //Number of Hits/ROC  North Channel w
     VMEout( 0x3,LAD_SC+CH_SubAddr_Sim[5],4,0x1); //Number of Hits/ROC  North Channel x
     VMEout( 0x3,LAD_SC+CH_SubAddr_Sim[6],4,0x1); //Number of Hits/ROC  North Channel y
     VMEout( 0x3,LAD_SC+CH_SubAddr_Sim[7],4,0x1); //Number of Hits/ROC  North Channel z
     VMEout( 0x3,LAD_SC+CH_SubAddr_Sim[8],4,0x1); //Number of Hits/ROC  North Channel aa
     
     VMEout( 0x3,LAD_S+CH_SubAddr_Sim[0],4,0x1); //Number of Hits/ROC  North Channel ab
     VMEout( 0x3,LAD_S+CH_SubAddr_Sim[1],4,0x1); //Number of Hits/ROC  North Channel ac
     VMEout( 0x3,LAD_S+CH_SubAddr_Sim[2],4,0x1); //Number of Hits/ROC  North Channel ad
     VMEout( 0x3,LAD_S+CH_SubAddr_Sim[3],4,0x1); //Number of Hits/ROC  North Channel ae
     VMEout( 0x3,LAD_S+CH_SubAddr_Sim[4],4,0x1); //Number of Hits/ROC  North Channel af
     VMEout( 0x3,LAD_S+CH_SubAddr_Sim[5],4,0x1); //Number of Hits/ROC  North Channel ag
     VMEout( 0x3,LAD_S+CH_SubAddr_Sim[6],4,0x1); //Number of Hits/ROC  North Channel ah
     VMEout( 0x3,LAD_S+CH_SubAddr_Sim[7],4,0x1); //Number of Hits/ROC  North Channel ai
     VMEout( 0x3,LAD_S+CH_SubAddr_Sim[8],4,0x5); //Number of Hits/ROC  North Channel aj
     
     VMEout(0x3,LAD_C+0x1c0000,4,0xa);  //SLink disable ;Disable SpyMemWrite during readout     
     
     
     VMEout(0x3,LAD_C+0x1a0000,4,0x14); // 0-TTCEn-0-EnableDAC-0-0			TTC Trigger  
     
     ///////
     if ( ttcconfigured ){
     
       for(l=0;l<3;l++) {
	 
	 ////////////////VMEout (0x3, TTCVIBASE+0x86, 2, 1); // send L1A Trigger  
	 data = 0x9; // bgo 9 = start
	 ret = CAENVME_WriteCycle(0,ttcaddcsr0,&data,cvA24_U_DATA,cvD32);
	 
	 //you can call this now when you need a trigger, it should not be called more often than 1/LHC orbit ~90us!
	 if(ret != cvSuccess) { 
	   cout<<"Error in write TTCci BGO Trigger"<<hex<<ret<<" "<<d<<endl;  
	   printf("Error in TTCci trigger!\n");
	   analyzeError(ret); 
	 }
	 
	 usleep(100);
	 
	 VMEout(0x3,LAD_C+0x1c0000,4,0x0);  //SLink disable ;Disable SpyMemWrite during readout     
	 usleep(100);
	 //////////////////////////////////////////////////    
	 
	 s=1024;
	 VMEmove(0x1003,LAD_C+0x160000,0,destaddr,s,4);  //  DOWN BT - FIFO Mode
	 for(i=0;i<1024;i++) D_down[i]=destaddr[i];
	 
	 s=1024;
	 VMEmove(0x1003,LAD_C+0x140000,0,destaddr,s,4);  //  UP BT - FIFO Mode
	 for(i=0;i<1024;i++) D_up[i]=destaddr[i];
	 	 
	 printf("\n\n");
	 printf("|||||SLID|EVTY|--Event#--|--BunchC#--|--SourceID--|FOV|FH|\n");
	 printf("  0:|  %1x |  %1x |  %6x  |"  ,(D_up[0]&0xf0000000)>>28,(D_up[0]&0xf000000)>>24,(D_up[0]&0xffffff) );
	 printf("      %3x  |       %3x  |  %1x| %1x| %8x          %8x\n",
		(D_down[0]&0xfff00000)>>20,(D_down[0]&0xfff00)>>8,(D_down[0]&0xf0)>>4,D_down[0]&0xf,D_up[0],D_down[0]);
	 printf("|||||----|----|----------|-----------|------------|---|--|\n");
	 printf("\n\n|||||CH|RO|DC|PXL|PH| S-Link |CH|RO|DC|PXL|PH|||||\n");
	 
	 for(i=1;i<73;i++) {  //110
	   printf("%3d:  %2d %2d %2d %3d %2x       ",
		  i,(D_up[i]>>26)&0x3f,(D_up[i]>>21)&0x1f,(D_up[i]>>16)&0x1f,(D_up[i]>>8)&0xff,D_up[i]&0xff);
	   printf("  %2d %2d %2d %3d %2x              %8x          %8x\n",
		  (D_down[i]>>26)&0x3f,(D_down[i]>>21)&0x1f,(D_down[i]>>16)&0x1f,(D_down[i]>>8)&0xff,D_down[i]&0xff,D_up[i],D_down[i]);
	 }
	 
	 VMEin(0x3,LAD_C+0x198000,4,&eventnum);
	 printf("Eventnumber read from CENTER FPGA: %6x\n",eventnum& 0xffffff);
	 
       }// endloop
     }
     
   };break;    
   
   case'j':{   // event number loop  
       
       VMEout(0x3,LRES,4,0x80000000); 
       VMEout(0x3,CLRES,4,0x80000000); 

       // North Piggy all chs enabled 
       VMEout( 0x3,LAD_N+0x1a0000,4,0x000);
       VMEout(0x3,LAD_NC+0x1a0000,4,0x1f8);
       VMEout(0x3,LAD_SC+0x1a0000,4,0x1ff);
       VMEout( 0x3,LAD_S+0x1a0000,4,0x1ff); 
       
       // South Piggy all chs enabled
       //VMEout( 0x3,LAD_N+0x1a0000,4,0x1ff);
       //VMEout(0x3,LAD_NC+0x1a0000,4,0x1ff);
       //VMEout(0x3,LAD_SC+0x1a0000,4,0x03f);
       //VMEout( 0x3,LAD_S+0x1a0000,4,0x000); 
       
       // Both North and South Piggy enabled
       //VMEout( 0x3,LAD_N+0x1a0000,4,0x000);
       //VMEout(0x3,LAD_NC+0x1a0000,4,0x1f8);
       //VMEout(0x3,LAD_SC+0x1a0000,4,0x03f);
       //VMEout( 0x3,LAD_S+0x1a0000,4,0x000); 

       
       VMEout( 0x3,LAD_C+0x1a0000,4,0x14);   //set the gdmf control register
       VMEout(0x3,LAD_C+0x1d8000,4,0x0); // Trigger delay in the center chip
       

       // THIS IS REGRESET for SPYING
       VMEout (0x3,LAD_S+0x1c0000, 4, 0x2);   //  REGreset PIGGY bottom   
       usleep(10);
       VMEout (0x3,LAD_S+0x1c0000, 4, 0x0); 
       
       VMEout (0x3,LAD_NC+0x1c0000, 4, 0x2);   //  REGreset PIGGY top   
       usleep(10);
       VMEout (0x3,LAD_NC+0x1c0000, 4, 0x0);
       //////

       if ( ttcconfigured ){
	 int type0=0;
	 int type1=0;
         int type2=0;
         int type3=0;
	 
         int ntype0=0;
         int ntype1=0;
         int ntype2=0;
         int ntype3=0;
         int ntype4=0;
         int ntype5=0;
	 int ntypold2=0;
	 int ntypold5=0;
	 
	 a[0]=0;a[1]=0;a[2]=0;a[3]=0;
	 for(l=0;l<1000;l++) {
	   
	   //VMEout(0x3,LRES,4,0x80000000);
	   //VMEout(0x3,CLRES,4,0x80000000); 
	   
	   // printf("  \n");
	   // VMEin( 0x3,LAD_N+0x1a0000,4,&d);
	   // printf("LAD_N+0x1a0000 should read 0xff and it reads = %x\n",d);
	   // VMEin( 0x3,LAD_NC+0x1a0000,4,&d);
	   // printf("LAD_NC+0x1a0000 should read 0x1fe and it reads = %x\n",d);
	   // VMEin( 0x3,LAD_SC+0x1a0000,4,&d);
	   // printf("LAD_SC+0x1a0000 should read 0x1ff and it reads = %x\n",d);
	   // VMEin( 0x3,LAD_S+0x1a0000,4,&d);
	   // printf("LAD_S+0x1a0000 should read 0x1ff and it reads = %x\n",d);
	   // printf("  \n");
	   
	   // VMEout(0x3,LAD_C+0x120000,4,0x1);  //latch fifo fill levels for this event
	   
	   	   
	   //caenret = CAENVME_ReadRegister(0,cvScaler1,&caendat);
	   	   
	   //	   data=0xb;  // bgo 0xb = 11 = single l1a
	   data = 0x9; // bgo 9 = start
	   ret = CAENVME_WriteCycle(0,ttcaddcsr0,&data,cvA24_U_DATA,cvD32);
	   
	   if(ret != cvSuccess) { 
	     cout<<"Error in write TTCci BGO Trigger"<<hex<<ret<<" "<<d<<endl;  
	     printf("Error in TTCci trigger!\n");
	     analyzeError(ret); 
	   }
	   //usleep(10000);
	   //caenret = CAENVME_ReadRegister(0,cvScaler1,&caendat1);
	   //cout<<"scaler "<<caendat<<endl;
	   //cout<<"scaler after trigger "<<caendat1<<endl;
	   	   
	   //VMEout(0x3,LAD_C+0x1c0000,4,0x0);  //SLink disable ;Disable SpyMemWrite during readout     
	   
	   //usleep(10);    
	   //////////////////////////////////////////////////    
	   
	   // s=1024;
	   // VMEmove(0x1003,LAD_C+0x160000,0,destaddr,s,4);  //  DOWN BT - FIFO Mode
	   // for(i=0;i<1024;i++) D_down[i]=destaddr[i];
	   
	   // s=1024;
	   // VMEmove(0x1003,LAD_C+0x140000,0,destaddr,s,4);  //  UP BT - FIFO Mode
	   // for(i=0;i<1024;i++) D_up[i]=destaddr[i];
	   
	   // printf("\n\n");
	   // printf("|||||SLID|EVTY|--Event#--|--BunchC#--|--SourceID--|FOV|FH|\n");
	   // printf("  0:|  %1x |  %1x |  %6x  |"  ,(D_up[0]&0xf0000000)>>28,(D_up[0]&0xf000000)>>24,(D_up[0]&0xffffff) );
	   // printf("      %3x  |       %3x  |  %1x| %1x| %8x          %8x\n",
	   // 	  (D_down[0]&0xfff00000)>>20,(D_down[0]&0xfff00)>>8,(D_down[0]&0xf0)>>4,D_down[0]&0xf,D_up[0],D_down[0]);
	   // printf("|||||----|----|----------|-----------|------------|---|--|\n");
	   
	   // printf("\n\n|||||CH|RO|DC|PXL|PH| S-Link |CH|RO|DC|PXL|PH|||||\n");
	   
	   // for(i=1;i<163;i++) {  //110
	     
	   //   printf("%3d:  %2d %2d %2d %3d %2x       ",
	   // 	    i,(D_up[i]>>26)&0x3f,(D_up[i]>>21)&0x1f,(D_up[i]>>16)&0x1f,(D_up[i]>>8)&0xff,D_up[i]&0xff);
	   //   printf("  %2d %2d %2d %3d %2x              %8x          %8x\n",
	   // 	    (D_down[i]>>26)&0x3f,(D_down[i]>>21)&0x1f,(D_down[i]>>16)&0x1f,(D_down[i]>>8)&0xff,D_down[i]&0xff,D_up[i],D_down[i]);
	     
	   // }
	   
	   // VMEin(0x3,LAD_C+0x198000,4,&eventnum);
	   // printf("Eventnumber read from CENTER FPGA: %6x\n",eventnum& 0xffffff);

	   
       
       // 	     printf("\nPIGGYup\n");
       // for(i=0;i<256;i++) {
       //   VMEin(0x3,LAD_N+0x20000,4,&d);
       //   if (i<42) {
       //     printf("%04x %04x              ",(d>>16)&0xffff, d&0xffff);
       //     BinDisplay((d>>16)&0xffff, 16,0,0);
       //     printf("    ");
       //     BinDisplay(d&0xffff, 16,0,0);
       //     printf("\n");

       //     file<<hex<<((d>>16)&0xffff)<< " " <<hex<< (d&0xffff)<< "   ";
       //     int Bits(16); int i1; unsigned long mask;

       //     mask=1<<(Bits-1);
       //     for (i1=0;i1<Bits;i1++) {
       //       if ( (((d>>16)&0xffff)<<i1) & mask) file<<"1";
       //       else  file<<"0";
       //     }
       //     file<<"    ";
       //     for (i1=0;i1<Bits;i1++) {
       //       if ( ((d&0xffff)<<i1) & mask) file<<"1";
       //       else  file<<"0";
       //     }
       //     file<<endl;

       //   }

       // }
       // 	   

	          	      
       //try reading spy fifo
	   for(int j1=0;j1<128;j1++){ 
	     VMEin(0x3,LAD_N+0x150000,4,&d);
	     buffer1[j1]=d;
	     printf("Fifo-2 North up %d %x ",j1,d);
	     VMEin(0x3,LAD_NC+0x150000,4,&d);
	     printf("Fifo-2 NorthCenter up %d %x\n ",j1,d);
	     buffer2[j1]=d;
	   }       	      
           
	   type0=decodePTrans(buffer1,buffer2,16); //cout<<" check = "<<type0<<endl;
       type1+=(type0&0x1);
       type2+=(type0&0x2)/2;
       type3+=(type0&0x4)/4; 
       cout<<" loop = "<<l<<endl;
       cout<<"Number of buffer with unequal markers: "<<type1<<endl;
       cout<<"Number of buffer with unmatched pattern: "<<type2<<endl;
       cout<<"Number of buffer with unequal event numbers: "<<type3<<endl;

       ntype0=decodePTrans2(buffer1,buffer2,128); //cout<<" check = "<<ntype0<<endl;
       ntype1+=(ntype0&0x1);
       ntype2+=(ntype0&0x2)/2;
       ntype3+=(ntype0&0x4)/4;
       ntype4+=(ntype0&0x8)/8;
       ntype5+=(ntype0&0x10)/16;

       cout<<" loop = "<<l<<endl;
       cout<<"Number of buffer with missed trailer: "<<ntype1<<endl;
       cout<<"Number of buffer with missed roc: "<<ntype2<<endl;
       cout<<"Number of buffer with unequal event numbers: "<<ntype3<<endl;
       cout<<"Number of buffer with missed header: "<<ntype4<<endl;
       cout<<"Number of buffer with a missed hit: "<<ntype5<<endl;
	if(ntype2!=ntypold2) cin>>ntypold2;
       if(ntype5!=ntypold5) cin>>ntypold5;




       
       cout << "  " << endl;
       decode(buffer1,buffer2,64);
       cout << "tbmheader = " << a[0]<< endl;
       cout << "rocheader = " << a[1]<< endl;
       cout << "correctdata = " << a[2]<< endl;
       cout << "tbmtrailer = " << a[3]<< endl;
       
       if ( caendat != caendat1 ) break;
       
       	 }// endloop
       }
       
     };break;    

   case'w':{   // event loop  
       
       VMEout(0x3,LRES,4,0x80000000); 
       VMEout(0x3,CLRES,4,0x80000000); 

       // North Piggy all chs enabled 
      
       VMEout( 0x3,LAD_N+0x1a0000,4,0x030); // ch1/2, 3/4, 7/8, 9 enabled
       VMEout(0x3,LAD_NC+0x1a0000,4,0x1fe); // ch10 enabled
       VMEout(0x3,LAD_SC+0x1a0000,4,0x1ff);
       VMEout( 0x3,LAD_S+0x1a0000,4,0x1ff);


 
       // South Piggy all chs enabled
       //VMEout( 0x3,LAD_N+0x1a0000,4,0x1ff);
       //VMEout(0x3,LAD_NC+0x1a0000,4,0x1ff);
       //VMEout(0x3,LAD_SC+0x1a0000,4,0x03f);
       //VMEout( 0x3,LAD_S+0x1a0000,4,0x000); 
       
       // Both North and South Piggy enabled
       //VMEout( 0x3,LAD_N+0x1a0000,4,0x000);
       //VMEout(0x3,LAD_NC+0x1a0000,4,0x1f8);
       //VMEout(0x3,LAD_SC+0x1a0000,4,0x03f);
       //VMEout( 0x3,LAD_S+0x1a0000,4,0x000); 

//tbm trailer mask (just cal)

       VMEout( 0x3,LAD_N+0x1c0000,4,(0x2<<24));



       
       VMEout( 0x3,LAD_C+0x1a0000,4,0x14);   //set the gdmf control register
       VMEout(0x3,LAD_C+0x1d8000,4,0x0); // Trigger delay in the center chip
       
       // THIS IS REGRESET for SPYING
       VMEout (0x3,LAD_S+0x1c0000, 4, 0x2);   //  REGreset PIGGY bottom   
       usleep(10);
       VMEout (0x3,LAD_S+0x1c0000, 4, 0x0); 
       
       VMEout (0x3,LAD_NC+0x1c0000, 4, 0x2);   //  REGreset PIGGY top   
       usleep(10);
       VMEout (0x3,LAD_NC+0x1c0000, 4, 0x0);
       //////
       
       if ( ttcconfigured ){
	 int type0=0;
	 int type1=0;
         int type2=0;
         int type3=0;
	 
         int ntype0=0;
         int ntype1=0;
         int ntype2=0;
         int ntype3=0;
         int ntype4=0;
         int ntype5=0;
	 int ntypold2=0;
	 int ntypold5=0;
	unsigned long int mylast=1;
	 a[0]=0;a[1]=0;a[2]=0;a[3]=0;
	 for(l=0;l<500000000;l++) {
	   
	   // VMEout(0x3,LRES,4,0x80000000);
	   //VMEout(0x3,CLRES,4,0x80000000); 
	   
	   // printf("  \n");
	   // VMEin( 0x3,LAD_N+0x1a0000,4,&d);
	   // printf("LAD_N+0x1a0000 should read 0xff and it reads = %x\n",d);
	   // VMEin( 0x3,LAD_NC+0x1a0000,4,&d);
	   // printf("LAD_NC+0x1a0000 should read 0x1fe and it reads = %x\n",d);
	   // VMEin( 0x3,LAD_SC+0x1a0000,4,&d);
	   // printf("LAD_SC+0x1a0000 should read 0x1ff and it reads = %x\n",d);
	   // VMEin( 0x3,LAD_S+0x1a0000,4,&d);
	   // printf("LAD_S+0x1a0000 should read 0x1ff and it reads = %x\n",d);
	   // printf("  \n");
	   // 
	   
	   //VMEout(0x3,LAD_C+0x120000,4,0x1);  //latch fifo fill levels for this event

	   //caenret = CAENVME_ReadRegister(0,cvScaler1,&caendat);
// Check for a busy
//	for(int ig=0;ig<100000;ig++){
//       VMEin(0x3,LAD_C+0x60000,4,&d);cout<<"busy? s<<d<<" "<<(d&0xf)<<endl;
//       usleep(1000000);}
//       now look at the last entry in the fifo, if it is an 8, we're good to go
	 while(mylast!=0){
	VMEin(0x3,LAD_C+0x190000,4,&d);
        mylast=(0x40000000&d);usleep(5);
	}
	mylast=1;
	//if((l%50000)==0)cin>>ntypold5;	   	   
	   //	   data=0xb;  // bgo 0xb = 11 = single l1a
	   data = 0x9; // bgo 9 = start
	   ret = CAENVME_WriteCycle(0,ttcaddcsr0,&data,cvA24_U_DATA,cvD32);
	   
	   if(ret != cvSuccess) { 
	     cout<<"Error in write TTCci BGO Trigger"<<hex<<ret<<" "<<d<<endl;  
	     printf("Error in TTCci trigger!\n");
	     analyzeError(ret); 
	   }
	//   usleep(1000);
	   //caenret = CAENVME_ReadRegister(0,cvScaler1,&caendat1);
	   //cout<<"scaler "<<caendat<<endl;
	   //cout<<"scaler after trigger "<<caendat1<<endl;
	   	   
	   //VMEout(0x3,LAD_C+0x1c0000,4,0x0);  //SLink disable ;Disable SpyMemWrite during readout     
	   
	   //usleep(10);    
	   //////////////////////////////////////////////////    
	   
	
       //try reading spy fifo

 //    if(ntype2!=ntypold2) cin>>ntypold2;
   //    if(ntype5!=ntypold5) cin>>ntypold5;
       
       //cout << "  " << endl;
       //decode(buffer1,buffer2,64);
       //cout << "tbmheader = " << a[0]<< endl;
       //cout << "rocheader = " << a[1]<< endl;
       //cout << "correctdata = " << a[2]<< endl;
       //cout << "tbmtrailer = " << a[3]<< endl;
       
       //if ( caendat != caendat1 ) break;
       
       	 }// endloop
       }
       
   };break;  
     
   case'd':{   
       
       int count(0);
       for(i=0;i<10;i++)  {
	 
	 //VMEin(0x3,LAD_S+0x178000,4,&d);
	 VMEin(0x3,LAD_N+0x178000,4,&d);
	 printf("                               %2x         %2x          %2x         %1d\n", (d)&0xff, (d>>8)&0xff, (d>>16)&0xff, (d>>24)&0x1);
	 count+=(d>>24)&0x1;
       }
       cout << "# of times locked = "  << count << endl;
       
       VMEout(0x3,LRES,4,0x80000000); 
       VMEout(0x3,CLRES,4,0x80000000); 
       
       count=0;
       for(i=0;i<10;i++)  {
	 
	 //VMEin(0x3,LAD_S+0x178000,4,&d);
	 VMEin(0x3,LAD_N+0x178000,4,&d);
	 printf("                               %2x         %2x          %2x         %1d\n", (d)&0xff, (d>>8)&0xff, (d>>16)&0xff, (d>>24)&0x1);
	 count+=(d>>24)&0x1;
       }
       cout << "after the FED reset # of times locked = "  << count << endl;

       // North Piggy all chs enabled 
       VMEout( 0x3,LAD_N+0x1a0000,4,0x03c);
       VMEout(0x3,LAD_NC+0x1a0000,4,0x1f8);
       VMEout(0x3,LAD_SC+0x1a0000,4,0x1ff);
       VMEout( 0x3,LAD_S+0x1a0000,4,0x1ff); 
       
       // South Piggy all chs enabled
       //VMEout( 0x3,LAD_N+0x1a0000,4,0x1ff);
       //VMEout(0x3,LAD_NC+0x1a0000,4,0x1ff);
       //VMEout(0x3,LAD_SC+0x1a0000,4,0x03f);
       //VMEout( 0x3,LAD_S+0x1a0000,4,0x000); 
       
       // Both North and South Piggy enabled
       //VMEout( 0x3,LAD_N+0x1a0000,4,0x000);
       //VMEout(0x3,LAD_NC+0x1a0000,4,0x1f8);
       //VMEout(0x3,LAD_SC+0x1a0000,4,0x03f);
       //VMEout( 0x3,LAD_S+0x1a0000,4,0x000); 
       
       VMEout( 0x3,LAD_C+0x1a0000,4,0x14);   //set the gdmf control register
       VMEout(0x3,LAD_C+0x1d8000,4,0x0); // Trigger delay in the center chip
       count=0;

       // Turn on the channel to spy on  bits 8-11 in the mode register for the channel
       // mask trailer status bits (<<24)
       
       VMEout(0x3,LAD_N+0x1c0000,4,((0x8<<8)+(0x2<<24)));  //North Mode register     
       VMEout(0x3,LAD_NC+0x1c0000,4,(0x0<<8));  //NorthCenter Mode register     
       
       for(i=0;i<10;i++)  {
	 
	 //VMEin(0x3,LAD_S+0x178000,4,&d);
	 VMEin(0x3,LAD_N+0x178000,4,&d);
	 printf("                               %2x         %2x          %2x         %1d\n", (d)&0xff, (d>>8)&0xff, (d>>16)&0xff, (d>>24)&0x1);
	 count+=(d>>24)&0x1;
       }
       cout << "after disabling the unused channels  # of times locked = "  << count << endl;
       //       VMEout (0x3, TTCVIBASE+0x86, 2, 1); // send L1A Trigger  with the TTCvi (no longer used)
              
       // THIS IS REGRESET for SPYING
       VMEout (0x3,LAD_S+0x1c0000, 4, 0x2);   //  REGreset PIGGY bottom   
       usleep(10);
       VMEout (0x3,LAD_S+0x1c0000, 4, 0x0); 
       
       VMEout (0x3,LAD_NC+0x1c0000, 4, 0x2);   //  REGreset PIGGY top   
       usleep(10);
       VMEout (0x3,LAD_NC+0x1c0000, 4, 0x0);
       //////
       
       // values must match with TTCciConfiguration.txt
       //Sequence for BGO 12 = 0xc   Channel #12 (CAL+L1): Private Pixel Command, Send CAL + L1A
       //data=0x9 ;	// sequence for BG0 9 =0x9  START (starts cyclic trigger and cyclic BGO calsync)
       //data=0xc ;  // send CalSync + L1A
       //data=0xb;  // bgo 0xb = 11 = single l1a
       data = 0x9; // bgo 9 = start
       ret = CAENVME_WriteCycle(0,ttcaddcsr0,&data,cvA24_U_DATA,cvD32);
       
       //you can call this now when you need a trigger, it should not be called more often than 1/LHC orbit ~90us!
       if(ret != cvSuccess) { 
	 cout<<"Error in write TTCci BGO Trigger"<<hex<<ret<<" "<<d<<endl;  
	 printf("Error in TTCci trigger!\n");
	 analyzeError(ret); 
       }
       printf("Trigger sent!\n");
              
       usleep(100);
              
       VMEout(0x3,LAD_C+0x1c0000,4,0x0);  //SLink disable ;Disable SpyMemWrite during readout     
       //VMEout(0x3,LAD_C+0x1c0000,4,0x8);  //SLink ignore LFF
       usleep(10);    
       
       s=1024;
       VMEmove(0x1003,LAD_C+0x160000,0,destaddr,s,4);  //  DOWN BT - FIFO Mode
       for(i=0;i<1024;i++) D_down[i]=destaddr[i];
       
       s=1024;
       VMEmove(0x1003,LAD_C+0x140000,0,destaddr,s,4);  //  UP BT - FIFO Mode
       for(i=0;i<1024;i++) D_up[i]=destaddr[i];
       
       printf("\n\n");
       printf("|||||SLID|EVTY|--Event#--|--BunchC#--|--SourceID--|FOV|FH|\n");
       printf("  0:|  %1x |  %1x |  %6x  |"  ,(D_up[0]&0xf0000000)>>28,(D_up[0]&0xf000000)>>24,(D_up[0]&0xffffff) );
       printf("      %3x  |       %3x  |  %1x| %1x| %8x          %8x\n",
	      (D_down[0]&0xfff00000)>>20,(D_down[0]&0xfff00)>>8,(D_down[0]&0xf0)>>4,D_down[0]&0xf,D_up[0],D_down[0]);
       printf("|||||----|----|----------|-----------|------------|---|--|\n");
       
       printf("\n\n|||||CH|RO|DC|PXL|PH| S-Link |CH|RO|DC|PXL|PH|||||\n");
       
       for(i=1;i<83;i++) {  //110
	 
	 printf("%3d:  %2d %2d %2d %3d %2x       ", i,(D_up[i]>>26)&0x3f,(D_up[i]>>21)&0x1f,(D_up[i]>>16)&0x1f,(D_up[i]>>8)&0xff,D_up[i]&0xff);
	 printf("  %2d %2d %2d %3d %2x              %8x          %8x\n",(D_down[i]>>26)&0x3f,(D_down[i]>>21)&0x1f,(D_down[i]>>16)&0x1f,(D_down[i]>>8)&0xff,D_down[i]&0xff,D_up[i],D_down[i]);
	 usleep(10);  
	 
	 
	 
       }
       for(i=1;i<83;i++) {  //110
	 if(((D_up[i]>>21)&0x1f)>27)cout<<dec<<DecodeError(D_up[i])<<dec<<" data 0x"<<hex<<D_up[i]<<dec<<endl;
	 if(((D_down[i]>>21)&0x1f)>27)cout<<dec<<DecodeError(D_down[i])<<dec<<" data 0x"<<hex<<D_up[i]<<dec<<endl;
	 usleep(10);  
       }
              
       //try reading spy fifo
       for(int j1=0;j1<65;j1++){ 
	 VMEin(0x3,LAD_N+0x150000,4,&d);
	 buffer1[j1]=d;
	 printf("Fifo-2 North up %d %x ",j1,d);
	 VMEin(0x3,LAD_NC+0x150000,4,&d);
	 printf("Fifo-2 NorthCenter up %d %x\n ",j1,d);
	 buffer2[j1]=d;
       }       	      
       cout<<decodePTrans(buffer1,buffer2,16)<<endl; 
       cout << " " << endl;
       decode(buffer1,buffer2,64);
       
   };break;    
  
     
   case'0':{
     goto start;
   };break;
   default:
     printf("%c is not a valid choice!\n",c);
     goto start;
     //break;
   }
   goto sub1;
   cout << "aa" << endl;
   };break;
   
   
   case'2':{
     VMEout(0x3,nCONFIG,4,0x1);
     usleep(10);
     VMEout(0x3,nCONFIG,4,0x0);
     usleep(100000); //700ms reconfigure time !!
     printf("\n.... ALTERA FRONT  devices reconfigured ! \n\n");      	    
     
   };break; 
   
   case'3':{
     VMEout(0x3,nCONFIG,4,0x2);
     usleep(10);
     VMEout(0x3,nCONFIG,4,0x0);
     usleep(100000); //700ms reconfigure time !!
     printf("\n.... ALTERA CENTER  device reconfigured ! \n\n");      	    
     
   };break; 
   
   case'0':{
     goto ende;
   };break;
   
 default:
   printf("%c is not a valid choice!\n",c);
   goto start;
   //break;
 }
 goto start;
 ende:; ; 
  */
 
 CAENVME_End(BHandle);
}// end of main

void VMEin(int AddressType, unsigned long int Address, int DataSize, unsigned long int *Data) {
  
  // cout << "vmein " << endl;
  CVAddressModifier AM;
  CVDataWidth DW;
  
  AM=cvA32_U_DATA;
  
  switch (DataSize)
    {
    case 4:  DW=cvD32; break;
    case 2:  DW=cvD16; break;
    default: DW=cvD32; break;
    }
  
  ret = CAENVME_ReadCycle(BHandle,Address,Data,AM,DW);
  //printf("%x\n",(unsigned long int)*Data);
  if(ret != cvSuccess) analyzeError(ret);
  
}

void VMEout(int AddressType, unsigned long int Address, int DataSize, unsigned long int Data) {
  
  //cout << "vmeout " << endl;
  CVAddressModifier AM;
  CVDataWidth DW;
  
  AM=cvA32_U_DATA;
  
  switch (DataSize)
    {
    case 4:  DW=cvD32; break;
    case 2:  DW=cvD16; break;
    default: DW=cvD32; break;
    }
  
  ret = CAENVME_WriteCycle(BHandle,Address,&Data,AM,DW);
  if(ret != cvSuccess) analyzeError(ret);
}

void VMEmove(int SRCpar, unsigned long int SRCaddr, int DSTpar, unsigned long int *DSTaddr,int length,int DataWidth) {
  
  CVAddressModifier AM;
  CVDataWidth DW;
  int i,byte_cnt;
  unsigned char rd_buffer[8192];
  
  AM=cvA32_U_BLT;
  
  switch (DataWidth)
    {
    case 4:  DW=cvD32; break;
    case 2:  DW=cvD16; break;
    default: DW=cvD32; break;
    }
  
  
  ret=CAENVME_BLTReadCycle(BHandle,SRCaddr,(unsigned char*)rd_buffer,(length*DataWidth),AM,DW,&byte_cnt);
  if(ret != cvSuccess)
    {
      printf("Read Failed %d\n",ret);
      analyzeError(ret);
    }
  for(i=0;i<length;i++) DSTaddr[i]=rd_buffer[(i*4)+0] + (rd_buffer[(i*4)+1]<<8) + (rd_buffer[(i*4)+2]<<16) + (rd_buffer[(i*4)+3]<<24);
  
  
  
}

int TTCRX_I2C_REG_READ( int Register_Nr){
  
  unsigned long int d;
  int  i2c_addr,i2c_nbytes;
  
  VMEout (0x3,I2C_RES, 2, 0x2);   //  RESET I2C STATE MACHINE
  usleep(10);
  VMEout (0x3,I2C_RES, 2, 0x0);
  VMEout (0x3,I2C_LOAD, 2, Register_Nr);  // I2C PAYLOAD
  
  i2c_addr=7*2; i2c_nbytes=1;
  VMEout (0x3,I2C_ADDR_RW, 2, (i2c_nbytes<<8)+(i2c_addr<<1)+0/*WRITE*/);
  //printf("%x\n",(i2c_nbytes<<8)+(i2c_addr<<1)+0 );
  
  usleep(30);
  VMEin (0x3,I2C_RD_STAT,2,&d);
  if((d&0xff)==1)printf("ERROR: BUS BUSY !! \n");
  if((d&0xff)==2)printf("ERROR: I2C_ADDR NOT ACKNOWLEDGED !! \n");
  if((d&0xff)==4)printf("ERROR: I2C_WBYTE NOT ACKNOWLEDGED !! \n");
  if((d&0xff)==8)printf("ERROR: I2C_LBYTE NOT ACKNOWLEDGED !! \n");
  
  VMEout (0x3,I2C_RES, 2, 0x2);   //  RESET I2C STATE MACHINE
  usleep(10);
  VMEout (0x3,I2C_RES, 2, 0x0);
  usleep(10);
  
  /////////////////////////////  I2C READ  //////////////////////////////////
  
  i2c_addr=(7*2)+1; i2c_nbytes=1;
  VMEout (0x3,I2C_ADDR_RW, 2, (i2c_nbytes<<8)+(i2c_addr<<1)+1/*READ*/);
  usleep(30);
  VMEin (0x3,I2C_RD_STAT,2,&d);
  if((d&0xff)==1)printf("ERROR: BUS BUSY !! \n");
  if((d&0xff)==2)printf("ERROR: I2C_ADDR NOT ACKNOWLEDGED !! \n");
  VMEin (0x3,I2C_RD_DATA,2,&d);
  return(d&0xff);
}

int TTCRX_I2C_REG_WRITE( int Register_Nr, int Value){
  
  unsigned long int d;
  int  i2c_addr,i2c_nbytes;
  
  VMEout (0x3,I2C_RES, 2, 0x2);   //  RESET I2C STATE MACHINE
  usleep(10);
  VMEout (0x3,I2C_RES, 2, 0x0);
  VMEout (0x3,I2C_LOAD, 2, Register_Nr); // I2C PAYLOAD
  
  i2c_addr=7*2; i2c_nbytes=1;
  VMEout (0x3,I2C_ADDR_RW, 2, (i2c_nbytes<<8)+(i2c_addr<<1)+0/*WRITE*/);
  //printf("%x\n",(i2c_nbytes<<8)+(i2c_addr<<1)+0 );
  
  usleep(30);
  
  VMEin (0x3,I2C_RD_STAT,2,&d);
  if((d&0xff)==1)printf("ERROR: BUS BUSY !! \n");
  if((d&0xff)==2)printf("ERROR: I2C_ADDR NOT ACKNOWLEDGED !! \n");
  if((d&0xff)==4)printf("ERROR: I2C_WBYTE NOT ACKNOWLEDGED !! \n");
  if((d&0xff)==8)printf("ERROR: I2C_LBYTE NOT ACKNOWLEDGED !! \n");
  
  VMEout (0x3,I2C_RES, 2, 0x2);   //  RESET I2C STATE MACHINE
  usleep(10);
  VMEout (0x3,I2C_RES, 2, 0x0);
  usleep(10);
  
  VMEout (0x3,I2C_LOAD, 2, Value);    // I2C PAYLOAD
  
  i2c_addr=(7*2)+1; i2c_nbytes=1;
  VMEout (0x3,I2C_ADDR_RW, 2, (i2c_nbytes<<8)+(i2c_addr<<1)+0/*WRITE*/);
  usleep(30);
  
  VMEin (0x3,I2C_RD_STAT,2,&d);
  if((d&0xff)==1)printf("ERROR: BUS BUSY !! \n");
  if((d&0xff)==2)printf("ERROR: I2C_ADDR NOT ACKNOWLEDGED !! \n");
  if((d&0xff)==4)printf("ERROR: I2C_WBYTE NOT ACKNOWLEDGED !! \n");
  if((d&0xff)==8)printf("ERROR: I2C_LBYTE NOT ACKNOWLEDGED !! \n");
  
  return(0);
}

int CLOCK_DEL_V2( int CHIPnr, int CHANNELnr, int delay)
  
{
  
  //unsigned long int d;
  //int i2c_nbytes;
  
  VMEout(0x3,LAD_C+0x198000,4,0x800+( ((CHIPnr*9) + CHANNELnr)<<4 )+delay); 
  
  if ( (delay >= 0) && ( delay < 6 ) ){ 
    CLOCKsel[CHIPnr] = CLOCKsel[CHIPnr] |  ( 1 << (CHANNELnr-1) ); // use neg. clocked Register ! 
    VMEout(0x3,CHIP[CHIPnr] + 0x1b0000,4,CLOCKsel[CHIPnr]  );
  }
  if ( (delay >= 6) && ( delay < 13 ) ){  
    CLOCKsel[CHIPnr] = CLOCKsel[CHIPnr] &  (~( 1 << (CHANNELnr-1)) );  // use pos. clocked Register !      
    VMEout(0x3,CHIP[CHIPnr] + 0x1b0000,4,CLOCKsel[CHIPnr]  );
  }
  if ( (delay >= 13) && ( delay < 16 ) ){ 
    CLOCKsel[CHIPnr] = CLOCKsel[CHIPnr] |  ( 1 << (CHANNELnr-1) ); // use neg. clocked Register ! 
    VMEout(0x3,CHIP[CHIPnr] + 0x1b0000,4,CLOCKsel[CHIPnr]  );
  }
  
  //printf("%x\n",0x800+( ((CHIPnr*9) + CHANNELnr)<<4 )+delay);
  
  return(0);
}
 
///////////////////////////////////////////////////////////////////////////
// Decode the FIFO-2 data in  transparent mode from piggy
// ADD SIZE
int decodePTrans(unsigned long * data1, unsigned long * data2, const int length) {
  unsigned long mydat[16]=
  {0x80,0x90,0xa0,0xb0,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0xc0,0xd0,0xe0,0xf0};

if(length<16) return -2;
  // Print & analyze the data buffers
int tempcode1=0;
int tempcode2=0;
int tempcode3=0;

  for(int icx=0;icx<16;icx++) {
if( ((data1[icx]&0xf0)==0x80) && ((data2[icx]&0xf0)==0x80) ) {if((data1[icx]!=data2[icx])) tempcode3=4;}
if( ((data1[icx]&0xf0)==0x90) && ((data2[icx]&0xf0)==0x90) ) {if((data1[icx]!=data2[icx])) tempcode3=4;}



if((data1[icx]&0xf0)!=(data2[icx]&0xf0))tempcode1=1;
if( ((data1[icx]&0xf0)!=mydat[icx])|((data2[icx]&0xf0)!=mydat[icx]))tempcode2=2;

  }





  //if((tempcode1)!=0)cout<<"Buffers 0-15 dont match each other!"<<endl;
  //if((tempcode2)!=0)cout<<"Buffers 0-15 dont match expected pattern!"<<endl;
  //if((tempcode3)!=0)cout<<"Buffers 0-15 dont match event numbers!"<<endl;

return (tempcode1+tempcode2+tempcode3);


} // end

///////////////////////////////////////////////////////////////////////////
//// Decode the FIFO-2 data in  transparent mode from piggy
//// ADD SIZE
int decodePTrans2(unsigned long * data1, unsigned long * data2, const int length) {
// unsigned long mydat[16]=
// {0x80,0x90,0xa0,0xb0,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0xc0,0xd0,0xe0,0xf0};

if(length<16) return -2;
  // Print & analyze the data buffers
  int tempcode1=0;//trailer
  int tempcode2=0;//rocs
  int tempcode3=0;//event
 int tempcode4=0;//header
 int tempcode5=0;
 //int tempcode6=0;
//header event number check
//
int mytr1=0;
int mytr2=0;

    for(int icx=0;icx<8;icx++) {
    if( ((data1[icx]&0xf0)==0x80) && ((data2[icx]&0xf0)==0x80) ) {if((data1[icx]!=data2[icx])) tempcode3=4;}
    if( ((data1[icx]&0xf0)==0x90) && ((data2[icx]&0xf0)==0x90) ) {if((data1[icx]!=data2[icx])) tempcode3=4;}

    //if( ((data1[icx]&0xf0)==0xa0) && ((data2[icx]&0xf0)==0xa0) ) {if((data1[icx]!=data2[icx])) tempcode3=4;}
   // if( ((data1[icx]&0xf0)==0xb0) && ((data2[icx]&0xf0)==0xb0) ) {if((data1[icx]!=data2[icx])) tempcode3=4;}

    if((data1[icx]&0xf0)==0x80)mytr1++;
    if((data1[icx]&0xf0)==0x90)mytr1++;
    if((data1[icx]&0xf0)==0xa0)mytr1++;
    if((data1[icx]&0xf0)==0xb0)mytr1++;
    if((data2[icx]&0xf0)==0x80)mytr2++;
    if((data2[icx]&0xf0)==0x90)mytr2++;
    if((data2[icx]&0xf0)==0xa0)mytr2++;
    if((data2[icx]&0xf0)==0xb0)mytr2++;


      }
if((mytr1!=4)|(mytr2!=4))tempcode4=8;
//Trailer check
mytr1=0;
mytr2=0;
    for(int icx=0;icx<128;icx++) {
    if((data1[icx]&0xf0)==0xc0)mytr1++;
    if((data1[icx]&0xf0)==0xd0)mytr1++;
    if((data1[icx]&0xf0)==0xe0)mytr1++;
    if((data1[icx]&0xf0)==0xf0)mytr1++;
    if((data2[icx]&0xf0)==0xc0)mytr2++;
    if((data2[icx]&0xf0)==0xd0)mytr2++;
    if((data2[icx]&0xf0)==0xe0)mytr2++;
    if((data2[icx]&0xf0)==0xf0)mytr2++; 

      }
if((mytr1!=4)|(mytr2!=4))tempcode1=1;

//Rocs check
mytr1=0;
mytr2=0;
    for(int icx=0;icx<128;icx++) {
    if((data1[icx]&0xf0)==0x70)mytr1++;
    if((data2[icx]&0xf0)==0x70)mytr2++;

      }
if((mytr1!=8)||(mytr2!=8))tempcode2=2;
//hits check
mytr1=0;
mytr2=0;

    for(int icx=0;icx<128;icx++) {
    if((data1[icx]&0xff)==0x10)mytr1++;
    if((data2[icx]&0xff)==0x10)mytr2++;
    if((data1[icx]&0xff)==0x2a)mytr1++;
    if((data2[icx]&0xff)==0x2a)mytr2++;
    if((data1[icx]&0xff)==0x31)mytr1++;
    if((data2[icx]&0xff)==0x31)mytr2++;
    if((data1[icx]&0xfe)==0x44)mytr1++;
    if((data2[icx]&0xfe)==0x44)mytr2++;

      }

if((mytr1<7*4)||(mytr2<6*4))tempcode5=16;



      if((tempcode1)!=0)cout<<"missed trailer"<<endl;
      if((tempcode2)!=0)cout<<"missed roc"<<endl;
      if((tempcode3)!=0)cout<<"event number mismatch"<<endl;
      if((tempcode4)!=0)cout<<"missed header"<<endl;
      if((tempcode5)!=0)cout<<"missed hits"<<endl;


      return (tempcode1+tempcode2+tempcode3+tempcode4+tempcode5);


      } // end
//


///////////////////////////////////////////////////////////////////////////
// Decode the FIFO-2 data in  transparent mode from piggy
// ADD SIZE
// Checks for 1 hit per ROC from both chs
void decode(unsigned long * data1, unsigned long * data2, const int length) {
  //unsigned long mydat[16]=
  //   {0x80,0x90,0xa0,0xb0,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0xc0,0xd0,0xe0,0xf0};
  
  if(length<64) return;
  // Print & analyze the data buffers
  int tbmheader=0; //int rocheader=0; int correctdata=0; 
  int tbmtrailer=0;
  //bool tbmh=false; bool tbmt=false; bool roch=false; bool data=false;

  int tbmheader2=0;

  if ( (data1[0]&0xf0)==0x80 && (data1[1]&0xf0)==0x90 && (data1[2]&0xf0)==0xa0 && (data1[3]&0xf0)==0xb0){
    tbmheader++;
    a[0]++;
  }

  if( (data2[0]&0xf0)==0x80 && (data2[1]&0xf0)==0x90 && (data2[2]&0xf0)==0xa0 && (data2[3]&0xf0)==0xb0 ) {
    tbmheader2++;
    a2[0]++;
  }

  if ( (data1[4]&0xf0)==0xc0 && (data1[5]&0xf0)==0xd0 && (data1[6]&0xf0)==0xe0 && (data1[7]&0xf0)==0xf0 ) {
    tbmtrailer++;
    a[3]++;
  }

  if( (data2[4]&0xf0)==0xc0 && (data2[5]&0xf0)==0xd0 && (data2[6]&0xf0)==0xe0 && (data2[7]&0xf0)==0xf0 ) {
    tbmtrailer++;
    a2[3]++;
  }
  
  //cout << "tbmheader = " << tbmheader << endl;
  //cout << "rocheader = " << rocheader << endl;
  //cout << "correctdata = " << correctdata << endl;
  //cout << "tbmtrailer = " << tbmtrailer << endl;
  
  return;
      
      
} 

// Decode the FIFO-2 data in  transparent mode from piggy
// ADD SIZE
// Checks for 1 hit per ROC from both chs
void decode_withROCs(unsigned long * data1, unsigned long * data2, const int length) {
  //unsigned long mydat[16]=
  //   {0x80,0x90,0xa0,0xb0,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0xc0,0xd0,0xe0,0xf0};
  
  if(length<64) return;
  // Print & analyze the data buffers
  int tbmheader=0; int rocheader=0; int correctdata=0; int tbmtrailer=0;
  // bool tbmh=false; bool tbmt=false; bool roch=false; bool data=false;
  if ( (data1[0]&0xf0)==0x80 && (data1[1]&0xf0)==0x90 && (data1[2]&0xf0)==0xa0 && (data1[3]&0xf0)==0xb0  ) {
    tbmheader++;
    a[0]++;
  }
  
  if ( (data2[0]&0xf0)==0x80 && (data2[1]&0xf0)==0x90 && (  data2[2]&0xf0)==0xa0 && (data2[3]&0xf0)==0xb0 ) {
    tbmheader++;
    a2[0]++;
  }
  
  
  if ( (data1[4]&0xf0)==0x70 && (data1[11]&0xf0)==0x70 && (data1[18]&0xf0)==0x70 && (data1[25]&0xf0)==0x70 && (data1[32]&0xf0)==0x70 && (data1[39]&0xf0)==0x70 && (data1[46]&0xf0)==0x70 && (data1[53]&0xf0)==0x70  ){
    rocheader++;
    a[1]++;
  }
  
  if (  (data2[4]&0xf0)==0x70 && (data2[11]&0xf0)==0x70 && (data2[18]&0xf0)==0x70 && (data2[25]&0xf0)==0x70 && (data2[32]&0xf0)==0x70 && (data2[39]&0xf0)==0x70 && (data2[46]&0xf0)==0x70 && (data2[53]&0xf0)==0x70 ){
    rocheader++;
    a2[1]++;
  }
  
  for ( int icx=0;icx<8;icx++  ) {
    if ( (data1[5+(7*icx)]&0xf0)==0x10 &&  (data1[6+(7*icx)]&0xf0)==0x20 && (data1[7+(7*icx)]&0xf0)==0x30 &&  (data1[8+(7*icx)]&0xf0)==0x40 && (data1[9+(7*icx)]&0xf0)==0x50 &&  (data1[10+(7*icx)]&0xf0)==0x60  ){
      correctdata++;
      a[2]++;
    }
  }
  
  for ( int icx=0;icx<8;icx++  ) {
    if (  (data2[5+(7*icx)]&0xf0)==0x10 &&  (data2[6+(7*icx)]&0xf0)==0x20 && (data2[7+(7*icx)]&0xf0)==0x30 &&  (data2[8+(7*icx)]&0xf0)==0x40 && (data2[9+(7*icx)]&0xf0)==0x50 &&  (data2[10+(7*icx)]&0xf0)==0x60  ){
      correctdata++;
      a2[2]++;
    }
  }
  
  
  if ( (data1[60]&0xf0)==0xc0 && (data1[61]&0xf0)==0xd0 && (data1[62]&0xf0)==0xe0 && (data1[63]&0xf0)==0xf0  ) {
    tbmtrailer++;
    a[3]++;
  }
  
  if (  (data2[60]&0xf0)==0xc0 && (data2[61]&0xf0)==0xd0 && (data2[62]&0xf0)==0xe0 && (data2[63]&0xf0)==0xf0 ) {
    tbmtrailer++;
    a2[3]++;
  }
  
  
  //cout << "tbmheader = " << tbmheader << endl;
  //cout << "rocheader = " << rocheader << endl;
  //cout << "correctdata = " << correctdata << endl;
  //cout << "tbmtrailer = " << tbmtrailer << endl;
  
  return;
  
  
}



// end
// Decode error FIFO
// Works for both, the error FIFO and the SLink error words. d.k. 25/04/07
int DecodeError(int word) {
  int status = -1;
  const unsigned int  errorMask      = 0x3e00000;
  const unsigned int  dummyMask      = 0x03600000;
  const unsigned int  gapMask        = 0x03400000;
  const unsigned int  timeOut        = 0x3a00000;
  const unsigned int  eventNumError  = 0x3e00000;
  const unsigned int  trailError     = 0x3c00000;
  const unsigned int  fifoError      = 0x3800000;
 
//  const unsigned int  timeOutChannelMask = 0x1f;  // channel mask for timeouts
  //const unsigned int  eventNumMask = 0x1fe000; // event number mask
  const unsigned int  channelMask = 0xfc000000; // channel num mask
  const unsigned int  tbmEventMask = 0xff;    // tbm event num mask
  const unsigned int  overflowMask = 0x100;   // data overflow
  const unsigned int  tbmStatusMask = 0xff;   //TBM trailer info
  const unsigned int  BlkNumMask = 0x700;   //pointer to error fifo #
  const unsigned int  FsmErrMask = 0x600;   //pointer to FSM errors
  const unsigned int  RocErrMask = 0x800;   //pointer to #Roc errors
  const unsigned int  ChnFifMask = 0x1f;   //channel mask for fifo error
  const unsigned long  ChnFifMask0 = 0x1;   //channel mask for fifo error
  const unsigned long  ChnFifMask1 = 0x2;   //channel mask for fifo error
  const unsigned long  ChnFifMask2 = 0x4;   //channel mask for fifo error
  const unsigned long  ChnFifMask3 = 0x8;   //channel mask for fifo error
  const unsigned long  ChnFifMask4 = 0x10;   //channel mask for fifo error
  const unsigned int  Fif2NFMask = 0x40;   //mask for fifo2 NF
  const unsigned int  TrigNFMask = 0x80;   //mask for trigger fifo NF
 
 const int offsets[8] = {0,4,9,13,18,22,27,31};
 
 //cout<<"error word "<<hex<<word<<dec<<endl;                                                                             
 bool PRINT_DUMMY=true;
  if( (word&errorMask) == dummyMask ) { // DUMMY WORD
    if(PRINT_DUMMY) cout<<" Dummy word"<<endl;
    return 0;
  } else if( (word&errorMask) == gapMask ) { // GAP WORD
    if(PRINT_DUMMY) cout<<" Gap word"<<endl;
    return 0;
  } else if( (word&errorMask)==timeOut ) { // TIMEOUT
     // More than 1 channel within a group can have a timeout error
     unsigned int index = (word & 0x1F);  // index within a group of 4/5
     unsigned int chip = (word& BlkNumMask)>>8;
     int offset = offsets[chip];
     cout<<"Timeout Error- channels: ";
     for(int i=0;i<5;i++) {
       if( (index & 0x1) != 0) {
         int chan = offset + i + 1;
         cout<<chan<<" ";
       }
       index = index >> 1;
     }
     cout<<endl;
     //end of timeout  chip and channel decoding
      
   } else if( (word&errorMask) == eventNumError ) { // EVENT NUMBER ERROR
     unsigned int channel =  (word & channelMask) >>26;
     unsigned int tbm_event   =  (word & tbmEventMask);
      
     cout<<"Event Number Error- channel: "<<channel<<" tbm event nr. "
         <<tbm_event<<endl;
                                                                                            
   } else if( ((word&errorMask) == trailError)) {
    unsigned int channel =  (word & channelMask) >>26;
    unsigned int tbm_status   =  (word & tbmStatusMask);
    if(word & RocErrMask)
      cout<<"Number of Rocs Error- "<<"channel: "<<channel<<" "<<endl;
    if(word & FsmErrMask)
      cout<<"Finite State Machine Error- "<<"channel: "<<channel
	  <<" Error status:0x"<<hex<< ((word & FsmErrMask)>>9)<<dec<<" "<<endl;
    if(word & overflowMask)
      cout<<"Overflow Error- "<<"channel: "<<channel<<" "<<endl;
    //if(!((word & RocErrMask)|(word & FsmErrMask)|(word & overflowMask)))
    if(tbm_status!=0)
      cout<<"Trailer Error- "<<"channel: "<<channel<<" TBM status:0x"
	  <<hex<<tbm_status<<dec<<" "<<endl;
    
  } else if((word&errorMask)==fifoError) {
    if(word & Fif2NFMask) cout<<"A fifo 2 is Nearly full- ";
    if(word & TrigNFMask) cout<<"The trigger fifo is nearly Full - ";
    if(word & ChnFifMask){
     cout<<"fifo-1 is nearly full for channel(s) of this FPGA//";
     if(word & ChnFifMask0)cout<<" 1 //";
     if(word & ChnFifMask1)cout<<" 2 //";
     if(word & ChnFifMask2)cout<<" 3 //";
     if(word & ChnFifMask3)cout<<" 4 //";
     if(word & ChnFifMask4)cout<<" 5 ";
                         }
    
    cout<<endl;
    
  } else {
    cout<<" Unknown error?";
  }
  
  //unsigned int event   =  (word & eventNumMask) >>13;
  //unsigned int tbm_status   =  (word & tbmStatusMask);
    
  //if(event>0) cout<<":event: "<<event;
  //cout<<endl;

  return status;
}
///////////////////////////////////////////////////////////////////////////

void BinDisplay(unsigned long int In_Word, int Bits,int Header,int del)  

{
  int i1;
  unsigned long mask;
  
  mask=1<<(Bits-1);  
  
  if (Header) {
    //printf("\n\233\67\155\n");
    if(del) printf("|");
    for (i1=Bits;i1>0;i1--) {
      printf("%1d",(i1-1)%10);
    if(del) {if ( !((i1-1)%4) ) printf("|");}
    }
    printf("\n");
    //printf("\233\60\155 \n");
  } 
  if (del) printf("|");
  for (i1=0;i1<Bits;i1++) {
    if ((In_Word<<i1) & mask) printf("1");
    else printf("0");
    if(del) {if ( !((i1+1)%4) ) printf("|");  }
  }
  //printf("\n");
  
  return;
  
}

std::string BinToString(unsigned long int In_Word, int Bits)  

{
  int i1;
  unsigned long mask;
 
  std::string buffer("") ; 
  mask=1<<(Bits-1);  
  
  for (i1=0;i1<Bits;i1++) {
    if ((In_Word<<i1) & mask) buffer.append("1");
    else  buffer.append("0");
  }
  //printf("\n");
  
  //std::cout << "buffer " << buffer << endl;

  return buffer;
  
}
