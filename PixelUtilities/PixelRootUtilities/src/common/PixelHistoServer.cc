#include "PixelUtilities/PixelRootUtilities/include/PixelHistoServer.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelHistoTransactionKeywords.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelHistoReadFileElements.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelHistoReadWriteFile.h"
//#include "PixelUtilities/PixelRootUtilities/include/PixelHistoScanDirectory.h"

#include <TThread.h>
#include <iostream>
#include <TROOT.h>
#include <TObjString.h>
////////////////////////////
#include <TSocket.h>
#include <TMonitor.h>
#include <TMessage.h>
#include <TList.h>
#include <TDirectory.h>

#include <TProfile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TFile.h>
/////////////////////////////
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <stdlib.h>

using namespace std;
using namespace pixelHistoTransactionKeywords;

#define DEST_IP   "localhost.localdomain"//"ppdnwpixelsoft.fnal.gov"//"localhost.localdomain"
#define DEST_PORT 9080


////////////////////////////////////////////////////////////////////////
PixelHistoServer::PixelHistoServer():PixelHistoThreadFrame(){
//  cout << "[PixelHistoServer::PixelHistoServer()]" << endl;
  dispatcherSocket_ = 0; 
  socketMonitor_    = 0;
  autoRefresh_      = false;
  init();
}

////////////////////////////////////////////////////////////////////////
void PixelHistoServer::init(){
}


////////////////////////////////////////////////////////////////////////
PixelHistoServer::~PixelHistoServer(){
  //  cout << "[PixelHistoServer::~PixelHistoServer()]\tDistructor called..." << endl;
  //Threads must be stopped first otherwise, while still running, they can use pointers that
  //we want to delete in the destroy method
  stopThreads();  
  destroy();
  //  cout << "[PixelHistoServer::~PixelHistoServer()]\tThis is the end..." << endl;
}

////////////////////////////////////////////////////////////////////////
void PixelHistoServer::destroy(){
  // Clean up

  if(socketMonitor_){
    socketMonitor_->RemoveAll();
    delete socketMonitor_;
    socketMonitor_ = 0;
  }

  if(dispatcherSocket_){
    dispatcherSocket_->Close();
    delete dispatcherSocket_;
    dispatcherSocket_ = 0;
  }
}

////////////////////////////////////////////////////////////////////////
int PixelHistoServer::startThreads(){
  int retVal = PixelHistoThreadFrame::startThreads();
  return retVal;
}

////////////////////////////////////////////////////////////////////////
int PixelHistoServer::stopThreads(){
  int retVal = PixelHistoThreadFrame::stopThreads();
  return retVal;
}

////////////////////////////////////////////////////////////////////////
void PixelHistoServer::userFunc0(void){
  string mthn = "[PixelHistoServer::userFunc0()]\t";

  PixelHistoThreadFrame::funcRunning_[0] = true;

  // If no connection, try to Open connection to dispatcher. If fails, sleep.
  if(!socketMonitor_){
		if(pingDispatcher() != -1){
      cout << mthn << "We are trying to connect to default localhost...must be changed!" << endl;
			int port = DEST_PORT;
			if( getenv("PORT") ){
				port = atoi(getenv("PORT"))+1000;
			}
//			cout << "[PixelHistoServer::userFunc0()]\tport" << port << endl;
 		  dispatcherSocket_= new TSocket(DEST_IP, port);
	  	if(dispatcherSocket_->IsValid()){
    	  socketMonitor_  = new TMonitor;
    	  socketMonitor_->Add(dispatcherSocket_); //add dispatcher socket to listen for requests

    	  TThread::Lock();
    	  cout << mthn << "Made connection to Dispatcher: " << dispatcherSocket_->GetInetAddress().GetHostName() << endl;
    	  TThread::UnLock();	
    	}
    	else{
    	  dispatcherSocket_->Close(); //clean up failed socket
    	  delete dispatcherSocket_;
    	  dispatcherSocket_ = 0;
    	  usleep(sleepBetweenPing_us);
    	}
    }
		else{
		  usleep(sleepBetweenPing_us);
		}
  }
  else{
    // Check if there is a message waiting on the dispatcher socket.
    // Wait not longer than 20ms (returns -1 in case of time-out).
    if ( socketMonitor_->Select(monitorTimeout_ms) == dispatcherSocket_)
      handleSocket();
  }
  PixelHistoThreadFrame::funcRunning_[0] = false;
}

////////////////////////////////////////////////////////////////////////
void PixelHistoServer::handleSocket(){
	if(dispatcherSocket_ == 0 || !dispatcherSocket_->IsValid()){
		return;
	}
  string mthn = "[PixelHistoServer::handleSocket()]\t";
	
  static int requestLengthTK = 2000;

  char request[requestLengthTK];
  	
  if (dispatcherSocket_->Recv(request, sizeof(request)) <= 0) {
    socketMonitor_->RemoveAll(); //there should only be ONE dispatcher!!!
    delete socketMonitor_;
    socketMonitor_ = 0;

    TThread::Lock();
    cout << mthn << "Closed connection from Dispatcher: " << dispatcherSocket_->GetInetAddress().GetHostName() << endl;
    TThread::UnLock();

    dispatcherSocket_->Close(); //s and dispatcherSocket_ must be the same since there should only be ONE dispatcher!!!
    delete dispatcherSocket_;
    dispatcherSocket_ = 0;
    return;
  }
  else{
//    cout << mthn << "Request: " << request << endl;
    //Handle request by locating the histogram requested and sending to dispatcher. If
    //the request is not found then send a null message (4 bytes)
		TString requestString(request);
		if(requestString == identifyTK){
			dispatcherSocket_->Send(producerIdentifierTK);		
		}
		else if(requestString == enableAutoRefreshTK){
			autoRefresh_ = true;
			
// 			dispatcherSocket_->Send(producerUpdateFileTK);
//   		updateFileList();
// 			dispatcherSocket_->Send(producerUpdateContentTK + "root");
//   		updateFileContent("root");
		}
		else if(requestString == disableAutoRefreshTK){
			autoRefresh_ = false;
		}
		else if(requestString == requestFileListTK){
			updateFileList();
		}
		else if(requestString(0,requestFileContentTK.Length()) == requestFileContentTK){
			requestString.Remove(requestString.Index(requestFileContentTK), requestFileContentTK.Length());
			updateFileContent(requestString.Data());			
		}
		else if (requestString(0,getTK.Length()) == getTK){
	    requestString.Remove(requestString.Index(getTK),getTK.Length());
			handleHistogramRequest(requestString);
		}
		else{
  		cout << mthn << "Invalid Request: " << requestString << endl;
  		dispatcherSocket_->Send(errorTK); //send not found;
  		return;
		}
		
  }
}

////////////////////////////////////////////////////////////////////////
int PixelHistoServer::pingDispatcher(){
		string mthn = "[PixelHistoServer::pingDispatcher()]\t";

    int sockfd;
    struct sockaddr_in dest_addr;   // will hold the destination addr
    struct hostent *h;
    if ((h=gethostbyname(DEST_IP)) == NULL) {  // get the host info
		  herror("gethostbyname");
		  exit(1);
		}
		
		sockfd = socket(PF_INET, SOCK_STREAM, 0); // do some error checking!

    dest_addr.sin_family      = AF_INET;           // host byte order
			int port = DEST_PORT;
			if( getenv("PORT") ){
				port = atoi(getenv("PORT"))+1000;
			}
    dest_addr.sin_port        = htons(port);  // short, network byte order
    dest_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr)));
    memset(dest_addr.sin_zero, '\0', sizeof dest_addr.sin_zero);

    // don't forget to error check the connect()!
    int retVal = connect(sockfd, (struct sockaddr *)&dest_addr, sizeof dest_addr);
		close(sockfd);
		return retVal;
}

///////////////////////////////////////////////////////////////////////
int PixelHistoServer::updateFileList(){
	TThread::Lock();
	if(dispatcherSocket_ == 0 || !dispatcherSocket_->IsValid()){
		return -1;
	}

	string mthn = "[PixelHistoServer::updateFileList()]\t";
	
	TList *fileList = new TList();
	fileList->Add(new TObjString("root"));
/*This has been moved to the PixelHistoDispatcher since the disk where all the data
are stored is shared
	PixelHistoScanDirectory sD;
	vector<string> v = sD.ls();
	for(vector<string>::iterator it=v.begin(); it != v.end(); it++){
  	fileList->Add(new TObjString(it->c_str()));
	}
*/	
	TMessage update(kMESS_OBJECT);
  update.WriteObject(fileList);
	dispatcherSocket_->Send(update); //send TList of all fileNames

	delete fileList;
	update.Reset();
	TThread::UnLock();
  return 0;
}

////////////////////////////////////////////////////////////////////////
int PixelHistoServer::updateFileContent(string name){
	if(dispatcherSocket_ == 0 || !dispatcherSocket_->IsValid()){
		return -1;
	}
	string mthn = "[PixelHistoServer::updateFileContent()]\t";
	
	PixelHistoReadFileElements rFE;
	PixelHistoReadWriteFile rWF;
  string fileName;
	string path;
	splitFileNameAndPath(name,fileName,path);

  if(fileName == "root" || fileName == "/root" || fileName == "RootApp" || fileName == "/RootApp"){
		gROOT->cd();
	}
	else{
		rWF.open(fileName,"READ");
  }
	
  //read the current directory into TList
	rFE.read(path,1);

  TMessage update(kMESS_OBJECT);
  update.WriteObject(&rFE.getAllNames());
  
//	rFE.dump();
  //int mLen = 
	dispatcherSocket_->Send(update); //send TList of all names
//  cout << mthn << "Content update sent: " << mLen << " bytes" <<  endl;
	rWF.close();
  return 0;
}

////////////////////////////////////////////////////////////////////////
int PixelHistoServer::handleHistogramRequest(TString requestString){
	if(dispatcherSocket_ == 0 || !dispatcherSocket_->IsValid()){
		return -1;
	}
	string mthn = "[PixelHistoServer::handleHistogramRequest()]\t";
	
  string fileName;
	string objectName;
	splitFileNameAndPath(requestString.Data(),fileName,objectName);
	
	PixelHistoReadWriteFile rWF;
  if(fileName == "root" || fileName == "/root" || fileName == "RootApp" || fileName == "/RootApp"){
		gROOT->cd();
	}
	else{
		rWF.open(fileName,"READ");
  }
	
	TObject * obj = gDirectory->Get(objectName.c_str());
  TMessage answer(kMESS_OBJECT);

  if(!obj){ //object not recognized
    cout << mthn << "Object not found: " << requestString << endl;
    dispatcherSocket_->Send(errorTK); //send null
    return -1;
  }
  else if(obj->InheritsFrom(TDirectory::Class())){
    cout << mthn << "Error: Request was for a directory!" << endl;  //only send histograms so send null
    dispatcherSocket_->Send(errorTK);
    return -1;
  }
  else if(obj){
    answer.WriteObject(obj);
  }
  cout << mthn << "Sending..." << endl;
  dispatcherSocket_->Send(answer);
	
	return 0;
}

////////////////////////////////////////////////////////////////////////
void PixelHistoServer::splitFileNameAndPath(string name, string &fileName, string &path){
	string mthn = "[PixelHistoServer::splitFileNameAndPath()]\t";
	int pos;
	if((pos = name.find(":/")) >= 0){ //path within file
		fileName = name.substr(0,pos);
		path = name.substr(pos+1);
	}else{
    fileName=name;
		path="/";
  }
//	cout << mthn << "FileName: " << fileName << endl;
//	cout << mthn << "Path: " << path << endl;
}

