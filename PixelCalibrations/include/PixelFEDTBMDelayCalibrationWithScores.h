#ifndef _PixelFEDTBMDelayCalibrationWithScores_h_
#define _PixelFEDTBMDelayCalibrationWithScores_h_

#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"
#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"

#include <fstream>

class TString;
class TFile;
class TH1F;
class TH2F;

class PixelFEDTBMDelayCalibrationWithScores: public PixelFEDCalibrationBase {
 public:
  PixelFEDTBMDelayCalibrationWithScores(const PixelFEDSupervisorConfiguration&, SOAPCommander*);

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

  bool Dumps;
  std::vector<std::string> dacsToScan;
  unsigned lastTBMPLL;
  TFile* rootf;

  enum {
    nF1OK,
    nF1TBMHeaders, nF1TBMTrailers, nF1ROCHeaders, nF1Hits, nF1CorrectHits, nF1WrongHits,
    nF1TBMAHeaders, nF1TBMATrailers, nF1ROCAHeaders, nF1AHits, nF1ACorrectHits, nF1AWrongHits,
    nF1TBMBHeaders, nF1TBMBTrailers, nF1ROCBHeaders, nF1BHits, nF1BCorrectHits, nF1BWrongHits,
    nFib01ScoresOK, nFib02ScoresOK, nFib03ScoresOK, nFib04ScoresOK,
    nFib05ScoresOK, nFib06ScoresOK, nFib07ScoresOK, nFib08ScoresOK,
    nFib09ScoresOK, nFib10ScoresOK, nFib11ScoresOK, nFib12ScoresOK,
    nFib13ScoresOK, nFib14ScoresOK, nFib15ScoresOK, nFib16ScoresOK,
    nFib17ScoresOK, nFib18ScoresOK, nFib19ScoresOK, nFib20ScoresOK,
    nFib21ScoresOK, nFib22ScoresOK, nFib23ScoresOK, nFib24ScoresOK,
    nFib01AScoresOK, nFib02AScoresOK, nFib03AScoresOK, nFib04AScoresOK,
    nFib05AScoresOK, nFib06AScoresOK, nFib07AScoresOK, nFib08AScoresOK,
    nFib09AScoresOK, nFib10AScoresOK, nFib11AScoresOK, nFib12AScoresOK,
    nFib13AScoresOK, nFib14AScoresOK, nFib15AScoresOK, nFib16AScoresOK,
    nFib17AScoresOK, nFib18AScoresOK, nFib19AScoresOK, nFib20AScoresOK,
    nFib21AScoresOK, nFib22AScoresOK, nFib23AScoresOK, nFib24AScoresOK,
    nFib01BScoresOK, nFib02BScoresOK, nFib03BScoresOK, nFib04BScoresOK,
    nFib05BScoresOK, nFib06BScoresOK, nFib07BScoresOK, nFib08BScoresOK,
    nFib09BScoresOK, nFib10BScoresOK, nFib11BScoresOK, nFib12BScoresOK,
    nFib13BScoresOK, nFib14BScoresOK, nFib15BScoresOK, nFib16BScoresOK,
    nFib17BScoresOK, nFib18BScoresOK, nFib19BScoresOK, nFib20BScoresOK,
    nFib21BScoresOK, nFib22BScoresOK, nFib23BScoresOK, nFib24BScoresOK,
    nDecode
  };

  std::vector<TH1F*> scans1d[nDecode];
  std::vector<TH2F*> scans2d[nDecode];
};

#endif
