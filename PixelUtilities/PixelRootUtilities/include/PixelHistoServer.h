#ifndef _PixelHistoServer_h_
#define _PixelHistoServer_h_

#include "PixelUtilities/PixelRootUtilities/include/PixelHistoThreadFrame.h"
#include <string>
#include <TString.h>

class TMonitor;      
class TList;
class TSocket;     

class PixelHistoServer : public PixelHistoThreadFrame{
 public:
  PixelHistoServer();
  virtual ~PixelHistoServer();

  void init           	(void); 
  void destroy        	(void); 
  int  startThreads   	(void); 
  int  stopThreads    	(void); 
  int  updateFileList   (void); 
  int  updateFileContent(std::string filename); 
	bool getAutoRefresh   (void) { return autoRefresh_;}
	
 private:
  enum {sleepBetweenPing_us  = 1000000, 
	      monitorTimeout_ms      = 20, 
				sleepDuringSelect_us = 20000};
				
  void userFunc0						 (void);
  void handleSocket 				 (void);
  int  pingDispatcher 			 (void);
	int  handleHistogramRequest(TString requestString);
	
	bool  		 autoRefresh_; 
  TSocket   *dispatcherSocket_; // dispatcher socket
  TMonitor  *socketMonitor_;	  // socket monitor
	void       splitFileNameAndPath(std::string name, std::string &fileName, std::string &path);
};

#endif
