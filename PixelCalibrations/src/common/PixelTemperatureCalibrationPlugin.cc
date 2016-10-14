#include "PixelTemperatureCalibrationPlugin.h"

// $Id: PixelTemperatureCalibrationPlugin.cc,v 1.2 2007/08/13 14:43:53 veelken Exp $

/*************************************************************************
 * Base class for Last DAC and DCU calibration routines,                 *
 * implemented as plug-ins for PixelTemperatureCalibration class         *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2007/08/13 14:43:53 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

PixelTemperatureCalibrationPlugin::PixelTemperatureCalibrationPlugin(PixelSupervisorConfiguration* globalConfigurationParameters,
								     SOAPCommander* soapCommander, 
								     PixelDCSSOAPCommander* dcs_soapCommander, 
								     PixelDCSPVSSCommander* pvssCommander,
								     xercesc::DOMNode* pluginConfigNode)
  : globalConfigurationParameters_(globalConfigurationParameters), 
    soapCommander_(soapCommander), dcs_soapCommander_(dcs_soapCommander), pvssCommander_(pvssCommander),
    pluginConfigNode_(pluginConfigNode)
{
//--- nothing to be done yet...
}
