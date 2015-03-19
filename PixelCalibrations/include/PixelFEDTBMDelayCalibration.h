#ifndef _PixelFEDTBMDelayCalibration_h_
#define _PixelFEDTBMDelayCalibration_h_

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

class PixelFEDTBMDelayCalibration: public PixelFEDCalibrationBase {
 public:
  PixelFEDTBMDelayCalibration(const PixelFEDSupervisorConfiguration&, SOAPCommander*);

  virtual void initializeFED();
  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);
  virtual xoap::MessageReference execute(xoap::MessageReference msg);
  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

 private:
  void RetrieveData(unsigned int state);
  void Analyze();
  void CloseRootf();
  void BookEm(const TString& path);
  void FillEm(unsigned state, int which, float c);

  bool DumpFIFOs;
  std::vector<std::string> dacsToScan;
  unsigned lastTBMPLL;
  TFile* rootf;

  enum { 
    F11nTBMHeader, F11nTBMHeaders, F11nTBMTrailer, F11nTBMTrailers, F11nROCHeaders,
    F17nTBMHeader, F17nTBMHeaders, F17nTBMTrailer, F17nTBMTrailers, F17nROCHeaders,
    F3fifoErr, F3wrongRoc, F3wrongPix, F3rightPix,
    nDecode
  };

  std::vector<TH1F*> scans1d[nDecode];
  std::vector<TH2F*> scans2d[nDecode];
};

#endif
