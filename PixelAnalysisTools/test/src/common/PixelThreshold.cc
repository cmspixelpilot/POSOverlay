//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
// authors A. Ryd//
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"
#include "CalibFormats/SiPixelObjects/interface/PixelConfigKey.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTrimBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACSettings.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "PixelAnalysisTools/include/PixelROCTrimer.h"
#include <fstream>
#include <iostream>
#include <string>
#include <map>

using namespace std;
using namespace pos;


int main(int argc, char **argv){

  double deltaThreshold=-30;

  PixelConfigKey key(3253);
 
  //do something horrible...
  //                    row      col
  map<PixelROCName, double > theMap;


  map<PixelModuleName, PixelDACSettings* > theDACs;

  string roc;

  ifstream inDefault("BmOVcThrSlopes.dat");

  double tmp;

  inDefault >> roc;

  while(!inDefault.eof()) {

    inDefault >> tmp;

    cout << "roc="<<roc << " der = "<<tmp<< endl;
    
    PixelROCName theRoc(roc);

    theMap[theRoc]=tmp;

    inDefault >> roc;
    
  }


  map<PixelROCName, double >::const_iterator iMap;

  iMap=theMap.begin();

  for(;iMap!=theMap.end();iMap++){
    double der=iMap->second;
    cout << iMap->first << endl;
    PixelModuleName module(iMap->first.rocname());

    PixelROCTrimmer trimROC(iMap->first);

    if (theDACs.find(module)==theDACs.end()){
      PixelConfigInterface::get(theDACs[module], 
				"pixel/dac/"+module.modulename(), key);
      assert(theDACs[module]!=0);
    }

    PixelDACSettings* dacs=theDACs[module];
    
    int vcThr=dacs->getDACSettings(iMap->first)->getVcThr();

    double newVcThr=vcThr+deltaThreshold/der;

    dacs->getDACSettings(iMap->first)->setVcThr(newVcThr);

  }

  map<PixelModuleName, PixelDACSettings* >::const_iterator iModuleDAC;
  iModuleDAC=theDACs.begin();

  for(;iModuleDAC!=theDACs.end();iModuleDAC++){
    iModuleDAC->second->writeASCII("./");
  }

  
  return 0;
}

