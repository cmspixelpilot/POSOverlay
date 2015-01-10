#include "PixelHit.h"

using namespace pos;

// ------------- Constructors ----------------

PixelHit::PixelHit (){
	hit_    =0;
  w32Hit_ =Word32(hit_);
}

PixelHit::PixelHit (Word32 word){
  w32Hit_ =word;
	hit_    =word.getWord();
}

PixelHit::PixelHit (uint32_t word){
  w32Hit_ =Word32(word);
	hit_    =word;
}


//PixelHit::PixelHit (unsigned int word){
//w32Hit_ =Word32(word);
//hit_    =word;
//}

// ------------------------------------------

// -------------- Getters -------------------

Word32 PixelHit::getWord32(){
  return w32Hit_;
}

unsigned int PixelHit::getLink_id(){
//  unsigned int link_id=(unsigned int)w32Hit_.getBitsFast(26, 31);
//  return link_id;
  return ((hit_>>26)&0x3f);
}

unsigned int PixelHit::getROC_id(){
//  unsigned int roc_id=(unsigned int)w32Hit_.getBitsFast(21, 25);
//  return roc_id;
//  unsigned int roc_id=(unsigned int)w32Hit_.getBitsFast(21, 25);
  return ((hit_>>21)&0x1f);
}

unsigned int PixelHit::getDCol_id(){
//  unsigned int dcol_id=(unsigned int)w32Hit_.getBitsFast(16, 20);
//  return dcol_id;
  return ((hit_>>16)&0x1f);
}

unsigned int PixelHit::getPix_id(){
//  unsigned int pix_id=(unsigned int)w32Hit_.getBitsFast(8, 15);
//  return pix_id;
  return ((hit_>>8)&0xff);
}

unsigned int PixelHit::getADC(){
//  unsigned int adc=(unsigned int)w32Hit_.getBitsFast(0, 7);
//  return adc;
  return (hit_&0xff);
}

unsigned int PixelHit::getRow(){
  return (80-getPix_id()/2);
}

unsigned int PixelHit::getColumn(){
  return (getDCol_id()*2+(getPix_id()%2));
}
