// $Id: PixelEfficiency2D.cc,v 1.18 2010/02/01 19:13:22 aryd Exp $
// Modified by Jennifer Vaughan 2007/06/01

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelEfficiency2D.h"
#include <assert.h>
#include <iostream>
#include <fstream>

#include "TH2F.h"
#include "TColor.h"

typedef unsigned int *intPTR;

using namespace std;

PixelEfficiency2D::PixelEfficiency2D(std::string title, std::string name1,unsigned int nbins1,
                                     double min1,double max1, 
                                     std::string name2,unsigned int nbins2,
                                     double min2,double max2){
  set_plot_style();

  title_=title;

  name1_=name1;
  name2_=name2;

  nbins1_=nbins1;
  nbins2_=nbins2;

  min1_=min1;
  max1_=max1;

  min2_=min2;
  max2_=max2;

  if (nbins1_==0) {
    efficiency_=0;
  } else{
    efficiency_=new TH2F(title_.c_str(),title_.c_str(),
			 nbins1_,min1_,max1_,
			 nbins2_,min2_,max2_);
  }

}

PixelEfficiency2D::PixelEfficiency2D(){

  title_="Efficiency";

  name1_="";
  name2_="";

  nbins1_=0;
  nbins2_=0;

  min1_=0.0;
  max1_=0.0;

  min2_=0.0;
  max2_=0.0;

  efficiency_=0;



}

PixelEfficiency2D::PixelEfficiency2D(const PixelEfficiency2D& anEff){

  title_=anEff.title_;

  name1_=anEff.name1_;
  name2_=anEff.name2_;

  nbins1_=anEff.nbins1_;
  nbins2_=anEff.nbins2_;

  min1_=anEff.min1_;
  max1_=anEff.max1_;

  min2_=anEff.min2_;
  max2_=anEff.max2_;

  if (anEff.efficiency_!=0){
    efficiency_=new TH2F(*anEff.efficiency_);
  }
  else {
    efficiency_=0;
  }


}


const PixelEfficiency2D& PixelEfficiency2D::operator=(const PixelEfficiency2D& anEff){


  title_=anEff.title_;

  name1_=anEff.name1_;
  name2_=anEff.name2_;

  nbins1_=anEff.nbins1_;
  nbins2_=anEff.nbins2_;

  min1_=anEff.min1_;
  max1_=anEff.max1_;

  min2_=anEff.min2_;
  max2_=anEff.max2_;

 if (anEff.efficiency_!=0){
    efficiency_=new TH2F(*anEff.efficiency_);
  }
  else {
    efficiency_=0;
  }

  return *this;

}



void PixelEfficiency2D::add(unsigned int ibin1, unsigned int ibin2){


  assert(ibin1<nbins1_);
  assert(ibin2<nbins2_);

  efficiency_->Fill(min1_+(ibin1+0.5)*(max1_-min1_)/nbins1_,
		    min2_+(ibin2+0.5)*(max2_-min2_)/nbins2_);

}

void PixelEfficiency2D::writeEff(double numerator){

  for(unsigned int i=0;i<nbins1_;i++){
    for(unsigned int j=0;j<nbins2_;j++){
      std::cout <<efficiency_->GetBinContent(i,j)/numerator<<" ";
    }
    std::cout << std::endl;
  }
}


void PixelEfficiency2D::saveEff(double numerator,std::string filename){

  std::ofstream out(filename.c_str());

  out << title_ <<std::endl;
  out << name1_ <<std::endl;
  out << nbins1_ << std::endl;
  out << min1_ << std::endl;
  out << max1_ << std::endl;
  out << name2_ <<std::endl;
  out << nbins2_ << std::endl;
  out << min2_ << std::endl;
  out << max2_ << std::endl;

  for(unsigned int i=0;i<nbins1_;i++){
    for(unsigned int j=0;j<nbins2_;j++){
      out <<efficiency_->GetBinContent(i,j)/numerator<<std::endl;
    }
  }

  out.close();
}

TH2F PixelEfficiency2D::FillEfficiency(double numerator){

  TH2F tmp=*efficiency_;
  tmp.Scale(1.0/numerator);

  return tmp;
}


PixelEfficiency2D::~PixelEfficiency2D(){

  delete efficiency_;

}

void PixelEfficiency2D::set_plot_style(){

    gStyle->SetFillColor(0);
    gStyle->SetOptStat(0);
    gStyle->SetPalette(1);
    
}

double PixelEfficiency2D::getBinContent(int i, int j) const {

  return efficiency_->GetBinContent(i+1,j+1);

}
