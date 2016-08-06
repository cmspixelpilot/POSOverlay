/*************************************************************************
 * PixelROCDelay25Calibration Class header file,                         *
 * class to set different Delay25 settings and different DAC settings    *
 * that then requests the FED to determine if the ROCs reconfigured      *
 * themselves via the PixelFEDROCDelay25Calibration class                *
 *                                                                       *
 * Author: James Zabel, Rice University                                  *
 *                                                                       *
 * Last update: $Date: 2012/06/16 14:13:19 $ (UTC)                       *
 *          by: $Author: mdunser $                                        *
 *************************************************************************/

#ifndef _PixelROCDelay25Calibration_h_
#define _PixelROCDelay25Calibration_h_

#include "PixelCalibrations/include/PixelCalibrationBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"

class PixelROCDelay25Calibration: public PixelCalibrationBase {
  public:
    PixelROCDelay25Calibration(const PixelSupervisorConfiguration&, SOAPCommander*);
    virtual ~PixelROCDelay25Calibration() {};
    void beginCalibration();
    bool execute();
    void endCalibration();
    std::vector<std::string> calibrated();

  private:
    PixelTimer totalTime_;
};

#endif
