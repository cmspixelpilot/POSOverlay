/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFEDAOHGainCalibration_h_
#define _PixelFEDAOHGainCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"

#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "CalibFormats/SiPixelObjects/interface/PixelChannel.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelScanRecord.h"

#include <map>

class PixelFEDAOHGainCalibration: public PixelFEDCalibrationBase {
 public:

  PixelFEDAOHGainCalibration( const PixelFEDSupervisorConfiguration &, SOAPCommander* );
  virtual ~PixelFEDAOHGainCalibration(){};

  virtual xoap::MessageReference execute(xoap::MessageReference msg);

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  virtual void initializeFED();

 private:

  // PixelFEDAOHGainCalibration Constructor should never be called
  //PixelFEDAOHGainCalibration();

  // variable to store TBM UB scan for each channel.
  std::map <pos::PixelChannel, PixelScanRecord> TBM_UB_;
  
  // variable to store FED automatic baseline correction as a function of AOH gain for each channel
  std::map <pos::PixelChannel, PixelScanRecord> baselineCorrection_;

  // variables to store info from analysis until info is returned to PixelSupervisor
  std::map <pos::PixelChannel, unsigned int> recommended_AOHGain_values_;
  std::map <pos::PixelChannel, std::string> failures_;

};

#endif
