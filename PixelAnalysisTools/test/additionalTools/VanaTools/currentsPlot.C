#include "TH1D.h"
#include "TLegend.h"
#include "TROOT.h"
#include <fstream>
#include <iostream>

using namespace std;

void currentsPlot(){
  gROOT->SetStyle("Plain");

  //INPUT
  ifstream fin("../0deg_iter0/0deg_iter0.csv.dat");
  //ifstream fin("0deg_shift1.csv.dat");
  bool normalizeByROCs = true;

  double min, max;
  if(normalizeByROCs){
    min=20;
    max=40;
  }
  else{
    min=2500;
    max=6000;
  }
  
  TH1D* hall = new TH1D("all", "all", 20, min, max);
  TH1D* hFpix = new TH1D("Fpix", "Fpix", 20, min, max);
  TH1D* hBpix = new TH1D("Bpix", "BPix", 20, min, max);
  
  TString det = "";
  double avgc = -99;
  double avgcN = -99;
  double avgcUN = -99;
  while(fin>>det>>avgcUN>>avgcN){

    if(normalizeByROCs){
      avgc=avgcN;
    }
    else{
      avgc=avgcUN;
    }
    avgc = avgc*1000.;
    hall->Fill(avgc);
    if(det.Contains("BPix")) hBpix->Fill(avgc);
    if(det.Contains("FPix")) hFpix->Fill(avgc);
  }
  
  fin.close();

  //hall->Draw();
  //hall->GetXaxis()->SetTitle("InTime-Absolute Thr");
  //hall->GetYaxis()->SetTitle("Number of ROCs");

  hBpix->SetLineColor(kRed);
  hFpix->SetLineColor(kBlue);
  hBpix->SetLineWidth(2);
  hFpix->SetLineWidth(2);
  hBpix->SetTitle("");

  if(normalizeByROCs){
    hBpix->GetXaxis()->SetTitle("average current per ROC [mA]");
  }
  else{
    hBpix->GetXaxis()->SetTitle("current [mA]");
    }
  hBpix->GetYaxis()->SetTitle("Number of Power Groups");
  hBpix->Draw();
  hFpix->Draw("SAMES");

  TLegend *leg = new TLegend(0.55, 0.70, 0.75, 0.8);
  leg->AddEntry(hBpix, "BPix", "F");
  leg->AddEntry(hFpix, "FPix", "F");
  leg->SetFillColor(0);
  leg->SetLineColor(0);
  leg->SetTextSize(0.04);
  leg->Draw();

}
