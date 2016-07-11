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

  bool OnlyFIFO1;
  bool OnlyFIFO3;
  bool DumpFIFOs;
  bool PrintHits;
  std::vector<std::string> dacsToScan;
  unsigned lastTBMPLL;
  TFile* rootf;

  enum { 
    F11almostFull, F13almostFull, F15almostFull, F17almostFull,
    F21almostFull, F23almostFull, F25almostFull, F27almostFull,
    F31almostFull, F37almostFull,
    FT1nTBMHeader, FT1nTBMHeaders, FT1nTBMTrailer, FT1nTBMTrailers, FT1nROCHeaders, FT1wrongPix, FT1rightPix,
    FT7nTBMHeader, FT7nTBMHeaders, FT7nTBMTrailer, FT7nTBMTrailers, FT7nROCHeaders, FT7wrongPix, FT7rightPix,
    FS1nTBMHeader, FS1nTBMTrailer, FS1nROCHeaders, FS1wrongPix, FS1rightPix, FS1dangling,
    FS3nTBMHeader, FS3nTBMTrailer, FS3nROCHeaders, FS3wrongPix, FS3rightPix, FS3dangling,
    FS5nTBMHeader, FS5nTBMTrailer, FS5nROCHeaders, FS5wrongPix, FS5rightPix, FS5dangling,
    FS7nTBMHeader, FS7nTBMTrailer, FS7nROCHeaders, FS7wrongPix, FS7rightPix, FS7dangling,
    F1nTBMHeaders, F1nTBMTrailers, F1nROCHeaders,
    F1nTBMAHeaders, F1nTBMATrailers, F1nROCAHeaders,
    F1nTBMBHeaders, F1nTBMBTrailers, F1nROCBHeaders,
    F3fifoErr, F3wrongRoc, F3wrongPix, F3rightPix,
    nDecode
  };

  std::vector<TH1F*> scans1d[nDecode];
  std::vector<TH2F*> scans2d[nDecode];
};

#endif
