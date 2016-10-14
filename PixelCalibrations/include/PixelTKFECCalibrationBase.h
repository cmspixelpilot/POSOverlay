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

// temporary DiagSystem wrapper
#include "PixelCalibrations/include/DiagWrapper.h"

class PixelTKFECCalibrationBase : public PixelTKFECSupervisorConfiguration, 
	    public SOAPCommander {
 public:

  PixelTKFECCalibrationBase( const PixelTKFECSupervisorConfiguration &,
			   const SOAPCommander& soapCommander );

  virtual ~PixelTKFECCalibrationBase(){};

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference execute(xoap::MessageReference) = 0;

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);
  
  inline std::string stringF(int number) { std::stringstream ss; ss << number; return ss.str(); };
  inline std::string stringF(const char* text) { std::stringstream ss; ss << text; return ss.str(); };

  DiagWrapper* diagService_;
  static const int DIAGDEBUG = 0;
  static const int DIAGTRACE = 1;
  static const int DIAGUSERINFO = 2;
  static const int DIAGINFO = 3;
  static const int DIAGWARN = 4;
  static const int DIAGERROR = 5;
  static const int DIAGFATAL = 6;

 private:

  // PixelCalibrationBase Constructor should neve be called
  PixelTKFECCalibrationBase(const SOAPCommander& soapCommander);
  PixelTKFECCalibrationBase();

};

#endif
