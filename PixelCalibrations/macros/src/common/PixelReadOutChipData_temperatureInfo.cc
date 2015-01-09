#include "PixelCalibrations/macros/include/PixelReadOutChipData_temperatureInfo.h"

/*************************************************************************
 * Auxiliary class for Analysis of last DAC calibration data;            *
 * stores value of nominal and actual temperature                        *
 * plus information whether last DAC ADC values were taken               *
 * during cooling or heating cycle                                       *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2007/11/09 17:19:19 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#include <iostream>
#include <iomanip>

PixelReadOutChipData_temperatureInfo::PixelReadOutChipData_temperatureInfo()
{
  nominalTemperature_ = 0.;
  actualTemperature_ = 0.;

  temperatureCycleType_ = kUndefined;
}

PixelReadOutChipData_temperatureInfo::PixelReadOutChipData_temperatureInfo(double nominalTemperature, double actualTemperature, const std::string& temperatureCycleType)
{
  nominalTemperature_ = nominalTemperature;
  actualTemperature_ = actualTemperature;

  if ( temperatureCycleType == "Cooling" ) {
    temperatureCycleType_ = kCoolingCycle;
  } else if ( temperatureCycleType == "Heating" ) {
    temperatureCycleType_ = kHeatingCycle;
  } else {
    std::cerr << "Error in <PixelReadOutChipData_temperatureInfo::PixelReadOutChipData_temperatureInfo>:" 
	      << " temperature cycle type = " << temperatureCycleType << " not defined !" << std::endl;
    temperatureCycleType_ = kUndefined;
  }
}

PixelReadOutChipData_temperatureInfo::PixelReadOutChipData_temperatureInfo(const PixelReadOutChipData_temperatureInfo& temperatureInfo)
  : nominalTemperature_(temperatureInfo.nominalTemperature_), actualTemperature_(temperatureInfo.actualTemperature_), 
    temperatureCycleType_(temperatureInfo.temperatureCycleType_)
{}

