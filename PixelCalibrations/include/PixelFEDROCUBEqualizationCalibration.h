/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFEDROCUBEqualizationCalibration_h_
#define _PixelFEDROCUBEqualizationCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"

#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelScanRecord.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"

#include <map>

class PixelFEDROCUBEqualizationCalibration: public PixelFEDCalibrationBase {
 public:

  PixelFEDROCUBEqualizationCalibration( const PixelFEDSupervisorConfiguration &, SOAPCommander* );
  virtual ~PixelFEDROCUBEqualizationCalibration(){};

  virtual xoap::MessageReference execute(xoap::MessageReference msg);

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  virtual void initializeFED();

 private:

  // PixelFEDROCUBEqualizationCalibration Constructor should never be called
  //PixelFEDROCUBEqualizationCalibration();

  // Calibration steps
  void RetrieveData(unsigned int state);
  void Analyze();

  // variables to store averages
  std::map <unsigned int, map <unsigned int, Moments> > TBM_UB_;
  std::map <unsigned int, map <unsigned int, Moments> > TBM_B_;
  //        FED number,        Channel

  std::map <pos::PixelROCName, PixelScanRecord > ROC_UB_;
  std::map <pos::PixelROCName, PixelScanRecord > ROC_B_;

   struct branch{ 
    float pass; 
    char rocName[38];
  }; 

   struct branch_sum{
    float new_VIbias_DAC;
    float delta_VIbias_DAC;
    char rocName[38];


  };

};

#endif
