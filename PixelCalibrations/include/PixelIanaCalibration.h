// $Id: PixelIanaCalibration.h,v 1.10 2009/05/27 19:20:32 joshmt Exp $
/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2009, Cornell.			                         *
 * All rights reserved.                                                  *
 * Authors: A. Ryd              					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelIanaCalibration_h_
#define _PixelIanaBCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"
#include "CalibFormats/SiPixelObjects/interface/PixelModuleName.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"

//class pos::PixelLowVoltageMap;
//class pos::PixelDACSettings;

class PixelIanaCalibration: public PixelCalibrationBase {
 public:

  PixelIanaCalibration( const PixelSupervisorConfiguration &, SOAPCommander* );

  virtual ~PixelIanaCalibration(){};

  virtual bool execute();

  virtual void beginCalibration();

  virtual void endCalibration();

  virtual std::vector<std::string> calibrated();

 private:
  
  void testTiming();

  std::map<std::string, std::vector<pos::PixelROCName> > dpMap_;
  //        dpName                   
  std::map<std::string, std::vector<std::vector<Moments> > > Iana_;

  std::map<std::string, std::vector<std::vector<Moments> > > Iana_OSD_;
  //        dpName                   
  unsigned int maxROC_;

  pos::PixelLowVoltageMap* lowVoltageMap_;

  std::map<pos::PixelModuleName,pos::PixelDACSettings*> dacsettings_;

  unsigned int npoints_;
  unsigned int sleeptime_;
  unsigned int sleeptime0_;
  double ianares_;
  bool MixVanas_;
  bool TurnOffVsf_;
  bool ReadCurrent_;
  bool ManualReads_;
  bool UseOSD_;

   struct branch{
    float pass;
    char rocName[38];
  };

  struct branch_sum{
    float deltaVana;
    float newVana;
    float newIana;
    float maxIana;
    float fitChisquare;
    char rocName[38];
  };
};

#endif
 
