/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFEDCalibrationBase_h_
#define _PixelFEDCalibrationBase_h_

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"

#include "PixelSupervisorConfiguration/include/PixelFEDSupervisorConfiguration.h"


class PixelFEDCalibrationBase : public PixelFEDSupervisorConfiguration, 
	    public SOAPCommander {
 public:

  PixelFEDCalibrationBase( const PixelFEDSupervisorConfiguration &,
			   const SOAPCommander& soapCommander );

  virtual ~PixelFEDCalibrationBase(){};

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference execute(xoap::MessageReference msg) = 0;

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  unsigned int TransparentDataStart (uint32_t *buffer, int fed=-1, int channel=-1);

  virtual void initializeFED()=0;


 protected:

  void setFEDModeAndControlRegister(unsigned int mode, unsigned int control);

  void baselinecorr_off();

  void setSpecialDac(unsigned int mode);

  void fillTestDAC(xoap::MessageReference fillTestDACmsg);

  void setBlackUBTrans();

  //Event counter, incremented in runEvent after call to execute
  unsigned int event_;

 private:

  // PixelCalibrationBase Constructor should neve be called
  PixelFEDCalibrationBase(const SOAPCommander& soapCommander);
  PixelFEDCalibrationBase();
 
  
};

#endif
