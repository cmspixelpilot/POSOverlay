#ifndef _PixelFEDTBMDelayCalibrationBPIX_h_
#define _PixelFEDTBMDelayCalibrationBPIX_h_

#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"
#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelScanRecord.h"

#include <cstdint>
#include <fstream>

class TFile;
class TH1F;
class TH2F;
class TH3F;

class PixelFEDTBMDelayCalibrationBPIX: public PixelFEDCalibrationBase {
 public:
  PixelFEDTBMDelayCalibrationBPIX(const PixelFEDSupervisorConfiguration&, SOAPCommander*);

  virtual void initializeFED();
  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);
  virtual xoap::MessageReference execute(xoap::MessageReference msg);
  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

 private:
  void RetrieveData(unsigned int state);
  void Analyze();
  void CloseRootf();
  void BookEm(const TString& path);
  void FillEm(unsigned state,int fedid, int ch, int which, float c);

  bool DumpFIFOs;
  bool PrintHits;
  bool ReadFifo1;
  std::vector<std::string> dacsToScan;
  unsigned lastTBMPLL;
  TFile* rootf;
  bool inject_;

  std::map<int,std::map<int,std::vector<TH1F*> > > ntrigsTBM;
  std::map<int,std::map<int,std::vector<TH2F*> > > scansTBM;
  std::map<std::string,std::vector<TH2F*> > TBMsHistoSum;

   struct branch{
    float pass;
    char moduleName[38];
  };

  struct branch_sum{
    int deltaTBMPLLdelayX;
    int deltaTBMPLLdelayY;
    int newTBMPLLdelayX;
    int newTBMPLLdelayY;
    double efficiency;
    char moduleName[38];
  };

};

#endif
