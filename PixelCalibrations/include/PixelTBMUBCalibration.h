/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelTBMUBCalibration_h_
#define _PixelTBMUBCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"

class PixelTBMUBCalibration: public PixelCalibrationBase {
 public:

  // PixelTBMUBCalibration Constructor
  //PixelTBMUBCalibration();

  PixelTBMUBCalibration( const PixelSupervisorConfiguration &, SOAPCommander* );

  virtual ~PixelTBMUBCalibration(){};

  void beginCalibration();

  virtual bool execute();

  void endCalibration();

  virtual std::vector<std::string> calibrated();

  private:

};

#endif
 
