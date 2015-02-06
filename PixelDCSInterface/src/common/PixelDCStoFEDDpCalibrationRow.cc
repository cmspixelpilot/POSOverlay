#include "PixelDCSInterface/include/PixelDCStoFEDDpCalibrationRow.h"

/**************************************************************************
 * Auxiliary class for storage of calibration coefficients                *
 * for last DAC ADC --> temperature conversion                            *
 * (in units of degrees Celsius)                                          *
 * for a single Read-out Chip                                             *
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2007/12/03 11:24:24 $ (UTC)                        *
 *          by: $Author: veelken $                                        *
 **************************************************************************/

#include <math.h>

#include <iostream>
#include <iomanip>

PixelDCStoFEDDpCalibrationRow::PixelDCStoFEDDpCalibrationRow(unsigned int fedBoardId, unsigned int fedChannelId, unsigned int rocId,
							     float a0, float a1, float b0, float b1, float A[numTempRangeSettings], float B[numTempRangeSettings])
  : PixelDCStoFEDDpRow(fedBoardId, fedChannelId, rocId), 
    a0_(a0), a1_(a1), b0_(b0), b1_(b1)
{
  for ( unsigned iTempRange = 0; iTempRange < numTempRangeSettings; ++iTempRange ) {
    A_[iTempRange] = A[iTempRange];
    B_[iTempRange] = B[iTempRange];
  }
}

float PixelDCStoFEDDpCalibrationRow::getCalibratedValue(int adcCount, int TempRange) const throw (xdaq::exception::Exception)
{
//--- conversion from raw ADC count to calibrated temperature in degrees Celsius
//    (functional form of calibration defined in document #1007 in CMS Pixel DocDB)

//--- check that value of "TempRange" DAC is within the limits defined in the ROC
//    (cf. document #52 in CMS Pixel DocDB)
  if ( TempRange >= 0 && TempRange <= 7 ) {
//--- "TempRange" DAC within limits;
//    determine reference voltage of constant voltage source 
//    used for last DAC temperature measurement in ROC
//    (and hence for ADC sampling; 
//     cf. document #52 in CMS Pixel DocDB)
    float refVoltage = 399.5 + 23.5*TempRange;

    return (a1_*adcCount + b1_ - B_[TempRange] + refVoltage)/(A_[TempRange] - a0_*adcCount - b0_);
  } else {
//--- "TempRange" DAC outside limits;
//    generate error message/throw exception
    XCEPT_RAISE (xdaq::exception::Exception, "TempRange DAC outside limits");
    return 0.;
  }
}

void PixelDCStoFEDDpCalibrationRow::writeTo(std::ostream& stream) const
{
  stream << "   " 
	 << std::setw(13) << fedBoardId_
	 << std::setw(13) << fedChannelId_
	 << std::setw(13) << rocId_
	 << std::setw(9) << a0_
	 << std::setw(9) << a1_
	 << std::setw(9) << b0_
	 << std::setw(9) << b1_;
  
  for ( unsigned iTempRange = 0; iTempRange < numTempRangeSettings; ++iTempRange ) {
    stream << std::setw(9) << A_[iTempRange];
  }
  
  for ( unsigned iTempRange = 0; iTempRange < numTempRangeSettings; ++iTempRange ) {
    stream << std::setw(9) << B_[iTempRange];
  }
  
  stream << std::endl;
}

