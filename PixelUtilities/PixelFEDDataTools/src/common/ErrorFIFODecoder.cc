#include "PixelUtilities/PixelFEDDataTools/include/ErrorFIFODecoder.h"
#include <iostream>
#include <assert.h>

ErrorFIFODecoder::ErrorFIFODecoder(uint32_t *buffer, unsigned int size)
{

  buffer_=buffer;
  size_=size;
  
  errCodeMask_=   0x03e00000;
  ttcrxEventMask_=0x001fe000;
  dataWordMask_=  0x000000ff;
  errDetailMask_= 0x00000f00;

}

void ErrorFIFODecoder::printToStream (std::ostream& out)
{

  out<<"--- Error FIFO Data ---"<<std::endl;
  for (unsigned int i=0;i<size_;++i) {
    uint32_t word=buffer_[i];
    out<<"Word #"<<i<<" is 0x"<<std::hex<<word<<std::dec;

    int errCode=(word & errCodeMask_)>>21;
    int ttcrxEvent=(word & ttcrxEventMask_)>>13;
    int dataWord=(word & dataWordMask_);
      
    switch (errCode) {
    
       case 0x1f: out<<" : Event Number Error. TTCrx Event #"<<ttcrxEvent
		     <<", Channel Event #"<<dataWord<<std::endl; break;
       case 0x1e: {
      	            out<<" : Trailer Error. TTCrx Event #"<<ttcrxEvent
		       <<", TBM Trailer="<<dataWord;
		    int errorDetail=(word & errDetailMask_)>>8;
		    if ((errorDetail & 0x8)>>3) out<<", Invalid number of ROCs";
		    if ((errorDetail & 0x4)>>2) out<<", FSM Error 1";
		    if ((errorDetail & 0x2)>>1) out<<", FSM Error 2";
		    if (errorDetail & 0x1) out<<", Data stream too long";
		    out<<std::endl;
                  }
	          break;
    case 0x1d: out<<" : Time Out Error. TTCrx Event #"<<ttcrxEvent<<", Error on Channel "<<dataWord<<std::endl; break;
    case 0x1c: out<<" : Nearly Full. TTCrx Event #"<<ttcrxEvent<<", Nearly Full on Channel "<<dataWord<<std::endl; break;
    case 0x1b: out<<" : Dummy"<<std::endl; break;
    case 0x1a: out<<" : Gap"<<std::endl; break;
    }
    
  }
  out<<"---------------------"<<std::endl;

}

unsigned int ErrorFIFODecoder::size()
{
  return size_;
}

uint32_t ErrorFIFODecoder::word(unsigned int i)
{
  if (i>size_) assert(0);
  return buffer_[i];
}

std::string ErrorFIFODecoder::errorType(uint32_t word)
{
  int errCode=(word & errCodeMask_)>>21;
  std::string errString;

  if (errCode==0x1f) errString="EventNumber"; 
  else if (errCode==0x1e) errString="Trailer";
  else if (errCode==0x1d) errString="TimeOut";
  else if (errCode==0x1c) errString="NearlyFull";
  else if (errCode==0x1b) errString="Dummy";
  else if (errCode==0x1a) errString="Gap";
  else errString="WTF?";

  return errString;
}
