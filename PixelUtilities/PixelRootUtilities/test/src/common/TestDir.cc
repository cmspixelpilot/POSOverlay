#include "PixelUtilities/PixelRootUtilities/include/PixelHistoScanDirectory.h"
#include "PixelUtilities/PixelXmlUtilities/include/PixelXmlReader.h"
#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>

using namespace std;

int main(){
	string mthn = "[main()]\t";
	PixelHistoScanDirectory sD;
//	vector<string> v;
//	sD.ls(v,dirsToScan,"root");
	vector<string> v = sD.ls();
	for(vector<string>::iterator it=v.begin(); it != v.end(); it++){
	  cout << "[main()]\t" << *it << endl;
	}
	return 0;
}
