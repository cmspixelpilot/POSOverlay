/*************************************************************************
 * Auxiliary class for Analysis of last DAC calibration data;            *
 * stores value of nominal and actual temperature                        *
 * plus information whether last DAC ADC values were taken               *
 * during cooling or heating cycle                                       *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2007/11/09 17:19:16 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#ifndef _PixelReadOutChipData_temperatureInfo_h_
#define _PixelReadOutChipData_temperatureInfo_h_

#include <string>

class PixelReadOutChipData_temperatureInfo
{
 public:
  PixelReadOutChipData_temperatureInfo();
  PixelReadOutChipData_temperatureInfo(double nominalTemperature, double actualTemperature, const std::string& temperatureCycleType);
  PixelReadOutChipData_temperatureInfo(const PixelReadOutChipData_temperatureInfo& temperatureInfo);

  double getNominalTemperature() const { return nominalTemperature_; }
  double getActualTemperature() const { return actualTemperature_; }

  unsigned int getTemperatureCycleType() const { return temperatureCycleType_; }

  enum temperatureCycleTypes { kCoolingCycle, kHeatingCycle, kUndefined };

 private:
  double nominalTemperature_;
  double actualTemperature_;

  unsigned int temperatureCycleType_;
};

#endif

