
#ifndef _PixelTBMHeader_h_
#define _PixelTBMHeader_h_

#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDRawDataNtuple.h"

#include <iostream>

class PixelTBMHeader : public PixelFEDRawDataNtuple
{
	public:

	PixelTBMHeader();

	void setUB1(unsigned int value) {setPoint(1, value);}
	void setUB2(unsigned int value) {setPoint(2, value);}
	void setUB3(unsigned int value) {setPoint(3, value);}
	void setB(unsigned int value) {setPoint(4, value);}
	void setEventCounter1(unsigned int value) {setPoint(5, value);}
	void setEventCounter2(unsigned int value) {setPoint(6, value);}
	void setEventCounter3(unsigned int value) {setPoint(7, value);}
	void setEventCounter4(unsigned int value) {setPoint(8, value);}

	unsigned int UB1() {return point(1);}
	unsigned int UB2() {return point(2);}
	unsigned int UB3() {return point(3);}
	unsigned int B() {return point(4);}
	unsigned int eventCounter1() {return point(5);}
	unsigned int eventCounter2() {return point(6);}
	unsigned int eventCounter3() {return point(7);}
	unsigned int eventCounter4() {return point(8);}

	void printToStream(std::ostream& out);

	private:

};

#endif



