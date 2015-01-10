#include <cmath>
#include "Word64.h"

using namespace pos;

//----------- Constructors ---------------------

Word64::Word64(uint64_t word)
{
  word_=(uint64_t)word;
}

//Word64::Word64(unsigned int wordlow, unsigned int wordhigh)
//{
//word_=((uint64_t)wordhigh<<32)||(uint64_t)wordlow;
//}

Word64::Word64(uint32_t wordlow, uint32_t wordhigh)
{
  word_=((uint64_t)wordhigh<<32)||(uint64_t)wordlow;
}

Word64::Word64(Word32 wordlow, Word32 wordhigh)
{
  word_=((uint64_t)wordhigh.getWord()<<32)||((uint64_t)wordlow.getWord());
}

//----------------------------------------------

//----------- Getters ---------------------------

uint64_t Word64::getWord()
{
  return word_;
}

uint64_t *Word64::getWordPointer(){
  return &word_;
}

uint32_t Word64::getLowWord()
{
  uint32_t word=0xffffffff & word_;
  return word;
}

uint32_t Word64::getHighWord()
{
  uint32_t word=(0xffffffff00000000LL & word_)>>32;
  return word;
}

Word32 Word64::getLowWord32()
{
  Word32 word(this->getLowWord());
  return word;
}

Word32 Word64::getHighWord32()
{
  Word32 word(this->getHighWord());
  return word;
}

uint64_t Word64::getBits(unsigned int lowbit, unsigned int highbit)
{

  if (highbit<lowbit) {return 0;}
  if (highbit>63 || lowbit>63) {return 0;}

  uint64_t mask=((uint64_t)(pow(2,(highbit-lowbit+1))-1))<<lowbit;
  
  uint64_t returnword=(word_ & mask)>>lowbit;

  return returnword;

}

//--------------------------------------------------
