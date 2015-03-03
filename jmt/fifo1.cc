#include <string.h>
#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <vector>
#include <stdint.h>

#include "PixelUtilities/PixelFEDDataTools/include/FIFO1DigDecoder.h"

int main() {

  uint32_t buf[1024];
  memset(buf, 0xFF, 4096);

#if 0  
  buf[0]  =   0xffffffff;
  buf[1]  =   0xfbfef7fc;
  buf[2]  =   0x00a00140;
  buf[3]  =   0x3fd07fa1;
  buf[4]  =   0xaeac5d58;
  buf[5]  =   0x33fda7fa;
  buf[6]  =   0x0aea05d5;
  buf[7]  =   0xd73f627f;
  buf[8]  =   0xd0aea15d;
  buf[9]  =   0xad135a27;
  buf[10] =   0xfd0afa15;
  buf[11] =   0xeab7d563;
  buf[12] =   0x3fd03f80;
  buf[13] =   0xaeab0c08;
  buf[14] =   0x5bfd03f8;
  buf[15] =   0x0aea00c0;
  buf[16] =   0xc4bf803f;
  buf[17] =   0xd0ae800c;
  buf[18] =   0xab1b0823;
  buf[19] =   0xfc3ff800;
  buf[20] =   0xf310c080;
  buf[21] =   0x07ff7fe6;
  buf[22] =   0xffff200f;
  buf[23] =   0xffffffff;
#endif

  buf[0]  =   0xffffffff;
  buf[1]  =   0xfdfffbfe;
  buf[2]  =   0x006c00d8;
  buf[3]  =   0x1fe03fc0;
  buf[4]  =   0x5756aeac;
  buf[5]  =   0x21fe63fc;
  buf[6]  =   0x05750aea;
  buf[7]  =   0x6b9fb13f;
  buf[8]  =   0xe057c0ae;
  buf[9]  =   0x5689ad13;
  buf[10] =   0xfe05fc0a;
  buf[11] =   0x755beab2;
  buf[12] =   0x9fe03fc0;
  buf[13] =   0x5755aeac;
  buf[14] =   0xa9fe53fc;
  buf[15] =   0x05750aea;
  buf[16] =   0x625fd13f;
  buf[17] =   0xe057c0ae;
  buf[18] =   0x5591ac2b;
  buf[19] =   0xfe5ffc0a;
  buf[20] =   0xf988eab7;
  buf[21] =   0x03ffbff3;
  buf[22] =   0xffff1007;
  buf[23] =   0xffffffff;

  std::cout << "FIFO1DigDecoder thinks:\n";
  FIFO1DigDecoder dec1(buf);
  dec1.printToStream(std::cout);
}
























