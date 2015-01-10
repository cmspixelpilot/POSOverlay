
#ifndef _PixelFEDRawDataNtuple_h_
#define _PixelFEDRawDataNtuple_h_

#include <vector>
#include <iostream>
#include <assert.h>

// Generic base class for a sequence of FED raw data -- for example, TBM header, TBM event number, ROC header, TBM trailer

class PixelFEDRawDataNtuple
{
	public:

	PixelFEDRawDataNtuple(unsigned int numPoints);

	// points specified by an integer from 1 to numPoints
	void setPoint(unsigned int position, unsigned int value) {assert(!isSet_[position-1]); isSet_[position-1] = true;  points_[position-1] = value;}

	unsigned int point(unsigned int position) {assert(isSet_[position-1]); return  points_[position-1];}

	void setStartPosition(unsigned int startPosition) {startPosition_=startPosition;}
	int startPosition() {return startPosition_;}

	// print out slot and raw data
	void printPointsToStream(std::ostream& out);

	private:

	std::vector<unsigned int> points_;   // values
	std::vector<bool> isSet_;            // whether each value has been set
	int startPosition_;                  // slot in which this Ntuple began (0-255), -1 if not specified

};

#endif


