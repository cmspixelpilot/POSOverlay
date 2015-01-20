// $Id: PixelDCStoFEDDpValueRow.h,v 1.2 2007/12/03 11:24:22 veelken Exp $

/***************************************************************************
 * Auxiliary class for storage of last DAC temperature information         *
 * for a single Read-out Chip                                              *
 * (sum of "raw" ADC values, number of ADC readings to compute an average, *
 *  calibrated temperature in degrees Celsius                              *
 *  and time of last temperature update send to PVSS)                      *
 *                                                                         *
 * Author: Christian Veelken, UC Davis			 	           *
 *                                                                         *
 * Last update: $Date: 2007/12/03 11:24:22 $ (UTC)                         *
 *          by: $Author: veelken $                                         *
 ***************************************************************************/

#ifndef _PixelDCStoFEDDpValueRow_h_
#define _PixelDCStoFEDDpValueRow_h_

#include "PixelDCSInterface/include/PixelDCStoFEDDpRow.h"

class PixelDCStoFEDDpValueRow : public PixelDCStoFEDDpRow
{
 public:

  PixelDCStoFEDDpValueRow(unsigned int fedBoardId, unsigned int fedChannelId, unsigned int rocId)
    : PixelDCStoFEDDpRow(fedBoardId, fedChannelId, rocId)
  { lastDpValue_ = 0.; lastDpUpdate_ = 0; dpValueSum_ = 0.; numDp_ = 0; adcSum_ = 0.; numADC_ = 0; currentTempRangeDAC_ = 0; }
  ~PixelDCStoFEDDpValueRow() {}
  
  void addValue(float dpValue, int adcCount) { dpValueSum_ += dpValue; ++numDp_; adcSum_ += adcCount; ++numADC_; }
  void setCurrentTempRangeDAC(unsigned int currentTempRangeDAC) { currentTempRangeDAC_ = currentTempRangeDAC; }
  void reset();
  
  unsigned int getBoardId() const { return fedBoardId_; }
  unsigned int getChannelId() const { return fedChannelId_; }
  unsigned int getRocId() const { return rocId_; }
  
  float getDpValue() const { return (numDp_ > 0) ? dpValueSum_/numDp_ : 0.; }
  
  float getLastDpValue() const { return lastDpValue_; }
  time_t getLastDpUpdate() const { return lastDpUpdate_; }
  float getADCCount() const { return (numADC_ > 0) ? adcSum_/numADC_ : 0.; }
  unsigned int getCurrentTempRangeDAC() const { return currentTempRangeDAC_; }
  unsigned int getNumDp() const { return numDp_; }
  
  void writeTo(std::ostream& stream) const;
  
 protected:
  
  float dpValueSum_;
  unsigned int numDp_;
  
  float lastDpValue_;
  time_t lastDpUpdate_;
  
  float adcSum_;
  unsigned int numADC_;

  unsigned int currentTempRangeDAC_;
};

#endif
