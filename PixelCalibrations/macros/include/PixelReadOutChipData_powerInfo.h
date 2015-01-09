/*************************************************************************
 * Auxiliary class for Analysis of last DAC calibration data;            *
 * stores values of voltages and currents provided to read-out chips     *
 * on analog and digital supply lines                                    *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2007/11/09 17:19:16 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#ifndef _PixelReadOutChipData_powerInfo_h_
#define _PixelReadOutChipData_powerInfo_h_

class PixelReadOutChipData_powerInfo
{
 public:
  PixelReadOutChipData_powerInfo();
  PixelReadOutChipData_powerInfo(double v0_analog, double i0_analog, 
				 double v0_digital, double i0_digital);
  PixelReadOutChipData_powerInfo(const PixelReadOutChipData_powerInfo& powerInfo);

  double getV0_analog() const { return v0_analog_; }
  double getI0_analog() const { return i0_analog_; }

  double getV0_digital() const { return v0_digital_; }
  double getI0_digital() const { return i0_digital_; }

  double getPowerConsumption() const { return v0_analog_*i0_analog_ + v0_digital_*i0_digital_; }

 private:
  double v0_analog_;
  double i0_analog_;

  double v0_digital_;
  double i0_digital_;
};

#endif

