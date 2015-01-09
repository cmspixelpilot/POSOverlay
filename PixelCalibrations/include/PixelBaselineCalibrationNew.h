//--*-C++-*--
// $Id: PixelBaselineCalibrationNew.h,v 1.1 2009/02/19 11:00:00 joshmt Exp $
/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2009, Cornell University		                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd, S. Das, J. Thompson					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelBaselineCalibrationNew_h_
#define _PixelBaselineCalibrationNew_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"
#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"

class PixelBaselineCalibrationNew: public PixelCalibrationBase {
 public:

  // PixelBaselineCalibrationNew Constructor
  //PixelBaselineCalibrationNew();

  PixelBaselineCalibrationNew( const PixelSupervisorConfiguration &, SOAPCommander* );

  virtual ~PixelBaselineCalibrationNew(){};

  virtual void beginCalibration();
  virtual void endCalibration();

  virtual bool execute();

  virtual std::vector<std::string> calibrated();

  void sendToFED(std::string& action, bool& continueIterating);

 private:
  unsigned int iteration_;
  unsigned int MaxIterations_;

  float tolerance_;

};

#endif
