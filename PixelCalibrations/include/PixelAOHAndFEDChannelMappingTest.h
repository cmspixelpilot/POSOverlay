/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelAOHAndFEDChannelMappingTest_h_
#define _PixelAOHAndFEDChannelMappingTest_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"

#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"

class PixelAOHAndFEDChannelMappingTest: public PixelCalibrationBase {
 public:

  // PixelAOHAndFEDChannelMappingTest Constructor
  //PixelAOHAndFEDChannelMappingTest();

  PixelAOHAndFEDChannelMappingTest( const PixelSupervisorConfiguration &, SOAPCommander* );

  virtual ~PixelAOHAndFEDChannelMappingTest(){};

  virtual bool execute();

  virtual std::vector<std::string> calibrated();
  
  private:
  void SetAOHBiasOneChannel(std::string portCardName, unsigned int AOHNumber, unsigned int AOHBiasValue);
  void triggeringLoop(unsigned int fedcrate, unsigned int fednumber, unsigned int fedchannel, unsigned int AOHBias, unsigned int Ntriggers);
};

#endif
 
