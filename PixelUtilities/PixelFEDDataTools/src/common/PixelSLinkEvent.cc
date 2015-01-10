#include "PixelSLinkEvent.h"


using namespace pos;

// -------------- Constructors -----------------

PixelSLinkEvent::PixelSLinkEvent(std::vector<Word64> buffer)
{
  buffer_=buffer;
}

PixelSLinkEvent::PixelSLinkEvent (uint64_t* buffer, unsigned int length)
{
  for (unsigned int i=0; i<length; ++i) {
    buffer_.push_back(Word64(buffer[i]));
  }

}

// --------------------------------------------------------

void PixelSLinkEvent::push_back(Word64 word){
  buffer_.push_back(word);
}

void PixelSLinkEvent::clearBuffer(){
  buffer_.clear();
}

bool PixelSLinkEvent::decodeEvent(){
  bool decoded=true;
  
  unsigned int sizeBuffer=buffer_.size();
  if(sizeBuffer == 0){
    std::cout << "[PixelSLinkEvent::decodeEvent()]\tBufferSize=0" << std::endl;
    return false;
  }
  //std::cout << "sizeBuffer:"<<sizeBuffer<<std::endl;
 
  header_=PixelSLinkHeader(buffer_.at(0));
  if (header_.getWord64().getWord()==0) {
    std::cout<<"PixelSLinkEvent::decodeEvent() - First 64 bit word 0x"<<std::hex<<buffer_.at(0).getWord()<<std::dec<<" is not an SLink Header!"<<std::endl;
    decoded=false;
  }
  hits_.clear();
  errors_.clear();
  for (unsigned int i=1; i<sizeBuffer-1;++i) {

    unsigned int rocid;
    Word32 wordlow=buffer_.at(i).getLowWord();
    rocid=wordlow.getBitsFast(21, 25);
		if (rocid >= 0x1a) {
      errors_.push_back(PixelError(wordlow));
    } else if (rocid>0 && rocid <= 24) {
      hits_.push_back(PixelHit(wordlow));
    } else {
      std::cout<<"PixelSLinkEvent::decodeEvent() - low word of 0x"<<std::hex<<buffer_.at(i).getWord()<<std::dec
	       <<" has ROC ID = "<<rocid
	       <<" which is ROC ID ==0 || (ROC ID > 24 && ROC ID < 0x1a) and is invalid!"<<std::endl;
      decoded=false;
    }
    
    Word32 wordhigh=buffer_.at(i).getHighWord();
    rocid=wordhigh.getBitsFast(21, 25);
    if (rocid >= 0x1a) {
      errors_.push_back(PixelError(wordhigh));
    } else if (rocid>0 && rocid <= 24) {
      hits_.push_back(PixelHit(wordhigh));
    } else {
      std::cout<<"PixelSLinkEvent::decodeEvent() - low word of 0x"<<std::hex<<buffer_.at(i).getWord()<<std::dec
	       <<" has ROC ID = "<<rocid
	       <<" which is ROC ID ==0 || (ROC ID > 24 && ROC ID < 0x1a) and is invalid!"<<std::endl;
      decoded=false;
    }

  }

  trailer_=PixelSLinkTrailer(buffer_.at(sizeBuffer-1));
  if (trailer_.getWord64().getWord()==0) {
    std::cout<<"PixelSLinkEvent::decodeEvent() - Last 64 bit word 0x"<<std::hex<<buffer_.at(sizeBuffer-1).getWord()<<std::dec<<" is not an SLink Trailer!"<<std::endl;
    decoded=false;
  }

  return decoded;

}

// -------------- Getters --------------------------------

PixelSLinkHeader PixelSLinkEvent::getHeader()
{
  return header_;
}

PixelSLinkTrailer PixelSLinkEvent::getTrailer()
{
  return trailer_;
}

std::vector<PixelHit> PixelSLinkEvent::getHits()
{
  return hits_;
}

std::vector<PixelError> PixelSLinkEvent::getErrors()
{
  return errors_;
}

// ----------------------------------------------------
