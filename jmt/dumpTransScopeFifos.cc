#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include "PixelUtilities/PixelFEDDataTools/include/DigScopeDecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/DigTransDecoder.h"

int main(int argc, char** argv) {
  assert(argc >= 4);
  const int maxNumHitsPerROC = atoi(argv[1]); //tempCalibObject->maxNumHitsPerROC()
  FILE* ftrans = fopen(argv[2], "rb");
  FILE* fscope = fopen(argv[3], "rb");
  assert(ftrans && fscope);

  const int MaxChips = 8;
  uint32_t bufferT[MaxChips][4096];
  uint32_t bufferS[MaxChips][2048];
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
	fread(bufferT[chip], sizeof(uint32_t), 4096, ftrans);
      fread(&statusS[chip], sizeof(int), 1, fscope);
      if (statusS[chip] > 0)
	fread(bufferS[chip], sizeof(uint32_t), statusS[chip], fscope);
    }

    for (int chip = 1; chip <= 7; chip += 6) {
      if (chip == 1 || chip == 7) {
	bool trans_all_ff = false;
	int trans_found = 0;
	uint32_t pattern = 0;
	uint32_t* data  = bufferT[chip];
	uint32_t* datae = data + 1023;
	std::cout << "-----------------------------------------\n";
	std::cout << "Contents of transparent FIFO for chip = " << chip << std::endl;
	std::cout << "-----------------------------------------\n";
	if (*data == 0xffffffff && *datae == 0xffffffff) {
	  int nbeg = 0, nend = 0;
	  while (*data == 0xffffffff && data != datae)
	    ++nbeg, ++data;
	  if (data == datae) {
	    trans_all_ff = true;
	    std::cout << "all 0xFFFFFFFF" << std::endl;
	  }
	  else {
	    std::vector<char> bits[2]; // ha
	    while (*datae == 0xffffffff)
	      ++nend, --datae;
	    trans_found = datae-data+1;
	    std::cout << nbeg << " 0xFFFFFFFF then " << trans_found << " words:" << std::endl;
	    while (data != datae + 1) {
	      uint32_t d = *data;
	      uint16_t h(d >> 16);
	      uint16_t l(d & 0xFFFF);
	      uint16_t ab[2] = {h, l};
	      std::cout << std::hex << std::setw(4) << h << std::dec << " ";
	      std::cout << std::hex << std::setw(4) << l << std::dec << "  ";
	      for (int j = 0; j < 2; ++j) {
		for (int i = 15; i >= 0; --i) {
		  char bit = (ab[j] & (1 << i)) ? '1' : '0';
		  bits[!j].push_back(bit);
		  std::cout << bit;
		  if (i % 4 == 0) std::cout << " ";
		}
		std::cout << "  ";
	      }
	      std::cout << std::endl;
	      ++data;
	    }
	    std::cout << "then " << nend << " 0xFFFFFFFF" << std::endl;

	    std::cout << "try to align with headers:\n";
	    const int nroccands = 8;
	    for (int j = 0; j < 2; ++j) {
	      std::cout << "tbm " << j << ":\n";
	      int besttbmhead = -1;
	      int besttbmheadcount = -1;
	      int besttbmtrail = -1;
	      int besttbmtrailcount = -1;
	      std::vector<int> bestroc(nroccands, -1);
	      std::vector<int> bestroccount(nroccands, -1);
	      std::vector<int> bestrocalign(nroccands, -1);
	      int count = -1;
	      const int nbits = bits[j].size();
	      if (nbits < 12)
		std::cout << "not enough bits\n";
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

		std::cout << "best match of tbm header  at " << std::setw(4) << nbeg*16 + besttbmhead  << " with count " << besttbmheadcount << "\n";
		std::cout << "best match of tbm trailer at " << std::setw(4) << nbeg*16 + besttbmtrail << " with count " << besttbmtrailcount << "\n";
		if ((besttbmtrail - (besttbmhead + 12 + 16)) % 12 != 0)
		  std::cout << "  ^ tbm trailer misaligned wrt tbm header!\n";
		std::cout << "matches of roc headers:\n";
		int bestroccountsum = 0;
		for (int k = 0; k < nroccands; ++k) {
		  std::cout << "  at " << std::setw(4) << nbeg*16 + bestroc[k] << " with count " << bestroccount[k] << "\n";
		  if (k < 8) {
		    bestroccountsum += bestroccount[k];
		    if ((bestroc[k] - (besttbmhead + 12 + 16)) % 12 != 0)
		      std::cout << "    ^ roc header misaligned wrt tbm header!\n";
		  }
		}

		//		  std::vector<std::pair<int, int> > roccands;
		//		  for (int k = 0; k < nroccands; ++k) {
		//		    if (bestroccount[k] == 10) {
		//		      roccands.push_back(std::make_pair(
		//		  }
		  

		if (besttbmheadcount != 12 || besttbmtrailcount != 12 || bestroccountsum != 80)
		  std::cout << "problem with headers or trailers!\n";

		std::cout << "print, aligning only with tbm header, and guessing where roc headers and hit bits should be based on " << maxNumHitsPerROC << " hits / roc in calib\n";
		std::cout << "throw away: ";
		for (int i = 0; i < besttbmhead; ++i) {
		  std::cout << bits[j][i];
		  if (i % 4 == 3) std::cout << " ";
		}
		std::cout << "\n";

		std::cout << "tbm header: ";
		for (int i = besttbmhead; i < besttbmhead+12; ++i) {
		  std::cout << bits[j][i];
		}
		std::cout << "  payload: ";
		for (int i = besttbmhead+12; i < besttbmhead+12+2*8; ++i) {
		  std::cout << bits[j][i];
		  const int id = i-(besttbmhead+12);
		  if (id == 7 || id == 9) std::cout << " ";
		}
		std::cout << "\n";

		const int nhitsperroc = maxNumHitsPerROC;
		const int nbitsperroc = 12 + 3*8*nhitsperroc;
		for (int k = 0; k < 8; ++k) {
		  const int ib = besttbmhead+12+2*8 + nbitsperroc*k;
		  const int ic = besttbmhead+12+2*8 + nbitsperroc*k + 12;
		  const int ie = besttbmhead+12+2*8 + nbitsperroc*(k+1);
		  std::cout << "roc " << k << " header: ";
		  for (int i = ib; i < ic; ++i) {
		    std::cout << bits[j][i];
		    if ((i-ib) == 9) std::cout << " ";
		  }
		  std::cout << "\nhits:\n";
		  for (int i = ic; i < ie; ++i) {
		    std::cout << bits[j][i];
		    const int id = (i - ic) % 24;
		    if (id == 5 || id == 14 || id == 18 || id == 19) std::cout << " ";
		    else if (id == 23) std::cout << "\n";
		  }
		}

		std::cout << "tbm trailer: ";
		{
		  const int ib = besttbmhead+12+2*8 + nbitsperroc*8;
		  const int ie = besttbmhead+12+2*8 + nbitsperroc*8 + 12;
		  for (int i = ib; i < ie; ++i)
		    std::cout << bits[j][i];
		}
		std::cout << "  payload: ";
		{
		  const int ib = besttbmhead+12+2*8 + nbitsperroc*8 + 12;
		  const int ie = besttbmhead+12+2*8 + nbitsperroc*8 + 12 + 16;
		  for (int i = ib; i < ie; ++i) {
		    std::cout << bits[j][i];
		    if ((i-ib) % 4 == 3) std::cout << " ";
		  }
		}
		std::cout << std::endl;
	      }
	    }
	  }
	}
	else {
	  pattern = *((uint32_t*)data);
	  bool same = true;
	  while (data != datae + 1) {
	    uint32_t p = *((uint32_t*)data);
	    if (p != pattern)
	      same = false;
	    data += 4;
	  }
	  if (same)
	    std::cout << "1024 repetitions of " << std::hex << pattern << std::dec << std::endl;
	  else {
	    uint8_t* data8 = (uint8_t*)bufferT[chip];
	    std::cout << "rw | ";
	    for (int j = 0; j < 64; ++j)
	      std::cout << std::setw(2) << j << " ";
	    std::cout << std::endl;
	    for (int i = 0; i < 64; ++i) {
	      std::cout << std::setw(2) << i << " | ";
	      for (int j = 0; j < 64; ++j)
		std::cout << std::hex << std::setw(2) << unsigned(data8[i*64+j]) << std::dec << " ";
	      std::cout << std::endl;
	    }
	  }
	}

	std::cout << "DigTransDecoder thinks:\n";
	decodeT[chip]->printToStream(std::cout);
      }
      //if (chip != 1 && chip != 7 && !trans_all_ff)
      //  std::cout << "bad trans_all_ff: chip is " << chip << std::endl;

      std::cout << "----------------------------------" << std::endl;
      if (statusS[chip] < 0)
	std::cout << "Scope FIFO for chip = " << chip << " status = " << statusS[chip] << std::endl;
      else {
	std::cout << "Contents of Scope FIFO for chip = " << chip << "(statusS = " << statusS[chip] << ")" <<std::endl;
	std::cout << "----------------------------------" << std::endl;
	for (int i = 0; i <= statusS[chip]; ++i) {
	  uint32_t d = bufferS[chip][i];
	  uint32_t dh = d & 0xf0;
	  if (dh == 0x70 || dh == 0x10 || dh == 0xc0)
	    std::cout << "\n";
	  if (d > 0xFF)
	    std::cout << "\nweird word: " << std::hex << d << "\n";
	  else 
	    std::cout << std::setw(2) << std::hex << d << std::dec << " ";
	  if ((dh >> 4) >= 1 && (dh >> 4) <= 5)
	    std::cout << " hello!\n";
	}
	std::cout << "\n----------------------------------" << std::endl;
      }
      std::cout << "DigScopeDecoder thinks:\n";
      decodeS[chip]->printToStream(std::cout);
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
