#ifndef PIXEL_ERROR_H
#define PIXEL_ERROR_H

#include <string>

#include "Word32.h"

namespace pos {

  class PixelError {

  public:
	PixelError (){};
    PixelError (Word32);
    PixelError (uint32_t);
    //PixelError (unsigned int);

    Word32 getWord32();
    int getChannel();
    std::string getErrorCode();
    unsigned int getTTCrxEventNumber();
    int getErrorDetail();
    unsigned int getDataWord();
    
  private:
    Word32 error_;
    
  };

}

#endif
    
