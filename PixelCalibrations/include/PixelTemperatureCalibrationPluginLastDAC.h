// $Id: PixelTemperatureCalibrationPluginLastDAC.h,v 1.7 2007/11/07 16:06:22 veelken Exp $

/*************************************************************************
 * Class for Last DAC calibration routines,                              *
 * implemented as plug-in for PixelTemperatureCalibration class          *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2007/11/07 16:06:22 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#ifndef _PixelTemperatureCalibrationPluginLastDAC_h_
#define _PixelTemperatureCalibrationPluginLastDAC_h_

#include <map>
#include <list>

#include "xercesc/dom/DOMNode.hpp"

#include "xdaq/exception/Exception.h"

#include <diagbag/DiagBagWizard.h>

#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"
#include "CalibFormats/SiPixelObjects/interface/PixelModuleName.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACSettings.h"

#include "PixelCalibrations/include/PixelTemperatureCalibrationPlugin.h"

class SOAPCommander;
class PixelDCSSOAPCommander;
class PixelDCSPVSSCommander;

class PixelTemperatureCalibrationPluginLastDAC : public PixelTemperatureCalibrationPlugin
{
 public:
  PixelTemperatureCalibrationPluginLastDAC(PixelSupervisorConfiguration* globalConfigurationParameters,
					   SOAPCommander* soapCommander, 
					   PixelDCSSOAPCommander* dcs_soapCommander, 
					   PixelDCSPVSSCommander* pvssCommander,
					   xercesc::DOMNode* pluginConfigNode); 
  ~PixelTemperatureCalibrationPluginLastDAC();
  
  void execute(unsigned int iCycle, unsigned int numCycles) throw (xdaq::exception::Exception);

  void printConfiguration(std::ostream& stream) const;

  void archiveConfigurationParameters(std::ostream& stream) const;

  enum powerMode { kNominalPower, kReducedPower };

 private:
  void loadConfiguration() throw (xdaq::exception::Exception);

  void loadPixelDACSettings(const pos::PixelDetectorConfig* detectorConfiguration);

  void clearLastDACReadings();
  void decodeLastDACReadings(xoap::MessageReference soapMessage);

  void archiveCalibrationData(std::ostream& stream,
			      std::map<pos::PixelROCName, unsigned int>& dacValues_Vana,
			      std::map<pos::PixelROCName, unsigned int>& dacValues_Vsf,
			      unsigned int dacValue_TempRange) const;

  unsigned int minTempRangeDAC_;
  unsigned int maxTempRangeDAC_;
  unsigned int stepSizeTempRangeDAC_;
  unsigned int numTriggers_; 

  std::map<pos::PixelModuleName, pos::PixelDACSettings*> dacSettingsModule_;

/*
//--- mapping between PixelFEC hardware address and Read-out Chip name;
//     format = [fecBoard][mfec][mfecChannel][hubAddress][portAddress][rocId] --> ROC name
  std::map<unsigned int, std::map<unsigned int, std::map<unsigned int, 
    std::map<unsigned int, std::map<unsigned int, std::map<unsigned int, const pos::PixelROCName*> > > > > > pixelfecReadOutChipName_;
//--- PixelFEC settings;
//     format = [fecBoard][mfec][mfecChannel][hubAddress][portAddress][rocId] --> { DAC name, DAC setting }
  std::map<unsigned int, std::map<unsigned int, std::map<unsigned int, 
    std::map<unsigned int, std::map<unsigned int, std::map<unsigned int, std::list<std::pair<std::string, unsigned int> > > > > > > > pixelfecData_;
 */

//--- mapping between FED hardware address and Read-out Chip name;
//     format = [fedBoard][fedChannel][rocId] --> ROC name
  std::map<unsigned int, std::map<unsigned int, std::map<unsigned int, const pos::PixelROCName*> > > fedReadOutChipName_;
//--- FED readings;
//     format = [fedBoard][fedChannel][rocId] --> ADC value
  std::map<unsigned int, std::map<unsigned int, std::map<unsigned int, std::list<int> > > > fedData_;
};

#endif
