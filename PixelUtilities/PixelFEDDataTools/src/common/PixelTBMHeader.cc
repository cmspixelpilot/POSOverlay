#include "PixelUtilities/PixelFEDDataTools/include/PixelTBMHeader.h"

PixelTBMHeader::PixelTBMHeader()
   : PixelFEDRawDataNtuple(8)
{
}

void PixelTBMHeader::printToStream(std::ostream& out)
{
	out << "--- TBM Header ---\n";
	printPointsToStream(out);
	return;
}
