#include "PixelUtilities/PixelRootUtilities/include/PixelHistoScanDirectory.h"
#include "PixelUtilities/PixelXmlUtilities/include/PixelXmlReader.h"

#include <iostream>
#include <dirent.h>
#include <errno.h>
#include <algorithm>

using namespace std;

////////////////////////////////////////////////////////////////////
PixelHistoScanDirectory::PixelHistoScanDirectory(){
  string mthn = "[PixelHistoScanDirectory::PixelHistoScanDirectory()]\t"; 
  PixelXmlReader thePixelXmlReader;
  //------------------------------------------------------------------------------------------------
  //Getting main directory environment variable and building path to home
  if( getenv("BUILD_HOME") == 0 ){
    cout << mthn << "BUILD_HOME environment variable undefined...You need to set this variable!" << endl;
    exit(0);
  }
  string projectHome = string(getenv("BUILD_HOME"))+"/pixel/PixelHistoViewer/";
  string document = projectHome+"Configuration/SearchPaths.xml";

  thePixelXmlReader.readDocument(document);
  unsigned int numberOfDirectories = thePixelXmlReader.getNumberOfBranches("Path");
	for(unsigned int i=0; i<numberOfDirectories; ++i){
    string subDirAttribute = thePixelXmlReader.getXMLAttribute("Path","ScanSubdirectories",i);
    bool   scanSubDir = true;
		if(subDirAttribute == "No"){
			scanSubDir = false;
		}
		string dir = thePixelXmlReader.getXMLAttribute("Path","Directory",i);
		if( '/' != dir[dir.length()-1] ){
			dir += "/";
		}
		thePixelXmlReader.convertEnvVariables(dir);
		dirsToScan_.push_back(make_pair(dir,scanSubDir));
  }
}

////////////////////////////////////////////////////////////////////
PixelHistoScanDirectory::~PixelHistoScanDirectory(){
}

////////////////////////////////////////////////////////////////////
vector<string> PixelHistoScanDirectory::ls(string dir,string filter){
  string mthn = "[PixelHistoScanDirectory::ls(string)]\t";
	vector<string> list;
  struct dirent *pEntries;
	DIR *pDir=opendir(dir.c_str());
	if (!pDir){
		cout << mthn 
		     << "The directory " 
				 << dir 
		     << " doesn't exist or you don't have the permission to read it!" << endl;
	}
	else{
		errno=0;																			       
		while ((pEntries=readdir(pDir))){							       
			string tmpName = pEntries->d_name;
			if(filter != "*" && (tmpName.find(filter)==string::npos)){
				continue;
			}
			list.push_back(dir+pEntries->d_name);	       
		} 																				       
		closedir(pDir);
		if (errno){ 															       
	  	cout << mthn << "Readdir() failure; terminating" << endl;     
		}																			       
  }
	return list;
}

////////////////////////////////////////////////////////////////////
void PixelHistoScanDirectory::ls(vector<string> & list, string dir, string filter){
  string mthn = "[PixelHistoScanDirectory::ls(vector)]\t";
  struct dirent *pEntries;
	DIR *pDir=opendir(dir.c_str());
	if (!pDir){
		cout << mthn 
		     << "The directory " 
				 << dir 
		     << " doesn't exist or you don't have the permission to read it!" << endl;
		return;
	}
	errno=0;																			        						
	while ((pEntries=readdir(pDir))){							        					
		string tmpName = pEntries->d_name; 														
		if(filter != "*" && (tmpName.find(filter)==string::npos)){			
			continue; 																										
		} 																															
		list.push_back(dir+pEntries->d_name);	        								
	} 																				        								
	closedir(pDir);																									
	if (errno){ 															        								
		cout << mthn << "Readdir() failure; terminating" << endl;       
	}																			        										
}

////////////////////////////////////////////////////////////////////
void PixelHistoScanDirectory::ls(vector<string> & list, string dir, bool recursive, string filter){
  string mthn = "[PixelHistoScanDirectory::ls(vector,bool)]\t";
  struct dirent *pEntries;
	DIR *pDir=opendir(dir.c_str());
	if (!pDir){
		cout << mthn 
		     << "The directory " 
				 << dir 
		     << " doesn't exist or you don't have the permission to read it!" << endl;
		return;
	}
	errno=0;																			       
	while ((pEntries=readdir(pDir))){							       
		string tmpName = pEntries->d_name;
	  if(pEntries->d_type == DT_DIR){
			if(tmpName != "lost+found" && tmpName != "." && tmpName != ".." && tmpName != "CVS"){
				if(recursive){
  				ls(list,dir+tmpName+"/",recursive,filter);
				}
				else{
  		  	list.push_back(dir+tmpName+"/");				
				}
			}
		}
		else{
			if(filter != "*" && (tmpName.find(filter)==string::npos)){
		  	continue;
	  	}
	  	list.push_back(dir+tmpName);
  	}
	} 																				       
	closedir(pDir);
	if (errno){ 															       
		cout << mthn << "Readdir() failure; terminating" << endl;     
	}																			       
}

////////////////////////////////////////////////////////////////////
vector<string> PixelHistoScanDirectory::ls(bool overrideRecursive, bool recursive){
  string mthn = "[PixelHistoScanDirectory::ls(bool,bool)]\t";
  vector<string> fileNames;
	if(overrideRecursive){
    for(vector<pair<string,bool> >::iterator it=dirsToScan_.begin(); it != dirsToScan_.end(); ++it){
  		ls(fileNames,(*it).first,recursive,".root");
	  }
	}
	else{
    ls(fileNames, dirsToScan_,".root");	
	}
	return fileNames;
}

////////////////////////////////////////////////////////////////////
void PixelHistoScanDirectory::ls(vector<string> & list, vector<pair<string,bool> > dirs, string filter){
  for(vector<pair<string,bool> >::iterator it=dirs.begin(); it != dirs.end(); ++it){
		ls(list,(*it).first,(*it).second,filter);
	}
	std::sort(list.rbegin(), list.rend());
}
