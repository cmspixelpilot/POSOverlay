/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFEDAOHAndFEDChannelMappingTest_h_
#define _PixelFEDAOHAndFEDChannelMappingTest_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"

#include "PixelUtilities/PixelFEDDataTools/include/PixelMode.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelScanRecord.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"

#include "TFile.h"
#include "TStyle.h"

class PixelFEDAOHAndFEDChannelMappingTest: public PixelFEDCalibrationBase {
 public:

  PixelFEDAOHAndFEDChannelMappingTest( const PixelFEDSupervisorConfiguration &, SOAPCommander* );
  virtual ~PixelFEDAOHAndFEDChannelMappingTest(){};

  virtual xoap::MessageReference execute(xoap::MessageReference msg);

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  virtual void initializeFED();

 private:

  void MeasureBlackLevel(unsigned int fednumber, unsigned int fedchannel, unsigned int AOHBias);
  xoap::MessageReference CheckForChangeInBlackLevel(unsigned int fednumber, unsigned int fedchannel);


  // PixelFEDAOHAndFEDChannelMappingTest Constructor should never be called
  //PixelFEDAOHAndFEDChannelMappingTest();

  // stored black levels as a function of AOHBias
  std::map <unsigned int, map <unsigned int, PixelScanRecord> > BVsAOHBias_;

	// For outputing plots.
	TStyle* plainStyle_;
	TFile* outputFile_;

};

#endif
