#ifndef PIXELFIFODATACOLLECTION_H_
#define PIXELFIFODATACOLLECTION_H_

namespace pixel
{

class PixelFifoDataCollection
{
public:
	PixelFifoDataCollection();
	virtual ~PixelFifoDataCollection();
	
	unsigned long data1[4112];
};

}

#endif /*PIXELFIFODATACOLLECTION_H_*/
