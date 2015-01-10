
#ifndef _PixelBlackData_h_
#define _PixelBlackData_h_

#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"

#include <vector>
#include <iostream>

class PixelBlackData
{
	public:

	PixelBlackData();

	void addPoint(unsigned int newPoint) {points_.push_back(newPoint); moments_.push_back(newPoint);}

	unsigned int point(unsigned int position) {return points_[position];}
	unsigned int numPoints() {return points_.size();}

	Moments getMoments() {return moments_;}
	double mean() {return moments_.mean();}

	void setStartPosition(unsigned int startPosition) {startPosition_=startPosition;}
	int startPosition() {return startPosition_;}

	void printSummaryToStream(std::ostream& out);

	private:

	// decoded info
	std::vector<unsigned int> points_;
	int startPosition_;                  // slot in which this Ntuple began (0-255), -1 if not specified
	Moments moments_;
};

#endif


