#ifndef _PixelHistoScanDirectory_h_
#define _PixelHistoScanDirectory_h_

#include <vector>
#include <string>

class PixelHistoScanDirectory{
 public:
	PixelHistoScanDirectory();
	~PixelHistoScanDirectory();
	std::vector<std::string> ls(std::string dir, std::string filter = "*");	
	void                     ls(std::vector<std::string> & list, std::string dir, std::string filter = "*");	
	void                     ls(std::vector<std::string> & list, std::string dir, bool recursive, std::string filter = "*");	
	void                     ls(std::vector<std::string> & list, std::vector<std::pair<std::string,bool> > dirs, std::string filter = "*");	
	std::vector<std::string> ls(bool overrideRecursive=false, bool recursive=true);	
 private:
	std::vector< std::pair<std::string,bool> >  dirsToScan_; 
};
#endif
