#ifndef _PixelHistoSpyData_h_
#define _PixelHistoSpyData_h_

#include "PixelUtilities/PixelRootUtilities/include/PixelHistoThreadFrameWithArgs.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelSLinkEvent.h"
#include "PixelUtilities/PixelFEDDataTools/include/Word64.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"
#include "CalibFormats/SiPixelObjects/interface/PixelNameTranslation.h"
#include <vector>
#include <fstream>
#include <string>
#include <map>

class TH2F;          
class TTree;
class PixelHistoTBranch;
class PixelRootDirectoryMaker;

class PixelHistoSpyData: public PixelHistoThreadFrameWithArgs{
 public:
  PixelHistoSpyData(std::vector<int> fedList,std::vector<pos::PixelROCName> rocList,pos::PixelNameTranslation &nameTranslation);
  ~PixelHistoSpyData();
  void init(std::string filesDir="",int runNumber=-1);
  void destroy(void);
  void userFunc0(int &threadNumber);
 private:
 	int getNextEvent(std::ifstream* file,pos::PixelSLinkEvent &sLinkEvent);
	int getNextWord64(std::ifstream* file, pos::Word64 &word);
  void fill();
  int checkedTimes;
	std::vector<int>                  	 fedList_;
	std::vector<std::ifstream*>       	 fileList_;
	std::vector<pos::PixelSLinkEvent> 	 sLinkEvents_;
	TTree             							  	*theTree_;
	PixelHistoTBranch 							  	*theBranch_;
	PixelRootDirectoryMaker             *thePixelRootDirectoryMaker_;
	//map<rocName,entryNumber>
	std::map<std::string, unsigned int> rocNameEntryMap_;
	//map<fed,map<channel,map<roc,entryNumber> > >
	std::map<unsigned int,std::map<unsigned int,std::map<unsigned int,unsigned int> > > dataEntryMap_;
	//map<fed,map<channel,map<roc,rochisto> > >
	std::map<unsigned int,std::map<unsigned int,std::map<unsigned int,TH2F*> > > histoMap_;
	  struct BranchStruct{
		float hasHits;
		float occupancy;
		char  rocName[38];
	};
};


#endif
