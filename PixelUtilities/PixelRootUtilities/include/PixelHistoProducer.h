#ifndef _PixelHistoProducer_h_
#define _PixelHistoProducer_h_

#include "PixelUtilities/PixelRootUtilities/include/PixelHistoThreadFrame.h"

class TCanvas;       
class TH1F;          
class TH2F;          
class TProfile;      
class TDirectory;

class PixelHistoProducer: public PixelHistoThreadFrame{
 public:
  PixelHistoProducer();
  void init();
  ~PixelHistoProducer();
  void userFunc0();
 private:
  void fill();
	TCanvas       *fCanvas;    // main canvas
  TH1F          *fHpx;       // 1-D histogram
  TH2F          *fHpxpy;     // 2-D histogram
  TProfile      *fHprof;     // profile histogram
  TDirectory    *histoDir;
  int checkedTimes;
};


#endif
