#include "PixelUtilities/PixelFEDDataTools/include/PixelBlackData.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"

PixelBlackData::PixelBlackData()
{
	startPosition_ = 1000000;
}

void PixelBlackData::printSummaryToStream(std::ostream& out)
{
	Moments moment = getMoments();
	out << "Black data: " << numPoints() << " points, mean = " << moment.mean() << ", stddev = " << moment.stddev() << std::endl;
	return;
}
