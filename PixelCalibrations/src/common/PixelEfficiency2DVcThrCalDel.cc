// Modified by Jennifer Vaughan 2007/06/01
// $Id: PixelCalibrationBase.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelEfficiency2DVcThrCalDel.h"
#include <assert.h>
#include <iostream>
#include <fstream>

typedef unsigned int *intPTR;


PixelEfficiency2DVcThrCalDel::PixelEfficiency2DVcThrCalDel(std::string title, 
				     std::string name1,unsigned int nbins1,
                                     double min1,double max1, 
                                     std::string name2,unsigned int nbins2,
                                     double min2,double max2):
  PixelEfficiency2D(title,name1,nbins1,min1,max1,
		    name2,nbins2,min2,max2)
{

  validSettings_=false;
  threshold_=0;
  calDelay_=0;

}

PixelEfficiency2DVcThrCalDel::PixelEfficiency2DVcThrCalDel():
  PixelEfficiency2D("Efficiency","",0,0.0,0.0,"",0,0.0,0.0)
{

  validSettings_=false;
  threshold_=0;
  calDelay_=0;

}

PixelEfficiency2DVcThrCalDel::PixelEfficiency2DVcThrCalDel(const PixelEfficiency2DVcThrCalDel& anEff):
  PixelEfficiency2D(anEff.title_,
		    anEff.name1_,anEff.nbins1_,anEff.min1_,anEff.max1_,
		    anEff.name2_,anEff.nbins2_,anEff.min2_,anEff.max2_)

{
  validSettings_=anEff.validSettings_;
  threshold_=anEff.threshold_; 
  calDelay_=anEff.calDelay_;
}

PixelEfficiency2DVcThrCalDel::~PixelEfficiency2DVcThrCalDel(){
}


const PixelEfficiency2DVcThrCalDel& PixelEfficiency2DVcThrCalDel::operator=(const PixelEfficiency2DVcThrCalDel& anEff){

  PixelEfficiency2D::operator=(anEff);

  validSettings_=anEff.validSettings_;
  threshold_=anEff.threshold_; 
  calDelay_=anEff.calDelay_;


  return *this;

}



void PixelEfficiency2DVcThrCalDel::saveEff(double numerator,std::string filename){

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
  if (validSettings_) {
    out << 1 << std::endl;
  } else {
    out << 0 << std::endl;
  }
  out << threshold_ << std::endl;
  out << calDelay_ << std::endl;

  out << oldThreshold_ << std::endl;
  out << oldCalDelay_ << std::endl;

  for(unsigned int i=0;i<nbins1_;i++){
    for(unsigned int j=0;j<nbins2_;j++){
      out <<getBinContent(i,j)/numerator<<std::endl;
    }
  }

  out.close();
}

void PixelEfficiency2DVcThrCalDel::findSettings(double numerator){

  //std::cout << "In find Settings" << std::endl;

  unsigned int Vthreshold=0;

  //Note that if you use ibin1>=0 this will always be
  //true for an unsigned integer
  for (unsigned int ibin2=nbins2_-1;ibin2>0;ibin2--){

    //std::cout << "ibin1:" << ibin1 << std::endl;

    unsigned int countEfficientBins=0;

    for (unsigned int ibin1=0;ibin1<nbins1_;ibin1++){
      if (getBinContent(ibin1,ibin2)>0.9*numerator) {
        countEfficientBins++;
      }
    }

    //std::cout << "ibin1, countEfficientBins:"
    //          << ibin1<<" "<<countEfficientBins<<std::endl;

    if (countEfficientBins>1){
      Vthreshold=ibin2; 
      break;
    }

  }

  //std::cout << "Vthreshold:" << Vthreshold << std::endl;

  if (Vthreshold==0) {
    std::cout << "Did not find Vthreshold for:"<<title_<<std::endl;
    return;
  }

  threshold_=(int)(min2_+(Vthreshold+0.5)*(max2_-min2_)/nbins2_-25.0);

  
  //recalculate VThreshold
  int ibin2=(int)(Vthreshold-25/((max2_-min2_)/nbins1_));

  if (ibin2<1) {
    std::cout << "Vthreshold to small for:"<<title_<<std::endl;
    return;
  }
  
  //std::cout << "ibin1:"<<ibin1<<std::endl;
    
  
  int ibin1Start=-1;
  
  for (unsigned int ibin1=0;ibin1<nbins1_;ibin1++){
    if (getBinContent(ibin1,ibin2)>0.9*numerator) {
      ibin1Start=ibin1;
      break;
    }
  }
  
  if (ibin1Start==-1){
    std::cout << "Did not find CalDelay start for:"<<title_<<std::endl;
    return;
  }

  int ibin1End=-1;

  for (unsigned int ibin1=nbins1_-1;ibin1>0;ibin1--){
    if (getBinContent(ibin1,ibin2)>0.9*numerator) {
      ibin1End=ibin1;
      break;
    }
  }

  if (ibin1End==-1){
    std::cout << "Did not find CalDelay end for:"<<title_<<std::endl;
    return;
  }

  if (ibin1Start>=ibin1End){
    std::cout << "CalDelay range is to small for:"<<title_<<std::endl;
    return;
  }
  
  //std::cout << "ibin1Start:"<<ibin1Start<<std::endl;
  //std::cout << "ibin1End:"<<ibin1End<<std::endl;
  
  calDelay_=(unsigned int) (min1_+(0.5*(ibin1Start+ibin1End+1)*(max1_-min1_))/nbins1_); 

  validSettings_=true;

}
 





