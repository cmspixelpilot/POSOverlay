#include "TH2D.h"
#include "TLegend.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TCanvas.h"
#include <fstream>
#include <iostream>

using namespace std;

void overallPlot(){
  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(111111);
  gStyle->SetPalette(1);

  //INPUT
  ifstream fin("overallOut.dat");

  TH2D* hall = new TH2D("all", "all", 20, -10., 10., 20, -10., 10.);
  TH2D* hFpix = new TH2D("Fpix", "Fpix", 20, -10., 10., 20, -10., 10.);
  TH2D* hBpix = new TH2D("Bpix", "BPix", 20, -10., 10., 20, -10., 10.);
  
  TString det = "";
  double dvana = -99; 
  double dthr = -99;
  while(fin>>det>>dvana>>dthr){

    hall->Fill(dvana, dthr);
    if(det.Contains("BPix")) hBpix->Fill(dvana, dthr);
    if(det.Contains("FPix")) hFpix->Fill(dvana, dthr);
  }
  
  fin.close();

  //hall->Draw("COLZ");
  TCanvas * c1 = new TCanvas("c1","c1", 640, 640);
  TCanvas * c2 = new TCanvas("c2","c2", 640, 640);
  c1->cd();
  hFpix->Draw("COLZ");
  c2->cd();
  hBpix->Draw("COLZ");
  //hall->GetXaxis()->SetTitle("InTime-Absolute Thr");
  //hall->GetYaxis()->SetTitle("Number of ROCs");

  //hBpix->SetLineColor(kRed);
  //hFpix->SetLineColor(kBlue);
  //hBpix->SetLineWidth(2);
  //hFpix->SetLineWidth(2);
  //hBpix->SetTitle("");

  //hBpix->GetXaxis()->SetTitle("average current [mA]");
  //hBpix->GetYaxis()->SetTitle("Number of Power Groups");
  //hBpix->Draw();
  //hFpix->Draw("SAMES");
  
  /*
  TLegend *leg = new TLegend(0.55, 0.70, 0.75, 0.8);
  leg->AddEntry(hBpix, "BPix", "F");
  leg->AddEntry(hFpix, "FPix", "F");
  leg->SetFillColor(0);
  leg->SetLineColor(0);
  leg->SetTextSize(0.04);
  leg->Draw();
  */
}
