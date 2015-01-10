#ifndef PIXEL_SLINK_HEADER_H
#define PIXEL_SLINK_HEADER_H

#include "Word64.h"

namespace pos {

  class PixelSLinkHeader {

  public:    
    PixelSLinkHeader(uint64_t=0);
    PixelSLinkHeader(Word64);

    bool isHeader();
    Word64 getWord64();
    unsigned int getBOE();
    unsigned int getEvt_ty();
    unsigned int getLV1_id();
    unsigned int getBX_id();
    unsigned int getSource_id();
    unsigned int getFOV();
    unsigned int getH();
    unsigned int getSLinkHardwareBits();

  private:
    Word64 header_;
  };

}

#endif
