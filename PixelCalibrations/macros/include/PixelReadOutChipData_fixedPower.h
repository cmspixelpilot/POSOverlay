
/*************************************************************************
 * Auxiliary class for Analysis of last DAC calibration data;            *
 * stores all information neccessary to calibrate a single read-out chip *
 * for a fixed power dissipation level (i.e. Vana and Vsf DAC value)     *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2007/11/09 17:19:15 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#ifndef _PixelReadOutChipData_fixedPower_h_
#define _PixelReadOutChipData_fixedPower_h_

#include <map>
#include <list>

#include "PixelCalibrations/macros/include/PixelReadOutChipData_dcsInfo.h"

class TGraph;
class TF1;
class TMinuit;
class TCanvas;

const unsigned int numTempRanges = 8;
const unsigned int numTempCycleTypes = 2;

void getDimension(TGraph* graph, double& xMin, double& xMax, double& yMin, double& yMax);

class PixelReadOutChipData_fixedPower
{
 public:
  PixelReadOutChipData_fixedPower(const std::string& name, 
				  const std::list<std::pair<std::map<unsigned int, std::list<unsigned int> >, PixelReadOutChipData_dcsInfo> >& rawData);
  PixelReadOutChipData_fixedPower(const PixelReadOutChipData_fixedPower& fixedPower);
  ~PixelReadOutChipData_fixedPower();

  void Calibrate();

  std::map<std::string, double> getCalibrationConstants() const;

  double getCalibratedTemperature(unsigned int adcValue, unsigned int TempRange) const;
  double getCalibratedTemperature(double adcValue, unsigned int TempRange) const;
  double getCalibratedTemperature(unsigned int adcValues[numTempRanges]) const;
  double getCalibratedTemperature(double adcValues[numTempRanges]) const;

  const std::list<std::pair<std::map<unsigned int, std::list<unsigned int> >, PixelReadOutChipData_dcsInfo> >& getRawData() const { return rawData_; }
  const std::list<std::pair<std::map<unsigned int, double>, PixelReadOutChipData_dcsInfo> >& getAveragedRawData() const { return rawData_averaged_; }

  const PixelReadOutChipData_powerInfo* getPowerInfo(double nominalTemperature) const;

  void Write();

 private:

  std::string name_;
  static unsigned int instanceId;

//--- mapping between TempRange DAC setting and last DAC ADC values
//     format = list of pairs < [TempRange] --> list of ADC values, DCS conditions >
  const std::list<std::pair<std::map<unsigned int, std::list<unsigned int> >, PixelReadOutChipData_dcsInfo> >& rawData_;

//--- mapping between TempRange DAC setting and **average** last DAC ADC value
//    (averaged over all temperature cycles and triggers)
//     format = list of pairs < [TempRange] --> average ADC value, DCS conditions >
  std::list<std::pair<std::map<unsigned int, double>, PixelReadOutChipData_dcsInfo> > rawData_averaged_;

  void averageRawData();

  TGraph* calibrationGraphs_[numTempRanges];
  TF1* calibrationFunction_slope;
  TF1* calibrationFunction_offset;
  TMinuit* calibrationFit_;
  void fillCalibrationGraph(unsigned int TempRange);
  void fitCalibrationData();
  TCanvas* calibrationCanvas_;

  TGraph* tempGraphs_fit_[numTempRanges];
  TCanvas* fitCanvas_;

  TGraph* measurementGraphs_[numTempRanges];
  TF1* measurementFunction_[numTempRanges];
  void fillMeasurementGraph(unsigned int TempRange);
  void fitMeasurementData();
  TCanvas* measurementCanvas_;

  TGraph* linearityGraphs_[numTempRanges];
  void fillLinearityGraph(unsigned int TempRange);
  TCanvas* linearityCanvas_;

  TGraph* precisionGraphs_[numTempCycleTypes];
  void fillPrecisionGraph(unsigned int temperatureCycleType);
  TCanvas* precisionCanvas_;
};

#endif
