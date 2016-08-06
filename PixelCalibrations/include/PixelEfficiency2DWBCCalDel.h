/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2007, Cornell		        	                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd               					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/
#ifndef _PixelEfficiency2DWBCCalDel_h_
#define _PixelEfficiency2DWBCCalDel_h_

#include <string>

#include "PixelCalibrations/include/PixelEfficiency2D.h"

class PixelEfficiency2DWBCCalDel: public PixelEfficiency2D{
 public:

  // Constructor
  PixelEfficiency2DWBCCalDel(std::string title, 
			       std::string name1,
			       unsigned int nbins1,
			       double min1,double max1, 
			       std::string name2,
			       unsigned int nbins2,
			       double min2,double max2);
  
  PixelEfficiency2DWBCCalDel(const PixelEfficiency2DWBCCalDel&);

  PixelEfficiency2DWBCCalDel();

  virtual ~PixelEfficiency2DWBCCalDel();

  const PixelEfficiency2DWBCCalDel& operator=(const PixelEfficiency2DWBCCalDel& anEff);

  void findSettings(double numerator, double WBC, double fract);

  virtual void saveEff(double numerator, std::string filename);

  bool validSlope(){ return (nleft_>0)&&(nright_>0);}

  double getSlope() { return slope_; }

  double getCalDel() { return caldel_; }

 private:

  unsigned int nleft_, nright_;
  double slopeleft_, sloperight_;
  double slope_;
  double caldel_;

};

#endif
