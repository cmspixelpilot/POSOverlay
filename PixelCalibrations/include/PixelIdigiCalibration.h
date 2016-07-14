/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2007, Cornell.			                         *
 * All rights reserved.                                                  *
 * Authors: A. Ryd              					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelIdigiCalibration_h_
#define _PixelIdigiBCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"
#include "CalibFormats/SiPixelObjects/interface/PixelModuleName.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"

//class pos::PixelLowVoltageMap;
//class pos::PixelDACSettings;

class PixelIdigiCalibration: public PixelCalibrationBase {
 public:

  PixelIdigiCalibration( const PixelSupervisorConfiguration &, SOAPCommander* );

  virtual ~PixelIdigiCalibration(){};

  virtual bool execute();

  virtual void beginCalibration();

  virtual void endCalibration();

  virtual std::vector<std::string> calibrated();

 private:
  
  std::map<std::string, std::vector<pos::PixelROCName> > dpMap_;
  //        dpName                   
  std::map<std::string, std::vector<std::vector<Moments> > > Idigi_;
  //        dpName                   
  unsigned int maxROC_;

  pos::PixelLowVoltageMap* lowVoltageMap_;
  pos::PixelMaxVsf* maxVsf_;

  std::map<pos::PixelModuleName,pos::PixelDACSettings*> dacsettings_;

};

#endif
 
