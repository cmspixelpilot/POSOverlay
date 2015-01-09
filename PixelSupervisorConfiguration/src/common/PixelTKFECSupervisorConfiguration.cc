// $Id: PixelTKFECSupervisorConfiguration.cc,v 1.4 2008/09/05 20:47:15 yw266 Exp $: PixelSupervisorConfiguration.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelSupervisorConfiguration/include/PixelTKFECSupervisorConfiguration.h"

using namespace pos;
using namespace std;

PixelTKFECSupervisorConfiguration::PixelTKFECSupervisorConfiguration(std::string* runNumber, 
								     std::string* outputDir):PixelSupervisorConfigurationBase(runNumber, outputDir)
{
  theGlobalKey_=0;
  theCalibObject_=0;
  thePortcardMap_=0;
  theTKFECConfiguration_=0;
  theNameTranslation_=0;
  theFECConfiguration_=0;
  diagService_=0;
  fecAccess_=0;
  //  runNumber_=runNumber;
  //  outputDir_=outputDir;
  theGlobalDelay25_=0;
}

PixelTKFECSupervisorConfiguration::PixelTKFECSupervisorConfiguration(const PixelTKFECSupervisorConfiguration & tempConfiguration):PixelSupervisorConfigurationBase(tempConfiguration)
{
  
  //just shallow copying for the moment
  //pointers
  theGlobalKey_ = tempConfiguration.theGlobalKey_;
  theTKFECConfiguration_ = tempConfiguration.theTKFECConfiguration_;
  theCalibObject_ = tempConfiguration.theCalibObject_;
  mapNamePortCard_ = tempConfiguration.mapNamePortCard_;
  thePortcardMap_ = tempConfiguration.thePortcardMap_;
  theNameTranslation_ = tempConfiguration.theNameTranslation_;
  theFECConfiguration_ = tempConfiguration.theFECConfiguration_;
  PixelFECSupervisors_ = tempConfiguration.PixelFECSupervisors_;
  theGlobalDelay25_ = tempConfiguration.theGlobalDelay25_;

  diagService_ = tempConfiguration.diagService_;

  fecAccess_ = tempConfiguration.fecAccess_;

  //  runNumber_=tempConfiguration.runNumber_;
  //  outputDir_=tempConfiguration.outputDir_;

  crate_=tempConfiguration.crate_;
}

// void PixelTKFECSupervisorConfiguration::setupOutputDir(){

//   string basedir=getenv("POS_OUTPUT_DIRS");

//   struct stat stbuf;
//   if (stat(basedir.c_str(),&stbuf)!=0){
//     cout << "[PixelTKFECSupervisorConfiguration::setupOutputDir] basedir="
// 	 << basedir.c_str() << " does not exist."<<endl;
//     assert(0);
//   }

//   *outputDir_=basedir+"/Run_"+*runNumber_;

  
//   if (stat(outputDir_->c_str(),&stbuf)!=0){
//     cout << "[PixelTKFECSupervisorConfiguration::setupOutputDir] outputDir_="
// 	 << outputDir_->c_str() << " does not exist. Will create"<<endl;
//     mkdir(outputDir_->c_str(),0777);
//   }

// }    

// std::string PixelTKFECSupervisorConfiguration::runDir(){
  
//   std::string runDir="Run_"+*runNumber_;

//   return runDir;

// }


// std::string PixelTKFECSupervisorConfiguration::outputDir(){

//   //cout << "[PixelTKFECSupervisorConfiguration::outputDir] outputDir_="
//   //     << *outputDir_ << endl;

//   return *outputDir_;

// }
