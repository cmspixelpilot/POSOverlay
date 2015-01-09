// $Id: PixelTemperatureCalibrationPluginDCU.h,v 1.7 2012/01/20 19:14:13 kreis Exp $

/*************************************************************************
 * Class for DCU calibration routines,                                   *
 * implemented as plug-in for PixelTemperatureCalibration class          *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2012/01/20 19:14:13 $ (UTC)                       *
 *          by: $Author: kreis $                                       *
 *************************************************************************/

#ifndef _PixelTemperatureCalibrationPluginDCU_h_
#define _PixelTemperatureCalibrationPluginDCU_h_

#include "xercesc/dom/DOMNode.hpp"

#include "xdaq/exception/Exception.h"

#include "PixelCalibrations/include/PixelTemperatureCalibrationPlugin.h"

#include "PixelUtilities/PixelTKFECDataTools/include/PortCardDCU.h"

#include <string>
#include <vector>
#include <map>

class SOAPCommander;
class PixelDCSSOAPCommander;
class PixelDCSPVSSCommander;

class PixelTemperatureCalibrationPluginDCU : public PixelTemperatureCalibrationPlugin
{
 public:
  PixelTemperatureCalibrationPluginDCU(PixelSupervisorConfiguration* globalConfigurationParameters,
				       SOAPCommander* soapCommander, 
				       PixelDCSSOAPCommander* dcs_soapCommander, 
				       PixelDCSPVSSCommander* pvssCommander,
				       xercesc::DOMNode* pluginConfigNode);  
  ~PixelTemperatureCalibrationPluginDCU();
  
  void execute(unsigned int iCycle, unsigned int numCycles) throw (xdaq::exception::Exception);

 protected:
  void loadConfiguration() throw (xdaq::exception::Exception);

  void archiveConfigurationParameters(std::ostream& stream) const;

  void archiveCalibrationData(std::ostream& stream) const;

  void printConfiguration(std::ostream& stream) const;
  
private:
   void decodeDCUReadings(xoap::MessageReference soapMessage);
  
private:
  unsigned int numPoints_; 
  std::string mode_;                      // LIR = Low Input Mode or HIR = High Input Mode
  
  std::map<PortCard::Address, std::vector<PortCard::DCU> > address_vpoints_map_;
};

#endif
