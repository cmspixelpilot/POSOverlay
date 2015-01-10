#ifndef _PixelHistoDispatcher_h_
#define _PixelHistoDispatcher_h_

#include "PixelUtilities/PixelRootUtilities/include/PixelHistoThreadFrame.h"
#include <map>
#include "TList.h"
#include <string>


///////////////////////
class TThread;
class TServerSocket; 
class TMonitor;      
class TSocket;     
class TObject;     
class PixelHistoProducerDescriptor;
//class TMessage;
class PixelHistoReadFileElements;
///////////////////////

class PixelHistoDispatcher : public PixelHistoThreadFrame{
 public:
  PixelHistoDispatcher();
  virtual ~PixelHistoDispatcher();
	
  void			  init 							 (void);
  void			  destroy						 (void);
  int 			  startThreads 			 (void);
  int 			  stopThreads				 (void);
	
  void			  displayProdMap 		 (void);
	TList*		  uGetCompleteFileList(bool forceRefresh=true,std::string dirName="");
	TList*		  uGetFileContent		 (TString file, bool forceRefresh=true);
	TObject*    uGetHistogram 			 (TString histoName);
	std::string getClassName 			 (){return className_;}
	
 private:
	enum{
		U_COMPLETE_LIST,
		U_FILE_CONTENT,
		U_HISTOGRAM
	};
		//private member functions
	TObject*   getHistogram 			(TString histoName);
  TList*  	 getCompleteFileList(bool forceRefresh=false);
	TList*     getProducerList		();
	TList*		 getFileList				(TString prodName, bool forceRefresh=true, bool initiate=true);
	TList*		 getFileContent 		(TString fileName, bool forceRefresh=true, bool initiate=true);

  void			 userFunc0         (void);
  void			 HandleSocket      (TSocket *s);
 
  void			 closeSocket       (TSocket *s);
  void			 consumerRefresh   (TSocket *s);
	void			 addProducerToMap  (TSocket *s);
	TString 	 createProducerName(TSocket *s);
	
	TList*		 recvFileList      (TSocket *s);
	TList*		 recvFileContent   (TSocket *s, TString file);
	
		//private member variables
  TServerSocket 																	 *serverSocket_;
  TMonitor																				 *socketMonitor_;

  std::map<TString, PixelHistoProducerDescriptor*> *prodMap_;

	int 																							uCommandReq_;	
	bool																							uCommandDone_;
	bool																							uForceRefreshParam_;
	std::string																				uDirName_;
	TString																						uFileParam_;	
	TList 																					 *uReturnList_;
	TObject 																				 *uReturnMessage_;
	std::string                                       className_;
	PixelHistoReadFileElements                       *rFE_;
	
};

#endif
