#ifndef _DigScopeDecoder_h_
#define _DigScopeDecoder_h_

#include <iostream>
#include <stdint.h>
#include <vector>

int decodePTrans(unsigned* data1, unsigned* data2, const int length);
int decodePTrans2(unsigned * data1, unsigned* data2, const int length);
void decodePTrans3(unsigned* data1, unsigned* data2, const int length);

class DigScopeDecoder {
 public:
  DigScopeDecoder(const uint32_t* buffer, unsigned int size);
  void printToStream(std::ostream& out);

  struct hit_t {
    hit_t() : decoded(false) {}
    hit_t(int roc_, int col_, int row_, int ph_) : decoded(true), roc(roc_), col(col_), row(row_), ph(ph_) {}
    hit_t(int roc_, const std::vector<uint8_t>& s) : decoded(false), nibbles(s.size()), roc(roc_) {
      if (nibbles != 6 || (s[4] & 0x1) != 0)
	return;

      col = ((s[0] & 0xF) << 2) | ((s[1] & 0xC) >> 2);
      row = ((s[1] & 0x3) << 7) | ((s[2] & 0xF) << 3) | ((s[3] & 0xE) >> 1);
      ph = ((s[3] & 0x1) << 7) | ((s[4] & 0xE) << 3) | (s[5] & 0xF);

      decoded = true;
    }

    bool valid() const { return decoded && col >= 0 && col < 52 && row >= 0 && row < 80; }

    bool decoded;
    int nibbles;
    int roc;
    int col;
    int row;
    int ph;
  };

  int n_hits() const { return hits_.size(); }
  int n_decoded_hits() const { 
    int n = 0;
    for (size_t i = 0; i < hits_.size(); ++i)
      if (hits_[i].decoded)
	++n;
    return n;
  }
  int n_valid_hits() const { 
    int n = 0;
    for (size_t i = 0; i < hits_.size(); ++i)
      if (hits_[i].valid())
	++n;
    return n;
  }

  const std::vector<hit_t>& hits() const { return hits_; }

  void store_hit(std::vector<uint8_t>& s) {
    if (!s.size()) return;
    hits_.push_back(hit_t(roc_headers_.size(), s));
    s.clear();
  }

  bool tbm_header_found_;
  std::vector<unsigned> tbm_header_payload_;
  unsigned event_number_;
  unsigned tbm_header_data_id_;
  unsigned tbm_header_data_;
  std::vector<unsigned> roc_headers_;
  std::vector<hit_t> hits_; 
  unsigned dangling_hit_info_;
  bool tbm_trailer_found_;
  std::vector<unsigned> tbm_trailer_payload_;
};

#endif
