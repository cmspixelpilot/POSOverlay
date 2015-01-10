#include "PixelUtilities/PixelFEDDataTools/include/TemperatureFIFODecoder.h"
#include <stdint.h>
#include <iostream>

/*************************************************************************
 * Decoding of last DAC Temperature FIFO                                 *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2012/02/17 15:42:20 $ (UTC)                       *
 *          by: $Author: mdunser $                                       *
 *************************************************************************/

//--- define bit-masks used for decoding of FIFO buffer
//const uint32_t lastDACId_mask     = 0xC00000000;
//const uint32_t lastDACId_rightShift     = 32;
const uint32_t fedChannel_mask    = 0x0FC000000;
const uint32_t fedChannel_rightShift    = 26;
const uint32_t readOutChipId_mask = 0x003E00000;
const uint32_t readOutChipId_rightShift = 21;
const uint32_t dacValue_mask      = 0x0000000FF;

TemperatureFIFODecoder::TemperatureFIFODecoder(uint32_t* buffer, unsigned int numWords)
{
//--- save buffer and
//    number of data words stored in buffer
//
//    WARNING: only pointer to buffer memory is stored (no local copy of buffer memory is made);
//             therefore must not overwrite buffer while TemperatureFIFODecoder is in use !!!
//
  buffer_ = buffer;
  numWords_ = numWords;

//--- decode buffer
  decodeBuffer();
}

void TemperatureFIFODecoder::decodeBuffer()
{
//--- reset internal data structures
//    (should not be neccessary as decodeBuffer is only called from constructor; 
//     just done for extra safety...)
  lastDACTemperatures_.clear();

  for ( unsigned int iWord = 0; iWord < numWords_; ++iWord ) {
    uint32_t word = buffer_[iWord];

    //unsigned int lastDACId = ((word & lastDACId_mask) >> lastDACId_rightShift);

//--- skip "empty" words in FIFO buffer not associated to last DAC temperature read-out
//    (artefact of last DAC temperarure decoding 
//     implemented in FPGAs mounted on FED board)
    //if ( lastDACId != 0xC ) {
    if ( word == 0 ) {
      std::cerr << "Warning in <TemperatureFIFODecoder::decodeBuffer>: found word = " << std::hex << word << std::dec 
		<< " not associated to last DAC temperature read-out in last DAC FIFO !!!" << std::endl;
      continue;
    }
    
    unsigned int fedChannel = ((word & fedChannel_mask) >> fedChannel_rightShift);
    unsigned int readOutChipId = ((word & readOutChipId_mask) >> readOutChipId_rightShift);
    unsigned int dacValue = (word & dacValue_mask);
    
    lastDACTemperatures_.push_back(PixelLastDACTemperature(fedChannel, readOutChipId, dacValue));
  }
}

void TemperatureFIFODecoder::printBuffer(std::ostream& stream) const
{
  //stream << "last DAC Temperature FIFO ----" << std::endl;
  for ( unsigned int iWord = 0; iWord < numWords_; ++iWord ) {
    uint32_t word = buffer_[iWord];
    stream << " word #" << iWord << " has 0x" << std::hex << word << std::dec << std::endl;
  }
  stream << "------------------------------------" << std::endl;
}

