/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelAddressLevelCalibration_h_
#define _PixelAddressLevelCalibration_h_


#include "PixelCalibrations/include/PixelCalibrationBase.h"
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

//class pos::PixelCalibConfiguration;

class PixelAddressLevelCalibration: public PixelCalibrationBase {
 public:

  PixelAddressLevelCalibration( const PixelSupervisorConfiguration &, SOAPCommander* );

  virtual ~PixelAddressLevelCalibration(){};

  virtual void beginCalibration();

  virtual bool execute();

  virtual void endCalibration();

  virtual std::vector<std::string> calibrated();

 private:
  
  pos::PixelCalibConfiguration* tempCalibObject;

  PixelTimer fecTimer, ttcTimer, fedTimer, totalTimer;


};

#endif
