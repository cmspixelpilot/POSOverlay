/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFEDGainAliveSCurveCalibration_h_
#define _PixelFEDGainAliveSCurveCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"

class PixelFEDGainAliveSCurveCalibration: public PixelFEDCalibrationBase {
 public:

  PixelFEDGainAliveSCurveCalibration(const PixelFEDSupervisorConfiguration &, 
				     SOAPCommander*);

  virtual ~PixelFEDGainAliveSCurveCalibration(){};

  virtual xoap::MessageReference execute(xoap::MessageReference msg);

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  virtual void initializeFED();

};

#endif
