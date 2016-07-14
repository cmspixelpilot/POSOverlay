/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2007, Cornell		        	                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd               					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/
#ifndef _PixelEfficiency2D_h_
#define _PixelEfficiency2D_h_

#include <string>
#include <TStyle.h>

class TH2F;

class PixelEfficiency2D{
 public:

  // Constructor
  PixelEfficiency2D(std::string title, 
		    std::string name1,unsigned int nbins1,
		    double min1,double max1, 
		    std::string name2,unsigned int nbins2,
		    double min2,double max2);

  PixelEfficiency2D(const PixelEfficiency2D&);

  PixelEfficiency2D();

  virtual ~PixelEfficiency2D();

  const PixelEfficiency2D& operator=(const PixelEfficiency2D& anEff);

  //ibin1=0..nbins1-1  ibin2=0..nbins2-1
  virtual void add(unsigned int ibin1, unsigned int ibin2);

  virtual void writeEff(double numerator);

  virtual void saveEff(double numerator, std::string filename);

  virtual void setTitle(std::string title) {title_=title;}

  double getmin1(){return min1_;}
  double getmax1(){return max1_;}
  double getmin2(){return min2_;}
  double getmax2(){return max2_;}
  unsigned int getnbins1(){return nbins1_;}
  unsigned int getnbins2(){return nbins2_;}
  double getBinContent(int i, int j) const;
  TH2F FillEfficiency(double numerator);
  void set_plot_style();

 protected:

  unsigned int nbins1_, nbins2_;
  std::string name1_, name2_;

  std::string title_;

  double min1_, max1_;
  double min2_, max2_;

  TH2F* efficiency_;

 
};

#endif
