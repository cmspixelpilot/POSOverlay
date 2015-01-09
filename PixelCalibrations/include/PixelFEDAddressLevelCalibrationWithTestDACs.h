/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFEDAddressLevelCalibrationWithTestDACs_h_
#define _PixelFEDAddressLevelCalibrationWithTestDACs_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelFEDAddressLevelCalibrationBase.h"

class PixelFEDAddressLevelCalibrationWithTestDACs: public PixelFEDAddressLevelCalibrationBase {
 public:

  PixelFEDAddressLevelCalibrationWithTestDACs( const PixelFEDSupervisorConfiguration &, SOAPCommander* );
  virtual ~PixelFEDAddressLevelCalibrationWithTestDACs(){};

  virtual xoap::MessageReference execute(xoap::MessageReference msg);

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  virtual void initializeFED();

 private:

  std::string BUILD_HOME;

  // PixelFEDAddressLevelCalibrationWithTestDACs Constructor should never be called
  //PixelFEDAddressLevelCalibrationWithTestDACs();

};

#endif
