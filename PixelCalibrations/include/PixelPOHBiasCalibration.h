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
  unsigned POHBiasMin;
  unsigned POHBiasNSteps;
  unsigned POHBiasStepSize;

  std::map<int, TGraphErrors*> rssi_v_bias;
};

#endif

