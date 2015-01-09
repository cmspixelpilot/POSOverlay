void peak(TString file)
{
//   example of macro to read data from an ascii file and
//   create a root file with an histogram and an ntuple.


//   gROOT->Reset();
//#include "Riostream.h"

 

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

