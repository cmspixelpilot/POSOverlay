#include "PixelUtilities/PixelRootUtilities/include/PixelHistoConsumer.h"
#include <iostream>
#include <TROOT.h>
#include <TH1F.h>
#include <TH2F.h>
#include "TRint.h"
#include "TApplication.h"   
#include "TApplicationImp.h"   
#include "TMessage.h"
#include "TClass.h"

using namespace std;

extern void InitGui();
VoidFuncPtr_t initfuncs[] = {InitGui, 0 };


TROOT threadtest("threadtest","Test of threads", initfuncs);    
 

int main(int argc, char **argv){
  //  TApplication theApp("App", &argc, argv);
  string mthn = "[main()]\t";
  PixelHistoConsumer hc;
  TMessage *tm;

  gROOT->ls();
  hc.startThreads();
	
  char t='.';
  while(t!='0'){
    cout << mthn << "(4=Histogram2D,3=Directory,2=Histogram3D,1=Refresh) Enter '0' to exit: ";
    cin >> t; //input 0 to close

    switch(t){
    case '5':  //request histo
      tm = hc.requestHisto("root:/histoDir/suca");
      if(!tm)
	cout << mthn << "Failed to receive request." << endl;
      else{
	cout << mthn << "Received request successfully!" << endl;
	cout << mthn << "Is a directory? " << tm->GetClass()->InheritsFrom(TDirectory::Class()) << endl;
	cout << mthn << "Is a TROOT? " << tm->GetClass()->InheritsFrom(TROOT::Class()) << endl;
	cout << mthn << "Is a TH1F? " << tm->GetClass()->InheritsFrom(TH1F::Class()) << endl;
	cout << mthn << "Is a TH2F? " << tm->GetClass()->InheritsFrom(TH2F::Class()) << endl;
      }
      break;
    case '4':  //request histo
      tm = hc.requestHisto("root:/histoDir/hpx");
      if(!tm)
	cout << mthn << "Failed to receive request." << endl;
      else{
	cout << mthn << "Received request successfully!" << endl;
	cout << mthn << "Is a directory? " << tm->GetClass()->InheritsFrom(TDirectory::Class()) << endl;
	cout << mthn << "Is a TROOT? " << tm->GetClass()->InheritsFrom(TROOT::Class()) << endl;
	cout << mthn << "Is a TH1F? " << tm->GetClass()->InheritsFrom(TH1F::Class()) << endl;
	cout << mthn << "Is a TH2F? " << tm->GetClass()->InheritsFrom(TH2F::Class()) << endl;
      }
      break;
    case '3':  //request directory
      tm = hc.requestHisto("root:/histoDir");
      if(!tm)
	cout << mthn << "Failed to receive request." << endl;
      else{
	cout << mthn << "Received request successfully!" << endl;
	cout << mthn << "Is a directory? " << tm->GetClass()->InheritsFrom(TDirectory::Class()) << endl;
	cout << mthn << "Is a TROOT? " << tm->GetClass()->InheritsFrom(TROOT::Class()) << endl;
	cout << mthn << "Is a TH1F? " << tm->GetClass()->InheritsFrom(TH1F::Class()) << endl;
	cout << mthn << "Is a TH2F? " << tm->GetClass()->InheritsFrom(TH2F::Class()) << endl;
      }
      break;
    case '2':  //request histo
      tm = hc.requestHisto("root:/histoDir/hpxpy");
      if(!tm)
	cout << mthn << "Failed to receive request." << endl;
      else{
	cout << mthn << "Received request successfully!" << endl;
	cout << mthn << "Is a directory? " << tm->GetClass()->InheritsFrom(TDirectory::Class()) << endl;
	cout << mthn << "Is a TROOT? " << tm->GetClass()->InheritsFrom(TROOT::Class()) << endl;
	cout << mthn << "Is a TH1F? " << tm->GetClass()->InheritsFrom(TH1F::Class()) << endl;
	cout << mthn << "Is a TH2F? " << tm->GetClass()->InheritsFrom(TH2F::Class()) << endl;
      }
      break;
    case '1':  //consumer refresh
      hc.consumerRefresh();
      break;
    case '0': //exit
    default:
      break;
    }
  }

  hc.stopThreads();
  cout <<  mthn << "Thread stopped!" << endl;
  //	theApp.Run();
  return 1;
}
