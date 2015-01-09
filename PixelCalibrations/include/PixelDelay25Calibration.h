/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelDelay25Calibration_h_
#define _PixelDelay25Calibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"

class PixelDelay25Calibration: public PixelCalibrationBase {
 public:

  // PixelDelay25Calibration Constructor
  //PixelDelay25Calibration();

  PixelDelay25Calibration( const PixelSupervisorConfiguration &, SOAPCommander* );

  virtual ~PixelDelay25Calibration(){};

  virtual void beginCalibration();

  virtual bool execute();

  virtual void endCalibration();

  virtual std::vector<std::string> calibrated();
  
  void sendToFED(std::string& action, Attribute_Vector parametersToFED = Attribute_Vector(0)){};
  void sendToFEC(std::string& action, Attribute_Vector parameters = Attribute_Vector(0)){};

 private:


};

#endif
