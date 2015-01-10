#include "PixelUtilities/PixelRootUtilities/include/PixelHistoProducer.h"
#include <iostream>
#include <string>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>
#include <TCanvas.h>
#include <TFrame.h>
#include <TRandom.h>
#include <TThread.h>
#include <TROOT.h>
#include <TDirectory.h>

using namespace std;

PixelHistoProducer::PixelHistoProducer(){
  gROOT->cd();
  histoDir= new TDirectory("histoDir","Directory with part of the histos");
  histoDir->cd();
  // Create a new canvas
  fCanvas = new TCanvas("SpyServ","SpyServ",200,10,700,500);
  fCanvas->SetFillColor(42);
  fCanvas->GetFrame()->SetFillColor(21);
  fCanvas->GetFrame()->SetBorderSize(6);
  fCanvas->GetFrame()->SetBorderMode(-1);
	histoDir->Append(fCanvas);

  // Create a 1-D, 2-D and a profile histogram
  fHpx    = new TH1F("hpx","This is the px distribution",100,-4,4);
  fHpxpy  = new TH2F("hpxpy","py vs px",40,-4,4,40,-4,4);
  fHprof  = new TProfile("hprof","Profile of pz versus px",100,-4,4,0,20);
//  gDirectory->ls();

  //  Set canvas/frame attributes (save old attributes)
  fHpx->SetFillColor(48);

  // Fill histograms randomly
  gRandom->SetSeed();
  checkedTimes=0;
  gROOT->cd();
}
void PixelHistoProducer::init(){
}

PixelHistoProducer::~PixelHistoProducer(){
  //   delete fCanvas;
  //////////Histos will be deleted when the directory is closed
  PixelHistoThreadFrame::stopThreads();
  delete fHpx;
  delete fHpxpy;
  delete fHprof;
  //	if(histoDir){
  //	  histoDir->ls();
  //  	histoDir->Close();
  //	}
}

void PixelHistoProducer::userFunc0(void){
  PixelHistoThreadFrame::funcRunning_[0] = true;
  fill();
  PixelHistoThreadFrame::funcRunning_[0] = false;
}

void PixelHistoProducer::fill(){
  string mthn = "[PixelHistoProducer::fill()]\t";
  Float_t px, py, pz;
//  const Int_t kUPDATE = 1000;
  //	 cout << mthn << "Filling..." << endl;
  for (Int_t i = 1; i<=1000; i++) {
    gRandom->Rannor(px,py);
    pz = px*px + py*py;
    TThread::Lock();
    fHpx->Fill(px);
    fHpxpy->Fill(px,py);
    fHprof->Fill(px,pz);
    TThread::UnLock();
    /* 
       if (i && (i%kUPDATE) == 0) {
       if (i == kUPDATE) fHpx->Draw();
       fCanvas->Modified();
       fCanvas->Update();
       cout << mthn << "Drawing..." << i << endl;
       gSystem->Sleep(1);
       }
    */
  }
}
