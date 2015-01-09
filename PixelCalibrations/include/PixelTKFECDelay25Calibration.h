/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelTKFECDelay25Calibration_h_
#define _PixelTKFECDelay25Calibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TMarker.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include <TDirectory.h>

#include "PixelCalibrations/include/PixelTKFECCalibrationBase.h"

class PixelTKFECDelay25Calibration: public PixelTKFECCalibrationBase {
 public:

  // PixelTKFECDelay25Calibration Constructor
  //PixelTKFECDelay25Calibration();

  PixelTKFECDelay25Calibration( const PixelTKFECSupervisorConfiguration &, SOAPCommander* );

  virtual ~PixelTKFECDelay25Calibration(){};

  virtual xoap::MessageReference execute(xoap::MessageReference);

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  bool SetDelay(pos::PixelPortCardConfig* tempPortCard, std::string delay, unsigned int value, bool update=false, bool write=false);

  void portcardI2CDevice(FecAccess* fecAccess, tscType8 fecAddress, tscType8 ringAddress, tscType8 ccuAddress, tscType8 channelAddress, tscType8 deviceAddress, enumDeviceType modeType, unsigned int value, int flag);

  bool NS(std::set< std::pair<int,int> > CandidatePoints, std::set< std::pair<int,int> > &GoodPoints, int i, int &countRegion);
  bool EW(std::set< std::pair<int,int> > CandidatePoints, std::set< std::pair<int,int> > &GoodPoints, int i, int &countRegion);
  bool NESW(std::set< std::pair<int,int> > CandidatePoints, std::set< std::pair<int,int> > &GoodPoints, int i, int &countRegion);
  bool NWSE(std::set< std::pair<int,int> > CandidatePoints, std::set< std::pair<int,int> > &GoodPoints, int i, int &countRegion);

  void FinalPointSelection(std::set< std::pair<int,int> > GoodPoints, int &newSDA, int &newRDA);
  void Intersection(std::set< std::pair<int,int> > CandidatePoints, std::set< std::pair<int,int> > &GlobalCandidatePoints);
  void WriteRootFile(int newSDA, int newRDA, bool compare);

 private:

  pos::PixelDelay25Calib* tempDelay25_;

  bool done_;

  int event_;
  int origSData_, origRData_;
  int gridSize_, gridSteps_, range_;
  int numTests_, commands_;
  bool allModules_, runCompare_;

  int countSData_, countModules_, countPortcards_;
  bool nextModule_, nextPortcard_;
  int totalModules_, totalPortcards_;

  std::vector<std::string> vectorOfPortcards_;
  std::string portcardName_;
  pos::PixelPortCardConfig* portcardConfig_;

  std::vector<pos::PixelModuleName> vectorOfModules_;
  pos::PixelModuleName moduleName_;
  std::string moduleString_;

  TFile* outputFile_;
  PixelRootDirectoryMaker* rootDirs_;

  std::multiset<int> nCandidatePoints_;
  std::set< std::pair<int,int> > CandidatePoints_;
  std::set< std::pair<int,int> > GlobalCandidatePoints_;
  std::map< std::pair<int,int>, int> finalData_;
  std::map< std::pair<int,int>, int> GlobalFinalData_;
  std::map<int, std::map<std::pair<int,int>, int> > Efficiency_;

};

#endif
