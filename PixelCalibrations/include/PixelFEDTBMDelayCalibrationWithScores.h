#ifndef _PixelFEDTBMDelayCalibrationWithScores_h_
#define _PixelFEDTBMDelayCalibrationWithScores_h_

#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"
#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"
#include "TString.h"
#include <fstream>

class TFile;
class TH1;

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

  int OverrideFifo1Fiber;
  int the_col, the_row;

  bool Dumps;
  std::vector<std::string> dacsToScan;
  TFile* rootf;

  struct Key {
    int fednumber;
    int fedchannel; // -24 to -1 for fiber number, 1-48 for ch number
    std::string scoretype;

    Key(int fedn) : fednumber(fedn), fedchannel(0), scoretype("") {}

    TString name() const {
      if (fedchannel < 0)
        return TString::Format("FED%04i_Fb%02i_%s", fednumber, -fedchannel, scoretype.c_str());
      else
        return TString::Format("FED%04i_Ch%02i_%s", fednumber,  fedchannel, scoretype.c_str());
    }

    Key& operator()(int fedch, const std::string& scoret) {
      fedchannel = fedch;
      scoretype = scoret;
      return *this;
    }

    bool operator<(const Key& r) const {
      const Key& l = *this;
      if (l.fednumber < r.fednumber) return true;
      if (l.fednumber > r.fednumber) return false;
      if (l.fedchannel < r.fedchannel) return true;
      if (l.fedchannel > r.fedchannel) return false;
      if (l.scoretype < r.scoretype) return true;
      return false;
    }
  };

  std::map<Key, std::vector<TH1*> > scans;

  void BookEm(const Key& key);
  void FillEm(unsigned state, const Key& key, float c);

  PixelTimer timer;
};

#endif
