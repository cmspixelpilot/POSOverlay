#ifndef PIXEL_SLINK_TRAILER_H
#define PIXEL_SLINK_TRAILER_H

#include "Word64.h"

namespace pos {

  class PixelSLinkTrailer {

  public:    
    PixelSLinkTrailer (uint64_t=0);
    PixelSLinkTrailer (Word64);

    bool isTrailer();
    Word64 getWord64();
    unsigned int getEOE();
    unsigned int getEvt_lgth();
    unsigned int getCRC();
    unsigned int getEVT_stat();
    unsigned int getTTS();
    unsigned int getT();
    unsigned int getSLinkHardwareBits();

  private:
    Word64 trailer_;

  };

}

#endif
