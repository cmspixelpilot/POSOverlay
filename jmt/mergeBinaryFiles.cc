#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstdint>

int main(int argc, char** argv) {
  if (argc < 3) {
    fprintf(stderr, "need at least two input files\n");
    exit(1);
  }

  const char* out_fn = "merge.dmp";

  if (access(out_fn, F_OK) != -1) {
    fprintf(stderr, "refusing to clobber %s\n", out_fn);
    exit(1);
  }

  FILE* fout = fopen(out_fn, "wb");
  uint64_t run = 999999;
  fwrite(&run, sizeof(uint64_t), 1, fout);

  for (int i = 1; i < argc; ++i) {
    printf("reading %s\n", argv[i]);
    FILE* f = fopen(argv[i], "rb");
    if (!f) {
      fprintf(stderr, "problem reading\n");
      exit(1);
    }

    fread(&run, sizeof(uint64_t), 1, f); // throw away the run numbers

    size_t n, m;
    unsigned char buf[8192];
    do {
      n = fread(buf, 1, sizeof(buf), f);
      if (n)
	m = fwrite(buf, 1, n, fout);
      else
	m = 0;
    }
    while (n > 0 && n == m);

    if (m) {
      fprintf(stderr, "problem writing everything\n");
      exit(1);
    }

    fclose(f);
  }

  fclose(fout);
}
