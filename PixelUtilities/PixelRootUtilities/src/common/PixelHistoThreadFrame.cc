#include "PixelUtilities/PixelRootUtilities/include/PixelHistoThreadFrame.h"
#include <iostream>
#include <sstream>
#include <TThread.h> 
#include <TROOT.h>

using namespace std;

PixelHistoThreadFrame::PixelHistoThreadFrame(int nOfThreads){
  nOfThreads_= nOfThreads;
  for(int i=0; i<nOfThreads_; ++i){
    pThreads_.push_back(0);
    funcRunning_.push_back(true);//It is better to assume that the functions are running
	}
  threadsRun_=false;
} 


PixelHistoThreadFrame::~PixelHistoThreadFrame(){
  //	cout << "[PixelHistoThreadFrame::~PixelHistoThreadFrame()]" << endl;
  stopThreads();  
}

/////////////////////////////////////////////////////////////
// threaded methods calling user functions: 


void PixelHistoThreadFrame::thread0(void* arg){
  // thread function which calls user specified action Func0
  TThread::SetCancelOn();
  TThread::SetCancelDeferred();
  PixelHistoThreadFrame* inst = (PixelHistoThreadFrame*) arg;
  int threadId = TThread::SelfId(); // get pthread id
  TThread::Lock();
  cout << "[PixelHistoThreadFrame::thread0()]\tThread 0, id:" << threadId << " is running..\n"<<endl;
  TThread::UnLock();
  while(inst->getThreadsRun()){
    // loop keeps thread alive...
    TThread::CancelPoint();
    inst->userFunc0(); // call the user defined threaded function
  }
}

//void PixelHistoThreadFrame::thread1(void* arg){
//  // thread function which calls user specified action Func0
//  TThread::SetCancelOn();
//  TThread::SetCancelDeferred();
//  PixelHistoThreadFrame* inst = (PixelHistoThreadFrame*) arg;
//  int threadId = TThread::SelfId(); // get pthread id
//  cout << "[PixelHistoThreadFrame::thread1()]\tThread 1, id:" << threadId << " is running..\n"<<endl;
//  while(inst->getThreadsRun()){
//	// loop keeps thread alive...
//	  TThread::CancelPoint();
//	  inst->userFunc1(); // call the user defined threaded function
//  }
//}


///////////////////////////////////////////////////////////////////
// default user functions running within threaded methods, overwrite these:

void PixelHistoThreadFrame::userFunc0(){ 
  funcRunning_[0] = true;
  TThread::Lock();
  cout << "[PixelHistoThreadFrame::userFunc0()]\tDefault Threadfunc 0 running, please overwrite in derived class"<<endl;
  TThread::UnLock();
  gSystem->Sleep(1000);
  funcRunning_[0] = false;
}



//void PixelHistoThreadFrame::userFunc1(){ 
//	funcRunning_[1] = true;
//  cout << "[PixelHistoThreadFrame::userFunc1()]\tDefault Threadfunc 1 running, please overwrite in derived class"<<endl;
//  gSystem->Sleep(2000);
//	funcRunning_[1] = false;
//}

/////////////////////////////

int PixelHistoThreadFrame::startThreads(){
  // start all threads
  threadsRun_=true;
  int threadNumber = 0;
  for(vector<TThread*>::iterator it=pThreads_.begin(); it != pThreads_.end(); ++it){
    if(!(*it)){
//      if(threadNumber == 0){
	      stringstream threadName;
	      threadName << "thread" << threadNumber;
	      (*it)= new TThread(threadName.str().c_str(),
			                     (void(*) (void *))&thread0,
			                     (void*) this);
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


int PixelHistoThreadFrame::stopThreads(){
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

