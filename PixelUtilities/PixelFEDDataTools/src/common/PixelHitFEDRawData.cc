#include "PixelUtilities/PixelFEDDataTools/include/PixelHitFEDRawData.h"

PixelHitFEDRawData::PixelHitFEDRawData()
   : PixelFEDRawDataNtuple(6)
{
}

void PixelHitFEDRawData::printToStream(std::ostream& out)
{
	out << "--- Hit ---\n";
	printPointsToStream(out);
	return;
}
