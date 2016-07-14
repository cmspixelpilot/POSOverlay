#ifndef _PixelTBMDelayCalibrationWithScores_h_
#define _PixelTBMDelayCalibrationWithScores_h_

#include "PixelCalibrations/include/PixelCalibrationBase.h"

class PixelTBMDelayCalibrationWithScores : public PixelCalibrationBase {
 public:
  PixelTBMDelayCalibrationWithScores(const PixelSupervisorConfiguration&, SOAPCommander*);

  void beginCalibration();
  virtual bool execute();
  void endCalibration();
  virtual std::vector<std::string> calibrated();

  bool DelayBeforeFirstTrigger;
  bool DelayEveryTrigger;
};

#endif
