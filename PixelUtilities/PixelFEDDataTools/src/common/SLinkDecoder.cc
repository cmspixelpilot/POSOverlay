#include "SLinkDecoder.h"


using namespace pos;

// -------------- Constructors -----------------

SLinkDecoder::SLinkDecoder (std::istream *eventStream, std::ostream &out)
{
  if (eventStream->good()) {
    eventStream_=eventStream;
  } else {
    out<<"SLinkDecoder::SLinkDecoder - Istream is not valid!"<<std::endl;
    exit(1);
  }
}

// --------------------------------------------------------

bool SLinkDecoder::eof()
{
  return eventStream_->eof();
}

// -------------- Getters --------------------------------

bool SLinkDecoder::getNextWord64(Word64 &word, std::ostream &out)
{
  eventStream_->read((char*)word.getWordPointer(), 8);
  if (eventStream_->good()) {
    return true;
  } 
	else {
    out<<"SLinkDecoder::getNextWord64 - Stream ended trying reading next 64 bit word."<<std::endl;
	  return false;
  }
}

bool SLinkDecoder::getNextEvent(PixelSLinkEvent &sLinkEvent, std::ostream &out)
{
  
  bool goodRead=false;
  sLinkEvent.clearBuffer();
  Word64 word;

  //
  // First search for the next SLink Header in the next maxHeaderTries 64 bit words
  //
  unsigned int tries=1, maxHeaderTries=100;
  while (!goodRead && tries<maxHeaderTries) {
    if (getNextWord64(word)) {
      if (PixelSLinkHeader(word).isHeader()) {      
	      goodRead=true;
	      sLinkEvent.push_back(word);
      } else {
	      goodRead=false;
	      out<<"SLinkDecoder::getNextEvent - Junk word 0x"<<std::hex<<word.getWord()<<std::dec<<" caught looking for SLink Header!"<<std::endl;
      }
    } else {
      out<<"SLinkDecoder::getNextEvent - End of event stream reached."<<std::endl;
      goodRead=false;
      return goodRead;
    }
    ++tries;
  }
  if (goodRead==false) {
    out<<"SLinkDecoder::getNextEvent - New SLink Header not found in the next "<<maxHeaderTries<<" 64 bit words!"
       <<"Aborting parsing of stream"<<std::endl;
//    sLinkEvent=PixelSLinkEvent(eventContent);
//    sLinkEvent.decodeEvent();
    return goodRead;
  }

  //
  // Now keep taking in hits till an SLink Trailer is encountered
  //
  do {
    if (getNextWord64(word)) {
      goodRead=true;
      sLinkEvent.push_back(word);
    } else {
      goodRead=false;
      out<<"SLinkDecoder::getNextEvent -- Event stream ended while taking in hits before an SLinkTrailer was encountered!"<<std::endl;
    }

   
  } while (goodRead && !PixelSLinkTrailer(word).isTrailer());
  
  return goodRead;

}

// ----------------------------------------------------
