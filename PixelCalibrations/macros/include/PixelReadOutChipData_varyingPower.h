
/*************************************************************************
 * Auxiliary class for Analysis of last DAC calibration data;            *
 * computes thermal conductance and and estimates temperature difference *
 * between read-out chip and cooling reservoir,                          *
 * given two PixelReadOutChipData_fixedPower objects                     *
 * and the voltages and currents at which the raw data                   *
 * contained in the PixelReadOutChipData_fixedPower objects was recorded *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2007/11/09 17:19:16 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#ifndef _PixelReadOutChipData_varyingPower_h_
#define _PixelReadOutChipData_varyingPower_h_

#include <map>

class TGraph;
class TCanvas;

#include "PixelCalibrations/macros/include/PixelReadOutChipData_fixedPower.h"
#include "PixelCalibrations/macros/include/PixelReadOutChipData_powerInfo.h"

class PixelReadOutChipData_varyingPower
{
 public:
  PixelReadOutChipData_varyingPower(const std::string& name, 
				    const PixelReadOutChipData_fixedPower& dataSet_reducedPower,
				    const PixelReadOutChipData_fixedPower& dataSet_nominalPower);
  PixelReadOutChipData_varyingPower(const PixelReadOutChipData_varyingPower& varyingPower);
  ~PixelReadOutChipData_varyingPower();

  void Calibrate();

  const PixelReadOutChipData_fixedPower& getDataSet_reducedPower() const { return dataSet_reducedPower_; }
  const PixelReadOutChipData_fixedPower& getDataSet_nominalPower() const { return dataSet_nominalPower_; }

  double getTemperatureDifference() const { return temperatureDifference_; }

  void Write();

 private:

  std::string name_;
  static unsigned int instanceId;

  const PixelReadOutChipData_fixedPower dataSet_reducedPower_;
  const PixelReadOutChipData_fixedPower dataSet_nominalPower_;

  TGraph* temperatureDifferenceGraph1_;
  TGraph* temperatureDifferenceGraph2_;
  TCanvas* temperatureDifferenceCanvas_;

  double temperatureDifference_;
};

#endif
