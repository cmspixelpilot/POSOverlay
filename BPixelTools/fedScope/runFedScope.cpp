#include <iostream>
#include <time.h>
#include <unistd.h>  // for usleep

using namespace std;

// HAL includes
#include "VMEDevice.hh"
#include "VMEAddressTable.hh"
#include "VMEAddressTableASCIIReader.hh"
#include "CAENLinuxBusAdapter.hh"

// Pixel includes
#include "PixelFEDInterface/include/PixelFEDInterface.h" // PixFED class definition
#include "PixelFEDInterface/include/PixelFEDFifoData.h" // PixFED data decode
// NEW:
#include "CalibFormats/SiPixelObjects/interface/PixelFEDCard.h" //The FED settings structure
// OLD:
//#include "PixelConfigDataFormats/include/PixelFEDCard.h"

#include "FedProducer.hh"
#include "../tools/include/VMELock.h"


// NEW:
using pos::PixelFEDCard;

// ----------------------------------------------------------------------
int initialize(PixelFEDInterface &fed1, HAL::VMEDevice &PixFEDCard) {

  // Reset FED
  int status = fed1.reset();
  if (status != 0) exit(0); 
  
  // Try to read the board id
  uint32_t data = 0;
  PixFEDCard.read("READ_GA",&data);
  cout<<" Initilize FED, Board ID = "<<data<<endl;

  // Setup from file
  string fileName("params_fed.dat"); // define the file name
  PixelFEDCard pixelFEDCard(fileName); // instantiate the FED settings(should be private)
  
  status = fed1.setupFromDB(pixelFEDCard);
  if (status == -1) {
    cout<<" No configuration file "<<endl;
    return(0);
  }

  fed1.BaselineCorr_off();
  fed1.resetSlink();
  fed1.setModeRegister(0x8);  // ignore slink LFF
  status = fed1.enableSpyMemory(0); // bit 1 = 1 in mode reg
  int mode = fed1.getModeRegister(); //check the mode
  cout << " ----------------> Mode register = "<<hex<<mode<<endl;

  int cntrl = fed1.getControlRegister();  // read it
  int cntrl2 = fed1.getCcntrl();  // last written
  cout << " Control register " << hex << cntrl << " " << cntrl2 << dec << endl; 
  
  int value=0;
  value = 0x18; // transparent gate from L1A, TTC event#, for V4 fifoI =0x18
  value = value | 0x1; // set transparent bit
  // Set the control register
  cout << " Load control register with " << hex << value << dec << " dec= " << value << endl;
  status = fed1.setControlRegister(value);  
  return(0); 
}

 
// ======================================================================
int main(int argc, char *argv[]) {

  // -- FED base address
  unsigned long fedBase(0);
  // fedBase = 0x11000000; // window
  fedBase = 0x10000000; // door
  std::map<string, unsigned long> fedbases;
  fedbases["-1N"]=0x10000000;fedbases["-2N"]=0x11000000;fedbases["-3N"]=0x12000000;fedbases["-4N"]=0x13000000;
  fedbases["+4N"]=0x14000000;fedbases["+3N"]=0x15000000;fedbases["+2N"]=0x16000000;fedbases["+1N"]=0x17000000;
  fedbases["+8N"]=0x18000000;fedbases["+7N"]=0x19000000;fedbases["+6N"]=0x1A000000;fedbases["+5N"]=0x1B000000;
  fedbases["-5N"]=0x1C000000;fedbases["-6N"]=0x1D000000;fedbases["-7N"]=0x1E000000;fedbases["-8N"]=0x1F000000;

  fedbases["+1P"]=0x10000000;fedbases["+2P"]=0x11000000;fedbases["+3P"]=0x12000000;fedbases["+4P"]=0x13000000;
  fedbases["-4P"]=0x14000000;fedbases["-3P"]=0x15000000;fedbases["-2P"]=0x16000000;fedbases["-1P"]=0x17000000;
  fedbases["-8P"]=0x18000000;fedbases["-7P"]=0x19000000;fedbases["-6P"]=0x1A000000;fedbases["-5P"]=0x1B000000;
  fedbases["+5P"]=0x1C000000;fedbases["+6P"]=0x1D000000;fedbases["+7P"]=0x1E000000;fedbases["+8P"]=0x1F000000;

  VMELock lock(1);

  // -- command line arguments
  int fullInit(0);
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i], "-i")) fullInit = 1; 
    if (!strcmp(argv[i], "-f")) {
      sscanf(argv[++i], "%x", &fedBase);
      cout << "FED base = 0x" << hex << fedBase << dec << endl;
    }
    if (!strcmp(argv[i], "-s")) {
      fedBase=fedbases[argv[++i]];
      cout << argv[i] << "FED base = 0x" << hex << fedBase << dec << endl;
    }
  }

  lock.acquire();


  const int NUM_PRINT = 10;

  // Init ROOT
  FedProducer fedScope;

  cout << "0" << endl;
  HAL::CAENLinuxBusAdapter busAdapter( HAL::CAENLinuxBusAdapter::V2718 );
  cout << "1" << endl;
  //  HAL::CAENLinuxBusAdapter busAdapter( HAL::CAENLinuxBusAdapter::V1718 );
  cout << "2" << endl;
  HAL::VMEAddressTableASCIIReader addressTableReader("FEDAddressMap.dat");
  cout << "3" << endl;
  HAL::VMEAddressTable addressTable("Test address table",addressTableReader);
  cout << "4" << endl;
  HAL::VMEDevice PixFEDCard(addressTable, busAdapter, fedBase);
  cout << "5" << endl;
  
  PixelFEDInterface fed1(&PixFEDCard); // Instantiate the FED class
  cout << "6" << endl;

  if (fullInit == 1) {
    fullInit = initialize(fed1, PixFEDCard);
  }
  lock.release();

  //unsigned long buffer[(36*1024)]; // Data buffer for the FIFO (fifo1 = 36*1024?)
  //unsigned long long sbuffer[1024]; // For Slink format
  uint32_t buffer[(36*1024)]; // Data buffer for the FIFO (fifo1 = 36*1024?)
  uint64_t sbuffer[4096]; // For Slink format

  const int channel=0; // all chanels
  int chanMin=1;
  int chanMax=1;
  if (channel == 0) {
    chanMin=1, chanMax=36;
  } else {
    chanMin=channel, chanMax=channel;
  }
  lock.acquire();
  int eventNumber = fed1.readEventCounter();
  lock.release();
  int previousNumber = eventNumber; 

  // -- Loop forever
  int status;
  while(1) {
    lock.acquire();
    eventNumber=fed1.readEventCounter();  // This is now the new event counter
    lock.release();
    if (eventNumber > previousNumber) {
      if (eventNumber%NUM_PRINT == 0) 
	cout <<" New event "<<eventNumber<<" "<< endl;					  
    } 
    usleep(1000); //  about 20ms
    
    if (eventNumber==previousNumber) {
      cout<<" no new event"<<endl;
      continue;
    }
    lock.acquire();
    eventNumber=fed1.readEventCounter();  // This is now the new event counter
    previousNumber=eventNumber;
    
    for (int chan = chanMin; chan < (chanMax+1); ++chan) {  
      status = fed1.drainFifo1(chan, buffer, 4096); 
      fedScope.readBuffer(chan-1, buffer, eventNumber);
    } 
    lock.release();
  } 

  return 0;
}

