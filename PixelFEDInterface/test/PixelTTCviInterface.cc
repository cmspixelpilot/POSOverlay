 
#include <iostream>
#include <time.h>
#include <unistd.h> // for usleep()

#include "PixelTTCviInterface.h"

using namespace std;

namespace {
  const bool PRINT = false;
  //const bool PRINT = true;
}

#ifdef USE_HAL // Access VME with HAL

//// Constructor //////////////////////////////////////////////////
PixelTTCviInterface::PixelTTCviInterface(const HAL::VMEDevice * const vmeDeviceP ) : 
  vmeDevicePtr(vmeDeviceP) {
  cout<<" PixelTTCviInterface constructor "<<endl;

}
//////////////////////////////////////////////////////////////////////
PixelTTCviInterface::~PixelTTCviInterface(void) {
  cout<<" PixelTTCviInterface destructor "<<endl;
}

/////////////////////////////////////////////////////////////////////////

#else // Direct CAEN VME


PixelTTCviInterface::PixelTTCviInterface(const unsigned long ttcviBase, long aBHandle) {

  // For the CAEN interface
  BHandle = aBHandle;  // store the VME pointer 
  dw = cvD32; // data width (see CAENVMEtypes.h )
  am = cvA32_U_DATA;

  TTCVIBASE = ttcviBase; // FED base address

  cout<<" PixelTTCviInterface constructor "<<hex<<FEDBASE<<" "<<BHandle<<dec<<endl;

  // Define the FED registers
  //LAD_N       = (FEDBASE);          // N-Altera
}
//////////////////////////////////////////////////////////////////////
PixelTTCviInterface::~PixelTTCviInterface(void) {
  cout<<" PixelTTCviInterface destructor "<<endl;
}

extern void analyzeError(CVErrorCodes ret); // Wills VME error analyzer.

# endif // USE_HAL

//////////////////////////////////////////////////////////////////////
// TTCvi routines 
//
int PixelTTCviInterface::setup_trigger(int mode) {

//   // Reset 
  unsigned long data = 0;
  cout<<" TTCvi: Reset module "<<endl;

#ifdef USE_HAL // Use HAL

  vmeDevicePtr->write("SoftReset", data );

#else  // Use direct CAEN
//   CVErrorCodes ret = 
//     CAENVME_WriteCycle(BHandle,TTCVIBASE+0x84,&data,cvA24_U_DATA,cvD16);
//   if(ret != cvSuccess) {  // Error
//     cout<<"Error in TTCvi write "<<hex<<ret<<" "<<data<<dec<<endl;
//     analyzeError(ret);
//     return -1;
//   }
#endif // USE_HAL

  cout<<" TTCvi: Reset counters "<<endl;
  
#ifdef USE_HAL // Use HAL
  
  vmeDevicePtr->write("CounterReset", data );

#else  // Use direct CAEN

//   ret = 
//     CAENVME_WriteCycle(BHandle,TTCVIBASE+0x8C,&data,cvA24_U_DATA,cvD16);
//   if(ret != cvSuccess) {  // Error
//     cout<<"Error in TTCvi write "<<hex<<ret<<" "<<data<<dec<<endl;
//     analyzeError(ret);
// return -1;
//}

#endif // USE_HAL

//   //VME-L1A settings CSR1
  data = 0x0004; // select VME L1A
  //data = 0x0005 + 0x0000; // select random L1A 1Hz
  //data = 0x0005 + 0x1000; // select random L1A 100Hz
  //data = 0x0005 + 0x2000; // select random L1A 1kHz
  //data = 0x0001; // select L1A from input 1
  cout<<" TTCvi: Setup L1A "<<endl;

#ifdef USE_HAL // Use HAL

  vmeDevicePtr->unmaskedWrite("L1ATriggerSelect", data );

#else  // Use direct CAEN

  unsigned long TTCVICSR1 = TTCVIBASE + 0x80;
//   ret = CAENVME_WriteCycle(BHandle,TTCVICSR1,&data,cvA24_U_DATA,cvD16);
//   if(ret != cvSuccess) {  // Error
//     cout<<"Error in TTCvi write "<<hex<<ret<<" "<<data<<dec<<endl;
//     analyzeError(ret);
//     return -1;
//   }
#endif // USE_HAL

  //VME-L1A settings CSR2 (not realy needed)
  data = 0xff00;

#ifdef USE_HAL // Use HAL

  vmeDevicePtr->write("CSR2", data );

#else  // Use direct CAEN

// //     ret = CAENVME_WriteCycle(BHandle,TTCVICSR2,&data,cvA24_U_DATA,cvD16);
// //     if(ret != cvSuccess) {  // Error
// //       cout<<"Error in write TTCVICSR2"<<hex<<ret<<" "<<data<<dec<<endl;
// //       analyzeError(ret);
// //     }
#endif // USE_HAL

  // DO NOT DO THIS
//   // EVENT and BUNCH COUNTER RESET through a broadcast from TTCvi
//   unsigned long TTCVI_sBRD = TTCVIBASE + 0xc4;
//   data = 0x3; // bit 0&1 set, clear event and bx counters.
// #ifdef USE_HAL // Use HAL
//   vmeDevicePtr->write("SoftReset", data );
// #else  // Use direct CAEN
// //   ret = CAENVME_WriteCycle(BHandle,TTCVI_sBRD,&data,cvA24_U_DATA,cvD16);
// //   if(ret != cvSuccess) {  // Error
// //     cout<<"Error in write TTCVI_sBRD"<<hex<<ret<<" "<<data<<dec<<endl;
// //     analyzeError(ret);
// //   }        
//#endif // USE_HAL
  
  return 0;
} //end
//----------------------------------------------------------------------
//
int PixelTTCviInterface::trigger() {
  const bool LOCAL_PRINT = false;

  // send L1A Trigger
  uint32_t data = 0x1;

#ifdef USE_HAL // Use HAL

  vmeDevicePtr->unmaskedWrite("L1ASend", data );

#else  // Use direct CAEN

//   CVErrorCodes ret = 
//     CAENVME_WriteCycle(BHandle, TTCVIBASE+0x86,&data,cvA24_U_DATA,cvD16);
//   if(ret != cvSuccess) {  // Error
//     cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
//     analyzeError(ret);
//     return -1;
//   }

#endif // USE_HAL

  // REad the trigger counter from TTCvi
  uint32_t data1 = 0;
#ifdef USE_HAL // Use HAL

  vmeDevicePtr->read("CounterLSB", &data );
  vmeDevicePtr->read("CounterMSB", &data1 );

#else  // Use direct CAEN
 
//   ret = 
//     CAENVME_ReadCycle(BHandle, TTCVIBASE+0x8A,&data,cvA24_U_DATA,cvD16);
//   if(ret != cvSuccess) {  // Error
//     cout<<"Error in write "<<hex<<ret<<" "<<data<<dec<<endl;
//     analyzeError(ret);
//     return -1;
//   }

#endif // USE_HAL

  if(LOCAL_PRINT) cout<<" generate ttcvi trigger "<<data<<" "<<data1<<endl;
  return data;
}
//----------------------------------------------------------------------
// send a event counter reset
int PixelTTCviInterface::sendECR() {

  // send event counter reset
  unsigned long data = 0x02;

#ifdef USE_HAL // Use HAL

  //vmeDevicePtr->unmaskedWrite("BChannelShortBroadcast", data );
  vmeDevicePtr->write("BChannelShortBroadcast", data );

#else  // Use direct CAEN

// //   ret = CAENVME_WriteCycle(BHandle,TTCVI_sBRD,&data,cvA24_U_DATA,cvD16);
// //   if(ret != cvSuccess) {  // Error
// //     cout<<"Error in write TTCVI_sBRD"<<hex<<ret<<" "<<data<<dec<<endl;
// //     analyzeError(ret);
// //   }        

#endif // USE_HAL


  cout<<" ECR "<<data<<endl;
  return data;
}
// send a bx counter reset
int PixelTTCviInterface::sendBCR() {

  // send bx counter reset
  unsigned long data = 0x01;

#ifdef USE_HAL // Use HAL

  //vmeDevicePtr->unmaskedWrite("BChannelShortBroadcast", data );
  vmeDevicePtr->write("BChannelShortBroadcast", data );

#else  // Use direct CAEN

// //   ret = CAENVME_WriteCycle(BHandle,TTCVI_sBRD,&data,cvA24_U_DATA,cvD16);
// //   if(ret != cvSuccess) {  // Error
// //     cout<<"Error in write TTCVI_sBRD"<<hex<<ret<<" "<<data<<dec<<endl;
// //     analyzeError(ret);
// //   }        

#endif // USE_HAL


  //cout<<" BCR "<<data<<endl;
  return data;
}
////////////////////////////////////////////////////////
// send a broadcast command 
int PixelTTCviInterface::sendBRCST(int command) {

  // send event counter reset
  unsigned long data = command;

#ifdef USE_HAL // Use HAL

  vmeDevicePtr->write("BChannelShortBroadcast", data );

#else  // Use direct CAEN

// //   ret = CAENVME_WriteCycle(BHandle,TTCVI_sBRD,&data,cvA24_U_DATA,cvD16);
// //   if(ret != cvSuccess) {  // Error
// //     cout<<"Error in write TTCVI_sBRD"<<hex<<ret<<" "<<data<<dec<<endl;
// //     analyzeError(ret);
// //   }        

#endif // USE_HAL


  cout<<" Broadcast "<<data<<endl;
  return data;
}
