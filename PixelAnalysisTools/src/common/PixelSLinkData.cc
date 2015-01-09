// PixelSLinkData.cc

#include "PixelSLinkData.h"

#include <iostream>
#include <iomanip>

using namespace pixel;

// Definitions for pixel::Word32 //

Word32::Word32(word32 value)
{
  m_value = value;
}
    
Word32::Word32(int value)
{
  m_value = value;
}

bool Word32::is_filler()
{
  if (m_value==0x00000001) return true;
  if (m_value==0xffffffff) return true;
  return false;
}

bool Word32::is_error()
{
  if (this->is_filler()) return false;
  else {
    int rocID=this->getBits(21,5);
    if (rocID>25 && rocID<32) return true;
    else return false;
  }
}
    
int Word32::getBit(int bit)
{    
  word32 mask = 1 << bit;
  return m_value & mask;
}
    
void Word32::setBit(int bit, int value)
{
  word32 mask = 1 << bit;
  if (value)
    m_value |= mask;
  else
    m_value ^= mask;
}
    
int Word32::getBits(int low_bit_position, int nbits)
{
  word32 mask = 0;
  while (nbits--)
    {
      mask = (mask<<1) + 1;
    }
  mask <<= low_bit_position;
  return (m_value & mask) >> low_bit_position;
}
    
void Word32::setBits(int low_bit_position, int nbits, int value)
{
  word32 mask = 0;
  while (nbits--)
    mask = mask<<1 + 1;
  mask <<= low_bit_position;
  mask = ~mask;
  mask += (value << low_bit_position);
  m_value &= mask;    
}

////////////////////////////////////

// Definitions for pixel::Word64 //

Word64::Word64(word64 value)
{
  m_value = value;
}
    
Word64::Word64(word32 _up, word32 _down)
{
  up() = _up;
  down() = _down;
}

bool Word64::read(std::istream &in)
{
  in.read((char*)&m_value,8);
  return in.good();
}

bool Word64::is_header()
{
  return ((m_value >> 60) == 0x5);
}

bool Word64::is_trailer()
{
  return ((m_value >> 60) == 0xa);
}

////////////////////////////////////

// Definitions for pixel::SLinkHeader //

SLinkHeader::SLinkHeader(word64 header)
{
  m_header = header;
}

SLinkHeader::SLinkHeader(word32 up, word32 down)
{
  m_header = Word64(up, down);
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
const SLinkHeader& SLinkHeader::assign_from(const Word64 &header)
{
  m_header = header;
  return *this;
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Word64& SLinkHeader::cast_to_Word64()
{
  return m_header;
}

std::ostream& SLinkHeader::print(std::ostream &out)
{
  return out << "SLinkHeader" << this;
}

////////////////////////////////////

// Definitions for pixel::SLinkTrailer //

SLinkTrailer::SLinkTrailer(word64 trailer)
{
  m_trailer = trailer;
}

SLinkTrailer::SLinkTrailer(word32 up, word32 down)
{
  m_trailer = Word64(up,down);
}

const SLinkTrailer& SLinkTrailer::assign_from(const Word64 &trailer)
{
  m_trailer = trailer;
  return *this;
}

Word64& SLinkTrailer::cast_to_Word64()
{
  return m_trailer;
}

std::ostream& SLinkTrailer::print(std::ostream& out)
{
    return out << "SLinkTrailer: " << this;
}

////////////////////////////////////

// Definitions for pixel::SLinkHit //

SLinkHit::SLinkHit()
{

}

SLinkHit::SLinkHit(Word32 hit)
{
    m_hit = hit;
}

std::ostream& SLinkHit::print(std::ostream& out)
{
    using namespace std;
    out << "link id=" << get_link_id() << " ";
    out << "roc id=" << get_roc_id() << " ";
    out << "dcol id=" << get_dcol_id() << " ";
    out << "pix id=" << get_pix_id() << " ";
    out << "adc=" << get_adc();
    return out;
}

const SLinkHit& SLinkHit::assign_from(Word32 hit)
{
    m_hit = hit;
    return *this;
}

Word32 SLinkHit::cast_to_Word32()
{
    return m_hit;
}

///////////////////////////////////

// Definitions of pixel::SLinkError

SLinkError::SLinkError ()
{

}

SLinkError::SLinkError(Word32 errorWord)
{
    error_=errorWord;
}

std::ostream& SLinkError::print(std::ostream &out)
{
    //unsigned int errorCode=((error_ & 0x03e00000)>>21);
    int errorCode=error_.getBits(21, 5);
    int ttcrxEvent=error_.getBits(13, 7);
    int dataWord=error_.getBits(0, 8);

    switch (errorCode) {
	case 0x1f: out<<"Event Number Error. TTCrx Event #"<<ttcrxEvent
		      <<", Channel Event #"<<dataWord<<std::endl; break;
	case 0x1e: {
		     out<<"Trailer Error. TTCrx Event #"<<ttcrxEvent
		        <<", TBM Trailer="<<dataWord;
		     int errorDetail=error_.getBits(8, 4);
		     if (errorDetail & 0x8) out<<", Invalid number of ROCs"<<std::endl;
		     if (errorDetail & 0x4) out<<", FSM Error 1"<<std::endl;
		     if (errorDetail & 0x2) out<<", FSM Error 2"<<std::endl;
		     if (errorDetail & 0x1) out<<", Data stream too long"<<std::endl;
		   }
		   break;
	case 0x1d: out<<"Time Out Error. TTCrx Event #"<<ttcrxEvent<<", Error on Channel "<<dataWord<<std::endl; break;
	case 0x1c: out<<"Nearly Full. TTCrx Event #"<<ttcrxEvent<<", Nearly Full on Channel "<<dataWord<<std::endl; break;
	case 0x1b: out<<"Dummy"<<std::endl; break;
	case 0x1a: out<<"Gap"<<std::endl; break;
    }

    return out;
}

const SLinkError& SLinkError::assign_from(Word32 error)
{
    error_ = error;
    return *this;
}

Word32 SLinkError::cast_to_Word32()
{
    return error_;
}

////////////////////////////////////

// Definitions for pixel::SLinkData //


std::ostream& SLinkData::print(std::ostream &out)
{
    out << "SLinkData:" << std::endl;
    //out << getHeader() << std::endl;
    //out << getTrailer() << std::endl;
    return out;
}

SLinkHeader SLinkData::getHeader()
{
    return m_header;
}

SLinkTrailer SLinkData::getTrailer()
{
    return m_trailer;
}

bool SLinkData::load(std::istream & in)
{
    Word64 data;
    hits_.clear();
    errors_.clear();
    
    if (!data.read(in))
	return false;

    if (data.is_header())
	m_header.assign_from(data);
    else 
	return false;

    while (true) {
	if (!data.read(in))
	    return false;
	if (data.is_trailer()) {
	    m_trailer.assign_from(data);
            //std::cout << "Found trailer"<<std::endl;
	    return true;
	} else {
	    std::vector<Word32> word(2);
	    word.at(0)=data.up();
	    word.at(1)=data.down();

	    for (unsigned int i=0;i<=1;++i) {
		Word32 thisword=word.at(i);
	    	if (thisword.is_filler()) {
		} else if (thisword.is_error()) {
			SLinkError error(thisword);
			errors_.push_back(error);
		} else {				// It must be a hit
 			SLinkHit hit(thisword);
			hits_.push_back(hit);
	    	}
	    }
	}
    }
}

///////////////////////////////////

/*

std::ostream& operator<<(std::ostream &out, SLinkData &slink)
{
    return slink.print(out);
}

std::ostream& operator<<(std::ostream &out, SLinkHeader &header)
{
    return header.print(out);
}

//std::ostream& operator<<(std::ostream &out, SLinkTrailer &trailer)
{
    return trailer.print(out);
}

std::ostream& operator<<(std::ostream &out, SLinkHit &hit)
{
    return hit.print(out);
}

std::ostream& operator<<(std::ostream &out, Word32 word)
{
    return out << word.m_value;
}

std::ostream& operator<<(std::ostream &out, Word64 &dword)
{
    return out << dword.up() << " " << dword.down() ;
}

*/


