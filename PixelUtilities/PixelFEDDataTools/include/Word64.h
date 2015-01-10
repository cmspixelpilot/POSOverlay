#ifndef WORD64_H
#define WORD64_H

#include <stdint.h>

#include "Word32.h"

namespace pos {

  class Word64 {
  public:
    Word64(uint64_t=0);
    //Word64(unsigned int lowword, unsigned int highword);
    Word64(uint32_t lowword, uint32_t highword);
    Word64(Word32 lowword, Word32 highword);

    uint64_t getWord();
    uint64_t *getWordPointer();
    uint32_t getLowWord();
    uint32_t getHighWord();
    Word32 getLowWord32();
    Word32 getHighWord32();
    uint64_t getBits(unsigned int lowbit, unsigned int highbit);

  private:
    uint64_t word_;
  };

}

#endif
    
