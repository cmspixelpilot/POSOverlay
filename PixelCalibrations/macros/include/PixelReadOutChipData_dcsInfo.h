/*************************************************************************
 * Auxiliary class for Analysis of last DAC calibration data;            *
 * stores references to temperature, voltage and current information     *
 * read-out via the DetectorControlSystem                                *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2007/11/09 17:19:15 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#ifndef _PixelReadOutChipData_dcsInfo_h_
#define _PixelReadOutChipData_dcsInfo_h_

#include "PixelCalibrations/macros/include/PixelReadOutChipData_temperatureInfo.h"
#include "PixelCalibrations/macros/include/PixelReadOutChipData_powerInfo.h"

class PixelReadOutChipData_dcsInfo
{
 public:
  PixelReadOutChipData_dcsInfo();
  PixelReadOutChipData_dcsInfo(const PixelReadOutChipData_temperatureInfo& temperatureInfo,
			       const PixelReadOutChipData_powerInfo& powerInfo);
  PixelReadOutChipData_dcsInfo(const PixelReadOutChipData_dcsInfo& dcsInfo);

  const PixelReadOutChipData_temperatureInfo& getTemperatureInfo() const { return temperatureInfo_; }
  const PixelReadOutChipData_powerInfo& getPowerInfo() const { return powerInfo_; }

 private:
  PixelReadOutChipData_temperatureInfo temperatureInfo_; 
  PixelReadOutChipData_powerInfo powerInfo_; 
};

#endif

