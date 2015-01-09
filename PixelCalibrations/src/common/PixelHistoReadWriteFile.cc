#include <iostream>
#include "PixelCalibrations/include/PixelHistoReadWriteFile.h"
#include <TFile.h>
#include <TROOT.h>
#include <TObject.h>
#include <TKey.h>
#include <TClass.h>
#include <TTree.h>
#include <TH1.h>
#include <TDirectoryFile.h>

using namespace std;

/////////////////////////////////////////////////////
PixelHistoReadWriteFile::PixelHistoReadWriteFile(){
	file=0;
}

/////////////////////////////////////////////////////
PixelHistoReadWriteFile::~PixelHistoReadWriteFile(){
	close();
}

/////////////////////////////////////////////////////
void PixelHistoReadWriteFile::write(std::string fileName, string mode){
	open(fileName,mode);
	transferFromTo(gROOT,file);
	//cout <<"[PixelHistoReadWriteFile::write]"<<endl;
	deleteDir(gROOT);
	close();
}

/////////////////////////////////////////////////////
void PixelHistoReadWriteFile::read(std::string fileName){
	open(fileName,"READ");
	transferFromTo(file,gROOT);
	close();
}

/////////////////////////////////////////////////////
void PixelHistoReadWriteFile::open(std::string fileName, string mode){
	close();
  file = new TFile(fileName.c_str(),mode.c_str());
	if(file == 0 || !file->IsOpen()){
		cout << "[PixelHistoReadWriteFile::open()]\tCouldn't open file: " << fileName << endl;
	}
}

/////////////////////////////////////////////////////
void PixelHistoReadWriteFile::close(){
	if(file){
		file->Close();
		delete file;
		file=0;
	}
}

/////////////////////////////////////////////////////
void PixelHistoReadWriteFile::transferFromMemory(TDirectory *parentFileDir,TDirectory *parentMemDir){
  TIter next(parentMemDir->GetList());
	TObject *obj;
  while((obj = next())){
    parentFileDir->cd();
    if(obj->IsFolder()){ 
      TDirectory * subDir = parentFileDir->mkdir(obj->GetName(),obj->GetTitle());
      transferFromMemory(subDir,(TDirectory*)obj);
    }
    else{ 
      obj->Write();
    }
	}
}

/////////////////////////////////////////////////////
void PixelHistoReadWriteFile::transferToMemory(TDirectory *parentFileDir,TDirectory *parentMemDir){
  TIter next(parentFileDir->GetListOfKeys());
	TObject *obj,*key;
  while((key = next())){
    parentMemDir->cd();
		obj = ((TKey*)key)->ReadObj();
    if(obj->IsFolder()){ 
//      TDirectory * subDir = new TDirectory(obj->GetName(),obj->GetTitle()) ;
      TDirectory * subDir = parentMemDir->mkdir(obj->GetName(),obj->GetTitle());
      transferToMemory((TDirectory*)obj,subDir);
    }
	}
}

/////////////////////////////////////////////////////
void PixelHistoReadWriteFile::transferFromTo(TDirectory *fromDir,TDirectory *toDir){
	bool read = false;
	if(fromDir->InheritsFrom(TDirectoryFile::Class())){
		read = true;
	}
//  cout << "[PixelHistoReadWriteFile::transferFromTo()]\tRead? " << read << " => " << fromDir->IsA()->GetName() << endl;
	TIter *next;
	TObject *obj;
	if(read){
  	next = new TIter(fromDir->GetListOfKeys());
	}
	else{
		next = new TIter(fromDir->GetList());
	}
  while((obj = (*next)())){
    toDir->cd();
		if(read){
			obj = ((TKey*)obj)->ReadObj();
		}
    if(obj->InheritsFrom(TDirectory::Class())){
//      cout << "[PixelHistoReadWriteFile::transferFromTo()]\tIs folder? " << obj->InheritsFrom(TDirectory::Class()) << " => " << obj->GetName() << endl;
      TDirectory * fromSubDir = (TDirectory*)obj;
      TDirectory * toSubDir = toDir->mkdir(obj->GetName(),obj->GetTitle());
      transferFromTo(fromSubDir,toSubDir);
    }
		else{ 
			if(!read){
				obj->Write();
			}
		}
	}
	delete next;
}

/////////////////////////////////////////////////////
void PixelHistoReadWriteFile::deleteDir(TDirectory *dir){

  TIter *next;
  TObject *obj;
  next = new TIter(dir->GetList());
  
  //cout <<"[PixelHistoReadWriteFile::deleteDir]"<<endl;
  
  while((obj = (*next)())){
    //cout << "[PixelHistoReadWriteFile::deleteDir] obj->GetName()="
    //	 << obj->GetName()<<endl;
    if(obj->InheritsFrom(TDirectory::Class())){
      TDirectory * subDir = (TDirectory*)obj;
      deleteDir(subDir);
    }
    else{ 
      delete obj;
    }
  }
  delete next;
  if (dir!=gROOT) {
    delete dir;
  }
}

