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

  cout << "Usage: PixelModVana.exe <key> " <<endl;

  assert(argc==2);

  int ikey=atoi(argv[1]);
  cout << "key="<<ikey<<endl;

  ifstream inRocs("rocs_increase_vana.txt");
  assert(inRocs.good());

  
  PixelConfigKey key(ikey);

  string roc, tmp;

  inRocs >> roc;

  map<PixelModuleName, PixelDACSettings* > theDACs;


  while(!inRocs.eof()) {

    cout << "roc="<<roc << endl;
    
    PixelROCName theRoc(roc);

    PixelModuleName module(theRoc.rocname());

    if (theDACs.find(module)==theDACs.end()){
      PixelConfigInterface::get(theDACs[module], 
				"pixel/dac/"+module.modulename(), key);
      assert(theDACs[module]!=0); 
    }

    PixelDACSettings* dacs=theDACs[module];
    
    int vana=dacs->getDACSettings(theRoc)->getVana();
    
    vana=vana+5;

    dacs->getDACSettings(theRoc)->setVana(vana);

    
    inRocs >> tmp >> tmp >> tmp >> roc;
    
  }

  map<PixelModuleName, PixelDACSettings* >::const_iterator iModuleDAC;
  iModuleDAC=theDACs.begin();

  for(;iModuleDAC!=theDACs.end();iModuleDAC++){
    iModuleDAC->second->writeASCII("./");
  }


  
  return 0;
}

