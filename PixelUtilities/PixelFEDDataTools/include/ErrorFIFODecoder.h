#ifndef _ErrorFIFODecoder_h_
#define _ErrorFIFODecoder_h_

#include <vector>
#include <stdint.h>
#include <iostream>

class ErrorFIFODecoder
{
 public:
    
  ErrorFIFODecoder(uint32_t *buffer, unsigned int size);
  
  void printToStream(std::ostream &out);
  unsigned int size();
  uint32_t word(unsigned int);
  std::string errorType(uint32_t);

 private:
  
  uint32_t *buffer_;
  unsigned int size_;

  uint32_t errCodeMask_;
  uint32_t ttcrxEventMask_;
  uint32_t dataWordMask_;
  uint32_t errDetailMask_;

};

#endif
