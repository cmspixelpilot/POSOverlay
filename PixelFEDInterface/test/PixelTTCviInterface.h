#ifndef TP_PIXELTTCVIINTERFACE_H
#define TP_PIXELTTCVIINTERFACE_H
//
   
#include <string>
using namespace std;

#define USE_HAL 

#ifdef USE_HAL // Access VME with HALa
#include "VMEDevice.hh"
#else // direct CAEN
#include "CAENVMElib.h"  // CAEN library prototypes
#endif //USE_HAL


class PixelTTCviInterface {
 public:

#ifdef USE_HAL // Access VME with HAL

  PixelTTCviInterface(const HAL::VMEDevice * const);
  ~PixelTTCviInterface();

#else // direct CAEN

  PixelTTCviInterface(const unsigned long fedBase, long BHandle);
  ~PixelTTCviInterface();
  // Generic FIFO2 access  

#endif // USE_HAL

  int setup_trigger(int mode); // control TTCvi
  int trigger(void);
  int sendECR(void);
  int sendBCR(void);
  int sendBRCST(int message);

 private:


#ifdef USE_HAL  // Access VME with HAL
  const HAL::VMEDevice *const vmeDevicePtr;


#else  // Direct with CANE

  // VME Addresses 
  unsigned long TTCVIBASE; //Fed Base Address
  //unsigned long LAD_BRCST;


  // For the CAEN VME 
  long BHandle; // pointer to the device
  CVDataWidth dw; // data width (see CAENVMEtypes.h )
  CVAddressModifier am;  // VME address modifier
  CVErrorCodes    ret; // return code

#endif // USE_HAL 

};

#endif // ifdef declare
