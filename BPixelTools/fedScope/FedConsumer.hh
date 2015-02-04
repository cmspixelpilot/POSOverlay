#ifndef FEDCONSUMER
#define FEDCONSUMER


#include "TGButton.h"
#include "TRootEmbeddedCanvas.h"
#include "TGLayout.h"
#include "TGButton.h"
#include "TGTextBuffer.h" 
#include "TGTextEntry.h" 
#include "TGComboBox.h"
#include "TSocket.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TString.h"
#include "RQ_OBJECT.h"


class FedConsumer {
  
  RQ_OBJECT("FedConsumer")
    
public:
  FedConsumer(const char* servername);
  ~FedConsumer();
  
  void DoButton();
  void DoConnectHost();
  void Refresh();
  void SingleChannel(); 
  void AllChannels();
  void ShowHistogram();

private:
  // GUI 
  TGMainFrame         *fMain;
  TRootEmbeddedCanvas *fCanvas;
  TGHorizontalFrame   *fHorz;
  TGHorizontalFrame   *fHorz2;
  TGLayoutHints       *fLbut;
  TGLayoutHints       *fLhorz;
  TGLayoutHints       *fLcan;
  TGComboBox          *fChannelSel, *fHistSel;
  TGButton            *fHpx, *fHpy, *fHpz;
  TGButton            *fChannel;
  TGButton            *fGetHist;
  TGButton            *fConnect;

  TGTextEntry         *fConnectSel;
  //  TGTextBuffer        *fConnectHostText;

  TGButton            *fQuit;
  TGCheckButton       *fUpdate;
  TSocket             *fSock;

  // Histograms
  TH1D                *fHist, *fHist1;
  TH1D                *fHarray[12];
  
  TCanvas             *fSingleCanvas, *fMultiCanvas; 
  
  // stuff
  char fServer[255];
  int                 fNdiv;
  int                 fRefresh;
  int                 fSingleChannel, fAllChannels;
  int                 fHistogram;

};


#endif

