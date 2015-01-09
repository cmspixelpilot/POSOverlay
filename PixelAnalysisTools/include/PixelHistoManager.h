#ifndef _PIXELHISTOMANAGER_
#define _PIXELHISTOMANAGER_

#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"

class TH1;
class PixelXmlReader;
class PixelConfigurationsManager;
class PixelCalibConfigurationExtended;
class PixelRootDirectoryMaker;
class TTree;
class TDirectory;

class PixelHistoManager{
 public:
  PixelHistoManager(PixelXmlReader* xmlReader, PixelCalibConfigurationExtended *calib, PixelConfigurationsManager * configurationsManager, std::ostream *logger=&std::cout);
  virtual ~PixelHistoManager();
  virtual void bookHistos 				 (void) = 0;
  virtual void saveHistos 				 (void);
  virtual void init       				 (void);
  virtual void destroy    				 (void);
  virtual void endOfPatternAnalysis(void){;}
  virtual void endOfFileAnalysis   (void){;}
  virtual void reset               (void);
  virtual void deleteServiceHistos (void);
	virtual void setRunNumber        (unsigned int runNumber){currentRunNumber_ = runNumber;}
  virtual void makeSummaryPlots    (void) = 0;
  virtual void makeSummary         (std::string summary);
  virtual void setPatternHistos    (unsigned int eventNumber);
  virtual void fillHistos          (unsigned int fed, unsigned int channel, unsigned int roc, unsigned int row, unsigned int col, unsigned int vcalvalue, unsigned int adc){;}
  virtual void fillHistos          (unsigned int fed, unsigned int channel, unsigned int roc, unsigned int row, unsigned int col){;}
  virtual void mirrorHisto         (TH1 * histo);
  virtual void drawHisto           (unsigned int fed, unsigned int channel, unsigned int roc, std::string summary, std::string panelType, int plaquette, TH1* &summaryH)=0;
  virtual void setWrongAddress     (unsigned int fed, unsigned int channel, unsigned int roc);

 protected:
  virtual void initializeSummaries(void);
  int setNumberOfLabels(int &nLabels, int nBins);
  //map[fed][channel][roc]= service TH1*
  std::map<unsigned int, std::map<unsigned int, std::map<unsigned int , std::vector<TH1 *> > > >  histoMap_;
  //map[rocName]= service TH1*
  std::map<std::string , std::vector<TH1 *> * >                                                   rocNameHistoMap_;
  //map[fed][channel][roc]=roc name string
  std::map<unsigned int, std::map<unsigned int, std::map<unsigned int , std::string > > >         rocNameMap_;
  //map[fed][channel][roc]=Number of wrong addresses
  std::map<unsigned int, std::map<unsigned int, std::map<unsigned int , unsigned int > > >        wrongAddressMap_;
  //map[panelType][roc]=(canvasframe,invert)
	std::map<std::string, std::map<unsigned int, std::pair<unsigned int,bool> > >                   rocCanvasMap_;
  //map[panelType]=(rows,columns)
	std::map<std::string, std::map<unsigned int, std::pair<unsigned int,unsigned int> > >           moduleTypeMap_;
  //map[row][col]=position in the service vector
	std::map<std::pair<int,int>,unsigned int> cellMap_;
  PixelCalibConfigurationExtended          *thePixelCalib_;
  std::vector<pos::PixelROCName>            rocList_;
	PixelRootDirectoryMaker                  *thePixelRootDirectoryMaker_;
	PixelRootDirectoryMaker                  *thePixelRootDirectoryMakerFED_;
  std::vector<TH1*>                         theHistoList_;
  PixelXmlReader                           *thePixelXmlReader_;
  std::ostream  													 *logger_;
  unsigned int  													  numberOfTriggers_;
  unsigned int  													  numberOfHistosPerRoc_;
  unsigned int  													  currentRunNumber_;
  TTree                                    *summaryTree_;
	TDirectory  														 *summaryTreeDir_;
	TDirectory  														 *summaryDir_;
	PixelConfigurationsManager               *thePixelConfigurationsManager_;
 private:
  void setRocCanvasMap (void);
	void setModuleTypeMap(void);
};

#endif
