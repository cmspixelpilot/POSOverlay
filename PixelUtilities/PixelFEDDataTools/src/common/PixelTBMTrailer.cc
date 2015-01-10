#include "PixelUtilities/PixelFEDDataTools/include/PixelTBMTrailer.h"

PixelTBMTrailer::PixelTBMTrailer()
   : PixelFEDRawDataNtuple(8)
{
}

void PixelTBMTrailer::printToStream(std::ostream& out)
{
	out << "--- TBM Trailer ---\n";
	printPointsToStream(out);
	return;
}
