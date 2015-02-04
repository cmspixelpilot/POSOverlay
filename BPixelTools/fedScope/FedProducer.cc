#include <fstream>
#include <iostream>

#include "FedProducer.hh"
#include "TStyle.h"
#include "TROOT.h"

using namespace std;

// ----------------------------------------------------------------------
FedProducer::FedProducer() {

  cout << "======================================================================" << endl;
  cout << " FED Scope" << endl;

  fServ = new TServerSocket(9090, kTRUE);
  fMon  = new TMonitor;
  fMon->Add(fServ);
  
  fSockets = new TList;

  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(0);

  // -- The histograms
  fH1 = new TH1D("h1","adc", 1025, 0., 1025.);

  for (int i = 0; i < 36; ++i) {
    fChannels[i] = new TH1D(Form("Channel %d", i+1), Form("Channel %d", i+1), 250, 0., 250.);
    fChannels[i]->SetMaximum(1025.);
    fChannels[i]->SetMinimum(0.);

    fChannelLevels[i] = new TH1D(Form("Levels %d", i+1), Form("Levels, channel %d", i+1), 1025, 0., 1025.);
  }

  cout << "======================================================================" << endl;

}


// ----------------------------------------------------------------------
FedProducer::~FedProducer() {
  fSockets->Delete();
  delete fSockets;
  delete fServ;
}


// ----------------------------------------------------------------------
void FedProducer::HandleSocket(TSocket *s) {
  //  cout << "  FedProducer::HandleSocket" << endl;
  if (s->IsA() == TServerSocket::Class()) {
    // accept new connection from spy
    TSocket *sock = ((TServerSocket*)s)->Accept();
    fMon->Add(sock);
    fSockets->Add(sock);
    printf("accepted connection from %s\n", sock->GetInetAddress().GetHostName());
  } else {
    // we only get string based requests from the spy
    char request[64];
    if (s->Recv(request, sizeof(request)) <= 0) {
      fMon->Remove(s);
      fSockets->Remove(s);
      printf("closed connection from %s\n", s->GetInetAddress().GetHostName());
      delete s;
      return;
    }
    
    // send requested object back
    TMessage answer(kMESS_OBJECT);

    // -- parse request
    int channel;
    sscanf(request, "channel%d", &channel);
    //if (channel >= 24 && channel <= 36) {
    if (channel >= 0 && channel < 36) {
      answer.WriteObject(fChannels[channel]);
      s->Send(answer);
      return;
    }

    sscanf(request, "h%d", &channel);
    if (1==channel) {
      answer.WriteObject(fH1);
      s->Send(answer);
      return;
    }

    if (channel > 1 && channel <= 37) {
      answer.WriteObject(fChannelLevels[channel-2]);
      s->Send(answer);
      return;    
    }

    Error("FedProducer::HandleSocket", "unexpected message");
  }
}


// ----------------------------------------------------------------------
void FedProducer::fillHist(int iHist, double value) {
  cout << "Alles Mist" << endl;
  //   switch(iHist) {
  //   case 1: fH1->Fill(value); break; 
  //   default:
  //     cout << " histogram not defined " << iHist << endl;
  //   }

}

// ----------------------------------------------------------------------
//void FedProducer::fillBufferHist(int chan, unsigned long *buffer, int nevt) {
void FedProducer::fillBufferHist(int chan, uint32_t *buffer, int nevt) {
 
  fChannels[chan]->Reset();
  for(int i = 0; i < 250; ++i) {
    unsigned long data = (buffer[i] & 0xffc00000)>>22; // analyze word
    fChannels[chan]->SetBinContent(i+1, static_cast<float>(data));
    fH1->Fill(static_cast<float>(data));
    fChannelLevels[chan]->Fill(static_cast<float>(data));
  }

  if (nevt > 0) fChannels[chan]->SetTitle(Form("Channel %d, event: %d", chan+1, nevt));

  return;

}




// ----------------------------------------------------------------------
//void FedProducer::readBuffer(int channel, unsigned long *buffer, int nevt) {
void FedProducer::readBuffer(int channel, uint32_t *buffer, int nevt) {

  //  cout << "===> nevt: " << nevt << " channel: " << channel;
  fillBufferHist(channel, buffer, nevt);
  TSocket *s;
  if ((s = fMon->Select(20)) != (TSocket*)-1) {
    HandleSocket(s);
  }
}


// ----------------------------------------------------------------------
void FedProducer::readFile(const char *filename) {
  ifstream is(filename);
  int length, channel, d1, d2; 
  uint32_t buffer[(36*1024)]; // Data buffer for the FIFO (fifo1 = 36*1024?)

  int nevt(1); 
  while (!is.eof()) {
    is >> length 
       >> channel 
       >> d1 
       >> d2; 

    for (int i = 0; i < length; ++i) {
      is >> d1; 
      buffer[i] = static_cast<unsigned long>(d1<<22);
    }

    cout << "-> event " << nevt << " channel " << channel << " with length " << length << endl;       
    fillBufferHist(channel-1, buffer, nevt);

    TSocket *s;
    if ((s = fMon->Select(2000)) != (TSocket*)-1) {
      HandleSocket(s);
    }

  ++nevt;
  }


}



// ----------------------------------------------------------------------
void FedProducer::randomSignal() {
  uint32_t buffer[(36*1024)]; // Data buffer for the FIFO (fifo1 = 36*1024?)
  iniBuffer(buffer); 

  cout << " -> random signal generation" << endl;

  // Fill histograms randomly
  gRandom->SetSeed();
  const Int_t kUPDATE = 100;
  for (Int_t i = 0; ; i++) {
 
    // -- DK
    //for (int chan = 24; chan < 36; ++chan) {
    for (int chan = 0; chan < 36; ++chan) {
      iniBuffer(buffer); 
      fillBufferHist(chan, buffer);

      TSocket *s;
      if ((s = fMon->Select(20)) != (TSocket*)-1)
	HandleSocket(s);
      if (gROOT->IsInterrupted())
	break;
    }

    if (i && (i%kUPDATE) == 0) {
     
      // Check if there is a message waiting on one of the sockets.
      // Wait not longer than 20ms (returns -1 in case of time-out).
      TSocket *s;
      if ((s = fMon->Select(20)) != (TSocket*)-1)
	HandleSocket(s);
      if (gROOT->IsInterrupted())
	break;
    }
  }
}


// ----------------------------------------------------------------------
void FedProducer::iniBuffer(uint32_t *buffer) {

  int eps = static_cast<int>(50*(0.5-gRandom->Rndm()));
  
  for (int i =  0; i < 10; ++i) {
    eps = static_cast<int>(50*(0.5-gRandom->Rndm()));
    buffer[i] = (500+eps)<<22;
  }

  for (int i = 10; i < 13; ++i) {
    eps = static_cast<int>(50*(0.5-gRandom->Rndm()));
    buffer[i] = (200+eps)<<22;
  }

  eps = static_cast<int>(50*(0.5-gRandom->Rndm()));
  buffer[13] = (500+eps)<<22;

  for (int i = 14; i < 18; ++i) {
    eps = static_cast<int>(50*(0.5-gRandom->Rndm()));    
    buffer[i] = (700+eps)<<22;
  }

  for (int j = 0; j < 16; ++j) {
    eps = static_cast<int>(50*(0.5-gRandom->Rndm()));
    buffer[18+j*9+0] = (200+eps)<<22;
    eps = static_cast<int>(50*(0.5-gRandom->Rndm()));
    buffer[18+j*9+1] = (500+eps)<<22;
    eps = static_cast<int>(50*(0.5-gRandom->Rndm()));
    buffer[18+j*9+2] = (600+eps)<<22;
    eps = static_cast<int>(50*(0.5-gRandom->Rndm()));
    buffer[18+j*9+3] = (400+eps)<<22;
    eps = static_cast<int>(50*(0.5-gRandom->Rndm()));
    buffer[18+j*9+4] = (500+eps)<<22;
    eps = static_cast<int>(50*(0.5-gRandom->Rndm()));
    buffer[18+j*9+5] = (600+eps)<<22;
    eps = static_cast<int>(50*(0.5-gRandom->Rndm()));
    buffer[18+j*9+6] = (700+eps)<<22;
    eps = static_cast<int>(50*(0.5-gRandom->Rndm()));
    buffer[18+j*9+7] = (700+eps)<<22;
    eps = static_cast<int>(50*(0.5-gRandom->Rndm()));
    buffer[18+j*9+8] = (700+eps)<<22;
  }
}
