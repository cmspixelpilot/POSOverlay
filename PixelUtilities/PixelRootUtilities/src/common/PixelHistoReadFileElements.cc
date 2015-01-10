#include "PixelUtilities/PixelRootUtilities/include/PixelHistoReadFileElements.h"
#include <TROOT.h>
#include <TObjString.h>
#include <TObject.h>
#include <TDirectoryFile.h>
#include <TKey.h>
#include <TClass.h>
#include <iostream>

using namespace std;

/////////////////////////////////////////////////////
PixelHistoReadFileElements::PixelHistoReadFileElements(){
}

/////////////////////////////////////////////////////
PixelHistoReadFileElements::~PixelHistoReadFileElements(){
	destroy();
}

/////////////////////////////////////////////////////
void PixelHistoReadFileElements::destroy(){
  dirNames.Delete();//Delete also the objects in the TList
  allNames.Delete();
  objNames.Delete();	
}

/////////////////////////////////////////////////////
void PixelHistoReadFileElements::read(TString startDirName,int level){
  destroy();
  TDirectory * startDir = gDirectory->GetDirectory(startDirName);
  if(startDir){
    startDir->cd();
  }
  else{
    cout << "[PixelHistoReadFileElements::read()]\tERROR: Cant' find dir " << startDirName << ". Did you give the full path starting from gROOT?" << endl;
  }
  readService(gDirectory->GetPath(),level);
}

/////////////////////////////////////////////////////
void PixelHistoReadFileElements::read(TDirectory *startDir,int level){
  destroy();
  if(startDir){
    startDir->cd();
  }
  readService(gDirectory->GetPath(),level);
}

/////////////////////////////////////////////////////
void PixelHistoReadFileElements::readService(TString dirName,int level){
  if(level != 0){
    TDirectory * startDir = gDirectory->GetDirectory(gDirectory->GetPath());
    bool file;
    TIter *next;
    TObject *obj;
    if(startDir->InheritsFrom(TDirectoryFile::Class())){
      file = true;
      next = new TIter(startDir->GetListOfKeys());
    }
    else{
      file = false;
      next = new TIter(startDir->GetList());
    }
    while((obj = (*next)())){
      TString name = dirName+"/"+obj->GetName();
			if((!file && obj->InheritsFrom(TDirectory::Class())) || 
			    (file && TString(((TKey*)obj)->GetClassName()).BeginsWith("TDirectory"))){
	      dirNames.Add ( new TObjString(name+"/") );
				allNames.Add ( new TObjString(name+"/") );
				gDirectory->cd(name);
				readService(name+"/",level-1);
      }
      else{
				objNames.Add ( new TObjString(name) );
				allNames.Add ( new TObjString(name) );
      }
    }
    delete next;
    startDir->cd();
  }
}

/////////////////////////////////////////////////////
void PixelHistoReadFileElements::dump(){
  string mthn = "[PixelHistoReadFileElements::dump()]\t";
  TObject * obj;

  cout << mthn << "All names:" << endl;
  TIter next1(&allNames);
  while ( (obj = next1())){
    cout << mthn << obj->GetName() << endl;
  }
  cout << endl;
	
  cout << mthn << "Dir names:" << endl;
  TIter next2(&dirNames);
  while ( (obj = next2())){
    cout << mthn << obj->GetName() << endl;
  }
  cout << endl;
	
  cout << mthn << "Obj names:" << endl;
  TIter next3(&objNames);
  while ( (obj = next3())){
    cout << mthn << obj->GetName() << endl;
  }
  cout << endl;	
}
