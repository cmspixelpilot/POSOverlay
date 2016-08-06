/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFEDClockPhaseCalibration_h_
#define _PixelFEDClockPhaseCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include "TFile.h"
#include "TLine.h"
#include "TProfile.h"

class TProfile;
//class pos::PixelCalibConfiguration;

class PixelFEDClockPhaseCalibration: public PixelFEDCalibrationBase {
 public:

  PixelFEDClockPhaseCalibration( const PixelFEDSupervisorConfiguration &, SOAPCommander* );
  virtual ~PixelFEDClockPhaseCalibration(){};

  virtual xoap::MessageReference execute(xoap::MessageReference msg);

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  virtual void initializeFED();

 private:

  int AutomaticModeDelay(int delayTBM, int delayROC, int warn);

  std::vector<TProfile*> draw(std::string title,
	    unsigned int fednumber,
	    unsigned int channel,
	    TProfile* phase0, 
	    TProfile* phase1, 
	    unsigned int slotstart, 
	    unsigned int nslot, 
	    int delay0, int delay1=-1);

  void drawDelay(unsigned int slotstart, unsigned int nslot, int delay, int color=1);

  void writeFEDCard(unsigned int fednumber);
  
  void updatePhaseAndDelay(unsigned int fednumber,
			   unsigned int channel,
			   unsigned int phase,
			   unsigned int delay);

  int phase0Shift_[4];
  int phase1Shift_[4];

  int phase0Phase_[4];
  int phase1Phase_[4];

  unsigned int nslot_;
  unsigned int startslot_;
  int offsetphase0_;
  int offsetphase1_;

  unsigned int nslotzoom_;
  unsigned int nzoom_;

  pos::PixelCalibConfiguration* tempCalibObject_;
  std::string oldMode_;

  std::map <unsigned int, std::map<unsigned int, TProfile*> > histogramPhase0_; 
  std::map <unsigned int, std::map<unsigned int, TProfile*> > histogramPhase1_; 
  std::map <unsigned int, std::map<unsigned int, TProfile*> > histogramPurgedPhase0_; 
  std::map <unsigned int, std::map<unsigned int, TProfile*> > histogramPurgedPhase1_; 
  std::map <unsigned int, std::map<unsigned int, TProfile*> > histogramOrderedPhase0_; 
  std::map <unsigned int, std::map<unsigned int, TProfile*> > histogramOrderedPhase1_; 
  std::map <unsigned int, std::map<unsigned int, TProfile*> > histogramFinalPhase0_; 
  std::map <unsigned int, std::map<unsigned int, TProfile*> > histogramFinalPhase1_; 
  //               FED Number             channel

  std::map <unsigned int, std::set<unsigned int> > fedsAndChannels_;
  TFile* outputFile_;
  PixelRootDirectoryMaker* dirMakerFED_;


};

#endif
