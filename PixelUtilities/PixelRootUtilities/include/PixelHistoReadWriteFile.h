#ifndef _PixelHistoReadWriteFile_h_ 
#define _PixelHistoReadWriteFile_h_ 

#include <string>

class TFile;
class TDirectory;

class PixelHistoReadWriteFile{
	public:
	PixelHistoReadWriteFile();
	~PixelHistoReadWriteFile();
	void write(std::string fileName,std::string mode="RECREATE");
	void read (std::string fileName);
	void open (std::string fileName,std::string mode);
	void close();
	void transferFromTo(TDirectory *fromDir      ,TDirectory *toDir);
	TFile *getFile     (void){return file_;}
	private:
	void transferFromMemory(TDirectory *parentFileDir,TDirectory *parentMemDir);
	void transferToMemory  (TDirectory *parentFileDir,TDirectory *parentMemDir);
	TFile * file_;
};

#endif
