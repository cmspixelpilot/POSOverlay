#ifndef TP_PIXELFEDFIFODATA_H
#define TP_PIXELFEDFIFODATA_H

// A Class to handle the docodeing of the PixelFED FIFO information.
// Extarcted from Will Johns's PixelFED prototype class.
// d.k. 14/4/06
   
#include <iostream>
#include <string>
#include <stdint.h>
using namespace std;

class PixelFEDFifoData {
 public:

  // Constructor and destructor
  PixelFEDFifoData() {};  // empty
  ~PixelFEDFifoData() {}; // empty

  // Decode data from FIFO1
  // Decode the transparent mode 
  static void decodeTransparentData(uint32_t *data, const int length = 255); 
  // Decode the normal mode
  static void decodeNormalData(uint32_t *data, const int length = 1024);  
  
  // Decode data from FIFO2, spy, error and remperature
  static void decodeSpyDataFifoSlink64(uint32_t word, ostream &out = cout); 
  static void decodeSpyDataFifo(uint32_t word, ostream &out = cout); 
  static void decodeErrorFifoSlink64(uint32_t word); 
  static void decodeErrorFifo(uint32_t word); 
  static void decodeTemperatureFifo(uint32_t word); 
  static void decodeTTSFifo(uint32_t word); 

  static void decodeSpyDataFifo(const uint32_t *const data, const int length = 128); 
  static void decodeErrorFifo(const uint32_t *const data, const int length = 256); 
  static uint32_t decodeErrorFifo(const uint32_t *const data, const int length, const uint64_t mask);
  static void decodeTemperatureFifo(const uint32_t *const data, const int length = 256); 
  static void decodeTTSFifo(const uint32_t *const data, const int length = 256); 

  // Decode FIFO3, length in words
  static void decodeSpyFifo3(uint32_t *data, const int length = 1024);  
  static void decodeSlink64(uint64_t *data, const int length, ostream &out = cout);  

 private:


};

#endif // ifdef declare
