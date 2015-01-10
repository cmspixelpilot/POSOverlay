/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2008 Cornell.			                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd	                				 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelRootDirectoryMaker_h_
#define _PixelRootDirectoryMaker_h_

#include <string>
#include <vector>
#include <map>
#include <set>

//#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"
#include "CalibFormats/SiPixelObjects/interface/PixelChannel.h"

class TDirectory;

class PixelRootDirectoryMaker {

 public:
 
  // This constructor takes a list of strings
  // As an example consider the input of
  //  FPix_BmI_D1_BLD1_PNL1_PLQ1_ROC0
  //  FPix_BmI_D1_BLD1_PNL0_PLQ1_ROC0
  //  BPix_BmI_SEC1_LYR1_LDR1H_MOD1_ROC0
  //  BPix_BmI_SEC1_LYR1_LDR1H_MOD1_ROC1
  // This would create the following directory structure
  //  FPix/BmI/D1/BLD1/PNL1/PLQ1/ROC0
  //                  /PNL0/PLQ1/ROC0
  //  BPix/BmI/SEC1/LYR1/LDR1H/MOD1/ROC0
  //                               /ROC1
  // The '_' will generate a new sub directory.

  //Make the directories from ROC names
  PixelRootDirectoryMaker(std::vector<pos::PixelROCName> names);
  PixelRootDirectoryMaker(std::vector<pos::PixelROCName> names, TDirectory* directory);

  //Make the directories from a list of names
  PixelRootDirectoryMaker(std::vector<std::string> names);
  PixelRootDirectoryMaker(std::vector<std::string> names,TDirectory* directory);

  //Make the directories from FED number and FED channels
  PixelRootDirectoryMaker(std::vector<std::pair<unsigned int,std::vector<unsigned int> > >);
  PixelRootDirectoryMaker(std::vector<std::pair<unsigned int,std::vector<unsigned int> > > fedsAndChannels, TDirectory* directory);

  PixelRootDirectoryMaker(std::set<pos::PixelChannel> names,    TDirectory* directory);
 


  //Returns the directory corresponding to the string
  TDirectory* getDirectory(std::string directory);
 
  void cdDirectory(const std::string name,          						 TDirectory *dir=0);
  void cdDirectory(const pos::PixelROCName& aROC, 							 TDirectory *dir=0);
  void cdDirectory(const pos::PixelChannel& aCha, 							 TDirectory *dir=0);
  void cdDirectory(unsigned int fedNumber, unsigned int channel, TDirectory *dir=0);

 private:
  //This method replaces the '_' with '/' to make the
  //directory structure
   std::string getPath(std::string name);
    
  //This method returns the number of levels in the path
  // e.g. FPix/BmO/D1 returns 3. 
  unsigned int dirLevels(std::string path);

  //This method returns the sub path for the first N levels
  //N=1, 2, 3, etd
  // e.g. pathN(2,"FPix/BmO/D1") returns "FPix/BmO"
  std::string pathN(unsigned int N, std::string path);

  //Returns the Nth path.
  //N=1, 2, 3, etd
  // e.g. pathNth(2,"FPix/BmO/D1") returns "BmO"
  std::string pathNth(unsigned int N, std::string path);

  void init (std::vector<std::string> names);

  std::map<std::string,TDirectory *> directorymap_;

  // base directory in which the directory structure is constructed
  TDirectory* directory_;
};

#endif
