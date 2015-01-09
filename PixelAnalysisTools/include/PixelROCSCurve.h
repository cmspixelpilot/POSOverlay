#include "TH1F.h"
#include "TH2F.h"
#include "TFile.h"
//#include "PixelXmlReader.h"
#include <iostream>
#include <vector>
#include <utility>

class PixelROCSCurve {

 public:
  
  PixelROCSCurve();

  void init(unsigned int linkid, unsigned int rocid, 
	    unsigned int nvcal,unsigned int vcalmin, unsigned int vcalmax, int ntrig);

  bool filled(unsigned int row,unsigned int col);

  bool isValid() {
    //cout << (noise1d) << " " << ((bool) noise1d!=0) <<endl;
    return (threshold1d!=0&&noise1d!=0&&fitprob!=0&&threshold!=0&&noise!=0&&fitprob!=0);
  }

  void draw(unsigned int row,unsigned int col);

  void fill(unsigned int row,unsigned int col, unsigned int vcal);  

  bool fit(unsigned int row, unsigned int col);

  // draw 2d results for one roc
  void drawThresholds();

  void drawNoises();

  void drawFitProbs();

  // draw 1d histograms of fit parameters
  void drawThreshold();

  void drawNoise();

  void drawFitProb();

  void draw(int k){
    //cout<<"t1="<<threshold1d<<" n1="<<noise1d<<" f1="<<fitprob<<" t="<<threshold<<"n="<<noise<<" f="<<fitprob<<endl;
    if (isValid()){
      if (k==0) drawNoise();
      else if (k==1) drawThreshold();
      else if (k==2) drawFitProb();
      else if (k==3) drawNoises();
      else if (k==4) drawThresholds();
      else drawFitProbs();
    }
  }

  std::vector<std::pair<unsigned int, unsigned int> > getNoisyCells() { return noisyCells;}

  std::vector<std::pair<unsigned int, unsigned int> > getErrorCells() { return errorCells;}

  double getMeanNoise() { return noise1d->GetMean(); }
  double getMeanThreshold() { return threshold1d->GetMean(); }

  void write(TFile* file);

  int ntrials;
  double StartCurve;
  double EndCurve;
  double Accept;
  double AcceptFit;  
  double Noisy;
  double Error;


 private:

  static const unsigned int kmincol_=21;
  static const unsigned int kmaxcol_=30;
  static const unsigned int kminrow_=35;
  static const unsigned int kmaxrow_=44;

  unsigned int linkid_;
  unsigned int rocid_;
  unsigned int nvcal_;
  unsigned int vcalmin_;
  unsigned int vcalmax_;
  int ntrig_;

  unsigned char turnon_[80][52];  //vcal value where hits are first seen

  static Double_t fitfcn(Double_t * x, Double_t * par);

  TH1F* scurve[80][52];
  
  // 2d histograms (col,row) of scurve fit results
  TH2F* threshold;
  
  TH2F* noise;

  TH2F* fitprob;

  // 1d histograms of fit parameters

  TH1F* threshold1d;

  TH1F* noise1d;

  TH1F* fitprob1d;
  
  std::vector<std::pair<unsigned int, unsigned int> > noisyCells;
  std::vector<std::pair<unsigned int, unsigned int> > errorCells;
};
