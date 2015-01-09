#include "TCanvas.h"
#include "TH1D.h"
#include "TLegend.h"
#include "TROOT.h"
#include <fstream>
#include <iostream>

using namespace std;

void inabsdiff(){
  gROOT->SetStyle("Plain");

  TH1D* hdiffall = new TH1D("diffall", "diffall", 30, 0, 30);
  TH1D* hdiffFpix = new TH1D("diffFpix", "diffFpix", 30, 0, 30);
  TH1D* hdiffBpix = new TH1D("diffBpix", "diffBpix", 30, 0, 30);
  TH1D* hdiffBpix1 = new TH1D("diffBpix1", "diffBpix1", 30, 0, 30);
  TH1D* hdiffBpix2 = new TH1D("diffBpix2", "diffBpix2", 30, 0, 30);
  TH1D* hdiffBpix3 = new TH1D("diffBpix3", "diffBpix3", 30, 0, 30);
  
  ifstream fin("initial_inabsdiff.dat");
  
  TString roc = "";
  double diff = -99; 
  while(fin>>roc>>diff){
    hdiffall->Fill(diff);
    if(roc.Contains("FPix")) hdiffFpix->Fill(diff);
    
    //if(!roc.Contains("BPix_BmI_SEC2")) continue;
    if(roc.Contains("BPix")) hdiffBpix->Fill(diff);
    if(roc.Contains("LYR1")) hdiffBpix1->Fill(diff);
    if(roc.Contains("LYR2")) hdiffBpix2->Fill(diff);
    if(roc.Contains("LYR3")) hdiffBpix3->Fill(diff);

  }
  
  fin.close();

  //hdiffall->Draw();
  //hdiffall->GetXaxis()->SetTitle("InTime-Absolute Thr");
  //hdiffall->GetYaxis()->SetTitle("Number of ROCs");

  hdiffBpix->SetLineColor(kRed);
  hdiffBpix1->SetLineColor(kRed);
  hdiffBpix2->SetLineColor(kRed+2);
  hdiffBpix3->SetLineColor(kRed+4);
  hdiffFpix->SetLineColor(kBlue);
  hdiffBpix->SetLineWidth(2);
  hdiffBpix1->SetLineWidth(2);
  hdiffBpix2->SetLineWidth(2);
  hdiffBpix3->SetLineWidth(2);
  hdiffFpix->SetLineWidth(2);
  hdiffBpix->SetTitle("");

  TCanvas * c1 = new TCanvas("c1", "c1", 640, 480);
  c1->cd();
  c1->SetLogy(1);

  hdiffFpix->GetXaxis()->SetTitle("InTime-Absolute Thr");
  hdiffFpix->GetYaxis()->SetTitle("Number of ROCs");

  //hdiffBpix->Draw();
  hdiffFpix->SetTitle("");
  hdiffFpix->Draw();
  hdiffBpix1->Draw("SAMES");
  hdiffBpix2->Draw("SAMES");
  hdiffBpix3->Draw("SAMES");


  TLegend *leg = new TLegend(0.55, 0.60, 0.65, 0.70);
  //leg->AddEntry(hdiffBpix, "BPix", "F");
  leg->AddEntry(hdiffBpix1, "BPix LYR1", "F");
  leg->AddEntry(hdiffBpix2, "BPix LYR2", "F");
  leg->AddEntry(hdiffBpix3, "BPix LYR3", "F");
  leg->AddEntry(hdiffFpix, "FPix", "F");
  leg->SetFillColor(0);
  leg->SetLineColor(0);
  leg->SetTextSize(.04);
  leg->Draw();

}
