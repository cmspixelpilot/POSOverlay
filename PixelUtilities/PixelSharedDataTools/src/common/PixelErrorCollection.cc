#include "PixelErrorCollection.h"

namespace pixel
{

const unsigned int PixelErrorCollection::size = 8*pos::errorDepth;
PixelErrorCollection::PixelErrorCollection()
{
	// Initialize error count to zero
	_errorCount = 0;	
}

PixelErrorCollection::~PixelErrorCollection()
{
}

void PixelErrorCollection::setErrors(uint32_t data[],unsigned int count) {
	this->_errorCount = count;
	for(unsigned int i = 0; i<getErrorCount(); i++) {
		errors[i] = pos::PixelError(data[i]);
	}
}
void PixelErrorCollection::setErrors(char data[], unsigned int count) {
	this->_errorCount = count;
	for(unsigned int i = 0; i<getErrorCount(); i++) {
		errors[i] = pos::PixelError(static_cast<unsigned long>(data[i]));
	}
}
unsigned int PixelErrorCollection::getErrorCount() {
	return _errorCount;
}

void PixelErrorCollection::setErrorCount(int count) {
	_errorCount = count;
}

}
