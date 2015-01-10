
#ifndef _FIFO3Decoder_h_
#define _FIFO3Decoder_h_

#include <vector>
#include <stdint.h>


class FIFO3Decoder
{
 public:
    
    FIFO3Decoder(uint64_t *buffer);

    unsigned int nhits() const {return hits_.size();}

    unsigned int channel(unsigned int ihit) const {return (hits_[ihit]>>26)&0x3f;}
    unsigned int rocid(unsigned int ihit) const {return (hits_[ihit]>>21)&0x1f;}
    
    unsigned int dcol(unsigned int ihit) const {return (hits_[ihit]>>16)&0x1f;}
    unsigned int pxl(unsigned int ihit) const {return (hits_[ihit]>>8)&0xff;}
    unsigned int pulseheight(unsigned int ihit) const {return (hits_[ihit]>>0)&0xff;}
    
    unsigned int column(unsigned int ihit) const {return dcol(ihit)*2 + pxl(ihit)%2;}
    unsigned int row(unsigned int ihit) const {return 80 - (pxl(ihit)/2);}


 private:
    std::vector<unsigned int> hits_;

};

#endif
