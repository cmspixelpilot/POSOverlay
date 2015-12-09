// g++ -g -I ~/build/TriDAS/pixel -L ~/build/TriDAS/pixel/lib $(root-config --cflags --libs) -lPixelFEDDataTools ~/jmt/dumpTransScopeFifos.cc  -o ~/jmt/dumpTransScopeFifos.exe
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <vector>
#ifdef DO_DECODE
#include "PixelUtilities/PixelFEDDataTools/include/DigScopeDecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/DigTransDecoder.h"
#endif

using namespace std;

struct marker_t {
  marker_t(int a, int c, int t) : at(a), count(c), type(t), d0(0), d1(0), d2(0), d3(0), d0_valid(false), d1_valid(false), d2_valid(false), d3_valid(false) {}
  int at;
  int count;
  int type;
  unsigned char d0;
  unsigned char d1;
  unsigned char d2;
  unsigned char d3;
  bool d0_valid;
  bool d1_valid;
  bool d2_valid;
  bool d3_valid;
};

ostream& operator<<(ostream& o, const marker_t& m) {
  o << setw(2) << m.type << " (" << setw(5) << m.at << ":" << setw(2) << m.count;
  if (m.d0_valid) o << " d0: " << hex << setw(2) << unsigned(m.d0) << dec;
  if (m.d1_valid) o << " d1: " << hex << setw(2) << unsigned(m.d1) << dec;
  if (m.d2_valid) o << " d2: " << hex << setw(2) << unsigned(m.d2) << dec;
  if (m.d3_valid) o << " d3: " << hex << setw(2) << unsigned(m.d3) << dec;
  o << ")";
  if (m.type == 7 && m.d1_valid && m.d2_valid && m.d3_valid) {
    //const int dcol = (m.d1 & 0xfc) >> 2;
    //const int pxl = ((m.d1 & 1) << 7) | ((m.d2 & 0xe0) >> 1);
    //const int hit = ((m.d2&1)<<7) | ((m.d3&0xe0)>>1) | (m.d3&0xf);
    //cout << " (dc: " << setw(2) << dcol << " pxl: " << setw(3) << pxl << " hit " << setw(3) << hit << ")";
  }
  if (m.type == 12 && m.d0_valid && m.d1_valid) {
    if (m.d0 & 0x80) cout << " NTP ";
    if (m.d1 & 0x40) cout << " PKAM ";
    if (m.d1 & 0x1f) cout << " STACK ";
  }
  return o;
}

struct sort_by_count {
  bool operator()(const marker_t& left, const marker_t& right) {
    if (left.count < right.count)
      return false;
    else if (left.count == right.count)
      return left.at < right.at;
    else
      return true;
  }
};

struct sort_by_at {
  bool operator()(const marker_t& left, const marker_t& right) {
    return left.at < right.at;
  }
};

int main(int argc, char** argv) {
  assert(argc >= 3);
  FILE* ftrans = fopen(argv[1], "rb");
  FILE* fscope = fopen(argv[2], "rb");
  assert(ftrans && fscope);

  const bool interp_scope = false;

  const int MaxChips = 8;
  uint32_t bufferT[MaxChips][256];
  uint32_t bufferS[MaxChips][1024];
  int statusS[MaxChips] = {0};

#ifdef DO_DECODE
  DigTransDecoder* decodeT[MaxChips] = {0};
  DigScopeDecoder* decodeS[MaxChips] = {0};
  for (int chip = 1; chip <= 7; chip += 2) {
    if (chip == 1 || chip == 7)
      decodeT[chip] = new DigTransDecoder(bufferT[chip]);
    decodeS[chip] = new DigScopeDecoder(bufferS[chip], statusS[chip]);
  }
#endif

  int nevents = 0;
  int nok[MaxChips][2] = {{0}};
  int num_tbmhead[MaxChips][2] = {{0}};
  int num_rochead[MaxChips][2] = {{0}};
  int num_tbmtrail[MaxChips][2] = {{0}};

  while (1) {
    for (int chip = 1; chip <= 7; chip += 2) {
      if (chip == 1 || chip == 7) {
	const size_t n = fread(bufferT[chip], sizeof(uint32_t), 256, ftrans);
	if (n == 0)
	  goto done;
      }
      fread(&statusS[chip], sizeof(int), 1, fscope);
      printf("statusS[%i] is %u\n", chip, statusS[chip]);
      assert(statusS[chip] <= 1024);
      if (statusS[chip] > 0)
	fread(bufferS[chip], sizeof(uint32_t), statusS[chip], fscope);
    }

    ++nevents;

    for (int chip = 1; chip <= 1; chip += 2) {
      if (chip == 1 || chip == 7) {
	int trans_found = 0;
	//uint32_t pattern = 0;
	uint32_t* data  = bufferT[chip];
	uint32_t* datae = data + 255;
	cout << "-----------------------------------------\n";
	cout << "Contents of transparent FIFO for chip = " << chip << endl;
	cout << "-----------------------------------------\n";

	/*
	uint8_t* data8 = (uint8_t*)bufferT[chip];
	cout << "rw | ";
	for (int j = 0; j < 64; ++j)
	  cout << setw(2) << j << " ";
	cout << endl;
	for (int i = 0; i < 64; ++i) {
	  cout << setw(2) << i << " | ";
	  for (int j = 0; j < 64; ++j)
	    cout << hex << setw(2) << unsigned(data8[i*64+j]) << dec << " ";
	  cout << endl;
	}
	*/

	int nbeg = 0, nend = 0;
	while (*data == 0xffffffff && data != datae)
	  ++nbeg, ++data;
	if (data == datae)
	  cout << "all 0xFFFFFFFF" << endl;
	else {
	  vector<char> bits[2]; // ha
	  while (*datae == 0xffffffff)
	    ++nend, --datae;
	  trans_found = datae-data+1;
	  cout << nbeg << " 0xFFFFFFFF then " << trans_found << " words:" << endl;
	  while (data != datae + 1) {
	    uint32_t d = *data;
	    uint16_t h(d >> 16);
	    uint16_t l(d & 0xFFFF);
	    uint16_t ab[2] = {h, l};
	    cout << hex << setw(4) << h << dec << " ";
	    cout << hex << setw(4) << l << dec << "  ";
	    for (int j = 0; j < 2; ++j) {
	      for (int i = 15; i >= 0; --i) {
		char bit = (ab[j] & (1 << i)) ? '1' : '0';
		bits[!j].push_back(bit);
		cout << bit;
		if (i % 4 == 0) cout << " ";
	      }
	      cout << "  ";
	    }
	    cout << endl;
	    ++data;
	    //int nrun = 0;
	    //while (*data == 0xFFFFFFFF && data != datae + 1) {
	    //  for (int i = 15; i >=0; --i) {
	    //	bits[0].push_back('1');
	    //	bits[1].push_back('1');
	    //  }
	    //  ++nrun, ++data;
	    //}
	    //if (nrun) cout << "then a " << nrun << " run of 0xFFFFFFFF, then" << endl;
	  }
	  cout << "then " << nend << " 0xFFFFFFFF" << endl;

	  for (int j = 0; j < 2; ++j) {
	    cout << "tbm " << j << ": " << (j == 0 ? "A = 16 LSB" : "B = 16 MSB") << "\n";
	    const int nbits = bits[j].size();
	    if (nbits < 12)
	      cout << "not enough bits\n";
	    else {
	      vector<marker_t> tbmhead;
	      vector<marker_t> tbmtrail;
	      vector<marker_t> rochead;
	      int count = -1;

	      for (int i = 0; i < nbits - 12; ++i) {
		count = 
		  int(bits[j][i   ] == '0') +
		  int(bits[j][i+ 1] == '1') +
		  int(bits[j][i+ 2] == '1') +
		  int(bits[j][i+ 3] == '1') +
		  int(bits[j][i+ 4] == '1') +
		  int(bits[j][i+ 5] == '1') +
		  int(bits[j][i+ 6] == '1') +
		  int(bits[j][i+ 7] == '1') +
		  int(bits[j][i+ 8] == '1') +
		  int(bits[j][i+ 9] == '1') +
		  int(bits[j][i+10] == '0') +
		  int(bits[j][i+11] == '0');
		if (count == 12) {
		  marker_t h(i, count, 8);
		  if (nbits >= i+12+8) {
		    unsigned char d = 0;
		    for (int k = 12; k < 12+8; ++k) {
		      d <<= 1;
		      d |= bits[j][i+k] - '0';
		    }
		    h.d0 = d;
		    h.d0_valid = true;
		  }
		  if (nbits >= i+12+16) {
		    unsigned char d = 0;
		    for (int k = 12+8; k < 12+16; ++k) {
		      d <<= 1;
		      d |= bits[j][i+k] - '0';
		    }
		    h.d1 = d;
		    h.d1_valid = true;
		  }
		  tbmhead.push_back(h);
		}

		count = 
		  int(bits[j][i   ] == '0') +
		  int(bits[j][i+ 1] == '1') +
		  int(bits[j][i+ 2] == '1') +
		  int(bits[j][i+ 3] == '1') +
		  int(bits[j][i+ 4] == '1') +
		  int(bits[j][i+ 5] == '1') +
		  int(bits[j][i+ 6] == '1') +
		  int(bits[j][i+ 7] == '1') +
		  int(bits[j][i+ 8] == '1') +
		  int(bits[j][i+ 9] == '1') +
		  int(bits[j][i+10] == '1') +
		  int(bits[j][i+11] == '0');
		if (count == 12) {
		  marker_t t(i, count, 12);
		  if (nbits >= i+12+8) {
		    unsigned char d = 0;
		    for (int k = 12; k < 12+8; ++k) {
		      d <<= 1;
		      d |= bits[j][i+k] - '0';
		    }
		    t.d0 = d;
		    t.d0_valid = true;
		  }
		  if (nbits >= i+12+16) {
		    unsigned char d = 0;
		    for (int k = 12+8; k < 12+16; ++k) {
		      d <<= 1;
		      d |= bits[j][i+k] - '0';
		    }
		    t.d1 = d;
		    t.d1_valid = true;
		  }
		  tbmtrail.push_back(t);
		}

		count = 
		  int(bits[j][i   ] == '0') +
		  int(bits[j][i+ 1] == '1') +
		  int(bits[j][i+ 2] == '1') +
		  int(bits[j][i+ 3] == '1') +
		  int(bits[j][i+ 4] == '1') +
		  int(bits[j][i+ 5] == '1') +
		  int(bits[j][i+ 6] == '1') +
		  int(bits[j][i+ 7] == '1') +
		  int(bits[j][i+ 8] == '1') +
		  int(bits[j][i+ 9] == '0');
		if (count == 10) {
		  marker_t r(i, count, 7);
		  if (nbits >= i+10+2) {
		    unsigned char d = 0;
		    for (int k = 10; k < 10+2; ++k) {
		      d <<= 1;
		      d |= bits[j][i+k] - '0';
		    }
		    r.d0 = d;
		    r.d0_valid = true;
		  }
		  if (nbits >= i+10+10) {
		    unsigned char d = 0;
		    for (int k = 10+2; k < 10+10; ++k) {
		      d <<= 1;
		      d |= bits[j][i+k] - '0';
		    }
		    r.d1 = d;
		    r.d1_valid = true;
		  }
		  if (nbits >= i+10+18) {
		    unsigned char d = 0;
		    for (int k = 10+10; k < 10+18; ++k) {
		      d <<= 1;
		      d |= bits[j][i+k] - '0';
		    }
		    r.d2 = d;
		    r.d2_valid = true;
		  }
		  if (nbits >= i+10+26) {
		    unsigned char d = 0;
		    for (int k = 10+18; k < 10+26; ++k) {
		      d <<= 1;
		      d |= bits[j][i+k] - '0';
		    }
		    r.d3 = d;
		    r.d3_valid = true;
		  }

		  rochead.push_back(r);
		}
	      }

	      const int ntbmh = int(tbmhead.size());
	      const int ntbmt = int(tbmtrail.size());
	      const int nroch = int(rochead.size());

	      sort(tbmhead.begin(), tbmhead.end(), sort_by_count());
	      sort(tbmtrail.begin(), tbmtrail.end(), sort_by_count());
	      sort(rochead.begin(), rochead.end(), sort_by_count());

	      cout << "tbm headers:\n";
	      for (int k = 0; k < ntbmh; ++k)
		cout << tbmhead[k] << "\n";
	      cout << endl;
	      cout << "tbm trailers:\n";
	      for (int k = 0; k < ntbmt; ++k)
		cout << tbmtrail[k] << "\n";
	      cout << endl;
	      cout << "roc headers:\n";
	      for (int k = 0; k < nroch; ++k)
		cout << rochead[k] << "\n";
	      cout << endl;
	      vector<marker_t> markers;
	      markers.insert(markers.end(), tbmhead.begin(), tbmhead.end());
	      markers.insert(markers.end(), tbmtrail.begin(), tbmtrail.end());
	      markers.insert(markers.end(), rochead.begin(), rochead.end());
	      sort(markers.begin(), markers.end(), sort_by_at());
	      cout << "merged, sorted by at:\n";
	      for (size_t k = 0; k < markers.size(); ++k)
		cout << markers[k] << "\n";
	      cout << endl;

	      const bool ok = ntbmt != 0  && ntbmt == ntbmh && nroch/ntbmt == 8;
	      if (ok) ++nok[chip][j];
	      if (chip == 7 && j == 0 && !ok)
		printf("HELLO ntbmh %i ntbmt %i nroch %i\n", ntbmh, ntbmt, nroch);
	      num_tbmhead[chip][j] += ntbmh;
	      num_rochead[chip][j] += nroch;
	      num_tbmtrail[chip][j] += ntbmt;

	      if (markers.size()) {
		bool in = false;
		int nroc = 0;
		marker_t first_marker = markers[0];
		vector<int> nrocs;
		if (first_marker.type == 8 || first_marker.type == 7)
		  in = true;
		printf("markers: ");
		for (size_t k = 0; k < markers.size(); ++k) {
		  const marker_t& m = markers[k];
		  printf("%i ", m.type);
		  if (m.type == 7) {
		    if (in)
		      ++nroc;
		    else
		      --nroc;
		  }
		  else if (m.type == 12) {
		    in = false;
		    nrocs.push_back(nroc);
		    nroc = 0;
		  }
		  else if (m.type == 8) {
		    in = true;
		    nrocs.push_back(nroc);
		    nroc = 0;
		  }
		}
		printf("\nseq: ");
		for (size_t k = 0; k < nrocs.size(); ++k)
		  printf("%i ", nrocs[k]);
		printf("\n");
	      }

	      for (int k = 0; k < ntbmt; ++k) {
		const marker_t& t = tbmtrail[k];
		if (t.d1 & 0x40) {
		  marker_t rh(0, 0, 0);
		  for (int kk = 0; kk < nroch; ++kk) {
		    const marker_t& r = rochead[kk];
		    if (r.at < t.at && r.at > rh.at)
		      rh = r;
		  }

		  int streak = 0, streak_at = 0;
		  for (int i = rh.at + 12; i < t.at; ++i) {
		    if (bits[j][i] == '1')
		      ++streak;
		    else {
		      streak = 0;
		      streak_at = i;
		    }
		  }
		  printf("pkam 1s streak between %i and %i: %i starting at %i (%i from end of roc header)\n", rh.at+12, t.at, streak, streak_at, streak_at - (rh.at+12));
		}
	      }

	    }
	  }
	}

#ifdef DO_DECODE
	cout << "DigTransDecoder thinks:\n";
	decodeT[chip]->printToStream(cout);
#endif
      }

      cout << "----------------------------------" << endl;
      if (statusS[chip] < 0)
	cout << "Scope FIFO for chip = " << chip << " status = " << statusS[chip] << endl;
      else {
	std::vector<uint8_t> events;
	cout << "Contents of Scope FIFO for chip = " << chip << "(statusS = " << statusS[chip] << ")" <<endl;
	cout << "----------------------------------" << endl;
	bool weird = false;
	bool weird_first = false;
	uint16_t last_ts = 0;
	for (int i = 0; i < statusS[chip]; ++i) {
	  const uint16_t ts = (bufferS[chip][i] & 0x3ff00) >> 8;
	  if (ts < last_ts && i > 0)
	    printf("WTS ");
	  const uint8_t d = (bufferS[chip][i]) & 0xff;
	  const uint8_t dh = d & 0xf0;
	  const uint8_t last_dh = i > 0 ? (bufferS[chip][i-1] & 0xf0) : 0;

	  cout << setw(2) << dec << int(ts) << ":" << hex << int(d) << dec << " ";

	  if ((i == 0 && dh != 0x80 && statusS[chip] != 1023) ||
	      (last_dh == 0x80 && dh != 0x90) ||
	      (last_dh == 0x90 && dh != 0xa0) ||
	      (last_dh == 0xa0 && dh != 0xb0) ||
	      (last_dh == 0xb0 && dh != 0x70 && dh != 0xc0) ||
	      (last_dh == 0x70 && dh != 0x70 && dh != 0x10 && dh != 0xc0) ||
	      (last_dh == 0x10 && dh != 0x20) ||
	      (last_dh == 0x20 && dh != 0x30) ||
	      (last_dh == 0x30 && dh != 0x40) ||
	      (last_dh == 0x40 && dh != 0x50) ||
	      (last_dh == 0x50 && dh != 0x60) ||
	      (last_dh == 0x60 && dh != 0x10 && dh != 0x70 && dh != 0xc0) ||
	      (last_dh == 0xc0 && dh != 0xd0) ||
	      (last_dh == 0xd0 && dh != 0xe0) ||
	      (last_dh == 0xe0 && dh != 0xf0) ||
	      (last_dh == 0xf0 && dh != 0x0 && dh != 0x80)) {
	    //printf("weird! (last_dh 0x%02x dh 0x%02x) ", last_dh, dh);
	    printf("WW ");
	    weird = true;
	    if (i == 0)
	      weird_first = true;
	  }

	  if (dh == 0x70 || dh == 0xb0 || dh == 0xf0 || dh == 0x60)
	    cout << "\n";

	  if (i >= 3 && dh == 0xb0 &&
	      (bufferS[chip][i-1] & 0xf0) == 0xa0 &&
	      (bufferS[chip][i-2] & 0xf0) == 0x90 &&
	      (bufferS[chip][i-3] & 0xf0) == 0x80) {
	    const int ev = ((bufferS[chip][i-3] & 0xf) << 4) | (bufferS[chip][i-2] & 0xf);
	    events.push_back(ev);
	    const int da = ((bufferS[chip][i-1] & 0xf) << 4) | (bufferS[chip][i]   & 0xf);
	    if (interp_scope) {
	      cout << "| ev: " << setw(3) << ev << " data: " << hex << da << dec << " = ";
	      for (int k = 7; k >= 0; --k) { cout << (da&(1<<k) ? 1 : 0); if (k == 4 || k == 0) cout << " "; }
	      cout << "\n";
	    }
	  }
	  else if (i >= 3 && dh == 0xf0 &&
		   (bufferS[chip][i-1] & 0xf0) == 0xe0 &&
		   (bufferS[chip][i-2] & 0xf0) == 0xd0 &&
		   (bufferS[chip][i-3] & 0xf0) == 0xc0) {
	    const int da0 = ((bufferS[chip][i-3] & 0xf) << 4) | (bufferS[chip][i-2] & 0xf);
	    const int da1 = ((bufferS[chip][i-1] & 0xf) << 4) | (bufferS[chip][i]   & 0xf);
	    if (interp_scope) {
	      cout << "| d0: " << setw(2) << hex << da0 << dec << " = ";
	      for (int k = 7; k >= 0; --k) { cout << (da0&(1<<k) ? 1 : 0); if (k == 4 || k == 0) cout << " "; }
	    }
	    if (da0 & 0x80) cout << " NTP ";
	    if (da1 & 0x40) cout << " PKAM ";
	    if (da1 & 0x1f) cout << " STACK ";
	    if (interp_scope) {
	      cout << " d1: " << hex << da1 << dec << " = ";
	      for (int k = 7; k >= 0; --k) { cout << (da1&(1<<k) ? 1 : 0); if (k == 4 || k == 0) cout << " "; }
	      cout << "\n";
	    }
	  }
	  else if (i >= 5 && dh == 0x60 &&
		   (bufferS[chip][i-1] & 0xf0) == 0x50 &&
		   (bufferS[chip][i-2] & 0xf0) == 0x40 &&
		   (bufferS[chip][i-3] & 0xf0) == 0x30 &&
		   (bufferS[chip][i-4] & 0xf0) == 0x20 &&
		   (bufferS[chip][i-5] & 0xf0) == 0x10) {
	    const int a = int(((bufferS[chip][i-5] & 0xf) << 4) | (bufferS[chip][i-4] & 0xf));
	    const int b = int(((bufferS[chip][i-3] & 0xf) << 4) | (bufferS[chip][i-2] & 0xf));
	    const int c = int(((bufferS[chip][i-1] & 0xf) << 4) | (bufferS[chip][i]   & 0xf));
	    const int dcol = a >> 2;
	    const int pxl = ((a&3) << 7) | (b >> 1);
	    const int hit = ((b&1)<<7) | ((c&0xe0)>>1) | (c&0xf);
	    if (interp_scope) {
	      cout << "| ";
	      cout << hex << setw(2) << a << " " << dec;
	      cout << hex << setw(2) << b << " " << dec;
	      cout << hex << setw(2) << c << " " << dec;
	      cout << " | ";
	      for (int k = 7; k >= 0; --k) { cout << (a&(1<<k) ? 1 : 0); if (k == 4 || k == 0) cout << " "; }
	      for (int k = 7; k >= 0; --k) { cout << (b&(1<<k) ? 1 : 0); if (k == 4 || k == 0) cout << " "; }
	      for (int k = 7; k >= 0; --k) { cout << (c&(1<<k) ? 1 : 0); if (k == 4 || k == 0) cout << " "; }
	      cout << "| dc: " << setw(2) << dcol << " pxl: " << setw(3) << pxl << " hit " << setw(3) << hit << "\n";
	    }
	  }

	  last_ts = ts;
	}

	if (weird) {
	  if (statusS[chip] < 1023) {
	    if (!weird_first)
	      printf("weird non-full event not at first\n");
	    else
	      printf("weird non-full event\n");
	  }
	  else
	    printf("weird full event\n");
	}
	cout << "\nevents seen: ";
	for (size_t k = 0; k < events.size(); ++k)
	  cout << hex << setw(2) << int(events[k]) << dec << " ";
	cout << "\n----------------------------------" << endl;
      }
#ifdef DO_DECODE
      cout << "DigScopeDecoder thinks:\n";
      decodeS[chip]->printToStream(cout);
#endif
    }

    bool end_trans = feof(ftrans);
    bool end_scope = feof(fscope);
    if (end_trans || end_scope) {
      assert(end_trans && end_scope);
      break;
    }
  }

 done:
  printf("nevents: %i\n", nevents);
  for (int chip = 1; chip <= 7; chip += 6)
    for (int j = 0; j < 2; ++j)
      printf("chip %i tbm %i  #ok: %i  #tbm head: %i  #tbm trail: %i  #roc head: %i\n", chip, j, nok[chip][j], num_tbmhead[chip][j], num_tbmtrail[chip][j], num_rochead[chip][j]);

#ifdef DO_DECODE
  for (int chip = 1; chip <= 7; chip += 2) {
    if (chip == 1 || chip == 7)
      delete decodeT[chip];
    delete decodeS[chip];
  }
#endif
}
