#include "PixelUtilities/PixelFEDDataTools/include/FIFO3Decoder.h"
#include <iostream>
#include <assert.h>

bool FIFO3Decoder::is_trailer_word(uint64_t word) const {
  return (word >> 46) == 0x28000; // utca channel 40 rocs 0-7 can reproduce the old 0xa0
}

FIFO3Decoder::FIFO3Decoder(uint64_t *buffer)
{

  unsigned int *buf=(unsigned int *)buffer;

  //std::cout << "buf[0]=0x"<<std::hex<<buf[0]<<std::dec<<std::endl;

  assert((buffer[0]>>60)==0x5);
  
  unsigned int counter=1;

  while (!is_trailer_word(buffer[counter])) {

    for(unsigned int index=counter*2;index<counter*2+2;index++){


      //unsigned int channel=(buf[index]>>26)&0x3f;
      unsigned int rocid=(buf[index]>>21)&0x1f;
    

      if (rocid<25) hits_.push_back(buf[index]);
      if (rocid>27&&(rocid!=30)) {
        //FIXME Code should set some error FLAG but can not print message here
        //std::cout << "buf="<<std::hex<<buf[index]<<std::dec<<" channel="<<channel<<" rocid="<<rocid<<std::endl;
      }

    }

    counter++;

    //FIXME what should this check actually be???

    if (counter>7000) {
      std::cout << "counter="<<counter<<" will exit loop!"<<std::endl;
      break;
    }
    
  }  

}

