#include "PixelUtilities/PixelFEDDataTools/include/FIFO1DigDecoder.h"
#include <cassert>
#include <iomanip>
#include <stdio.h>

namespace {
  template <typename T>
  void binprint(T d, int n) {
    for (int i = (n-1); i >= 0; --i) {
      std::cout << ((d & (1 << i)) ? "1" : "0");
      if (i % 4 == 0) std::cout << " ";
    }
  }
}

FIFO1DigDecoder::FIFO1DigDecoder(const uint32_t* buffer) {
  const int size = 1024;
  const unsigned tbm_header = 2044;  // 0 1111 1111 100
  const unsigned tbm_trailer = 2046; // 0 1111 1111 110
  const unsigned roc_header = 510;   // 0 1111 1111 0xx <- read back data

  for (int i = 0; i < size-1; ++i) {
    for (int j = 0; j < 2; ++j) {
      uint32_t word;
      if (j == 0)
	word = ((buffer[i] & 0xFFFF) << 16) | (buffer[i+1] & 0xFFFF);
      else
	word = (buffer[i] & 0xFFFF0000) | (buffer[i+1] >> 16);

      for (int k = 0; k < 16; ++k) {
	const int nb = i*16 + k;
	const uint32_t word12 = (word & (0xFFF00000 >> k)) >> (20-k);
	const uint32_t word10 = (word & (0xFFC00000 >> k)) >> (22-k);
	assert((word12 & 0xFFFFF000) == 0);
	assert((word10 & 0xFFFFFC00) == 0);

	//if ((word12 & 0xFFFFF000) != 0) {
	//  std::cout << "problem with word12: j = " << j << " nb = " << nb << " word12 = " << std::hex << word12 << std::dec << " ";
	//  binprint(word12, 32);
	//  std::cout << "\n";
	//}
	//if ((word10 & 0xFFFFFC00) != 0) {
	//  std::cout << "problem with word10: j = " << j << " nb = " << nb << " word10 = " << std::hex << word10 << std::dec << " ";
	//  binprint(word10, 32);
	//  std::cout << "\n";
	//}
	//
	//if (j == 0) {
	//  std::cout << "nb: " << std::setw(4) << nb << " ";
	//  //std::cout << "word12: ";
	//  //binprint(word12, 12);
	//  std::cout << "word10: " << std::setw(4) << std::hex << word10 << std::dec << "   ";
	//  binprint(word10, 10);
	//  std::cout << std::endl;
	//}

	if (word12 == tbm_header) {
	  //if (j==0) std::cout << "tbm header\n";
	  tbm_header_l[j].push_back(nb);
	}
	else if (word12 == tbm_trailer) {
	  //if (j==0) std::cout << "tbm trailer\n";
	  tbm_trailer_l[j].push_back(nb);
	}
	if (word10 == roc_header) {
	  //if (j==0) std::cout << "roc header\n";
	  roc_header_l[j].push_back(nb);
	}
      }
    }
  }
}

void FIFO1DigDecoder::printToStream(std::ostream& out) {
  for (int j = 0; j < 2; ++j) {
    out << "tbm core " << j << "\n";
    const std::vector<int>* vs[3] = { &tbm_header_l[j], &roc_header_l[j], &tbm_trailer_l[j] };
    const char* ns[3] = { "tbm headers", "roc headers", "tbm trailers" };
    for (int k = 0; k < 3; ++k) {
      const std::vector<int>& v = *vs[k]; 
      const size_t ni = v.size();
      out << ns[k] << " (" << ni << ") at ";
      std::vector<int> diffs(ni > 1 ? ni-1 : 1, -1);
      for (size_t i = 0; i < ni; ++i) {
	if (i > 0)
	  diffs[i-1] = v[i] - v[i-1];
	out << v[i] << " ";
      }
      if (ni>1) {
	out << " (deltas: ";
	for (size_t i = 0; i < ni-1; ++i)
	  out << diffs[i] << " ";
	out << ")";
      }
      out << "\n";
    }
  }
  out << std::endl;
}
