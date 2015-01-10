#ifndef _PixelHistoProducerDescriptor_h_
#define _PixelHistoProducerDescriptor_h_
#include "TString.h"
#include "TList.h"
#include "TSocket.h"

class PixelHistoProducerDescriptor{

  TString name;
  TSocket *sock;
	
  TList *fileList;
	std::map<TString, TList*> *contentMap;

 public:
  PixelHistoProducerDescriptor(){
    name = "unnamed";
    fileList = 0;
    sock = 0;
		contentMap = new std::map<TString,TList*>();
  }

  PixelHistoProducerDescriptor(TString s, TList *l, TSocket *sk){
    name = s;
    fileList = l;
    sock = sk;
		contentMap = new std::map<TString,TList*>();
  }

  ~PixelHistoProducerDescriptor(){
    //dont delete socket... other structures will
    if(fileList){
      delete fileList;
			fileList = 0;
		}
		if(contentMap){
			delete contentMap;
			contentMap = 0;
		}
  }

  TSocket 									*getSocket()		 { return sock;}
  TString 									 getName()			 { return name;}
  TList 										*getFileList()	 { return fileList;}
  std::map<TString, TList*> *getContentMap() { return contentMap;}
	
  TList* getContentList(TString file){
		if(!contentMap){
			return 0;
		}
		std::map<TString,TList*>::iterator contentIt = contentMap->find(file);
		if(contentIt == contentMap->end()){
			return 0;
		}
		return contentIt->second;
	}
	
	void setFileList(TList *fl){ 
		if(fileList){
      delete fileList;
			fileList = 0;
		}
		fileList = fl; 
	}
	
	void setContentList(TString file, TList *cl){ 
		contentMap->erase(file); //erase if already present
		(*contentMap)[file] = cl;
	}

};

#endif
