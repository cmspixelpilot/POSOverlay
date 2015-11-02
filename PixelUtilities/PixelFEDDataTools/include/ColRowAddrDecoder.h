#ifndef ColRowAddrDecoder_h
#define ColRowAddrDecoder_h

struct ColRowAddrDecoder {
  int dcoldecode(int dc) {
    assert(dc >= 0 && dc <= 33 &&
	   dc != 6 && 
	   dc != 7 && 
	   dc != 14 && 
	   dc != 15 && 
	   dc != 22 && 
	   dc != 23 && 
	   dc != 30 && 
	   dc != 31);

    static const int X[34] = { 0, 1, 2, 3, 4, 5, 0, 0,
			       6, 7, 8, 9, 10, 11, 0, 0,
			       12, 13, 14, 15, 16, 17, 0, 0,
			       18, 19, 20, 21, 22, 23, 0, 0,
			       24, 25 };
    return X[dc];
  }

  int dcolencode(int dc) {
    assert(dc >= 0 && dc <= 25);
    static const int X[26] = { 0, 1, 2, 3, 4, 5,
			       8, 9, 10, 11, 12, 13,
			       16, 17, 18, 19, 20, 21,
			       24, 25, 26, 27, 28, 29,
			       32, 33 };
    return X[dc];
  }

  int rowdecode(int row) {
    return -1;
  }

  int rowencode(int row) {
    return -1;
  }

  std::pair<int, int> colrowdecode(std::pair<int, int> colrow) {
    const int dcol = dcoldecode(colrow.first);
    const int col = dcol*2 + colrow.second % 2;
    return std::make_pair(col, rowdecode(colrow.second));
  }

  std::pair<int, int> colrowencode(std::pair<int, int> colrow) {
    const int dcol = colrow.first / 2;
    return std::make_pair(dcolencode(colrow.first / 2), rowencode(colrow.second));
  }
};

#endif
