#ifndef _PixelTBMDelayCalibrationBPIX_h_
#define _PixelTBMDelayCalibrationBPIX_h_

#include "PixelCalibrations/include/PixelCalibrationBase.h"

class PixelTBMDelayCalibrationBPIX : public PixelCalibrationBase {
 public:
  PixelTBMDelayCalibrationBPIX(const PixelSupervisorConfiguration&, SOAPCommander*);

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
