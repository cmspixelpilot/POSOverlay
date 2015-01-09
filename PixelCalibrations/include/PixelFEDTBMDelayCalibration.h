/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFEDTBMDelayCalibration_h_
#define _PixelFEDTBMDelayCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"

#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelScanRecord.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"

#include <map>

class TGraph2DErrors;

class PixelFEDTBMDelayCalibration: public PixelFEDCalibrationBase {
 public:

  PixelFEDTBMDelayCalibration( const PixelFEDSupervisorConfiguration &, SOAPCommander* );
  virtual ~PixelFEDTBMDelayCalibration(){};

  virtual xoap::MessageReference execute(xoap::MessageReference msg);

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  virtual void initializeFED();

 private:

  // PixelFEDTBMDelayCalibration Constructor should never be called
  //PixelFEDTBMDelayCalibration();

  // Calibration steps
  void RetrieveData(unsigned int state);
  void Analyze();

  typedef std::map<unsigned, unsigned> pixel;
  typedef std::map<std::string,unsigned int> dacsettings;
  std::map<pos::PixelROCName, std::map<dacsettings, std::map<pixel, int> > > hit_counts;
};

#endif
