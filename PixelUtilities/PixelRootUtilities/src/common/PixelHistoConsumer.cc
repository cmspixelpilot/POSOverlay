#include "PixelUtilities/PixelRootUtilities/include/PixelHistoConsumer.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelHistoTransactionKeywords.h"
#include <TThread.h>
#include <iostream>
#include <TROOT.h>

////////////////////////////
#include "TSocket.h"
#include "TServerSocket.h"
#include "TMonitor.h"
#include "TMessage.h"
#include "TList.h"

#include <TProfile.h>
#include <TH1F.h>
#include <TH2F.h>
/////////////////////////////

using namespace std;
using namespace pixelHistoTransactionKeywords;

PixelHistoConsumer::PixelHistoConsumer():PixelHistoThreadFrame(){
  cout << "[PixelHistoConsumer::PixelHistoConsumer()]" << endl;
  init();
}

void PixelHistoConsumer::init(){
  ds=0; fMon=0; allNames=0;
}


PixelHistoConsumer::~PixelHistoConsumer(){
  //Threads must be stopped first otherwise, while still running, they can use pointers that
  //we want to delete in the destroy method
  stopThreads();  
  destroy();
}

void PixelHistoConsumer::destroy(){
  // Clean up

  if(allNames){
    allNames->Delete();
    delete allNames;
    allNames = 0;
  }
  
  if(fMon){
    fMon->RemoveAll();
    delete fMon;
    fMon = 0;
  }

  if(ds){
    ds->Close();
    delete ds;
    ds = 0;
  }

}

int PixelHistoConsumer::startThreads(){
  int retVal = PixelHistoThreadFrame::startThreads();
  return retVal;
}

int PixelHistoConsumer::stopThreads(){
  int retVal = PixelHistoThreadFrame::stopThreads();
  return retVal;
}

void PixelHistoConsumer::userFunc0(void){
  string mthn = "[PixelHistoConsumer::userFunc0()]\t";
  PixelHistoThreadFrame::funcRunning_[0] = true;
  
  // If no connection, try to Open connection to dispatcher. If fails, sleep.
  if(!fMon){
    
    ds= new TSocket("localhost", 9090);
   
    if(ds->IsValid()){
      fMon  = new TMonitor;
      fMon->Add(ds); //add dispatcher socket to listen for requests

      TThread::Lock();
      cout << mthn << "Made connection to Dispatcher: " << ds->GetInetAddress().GetHostName() << endl;
      TThread::UnLock();

      consumerRefresh(); //initially acquire list of files from Dispatcher
    }
    else{
      ds->Close(); //clean up failed socket
      delete ds;
      ds = 0;
      usleep(1000000);
    }
  }
  else{
    
    usleep(1000000); //sleep if not looking for Dispatcher(dont sleep infinitely because
    //if connection is lost with the Dispatcher the search thread re-starts
    


    //if(!isRefreshing){ // dont check sockets if updating
    // Check if there is a message waiting on one of the sockets.
    // Wait not longer than 20ms (returns -1 in case of time-out).
    //   TSocket *s;
    
    //       if ((s = fMon->Select(20)) != (TSocket*)-1)
    // 	HandleSocket(s);
    // }
  }

  PixelHistoThreadFrame::funcRunning_[0] = false;
}

void PixelHistoConsumer::HandleSocket(TSocket *s){ //NOTE: never called...
  string mthn = "[PixelHistoConsumer::HandleSocket()]\t";
  
  char request[64];
  	
  if (s->Recv(request, sizeof(request)) <= 0) {
    fMon->RemoveAll(); //there should only be ONE dispatcher!!!
    delete fMon;
    fMon = 0;

    TThread::Lock();
    cout << mthn << "Closed connection from Dispatcher: " << s->GetInetAddress().GetHostName() << endl;
    TThread::UnLock();

    ds->Close();  //s and ds must be same since only ONE dispatcher!!!
    delete ds;
    ds = 0;
    return;
  }
  cout << mthn << "recv: " << request << endl;
	
  return;
}

int PixelHistoConsumer::consumerRefresh(){
  string mthn =  "[PixelHistoConsumer::ConsumerRefresh()]\t";

  if(!fMon){ //no dispatcher present
    cout << mthn << "No dispatcher present." << endl;
    return -1;
  }

  isRefreshing = true; //theoretically could be used to lock receive thread while refreshing file list, but not working

  
  if(!ds->IsValid()){
    isRefreshing = false;
    return -1;
  }

  ds->Send(consumerRefreshTK); //initiates refresh exchange with dispatcher
  
  cout << mthn << "Requesting Refresh...\n";

  if (fMon->Select(5000) == (TSocket*)-1){ //give up after 5s if nothing received
    cout << mthn << "Refresh time out.\n";

    
    isRefreshing = false;
    return -1;
  }

  //receive TList of all names
  TMessage *refresh;
  if (ds->Recv(refresh) <= 0) {
    cout << mthn << "Error receiving refresh list.\n";

    //End connection with lost Dispatcher
    fMon->RemoveAll(); //there should only be ONE dispatcher!!!
    delete fMon;
    fMon = 0;

    TThread::Lock();
    cout << mthn << "Dispatcher not found" << endl;
    TThread::UnLock();

    ds->Close(); 
    delete ds;
    ds = 0;

    //Attempt to Reconnect with Dispatcher
    ds= new TSocket("localhost", 9090);
   
    if(ds->IsValid()){ //refound Dispatcher
      fMon  = new TMonitor;
      fMon->Add(ds); //add dispatcher socket to listen for requests

      TThread::Lock();
      cout << mthn << "Made connection to Dispatcher: " << ds->GetInetAddress().GetHostName() << endl;
      TThread::UnLock();
       
      ds->Send(consumerRefreshTK); //re-initiates refresh exchange with dispatcher
      if (ds->Recv(refresh) <= 0) {
	//End connection with lost Dispatcher -- GIVE UP...
	fMon->RemoveAll(); //there should only be ONE dispatcher!!!
	delete fMon;
	fMon = 0;
	ds->Close(); //clean up failed socket
	delete ds;
	ds = 0;
	isRefreshing = false;
	return -1;
      }
    }
    else{ //give up and return
      ds->Close(); //clean up failed socket
      delete ds;
      ds = 0;
      isRefreshing = false;
      return -1;
    }   
  }

  cout << mthn << "Received TMessage" << endl;

  if(allNames){
    allNames->Delete();
    delete allNames;
    allNames=0;
  }

  allNames = (TList*)refresh->ReadObject(refresh->GetClass());

  cout << mthn << "Histograms: " << endl;
  TObject * obj;
  TIter next(allNames);
  while ( (obj = next())){
    cout << mthn << "    " <<  obj->GetName() << endl;
  }

  isRefreshing = false;
  return 0;

}

TMessage *PixelHistoConsumer::requestHisto(TString hs){
  string mthn =  "[PixelHistoConsumer::requestHisto()]\t";

  TIter it(allNames); //point to first name 
  hs = getTK + (it()->GetName()) + hs;
  //hs = getTK + hs;
  ds->Send(hs);

  TMessage *m;
  if (ds->Recv(m) <= 4) { //size of null message is 4 bytes
    cout << mthn << "Error receiving request from Dispatcher." << endl;
    return 0;
  }
  else if(m->GetClass() == 0){
    cout << mthn << "Error receiving request from Dispatcher." << endl;
    return 0;
  } 

  return m;

}
