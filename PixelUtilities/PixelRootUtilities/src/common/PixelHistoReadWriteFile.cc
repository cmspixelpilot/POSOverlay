#include <iostream>
#include "PixelUtilities/PixelRootUtilities/include/PixelHistoReadWriteFile.h"
#include <TFile.h>
#include <TROOT.h>
#include <TObject.h>
#include <TKey.h>
#include <TCanvas.h>
#include <TClass.h>


using namespace std;

/////////////////////////////////////////////////////
PixelHistoReadWriteFile::PixelHistoReadWriteFile(){
	file_=0;
}

/////////////////////////////////////////////////////
PixelHistoReadWriteFile::~PixelHistoReadWriteFile(){
	close();
}

/////////////////////////////////////////////////////
void PixelHistoReadWriteFile::write(std::string fileName, string mode){
	open(fileName,mode);
	transferFromTo(gROOT,file_);
	close();
}

/////////////////////////////////////////////////////
void PixelHistoReadWriteFile::read(std::string fileName){
	open(fileName,"READ");
	transferFromTo(file_,gROOT);
	close();
}

/////////////////////////////////////////////////////
void PixelHistoReadWriteFile::open(std::string fileName, string mode){
	close();
  file_ = new TFile(fileName.c_str(),mode.c_str());
	if(file_ == 0 || !file_->IsOpen()){
		cout << "[PixelHistoReadWriteFile::open()]\tCouldn't open file: " << fileName << endl;
	}
}

/////////////////////////////////////////////////////
void PixelHistoReadWriteFile::close(){
	if(file_){
		file_->Close();
		delete file_;
		file_=0;
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
	string mthn = "[PixelHistoReadWriteFile::transferFromTo()]\t";
	bool read = false;
	if(fromDir->InheritsFrom(TDirectoryFile::Class())){
		read = true;
	}
//  cout << "[PixelHistoReadWriteFile::transferFromTo()]\tRead? " << read << " From: " << fromDir->IsA()->GetName() << " To --->" << toDir->IsA()->GetName()<< endl;
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
      if(obj->InheritsFrom(TCanvas::Class())){
        toDir->Append(obj);
      }	
		}
    if(obj->InheritsFrom(TDirectory::Class())){
//  	  cout << "[PixelHistoReadWriteFile::transferFromTo()]\tIs folder? " << obj->InheritsFrom(TDirectory::Class()) << " => " << obj->GetName() << endl;
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

