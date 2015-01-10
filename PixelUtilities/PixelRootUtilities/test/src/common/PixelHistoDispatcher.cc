#include "PixelUtilities/PixelRootUtilities/include/PixelHistoDispatcher.h"
#include <iostream>
#include <TROOT.h>
#include <TH1F.h>
#include "TRint.h"
#include "TApplication.h"   
#include <unistd.h>

using namespace std;

extern void InitGui();
VoidFuncPtr_t initfuncs[] = {InitGui, 0 };


TROOT threadtest("threadtest","Test of threads", initfuncs);    
 

int main(int argc, char **argv){
  //  TApplication theApp("App", &argc, argv);
       
  PixelHistoDispatcher hd;
	
  gROOT->ls();
  hd.startThreads();
  usleep(10000); //delay so prompt is visible
	
  char t = 'b';
  while(t !='a'){
    cout << "Enter any character to exit: ";
    cin >> t; //input an int to close
  	switch (t){
		case '1':
			//hd.requestHisto("sucabene/sucabene");
		  break;
		default:
			break;
		}
		
	}
  hd.stopThreads();
  cout << "[main()]\tThread stopped!" << endl;
  //	theApp.Run();
  return 1;
}
