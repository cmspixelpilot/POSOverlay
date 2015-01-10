#include "PixelSLinkTrailer.h"

using namespace pos;

// ----------- Constructors -----------------

PixelSLinkTrailer::PixelSLinkTrailer(Word64 word)
{
  if (word.getBits(60, 63)==0xa) {
    trailer_=word;
  } else {
    trailer_=Word64(0);
  }
}

PixelSLinkTrailer::PixelSLinkTrailer(uint64_t word)
{
  if (Word64(word).getBits(60, 63)==0xa) {
    trailer_=Word64(word);
  } else {
    trailer_=Word64(0);
  }
}

// ------------------------------------------

// ------------ Getters ---------------------

bool PixelSLinkTrailer::isTrailer()
{
  if (trailer_.getWord()==0) {
    return false;
  } else {
    return true;
  }
}

Word64 PixelSLinkTrailer::getWord64()
{
  return trailer_;
}

unsigned int PixelSLinkTrailer::getEOE()
{
  unsigned int eoe_1=(unsigned int)trailer_.getBits(60, 63);
  return eoe_1;
}

unsigned int PixelSLinkTrailer::getEvt_lgth()
{
  unsigned int evt_lgth=(unsigned int)trailer_.getBits(32, 55);
  return evt_lgth;
}

unsigned int PixelSLinkTrailer::getCRC()
{
  unsigned int crc=(unsigned int)trailer_.getBits(16, 31);
  return crc;
}

unsigned int PixelSLinkTrailer::getEVT_stat()
{
  unsigned int evt_stat=(unsigned int)trailer_.getBits(8, 11);
  return evt_stat;
}

unsigned int PixelSLinkTrailer::getTTS()
{
  unsigned int tts=(unsigned int)trailer_.getBits(4, 7);
  return tts;
}

unsigned int PixelSLinkTrailer::getT()
{
  unsigned int t=(unsigned int)trailer_.getBits(3, 3);
  return t;
}

unsigned int PixelSLinkTrailer::getSLinkHardwareBits()
{
  unsigned int hardware=(unsigned int)trailer_.getBits(0, 1);
  return hardware;
}
