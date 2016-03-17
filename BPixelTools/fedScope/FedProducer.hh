#ifndef FEDPRODUCER
#define FEDPRODUCER

#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include "TCanvas.h"
#include "TFrame.h"
#include "TSocket.h"
#include "TServerSocket.h"
#include "TMonitor.h"
#include "TMessage.h"
#include "TRandom.h"
#include "TList.h"
#include "TError.h"
#include <stdint.h>
// ----------------------------------------------------------------------
// FedProducer
// -----------
// Class to produce dummy FED data to setup the 'FED-scope'
// ----------------------------------------------------------------------

class FedProducer {
public:
  FedProducer();
  ~FedProducer();
  void HandleSocket(TSocket *s);
  void iniBuffer(uint32_t *buffer);
  void randomSignal();
  void readFile(const char *filename);
  void readBuffer(int channel, uint32_t *buffer, int nevt = 0);
  float readBuffer2(int channel, uint32_t *buffer, int nevt = 0);
  void fillHist(int iHist, double value);
  void fillBufferHist(int chan, uint32_t *buffer, int nevt = 0);
  float fillBufferHist2(int chan, uint32_t *buffer, int nevt = 0);

private:
  TServerSocket *fServ;      // server socket
  TMonitor      *fMon;       // socket monitor
  TList         *fSockets;   // list of open spy sockets

  // -- histograms
  TH1D          *fH1; 
  TH1D          *fChannels[36];
  TH1D          *fChannelLevels[36];

};


#endif

