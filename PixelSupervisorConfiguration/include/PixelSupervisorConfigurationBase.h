#ifndef _PixelSupervisorConfigurationBase_h_
#define _PixelSupervisorConfigurationBase_h_

#include<iostream>
#include <assert.h>

class PixelSupervisorConfigurationBase{

 public:
  
  PixelSupervisorConfigurationBase(std::string* runNumber, std::string* outputDir);
  PixelSupervisorConfigurationBase( const PixelSupervisorConfigurationBase & );
  PixelSupervisorConfigurationBase();

  ~PixelSupervisorConfigurationBase(){};

  void setupOutputDir();
  std::string outputDir();
  std::string runDir();

  std::string datDir() {return datDir_;}

  std::string itostring (int value);
	

 private:
  
  std::string* outputDir_;
  std::string* runNumber_;
  std::string* outputSubDir_;
  std::string datDir_;

};
#endif 
