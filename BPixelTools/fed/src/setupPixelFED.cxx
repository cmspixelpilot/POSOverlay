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
// Take off TTCVI

#include <iostream>
#include <fstream>
#include <time.h>
#include <unistd.h>  // for usleep
#include <stdio.h>
#include <string.h>

using namespace std;
#include "VMELock.h" 

// HAL includes
#include "VMEDevice.hh"
#include "VMEAddressTable.hh"
#include "VMEAddressTableASCIIReader.hh"
#include "CAENLinuxBusAdapter.hh"
//#include "VMEDummyBusAdapter.hh"

// Pixel includes
#include "PixelFEDInterface/include/PixelFEDInterface.h" // PixFED class definition
#include "PixelFEDInterface/include/PixelFEDFifoData.h" // PixFED data decode
#include  "CalibFormats/SiPixelObjects/interface/PixelFEDCard.h" //The FED settings structure

using namespace pos;

//#define DELAY_SCAN
//#define CHECK_TTCRX
#define VERIFY_EVENTNUM

// Function prototypes
void defineTestDACs(int *const dac1, int *const dac2, int *const dac3);
void modifyTestDACs(int *const dac1, int *const dac2, int *const dac3);

//#define WRITE_FILE 
//#define BINARY_WRITE

const bool PRINT = false;

const int DAC_SIZE = 512; // 256; // size of the DAC emulation arrays (trans-mode<256).
const bool FIFO3 = false;
//const bool FIFO3 = true;

int main(int argc, char *argv[]) {
  int VMEBoard=1; //CAEN interface   define with option -vmecaenpci or -vmecaenusb
  VMELock lock(1);
  lock.acquire();
  uint32_t fedBase = 0x10000000; // FED base addresss   V5
  string fileName("data/params_fed.dat"); // define the file name
  
  if(argc>1){
    fedBase=strtol(argv[1],NULL,16);
    cout << "setup fed base=" << std::hex << fedBase << endl;
  }else{
    cout << "no base address " << endl;
    exit(1);
  }
  if(argc>2){
    fileName=argv[2];
  }
  cout << "using fed configuration from " <<  fileName << endl;

  if(argc>3){
    if(strcmp(argv[3],"-vmecaenpci")==0){
      VMEBoard=1; // Optical CAEN interface
      cout << "using pci vme interface" << endl;
    }else if(strcmp(argv[3],"-vmecaenusb")==0){
      VMEBoard=2; // USB CAEN interface
      cout << "using usb vme interface" << endl;
    }else{
      cerr << "usage: pxfec [-port <port>] [-vmecaenpci | -vmecaenusb] [-init <filename>]" << endl;
      exit(1);
    }
  }
    

  try {
  int status=0;
  bool transparentMode = true; // use FED in transparent mode
  bool readFifo1 = true; // read data from Fifo1 or transfer to Fifo2/3?
  bool readFifo2 = false; // read data from Fifo1 or transfer to Fifo2/3?
  bool readFifo3 = false; // read data from Fifo1 or transfer to Fifo2/3?
  bool DACMode = false; // read emulated data from the DAC 
  const bool testingOthers = true; // for more complete testing, switch off for speed tests
  const bool decodeData = true; // select to decode the fifo data
  const bool readError=true;
  const bool readTemperatures=false;
  const bool readTTS=false;
  //const bool modifyDACs = false;
  const bool Lite_Mode = false; // in this more the setup is skipped 
  const bool Setup_Only = true;  // do the setup only, no DAQ
  const bool fifoStatus = true;

  bool BaselineCorr = false; // set the fed to perform baseline correction
  //bool BaselineCorr = true; // set the fed to perform baseline correction

  const int nMax = 100; // max number of loops
  const int NUM_PRINT = 1;

  if(FIFO3) {
    transparentMode = false; // use FED in transparent mode
    readFifo1 = false; // read data from Fifo1 or transfer to Fifo2/3?
    readFifo2 = false; // read data from Fifo1 or transfer to Fifo2/3?
    readFifo3 = true; // read data from Fifo1 or transfer to Fifo2/3?
    BaselineCorr = true; // set the fed to perform baseline correction
  }  // if FIFO3

  //const bool ReadHis = false;
  //const bool DumpHis = false;
  const bool DumpBaselineValues = false;
  
  const bool enableTTSReady = false;
  const bool enableTTSError = false;
  const bool enableOutOfSyncError = false;

  const bool generateTrigger = false;
  //const bool TTCvxyes = false; // VME trigger
  const bool TTCvxyes = true; // TTC trigger

  //  const int WRITE_LENGTH = 250;
  const int LIST_FULL = 0; // Ful printout of first n events
  const bool READ_CONTINOUS = false; // Do not wait for new event number (TESTONLY)
  // const int PRINT_MIN = 14; // printout starts from slot
  //const int PRINT_MAX = 100; // printour stop in slot
  //const int UB_LIMIT = 0; // cit tp recognize IB, usually=300

  // Select channel for Fifo1 transparent readout
  //const int channel=1; // One FED channel, channel 1
  //const int channel=25; // One FED channel = 25
  const int channel=0; // all FED chanels
  int chanMin=1;
  int chanMax=1;
  if(channel==0) {
    chanMin=25, chanMax=36;
  } else {
    chanMin=channel, chanMax=channel;
  }




  // Buffer to hold readout data, used for fifo1,2,3,error,tts and last-dac.
  // Should equal to the largest possible fifo. 
  // unsigned long buffer[(36*1024)]; // Data buffer for the FIFO (fifo1 = 36*1024?)
  //unsigned long long sbuffer[1024]; // For Slink format
  uint32_t buffer[(36*1024)]; // Data buffer for the FIFO (fifo1 = 36*1024?)
  uint64_t sbuffer[4096]; // For Slink format


  // File to store data
#ifdef WRITE_FILE
  ofstream outfile("data.dat");
#endif

#ifdef  BINARY_WRITE // open a new file 
  ofstream dataFile("PhysicsData1.dmp",ios::binary);
  dataFile.close();
#endif

  // Init HAL

//   HAL::CAENLinuxBusAdapter *busAdapter=0;
//   if(VMEBoard==1)
//     busAdapter=new HAL::CAENLinuxBusAdapter( HAL::CAENLinuxBusAdapter::V2718 ); //optical
//   else if(VMEBoard==2)
//     busAdapter=new HAL::CAENLinuxBusAdapter(HAL::CAENLinuxBusAdapter::V1718); //usb
//   else {
//     cout<<" VME interface not chosen "<<VMEBoard<<endl;
//     exit(1);
//   }

  HAL::CAENLinuxBusAdapter busAdapter( HAL::CAENLinuxBusAdapter::V2718 );
  HAL::VMEAddressTableASCIIReader addressTableReader("data/FEDAddressMap.dat");
  HAL::VMEAddressTable addressTable("Test address table",addressTableReader);
  HAL::VMEDevice PixFEDCard(addressTable, busAdapter, fedBase);
  
  
//   HAL::VMEAddressTableASCIIReader addressTableReader("data/FEDAddressMap.dat");
//   HAL::VMEAddressTable addressTable("Test address table",addressTableReader);
//   HAL::VMEDevice PixFEDCard(addressTable, *busAdapter, fedBase);
  
  // Init PxFED
  PixelFEDInterface fed1(&PixFEDCard); // Instantiate the FED class
 
  cout<<" Init PFED, base = "<<hex<<fedBase<<dec<<endl;

  // Do the setup only if NOT in Lite_Mode
  if(!Lite_Mode) {

    // Try the FPGA reload (normaly not done) 
    // resets histogram fifo counters
    //fed1.loadFPGA();  // clears the error and lastdac fifos (tts fifo still has 1 entry)
    
    // Reset FED 
    status=fed1.reset();
    if(status!=0) exit(0); // exit error
    
    // Try to read the board id
    uint32_t data = 0;
    PixFEDCard.read("READ_GA",&data);
    cout<<" PFED Board ID = "<<data<<endl;
    //PixFEDCard.write("WriteID",data);

    // get Firmware
    //cout<<" FPGA Firmware version "<<endl;
    for(int i=0;i<5;++i) {
      //      uint32_t fdate=fed1.get_FirmwareDate(i);
      //cout<<" 0x"<<hex<<fdate<<dec<<endl;
    }
    
    
    // set all channels OF
    //fed1.set_chnls_onoff(1); // normaly not done
    
    // SETUP FROM THE PARM FILE 
    // Read in parameters from a file and download them to the FED
    PixelFEDCard pixelFEDCard(fileName); // instantiate the FED settings(should be private)
    
    //status = fed1.setupFromDB(fileName);
    status = fed1.setupFromDB(pixelFEDCard);
    if(status==-1) {
      cout<<" No configuration file "<<endl;
      return(0);
    }
    
    
    // Do not do it. It overrides the settings in the setup file.
    //fed1.set_chnls_onoff(1); // 0=Channels on, needed for fifo1 (norm & trans),fifo2
    
    // DAC SETUP
    // Fill up the dac memory with simulated data
    if(DACMode) { // read emulated data from the DAC     
      //fed1.fillDACRegister();
      // For the test DACs
      int dac1[DAC_SIZE], dac2[DAC_SIZE], dac3[DAC_SIZE];
      defineTestDACs(dac1, dac2, dac3);
      fed1.fillDACRegister(dac1, dac2, dac3);
    }
    
    
    // BASE LINE CORRECTION
    if(BaselineCorr) {
      //fed1.BaselineCorr_off();
      fed1.set_BaselineCorr(1,512); // channel 1-9 , value=512
      fed1.set_BaselineCorr(2,512); // channel 10-18 , value=512
      fed1.set_BaselineCorr(3,512); // channel 19-27 , value=512
      fed1.set_BaselineCorr(4,512); // channel 28-36 , value=512
      fed1.BaselineCorr_on(); //All
      //fed1.BaselineCorr_on(25);  // only for  channel 25    
      //fed1.BaselineCorr_on(26);  // only for  channel 26    
      //fed1.BaselineCorr_on(27);  // only for  channel 26    
      //fed1.BaselineCorr_on(28);  // only for  channel 26    
      //fed1.BaselineCorr_on(29);  // only for  channel 26    
      //fed1.BaselineCorr_on(30);  // only for  channel 26    
      
      //unsigned long int value = (0x1<<(15+36-27)) +  512;
      //cout<<" base = "<<hex<<value<<dec<<endl;
      //PixFEDCard.write("SWrBaseLAdj",value);
      
    } else {
      fed1.BaselineCorr_off();
    }
    
    
    // ADC GAIN, revisit!
    //fed1.set_adc_1v2v(1);  // does not seem to have any effect
    
    
    // MODE register
    // Reset the SLink (no needed, no slink in my setup)
    fed1.resetSlink();
    
    // Blocks data from reaching Fifo3 (comment out) 
    //fed1.setModeRegister(0x1);  // disable slink, if this set there is no data in fifo3
    //fed1.setModeRegister(0xB);  // disable Slink & fifo3 & ignore LFF
    fed1.setModeRegister(0x8);  // ignore slink LFF
    //fed1.setModeRegister(0x9);  // ignore slink LFF + disable slink
    
    // Control write to FIFO3 spy memory
    //fed1.setModeRegister(0x2);  // same effect as this
    
    // Set the mode  register bit 1, ->  enable writinig to spy memory
    //status=fed1.enableSpyMemory(1); // bit 1 = 0 in mode reg 
    // set the mode to disable spy memory
    status=fed1.enableSpyMemory(0); // bit 1 = 1 in mode reg
    
    
    // Check the Mode register 
    int mode = fed1.getModeRegister(); //check the mode
    cout<<" ----------------> Mode register = "<<hex<<mode<<dec<<endl;
    
    // CONTROL REGISTER
    // Get control register value after it was set from DB
    int cntrl = fed1.getControlRegister();  // read it
    int cntrl2 = fed1.getCcntrl();  // last written
    cout<<" Control register "<<hex<<cntrl<<" "<<cntrl2<<dec<<endl; // 
    
    int value=0;
    if(TTCvxyes) {  // use TTC triggers
      
      // enable L1A from TTC 
      //value = 0x10; // transparent gate from L1A, TTC event#, use this for fifoII 
      value = 0x18; // transparent gate from L1A, TTC event#, for V4 fifoI =0x18 
      if(readFifo2 || readFifo3) value = 0x10;
      
      if(transparentMode) value = value | 0x1; // set transparent bit
      if(DACMode) value= value | 0x4; // enable DAC data
      
      if(enableTTSReady) value  = value | 0x10000;  // Enable the TTSReady messgae to the  TTS fifo
      if(enableTTSError) value = value | 0x20000;  // Enable the TTSError message to the TTS fifo
      if(enableOutOfSyncError) value = value | 0x40000;  // Enable the out-of-sync error to TTS
      
    } else {  // VME triggers
      
      value = 0x0a; // transparent gate from VME & VME event#
      if(transparentMode) value = value | 0x1; // set transparent bit
      if(DACMode) value = value | 0x4; // enable DAC data
      
      if(enableTTSReady) value  = value | 0x10000;  // Enable the TTSReady messgae to the  TTS fifo
      if(enableTTSError) value = value | 0x20000;  // Enable the TTSError message to the TTS fifo
      if(enableOutOfSyncError) value = value | 0x40000;  // Enable the out-of-sync error to TTS
      

    } // end if TTCvxyes

    // Set the control register
    cout<<" Load control register with "<<hex<<value<<dec<<" dec="<<value<<endl;
    status = fed1.setControlRegister(value);
  } // end if Lite_Mode

  if(Setup_Only) return 0;

    
  // Get control register
  int cntrl = fed1.getControlRegister();  // read it
  int cntrl2 = fed1.getCcntrl();  // last written
  cout<<" --------------------> Control register "<<hex<<cntrl<<" "<<cntrl2<<dec<<endl; // 



#ifdef DELAY_SCAN
  // Select the clock phase
  int delay=0;  // delay value
  cout<<" enter delay = ";
  cin>>delay;
  status = fed1.setPhases(channel,delay); // select phase for channel 1
  cout<<" delay selected "<<delay<<" "<<channel<<" "<<status<<endl;
#endif


//   if( readFifo3 ) { 
//     if(PRINT) cout<<" read fifo3 "<<endl;
//     int wdcnt=fed1.spySlink64(sbuffer); // Read FIFO3
//     //int wdcnt=fed1.drainDataFifo3(buffer); // Read FIFO3
//     if(PRINT) cout<<" read fifo3 length  "<<wdcnt<<endl;
//   }

  //int param = 0;
  int baseCorr[36] = {0};;

  // Test the even register
  int eventNumber=fed1.readEventCounter();
  cout<<"VME event Number "<<hex<<eventNumber<<dec<<endl;
  int previousNumber=eventNumber; // -1;

#ifdef CHECK_TTCRX
  // Get the event number directly from the TTCrx register. TESTING only.
  int ttcrx_ev1 = fed1.TTCRX_I2C_REG_READ( 26); //  
  int ttcrx_ev2 = fed1.TTCRX_I2C_REG_READ( 27); //  
  int ttcrx_ev3 = fed1.TTCRX_I2C_REG_READ( 28); //  
  cout<<" TTCrx event "<<hex<<ttcrx_ev1<<" "<<ttcrx_ev2<<" "<<ttcrx_ev3<<dec<<endl;
#endif

  int dummy; 
  //   cout<<" continue ";
  //   cin>>dummy;
  cout<<" Start DAQ"<<endl;


  //float   time1 ,time2;  
  //time1 = (float) clock() / (float) CLOCKS_PER_SEC;
  clock_t   time1 ,time2;  
  time1=time(0);

  // UnBlock Fifo3 
  if( readFifo3 ) {
    status=fed1.enableSpyMemory(1); // enable spy-memory
  }

  // Loop for n events.
  for(int n=0;n<nMax;n++) {   
    if(PRINT) cout<<" loop "<<n<<endl;

    // reset fifos again
    //PixFEDCard.write("LRES",0x80000000);

    if(DumpBaselineValues) {
      cout<<" Baseline Correction Values"<<endl; fed1.dump_BaselineCorr();
    }

    // Funny tests
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
    //if(modifyDACs) {
    // modifyTestDACs(dac1, dac2, dac3);
    // fed1.fillDACRegister(dac1, dac2, dac3);
    //}


    // Trigger part
    if(TTCvxyes) {   // TTC trigger

      eventNumber=fed1.readEventCounter();  // This is now the new event counter
      if(eventNumber>previousNumber) { 
	cout<<" Missed an event, wait for another "<<eventNumber<<" "<<previousNumber<<endl;
	previousNumber=eventNumber;
      }
      if(PRINT) cout<<" event "<<eventNumber<<endl;

      // wait for trigger
      for(int wait=0;wait<1000;wait++) {

	// Event number
	eventNumber=fed1.readEventCounter();  // This is now the new event counter
	if(eventNumber>previousNumber) {  // Might be a problem with wrap around ????
	  

	  if(eventNumber==0) cout<<" After a reset or something wrong "<<previousNumber<<endl;
	  if(eventNumber%NUM_PRINT==0) cout<<" New event "<<eventNumber<<" "
					   <<wait<<endl;
	  break;
	} else if(READ_CONTINOUS) {  // Do NOT wait for new event
	  cout<<" enter something ";
	  cin>>dummy;
	  break;  
	}
	usleep(1000); //  
      } // for wait

      if(eventNumber<=previousNumber && !READ_CONTINOUS) {
	if(eventNumber==0) cout<<" After a reset"<<endl;
	else cout<<" no new event"<<endl;
	continue;  // skip the rest if event provessing
      }
      

    } else {   // VME trigger
      if(testingOthers) cout<<" Generate a VME trigger "<<endl;
      if(generateTrigger) status = fed1.generateVMETrigger();
    }
    
    // Event number
    eventNumber=fed1.readEventCounter();  // This is now the new event counter
    if(eventNumber>1 && eventNumber>(previousNumber+1))
      cout<<"Some events were lost "<<eventNumber<<" "<<previousNumber<<endl;
    else if(eventNumber>1 && eventNumber<(previousNumber+1))
      cout<<" NO NEW EVENT OR WRONG EVENT NUMBER "<<eventNumber<<" "<<previousNumber<<endl;

    previousNumber=eventNumber;

    // Some additinal readout tests
    if(testingOthers  && eventNumber%NUM_PRINT==0 ) {
      int bx = fed1.readBXCounter();  // Check bx

#ifdef CHECK_TTCRX
      // Get the event number directly from the TTCrx register. TESTING only.
      int ttcrx_ev1 = fed1.TTCRX_I2C_REG_READ( 26); //  
      int ttcrx_ev2 = fed1.TTCRX_I2C_REG_READ( 27); //  
      int ttcrx_ev3 = fed1.TTCRX_I2C_REG_READ( 28); //  
#endif
      
      // Try to access the baseline correction
      if(BaselineCorr) {
	for(int chan=chanMin;chan<(chanMax+1);chan++) {  // loop over channels 
	  int tmp = fed1.get_BaselineCorr(chan);
	  // Convert from unsgied to sigend
	  if(tmp>511 ) {
	    tmp = tmp | 0xfffffc00;
	  }
	  baseCorr[chan] =tmp;
	}
      }

      if(eventNumber%NUM_PRINT==0) {
	cout<<" Loop "<<n<<" ";
	cout<<"Event number "<<eventNumber<<" bx="<<hex<<bx<<dec
	    <<" ";
	//cout<<" TTCrx event "<<ttcrx_ev1<<" "<<ttcrx_ev2<<" "<<ttcrx_ev3<<" ";
	//cout<<" Control registers "<<hex<<cntrl0<<" "<<cntrl<<dec;
	cout<<endl;
	
	if(BaselineCorr) {
	  cout<<" Base correction = ";
	  for(int chan=chanMin;chan<(chanMax+1);chan++)  // loop over channels 
	    cout<<baseCorr[chan]<<" ";
	  cout<<endl;
	}

	//gets fifo status and dumps it
	if(fifoStatus) {
	  uint32_t fstat=fed1.getFifoStatus();
	  cout<<" FIFO status "<<hex<<fstat<<dec<<endl;
	  fed1.dump_FifoStatus(fstat);
	} // if fifoStatus

      }
    } // end if testOthers

    // Read data from FIFO 1
    if(readFifo1) { 

      //Check to see if transparent mode is set
      if(transparentMode) {  // In transparent mode
	  
	for(int chan=chanMin;chan<(chanMax+1);chan++) {  // loop over channels 
	  //for(int chan=channel;chan<(channel+1);chan++) {  // one channel 
	  // For transparent data it is difficult to find where the valid data ends
	  // so sttaus does not return the correct data length. 
	  //status=fed1.drainFifo1(chan,buffer,1024); // Read FIFO1 for  channel, length in bytes
	  status=fed1.drainFifo1(chan,buffer,4096); // Read FIFO1 for  channel, length in bytes
          if(eventNumber%NUM_PRINT==0 && testingOthers) 
	    cout<<" Fifo1 transparent readout, channel "<<chan
				<<" status = "<<status<<endl;
	  if(status>-1 && decodeData && n<LIST_FULL) 
	    PixelFEDFifoData::decodeTransparentData(buffer,255); // Decode data, length in words
	  //PixelFEDFifoData::decodeTransparentData(buffer,1024); // Decode data, length in words

#ifdef WRITE_FILE

          param = baseCorr[chan];
          //const int NUM_OF_CHANNELS = 1;
          //const int VERSION = 1;
          //if(chan==chanMin) 
	  outfile<<WRITE_LENGTH<<" "<<chan<<" "<<eventNumber<<" "<<param<<endl;
           
          //int i0=0;
          //unsigned long data = 0, maxdata=0, data1=0, data2=0, sum=0, sum1=0, sum2=0, temp; 
          unsigned long data = 0, sum=0, sum1=0, sum2=0; 
          for(int i=0;i<WRITE_LENGTH;++i) {
            data = (buffer[i]  & 0xffc00000)>>22; // analyze word
            //data1= (buffer1[i] & 0xffc00000)>>22; // analyze word
            //data2= (buffer2[i] & 0xffc00000)>>22; // analyze word
             
            if(eventNumber%NUM_PRINT==0) {
              //cout<<data<<" ";  // print all
              if( i>=PRINT_MIN && i<PRINT_MAX) cout<<data<<" ";  // skip first 10 bx
	      else if(i>=PRINT_MAX && data<UB_LIMIT) cout<<i<<" "<<data<<" ";  // print onlu ub
              if(i>=2 && i<=PRINT_MIN) { // use only before the UB                   
                sum += data;
                //sum1 += data1;
                //sum2 += data2;
              }
            }
            //outfile<<i<<" "<<data<<" "<<data1<<endl; // OLD, format until run 69
            //if(chan==chanMin) 
	    outfile<<data<<endl; // starting fom run 70
          }

          if(eventNumber%NUM_PRINT==0) {
            sum = sum/(PRINT_MIN-2+1);
            //sum1 = sum1/20;
            //sum2 = sum2/20;
            cout<<" -- "<<sum<<" "<<sum1<<" "<<sum2<<endl;
            //outfile<<temp<<" "<<sum1<<" "<<sum2<<endl; // special test for base-line
          }

#endif
	}  // end for loop over channels
  
      } else { // In normal mode 

	//status=fed1.drainFifo1(25,buffer,4096); // Read FIFO1, one channel, length in bytes, def=1024
	status=fed1.drainFifo1(buffer); // Read FIFO1, all channels (defaut fifo size) 
	if (testingOthers) cout<<" FIFO 1, normal mode, status = "<<status<<endl;
	if(status>-1 && decodeData) 
	  PixelFEDFifoData::decodeNormalData(buffer,status); //  Decode data	

	
      }	// end if

    } // if read fifo1
    
    // Now do FIFO2
    if( readFifo2 && !transparentMode ) {  //  do only in normal mode mode
      cout<<" Read Spy FIFO2 "<<endl;
      int wdcnt=0;
 
//       wdcnt=fed1.drainDataFifo2(1,buffer); // Read FIFO2, chip N UP
//       cout<<" Spy fifo2-1up, count = "<<wdcnt<<endl; 
//       if(wdcnt>0 && decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer,wdcnt);

//       wdcnt=fed1.drainDataFifo2(2,buffer); // Read FIFO2, chip N DOWN
//       cout<<" Spy fifo2-1dw, count = "<<wdcnt<<endl; 
//       if(wdcnt>0 && decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer,wdcnt);

//       wdcnt=fed1.drainDataFifo2(3,buffer); // Read FIFO2, chip NC UP
//       cout<<" Spy fifo2-2up, count = "<<wdcnt<<endl; 
//       if(wdcnt>0 && decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer,wdcnt);

//       wdcnt=fed1.drainDataFifo2(4,buffer); // Read FIFO2, chip NC DOWN
//       cout<<" Spy fifo2-2dw, count = "<<wdcnt<<endl; 
//       if(wdcnt>0 && decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer,wdcnt);

//       wdcnt=fed1.drainDataFifo2(5,buffer); // Read FIFO2, chip SC UP
//       cout<<" Spy fifo2-3up, count = "<<wdcnt<<endl; 
//       if(wdcnt>0 && decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer,wdcnt);

//       wdcnt=fed1.drainDataFifo2(6,buffer); // Read FIFO2, chip SC DOWN
//       cout<<" Spy fifo2-3dw, count = "<<wdcnt<<endl; 
//       if(wdcnt>0 && decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer,wdcnt);

      wdcnt=fed1.drainDataFifo2(7,buffer); // Read FIFO2, chip S UP
      cout<<" Spy fifo2-4up, count = "<<wdcnt<<endl; 
      if(wdcnt>0 && decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer,wdcnt);

      wdcnt=fed1.drainDataFifo2(8,buffer); // Read FIFO2, chip S DOWN
      cout<<" Spy fifo2-4dw, count = "<<wdcnt<<endl; 
      if(wdcnt>0 && decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer,wdcnt);

    } // end if
     
    // Data Spy-Fifo3
    if( readFifo3 ) { 

      if(PRINT) cout<<" read fifo3 "<<eventNumber<<endl;

      status=fed1.enableSpyMemory(0); // blocks writing to spy-memory, during reading time

      int wdcnt=fed1.spySlink64(sbuffer); // Read FIFO3
      //int wdcnt=fed1.drainDataFifo3(buffer); // Read FIFO3

      if(PRINT) cout<<" read fifo3 length  "<<wdcnt<<endl;

      // In case of error
      if(wdcnt==-1) {
	cout<<" empty readout, event "<<eventNumber<<endl;
	//for(int i=0;i<(wdcnt1);i++) {	  
	//  cout<<i<<" "<<hex<<buffer[i]<<dec<<" ";
	//}
	//PixelFEDFifoData::decodeSpyFifo3(buffer,wdcnt1);
	//return 1;  // stop
      }

      if(eventNumber%NUM_PRINT==0) {
	if(testingOthers) cout<<" Spy fifo3, count = "<<wdcnt<<" "<<endl; 
	if(wdcnt>0 && decodeData) 
	  //PixelFEDFifoData::decodeSpyFifo3(buffer,wdcnt);
	  PixelFEDFifoData::decodeSlink64(sbuffer,wdcnt);
      }

#ifdef WRITE_FILE
      wdcnt++;  // For Slink format add 1
      outfile<<eventNumber<<" "<<wdcnt<<dec<<endl;
      for(int i=0;i<(wdcnt);i++) {	  
	//cout<<i<<" "<<hex<<buffer[i]<<dec<<" ";
	//cout<<i<<" "<<hex<<sbuffer[i]<<dec<<endl;
	//if(decodeData) PixelFEDFifoData::decodeSpyDataFifo(buffer[i]);
	//outfile<<hex<<buffer[i]<<dec<<endl; // fifo3 format
	outfile<<hex<<sbuffer[i]<<dec<<endl;  // Slink format
      } // for
#endif
// #ifdef BINARY_WRITE
//       // Write binary data files in Souviks format.
//       ofstream dataFile("PhysicsData1.dmp",ios::binary |ios::app);
//       dataFile.write((char*)sbuffer, wdcnt*sizeof(unsigned long long));
//       dataFile.close();

// #endif
      
      status=fed1.enableSpyMemory(1);  // enable again fifo writting

//       /////////////////////
// 	}  // testing
//       }
//       ///////////////////

    } // end if readFifo3


    // ERROR FIFOs
    if(readError && (eventNumber%NUM_PRINT==0)) { 
      //if( readError ) { 
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


//     int dummy=0;
//     cout<<" enter "<<endl;
//     cin>>dummy;
//     if(dummy==-1) break;


  } // end for events

// 	if(ReadHis) { int status=fed1.enableHisMemory(0);//disable
// 	int size=26*pixelFEDCard.NRocs*12;
// 	unsigned long HisBuf[size];//Dcol*pixelFEDCard.NRocs*tripplets
// 	status=fed1.drainHisMemory(HisBuf);
	
// // Just a sample to let you know how to histogram in root or paw or other	
// if(DumpHis){
//        for(int i=0;i<12;i++){
//         for(int j=0;j<pixelFEDCard.NRocs;j++){
//          for(int k=0;k<26;k++){if(HisBuf[i*pixelFEDCard.NRocs*26+j*26+(k)]!=0){
//         cout<<"trip "<<dec<<i+1<<" Roc "<<dec<<j+1<<" DC "<<dec<<k<<" "<<hex<<HisBuf[i*pixelFEDCard.NRocs*26+j*26+(k)]<<endl;
// 	//decode tripplet
// 	if((HisBuf[i*pixelFEDCard.NRocs*26+j*26+(k)]&0xff)!=0)cout<<"channel "<<dec<<i*3+1<<" Roc "<<dec<<j+1<<
// 	" DC "<<dec<<k<<" #hits = "<<dec<<(HisBuf[i*pixelFEDCard.NRocs*26+j*26+(k)]&0xff)<<endl;//low 8bit
// 	if((HisBuf[i*pixelFEDCard.NRocs*26+j*26+(k)]&0xff00)!=0)cout<<"channel "<<dec<<i*3+2<<" Roc "<<dec<<j+1<<
// 	" DC "<<dec<<k<<" #hits = "<<dec<<((HisBuf[i*pixelFEDCard.NRocs*26+j*26+(k)]&0xff00)>>8)<<endl;//med 8bit
// 	if((HisBuf[i*pixelFEDCard.NRocs*26+j*26+(k)]&0xff0000)!=0)cout<<"channel "<<dec<<i*3+3<<" Roc "<<dec<<j+1<<
// 	" DC "<<dec<<k<<" #hits = "<<dec<<((HisBuf[i*pixelFEDCard.NRocs*26+j*26+(k)]&0xff0000)>>16)<<endl;//high 8bit	
// 	                      }
//        }}}
// }	
	
	
//	}

  //time2 = (float) clock() / (float) CLOCKS_PER_SEC;
  //cout<<" time for "<<nMax<<" loops, is = "<<(time2-time1)<<endl;
  time2=time(0);
  double sec = difftime(time2,time1);
  cout<<" time for "<<nMax<<" loops, is = "<<sec<<endl;
 
  
  } catch ( HAL::HardwareAccessException& e ) {
    cout << "*** Exception occurred : " << endl;
    cout << e.what() << endl;
  } catch ( exception e ) {
    cout << "*** Unknown exception occurred" << endl;
  }

  lock.release();
  return 0;
}
////////////////////////////////////////////////////////////////////
// Functions (load each 24 channel with 8 rocs + 1 pixel, and 12 channels with 16rocs+ 1 pixel)
void modifyTestDACs(int *const dac1,int *const dac2, int *const dac3) {

  const unsigned long int V_OFFSET=100; // 100; 
  const unsigned long int UB =50;
  const unsigned long int B = 300;

  const int levels[6] = {200,300,410,500,650,750}; //OK for groups 1,2,3,4,5, not for 6,7,8
  // to be exact for channels: 24,25 - 28 - 33,36
  //const int levels[6] = {200,300,420,510,650,750}; //OK for groups 1,6,7,8, not for 2 


  static int n1=0, n2=0, n3=0, n4=0;  // 1st event is 0
  //int n11=0, n12=0, n13=0, n14=0; // for the status
  //static int count = 0;

  int START=0;  // define the start of the data packet (starts in fifo=10)

  // Limits are     :    270, 400, 500, 600, 700
  // The levels are : 200, 300, 450, 550, 650, 750  

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

  cout<<" DACs "<<dac1[START+4]<<" "<<dac1[START+5]<<" "<<dac1[START+6]<<" "<<dac1[START+7]<<endl;

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
// Maxiumu words = DAC_SIZE 
void defineTestDACs(int *const dac1,int *const dac2, int *const dac3) {

  const unsigned long int V_OFFSET = 100; // 100; 
  const unsigned long int UB = 50; 
  const unsigned long int B = 500;

  // Limits are     :    450, 550, 650, 750, 850
  // The levels are : 400, 500, 600, 700, 800, 900  
  const unsigned long int Lvl1 = 400;
  const unsigned long int Lvl2 = 500;
  const unsigned long int Lvl3 = 600;
  const unsigned long int Lvl4 = 700;
  const unsigned long int Lvl5 = 800;
  const unsigned long int Lvl6 = 900;

  cout<<"Filling the dac registers"<<endl;

  // DAC TEST DATA //////////////////////////////////// 
  for(int i=0;i<DAC_SIZE;i++) {
    dac1[i]=V_OFFSET+B; //+B
    dac2[i]=V_OFFSET+B; //+B
    dac3[i]=V_OFFSET+B; //+B
  }

  int START = 0;  // define the start of the data packet (starts in fifo=10)

  //dac1[START+0] = V_OFFSET+UB;   //UB  TBM-header
  //return;


  dac1[START+0] = V_OFFSET+UB;   //UB  TBM-header
  dac1[START+1] = V_OFFSET+UB;   //UB
  dac1[START+2] = V_OFFSET+UB;   //UB
  dac1[START+3] = V_OFFSET+B;    //B
  dac1[START+4] = V_OFFSET+Lvl2;  // event number
  dac1[START+5] = V_OFFSET+Lvl2;
  dac1[START+6] = V_OFFSET+Lvl2;
  dac1[START+7] = V_OFFSET+Lvl2;

  START += 8;                    // ROC#0

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#0
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2] = V_OFFSET+Lvl6;  //  LastDac

  dac1[START+3] = V_OFFSET+Lvl1;  // dcol MSB PIXEL 1
  dac1[START+4] = V_OFFSET+Lvl1;  // dcol LSB     
  dac1[START+5] = V_OFFSET+Lvl1;  // pix 
  dac1[START+6] = V_OFFSET+Lvl1;  // pix 
  dac1[START+7] = V_OFFSET+Lvl2;  // pix there can be not pixel 0,0   
  dac1[START+8] = V_OFFSET+Lvl1;  // PH

  START += 9;                    // ROC#1

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#1
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2] = V_OFFSET+Lvl2;  //  LastDac

  dac1[START+3] = V_OFFSET+Lvl1;  // dcol MSB PIXEL 1
  dac1[START+4] = V_OFFSET+Lvl2;  // dcol LSB     
  dac1[START+5] = V_OFFSET+Lvl1;  // pix 
  dac1[START+6] = V_OFFSET+Lvl1;  // pix 
  dac1[START+7] = V_OFFSET+Lvl2;  // pix   
  dac1[START+8] = V_OFFSET+Lvl2;  // PH

  START += 9;                    // ROC#2

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#2
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2] = V_OFFSET+Lvl3;  //  LastDac

  dac1[START+3]= V_OFFSET+Lvl1;  // dcol MSB PIXEL 1
  dac1[START+4]= V_OFFSET+Lvl3;  // dcol LSB     
  dac1[START+5]= V_OFFSET+Lvl1;  // pix 
  dac1[START+6]= V_OFFSET+Lvl1;  // pix 
  dac1[START+7]= V_OFFSET+Lvl3;  // pix   
  dac1[START+8]= V_OFFSET+Lvl3;  // PH

  START += 9;                    // ROC#3

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#3
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2]= V_OFFSET+Lvl4;  //  LastDac

  dac1[START+3]= V_OFFSET+Lvl1;  // dcol MSB PIXEL 1
  dac1[START+4]= V_OFFSET+Lvl4;  // dcol LSB     
  dac1[START+5]= V_OFFSET+Lvl1;  // pix 
  dac1[START+6]= V_OFFSET+Lvl1;  // pix 
  dac1[START+7]= V_OFFSET+Lvl4;  // pix   
  dac1[START+8]= V_OFFSET+Lvl4;  // PH

  START += 9;                    // ROC#4

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#4
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2] = V_OFFSET+Lvl5;  //  LastDac

  dac1[START+3]= V_OFFSET+Lvl1;  // dcol MSB PIXEL 1
  dac1[START+4]= V_OFFSET+Lvl5;  // dcol LSB     
  dac1[START+5]= V_OFFSET+Lvl1;  // pix 
  dac1[START+6]= V_OFFSET+Lvl1;  // pix 
  dac1[START+7]= V_OFFSET+Lvl5;  // pix   
  dac1[START+8]= V_OFFSET+Lvl5;  // PH

  START += 9;                    // ROC#5

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#5
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2]= V_OFFSET+Lvl6;  //  LastDac

  dac1[START+3]= V_OFFSET+Lvl1;  // dcol MSB PIXEL 1
  dac1[START+4]= V_OFFSET+Lvl6;  // dcol LSB     
  dac1[START+5]= V_OFFSET+Lvl1;  // pix 
  dac1[START+6]= V_OFFSET+Lvl1;  // pix 
  dac1[START+7]= V_OFFSET+Lvl6;  // pix   
  dac1[START+8]= V_OFFSET+Lvl6;  // PH

  START += 9;                    // ROC#6

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#6
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2]= V_OFFSET+Lvl1;  //  LastDac

  dac1[START+3]= V_OFFSET+Lvl2;  // dcol MSB PIXEL 1
  dac1[START+4]= V_OFFSET+Lvl1;  // dcol LSB     
  dac1[START+5]= V_OFFSET+Lvl1;  // pix 
  dac1[START+6]= V_OFFSET+Lvl2;  // pix 
  dac1[START+7]= V_OFFSET+Lvl1;  // pix   
  dac1[START+8]= V_OFFSET+Lvl1;  // PH

  START += 9;                    // ROC#7

  dac1[START+0] = V_OFFSET+UB;   //UB  ROC#7
  dac1[START+1] = V_OFFSET+B;    //B
  dac1[START+2] = V_OFFSET+Lvl2;  //  LastDac
  
  dac1[START+3]= V_OFFSET+Lvl3;  // dcol MSB PIXEL 1
  dac1[START+4]= V_OFFSET+Lvl1;  // dcol LSB     
  dac1[START+5]= V_OFFSET+Lvl1;  // pix 
  dac1[START+6]= V_OFFSET+Lvl3;  // pix 
  dac1[START+7]= V_OFFSET+Lvl1;  // pix   
  dac1[START+8]= V_OFFSET+Lvl1;  // PH

  START += 9;  // next pixel

  int adc = 400;
  int level = 500;
  for(int n=0;n<20;n++) {
  //for(int n=0;n<28;n++) {
    dac1[START+0]= V_OFFSET+Lvl1;  // dcol MSB PIXEL 1
    dac1[START+1]= V_OFFSET+level;  // dcol LSB     
    dac1[START+2]= V_OFFSET+Lvl1;  // pix 
    dac1[START+3]= V_OFFSET+Lvl1;  // pix 
    dac1[START+4]= V_OFFSET+level;  // pix   
    dac1[START+5]= V_OFFSET+adc ;  // PH
    //cout<<n<<" "<<adc<<endl;
    adc += 100;
    level += 100;
    if(adc==1000) adc=400;    
    if(level==1000) level=500;    
    START += 6;                    // update
    if(START>DAC_SIZE) cout<<" Error DAC array too small"<<DAC_SIZE<<endl;
  }

  //START += 9;                    // TBM Trailer

  //cout<<" START = "<<START<<endl;

  dac1[START+0]= V_OFFSET+UB;   // UB  TBM trailer
  dac1[START+1]= V_OFFSET+UB;   // UB
  dac1[START+2]= V_OFFSET+B;    // B
  dac1[START+3]= V_OFFSET+B;    // B
  dac1[START+4]= V_OFFSET+Lvl1;  // Status
  dac1[START+5]= V_OFFSET+Lvl1;
  dac1[START+6]= V_OFFSET+Lvl1;
  dac1[START+7]= V_OFFSET+Lvl1;
  
  START+=8;
  if(START>=DAC_SIZE) cout<<" Error: last DAC address too large "<<START<<endl;
  cout<<" DAC array = "<<START<<endl;

  // DAC2 same 
  for(int i=0;i<DAC_SIZE;i++) dac2[i]=dac1[i]; // copy same to dac2

  // DAC3 same
  for(int i=0;i<DAC_SIZE;i++) dac3[i]=dac1[i]; // copy same to dac3

  return;


  // DAC3, make it with 16 rocs
  // DAC TEST DATA //////////////////////////////////// 
  for(int i=0;i<DAC_SIZE;i++) dac3[i]=V_OFFSET+B; //+B

  START=0;  // define the start of the data packet (starts in fifo=10)

  // Limits are     :    270, 400, 500, 600, 700
  // The levels are : 200, 300, 450, 550, 650, 750  

  dac3[START+0] = V_OFFSET+UB;   //UB  TBM-header
  dac3[START+1] = V_OFFSET+UB;   //UB
  dac3[START+2] = V_OFFSET+UB;   //UB
  dac3[START+3] = V_OFFSET+B;    //B
  dac3[START+4] = dac1[START+4];  // event number
  dac3[START+5] = dac1[START+5];  // event number
  dac3[START+6] = dac1[START+6];  // event number
  dac3[START+7] = dac1[START+7];  // event number

  START += 8;                    // ROC#0

  dac3[START+0] = V_OFFSET+UB;   //UB  ROC#0
  dac3[START+1] = V_OFFSET+B;    //B
  dac3[START+2]= V_OFFSET+Lvl1;  //  LastDac

  dac3[START+3]= V_OFFSET+Lvl5;  // dcol MSB PIXEL 1
  dac3[START+4]= V_OFFSET+Lvl2;  // dcol LSB     
  dac3[START+5]= V_OFFSET+Lvl2;  // pix 
  dac3[START+6]= V_OFFSET+Lvl1;  // pix 
  dac3[START+7]= V_OFFSET+Lvl1;  // pix   
  dac3[START+8]= V_OFFSET+Lvl1;  // PH

  START += 9;                    // ROC#1

  dac3[START+0] = V_OFFSET+UB;   //UB  ROC#1
  dac3[START+1] = V_OFFSET+B;    //B
  dac3[START+2]= V_OFFSET+Lvl2;  //  LastDac

  dac3[START+3]= V_OFFSET+Lvl5;  // dcol MSB PIXEL 1
  dac3[START+4]= V_OFFSET+Lvl2;  // dcol LSB     
  dac3[START+5]= V_OFFSET+Lvl3;  // pix 
  dac3[START+6]= V_OFFSET+Lvl1;  // pix 
  dac3[START+7]= V_OFFSET+Lvl1;  // pix   
  dac3[START+8]= V_OFFSET+Lvl2;  // PH

  START += 9;                    // ROC#2

  dac3[START+0] = V_OFFSET+UB;   //UB  ROC#2
  dac3[START+1] = V_OFFSET+B;    //B
  dac3[START+2]= V_OFFSET+Lvl3;  //  LastDac

  dac3[START+3]= V_OFFSET+Lvl5;  // dcol MSB PIXEL 1
  dac3[START+4]= V_OFFSET+Lvl2;  // dcol LSB     
  dac3[START+5]= V_OFFSET+Lvl4;  // pix 
  dac3[START+6]= V_OFFSET+Lvl1;  // pix 
  dac3[START+7]= V_OFFSET+Lvl1;  // pix   
  dac3[START+8]= V_OFFSET+Lvl3;  // PH

  START += 9;                    // ROC#3

  dac3[START+0] = V_OFFSET+UB;   //UB  ROC#3
  dac3[START+1] = V_OFFSET+B;    //B
  dac3[START+2]= V_OFFSET+Lvl4;  //  LastDac

  dac3[START+3]= V_OFFSET+Lvl5;  // dcol MSB PIXEL 1
  dac3[START+4]= V_OFFSET+Lvl2;  // dcol LSB     
  dac3[START+5]= V_OFFSET+Lvl5;  // pix 
  dac3[START+6]= V_OFFSET+Lvl1;  // pix 
  dac3[START+7]= V_OFFSET+Lvl1;  // pix   
  dac3[START+8]= V_OFFSET+Lvl4;  // PH

  START += 9;                    // ROC#4

  dac3[START+0] = V_OFFSET+UB;   //UB  ROC#4
  dac3[START+1] = V_OFFSET+B;    //B
  dac3[START+2]= V_OFFSET+Lvl5;  //  LastDac

  dac3[START+3]= V_OFFSET+Lvl5;  // dcol MSB PIXEL 1
  dac3[START+4]= V_OFFSET+Lvl2;  // dcol LSB     
  dac3[START+5]= V_OFFSET+Lvl6;  // pix 
  dac3[START+6]= V_OFFSET+Lvl1;  // pix 
  dac3[START+7]= V_OFFSET+Lvl1;  // pix   
  dac3[START+8]= V_OFFSET+Lvl5;  // PH

  START += 9;                    // ROC#5

  dac3[START+0] = V_OFFSET+UB;   //UB  ROC#5
  dac3[START+1] = V_OFFSET+B;    //B
  dac3[START+2]= V_OFFSET+Lvl6;  //  LastDac

  dac3[START+3]= V_OFFSET+Lvl5;  // dcol MSB PIXEL 1
  dac3[START+4]= V_OFFSET+Lvl2;  // dcol LSB     
  dac3[START+5]= V_OFFSET+Lvl1;  // pix 
  dac3[START+6]= V_OFFSET+Lvl2;  // pix 
  dac3[START+7]= V_OFFSET+Lvl1;  // pix   
  dac3[START+8]= V_OFFSET+Lvl6;  // PH

  START += 9;                    // ROC#6

  dac3[START+0] = V_OFFSET+UB;   //UB  ROC#6
  dac3[START+1] = V_OFFSET+B;    //B
  dac3[START+2]= V_OFFSET+Lvl1;  //  LastDac

  dac3[START+3]= V_OFFSET+Lvl5;  // dcol MSB PIXEL 1
  dac3[START+4]= V_OFFSET+Lvl2;  // dcol LSB     
  dac3[START+5]= V_OFFSET+Lvl2;  // pix 
  dac3[START+6]= V_OFFSET+Lvl2;  // pix 
  dac3[START+7]= V_OFFSET+Lvl1;  // pix   
  dac3[START+8]= V_OFFSET+Lvl1;  // PH

  START += 9;                    // ROC#7

  dac3[START+0] = V_OFFSET+UB;   //UB  ROC#7
  dac3[START+1] = V_OFFSET+B;    //B
  dac3[START+2]= V_OFFSET+Lvl2;  //  LastDac

  dac3[START+3]= V_OFFSET+Lvl5;  // dcol MSB PIXEL 1
  dac3[START+4]= V_OFFSET+Lvl2;  // dcol LSB     
  dac3[START+5]= V_OFFSET+Lvl3;  // pix 
  dac3[START+6]= V_OFFSET+Lvl2;  // pix 
  dac3[START+7]= V_OFFSET+Lvl1;  // pix   
  dac3[START+8]= V_OFFSET+Lvl2;  // PH

  START += 9;                    // ROC#8

  dac3[START+0] = V_OFFSET+UB;   //UB  ROC#8
  dac3[START+1] = V_OFFSET+B;    //B
  dac3[START+2]= V_OFFSET+Lvl3;  //  LastDac

  dac3[START+3]= V_OFFSET+Lvl5;  // dcol MSB PIXEL 1
  dac3[START+4]= V_OFFSET+Lvl2;  // dcol LSB     
  dac3[START+5]= V_OFFSET+Lvl4;  // pix 
  dac3[START+6]= V_OFFSET+Lvl2;  // pix 
  dac3[START+7]= V_OFFSET+Lvl1;  // pix   
  dac3[START+8]= V_OFFSET+Lvl3;  // PH

  START += 9;                    // ROC#9

  dac3[START+0] = V_OFFSET+UB;   //UB  ROC#9
  dac3[START+1] = V_OFFSET+B;    //B
  dac3[START+2]= V_OFFSET+Lvl4;  //  LastDac

  dac3[START+3]= V_OFFSET+Lvl5;  // dcol MSB PIXEL 1
  dac3[START+4]= V_OFFSET+Lvl2;  // dcol LSB     
  dac3[START+5]= V_OFFSET+Lvl5;  // pix 
  dac3[START+6]= V_OFFSET+Lvl2;  // pix 
  dac3[START+7]= V_OFFSET+Lvl1;  // pix   
  dac3[START+8]= V_OFFSET+Lvl4;  // PH

  START += 9;                    // ROC#10

  dac3[START+0] = V_OFFSET+UB;   //UB  ROC#10
  dac3[START+1] = V_OFFSET+B;    //B
  dac3[START+2]= V_OFFSET+Lvl5;  //  LastDac

  dac3[START+3]= V_OFFSET+Lvl5;  // dcol MSB PIXEL 1
  dac3[START+4]= V_OFFSET+Lvl2;  // dcol LSB     
  dac3[START+5]= V_OFFSET+Lvl6;  // pix 
  dac3[START+6]= V_OFFSET+Lvl2;  // pix 
  dac3[START+7]= V_OFFSET+Lvl1;  // pix   
  dac3[START+8]= V_OFFSET+Lvl5;  // PH

  START += 9;                    // ROC#11

  dac3[START+0] = V_OFFSET+UB;   //UB  ROC#11
  dac3[START+1] = V_OFFSET+B;    //B
  dac3[START+2]= V_OFFSET+Lvl6;  //  LastDac

  dac3[START+3]= V_OFFSET+Lvl5;  // dcol MSB PIXEL 1
  dac3[START+4]= V_OFFSET+Lvl2;  // dcol LSB     
  dac3[START+5]= V_OFFSET+Lvl1;  // pix 
  dac3[START+6]= V_OFFSET+Lvl3;  // pix 
  dac3[START+7]= V_OFFSET+Lvl1;  // pix   
  dac3[START+8]= V_OFFSET+Lvl6;  // PH

  START += 9;                    // ROC#12

  dac3[START+0] = V_OFFSET+UB;   //UB  ROC#12
  dac3[START+1] = V_OFFSET+B;    //B
  dac3[START+2]= V_OFFSET+Lvl1;  //  LastDac

  dac3[START+3]= V_OFFSET+Lvl5;  // dcol MSB PIXEL 1
  dac3[START+4]= V_OFFSET+Lvl2;  // dcol LSB     
  dac3[START+5]= V_OFFSET+Lvl2;  // pix 
  dac3[START+6]= V_OFFSET+Lvl3;  // pix 
  dac3[START+7]= V_OFFSET+Lvl1;  // pix   
  dac3[START+8]= V_OFFSET+Lvl1;  // PH

  START += 9;                    // ROC#13

  dac3[START+0] = V_OFFSET+UB;   //UB  ROC#13
  dac3[START+1] = V_OFFSET+B;    //B
  dac3[START+2]= V_OFFSET+Lvl2;  //  LastDac

  dac3[START+3]= V_OFFSET+Lvl5;  // dcol MSB PIXEL 1
  dac3[START+4]= V_OFFSET+Lvl2;  // dcol LSB     
  dac3[START+5]= V_OFFSET+Lvl3;  // pix 
  dac3[START+6]= V_OFFSET+Lvl3;  // pix 
  dac3[START+7]= V_OFFSET+Lvl1;  // pix   
  dac3[START+8]= V_OFFSET+Lvl2;  // PH

  START += 9;                    // ROC#14

  dac3[START+0] = V_OFFSET+UB;   //UB  ROC#14
  dac3[START+1] = V_OFFSET+B;    //B
  dac3[START+2]= V_OFFSET+Lvl3;  //  LastDac

  dac3[START+3]= V_OFFSET+Lvl5;  // dcol MSB PIXEL 1
  dac3[START+4]= V_OFFSET+Lvl2;  // dcol LSB     
  dac3[START+5]= V_OFFSET+Lvl4;  // pix 
  dac3[START+6]= V_OFFSET+Lvl3;  // pix 
  dac3[START+7]= V_OFFSET+Lvl1;  // pix   
  dac3[START+8]= V_OFFSET+Lvl3;  // PH

  START += 9;                    // ROC#15

  dac3[START+0] = V_OFFSET+UB;   //UB  ROC#15
  dac3[START+1] = V_OFFSET+B;    //B
  dac3[START+2]= V_OFFSET+Lvl4;  //  LastDac

  dac3[START+3]= V_OFFSET+Lvl5;  // dcol MSB PIXEL 1
  dac3[START+4]= V_OFFSET+Lvl2;  // dcol LSB     
  dac3[START+5]= V_OFFSET+Lvl5;  // pix 
  dac3[START+6]= V_OFFSET+Lvl3;  // pix 
  dac3[START+7]= V_OFFSET+Lvl1;  // pix   
  dac3[START+8]= V_OFFSET+Lvl4;  // PH

  START += 9;  // next pixel

  adc = 0;
  for(int n=0;n<8;n++) {
    dac3[START+0]= V_OFFSET+Lvl5;  // dcol MSB PIXEL 1
    dac3[START+1]= V_OFFSET+Lvl2;  // dcol LSB     
    dac3[START+2]= V_OFFSET+Lvl3;  // pix 
    dac3[START+3]= V_OFFSET+Lvl2;  // pix 
    dac3[START+4]= V_OFFSET+Lvl1;  // pix   
    dac3[START+5]= V_OFFSET+adc ;  // PH
    adc += 100;
    if(adc>800) adc=0;
    
    START += 6;                    // update
  }

  //START += 9;                    // TBM Trailer

  dac3[START+0]= V_OFFSET+UB;   // UB  TBM trailer
  dac3[START+1]= V_OFFSET+UB;   // UB
  dac3[START+2]= V_OFFSET+B;    // B
  dac3[START+3]= V_OFFSET+B;    // B
  dac3[START+4]= V_OFFSET+Lvl1;  // Status
  dac3[START+5]= V_OFFSET+Lvl1;
  dac3[START+6]= V_OFFSET+Lvl1;
  dac3[START+7]= V_OFFSET+Lvl1;
  
  START+=8;
  if(START>=DAC_SIZE) cout<<" Error: last DAC address too large "<<START<<endl;


}


