/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelAOHBiasCalibration_h_
#define _PixelAOHBiasCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"

#include "CalibFormats/SiPixelObjects/interface/PixelChannel.h"

#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"

class PixelAOHBiasCalibration: public PixelCalibrationBase {
 public:

  // PixelAOHBiasCalibration Constructor
  //PixelAOHBiasCalibration();

  PixelAOHBiasCalibration( const PixelSupervisorConfiguration &, SOAPCommander* );

  virtual ~PixelAOHBiasCalibration(){};

  void beginCalibration();

  virtual bool execute();
  
  void endCalibration();

  virtual std::vector<std::string> calibrated();
  
  private:
  void SetAllFEDOffsets(std::string FEDReceiverInputOffset, std::string FEDChannelOffset);
  void AOHBiasLoop(Attribute_Vector parametersToFED);
  void triggeringLoop(Attribute_Vector parametersToFED, std::set<unsigned int> fedcrates, unsigned int Ntriggers);
  void SetAOHBiasToCurrentValue(std::string portCardName, int AOHNumber);
  Attribute_Vector SetupParametersToFED() const;

	// Records which channels we changed from 1Vpp to 2Vpp, so that we can change them back at the end.
	std::set< std::pair<unsigned int, unsigned int> > FEDChannelsWith1Vpp_;
	//                  FED #         FED channel

	//        port card name         AOH number
	std::map< std::string, std::map< unsigned int, unsigned int > > AOHBiasSaturationPoints_;
	std::map< std::string, std::map< unsigned int, unsigned int > > currentAOHBiasValues_;
	
	// For summary
	Moments finalBlackLevels_;
	std::map< pos::PixelChannel, std::string > failures_;
	std::map< pos::PixelChannel, std::string > warnings_;
	//                     failure mode string
	Moments AOHBiasSaturationPoints_Moments_;
	Moments finalAOHBiasValues_Moments_;
	Moments finalMinusSaturationAOHBiasValues_Moments_;
	
	// to keep track of step over many execute calls
	unsigned int currentStep_;
	enum { kBeginning = 0, kFindTimeSlots, kMeasureBUBSeparation, kFindSaturationPoints, kAdjustFEDOffsets, kAdjustBlackWithAOHBias, kDone }; // order is very important!
	std::string stepDescription(unsigned int step); // string describing each step
	
	// functions for each step
	void FindTimeSlots();
	void MeasureBUBSeparation();
	void FindSaturationPoints();
	void AdjustFEDOffsets();
	void AdjustBlackWithAOHBias();

};

#endif
 
