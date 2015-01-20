// $Id: PixelDCStoFEDDpFilterRow.h,v 1.2 2007/12/03 11:24:21 veelken Exp $

/**************************************************************************
 * Auxiliary class for storage of lower and upper ADC limits within which *
 * the last DAC temperature values of a single Read-out Chip              *
 * are suitable for calibration                                           *
 * (cf. https://docdb.fnal.gov/CMS-private/DocDB/ShowDocument?docid=1007) *
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2007/12/03 11:24:21 $ (UTC)                        *
 *          by: $Author: veelken $                                        *
 **************************************************************************/

#ifndef _PixelDCStoFEDDpFilterRow_h_
#define _PixelDCStoFEDDpFilterRow_h_

#include "PixelDCSInterface/include/PixelDCStoFEDDpRow.h"

class PixelDCStoFEDDpFilterRow : public PixelDCStoFEDDpRow
{
 public:

  PixelDCStoFEDDpFilterRow(unsigned int fedBoardId, unsigned int fedChannelId, unsigned int rocId,
			   int minAdc, int maxAdc, float deadband)
    : PixelDCStoFEDDpRow(fedBoardId, fedChannelId, rocId), minAdc_(minAdc), maxAdc_(maxAdc), deadband_(deadband) {}
  ~PixelDCStoFEDDpFilterRow() {}
        
//--- check that raw ADC count is within the lower and upper limits
//    for which the ADC precision is sufficient
  bool isAboveUpperLimit(int adcCount) const { return adcCount > maxAdc_; }
  bool isBelowLowerLimit(int adcCount) const { return adcCount < minAdc_; }
  bool isWithinLimits(int adcCount) const { return !(isAboveUpperLimit(adcCount) || isBelowLowerLimit(adcCount)); }

  bool isWithinDeadband(float value, float lastValue) const;

  void writeTo(std::ostream& stream) const;

 protected:

//--- lower and upper limits
//    for which the ADC precision is sufficient
  int minAdc_;
  int maxAdc_;

//--- dead-band applied in decision
//    whether or not to send calibrated data-point value to PVSS
  float deadband_;
};
#endif
