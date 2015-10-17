// g++ -g -I ~/build/TriDAS/pixel -L ~/build/TriDAS/pixel/lib $(root-config --cflags --libs) -lPixelFEDDataTools ~/jmt/dumpTransScopeFifos.cc  -o ~/jmt/dumpTransScopeFifos.exe
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include "PixelUtilities/PixelFEDDataTools/include/DigScopeDecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/DigTransDecoder.h"

using namespace std;

struct marker_t {
  marker_t(int a, int c, int t) : at(a), count(c), type(t), d0(0), d1(0), d0_valid(false), d1_valid(false) {}
  int at;
  int count;
  int type;
  unsigned char d0;
  unsigned char d1;
  bool d0_valid;
  bool d1_valid;
};

ostream& operator<<(ostream& o, const marker_t& m) {
  o << setw(2) << m.type << " (" << setw(5) << m.at << ":" << setw(2) << m.count;
  if (m.d0_valid)
    o << " d0: " << hex << setw(2) << unsigned(m.d0) << dec;
  if (m.d1_valid)
    o << " d1: " << hex << setw(2) << unsigned(m.d1) << dec;
  o << ")";
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
  assert(argc >= 4);
  const int maxNumHitsPerROC = atoi(argv[1]); //tempCalibObject->maxNumHitsPerROC()
  FILE* ftrans = fopen(argv[2], "rb");
  FILE* fscope = fopen(argv[3], "rb");
  assert(ftrans && fscope);

  const int MaxChips = 8;
  uint32_t bufferT[MaxChips][256];
  uint32_t bufferS[MaxChips][256];
  int statusS[MaxChips] = {0};

  DigTransDecoder* decodeT[MaxChips] = {0};
  DigScopeDecoder* decodeS[MaxChips] = {0};
  for (int chip = 1; chip <= 7; chip += 2) {
    if (chip == 1 || chip == 7)
      decodeT[chip] = new DigTransDecoder(bufferT[chip]);
    decodeS[chip] = new DigScopeDecoder(bufferS[chip], statusS[chip]);
  }

  while (1) {
    for (int chip = 1; chip <= 7; chip += 2) {
      if (chip == 1 || chip == 7)
	fread(bufferT[chip], sizeof(uint32_t), 256, ftrans);
      fread(&statusS[chip], sizeof(int), 1, fscope);
      assert(statusS[chip] <= 256);
      if (statusS[chip] > 0)
	fread(bufferS[chip], sizeof(uint32_t), statusS[chip], fscope);
    }

    for (int chip = 1; chip <= 7; chip += 6) {
      if (chip == 1 || chip == 7) {
	int trans_found = 0;
	uint32_t pattern = 0;
	uint32_t* data  = bufferT[chip];
	uint32_t* datae = data + 1023;
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
	    int nrun = 0;
	    while (*data == 0xFFFFFFFF && data != datae + 1) {
	      for (int i = 15; i >=0; --i) {
		bits[0].push_back('1');
		bits[1].push_back('1');
	      }
	      ++nrun, ++data;
	    }
	    if (nrun) cout << "then a " << nrun << " run of 0xFFFFFFFF, then" << endl;
	  }
	  cout << "then " << nend << " 0xFFFFFFFF" << endl;

	  for (int j = 0; j < 2; ++j) {
	    cout << "tbm " << j << ":\n";
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
		  if (nbits >= i+12+12) {
		    unsigned char d = 0;
		    for (int k = 12+10; k < 12+12; ++k) {
		      d <<= 1;
		      d |= bits[j][i+k] - '0';
		    }
		    r.d0 = d;
		    r.d0_valid = true;
		  }
		  rochead.push_back(r);
		}
	      }

	      sort(tbmhead.begin(), tbmhead.end(), sort_by_count());
	      sort(tbmtrail.begin(), tbmtrail.end(), sort_by_count());
	      sort(rochead.begin(), rochead.end(), sort_by_count());
	      cout << "tbm headers:\n";
	      for (size_t k = 0; k < tbmhead.size(); ++k)
		cout << tbmhead[k] << "\n";
	      cout << endl;
	      cout << "tbm trailers:\n";
	      for (size_t k = 0; k < tbmtrail.size(); ++k)
		cout << tbmtrail[k] << "\n";
	      cout << endl;
	      cout << "roc headers:\n";
	      for (size_t k = 0; k < rochead.size(); ++k)
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
	  }
	  

	  cout << "try to align with headers:\n";
	  const int nroccands = 8;
	  for (int j = 0; j < 2; ++j) {
	    cout << "tbm " << j << ":\n";
	    int besttbmhead = -1;
	    int besttbmheadcount = -1;
	    int besttbmtrail = -1;
	    int besttbmtrailcount = -1;
	    vector<int> bestroc(nroccands, -1);
	    vector<int> bestroccount(nroccands, -1);
	    vector<int> bestrocalign(nroccands, -1);
	    int count = -1;
	    const int nbits = bits[j].size();
	    if (nbits < 12)
	      cout << "not enough bits\n";
	    else {
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
		if (count > besttbmheadcount) {
		  besttbmheadcount = count;
		  besttbmhead = i;
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
		if (count > besttbmtrailcount) {
		  besttbmtrailcount = count;
		  besttbmtrail = i;
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
		const int align = (i - (besttbmhead + 12 + 16)) % 12;
		if (align == 0) {
		  for (int k = 0; k < nroccands; ++k) {
		    if (count > bestroccount[k]) {
		      int tmpcount = bestroccount[k];
		      int tmp = bestroc[k];
		      bestroccount[k] = count;
		      bestroc[k] = i;
		      for (int l = nroccands-1; l > k+1; --l) {
			bestroccount[l] = bestroccount[l-1];
			bestroc[l] = bestroc[l-1];
		      }
		      if (k < nroccands-1) {
			bestroccount[k+1] = tmpcount;
			bestroc[k+1] = tmp;
		      }
		      break;
		    }
		  }
		}
	      }

	      cout << "best match of tbm header  at " << setw(4) << nbeg*16 + besttbmhead  << " with count " << besttbmheadcount << "\n";
	      cout << "best match of tbm trailer at " << setw(4) << nbeg*16 + besttbmtrail << " with count " << besttbmtrailcount << "\n";
	      if ((besttbmtrail - (besttbmhead + 12 + 16)) % 12 != 0)
		cout << "  ^ tbm trailer misaligned wrt tbm header!\n";
	      cout << "matches of roc headers:\n";
	      int bestroccountsum = 0;
	      for (int k = 0; k < nroccands; ++k) {
		cout << "  at " << setw(4) << nbeg*16 + bestroc[k] << " with count " << bestroccount[k] << "\n";
		if (k < 8) {
		  bestroccountsum += bestroccount[k];
		  if ((bestroc[k] - (besttbmhead + 12 + 16)) % 12 != 0)
		    cout << "    ^ roc header misaligned wrt tbm header!\n";
		}
	      }

	      //		  vector<pair<int, int> > roccands;
	      //		  for (int k = 0; k < nroccands; ++k) {
	      //		    if (bestroccount[k] == 10) {
	      //		      roccands.push_back(make_pair(
	      //		  }
		  

	      if (besttbmheadcount != 12 || besttbmtrailcount != 12 || bestroccountsum != 80)
		cout << "problem with headers or trailers!\n";

	      cout << "print, aligning only with tbm header, and guessing where roc headers and hit bits should be based on " << maxNumHitsPerROC << " hits / roc in calib\n";
	      cout << "throw away: ";
	      for (int i = 0; i < besttbmhead; ++i) {
		cout << bits[j][i];
		if (i % 4 == 3) cout << " ";
	      }
	      cout << "\n";

	      cout << "tbm header: ";
	      for (int i = besttbmhead; i < besttbmhead+12; ++i) {
		cout << bits[j][i];
	      }
	      cout << "  payload: ";
	      for (int i = besttbmhead+12; i < besttbmhead+12+2*8; ++i) {
		cout << bits[j][i];
		const int id = i-(besttbmhead+12);
		if (id == 7 || id == 9) cout << " ";
	      }
	      cout << "\n";

	      const int nhitsperroc = maxNumHitsPerROC;
	      const int nbitsperroc = 12 + 3*8*nhitsperroc;
	      for (int k = 0; k < 8; ++k) {
		const int ib = besttbmhead+12+2*8 + nbitsperroc*k;
		const int ic = besttbmhead+12+2*8 + nbitsperroc*k + 12;
		const int ie = besttbmhead+12+2*8 + nbitsperroc*(k+1);
		cout << "roc " << k << " header: ";
		for (int i = ib; i < ic; ++i) {
		  cout << bits[j][i];
		  if ((i-ib) == 9) cout << " ";
		}
		cout << "\nhits:\n";
		for (int i = ic; i < ie; ++i) {
		  cout << bits[j][i];
		  const int id = (i - ic) % 24;
		  if (id == 5 || id == 14 || id == 18 || id == 19) cout << " ";
		  else if (id == 23) cout << "\n";
		}
	      }

	      cout << "tbm trailer: ";
	      {
		const int ib = besttbmhead+12+2*8 + nbitsperroc*8;
		const int ie = besttbmhead+12+2*8 + nbitsperroc*8 + 12;
		for (int i = ib; i < ie; ++i)
		  cout << bits[j][i];
	      }
	      cout << "  payload: ";
	      {
		const int ib = besttbmhead+12+2*8 + nbitsperroc*8 + 12;
		const int ie = besttbmhead+12+2*8 + nbitsperroc*8 + 12 + 16;
		for (int i = ib; i < ie; ++i) {
		  cout << bits[j][i];
		  if ((i-ib) % 4 == 3) cout << " ";
		}
	      }
	      cout << endl;
	    }
	  }
	}

	cout << "DigTransDecoder thinks:\n";
	decodeT[chip]->printToStream(cout);
      }

      cout << "----------------------------------" << endl;
      if (statusS[chip] < 0)
	cout << "Scope FIFO for chip = " << chip << " status = " << statusS[chip] << endl;
      else {
	cout << "Contents of Scope FIFO for chip = " << chip << "(statusS = " << statusS[chip] << ")" <<endl;
	cout << "----------------------------------" << endl;
	for (int i = 0; i <= statusS[chip]; ++i) {
	  uint32_t d = bufferS[chip][i];
	  uint32_t dh = d & 0xf0;
	  if (dh == 0x70 || dh == 0x10 || dh == 0xc0)
	    cout << "\n";
	  if (d > 0xFF)
	    cout << "\nweird word: " << hex << d << "\n";
	  else 
	    cout << setw(2) << hex << d << dec << " ";
	  if ((dh >> 4) >= 1 && (dh >> 4) <= 5)
	    cout << " hello!\n";
	}
	cout << "\n----------------------------------" << endl;
      }
      cout << "DigScopeDecoder thinks:\n";
      decodeS[chip]->printToStream(cout);
    }

    bool end_trans = feof(ftrans);
    bool end_scope = feof(fscope);
    if (end_trans || end_scope) {
      assert(end_trans && end_scope);
      break;
    }
  }

  for (int chip = 1; chip <= 7; chip += 2) {
    if (chip == 1 || chip == 7)
      delete decodeT[chip];
    delete decodeS[chip];
  }
}
