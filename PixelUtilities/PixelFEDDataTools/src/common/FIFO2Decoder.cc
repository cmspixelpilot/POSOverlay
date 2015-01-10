#include "PixelUtilities/PixelFEDDataTools/include/FIFO2Decoder.h"
#include <iostream>

FIFO2Decoder::FIFO2Decoder(uint32_t *buffer, unsigned int size)
{

  buffer_=buffer;
  size_=size;
  plsmsk_=0xff;
  pxlmsk_ = 0xff00;
  dclmsk_ = 0x1f0000;
  rocmsk_ = 0x3e00000;
  chnlmsk_ = 0xfc000000;

}

void FIFO2Decoder::printToStream(std::ostream& out)
{
  out<<"--- FIFO 2 Data ---"<<std::endl;
  for (unsigned int i=0;i<size_;++i) {
    out<<"Word #"<<i<<" is 0x"<<std::hex<<buffer_[i]<<std::dec;

    uint32_t chan= ((buffer_[i]&chnlmsk_)>>26);
    uint32_t roc= ((buffer_[i]&rocmsk_)>>21);

    if (roc>25) {
      if((buffer_[i]&0xffffffff)==0xffffffff) {
	out<<" fifo-2 End of Event word"<<std::endl;
      } else if (roc==26) {
	out<<"Gap word"<<std::endl;
      } else if (roc==27) {
	out<<"Dummy Data Word"<<std::endl;
      } else if(chan>0 && chan<37) {
	//cout<<hex<<word<<dec;
	out<<" Channel: "<<chan;
	out<<" ROC: "<<((buffer_[i]&rocmsk_)>>21);
	out<<" DCOL: "<<((buffer_[i]&dclmsk_)>>16);
	out<<" Pixel: "<<((buffer_[i]&pxlmsk_)>>8);
	out<<" ADC: "<<(buffer_[i]&plsmsk_)<<std::endl;
      }
    }

  }

}
