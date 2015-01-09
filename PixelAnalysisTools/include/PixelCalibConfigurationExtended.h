#ifndef _PixelCalibConfigurationExtended_h_
#define _PixelCalibConfigurationExtended_h_


#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelNameTranslation.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDetectorConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

class PixelCalibConfigurationExtended : public pos::PixelCalibConfiguration{
  public:
  
  PixelCalibConfigurationExtended(std::string, pos::PixelNameTranslation*, pos::PixelDetectorConfig*);
  virtual ~PixelCalibConfigurationExtended(){;}
  
  std::vector<std::pair<std::vector<unsigned int>, std::string> >getFedChannelHWandTypeInfo(){return fedChannelHWandTypeInfo_;}
  void getRowsAndCols(unsigned int state, const std::vector<unsigned int> *&rows, const std::vector<unsigned int> *&cols) const;
  
  unsigned int getNumberOfFeds(){
     return 1; 
  };
  std::string getPanelType(unsigned int fed, unsigned int channel){
		return panelTypeMap_[fed][channel];
	}
  pos::PixelNameTranslation* getPixelNameTranslation(void){return thePixelNameTranslation_;}
  unsigned int getNumberOfPixelInjected();
  private:
  
  void fillFedChannelHWandTypeInfo();
  std::vector<pos::PixelROCName> rocList_;
  std::vector<std::pair<std::vector<unsigned int>, std::string> > fedChannelHWandTypeInfo_;
  //map[fed][channel]=panelType
	std::map<unsigned int, std::map< unsigned int, std::string > >  panelTypeMap_;
  pos::PixelNameTranslation *thePixelNameTranslation_;
};
 
#endif
