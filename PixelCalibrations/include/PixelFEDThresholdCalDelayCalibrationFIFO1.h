/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFEDThresholdCalDelayCalibrationFIFO1_h_
#define _PixelFEDThresholdCalDelayCalibrationFIFO1_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"

class PixelFEDThresholdCalDelayCalibrationFIFO1: public PixelFEDCalibrationBase {
 public:

  PixelFEDThresholdCalDelayCalibrationFIFO1( const PixelFEDSupervisorConfiguration &, SOAPCommander* );
  virtual ~PixelFEDThresholdCalDelayCalibrationFIFO1(){};

  virtual xoap::MessageReference execute(xoap::MessageReference msg);

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  virtual void initializeFED();

 private:

  // PixelFEDThresholdCalDelayCalibrationFIFO1 Constructor should never be called
  //PixelFEDThresholdCalDelayCalibrationFIFO1();


};

#endif
