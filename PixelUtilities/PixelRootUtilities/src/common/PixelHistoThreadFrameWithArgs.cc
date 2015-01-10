#include "PixelUtilities/PixelRootUtilities/include/PixelHistoThreadFrameWithArgs.h"
#include <iostream>
#include <sstream>
#include <TThread.h> 
#include <TROOT.h>

using namespace std;
/*
vector<pair<string,string> > ThreadArgs::getArgsVector(){
	int beginPair=0;
	int endPair=-1;
	int equal=-1;
	vector<pair<string,string> > retVec;
	while(beginPair <= values_.length()){
	  endPair=values_.find("&",beginPair+1);
		if(endPair == -1){
			endPair=values_.length();
		}
		equal=values_.find("=",beginPair+1);
		string key = values_.substr(beginPair,equal-beginPair);
		string value = values_.substr(equal+1,endPair-equal-1);
		retVec.push_back(make_pair<string,string>(key,value));
		cout << "[ThreadArgs::getArgsVector()]\tKey(" << key << ")---value(" << value << ")"<< endl;
		beginPair=endPair+1; 
//		beginPair=values_.length()+1; 
	}
	return retVec;
}
*/
PixelHistoThreadFrameWithArgs::PixelHistoThreadFrameWithArgs(int nOfThreads){
//  if(nOfThreads>1){
//    nOfThreads=1; 
//  }
  nOfThreads_= nOfThreads;
  for(int i=0; i<nOfThreads_; ++i){
    pThreads_.push_back(0);
    pThreadArgs_.push_back(0);
    funcRunning_.push_back(true);//It is better to assume that the functions are running
	}
  threadsRun_=false;
} 


PixelHistoThreadFrameWithArgs::~PixelHistoThreadFrameWithArgs(){
  //	cout << "[PixelHistoThreadFrameWithArgs::~PixelHistoThreadFrameWithArgs()]" << endl;
  stopThreads();  
}

/////////////////////////////////////////////////////////////
// threaded methods calling user functions: 


void PixelHistoThreadFrameWithArgs::thread0(void* arg){
  // thread function which calls user specified action Func0
  TThread::SetCancelOn();
  TThread::SetCancelDeferred();
  PixelHistoThreadFrameWithArgs* inst = (PixelHistoThreadFrameWithArgs*) ((ThreadArgs*) arg)->getFrame();
	cout << "[PixelHistoThreadFrameWithArgs::thread0()]\t" << inst << endl;
	int value = (int)((ThreadArgs*) arg)->getIndex();
  int threadId = TThread::SelfId(); // get pthread id
  TThread::Lock();
  cout << "[PixelHistoThreadFrameWithArgs::thread0()]\tThread " << value << ", id:" << threadId << " is running..\n"<<endl;
  TThread::UnLock();
  while(inst->getThreadsRun()){
    // loop keeps thread alive...
    TThread::CancelPoint();
    inst->userFunc0(value); // call the user defined threaded function
  }
}

//void PixelHistoThreadFrameWithArgs::thread1(void* arg){
//  // thread function which calls user specified action Func0
//  TThread::SetCancelOn();
//  TThread::SetCancelDeferred();
//  PixelHistoThreadFrameWithArgs* inst = (PixelHistoThreadFrameWithArgs*) arg;
//  int threadId = TThread::SelfId(); // get pthread id
//  cout << "[PixelHistoThreadFrameWithArgs::thread1()]\tThread 1, id:" << threadId << " is running..\n"<<endl;
//  while(inst->getThreadsRun()){
//	// loop keeps thread alive...
//	  TThread::CancelPoint();
//	  inst->userFunc1(); // call the user defined threaded function
//  }
//}


///////////////////////////////////////////////////////////////////
// default user functions running within threaded methods, overwrite these:

void PixelHistoThreadFrameWithArgs::userFunc0(int &threadNumber){ 
  funcRunning_[0] = true;
  TThread::Lock();
  cout << "[PixelHistoThreadFrameWithArgs::userFunc0()]\tDefault Threadfunc " << threadNumber << " running, please overwrite in derived class"<<endl;
  TThread::UnLock();
  gSystem->Sleep(1000);
  funcRunning_[0] = false;
}



//void PixelHistoThreadFrameWithArgs::userFunc1(){ 
//	funcRunning_[1] = true;
//  cout << "[PixelHistoThreadFrameWithArgs::userFunc1()]\tDefault Threadfunc 1 running, please overwrite in derived class"<<endl;
//  gSystem->Sleep(2000);
//	funcRunning_[1] = false;
//}

/////////////////////////////

int PixelHistoThreadFrameWithArgs::startThreads(){
  // start all threads
  threadsRun_=true;
  int threadNumber = 0;
  for(vector<TThread*>::iterator it=pThreads_.begin(); it != pThreads_.end(); ++it){
    if(!(*it)){
//      if(threadNumber == 0){
	      stringstream threadName;
	      threadName << "thread" << threadNumber;
				pThreadArgs_.push_back(new ThreadArgs(this,threadNumber));
	      (*it)= new TThread(threadName.str().c_str(),
			                     (void(*) (void *))&thread0,
			                     (void*) pThreadArgs_[pThreadArgs_.size()-1]);
//      }
      if(*it){
	      (*it)->Run();
      }
      else{
	      return 1;
      }
    }
    if(++threadNumber>=nOfThreads_) return 0;
  }
  return 0;
}


int PixelHistoThreadFrameWithArgs::stopThreads(){
  // stop all active threads 
  threadsRun_= false;	// aborting flag
  int threadNumber=0;

  for(vector<TThread*>::iterator it=pThreads_.begin(); it != pThreads_.end(); it++){
    if((*it)){
      int timeout = 0;
      while(funcRunning_[threadNumber] && timeout < 100){
	      timeout++;
	      gSystem->Sleep(10); // wait a while for threads to halt
      }
      TThread::Delete(*it);
      delete *it;
      *it = 0;
    }
    if (++threadNumber>=nOfThreads_) return 0;
  }
  return 0;
} 

