
#ifndef _PixelROCOutput_h_
#define _PixelROCOutput_h_

#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDRawDataNtuple.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelROCHeader.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelHitFEDRawData.h"

#include <vector>
#include <iostream>

// This class stores the FED raw data produced by a single ROC.

class PixelROCOutput
{
	public:

	PixelROCHeader& header() {return header_;} // returns by reference to allow modification of the PixelROCHeader object

	void addHit( const PixelHitFEDRawData& newHit ) { hits_.push_back(newHit); }

	// Hit numbering starts from 1.
	PixelHitFEDRawData hit(unsigned int hitNumber) {assert(0 < hitNumber && hitNumber <= numHits()); return hits_[hitNumber-1];}
	unsigned int numHits() {return hits_.size();}

	int startPosition() {return header_.startPosition();}

	void printToStream(std::ostream& out);

	private:
	PixelROCHeader header_;
	std::vector<PixelHitFEDRawData> hits_;

};

#endif




