#ifndef _PixelPOHBiasCalibration_h_
#define _PixelPOHBiasCalibration_h_

#include "PixelCalibrations/include/PixelCalibrationBase.h"

class TFile;
class TGraphErrors;

class PixelPOHBiasCalibration: public PixelCalibrationBase {
 public:
  PixelPOHBiasCalibration(const PixelSupervisorConfiguration &, SOAPCommander*);
  void beginCalibration();
  virtual bool execute();
  void endCalibration();

  virtual std::vector<std::string> calibrated();

 private:
  std::vector<unsigned> POHGains;
  unsigned POHBiasMin;
  unsigned POHBiasNSteps;
  unsigned POHBiasStepSize;

  unsigned key(int gain, int NFed, int NFiber) { return (gain << 30) | (NFed << 5) | NFiber; }

  std::map<unsigned, TGraphErrors*> rssi_v_bias;
};

#endif

