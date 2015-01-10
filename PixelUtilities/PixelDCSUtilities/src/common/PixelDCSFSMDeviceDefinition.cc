#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMDeviceDefinition.h"

/*************************************************************************
 * Auxiliary class for storage of all information neccessary             *
 * for translation between XDAQ FSM states on the one hand               *
 * and PVSS data-point values and FSM colors on the other hand           *
 * for a single FSM device (device unit)                                 *
 *                                                                       *
 * Author: Christian Veelken, UC Davis			 	         *
 *                                                                       *
 * Last update: $Date: 2009/07/29 14:22:16 $ (UTC)                       *
 *          by: $Author: joshmt $                                       *
 *************************************************************************/

#include <iostream>
#include <iomanip>

PixelDCSFSMDeviceDefinition::PixelDCSFSMDeviceDefinition(const std::string& type,
							 const std::list<std::pair<PixelDCSFSMStateToDpValueTranslation, std::string> >& stateList)
  : fsmType_(type)
{
//--- initialize PVSS to XDAQ 
//    state translation map
  for ( std::list<std::pair<PixelDCSFSMStateToDpValueTranslation, std::string> >::const_iterator state = stateList.begin();
	state != stateList.end(); ++state ) {
    stateMap_[state->first.getXdaqState()] = std::pair<std::string, std::string>(state->first.getPvssDpValue(), state->second);
  }
}

PixelDCSFSMDeviceDefinition::PixelDCSFSMDeviceDefinition(const PixelDCSFSMDeviceDefinition& deviceDefinition)
{
  //  std::cout << "<PixelDCSFSMDeviceDefinition::PixelDCSFSMDeviceDefinition>:" << std::endl;

  fsmType_ = deviceDefinition.fsmType_;
  dpType_ = deviceDefinition.dpType_;

  for ( std::map<std::string, std::pair<std::string, std::string> >::const_iterator stateMapEntry = deviceDefinition.stateMap_.begin();
	stateMapEntry != deviceDefinition.stateMap_.end(); ++stateMapEntry ) {
    stateMap_[stateMapEntry->first] = std::pair<std::string, std::string>(stateMapEntry->second.first, stateMapEntry->second.second);
  }
}

void PixelDCSFSMDeviceDefinition::writeTo(std::ostream& stream) const
{
//--- print device type
  stream << "   " 
	 << "fsmType = " << fsmType_ << std::endl;

//--- print state translation table
  for ( std::map<std::string, std::pair<std::string, std::string> >::const_iterator state = stateMap_.begin();
	state != stateMap_.end(); ++state ) {
    stream << "    state: XDAQ name = " << std::setw(13) << state->first << ","
	   << " PVSS dpValue = " << state->second.first << ","
	   << " color = " << state->second.second << std::endl;
  }
}
