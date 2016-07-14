/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _Pixel2DEfficiencyScan_h_
#define _Pixel2DEfficiencyScan_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"

class Pixel2DEfficiencyScan: public PixelCalibrationBase {
 public:

  Pixel2DEfficiencyScan( const PixelSupervisorConfiguration &, SOAPCommander* );
  virtual ~Pixel2DEfficiencyScan(){};

  virtual void beginCalibration();

  virtual bool execute();

  virtual void endCalibration();

  virtual std::vector<std::string> calibrated();

 private:

  PixelTimer fecTimer_, ttcTimer_, fedTimer_;


};

#endif
