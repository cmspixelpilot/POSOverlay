#include "PixelSLinkHeader.h"

using namespace pos;

// ------------ Constructors -----------------

PixelSLinkHeader::PixelSLinkHeader(Word64 word)
{
  if (word.getBits(60, 63)==0x5) {
    header_=word;
  } else {
    header_=Word64(0);
  }
}

PixelSLinkHeader::PixelSLinkHeader(uint64_t word)
{
  if (Word64(word).getBits(60, 63)==0x5) {
    header_=Word64(word);
  } else {
    header_=Word64(0);
  }
}

// -------------------------------------------

// -------------- Getters --------------------

bool PixelSLinkHeader::isHeader()
{
  if (header_.getWord()==0) {
    return false;
  } else {
    return true;
  }
}

Word64 PixelSLinkHeader::getWord64()
{
  return header_;
}

unsigned int PixelSLinkHeader::getBOE()
{
  unsigned int boe=(unsigned int)header_.getBits(60, 63);
  return boe;
}

unsigned int PixelSLinkHeader::getEvt_ty()
{
  unsigned int evt_ty=(unsigned int)header_.getBits(56, 59);
  return evt_ty;
}

unsigned int PixelSLinkHeader::getLV1_id()
{
  unsigned int lv1_id=(unsigned int)header_.getBits(32, 55);
  return lv1_id;
}

unsigned int PixelSLinkHeader::getBX_id()
{
  unsigned int bx_id=(unsigned int)header_.getBits(20, 31);
  return bx_id;
}

unsigned int PixelSLinkHeader::getSource_id()
{
  unsigned int source_id=(unsigned int)header_.getBits(8, 19);
  return source_id;
}

unsigned int PixelSLinkHeader::getFOV()
{
  unsigned int fov=(unsigned int)header_.getBits(4, 7);
  return fov;
}

unsigned int PixelSLinkHeader::getH()
{
  unsigned int h=(unsigned int)header_.getBits(3, 3);
  return h;
}

unsigned int PixelSLinkHeader::getSLinkHardwareBits()
{
  unsigned int hardware=(unsigned int)header_.getBits(0, 1);
  return hardware;
}
