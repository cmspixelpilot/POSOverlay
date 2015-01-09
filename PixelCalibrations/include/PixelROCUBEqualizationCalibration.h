/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelROCUBEqualizationCalibration_h_
#define _PixelROCUBEqualizationCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"

class PixelROCUBEqualizationCalibration: public PixelCalibrationBase {
 public:

  // PixelROCUBEqualizationCalibration Constructor
  //PixelROCUBEqualizationCalibration();

  PixelROCUBEqualizationCalibration( const PixelSupervisorConfiguration &, SOAPCommander* );

  virtual ~PixelROCUBEqualizationCalibration(){};

  void beginCalibration();

  virtual bool execute();

  void endCalibration();

  virtual std::vector<std::string> calibrated();

};

#endif
 
