// $Id: PixelTemperatureCalibration.h,v 1.8 2008/03/25 16:59:09 aryd Exp $

/*************************************************************************
 * Class implementing thermal cycles for temperature calibration;        *
 * note that this class implements the thermal cycle only,               *
 * the individual Last DAC and DCU calibration routines                  *
 * are implemented as plug-ins                                           *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2008/03/25 16:59:09 $ (UTC)                       *
 *          by: $Author: aryd $                                       *
 *************************************************************************/

#ifndef _PixelTemperatureCalibration_h_
#define _PixelTemperatureCalibration_h_

#include <string>
#include <time.h>
#include <list>
#include <fstream>

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelSupervisorConfiguration/include/PixelSupervisorConfiguration.h"

#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSSOAPCommander.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSPVSSCommander.h"

#include "PixelCalibrations/include/PixelCalibrationBase.h"

class PixelTemperatureCalibrationPlugin;

class PixelTemperatureCalibration : public PixelCalibrationBase 
{
 public:
  PixelTemperatureCalibration(const PixelSupervisorConfiguration& configurationParameters, 
			      SOAPCommander* soapCommander, 
			      PixelDCSSOAPCommander* dcs_soapCommander, 
			      PixelDCSPVSSCommander* pvssCommander);

  virtual ~PixelTemperatureCalibration();

  virtual bool execute();

  virtual std::vector<std::string> calibrated();

  virtual void printConfiguration(std::ostream& stream) const;    //--AZ: should be virtual

  enum cycles { kCoolingCycle, kHeatingCycle };

 private:
  void loadConfigFile() throw (xdaq::exception::Exception);
  
  void doHeatingCycle(unsigned int iCycle);
  void doCoolingCycle(unsigned int iCycle);

  void setTemperature(double temperature) throw (xdaq::exception::Exception);

  void doCalibration(unsigned int iCycle = 0);

  void archiveConfigurationParameters(std::ostream& stream) const;
  void archiveDACSettings(std::ostream& stream) const;
  void archiveHeader(std::ostream& stream) const;
  void archivePVSSDpeData(std::ostream& stream) const;
  void archiveTrailer(std::ostream& stream) const;

  std::string configFileName_;
  bool configFileLoaded_;

  std::list<PixelDCSPVSSDpe> archivedDataPointElements_;

  PixelDCSSOAPCommander* dcs_soapCommander_;
  PixelDCSPVSSCommander* pvssCommander_;

  std::list<PixelTemperatureCalibrationPlugin*> plugins_;

  std::string dataFileName_;
  std::ofstream* dataFile_;

  unsigned int numCycles_;
  unsigned int startCycle_;
  double minTemperature_;
  double maxTemperature_;
  double stepSizeTemperature_;
  std::string dpeNameBathTemperatureReading_;
  std::string dpeNameBathTemperatureSetting_;
  double maxTemperatureDeviationBath_;
  unsigned int maxDelayBath_;
  unsigned int addDelayBath_;

  double currentNominalTemperature_;
  std::string currentTemperatureCycle_;

  time_t calibrationStartTime_;
  time_t calibrationEndTime_;
};

#endif
