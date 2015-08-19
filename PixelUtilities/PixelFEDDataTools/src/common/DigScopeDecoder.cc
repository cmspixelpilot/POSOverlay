#include "PixelUtilities/PixelFEDDataTools/include/DigScopeDecoder.h"
#include <cassert>
#include <cstdio>
#include <iomanip>

///////////////////////////////////////////////////////////////////////////
// Decode the scope data in  transparent mode from piggy
// ADD SIZE
int decodePTrans(unsigned * data1, unsigned * data2, const int length) {
unsigned long mydat[16]=
{0x80,0x90,0xa0,0xb0,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0xc0,0xd0,0xe0,0xf0};

if(length<16) return -2;
  // Print & analyze the data buffers
int tempcode1=0;
int tempcode2=0;
int tempcode3=0;

  for(int icx=0;icx<16;icx++) {
if( ((data1[icx]&0xf0)==0x80) && ((data2[icx]&0xf0)==0x80) ) {if((data1[icx]!=data2[icx])) tempcode3=4;}
if( ((data1[icx]&0xf0)==0x90) && ((data2[icx]&0xf0)==0x90) ) {if((data1[icx]!=data2[icx])) tempcode3=4;}



if((data1[icx]&0xf0)!=(data2[icx]&0xf0))tempcode1=1;
if( ((data1[icx]&0xf0)!=mydat[icx])|((data2[icx]&0xf0)!=mydat[icx]))tempcode2=2;

  }





  //if((tempcode1)!=0)std::cout<<"Buffers 0-15 dont match each other!"<<std::endl;
  //if((tempcode2)!=0)std::cout<<"Buffers 0-15 dont match expected pattern!"<<std::endl;
  //if((tempcode3)!=0)std::cout<<"Buffers 0-15 dont match event numbers!"<<std::endl;

return (tempcode1+tempcode2+tempcode3);


} // end

///////////////////////////////////////////////////////////////////////////
//// Decode the scope data in  transparent mode from piggy
//// ADD SIZE
int decodePTrans2(unsigned  * data1, unsigned * data2, const int length) {
  //unsigned long mydat[16]=
  //{0x80,0x90,0xa0,0xb0,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0xc0,0xd0,0xe0,0xf0};

if(length<16) return -2;
  // Print & analyze the data buffers
  int tempcode1=0;//trailer
  int tempcode2=0;//rocs
  int tempcode3=0;//event
 int tempcode4=0;//header
 int tempcode5=0;
 //int tempcode6=0;
//header event number check
//
int mytr1=0;
int mytr2=0;

    for(int icx=0;icx<8;icx++) {
    if( ((data1[icx]&0xf0)==0x80) && ((data2[icx]&0xf0)==0x80) ) {if((data1[icx]!=data2[icx])) tempcode3=4;}
    if( ((data1[icx]&0xf0)==0x90) && ((data2[icx]&0xf0)==0x90) ) {if((data1[icx]!=data2[icx])) tempcode3=4;}

    //if( ((data1[icx]&0xf0)==0xa0) && ((data2[icx]&0xf0)==0xa0) ) {if((data1[icx]!=data2[icx])) tempcode3=4;}
   // if( ((data1[icx]&0xf0)==0xb0) && ((data2[icx]&0xf0)==0xb0) ) {if((data1[icx]!=data2[icx])) tempcode3=4;}

    if((data1[icx]&0xf0)==0x80)mytr1++;
    if((data1[icx]&0xf0)==0x90)mytr1++;
    if((data1[icx]&0xf0)==0xa0)mytr1++;
    if((data1[icx]&0xf0)==0xb0)mytr1++;
    if((data2[icx]&0xf0)==0x80)mytr2++;
    if((data2[icx]&0xf0)==0x90)mytr2++;
    if((data2[icx]&0xf0)==0xa0)mytr2++;
    if((data2[icx]&0xf0)==0xb0)mytr2++;


      }
if((mytr1!=4)|(mytr2!=4))tempcode4=8;
//Trailer check
mytr1=0;
mytr2=0;
    for(int icx=0;icx<128;icx++) {
    if((data1[icx]&0xf0)==0xc0)mytr1++;
    if((data1[icx]&0xf0)==0xd0)mytr1++;
    if((data1[icx]&0xf0)==0xe0)mytr1++;
    if((data1[icx]&0xf0)==0xf0)mytr1++;
    if((data2[icx]&0xf0)==0xc0)mytr2++;
    if((data2[icx]&0xf0)==0xd0)mytr2++;
    if((data2[icx]&0xf0)==0xe0)mytr2++;
    if((data2[icx]&0xf0)==0xf0)mytr2++; 

      }
if((mytr1!=4)|(mytr2!=4))tempcode1=1;

//Rocs check
mytr1=0;
mytr2=0;
    for(int icx=0;icx<128;icx++) {
    if((data1[icx]&0xf0)==0x70)mytr1++;
    if((data2[icx]&0xf0)==0x70)mytr2++;

      }
if((mytr1!=8)||(mytr2!=8))tempcode2=2;
//hits check
mytr1=0;
mytr2=0;

    for(int icx=0;icx<128;icx++) {
    if((data1[icx]&0xff)==0x10)mytr1++;
    if((data2[icx]&0xff)==0x10)mytr2++;
    if((data1[icx]&0xff)==0x2a)mytr1++;
    if((data2[icx]&0xff)==0x2a)mytr2++;
    if((data1[icx]&0xff)==0x31)mytr1++;
    if((data2[icx]&0xff)==0x31)mytr2++;
    if((data1[icx]&0xfe)==0x44)mytr1++;
    if((data2[icx]&0xfe)==0x44)mytr2++;

      }

if((mytr1<7*4)||(mytr2<6*4))tempcode5=16;



      if((tempcode1)!=0)std::cout<<"missed trailer"<<std::endl;
      if((tempcode2)!=0)std::cout<<"missed roc"<<std::endl;
      if((tempcode3)!=0)std::cout<<"event number mismatch"<<std::endl;
      if((tempcode4)!=0)std::cout<<"missed header"<<std::endl;
      if((tempcode5)!=0)std::cout<<"missed hits"<<std::endl;


      return (tempcode1+tempcode2+tempcode3+tempcode4+tempcode5);


      } // end

///////////////////////////////////////////////////////////////////////////
// Decode the scope data in  transparent mode from piggy
// ADD SIZE
// Checks for 1 hit per ROC from both chs
void decodePTrans3(unsigned * data1, unsigned * data2, const int length) {
  //  unsigned long mydat[16]=
  //    {0x80,0x90,0xa0,0xb0,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0xc0,0xd0,0xe0,0xf0};
  
int a[4]={0,0,0,0};
  if(length<64) return;
  // Print & analyze the data buffers
  int tbmheader=0; int rocheader=0; int correctdata=0; int tbmtrailer=0;
  //  bool tbmh=false; bool tbmt=false; bool roch=false; bool data=false;
  if ( (data1[0]&0xf0)==0x80 && (data1[1]&0xf0)==0x90 && (data1[2]&0xf0)==0xa0 && (data1[3]&0xf0)==0xb0 && (data2[0]&0xf0)==0x80 && (data2[1]&0xf0)==0x90 && (data2[2]&0xf0)==0xa0 && (data2[3]&0xf0)==0xb0 ) {
    tbmheader++;
    a[0]++;
  }
  
  if ( (data1[4]&0xf0)==0x70 && (data1[11]&0xf0)==0x70 && (data1[18]&0xf0)==0x70 && (data1[25]&0xf0)==0x70 && (data1[32]&0xf0)==0x70 && (data1[39]&0xf0)==0x70 && (data1[46]&0xf0)==0x70 && (data1[53]&0xf0)==0x70 && (data2[4]&0xf0)==0x70 && (data2[11]&0xf0)==0x70 && (data2[18]&0xf0)==0x70 && (data2[25]&0xf0)==0x70 && (data2[32]&0xf0)==0x70 && (data2[39]&0xf0)==0x70 && (data2[46]&0xf0)==0x70 && (data2[53]&0xf0)==0x70 ){
    rocheader++;
    a[1]++;
  }

  for ( int icx=0;icx<8;icx++  ) {
    if ( (data1[5+(7*icx)]&0xf0)==0x10 &&  (data1[6+(7*icx)]&0xf0)==0x20 && (data1[7+(7*icx)]&0xf0)==0x30 &&  (data1[8+(7*icx)]&0xf0)==0x40 && (data1[9+(7*icx)]&0xf0)==0x50 &&  (data1[10+(7*icx)]&0xf0)==0x60 && (data2[5+(7*icx)]&0xf0)==0x10 &&  (data2[6+(7*icx)]&0xf0)==0x20 && (data2[7+(7*icx)]&0xf0)==0x30 &&  (data2[8+(7*icx)]&0xf0)==0x40 && (data2[9+(7*icx)]&0xf0)==0x50 &&  (data2[10+(7*icx)]&0xf0)==0x60  ){
	  correctdata++;
	  a[2]++;
    }
  }
  
  if ( (data1[60]&0xf0)==0xc0 && (data1[61]&0xf0)==0xd0 && (data1[62]&0xf0)==0xe0 && (data1[63]&0xf0)==0xf0 && (data2[60]&0xf0)==0xc0 && (data2[61]&0xf0)==0xd0 && (data2[62]&0xf0)==0xe0 && (data2[63]&0xf0)==0xf0 ) {
    tbmtrailer++;
    a[3]++;
  }
  
  
  std::cout << "tbmheader = " << tbmheader << std::endl;
  std::cout << "rocheader = " << rocheader << std::endl;
  std::cout << "correctdata = " << correctdata << std::endl;
  std::cout << "tbmtrailer = " << tbmtrailer << std::endl;
  std::cout << "a[] = " << a[0] << " "  << a[1] << " "  << a[2] << " "  << a[3] << std::endl;
  return;
      
      
} // end

DigScopeDecoder::DigScopeDecoder(const uint32_t* buffer, unsigned int size) {
  const uint8_t tbm_header_magic[4] = { 0x80, 0x90, 0xA0, 0xB0 };
  const uint8_t tbm_trailer_magic[4] = { 0xC0, 0xD0, 0xE0, 0xF0 };
  const uint8_t roc_header_magic = 0x70;

  tbm_header_found_ = false;
  tbm_trailer_found_ = false;
  
  std::vector<uint8_t> buf(size);
  for (unsigned i = 0; i < size; ++i) {
    assert((buffer[i] & 0xFFFFFF00) == 0);
    buf[i] = buffer[i] & 0xFF;
  }

  if (size < 4)
    return;

  tbm_header_found_ = true; // maybe
  for (unsigned i = 0; i < 4; ++i) {
    if ((buf[i] & 0xF0) != tbm_header_magic[i]) {
      tbm_header_found_ = false;
      return;
    }

    buf[i] &= 0xF;
    tbm_header_payload_.push_back(buf[i]);
  }

  event_number_ = (buf[0] << 4) | buf[1];
  tbm_header_data_id_ = (buf[2] & 0xC) >> 2;
  tbm_header_data_ = ((buf[2] & 0x3) << 4) | buf[3];

  unsigned i = 4;
  std::vector<uint8_t> hit;
  for (; i < size; ++i) {
    const uint8_t hi = buf[i] & 0xF0;
    const uint8_t lo = buf[i] & 0x0F;
    const uint8_t hit_s = uint8_t(hit.size());
    //printf("i %u buf %x hi %x lo %x hit_s %u\n", i, buf[i], hi, lo, hit_s);
    if (hi == roc_header_magic) {
      //printf("roc header\n");
      store_hit(hit);
      roc_headers_.push_back(lo);
    }
    else if ((hi >> 4) == hit_s + 1) {
      //printf("avlid hit info \n");
      hit.push_back(lo);
      if (hit.size() == 6)
	store_hit(hit);
    }	
    else if (hit_s && (hi >> 4) >= 1 && (hi >> 4) <= 6) {
      //printf("malformed hit info\n");
      continue; // let it be malformed
    }
    else if (hi == tbm_trailer_magic[0]) {
      //printf("trailer magic\n");
      store_hit(hit);
      break;
    }
  }

  //printf("i at end is %u   buf %x\n", i, buf[i]);

  dangling_hit_info_ = hit.size();

  if (size - i != 4 || (buf[i] & 0xF0) != tbm_trailer_magic[0])
    return;
  unsigned is = 0;
  for (; i < size; ++i, ++is) {
    //printf("trail mag %x  %x \n", buf[i] & 0xF0, tbm_trailer_magic[is]);
    if ((buf[i] & 0xF0) != tbm_trailer_magic[is]) {
      return;
    }

    buf[i] &= 0xF;
    tbm_trailer_payload_.push_back(buf[i]);
  }

  tbm_trailer_found_ = true;
}

void DigScopeDecoder::printToStream(std::ostream& out) {
  out << "TBM header found? " << tbm_header_found_ << " payload (sz: " << tbm_header_payload_.size() << "): ";
  for (size_t i = 0; i < tbm_header_payload_.size(); ++i)
    out << std::hex << tbm_header_payload_[i] << std::dec << " ";
  out << "  decoded: ev# " << event_number_ << " data_id: " << tbm_header_data_id_ << " data: " << tbm_header_data_ << std::endl;
  out << "# ROC headers found: " << roc_headers_.size() << " payload: ";
  for (size_t i = 0; i < roc_headers_.size(); ++i)
    out << std::hex << roc_headers_[i] << std::dec << " ";
  out << std::endl;
  out << "# hits found: " << n_hits() << " (decoded: " << n_decoded_hits() << " valid: " << n_valid_hits() << ")" << std::endl;
  for (size_t i = 0; i < hits_.size(); ++i) {
    hit_t h = hits_[i];
    out << "hit #" << std::setw(2) << i << " decoded? " << h.decoded << " valid? " << h.valid() << " # nibbles: " << h.nibbles;
    //    if (h.valid())
      out << " roc " << h.roc << " col " << std::setw(2) << h.col << " row " << std::setw(2) << h.row << " ph " << std::setw(3) << h.ph;
    out << std::endl;
  }
  out << "Dangling hit info? " << dangling_hit_info_ << std::endl;
  out << "TBM trailer found? " << tbm_trailer_found_ << " payload (sz: " << tbm_trailer_payload_.size() << "): ";
  for (size_t i = 0; i < tbm_trailer_payload_.size(); ++i)
    out << std::hex << tbm_trailer_payload_[i] << std::dec << " ";
  out << std::endl;
}
