#include "PixelError.h"

using namespace pos;

// -------------- Constructors -------------------


PixelError::PixelError (const Word32 word)
{
  error_=word;
}

PixelError::PixelError (uint32_t word)
{
  error_=Word32(word);
}

//PixelError::PixelError (unsigned int word)
//{
// error_=Word32(word);
//}

// ---------------------------------------------

// --------------- Getters ----------------------

Word32 PixelError::getWord32()
{
  return error_;
}


int PixelError::getChannel()
{
  int channel;
  std::string errorCode=getErrorCode();
  if (errorCode=="EventNumberError" || errorCode=="TrailerError") {
    channel=(int)error_.getBits(26, 31);
  } else {
    channel=-1;
  }

  return channel;
}

std::string PixelError::getErrorCode()
{
  std::string errorCode;
  int err=error_.getBits(21, 25);
  
  if (err==0x1f) {
    errorCode="EventNumber";
  } else if (err==0x1e) {
    errorCode="TrailerError";
  } else if (err==0x1d) {
    errorCode="TimeOut";
  } else if (err==0x1c) {
    errorCode="NearlyFull";
  } else if (err==0x1b) {
    errorCode="Dummy";
  } else if (err==0x1a) {
    errorCode="Gap";
  } else {
    errorCode="WTF?";
  }

  return errorCode;
}
    
unsigned int PixelError::getTTCrxEventNumber()
{
  unsigned int event=(unsigned int)error_.getBits(13, 20);
  return event;
}

int PixelError::getErrorDetail()
{
  int errDetail;
  std::string errorCode=getErrorCode();
  
  if (errorCode=="TimeOut" || errorCode=="TrailerError") {
    errDetail=(int)error_.getBits(8, 11);
  } else {
    errDetail=-1;
  }

  return errDetail;

}

unsigned int PixelError::getDataWord()
{
  unsigned int dataword=(unsigned int)error_.getBits(0, 7);
  return dataword;
}
