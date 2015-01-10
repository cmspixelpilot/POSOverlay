/*************************************************************************
 * Decoding of last DAC value FIFO                                       *
 *                                                                       *
 * Author: James Zabel, Rice University                                  *
 *                                                                       *
 * Last update: $Date: 2012/02/17 14:03:18 $ (UTC)                       *
 *          by: $Author: mdunser $                                         *
 *************************************************************************/

#ifndef _LastDACFIFODecoder_h_
#define _LastDACFIFODecoder_h_

#include <iostream>
#include <list>
#include <stdint.h>

#include "PixelUtilities/PixelFEDDataTools/include/PixelLastDACReading.h"

class LastDACFIFODecoder
{
  public:
    LastDACFIFODecoder(uint32_t * buffer, unsigned int numWords);
    const std::list<PixelLastDACReading>& getLastDACReading() const {return lastDACReading_;}
    void printBuffer(std::ostream& stream) const;

  private:
    void decodeBuffer();

    uint32_t * buffer_;
    unsigned int numWords_;

    std::list<PixelLastDACReading> lastDACReading_;
};

#endif

