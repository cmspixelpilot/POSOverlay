// $Id: PixelTemperatureCalibrationPlugin.h,v 1.6 2007/11/12 16:26:58 ursl Exp $

/*************************************************************************
 * Base class for Last DAC and DCU calibration routines,                 *
 * implemented as plug-ins for PixelTemperatureCalibration class         *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2007/11/12 16:26:58 $ (UTC)                       *
 *          by: $Author: ursl $                                       *
 *************************************************************************/

#ifndef _PixelTemperatureCalibrationPlugin_h_
#define _PixelTemperatureCalibrationPlugin_h_

#include <fstream>

#include "xdaq/exception/Exception.h"

#include "xercesc/dom/DOMNode.hpp"

#include "PixelSupervisorConfiguration/include/PixelSupervisorConfiguration.h"

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSSOAPCommander.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSPVSSCommander.h"

class PixelTemperatureCalibrationPlugin
{
 public:
  PixelTemperatureCalibrationPlugin(PixelSupervisorConfiguration* globalConfigurationParameters,
				    SOAPCommander* soapCommander, PixelDCSSOAPCommander* dcs_soapCommander, PixelDCSPVSSCommander* pvssCommander,
				    xercesc::DOMNode* pluginConfigNode); 
  virtual ~PixelTemperatureCalibrationPlugin() {};
  
  void setDataFile(std::ofstream* dataFile) { dataFile_ = dataFile; }

  void SetNominalTemperature(double nominalTemperature) { currentNominalTemperature_ = nominalTemperature; }
  void SetTemperatureCycle(const std::string& temperatureCycle) { currentTemperatureCycle_ = temperatureCycle; }

  virtual void execute(unsigned int iCycle, unsigned int numCycles) throw (xdaq::exception::Exception) = 0;

  virtual void printConfiguration(std::ostream& stream) const {};

  virtual void archiveConfigurationParameters(std::ostream& stream) const {};

 protected:
  virtual void loadConfiguration() throw (xdaq::exception::Exception) {};
  
  virtual void archiveCalibrationData(std::ostream& stream) {};

  bool executeCalibration_;

  PixelSupervisorConfiguration* globalConfigurationParameters_;

  SOAPCommander* soapCommander_;
  PixelDCSSOAPCommander* dcs_soapCommander_;
  PixelDCSPVSSCommander* pvssCommander_;

  xercesc::DOMNode* pluginConfigNode_;

  //DiagBagWizard* diagService_;

  std::ofstream* dataFile_;

  double currentNominalTemperature_;
  std::string currentTemperatureCycle_;
};

#endif
