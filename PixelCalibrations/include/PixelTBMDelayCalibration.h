#ifndef _PixelTBMDelayCalibration_h_
#define _PixelTBMDelayCalibration_h_

#include "PixelCalibrations/include/PixelCalibrationBase.h"

class PixelTBMDelayCalibration : public PixelCalibrationBase {
 public:
  PixelTBMDelayCalibration(const PixelSupervisorConfiguration&, SOAPCommander*);

  void beginCalibration();
  virtual bool execute();
  void endCalibration();
  virtual std::vector<std::string> calibrated();

  bool ToggleChannels;
  bool CycleScopeChannels;
  bool DelayBeforeFirstTrigger;
  bool DelayEveryTrigger;
};

#endif
