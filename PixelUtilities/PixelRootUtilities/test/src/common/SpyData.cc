#include "PixelUtilities/PixelRootUtilities/include/PixelHistoServer.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelHistoSpyData.h"
#include "CalibFormats/SiPixelObjects/interface/PixelNameTranslation.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDetectorConfig.h"
//#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include <iostream>
#include <vector>
#include <map>

#include <stdlib.h>

using namespace std;
using namespace pos;

int main(int argc, char **argv){
	string mthn = "[main()]\t";
	if(argc != 2){
    cout << mthn << "Usage: SpyData RunNumber" << endl;
    exit(0);
  }
  PixelNameTranslation    thePixelNameTranslation    ("/PixelConfig/Pix/nametranslation/0/translation.dat");
  PixelDetectorConfig     thePixelDetectorConfig     ("/PixelConfig/Pix/detconfig/12/detectconfig.dat");
//  PixelCalibConfiguration thePixelCalibConfiguration ("/nfshome0/pixelpro/TriDAS/pixel/PixelRun/Runs/Run_54842/calib.dat");
//  thePixelCalibConfiguration.buildROCAndModuleLists(&thePixelNameTranslation, &thePixelDetectorConfig);
//  vector<pos::PixelROCName> rocList = thePixelCalibConfiguration.rocList();
  map<PixelROCName, PixelROCStatus> rocList    = thePixelDetectorConfig.getROCsList();
  vector<pos::PixelModuleName>      moduleList = thePixelDetectorConfig.getModuleList() ; 
	if(rocList.size() == 0){
    std::list<const PixelROCName*> ROCNameList = thePixelNameTranslation.getROCs();
    for( std::list<const PixelROCName*>::iterator itRoc=ROCNameList.begin(); itRoc!=ROCNameList.end(); ++itRoc){
//    	for(vector<pos::PixelModuleName>::iterator itMod = moduleList.begin(); itMod != moduleList.end() ; ++itMod){
//			  if((*itRoc)->rocname().find((*itMod).modulename()) == 0){
					rocList[**itRoc] = PixelROCStatus();
//					break;
//				}
//			}
		}
	}
  vector<PixelROCName>              rocToAnalyzeList;
	for (map<PixelROCName, PixelROCStatus>::iterator it=rocList.begin();it!=rocList.end();it++){
		if(it->second.statusName() == ""){
		  rocToAnalyzeList.push_back(it->first);
//			cout << mthn << it->first.rocname() << endl;
		}
	}  
  vector<int> fedList;
/*
*/	
	fedList.push_back(0);
	fedList.push_back(1);
	fedList.push_back(2);
	fedList.push_back(3);
	fedList.push_back(4);
	fedList.push_back(5);
	fedList.push_back(6);
	fedList.push_back(7);
	fedList.push_back(8);
	fedList.push_back(9);
	fedList.push_back(10);
	fedList.push_back(11);
	fedList.push_back(12);
	fedList.push_back(13);
	fedList.push_back(14);
	fedList.push_back(15);
	fedList.push_back(16);
	fedList.push_back(17);
	fedList.push_back(18);
	fedList.push_back(19);
	fedList.push_back(20);
	fedList.push_back(21);
	fedList.push_back(22);
	fedList.push_back(23);	
	fedList.push_back(24);
	fedList.push_back(25);
	fedList.push_back(26);
	fedList.push_back(27);
	fedList.push_back(28);
	fedList.push_back(29);
	fedList.push_back(30);
	fedList.push_back(31);
	fedList.push_back(32);
	fedList.push_back(33);
	fedList.push_back(34);
	fedList.push_back(35);
	fedList.push_back(36);
	fedList.push_back(37);
	fedList.push_back(38);
	fedList.push_back(39);
/*
*/	
	PixelHistoSpyData spyData(fedList,rocToAnalyzeList,thePixelNameTranslation);
	spyData.init("/nfshome0/pixelpro/TriDAS/pixel/PixelRun/Runs/",atoi(argv[1]));
  static PixelHistoServer hs;
  hs.startThreads();

	spyData.startThreads();
	cout << mthn << "Press q to quit";
	char enter = 'a';
	while(enter != 'q'){
		cin >> enter;
	}
	cout << mthn << "Deleting all Objects..." << endl;
	spyData.stopThreads();
	hs.stopThreads();
	cout << mthn << "Bye bye" << endl;
	return 0;
}
