#ifndef PIXEL_HIT_H
#define PIXEL_HIT_H

#include "Word32.h"

namespace pos {

  class PixelHit {

  public:
    PixelHit ();
    PixelHit (Word32);
    PixelHit (uint32_t);
    //PixelHit (unsigned int);

    Word32 getWord32();
    unsigned int getLink_id();
    unsigned int getROC_id();
    unsigned int getDCol_id();
    unsigned int getPix_id();
    unsigned int getADC();
    unsigned int getRow();
    unsigned int getColumn();

  private:
    Word32   w32Hit_;
    uint32_t hit_;

  };

}

#endif
