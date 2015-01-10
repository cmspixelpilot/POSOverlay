#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"

#include "PixelUtilities/PixelFEDDataTools/include/PixelDecodedFEDRawData.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelBlackData.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelTBMHeader.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelROCOutput.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelROCHeader.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelHitFEDRawData.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelTBMTrailer.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include <iostream>
#include <iomanip>

#include "TCanvas.h"
#include "TGraph.h"
#include "TH1F.h"

void PixelDecodedFEDRawData::printToStream(std::ostream& out)
{
	if ( badBuffer() )
	{
		out << "Bad buffer, but here's the data anyway.\n";
		for ( unsigned int slot=0; slot < numSlots_; slot++ )
		{
			out << std::setw(3) << slot << " : " << std::setw(4) << adcValue(slot) << std::endl;
		}
	}
	else
	{
		decode();
		
		out << "--- Initial black data ---\n";
		initialBlack_.printSummaryToStream(out);
		TBMHeader_.printToStream(out);
		for (unsigned int i = 0; i < ROCOutput_.size(); i++)
		{
			out << "ROC # " << i << ": ";
			ROCOutput_[i].printToStream(out);
		}
		TBMTrailer_.printToStream(out);
		out << "--- Final black data ---\n";
		finalBlack_.printSummaryToStream(out);
	}
}

void PixelDecodedFEDRawData::drawToFile(std::string filename, std::string filenameshort,
					unsigned int nclock, 
					unsigned int columns, 
					unsigned int rows)
{
	TCanvas* c = new TCanvas((filenameshort+"_Canvas").c_str(),filenameshort.c_str() , columns, rows);


	TH1F h((filenameshort).c_str(), filenameshort.c_str(), nclock, -0.5, (double)nclock-0.5);
	h.SetMinimum(0.0);
	h.SetMaximum(1023.0);
	h.SetLineColor(4);
	for (unsigned int i=0;i<nclock;++i) {
		h.Fill(i, adcValue(i));
	}
	h.Draw();
	
       	//c->Write(filenameshort.c_str());
       	c->Write();

	//c->Print(filename.c_str());

        delete c;

}


void PixelDecodedFEDRawData::drawToFile(std::string filename, 
					unsigned int nclock, 
					unsigned int columns, 
					unsigned int rows)
{
	TCanvas* c = new TCanvas("c", "TBM Signal", columns, rows);


	TH1F h("h", "TBM Signal", nclock, -0.5, (double)nclock-0.5);
	h.SetMinimum(0.0);
	h.SetMaximum(1023.0);
	h.SetLineColor(4);
	for (unsigned int i=0;i<nclock;++i) {
		h.Fill(i, adcValue(i));
	}
	h.Draw();
	
	c->Print(filename.c_str());
}


PixelDecodedFEDRawData::PixelDecodedFEDRawData(UINT32 * buffer, double TBMHeader_UB_minDistance, double TBM_B_maxDelta, double TBM_UB_maxDelta, double ROC_B_offset, double ROC_B_maxDelta, double ROC_UB_offset, double ROC_UB_maxDelta)
{
	initialize_fullDecoding(buffer, TBMHeader_UB_minDistance, TBM_B_maxDelta, TBM_UB_maxDelta, ROC_B_offset, ROC_B_maxDelta, ROC_UB_offset, ROC_UB_maxDelta);
}

PixelDecodedFEDRawData::PixelDecodedFEDRawData(UINT32 * buffer)
{
	initialize_fullDecoding(buffer, 100., 100., 150., 0., 100., 0., 150.);
}

void PixelDecodedFEDRawData::initialize_fullDecoding(UINT32 * buffer, double TBMHeader_UB_minDistance, double TBM_B_maxDelta, double TBM_UB_maxDelta, double ROC_B_offset, double ROC_B_maxDelta, double ROC_UB_offset, double ROC_UB_maxDelta)
{
	TBMHeader_UB_minDistance_ = TBMHeader_UB_minDistance;
	TBM_B_maxDelta_  = TBM_B_maxDelta;
	TBM_UB_maxDelta_ = TBM_UB_maxDelta;
	ROC_B_offset_    = ROC_B_offset;
	ROC_B_maxDelta_  = ROC_B_maxDelta;
	ROC_UB_offset_   = ROC_UB_offset;
	ROC_UB_maxDelta_ = ROC_UB_maxDelta;
	altMethod_=false;
	decoded_ = false;
	assumedROCsAndHits_ = false;
	copyBuffer(buffer);
}

PixelDecodedFEDRawData::PixelDecodedFEDRawData(UINT32 * buffer, double TBMHeader_UB_minDistance, double TBM_B_maxDelta, double TBM_UB_maxDelta, std::vector<unsigned int> assumedHitsOnEachROC)
{
	TBMHeader_UB_minDistance_ = TBMHeader_UB_minDistance;
	TBM_B_maxDelta_  = TBM_B_maxDelta;
	TBM_UB_maxDelta_ = TBM_UB_maxDelta;
	assumedHitsOnEachROC_ = assumedHitsOnEachROC;
	altMethod_=false;
	decoded_ = false;
	assumedROCsAndHits_ = true;
	copyBuffer(buffer);
}

bool PixelDecodedFEDRawData::ROCHeaderStartsInSlot(unsigned int slot)
{
	return is_ROC_UB(adcValue(slot)) && is_ROC_B(adcValue(slot+1));
}

bool PixelDecodedFEDRawData::TBMTrailerStartsInSlot(unsigned int slot)
{
	return is_TBM_UB(adcValue(slot)) && is_TBM_UB(adcValue(slot+1)) && is_TBM_B(adcValue(slot+2)) && is_TBM_B(adcValue(slot+3));
}

void PixelDecodedFEDRawData::copyBuffer(UINT32 * buffer)
{
	for ( unsigned int slot = 0; slot < numSlots_; slot++ )
	{
		adcValues_.push_back(decodeSlot(buffer, slot));
	}
	if ( ((0x000000ff & buffer[numSlots_])) != 0xff ) // rightmost 8 bits are 1
	{
		std::cout << "PixelDecodedFEDRawData::copyBuffer():  Buffer does not end in 0xff." << std::endl;
		badBuffer_ = true;
	}
	else
		badBuffer_ = false;
}

void PixelDecodedFEDRawData::decode()
{
	// Only run this function once.
	if ( decoded_ ) return;
	decoded_ = true;
	
	if ( badBuffer() )
	{
		std::cout << "ERROR: bad buffer -- should not decode data.  Check for bad buffer and do not use if it's bad.  Or use valid() to check before decoding.  Now exiting...\n";
		assert(0);
	}
	
	assert( adcValues_.size() == numSlots_ );

	valid_=false;
	overflow_=false;

	unsigned int slot=transparentDataStart();
	if (slot == numSlots_) // never found the TBM header
	{
		overflow_=true;
		return;
	}
	assert (initialBlackMoments_.count() == 0);
	initialBlackMoments_ = initialBlack_.getMoments();

	// TBM Header
	TBMHeader_.setStartPosition(slot);
	TBMHeader_.setUB1(adcValue(slot++));
	TBMHeader_.setUB2(adcValue(slot++));
	TBMHeader_.setUB3(adcValue(slot++));
	TBMHeader_.setB(adcValue(slot++));
	TBMHeader_.setEventCounter1(adcValue(slot++));
	TBMHeader_.setEventCounter2(adcValue(slot++));
	TBMHeader_.setEventCounter3(adcValue(slot++));
	TBMHeader_.setEventCounter4(adcValue(slot++));

	assert (TBMHeaderUBMoments_.count() == 0);
	TBMHeaderUBMoments_.push_back(TBMHeader_.UB1()); TBMHeaderUBMoments_.push_back(TBMHeader_.UB2()); TBMHeaderUBMoments_.push_back(TBMHeader_.UB3());

	// Check that tolerances for B and UB levels don't overlap.
	if ( initialBlackMoments_.mean() - TBMHeaderUBMoments_.mean() < TBM_UB_maxDelta_ + TBM_B_maxDelta_ )
	{
		//std::cout << "ERROR: tolerance for TBM UB and B levels is too high for the observed spread between TBM UB and initial B:\n";
		//std::cout << "  TBM UB tolerance = +- " << TBM_UB_maxDelta_ << "\n";
		//std::cout << "  TBM  B tolerance = +- " << TBM_B_maxDelta_  << "\n";
		//std::cout << "  initial B - TBM UB = " << initialBlackMoments_.mean() << " - " << TBMHeaderUBMoments_.mean() << " = " << initialBlackMoments_.mean() - TBMHeaderUBMoments_.mean() << std::endl;
		return;
	}
	if ( (initialBlackMoments_.mean()+ROC_B_offset_) - (TBMHeaderUBMoments_.mean()+ROC_UB_offset_) < ROC_UB_maxDelta_ + ROC_B_maxDelta_ )
	{
		//std::cout << "ERROR: tolerance for ROC UB and B levels is too high for the observed spread between (TBM UB + offset) and (initial B + offset):\n";
		//std::cout << "  ROC UB tolerance = +- " << ROC_UB_maxDelta_ << "\n";
		//std::cout << "  ROC  B tolerance = +- " << ROC_B_maxDelta_  << "\n";
		//std::cout << "  (initial B + offset) - (TBM UB + offset) = " << (initialBlackMoments_.mean()+ROC_B_offset_) - (TBMHeaderUBMoments_.mean()+ROC_UB_offset_) << std::endl;
		return;
	}

	// ROCs
	if ( assumedROCsAndHits_ )
	{
		for ( std::vector<unsigned int>::const_iterator assumedHitsOnEachROC_itr = assumedHitsOnEachROC_.begin(); assumedHitsOnEachROC_itr != assumedHitsOnEachROC_.end(); assumedHitsOnEachROC_itr++ )
		{
			// Do the following for each ROC.
			PixelROCOutput thisROCOutput;
	
			thisROCOutput.header().setStartPosition(slot);
			thisROCOutput.header().setUB(adcValue(slot++));
			thisROCOutput.header().setB(adcValue(slot++));
			thisROCOutput.header().setLastDAC(adcValue(slot++));
			if (slot > numSlots_) break; // If the header overflowed the buffer, don't write it out.
	
			for ( unsigned int i = 0; i < *assumedHitsOnEachROC_itr; i++ )
			{
				PixelHitFEDRawData thisHit;
				thisHit.setStartPosition(slot);
				thisHit.setDCol1(adcValue(slot++));
				thisHit.setDCol2(adcValue(slot++));
				thisHit.setRow1(adcValue(slot++));
				thisHit.setRow2(adcValue(slot++));
				thisHit.setRow3(adcValue(slot++));
				thisHit.setPulseHeight(adcValue(slot++));
				if (slot > numSlots_) break; // If the hit overflowed the buffer, don't write it out.
				thisROCOutput.addHit(thisHit);
			} // end of loop over hits on this ROC
	
			ROCOutput_.push_back(thisROCOutput);
			if (slot > numSlots_) break; // If this ROC overflowed the buffer, don't try to read out any more.
		}
		if ( !TBMTrailerStartsInSlot(slot) ) return; // didn't find the TBM trailer where we expected it, decoding failed
	}
	else // full decoding of ROCs
	{
		if (!ROCHeaderStartsInSlot(slot) && !TBMTrailerStartsInSlot(slot) ) // didn't find what we expect next
		{
			return;
		}
		while ( !TBMTrailerStartsInSlot(slot) ) // as long as no TBM Trailer is detected, keep reading out ROCs
		{
			// Do the following for each ROC.
			PixelROCOutput thisROCOutput;
	
			thisROCOutput.header().setStartPosition(slot);
			thisROCOutput.header().setUB(adcValue(slot++));
			thisROCOutput.header().setB(adcValue(slot++));
			thisROCOutput.header().setLastDAC(adcValue(slot++));
			if (slot > numSlots_) break; // If the header overflowed the buffer, don't write it out.
	
			while ( (!ROCHeaderStartsInSlot(slot)) && (!TBMTrailerStartsInSlot(slot)) ) // as long as no new ROC header and no TBM trailer is detected
			{
				PixelHitFEDRawData thisHit;
				thisHit.setStartPosition(slot);
				thisHit.setDCol1(adcValue(slot++));
				thisHit.setDCol2(adcValue(slot++));
				thisHit.setRow1(adcValue(slot++));
				thisHit.setRow2(adcValue(slot++));
				thisHit.setRow3(adcValue(slot++));
				thisHit.setPulseHeight(adcValue(slot++));
				if (slot > numSlots_) break; // If the hit overflowed the buffer, don't write it out.
				thisROCOutput.addHit(thisHit);
			} // end of loop over hits on this ROC
	
			ROCOutput_.push_back(thisROCOutput);
			if (slot > numSlots_) break; // If this ROC overflowed the buffer, don't try to read out any more.
		} // end of loop over ROCs
	}

	// TBM Trailer, don't read a slot if it is past the overflow.
	if (slot >= numSlots_) { overflow_ = true; return; } TBMTrailer_.setStartPosition(slot); TBMTrailer_.setUB1(adcValue(slot++));
	if (slot >= numSlots_) { overflow_ = true; return; } TBMTrailer_.setUB2(adcValue(slot++));
	if (slot >= numSlots_) { overflow_ = true; return; } TBMTrailer_.setB1(adcValue(slot++));
	if (slot >= numSlots_) { overflow_ = true; return; } TBMTrailer_.setB2(adcValue(slot++));
	if (slot >= numSlots_) { overflow_ = true; return; } TBMTrailer_.setExtraInfo1(adcValue(slot++));
	if (slot >= numSlots_) { overflow_ = true; return; } TBMTrailer_.setExtraInfo2(adcValue(slot++));
	if (slot >= numSlots_) { overflow_ = true; return; } TBMTrailer_.setExtraInfo3(adcValue(slot++));
	if (slot >= numSlots_) { overflow_ = true; return; } TBMTrailer_.setExtraInfo4(adcValue(slot++));

	finalBlack_.setStartPosition(slot);
	while (slot < numSlots_)
	{
		finalBlack_.addPoint(adcValue(slot++));
	}

	// done decoding, now check if it's valid

	// If we've gotten this far without reaching an overflow, then we found the entire TBM trailer.
	valid_ = true;
}

unsigned int PixelDecodedFEDRawData::transparentDataStart ()
{
  unsigned int i=0;
  initialBlack_.setStartPosition(i);

	if (altMethod_) {
	  do {
	      initialBlack_.addPoint(adcValue(i++));
	  }
	  //copying this logic over from PixelFEDCalibrationBase::TransparentDataStart()
	  while ((
		  !(
		    (adcValue(i))  <(initialBlack_.mean()-3*initialBlack_.getMoments().stddev()-50) &&
		    (adcValue(i+1))<(initialBlack_.mean()-3*initialBlack_.getMoments().stddev()-50) &&
		    (adcValue(i+2))<(initialBlack_.mean()-3*initialBlack_.getMoments().stddev()-50)
		    ) || i<8)
		 && i<=numSlots_-3);
	}
	else {
	  do
	    {
	      initialBlack_.addPoint(adcValue(i++));
	    } while (
		     (
		      ((adcValue(i))   > (initialBlack_.mean()-TBMHeader_UB_minDistance_))	||
		      ((adcValue(i+1)) > (initialBlack_.mean()-TBMHeader_UB_minDistance_))	||
		      ((adcValue(i+2)) > (initialBlack_.mean()-TBMHeader_UB_minDistance_))	||
		      i<5
		      )
		     && i<=numSlots_-3
		     );
	}
	if (i>=numSlots_-3) i=numSlots_;	// Means an abrupt change never detected!;
	return i;
}
