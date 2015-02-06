/*************************************************************************
 * Authors: R. Rivera, L. Uplegger                              				 *
 *************************************************************************/

#ifndef _PixelHistoViewer_h_
#define _PixelHistoViewer_h_

#include "xdaq/WebApplication.h"
#include "xgi/Method.h"
#include "cgicc/HTMLClasses.h"

#include "PixelUtilities/PixelRootUtilities/include/PixelHistoDispatcher.h"
#include "PixelHistoPicGen.h"
#include "TObject.h"
#include "TCanvas.h"
#include "TTree.h"
#include <string>

typedef std::map<std::string, std::string> keyValueMap ;

const std::string loginPassword_ = "cmspixels";

struct canvasCacheStruct{
	std::string histoName;
	int 				typeCode;	
	TObject *   object;
	TCanvas *   canvas;
	TTree *   	tree;
	
	canvasCacheStruct(std::string h,int c){
		histoName = h;
		typeCode = c;
		object = 0;
		canvas = 0;
		tree = 0;
	}
	
	~canvasCacheStruct(){
		if(object);
			delete object;
		if(canvas);
			delete canvas;
		if(tree);
			delete tree;			
	}
};

class PixelHistoViewer: public xdaq::Application 
{
 public:
	static PixelHistoViewer * getInstance(void){return theInstance_;}
	
  XDAQ_INSTANTIATOR();
	
  PixelHistoViewer (xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);
  ~PixelHistoViewer(void);
  void destroy		 (void);
	
  void Default								 (xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
	void XGI_HistoViewer				 (xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
	void XGI_RefreshFileList		 (xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
	void XGI_RefreshFileDirectory(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
	void XGI_RefreshContentList	 (xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
	void XGI_RequestHistogram 	 (xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
	void XGI_DetectorNavigator 	 (xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
	void XGI_DetectorRocRequest	 (xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
	void XGI_CanvasControl	  	 (xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
	void XGI_PrintView	  			 (xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
	void XGI_RequestFedAssignment(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
	void XGI_Turtle 						 (xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

 private:
  static PixelHistoViewer *theInstance_;
	
	enum{CANVAS_CACHE_MAX_SIZE = 10};
	enum{
		TOBJECT_TYPECODE_TH1,
		TOBJECT_TYPECODE_TGRAPH,
		TOBJECT_TYPECODE_TCANVAS,
		TOBJECT_TYPECODE_TTREE,
	};
		
	enum{
		PLAIN_FILE_LIST,
		COMPLEX_FILE_LIST,
		COMPLEX_FILE_LIST_ROOT,
		COMPLEX_CONTENT_LIST
	};
		
  void insertFile 			  (xgi::Output * out, std::string filename);
  void insertPngRawData   (xgi::Output * out, std::string filename);
	void insertHeader 		  (xgi::Output * out, std::string type, std::string title="");
	void insertListAsJson   (xgi::Output * out, TList *list, unsigned long, std::string append="", int bunch=1);
	void generateFPixDiscs  (std::string filenameAppend = "");
	void generateBPixDiscs  (std::string filenameAppend = "");
	bool handleObjectRequest(xgi::Input * in, xgi::Output * out, std::string summary = "", std::string field = "");
	void getFieldsOfSummary (xgi::Output * out, TTree *summary);
	void colorRocsWithField (std::string filenameAppend, xgi::Output * out, TTree *summary, std::string field, const char *filter);
	void initFedAssignments ();
	void decodeQueryString	(std::string &postString, std::string &getString, keyValueMap &pairs);

  PixelHistoDispatcher					dispatcher_;
	PixelHistoPicGen							picGen_;
	std::string 									currentSummary_;
	std::string 									currentSummaryField_;
	std::string 									currentSummaryReqId_;
	std::string 									currentSummaryFilter_;
	std::deque<canvasCacheStruct> canvasCache_;
	
							//fpix_ 							[disc]   [panel] [halfDisc] [blade] [fed:channel]
	unsigned char fpixFedAssignment_  [4]		   [2] 		 [2] 			  [12]		[2];
};

#endif
