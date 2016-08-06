// $Id: PixelCalibrationBase.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2007-2008, Cornell U.			                 *
 * All rights reserved.                                                  *
 * Authors: Anders Ryd	                        			 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelEfficiency2DVcThr.h"
#include <assert.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>

typedef unsigned int *intPTR;

using namespace std;

PixelEfficiency2DVcThr::PixelEfficiency2DVcThr(std::string title, 
				     std::string name1,unsigned int nbins1,
                                     double min1,double max1, 
                                     std::string name2,unsigned int nbins2,
                                     double min2,double max2):
  PixelEfficiency2D(title,name1,nbins1,min1,max1,
		    name2,nbins2,min2,max2)
{

  validVcThr_=false;
  vcThr_=0.0;

}

PixelEfficiency2DVcThr::PixelEfficiency2DVcThr():
  PixelEfficiency2D("Efficiency","",0,0.0,0.0,"",0,0.0,0.0)
{

  validVcThr_=false;
  vcThr_=0.0;

}

PixelEfficiency2DVcThr::PixelEfficiency2DVcThr(const PixelEfficiency2DVcThr& anEff):
  PixelEfficiency2D(anEff.title_,
		    anEff.name1_,anEff.nbins1_,anEff.min1_,anEff.max1_,
		    anEff.name2_,anEff.nbins2_,anEff.min2_,anEff.max2_)

{

  vcThr_=anEff.vcThr_;
  validVcThr_=anEff.validVcThr_;

}

PixelEfficiency2DVcThr::~PixelEfficiency2DVcThr(){
}


const PixelEfficiency2DVcThr& PixelEfficiency2DVcThr::operator=(const PixelEfficiency2DVcThr& anEff){

  PixelEfficiency2D::operator=(anEff);

  vcThr_=anEff.vcThr_;
  validVcThr_=anEff.validVcThr_;

  return *this;

}



void PixelEfficiency2DVcThr::saveEff(double numerator,std::string filename){

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

  out << vcThr_ << std::endl;
  out << (int)validVcThr_ << std::endl;


  for(unsigned int i=0;i<nbins1_;i++){
    for(unsigned int j=0;j<nbins2_;j++){
      out <<getBinContent(i,j)/numerator<<std::endl;
    }
  }

  out.close();
}

void PixelEfficiency2DVcThr::findSettings(double numerator){

  cout << "nbins1_="<<nbins1_<<endl;
  cout << "nbins2_="<<nbins2_<<endl;

  //FIXME this should not be an array...
  unsigned int ibin2=0;
  for (unsigned int ibin1=nbins1_-1;ibin1>0;ibin1--){
    if ((getBinContent(ibin1,ibin2)>0.95*numerator)&& 
	(getBinContent(ibin1-1,ibin2)>0.95*numerator)){
        vcThr_=min1_+ibin1*(max1_-min1_)/nbins1_;
	vcThr_=vcThr_-25.0;
	validVcThr_=true;
	cout << "Found vcThr="<<vcThr_<<endl;
	break;
    }
  }

  cout << "[PixelEfficiency2DVcThr::findSettings] VcThr_     :"<<vcThr_<<endl;

}

void PixelEfficiency2DVcThr::findThreshold(double numerator){

  cout << "nbins1_="<<nbins1_<<endl;
  cout << "nbins2_="<<nbins2_<<endl;

  //FIXME this should not be an array...
  unsigned int ibin2=0;
  for (unsigned int ibin1=nbins1_-1;ibin1>0;ibin1--){
    if ((getBinContent(ibin1,ibin2)>0.50*numerator)&&    // this finds the lower threshold
	(getBinContent(ibin1-1,ibin2)<0.50*numerator)){  // si the vcal level in this case
        vcThr_=min1_+ibin1*(max1_-min1_)/nbins1_;
	//vcThr_=vcThr_-25.0;
	validVcThr_=true;
	cout << "Found vcThr="<<vcThr_<<endl;
	break;
    }
  }

  cout << "[PixelEfficiency2DVcThr::findThreshold] VcThr_     :"<<vcThr_<<endl;
 
}

void PixelEfficiency2DVcThr::findThreshold2(double numerator){
  unsigned int sum[nbins1_]; // local 1d array
  
  // Compress the array to 1D
  for (unsigned int ibin1=nbins1_-1;ibin1>0;ibin1--){
    sum[ibin1]=0;
    for(unsigned int ibin2=0;ibin2<nbins2_;++ibin2) {
      sum[ibin1] += getBinContent(ibin1,ibin2);
    }
  }

//   for (unsigned int ibin1=nbins1_-1;ibin1>0;ibin1--){
//     if ((sum[ibin1]>= 0.50*numerator)&&    this finds the lower threshold
// 	(sum[ibin1-1]<0.50*numerator)) {  si the vcal level in this case
//       vcThr_=min1_+ibin1*(max1_-min1_)/nbins1_; the edge 
//       vcThr_=vcThr_-25.0;
//       validVcThr_=true;
//       cout << "Found vcThr="<<vcThr_;
//       break;
//     } 
//   }
  float halfCut = 0.50 * numerator;
  for (unsigned int ibin1=1;ibin1<nbins1_;++ibin1){
    if ((float(sum[ibin1])>= halfCut)&&  // this finds the lower VcThr value
	(float(sum[ibin1-1])<halfCut)) { // so higher threshold, the vcal level in this case
      int ibin=ibin1;  // select the closer value 
      if( fabs(float(sum[ibin1])  - halfCut) >  
	  fabs(float(sum[ibin1-1])- halfCut) ) ibin=ibin1-1;
      vcThr_=min1_+ibin*(max1_-min1_)/nbins1_; // the edge 
      validVcThr_=true;
      cout << "Found vcThr="<<vcThr_;
      break;
    } 
  }

  cout << "[PixelEfficiency2DVcThr::findThreshold2] VcThr_     :"<<vcThr_<<endl;

  if(!validVcThr_) {  // not found 
    cout<<" VcThr not found "<<numerator<<" "<<nbins1_<<endl;
    for (unsigned int ibin1=nbins1_-1;ibin1>0;ibin1--){
	float tmp = min1_+ibin1*(max1_-min1_)/nbins1_; // the edge 
	cout<<ibin1<<" "<<tmp<<" "<<sum[ibin1]<<" "<<sum[ibin1-1]<<" "
	    <<0.50*numerator<<" ";
	if(sum[ibin1]  >=0.50*numerator) cout<<" >0.50 ";
	if(sum[ibin1-1]<0.50*numerator) cout<<" <0.50 ";
	cout<<endl;
    }
  }

 
}
 


