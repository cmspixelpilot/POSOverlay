/*
g++ -g $(root-config --cflags --glibs) -I.. \
../PixelCalibrations/src/common/PixelIanaAnalysis.cc redo_iana.cc \
-o redo_iana.exe
*/

#include "PixelCalibrations/include/PixelIanaAnalysis.h"
#include <cstdio>

int main(int argc, char** argv) {
  if (argc < 4) {
    fprintf(stderr, "usage: redo_iana.exe datfile roc outfile\n");
    return 1;
  }

  const char* in_fn = argv[1];
  const char* roc = argv[2];
  const char* out_fn = argv[3];
  PixelIanaAnalysis a(true);
  a.redoFromDat(in_fn, roc, std::cout, out_fn);
  printf("complicated fit: %i  linear fit: %i  diff: %i\n", int(a.oldVana), int(a.newVana), int(a.newVana) - int(a.oldVana));
}
