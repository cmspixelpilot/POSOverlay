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

	//Recursively delete all directoryies and content
	void    deleteDir    (TDirectory *dir );

	void close();

 private:
	void    transferFromMemory(TDirectory *parentFileDir,TDirectory *parentMemDir);
	void    transferToMemory  (TDirectory *parentFileDir,TDirectory *parentMemDir);
	void    transferFromTo    (TDirectory *fromDir      ,TDirectory *toDir);
	TFile * file;
};

#endif
