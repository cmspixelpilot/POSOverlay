#include "PixelCalibrations/macros/include/PixelReadOutChipData_powerInfo.h"

/*************************************************************************
 * Auxiliary class for Analysis of last DAC calibration data;            *
 * stores values of voltages and currents provided to read-out chips     *
 * on analog and digital supply lines                                    *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2007/11/09 17:19:18 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

PixelReadOutChipData_powerInfo::PixelReadOutChipData_powerInfo()
{
  v0_analog_ = 0.;
  i0_analog_ = 0.;

  v0_digital_ = 0.;
  i0_digital_ = 0.;
}

PixelReadOutChipData_powerInfo::PixelReadOutChipData_powerInfo(double v0_analog, double i0_analog, 
							       double v0_digital, double i0_digital)
{
  v0_analog_ = v0_analog;
  i0_analog_ = i0_analog;

  v0_digital_ = v0_digital;
  i0_digital_ = i0_digital;
}

PixelReadOutChipData_powerInfo::PixelReadOutChipData_powerInfo(const PixelReadOutChipData_powerInfo& powerInfo)
  : v0_analog_(powerInfo.v0_analog_), i0_analog_(powerInfo.i0_analog_), 
    v0_digital_(powerInfo.v0_digital_), i0_digital_(powerInfo.i0_digital_)
{}
