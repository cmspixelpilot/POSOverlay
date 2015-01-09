#ifndef _PixelGainHistoManager_h_
#define _PixelGainHistoManager_h_

#include "PixelAnalysisTools/include/PixelHistoManager.h"
#include <fstream>
#include <iostream>
#include <string>
#include <map>

class PixelXmlReader;
class PixelCalibConfigurationExtended;
class PixelConfigurationsManager;
class TDirectory;
class TH1F;
class TH2F;
class TF1;

class PixelGainHistoManager : public PixelHistoManager{
 public:
  PixelGainHistoManager(PixelXmlReader* xmlReader, PixelCalibConfigurationExtended *calib, PixelConfigurationsManager * configurationsManager, std::ostream *logger=&std::cout);
  ~PixelGainHistoManager();
  void init                (void);
  void bookHistos          (void);
  void reset               (void);
  void destroy             (void);
  void drawHisto           (unsigned int fed, unsigned int channel, unsigned int roc, std::string summary, std::string panelType, int plaquette, TH1* &summaryH);
  void makeSummaryPlots    (void);
  void endOfPatternAnalysis(void);
  void endOfFileAnalysis   (void);
  void fillHistos          (unsigned int fed, unsigned int channel, unsigned int roc, unsigned int row, unsigned int col, unsigned int vcalvalue, unsigned int adc);
  void fit                 (void);

 private:
  //the last map is: map<VcalValue,# of Entries>
  std::map<unsigned int, std::map<unsigned int, std::map<unsigned int , std::vector<std::map<int,int> > > > >  entriesMap_;

  std::map<std::string,TH1F *>  histoADC1DMap_;
  std::map<std::string,TH1F *>  histoSlope1DMap_;
  std::map<std::string,TH1F *>  histoIntercept1DMap_;
  std::map<std::string,TH1F *>  histoChisquare1DMap_;
  std::map<std::string,TH1F *>  histoProbability1DMap_;
  //std::map<std::string,TH1F *>  linearFitStatistic1DMap_;
  std::map<std::string,TH1F *>  histoTanhPar01DMap_;
  std::map<std::string,TH1F *>  histoTanhPar11DMap_;
  std::map<std::string,TH1F *>  histoTanhPar21DMap_;
  std::map<std::string,TH1F *>  histoTanhPar31DMap_;
  //std::map<std::string,TH1F *>  tanhFitStatistic1DMap_;
  std::map<std::string,TH1F *>  histoTanhChisquare1DMap_;

  std::map<std::string,TH2F *>  histoSlope2DMap_;
  std::map<std::string,TH2F *>  histoIntercept2DMap_;
  //std::map<std::string,TH2F *>  histoChisquare2DMap_;
  //std::map<std::string,TH2F *>  histoProbability2DMap_;
  //std::map<std::string,TH2F *>  linearFitStatistic2DMap_;
  //std::map<std::string,TH2F *>  histoTanhPar02DMap_;
  std::map<std::string,TH2F *>  histoTanhPar12DMap_;
//  std::map<std::string,TH2F *>  histoTanhPar22DMap_;
//  std::map<std::string,TH2F *>  histoTanhPar32DMap_;
  //std::map<std::string,TH2F *>  tanhFitStatistic2DMap_;

//	TH1F * chisquare_;
  TH1F * hADCOfAllPixels_; 

  TDirectory *dirErrorCells_, *dirGoodCells_;
  double   squareRootNumberOfTriggers_;

  double    	maxNumberOfHistos_, maxNumberOfGoodHistos_;
  double    	rocSlopeMean_;
  double    	rocInterceptMean_;
  double    	rocChisquareMean_;
  double    	rocProbabilityMean_;
  double    	tanhLinearityMean_;

  std::string fitFunctions_;
  double  	  linearFitFrom_;
  double  	  linearFitTo_;  
  bool    	  linearFit_;  
  double  	  tanhFitFrom_;
  double  	  tanhFitTo_;  
  bool    	  tanhFit_;  
  TF1     	 *line_;     
  TF1     	 *tanh_;     

  int              dumpGraphs_;
  std::ofstream         ascfile_;
};

#endif
