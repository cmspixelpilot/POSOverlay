#ifndef _FIFO2Decoder_h_
#define _FIFO2Decoder_h_

#include <vector>
#include <stdint.h>
#include <iostream>

class FIFO2Decoder
{
 public:
    
    FIFO2Decoder(uint32_t *buffer, unsigned int size);

    void printToStream(std::ostream &out);

    //unsigned int nhits(){return hits_.size();}

    //unsigned int channel(int ihit) {return (hits_[ihit]>>26)&0x3f;}
    //unsigned int rocid(int ihit) {return (hits_[ihit]>>21)&0x1f;}

 private:
    std::vector<unsigned int> hits_;
    uint32_t *buffer_;
    unsigned int size_;
    uint32_t plsmsk_;
    uint32_t pxlmsk_;
    uint32_t dclmsk_;
    uint32_t rocmsk_;
    uint32_t chnlmsk_;

};

#endif
