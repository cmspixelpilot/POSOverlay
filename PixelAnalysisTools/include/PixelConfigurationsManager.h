#ifndef _PixelConfigurationsManager_h_
#define _PixelConfigurationsManager_h_

#include <map>
#include <vector>
#include <iostream>
#include <string>

class PixelXmlReader;
class PixelCalibConfigurationExtended;

typedef  std::map<int, std::map<int, std::map< int, bool> > > Configuration;

class PixelConfigurationsManager{
 public:
  PixelConfigurationsManager(PixelXmlReader * xmlReader, PixelCalibConfigurationExtended *pixelCalibConfiguration, std::ostream *logger=&std::cout, int runNumber = -1);
  ~PixelConfigurationsManager();
  int getWBCChoice();
  bool dataIsExpected    (unsigned int fed,unsigned int channel, unsigned int roc);
  bool patternIsExpected (unsigned int fed, unsigned int channel, unsigned int roc, unsigned int row,unsigned int col);
  void setPatternToExpect(unsigned int eventNumber);
  bool isAnAllowedPixel  (unsigned int row,unsigned int col);
  bool isDataToAnalyze   (unsigned int fed,unsigned int channel, unsigned int roc);
  bool isDataToAnalyze   (std::string rocName);
  bool isChannelToAnalyze(unsigned int fed,unsigned int channel);
  std::multimap<std::string,unsigned int> getDataFileMap(void){return dataFileMap_;}
  int runNumber() {return runNumber_;}
  std::string getRunFilesDir() {return runFilesDir_;}
  int getLevel(unsigned int clock,unsigned row,unsigned col);
 private:
  //map[fed][channel][roc]=roc name string
  std::map<unsigned int, std::map<unsigned int, std::map<unsigned int , std::string > > >         rocNameMap_;
  PixelCalibConfigurationExtended         *thePixelCalibConfiguration_;
	Configuration                            expectedConfiguration_;
  Configuration                            configurationToAnalyze_;
  std::vector<unsigned int> const         *rows_;
  std::vector<unsigned int> const         *cols_;
	std::map<std::string,bool>               nameConfigurationToAnalyze_;
  std::vector<std::pair<int,int> >         currentPattern_;
  std::multimap<std::string,unsigned int>  dataFileMap_;
  std::ostream                            *logger_;
  int                                      runNumber_;
  std::string                              runFilesDir_;
  std::string                              chooseWBC_; 
};

#endif
