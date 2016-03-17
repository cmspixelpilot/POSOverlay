#include <iostream>

#include "TROOT.h"
#include "TMessage.h"
#include "TError.h"
#include "TFrame.h"
#include "TStyle.h"
#include "TSystem.h"

#include "FedConsumer.hh"

using namespace std;
// ----------------------------------------------------------------------
FedConsumer::FedConsumer(const char* servername) {

  strncpy(fServer, servername, 255);
  fNdiv = 516;
  fAllChannels = -1; 
  fSingleChannel = 0; 
  fRefresh = 1; 

  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(0);

  // Create a main frame
  fMain = new TGMainFrame(gClient->GetRoot(), 100, 100);
  fMain->SetCleanup(kDeepCleanup);
  
  // Create an embedded canvas and add to the main frame, centered in x and y
  // and with 30 pixel margins all around
  fCanvas = new TRootEmbeddedCanvas("Canvas", fMain, 600, 400);
  fLcan = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY,30,30,30,30);
  fMain->AddFrame(fCanvas, fLcan);
  
  // Create a horizonal frame containing three text buttons
  fLhorz = new TGLayoutHints(kLHintsExpandX, 0, 0, 0, 30);
  fHorz = new TGHorizontalFrame(fMain, 100, 100);
  fMain->AddFrame(fHorz, fLhorz);
  
  // -- Channel histograms 
  fLbut = new TGLayoutHints(kLHintsCenterX, 10, 10, 0, 0);
  
  fHpx = new TGTextButton(fHorz, "1", 1);
  fHpx->SetState(kButtonDisabled);
  fHpx->Connect("Clicked()", "FedConsumer", this, "DoButton()");
  fHorz->AddFrame(fHpx, fLbut);

  fHpy = new TGTextButton(fHorz, "2", 2);
  fHpy->SetState(kButtonDisabled);
  fHpy->Connect("Clicked()", "FedConsumer", this, "DoButton()");
  fHorz->AddFrame(fHpy, fLbut);

  fHpz = new TGTextButton(fHorz, "3", 3);
  fHpz->SetState(kButtonDisabled);
  fHpz->Connect("Clicked()", "FedConsumer", this, "DoButton()");
  fHorz->AddFrame(fHpz, fLbut);
  
  fHistSel = new TGComboBox(fHorz, 100); 
  fHistSel->AddEntry("all ADC", 1); 
  for (int i = 1; i <= 36; ++i) {
    fHistSel->AddEntry(Form("ADC Channel %d", i), i+1); 
  }
  fHistSel->Resize(150, 20); 
  fHistSel->Select(1);
  fHorz->AddFrame(fHistSel, fLbut);

  fGetHist = new TGTextButton(fHorz, "Get hist", 11);
  fGetHist->SetState(kButtonDisabled);
  fGetHist->Connect("Clicked()", "FedConsumer", this, "DoButton()");
  fHorz->AddFrame(fGetHist, fLbut);


  fChannelSel = new TGComboBox(fHorz, 100); 
  for (int i = 0; i < 36; ++i) {
    fChannelSel->AddEntry(Form("Channel %d", i+1), i); 
  }
  fChannelSel->Resize(100, 20); 
  fChannelSel->Select(24);
  fHorz->AddFrame(fChannelSel, fLbut);
  
  fChannel = new TGTextButton(fHorz, "Get channel", 10);
  fChannel->SetState(kButtonDisabled);
  fChannel->Connect("Clicked()", "FedConsumer", this, "DoButton()");
  fHorz->AddFrame(fChannel, fLbut);
  

  // -- Meta controls: connect and quit
  fHorz2 = new TGHorizontalFrame(fMain, 100, 100);
  fMain->AddFrame(fHorz2, fLhorz);
  

  fConnectSel = new TGTextEntry(fHorz2, new TGTextBuffer(100));
  fConnectSel->MoveResize(100, 180, 100, fConnectSel->GetDefaultHeight());
  //fConnectSel->SetText("localhost");
  cout << " connecting to " << fServer << endl;
  fConnectSel->SetText(fServer);
  fConnectSel->Connect("ReturnPressed()", "FedConsumer", this, "DoConnectHost()");
  fHorz2->AddFrame(fConnectSel);
  cout << fConnectSel << endl;

  fConnect = new TGTextButton(fHorz2, "Connect");
  fConnect->Connect("Clicked()", "FedConsumer", this, "DoConnectHost()");

  fUpdate = new TGCheckButton(fHorz2, "Refresh", 90);
  fUpdate->SetState(kButtonDown);
  fUpdate->Connect("Clicked()", "FedConsumer", this, "Refresh()");
  fHorz2->AddFrame(fUpdate, fLbut);

  fHorz2->AddFrame(fConnect, fLbut);
  fQuit = new TGTextButton(fHorz2, "Quit");
  fQuit->SetCommand("gApplication->Terminate()");
  fHorz2->AddFrame(fQuit, fLbut);

  
  // Set main frame name, map sub windows (buttons), initialize layout
  // algorithm via Resize() and map main frame
  fMain->SetWindowName("FedConsumer on SpyServ");
  fMain->MapSubwindows();
  fMain->Resize(fMain->GetDefaultSize());
  fMain->MapWindow();
  
  
  // -- Set up canvas for FEDchannels
  fCanvas->GetCanvas()->GetFrame()->SetFillColor(21);
  fCanvas->GetCanvas()->GetFrame()->SetBorderSize(6);
  fCanvas->GetCanvas()->GetFrame()->SetBorderMode(-1);
  fCanvas->GetCanvas()->Divide(3,4);
  
  fHist = 0;
}



// ----------------------------------------------------------------------
FedConsumer::~FedConsumer() {
   delete fHist;
   delete fSock;
   delete fLbut;
   delete fLhorz;
   delete fLcan;
   delete fHpx;
   delete fHpy;
   delete fHpz;
   delete fChannel;
   delete fConnect;
   delete fQuit;
   delete fHorz;
   delete fHorz2;
   delete fCanvas;
   delete fMain;
}


// ----------------------------------------------------------------------
void FedConsumer::AllChannels() {

  if (-1 == fAllChannels) return;

  TMessage *mess;

  if (!fMultiCanvas) {
    fMultiCanvas = new TCanvas("b","All Channels", 0, 0, 900, 800); 
    fMultiCanvas->GetFrame()->SetFillColor(21);
    fMultiCanvas->GetFrame()->SetBorderSize(6);
    fMultiCanvas->GetFrame()->SetBorderMode(-1);
    fMultiCanvas->Divide(3,4);
  }

  do {
    for (int i = 0; i < 12; ++i) {
      fSock->Send(Form("channel%d", i+fAllChannels));
      if (fSock->Recv(mess) <= 0) {
	Error("FedConsumer::AllChannels", "error receiving message");
	return;
      }
      fMultiCanvas->cd(i+1); 
      gPad->SetGridx();  gPad->SetGridy();
      if (fHarray[i]) delete fHarray[i]; 
      fHarray[i] = (TH1D*)mess->ReadObject(mess->GetClass());
      fHarray[i]->SetNdivisions(fNdiv, "Y");
      fHarray[i]->Draw();
      delete mess;
    }
    fMultiCanvas->Modified();
    fMultiCanvas->Update();
    gSystem->ProcessEvents();
    if (fRefresh && fSingleChannel) SingleChannel();
  } while (fRefresh); 
  
}


// ----------------------------------------------------------------------
void FedConsumer::SingleChannel() {

  TMessage *mess;
  TH1D *h;

  if (!fSingleCanvas) {
    fSingleCanvas = new TCanvas("a","Single Channel", 10, 40, 800, 400); 
  }
  do {
//     fSock->Send(Form("channel%d", fSingleChannel));
//     if (fSock->Recv(mess) <= 0) {
//       Error("FedConsumer::SingleChannel", "error receiving message");
//       return;
//     }
//     h = (TH1D*)mess->ReadObject(mess->GetClass());
//     if (!fHist) fHist = new TH1D(*h);
//     for (int i = 0; i < h->GetNbinsX(); ++i) {
//       fHist->SetBinContent(i+1, h->GetBinContent(i+1));
//     }
//     fHist->SetTitle(h->GetTitle());
//     delete h;
//     fSingleCanvas->GetCanvas()->cd();
//     gPad->SetGridx();  gPad->SetGridy();
//     fHist->Draw();
//     fSingleCanvas->Modified();
//     fSingleCanvas->Update();
//     delete mess;
//     gSystem->ProcessEvents();
    fSock->Send(Form("channel%d", fSingleChannel));
    if (fSock->Recv(mess) <= 0) {
      Error("FedConsumer::SingleChannel", "error receiving message");
      return;
    }
    h = (TH1D*)mess->ReadObject(mess->GetClass());
    fSingleCanvas->GetCanvas()->cd();
    gPad->SetGridx();  gPad->SetGridy();
    h->SetNdivisions(fNdiv, "Y");
    h->Draw();
    fSingleCanvas->Modified();
    fSingleCanvas->Update();
    delete mess;
    gSystem->ProcessEvents();
    if (fRefresh && (fAllChannels > -1)) AllChannels();
  } while (fRefresh); 
  return;
}


// ----------------------------------------------------------------------
void FedConsumer::ShowHistogram() {

  fHistogram = fHistSel->GetSelected();

  TMessage *mess;
  TH1D *h;

  //  cout << "Requesting " << Form("h%d", fHistogram) << endl;
  fSock->Send(Form("h%d", fHistogram)); 
  if (fSock->Recv(mess) <= 0) {
    Error("FedConsumer::ShowHistogram", "error receiving message");
    return;
  }
  h = (TH1D*)mess->ReadObject(mess->GetClass());
  if (!fHist1) fHist1 = new TH1D(*h);
  for (int i = 0; i < h->GetNbinsX(); ++i) {
    fHist1->SetBinContent(i+1, h->GetBinContent(i+1));
  }
  fHist1->SetTitle(h->GetTitle());
  delete h;
  fCanvas->GetCanvas()->cd();
  fHist1->Draw();
  fCanvas->GetCanvas()->Modified();
  fCanvas->GetCanvas()->Update();
  delete mess;
  return;
}


// ----------------------------------------------------------------------
void FedConsumer::Refresh() {
  if (0 == fRefresh) {
    fRefresh = 1; 
    cout << "Refresh activated" << endl;
  } else {
    fRefresh = 0; 
    cout << "Refresh deactivated" << endl;
  }
}  


// ----------------------------------------------------------------------
void FedConsumer::DoConnectHost() {
  cout << fConnectSel << endl;
  char host[1000]; 
  sprintf(host, fConnectSel->GetBuffer()->GetString());
  cout << "connecting to host " << host << "." << endl;

  fSock = new TSocket(host, 9090);
  fConnect->SetState(kButtonDisabled);
  fHpx->SetState(kButtonUp);
  fHpy->SetState(kButtonUp);
  fHpz->SetState(kButtonUp);
  fChannel->SetState(kButtonUp);
  fGetHist->SetState(kButtonUp);
}


// ----------------------------------------------------------------------
void FedConsumer::DoButton() {
  // Handle ALL requests except the initial socket connection request ...

  if (0 == fSock || !fSock->IsValid()) {
    cout << "Error: socket not valid" << endl;
    return;
  }

  TGButton *btn = (TGButton *)gTQSender;
  TMessage *mess;
  // -- all channels
  switch (btn->WidgetId()) {
  case 1:
    fAllChannels = 0; 
    AllChannels();
    return;
  case 2:
    fAllChannels = 12; 
    AllChannels();
    return;
  case 3:
    fAllChannels = 24; 
    AllChannels();
    return;
  case 10:
    fSingleChannel = fChannelSel->GetSelected();
    SingleChannel();
    return;
  case 11:
    ShowHistogram();
    return;
  }

  if (fSock->Recv(mess) <= 0) {
    Error("FedConsumer::DoButton", "error receiving message");
    delete mess;
    return;
  }
  
}


