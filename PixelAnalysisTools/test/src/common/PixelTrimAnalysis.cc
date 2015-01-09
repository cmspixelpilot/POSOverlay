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
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include <fstream>
#include <iostream>
#include <string>
#include <map>

using namespace std;
using namespace pos;

int main(int argc, char **argv){

  cout << "Usage: PixelTrimAnalysis.exe runTrimDefault" <<endl;

  assert(argc==2);

  string trimDefault=getenv("POS_OUTPUT_DIRS");
  trimDefault+="/Run_";
  trimDefault+=argv[1];
  trimDefault[trimDefault.size()-1]='0';
  trimDefault[trimDefault.size()-2]='0';
  trimDefault[trimDefault.size()-3]='0';
  trimDefault+="/Run_";
  trimDefault+=argv[1];
  trimDefault+="/TrimDefault.dat";

  cout << "trimDefault:"<<trimDefault<<endl;

  //do something horrible...
  //                    row      col
  map<PixelROCName, Moments > theMap;


  string roc;

  ifstream inDefault(trimDefault.c_str());
  assert(inDefault.good());

  string tmp;

  inDefault >> tmp;

  inDefault >> roc;

  while(!inDefault.eof()) {

    //cout << "tmp="<<tmp << endl;
    //cout << "roc="<<roc << endl;
    
    PixelROCName theRoc(roc);

    int row,col;
    double threshold;


    inDefault >> row >> col >> threshold >> tmp;

    if (threshold>200.0) threshold=-1.0;
    if (threshold<40.0) threshold=-1.0;

    if (threshold>0) {
      theMap[theRoc].push_back(threshold);
    }

    inDefault >> tmp;
    inDefault >> roc;
    
  }



  map<PixelROCName, Moments >::const_iterator iMap;

 
  iMap=theMap.begin();

  cout << "All ROCs"<<endl;

  for(;iMap!=theMap.end();iMap++){

    cout << iMap->first <<" "
	 << iMap->second.count()<<" "
	 << iMap->second.mean()<<" "
	 << iMap->second.stddev()<<endl;

  }


  iMap=theMap.begin();
  cout << "Problem ROCs"<<endl;

  for(;iMap!=theMap.end();iMap++){

    if (iMap->second.count()<60||
	iMap->second.mean()<50.0||
	iMap->second.stddev()>20.0){
      cout << iMap->first <<" "
	   << iMap->second.count()<<" "
	   << iMap->second.mean()<<" "
	   << iMap->second.stddev()<<endl;
    }
  }


  return 0;
}

