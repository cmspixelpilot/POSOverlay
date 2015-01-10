#ifndef _TemperatureFIFODecoder_h_
#define _TemperatureFIFODecoder_h_

/*************************************************************************
 * Decoding of last DAC Temperature FIFO                                 *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2012/02/17 15:42:20 $ (UTC)                       *
 *          by: $Author: mdunser $                                       *
 *************************************************************************/

#include <iostream>
#include <list>
#include <stdint.h>

#include "PixelUtilities/PixelFEDDataTools/include/PixelLastDACTemperature.h"

class TemperatureFIFODecoder
{
 public:
  TemperatureFIFODecoder(uint32_t * buffer, unsigned int numWords);
   
  const std::list<PixelLastDACTemperature>& getLastDACTemperatures() const { return lastDACTemperatures_; }

  void printBuffer(std::ostream& stream) const;

 private:
  void decodeBuffer();

  uint32_t * buffer_;
  unsigned int numWords_;

  std::list<PixelLastDACTemperature> lastDACTemperatures_;
};

#endif
