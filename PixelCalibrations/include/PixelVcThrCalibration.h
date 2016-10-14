/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelVcThrCalibration_h_
#define _PixelVcThrCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"

class PixelVcThrCalibration: public PixelCalibrationBase {
 public:

  PixelVcThrCalibration(const PixelSupervisorConfiguration &, 
			 SOAPCommander* );

  virtual ~PixelVcThrCalibration(){};

  virtual bool execute();

  virtual std::vector<std::string> calibrated();

};

#endif
