#include "PixelCalibrations/macros/include/PixelReadOutChipData_dcsInfo.h"

/*************************************************************************
 * Auxiliary class for Analysis of last DAC calibration data;            *
 * stores references to temperature, voltage and current information     *
 * read-out via the DetectorControlSystem                                *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2007/11/09 17:19:17 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

PixelReadOutChipData_dcsInfo::PixelReadOutChipData_dcsInfo()
{}

PixelReadOutChipData_dcsInfo::PixelReadOutChipData_dcsInfo(const PixelReadOutChipData_temperatureInfo& temperatureInfo,
							   const PixelReadOutChipData_powerInfo& powerInfo)
  : temperatureInfo_(temperatureInfo), powerInfo_(powerInfo)
{}

PixelReadOutChipData_dcsInfo::PixelReadOutChipData_dcsInfo(const PixelReadOutChipData_dcsInfo& dcsInfo)
  : temperatureInfo_(dcsInfo.temperatureInfo_), powerInfo_(dcsInfo.powerInfo_)
{}

