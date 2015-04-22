#include "PixelUtilities/PixelRootUtilities/include/PixelHistoDispatcher.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelHistoTransactionKeywords.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelHistoProducerDescriptor.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelHistoScanDirectory.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelHistoReadFileElements.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelHistoReadWriteFile.h"
#include <TThread.h>
#include <iostream>
#include <TROOT.h>
#include <TObjString.h>
#include <cstdlib>
#include <algorithm>

////////////////////////////
#include <TSocket.h>
#include <TServerSocket.h>
#include <TMonitor.h>
#include <TMessage.h>
#include <TList.h>
#include <TProfile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TTree.h>
#include "TClass.h"
#include "TKey.h"
/////////////////////////////

#define PORT 9080

using namespace std;
using namespace pixelHistoTransactionKeywords;

////////////////////////////////////////////////////////////////////////
PixelHistoDispatcher::PixelHistoDispatcher():PixelHistoThreadFrame(){
//  cout << "[PixelHistoDispatcher::PixelHistoDispatcher()]" << endl;
	uCommandDone_ = true; //no pending command, so all are complete
	uCommandReq_  = -1;		//no pending command
	uReturnList_  = 0;		//no pending command
	className_    = "PixelHistoDispatcher";
	rFE_          = new PixelHistoReadFileElements();
  init();
}

////////////////////////////////////////////////////////////////////////
void PixelHistoDispatcher::init(){
  // Open a server socket looking for connections on a named service or
  // on a specified port
	int port = PORT;
	if( getenv("PORT") ){
		port = atoi(getenv("PORT"))+1000;
	}
//	cout << "[PixelHistoDispatcher::init()]\tport" << port << endl;
  serverSocket_ = new TServerSocket(port, kTRUE);
	if(!serverSocket_->IsValid()){
	  cout << "[PixelHistoDispatcher::init()]\t Can't create a Server Socket! Exiting..." << endl;
		exit(1);
  }
  // Add server socket to monitor so we are notified when a client needs to be
  // accepted
  socketMonitor_  = new TMonitor;
  socketMonitor_->Add(serverSocket_);

  // Create a map with key corresponding to Producer ip and port which 
	// contains socket, Producer files, and file content
  prodMap_ = new map<TString,PixelHistoProducerDescriptor*>();
}


////////////////////////////////////////////////////////////////////////
PixelHistoDispatcher::~PixelHistoDispatcher(){
  //Threads must be stopped first otherwise, while still running, they can use pointers that
  //we want to delete in the destroy method
  stopThreads();  
  destroy();
}

////////////////////////////////////////////////////////////////////////
void PixelHistoDispatcher::destroy(){
  // Clean up
  
  if(prodMap_){
    delete prodMap_;
    prodMap_ = 0;
  }

  if(socketMonitor_){
    socketMonitor_->RemoveAll();
    delete socketMonitor_;
    socketMonitor_ = 0;
  }

  if(serverSocket_){
    serverSocket_->Close();
    delete serverSocket_;
    serverSocket_ = 0;
  }
	delete rFE_;
}

////////////////////////////////////////////////////////////////////////
int PixelHistoDispatcher::startThreads(){
  int retVal = PixelHistoThreadFrame::startThreads();
  return retVal;
}

////////////////////////////////////////////////////////////////////////
int PixelHistoDispatcher::stopThreads(){
  int retVal = PixelHistoThreadFrame::stopThreads();
  return retVal;
}

////////////////////////////////////////////////////////////////////////
void PixelHistoDispatcher::userFunc0(void){
  PixelHistoThreadFrame::funcRunning_[0] = true;
  string mthn ="[PixelHistoDispatcher::userFunc0()]\t";
  
  // Check if there is a message waiting on one of the sockets.
  // Wait not longer than 20ms (returns -1 in case of time-out).
  TSocket *s;
  if ((s = socketMonitor_->Select(20)) != (TSocket*)-1){
    HandleSocket(s);
	}
	else if(!uCommandDone_ && uCommandReq_ >= 0){ //new client command
		switch(uCommandReq_){
		
			case U_COMPLETE_LIST:
				uReturnList_ = getCompleteFileList(uForceRefreshParam_);
				break;
				
			case U_FILE_CONTENT:
				uReturnList_ = getFileContent(uFileParam_,uForceRefreshParam_);
				break;
		
			case U_HISTOGRAM:
				uReturnMessage_ = getHistogram(uFileParam_);
				break;
		
			default:
				uReturnList_ = 0;
				uReturnMessage_ = 0;
		}
		uCommandReq_ = -1;
		uCommandDone_ = true;			
	}
	
  PixelHistoThreadFrame::funcRunning_[0] = false;
}

////////////////////////////////////////////////////////////////////////
TList* PixelHistoDispatcher::uGetCompleteFileList(bool forceRefresh,string dirName){
//  string mthn = "[PixelHistoDispatcher::uGetCompleteFileList()]\t";
	uDirName_ = dirName;
	uCommandReq_ = U_COMPLETE_LIST;
	uForceRefreshParam_ = forceRefresh;
	uCommandDone_= false;

	while(!uCommandDone_)usleep(20); //wait for thread to complete command
	
	return uReturnList_;
}

////////////////////////////////////////////////////////////////////////
TList* PixelHistoDispatcher::uGetFileContent(TString file, bool forceRefresh){
	uFileParam_ = file;
	uCommandReq_ = U_FILE_CONTENT;
	uForceRefreshParam_ = forceRefresh;
	uCommandDone_= false;

	while(!uCommandDone_)usleep(20); //wait for thread to complete command
	
	return uReturnList_;
}

////////////////////////////////////////////////////////////////////////
TObject* PixelHistoDispatcher::uGetHistogram(TString histoName){
	uCommandReq_ = U_HISTOGRAM;
	uFileParam_ = histoName;
	uCommandDone_= false;

	while(!uCommandDone_)usleep(20); //wait for thread to complete command
	
	return uReturnMessage_;
}

////////////////////////////////////////////////////////////////////////
void PixelHistoDispatcher::HandleSocket(TSocket *s){
  string mthn = "[PixelHistoDispatcher::HandleSocket()]\t";
	static int requestLengthTK =2000;
//	cout << mthn << endl;
	
  char request[requestLengthTK];
  if (s->IsA() == TServerSocket::Class()) {
    // accept new connection
    TSocket *sock = ((TServerSocket*)s)->Accept();
		//request identification
	  sock->Send(identifyTK);
		socketMonitor_->Add(sock);
		TSocket *waitSocket;

		if((waitSocket = socketMonitor_->Select(20)) != sock){ 
			closeSocket(sock);
			return;
		}

		if (sock->Recv(request, sizeof(request)) <= 0) {
			closeSocket(sock);
			return;
		}
		
		if (request == producerIdentifierTK) {
		
		  TThread::Lock();
  	  cout << mthn << "\n\tAccepted connection from Producer: " << sock->GetInetAddress().GetHostName() << " " << sock->GetPort() << endl;
    	TThread::UnLock();
			
			addProducerToMap(sock);
			
			
// 	usage examples:
//--------------------------------------------
//			getCompleteFileList();
//
// 			getFileList(createProducerName(sock));
// 			
// 			getFileContent(createProducerName(sock)+"/" + "root");
// 			
// 			getFileContent(createProducerName(sock)+"/" + "suca.root");
// 			
// 			getHistogram(createProducerName(sock) + "/root:/histoDir/hpx");
//--------------------------------------------	

	
			sock->Send(enableAutoRefreshTK);
			
			//for(int i=0;i<10;++i)
			//getCompleteFileList(true);
			
			return;
		}
		
		TThread::Lock();
  	cout << mthn << "Unrecognized ID - closing connection." << endl;
    TThread::UnLock();
		closeSocket(sock);
		return;
  }
  else{ 
    //handle incoming command    
    if (s->Recv(request, sizeof(request)) <= 0) { //check if connection closed by other side
      socketMonitor_->Remove(s);

      TThread::Lock();
      cout << mthn << "Closed connection from " << s->GetInetAddress().GetHostName() << " " << s->GetPort() << endl;
      TThread::UnLock();

      prodMap_->erase(createProducerName(s)); //erase the disconnected producer
      closeSocket(s);
      return;
    }

		TString requestString(request);
//    cout << mthn << "recv: " << requestString << endl;
    
    if(requestString == producerUpdateFileTK){
      getFileList(createProducerName(s),true,false);
		}
    if(requestString(0,producerUpdateContentTK.Length()) == producerUpdateContentTK){
			requestString.Remove(requestString.Index(producerUpdateContentTK), producerUpdateContentTK.Length());
      getFileContent(requestString,true,false);
		}
    else if(requestString == consumerRefreshTK){ //only needed if TCP/IP connected Consumer
      consumerRefresh(s);
		}
    else if (requestString(0,getTK.Length()) == getTK){//only needed if TCP/IP connected Consumer
			getHistogram(createProducerName(s) + requestString);
    }
  }

  return;
}

////////////////////////////////////////////////////////////////////////
void PixelHistoDispatcher::displayProdMap(){
  string mthn = "[PixelHistoDispatcher::displayProdMap()]\t";

  map<TString,PixelHistoProducerDescriptor*>::iterator it;

	TString fullFileName;
	int i;
  cout << mthn << "Producer Map:" << endl;
  if(prodMap_->begin() == prodMap_->end())
    cout << mthn << "\t<empty>\n";
  for (i=1,it=prodMap_->begin(); it != prodMap_->end(); it++,++i){
    cout << mthn << i << ": " << it->second->getName() << endl;
  
		if(it->second->getFileList() == 0){
			cout << mthn << "\tEmpty file list." << endl;
		}
		else{
  		TObject * obj;
  		TIter next(it->second->getFileList());
  		while ( (obj = next())){
  		  cout << mthn << "\t" <<  obj->GetName() << endl;
				fullFileName = it->second->getName()+"/"+obj->GetName();
				if(it->second->getContentMap() && it->second->getContentList(fullFileName)){ 
					
					TObject * contentObj;
					TIter contentNext(it->second->getContentList(fullFileName));
  				while ( (contentObj = contentNext())){
  				  cout << mthn << "\t\t" <<  contentObj->GetName() << endl;
  				}
				}
				else{
					cout << mthn << "\t\tNo content." << endl;
				}			
			}
		}
  }
}

////////////////////////////////////////////////////////////////////////
void PixelHistoDispatcher::addProducerToMap (TSocket *s){
	//update producer map
  TString nm =  createProducerName(s);

	//with empty file list
  PixelHistoProducerDescriptor *pd = new PixelHistoProducerDescriptor(nm,0,s);

  prodMap_->erase(nm);//erase if already in producer list
  (*prodMap_)[nm] = pd;
}

////////////////////////////////////////////////////////////////////////
TString PixelHistoDispatcher::createProducerName(TSocket *s){
	return TString::Format("%s_%d",
				(char *)s->GetInetAddress().GetHostName(), 
				s->GetPort());
}

////////////////////////////////////////////////////////////////////////
TList* PixelHistoDispatcher::getProducerList(){
	TList* producers = new TList();
 
	map<TString,PixelHistoProducerDescriptor*>::iterator prodIt = prodMap_->begin();
	while(prodIt != prodMap_->end()){
		producers->Add(new TObjString(prodIt->first));
		prodIt++;
	}
	
	return producers;
}

////////////////////////////////////////////////////////////////////////
TList* PixelHistoDispatcher::getCompleteFileList(bool forceRefresh){
	string mthn = "[PixelHistoDispatcher::getCompleteFileList()]\t";
//  cout << mthn << endl;
	
	TList* completeFileList = new TList();
 
	if(uDirName_ == "" || uDirName_ == "fileRoot"){

    map<TString,PixelHistoProducerDescriptor*>::iterator it;

	  int i;   
    for (i=1,it=prodMap_->begin(); it != prodMap_->end(); it++,++i){		
		  getFileList(it->first,forceRefresh);	
		  if(it->second->getFileList() != 0){
  		  TObject * obj;
  		  TIter next(it->second->getFileList());
  		  while ( (obj = next())){	
				  completeFileList->Add(new TObjString(it->first + "//" + obj->GetName()));
			  }
		  }
    }
	}
/*This has been moved from the PixelHistoServer since the disk where all the data
are stored is shared*/
	PixelHistoScanDirectory sD;
	vector<string> v;
	string theAppend = className_+"/";
	if(uDirName_ == ""){
	  v = sD.ls();
	}
  else if(uDirName_ == "fileRoot"){
  	v = sD.ls(true,false);
	}
	else{
  	sD.ls(v,uDirName_,false,".root");
		theAppend = "";
	}
	std::sort(v.rbegin(), v.rend());

  for(vector<string>::iterator it=v.begin(); it != v.end(); it++){
//  	cout << mthn << it->c_str() << endl;
  	completeFileList->Add(new TObjString(TString(theAppend.c_str()) + it->c_str()));
	}
	return completeFileList;
}

////////////////////////////////////////////////////////////////////////
//user should NOT delete returned list. The dispatcher is using it.
TList* PixelHistoDispatcher::getFileList (TString prodName, bool forceRefresh, bool initiate){
  string mthn = "[PixelHistoDispatcher::getFileList()]\t";
//  cout << mthn << prodName << endl;
	
	map<TString,PixelHistoProducerDescriptor*>::iterator prodIt = prodMap_->find(prodName);
 
  if(prodIt == prodMap_->end()){
		cout << mthn << "Producer name not found:" << prodName << endl;
		return 0;
	}
	
	if(!forceRefresh && prodIt->second->getFileList() != 0){//check if already cached
		return prodIt->second->getFileList();
	}
	
	TSocket *s = prodIt->second->getSocket();
	
	if(initiate){ //if Dispatcher needs to initiate exchange
		s->Send(requestFileListTK);
	}
	
	TMessage *update;
	int recvSize;
  if ( (recvSize = s->Recv(update)) <= 4 || update->What() == 3){  //3 is TString(how to display?)
    cout << mthn << "Error receiving update list." << endl;
    return 0;
  }
		
  TList *fileList = (TList*)update->ReadObject(update->GetClass());

	prodIt->second->setFileList(fileList);
	
	return fileList; 
}

////////////////////////////////////////////////////////////////////////
//user should NOT delete returned list. The dispatcher is using it.
TList* PixelHistoDispatcher::getFileContent(TString prodAndFileName, bool forceRefresh, bool initiate){
	string mthn = "[PixelHistoDispatcher::getFileContent()]\t";
//  cout << mthn << prodAndFileName << endl;
	
	TString prodName = prodAndFileName(0,prodAndFileName.Index('/'));
	TString file = prodAndFileName(prodAndFileName.Index('/')+1,prodAndFileName.Length() - prodAndFileName.Index('/') - 1);
	
	
	if(prodName == className_){
	  PixelHistoReadWriteFile rWF;
		int pos;
		TString fileName, path;
	  if((pos = file.Index(":/")) >= 0){ //path within file
		  fileName = file(0,pos);
		  path = file(pos+1,file.Length() - pos - 2);
	  }else{
      fileName=file;
		  path="/";
    }
		rWF.open(fileName.Data(),"READ");
    //read the current directory into TList
	  rFE_->read(path.Data(),1);
		TList *contentList = &rFE_->getAllNames();
  	rWF.close();
		return contentList;
	}
	map<TString,PixelHistoProducerDescriptor*>::iterator prodIt = prodMap_->find(prodName);
 
  if(prodIt == prodMap_->end()){
		cout << mthn << "Producer name not found: " << prodName << endl;
		return 0;
	}
	
	//check if already cached
	if(!forceRefresh && prodIt->second->getContentMap() && prodIt->second->getContentList(prodAndFileName)){
		return prodIt->second->getContentList(prodAndFileName);
	}
	
	TSocket *s = prodIt->second->getSocket();
	
	if(initiate){
		s->Send(requestFileContentTK+file);
	}

	TMessage *update;
	int mLen;
  if ((mLen = s->Recv(update)) <= 0 || update->What() == 3){  //3 is TString(how to display?)
    cout << mthn << "Error receiving update list: " << prodAndFileName << endl;
    return 0;
  }
	
	TList *contentList = (TList*)update->ReadObject(update->GetClass());
 
  prodIt->second->setContentList(prodAndFileName,contentList);

//	cout << mthn << "Content List Received. " << mLen << " bytes" << endl;
	return contentList; 
}

////////////////////////////////////////////////////////////////////////
void PixelHistoDispatcher::consumerRefresh(TSocket *s){
  string mthn = "[PixelHistoDispatcher::ConsumerRefresh()]\t";

  cout << mthn << endl;

  TMessage refresh(kMESS_OBJECT); //create object to send
  TList all;
 
  if(prodMap_->begin() == prodMap_->end())
    all.Add ( new TObjString("<none available>") );
  else{
    map<TString,PixelHistoProducerDescriptor*>::iterator it;
    for ( it=prodMap_->begin() ; it != prodMap_->end(); it++ ){
      TObject * obj;
      TIter next((*it).second->getFileList());
      all.Add ( new TObjString((*it).second->getName() + "/") );
      while ( (obj = next()))
      all.Add ( new TObjString((*it).second->getName() + "/" + obj->GetName()) );
    }
  }


  refresh.WriteObject(&all);
  s->Send(refresh);

  all.Delete(); //clean up TObjStrings in list
}

////////////////////////////////////////////////////////////////////////
//user should delete TMessage * when done with it
TObject* PixelHistoDispatcher::getHistogram(TString ts){
  string mthn = "[PixelHistoDispatcher::getHistogram()]\t";

	int slashIndex;
  if(ts.Length() == 0 || (slashIndex = ts.Index("/")) < 0 ||slashIndex == ts.Length()-1){  //invalid request string
    cout << mthn << "Invalid request received" << endl;
    return 0;
  }

  TString nm = ts;
  nm.Resize(nm.Index("/")); //nm holds name of producer

	if(nm == className_){
	  PixelHistoReadWriteFile rWF;
		int pos;
		TString fileName, objectName;
		ts = ts.Remove(0,ts.Index("/")+1);
	  if((pos = ts.Index(":/")) >= 0){ //path within file
		  fileName = ts(0,pos);
		  objectName = ts(pos+1,ts.Length() - pos - 1);
    }
//		cout << mthn << "file: " << fileName << " obj: " << objectName << endl;
		rWF.open(fileName.Data(),"READ");
	  TObject * tmpObj = gDirectory->Get(objectName.Data());
		gROOT->cd();
//		cout << mthn << "ClassName:" << tmpObj->ClassName() << endl;
    TObject * obj = 0;
		if(tmpObj != 0 ){
  		if(TString(tmpObj->ClassName()).Index("TH") >= 0 || TString(tmpObj->ClassName()).Index("TProfile") >=0){
				obj = (TH1*)tmpObj->Clone();//->DrawClone();
			}
			else if(TString(tmpObj->ClassName()).Index("TGraph") >= 0){
	  		obj = (TGraph*)tmpObj->Clone();//->DrawClone();
		  }
			else if(TString(tmpObj->ClassName()).Index("TCanvas") >= 0){
          //THIS DOESN'T WORK WELL BECAUSE IT CREATES OBJECTS IN MEMORY THAT i DON'T KNOW 
					// HOW TO DELETE
//				gDirectory->Append((TCanvas*)tmpObj);
//				obj = gDirectory->FindObjectAny(tmpObj->GetName());
				obj = ((TCanvas*)tmpObj)->DrawClone();
		  }
			else if(TString(tmpObj->ClassName()).Index("TTree") >= 0){
			  ((TTree*)tmpObj)->LoadBaskets();
	  		obj = (TTree*) tmpObj->Clone();//->DrawClone();
		  }
		  rWF.close();
      return obj;
    }
		else{
	    cout << mthn << "Couldn't find " << objectName << endl;
		  rWF.close();
			return 0;
		}
	}
	  
  ts = getTK + ts.Remove(0,ts.Index("/")+1); //ts holds request to forward to producer
  //find requested producer TSocket
  map<TString,PixelHistoProducerDescriptor*>::iterator it = prodMap_->find(nm);
  if(it == prodMap_->end()){ //Producer not found
  	cout << mthn << "Producer not found" << endl;
    return 0;
  }
	
  TSocket *ps = it->second->getSocket();
  TMessage *rm; //receive message
  int rm_len;
 
  ps->Send(ts);
 
  if ((rm_len=ps->Recv(rm)) <= 4 || rm->What() == 3) { //type 3 is string, 4 is returned my a "null send" from the producer
    cout << mthn << "Error receiving request from Producer." << endl;
    return 0;
  }
 
//  cout << mthn << "Response from Producer: type " << rm->What() << ", " << rm_len << " bytes" << endl;
 
  return rm;
}
void PixelHistoDispatcher::closeSocket(TSocket *s){
	socketMonitor_->Remove(s);
	s->Close();
	delete s;
}
