/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2007, Cornell		        	                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd               					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/
#ifndef _PixelEfficiency2DVcThrCalDel_h_
#define _PixelEfficiency2DVcThrCalDel_h_

#include <string>

#include "PixelCalibrations/include/PixelEfficiency2D.h"

class PixelEfficiency2DVcThrCalDel: public PixelEfficiency2D{
 public:

  // Constructor
  PixelEfficiency2DVcThrCalDel(std::string title, 
			       std::string name1,
			       unsigned int nbins1,
			       double min1,double max1, 
			       std::string name2,
			       unsigned int nbins2,
			       double min2,double max2);
  
  PixelEfficiency2DVcThrCalDel(const PixelEfficiency2DVcThrCalDel&);

  PixelEfficiency2DVcThrCalDel();

  virtual ~PixelEfficiency2DVcThrCalDel();

  const PixelEfficiency2DVcThrCalDel& operator=(const PixelEfficiency2DVcThrCalDel& anEff);

  void findSettings(double numerator);

  virtual void saveEff(double numerator, std::string filename);

  bool validSettings() { return validSettings_;}

  unsigned getThreshold() { return threshold_; }
  unsigned getCalDelay() { return calDelay_; }

  void setOldThreshold(unsigned int oldThreshold) {oldThreshold_=oldThreshold;}
  void setOldCalDelay(unsigned int oldCalDelay) {oldCalDelay_=oldCalDelay;}

  unsigned getOldThreshold() { return oldThreshold_; }
  unsigned getOldCalDelay() { return oldCalDelay_; }
  bool getValid(){return validSettings_;}


 private:

  bool validSettings_;
  unsigned int threshold_, calDelay_;
  unsigned int oldThreshold_, oldCalDelay_;

};

#endif
