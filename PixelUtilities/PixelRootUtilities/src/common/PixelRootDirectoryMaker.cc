/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2008 Cornell.			                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd	                				 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/


#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include <TDirectory.h>
#include <TROOT.h>
#include <sstream>


using namespace pos;
using namespace std;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PixelRootDirectoryMaker::PixelRootDirectoryMaker(std::vector<std::string> names){
  directory_ = gROOT;
	init(names);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PixelRootDirectoryMaker::PixelRootDirectoryMaker(std::vector<std::string> names, TDirectory* directory){
  directory_ = directory;
  init(names);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PixelRootDirectoryMaker::PixelRootDirectoryMaker(std::vector<PixelROCName> names){
  directory_ = gROOT;
  std::vector<std::string> theNames;
  for(unsigned i=0;i<names.size();i++){
    theNames.push_back(names[i].rocname());
  }
  init(theNames);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PixelRootDirectoryMaker::PixelRootDirectoryMaker(std::vector<PixelROCName> names,TDirectory* directory){
  directory_ = directory;
  std::vector<std::string> theNames;
  for(unsigned i=0;i<names.size();i++){
    theNames.push_back(names[i].rocname());
  }
  init(theNames);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PixelRootDirectoryMaker::PixelRootDirectoryMaker(std::vector<std::pair<unsigned int,std::vector<unsigned int> > > fedsAndChannels){
  directory_ = gROOT;
  vector<string> namesVector;
  for(unsigned int i=0;i<fedsAndChannels.size();i++){
    stringstream name;
		name.str("");
		name << "FED" << fedsAndChannels[i].first << "_Channel";
    for(unsigned int j=0;j<fedsAndChannels[i].second.size();j++){
      stringstream fullName;
		  fullName.str("");
		  fullName << name.str() << fedsAndChannels[i].second[j]
			   <<"_Dummy";
      namesVector.push_back(fullName.str());
    }
  }
  init(namesVector);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PixelRootDirectoryMaker::PixelRootDirectoryMaker(std::vector<std::pair<unsigned int,std::vector<unsigned int> > > fedsAndChannels, TDirectory* directory){
  directory_ = directory;
  vector<string> namesVector;
  for(unsigned int i=0;i<fedsAndChannels.size();i++){
    stringstream name;
		name.str("");
		name << "FED" << fedsAndChannels[i].first << "_Channel";
    for(unsigned int j=0;j<fedsAndChannels[i].second.size();j++){
      stringstream fullName;
		  fullName.str("");
      fullName << name.str() << fedsAndChannels[i].second[j]
	       << "_Dummy";
      namesVector.push_back(fullName.str());
      //cout << "fullName:"<<fullName.str()<<endl;
    }
  }
  init(namesVector);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//added by yao
PixelRootDirectoryMaker::PixelRootDirectoryMaker(std::set<PixelChannel> names,TDirectory* directory){
  directory_ = directory;
  std::vector<std::string> theNames;
  for(std::set<PixelChannel>::iterator PixelChannel_itr=names.begin(), PixelChannel_itr_end = names.end();PixelChannel_itr!=PixelChannel_itr_end; ++PixelChannel_itr){
    theNames.push_back((*PixelChannel_itr).channelname());
  }
  init(theNames);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelRootDirectoryMaker::init(std::vector<std::string> names){

  //cout <<"[PixelRootDirectoryMaker::init] names.size()="<<names.size()<<endl;
  for(unsigned int ii=0;ii<names.size();ii++){
    std::string path=getPath(names[ii]);
    //cout << "[PixelRootDirectoryMaker::PixelRootDirectoryMaker] path=" << path << endl;
    unsigned int theDirLevels=dirLevels(path);
    //cout << "[PixelRootDirectoryMaker::PixelRootDirectoryMaker] theDirLevels="<< theDirLevels << endl;
    for(unsigned int i=1;i<=theDirLevels;i++){
      //cout << "[PixelRootDirectoryMaker::PixelRootDirectoryMaker] i=" << i << endl;
      std::string thePathN=pathN(i,path);
      //cout << "[PixelRootDirectoryMaker::PixelRootDirectoryMaker] thePathN=" << thePathN << endl;
      if (i==1){
	      TDirectory *obj=0;
        directory_->cd();
	      directory_->GetObject(thePathN.c_str(),obj);
	      if (obj==0){
	        //cout << "[PixelRootDirectoryMaker::PixelRootDirectoryMaker] " << "create i=1 " << thePathN << endl;
	        TDirectory *dir= gDirectory->mkdir(thePathN.c_str(),thePathN.c_str());
	        directorymap_[thePathN]=dir;
	      }
      }else {
 	      std::string pathNminusOne=pathN(i-1,path);
	      //cout << "[PixelRootDirectoryMaker::PixelRootDirectoryMaker] pathNminusOne"
	      //<< pathNminusOne<<endl;
        directory_->cd();
	      TDirectory *dir=getDirectory(pathNminusOne.c_str());
	      if (dir==0){
	  			cout << "[PixelRootDirectoryMaker::PixelRootDirectoryMaker] "
	  			     << "could not find directory:"<<pathNminusOne

	  			     << " in memory. This is likely because the root directory"
	  			     << " structure was not cleaned from a previous run."<<endl;
	        assert(0);
				}
				std::string thePathNth=pathNth(i,path);
				TDirectory *obj=0;
				dir->GetObject(thePathNth.c_str(),obj);
				if (obj==0){
				  directory_->cd(pathNminusOne.c_str());
				  std::string dirName=pathNth(i,path);
				  TDirectory *dir= gDirectory->mkdir(dirName.c_str(),dirName.c_str());
				  directorymap_[thePathN]=dir;
				}			            
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TDirectory* PixelRootDirectoryMaker::getDirectory(std::string directory){
  std::map<std::string,TDirectory *>::iterator mapIter=directorymap_.find(directory);
  if (mapIter != directorymap_.end()){
	  return mapIter->second;
  }
  else{
  	return 0;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelRootDirectoryMaker::cdDirectory(const std::string names, TDirectory *dir){
  if(dir == 0){
		dir = directory_;
	}
	if (!dir->cd(getPath(names).c_str())) {
    cout << "[PixelRootDirectoryMaker::cdDirectory] could not cd to:" << getPath(names) << endl;
    assert(0);
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelRootDirectoryMaker::cdDirectory(const PixelROCName& aROC, TDirectory *dir){
  if(dir == 0){
		dir = directory_;
	}
  if (!dir->cd(getPath(aROC.rocname()).c_str())) {
    cout << "[PixelRootDirectoryMaker::cdDirectory] could not cd to:" << getPath(aROC.rocname()) << endl;
    assert(0);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelRootDirectoryMaker::cdDirectory(const PixelChannel& aCha, TDirectory *dir){
  if(dir == 0){
		dir = directory_;
	}
  if (!dir->cd(getPath(aCha.channelname()).c_str())) {
    cout << "[PixelRootDirectoryMaker::cdDirectory] could not cd to:" << getPath(aCha.channelname()) << endl;
    assert(0);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelRootDirectoryMaker::cdDirectory(unsigned int fedNumber, unsigned int channel, TDirectory *dir){
  if(dir == 0){
		dir = directory_;
	}
  stringstream dirName;
	dirName.str("");
	dirName << "FED" << fedNumber << "/FED"<<fedNumber
		<<"_Channel" << channel;
  if (!dir->cd(dirName.str().c_str())) {
    cout << "[PixelRootDirectoryMaker::cdDirectory(int,int)] could not cd to:" << dirName.str() << endl;
    assert(0);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string PixelRootDirectoryMaker::getPath(std::string name){
  //This method gives back the new path assuming that we are looking for a path related to the ROC naming scheme
	//removing the last ROC part.
	//FPix_BpO_D1_BLD9_PNL2_PLQ3_ROC2 will become
	//FPix/FPix_BpO/FPix_BpO_D1/FPix_BpO_D1_BLD9/FPix_BpO_D1_BLD9_PNL2/FPix_BpO_D1_BLD9_PNL2_PLQ3
	string path = "";
	for (unsigned int i=0;i<name.size();i++){
    if (name[i]=='_') {
			path += name.substr(0,i)+"/";
    }
  }
	return path.substr(0,path.length()-1);

//ORIGINAL IMPLEMENTATION
//	for (unsigned int i=0;i<name.size();i++){
//    if (name[i]=='_') {
//      name[i]='/';
//    }
//  }
//  return name;
}
    
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int PixelRootDirectoryMaker::dirLevels(std::string path){
  unsigned int count=1;
  for (unsigned int i=0;i<path.size();i++){
    if (path[i]=='/') {
      count++;
    }
  }
  return count;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string PixelRootDirectoryMaker::pathN(unsigned int N, std::string path){
  unsigned int count=0;
  for(unsigned int i=0;i<path.size();i++){
    if (path[i]=='/') {
      count++;
      if (count==N) {
	      return path.substr(0,i);
      }
    }
  }
  if (count==N-1){
	  return path;
	}
  cout << "[PixelRootDirectoryMaker::pathN] reached end of string" << endl;
  return "";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string PixelRootDirectoryMaker::pathNth(unsigned int N, std::string path){
  std::string thePathN=pathN(N,path);
  unsigned int last=thePathN.find_last_of('/');
  if (last== (unsigned int )std::string::npos){
	  last=0;
	}
  return thePathN.substr(last+1);

}
