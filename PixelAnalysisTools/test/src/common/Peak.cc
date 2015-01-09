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
// we assume a file basic.dat in the current directory
// this file has 3 columns of float data
   in.open(file);

   TH1F *h= new TH1F(file,file,1024,0.5,1024.5);

   for(int i=0;i<1024;i++){
     double count;
     int tmp;
     in >> tmp >>count;
     for (int j=0;j<count;j++){
       h->Fill(tmp);
     }
   }

   in.close();

   TCanvas* c=new TCanvas("c","c", 700,800);

   c->Divide(1 ,1);

   c->cd(1);

   h->Draw();
    


}

