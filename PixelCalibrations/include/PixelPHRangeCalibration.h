/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelPHRangeCalibration_h_
#define _PixelPHRangeCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"

class PixelPHRangeCalibration: public PixelCalibrationBase {
 public:

  // PixelPHRangeCalibration Constructor
  //PixelPHRangeCalibration();

  PixelPHRangeCalibration( const PixelSupervisorConfiguration &, SOAPCommander* );

  virtual ~PixelPHRangeCalibration(){};

  void beginCalibration();

  virtual bool execute();

  void endCalibration();

  virtual std::vector<std::string> calibrated();

};

#endif
 
