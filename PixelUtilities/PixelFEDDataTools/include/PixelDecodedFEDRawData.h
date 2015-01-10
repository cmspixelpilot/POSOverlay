
#ifndef _PixelDecodedFEDRawData_h_
#define _PixelDecodedFEDRawData_h_

#include "PixelUtilities/PixelFEDDataTools/include/PixelBlackData.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelTBMHeader.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelROCOutput.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelROCHeader.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelHitFEDRawData.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelTBMTrailer.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"

#include <vector>
#include <iostream>
#include <stdint.h>

/********************************************************************
This class is a general-purpose decoder of FIFO1 on a FED.  It makes no assumptions about the number of ROCs or the number of hits per ROC.

Seven parameters can be specified to control the sensitivity of the UB and B detection, separately for ROCs and for the TBM.  All parameters are specified in units of FED ADC readings, which range from 0 to 1023.  The decoder starts at the beginning of the data and keeps track of the initial black level.  After 5 slots, it starts looking for 3 consecutive slots at least TBMHeader_UB_minDistance units below the average black level so far.  When these are found, they are assumed to be the start of the TBM header.  These three slots are averaged to determine the baseline UB level, and the average of the initial black data determines the baseline B level.

After the TBM header, the decoder looks through the slots in order.  When it finds a ROC header, it reads it out.  It then keeps reading out hits as long as it doesn't find a ROC header or TBM trailer.  When a new ROC header is found, that ROC's header and hits are read out.  When the TBM trailer is found, it is read out, and then the final black data are read out.

The ranges considered to be UB and B in the ROC headers and TBM trailer are controlled by 6 parameters.  For the TBM, the hit must be within TBM_B_maxDelta or TBM_UB_maxDelta units of the respective B or UB baseline level.  For the ROC, separate deltas may be specified: ROC_B_maxDelta and ROC_UB_maxDelta.  The ROC baselines may also be adjusted relative to the TBM values by the offsets: ROC_B_offset and ROC_UB_offset.

Suggested values of these 7 parameters are provided by a constructor, or they may be specified by a different constructor.
*******************************************************************/

typedef uint32_t UINT32; // FED data are 32 bit words.

class PixelDecodedFEDRawData
{
	public:

	static const unsigned int numSlots_ = pos::fifo1TranspDataLength-1; // last word in buffer is 0xff

	// constructors with full decoding
	PixelDecodedFEDRawData(UINT32 * buffer); // provides default values
	PixelDecodedFEDRawData(UINT32 * buffer, double TBMHeader_UB_minDistance, double TBM_B_maxDelta, double TBM_UB_maxDelta, double ROC_B_offset, double ROC_B_maxDelta, double ROC_UB_offset, double ROC_UB_maxDelta) ;
	
	// constructor with assumed number of ROCs and hits
	// assumedHitsOnEachROC has one element for each ROC.  Each element is the number of pixel hits assumed on that ROC.  The numbers are listed in the ROC readout order.  For example, if the vector is (4,0,2), this means that 3 ROCs are assumed, the first with 4 hits, the second with none, and the third with 2.
	PixelDecodedFEDRawData(UINT32 * buffer, double TBMHeader_UB_minDistance, double TBM_B_maxDelta, double TBM_UB_maxDelta, std::vector<unsigned int> assumedHitsOnEachROC);

	void setAltMethod(bool altMethod) { altMethod_=altMethod;}

	// functions to return info after processing is done
	unsigned int adcValue(unsigned int slot) {if (slot<numSlots_) return adcValues_[slot]; else return 10000;}
	std::vector<unsigned int> adcValues() {return adcValues_;}

	bool valid() {if ( badBuffer_ ) return false; decode(); return valid_;}
	bool badBuffer() {return badBuffer_;}         // true if the last word of the buffer doesn't end in 0xff
	bool overflow() {decode(); return overflow_;} // true if the data extended beyond pos::fifo1TranspDataLength-1 slots

	PixelBlackData initialBlack() {decode(); return initialBlack_;}
	PixelTBMHeader TBMHeader() {decode(); return TBMHeader_;}
	std::vector<PixelROCOutput> ROCOutput() {decode(); return ROCOutput_;}
	unsigned int numROCs() {decode(); return ROCOutput_.size();}
	PixelROCOutput ROCOutput(unsigned int ROCNumber) {decode(); assert(ROCNumber < numROCs()); return ROCOutput_[ROCNumber];}
	PixelTBMTrailer TBMTrailer() {decode(); return TBMTrailer_;}
	PixelBlackData finalBlack() {decode(); return finalBlack_;}

	// print out the entire raw data stream with annotation
	void printToStream(std::ostream& out);

	// draw raw data to an image file (GIF, PNG, EPS, etc.) -- specify type by extension

	void drawToFile(std::string filename, std::string filenameshort, unsigned int nclock=150, unsigned int columns=1024,unsigned int rows=256);
	void drawToFile(std::string filename, unsigned int nclock=150, unsigned int columns=1024,unsigned int rows=256);

	private:
	// function called by constructors
	void initialize_fullDecoding(UINT32 * buffer, double TBMHeader_UB_minDistance, double TBM_B_maxDelta, double TBM_UB_maxDelta, double ROC_B_offset, double ROC_B_maxDelta, double ROC_UB_offset, double ROC_UB_maxDelta);
	
	// step in initilization
	void copyBuffer(UINT32 * buffer);

	// step in the copying process
	unsigned int decodeSlot(UINT32 * buffer, unsigned int slot) {return ((0xffc00000 & buffer[slot])>>22);} // Take the leftmost 10 bits of the 32-bit integer buffer[slot].

	// Decoding function, called as needed.
	void decode();

	// step in the decoding process
	unsigned int transparentDataStart();

	// functions to determine whether a value (output of decodeSlot(slot)) is considered ultrablack or black
	bool is_TBM_UB(unsigned int value) {return is_within(value, TBMHeaderUBMoments_,  0., TBM_UB_maxDelta_);}
	bool is_TBM_B (unsigned int value) {return is_within(value, initialBlackMoments_, 0.,  TBM_B_maxDelta_);}
	bool is_ROC_UB(unsigned int value) {return is_within(value, TBMHeaderUBMoments_,  ROC_UB_offset_, ROC_UB_maxDelta_);}
	bool is_ROC_B (unsigned int value) {return is_within(value, initialBlackMoments_,  ROC_B_offset_,  ROC_B_maxDelta_);}
	bool is_within(unsigned int value, Moments moments, double offset, double tolerance)
	        {assert(moments.count() > 0); return fabs( value - (moments.mean()+offset) ) < tolerance;}

	// function to find ROC header
	bool ROCHeaderStartsInSlot(unsigned int slot);

	// function to find TBM trailer
	bool TBMTrailerStartsInSlot(unsigned int slot);

	// vector to store decoded adc values
	std::vector<unsigned int> adcValues_;

	// parameters to determine UB and B sensitivity
	double TBMHeader_UB_minDistance_; // Values at least this far below the black level seen so far are considered ultrablack in the TBM header.
	double TBM_B_maxDelta_; // Values within this distance of the initial black mean are considered black in the TBM trailer.
	double TBM_UB_maxDelta_; // Values within this distance of the TBM header ultrablack mean are considered ultrablack in the TBM trailer.
	double ROC_B_offset_;
	double ROC_B_maxDelta_; // Values within (initial black mean + offset) +- maxDelta are considered black in the ROC header.
	double ROC_UB_offset_;
	double ROC_UB_maxDelta_; // Values within (TBM header ultrablack mean + offset) +- maxDelta are considered ultrablack in the ROC header.

	// whether to assume the number of ROCs and hits, and, if so, the assumed numbers
	bool assumedROCsAndHits_;
	std::vector<unsigned int> assumedHitsOnEachROC_;

	// UB info from the TBM header
	Moments TBMHeaderUBMoments_;
	// B info from the initial black data
	Moments initialBlackMoments_;

	// status info
	bool valid_;
	bool badBuffer_; // true if the last word of the buffer doesn't end in 0xff
	bool overflow_;  // true if the data extended beyond pos::fifo1TranspDataLength-1 slots

	bool altMethod_; //use alternate algorithm for determining point of B->UB transition

	// decoded info
	bool decoded_; // whether decoding has been attempted
	PixelBlackData initialBlack_;
	PixelTBMHeader TBMHeader_;
	std::vector<PixelROCOutput> ROCOutput_;
	PixelTBMTrailer TBMTrailer_;
	PixelBlackData finalBlack_;

};

#endif

