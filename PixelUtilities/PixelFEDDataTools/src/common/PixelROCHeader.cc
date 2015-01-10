#include "PixelUtilities/PixelFEDDataTools/include/PixelROCHeader.h"

PixelROCHeader::PixelROCHeader()
   : PixelFEDRawDataNtuple(3)
{
}


void PixelROCHeader::printToStream(std::ostream& out)
{
	out << "--- ROC Header ---\n";
	printPointsToStream(out);
	return;
}
