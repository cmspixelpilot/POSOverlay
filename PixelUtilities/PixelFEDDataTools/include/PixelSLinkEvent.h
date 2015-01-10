#ifndef PIXEL_SLINK_EVENT_H
#define PIXEL_SLINK_EVENT_H

#include <stdint.h>

#include <iostream>
#include <vector>

#include "Word32.h"
#include "Word64.h"
#include "PixelSLinkHeader.h"
#include "PixelSLinkTrailer.h"
#include "PixelHit.h"
#include "PixelError.h"

namespace pos {

  class PixelSLinkEvent {

  public:
    PixelSLinkEvent(){};
    PixelSLinkEvent(std::vector<Word64>);
    PixelSLinkEvent(uint64_t* buffer, unsigned int length);

    bool decodeEvent();

    PixelSLinkHeader getHeader();
    PixelSLinkTrailer getTrailer();
    std::vector<PixelHit> getHits();
    std::vector<PixelError> getErrors();
    void push_back(Word64 word);
    void clearBuffer();

  private:
    std::vector<Word64> buffer_;
    PixelSLinkHeader header_;
    PixelSLinkTrailer trailer_;
    std::vector<PixelHit> hits_;
    std::vector<PixelError> errors_;

  };

}

#endif
