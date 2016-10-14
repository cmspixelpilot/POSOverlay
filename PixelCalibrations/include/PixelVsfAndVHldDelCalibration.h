/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelVsfAndVHldDelCalibration_h_
#define _PixelVsfAndVHldDelCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"

class PixelVsfAndVHldDelCalibration: public PixelCalibrationBase {
 public:

  // PixelVsfAndVHldDelCalibration Constructor
  //PixelVsfAndVHldDelCalibration();

  PixelVsfAndVHldDelCalibration( const PixelSupervisorConfiguration &, SOAPCommander* );

  virtual ~PixelVsfAndVHldDelCalibration(){};

  void beginCalibration();

  virtual bool execute();

  void endCalibration();

  virtual std::vector<std::string> calibrated();

};

#endif
 
