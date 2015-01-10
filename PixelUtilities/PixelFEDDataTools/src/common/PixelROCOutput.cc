#include "PixelUtilities/PixelFEDDataTools/include/PixelROCOutput.h"

void PixelROCOutput::printToStream(std::ostream& out)
{
	out << "--- ROC Output ---\n";
	header_.printToStream(out);
	for (unsigned int i = 0; i < hits_.size(); i++)
	{
		out << "--- Hit # " << i+1 << " ---\n";
		hits_[i].printPointsToStream(out);
	}
	return;
}
