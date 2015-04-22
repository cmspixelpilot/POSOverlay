#ifndef PIXELERRORCOLLECTION_H_
#define PIXELERRORCOLLECTION_H_

#include "PixelUtilities/PixelFEDDataTools/include/PixelError.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"

#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"

namespace pixel
{

class PixelErrorCollection
{
public:
	PixelErrorCollection();
	virtual ~PixelErrorCollection();
	
	void setErrors(uint32_t data[],unsigned int count);
	void setErrors(char data[],unsigned int count);
	unsigned int getErrorCount();
	void setErrorCount(int count);
	
	pos::PixelError errors[8*pos::errorDepth];
	static const unsigned int size;
private:
	int _errorCount;
};

}

#endif /*PIXELERRORCOLLECTION_H_*/
