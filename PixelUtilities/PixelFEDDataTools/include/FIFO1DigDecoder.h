#ifndef _FIFO1DigDecoder_h_
#define _FIFO1DigDecoder_h_

#include <iostream>
#include <stdint.h>
#include <vector>

class FIFO1DigDecoder {
 public:
  FIFO1DigDecoder(const uint32_t* buffer);
  void printToStream(std::ostream& out);

  struct hit_t {
    hit_t() : decoded(false) {}
    hit_t(int roc_, const uint32_t h) : decoded(false), roc(roc_) {
//      
//      if (nibbles != 6 || (s[4] & 0x1) != 0)
//	return;
//
//      col = ((s[0] & 0xF) << 2) | ((s[1] & 0xC) >> 2);
//      row = ((s[1] & 0x3) << 7) | ((s[2] & 0xF) << 3) | ((s[3] & 0xE) >> 1);
//      ph = ((s[3] & 0x1) << 7) | ((s[4] & 0xE) << 3) | (s[5] & 0xF);
//
//      decoded = true;
    }

    bool valid() const { return decoded && col >= 0 && col < 52 && row >= 0 && row < 80; }

    bool decoded;
    int nibbles;
    int roc;
    int col;
    int row;
    int ph;
  };

//  int n_hits() const { return hits_.size(); }
//  int n_decoded_hits() const { 
//    int n = 0;
//    for (size_t i = 0; i < hits_.size(); ++i)
//      if (hits_[i].decoded)
//	++n;
//    return n;
//  }
//  int n_valid_hits() const { 
//    int n = 0;
//    for (size_t i = 0; i < hits_.size(); ++i)
//      if (hits_[i].valid())
//	++n;
//    return n;
//  }
//
//  const std::vector<hit_t>& hits() const { return hits_; }

  void store_hit(std::vector<uint8_t>& s) {
  //  if (!s.size()) return;
  //  hits_.push_back(hit_t(roc_headers_.size(), s));
  //  s.clear();
  }

  std::vector<int> tbm_header_l[2];
  std::vector<int> roc_header_l[2];
  std::vector<int> tbm_trailer_l[2];
  unsigned event_number;
  unsigned tbm_header_data_id;
  unsigned tbm_header_data;
  std::vector<unsigned> roc_header_payloads;
  std::vector<hit_t> hits;
};

#endif
