/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFED2DEfficiencyScan_h_
#define _PixelFED2DEfficiencyScan_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"
#include "PixelCalibrations/include/PixelEfficiency2D.h"
#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"
#include "TFile.h"

class PixelFED2DEfficiencyScan: public PixelFEDCalibrationBase {
 public:

  PixelFED2DEfficiencyScan( const PixelFEDSupervisorConfiguration &, SOAPCommander* );
  virtual ~PixelFED2DEfficiencyScan(){};

  virtual xoap::MessageReference execute(xoap::MessageReference msg);

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  virtual void initializeFED();

 private:

  pos::PixelCalibConfiguration* tempCalibObject_;
  std::map <unsigned int, std::set<unsigned int> > fedsAndChannels_;
  std::map <pos::PixelROCName, PixelEfficiency2D> eff_;
  std::string name1_;
  std::string name2_;
};

#endif
