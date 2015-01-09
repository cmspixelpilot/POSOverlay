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

class PixelValues{

public:

  PixelValues(){
    trim_=-1;
    default_=-1;
    vcthr_=-1;
    vtrim_=-1;
    off_=-1;
    on_=-1;
  }

public:

  int trim_;
  double default_;
  double vcthr_;
  double vtrim_;
  double off_;
  double on_;

};


int main(int argc, char **argv){

  cout << "Usage: PixelTrimVcThr.exe key runTrimDefault runTrimVcThr" <<endl;

  assert(argc==4);

  int ikey=atoi(argv[1]);
  cout << "key="<<ikey<<endl;

  string trimDefault=getenv("POS_OUTPUT_DIRS");
  trimDefault+="/Run_";
  trimDefault+=argv[2];
  trimDefault[trimDefault.size()-1]='0';
  trimDefault[trimDefault.size()-2]='0';
  trimDefault[trimDefault.size()-3]='0';
  trimDefault+="/Run_";
  trimDefault+=argv[2];
  trimDefault+="/TrimDefault.dat";

  cout << "trimDefault:"<<trimDefault<<endl;

  string trimVcThr=getenv("POS_OUTPUT_DIRS");
  trimVcThr+="/Run_";
  trimVcThr+=argv[3];
  trimVcThr[trimVcThr.size()-1]='0';
  trimVcThr[trimVcThr.size()-2]='0';
  trimVcThr[trimVcThr.size()-3]='0';
  trimVcThr+="/Run_";
  trimVcThr+=argv[3];
  trimVcThr+="/TrimVcThr.dat";

  cout << "trimVcthr:"<<trimVcThr<<endl;

  PixelConfigKey key(ikey);
 
  //do something horrible...
  //                    row      col
  map<PixelROCName, map<int, map<int, PixelValues> > > theMap;

  map<PixelModuleName, PixelTrimBase* > theTrims;
  map<PixelModuleName, PixelDACSettings* > theDACs;

  string roc;

  ifstream inDefault(trimDefault.c_str());
  assert(inDefault.good());

  double tmp;

  string tmp2;
  
  inDefault >> tmp2;
  
  inDefault >> roc;

  while(!inDefault.eof()) {

    cout << "roc="<<roc << endl;
    
    PixelROCName theRoc(roc);

    int row,col;
    double threshold;


    inDefault >> row >> col >> threshold >> tmp;

    if (threshold>150.0) threshold=-1.0;
    if (threshold<40.0) threshold=-1.0;

    theMap[theRoc][row][col].default_=threshold;

    inDefault >> tmp2;

    inDefault >> roc;
    
  }


  ifstream inVcThr(trimVcThr.c_str());
  assert(inVcThr.good());
  
  inVcThr >> tmp2;

  inVcThr >> roc;

  while(!inVcThr.eof()) {

    cout << "roc="<<roc << endl;
    
    PixelROCName theRoc(roc);

    int row,col;
    double threshold;

    inVcThr >> row >> col >> threshold >> tmp;

    if (threshold>150.0) threshold=-1.0;
    if (threshold<40.0) threshold=-1.0;

    theMap[theRoc][row][col].vcthr_=threshold;

   inVcThr >> tmp2; 
   inVcThr >> roc;
    
  }

  map<PixelROCName, map<int, map<int, PixelValues> > >::const_iterator iMap;

  ofstream outDer("rocder.dat");

  iMap=theMap.begin();

  for(;iMap!=theMap.end();iMap++){
    cout << iMap->first << endl;
    PixelModuleName module(iMap->first.rocname());

    PixelROCTrimmer trimROC(iMap->first);

    if (theTrims.find(module)==theTrims.end()){
      PixelConfigInterface::get(theTrims[module], 
				"pixel/trim/"+module.modulename(), key);
      assert(theTrims[module]!=0);
    }
    PixelTrimBase* trims=theTrims[module];

    if (theDACs.find(module)==theDACs.end()){
      PixelConfigInterface::get(theDACs[module], 
				"pixel/dac/"+module.modulename(), key);
      assert(theDACs[module]!=0);
    }
    PixelDACSettings* dacs=theDACs[module];
    
    int vcThr=dacs->getDACSettings(iMap->first)->getVcThr();
    int vtrim=dacs->getDACSettings(iMap->first)->getVtrim();

    trimROC.setVcThr(vcThr);
    trimROC.setVtrim(vtrim);
    trimROC.setThrTrim(60.0);
    trimROC.setNsigma(4.0);

    cout << "vcthr " << vcThr << endl;
    cout << "vtrim " << vtrim << endl;

    map<int, map<int, PixelValues> >::const_iterator irow;
    irow=iMap->second.begin();
    for(;irow!=iMap->second.end();irow++){
      int row=irow->first;
      map<int, PixelValues>::const_iterator icol;
      icol=irow->second.begin();
      for(;icol!=irow->second.end();icol++){
	int col=icol->first;
	int trimbits=trims->getTrimBits(iMap->first)->trim(col,row);
	cout << row << " " << col << " " 
	     << trimbits << " "
	     << icol->second.default_ << " " 
	     << icol->second.vcthr_ << " " 
	     << icol->second.vtrim_ << " " 
	     << icol->second.on_ << " " 
	     << icol->second.off_ << endl; 
	trimROC.addPixel(row,col,trimbits,icol->second.default_,
			 icol->second.vcthr_,icol->second.vtrim_,
			 icol->second.on_,icol->second.off_);
			 
      }
    }
    cout << "-1" << endl;
    trimROC.findNewSettings();

    outDer << iMap->first << " " 
	   << trimROC.vcThrDer() << " "
	   << trimROC.vtrimDer() << " "
	   << trimROC.trimBitDer() << endl;

    cout << "VcThr derivative for " << iMap->first << " is "
	 << trimROC.vcThrDer() << endl;
    irow=iMap->second.begin();
    for(;irow!=iMap->second.end();irow++){
      int row=irow->first;
      map<int, PixelValues>::const_iterator icol;
      icol=irow->second.begin();
      for(;icol!=irow->second.end();icol++){
	int col=icol->first;
	int newTrim=trimROC.newTrim(row,col);
	trims->getTrimBits(iMap->first)->setTrim(col,row,newTrim);
      }
    }
    dacs->getDACSettings(iMap->first)->setVtrim(trimROC.newVtrim());
    dacs->getDACSettings(iMap->first)->setVcThr(trimROC.newVcThr());
  }

  map<PixelModuleName, PixelTrimBase* >::const_iterator iModule;
  iModule=theTrims.begin();

  for(;iModule!=theTrims.end();iModule++){
    iModule->second->writeASCII("");
  }

  map<PixelModuleName, PixelDACSettings* >::const_iterator iModuleDAC;
  iModuleDAC=theDACs.begin();

  for(;iModuleDAC!=theDACs.end();iModuleDAC++){
    iModuleDAC->second->writeASCII("./");
  }

  
  return 0;
}

