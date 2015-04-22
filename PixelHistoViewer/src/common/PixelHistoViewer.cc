/*************************************************************************
 * Authors: R. Rivera, L. Uplegger                                				 *
 *************************************************************************/

#include "PixelHistoViewer.h"
#include "JsonList.h"
#include <iostream>
#include <stdlib.h>
#include <TList.h>
#include <TH1.h>
#include <TProfile.h>
#include <TH1F.h>
#include <TGraph.h>
#include <TImage.h>
#include <TClass.h>
#include <TMessage.h>
#include <TBranch.h>
#include <TObjArray.h>
#include <TLeaf.h>
#include <TStyle.h>
#include <TROOT.h>
#include <math.h>
#include <set>

#include "PixelUtilities/PixelRootUtilities/include/PixelHistoScanDirectory.h"

#include <signal.h>

using namespace std;

XDAQ_INSTANTIATOR_IMPL(PixelHistoViewer)
PixelHistoViewer *PixelHistoViewer::theInstance_ = 0;
void sigprocess(int);
//------------------------------------------------------------------------------
void sigprocess(int sigproc){
  if(sigproc == 2){  
    signal(SIGINT,sigprocess);
    cout << "[PixelHistoViewer::sigprocess(int, char**)]\t\t"
         << "Someone killed me pressing Ctrl-c " 
	       << endl;
  }
  else if(sigproc == 3){
    signal(SIGQUIT,sigprocess);
    cout << "[PixelHistoViewer:sigprocess(int, char**)]\t\t"
         << "Someone killed me pressing Ctrl-\\ " 
	       << endl;
  }
	PixelHistoViewer::getInstance()->destroy();
  exit(0);
}

////////////////////////////////////////////////////////////////////////
PixelHistoViewer::PixelHistoViewer(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception): xdaq::Application(s){			
  signal(SIGINT,sigprocess);
  signal(SIGQUIT,sigprocess);
  std::string mthn = "[PixelHistoViewer::PixelHistoViewer()]\t";
	
  xgi::bind(this, &PixelHistoViewer::Default, 								"Default");
  xgi::bind(this, &PixelHistoViewer::XGI_HistoViewer, 				"XGI_HistoViewer");
  xgi::bind(this, &PixelHistoViewer::XGI_RefreshFileList, 		"XGI_RefreshFileList");
  xgi::bind(this, &PixelHistoViewer::XGI_RefreshFileDirectory,"XGI_RefreshFileDirectory");
  xgi::bind(this, &PixelHistoViewer::XGI_RefreshContentList,	"XGI_RefreshContentList");
	xgi::bind(this, &PixelHistoViewer::XGI_RequestHistogram,		"XGI_RequestHistogram");
	xgi::bind(this, &PixelHistoViewer::XGI_DetectorNavigator,		"XGI_DetectorNavigator");	
	xgi::bind(this, &PixelHistoViewer::XGI_DetectorRocRequest,	"XGI_DetectorRocRequest");	
	xgi::bind(this, &PixelHistoViewer::XGI_CanvasControl,				"XGI_CanvasControl");		
	xgi::bind(this, &PixelHistoViewer::XGI_PrintView,						"XGI_PrintView");		
	xgi::bind(this, &PixelHistoViewer::XGI_RequestFedAssignment,"XGI_RequestFedAssignment");	
	xgi::bind(this, &PixelHistoViewer::XGI_Turtle,							"XGI_Turtle");	

  theInstance_ = this;
	
  dispatcher_.startThreads();	  
	
	//clear old generated images
	system("mkdir -p images/generated");
	system("rm -f images/generated/*");
	
	picGen_.initImgBuffer(PixelHistoPicGen::IMG_INIT_BLACK);
	
	picGen_.PrepareDetectorNavigatorHtml();
	picGen_.PrepareDetectorNavigatorJava();
	picGen_.createAuxImages();
	
	picGen_.fillFPixColors();
	picGen_.fillBPixColors();
	
	currentSummary_ = "";
	currentSummaryField_ = "";
	currentSummaryReqId_ = "";
	currentSummaryFilter_ = "";
	
	gStyle->SetPalette(1,0);
	
	generateFPixDiscs();
	generateBPixDiscs();
	//initFedAssignments();
	
	this->getApplicationDescriptor()->setAttribute("icon","pixel/PixelHistoViewer/images/turtleIcon.png");

}

////////////////////////////////////////////////////////////////////////
PixelHistoViewer::~PixelHistoViewer(){
	destroy();
}


////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::destroy(void){
	dispatcher_.stopThreads();
}

////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::insertHeader (xgi::Output * out, string type, string title){

	cgicc::HTTPResponseHeader response("HTTP/1.1",200,"OK");
	if(type == "html"){
		response.addHeader("Content-type","text/html");
	}
	else if(type == "xml" || type == "xhtml"){
		response.addHeader("Content-type","application/xhtml+xml");	
	}
	out->setHTTPResponseHeader(response);
	
	*out << "<html   xmlns       = 'http://www.w3.org/1999/xhtml'\n"
			 << "        xmlns:svg   = 'http://www.w3.org/2000/svg'\n"
			 << "        xmlns:xlink = 'http://www.w3.org/1999/xlink'\n"
			 << "        xmlns:ext   = 'http://www.extjs.com'>\n";
	
	if(title != ""){
	  *out << cgicc::title(title) << std::endl;
	}
}

////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::insertFile (xgi::Output * out,string filename){
	string mthn = "[PixelHistoViewer::insertFile()]\t";
	ifstream file(filename.c_str());
	if(!file){
		cout << mthn << "File not found: " << filename << endl;
		*out << "  <body>\n"
		     << "    <p><b>File " << filename << " not found!</b></p>"
				 << "  </body>"
				 << "</html>";
		return;
	}
  int size;
  file.seekg (0, ios::end);
  size = file.tellg();
  file.seekg(0,ios::beg);
  char buffer[size];
  file.read(buffer,size);
  out->write(buffer,size);  
  file.close();
}

////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception){
  //std::string path = in->getenv("PATH_INFO");
//  string mthn = "[PixelHistoViewer::Default()]\t";
//  cout << mthn << endl;
	insertHeader(out, "html","Login to Pixel Histogram Viewer");	
	insertFile(out,"html_files/Login.html");
}


////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::XGI_HistoViewer(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception){
	string mthn = "[PixelHistoViewer::XGI_HistoViewer()]\t"; //KMTOS This was commented out
  cout << mthn << endl; //KMTOS This was commented out
	
	keyValueMap pairs;
	cgicc::CgiEnvironment environment(in);
	std::string postString = environment.getPostData();
  std::string getString = environment.getQueryString();
	decodeQueryString(postString,getString,pairs);
	
	if(pairs.count("upw")) {	
		TString password = pairs["upw"];
		if(password == loginPassword_){
			insertHeader(out,"html","Pixel Histogram Viewer");
			insertFile(out,"html_files/HistoViewer.html");
			return;
		}
	}
  insertHeader(out,"html","Password Invalid - Pixel Histogram Viewer");
	insertFile(out,"html_files/LoginFailed.html");		
}

////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::generateFPixDiscs(string filenameAppend){
  string mthn = "[PixelHistoViewer::generateFPixDiscs()]\t";
		//generate FPix disc images
	char bmpStr[] = "images/generated/temp.bmp";
	char convStr[1000];	

	if(filenameAppend == "") //reset roc colors
		picGen_.fillFPixColors();
	else
		filenameAppend += ".";
	
	for(int i=0;i<8;++i){	
		picGen_.initImgBuffer(picGen_.IMG_INIT_BLACK);	
  	picGen_.drawDiscToImg(i);
		sprintf(convStr,"images/generated/%sdisc%d.png",filenameAppend.c_str(),i);
  	picGen_.writeImgToBmp(bmpStr);
  	picGen_.convertBmp(bmpStr,convStr);
//		cout << mthn << "Disc " << i << " good?" << picGen_.getFPixGood(i) << endl;
		sprintf(convStr,"images/generated/discGood%d.png",i);
		string cmd;
		if(picGen_.getFPixGood(i) == 1){
			cmd = string("cp images/generated/good.png ") + string(convStr);
		}
		else if(picGen_.getFPixGood(i) == -1){
			cmd = string("cp images/generated/bad.png ") + string(convStr);
		}
		else if(picGen_.getFPixGood(i) == 0){
			cmd = string("cp images/generated/off.png ") + string(convStr);
		}
		else{
			cmd = string("cp images/generated/invisible.png ") + string(convStr);
		}
//		cout << mthn << cmd << endl;
		system(cmd.c_str());
	}
}

////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::generateBPixDiscs(string filenameAppend){

  string mthn = "[PixelHistoViewer::generateBPixDiscs()]\t";
		//generate BPix disc images
	char bmpStr[] = "images/generated/temp.bmp";
	char convStr[1000];

	if(filenameAppend == "") //reset roc colors
		picGen_.fillBPixColors();
	else
		filenameAppend += ".";
		
	for(int i=0;i<6;++i){	
		picGen_.initBImgBuffer(picGen_.IMG_INIT_BLACK);
		picGen_.drawLayerToBImg(i);
		sprintf(convStr,"images/generated/%sbarrel%d.png",filenameAppend.c_str(),i);
  	picGen_.writeBImgToBmp(bmpStr);
  	picGen_.convertBmp(bmpStr,convStr);
		sprintf(convStr,"images/generated/barrelGood%d.png",i);
		string cmd;
//		cout << mthn << "HalfLayer " << i << " good?" << picGen_.getBPixGood(i) << endl;
		if(picGen_.getBPixGood(i) == 1){
			cmd = string("cp images/generated/good.png ") + string(convStr);
		}
		else if(picGen_.getBPixGood(i) == -1){
			cmd = string("cp images/generated/bad.png ") + string(convStr);
		}
		else if(picGen_.getBPixGood(i) == 0){
			cmd = string("cp images/generated/off.png ") + string(convStr);
		}
		else{
			cmd = string("cp images/generated/invisible.png ") + string(convStr);
		}
//		cout << mthn << cmd << endl;
		system(cmd.c_str());
	}
}

////////////////////////////////////////////////////////////////////////
//specialized JSON response for File and Content lists
void PixelHistoViewer::insertListAsJson (xgi::Output * out, TList *list, unsigned long listType, string theAppend, int bunch){
	string mthn = "[PixelHistoViewer::insertListAsJson()]\t";
	
	if(listType==COMPLEX_FILE_LIST_ROOT){   //used for file list
    	  JsonRoot jList(list,"click:FileTreeWindow.onFileClick","");
	  jList.setList();
	  stringstream s;
	  s << jList;
//I added everything from here to the *out statement
          string desired = s.str();
	  string desiredEnd = "}]}]", nothing = "", bracket = "[";
//begin manipulation
	  char* charPATH = getenv("POS_OUTPUT_DIRS");
	  string path(charPATH); 
cout << "\n\nPATH= " << path << endl;
  	  size_t  pathStart= desired.find(path);
	  string beginning  = desired.substr(0,pathStart);
	  size_t lastBracket=beginning.find_last_of(bracket);
          desired.replace(0,lastBracket,nothing);
          size_t  desiredEndStart = desired.find(desiredEnd), sEnd = desired.size(), nChar= sEnd - (desiredEndStart + 4);
	  desired.replace(desiredEndStart + 4,nChar,nothing);
	  *out << desired; //I changed s.str to desired
	  return;
	}

	//JSON format...	
	*out << "[";
	bool firstElement = true;
	if(list){
		TObject * contentObj;
		TIter contentNext(list);
		if(listType==PLAIN_FILE_LIST){   //used for file list
			while ( (contentObj = contentNext())){
				if(firstElement){
					firstElement = false;
				}
				else{
					*out << ",";
				}
				*out << "'" << theAppend << contentObj->GetName() << "'";
			}
		  *out << "]";
			return;
		}

		//else for content lists
		TString name,nodeText,bunchParent;
		set<TString> nodeFathers;
		int bunchSize = 1000;
		bool needsBunches = false;
		if(list->At(bunchSize) != 0){ //exists more than bunchSize number of elements
			if(bunch == 1){
				needsBunches = true;
			}
			bunchParent = list->At(0)->GetName();
			bunchParent = bunchParent(0,bunchParent.Length()-1);//Make sure to remove final / in case it is there
			bunchParent = theAppend + "/" + bunchParent(0,bunchParent.Last('/'));
		}		
		
		int i=0;
		int bi=1;
  	while ( (contentObj = contentNext())){
			name = contentObj->GetName();
			if(firstElement){
				firstElement = false;
			}
			else {
				*out << "},";
			}
			if(needsBunches && i%bunchSize == 0){ //make a folder for the bunch
			  *out << "{";
			  *out  << "id:'" << bunchParent << "/$b" << bi << "',";
			  *out << "text:'Content " << bunchSize*(bi-1) << ":" << bunchSize*bi-1 << "'";
			  *out << ",children: [";
			}
			if(i>=bunchSize*(bi-1) && i<bunchSize*bi){
				*out << "{";
				if(name[name.Length()-1] == '/'){ //folder
					if(listType==COMPLEX_FILE_LIST){
  					*out << "id:'" << name << "',";
					}
					else{

  					*out << "id:'" << theAppend << "/" << name << "',";
	        }
					nodeText = name(0,name.Length()-1);
					*out << "text:'" << nodeText(nodeText.Last('/')+1,nodeText.Length()) << "'";
				}
				else{ //leaf
					*out << "id:'" << theAppend << "/" << name << "',";
					*out << "text:'" << name(name.Last('/')+1,name.Length()) << "',";
					*out << "leaf:true,";
					if(listType==COMPLEX_FILE_LIST){
					  *out << "listeners: {click: FileTreeWindow.onFileClick}";
					}
					else{					
					  *out << "listeners: {click: FileTreeWindow.onContentClick}";
				  }
				}	
				
				if(needsBunches && i == bunchSize*bi-1){ //close bunch folder
				  *out << "}]";
				  ++bi; //increment bunch counter				
				}
			}
			++i; 
  	}
		if(needsBunches && i < bunchSize*bi-1){ //close abbreviated bunch folder
			*out << "}]";
		} 
	}
	
	if(firstElement){ //no elements sent
		*out << "{id:'< no content >',text:'< no content - refresh file list >',leaf:true";
	}
	
	*out << "}]";
}
	
////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::XGI_RefreshFileList(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception){
	TList *allNames = dispatcher_.uGetCompleteFileList(true);
	
	insertListAsJson(out,allNames,PLAIN_FILE_LIST);
 
	delete allNames;
}

////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::XGI_RefreshFileDirectory(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception){
  cgicc::CgiEnvironment environment(in);
  string postString = environment.getPostData();
  string getString  = environment.getQueryString();
  keyValueMap pairs;
  decodeQueryString(postString,getString,pairs) ;
  string dirName = pairs["node"];	
/*
	PixelHistoScanDirectory sD;
	vector<string> v;
	TList * dirList;
	if(dirName == "fileRoot"){
  	v = sD.ls(true,false);
		dirList = dispatcher_.uGetCompleteFileList(true);
	}
	else{
  	sD.ls(v,dirName,false,".root");
		dirList = new TList();
	}
	cout << mthn << dirName << endl;
	for(vector<string>::iterator it=v.begin(); it != v.end(); it++){
	  cout << mthn << it->c_str() << endl;
  	dirList->Add(new TObjString(it->c_str()));
	}
*/
	TList * dirList = dispatcher_.uGetCompleteFileList(true,dirName);
	if(dirName == "fileRoot"){
	  insertListAsJson(out,dirList,COMPLEX_FILE_LIST_ROOT,dispatcher_.getClassName());//I changed COMPLEX_FILE_LIST_ROOT to COMPLEX_FILE_LIST to see what would happen
  }
	else{
	  insertListAsJson(out,dirList,COMPLEX_FILE_LIST,dispatcher_.getClassName());	
	}
	delete dirList;
}

////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::XGI_RefreshContentList(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception){
  cgicc::CgiEnvironment environment(in);
  std::string postString  = environment.getPostData();
  std::string getString  = environment.getQueryString();
  keyValueMap pairs;
  decodeQueryString(postString,getString,pairs) ;
  TString fileName = pairs["node"];	
	
	if(fileName == "contentRoot"){
		*out << "[]";
		return;
	}
	
	int bunch = 1;
	if(fileName.Index('$') >= 0){ //is a bunch request
		bunch = atoi(string(fileName(fileName.Index('$')+2,1).Data()).c_str());
		fileName = fileName(0,fileName.Index('$'));
	}
	
	string producerName = TString(fileName(0,fileName.Index('/'))).Data();
	
	TList *content = dispatcher_.uGetFileContent(fileName,true); //dont delete content!
		
	insertListAsJson(out,content,COMPLEX_CONTENT_LIST,producerName,bunch);
}

////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::colorRocsWithField(string filenameAppend, xgi::Output * out, TTree *summary, string field, const char *filter){
  string mthn = "In PixelHistoViewer::colorRocsWithField";	
  char  rocName[38];
  float *values = 0,
		maxValue=-std::numeric_limits<float>::max(),
		minValue=std::numeric_limits<float>::max(),
		avgValue=0,
		sigValue=0;
	int numberOfEntries = 0;
	int valueIndex = -1;
	int branchIndex = -1;

  TObjArray *branchList = summary->GetListOfBranches();
	int 			 branchListSize = branchList->GetEntries();
  TObjArray *leafList;
  TBranch   *branch;
  TLeaf     *leaf;
	int 			 leafNum;
	int 			 branchEntry;
		
	picGen_.fillFPixColors();
	picGen_.fillBPixColors();
	
	bool isBooleanField = true;
	
	//There can be many branches but ONLY 1 field can be clicked at any time,
	//so only 1 branch at a time can be called. 
	
//	cout << mthn << "Branch list size: " << branchListSize << endl;
		//get all values and find min/max and valCount-------------------------------
	for(int entry=0;entry<branchListSize;++entry){
//		numberOfEntries = 0;There is only one field that can be active so the number of entries must be initialized only once
    if(branchList->At(entry)->InheritsFrom(TBranch::Class())){
      branch = (TBranch *)branchList->At(entry);	
      if(field.substr(0,string(branch->GetName()).length()) != string(branch->GetName())){continue;}
		  branchIndex = entry;
			leafList = branch->GetListOfLeaves();			
      values = new float[branch->GetNleaves()-1];
			
      for(leafNum=0;leafNum<branch->GetNleaves()-1;++leafNum){
        leaf = (TLeaf*)leafList->At(leafNum);
        if(strcmp(field.c_str(),(string(branch->GetName()) + string(" - ") + string(leaf->GetName())).c_str()) == 0){
//					cout << mthn << field << "=" << leaf->GetName() << endl;
					valueIndex = leafNum;
				}
        leaf->SetAddress(&values[leafNum]);
      }
	    ((TLeaf*)leafList->At(branch->GetNleaves()-1))->SetAddress(rocName);					     
        
			if(valueIndex == -1){ //check if field not found
				cout << mthn << "Field " << field << " not found." << endl;
				if(values){
  				delete values;
				}
				return;
			}

			numberOfEntries = branch->GetEntries();
//			cout << mthn << "Entries: " << numberOfEntries << endl;
      for(branchEntry=0;branchEntry<numberOfEntries;++branchEntry){
        branch->GetEntry(branchEntry);
	      //find avg--------------------------
				avgValue += values[valueIndex];
				if(values[valueIndex] > maxValue){
				  maxValue = values[valueIndex];
				}
				if(values[valueIndex] < minValue){
				  minValue = values[valueIndex];
				}
				if(values[valueIndex] != 0 && values[valueIndex] != 1){
					isBooleanField = false;
				}
      }
    }
		else{
			values = 0;
  	}
	}
	if(minValue == maxValue){
		float sigma = maxValue/100.;
		minValue -=sigma;
		maxValue += sigma;
	}
	if(numberOfEntries==0){
		cout << mthn << "WARNING - Number of entries = 0!" << endl;
		numberOfEntries = 1;
	}
	avgValue /= numberOfEntries;
	//find sigma--------------------------
	if(isBooleanField)
		sigValue = -1;
	else{
  	if(values){
      branch = (TBranch *)branchList->At(branchIndex);	
  	  for(branchEntry=0;branchEntry<numberOfEntries;++branchEntry){
        branch->GetEntry(branchEntry);
				sigValue += (values[valueIndex] - avgValue)*(values[valueIndex] - avgValue);
  	  }
		}
	  sigValue = sqrt(sigValue)/numberOfEntries;
	}
	
	
	
	
	//interpret filter command
	float sigmas = 0;
	float hi=-1,lo=-1,eq=-1;
	bool useLess=false,useLessEq=false,useGreater=false,useGreaterEq=false,useEqual=false;
	bool isAnd=false,isOr=false;
	bool boolFilter = false;
	bool validFilter = false; //validFilter only indicates valid 'B' or 'C'
	bool throwFilterError = false;
	bool ul=false;
	bool ug=false;
	bool ulag;
	bool oall;
	
	char keyChar[100];
	sscanf(filter,"%s",keyChar);
	int start=0;	//find command start point
	while(filter[start] != '\0' && filter[start] != keyChar[0]) ++start;	
	
//	cout << mthn << "FILTER: " << filter << " ~Key~ " << keyChar[0] << endl;
	
	if(!isBooleanField && filter[0] != '\0'){
		switch(filter[start])
		{
		case '<': 														//ignore command
		case '*': 														//ignore command
			break;
		case 'S': 														//Sigma command
			sscanf(&filter[start+1],"%f",&sigmas);
			break;		
		case 'B': 														//Bool command
			boolFilter = true;
			sigValue = -1;
		case 'M': 														//Min/Max command
			if(1){
				int i=start+1;	
					//find hi
				while(filter[i] != '\0' && filter[i] != '<') ++i;
				if(filter[i] != '\0'){				
					if(filter[i+1] == '=' && ++i) useLessEq = true;
					else
						useLess = true;
	
					sscanf(&filter[i+1],"%f",&hi);				
				}
				
				i=start+1;				
					//find lo
				while(filter[i] != '\0' && filter[i] != '>') ++i;
				if(filter[i] != '\0'){				
					if(filter[i+1] == '=' && ++i) useGreaterEq = true;
					else
						useGreater = true;
	
					sscanf(&filter[i+1],"%f",&lo);		
				}
				
				ul = useLess | useLessEq;
				ug = useGreater | useGreaterEq;
				ulag = ul & ug; //true if both are used
				oall = ul | ug; //true if one is used

				if(!oall){
					i=start+1;
						//find eq
					while(filter[i] != '\0' && filter[i] != '=') ++i;
					if(filter[i] != '\0'){
						useEqual = true;	boolFilter = true;
						sscanf(&filter[i+1],"%f",&eq);
					}
				}
				oall |= useEqual;
				
				i=start+1;
					//find AND or OR
				while(filter[i] != '\0'){
					if(filter[i] == '&') isAnd = true;
					if(filter[i] == '|') isOr = true;
					++i;
				}			
/*				
				cout << mthn << "BC: " << hi << " " << useLess << " " << useLessEq << " CB: " <<
					lo << " " << useGreater << " " << useGreaterEq << " AO: " << isAnd << " " << isOr << 
					" Eq: " << useEqual << " " << eq << endl;		
*/					
				if((isAnd && isOr) || (!ulag && (isAnd || isOr)) || !oall) {throwFilterError = true; break;} //illegal cases of AND and OR
				
				if(!boolFilter && 
					(isOr || (ulag && hi<lo))) {throwFilterError = true; break;} //illegal color case
				
				validFilter = true;				
			}			
			
			break;
		default:
			throwFilterError = true;
		}	
	}
	
	//limit the color scale based on input number of sigmas
	float minLimit = (!isBooleanField && sigmas > 0) ? avgValue - sigValue * sigmas : minValue;
	float maxLimit = (!isBooleanField && sigmas > 0) ? avgValue + sigValue * sigmas : maxValue;
	
	if(!validFilter){
		if(sigmas <= 0){
//			cout << mthn << ((throwFilterError && !validFilter)?"Invalid Filter":
//				"No action caused by Filter.") << endl;
			if(throwFilterError && !validFilter){
				cout << mthn << "Invalid Filter!" << endl;
			}
		}
	  //else is valid 'S' command despite validFilter being false
	  	//validFilter only indicates valid 'B' or 'C'
			
		
	}
	else{
		if(ul)
			maxLimit = hi;
		if(ug)
			minLimit = lo;
	}
/*	
	cout << mthn << "n: " << numberOfEntries << " min: " << minValue << " max: " << maxValue 
		<< " avg: " << avgValue << " sigma: " << sigValue << " ml: " << minLimit << " Ml: " << maxLimit << endl;
*/		
		//pass CSV summary statistics to JavaScript
	*out << numberOfEntries << "," << minValue << "," << maxValue 
		<< "," << avgValue << "," << sigValue << 
		((throwFilterError && !validFilter)?" <font color='red'>* Invalid Filter *</font>,":",") << 
		minLimit << "," << maxLimit << "," << endl;
		
	float scale = maxLimit - minLimit;	
	
	//must be the same code as in void PixelHistoPicGen::createAuxImages() to match key
	int numOfColors = 6;
	int colors[6][3] = {
		{255,0,255},
		{0,0,255},
		{0,255,255}, 
		{0,255,0},	 
		{255,255,0}, 
		{255,0,0},
	};
	
	float v;
	float sizeOfGrade = 1.0/(numOfColors-1);
	int ci;
		
  branch = (TBranch *)branchList->At(branchIndex);															     
  leafList = branch->GetListOfLeaves();																					     
																																								     

  for(branchEntry=0;branchEntry<branch->GetEntries();++branchEntry){						     
    branch->GetEntry(branchEntry);																							     
																																								     
		if(isBooleanField){																													     
    	picGen_.setRocColor(rocName,values[valueIndex]);
		}								     
		else if(validFilter && boolFilter){ 																				     
																																								     
			v = values[valueIndex];																						     
			if( 																																			     
					(isAnd && 																														     
						( ((useLess && v<hi) || (useLessEq && v<=hi)) &&										     
							((useGreater && v>lo) || (useGreaterEq && v>=lo)) ) 							     
					) ||																																	     
					((isOr || (!isAnd && !isOr)) && 																			     
						( ((useLess && v<hi) || (useLessEq && v<=hi)) ||										     
							((useGreater && v>lo) || (useGreaterEq && v>=lo)) ) 							     
					) ||																															     
					(useEqual && v==eq) 																									     
				) 																																			     
				picGen_.setRocColor(rocName,1); 																				     
			else																																			     
				picGen_.setRocColor(rocName,0); 																				     
		} 																																					     
		else{ 																																			     
																																								     
			v = (values[valueIndex]-minLimit)/scale; //here [0,1] of total scale    
			if(v < 0) 																																     
				picGen_.setRocColor(rocName,																						     
					picGen_.COLOR_LO_R, 																									     
					picGen_.COLOR_LO_G, 																									     
					picGen_.COLOR_LO_B);//low outliers																		     
			else if(v > 1)																														     
				picGen_.setRocColor(rocName,																						     
					picGen_.COLOR_HI_R, 																									     
					picGen_.COLOR_HI_G, 																									     
					picGen_.COLOR_HI_B); //hi outliers																		     
			else{																																			     
				ci = (int)(v/sizeOfGrade);																							     
				v -= ci*sizeOfGrade;																										     
				v /= sizeOfGrade; //here [0,1] of specific color blend portion					     
				picGen_.setRocColor(rocName,																						     
					(int)(colors[ci][0]*(1-v) + colors[ci+1][0]*v), 											     
					(int)(colors[ci][1]*(1-v) + colors[ci+1][1]*v), 											     
					(int)(colors[ci][2]*(1-v) + colors[ci+1][2]*v));											     
			} 																																				     
		} 																																					     
  } 																																						     
	
		//create png's with id appended	
	generateFPixDiscs(filenameAppend);
	generateBPixDiscs(filenameAppend);
//	cout << mthn << "Client ID: " << filenameAppend << endl;
	

  if(values){
	  delete values;
	}
	return;
}

////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::getFieldsOfSummary(xgi::Output * out, TTree *summary){
		
	*out << "[";
	
 	TObjArray *branchList = summary->GetListOfBranches();
 	TObjArray *leafList;
	TBranch   *branch;
	TLeaf     *leaf;
	for(int entry=0; entry<branchList->GetEntries(); ++entry){
	  if(branchList->At(entry)->InheritsFrom(TBranch::Class())){
		  branch = (TBranch *)branchList->At(entry);
			leafList = branch->GetListOfLeaves();
			for(int leafN=0; leafN<branch->GetNleaves()-1; ++leafN){
         leaf = (TLeaf*)leafList->At(leafN);
//				 cout << "[PixelHistoViewer::getFieldsOfSummary()]\t" << branch->GetName() << " - " << leaf->GetName() << endl;
				*out << branch->GetName() << " - " << leaf->GetName() << ",";
			}
		}
	}
	*out << "]";
}

////////////////////////////////////////////////////////////////////////
bool PixelHistoViewer::handleObjectRequest(xgi::Input * in, xgi::Output * out, string summary, string summaryField ){
	string mthn = "[PixelHistoViewer::handleObjectRequest()]\t";
	
	if(!out) {
		cout << mthn << "No xgi::Output paramater" << endl; return false;}
	
  keyValueMap pairs;
	if(in){
		cgicc::CgiEnvironment environment(in);
  	std::string postString = environment.getPostData();
		std::string getString = environment.getQueryString();
  	decodeQueryString(postString,getString,pairs);
	}
	
	string histoName;	
	string sourceWin = "";
	
	bool isCanvasControlsRequest = false;	
	bool isSvgRequest = false;
	bool useCanvasCache = false; // use canvasCache_
	bool getFullImageTag = false;

	//FOR CANVAS CONTROLS REQUEST
		//Basics
	int 	 canvasWidth = 100;
	int 	 canvasHeight = 100;	
	string drawOptions = "";
		//Axes
	float  xmin = 0;
	float  ymin = 0;
	float  zmin = 0;
	float  xmax = 0;
	float  ymax = 0;
	float  zmax = 0;
		//Margin
	float  marginLt=0;
	float  marginUp=0;
	float  marginRt=0;
	float  marginDn=0;
		//Flags
	int 	 xlog=0;
	int 	 ylog=0;
	int 	 zlog=0;
	int 	 xtick=0;
	int 	 ytick=0;
	int 	 xgrid=0;
	int 	 ygrid=0;

	if(in){		
		histoName = pairs["histoname"];
		
		if(pairs.count("w")) //summaries do not have w,h parameters
		{
			canvasWidth  = atoi(pairs["w"].c_str());
			canvasHeight = atoi(pairs["h"].c_str());			 
//			cout << mthn << "w: " << canvasWidth << " h: " << canvasHeight << endl;
		}
		
		if(pairs.count("zlog")) //check for if this is a request from Canvas Controls dialog
		{			
			isCanvasControlsRequest = true;
			
			if(pairs.count("print"))
				isSvgRequest = !(atoi(pairs["print"].c_str()));
				
			if(pairs.count("sourceWin"))
			{
				sourceWin = pairs["sourceWin"];
				*out << sourceWin << "---"; //return source window to javascript
			}
			
			if(pairs.count("getFullImgTag"))
				getFullImageTag = true;
			
			if(pairs.count("useCache"))
				useCanvasCache = atoi(pairs["useCache"].c_str());

			cout << mthn << "Cache Mode: " << useCanvasCache << endl;
			
			drawOptions	 = pairs["draw"];
	
			if(drawOptions == "Default")
				drawOptions = "";
				
				//Axes
			xmin         = atof(pairs["xmin"].c_str());
			ymin         = atof(pairs["ymin"].c_str());
			zmin         = atof(pairs["zmin"].c_str());
			xmax         = atof(pairs["xmax"].c_str());
			ymax         = atof(pairs["ymax"].c_str());
			zmax         = atof(pairs["zmax"].c_str());
				//Margin
			marginLt     = atof(pairs["marginLt"].c_str());
			marginUp     = atof(pairs["marginUp"].c_str());
			marginRt     = atof(pairs["marginRt"].c_str());
			marginDn     = atof(pairs["marginDn"].c_str());
				//Flags
			xlog         = atoi(pairs["xlog"].c_str());
			ylog         = atoi(pairs["ylog"].c_str());
			zlog         = atoi(pairs["zlog"].c_str());
			xtick        = atoi(pairs["xtick"].c_str());
			ytick        = atoi(pairs["ytick"].c_str());
			xgrid        = atoi(pairs["xgrid"].c_str());
			ygrid        = atoi(pairs["ygrid"].c_str());

/*		
			cout << mthn << "o: " << drawOptions << endl;
			cout << mthn << " x: " << xmin << " " << xmax << " y: " << ymin << " " << ymax << " z: " << zmin << " " << zmax << endl;
			cout << mthn << " x: " << marginLt << " " << marginRt << " y: " << marginUp << " " << marginDn << endl;
			cout << mthn << " x: " << xlog << " " << xtick << " " << xgrid << " y: " << ylog << " " << ytick <<
				" " << ygrid << " z: " << zlog << endl;
*/
		}
		else if(pairs.count("sourceWin"))
			sourceWin = pairs["sourceWin"];
			
	}
	else
		histoName = summary;
	
	
	if(histoName == ""){
		*out << "No histogram found.";
		return false;
	}
	
	int numberOfHistos = 1;
	int findIndex;
	string tmpHistoName = histoName;
	while((findIndex = tmpHistoName.find("--")) != -1 && (unsigned int)findIndex != tmpHistoName.length()-2){
		++numberOfHistos;
		tmpHistoName = tmpHistoName.substr(findIndex+2,tmpHistoName.length()-findIndex-2);
	}
	tmpHistoName = histoName;
	
//	cout <<	mthn << "Number of histos: " << numberOfHistos << endl;
	
	TObject  *tmpObject[numberOfHistos];
	TCanvas  *tmpCanvas[numberOfHistos];
	TTree    *tmpTree[numberOfHistos];
	TCanvas  *multiCanvas = 0;
	int typeCode;
	
	if(numberOfHistos > 1){
		int canvasRows = (int)floor(sqrt(numberOfHistos));
		int canvasCols = numberOfHistos/canvasRows;
		canvasCols += (canvasRows*canvasCols < numberOfHistos)?1:0;
		multiCanvas = new TCanvas();
		multiCanvas->Divide(canvasCols,canvasRows);
//		cout << mthn << "Rows: " << canvasRows << " Cols: " << canvasCols << endl;
	}
	
	// **********
	// IMPORTANT!! only delete object/canvas/tree before getting new and when popped from Cache!
	// **********
	
	for(int i=0;i<numberOfHistos;++i){
			
			//init
		tmpObject[i] = 0; tmpCanvas[i] = 0; tmpTree[i] = 0; 
		typeCode = -1;
		
		findIndex = tmpHistoName.find("--");
		if(findIndex == -1){
			findIndex = tmpHistoName.length();
		}
		histoName = tmpHistoName.substr(0,findIndex);
		if((unsigned int)findIndex != tmpHistoName.length()){
			tmpHistoName = tmpHistoName.substr(findIndex+2,tmpHistoName.length()-findIndex-2);
		}		
		
//		cout << mthn << "Req. " << i << ": " << histoName << endl;
		
		
	 			//check cache (most recent requests at front of queue)

 // 		int ccSize = canvasCache_.size();
 		bool foundInCache = false;
//  		cout << mthn << "Cache Depth: " << ccSize << endl;
// 		if(ccSize != 0){ 		
//  			int c = 0;
// 			while(!foundInCache && c < ccSize)
//  				if(canvasCache_[c].histoName == histoName)  //if found in cache set as respMsg and remove from queue
//  				{
// 					foundInCache = true;
// 					typeCode = canvasCache_[c].typeCode;
// 					
// 					if(typeCode == 0) //TODO: SOLVE CANVAS CACHEING PROBLEM
// 					{ 
// 	
// 						switch(typeCode)
// 						{
// 						case TOBJECT_TYPECODE_TH1:
// 						case TOBJECT_TYPECODE_TGRAPH:
// 							tmpObject[i] = (TObject*) canvasCache_[c].object;//->DrawClone();
// 							canvasCache_[c].object = 0;  //prevent desctruction of TObject
// 							break;
// 						case TOBJECT_TYPECODE_TCANVAS:
// 							tmpCanvas[i] = (TCanvas*) canvasCache_[c].canvas;//->DrawClone();
// 							canvasCache_[c].canvas = 0;  //prevent desctruction of TCanvas
// 							break;
// 						case TOBJECT_TYPECODE_TTREE:
// 							tmpTree[i] = (TTree*) canvasCache_[c].tree;//->DrawClone();
// 							canvasCache_[c].tree = 0;  //prevent desctruction of TTree
// 							break;
// 						default:
// 							*out << "No histogram found.";
// 							return false;
// 						}
//  
//   					canvasCache_.erase(canvasCache_.begin()+c);
// 					}
//  				}		
//  				else
//  					++c;
//  		}
// 		
// 		if(typeCode != 0)  //TODO: SOLVE CANVAS CACHEING PROBLEM
//			foundInCache = false;

		if(!foundInCache || !useCanvasCache) //if not using cache or not found
		{
			if(tmpObject[i])
				delete tmpObject[i];//delete cached tmpObject to prepare for update
			if(tmpCanvas[i])
				delete tmpCanvas[i];//delete cached tmpCanvas to prepare for update
			if(tmpTree[i])
				delete tmpTree[i];//delete cached tmpTree to prepare for update
						
			TObject *retObj = dispatcher_.uGetHistogram(histoName); //delete message when done
		  if(retObj == 0){
			  cout << mthn << "Request Failed" << endl;
				*out << "No histogram found.";
				return false;
			}
//			cout << mthn << "Object class: " << retObj->ClassName() << endl;
			if(TString(retObj->ClassName()).Index("TMessage") >= 0){
				TMessage *respMsg = (TMessage*)retObj;
  			if(respMsg->GetClass()->InheritsFrom(TH1::Class())|| respMsg->GetClass()->InheritsFrom(TProfile::Class())){
	  			typeCode = TOBJECT_TYPECODE_TH1;
			  	tmpObject[i] = (TObject*) respMsg->ReadObject(respMsg->GetClass());//->DrawClone();
			  }
			  else if(respMsg->GetClass()->InheritsFrom(TGraph::Class())){
  				typeCode = TOBJECT_TYPECODE_TGRAPH;
	  			tmpObject[i] = (TObject*) respMsg->ReadObject(respMsg->GetClass());//->DrawClone();
		  	}
			  else if(respMsg->GetClass()->InheritsFrom(TCanvas::Class())){
  				typeCode = TOBJECT_TYPECODE_TCANVAS;
	  			tmpCanvas[i] = (TCanvas*) respMsg->ReadObject(respMsg->GetClass())->DrawClone();
		  	}
			  else if(respMsg->GetClass()->InheritsFrom(TTree::Class())){
  				typeCode = TOBJECT_TYPECODE_TTREE;
	  			tmpTree[i] = (TTree*) respMsg->ReadObject(respMsg->GetClass());//->DrawClone();
		  	}
			  delete respMsg;
	 	  }		
		  else{
  			if(TString(retObj->ClassName()).Index("TH") >= 0 || TString(retObj->ClassName()).Index("TProfile") >= 0){
	  			typeCode = TOBJECT_TYPECODE_TH1;
			  	tmpObject[i] = retObj;//->DrawClone();
			  }
			  else if(TString(retObj->ClassName()).Index("TGraph") >= 0){
  				typeCode = TOBJECT_TYPECODE_TGRAPH;
	  			tmpObject[i] = retObj;//->DrawClone();
		  	}
			  else if(TString(retObj->ClassName()).Index("TCanvas") >= 0){
  				typeCode = TOBJECT_TYPECODE_TCANVAS;
	  			tmpCanvas[i] = (TCanvas*)retObj;//->DrawClone()
		  	}
			  else if(TString(retObj->ClassName()).Index("TTree") >= 0){
  				typeCode = TOBJECT_TYPECODE_TTREE;
	  			tmpTree[i] = (TTree*) retObj;//->DrawClone();
		  	}
		  }
 		}
		else{
 		  cout << mthn << "Using Cached!" << endl;		
		}
//		cout << mthn << "TypeCode: " << typeCode << endl;
//		gROOT->ls();

			
		// if(typeCode == 0){//TODO: SOLVE CANVAS CACHEING PROBLEM
// 				//add respMsg to cache, maintain queue size maximums
// 			canvasCacheStruct *tmpCCS = new canvasCacheStruct(histoName,typeCode);
// 			canvasCache_.push_front(*tmpCCS);
// 			switch(typeCode)
// 					{
// 					case TOBJECT_TYPECODE_TH1:
// 					case TOBJECT_TYPECODE_TGRAPH:
// 						canvasCache_.begin()->object = (TObject*) tmpObject[i];//->DrawClone();
// 						break;
// 					case TOBJECT_TYPECODE_TCANVAS:
// 						canvasCache_.begin()->canvas = (TCanvas*) tmpCanvas[i];//->DrawClone();
// 						break;
// 					case TOBJECT_TYPECODE_TTREE:
// 						canvasCache_.begin()->tree = (TTree*) tmpTree[i];//->DrawClone();
// 						break;
// 					default:
// 						*out << "No histogram found.";
// 				  	return false;
// 					}
// 	
// 			while(canvasCache_.size() > CANVAS_CACHE_MAX_SIZE) //should only loop one time
// 				canvasCache_.pop_back(); //pop and delete object/canvas/tree
// 	
// 			int c = 0;
// 			ccSize = canvasCache_.size();
// 			while(c < ccSize){
// 				cout << mthn << c << ": " << canvasCache_[c].histoName << endl;
// 				++c;
// 			}
// 		}
		
	  //--------------- Have TObject Here --------------
		switch(typeCode)
		{
		case TOBJECT_TYPECODE_TH1:	
		case TOBJECT_TYPECODE_TGRAPH:
	  
			if(numberOfHistos == 1){
				multiCanvas = new TCanvas();	//else canvas is already made
			}
			if(!isCanvasControlsRequest){
				if(typeCode == TOBJECT_TYPECODE_TH1){
					drawOptions = "";
				}else{
					drawOptions = "ACP";
				}
			}
			if(xmin < xmax)
				((TH1 *)tmpObject[i])->GetXaxis()->SetRangeUser(xmin,xmax);

			if(ymin < ymax)
				((TH1 *)tmpObject[i])->GetYaxis()->SetRangeUser(ymin,ymax);
				
			if(zmin < zmax)
				((TH1 *)tmpObject[i])->GetZaxis()->SetRangeUser(zmin,zmax);
			
		  if(numberOfHistos == 1)
		    multiCanvas->cd();
		  else //multi canvas
		    multiCanvas->cd(1+i);
	
		  tmpObject[i]->Draw(drawOptions.c_str());
	
			break;			
			
		case TOBJECT_TYPECODE_TCANVAS:
	
			//multiCanvas = tmpCanvas[i];
			tmpCanvas[i]->cd();
			tmpCanvas[i]->SetCanvasSize(canvasWidth,canvasHeight); //484 x 342 is default javascript canvas size
	
			if(1){
				char canvasFile[] = "images/generated/tmpcanvas.png";
				if(!isSvgRequest)
					tmpCanvas[i]->Print(canvasFile,"png");
				else
					tmpCanvas[i]->Print(canvasFile,"svg");
	
				if(!isCanvasControlsRequest || getFullImageTag)
					*out << "<img id='pad-" << sourceWin << "' width='100%' height='100%' src='";
	
				if(!isSvgRequest)
					insertPngRawData(out,canvasFile);
				else
					insertFile(out,canvasFile);
				
				if(!isCanvasControlsRequest || getFullImageTag)
					*out << "' />" << endl;	
		
				if(tmpCanvas[i] != 0){
//				  cout << mthn << tmpCanvas[i] << " : " << tmpCanvas[i]->GetName() << endl;
//				  cout << mthn << "deleting" << endl;
				  delete tmpCanvas[i];
				}
				return true;
			}
			break;	
			
		case TOBJECT_TYPECODE_TTREE:				
		
			if(1) {
				string field;
				if(in)field = pairs["field"];
				else  field = summaryField;	
		
				TTree *tree = (TTree*) tmpTree[i];
	
				if(field == ""){ //requesting field names (client currently doesn't know them)
					getFieldsOfSummary(out,tree);
//					cout << mthn << "getFieldsOfSummary()" << endl;
				}
				else{
					string filenameAppend = "";
					if(pairs.count("requestId")){ //use as filename append
//						cout << mthn << "filenameAppend" << endl;
						filenameAppend = pairs["requestId"];
					}
//					cout << mthn << "filterStr" << endl;
					string filterStr = "";
					if(pairs.count("filter")){ //use as filename append
						filterStr = pairs["filter"];
//						cout << mthn << "filterStr = pairs[filter]" << endl;
          }
					if(currentSummaryFilter_ != filterStr || currentSummaryReqId_ != filenameAppend ||
						 currentSummaryField_ != field || currentSummary_ != histoName){ //dont recolor if current colors are correct						
//						cout << mthn << "colorRocsWithField" << endl;
						colorRocsWithField(filenameAppend,out,tree,field,filterStr.c_str());
						currentSummary_ = histoName;
						currentSummaryField_ = field;
						currentSummaryReqId_ = filenameAppend;
						currentSummaryFilter_ = filterStr;
					}
					else{
						cout << mthn << "Summary and Field already loaded for requestor." << endl;
          }
				}
	
				delete tree; //Tree should never be part of a multi-request
				return true;
			}
			break;
			
		default:

			cout << mthn << "Unrecognized Data Type Received." << endl;
			*out << "No histogram found.";
			for(int j=0;j<i;++j){ //delete all respMsgs up to this point
				if(tmpObject[j])
					delete tmpObject[j];
			}
			if(multiCanvas)
				delete multiCanvas;
	    return false;
		}
	}

	if(isCanvasControlsRequest)
	{
		if(!isSvgRequest)
			multiCanvas->SetCanvasSize(canvasWidth,canvasHeight);
		
		multiCanvas->SetLeftMargin(marginLt);
		multiCanvas->SetTopMargin(marginUp);
		multiCanvas->SetRightMargin(marginRt);
		multiCanvas->SetBottomMargin(marginDn);
	
		multiCanvas->SetLogx(xlog);
		multiCanvas->SetLogy(ylog);
		multiCanvas->SetLogz(zlog);
 
		multiCanvas->SetGrid(xgrid,ygrid);
		multiCanvas->SetTicks(xtick,ytick);
	}
	else
		multiCanvas->SetCanvasSize(canvasWidth,canvasHeight); //484 x 342 is default javascript canvas size
	
	

	char canvasFile[] = "images/generated/tmpcanvas.png";
	
	if(!isSvgRequest)
		multiCanvas->Print(canvasFile,"png");
	else
		multiCanvas->Print(canvasFile,"svg");
	
	if(!isCanvasControlsRequest || getFullImageTag)
		*out << "<img id='pad-" << sourceWin << "' width='100%' height='100%' src='";
	
	if(!isSvgRequest)
		insertPngRawData(out,canvasFile);
	else
		insertFile(out,canvasFile);
	
	if(!isCanvasControlsRequest || getFullImageTag)
		*out << "' />" << endl;	
		
		//clean up			
	for(int i=0;i<numberOfHistos;++i){
	  if(tmpObject[i])
		  delete tmpObject[i];
	}
	
	if(multiCanvas)
		delete multiCanvas;
	return true;
}

////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::XGI_RequestHistogram(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception){
	string mthn = "[PixelHistoViewer::XGI_RequestHistogram()]\t";
	
//  cout << mthn << endl;
	handleObjectRequest(in,out);
}


////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::XGI_DetectorNavigator(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception){
	string mthn = "[PixelHistoViewer::XGI_DetectorNavigator()]\t";
//	cout << mthn << endl;
		
	cgicc::CgiEnvironment environment(in);
  std::string postString  = environment.getPostData();
	std::string getString  = environment.getQueryString();
  keyValueMap pairs;
  decodeQueryString(postString,getString,pairs) ;
	
	string filenameAppend = "";
	if(pairs.count("requestId")) //use as filename append
		filenameAppend = pairs["requestId"];
		
	char pngStr[200]; 
	
		//test for file created for client previously, if none create them
	sprintf(pngStr,"images/generated/%s.disc0.png",filenameAppend.c_str());
	ifstream pngTestFile(pngStr);
	if(pngTestFile.is_open())
		pngTestFile.close();
	else
		filenameAppend = "";
		
	if(filenameAppend == ""){
		filenameAppend = "";
//		cout << mthn << "Using default detector images for user." << endl;
	}
	else{
//		cout << mthn << "Client ID: " << filenameAppend << endl;
		filenameAppend += ".";
	}
	
	char rFile[] = "html_files/DetectorNavigator.html";
	
	//============ Keyword:  		##constant-name##

		//input java stream
	ifstream infile(rFile);
	if(!infile.is_open())
	  {cout << mthn << "IN File not found." << endl; return;}

	infile.seekg(0,ifstream::end);
	unsigned long size = infile.tellg();
	infile.seekg(0);
	
	char c;
	unsigned long maxSize = 100;
	char constantName[maxSize];
	
	int dist;
	unsigned long i;
	for(unsigned long p=0;p<size;++p){ //locate each insert and handle
	  infile.get(c);
		
	  if(c == '#'){ //found one
			dist = -1;
			++p;
 			infile.get(c);
 			
 			if(c == '#'){ //found two
 				
				++p;
				infile.get(c);
				
				i=0;
 				while(c != '#' && i<maxSize && p<size){
					
 					constantName[i] = c;
					++p; ++i;
 					infile.get(c);
 				}				
				constantName[i] = '\0';
				
				if(i == maxSize){
					cout << mthn << "Improper C++ Constant Formation!!! --> " << constantName << endl;
					infile.close();
					return;
				}
				++p;
 				infile.get(c); //read out last '#' 				
				
					//------------ have constantName
				int index = atoi(constantName);
				
				if(index < 0 || (index > 13 && index < 20) || index > 33){
					cout << mthn << "C++ index not found: " << index << endl;
					infile.close();
					return;
				}
				
					//----------- have valid index
				if(index < 8){ // fpix
					sprintf(pngStr,"images/generated/%sdisc%d.png",filenameAppend.c_str(),index);
				}
				else if(index >=8 && index <=13){
					index -= 8;
					sprintf(pngStr,"images/generated/%sbarrel%d.png",filenameAppend.c_str(),index);
				}
				else if(index >=20 && index <=27){
					index -= 20;
					sprintf(pngStr,"images/generated/discGood%d.png",index);
				}
				else if(index >=28 && index <=33){
					index -= 28;
					sprintf(pngStr,"images/generated/barrelGood%d.png",index);
				}
				insertPngRawData(out,pngStr);		
				
			}
			else{ //not keyword
				out->put('#');
			}   
		}
		else{ //normal character
      out->put(c);
	  }			
	}
	
	infile.close();
}

////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::XGI_DetectorRocRequest(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception){
	string mthn = "[PixelHistoViewer::XGI_DetectorRocRequest()]\t";
//	cout << mthn << endl;
	
	keyValueMap pairs;
	cgicc::CgiEnvironment environment(in);
  std::string postString = environment.getPostData();
	std::string getString  = environment.getQueryString();
  decodeQueryString(postString,getString,pairs);
	
	string rocpath    (pairs["rocpath"]);
	string clickmask  (pairs["clickmask"]);
	string summary    (pairs["summary"]);
	string field      (pairs["field"]);
	int    clicksize = atoi(pairs["clicksize"].c_str());

//	cout << mthn 
//	     << "rocpath:" << rocpath 
//	     << " clickmask:" << clickmask 
//	     << "summary:" << summary 
//	     << " field:" << field
//			 << endl;
	
	if( field != "-" && summary != "-" && (currentSummaryField_ != field || currentSummary_ != summary)){ //update colors
		cout << mthn << "Updating Summary Colors" << endl;
		if(!handleObjectRequest(0,0,summary,field)){ //if failed
			cout << mthn << "Request for summary failed." << endl;
			*out << "[]";
			return;
		}		
	}
	
	picGen_.createClickMask(clickmask,clicksize);
	
	char bmpStr[] = "images/generated/temp.bmp";
	char convStr[1000];
	
	if(rocpath[0] == 'F'){ //fpix
		int a,b,c,d,e;
		picGen_.getFPixIndices(rocpath,a,b,c,d,e);
		if(a == -1){
			cout << mthn << "Invalid FPix ROC Path Name." << endl;
			return;
		}
	
		string stdName = picGen_.getFPixStandardName(a,b,c,d,e);
	
		int i = a*2+b;
			
		picGen_.initImgBuffer(picGen_.IMG_INIT_BLACK);
		picGen_.drawDiscToImg(i,true);
		sprintf(convStr,"images/generated/disc%d.png",i);
	  picGen_.writeImgToBmp(bmpStr);
	  picGen_.convertBmp(bmpStr,convStr);
	}
	else if(rocpath[0] == 'B'){ //bpix
		int a,b,c,d;
		picGen_.getBPixIndices(rocpath,a,b,c,d);
		if(a == -1){
			cout << mthn << "Invalid BPix ROC Path Name." << endl;
			return;
		}
		string stdName = picGen_.getBPixStandardName(a,b,c,d);
//		cout << mthn << stdName << endl;
		
		int i = a*2+b;
	
		picGen_.initBImgBuffer(picGen_.IMG_INIT_BLACK);
		picGen_.drawLayerToBImg(i,true);
		sprintf(convStr,"images/generated/barrel%d.png",i);
  	picGen_.writeBImgToBmp(bmpStr);
  	picGen_.convertBmp(bmpStr,convStr);
	
	}
	else
		cout << mthn << "Invalid path." << endl;

	insertPngRawData(out,convStr);
	
	//clock_t start = clock();
	//cout << mthn << 1.0*(clock() - start)/CLOCKS_PER_SEC << " sec" << endl;
}


////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::insertPngRawData	(xgi::Output * out,string filename){
	
		//write raw picture data to output stream
	ifstream is;
	is.open(filename.c_str());
	
  *out << "data:image/png;charset=US-ASCII,";

  char CodeURL[4];
  while(is.good()){ //print out safe Ascii equivalent
    sprintf(CodeURL,"%%%2.2X",(unsigned char)(is.get()));
    *out << CodeURL;
  }
	
	is.close();
}

////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::XGI_CanvasControl(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception){
	string mthn = "[PixelHistoViewer::XGI_CanvasControl()]\t";
//	cout << mthn << endl;
	
	if(!handleObjectRequest(in,out))
		cout << mthn << "Error Detected!" << endl;
}

////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::XGI_PrintView(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception){
	string mthn = "[PixelHistoViewer::XGI_PrintView()]\t";
	cout << mthn << endl;
	
	keyValueMap pairs;
	cgicc::CgiEnvironment environment(in);
  std::string postString = environment.getPostData();
	std::string getString = environment.getQueryString();
  decodeQueryString(postString,getString,pairs);
	
	string histoName = "";
	if(!pairs.count("print")) //check for valid parameter
	{	
		cout << "Invalid Parameters...";
		*out << "Invalid Parameters...";		
		return;       
  }
		
	histoName = pairs["histo"];
	bool print = atoi(pairs["print"].c_str());
 
  insertHeader(out, "xml",histoName);	
	
	cout << mthn << "Name: " << histoName << endl;

	if(print)
		*out << "<header><script type='text/javascript'>\n" <<
			"function myPrint(){\nwindow.print();}\n</script>\n</header>\n" << 
			"<body onload='myPrint()'><img width='100%' height='100%' src='";
	else
		*out << "<body>\n";
	if(!handleObjectRequest(in,out))
		cout << mthn << "Error Detected!" << endl;
		
	if(print)
		*out << "' />\n\n";
	
	*out << "</body>\n</html>";
}

////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::XGI_RequestFedAssignment(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception){
	string mthn = "[PixelHistoViewer::XGI_RequestFedAssignment()]\t";
//	cout << mthn << endl;
	
	keyValueMap pairs;
	cgicc::CgiEnvironment environment(in);
  std::string postString = environment.getPostData();
	std::string getString = environment.getQueryString();
  decodeQueryString(postString,getString,pairs);
	
	int disc = atoi(pairs["disc"].c_str());
	int inner = atoi(pairs["inner"].c_str());
	
	char fdCh[10];
	for(int c=0;c<2;++c)
		for(int d=0;d<12;++d){
			sprintf(fdCh,"%2.2d:%2.2d",
				fpixFedAssignment_[disc][inner][c][d][0],
				fpixFedAssignment_[disc][inner][c][d][1]);
			*out << fdCh << ",";
		} 
}

////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::initFedAssignments(){
	for(int a=0;a<4;++a)
		for(int b=0;b<2;++b)
			for(int c=0;c<2;++c)
				for(int d=0;d<12;++d){
						fpixFedAssignment_[a][b][c][d][0] = 0;
						fpixFedAssignment_[a][b][c][d][1] = 0;		
					}

	int numOfFiles = 4;
	string files[4] = {
		"/FPixDAQ/DAQ/PCDE_BmI_all/nametranslation/0/translation.dat",
		"/FPixDAQ/DAQ/PCDE_BmO_all/nametranslation/0/translation.dat",
		"/FPixDAQ/DAQ/PCDE_BpI_all/nametranslation/0/translation.dat",
		"/FPixDAQ/DAQ/PCDE_BpO_all/nametranslation/0/translation.dat",
	};
	
	for(int i=0;i<numOfFiles;++i){
		FILE *fp = fopen(files[i].c_str(),"r");
	
		char line[400];
		char rocName[50];
		int fed,ch;
		int disc,panel,halfDisc,blade,roc;
		fgets(line,400,fp); //flush out header line;
		while(fgets(line,400,fp)){
			sscanf(line,"%s %*d %*d %*d %*d %*d %*d %d %d",rocName,&fed,&ch);
			picGen_.getFPixIndices(rocName,disc,panel,halfDisc,blade,roc);
			if(disc != -1){
				fpixFedAssignment_[disc][panel][halfDisc][blade][0] = fed;
				fpixFedAssignment_[disc][panel][halfDisc][blade][1] = ch;
			}
		}
	
		fclose(fp);
	}
}

//==========================================================================================================================
void PixelHistoViewer::decodeQueryString(string &postString,string &getString,keyValueMap &pairs)
{
  string mthn = "[PixelConfigDBGUI::decodeQueryString(keyValueMap)]" ;
	string *queryString;
	
  if(postString.size() > getString.size())
		queryString = &postString;
	else
		queryString = &getString;	
	
	pairs.clear() ;
  toolbox::StringTokenizer qs_elements((*queryString),"&");
  int counts = (int)qs_elements.countTokens();
  if( (*queryString).find("&") >= (*queryString).size() ) {counts=1;}
	
	int specialIndex;
	char specialCharStr[3];
	specialCharStr[2] = '\0';
	
  for( int i=0; i<counts; i++)
    {
      string token = qs_elements.nextToken();
      toolbox::StringTokenizer localPairs(token,"=");
      int c = (int)localPairs.countTokens();
      for( int j=0; j<c/2; j++)
        {
          string key   = localPairs.nextToken() ;
          string value = localPairs.nextToken() ;
					while((unsigned int)(specialIndex = value.find('%')) != (unsigned int) string::npos){ //replace special charcters with proper character
						specialCharStr[0] = value[specialIndex+1];
						specialCharStr[1] = value[specialIndex+2];	
						value.replace(specialIndex,3,1,char(strtol(specialCharStr,0,16)));
					}
          pairs[key] = value ;
        }
    }
}

////////////////////////////////////////////////////////////////////////
void PixelHistoViewer::XGI_Turtle(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception){
	picGen_.generateTurtle();
	insertPngRawData(out,"images/generated/turtle.png");
}
