#include "PixelUtilities/PixelFEDDataTools/include/AddressLevels.h"

#include <math.h>
#include <iostream>
#include <fstream>

#include "TCanvas.h"
#include "TGraph.h"
#include "TH1F.h"



AddressLevels::AddressLevels(){
  for (unsigned int i=0;i<MAX_ADC;++i){
    histogram_[i]=0;
  }
  for (unsigned int i=0;i<=5;++i){
    L_[i]=0;
  }
  validAddressLevels_=false;
}

unsigned int AddressLevels::getPoint(unsigned int pos){
  return histogram_[pos];
}

void AddressLevels::setPoint(unsigned int pos, unsigned int value){
  if (pos<MAX_ADC) histogram_[pos]=value;
}

void AddressLevels::addPoint(unsigned int a){
  if (a<MAX_ADC)
    histogram_[a]+=1;
}

unsigned int AddressLevels::findPeaks(unsigned int minimumPeakCount){
  bool f=false;
  Moments aPeak;
  for (unsigned int bin=0;bin<MAX_ADC-1;++bin){
    if ((histogram_[bin]>0 && histogram_[bin+1]>0) && f==false){
      f=true;
      //cout<<"Methinks there is peak at "<<bin<<endl;
      aPeak.clear();
      aPeak.push_back(bin, histogram_[bin]);
    }
    else if (histogram_[bin]>0 && f==true){
      //cout<<"peak continues in bin "<<bin<<" with "<<AddressLevels.at(bin)<<endl;
      aPeak.push_back(bin, histogram_[bin]);
    }
    else if ((histogram_[bin]==0 && histogram_[bin+1]==0) && f==true){
      f=false;
      //std::cout<<"End of peak at bin "<<bin<<endl;
      //std::cout<<"a peak mean="<<aPeak.mean()<<" with a stddev="<<aPeak.stddev()<<endl;
      //std::cout<<"N="<<aPeak.count()<<" sum="<<aPeak.sum()<<" squares="<<aPeak.squares()<<endl;
      if (aPeak.count()>minimumPeakCount) peaks_.push_back(aPeak);
      else std::cout << "[AddressLevels::findPeaks()]\tI'm sorry, peaks with less than " << minimumPeakCount << " data points are not allowed in!" << std::endl;
    }
  }

  if (peaks_.size()==6){
    validAddressLevels_=true;
    for (unsigned int p=0;p<5;++p){
      const Moments& peak1=peaks_.at(p);
      const Moments& peak2=peaks_.at(p+1);

      L_[p]=(unsigned int)(peak1.mean()+peak2.mean())/2;
      if (L_[p] < peak1.mean()+3*peak1.stddev() || L_[p] > peak2.mean()-3*peak2.stddev()){
	std::cout<<"WARNING: Recommended Threshold Level "<<p<<"within 3 standard deviations from an Address Level!"<<std::endl;
	validAddressLevels_=false;
      }
    }
  }
  return peaks_.size();
}

double AddressLevels::rms(unsigned int n) {
  if( n>=peaks_.size() ) return -1;
  return peaks_.at(n).stddev();
}

double AddressLevels::maxrms(){
  double max=-1;
  for (unsigned int i=0;i<peaks_.size();i++) {
    if (rms(i) > max) max=rms(i);
  }
  return max;
}

double AddressLevels::separation(unsigned int n) {
  if( (n+1) > peaks_.size() ) return -1; // must have peaks at n and n+1
  double sep=0;
  double wide;
  wide=rms(n+1)*rms(n+1)+rms(n)*rms(n);
  if(wide<=0) wide=1.;  // just return peak difference in adc counts
  wide=sqrt(wide);
  sep = (peaks_.at(n+1).mean() - peaks_.at(n).mean())/wide;
  return sep;
}

double AddressLevels::minseparation() {
  double min=1.e6;
  for(unsigned int i=0; i<peaks_.size()-1 ; i++) {
    if(min>separation(i)) min=separation(i);
  }
  return min;
}

void AddressLevels::printHistogram(){
  for (unsigned int i=0;i<MAX_ADC;++i){
    std::cout<<"i="<<i<<"count="<<histogram_[i]<<std::endl;
  }
}

void AddressLevels::dump(std::string fname){

  std::ofstream out;
  out.open(fname.c_str());


  for (unsigned int i=0;i<MAX_ADC;++i){
    out<<i<<" "<<histogram_[i]<<std::endl;
  }

  out.close();

}

void AddressLevels::drawHistogram(std::string fileName){

  const char *option="gif";
  TCanvas c("c", "Address Levels", 400, 400);
  
  TH1F h("ROC", "Read Out Chip", 1024, -0.5, 1023.5);
  for (unsigned int i=0;i<MAX_ADC;++i){
    h.Fill(i, histogram_[i]);
  }
  h.Draw();
  if (fileName!=""){
    c.Print(fileName.c_str(), option);
  }
}

AddressLevels AddressLevels::operator+ (AddressLevels a){
  AddressLevels result;
  for (unsigned int i=0;i<MAX_ADC;++i){
    result.setPoint(i, histogram_[i]+a.getPoint(i));
  }
  return result;
}

void AddressLevels::operator+= (AddressLevels a){
  for (unsigned int i=0; i<MAX_ADC;++i){
    histogram_[i]+=a.getPoint(i);
  }
}

