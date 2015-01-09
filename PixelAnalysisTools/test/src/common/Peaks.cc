#include <iostream>
#include <fstream>
#include <TString.h>
#include <TH1F.h>
#include <TCanvas.h>

using namespace std;

int main(int argv, char** argc)
{
  TString file;
  if(argv != 2){
    cout << "[main()]\tUsage: Peak filename" << endl;
    exit(0);
  }  
  else{
    file = argc[1];
  }

  ifstream in;
//we assume files peaks_chan_roc.dat in the current directory
//each file has 3 columns of float data
//will loop over all existing files for roc in [0,23]
  
  TCanvas* c=new TCanvas("c","c", 700,800);

  c->Divide(4 ,6);

  for(Long_t roc=0;roc<24;roc++){
    TString filename = file + '_' + roc + ".dat";
    TString title = file + '_' + roc;

    //       cout << "roc=" << roc <<endl;
    in.open(filename);

    if( !in.good() ) //ran out of files
      {
  	break;
      }

    c->cd(roc+1);

    //       TH1F *h= new TH1F(title,title,1024,-0.5,1023.5);
    TH1F *h= new TH1F(title,title,256,-0.5,1023.5);
    for(int i=0;i<1024;i++){
      double count;
      int tmp;
      in >> tmp >>count;
      h->Fill(tmp,count);
    }
    in.close();

    h->Draw();
  }
  c->Print(file+".ps");
}

