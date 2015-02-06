// $Id: PixelDCStoFEDDpCalibrationRow.h,v 1.2 2007/12/03 11:24:20 veelken Exp $

/**************************************************************************
 * Auxiliary class for storage of calibration coefficients                *
 * for last DAC ADC --> temperature conversion                            *
 * (in units of degrees Celsius)                                          *
 * for a single Read-out Chip                                             *
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2007/12/03 11:24:20 $ (UTC)                        *
 *          by: $Author: veelken $                                        *
 **************************************************************************/

#ifndef _PixelDCStoFEDDpCalibrationRow_h_
#define _PixelDCStoFEDDpCalibrationRow_h_

#include "xdaq/exception/Exception.h"

#include "PixelDCSInterface/include/PixelDCStoFEDDpRow.h"

const unsigned numTempRangeSettings = 8;

class PixelDCStoFEDDpCalibrationRow : public PixelDCStoFEDDpRow
{
 public:
  
  PixelDCStoFEDDpCalibrationRow(unsigned int fedBoardId, unsigned int fedChannelId, unsigned int rocId,
				float a0, float a1, float b0, float b1, float A[numTempRangeSettings], float B[numTempRangeSettings]);
  ~PixelDCStoFEDDpCalibrationRow() {}
  
  float getCalibratedValue(int adcCount, int TempRange) const throw (xdaq::exception::Exception);
  
  void writeTo(std::ostream& stream) const;
  
 protected:

//--- calibration constants
//    (names defined according to https://docdb.fnal.gov/CMS-private/DocDB/ShowDocument?docid=1007)
  float a0_;
  float a1_;
  float b0_;
  float b1_;
  float A_[8];
  float B_[8];
};
#endif
