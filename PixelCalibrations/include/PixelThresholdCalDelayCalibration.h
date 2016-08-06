/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelThresholdCalDelayCalibration_h_
#define _PixelThresholdCalDelayCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"

class PixelThresholdCalDelayCalibration: public PixelCalibrationBase {
 public:

  PixelThresholdCalDelayCalibration( const PixelSupervisorConfiguration &, SOAPCommander* );
  virtual ~PixelThresholdCalDelayCalibration(){};

  virtual void beginCalibration();

  virtual bool execute();

  virtual void endCalibration();

  virtual std::vector<std::string> calibrated();

 private:

  PixelTimer fecTimer_, ttcTimer_, fedTimer_;


};

#endif
