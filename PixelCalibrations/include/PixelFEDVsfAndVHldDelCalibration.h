/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFEDVsfAndVHldDelCalibration_h_
#define _PixelFEDVsfAndVHldDelCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"

#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelScanRecord.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"

#include <map>

class PixelFEDVsfAndVHldDelCalibration: public PixelFEDCalibrationBase {
 public:

  PixelFEDVsfAndVHldDelCalibration( const PixelFEDSupervisorConfiguration &, SOAPCommander* );
  virtual ~PixelFEDVsfAndVHldDelCalibration(){};

  virtual xoap::MessageReference execute(xoap::MessageReference msg);

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  virtual void initializeFED();

 private:

  // PixelFEDVsfAndVHldDelCalibration Constructor should never be called
  //PixelFEDVsfAndVHldDelCalibration();

  // Calibration steps
  void RetrieveData(unsigned int state);
  void Analyze();
  
  unsigned int VsfFromConfig(const pos::PixelROCName& roc);

  std::map <pos::PixelROCName, map <unsigned int, PixelScanRecord> > PH_vs_VHldDel_;
  //                                Vsf value
  
  std::map <pos::PixelROCName, unsigned int> VsfValuesFromConfig_;
  
   struct branch{
    float pass;
    char rocName[38];
  };

   struct branch_sum{
    float delta_Vsf;
    float new_Vsf;
    float delta_VHldDel;
    float new_VHldDel;
    char rocName[38];
  };

};

#endif
