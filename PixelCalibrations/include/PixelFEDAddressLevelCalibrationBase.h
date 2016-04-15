/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFEDAddressLevelCalibrationBase_h_
#define _PixelFEDAddressLevelCalibrationBase_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/AddressLevels.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"
#include "TFile.h"
#include "TLine.h"
#include "TCanvas.h"
//class pos::PixelCalibConfiguration;
class PixelRootDirectoryMaker;
class TH1F;
class TH2F;

class PixelFEDAddressLevelCalibrationBase: public PixelFEDCalibrationBase {
 public:

  PixelFEDAddressLevelCalibrationBase( const PixelFEDSupervisorConfiguration &, SOAPCommander* );

  virtual ~PixelFEDAddressLevelCalibrationBase();

  void initAddressLevelCalibration();
  
  void summary();

  void analyze(bool noHits);

 protected:

  
  std::vector<std::pair<unsigned int,std::vector<unsigned int> > > fedsAndChannels_;
  std::vector<pos::PixelROCName> aROC_string_;

  // map<FED number, channels,..>
  std::map <unsigned long, map <unsigned int, Moments> > UB, B, UB_TBM, UB_ROC, baselineCorrection;
  std::map <unsigned long, map <unsigned int, float> > recommended_UB, recommended_BH, recommended_BL;
  std::map <unsigned long, map <unsigned int, AddressLevels> > TBM_AddressLevels;
  // 		VME Base Address,    Channel

  std::map <pos::PixelROCName, TH1F*> ROC_AddressLevelsHist;
  std::map <pos::PixelROCName, AddressLevels> ROC_AddressLevels;
  //		ROC String Name

  std::map <unsigned int, std::map <unsigned int, TH1F*> > blackLevel_;
  std::map <unsigned int, std::map <unsigned int, TH2F*> > scopePlotsMap_;
  std::map <unsigned int, std::map <unsigned int, TH1F*> > rmsSummary_;
  std::map <unsigned int, std::map <unsigned int, AddressLevels> > ROC_ChannelSum_AddressLevels_;
  std::map <unsigned int, std::map <unsigned int, TH1F*> > tbmLevelsHist_;
  //                 FED ID                FED Channel

  std::map<unsigned int, TH2F*> RMSvsChannel_; 
  TH2F* RMSvsFED;
  
  std::map<unsigned int, TH2F*> separationvsChannel_; //for each FED show the separation between Levels
  TH2F* SeparationvsFED;

  std::map<unsigned int, std::map<unsigned int, TH2F*> > levelsPlot_;
  std::map<unsigned int, std::map<unsigned int, TCanvas*> > levelsCanvas_;
  
  std::map <unsigned int, AddressLevels> ROC_FEDSum_AddressLevels;
  //                 FED ID

  PixelRootDirectoryMaker* dirMakerPIX_;
  PixelRootDirectoryMaker* dirMakerFED_;

  int eventNumber;

  TFile* outputFile_;

   struct addressLevelBranch{ 
    float pass; 
    float nPeaks;
    float maxrms;
    float minseparation;
    float blackrms;
    char  rocName[38];
  };  
  
};

#endif
