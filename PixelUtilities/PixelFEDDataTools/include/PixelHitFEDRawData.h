
#ifndef _PixelHitFEDRawData_h_
#define _PixelHitFEDRawData_h_

#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDRawDataNtuple.h"

#include <iostream>

class PixelHitFEDRawData : public PixelFEDRawDataNtuple
{
	public:

	PixelHitFEDRawData();

	void setDCol1(unsigned int value) {setPoint(1, value);}
	void setDCol2(unsigned int value) {setPoint(2, value);}
	void setRow1(unsigned int value) {setPoint(3, value);}
	void setRow2(unsigned int value) {setPoint(4, value);}
	void setRow3(unsigned int value) {setPoint(5, value);}
	void setPulseHeight(unsigned int value) {setPoint(6, value);}

	unsigned int DCol1() {return point(1);}
	unsigned int DCol2() {return point(2);}
	unsigned int Row1() {return point(3);}
	unsigned int Row2() {return point(4);}
	unsigned int Row3() {return point(5);}
	unsigned int pulseHeight() {return point(6);}

	void printToStream(std::ostream& out);

	private:

};

#endif



