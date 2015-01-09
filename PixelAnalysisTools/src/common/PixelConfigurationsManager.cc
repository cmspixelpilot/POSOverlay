#include "PixelAnalysisTools/include/PixelConfigurationsManager.h"
#include "PixelUtilities/PixelXmlUtilities/include/PixelXmlReader.h"
#include "PixelAnalysisTools/include/PixelCalibConfigurationExtended.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"
#include <string>
#include <sstream>
#include <vector>

using namespace std;
using namespace pos;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PixelConfigurationsManager::PixelConfigurationsManager(PixelXmlReader * xmlReader, PixelCalibConfigurationExtended *pixelCalibConfiguration, ostream *logger, int runNumber){
  string mthn = "[PixelConfigurationsManager::PixelConfigurationsManager()]\t";
  runNumber_=runNumber;
  logger_ = logger;
  chooseWBC_ = xmlReader->getXMLAttribute("WhichWBC","ChooseWBC");
	thePixelCalibConfiguration_ = pixelCalibConfiguration;
	runFilesDir_ = xmlReader->getXMLAttribute("RootOutputFile","Directory");
	if(runFilesDir_ == "Default"){
	  runFilesDir_ = xmlReader->getXMLAttribute("DefaultDirectory","Directory");
  	int groupDirectory = ((int)runNumber_/1000)*1000;
	  stringstream tmpDir; tmpDir.str("");
	  tmpDir << runFilesDir_ << "/Run_" << groupDirectory <<  "/Run_" << runNumber << "/";
	  runFilesDir_ = tmpDir.str();
	}
  xmlReader->convertEnvVariables(runFilesDir_);
  //------------------------------------------------------------------------------------------------
  // Building the configuration to analyze
  int    listCharacterPosition = 0;
  map<unsigned int,bool> fedsToAnalyze;
  string listOfFeds= xmlReader->getXMLAttribute("Feds","Analyze");
  while( (listCharacterPosition = listOfFeds.find("-")) >= 0){
    fedsToAnalyze[atoi(listOfFeds.substr(0,listCharacterPosition).c_str())] = true;
    listOfFeds=listOfFeds.substr(listCharacterPosition+1,listOfFeds.size()-listCharacterPosition);
    //cout << mthn << listOfFeds << endl;
  }
  fedsToAnalyze[atoi(listOfFeds.substr(0,listCharacterPosition).c_str())] = true;
  unsigned int numberOfConfiguredFeds = xmlReader->getNumberOfBranches("Fed");
  
  if(fedsToAnalyze.size() > numberOfConfiguredFeds){
    *logger_ << mthn << "Wrong configuration. There are more Feds to analyze than the ones that are configured!" << endl;
    exit(0);
  }
  
  for(unsigned int i=0; i<numberOfConfiguredFeds; ++i){
    unsigned int fed = atoi(xmlReader->getXMLAttribute("Fed","Id",i).c_str());
    if(fedsToAnalyze.find(fed) != fedsToAnalyze.end()){
			string fileName = xmlReader->getXMLAttribute("Fed","DataFileName","FileName",i);
			if(fileName == "Default"){
        if(runNumber == -1){
					cout << mthn << endl
					     << "You are using the default keyword for fed " << fed << " filename but when you are running the PixelAnalysis.exe you are not providing the run number!" << endl
					     << "To fix this you have 2 ways:" << endl
							 << "1) Write the right file name in the DataFileName for fed " << fed << endl
							 << "2) Run the executable with a run number PixelAnalysis.exe TestType RunNumber ( ex PixelAnalysis.exe PixelAlive 1984 )" << endl;
							 exit(0);
				}
				string fileTypeName = xmlReader->getXMLAttribute("Calibration","Type");
				if(fileTypeName == "Gain"){
					fileTypeName += "Calibration";
				}
				stringstream file; 
				file.str("");
				file << xmlReader->getXMLAttribute("DefaultDirectory","Directory");
		    if(file.str()[file.str().length()-1] != '/'){
			    file << "/";
		    }
				int groupDirectory = ((int)runNumber/1000)*1000;
				file << "Run_" << groupDirectory << "/Run_" << runNumber << "/" << fileTypeName << "_" << fed << "_" << runNumber << ".dmp";
				fileName = file.str();
			}
			xmlReader->convertEnvVariables(fileName);
			dataFileMap_.insert(pair<string,unsigned int>(fileName,fed));
//       cout << mthn << "Found fed: " << fed << endl;
      int fromChannel = atoi(xmlReader->getXMLAttribute("Fed","Channels","From",i).c_str());
      int toChannel   = atoi(xmlReader->getXMLAttribute("Fed","Channels","To",i).c_str());
      //      if( from < 1 || >36)
      map<unsigned int, bool> channelsToExclude;
      string listOfChannelsToExclude= xmlReader->getXMLAttribute("Fed","Channels","Exclude",i);
      while( (listCharacterPosition = listOfChannelsToExclude.find("-")) >= 0){
				channelsToExclude[atoi(listOfChannelsToExclude.substr(0,listCharacterPosition).c_str())] = true;
				listOfChannelsToExclude=listOfChannelsToExclude.substr(listCharacterPosition+1,listOfChannelsToExclude.size()-listCharacterPosition);
      }
      if(listOfChannelsToExclude.size() != 0){
	      channelsToExclude[atoi(listOfChannelsToExclude.substr(0,listCharacterPosition).c_str())] = true;
      }
      for(int channel=fromChannel; channel<=toChannel; ++channel){
        if(channelsToExclude.find(channel) == channelsToExclude.end()){
//  	     cout << mthn << "Channel " << channel << " not excluded!" << endl; 
	  			//From roc to roc should be taken from the fedconfiguration file but since this file has never
	  			//been considered we are using a trick here that works only if the configuration is
	  			// the standard one that is a 4type panel on channels 1-3-5 and a 3type panel on channel 2-4-6
	  			int fromRoc = 0;
	  			int toRoc;
	  			if(channel%2==0){
	  			  //3type panel which sits typically on a even channel
	  			  toRoc=23;
	  			}else{
	  			  //4type panel which sits typically on a even channel
	  			  toRoc=20;
	  			}
	  			//      if( from < 1 || >36)
	  			map<unsigned int, bool> rocsToExclude;
	  			string listOfRocsToExclude= xmlReader->getXMLAttribute("Fed","Rocs","Exclude",i);
// 				  cout << mthn << "List of Rocs to exclude: " << listOfRocsToExclude << endl;
	  			while( (listCharacterPosition = listOfRocsToExclude.find("-")) >= 0){
	  			  rocsToExclude[atoi(listOfRocsToExclude.substr(0,listCharacterPosition).c_str())] = true;
	  			  listOfRocsToExclude=listOfRocsToExclude.substr(listCharacterPosition+1,listOfRocsToExclude.size()-listCharacterPosition);
	  			  //     cout << mthn << listOfFeds << endl;
	  			}
	  			if(rocsToExclude.size() != 0){
	  			  rocsToExclude[atoi(listOfRocsToExclude.substr(0,listCharacterPosition).c_str())] = true;
	  			}
	  			for(int roc=fromRoc; roc<=toRoc; ++roc){
	  			  if(rocsToExclude.find(roc) == rocsToExclude.end()){
	  			    configurationToAnalyze_[fed][channel][roc] = true;
// 				      cout <<"f: " << fed <<" c: " << channel << " r: " << roc << endl;    
	  			  }	  
	  			}
	      }
      }
    }
  }
  //------------------------------------------------------------------------------------------------
  // Building the expected configuration
  
  vector<PixelROCName> rocList = pixelCalibConfiguration->rocList();
  for (vector<PixelROCName>::iterator it=rocList.begin();it!=rocList.end();it++){
    string rocName =  it->rocname();
    //    cout << mthn << rocName << endl;
    int fed     = pixelCalibConfiguration->getPixelNameTranslation()->getHdwAddress(*it)->fednumber();//+32;//MUST BE REMOVED
    int channel = pixelCalibConfiguration->getPixelNameTranslation()->getHdwAddress(*it)->fedchannel();
    int roc     = pixelCalibConfiguration->getPixelNameTranslation()->getHdwAddress(*it)->fedrocnumber();
    expectedConfiguration_[fed][channel][roc] = true;
		rocNameMap_[fed][channel][roc] = rocName;
		if(isDataToAnalyze(fed,channel,roc)){
		  nameConfigurationToAnalyze_[rocName] = true;
		}else{
		  nameConfigurationToAnalyze_[rocName] = false;
		}
  }

  for(multimap<string,unsigned int>::iterator it=dataFileMap_.begin(); it!=dataFileMap_.end(); ++it){
    ifstream dataFile;
    dataFile.open(it->first.c_str(),ios::binary|ios::in);
    if(!dataFile.is_open()){
      cout << mthn << "Can't open file: " << it->first << "...Check the file name for fed " << it->second << endl;
      exit(0);
    }   
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PixelConfigurationsManager::~PixelConfigurationsManager(){
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int PixelConfigurationsManager::getWBCChoice(){
  return atoi((chooseWBC_).c_str());
}
///////////////////////////////////////////////////////////////////////////////
bool PixelConfigurationsManager::dataIsExpected(unsigned int fed,unsigned int channel, unsigned int roc){
//  if(!expectedConfiguration_[fed][channel][roc]){
    map<int, map<int, map< int, bool> > >::iterator fedIt = expectedConfiguration_.find(fed);
    if(fedIt != expectedConfiguration_.end()){
      map<int, map< int, bool> >::iterator channelIt = fedIt->second.find(channel);
      if(channelIt != fedIt->second.end()){
        map< int, bool>::iterator rocIt = channelIt->second.find(roc);
        if(rocIt != channelIt->second.end()){
//          *logger_ << "I should always get in here!" << endl;
          return true;
        }
        else{
          *logger_ << "[PixelConfigurationsManager::dataIsExpected()]\tFor fed=" << fed << " and channel=" << channel << " the roc=" << roc << " doesn't exist in the configuration!" << endl;
        }
      }
      else{
        *logger_ << "[PixelConfigurationsManager::dataIsExpected()]\tFor fed=" << fed << " the channel=" << channel << " doesn't exist in the configuration!" << endl;
      }
    }
    else{
      *logger_ << "[PixelConfigurationsManager::dataIsExpected()]\tFed=" << fed << " doesn't exist in the configuration!" << endl;
    }
    return false;
//  }
//  return true;
}


int PixelConfigurationsManager::getLevel(unsigned int clock,unsigned row,unsigned col){

  int pixid=2*(80-row)+col%2;
  int dcol=col/2;

  if (clock==1) return dcol/6;
  if (clock==2) return dcol%6;
  if (clock==3) return pixid/36;
  if (clock==4) return (pixid%36)/6;
  if (clock==5) return pixid%6;

  return -1; 

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PixelConfigurationsManager::patternIsExpected(unsigned int fed, unsigned int channel, unsigned int roc, unsigned int row, unsigned int col){

  bool itIsNotOk = true;
  for (vector<unsigned int>::const_iterator itr=rows_->begin(); itr!=rows_->end() && itIsNotOk;itr++){
    for (vector<unsigned int>::const_iterator itc=cols_->begin(); itc!=cols_->end() && itIsNotOk;itc++){
      if(*itr == row && *itc == col){itIsNotOk=false;}
    }
  }
  if(itIsNotOk){
    *logger_ << "[PixelConfigurationsManager::patternIsExpected()]\tInvalid pixel injected. Expecting";
    for (vector<unsigned int>::const_iterator itr=rows_->begin(); itr!=rows_->end() && itIsNotOk;itr++){
      for (vector<unsigned int>::const_iterator itc=cols_->begin(); itc!=cols_->end() && itIsNotOk;itc++){
	*logger_ << " (" << *itr << "," << *itc << ")["<<getLevel(1,*itr,*itc)<<getLevel(2,*itr,*itc)
		 <<getLevel(3,*itr,*itc)<<getLevel(4,*itr,*itc)<<getLevel(5,*itr,*itc)<<"]";
      }
    }
    *logger_ << " got (" << row << "," << col << ")["<<getLevel(1,row,col)<<getLevel(2,row,col)
	     << getLevel(3,row,col)<<getLevel(4,row,col)<<getLevel(5,row,col)<<"]"
	     << " for roc " << rocNameMap_[fed][channel][roc] << endl; 
    return false;
  } 
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelConfigurationsManager::setPatternToExpect(unsigned int eventNumber){
	thePixelCalibConfiguration_->getRowsAndCols(eventNumber/thePixelCalibConfiguration_->nTriggersPerPattern(),rows_,cols_);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PixelConfigurationsManager::isAnAllowedPixel(unsigned int row,unsigned int col){
  if(row>= 80 || col >=52){
    *logger_ << "[PixelConfigurationsManager::isAnAllowedPixel()]\tInvalid pixel with (row,col)=(" << row << "," << col << ")" << endl;
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PixelConfigurationsManager::isDataToAnalyze(unsigned int fed,unsigned int channel, unsigned int roc){
  if(isChannelToAnalyze(fed,channel)){
    if( configurationToAnalyze_[fed][channel].find(roc) != configurationToAnalyze_[fed][channel].end()){
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PixelConfigurationsManager::isDataToAnalyze(string rocName){
	return nameConfigurationToAnalyze_[rocName];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PixelConfigurationsManager::isChannelToAnalyze(unsigned int fed,unsigned int channel){
  map<int, map<int, map< int, bool> > >::iterator itFed;
  if( (itFed=configurationToAnalyze_.find(fed)) != configurationToAnalyze_.end()){
    map<int, map< int, bool> > ::iterator itChannel;
    if((itChannel=itFed->second.find(channel)) != itFed->second.end()){
      return true;
    }
  }
  return false;
}
