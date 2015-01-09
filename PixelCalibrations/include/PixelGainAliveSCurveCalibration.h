/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelGainAliveSCurveCalibration_h_
#define _PixelGainAliveSCurveCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include <string>

#include "PixelCalibrations/include/PixelCalibrationBase.h"
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"

class PixelGainAliveSCurveCalibration: public PixelCalibrationBase {
 public:

  // PixelGainAliveSCurveCalibration Constructor
  //PixelGainAliveSCurveCalibration();

  PixelGainAliveSCurveCalibration( const PixelSupervisorConfiguration &, SOAPCommander* );

  virtual ~PixelGainAliveSCurveCalibration(){};

  virtual void beginCalibration();

  virtual bool execute();

  virtual void endCalibration();

  virtual std::vector<std::string> calibrated();

 private:

  std::string mode_;
  unsigned int nConfigs_;
  unsigned int nTriggersTotal_;
  PixelTimer fecTimer_, fedTimer_;
  bool useLTC_;
 
};

#endif
