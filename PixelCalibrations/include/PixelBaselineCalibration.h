/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelBaselineCalibration_h_
#define _PixelBaselineCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"
#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"

class PixelBaselineCalibration: public PixelCalibrationBase {
 public:

  // PixelBaselineCalibration Constructor
  //PixelBaselineCalibration();

  PixelBaselineCalibration( const PixelSupervisorConfiguration &, SOAPCommander* );

  virtual ~PixelBaselineCalibration(){};

  virtual bool execute();

  virtual std::vector<std::string> calibrated();

  void sendToFED(std::string& action, bool& continueIterating);

};

#endif
