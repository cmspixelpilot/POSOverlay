/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelTKFECCalibrationBase_h_
#define _PixelTKFECCalibrationBase_h_

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"

#include "PixelSupervisorConfiguration/include/PixelTKFECSupervisorConfiguration.h"

class PixelTKFECCalibrationBase : public PixelTKFECSupervisorConfiguration, 
	    public SOAPCommander {
 public:

  PixelTKFECCalibrationBase( const PixelTKFECSupervisorConfiguration &,
			   const SOAPCommander& soapCommander );

  virtual ~PixelTKFECCalibrationBase(){};

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference execute(xoap::MessageReference) = 0;

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

 private:

  // PixelCalibrationBase Constructor should neve be called
  PixelTKFECCalibrationBase(const SOAPCommander& soapCommander);
  PixelTKFECCalibrationBase();

};

#endif
