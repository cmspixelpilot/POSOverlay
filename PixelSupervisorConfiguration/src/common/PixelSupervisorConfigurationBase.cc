#include "PixelSupervisorConfigurationBase.h"
#include "sys/stat.h"
#include <cstdlib>
#include <stdio.h>

using namespace std;

PixelSupervisorConfigurationBase::PixelSupervisorConfigurationBase(std::string* runNumber, 
								   std::string* outputDir)
{
 
  runNumber_=runNumber;
  outputDir_=outputDir;
  outputSubDir_=outputDir;

  if (getenv("BUILD_HOME")==0) {
    string xdaqroot = string(getenv("XDAQ_ROOT"));
    datDir_=xdaqroot+"/dat/PixelSupervisor/dat/";
  }
  else {
    string buildhome = string(getenv("BUILD_HOME"));
    datDir_=buildhome+"/pixel/PixelSupervisor/dat/";
  }

}

PixelSupervisorConfigurationBase::PixelSupervisorConfigurationBase(const PixelSupervisorConfigurationBase & tempConfigurationBase)
{
  
   runNumber_=tempConfigurationBase.runNumber_;
   outputDir_=tempConfigurationBase.outputDir_;
   datDir_=tempConfigurationBase.datDir_;
   outputSubDir_=tempConfigurationBase.outputSubDir_;
}

PixelSupervisorConfigurationBase::PixelSupervisorConfigurationBase(){}

void PixelSupervisorConfigurationBase::setupOutputDir(){

  int runnumber=atoi((*runNumber_).c_str());
  std::string runnumber_string;

  char * basedirPointer = getenv("POS_OUTPUT_DIRS");
  if ( basedirPointer == 0 )
  {
    cout << "ERROR: Environment variable POS_OUTPUT_DIRS is not defined.  It must be defined to run.  Now exiting..." << endl;
    exit(1);
  }
  std::string basedir=basedirPointer;

  struct stat stbuf;
  if (stat(basedir.c_str(),&stbuf)!=0){
    cout << "[PixelSupervisorConfiguration::setupOutputDir] basedir="
	 << basedir.c_str() << " does not exist."<<endl;
    assert(0);
  }


  //FIXME this code will fail for run numbers above 1,000,000
  for(int i=0; i<1000000; i=i+1000){
   

    if(runnumber<=(i+999) && runnumber>=i){

      runnumber_string=itostring(i);
      *outputSubDir_=basedir+"/Run_"+runnumber_string;
      
      if (stat(outputSubDir_->c_str(),&stbuf)!=0){
	cout << "[PixelSupervisorConfiguration::setupOutputDir] outputSubDir="
	     << outputSubDir_->c_str() << " does not exist. Will create"<<endl;
	mkdir(outputSubDir_->c_str(),0777);
      }
    }
  }

   
  
   *outputDir_=*outputSubDir_+"/Run_"+*runNumber_;

   // *outputDir_=basedir+"/Run_"+*runNumber_;

  
  if (stat(outputDir_->c_str(),&stbuf)!=0){
    cout << "[PixelSupervisorConfiguration::setupOutputDir] outputDir_="
	 << outputDir_->c_str() << " does not exist. Will create"<<endl;
    mkdir(outputDir_->c_str(),0777);
  }

}    

std::string PixelSupervisorConfigurationBase::runDir(){
  
  std::string runDir="Run_"+*runNumber_;

  return runDir;

}


std::string PixelSupervisorConfigurationBase::outputDir(){

  //cout << "[PixelSupervisorConfiguration::outputDir] outputDir_="
  //     << *outputDir_ << endl;

  return *outputDir_;

}

std::string PixelSupervisorConfigurationBase::itostring (int value)
{
  char buffer[50];
  sprintf(buffer,"%d",value);
  std::string str(buffer);
  return str;
}
