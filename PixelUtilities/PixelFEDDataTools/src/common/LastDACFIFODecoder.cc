/*************************************************************************
 * Decoding of last DAC value FIFO                                       *
 *                                                                       *
 * Author: James Zabel, Rice University                                  *
 *                                                                       *
 * Last update: $Date: 2012/02/17 14:03:17 $ (UTC)                       *
 *          by: $Author: mdunser $                                         *
 *************************************************************************/

#include "PixelUtilities/PixelFEDDataTools/include/LastDACFIFODecoder.h"

//define bit-masks used for decoding of FIFO buffer
const uint32_t fedChannel_mask = 0x0FC000000;
const uint32_t fedChannel_rightShift = 26;
const uint32_t readOutChipId_mask = 0x003E00000;
const uint32_t readOutChipId_rightShift = 21;
const uint32_t dacValue_mask = 0x0000000FF;

LastDACFIFODecoder::LastDACFIFODecoder(uint32_t * buffer, unsigned int numWords)
{
  buffer_ = buffer;
  numWords_ = numWords;

  decodeBuffer();
}

void LastDACFIFODecoder::decodeBuffer()
{
  lastDACReading_.clear();
  uint32_t word;
  unsigned int fedChannel;
  unsigned int readOutChipId;
  unsigned int dacValue;

  for(unsigned int iWord = 0; iWord < numWords_; ++iWord){
    word = buffer_[iWord];
    if(word != 0){
      fedChannel = ((word & fedChannel_mask) >> fedChannel_rightShift);
      readOutChipId = ((word & readOutChipId_mask) >> readOutChipId_rightShift);
      dacValue = (word & dacValue_mask);
      lastDACReading_.push_back(PixelLastDACReading(fedChannel, readOutChipId, dacValue));
    }
  }
}

void LastDACFIFODecoder::printBuffer(std::ostream& stream) const
{
  stream << "-------- LastDACFIFODecoder --------" << std::endl;
  for ( unsigned int iWord = 0; iWord < numWords_; ++iWord ) {
    uint32_t long word = buffer_[iWord];
    stream << " word #" << iWord << " has 0x" << std::hex << word << std::dec << std::endl;
  }
  stream << "------ End LastDACFIFODecoder ------" << std::endl;
}
