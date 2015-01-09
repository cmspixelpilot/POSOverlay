#include "PixelCalibrations/macros/include/PixelReadOutChipData_fixedPower.h"

/*************************************************************************
 * Auxiliary class for Analysis of last DAC calibration data;            *
 * stores all information neccessary to calibrate a single read-out chip *
 * for a fixed power dissipation level (i.e. Vana and Vsf DAC value)     *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2007/11/09 17:19:18 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#include <iostream>
#include <iomanip>

#include <TGraph.h>
#include <TF1.h>
#include <TMinuit.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TMath.h>

double referenceVoltage[numTempRanges] = { 399.5, 423.0, 446.5, 470.0, 493.5, 517.0, 540.5, 564.0 }; // mV
double calibrationVoltage = 470; // mV

const unsigned int adcValue_blackLevel = 128;
const unsigned int adcValue_threshold = 5;

const unsigned int numFitParameter_calibration = 4;

void fillGraph(TGraph* graph, const std::list<std::pair<std::map<unsigned int, double>, PixelReadOutChipData_dcsInfo> >& rawData, unsigned int TempRange);

void drawGraphs(TGraph* graphs[numTempRanges], const char* drawOption, const char* labelAxisX, const char* labelAxisY);
void drawGraphs(TGraph* graphs[numTempRanges], const char* drawOption, TF1* functions[numTempRanges], const char* labelAxisX, const char* labelAxisY);
void drawGraphs(TGraph* graphs[numTempCycleTypes], const char* labelAxisX, const char* labelAxisY);

void fitCalibrationData_fcn(Int_t& npar, Double_t* gin, Double_t& f, Double_t* parameter, Int_t iflag);

void createCanvas(TCanvas*& canvas, const char* name, const char* title);

//--- define global variables neccessary for fit-functions
TGraph** gGraphArray_fit;

//
//---------------------------------------------------------------------------------------------------
//

unsigned int PixelReadOutChipData_fixedPower::instanceId = 0;

PixelReadOutChipData_fixedPower::PixelReadOutChipData_fixedPower(const std::string& name, 
								 const std::list<std::pair<std::map<unsigned int, std::list<unsigned int> >, 
								                           PixelReadOutChipData_dcsInfo> >& rawData)
  : rawData_(rawData)
{  
  TString objectName = Form("%s_id%06X", name.data(), instanceId);
  ++instanceId;
  name_ = objectName.Data();

  std::cout << "<PixelReadOutChipData_fixedPower::PixelReadOutChipData_fixedPower(const std::list<std::pair<... )>:" << std::endl;
  std::cout << " name = " << name_ << std::endl;

  averageRawData();

  TString calibrationFunction_slope_name = Form("%s_%s", "calibrationFunction_slope", name_.data());
  calibrationFunction_slope = new TF1(calibrationFunction_slope_name, "x++1");
  TString calibrationFunction_offset_name = Form("%s_%s", "calibrationFunction_offset", name_.data());
  calibrationFunction_offset = new TF1(calibrationFunction_offset_name, "x++1");
  calibrationFit_ = new TMinuit(numFitParameter_calibration);
  
  for ( unsigned int iTempRange = 0; iTempRange < numTempRanges; ++iTempRange ) {
    calibrationGraphs_[iTempRange] = NULL;
    measurementGraphs_[iTempRange] = NULL;

    TString functionName = Form("measurementFunction_TempRange%02X_%s", iTempRange, name_.data());
    measurementFunction_[iTempRange] = new TF1(functionName, "x++1");

    tempGraphs_fit_[iTempRange] = NULL;
    linearityGraphs_[iTempRange] = NULL;
  }

  for ( unsigned int iTempCycle = 0; iTempCycle < numTempCycleTypes; ++iTempCycle ) {
    precisionGraphs_[iTempCycle] = NULL;
  }

  calibrationCanvas_ = NULL;
  fitCanvas_ = NULL;
  measurementCanvas_ = NULL;
  linearityCanvas_ = NULL;
  precisionCanvas_ = NULL;
}

PixelReadOutChipData_fixedPower::PixelReadOutChipData_fixedPower(const PixelReadOutChipData_fixedPower& fixedPower)
  : rawData_(fixedPower.rawData_)
{
  TString objectName = Form("%s_id%06X", fixedPower.name_.data(), instanceId);
  ++instanceId;
  name_ = objectName.Data();

  std::cout << "<PixelReadOutChipData_fixedPower::PixelReadOutChipData_fixedPower(const PixelReadOutChipData_fixedPower& )>:" << std::endl;
  std::cout << " name = " << name_ << std::endl;

  averageRawData();

  if ( fixedPower.calibrationFunction_slope != NULL ) {
    calibrationFunction_slope = new TF1(*fixedPower.calibrationFunction_slope);
  } else {
    calibrationFunction_slope = NULL;
  }
  if ( fixedPower.calibrationFunction_offset != NULL ) {
    calibrationFunction_offset = new TF1(*fixedPower.calibrationFunction_offset);
  } else {
    calibrationFunction_offset = NULL;
  }
  calibrationFit_ = new TMinuit(numFitParameter_calibration);

  for ( unsigned int iTempRange = 0; iTempRange < numTempRanges; ++iTempRange ) {
    calibrationGraphs_[iTempRange] = NULL;
    measurementGraphs_[iTempRange] = NULL;

    if ( fixedPower.measurementFunction_[iTempRange] != NULL ) {
      measurementFunction_[iTempRange] = new TF1(*fixedPower.measurementFunction_[iTempRange]);
    } else {
      measurementFunction_[iTempRange] = NULL;
    }
    std::cout << "measurementFunction_[" << iTempRange << "]->GetName = " << measurementFunction_[iTempRange]->GetName() << std::endl;

    tempGraphs_fit_[iTempRange] = NULL;
    linearityGraphs_[iTempRange] = NULL;
  }

  for ( unsigned int iTempCycle = 0; iTempCycle < numTempCycleTypes; ++iTempCycle ) {
    precisionGraphs_[iTempCycle] = NULL;
  }

  calibrationCanvas_ = NULL;
  fitCanvas_ = NULL;
  measurementCanvas_ = NULL;
  linearityCanvas_ = NULL;
  precisionCanvas_ = NULL;
}

PixelReadOutChipData_fixedPower::~PixelReadOutChipData_fixedPower()
{
  std::cout << "<PixelReadOutChipData_fixedPower::~PixelReadOutChipData_fixedPower>:" << std::endl;
  std::cout << " name = " << name_ << std::endl;

  delete calibrationFunction_slope;
  delete calibrationFunction_offset;
  delete calibrationFit_;
  delete calibrationCanvas_;
  delete measurementCanvas_;
  delete fitCanvas_;
  delete linearityCanvas_;
  delete precisionCanvas_;

  for ( unsigned int iTempRange = 0; iTempRange < numTempRanges; ++iTempRange ) {
    delete calibrationGraphs_[iTempRange];
    delete measurementGraphs_[iTempRange];
    delete measurementFunction_[iTempRange];
    delete tempGraphs_fit_[iTempRange];
    delete linearityGraphs_[iTempRange];
  }

  for ( unsigned int iTempCycle = 0; iTempCycle < numTempCycleTypes; ++iTempCycle ) {
    delete precisionGraphs_[iTempCycle];
  }
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelReadOutChipData_fixedPower::Calibrate()
{
  for ( unsigned int iTempRange = 8; iTempRange < (numTempRanges + 8); ++iTempRange ) {
    fillCalibrationGraph(iTempRange);
  }

  fitCalibrationData();

  for ( unsigned int iTempRange = 0; iTempRange < numTempRanges; ++iTempRange ) {
    fillMeasurementGraph(iTempRange);
  }

  fitMeasurementData();

  for ( unsigned int iTempRange = 0; iTempRange < numTempRanges; ++iTempRange ) {
    fillLinearityGraph(iTempRange);
  }

  for ( unsigned int iTempCycle = 0; iTempCycle < numTempCycleTypes; ++iTempCycle ) {
    fillPrecisionGraph(iTempCycle);
  }
}

//
//---------------------------------------------------------------------------------------------------
//

std::map<std::string, double> PixelReadOutChipData_fixedPower::getCalibrationConstants() const
{
  std::map<std::string, double> calibrationConstants;

  calibrationConstants["a0"] = calibrationFunction_slope->GetParameter(0);
  calibrationConstants["a1"] = calibrationFunction_slope->GetParameter(1);
  calibrationConstants["b0"] = calibrationFunction_offset->GetParameter(0);
  calibrationConstants["b1"] = calibrationFunction_offset->GetParameter(1);
  for ( unsigned int iTempRange = 0; iTempRange < numTempRanges; ++iTempRange ) {
    //TString calibrationConstantName_A = Form("A TempRange=\"%02X\"", iTempRange);
    TString calibrationConstantName_A = Form("A_TempRange%02X", iTempRange);
    calibrationConstants[calibrationConstantName_A.Data()] = measurementFunction_[iTempRange]->GetParameter(0);
    //TString calibrationConstantName_B = Form("B TempRange=\"%02X\"", iTempRange);
    TString calibrationConstantName_B = Form("B_TempRange%02X", iTempRange);
    calibrationConstants[calibrationConstantName_B.Data()] = measurementFunction_[iTempRange]->GetParameter(1);
  }
  
  return calibrationConstants;
}

//
//---------------------------------------------------------------------------------------------------
//

double PixelReadOutChipData_fixedPower::getCalibratedTemperature(unsigned int adcValue, unsigned int TempRange) const
{
  double adcValue_double = adcValue;
  return getCalibratedTemperature(adcValue_double, TempRange);
}

double PixelReadOutChipData_fixedPower::getCalibratedTemperature(double adcValue, unsigned int TempRange) const
{
  if ( TempRange >= 0 && TempRange < numTempRanges ) {
    double a0 = calibrationFunction_slope->GetParameter(0);
    double a1 = calibrationFunction_slope->GetParameter(1);
    double b0 = calibrationFunction_offset->GetParameter(0);
    double b1 = calibrationFunction_offset->GetParameter(1);

    double A = measurementFunction_[TempRange]->GetParameter(0);
    double B = measurementFunction_[TempRange]->GetParameter(1);

    return (a1*adcValue - B + b1 + referenceVoltage[TempRange] )/(A - a0*adcValue -b0);
  } else {
//--- TempRange DAC setting does not correspond to measurement mode;
//    return absolute zero temperature
    std::cerr << "Error in <PixelReadOutChipData_fixedPower::getCalibratedTemperature>: TempRange = " << TempRange << " outside of Measurement Range !!!" << std::endl;
    return -273.15; 
  }
}

double PixelReadOutChipData_fixedPower::getCalibratedTemperature(unsigned int adcValues[numTempRanges]) const
{
  double adcValues_double[numTempRanges];
  for ( unsigned int iTempRange = 0; iTempRange < numTempRanges; ++iTempRange ) {
    adcValues_double[iTempRange] = adcValues[iTempRange];
  }
  
  return getCalibratedTemperature(adcValues_double);
}

double PixelReadOutChipData_fixedPower::getCalibratedTemperature(double adcValues[numTempRanges]) const
{
  for ( unsigned int iTempRange = 0; iTempRange < numTempRanges; ++iTempRange ) {
    TF1* measurementFunction_TempRange = measurementFunction_[iTempRange];

//--- get calibrated temperature
//    estimated by using iTempRange as reference voltage
    double temperature = getCalibratedTemperature(adcValues[iTempRange], iTempRange);

//--- check that estimated temperature 
//    is within the range of temperatures included in the fit 
//    of measurementFunction_[iTempRange]
    double validityRange_minTemperature, validityRange_maxTemperature;
    measurementFunction_TempRange->GetRange(validityRange_minTemperature, validityRange_maxTemperature);

    if ( temperature >= validityRange_minTemperature && temperature <= validityRange_maxTemperature ) {
      return temperature;
    }
  }

  std::cerr << "Error in <PixelReadOutChipData_fixedPower::getCalibratedTemperature>:"
	    << " failed to find suitable Temperature calibration function !!!" << std::endl;
  return 0.;
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelReadOutChipData_fixedPower::averageRawData()
{
  typedef std::list<std::pair<std::map<unsigned int, std::list<unsigned int> >, PixelReadOutChipData_dcsInfo> > rawDataType_temperature;
  typedef std::map<unsigned int, std::list<unsigned int> > rawDataType_TempRange;

  for ( rawDataType_temperature::const_iterator rawDataEntry_temperature = rawData_.begin();
	rawDataEntry_temperature != rawData_.end(); ++rawDataEntry_temperature ) {
    const PixelReadOutChipData_dcsInfo& dcsInfo = rawDataEntry_temperature->second;
    
    std::map<unsigned int, double> adcValues_averaged;

    for ( rawDataType_TempRange::const_iterator rawDataEntry_TempRange = rawDataEntry_temperature->first.begin();
	  rawDataEntry_TempRange != rawDataEntry_temperature->first.end(); ++rawDataEntry_TempRange ) {
      unsigned int TempRange = rawDataEntry_TempRange->first;

      double adcValue_sum = 0.;
      unsigned int adcValue_numSummands = 0;
      
      for ( std::list<unsigned int>::const_iterator adcValue = rawDataEntry_TempRange->second.begin();
	    adcValue != rawDataEntry_TempRange->second.end(); ++adcValue ) {
	adcValue_sum += (*adcValue);
	++adcValue_numSummands;
      }

      if ( adcValue_numSummands > 0 ) {
	adcValues_averaged[TempRange] = adcValue_sum/adcValue_numSummands;
      } else {
	std::cerr << "Error in <PixelReadOutChipData_fixedPower::averageRawData>:" 
		  << " list of ADC values empty for" 
		  << " Temperature = " << dcsInfo.getTemperatureInfo().getNominalTemperature() << ","
		  << " TempRange = " << TempRange << " !!!" << std::endl;
      }
    }

    rawData_averaged_.push_back(std::pair<std::map<unsigned int, double>, PixelReadOutChipData_dcsInfo>(adcValues_averaged, dcsInfo));
  }
}

void PixelReadOutChipData_fixedPower::fillCalibrationGraph(unsigned int TempRange)
{
//--- check that TempRange DAC setting corresponds to calibration mode
  if ( (TempRange - 8) >= 0 && (TempRange - 8) < numTempRanges ) {

    unsigned int calibrationGraph_index = TempRange - 8;
    TGraph*& calibrationGraph = calibrationGraphs_[calibrationGraph_index];

//--- create graph if neccessary
    if ( calibrationGraph == NULL ) {
      calibrationGraph = new TGraph();
      TString graphName = Form("calibrationGraph_TempRange%02X", TempRange);
      calibrationGraph->SetName(graphName);
      std::cout << "--> creating new calibration Graph, name = " << graphName << std::endl;
    }

    fillGraph(calibrationGraph, rawData_averaged_, TempRange);

    std::cout << "calibration Graph, name = " << calibrationGraph->GetName() << ","
	      << " has " << calibrationGraph->GetN() << " points." << std::endl;
  } else {
    std::cerr << "Error in <PixelReadOutChipData_fixedPower::fillCalibrationGraph>: TempRange = " << TempRange << " outside of Calibration Range !!!" << std::endl;
  }
}

void PixelReadOutChipData_fixedPower::fillMeasurementGraph(unsigned int TempRange)
{
//--- check that TempRange DAC setting corresponds to measurement mode
  if ( TempRange >= 0 && TempRange < numTempRanges ) {

    unsigned int measurementGraph_index = TempRange;
    TGraph*& measurementGraph = measurementGraphs_[measurementGraph_index];

//--- create graph if neccessary
    if ( measurementGraph == NULL ) {
      measurementGraph = new TGraph();
      TString graphName = Form("measurementGraph_TempRange%02X", TempRange);
      measurementGraph->SetName(graphName);
    }

    fillGraph(measurementGraph, rawData_averaged_, TempRange);

    std::cout << "measurement Graph, name = " << measurementGraph->GetName() << ","
	      << " has " << measurementGraph->GetN() << " points." << std::endl;
  } else {
    std::cerr << "Error in <PixelReadOutChipData_fixedPower::fillMeasurementGraph>:" 
	      << " TempRange = " << TempRange << " outside of Measurement Range !!!" << std::endl;
  }
}

void PixelReadOutChipData_fixedPower::fillLinearityGraph(unsigned int TempRange)
{
//--- check that TempRange DAC setting corresponds to measurement mode
  if ( TempRange >= 0 && TempRange < numTempRanges ) {

    TGraph*& linearityGraph = linearityGraphs_[TempRange];

//--- create graph if neccessary
    if ( linearityGraph == NULL ) {
      linearityGraph = new TGraph();
      TString graphName = Form("linearityGraph_TempRange%02X", TempRange);
      linearityGraph->SetName(graphName);
      std::cout << "--> creating new linearity Graph, name = " << graphName << std::endl;
    }

    typedef std::list<std::pair<std::map<unsigned int, std::list<unsigned int> >, PixelReadOutChipData_dcsInfo> > rawDataType_temperature;
    typedef std::map<unsigned int, std::list<unsigned int> > rawDataType_TempRange;

    for ( rawDataType_temperature::const_iterator rawDataEntry_temperature = rawData_.begin();
          rawDataEntry_temperature != rawData_.end(); ++rawDataEntry_temperature ) {
      const PixelReadOutChipData_dcsInfo& dcsInfo = rawDataEntry_temperature->second;
    
      double actualTemperature = dcsInfo.getTemperatureInfo().getActualTemperature();

      for ( rawDataType_TempRange::const_iterator rawDataEntry_TempRange = rawDataEntry_temperature->first.begin();
	    rawDataEntry_TempRange != rawDataEntry_temperature->first.end(); ++rawDataEntry_TempRange ) {
        unsigned int actualTempRange = rawDataEntry_TempRange->first;

        if ( actualTempRange == TempRange ) {

//--- check that actual temperature 
//    is within the range of temperatures included in the fit 
//    of measurementFunction_[iTempRange]
	  double validityRange_minTemperature, validityRange_maxTemperature;
	  measurementFunction_[TempRange]->GetRange(validityRange_minTemperature, validityRange_maxTemperature);
      
	  if ( actualTemperature >= validityRange_minTemperature && actualTemperature <= validityRange_maxTemperature ) {
	    for ( std::list<unsigned int>::const_iterator adcValue = rawDataEntry_TempRange->second.begin();
		  adcValue != rawDataEntry_TempRange->second.end(); ++adcValue ) {
	      double estimatedTemperature = getCalibratedTemperature(*adcValue, TempRange);
		  
	      linearityGraph->SetPoint(linearityGraphs_[TempRange]->GetN(), actualTemperature, estimatedTemperature);
	    }
	  }
	}
      }
    }
  } else {
    std::cerr << "Error in <PixelReadOutChipData_fixedPower::fillLinearityGraph>:" 
	      << " TempRange = " << TempRange << " outside of Measurement Range !!!" << std::endl;
  }
}

void PixelReadOutChipData_fixedPower::fillPrecisionGraph(unsigned int temperatureCycleType)
{
//--- check that TempRange DAC setting corresponds to measurement mode
  if ( temperatureCycleType == PixelReadOutChipData_temperatureInfo::kCoolingCycle ||
       temperatureCycleType == PixelReadOutChipData_temperatureInfo::kHeatingCycle ) {
    TGraph*& precisionGraph = precisionGraphs_[temperatureCycleType];

//--- create graph if neccessary
    if ( precisionGraph == NULL ) {
      precisionGraph = new TGraph();
      TString graphName;
      if ( temperatureCycleType == PixelReadOutChipData_temperatureInfo::kCoolingCycle ) graphName = "precisionGraph_CoolingCycle";
      if ( temperatureCycleType == PixelReadOutChipData_temperatureInfo::kHeatingCycle ) graphName = "precisionGraph_HeatingCycle";
      precisionGraph->SetName(graphName);
      std::cout << "--> creating new precision Graph, name = " << graphName << std::endl;
    }

    typedef std::list<std::pair<std::map<unsigned int, std::list<unsigned int> >, PixelReadOutChipData_dcsInfo> > rawDataType_temperature;
    typedef std::map<unsigned int, std::list<unsigned int> > rawDataType_TempRange;
  
    for ( rawDataType_temperature::const_iterator rawDataEntry_temperature = rawData_.begin();
	  rawDataEntry_temperature != rawData_.end(); ++rawDataEntry_temperature ) {
      const PixelReadOutChipData_dcsInfo& dcsInfo = rawDataEntry_temperature->second;
      
      const PixelReadOutChipData_temperatureInfo& temperatureInfo = dcsInfo.getTemperatureInfo();
      
      double actualTemperature = temperatureInfo.getActualTemperature();
      
      unsigned int actualTemperatureCycleType = temperatureInfo.getTemperatureCycleType();

      if ( actualTemperatureCycleType == temperatureCycleType ) {

        for ( rawDataType_TempRange::const_iterator rawDataEntry_TempRange = rawDataEntry_temperature->first.begin();
	      rawDataEntry_TempRange != rawDataEntry_temperature->first.end(); ++rawDataEntry_TempRange ) {
	  unsigned int TempRange = rawDataEntry_TempRange->first;

//--- check that TempRange DAC setting corresponds to measurement mode
	  if ( TempRange >= 0 && TempRange < numTempRanges ) {

//--- check that actual temperature 
//    is within the range of temperatures included in the fit 
//    of measurementFunction_[iTempRange]
            double validityRange_minTemperature, validityRange_maxTemperature;
	    measurementFunction_[TempRange]->GetRange(validityRange_minTemperature, validityRange_maxTemperature);
	    if ( actualTemperature >= validityRange_minTemperature && actualTemperature <= validityRange_maxTemperature ) {
	      for ( std::list<unsigned int>::const_iterator adcValue = rawDataEntry_TempRange->second.begin();
		    adcValue != rawDataEntry_TempRange->second.end(); ++adcValue ) {
	        double estimatedTemperature = getCalibratedTemperature(*adcValue, TempRange);
	        precisionGraph->SetPoint(precisionGraph->GetN(), actualTemperature, estimatedTemperature - actualTemperature);
	      }
	    }
	  }
	}
      }
    }
  } else {
    std::cerr << "Error in <PixelReadOutChipData_fixedPower::fillPrecisionGraph>:" 
	      << " temperatureCycleType = " << temperatureCycleType << " not defined !!!" << std::endl;
  }
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelReadOutChipData_fixedPower::fitCalibrationData()
{
  calibrationFit_->Command("SET PRINT -1"); // run minuit in silent mode, omits lots of output
  calibrationFit_->SetFCN(fitCalibrationData_fcn);

  Int_t error = 0;
  calibrationFit_->mnparm(0, "a0", -2.5e-4,  2.5e-5, -2.5e-3, 2.5e-3, error);
  calibrationFit_->mnparm(1, "a1",  1.e-1,   1.e-2,  -1.,     1.,     error);
  calibrationFit_->mnparm(2, "b0", -5.e-2,   5.e-3,  -5.e-1,  5.e-1,  error);
  calibrationFit_->mnparm(3, "b1", -1.e1,    1.,     -1.e2,   1.e2,   error);

  gGraphArray_fit = calibrationGraphs_;

  calibrationFit_->mncomd("SET STRATEGY 0", error);

  calibrationFit_->mnexcm("MIGRAD", 0, 0, error);
  //calibrationFit_->mnexcm("SIMPLEX", 0, 0, error);
  //calibrationFit_->mnexcm("MINIMIZE", 0, 0, error);

  std::cout << "--> Status of two-dimensional deltaV(ADC,T) Fit = " << gMinuit->GetStatus() << " : " << std::endl;

  Double_t a0, a1, b0, b1, dummy;
  calibrationFit_->GetParameter(0, a0, dummy);
  calibrationFit_->GetParameter(1, a1, dummy);
  calibrationFit_->GetParameter(2, b0, dummy);
  calibrationFit_->GetParameter(3, b1, dummy);

  std::cout << " fitted Parameter a0 = " << a0 << std::endl;
  std::cout << " fitted Parameter a1 = " << a1 << std::endl;
  std::cout << " fitted Parameter b0 = " << b0 << std::endl;
  std::cout << " fitted Parameter b1 = " << b1 << std::endl;
  
  calibrationFunction_slope->SetParameter(0, a0);
  calibrationFunction_slope->SetParameter(1, a1);
  calibrationFunction_offset->SetParameter(0, b0);
  calibrationFunction_offset->SetParameter(1, b1);
}

void PixelReadOutChipData_fixedPower::fitMeasurementData()
{
//--- create temporary graph objects
//    used by fit
  for ( unsigned int iTempRange = 0; iTempRange < numTempRanges; ++iTempRange ) {
    TGraph*& tempGraph = tempGraphs_fit_[iTempRange];

    tempGraph = new TGraph();
    TString graphName = Form("tempGraph_TempRange%02X", iTempRange);
    tempGraph->SetName(graphName);
  }

//--- fill temporary graph objects
  for ( unsigned int iTempRange = 0; iTempRange < numTempRanges; ++iTempRange ) {
    TGraph* graph_measurement = measurementGraphs_[iTempRange];
    TGraph* tempGraph_fit = tempGraphs_fit_[iTempRange];
    
    unsigned int numPoints_fit = 0;
    
    unsigned int numTemperatures = graph_measurement->GetN();
    for ( unsigned int iTemperature = 0; iTemperature < numTemperatures; ++iTemperature ) {
      double temperature, adcValue;
      graph_measurement->GetPoint(iTemperature, temperature, adcValue);
      
      double voltageDifference = calibrationFunction_slope->Eval(temperature)*adcValue + calibrationFunction_offset->Eval(temperature);
      double voltage = referenceVoltage[iTempRange] + voltageDifference;
      
      std::cout << "T = " << temperature << ":" 
		<< " ADC value = " << adcValue << ","
		<< " TempRange = " << iTempRange
		<< " --> Voltage difference = " << voltageDifference << "," 
		<< " sensor Voltage = " << voltage << std::endl;

      if ( adcValue >= (adcValue_blackLevel + adcValue_threshold) ) {
	tempGraph_fit->SetPoint(numPoints_fit, temperature, voltage);
	++numPoints_fit;
      }
    }
  }

//--- fit temporary graph objects
  for ( unsigned int iTempRange = 0; iTempRange < numTempRanges; ++iTempRange ) {
    TGraph* tempGraph = tempGraphs_fit_[iTempRange];

    if ( tempGraph->GetN() >= 2 ){
//--- restrict fit to temperature range in which ADC as function of sensor voltage is linear,
//    i.e. to the range where the difference deltaV between the sensor voltage and the reference voltage is not too large
//    (--> for a given temperature, choose the measurement at the higher reference voltage if its ADC value is above black level threshold)
      double temperature_firstPoint, temperature_fourthPoint, temperature_lastPoint, dummy;

      tempGraph->GetPoint(0, temperature_firstPoint, dummy);
      tempGraph->GetPoint(3, temperature_fourthPoint, dummy);
      tempGraph->GetPoint(tempGraph->GetN() - 1, temperature_lastPoint, dummy);
      
      double minTemperature_fit, maxTemperature_fit;

      const double epsilon = 0.25;

      if ( iTempRange <= 6 && tempGraphs_fit_[iTempRange + 1]->GetN() >= 4 ) {
	TGraph* tempGraph_nextTempRange = tempGraphs_fit_[iTempRange + 1];

	double temperature_firstPoint_nextTempRange, temperature_fourthPoint_nextTempRange;
	tempGraph_nextTempRange->GetPoint(0, temperature_firstPoint_nextTempRange, dummy);
	tempGraph_nextTempRange->GetPoint(3, temperature_fourthPoint_nextTempRange, dummy);

	if ( TMath::Abs(temperature_firstPoint - temperature_firstPoint_nextTempRange) < epsilon ) {
//--- exclude graph for current TempRange from fit;
//    use graph for TempRange + 1 instead
	  minTemperature_fit =  1.e-3;
	  maxTemperature_fit = -1.e-3;
	} else {
//--- include in fit part of graph 
//    that is not covered by graph for TempRange + 1
	  minTemperature_fit = temperature_firstPoint;
	  maxTemperature_fit = TMath::Max(temperature_fourthPoint_nextTempRange, temperature_firstPoint_nextTempRange);
	}
      } else {
//--- graph for TempRange + 1 does not have enough points above black level threshold to be fitted;
//    include **all** points of graph for current TempRange in fit
	minTemperature_fit = temperature_firstPoint;
	maxTemperature_fit = temperature_lastPoint;
      }

      std::cout << "fitting measurement function for TempRange = " << iTempRange << std::endl;
      
      TF1* measurementFunction_TempRange = measurementFunction_[iTempRange];

      tempGraph->Fit(measurementFunction_TempRange, "QWN", "", minTemperature_fit - epsilon, maxTemperature_fit + epsilon);
      
      double slope = measurementFunction_TempRange->GetParameter(0);
      double offset = measurementFunction_TempRange->GetParameter(1);
      
      std::cout << "--> fit result: slope = " << slope << ", offset = " << offset << std::endl;
      std::cout << " validityRange = [" << minTemperature_fit - epsilon << ", " << maxTemperature_fit + epsilon << "]" << std::endl;

//--- set validity interval of fitted function
//    to range included in fit
      measurementFunction_TempRange->SetRange(minTemperature_fit - epsilon, maxTemperature_fit + epsilon);
    }
  } 
}

//
//---------------------------------------------------------------------------------------------------
//

const PixelReadOutChipData_powerInfo* PixelReadOutChipData_fixedPower::getPowerInfo(double nominalTemperature) const
{
  typedef std::list<std::pair<std::map<unsigned int, double>, PixelReadOutChipData_dcsInfo> > rawDataType_temperature;

  for ( rawDataType_temperature::const_iterator rawDataEntry_temperature = rawData_averaged_.begin();
	rawDataEntry_temperature != rawData_averaged_.end(); ++rawDataEntry_temperature ) {
    const PixelReadOutChipData_dcsInfo& dcsInfo = rawDataEntry_temperature->second;

    double temperature = dcsInfo.getTemperatureInfo().getNominalTemperature();

    const double epsilon = 0.25;

    if ( TMath::Abs(temperature - nominalTemperature) < epsilon ) {
      return &(dcsInfo.getPowerInfo());
    }
  }

  std::cerr << "Error in <PixelReadOutChipData_fixedPower::getPowerInfo>:" 
	    << " no DCS Information available for nominal Temperature = " << nominalTemperature << " !!!" << std::endl;
  return NULL;
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelReadOutChipData_fixedPower::Write()
{
  TString calibrationCanvas_name = Form("%s_%s", "calibrationCanvas", name_.data());
  createCanvas(calibrationCanvas_, calibrationCanvas_name.Data(), "reference Voltage Calibration");
  calibrationCanvas_->cd();
  drawGraphs(calibrationGraphs_, "L", "T_{Cooling-Box} / degrees", "ADC");
  calibrationCanvas_->Write();

  TString measurementCanvas_name = Form("%s_%s", "measurementCanvas", name_.data());
  createCanvas(measurementCanvas_, measurementCanvas_name.Data(), "Temperature Sensor Output");
  measurementCanvas_->cd();
  drawGraphs(measurementGraphs_, "L", "T_{Cooling-Box} / degrees", "ADC");
  measurementCanvas_->Write();

  TString fitCanvas_name = Form("%s_%s", "fitCanvas", name_.data());
  createCanvas(fitCanvas_, fitCanvas_name.Data(), "Temperature Dependence of Sensor Voltage");
  fitCanvas_->cd();
  drawGraphs(tempGraphs_fit_, "P", measurementFunction_, "T_{Cooling-Box} / degrees", "V_{Sensor} / mV");
  fitCanvas_->Write();

  TString linearityCanvas_name = Form("%s_%s", "linearityCanvas", name_.data());
  createCanvas(linearityCanvas_, linearityCanvas_name.Data(), "Linearity of Temperature Readout");
  linearityCanvas_->cd();
  drawGraphs(linearityGraphs_, "P", "T_{Cooling-Box} / degrees", "T_{last DAC} / degrees");
  linearityCanvas_->Write();

  TString precisionCanvas_name = Form("%s_%s", "precisionCanvas", name_.data());
  createCanvas(precisionCanvas_, precisionCanvas_name.Data(), "Precision of Temperature Readout");
  precisionCanvas_->cd();
  drawGraphs(precisionGraphs_, "T_{Cooling-Box} / degrees", "T_{last DAC} - T_{Cooling-Box} / degrees");
  precisionCanvas_->Write();
}

//
//---------------------------------------------------------------------------------------------------
//

void fillGraph(TGraph* graph, const std::list<std::pair<std::map<unsigned int, double>, PixelReadOutChipData_dcsInfo> >& rawData, unsigned int TempRange)
{
  typedef std::list<std::pair<std::map<unsigned int, double>, PixelReadOutChipData_dcsInfo> > rawDataType_temperature;
  typedef std::map<unsigned int, double> rawDataType_TempRange;
  
  unsigned int graph_numPoints = 0;
  for ( rawDataType_temperature::const_iterator rawDataEntry_temperature = rawData.begin();
	rawDataEntry_temperature != rawData.end(); ++rawDataEntry_temperature ) {
    const PixelReadOutChipData_dcsInfo& dcsInfo = rawDataEntry_temperature->second;

    double temperature = dcsInfo.getTemperatureInfo().getNominalTemperature();

    for ( rawDataType_TempRange::const_iterator rawDataEntry_TempRange = rawDataEntry_temperature->first.begin();
	  rawDataEntry_TempRange != rawDataEntry_temperature->first.end(); ++rawDataEntry_TempRange ) {
      unsigned int TempRange_setting = rawDataEntry_TempRange->first;

      if ( TempRange_setting == TempRange ) {
	double adcValue = rawDataEntry_TempRange->second;
      
	//std::cout << " adcValue = " << adcValue << std::endl;
	//std::cout << " adcValue_blackLevel = " << adcValue_blackLevel << std::endl;
	//std::cout << " adcValue_threshold = " << adcValue_threshold << std::endl;

	if ( adcValue >= (adcValue_blackLevel + adcValue_threshold) ) {
	  graph->SetPoint(graph_numPoints, temperature, adcValue);
	  ++graph_numPoints;
	}
      }
    }
  }
}

//
//---------------------------------------------------------------------------------------------------
//

void drawGraphs(TGraph* graphs[numTempRanges], const char* drawOption, const char* labelAxisX, const char* labelAxisY)
{
  std::cout << "<drawGraphs>:" << std::endl;

  double xMin, xMax, yMin, yMax;

  xMin =  1e6;
  xMax = -1e6;
  
  yMin =  1e6;
  yMax = -1e6;

  for ( unsigned int iTempRange = 0; iTempRange < numTempRanges; ++iTempRange ) {
    double xMin_TempRange, xMax_TempRange, yMin_TempRange, yMax_TempRange;
    getDimension(graphs[iTempRange], xMin_TempRange, xMax_TempRange, yMin_TempRange, yMax_TempRange);
    
    if ( xMin_TempRange < xMin ) xMin = xMin_TempRange;
    if ( xMax_TempRange > xMax ) xMax = xMax_TempRange;
    
    if ( yMin_TempRange < yMin ) yMin = yMin_TempRange;
    if ( yMax_TempRange > yMax ) yMax = yMax_TempRange;
  }

  std::cout << "yMin = " << yMin << ", yMax = " << yMax << std::endl;

//--- initialise dummy histogram
//    (neccessary for drawing graphs)
  TString dummyHistogramName = TString("dummyHistogram_") + graphs[0]->GetName();
  TString dummyHistogramTitle = dummyHistogramName;
  TH1* dummyHistogram = new TH1D(dummyHistogramName, dummyHistogramTitle, 10, xMin, xMax);
  dummyHistogram->SetTitle("");
  dummyHistogram->SetStats(false);
  dummyHistogram->GetXaxis()->SetTitle(labelAxisX);
  dummyHistogram->GetXaxis()->SetTitleOffset(1.2);
  dummyHistogram->GetYaxis()->SetTitle(labelAxisY);
  dummyHistogram->GetYaxis()->SetTitleOffset(1.3);
  dummyHistogram->SetMinimum(1.35*yMin);
  dummyHistogram->SetMaximum(1.35*yMax);
  dummyHistogram->Draw();

  TLegend* legendTempRanges = new TLegend(0.13, 0.57, 0.38, 0.87, NULL, "brNDC");
  legendTempRanges->SetFillColor(10);
  legendTempRanges->SetLineColor(10);

  for ( unsigned int iTempRange = 0; iTempRange < numTempRanges; ++iTempRange ) {
    TGraph* graph = graphs[iTempRange];

    std::cout << "graph, name = " << graph->GetName() << ","
	      << " has " << graph->GetN() << " points." << std::endl;

    graph->SetLineColor((iTempRange % 8) + 1);
    graph->SetLineStyle((iTempRange / 8) + 1);
    graph->SetLineWidth(2);
    graph->Draw(drawOption);

    TString labelLegend = Form("V_{reference} = %3.2f", referenceVoltage[iTempRange]);
    legendTempRanges->AddEntry(graph, labelLegend, drawOption);
  }

  legendTempRanges->Draw();
}

void drawGraphs(TGraph* graphs[numTempRanges], const char* drawOption, TF1* functions[numTempRanges], const char* labelAxisX, const char* labelAxisY)
{
  drawGraphs(graphs, drawOption, labelAxisX, labelAxisY);

  for ( unsigned int iTempRange = 0; iTempRange < numTempRanges; ++iTempRange ) {
    TF1* function = functions[iTempRange];

    function->SetLineColor((iTempRange % 8) + 1);
    function->Draw("same");
  }
}

void drawGraphs(TGraph* graphs[numTempCycleTypes], const char* labelAxisX, const char* labelAxisY)
{
  double xMin, xMax, yMin, yMax;

  xMin =  1e6;
  xMax = -1e6;
  
  yMin =  1e6;
  yMax = -1e6;

  for ( unsigned int iTempCycleType = 0; iTempCycleType < numTempCycleTypes; ++iTempCycleType ) {
    double xMin_TempCycleType, xMax_TempCycleType, yMin_TempCycleType, yMax_TempCycleType;
    getDimension(graphs[iTempCycleType], xMin_TempCycleType, xMax_TempCycleType, yMin_TempCycleType, yMax_TempCycleType);
    
    if ( xMin_TempCycleType < xMin ) xMin = xMin_TempCycleType;
    if ( xMax_TempCycleType > xMax ) xMax = xMax_TempCycleType;
    
    if ( yMin_TempCycleType < yMin ) yMin = yMin_TempCycleType;
    if ( yMax_TempCycleType > yMax ) yMax = yMax_TempCycleType;
  }

//--- initialise dummy histogram
//    (neccessary for drawing graphs)
  TString dummyHistogramName = TString("dummyHistogram_") + graphs[0]->GetName();
  TString dummyHistogramTitle = dummyHistogramName;
  TH1* dummyHistogram = new TH1D(dummyHistogramName, dummyHistogramTitle, 10, xMin, xMax);
  dummyHistogram->SetTitle("");
  dummyHistogram->SetStats(false);
  dummyHistogram->GetXaxis()->SetTitle(labelAxisX);
  dummyHistogram->GetXaxis()->SetTitleOffset(1.2);
  dummyHistogram->GetYaxis()->SetTitle(labelAxisY);
  dummyHistogram->GetYaxis()->SetTitleOffset(1.3);
  dummyHistogram->SetMinimum(1.2*yMin);
  dummyHistogram->SetMaximum(1.2*yMax);
  dummyHistogram->Draw();

  TGraph* graph_coolingCycles = graphs[PixelReadOutChipData_temperatureInfo::kCoolingCycle];
  graph_coolingCycles->SetLineColor(4); // blue
  graph_coolingCycles->SetLineWidth(2);
  graph_coolingCycles->Draw("P");

  TGraph* graph_heatingCycles = graphs[PixelReadOutChipData_temperatureInfo::kHeatingCycle];
  graph_heatingCycles->SetLineColor(2); // red
  graph_heatingCycles->SetLineWidth(2);
  graph_heatingCycles->Draw("P");

  TLegend* legendTempCycleTypes = new TLegend(0.13, 0.57, 0.38, 0.87, NULL, "brNDC");
  legendTempCycleTypes->SetFillColor(10);
  legendTempCycleTypes->SetLineColor(10);
  legendTempCycleTypes->AddEntry(graph_coolingCycles, "Cooling Cycles", "P");
  legendTempCycleTypes->AddEntry(graph_coolingCycles, "Heating Cycles", "P");
  legendTempCycleTypes->Draw();
}

//
//---------------------------------------------------------------------------------------------------
//

void fitCalibrationData_fcn(Int_t& npar, Double_t* gin, Double_t& f, Double_t* parameter, Int_t iflag)
{
  Double_t chi2 = 0;

  Double_t a0 = parameter[0];
  Double_t a1 = parameter[1];
  Double_t b0 = parameter[2];
  Double_t b1 = parameter[3];

  for ( unsigned int index_TempRange = 0; index_TempRange < numTempRanges; ++index_TempRange ) {
    TGraph* graph = gGraphArray_fit[index_TempRange];

    double voltageDifference_calibration = calibrationVoltage - referenceVoltage[index_TempRange];

    unsigned int numTemperatures = graph->GetN();
    for ( unsigned int iTemperature = 0; iTemperature < numTemperatures; ++iTemperature ) {
      double temperature, adcValue;

      graph->GetPoint(iTemperature, temperature, adcValue);

      double voltageDifference_fit = (a0*temperature + a1)*adcValue + (b0*temperature + b1);

      chi2 += TMath::Power(voltageDifference_calibration - voltageDifference_fit, 2);
    }
  }

  std::cout << "a0 = " << a0 << ", a1 = " << a1 << ", b0 = " << b0 << ", b1 = " << b1 << " : chi2 = " << chi2 << std::endl;

  f = chi2;
}

//
//---------------------------------------------------------------------------------------------------
//

void getDimension(TGraph* graph, double& xMin, double& xMax, double& yMin, double& yMax)
{
  if ( graph == NULL ) {
    std::cerr << "Error in <getDimension>: graph == NULL !!!" << std::endl;
    return; 
  }

  xMin =  1e6;
  xMax = -1e6;

  yMin =  1e6;
  yMax = -1e6;

  unsigned int numPoints = graph->GetN();
  for ( unsigned int iPoint = 0; iPoint < numPoints; ++iPoint ) {
    double x, y;
    graph->GetPoint(iPoint, x, y);

    if ( x < xMin ) xMin = x;
    if ( x > xMax ) xMax = x;

    if ( y < yMin ) yMin = y;
    if ( y > yMax ) yMax = y;    
  }
}

//
//---------------------------------------------------------------------------------------------------
//

void createCanvas(TCanvas*& canvas, const char* name, const char* title)
{
//--- create canvases if neccessary

  if ( canvas == NULL ) {
    std::cout << "creating new Canvas, name = " << name << ", title = " << title << std::endl;    
    canvas = new TCanvas(name, title, 800, 600);
    canvas->SetFillColor(10);
    canvas->SetBorderSize(2);
  }
}
