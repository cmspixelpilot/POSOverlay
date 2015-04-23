/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelClockPhaseCalibration_h_
#define _PixelClockPhaseCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"

//class pos::PixelCalibConfiguration;

class PixelClockPhaseCalibration: public PixelCalibrationBase {
 public:

  // PixelClockPhaseCalibration Constructor
  //PixelClockPhaseCalibration();

  PixelClockPhaseCalibration( const PixelSupervisorConfiguration &, SOAPCommander* );

  virtual ~PixelClockPhaseCalibration(){};


  virtual void beginCalibration();

  virtual bool execute();

  virtual void endCalibration();

  virtual std::vector<std::string> calibrated();

 private:
  
  pos::PixelCalibConfiguration* tempCalibObject_;
  std::map <unsigned int, std::set<unsigned int> > fedsAndChannels_;

};

#endif
