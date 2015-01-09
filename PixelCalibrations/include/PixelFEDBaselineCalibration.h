/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFEDBaselineCalibration_h_
#define _PixelFEDBaselineCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include "TFile.h"

class PixelFEDBaselineCalibration: public PixelFEDCalibrationBase {
 public:

  PixelFEDBaselineCalibration( const PixelFEDSupervisorConfiguration &, SOAPCommander* );
  virtual ~PixelFEDBaselineCalibration(){};

  virtual xoap::MessageReference execute(xoap::MessageReference msg);

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  virtual void initializeFED();

 private:

  unsigned int iteration_;
  std::vector<std::pair<unsigned int,std::vector<unsigned int> > > fedsAndChannels_;
  std::map <unsigned int, std::map<unsigned int, std::vector<std::stringstream*> > > summary_long_;
  std::map <unsigned int, std::map<unsigned int, std::vector<unsigned int> > > summary_short_;
  //               fednumber              channel         iteration  summary

  TFile* outputFile_;
  PixelRootDirectoryMaker* dirMakerFED_;



};

#endif
