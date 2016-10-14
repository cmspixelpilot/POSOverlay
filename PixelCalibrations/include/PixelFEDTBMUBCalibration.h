/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFEDTBMUBCalibration_h_
#define _PixelFEDTBMUBCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"

#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "CalibFormats/SiPixelObjects/interface/PixelChannel.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelScanRecord.h"

#include <map>

class PixelFEDTBMUBCalibration: public PixelFEDCalibrationBase {
 public:

  PixelFEDTBMUBCalibration( const PixelFEDSupervisorConfiguration &, SOAPCommander* );
  virtual ~PixelFEDTBMUBCalibration(){};

  virtual xoap::MessageReference execute(xoap::MessageReference msg);

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  virtual void initializeFED();

 private:

  // PixelFEDTBMUBCalibration Constructor should never be called
  //PixelFEDTBMUBCalibration();

  // variable to store TBM UB scan for each channel.
  std::map <pos::PixelChannel, PixelScanRecord> TBM_UB_;

};

#endif
