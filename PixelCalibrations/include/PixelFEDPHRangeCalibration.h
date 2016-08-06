/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFEDPHRangeCalibration_h_
#define _PixelFEDPHRangeCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"

#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelScanRecord.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"

#include <map>

class TGraph2DErrors;

class PixelFEDPHRangeCalibration: public PixelFEDCalibrationBase {
 public:

  PixelFEDPHRangeCalibration( const PixelFEDSupervisorConfiguration &, SOAPCommander* );
  virtual ~PixelFEDPHRangeCalibration(){};

  virtual xoap::MessageReference execute(xoap::MessageReference msg);

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  virtual void initializeFED();

 private:

  // PixelFEDPHRangeCalibration Constructor should never be called
  //PixelFEDPHRangeCalibration();

  // Calibration steps
  void RetrieveData(unsigned int state);
  void Analyze();
  
  void WritePlot( TGraph2DErrors* graph, const std::map<std::string, unsigned int>& DACSettings, const std::vector<std::string>& plotAxisTitles ) const;

  std::map <pos::PixelROCName, std::map <std::map<std::string, unsigned int>, PixelScanRecord> > PH_vs_Vcal_;
  //                                         DAC name          DAC value

   struct branch{ 
    float pass; 
    char rocName[38];
  }; 

   struct branch_sum{
    float new_VIbias_PH;
    float delta_VIbias_PH;
    float new_VOffsetOp;
    float delta_VOffsetOp;
    float new_VIon;
    float delta_VIon;
    float new_VOffsetRO;
    float delta_VOffsetRO;
    char rocName[38];
};


};

#endif
