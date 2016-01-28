#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <vector>
#include <stdint.h>


class FIFO3Decoder
{
public:
    
  FIFO3Decoder(uint64_t *buffer);

  unsigned int nhits() const {return hits_.size();}

  unsigned int channel(unsigned int ihit) const {return (hits_[ihit]>>26)&0x3f;}
  unsigned int rocid(unsigned int ihit) const {return (hits_[ihit]>>21)&0x1f;}
    
  unsigned int dcol(unsigned int ihit) const {return (hits_[ihit]>>16)&0x1f;}
  unsigned int pxl(unsigned int ihit) const {return (hits_[ihit]>>8)&0xff;}
  unsigned int pulseheight(unsigned int ihit) const {return (hits_[ihit]>>0)&0xff;}
    
  unsigned int column(unsigned int ihit) const {return dcol(ihit)*2 + pxl(ihit)%2;}
  unsigned int row(unsigned int ihit) const {return 80 - (pxl(ihit)/2);}


private:
  std::vector<unsigned int> hits_;

};

FIFO3Decoder::FIFO3Decoder(uint64_t *buffer)
{

  unsigned int *buf=(unsigned int *)buffer;

  //std::cout << "buf[0]=0x"<<std::hex<<buf[0]<<std::dec<<std::endl;

  assert((buffer[0]>>60)==0x5);
  
  unsigned int counter=1;

  while ((buffer[counter]>>60)!=0xa){

    for(unsigned int index=counter*2;index<counter*2+2;index++){


      //unsigned int channel=(buf[index]>>26)&0x3f;
      unsigned int rocid=(buf[index]>>21)&0x1f;
      printf("counter %3u index %3u buf[index] = %8x rocid %2u\n", counter, index, buf[index], rocid);
    

      if (rocid<25) hits_.push_back(buf[index]);
      if (rocid>27&&(rocid!=30)) {
        //FIXME Code should set some error FLAG but can not print message here
        //std::cout << "buf="<<std::hex<<buf[index]<<std::dec<<" channel="<<channel<<" rocid="<<rocid<<std::endl;
      }

    }

    counter++;

    //FIXME what should this check actually be???

    if (counter>7000) {
      std::cout << "counter="<<counter<<" will exit loop!"<<std::endl;
      break;
    }
    
  }  

}


int main() {
#if 0
  const int n = 46;
  uint64_t buffer[n] = {
0x500000010cf01b00,
0x07c408b807e01800,
0x0bc408b80be01800,
0x03a0080403b00000,
0x03a0080803b00000,
0x0360000103600001,
0x0360000103600001,
0x0360000103600001,
0x1c258cc21fe01805,
0x1c658ccf1c458cde,
0x1ca58cde1c858cce,
0x1ce58cbe1cc58ccf,
0x23e018051d058caf,
0x20458cc220258cd2,
0x20858cbf20658cb6,
0x20c58cb620a58cba,
0x21058cc820e58cb4,
0x27c408b827e01800,
0x2bc408b82be01800,
0x0360000103600001,
0x0360000103600001,
0x0360000103600001,
0x0360000103600001,
0x0360000103600001,
0x0360000103600001,
0x0360000103600001,
0x0360000103600001,
0x0360000103600001,
0x03a00d0403b00000,
0x03a00d0803b00000,
0x03a00d1003b00000,
0x0360000103600001,
0x0360000103600001,
0x0360000103600001,
0x03a00e0103b00000,
0x03a00e0803b00000,
0x0360000103600001,
0x0360000103600001,
0x0360000103600001,
0x03a00f0103b00000,
0x03a00f0203b00000,
0x03a00f0403b00000,
0x0360000103600001,
0x0360000103600001,
0x0360000103600001,
0xa000002e7ae20080
  };
#endif

#if 0
|||||SLID|EVTY|--Event#--|--BunchC#--|--SourceID--|FOV|FH|
       0:|  5 |  0 |       1  |       cf  |        1b  |  0| 0| 50000001           cf01b00
|||||----|----|----------|-----------|------------|---|--|


|||||CH|RO|DC|PXL|PH| S-Link |CH|RO|DC|PXL|PH|||||
       1:   1 30  4   8 b8          1 31  0  24  0               7c408b8           7e01800
       2:   2 30  4   8 b8          2 31  0  24  0               bc408b8           be01800
       3:   0 29  0   8  4          0 29 16   0  0               3a00804           3b00000
       4:   0 29  0   8  8          0 29 16   0  0               3a00808           3b00000
       5:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       6:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       7:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       8:   7  1  5 140 c2          7 31  0  24  5              1c258cc2          1fe01805
       9:   7  3  5 140 cf          7  2  5 140 de              1c658ccf          1c458cde
       10:   7  5  5 140 de          7  4  5 140 ce              1ca58cde          1c858cce
       11:   7  7  5 140 be          7  6  5 140 cf              1ce58cbe          1cc58ccf
       12:   8 31  0  24  5          7  8  5 140 af              23e01805          1d058caf
       13:   8  2  5 140 c2          8  1  5 140 d2              20458cc2          20258cd2
       14:   8  4  5 140 bf          8  3  5 140 b6              20858cbf          20658cb6
       15:   8  6  5 140 b6          8  5  5 140 ba              20c58cb6          20a58cba
       16:   8  8  5 140 c8          8  7  5 140 b4              21058cc8          20e58cb4
       17:   9 30  4   8 b8          9 31  0  24  0              27c408b8          27e01800
       18:  10 30  4   8 b8         10 31  0  24  0              2bc408b8          2be01800
       19:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       20:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       21:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       22:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       23:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       24:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       25:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       26:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       27:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       28:   0 29  0  13  4          0 29 16   0  0               3a00d04           3b00000
       29:   0 29  0  13  8          0 29 16   0  0               3a00d08           3b00000
       30:   0 29  0  13 10          0 29 16   0  0               3a00d10           3b00000
       31:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       32:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       33:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       34:   0 29  0  14  1          0 29 16   0  0               3a00e01           3b00000
       35:   0 29  0  14  8          0 29 16   0  0               3a00e08           3b00000
       36:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       37:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       38:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       39:   0 29  0  15  1          0 29 16   0  0               3a00f01           3b00000
       40:   0 29  0  15  2          0 29 16   0  0               3a00f02           3b00000
       41:   0 29  0  15  4          0 29 16   0  0               3a00f04           3b00000
       42:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       43:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       44:   0 27  0   0  1          0 27  0   0  1               3600001           3600001
       45:  40  0  0   0 2e         30 23  2   0 80              a000002e          7ae20080
#endif

       const int n = 33;
  uint64_t buffer[n] = {
0x500000010cf02800,
0x360000103600001,
0x360000103600001,
0x360000103600001,
0x1c4298d81c2298c2,
0x1c8298c01c6298cf,
0x1cc298cf1ca298d8,
0x1d0298ab1ce298be,
0x1fcff0ff1d192318,
0x204298c3202298c6,
0x208298bb206298be,
0x20c298aa20a298b8,
0x2119231820e298b3,
0x340000123cff0ff,
0x360000103600001,
0x360000103600001,
0x360000103600001,
0x360000103600001,
0x360000103600001,
0x360000103600001,
0x360000103600001,
0x360000103600001,
0x360000103600001,
0x360000103600001,
0x360000103600001,
0x360000103600001,
0x360000103600001,
0x360000103600001,
0x360000103600001,
0x360000103600001,
0x360000103600001,
0x360000103600001,
0xa0000021e4150080
  };

  for (int i = 0; i < n; ++i)
    printf("%2i: %8llx %8llx\n", i, buffer[i]>>32, buffer[i]&0xFFFFFFFF);

  FIFO3Decoder decode3(buffer);
      std::cout << "FIFO3Decoder thinks:\n"
		<< "nhits: " << decode3.nhits() << "\n";
      for (unsigned i = 0; i < decode3.nhits(); ++i)
	std::cout << "#" << i << ": ch: " << decode3.channel(i)
		  << " rocid: " << decode3.rocid(i) << " dcol: " << decode3.dcol(i)
		  << " pxl: " << decode3.pxl(i) << " pulseheight: " << decode3.pulseheight(i)
		  << " col: " << decode3.column(i) << " row: " << decode3.row(i) << std::endl;


}
