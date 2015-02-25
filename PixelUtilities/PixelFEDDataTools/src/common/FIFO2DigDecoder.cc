#include "PixelUtilities/PixelFEDDataTools/include/FIFO2DigDecoder.h"
#include <cassert>
#include <iomanip>
#include <stdio.h>

FIFO2DigDecoder::FIFO2DigDecoder(const uint32_t* buffer, unsigned int size) {
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

void FIFO2DigDecoder::printToStream(std::ostream& out) {
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
