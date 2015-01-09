#ifndef _PixelSCurveHistoManager_h_
#define _PixelSCurveHistoManager_h_

#include "PixelAnalysisTools/include/PixelHistoManager.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>

class PixelXmlReader;
class PixelCalibConfigurationExtended;
class PixelConfigurationsManager;
class TDirectory;
class TH1F;
class TF1;
class TH2F;

class PixelSCurveHistoManager : public PixelHistoManager{
 public:
  PixelSCurveHistoManager(PixelXmlReader* xmlReader, PixelCalibConfigurationExtended *calib, PixelConfigurationsManager * configurationsManager, std::ostream *logger=&std::cout);
  ~PixelSCurveHistoManager();
  void init();
  void bookHistos          (void);
  void destroy             (void);
  void drawHisto           (unsigned int fed, unsigned int channel, unsigned int roc, std::string summary, std::string panelType, int plaquette, TH1* &summaryH);  
	void makeSummaryPlots    (void);
  void endOfPatternAnalysis(void);
  void endOfFileAnalysis   (void);
  void fillHistos          (unsigned int fed,unsigned int channel, unsigned int roc, unsigned int row, unsigned int col, unsigned int vcalvalue, unsigned int adc);
  void fit                 (void);

 private:

  std::map<std::string,TH1F *>  histoNoise1DMap_;
  std::map<std::string,TH1F *>  histoThreshold1DMap_;
  std::map<std::string,TH1F *>  histoChisquare1DMap_;
  std::map<std::string,TH1F *>  histoProbability1DMap_;
  std::map<std::string,TH2F *>  histoNoise2DMap_;
  std::map<std::string,TH2F *>  histoThreshold2DMap_;
  std::map<std::string,TH2F *>  histoChisquare2DMap_;
  std::map<std::string,TH2F *>  histoProbability2DMap_;
  static double fitfcn(double *x, double *par);
  TDirectory * dirNoisyCells_, * dirErrorCells_, * dirGoodFits_;

	TF1       		*fitFunction_;
	int       		 fitAttempts_;      
	double    		 startCurve_;       
	double    		 endCurve_;         
	double    		 maxNumberOfHistos_;
	double    		 noisyPixel_;       
	double    		 rocNoiseMean_;
	double    		 rocThresholdMean_;    
	double    		 rocChisquareMean_;
	double    		 rocProbabilityMean_;
	bool      		 writeTrimOutputFile_;
	bool      		 saveGoodFits_;
	std::ofstream *trimOutputFile_;
};

#endif
