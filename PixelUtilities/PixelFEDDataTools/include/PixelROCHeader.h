
#ifndef _PixelROCHeader_h_
#define _PixelROCHeader_h_

#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDRawDataNtuple.h"

#include <iostream>

class PixelROCHeader : public PixelFEDRawDataNtuple
{
	public:

	PixelROCHeader();

	void setUB(unsigned int value) {setPoint(1, value);}
	void setB(unsigned int value) {setPoint(2, value);}
	void setLastDAC(unsigned int value) {setPoint(3, value);}

	unsigned int UB() {return point(1);}
	unsigned int B() {return point(2);}
	unsigned int lastDAC() {return point(3);}

	void printToStream(std::ostream& out);

	private:

};

#endif



