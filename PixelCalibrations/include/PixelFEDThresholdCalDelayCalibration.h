/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFEDThresholdCalDelayCalibration_h_
#define _PixelFEDThresholdCalDelayCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"
#include "PixelCalibrations/include/PixelEfficiency2DVcThrCalDel.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "TFile.h"

class PixelFEDThresholdCalDelayCalibration: public PixelFEDCalibrationBase {
 public:

  PixelFEDThresholdCalDelayCalibration( const PixelFEDSupervisorConfiguration &, SOAPCommander* );
  virtual ~PixelFEDThresholdCalDelayCalibration(){};

  virtual xoap::MessageReference execute(xoap::MessageReference msg);

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  virtual void initializeFED();

 private:

  pos::PixelCalibConfiguration* tempCalibObject_;
  map <unsigned int, std::set<unsigned int> > fedsAndChannels_;
  map <pos::PixelROCName, PixelEfficiency2DVcThrCalDel> eff_;
  TFile* outputFile_;
  string name1_;
  string name2_;

   struct branch{
    float pass;
    char rocName[38];
  };

   struct branch_sum{
    float new_CalDel;
    float delta_CalDel;
    float new_VcThr;
    float delta_VcThr;
    char rocName[38];
  };

};

#endif
