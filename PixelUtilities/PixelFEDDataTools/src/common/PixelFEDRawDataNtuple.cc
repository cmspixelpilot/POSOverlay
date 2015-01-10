#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDRawDataNtuple.h"
#include <iomanip>

PixelFEDRawDataNtuple::PixelFEDRawDataNtuple(unsigned int numPoints)
{
	startPosition_ = 1000000;
	for ( unsigned int i=0; i < numPoints; i++)
	{
		points_.push_back(0);
		isSet_.push_back(false);
	}
}

void PixelFEDRawDataNtuple::printPointsToStream(std::ostream& out)
{
	for(unsigned int i = 0; i < points_.size(); i++)
	{
		out << std::setw(3) << startPosition_ + i << " : ";
		if (isSet_[i]) out << std::setw(4) << points_[i];
		else           out << "no info";
		out << std::endl;
	}
	return;
}
