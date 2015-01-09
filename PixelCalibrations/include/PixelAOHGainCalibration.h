/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelAOHGainCalibration_h_
#define _PixelAOHGainCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"

class PixelAOHGainCalibration: public PixelCalibrationBase {
 public:

  // PixelAOHGainCalibration Constructor
  //PixelAOHGainCalibration();

  PixelAOHGainCalibration( const PixelSupervisorConfiguration &, SOAPCommander* );

  virtual ~PixelAOHGainCalibration(){};

  void beginCalibration();

  virtual bool execute();

  void endCalibration();

  virtual std::vector<std::string> calibrated();

  private:
  enum {kAnalogInputBias = 0, kAnalogOutputBias = 1, kAnalogOutputGain = 2};
  std::string DACName(unsigned int index) const; // looks up name in the map DACNames_
  std::map<unsigned int, std::string> DACNames_; // filled by constructor

};

#endif
 
