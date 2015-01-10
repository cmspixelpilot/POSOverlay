#ifndef PIXEL_SLINK_DECODER_H
#define PIXEL_SLINK_DECODER_H

#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

#include "PixelSLinkEvent.h"

namespace pos {

  class SLinkDecoder {

  public:
    SLinkDecoder(std::istream *eventStream, std::ostream &out = std::cout);
    bool eof();
    bool getNextWord64(Word64 &word, std::ostream &out = std::cout);
    bool getNextEvent(PixelSLinkEvent &sLinkEvent, std::ostream &out = std::cout);

  private:
    std::istream *eventStream_;

  };

}

#endif
