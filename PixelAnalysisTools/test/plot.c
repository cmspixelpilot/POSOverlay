#include "TCanvas.h"
#include "TTree.h"
#include "TString.h"
#include <iostream>

TTree* read(TString s);

int plot(){
  TCanvas *c=new TCanvas("C","C");
  TTree* t4=read("/pixel/data0/Run_106000/Run_106369/TrimDefault.dat");
  //TTree* t4=read("FPix_BmO_D1_BLD5_PNL1_PLQ2_ROC0_65624.dat");
  //TTree* t4=read("/nfshome0/pixelpro/TriDAS/pixel/PixelRun/Runs/Run_66000/Run_66465/TrimOutputFile_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31-32-33-34-35-36-37-38-39.dat");
  //TTree* t4=read("FPix_BpI_D1_BLD1_PNL1_PLQ2_ROC4_62808.dat");
  //TTree* t4=read("FPix_BmO_D1_BLD1_PNL2_PLQ2_ROC6_62808.dat");
  //TTree* t4=read("TrimDefault_BPix_64268.dat");
  //t4->Draw("thr","abs(thr-80.0)<80.0&&abs(thr-82.0)>0.0")
  if (1) {
    c->Divide(1,1);
    c->cd(1);
    t4->Draw("thr","abs(thr-70.0)<40.0&&abs(thr-80)>0.0");
  }
  else {
    c->Divide(2,2);
    c->cd(1);
    t4->Draw("thr","abs(thr-80.0)<40.0&&abs(row-0)<2");
    c->cd(2);
    t4->Draw("thr","abs(thr-80.0)<40.0&&abs(row-24)<2");
    c->cd(3);
    t4->Draw("thr","abs(thr-80.0)<40.0&&abs(row-48)<2");
    c->cd(4);
    t4->Draw("thr","abs(thr-80.0)<40.0&&abs(row-72)<2");
    //t4->Draw("row");
  }
  c->Print("plot.ps");
  return 1;
}
