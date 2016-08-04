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
  void SetAOHBiasToCurrentValue(std::string portCardName, int AOHNumber, int AOHBiasNumber);

 private:
  std::vector<unsigned> POHGains;
  unsigned POHBiasMin;
  unsigned POHBiasNSteps;
  unsigned POHBiasStepSize;
  unsigned POHBiasMax;
  bool DoFits;
  bool SetBiasEnMass;

  unsigned key(int gain, int NFed, int NFiber) { return (gain << 30) | (NFed << 5) | NFiber; }

  std::map<unsigned, TGraphErrors*> rssi_v_bias;
  std::map<unsigned, unsigned> selected_poh_bias_values;
  std::map<std::string, std::map<unsigned, unsigned>> bias_values_by_portcard_and_aoh;
};

#endif
