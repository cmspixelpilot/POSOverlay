#include "PixelCalibrations/macros/include/PixelReadOutChipData_varyingPower.h"

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
 * Last update: $Date: 2007/11/09 17:19:19 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#include <iostream>
#include <iomanip>

#include <TGraph.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TLegend.h>

unsigned int PixelReadOutChipData_varyingPower::instanceId = 0;

PixelReadOutChipData_varyingPower::PixelReadOutChipData_varyingPower(const std::string& name, 
								     const PixelReadOutChipData_fixedPower& dataSet_reducedPower,
								     const PixelReadOutChipData_fixedPower& dataSet_nominalPower)
  : dataSet_reducedPower_(dataSet_reducedPower), dataSet_nominalPower_(dataSet_nominalPower)
{
  TString objectName = Form("%s_id%06X", name.data(), instanceId);
  ++instanceId;
  name_ = objectName.Data();

  std::cout << "<PixelReadOutChipData_varyingPower::PixelReadOutChipData_varyingPower(const PixelReadOutChipData_fixedPower&, .. )>:" << std::endl;
  std::cout << " name = " << name_ << std::endl;

  temperatureDifferenceGraph1_ = NULL;
  temperatureDifferenceGraph2_ = NULL;
  temperatureDifferenceCanvas_ = NULL;
}

PixelReadOutChipData_varyingPower::PixelReadOutChipData_varyingPower(const PixelReadOutChipData_varyingPower& varyingPower)
  : dataSet_reducedPower_(varyingPower.dataSet_reducedPower_), dataSet_nominalPower_(varyingPower.dataSet_nominalPower_)
{
  TString objectName = Form("%s_id%06X", varyingPower.name_.data(), instanceId);
  ++instanceId;
  name_ = objectName.Data();
  
  std::cout << "<PixelReadOutChipData_varyingPower::PixelReadOutChipData_varyingPower(const PixelReadOutChipData_varyingPower& )>:" << std::endl;
  std::cout << " name = " << name_ << std::endl;

  if ( varyingPower.temperatureDifferenceGraph1_ != NULL ) temperatureDifferenceGraph1_ = new TGraph(*varyingPower.temperatureDifferenceGraph1_);
  if ( varyingPower.temperatureDifferenceGraph2_ != NULL ) temperatureDifferenceGraph2_ = new TGraph(*varyingPower.temperatureDifferenceGraph2_);
  temperatureDifferenceCanvas_ = NULL;
}

PixelReadOutChipData_varyingPower::~PixelReadOutChipData_varyingPower()
{
  std::cout << "<PixelReadOutChipData_varyingPower::~PixelReadOutChipData_varyingPower>:" << std::endl;
  std::cout << " name = " << name_ << std::endl;

  delete temperatureDifferenceGraph1_;
  delete temperatureDifferenceGraph2_;
  delete temperatureDifferenceCanvas_;
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelReadOutChipData_varyingPower::Calibrate() 
{
  typedef std::list<std::pair<std::map<unsigned int, double>, PixelReadOutChipData_dcsInfo> > rawDataType;

  const rawDataType& rawData_reducedPower = dataSet_reducedPower_.getAveragedRawData();
  const rawDataType& rawData_nominalPower = dataSet_nominalPower_.getAveragedRawData();

  double temperatureDifference_sum = 0.;
  unsigned int temperatureDifference_numSummands = 0;

//--- create graphs if neccessary
  if ( temperatureDifferenceGraph1_ == NULL ) {
    temperatureDifferenceGraph1_ = new TGraph();
    temperatureDifferenceGraph1_->SetName("temperatureDifferenceGraph01");
  }

  if ( temperatureDifferenceGraph2_ == NULL ) {
    temperatureDifferenceGraph2_ = new TGraph();
    temperatureDifferenceGraph2_->SetName("temperatureDifferenceGraph02");
  }

  unsigned int graph_numPoints = 0;

  rawDataType::const_iterator rawDataEntry_reducedPower = rawData_reducedPower.begin();
  rawDataType::const_iterator rawDataEntry_nominalPower = rawData_nominalPower.begin();
  for ( ; rawDataEntry_reducedPower != rawData_reducedPower.end() && rawDataEntry_nominalPower != rawData_nominalPower.end(); 
  	++rawDataEntry_reducedPower, ++rawDataEntry_nominalPower ) {

    const std::map<unsigned int, double>& adcValues_reducedPower = rawDataEntry_reducedPower->first;
    const PixelReadOutChipData_dcsInfo& dcsInfo_reducedPower = rawDataEntry_reducedPower->second;

    const std::map<unsigned int, double>& adcValues_nominalPower = rawDataEntry_nominalPower->first;
    const PixelReadOutChipData_dcsInfo& dcsInfo_nominalPower = rawDataEntry_nominalPower->second;
    
    if ( dcsInfo_reducedPower.getTemperatureInfo().getNominalTemperature() != dcsInfo_nominalPower.getTemperatureInfo().getNominalTemperature() ) {
      std::cerr << "Error in <PixelReadOutChipData_varyingPower::getThermalConductance>:"
		<< " nominal Temperatures of the two Data-Sets do not match !!!" << std::endl;
      temperatureDifference_ = 0;
      return;
    }

    double nominalTemperature = dcsInfo_nominalPower.getTemperatureInfo().getNominalTemperature();

    double tempADCValues_reducedPower[numTempRanges];
    double tempADCValues_nominalPower[numTempRanges];
    for ( unsigned int iTempRange = 0; iTempRange < numTempRanges; ++iTempRange ) {
      std::map<unsigned int, double>::const_iterator adcValue_reducedPower = adcValues_reducedPower.find(iTempRange);
      std::map<unsigned int, double>::const_iterator adcValue_nominalPower = adcValues_nominalPower.find(iTempRange);

      if ( adcValue_reducedPower != adcValues_reducedPower.end() &&
	   adcValue_nominalPower != adcValues_nominalPower.end() ) {
	tempADCValues_reducedPower[iTempRange] = adcValue_reducedPower->second;
	tempADCValues_nominalPower[iTempRange] = adcValue_nominalPower->second;
      } else {
	std::cerr << "Error in <PixelReadOutChipData_varyingPower::getThermalConductance>:"
		  << " no raw Data available for TempRange = " << iTempRange << " !!!" << std::endl;
	temperatureDifference_ = 0;
	return;
      }
    }

    double estimatedTemperature1_1 = dataSet_reducedPower_.getCalibratedTemperature(tempADCValues_reducedPower);
    double estimatedTemperature1_2 = dataSet_reducedPower_.getCalibratedTemperature(tempADCValues_nominalPower);
    double estimatedTemperature2_1 = dataSet_nominalPower_.getCalibratedTemperature(tempADCValues_reducedPower);
    double estimatedTemperature2_2 = dataSet_nominalPower_.getCalibratedTemperature(tempADCValues_nominalPower);

    const PixelReadOutChipData_powerInfo* powerInfo_reducedPower = dataSet_reducedPower_.getPowerInfo(nominalTemperature);
    const PixelReadOutChipData_powerInfo* powerInfo_nominalPower = dataSet_nominalPower_.getPowerInfo(nominalTemperature);
    if ( powerInfo_reducedPower != NULL && 
	 powerInfo_nominalPower != NULL ) {
      double powerDissipation_reducedPower = powerInfo_reducedPower->getPowerConsumption();
      double powerDissipation_nominalPower = powerInfo_nominalPower->getPowerConsumption();

      double thermalConductance = TMath::Abs(powerDissipation_reducedPower - powerDissipation_nominalPower)/
	                    (0.5*(TMath::Abs(estimatedTemperature1_1 - estimatedTemperature1_2) + TMath::Abs(estimatedTemperature2_1 - estimatedTemperature2_2)));

      std::cout << "nominalTemperature = " << nominalTemperature << std::endl;
      std::cout << " powerDissipation_reducedPower = " << powerDissipation_reducedPower << std::endl;
      std::cout << " powerDissipation_nominalPower = " << powerDissipation_nominalPower << std::endl;
      std::cout << " estimatedTemperature1_1 - estimatedTemperature1_2 = " << estimatedTemperature1_1 - estimatedTemperature1_2 << std::endl;
      std::cout << " estimatedTemperature2_1 - estimatedTemperature2_2 = " << estimatedTemperature2_1 - estimatedTemperature2_2 << std::endl;
      std::cout << "--> thermalConductance = " << thermalConductance << std::endl;

      temperatureDifferenceGraph1_->SetPoint(graph_numPoints, nominalTemperature, estimatedTemperature1_1 - estimatedTemperature1_2); 
      temperatureDifferenceGraph2_->SetPoint(graph_numPoints, nominalTemperature, estimatedTemperature2_1 - estimatedTemperature2_2);
      ++graph_numPoints;

      double temperatureDifference1 = powerDissipation_reducedPower/thermalConductance;
      double temperatureDifference2 = powerDissipation_nominalPower/thermalConductance;
      
      temperatureDifference_sum += TMath::Max(temperatureDifference1,
					      temperatureDifference2);
      ++temperatureDifference_numSummands;
    } else {
      std::cerr << "Warning in <PixelReadOutChipData_varyingPower::getTemperatureDifference>:" 
		<< " insufficient Information about Power Dissipation available for nominal Temperature = " << nominalTemperature << " !!!" << std::endl;
    }
  }

  if ( temperatureDifference_numSummands == 0 ) {
    std::cerr << "Error in <PixelReadOutChipData_varyingPower::getThermalConductance>:"
	      << " failed to decode Data-Sets !!!" << std::endl;
    temperatureDifference_ = 0;
    return;
  }

  temperatureDifference_ = temperatureDifference_sum/temperatureDifference_numSummands;
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelReadOutChipData_varyingPower::Write()
{
//--- create canvas if neccessary
  if ( temperatureDifferenceCanvas_ == NULL ) {
    TString temperatureDifferenceCanvas_name = Form("%s_%s", "temperatureDifferenceCanvas", name_.data());
    temperatureDifferenceCanvas_ = new TCanvas(temperatureDifferenceCanvas_name.Data(), "temperatureDifferenceCanvas", 800, 600);
    temperatureDifferenceCanvas_->SetFillColor(10);
    temperatureDifferenceCanvas_->SetBorderSize(2);
  }

  temperatureDifferenceCanvas_->cd();

  double xMin1, xMax1, yMin1, yMax1;
  getDimension(temperatureDifferenceGraph1_, xMin1, xMax1, yMin1, yMax1);
  double xMin2, xMax2, yMin2, yMax2;
  getDimension(temperatureDifferenceGraph2_, xMin2, xMax2, yMin2, yMax2);

//--- initialise dummy histogram
//    (neccessary for drawing graphs)
  TH1* dummyHistogram = new TH1D("dummyHistogram", "dummyHistogram", 10, TMath::Min(xMin1, xMin2), TMath::Max(xMax1, xMax2));
  dummyHistogram->SetTitle("");
  dummyHistogram->SetStats(false);
  dummyHistogram->GetXaxis()->SetTitle("T / degrees");
  dummyHistogram->GetXaxis()->SetTitleOffset(1.2);
  dummyHistogram->GetYaxis()->SetTitle("#Delta T / degrees");
  dummyHistogram->GetYaxis()->SetTitleOffset(1.3);
  dummyHistogram->SetMinimum(1.2*TMath::Min(yMin1, yMin2));
  dummyHistogram->SetMaximum(1.2*TMath::Max(yMax1, yMax2));
  dummyHistogram->Draw();

  temperatureDifferenceGraph1_->SetMarkerStyle(2);
  temperatureDifferenceGraph1_->SetMarkerSize(2);
  temperatureDifferenceGraph1_->SetMarkerColor(4);
  temperatureDifferenceGraph1_->Draw("P");

  temperatureDifferenceGraph2_->SetMarkerStyle(2);
  temperatureDifferenceGraph2_->SetMarkerSize(2);
  temperatureDifferenceGraph2_->SetMarkerColor(4);
  temperatureDifferenceGraph2_->Draw("P");

  TLegend* legend = new TLegend(0.59, 0.15, 0.86, 0.31, NULL, "brNDC");
  legend->SetFillColor(10);
  legend->SetLineColor(10);
  legend->AddEntry(temperatureDifferenceGraph1_, "Calibration 1", "p");
  legend->AddEntry(temperatureDifferenceGraph2_, "Calibration 2", "p");
  legend->Draw();

  temperatureDifferenceCanvas_->Write();
}
