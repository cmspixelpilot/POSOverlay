/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2007, Cornell		        	                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd               					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/
#ifndef _PixelEfficiency2DVcThr_h_
#define _PixelEfficiency2DVcThr_h_

#include <string>

#include "PixelCalibrations/include/PixelEfficiency2D.h"

class PixelEfficiency2DVcThr: public PixelEfficiency2D{
 public:

  // Constructor
  PixelEfficiency2DVcThr(std::string title, 
			       std::string name1,
			       unsigned int nbins1,
			       double min1,double max1, 
			       std::string name2,
			       unsigned int nbins2,
			       double min2,double max2);
  
  PixelEfficiency2DVcThr(const PixelEfficiency2DVcThr&);

  PixelEfficiency2DVcThr();

  virtual ~PixelEfficiency2DVcThr();

  const PixelEfficiency2DVcThr& operator=(const PixelEfficiency2DVcThr& anEff);

  void findSettings(double numerator);
  void findThreshold(double numerator);
  void findThreshold2(double numerator);

  virtual void saveEff(double numerator, std::string filename);

  bool validVcThr(){ return validVcThr_;}
  double vcThr(){ return vcThr_;}

 private:

  double vcThr_;
  bool validVcThr_;

};

#endif
