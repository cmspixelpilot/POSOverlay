//
// This class calculates Vcthr, Vtrim, and trim bits for pixels
// on one ROC
//

#include "PixelAnalysisTools/include/PixelROCTrimer.h"
#include <fstream>
#include <iostream>
#include <math.h>

using namespace std;
using namespace pos;

PixelROCTrimmer::PixelROCTrimmer(PixelROCName rocname){

  rocname_=rocname;
 

  for (unsigned int icol=0;icol<52;icol++){
    for (unsigned int irow=0;irow<80;irow++){
      use_[icol][irow]=false;
    }
  }

}


void PixelROCTrimmer::setVcThr(double oldVcThr){

  vcthr_=oldVcThr;

}


void PixelROCTrimmer::setVtrim(double oldVtrim){

  vtrim_=oldVtrim;

}


void PixelROCTrimmer::addPixel(int row, int col, 
			       int oldTrim, 
			       double thresh,
			       double threshVcThr,
			       double threshVtrim,
			       double threshTrimOn,
			       double threshTrimOff){

  use_[col][row]=true;
  trim_[col][row]=oldTrim;
  threshold_[col][row]=thresh;
  thresholdVcThr_[col][row]=threshVcThr;
  thresholdVtrim_[col][row]=threshVtrim;
  thresholdTrimMax_[col][row]=threshTrimOn;
  thresholdTrimMin_[col][row]=threshTrimOff;

}


void PixelROCTrimmer::read(std::string filename){
  
  ifstream in(filename.c_str());
  assert(in.good());
  
  string tmp;
  
  in >> tmp;
  assert(tmp=="vcthr");

  in >> vcthr_;
  
  in >> tmp;
  assert(tmp=="vtrim");

  in >> vtrim_;

  int col,row,trim;
  double threshold;
  double thresholdVcThr;
  double thresholdVtrim;
  double thresholdTrimMax;
  double thresholdTrimMin;

  in >> col >> row >> trim >> threshold >> thresholdVcThr
     >> thresholdVtrim >> thresholdTrimMax >> thresholdTrimMin;

  while (col!=-1) {
    use_[col][row]=true;
    trim_[col][row]=trim;
    threshold_[col][row]=threshold;
    thresholdVcThr_[col][row]=thresholdVcThr;
    thresholdVtrim_[col][row]=thresholdVtrim;
    thresholdTrimMax_[col][row]=thresholdTrimMax;
    thresholdTrimMin_[col][row]=thresholdTrimMin;
    in >> col >> row >> trim >> threshold >> thresholdVcThr
       >> thresholdVtrim >> thresholdTrimMax >> thresholdTrimMin;
  }

}

void PixelROCTrimmer::setThrTrim(double thrTrim){
  thrTrim_=thrTrim;
}

void PixelROCTrimmer::setNsigma(double nSigma){
  nSigma_=nSigma;
}

void PixelROCTrimmer::findNewSettings(){

  findDerivatives();
  calcVtrim();
  calcVcThr();
  calcTrims();

}


void PixelROCTrimmer::findDerivatives(){

  double bavg=0.0;
  double cavg=0.0;
  double davg=0.0;
  int nb=0;
  int nc=0;
  int nd=0;
  
  for (unsigned int icol=0;icol<52;icol++){
    for (unsigned int irow=0;irow<80;irow++){
      if (!use_[icol][irow]) continue;

      if ((thresholdVcThr_[icol][irow]<0)&&
	  (threshold_[icol][irow]<0)&&
	  (thresholdVtrim_[icol][irow]<0)&&
	  (thresholdTrimMax_[icol][irow]<0)&&
	  (thresholdTrimMin_[icol][irow]<0)){
	//We did not have a single Scurve for this pixel.
	//we can not trim this pixel!
	use_[icol][irow]=false;
	continue;
      }

      if ((thresholdVcThr_[icol][irow]>0)&&
	  (threshold_[icol][irow]>0)){
	//FIXME should not be hard coded.
	b_[icol][irow]=(thresholdVcThr_[icol][irow]-threshold_[icol][irow])/(-5.0);
	bavg+=b_[icol][irow];
	nb++;
      }
      else{
	b_[icol][irow]=-100000.0;
      }

      if ((thresholdVtrim_[icol][irow]>0)&&
	  (threshold_[icol][irow]>0)){
	//FIXME should not be hard coded.
	c_[icol][irow]=(thresholdVtrim_[icol][irow]-threshold_[icol][irow])/(-20.0);
	cavg+=c_[icol][irow];
	nc++;
      }
      else{
	c_[icol][irow]=-100000.0;
      }
      

      if ((thresholdTrimMax_[icol][irow]>0)&&
	  (thresholdTrimMin_[icol][irow]>0)){
	d_[icol][irow]=(thresholdTrimMax_[icol][irow]-thresholdTrimMin_[icol][irow])/15.0;
	cout << "case 1:d_["<<icol<<"]["<<irow<<"]="<<d_[icol][irow]<<endl;
	davg+=d_[icol][irow];
	nd++;
      }
      else if ((thresholdTrimMax_[icol][irow]>0)&&
	  (thresholdTrimMin_[icol][irow]<0)&&
	  (threshold_[icol][irow]>0)&&
	  (trim_[icol][irow]<14)){
	d_[icol][irow]=(thresholdTrimMax_[icol][irow]-threshold_[icol][irow])/(15.0-trim_[icol][irow]);
	cout << "case 2:d_["<<icol<<"]["<<irow<<"]="<<d_[icol][irow]<<endl;
	davg+=d_[icol][irow];
	nd++;
      }
      else if ((thresholdTrimMax_[icol][irow]<0)&&
	  (thresholdTrimMin_[icol][irow]>0)&&
	  (threshold_[icol][irow]>0)&&
	  (trim_[icol][irow]>1)){
	d_[icol][irow]=(threshold_[icol][irow]-thresholdTrimMin_[icol][irow])/(0.0-trim_[icol][irow]);
	cout << "case 3:d_["<<icol<<"]["<<irow<<"]="<<d_[icol][irow]<<endl;
	davg+=d_[icol][irow];
	nd++;
      }
      else {
	d_[icol][irow]=-100000.0;	
      }

    }
  }

  if (nd==0) { 
    cout << "nb nc nd:"<<nb<<" "<<nc<<" "<<nd<<endl; 
    davg=0.5;
  }
  else{
    davg=davg/nd;
  }

  //assert(nb>0);
  //assert(nc>0);
  
  if (nb>0) {
    bavg=bavg/nb;
  } else{
    bavg=100000.0; //safer to make very large. will give smalle changes.
  }

  if (nc>0) cavg=cavg/nc;
  

  for (unsigned int icol=0;icol<52;icol++){
    for (unsigned int irow=0;irow<80;irow++){
      if (!use_[icol][irow]) continue;
      
      if (b_[icol][irow]<-10000) {
	b_[icol][irow]=bavg;
      }

      if (c_[icol][irow]<-10000) {
	c_[icol][irow]=cavg;
      }

      if (d_[icol][irow]<-10000) {
	d_[icol][irow]=davg;
	cout << "Set to avg: d_["<<icol<<"]["<<irow<<"]="<<d_[icol][irow]<<endl;

      }

      if ((threshold_[icol][irow])<0) {
	//need to make up a number here...
	
	//but for now let's not use the pixel
	use_[icol][irow]=false;
	
      }
    }
  }
}

void PixelROCTrimmer::calcVtrim(){

  double sum=0.0;
  double sumsq=0.0;
  int n=0;

  double sumOn=0.0;
  double sumsqOn=0.0;
  int nOn=0;

  double sumOff=0.0;
  double sumsqOff=0.0;
  int nOff=0;

  int nC=0;
  double sumC=0.0;

  for (unsigned int icol=0;icol<52;icol++){
    for (unsigned int irow=0;irow<80;irow++){
      if (!use_[icol][irow]) continue;
      if (threshold_[icol][irow]>0){
	n++;
	sum+=threshold_[icol][irow];
	sumsq+=threshold_[icol][irow]*threshold_[icol][irow];
      }
      if (thresholdTrimMax_[icol][irow]>0){
	nOn++;
	sumOn+=thresholdTrimMax_[icol][irow];
	sumsqOn+=thresholdTrimMax_[icol][irow]*thresholdTrimMax_[icol][irow];
      }
      if (thresholdTrimMin_[icol][irow]>0){
	nOff++;
	sumOff+=thresholdTrimMin_[icol][irow];
	sumsqOff+=thresholdTrimMin_[icol][irow]*thresholdTrimMin_[icol][irow];
      }
      if (trim_[icol][irow]>3) {
	sumC+=(c_[icol][irow]*15/trim_[icol][irow]);
	nC++;
      }
    }
  }

  //assert(nC>0);
   double meanC=0.0;
   if (nC>0){
     meanC=sumC/nC;
   }
  cout << "meanC               :"<<meanC<<endl;

  if (nOn<=1||nOff<=1){
    deltaVtrim_=0;
    return;
  }

  double mean=-1;
  double rms=-1;

  if (n>0) {
    mean=sum/n;
    rms=sqrt(sumsq/n-sum*sum/(n*n));
  }

  cout << "Mean and rms for "<<rocname_ <<" "<<mean<<" "<<rms<<" "<<n<<endl;

  assert(nOn>0);  
  double meanOn=sumOn/nOn;
  double rmsOn=sqrt(sumsqOn/nOn-sumOn*sumOn/(nOn*nOn));
  cout << "mean and rms for on :"<<meanOn<<" "<<rmsOn<<endl;

  assert(nOff>0);  
  double meanOff=sumOff/nOff;
  double rmsOff=sqrt(sumsqOff/nOff-sumOff*sumOff/(nOff*nOff));
  cout << "mean and rms for off:"<<meanOff<<" "<<rmsOff<<endl;


  cout << "nSigma_="<<nSigma_<<endl;

  double targetSpread=nSigma_*(rmsOn+rmsOff)*0.5;

  double currentSpread=meanOn-meanOff;

  cout << "targetSpread :"<<targetSpread<<endl;
  cout << "currentSpread:"<<currentSpread<<endl;
  
  if (targetSpread<10) targetSpread=10; //no reason to make the spread to
                                        //small.
  
  double deltaSpread=targetSpread-currentSpread;
  
  deltaVtrim_=-deltaSpread/meanC; //minus sign from definition of C.
  
  if (deltaVtrim_>0.20*vtrim_) deltaVtrim_=0.20*vtrim_;
  if (deltaVtrim_<-0.20*vtrim_) deltaVtrim_=-0.20*vtrim_;
  
  if (deltaVtrim_<0 && vtrim_+deltaVtrim_<50) deltaVtrim_=50-vtrim_;
  
  cout << "Found deltaVtrim["<<rocname_<<"] = "<<deltaVtrim_<<endl;
  
}

int PixelROCTrimmer::newTrim(int row, int col){

  return newTrim_[col][row];

}


int PixelROCTrimmer::newVtrim(){

  int vtrim=(int)(vtrim_+deltaVtrim_);
  if (vtrim<0) vtrim=0;
  if (vtrim>255) vtrim=255;
    
  return vtrim;

}

int PixelROCTrimmer::newVcThr(){

  int vcthr=(int)(vcthr_+deltaVcthr_);
  if (vcthr<0) vcthr=0;
  if (vcthr>255) vcthr=255;
    
  return vcthr;

}


void PixelROCTrimmer::calcVcThr(){

  double sum1=0.0;
  double sum2=0.0;
  double sum3=0.0;
  double sum4=0.0;
  double sum5=0.0;

  for (unsigned int icol=0;icol<52;icol++){
    for (unsigned int irow=0;irow<80;irow++){
      if (!use_[icol][irow]) continue;

      if (fabs(d_[icol][irow])<1e-2) {
	cout << "d_["<<icol<<"]["<<irow<<"]="<<d_[icol][irow]<<endl;
	continue;
      }

      cout << "d_["<<icol<<"]["<<irow<<"]="<<d_[icol][irow]<<endl;
      cout << thrTrim_<<" "<<threshold_[icol][irow]<<endl;

      sum1+=thrTrim_/d_[icol][irow];
      sum2+=threshold_[icol][irow]/d_[icol][irow];
      sum3+=b_[icol][irow]/d_[icol][irow];
      sum4+=deltaVtrim_*c_[icol][irow]/d_[icol][irow];
      sum5+=7.5-trim_[icol][irow];
    }
  }

  deltaVcthr_=(sum1-sum2-sum4-sum5)/sum3;

  if (deltaVcthr_!=deltaVcthr_||1) {
    cout << "sum1="<<sum1<<endl;
    cout << "sum2="<<sum2<<endl;
    cout << "sum3="<<sum3<<endl;
    cout << "sum4="<<sum4<<endl;
    cout << "sum5="<<sum5<<endl;
  }

  if (deltaVcthr_!=deltaVcthr_) deltaVcthr_=0; 

  if (deltaVcthr_>25) deltaVcthr_=25;
  if (deltaVcthr_<-25) deltaVcthr_=-25;

  cout << "Found deltaVcthr["<<rocname_<<"]  = "<<deltaVcthr_<<endl;

}

void PixelROCTrimmer::calcTrims(){

  for (unsigned int icol=0;icol<52;icol++){
    for (unsigned int irow=0;irow<80;irow++){
      if (!use_[icol][irow]) continue;

      deltaTrims_[icol][irow]=(int)((thrTrim_-threshold_[icol][irow]-
				    b_[icol][irow]*deltaVcthr_-
				    c_[icol][irow]*deltaVtrim_)/d_[icol][irow]);

      newTrim_[icol][irow]=trim_[icol][irow]+deltaTrims_[icol][irow];

      if (newTrim_[icol][irow]<0) {
	newTrim_[icol][irow]=0;
      }

      if (newTrim_[icol][irow]>15) {
	newTrim_[icol][irow]=15;
      }

      cout << "deltaTrims_["<<icol<<"]["<<irow<<"]="
	   <<deltaTrims_[icol][irow]<<endl;

      cout << "newTrim_["<<icol<<"]["<<irow<<"]="
	   <<newTrim_[icol][irow]<<endl;

    }
  }
}

void PixelROCTrimmer::findNewBits(double derBit){

  for (unsigned int icol=0;icol<52;icol++){
    for (unsigned int irow=0;irow<80;irow++){
      if (!use_[icol][irow]) continue;
      
      deltaTrims_[icol][irow]=(int)((thrTrim_-threshold_[icol][irow])/derBit);

      newTrim_[icol][irow]=trim_[icol][irow]+deltaTrims_[icol][irow];

      if (newTrim_[icol][irow]<0) {
	newTrim_[icol][irow]=0;
      }

      if (newTrim_[icol][irow]>15) {
	newTrim_[icol][irow]=15;
      }

      cout << "deltaTrims_["<<icol<<"]["<<irow<<"]="
	   <<deltaTrims_[icol][irow]<<endl;

      cout << "newTrim_["<<icol<<"]["<<irow<<"]="
	   <<newTrim_[icol][irow]<<endl;

    }
  }
}



  
double PixelROCTrimmer::vcThrDer(){

  double der=0.0;

  int nder=0;

  for (unsigned int icol=0;icol<52;icol++){
    for (unsigned int irow=0;irow<80;irow++){
      if (!use_[icol][irow]) continue;

      nder++;

      der+=b_[icol][irow];

    }
  }

  if (nder>0) {
    der=der/nder;
    return der;
  }
  else {
    return 999.9;
  }

}
double PixelROCTrimmer::vtrimDer(){

  double der=0.0;

  int nder=0;

  for (unsigned int icol=0;icol<52;icol++){
    for (unsigned int irow=0;irow<80;irow++){
      if (!use_[icol][irow]) continue;

      nder++;

      der+=c_[icol][irow];

    }
  }

  if (nder>0) {
    der=der/nder;
    return der;
  }
  else {
    return 999.9;
  }
}


double PixelROCTrimmer::trimBitDer(){

  double der=0.0;

  int nder=0;

  for (unsigned int icol=0;icol<52;icol++){
    for (unsigned int irow=0;irow<80;irow++){
      if (!use_[icol][irow]) continue;

      nder++;

      der+=d_[icol][irow];

    }
  }

  if (nder>0) {
    der=der/nder;
    return der;
  }
  else {
    return 999.9;
  }

}

double PixelROCTrimmer::avgThr(){

  int n=0;
  double sum=0;

  for (unsigned int icol=0;icol<52;icol++){
    for (unsigned int irow=0;irow<80;irow++){
      if (!use_[icol][irow]) continue;

      n++;

      sum+=threshold_[icol][irow];

    }
  }

  if (n==0) return 0;
  return sum/n;

}
