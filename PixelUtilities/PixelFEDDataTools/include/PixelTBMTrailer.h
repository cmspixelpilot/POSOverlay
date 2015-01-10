
#ifndef _PixelTBMTrailer_h_
#define _PixelTBMTrailer_h_

#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDRawDataNtuple.h"

#include <iostream>

class PixelTBMTrailer : public PixelFEDRawDataNtuple
{
	public:

	PixelTBMTrailer();

	void setUB1(unsigned int value) {setPoint(1, value);}
	void setUB2(unsigned int value) {setPoint(2, value);}
	void setB1(unsigned int value) {setPoint(3, value);}
	void setB2(unsigned int value) {setPoint(4, value);}
	void setExtraInfo1(unsigned int value) {setPoint(5, value);}
	void setExtraInfo2(unsigned int value) {setPoint(6, value);}
	void setExtraInfo3(unsigned int value) {setPoint(7, value);}
	void setExtraInfo4(unsigned int value) {setPoint(8, value);}

	unsigned int UB1() {return point(1);}
	unsigned int UB2() {return point(2);}
	unsigned int B1() {return point(3);}
	unsigned int B2() {return point(4);}
	unsigned int extraInfo1() {return point(5);}
	unsigned int extraInfo2() {return point(6);}
	unsigned int extraInfo3() {return point(7);}
	unsigned int extraInfo4() {return point(8);}

	void printToStream(std::ostream& out);

	private:

};

#endif




