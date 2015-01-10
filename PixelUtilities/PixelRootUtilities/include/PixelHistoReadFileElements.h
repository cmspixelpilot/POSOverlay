#ifndef _PixelHistoReadFileElements_h_
#define _PixelHistoReadFileElements_h_

#include <TList.h>
#include <TString.h>

class TDirectory;

class PixelHistoReadFileElements{
	public:
	PixelHistoReadFileElements();
	~PixelHistoReadFileElements();
	void destroy();
	void read(TString startDirName,int nOfSubLevels=-1);//-1 means all
	void read(TDirectory *startDir=0,int nOfSubLevels=-1);//-1 means all
	void dump();
	TList& getAllNames(void){return allNames;}
	TList& getDirNames(void){return dirNames;}
	TList& getObjNames(void){return objNames;}
	
	private:
	void readService(TString currentDirName="/",int nOfSubLevels=-1);
	TList allNames;
	TList dirNames;
	TList objNames; 
};

#endif
