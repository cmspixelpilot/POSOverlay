// Program to test the PixelFEDInterface class.
// Uses HAL VME FED and TTCvi access.
// Will Johns & Danek Kotlinski. 3/06.
//
// Modes : 
// trigger from TTC or from VME
// transparent      or normal data mode 
// real data        or simulated data from DAC memory
//
// Modifications:
// Add the TTCvi class for testing purposes. For users who do not 
// have or do not want to use the TTCvi please comment it out. d.k. 11/4/06
//

#include <iostream>
#include <time.h>

using namespace std;

// HAL includes
#include "VMEDevice.hh"
#include "VMEAddressTable.hh"
#include "VMEAddressTableASCIIReader.hh"
#include "CAENLinuxBusAdapter.hh"
//#include "VMEDummyBusAdapter.hh"

// Pixel includes
#include "PixelFEDInterface/include/PixelFEDInterface.h" // PixFED class definition
#include "PixelFEDInterface/include/PixelFEDFifoData.h" // PixFED data decode
#include "CalibFormats/SiPixelObjects/interface/PixelFEDCard.h"    // The FED settings structure

#include "PixelTTCviInterface.h" // TTCvi class definition, for tests only
#include <unistd.h> // for usleep()
// Function prototypes
void defineTestDACs(int *const dac1, int *const dac2, int *const dac3);
void modifyTestDACs(int *const dac1, int *const dac2, int *const dac3);

//#define WRITE_FILE 

int main(int argc, void *argv[]) {
  try {
  int status=0;
  bool transparentMode = false; // use FED in transparent mode
  bool DACMode = false; // read emulated data from the DAC 
  bool readFifo1 = false; // read data from Fifo1
  bool readSpyFifo1 = false; // read data from SpyFifo1's (chnls 1-4 and 32-36)
  bool readFifo2 = false; // read data Fifo2 Spy memories
  bool readFifo3 = true; // read data from Fifo3 Spy memories
  bool BaselineCorr = true; // set the fed to perform baseline correction
  const int nMax = 100000; // max number of loops
  const bool testingOthers = true; // for more complete testing, switch off for speed tests
  const bool decodeData = true; // select to decode the fifo data
  const bool readError=true;
  const bool readTemperatures=false;
  const bool readTTS=false;
  const bool generateTrigger = true;
  const bool modifyDACs = true;//increases event number for out of sync tests
  const bool stopTestDAC = true;//shuts off the test DAC at the end of testing
  const bool preloadTestDAC = true;//set a pedestal for the test to remove leading 0's
  const bool setlastDAC0 = false; //sets the very last testDAC memory to 0, similar to
                                  //shutting off testDAC, but no reset is done, i.e.
				  //event number won't reset
  const bool usefiniteTestDAClength=false; //can increase trigger rate with shorter length				   
  const bool ReadHis = false; //Read out hisogram memories
  const bool DumpHis = false; //Dump histogram readout to screen
  const bool DumpBaselineValues = false; //Dump Baseline Values to the screen
   const bool enableTTSReady = true;//enable or disable to reduce TTS type error reporting rate
   const bool enableTTSError = true;
   const bool enableOutOfSyncError = false;
  const bool testTTSfast = false;//toggles the TTS lines to see if they are alive
  const bool testTTSslow = false;//lets you toggle TTS lines one at a time
  const bool testSlinks = false;//Sends a test word through the Slink
  const bool getFirm = true;//readout firmware dates of FPGA's

  // For the test DACs
  int dac1[256], dac2[256], dac3[256];

  // File to store data
#ifdef WRITE_FILE
  ofstream outfile("data.dat");
#endif

  // Init HAL
  // if you want to play with real hardware you need a real busAdapter:
  // change the comments below:
  //HAL::VMEDummyBusAdapter busAdapter;
  
  cout<<"expecting arguments for baseaddress (e.g. 0x16000000) and link (e.g. 1 or 2)"<<endl;
  uint32_t fedBase = 0x18000000; // FED base addresss
  uint32_t number;
  sscanf((char*)argv[1], "%lx", &fedBase);
  printf("BaseAddress 0x%lx\n",fedBase);
  int link;
  sscanf((char*)argv[2], "%d", &link);
  printf("link %d\n",link);

  

//HAL::CAENLinuxBusAdapter busAdapter( HAL::CAENLinuxBusAdapter::V1718 );
//  HAL::CAENLinuxBusAdapter busAdapter( HAL::CAENLinuxBusAdapter::V2718 );
HAL::CAENLinuxBusAdapter busAdapter(HAL::CAENLinuxBusAdapter::V2718,link,0, HAL::CAENLinuxBusAdapter::A3818 );


  cout<<"mapper"<<endl;
  HAL::VMEAddressTableASCIIReader addressTableReader("FEDAddressMap.dat");
  cout<<"reader"<<endl;
  HAL::VMEAddressTable addressTable("Test address table",addressTableReader);
  cout<<"table"<<endl;
  HAL::VMEDevice PixFEDCard(addressTable, busAdapter, fedBase);
  cout<<"card"<<endl;
  
  // Init PxFED
  PixelFEDInterface fed1(&PixFEDCard); // Instantiate the FED class
 
  cout<<"card instant"<<endl;

  // Now the TTCVi 
  // TTCvi (this is a module external to the FED, used to control the TTC)
  // Setup the parameters for the TTCvi
  uint32_t TTCVIBASE = 0xFFFF00;
  HAL::VMEAddressTableASCIIReader addressTableReader2("TTCviAddressMap.dat");
  HAL::VMEAddressTable addressTable2("TTCvi address table",
				     addressTableReader2);
  HAL::VMEDevice TTCviCard(addressTable2, busAdapter, TTCVIBASE);
  PixelTTCviInterface ttcvi(&TTCviCard); // Instantiate the TTCvi class

  const bool TTCvxyes = false; // VME trigger
  //const bool TTCvxyes = true; // TTC trigger
  if(TTCvxyes) {  // setup the TTCVI
    
    //status = TTCvi_setup_trigger(BHandle,TTCVIBASE);
    int mode=0;
    //status = ttcvi.setup_trigger(mode);
    
  } //TTCVI

  // Try the FPGA reload 
  // resets histogram fifo counters
  //fed1.loadFPGA(); return 0;  // clears the error and lastdac fifos (tts fifo still has 1 entry)
  // Reset FED
  cout<<"************reset"<<endl;
  status=fed1.reset();
  if(status!=0) return(0); // exit error
 
  // Read in parameters from a file and download them to the FED
  string fileName("params_fed.dat"); // define the file name
cout<<"Starting conditions for Front control Registers:"<<endl;  
    uint32_t idata;
  PixFEDCard.read("NWrRdCntrReg",&idata);
  cout<<hex<<"North 0x"<<idata<<endl;
  PixFEDCard.read("NCWrRdCntrReg",&idata);
  cout<<hex<<"North Center 0x"<<idata<<endl;
  PixFEDCard.read("SCWrRdCntrReg",&idata);
  cout<<hex<<"South Center 0x"<<idata<<endl;
  PixFEDCard.read("SWrRdCntrReg",&idata);
  cout<<hex<<"South 0x"<<idata<<dec<<endl;

  pos::PixelFEDCard pixelFEDCard(fileName); // instantiate the FED settings (should be private)

//  status = fed1.setupFromDB(fileName);
 PixFEDCard.read("SWrRdCntrReg",&idata);
 cout<<hex<<"South 0x"<<idata<<dec<<endl;
  status = fed1.setupFromDB(pixelFEDCard);
  if(status==-1) {
    cout<<" No configuration file "<<endl;
    return(0);
  }
  //to Make sure dump of DB values works ok
  string tmpfil("./"); 
  pixelFEDCard.writeASCII(tmpfil); 
  
  // Get control register value after it was set from DB
  int cntrl = fed1.getControlRegister();  // read it
  int cntrl2 = fed1.getCcntrl();  // last written
  cout<<" Control register "<<hex<<cntrl<<" "<<cntrl2<<dec<<endl; // 

cout<<"After params_fed conditions for Front control Registers:"<<endl;  
  PixFEDCard.read("NWrRdCntrReg",&idata);
  cout<<hex<<"North 0x"<<idata<<endl;
  PixFEDCard.read("NCWrRdCntrReg",&idata);
  cout<<hex<<"North Center 0x"<<idata<<endl;
  PixFEDCard.read("SCWrRdCntrReg",&idata);
  cout<<hex<<"South Center 0x"<<idata<<endl;
  PixFEDCard.read("SWrRdCntrReg",&idata);
  cout<<hex<<"South 0x"<<idata<<dec<<endl;
cout<<hex<<pixelFEDCard.Ncntrl<<dec<<endl;
cout<<hex<<pixelFEDCard.NCcntrl<<dec<<endl;
cout<<hex<<pixelFEDCard.SCcntrl<<dec<<endl;
cout<<hex<<pixelFEDCard.Scntrl<<dec<<endl;




   if(getFirm){
   for(int i=0;i<5;i++)
   {uint32_t fdate=fed1.get_FirmwareDate(i);
   cout<<"0x"<<hex<<fdate<<dec<<endl;}
   uint32_t fdate=fed1.get_VMEFirmwareDate();
   cout<<"0x"<<hex<<fdate<<dec<<endl;
   
   }


  // If the TTC L1A are to be used change the setting
  if(TTCvxyes) {  // **************************  TTCVI
    //int value = cntrl;

    // enable L1A from TTC 
    int value = 0x10; // transparent gate from L1A, TTC event#, 
    if(transparentMode) value = value | 0x9; // set transparent bit & Vme event#
    if(DACMode&(!pixelFEDCard.SpecialDac)) value = value | 0x4; // enable DAC data
    if(readFifo1) value = value | 0x8; //  Vme event#
    if(enableTTSReady) value  = value | 0x10000;  // Enable the TTSReady messgae to the  TTS fifo
    if(enableTTSError) value = value | 0x20000;  // Enable the TTSError message to the TTS fifo
    if(enableOutOfSyncError) value = value | 0x40000;  // Enable the out-of-sync error to TTS

    status = fed1.setControlRegister(value);


  } else {  // VME triggers
    
    int value = 0x0a; // transparent gate from VME & VME event#
    if(transparentMode) value = value | 0x1; // set transparent bit
    if(DACMode&(!pixelFEDCard.SpecialDac)) value = value | 0x4; // enable DAC data
    if(readFifo1) value = value | 0x8; //  Vme event#
    if(enableTTSReady) value  = value | 0x10000;  // Enable the TTSReady messgae to the  TTS fifo
    if(enableTTSError) value = value | 0x20000;  // Enable the TTSError message to the TTS fifo
    if(enableOutOfSyncError) value = value | 0x40000;  // Enable the out-of-sync error to TTS

    status = fed1.setControlRegister(value);
    //PixFEDCard.write("NWrModeReg",0x0);

  }

  // Get control register
  cntrl = fed1.getControlRegister();  // read it
  cntrl2 = fed1.getCcntrl();  // last written
  cout<<" Control register "<<hex<<cntrl<<" "<<cntrl2<<dec<<endl; // 
      
//set preload pedestal before baseline correction
  if(preloadTestDAC)fed1.setup_testDAC(400);	

  // Fill up the dac memory with simulated data
  //fed1.fillDACRegister();
  defineTestDACs(dac1, dac2, dac3);
      if(setlastDAC0){dac1[255]=0;dac2[255]=0;dac3[255]=0;}
  
     if(usefiniteTestDAClength) 
     {fed1.fillDACRegisterLength(dac1, dac2, dac3,256);} else
     {fed1.fillDACRegister(dac1, dac2, dac3);}

  // MODE register
  // Reset the SLink (not needed if no slink)
  fed1.resetSlink();
  
  // Blocks data from reaching Fifo3 (comment out) 
  fed1.setModeRegister(0x8);  // if this set there is no data in fifo3

  // Control write to FIFO3 spy memory
  // Set the mode  register bit 1, ->  enable writinig to spy memory
  status=fed1.enableSpyMemory(1); 
  // set the mode to disable spy memory
  //status=fed1.enableSpyMemory(0); // blocks writing to spy-memory

  // Should equal to the largest possible fifo. 
  uint32_t buffer[(36*1024)]; // Data buffer for the FIFO (fifo1 = 36*1024?)

  // Check the Mode register 
  int mode = fed1.getModeRegister(); //check the mode
  cout<<" Mode register = "<<hex<<mode<<dec<<endl;
   if(testSlinks){
   cout<<"Sending a fake data event on the Slink"<<endl;
   int status=fed1.testSlink();
   }
   
  
  // Test the event register
  int eventNumber=fed1.readEventCounter();
  cout<<"VME event Number "<<hex<<eventNumber<<dec<<endl;

  // Get the event number directly from the TTCrx register. TESTING only.
  int ttcrx_ev1 = fed1.TTCRX_I2C_REG_READ( 26); //  
  int ttcrx_ev2 = fed1.TTCRX_I2C_REG_READ( 27); //  
  int ttcrx_ev3 = fed1.TTCRX_I2C_REG_READ( 28); //  
  cout<<" TTCrx event "<<hex<<ttcrx_ev1<<" "<<ttcrx_ev2<<" "<<ttcrx_ev3<<dec<<endl;


    //fed1.set_chnls_onoff(0); // Channels always on, transparent mode automatic

 fed1.set_chnls_onoff(); //Uses the database values
/*
cout<<"after onoff conditions for Front control Registers:"<<endl;  
  PixFEDCard.read("NWrRdCntrReg",&idata);
  cout<<hex<<"North 0x"<<idata<<endl;
  PixFEDCard.read("NCWrRdCntrReg",&idata);
  cout<<hex<<"North Center 0x"<<idata<<endl;
  PixFEDCard.read("SCWrRdCntrReg",&idata);
  cout<<hex<<"South Center 0x"<<idata<<endl;
  PixFEDCard.read("SWrRdCntrReg",&idata);
  cout<<hex<<"South 0x"<<idata<<dec<<endl;
cout<<"check copy"<<endl;
cout<<hex<<pixelFEDCard.Ncntrl<<dec<<endl;
cout<<hex<<pixelFEDCard.NCcntrl<<dec<<endl;
cout<<hex<<pixelFEDCard.SCcntrl<<dec<<endl;
cout<<hex<<pixelFEDCard.Scntrl<<dec<<endl;
fed1.set_chnls_onoff(1);
cout<<"after onoff(1) conditions for Front control Registers:"<<endl;  
  PixFEDCard.read("NWrRdCntrReg",&idata);
  cout<<hex<<"North 0x"<<idata<<endl;
  PixFEDCard.read("NCWrRdCntrReg",&idata);
  cout<<hex<<"North Center 0x"<<idata<<endl;
  PixFEDCard.read("SCWrRdCntrReg",&idata);
  cout<<hex<<"South Center 0x"<<idata<<endl;
  PixFEDCard.read("SWrRdCntrReg",&idata);
  cout<<hex<<"South 0x"<<idata<<dec<<endl;
fed1.set_chnls_onoff(0);
cout<<"after onoff(0) conditions for Front control Registers:"<<endl;  
  PixFEDCard.read("NWrRdCntrReg",&idata);
  cout<<hex<<"North 0x"<<idata<<endl;
  PixFEDCard.read("NCWrRdCntrReg",&idata);
  cout<<hex<<"North Center 0x"<<idata<<endl;
  PixFEDCard.read("SCWrRdCntrReg",&idata);
  cout<<hex<<"South Center 0x"<<idata<<endl;
  PixFEDCard.read("SWrRdCntrReg",&idata);
  cout<<hex<<"South 0x"<<idata<<dec<<endl;

fed1.set_chnls_onoff(1);
cout<<"after onoff(1) conditions for Front control Registers:"<<endl;  
  PixFEDCard.read("NWrRdCntrReg",&idata);
  cout<<hex<<"North 0x"<<idata<<endl;
  PixFEDCard.read("NCWrRdCntrReg",&idata);
  cout<<hex<<"North Center 0x"<<idata<<endl;
  PixFEDCard.read("SCWrRdCntrReg",&idata);
  cout<<hex<<"South Center 0x"<<idata<<endl;
  PixFEDCard.read("SWrRdCntrReg",&idata);
  cout<<hex<<"South 0x"<<idata<<dec<<endl;  // Buffer to hold readout data, used for fifo1,2,3,error,tts and last-dac.

cout<<"start sweeping through setChannel...false...true...false for all channels"<<endl;
for (unsigned int ig=1;ig<10;ig++){
pixelFEDCard.setChannel(ig,false);
pixelFEDCard.setChannel(ig+9,false);
pixelFEDCard.setChannel(ig+18,false);
pixelFEDCard.setChannel(ig+27,false);

cout<<hex<<"N  "<<pixelFEDCard.Ncntrl<<dec<<endl;
cout<<hex<<"NC "<<pixelFEDCard.NCcntrl<<dec<<endl;
cout<<hex<<"SC "<<pixelFEDCard.SCcntrl<<dec<<endl;
cout<<hex<<"S  "<<pixelFEDCard.Scntrl<<dec<<endl;


}

for (unsigned int ig=1;ig<10;ig++){
pixelFEDCard.setChannel(ig,true);
pixelFEDCard.setChannel(ig+9,true);
pixelFEDCard.setChannel(ig+18,true);
pixelFEDCard.setChannel(ig+27,true);

cout<<hex<<"N  "<<pixelFEDCard.Ncntrl<<dec<<endl;
cout<<hex<<"NC "<<pixelFEDCard.NCcntrl<<dec<<endl;
cout<<hex<<"SC "<<pixelFEDCard.SCcntrl<<dec<<endl;
cout<<hex<<"S  "<<pixelFEDCard.Scntrl<<dec<<endl;


}

for (unsigned int ig=1;ig<10;ig++){
pixelFEDCard.setChannel(ig,false);
pixelFEDCard.setChannel(ig+9,false);
pixelFEDCard.setChannel(ig+18,false);
pixelFEDCard.setChannel(ig+27,false);

cout<<hex<<"N  "<<pixelFEDCard.Ncntrl<<dec<<endl;
cout<<hex<<"NC "<<pixelFEDCard.NCcntrl<<dec<<endl;
cout<<hex<<"SC "<<pixelFEDCard.SCcntrl<<dec<<endl;
cout<<hex<<"S  "<<pixelFEDCard.Scntrl<<dec<<endl;


}
*/
   if(testTTSfast) {
     cout<<"beginning test,all set 0 to start (disconnected)"<<endl;
     cout<<"toggle in turn bit 0(Warn), 1(OOS), 2(Busy), 3(Ready)"<<endl;
     cout<<"repeating 20000 times"<<endl;
for(int k=0;k<20000;k++){
    uint32_t ttsbits=0;  
     fed1.testTTSbits(ttsbits,1); 
    ttsbits=0x1;  
     fed1.testTTSbits(ttsbits,1); 
    ttsbits=0x2;  
     fed1.testTTSbits(ttsbits,1); 
    ttsbits=0x4;  
     fed1.testTTSbits(ttsbits,1); 
    ttsbits=0x8;  
     fed1.testTTSbits(ttsbits,0); 
     }
    } // if read TTS fifo

   if(testTTSslow) {
   int dummy=0;
   cout<<"Slow test of TTS system"<<endl;
    while(dummy>-1){
   cout<<"enter which line to toggle"<<endl;
   cout<<"0=(unplugged) 1=(Warn), 2=(OOS), 4=(Busy), 8=(Ready) (-1 to end):";
   cin>>dummy;
   if(dummy>-1){uint32_t mdat=(uint32_t) dummy;fed1.testTTSbits(mdat,1);}
    }
    fed1.testTTSbits(0,0);
   }
  
  // Read error, last-dac and tts fifos before the 1st event to see if they are empty
  // ERROR FIFOs
  //if(readError || readTemperatures || readTTS) { 
  if(readError || readTTS) { 
    //int wdcnt=fed1.drainErrorFifo(1,buffer); // Read one FIFO only (group=1)
    int wdcnt=fed1.drainErrorFifo(buffer); // Read Error FIFOs for all channels
    cout<<" Error fifo, count = "<<wdcnt<<endl; 
    if(wdcnt>0) PixelFEDFifoData::decodeErrorFifo(buffer);  // decode it


    // TEMPERATURE FIFOs
//     wdcnt=fed1.drainTemperatureFifo(buffer); // Read All Temperature FIFOs
//     cout<<" Temperature fifos, count = "<<wdcnt<<endl; 
//     if(wdcnt>0) PixelFEDFifoData::decodeTemperatureFifo(buffer);

    // Read TTS FIFO 
    wdcnt=fed1.drainTTSFifo(buffer); // Read TTS FIFOs
    cout<<" TTS fifo, count = "<<wdcnt<<endl; 
    if(wdcnt>0) {
	PixelFEDFifoData::decodeTTSFifo(buffer,wdcnt); // stale word?
    } //if 
  } // if read fifos


//   int dummy; 
//   cout<<" continue ";
//   cin>>dummy;

  // Select the clock phase
//   int delay=0;  // delay value
//   int channel=36; // for channel
//   cout<<" enter delay = ";
//   cin>>delay;
//   status = fed1.setPhases(channel,delay); // select phase for channel 1
//   cout<<" delay selected "<<delay<<" "<<channel<<" "<<status<<endl;

//   int param = 0;


	if(ReadHis){fed1.clear_hismem(); int status=fed1.enableHisMemory(1);}

	 
        if(BaselineCorr){fed1.BaselineCorr_on();
	} else {fed1.BaselineCorr_off();}
  cout<<" Start DAQ"<<endl;

  //float   time1 ,time2;  
  //time1 = (float) clock() / (float) CLOCKS_PER_SEC;
  clock_t   time1 ,time2;  
  time1=time(0);

  // Loop for n events.
  for(int n=0;n<nMax;n++) {   
if(DumpBaselineValues){cout<<" Baseline Correction Values"<<endl;fed1.dump_BaselineCorr();}

//gets fifo status and dumps it
if(testingOthers) {uint32_t fstat=fed1.getFifoStatus();fed1.dump_FifoStatus(fstat);}
//    if(n==10)status = ttcvi.sendBRCST(0x14);    // Funny tests
    // Delay loop
//     if (n>0 && (n%10) == 0 ) {
//       delay++;
//       if(delay>31) delay=0;
//       cout<<" set delay to "<<delay<<endl;
//       status = fed1.setPhases(channel,delay); // select phase for channel=1
//     }
//     param=delay;

// corse delay 
//    fed1.TTCRX_I2C_REG_WRITE( 2, param); //COARSE DELAY REG 
//    if(n>0) param++;
//    if(param==16) param=0;

// fine delay
//    param = n;
//    fed1.TTCRX_I2C_REG_WRITE( 1, param); //Fine DELAY REG 

    //  Modify the test-DAC (works OK)
    if(modifyDACs) {
      modifyTestDACs(dac1, dac2, dac3);
     if(usefiniteTestDAClength) 
     {fed1.fillDACRegisterLength(dac1, dac2, dac3,256);} else
     {fed1.fillDACRegister(dac1, dac2, dac3);}
    }

    // Generate a trigger
    if(TTCvxyes) {   // TTC trigger

      //if (n>0 && (n%4) == 0 ) ttcvi.sendECR(); // send an event reset
      //ttcvi.sendBCR(); // send an bx reset
      //int tmp = 0;
      //tmp = tmp | (0x1 << (param));
      //ttcvi.sendBRCST(tmp); // send 
      //param++;
      //if(param==8) param=0;
      
      if(generateTrigger) status = ttcvi.trigger();
      if(testingOthers) cout<<endl<<" Generate TTC trigger, event = "<<status<<endl;

    } else {   // VME trigger
//  int rdata = 0x80000000; // data for reset bit
 
// PixFEDCard.write("LRES",rdata);status=fed1.reset();//usleep(4000);
      
      if(generateTrigger) {status = fed1.generateVMETrigger();if(testingOthers) cout<<" Generate a VME trigger "<<endl;}
      if(generateTrigger&!(transparentMode|readFifo1)) {status = fed1.setVMEevntnmbr(n+1);}
    }
/* 
{
int chnl=33;
uint32_t data=((chnl-28)<<8);//data=0x500;
cout<<"data for write set to 0x"<<hex<<data<<endl;
PixFEDCard.write("LAD_S",data,HAL::HAL_NO_VERIFY,0x1c0000);

//read the UBtrans
cout<<sizeof(short);
cout<<"***********************************************************************************"<<endl;
 for(int ij=0;ij<10;ij++){
 PixFEDCard.read("LAD_S",&data,0x20000);
cout<<dec<<ij<<"ch 31 data "<<data<<" 1st UB "<<(data&0x3ff)<<" 2nd UB "<<((data&0xffc00)>>10)<<" 3rd UB "<<((data&0x3ff00000)>>20)<<endl;  
}
cout<<"***********************************************************************************"<<endl;
PixFEDCard.read("LAD_S",&data,0x28000);
if(((data&0x3ff00000)>>20)>0)cout<<dec<<"ch 31 data 0x"<<hex<<data<<dec<<" pre Black "<<(data&0x3ff)<<" post black "<<((data&0xffc00)>>10)<<" black diff "<<((data&0x3ff00000)>>20)<<" comp bit "<<((data&0x80000000)>>31)<<endl;
PixFEDCard.read("LAD_S",&data,0x158000);
cout<<dec<<"ch 31 data 0x"<<hex<<(data)<<dec<<" Const 1: "<<((short)(data&0xffff))<<" const 2: "<<((short)((data&0xffff0000)>>16))<<endl;
PixFEDCard.read("LAD_S",&data,0x178000);
cout<<dec<<"ch 31 data 0x"<<hex<<((short)(data&0xffff))<<dec<<" const 3: "<<(data&0xffff)<<endl;
} 
{
int chnl=31;
uint32_t data=((chnl-28)<<8);//data=0x500;
cout<<"data for write set to 0x"<<hex<<data<<endl;
PixFEDCard.write("LAD_S",data,HAL::HAL_NO_VERIFY,0x1c0000);

//read the UBtrans
cout<<sizeof(short);
cout<<"***********************************************************************************"<<endl;
 for(int ij=0;ij<10;ij++){
 PixFEDCard.read("LAD_S",&data,0x20000);
cout<<dec<<ij<<"ch 33 data "<<data<<" 1st UB "<<(data&0x3ff)<<" 2nd UB "<<((data&0xffc00)>>10)<<" 3rd UB "<<((data&0x3ff00000)>>20)<<endl;  
}
cout<<"***********************************************************************************"<<endl;
PixFEDCard.read("LAD_S",&data,0x28000);
if(((data&0x3ff00000)>>20)>0)cout<<dec<<"ch 33 data 0x"<<hex<<data<<dec<<" pre Black "<<(data&0x3ff)<<" post black "<<((data&0xffc00)>>10)<<" black diff "<<((data&0x3ff00000)>>20)<<" comp bit "<<((data&0x80000000)>>31)<<endl;
PixFEDCard.read("LAD_S",&data,0x158000);
cout<<dec<<"ch 33 data 0x"<<hex<<(data)<<dec<<" Const 1: "<<((short)(data&0xffff))<<" const 2: "<<((short)((data&0xffff0000)>>16))<<endl;
PixFEDCard.read("LAD_S",&data,0x178000);
cout<<dec<<"ch 33 data 0x"<<hex<<((short)(data&0xffff))<<dec<<" const 3: "<<(data&0xffff)<<endl;
} 
{
int chnl=30;
uint32_t data=((chnl-28)<<8);//data=0x500;
cout<<"data for write set to 0x"<<hex<<data<<endl;
PixFEDCard.write("LAD_S",data,HAL::HAL_NO_VERIFY,0x1c0000);

//read the UBtrans
cout<<sizeof(short);
cout<<"***********************************************************************************"<<endl;
 for(int ij=0;ij<10;ij++){
 PixFEDCard.read("LAD_S",&data,0x20000);
cout<<dec<<ij<<"ch 34 data "<<data<<" 1st UB "<<(data&0x3ff)<<" 2nd UB "<<((data&0xffc00)>>10)<<" 3rd UB "<<((data&0x3ff00000)>>20)<<endl;  
}
cout<<"***********************************************************************************"<<endl;
PixFEDCard.read("LAD_S",&data,0x28000);
if(((data&0x3ff00000)>>20)>0)cout<<dec<<"ch 34 data 0x"<<hex<<data<<dec<<" pre Black "<<(data&0x3ff)<<" post black "<<((data&0xffc00)>>10)<<" black diff "<<((data&0x3ff00000)>>20)<<" comp bit "<<((data&0x80000000)>>31)<<endl;
PixFEDCard.read("LAD_S",&data,0x158000);
cout<<dec<<"ch 34 data 0x"<<hex<<(data)<<dec<<" Const 1: "<<((short)(data&0xffff))<<" const 2: "<<((short)((data&0xffff0000)>>16))<<endl;
PixFEDCard.read("LAD_S",&data,0x178000);
cout<<dec<<"ch 34 data 0x"<<hex<<((short)(data&0xffff))<<dec<<" const 3: "<<(data&0xffff)<<endl;
} 
*/ 
    
    // Event number
    eventNumber=fed1.readEventCounter();  // This is now the new event counter

    // Some additinal readout tests
    if(testingOthers) {
      int bx = fed1.readBXCounter();  // Check bx

      // The old event counter and bx counter (these are just copies from TTCrx)
      uint32_t oldEventNum = 0; 
      PixFEDCard.read("RdEventCntr",&oldEventNum);
      
      // Get the event number directly from the TTCrx register. TESTING only.
      int ttcrx_ev1 = fed1.TTCRX_I2C_REG_READ( 26); //  
      int ttcrx_ev2 = fed1.TTCRX_I2C_REG_READ( 27); //  
      int ttcrx_ev3 = fed1.TTCRX_I2C_REG_READ( 28); //  
      
      // Get status register
      int cntrl0 = fed1.getControlRegister();
      int tmp = cntrl0 & 0x3f;
      int tmp1 = (cntrl0 & 0x3c0);
      int tmp2 = (cntrl0 & 0xffc00) >> 10;
      
      cntrl = fed1.getCcntrl();  // use last written

      cout<<" Loop "<<n<<" ";
      cout<<"Event number "<<eventNumber<<" "<<oldEventNum<<" "<<hex<<bx<<dec
	<<" ";
      cout<<" TTCrx event "<<ttcrx_ev1<<" "<<ttcrx_ev2<<" "<<ttcrx_ev3<<" ";
      cout<<" Control registers "<<hex<<cntrl<<" "<<cntrl0<<" "<<tmp<<" "
	  <<tmp1<<" "<<tmp2<<dec<<endl;
    } // end if testOthers

    // Read data from FIFO 1
    if(readFifo1) { 

      //Check to see if transparent mode is set
      if(transparentMode) {  // In transparent mode
	  
	for(int chan=31;chan<32;chan++) {  // loop over all channels 
	  //for(int chan=channel;chan<(channel+1);chan++) {  // loop over all channels 
	  // For transparent data it is difficult to find where the valid data ends
	  // so sttaus does not return the correct data length.
	  uint32_t bufferx[4096]; 
	  status=fed1.drain_transBuffer(chan,bufferx); // Read FIFO1 for  channel, length in bytes
	  if(testingOthers) cout<<" Fifo1 transparent readout, status = "<<status<<endl;
	  if(status>-1 && decodeData && n<200){ 
	    PixelFEDFifoData::decodeTransparentData(bufferx,100); // Decode data, length in words

}
    

#ifdef WRITE_FILE

	  int i0=0;
	  uint32_t data = 0, maxdata=0, data3=0;	  
	  for(int i =0;i<200;i++) {
	    data = (buffer[i] & 0xffc00000)>>22; // analyze word 
	    if(data3==0 && data>400) data3=data;  // first channel above thr
	    if(data>maxdata) { // max channel
	      i0=i;
	      maxdata=data;
	    }
	  }
	  uint32_t data0 = (buffer[21] & 0xffc00000)>>22; // analyze word 
	  uint32_t data1 = (buffer[22] & 0xffc00000)>>22; // analyze word 
	  uint32_t data2 = (buffer[23] & 0xffc00000)>>22; // analyze word 
	  cout<<" data "<<data0<<" "<<data1<<" "<<data2<<" "<<data3<<" "<<maxdata<<" "
	      <<param<<" "<<i0<<endl;
	  outfile<<param<<" "<<data0<<" "<<data1<<" "<<data2<<" "<<i0<<" "<<maxdata<<endl;
	  //outfile<<data0<<" "<<param<<" "<<data1<<" "<<i0<<" "<<data2<<endl;
	  //outfile<<eventNumber<<" "<<param<<" "<<cntrl0<<" "<<ttcrx_ev1<<" "<<bx<<endl;
#endif
	}  // end for loop over channels
  
      } else { // In normal mode 

	//status=fed1.drainFifo1(1,buffer,4096); // Read FIFO1, channel=1, length in bytes, def=1024
	status=fed1.drainFifo1(buffer); // Read FIFO1, all channels (defaut fifo size) 
	if (testingOthers) cout<<" FIFO 1, normal mode, status = "<<status<<endl;
	if(status>-1 && decodeData) 
	  PixelFEDFifoData::decodeNormalData(buffer,status); //  Decode data	
	
      }	// end if

    } // if read fifo1
    

if(readSpyFifo1 && !transparentMode){ //do only in normal mode mode
status=fed1.drainSpyFifo1up(buffer);
cout<<"looking at spy fifo 1 up words= "<<status<<endl;
      if(status>0 && decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer,status);
status=fed1.drainSpyFifo1dn(buffer);
cout<<"looking at spy fifo 1 dn words= "<<status<<endl;
      if(status>0 && decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer,status);
}




    // Now do FIFO2
    if( readFifo2 && !transparentMode ) {  //  do only in normal mode mode
      cout<<" Read Spy FIFO2 "<<endl;
      int wdcnt=0;

      wdcnt=fed1.drainDataFifo2(1,buffer); // Read FIFO2, chip N UP
      cout<<" Spy fifo2-1up, count = "<<wdcnt<<endl; 
      if(wdcnt>0 && decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer,wdcnt);

      wdcnt=fed1.drainDataFifo2(2,buffer); // Read FIFO2, chip N DOWN
      cout<<" Spy fifo2-1dw, count = "<<wdcnt<<endl; 
      if(wdcnt>0 && decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer,wdcnt);

      wdcnt=fed1.drainDataFifo2(3,buffer); // Read FIFO2, chip NC UP
      cout<<" Spy fifo2-2up, count = "<<wdcnt<<endl; 
      if(wdcnt>0 && decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer,wdcnt);

      wdcnt=fed1.drainDataFifo2(4,buffer); // Read FIFO2, chip NC DOWN
      cout<<" Spy fifo2-2dw, count = "<<wdcnt<<endl; 
      if(wdcnt>0 && decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer,wdcnt);

      wdcnt=fed1.drainDataFifo2(5,buffer); // Read FIFO2, chip SC UP
      cout<<" Spy fifo2-3up, count = "<<wdcnt<<endl; 
      if(wdcnt>0 && decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer,wdcnt);

      wdcnt=fed1.drainDataFifo2(6,buffer); // Read FIFO2, chip SC DOWN
      cout<<" Spy fifo2-3dw, count = "<<wdcnt<<endl; 
      if(wdcnt>0 && decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer,wdcnt);

      wdcnt=fed1.drainDataFifo2(7,buffer); // Read FIFO2, chip S UP
      cout<<" Spy fifo2-4up, count = "<<wdcnt<<endl; 
      if(wdcnt>0 && decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer,wdcnt);

      wdcnt=fed1.drainDataFifo2(8,buffer); // Read FIFO2, chip S DOWN
      cout<<" Spy fifo2-4dw, count = "<<wdcnt<<endl; 
      if(wdcnt>0 && decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer,wdcnt);

    } // end if
     
    // Data Spy-Fifo3
    if( readFifo3 && !transparentMode ) { 
      //status=fed1.enableSpyMemory(0); // blocks writing to spy-memory, during reading time
      uint64_t sbuffer[1024];
      int wdcnt=fed1.spySlink64(sbuffer); // Read FIFO3
//*int wdcnt=fed1.drainDataFifo3(buffer);

      if(testingOthers) cout<<" Spy fifo3, 64bit word count = "<<wdcnt<<endl; 
#ifdef WRITE_FILE
      outfile<<eventNumber<<" "<<wdcnt<<dec<<endl;
#endif

      if(wdcnt>0) {
	if(decodeData&&(n==0)) PixelFEDFifoData::decodeSlink64(sbuffer,wdcnt);
if(wdcnt!=290)cout<<dec<<wdcnt<<endl;
 uint32_t oldhi=0;
 uint32_t oldlo=0;
  for(int i=0;i<wdcnt;i++) {
  uint64_t hidat = (sbuffer[i]&0xffffffff00000000LL)>>32;
  uint64_t lodat = (sbuffer[i]&0x00000000ffffffffLL);
//we may need to change this part if the lo/hi word is compiler dependent in future
  uint32_t data=(uint32_t) hidat;
//if(i==1)cout<<"1st word ="<<hex<<data<<endl;
  uint32_t data2=(uint32_t) lodat;
  if((data==oldhi)&&(data2==oldlo)){
   PixelFEDFifoData::decodeSpyDataFifoSlink64(data);
   PixelFEDFifoData::decodeSpyDataFifoSlink64(data2);

}
oldhi=data;oldlo=data2;
}

//*      for(int i=0;i<wdcnt;i++) {	
//*	cout<<i<<" "<<hex<<buffer[i]<<dec<<" ";
//*	if(decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer[i]);
// #ifdef WRITE_FILE
// 	  outfile<<hex<<buffer[i]<<dec<<endl;
// #endif
//* 	} // for
      } //if 

      //status=fed1.enableSpyMemory(1);  // enable again fifo writting

    } // end if readFifo3


    // ERROR FIFOs
    if(readError) { 
      //int wdcnt=fed1.drainErrorFifo(1,buffer); // Read one FIFO only (group=1)
      int wdcnt=fed1.drainErrorFifo(buffer); // Read Error FIFOs for all channels
      cout<<" Error fifo, count = "<<wdcnt<<endl; 
      if(wdcnt>0) PixelFEDFifoData::decodeErrorFifo(buffer,wdcnt);  // decode it
    }

    // TEMPERATURE FIFOs
    if(readTemperatures) {
      //wdcnt=fed1.drainTemperatureFifo(1,buffer); // Read FIFO
      int wdcnt=fed1.drainTemperatureFifo(buffer); // Read All Temperature FIFOs
      cout<<" Temperature fifos, count = "<<wdcnt<<endl; 
      if(wdcnt>0) PixelFEDFifoData::decodeTemperatureFifo(buffer,wdcnt);
    }

    // Read TTS FIFO 
    if(readTTS) { 
      int wdcnt=fed1.drainTTSFifo(buffer); // Read TTS FIFOs
      cout<<" TTS fifo, count = "<<wdcnt<<endl; 
      if(wdcnt>0) {
	  PixelFEDFifoData::decodeTTSFifo(buffer,wdcnt);  // stale word?
      } //if 
    } // if read TTS fifo

//if((n%100000==0)&&(n>0)){
//int dummy=0;
//cout<<"0 to get out!, 1 to continue"<<endl;
//cin>>dummy;
//if(dummy==0) return 0;
//}
//     int dummy=0;
//     cout<<" enter "<<endl;
//     cin>>dummy;
//     if(dummy==-1) break;


  } // end for events
	if(ReadHis){int status=fed1.enableHisMemory(0);//disable
	int size=26*32*12;//should always work
	uint32_t HisBuf[size];
	status=fed1.drainHisMemory(HisBuf);
if(DumpHis){
	
	//cout<<" histostatus= "<<status<<endl;
       int myhisp=0;int maxroc=0;int mytrip=0;
	while(status>0&&myhisp<status){
	//cout<<"mytrip "<<HisBuf[myhisp]<<" mymaxroc "<<HisBuf[myhisp+1]<<endl;
        //for(int ij=(myhisp+2);ij<myhisp+2+32*(HisBuf[myhisp+1]);ij++)
        //if(HisBuf[ij]>0)cout<<hex<<"0x"<<HisBuf[ij]<<dec<<" "<<ij<<endl;
	mytrip=HisBuf[myhisp];
	maxroc=HisBuf[myhisp+1];
	
for(int i=0;i<maxroc;i++){//loop over ROCs
for(int j=0;j<32;j++){//loop over DC's
if(HisBuf[myhisp+2+i*32+j]>0){
cout<<" ROC "<<i<<" DC "<<j<<endl;
	if((HisBuf[myhisp+2+i*32+j]&0xff)!=0)cout<<"channel "<<dec<<(mytrip-1)*3+1<<" Roc "<<dec<<i+1<<
	" DC "<<dec<<j<<" #hits = "<<dec<<(HisBuf[myhisp+2+i*32+j]&0xff)<<endl;//low 8bit
	if((HisBuf[myhisp+2+i*32+j]&0xff00)!=0)cout<<"channel "<<dec<<(mytrip-1)*3+2<<" Roc "<<dec<<i+1<<
	" DC "<<dec<<j<<" #hits = "<<dec<<((HisBuf[myhisp+2+i*32+j]&0xff00)>>8)<<endl;//med 8bit
	if((HisBuf[myhisp+2+i*32+j]&0xff0000)!=0)cout<<"channel "<<dec<<(mytrip-1)*3+3<<" Roc "<<dec<<i+1<<
	" DC "<<dec<<j<<" #hits = "<<dec<<((HisBuf[myhisp+2+i*32+j]&0xff0000)>>16)<<endl;//high 8bit	

}	

}}
myhisp+=HisBuf[myhisp+1]*32+2;cout<<myhisp<<endl;
}	
            }//dumphis

	
	
	}

  //time2 = (float) clock() / (float) CLOCKS_PER_SEC;
  //cout<<" time for "<<nMax<<" loops, is = "<<(time2-time1)<<endl;
  time2=time(0);
  double sec = difftime(time2,time1);
  cout<<" time for "<<nMax<<" loops, is = "<<sec<<endl;
  if(stopTestDAC)fed1.stop_testDAC();
  
  } catch ( HAL::HardwareAccessException& e ) {
    cout << "*** Exception occurred : " << endl;
    cout << e.what() << endl;
  } catch ( exception e ) {
    cout << "*** Unknown exception occurred" << endl;
  }

  return 0;
}
////////////////////////////////////////////////////////////////////
// Functions (load each 24 channel with 8 rocs + 1 pixel, and 12 channels with 16rocs+ 1 pixel)
void modifyTestDACs(int *const dac1,int *const dac2, int *const dac3) {

  const uint32_t V_OFFSET=100; // 100; 
  const uint32_t UB =50;
  const uint32_t B = 300;

  const int levels[6] = {200,300,410,500,650,750}; //OK for groups 1,2,3,4,5, not for 6,7,8
  // to be exact for channels: 24,25 - 28 - 33,36
  //const int levels[6] = {200,300,420,510,650,750}; //OK for groups 1,6,7,8, not for 2 


  static int n1=1, n2=0, n3=0, n4=0;  // 1st event is 0
  int n11=0, n12=0, n13=0, n14=0; // for the status
  static int count = 0;

  int START=0;  // define the start of the data packet (starts in fifo=10)

  // Limits are     :    270, 400, 500, 600, 700
  // The levels are : 200, 300, 450, 550, 650, 750  
  dac1[START+0] = V_OFFSET+B;   
  dac1[START+1] = V_OFFSET+B;   
  dac1[START+2] = V_OFFSET+B;   
  dac1[START+3] = V_OFFSET+B;   
  dac1[START+4] = V_OFFSET+B;  
  dac1[START+5] = V_OFFSET+B;
  dac1[START+6] = V_OFFSET+B;
  dac1[START+7] = V_OFFSET+B;
  dac1[START+8] = V_OFFSET+B;
  dac1[START+9] = V_OFFSET+B;

  START += 10;                    // to get State Machine ready

  // Change the header label
  dac1[START+0] = V_OFFSET+UB;   //UB  TBM-header
  dac1[START+1] = V_OFFSET+UB;   //UB
  dac1[START+2] = V_OFFSET+UB;   //UB
  dac1[START+3] = V_OFFSET+B;    //B

  // Change the event numebr
  dac1[START+4] = V_OFFSET+levels[n4];  // event number
  dac1[START+5] = V_OFFSET+levels[n3];
  dac1[START+6] = V_OFFSET+levels[n2];
  dac1[START+7] = V_OFFSET+levels[n1];

  //cout<<" DACs "<<dac1[START+4]<<" "<<dac1[START+5]<<" "<<dac1[START+6]<<" "<<dac1[START+7]<<endl;

  n1++;
  if(n1==4) {  // TBM counters use a 4 level coding
    n1=0;
    n2++;
    if(n2==4) {
      n2=0;
      n3++;
      if(n3==4) {
	n3=0;
	n4++;
	if(n4==4) {
	  n4=0;
	}
      }
    }
  }

  dac2[START+4] = dac1[START+4];  
  dac2[START+5] = dac1[START+5];  
  dac2[START+6] = dac1[START+6];  
  dac2[START+7] = dac1[START+7];  
  dac3[START+4] = dac1[START+4];  
  dac3[START+5] = dac1[START+5];  
  dac3[START+6] = dac1[START+6];  
  dac3[START+7] = dac1[START+7];  

  // Modify first ROC label
  //dac1[START+8] = V_OFFSET+UB; //UB
  //dac1[START+8+1] = V_OFFSET+900; //B

  // Change the last dac
//   dac1[8+2]= V_OFFSET+levels[n1];  //  LastDac
//   n1++;
//   if(n1==6) n1=0;
//   dac2[8+2] = dac1[8+2];  
//   dac3[8+2] = dac1[8+2];  

  // Modify TBM trailer
//   if(count==1) n11=1;
//   else if(count==2) n11=2;
//   else if(count==3) {
//     n11=0;
//     n12=1;
//   } else if(count==4) n12=2;
//   else if(count==5) {
//     n12=0;
//     n13=1;
//   } else if(count==6) n13=2;
//   else if(count==7) {
//     n13=0;
//     n14=1;
//   } else if(count==8) n14=2;
//   else {
//     n11=0;
//     n12=0;
//     n13=0;
//     n14=0;
//     count=0;
//   }

//   START=200;
//   dac1[START+4]= V_OFFSET+levels[n14];
//   dac1[START+5]= V_OFFSET+levels[n13];
//   dac1[START+6]= V_OFFSET+levels[n12];
//   dac1[START+7]= V_OFFSET+levels[n11];
//   cout<<" DACs "<<dac1[START+4]<<" "<<dac1[START+5]<<" "<<dac1[START+6]<<" "<<dac1[START+7]<<endl;

//   count++;
//  dac2[START+7]= V_OFFSET+levels[1];


  

}
////////////////////////////////////////////////////////////////////////////////////////////////
// Functions (load each 24 channel with 8 rocs + 1 pixel, and 12 channels with 16rocs+ 1 pixel)
void defineTestDACs(int *const dac1,int *const dac2, int *const dac3) {

  const uint32_t V_OFFSET = 100; // 100; 
  const uint32_t UB = 50; 
  const uint32_t B = 300;

  cout<<"Filling the dac registers"<<endl;

  // DAC TEST DATA //////////////////////////////////// 
  for(int i=0;i<256;i++) dac1[i]=V_OFFSET+B; //+B

  int START = 0;  // define the start of the data packet (starts in fifo=10)

  // Limits are     :    270, 400, 500, 600, 700
  // The levels are : 200, 300, 450, 550, 650, 750  
  dac1[START+0] = V_OFFSET+B;   
  dac1[START+1] = V_OFFSET+B;   
  dac1[START+2] = V_OFFSET+B;   
  dac1[START+3] = V_OFFSET+B;   
  dac1[START+4] = V_OFFSET+B;  
  dac1[START+5] = V_OFFSET+B;
  dac1[START+6] = V_OFFSET+B;
  dac1[START+7] = V_OFFSET+B;
  dac1[START+8] = V_OFFSET+B;
  dac1[START+9] = V_OFFSET+B;

  START += 10;                    // to get State Machine ready

  dac1[START+0] = V_OFFSET+UB;   //UB  TBM-header
  dac1[START+1] = V_OFFSET+UB;   //UB
  dac1[START+2] = V_OFFSET+UB;   //UB
  dac1[START+3] = V_OFFSET+B;    //B
  dac1[START+4] = V_OFFSET+200;  // event number
  dac1[START+5] = V_OFFSET+200;
  dac1[START+6] = V_OFFSET+200;
  dac1[START+7] = V_OFFSET+200;

  START += 8;                    // ROC#0

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#0
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2] = V_OFFSET+0;  //  LastDac

  dac1[START+3] = V_OFFSET+200;  // dcol MSB PIXEL 1
  dac1[START+4] = V_OFFSET+540;  // dcol LSB     
  dac1[START+5] = V_OFFSET+540;  // pix 
  dac1[START+6] = V_OFFSET+540;  // pix 
  dac1[START+7] = V_OFFSET+300;  // pix   
  dac1[START+8] = V_OFFSET+700;  // PH

  START += 9;                    // ROC#1

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#1
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2] = V_OFFSET+300;  //  LastDac
  dac1[START+3] = V_OFFSET+450;  // dcol MSB PIXEL 1
  dac1[START+4] = V_OFFSET+540;  // dcol LSB     
  dac1[START+5] = V_OFFSET+540;  // pix 
  dac1[START+6] = V_OFFSET+540;  // pix 
  dac1[START+7] = V_OFFSET+300;  // pix   
  dac1[START+8] = V_OFFSET+700;  // PH


  START += 9;                    // ROC#2

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#2
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2] = V_OFFSET+450;  //  LastDac

  dac1[START+3] = V_OFFSET+450;  // dcol MSB PIXEL 1
  dac1[START+4] = V_OFFSET+540;  // dcol LSB     
  dac1[START+5] = V_OFFSET+540;  // pix 
  dac1[START+6] = V_OFFSET+540;  // pix 
  dac1[START+7] = V_OFFSET+300;  // pix   
  dac1[START+8] = V_OFFSET+700;  // PH

  START += 9;                    // ROC#3

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#3
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2]= V_OFFSET+550;  //  LastDac

  dac1[START+3] = V_OFFSET+450;  // dcol MSB PIXEL 1
  dac1[START+4] = V_OFFSET+540;  // dcol LSB     
  dac1[START+5] = V_OFFSET+540;  // pix 
  dac1[START+6] = V_OFFSET+540;  // pix 
  dac1[START+7] = V_OFFSET+300;  // pix   
  dac1[START+8] = V_OFFSET+700;  // PH

  START += 9;                    // ROC#4

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#4
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2]= V_OFFSET+650;  //  LastDac

  dac1[START+3] = V_OFFSET+450;  // dcol MSB PIXEL 1
  dac1[START+4] = V_OFFSET+540;  // dcol LSB     
  dac1[START+5] = V_OFFSET+540;  // pix 
  dac1[START+6] = V_OFFSET+540;  // pix 
  dac1[START+7] = V_OFFSET+300;  // pix   
  dac1[START+8] = V_OFFSET+700;  // PH

  START += 9;                    // ROC#5

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#5
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2]= V_OFFSET+750;  //  LastDac

  dac1[START+3] = V_OFFSET+450;  // dcol MSB PIXEL 1
  dac1[START+4] = V_OFFSET+540;  // dcol LSB     
  dac1[START+5] = V_OFFSET+540;  // pix 
  dac1[START+6] = V_OFFSET+540;  // pix 
  dac1[START+7] = V_OFFSET+300;  // pix   
  dac1[START+8] = V_OFFSET+700;  // PH

  START += 9;                    // ROC#6

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#6
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2]= V_OFFSET+200;  //  LastDac

  dac1[START+3] = V_OFFSET+450;  // dcol MSB PIXEL 1
  dac1[START+4] = V_OFFSET+540;  // dcol LSB     
  dac1[START+5] = V_OFFSET+540;  // pix 
  dac1[START+6] = V_OFFSET+540;  // pix 
  dac1[START+7] = V_OFFSET+300;  // pix   
  dac1[START+8] = V_OFFSET+702;  // PH

  START += 9;                    // ROC#7

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#7
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2] = V_OFFSET+300;  //  LastDac
  
  dac1[START+3] = V_OFFSET+450;  // dcol MSB PIXEL 1
  dac1[START+4] = V_OFFSET+540;  // dcol LSB     
  dac1[START+5] = V_OFFSET+540;  // pix 
  dac1[START+6] = V_OFFSET+540;  // pix 
  dac1[START+7] = V_OFFSET+300;  // pix   
  dac1[START+8] = V_OFFSET+700;  // PH

  START += 9;                    // ROC#8

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#8
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2] = V_OFFSET+300;  //  LastDac
  dac1[START+3] = V_OFFSET+450;  // dcol MSB PIXEL 1
  dac1[START+4] = V_OFFSET+540;  // dcol LSB     
  dac1[START+5] = V_OFFSET+540;  // pix 
  dac1[START+6] = V_OFFSET+540;  // pix 
  dac1[START+7] = V_OFFSET+300;  // pix   
  dac1[START+8] = V_OFFSET+700;  // PH
  

  START += 9;                    // ROC#9

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#9
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2] = V_OFFSET+300;  //  LastDac
  
  dac1[START+3] = V_OFFSET+450;  // dcol MSB PIXEL 1
  dac1[START+4] = V_OFFSET+540;  // dcol LSB     
  dac1[START+5] = V_OFFSET+540;  // pix 
  dac1[START+6] = V_OFFSET+540;  // pix 
  dac1[START+7] = V_OFFSET+300;  // pix   
  dac1[START+8] = V_OFFSET+702;  // PH

  START += 9;                    // ROC#10

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#10
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2] = V_OFFSET+300;  //  LastDac
  
  dac1[START+3] = V_OFFSET+450;  // dcol MSB PIXEL 1
  dac1[START+4] = V_OFFSET+540;  // dcol LSB     
  dac1[START+5] = V_OFFSET+540;  // pix 
  dac1[START+6] = V_OFFSET+540;  // pix 
  dac1[START+7] = V_OFFSET+300;  // pix   
  dac1[START+8] = V_OFFSET+700;  // PH

  START += 9;                    // ROC#11

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#11
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2] = V_OFFSET+300;  //  LastDac
  
  dac1[START+3] = V_OFFSET+450;  // dcol MSB PIXEL 1
  dac1[START+4] = V_OFFSET+540;  // dcol LSB     
  dac1[START+5] = V_OFFSET+540;  // pix 
  dac1[START+6] = V_OFFSET+540;  // pix 
  dac1[START+7] = V_OFFSET+300;  // pix   
  dac1[START+8] = V_OFFSET+700;  // PH

  START += 9;                    // ROC#12

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#12
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2] = V_OFFSET+300;  //  LastDac
  dac1[START+3] = V_OFFSET+450;  // dcol MSB PIXEL 1
  dac1[START+4] = V_OFFSET+540;  // dcol LSB     
  dac1[START+5] = V_OFFSET+540;  // pix 
  dac1[START+6] = V_OFFSET+540;  // pix 
  dac1[START+7] = V_OFFSET+300;  // pix   
  dac1[START+8] = V_OFFSET+700;  // PH
  
  START += 9;                    // ROC#13

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#13
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2] = V_OFFSET+300;  //  LastDac
  dac1[START+3] = V_OFFSET+450;  // dcol MSB PIXEL 1
  dac1[START+4] = V_OFFSET+540;  // dcol LSB     
  dac1[START+5] = V_OFFSET+540;  // pix 
  dac1[START+6] = V_OFFSET+540;  // pix 
  dac1[START+7] = V_OFFSET+300;  // pix   
  dac1[START+8] = V_OFFSET+704;  // PH
  
  START += 9;                    // ROC#14

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#14
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2] = V_OFFSET+300;  //  LastDac
  
  dac1[START+3] = V_OFFSET+450;  // dcol MSB PIXEL 1
  dac1[START+4] = V_OFFSET+540;  // dcol LSB     
  dac1[START+5] = V_OFFSET+540;  // pix 
  dac1[START+6] = V_OFFSET+540;  // pix 
  dac1[START+7] = V_OFFSET+300;  // pix   
  dac1[START+8] = V_OFFSET+700;  // PH

  START += 9;                    // ROC#15

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#15
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2] = V_OFFSET+300;  //  LastDac
  
  dac1[START+3] = V_OFFSET+450;  // dcol MSB PIXEL 1
  dac1[START+4] = V_OFFSET+540;  // dcol LSB     
  dac1[START+5] = V_OFFSET+540;  // pix 
  dac1[START+6] = V_OFFSET+540;  // pix 
  dac1[START+7] = V_OFFSET+300;  // pix   
  dac1[START+8] = V_OFFSET+700;  // PH

 
  START += 9;                    // TBM Trailer

  //cout<<" START = "<<START<<endl;

  dac1[START+0]= V_OFFSET+UB;   // UB  TBM trailer
  dac1[START+1]= V_OFFSET+UB;   // UB
  dac1[START+2]= V_OFFSET+B;    // B
  dac1[START+3]= V_OFFSET+B;    // B
  dac1[START+4]= V_OFFSET+200;  // Status
  dac1[START+5]= V_OFFSET+200;
  dac1[START+6]= V_OFFSET+200;
  dac1[START+7]= V_OFFSET+200;
  
  START+=8;
  cout<<START<<"stqart"<<endl;
  if(START>255) cout<<" Error: last DAC address too large "<<START<<endl;
  // DAC2 same 
  for(int i=0;i<256;i++) dac2[i]=dac1[i]; // copy same to dac2

  // DAC3, make it with 16 rocs
  for(int i=0;i<256;i++) dac3[i]=dac1[i]; // copy same to dac3


}



