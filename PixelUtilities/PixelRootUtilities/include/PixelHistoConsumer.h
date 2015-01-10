#ifndef _PixelHistoConsumer_h_
#define _PixelHistoConsumer_h_

#include "PixelUtilities/PixelRootUtilities/include/PixelHistoThreadFrame.h"
#include "TString.h"

///////////////////////
class TThread;
class TMessage;
class TServerSocket; 
class TMonitor;      
class TList;
class TSocket;     
///////////////////////

class PixelHistoConsumer : public PixelHistoThreadFrame{
 public:
  PixelHistoConsumer();
  void init();
  virtual ~PixelHistoConsumer();
  void destroy();
  int startThreads();
  int stopThreads();

  int consumerRefresh(); //initiates a consumer refresh procedure with Dispatcher. -1 returned on failure.
  TMessage *requestHisto(TString hs); //requests histogram from Dispatcher
  
  TList *getAllNames(){return allNames;}
  
 private:
  void userFunc0();
  void HandleSocket(TSocket *s);
	
  TSocket 	*ds; 	 // dispatcher socket
  TMonitor      *fMon;       // socket monitor

  bool isRefreshing; //used to lock out recv thread while refreshing
  TList *allNames;

};

#endif
