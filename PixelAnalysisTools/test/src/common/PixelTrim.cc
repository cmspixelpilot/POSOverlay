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

  cout << "Usage: PixelTrim.exe <key> runTrimDefault runTrimVcThr runTrimVtrim runTrimOn runTrimOff" <<endl;

  assert(argc==7);

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

  string trimVtrim=getenv("POS_OUTPUT_DIRS");
  trimVtrim+="/Run_";
  trimVtrim+=argv[4];
  trimVtrim[trimVtrim.size()-1]='0';
  trimVtrim[trimVtrim.size()-2]='0';
  trimVtrim[trimVtrim.size()-3]='0';
  trimVtrim+="/Run_";
  trimVtrim+=argv[4];
  trimVtrim+="/TrimVtrim.dat";

  cout << "trimVtrim:"<<trimVtrim<<endl;
  
  string trimOn=getenv("POS_OUTPUT_DIRS");
  trimOn+="/Run_";
  trimOn+=argv[5];
  trimOn[trimOn.size()-1]='0';
  trimOn[trimOn.size()-2]='0';
  trimOn[trimOn.size()-3]='0';
  trimOn+="/Run_";
  trimOn+=argv[5];
  trimOn+="/TrimOn.dat";

  cout << "trimOn:"<<trimOn<<endl;

  string trimOff=getenv("POS_OUTPUT_DIRS");
  trimOff+="/Run_";
  trimOff+=argv[6];
  trimOff[trimOff.size()-1]='0';
  trimOff[trimOff.size()-2]='0';
  trimOff[trimOff.size()-3]='0';
  trimOff+="/Run_";
  trimOff+=argv[6];
  trimOff+="/TrimOff.dat";

  cout << "trimOff:"<<trimOff<<endl;


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
  std::string dummy;

  inDefault >> dummy;
  inDefault >> roc;

  while(!inDefault.eof()) {

    cout << "roc="<<roc << endl;
    
    PixelROCName theRoc(roc);

    int row,col;
    double threshold;

    inDefault >> row >> col >> tmp >> threshold;  // tmp is sigma = noise term

    if (threshold>200.0) threshold=-1.0;
    if (threshold<20.0) threshold=-1.0;

    theMap[theRoc][row][col].default_=threshold;

    inDefault >> dummy; // istat
    inDefault >> dummy; // chi2
    inDefault >> dummy; // prob
    inDefault >> dummy;
    inDefault >> roc;
    
  }


  ifstream inVcThr(trimVcThr.c_str());
  assert(inVcThr.good());
  
  inVcThr >> dummy;
  inVcThr >> roc;

  while(!inVcThr.eof()) {

    cout << "roc="<<roc << endl;
    
    PixelROCName theRoc(roc);

    int row,col;
    double threshold;

    inVcThr >> row >> col >> tmp >> threshold;  // tmp is sigma = noise term

    if (threshold>200.0) threshold=-1.0;
    if (threshold<20.0) threshold=-1.0;

    theMap[theRoc][row][col].vcthr_=threshold;

    inVcThr >> dummy; // istat
    inVcThr >> dummy; // chi2
    inVcThr >> dummy; // prob
    inVcThr >> dummy;
    inVcThr >> roc;
    
  }

  ifstream inVtrim(trimVtrim.c_str());
  assert(inVtrim.good());

  inVtrim >> dummy;
  inVtrim >> roc;

  while(!inVtrim.eof()) {

    cout << "roc="<<roc << endl;
    
    PixelROCName theRoc(roc);

    int row,col;
    double threshold;

    inVtrim >> row >> col >> tmp >> threshold;

    if (threshold>200.0) threshold=-1.0;
    if (threshold<20.0) threshold=-1.0;

    theMap[theRoc][row][col].vtrim_=threshold;

    inVtrim >> dummy; // istat
    inVtrim >> dummy; // chi2
    inVtrim >> dummy; // prob
    inVtrim >> dummy;
    inVtrim >> roc;
    
  }


  ifstream inOff(trimOff.c_str());
  assert(inOff.good());

  inOff >> dummy;
  inOff >> roc;

  while(!inOff.eof()) {

    cout << "roc="<<roc << endl;
    
    PixelROCName theRoc(roc);

    int row,col;
    double threshold;

    inOff >> row >> col >> tmp >> threshold;

    if (threshold>200.0) threshold=-1.0;
    if (threshold<20.0) threshold=-1.0;

    theMap[theRoc][row][col].off_=threshold;

    inOff >> dummy;
    inOff >> dummy;
    inOff >> dummy;
    inOff >> dummy;
    inOff >> roc;
    
  }

  ifstream inOn(trimOn.c_str());
  assert(inOn.good());

  inOn >> dummy;
  inOn >> roc;

  while(!inOn.eof()) {

    cout << "roc="<<roc << endl;
    
    PixelROCName theRoc(roc);

    int row,col;
    double threshold;

    inOn >> row >> col >> tmp >> threshold;

    if (threshold>200.0) threshold=-1.0;
    if (threshold<20.0) threshold=-1.0;

    theMap[theRoc][row][col].on_=threshold;

    inOn >> dummy;
    inOn >> dummy;
    inOn >> dummy;
    inOn >> dummy;
    inOn >> roc;
    
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
    trimROC.setThrTrim(80);
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
	cout << "(ben) " << newTrim-(int)trims->getTrimBits(iMap->first)->trim(col,row) << endl;
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

