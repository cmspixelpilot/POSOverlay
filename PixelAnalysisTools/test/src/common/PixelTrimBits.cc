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

class PixelDer{

public:

  PixelDer(){
    derVcThr_=999.9;
    derVtrim_=999.9;
    derBits_=999.9;
  }

public:

  double derVcThr_;
  double derVtrim_;
  double derBits_;

};


int main(int argc, char **argv){

  cout << "Usage: PixelTrim.exe <key> runTrimDefault" <<endl;

  assert(argc==3);

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

  PixelConfigKey key(ikey);
 
  //do something horrible...
  //                    row      col
  map<PixelROCName, map<int, map<int, PixelValues> > > theMap;

  map<PixelROCName, PixelDer > theDer;

  map<PixelModuleName, PixelTrimBase* > theTrims;

  string roc;

  ifstream inDefault(trimDefault.c_str());

  assert(inDefault.good());

  double tmp;

  string dummy;

  inDefault >> dummy >> roc;

  int count=0;

  while(!inDefault.eof()) {

    count++;

    if (count%1000000==0){
      cout << count<<" roc="<<roc << endl;
    }    

    PixelROCName theRoc(roc);

    int row,col;
    double threshold;


    inDefault >> row >> col >> threshold >> tmp;

    if (threshold>130.0) threshold=-1.0;
    if (threshold<40.0) threshold=-1.0;

    theMap[theRoc][row][col].default_=threshold;

    inDefault >> dummy >> roc;
    
  }

  cout << "Have read the pixels" << endl;

  ifstream inDer("rocder.dat");

  inDer >> roc;

  int counter=0;

  while(!inDer.eof()) {

    counter++;
    
    assert(counter<20000);

    cout << "roc="<<roc << endl;
    
    PixelROCName theRoc(roc);

    PixelDer der;


    inDer >> der.derVcThr_ >> der.derVtrim_ >> der.derBits_;
    cout << "***derVcThr is: " << der.derVcThr_ <<"***" <<endl;

    theDer[theRoc]=der;

    inDer >> roc;
    
  }

  cout << "Have read the derivatives" << endl;


  map<PixelROCName, map<int, map<int, PixelValues> > >::const_iterator iMap;

  

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

    trimROC.setThrTrim(80.0);

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
    trimROC.findNewBits(theDer[iMap->first].derBits_);

    cout << "VcThr derivative for " << iMap->first << " is "
      //    	 << trimROC.vcThrDer() << endl;
	 << theDer[iMap->first].derVcThr_ << endl;
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
  }

  map<PixelModuleName, PixelTrimBase* >::const_iterator iModule;
  iModule=theTrims.begin();

  for(;iModule!=theTrims.end();iModule++){
    iModule->second->writeASCII("");
  }

  return 0;
}

