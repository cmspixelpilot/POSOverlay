/*************************************************************************
 * XDAQ Components for Pixel Online Software                             *
 * Copyright (C) 2007, Cornell University		                 *
 * All rights reserved.                                                  *
 * Authors: Souvik Das          					 *
  *************************************************************************/

#ifndef _PixelGainAliveSCurveCalibrationWithSLink_h_
#define _PixelGainAliveSCurveCalibrationWithSLink_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"

class PixelGainAliveSCurveCalibrationWithSLink: public PixelCalibrationBase {
 public:

  PixelGainAliveSCurveCalibrationWithSLink( const PixelSupervisorConfiguration &, SOAPCommander* );

  virtual ~PixelGainAliveSCurveCalibrationWithSLink(){};

  virtual void beginCalibration();

  virtual bool execute();

  virtual void endCalibration();

  virtual std::vector<std::string> calibrated();

 private:

  std::string mode_;
  unsigned int nConfigs_;
  unsigned int nTriggersTotal_;
  unsigned int nTriggersPerPattern_;
  int missingTriggers_;
  PixelTimer fecTimer_, ttcTimer_;
  bool useLTC_;
  

};

#endif
