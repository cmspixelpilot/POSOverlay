#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>

#include "CalibFormats/SiPixelObjects/interface/PixelPh1FEDCard.h"

using namespace std;
using namespace pos;

PixelPh1FEDCard::PixelPh1FEDCard()
 : PixelConfigBase(" "," "," ")
{
  clear();
}

PixelPh1FEDCard::PixelPh1FEDCard(vector<vector<string> >& tableMat)
 : PixelConfigBase(" "," "," ")
{
  clear();

  assert(0);
}

PixelPh1FEDCard::PixelPh1FEDCard(string fileName)
  : PixelConfigBase(" "," "," ")
{
  std::string mthn = "]\t[PixelPh1FEDCard::PixelPh1FEDCard()]\t\t\t\t    " ;
  const bool debug = false;
  
  clear();

  if (debug) cout << __LINE__ << "]\t" << mthn <<" Get setup parameters from file "<<fileName<<endl;
  FILE *infile = fopen((fileName.c_str()),"r");
  if (infile == NULL)  throw std::runtime_error("Failed to open FED Card parameter file: "+fileName); 

  //Fed Base Address
  fscanf(infile,"FED Base address                         :%lx\n", &FEDBASE_0);
  fscanf(infile,"FEDID Number                             :%lx\n", &fedNumber);

//  if(debug) cout << __LINE__ << mthn << "FED Base address, FED # : " << std::hex << FEDBASE_0 << std::dec << std::endl ;
//  if(debug) printf("FED Base address, FED # :%lx\n",FEDBASE_0);
  //if(FEDBASE != FEDBASE_0) cout<< __LINE__ << "]\t" << mthn << " Inconsistent FED base address?"<<endl;
//  if(debug) cout << __LINE__ << mthn << "FEDID #                 : " << std::hex << fedNumber << std::dec << std::endl ;
//  if(debug) printf("FEDID # :%lx\n",fedNumber);
 
  // Number of ROCs
  int ijx=0;
  for(int i=0;i<48;i++){
  ijx=i+1;
    fscanf(infile,"Number of ROCs Chnl %d:%d\n",&ijx,&NRocs[i]);
    if(debug)printf("Number of ROCs per Chnl %d:%d\n",ijx,NRocs[i]);
  }

  //These bits turn off(1) and on(0) channels
  fscanf(infile,"Channel Enbable bits Ch  1-32 (on = 0):%x\n", &cntrl_1);
  if(debug) printf("Channel Enbable bits Ch  1-32 (on = 0):0x%x\n", cntrl_1);
  fscanf(infile,"Channel Enbable bits Ch 33-64 (on = 0):%x\n", &cntrl_2);
  if(debug) printf("Channel Enbable bits Ch 33-64 (on = 0):0x%x\n", cntrl_2);
  fscanf(infile,"Channel Enbable bits Ch 65-96 (on = 0):%x\n", &cntrl_3);
  if(debug) printf("Channel Enbable bits Ch 65-96 (on = 0):0x%x\n", cntrl_3);
  
  //These are delays to the TTCrx
  fscanf(infile,"TTCrx Coarse Delay Register 2:%d\n",&CoarseDel);
  fscanf(infile,"TTCrc      ClkDes2 Register 3:%x\n",&ClkDes2);
  fscanf(infile,"TTCrc Fine Dlay ClkDes2 Reg 1:%d\n",&FineDes2Del);
  if(debug)printf("TTCrx Coarse Delay Register 2:%d\n",CoarseDel);
  if(debug)printf("TTCrc      ClkDes2 Register 3:%x\n",ClkDes2);
  if(debug)printf("TTCrc Fine Dlay ClkDes2 Reg 1:%d\n",FineDes2Del);
  
  // Control register
  fscanf(infile,"Center Chip Control Reg:%x\n",&Ccntrl);
  if(debug)printf("Center Chip Control Reg:0x%x\n",Ccntrl);
  fscanf(infile,"Initial Slink DAQ mode:%d\n",&modeRegister);
  if(debug)printf("Initial Slink DAQ mode:%d\n",modeRegister);
  
   //These bits set ADC Gain/Range 1Vpp(0) and 2Vpp(1) for channels
  fscanf(infile,"Channel ADC Gain bits (1Vpp = 0):%llx\n", (long long unsigned*)&adcg);
  if(debug) printf("Channel ADC Gain bits (1Vpp = 0):%llx\n", (long long unsigned)adcg);

  // TBM trailer mask
  fscanf(infile,"TBM trailer mask (0xff = all masked):%llx\n", (long long unsigned*)&TBMmask);
  if(debug) printf("TBM trailer mask (0xff = all masked):%llx\n", (long long unsigned)TBMmask);

  // Private fill/gap word 
  fscanf(infile,"Private 32 bit word:%x\n", &Pword);
  if(debug) printf("Private 32 bit word:%x\n", Pword);

  //set the channel you want to read in transparent + scope fifos
  fscanf(infile,"Transparent/scope channel(0-47):%d\n", &TransScopeCh);
  if (debug) printf("Transparent/scope channel(0-47):%d\n", TransScopeCh);

  //These bits set the number of Out of consecutive out of sync events until a TTs OOs 
  fscanf(infile,"Number of Consecutive (max ?) Out of Syncs till TTs OOS set:%d\n", &Ooslvl);
  if(debug) printf("Number of Consecutive (max ?) Out of Syncs till TTs OOS set:%d\n",Ooslvl);

  //These bits set the number of Empty events until a TTs Error 
  fscanf(infile,"Number of Consecutive (max ?) Empty events till TTs ERR set:%d\n", &Errlvl);
  if(debug) printf("Number of Consecutive (max ?) Empty events till TTs ERR set:%d\n",Errlvl);

  //These bits set the Almost Full level in fifo-1, Almost full = TTs BUSY in fifo-1 N
  fscanf(infile,"Fifo-1 almost full level,sets TTs BUSY (max ?):%d\n", &fifo1Bzlvl);
  if(debug) printf("Fifo-1 almost full level,sets TTs BUSY (max ?):%d\n", fifo1Bzlvl);

  //These bits set the Almost Full level in fifo-3, Almost full = TTs WARN in fifo-3
  fscanf(infile,"Fifo-3 almost full level,sets TTs WARN (max ?):%d\n", &fifo3Wrnlvl);
  if(debug) printf("Fifo-3 almost full level,sets TTs WARN (max ?):%d\n",fifo3Wrnlvl);

  fscanf(infile,"FED Master delay 0=0,1=32,2=48,3=64:%d\n", &FedTTCDelay);
  if(debug) printf("FED Master delay 0=0,1=32,2=48,3=64:%d\n", FedTTCDelay);

  fscanf(infile,"TTCrx Register 0 fine delay ClkDes1:%d\n",&FineDes1Del);
  if(debug) printf("TTCrx Register 0 fine delay ClkDes1:%d\n",FineDes1Del);

  //These bits set the hit limit in fifo-1 for an event
  fscanf(infile,"fifo-1 hit limit (max ? (hard) ? (soft):%d\n", &hitlimit);
  if(debug) printf("fifo-1 hit limit (max ? (hard) ? (soft):%d\n",hitlimit);

  //testreg
  fscanf(infile,"testreg:%x\n", &testreg);
  if(debug) printf("testreg:%x\n", testreg);

  fscanf(infile,"packet_nb:%x\n", &packet_nb);
  if(debug) printf("packet_nb:0x%x\n", packet_nb);

  fscanf(infile,"Set BUSYWHENBEHIND by this many triggers with timeouts:%d\n",&BusyWhenBehind);
  if(debug) printf("Set BUSYWHENBEHIND by this many triggers with timeouts:%d\n",BusyWhenBehind);
				
  fscanf(infile,"D[0]=1 enable fed-stuck reset D[1]=1 disable ev# protect(dont):%x\n",&FeatureRegister);
  if(debug) printf("D[0]=1 enable fed-stuck reset D[1]=1 disable ev# protect(dont):%x\n",FeatureRegister); 
		 
  fscanf(infile,"Limit for fifo-2 almost full (point for the TTS flag):%x\n",&FIFO2Limit);
  if(debug) printf("Limit for fifo-2 almost full (point for the TTS flag):%x\n",FIFO2Limit);	 
		 
  fscanf(infile,"Limit for consecutive timeout OR OOSs:%d\n",&TimeoutOROOSLimit);
  if(debug) printf("Limit for consecutive timeout OR OOSs:%d\n",TimeoutOROOSLimit);	 
		 
  fscanf(infile,"Number of simulated hits per ROC for internal generator:%d\n",&SimHitsPerRoc);
  if(debug) printf("Number of simulated hits per ROC for internal generator:%d\n",SimHitsPerRoc);	 

  fscanf(infile,"Miniumum hold time for busy (changing definition):%d\n",&BusyHoldMin);
  if(debug) printf("Miniumum hold time for busy (changing definition):%d\n",BusyHoldMin);	 
		 
  fscanf(infile,"Trigger Holdoff in units of 25us(0=none):%d\n",&TriggerHoldoff);
  if(debug) printf("Trigger Holdoff in units of 25us(0=none):%d\n",TriggerHoldoff);	 
		 
  fscanf(infile,"Spare fedcard input 1:%d\n",&SPARE1);
  if(debug)
    printf("Spare fedcard input 1:%d\n",SPARE1);	 
  fscanf(infile,"Spare fedcard input 2:%d\n",&SPARE2);
  if(debug)
    printf("Spare fedcard input 2:%d\n",SPARE2);	 
  fscanf(infile,"Spare fedcard input 3:%d\n",&SPARE3);
  if(debug)
    printf("Spare fedcard input 3:%d\n",SPARE3);	 
  fscanf(infile,"Spare fedcard input 4:%d\n",&SPARE4);
  if(debug)
    printf("Spare fedcard input 4:%d\n",SPARE4);	 
  fscanf(infile,"Spare fedcard input 5:%d\n",&SPARE5);
  if(debug)
    printf("Spare fedcard input 5:%d\n",SPARE5);	 
  fscanf(infile,"Spare fedcard input 6:%d\n",&SPARE6);
  if(debug)
    printf("Spare fedcard input 6:%d\n",SPARE6);	 
  fscanf(infile,"Spare fedcard input 7:%d\n",&SPARE7);
  if(debug)
    printf("Spare fedcard input 7:%d\n",SPARE7);	 
  fscanf(infile,"Spare fedcard input 8:%d\n",&SPARE8);
  if(debug)
    printf("Spare fedcard input 8:%d\n",SPARE8);	 
  fscanf(infile,"Spare fedcard input 9:%d\n",&SPARE9);
  if(debug)
    printf("Spare fedcard input 9:%d\n",SPARE9);	 
  fscanf(infile,"Spare fedcard input 10:%d\n",&SPARE10);
  if(debug)
    printf("Spare fedcard input 10:%d\n",SPARE10);	 
   
  fclose(infile);

  Ccntrl_original=Ccntrl;
  modeRegister_original=modeRegister;

  cntrl_1_original=cntrl_1;
  cntrl_2_original=cntrl_2;
  cntrl_3_original=cntrl_3;
}

void PixelPh1FEDCard::clear() {
  FEDBASE_0 = 0;
  fedNumber = 999;

  for(int i=0;i<48;i++) 
    NRocs[i] = 0;

  cntrl_1        = 0; 
  cntrl_2        = 0; 
  cntrl_3        = 0; 
  cntrl_1_original = 0;
  cntrl_2_original = 0;
  cntrl_3_original = 0;
  CoarseDel    = 0;
  ClkDes2      = 0;
  FineDes2Del  = 0;
  FineDes1Del  = 0;
  Ccntrl       = 0;
  Ccntrl_original = 0;
  modeRegister = 0;
  modeRegister_original = 0;
  adcg         = 0;
  TBMmask      = 0;
  Pword        = 0;
  TransScopeCh = 0;
  Ooslvl       = 0;
  Errlvl       = 0;
  fifo1Bzlvl   = 0;
  fifo3Wrnlvl  = 0;
  packet_nb = 0;
  
  BusyHoldMin	    = 0;
  BusyWhenBehind    = 0;
  FeatureRegister   = 0;
  FIFO2Limit	    = 0;
  SimHitsPerRoc     = 0;
  TimeoutOROOSLimit = 0;
  TriggerHoldoff    = 0;

  SPARE1	    = 0;
  SPARE2	    = 0;
  SPARE3	    = 0;
  SPARE4	    = 0;
  SPARE5	    = 0;
  SPARE6	    = 0;
  SPARE7	    = 0;
  SPARE8	    = 0;
  SPARE9	    = 0;
  SPARE10	    = 0;
}

void PixelPh1FEDCard::writeASCII(std::string dir) const {
  std::string mthn = "[PixelPh1FEDCard::writeASCII()]\t\t\t\t    " ;

  ostringstream s1;
  s1<<fedNumber;
  std::string fedNum=s1.str();

  if (dir!="") dir+="/";

  std::string filename=dir+"params_fed_"+fedNum+".dat";

  FILE *outfile = fopen((filename.c_str()),"w");
  if (outfile == NULL) {
    cout<< __LINE__ << "]\t" << mthn << "Could not open file: " << filename << " for writing" << endl; 
    return;
  }
  
  fprintf(outfile,"FED Base address                         :0x%lx\n", FEDBASE_0);
  fprintf(outfile,"FEDID Number                             :0x%lx\n", fedNumber);
  for(int i=0;i<48;i++)
    fprintf(outfile,"Number of ROCs Chnl %d:%d\n",i+1,NRocs[i]);
  fprintf(outfile,"Channel Enbable bits Ch  1-32 (on = 0):0x%x\n", cntrl_1);
  fprintf(outfile,"Channel Enbable bits Ch 33-64 (on = 0):0x%x\n", cntrl_2);
  fprintf(outfile,"Channel Enbable bits Ch 65-96 (on = 0):0x%x\n", cntrl_3);
  fprintf(outfile,"TTCrx Coarse Delay Register 2:%d\n",CoarseDel);
  fprintf(outfile,"TTCrc      ClkDes2 Register 3:0x%x\n",ClkDes2);
  fprintf(outfile,"TTCrc Fine Dlay ClkDes2 Reg 1:%d\n",FineDes2Del);
  fprintf(outfile,"Center Chip Control Reg:0x%x\n",Ccntrl);
  fprintf(outfile,"Initial Slink DAQ mode:%d\n",modeRegister);
  fprintf(outfile,"Channel ADC Gain bits (1Vpp = 0):0x%llx\n", (long long unsigned)adcg);
  fprintf(outfile,"TBM trailer mask (0xff = all masked):0x%llx\n", (long long unsigned)TBMmask);
  fprintf(outfile,"Private 32 bit word:0x%x\n", Pword);
  fprintf(outfile,"Transparent/scope channel(0-47):%d\n", TransScopeCh);
  fprintf(outfile,"Number of Consecutive (max ?) Out of Syncs till TTs OOS set:%d\n", Ooslvl);
  fprintf(outfile,"Number of Consecutive (max ?) Empty events till TTs ERR set:%d\n", Errlvl);
  fprintf(outfile,"Fifo-1 almost full level,sets TTs BUSY (max ?):%d\n", fifo1Bzlvl);
  fprintf(outfile,"Fifo-3 almost full level,sets TTs WARN (max ?):%d\n", fifo3Wrnlvl);
  fprintf(outfile,"FED Master delay 0=0,1=32,2=48,3=64:%d\n", FedTTCDelay);
  fprintf(outfile,"TTCrx Register 0 fine delay ClkDes1:%d\n", FineDes1Del);
  fprintf(outfile,"fifo-1 hit limit (max ? (hard) ? (soft):%d\n", hitlimit); //ch 1-9
  fprintf(outfile,"testreg:0x%x\n", testreg);
  fprintf(outfile,"packet_nb:0x%x\n", packet_nb);
  fprintf(outfile,"Set BUSYWHENBEHIND by this many triggers with timeouts:%d\n", BusyWhenBehind);
  fprintf(outfile,"D[0]=1 enable fed-stuck reset D[1]=1 disable ev# protect(dont):0x%x\n", FeatureRegister);	 
  fprintf(outfile,"Limit for fifo-2 almost full (point for the TTS flag):0x%x\n", FIFO2Limit);	 
  fprintf(outfile,"Limit for consecutive timeout OR OOSs:%d\n", TimeoutOROOSLimit);	 
  fprintf(outfile,"Number of simulated hits per ROC for internal generator:%d\n", SimHitsPerRoc);	 
  fprintf(outfile,"Miniumum hold time for busy (changing definition):%d\n", BusyHoldMin);	 
  fprintf(outfile,"Trigger Holdoff in units of 25us(0=none):%d\n", TriggerHoldoff);	 
		 
  fprintf(outfile,"Spare fedcard input 1:%d\n",SPARE1);	 
  fprintf(outfile,"Spare fedcard input 2:%d\n",SPARE2);	 
  fprintf(outfile,"Spare fedcard input 3:%d\n",SPARE3);	 
  fprintf(outfile,"Spare fedcard input 4:%d\n",SPARE4);	 
  fprintf(outfile,"Spare fedcard input 5:%d\n",SPARE5);	 
  fprintf(outfile,"Spare fedcard input 6:%d\n",SPARE6);	 
  fprintf(outfile,"Spare fedcard input 7:%d\n",SPARE7);	 
  fprintf(outfile,"Spare fedcard input 8:%d\n",SPARE8);	 
  fprintf(outfile,"Spare fedcard input 9:%d\n",SPARE9);	 
  fprintf(outfile,"Spare fedcard input 10:%d\n",SPARE10);	 

  fclose(outfile);
}

uint64_t PixelPh1FEDCard::enabledChannels() {
  return ~cntrl; 
}

bool PixelPh1FEDCard::useChannel(unsigned int iChannel){
  assert(iChannel>0&&iChannel<49);
  return (enabledChannels()>>(iChannel-1))&0x1LL;
} 

void PixelPh1FEDCard::setChannel(unsigned int iChannel, bool mode){
  assert(iChannel>0&&iChannel<49);
  uint64_t mask=enabledChannels();
  uint64_t bit=0x1LL<<(iChannel-1);
  if (mode) {
    mask=mask|bit;
  }
  else{
    bit=~bit;
    mask=mask&bit;
  }
  cntrl = ~mask;
}  

void PixelPh1FEDCard::restoreChannelMasks(){
  cntrl=cntrl_original;
}

void PixelPh1FEDCard::restoreControlAndModeRegister(){
  Ccntrl=Ccntrl_original;
  modeRegister=modeRegister_original;
}
