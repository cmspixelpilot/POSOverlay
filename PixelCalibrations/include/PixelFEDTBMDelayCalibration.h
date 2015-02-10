#ifndef _PixelFEDTBMDelayCalibration_h_
#define _PixelFEDTBMDelayCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"

#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelScanRecord.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"

#include <fstream>
#include <map>

class TFile;
class TH1F;
class TH2F;

class PixelFEDTBMDelayCalibration: public PixelFEDCalibrationBase {
 public:

  PixelFEDTBMDelayCalibration( const PixelFEDSupervisorConfiguration &, SOAPCommander* );
  virtual ~PixelFEDTBMDelayCalibration(){};

  virtual xoap::MessageReference execute(xoap::MessageReference msg);

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  virtual void initializeFED();

 private:

  // PixelFEDTBMDelayCalibration Constructor should never be called
  //PixelFEDTBMDelayCalibration();

  // Calibration steps
  void RetrieveData(unsigned int state);
  void Analyze();

  std::ofstream retrf;
  TFile* rootf;

  std::vector<std::string> dacstoscan;

  TH1F* h_nfiforeaderrors;
  TH1F* h_nerrors;
  TH1F* h_nhits;
  TH1F* h_nskip;
  enum { wrongPix, rightPix, nDecode };
  std::vector<TH1F*> scans1d[nDecode];
  std::vector<TH2F*> scans2d[nDecode];
};

#endif
