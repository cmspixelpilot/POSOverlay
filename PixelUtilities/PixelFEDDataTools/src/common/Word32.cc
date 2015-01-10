#include <cmath>
#include "Word32.h"
#include <iostream>

using namespace pos;
uint32_t Word32::mask_[32]={
	0x1,
	0x3,
	0x7,
	0xf,
	0x1f,
	0x3f,
	0x7f,
	0xff,
	0x1ff,
	0x3ff,
	0x7ff,
	0xfff,
	0x1fff,
	0x3fff,
	0x7fff,
	0xffff,
	0x1ffff,
	0x3ffff,
	0x7ffff,
	0xfffff,
	0x1fffff,
	0x3fffff,
	0x7fffff,
	0xffffff,
	0x1ffffff,
	0x3ffffff,
	0x7ffffff,
	0xfffffff,
	0x1fffffff,
	0x3fffffff,
	0x7fffffff,
	0xffffffff
};
// ---------Constructors --------------//

Word32::Word32(uint32_t word){
  word_=word;
}

//Word32::Word32(unsigned int word){
//word_=(uint32_t)word;
//}
//--------------------------------------


//---------- Getters ------------------//

uint32_t Word32::getWord(){
  return word_;
}

uint32_t Word32::getBits(unsigned int lowbit, unsigned int highbit){
  if (highbit<lowbit) {return 0;}
  if (highbit>31 || lowbit>31) {return 0;}

  uint32_t mask=((uint32_t)(pow(2,(highbit-lowbit+1))-1))<<lowbit;

  return (word_ & mask)>>lowbit;
}

void Word32::getBitsFast(unsigned int lowbit, unsigned int highbit,uint32_t &returnword){ // Start from bit 0
  if (highbit<lowbit) {returnword=0; return;}
  if (highbit>31 || lowbit>31) {returnword=0; return;}

  returnword=(word_ >>lowbit) & mask_[highbit-lowbit];
}

uint32_t Word32::getBitsFast(unsigned int lowbit, unsigned int highbit){ // Start from bit 0
  if (highbit<lowbit) {return 0;}
  if (highbit>31 || lowbit>31) {return 0;}

  return (word_ >>lowbit) & mask_[highbit-lowbit];
}
//-----------------------------------//
