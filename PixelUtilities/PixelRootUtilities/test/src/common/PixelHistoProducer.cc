#include "PixelUtilities/PixelRootUtilities/include/PixelHistoServer.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelHistoProducer.h"
#include <iostream>
//#include <TROOT.h>
//#include <TH1F.h>
//#include "TRint.h"
//#include "TApplication.h"
#include <unistd.h>   

using namespace std;

//extern void InitGui();
//VoidFuncPtr_t initfuncs[] = {InitGui, 0 };


//TROOT threadtest("threadtest","Test of threads", initfuncs);    
 

int main(int argc, char **argv){
  //  TApplication theApp("App", &argc, argv);       


  PixelHistoProducer hp;
  //THIS IS TO BE ADDED IN STEVE CLASSES
  static PixelHistoServer hs;

  //gROOT->ls();
  //cout << gROOT << endl;
  //THIS IS TO BE ADDED IN STEVE CLASSES
  hs.startThreads();
  hp.startThreads();
  usleep(10000000);
  hp.stopThreads();

//   char t='.';
//   while(t!='0'){
//     cout << "Enter '0' to exit: ";
//     cin >> t; //input 0 to close

//     switch(t){
// 		case '3': //producer update
//       hs.updateFileContent("suca.root");
//       break;
// 		case '2': //producer update
//       hs.updateFileContent("root");
//       break;
//     case '1': //producer update
//       hs.updateFileList();
//       break;
//     case '0': //exit
//     default:
//       break;
//     }
//   }

  while(1){usleep(10000000);}
  hs.stopThreads();
  cout << "[main()]\tThread stopped!" << endl;
  
  //	theApp.Run();
  return 1;
}
