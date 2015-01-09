/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelThresholdCalDelayCalibrationFIFO1_h_
#define _PixelThresholdCalDelayCalibrationFIFO1_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"

class PixelThresholdCalDelayCalibrationFIFO1: public PixelCalibrationBase {
 public:

  // PixelThresholdCalDelayCalibrationFIFO1 Constructor
  //PixelThresholdCalDelayCalibrationFIFO1();

  PixelThresholdCalDelayCalibrationFIFO1( const PixelSupervisorConfiguration &, SOAPCommander* );

  virtual ~PixelThresholdCalDelayCalibrationFIFO1(){};

  virtual bool execute();

  virtual std::vector<std::string> calibrated();

};

#endif
