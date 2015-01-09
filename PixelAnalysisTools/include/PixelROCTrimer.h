//
// This class calculates Vcthr, Vtrim, and trim bits for pixels
// on one ROC
//

#include<string>
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"

class PixelROCTrimmer {

 public:

  PixelROCTrimmer(pos::PixelROCName rocname);

  void read(std::string filename);

  void setVcThr(double oldVcThr);
  void setVtrim(double oldVtrim);
  void addPixel(int row, int col, int oldTrim, double thresh,
		double threshVcThr,
		double threshVtrim,
		double threshTrimOn,
		double threshTrimOff);

  void setThrTrim(double thrTrim);
  void setNsigma(double nSigma);

  void findNewSettings();

  void findNewBits(double derBit);

  double avgThr();

  int newTrim(int row, int col);

  int newVcThr();
  int newVtrim();

  double vcThrDer();
  double vtrimDer();
  double trimBitDer();

 private:

  void findDerivatives();
  void calcVtrim();
  void calcVcThr();
  void calcTrims();
  
  //should pixel be used in analysis
  bool use_[52][80];

  //initial Vcthr
  double vcthr_;
  //initial Vtrim
  double vtrim_;

  //initial trims
  int trim_[52][80];

  //new trims
  int newTrim_[52][80];

  //initial threshold (A)
  double threshold_[52][80];
  
  //Threshold after changing vcthr by +10;
  double thresholdVcThr_[52][80];

  //Threshold after changing vtrim by +10;
  double thresholdVtrim_[52][80];

  //Threshold after setting trim=15;
  double thresholdTrimMax_[52][80];

  //Threshold after setting trim=0;
  double thresholdTrimMin_[52][80];

  //Derivative B, with respect to VcTrh
  double b_[52][80];

  //Derivative C, with respect to Vtrim
  double c_[52][80];

  //Derivative D, with respect to trim
  double d_[52][80];

  //threshold to trim to
  double thrTrim_;
  
  //Number of sigmas to trim to.
  double nSigma_;

  //change in Vcthr
  double deltaVcthr_;

  //change in Vtrim
  double deltaVtrim_;

  //Change in trim bits;
  int deltaTrims_[52][80];

  pos::PixelROCName rocname_;

};
