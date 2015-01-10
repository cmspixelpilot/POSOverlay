#ifndef WORD32_H
#define WORD32_H

#include <stdint.h>

namespace pos {

  class Word32 {
  public:
    Word32(uint32_t=0);
    //Word32(unsigned int);
    //void operator=(Word32);

    uint32_t getWord();
    uint32_t getBits(unsigned int, unsigned int); // Start from bit 0
    void getBitsFast(unsigned int, unsigned int,uint32_t &); // Start from bit 0
    uint32_t getBitsFast(unsigned int, unsigned int); // Start from bit 0

  private:
    void initMask(void);
    uint32_t word_;
    static uint32_t mask_[32];
  };

}

#endif
