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

  bool verbose = false;
  bool assumeGlobal = false;//true if you want to assume a common absolute threshold for all ROCs
  double targetIntimeThr=-99;
  double targetIntimeThr_FPix=78;
  double targetIntimeThr_BPix=83;
  double targetIntimeThr_BPix_special=84;
  double globalThr=70;

  cout << "Usage: PixelIntimeVana.exe key runTrimDefault" <<endl;

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
  string trimDefault_abs = trimDefault;
  trimDefault+="/TrimDefault.dat";
  trimDefault_abs+="/TrimDefault_abs.dat";
  

  cout << "trimDefault:"<<trimDefault<<endl;
  if(!assumeGlobal)   cout << "trimDefault_abs:"<<trimDefault_abs<<endl;
  PixelConfigKey key(ikey);
 
  //do something horrible...
  //                    row      col
  map<PixelROCName, map<int, map<int, PixelValues> > > theMap;
  map<PixelROCName, map<int, map<int, PixelValues> > > theMap_abs;

  map<PixelModuleName, PixelDACSettings* > theDACs;

  string roc;

  ifstream inDefault(trimDefault.c_str());
  assert(inDefault.good());

  double tmp;
  string tmp2;
 

  //inDefault >> tmp2;
  //inDefault >> roc;

  while(!inDefault.eof()) {
    inDefault >> tmp2;
    inDefault >> roc;
    
    if(verbose) cout << "roc="<<roc << endl;
    PixelROCName theRoc(roc);

    int row,col;
    double threshold;
    //inDefault >> row >> col >> threshold >> tmp;
    inDefault >> row >> col >> tmp >> threshold >> tmp;

    if (threshold>150.0) threshold=-1.0;
    if (threshold<40.0) threshold=-1.0;

    theMap[theRoc][row][col].default_=threshold;


    inDefault >> tmp;
    //inDefault >> roc;
    inDefault >> tmp;
  }

  if(!assumeGlobal){
    string roc_abs;
    ifstream inDefault_abs(trimDefault_abs.c_str());
    assert(inDefault_abs.good());
    
    double tmp_abs ;
    string tmp2_abs;
    
    while(!inDefault_abs.eof()) {
      
      inDefault_abs >> tmp2_abs;
      inDefault_abs >> roc_abs;
    
      if(verbose) cout << "roc_abs="<<roc_abs << endl;
      PixelROCName theRoc_abs(roc_abs);

      int row_abs,col_abs;
      double threshold_abs;

      //inDefault_abs >> row_abs >> col_abs >> threshold_abs >> tmp_abs;
      inDefault_abs >> row_abs >> col_abs >> tmp_abs >> threshold_abs >> tmp_abs;

      if (threshold_abs>150.0) threshold_abs=-1.0;
      if (threshold_abs<40.0) threshold_abs=-1.0;
      
      theMap_abs[theRoc_abs][row_abs][col_abs].default_=threshold_abs;

      inDefault_abs >> tmp_abs;
      //inDefault_abs >> roc_abs;
      inDefault_abs >> tmp_abs;
    }
  }
  
  map<PixelROCName, map<int, map<int, PixelValues> > >::const_iterator iMap;
  map<PixelROCName, map<int, map<int, PixelValues> > >::const_iterator iMap_abs;


  iMap=theMap.begin();
  iMap_abs=theMap_abs.begin();

  for(;iMap!=theMap.end();iMap++){
    if(!assumeGlobal) assert(iMap->first.rocname() == iMap_abs->first.rocname());

    cout << iMap->first << endl;
    PixelModuleName module(iMap->first.rocname());

    // find out if it is BPix or FPix
    string currentRoc = (string)iMap->first.rocname();
    if ( currentRoc.find("BPix") == 0 ) {
      if(verbose) cout << "This is BPix" << endl;
      targetIntimeThr = targetIntimeThr_BPix;

      // apply special settings to the following list of BPix PowerGroups
      if ( currentRoc.find("BPix_BmO_SEC2_LYR3") == 0 ||
	   currentRoc.find("BPix_BmO_SEC3_LYR3") == 0 ||
	   currentRoc.find("BPix_BmO_SEC8_LYR1") == 0 || currentRoc.find("BPix_BmO_SEC8_LYR2") == 0 ) 
	{
	  if(verbose)cout << "This is BPix special" << endl;
	  targetIntimeThr = targetIntimeThr_BPix_special;
	}
    }
    else {
      if(verbose) cout << "This is FPix" << endl;
      targetIntimeThr = targetIntimeThr_FPix;
    }


    PixelROCTrimmer trimROC(iMap->first);
    PixelROCTrimmer trimROC_abs(iMap_abs->first);

    if (theDACs.find(module)==theDACs.end()){
      PixelConfigInterface::get(theDACs[module], 
				"pixel/dac/"+module.modulename(), key);
      assert(theDACs[module]!=0);
    }
    PixelDACSettings* dacs=theDACs[module];
    
    int vAna=dacs->getDACSettings(iMap->first)->getVana();

    if(verbose) cout << "vAna " << vAna << endl;

    map<int, map<int, PixelValues> >::const_iterator irow;
    irow=iMap->second.begin();
    for(;irow!=iMap->second.end();irow++){
      int row=irow->first;
      map<int, PixelValues>::const_iterator icol;
      icol=irow->second.begin();
      for(;icol!=irow->second.end();icol++){
	int col=icol->first;;
	//cout << row << " " << col << " " 
	//     << trimbits << " "
	//     << icol->second.default_ << " " 
	//     << icol->second.vcthr_ << " " 
	//     << icol->second.vtrim_ << " " 
	//     << icol->second.on_ << " " 
	//     << icol->second.off_ << endl; 
	trimROC.addPixel(row,col,0,icol->second.default_,
			 icol->second.vcthr_,icol->second.vtrim_,
			 icol->second.on_,icol->second.off_);
			 
      }
    }
    //cout << "-1" << endl;
    double avgThr=trimROC.avgThr();

    double avgThr_abs = 0;
    if(!assumeGlobal){
      map<int, map<int, PixelValues> >::const_iterator irow_abs;
      irow_abs=iMap_abs->second.begin();
      for(;irow_abs!=iMap_abs->second.end();irow_abs++){
	int row_abs=irow_abs->first;
	map<int, PixelValues>::const_iterator icol_abs;
	icol_abs=irow_abs->second.begin();
	for(;icol_abs!=irow_abs->second.end();icol_abs++){
	  int col_abs=icol_abs->first;;
	  trimROC_abs.addPixel(row_abs,col_abs,0,icol_abs->second.default_,
			       icol_abs->second.vcthr_,icol_abs->second.vtrim_,
			       icol_abs->second.on_,icol_abs->second.off_);
	  
	}
      }
      avgThr_abs=trimROC_abs.avgThr();
    }

    cout << "(ben) ROC: " << iMap->first << " avgThr: " << avgThr << endl;
    if(!assumeGlobal) cout << "(ben) ROC: " << iMap->first << " avgThr_abs: " << avgThr_abs << endl;

    if(!assumeGlobal && verbose) cout << "(ben) inabs, ROC: " <<  iMap->first << " " << avgThr << " " << avgThr_abs << endl;
    
    int newVana=vAna;
    if(assumeGlobal){
      if(verbose) cout << "Current Thr Unprotected= " << avgThr << endl;
      
      if (avgThr<globalThr+5) {
	//protection
	avgThr=globalThr+5;
      }
      
      if(verbose) cout << "OldVana= "<<vAna<<endl;;
      if(verbose) cout << "Target Intime Thr= "<<targetIntimeThr<<endl;
      if(verbose) cout << "Current Thr= "<<avgThr<<endl;
      
      // apply only half of the correction
      //double deltaVana=(avgThr-targetIntimeThr);
      double deltaVana=(avgThr-targetIntimeThr)/2;
      
      cout << "deltaVana= "<<deltaVana<<endl;
      if (deltaVana<-7) deltaVana=-7;
      if (deltaVana>10) deltaVana=10;
      newVana=(int)(vAna+deltaVana);
      cout << "deltaVana_corr= "<<deltaVana<<endl;
      if(verbose) cout << "newVana= "<<newVana<<endl;
      
      if(deltaVana>1.5 || deltaVana<-1.5) cout << "+ " << iMap->first << endl; //output for calib file -- rocs with large changes
      // all relevant numbers in one line:
      // ROC name : OldVana : Current Thr : deltaVana 
      cout << "(ag) " << iMap->first << " " << vAna << " " << avgThr << " " << deltaVana << endl;
    }
    else{
      double inabsdiff = avgThr-avgThr_abs;
      double inabsdiffTarget = targetIntimeThr-globalThr;
      double deltaVana =(inabsdiff-inabsdiffTarget)/2;

      cout << iMap->first<<"deltaVana= "<<deltaVana<<endl;
      if (deltaVana<-7) deltaVana=-7;
      if (deltaVana>10) deltaVana=10;
      newVana=(int)(vAna+deltaVana);
      cout << iMap->first << "deltaVana_corr= "<<deltaVana<<endl;
      if(verbose) cout << "newVana= "<<newVana<<endl;
      if(deltaVana>1.5 || deltaVana<-1.5) cout << "+ " << iMap->first << endl; //output for calib file -- rocs with large changes
      cout << "(ag) " << iMap->first << " " << vAna << " " << avgThr << " " << avgThr_abs<< " " << deltaVana << " " << inabsdiff << endl;
    }
    dacs->getDACSettings(iMap->first)->setVana(newVana);
    if(!assumeGlobal) iMap_abs++;
  }
  
  map<PixelModuleName, PixelDACSettings* >::const_iterator iModuleDAC;
  iModuleDAC=theDACs.begin();

  for(;iModuleDAC!=theDACs.end();iModuleDAC++){
    iModuleDAC->second->writeASCII("./");
  }

  
  return 0;
}

