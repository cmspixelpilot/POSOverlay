/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFEDLinearityVsVsfCalibration_h_
#define _PixelFEDLinearityVsVsfCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"

#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelScanRecord.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"

#include <map>

class PixelFEDLinearityVsVsfCalibration: public PixelFEDCalibrationBase {
 public:

  PixelFEDLinearityVsVsfCalibration( const PixelFEDSupervisorConfiguration &, SOAPCommander* );
  virtual ~PixelFEDLinearityVsVsfCalibration(){};

  virtual xoap::MessageReference execute(xoap::MessageReference msg);

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  virtual void initializeFED();

 private:

  // PixelFEDLinearityVsVsfCalibration Constructor should never be called
  //PixelFEDLinearityVsVsfCalibration();

  // Calibration steps
  void RetrieveData(unsigned int state);
  void Analyze();

  std::map <pos::PixelROCName, map < std::pair <unsigned int, unsigned int>, map <unsigned int, PixelScanRecord> > > PH_vs_Vcal_;
  //                                            pixel column  pixel row           Vsf value

   struct branch{
    float pass;
    char rocName[38];
  };

   struct branch_sum{
    float delta_Vsf;
    float new_Vsf;
    char rocName[38];
  };

};

#endif
