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

#include "PixelCalibrations/include/PixelEfficiency2DWBCCalDel.h"
#include <assert.h>
#include <iostream>
#include <fstream>
#include <vector>

typedef unsigned int *intPTR;

using namespace std;

PixelEfficiency2DWBCCalDel::PixelEfficiency2DWBCCalDel(std::string title, 
				     std::string name1,unsigned int nbins1,
                                     double min1,double max1, 
                                     std::string name2,unsigned int nbins2,
                                     double min2,double max2):
  PixelEfficiency2D(title,name1,nbins1,min1,max1,
		    name2,nbins2,min2,max2)
{

  nleft_=0;
  nright_=0;
  slopeleft_=0.0;
  sloperight_=0.0;
  slope_=0.0;
  caldel_=0.0;

}

PixelEfficiency2DWBCCalDel::PixelEfficiency2DWBCCalDel():
  PixelEfficiency2D("Efficiency","",0,0.0,0.0,"",0,0.0,0.0)
{

  nleft_=0;
  nright_=0;
  slopeleft_=0.0;
  sloperight_=0.0;
  slope_=0.0;
  caldel_=0.0;

}

PixelEfficiency2DWBCCalDel::PixelEfficiency2DWBCCalDel(const PixelEfficiency2DWBCCalDel& anEff):
  PixelEfficiency2D(anEff.title_,
		    anEff.name1_,anEff.nbins1_,anEff.min1_,anEff.max1_,
		    anEff.name2_,anEff.nbins2_,anEff.min2_,anEff.max2_)

{

  nleft_=anEff.nleft_;
  nright_=anEff.nright_;
  slopeleft_=anEff.slopeleft_;
  sloperight_=anEff.sloperight_;
  slope_=anEff.slope_;
  caldel_=anEff.caldel_;

}

PixelEfficiency2DWBCCalDel::~PixelEfficiency2DWBCCalDel(){
}


const PixelEfficiency2DWBCCalDel& PixelEfficiency2DWBCCalDel::operator=(const PixelEfficiency2DWBCCalDel& anEff){

  PixelEfficiency2D::operator=(anEff);

  nleft_=anEff.nleft_;
  nright_=anEff.nright_;
  slopeleft_=anEff.slopeleft_;
  sloperight_=anEff.sloperight_;
  slope_=anEff.slope_;
  caldel_=anEff.caldel_;

  return *this;

}



void PixelEfficiency2DWBCCalDel::saveEff(double numerator,std::string filename){

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

  out << nleft_ << std::endl;
  out << nright_ << std::endl;
  out << slopeleft_ << std::endl;
  out << sloperight_ << std::endl;
  out << slope_ << std::endl;


  for(unsigned int i=0;i<nbins1_;i++){
    for(unsigned int j=0;j<nbins2_;j++){
      out <<getBinContent(i,j)/numerator<<std::endl;
    }
  }

  out.close();
}

void PixelEfficiency2DWBCCalDel::findSettings(double numerator, double WBC,
					      double fract){

  const bool localPrint=true;

  if(localPrint) cout << "nbins1_="<<nbins1_<<endl;
  if(localPrint) cout << "nbins2_="<<nbins2_<<endl;

  vector<double> left(nbins2_,-1.0);
  vector<double> right(nbins2_,-1.0);

  nleft_=0;
  nright_=0;

  //Y=MX

  double y1l=0.0;
  double y2l=0.0;
  double m11l=0.0;
  double m22l=0.0;
  double m12l=0.0;

  double y1r=0.0;
  double y2r=0.0;
  double m11r=0.0;
  double m22r=0.0;
  double m12r=0.0;


  for (unsigned int ibin2=0;ibin2<nbins2_;ibin2++){  // loop over WBC
    double wbc=min2_+(ibin2+0.5)*(max2_-min2_)/nbins2_;
    int nineff=0;
    int neff=0;
    if(localPrint) cout<<" wbc "<<wbc<<endl;
    for (unsigned int ibin1=0;ibin1<nbins1_-1;ibin1++){  // loop over CalDel
      if (getBinContent(ibin1,ibin2)>0.9*numerator) neff++;
      if (getBinContent(ibin1,ibin2)<0.1*numerator) nineff++;
      if (getBinContent(ibin1,ibin2)>=0.5*numerator&&
	  getBinContent(ibin1+1,ibin2)<0.5*numerator&&
	  neff>5) {
	//Found a right edge
	neff=-999; //will prevent finding more than one edge
	double rightedge=min1_+ibin1*(max1_-min1_)/nbins1_;
	if(localPrint)cout << "Found right edge for ibin2="<<ibin2<<" "<<rightedge<<endl;
	right[ibin2]=rightedge;
	nright_++;
        y1r+=rightedge;
        y2r+=rightedge*wbc;
        m11r+=1.0;
	m22r+=wbc*wbc;
	m12r+=wbc;
      }
      if (getBinContent(ibin1,ibin2)<=0.5*numerator&&
	  getBinContent(ibin1+1,ibin2)>0.5*numerator&&
	  nineff>5) {
	//Found a left edge
	nineff=-999; //will prevent finding more than one edge
	double leftedge=min1_+ibin1*(max1_-min1_)/nbins1_;
	if(localPrint) cout << "Found left edge for ibin2="<<ibin2<<" "<<leftedge<<endl;
	left[ibin2]=leftedge;
	nleft_++;
        y1l+=leftedge;
        y2l+=leftedge*wbc;
        m11l+=1.0;
	m22l+=wbc*wbc;
	m12l+=wbc;
      }
    }
  }

  if(localPrint) cout << "nleft_="<<nleft_<<" "<<y1l<<endl;
  if(localPrint) cout << "nright_="<<nright_<<" "<<y1r<<endl;

  // failed
  if (nleft_<1 || nright_<1 ) {
    caldel_=0;
    cout<<" Failed either left or right edge has not been found left/right = "
	<<nleft_<<"/"<<nright_<<endl;
    cout<<" Return caldel=0, old value will be stored"<<endl;
    return;
  }

  //for (unsigned int ibin2=0;ibin2<nbins2_;ibin2++){
  //cout << "ibin2="<<ibin2<<" left="<<left[ibin2]<<" right="
  // << right[ibin2] << endl;
  //}

  if(m11l == 0 || m11r == 0 ) {
    caldel_=0;
    cout<<" Failed-1 "<<m11l<<" "<<m11r;
    cout<<" Return caldel=0, old value will be stored"<<endl;
    return;
  }


  double caldel_right=-1.;
  double caldel_left =-1.;
  // take care of 1 WBC case, d.k. 13/1/15
  if( (nleft_ ==1) && (nright_ == 1) )  { //special case with 1 WBC only 

    if(localPrint) cout<<" 1 WBC only use simple non-interpolated edges "<<endl;
    caldel_right=y1r;
    caldel_left=y1l;

  } else { // more than 1 WBC, standard option
    double tmp1 = m22l-m12l*m12l/m11l;
    double tmp2 = m22r-m12r*m12r/m11r;
    if( tmp1==0 || tmp2 == 0 ) {
      caldel_=0;
      cout<<" Failed-2 "<<tmp1<<" "<<tmp2;
      cout<<" Return caldel=0, old value will be stored"<<endl;
      return;
    }
    
    //double bl=(y2l-m12l*y1l/m11l)/(m22l-m12l*m12l/m11l);
    double bl=(y2l-m12l*y1l/m11l)/tmp1;
    double al=(y1l-m12l*bl)/m11l;
    
    //double br=(y2r-m12r*y1r/m11r)/(m22r-m12r*m12r/m11r);
    double br=(y2r-m12r*y1r/m11r)/tmp2;
    double ar=(y1r-m12r*br)/m11r;
    
    if(localPrint) cout << "left  a,b:"<<al<<" "<<bl<<" "<<WBC<<endl;
    if(localPrint) cout << "right a,b:"<<ar<<" "<<br<<" "<<WBC<<endl;
    
    caldel_right=ar+br*WBC;
    caldel_left=al+bl*WBC;
  }

  if(localPrint) cout<<y1r<<" "<<y1l<<" "<<caldel_right<<" "<<caldel_left<<endl;


  // failed
  if (caldel_left<0||caldel_right<0 || caldel_left>255||caldel_right>255) {
    caldel_=0;
    nleft_=0;
    nright_=0;
    cout<<" Failed either left or right caldel = "
	<<caldel_left<<"/"<<caldel_right<<endl;
    cout<<" Return caldel=0, old value will be stored"<<endl;
    return;
  }

  caldel_=fract*caldel_right+(1.0-fract)*caldel_left;

  if(caldel_ <= 0 || caldel_ > 255 ) {
    cout << "[PixelEfficiency2DWBCCalDel::findSettings] caldel_right:"<<caldel_right<<endl;
    cout << "[PixelEfficiency2DWBCCalDel::findSettings] caldel_left :"<<caldel_left<<endl;
    cout << "[PixelEfficiency2DWBCCalDel::findSettings] caldel_     :"<<caldel_<<endl;
    caldel_ = 0;
    nleft_=0;
    nright_=0;
  }

  //cout << "[PixelEfficiency2DWBCCalDel::findSettings] caldel_     :"
  //   <<caldel_<<" "<<caldel_right<<" "<<caldel_left<<endl;

}
 





