#include <cassert>
#include <cstdio>
#include <cstdint>

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s file.ld\n", argv[0]);
    return 1;
  }

  FILE* f = fopen(argv[1], "rb");
  if (!f) {
    fprintf(stderr, "cannot open %s\n", argv[1]);
    return 1;
  }

  const uint32_t channel_mask = 0xfc000000;
  const uint32_t htmark_mask  = 0x03e00000;
  const uint32_t tbmdata_mask = 0x0001fe00;
  const uint32_t tbmevt_mask  = 0x000000ff;
  const uint32_t errort_mask  = 0x001ff000;

  int autoreset_c = 0;
  int pkamreset_c = 0;

  while (!feof(f)) {
    uint32_t d;
    if (fread(&d, 4, 1, f) == 1) {
      for (int i = 24; i >= 0; i -= 8)
	printf("%02x ", (d & (0xff << i)) >> i);

      printf(" ");

      for (int i = 31; i >= 0; --i) {
	printf("%i", (d & (1 << i)) ? 1 : 0);
	if (i % 4 == 0)
	  printf(" ");
      }

      const uint32_t channel = (d & channel_mask) >> 26;
      printf("ch %2i ", channel);

      const uint32_t htmark = (d & htmark_mask) >> 21;
      if (htmark == 31) {
	printf("header ");
	//assert((d & 0x1e0100) == 0);
	const uint32_t tbmdata = (d & tbmdata_mask) >> 9;
	const uint32_t tbmevt = d & tbmevt_mask;
	printf("data %02x evt %02x ", tbmdata, tbmevt);
      }
      else if (htmark == 30) {
	if (d & 0x100) {
	  printf("error trailer ");
	}
	else {
	  printf("trailer ");
	  //assert((d & 0x101) == 0);
	  const uint32_t tbm_autoreset = (d & (1<<19)) >> 19;
	  const uint32_t tbm_pkamreset = (d & (1<<18)) >> 18;
	  const uint32_t tbm_stackcount = (d & (0x3f << 12)) >> 12;
	  const uint32_t num_rocs_err = (d & (1<<11)) >> 11;
	  const uint32_t tbm_status = d & 0xff;
	  if (tbm_autoreset) {
	    printf("autoreset ");
	    ++autoreset_c;
	  }
	  if (tbm_pkamreset) {
	    printf("pkamreset ");
	    ++pkamreset_c;
	  }
	  if (num_rocs_err)
	    printf("num_rocs_err ");
	  printf("stackcount %2i tbm status %02x ", tbm_stackcount, tbm_status);
	}
      }
      
      printf("\n");
    }
  }

  printf("# auto: %i   # pkam: %i\n", autoreset_c, pkamreset_c);
}
