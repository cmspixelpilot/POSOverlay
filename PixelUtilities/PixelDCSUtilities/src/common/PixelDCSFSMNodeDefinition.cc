#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMNodeDefinition.h"

/*************************************************************************
 * Auxiliary class for storage of all information neccessary             *
 * for translation between XDAQ FSM states and commands on the one hand  *
 * and PVSS FSM states and commands on the other hand                    *
 * for a single FSM node (logical or control unit)                       *
 *                                                                       *
 * Author: Christian Veelken, UC Davis			 	         *
 *                                                                       *
 * Last update: $Date: 2007/08/10 14:47:04 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#include <iostream>
#include <iomanip>

PixelDCSFSMNodeDefinition::PixelDCSFSMNodeDefinition(const std::string& type,
						     const std::list<std::pair<PixelDCSFSMStateTranslation, std::string> >& stateList,
						     const std::list<PixelDCSFSMCommandTranslation>& commandList)
  : type_(type)
{
//--- initialize PVSS to XDAQ 
//    state translation map
  for ( std::list<std::pair<PixelDCSFSMStateTranslation, std::string> >::const_iterator state = stateList.begin();
	state != stateList.end(); ++state ) {
    stateMap_[state->first.getPvssState()] = std::pair<std::string, std::string>(state->first.getXdaqState(), state->second);
  }

//--- initialize XDAQ to PVSS
//    command translation map
  for ( std::list<PixelDCSFSMCommandTranslation>::const_iterator command = commandList.begin();
	command != commandList.end(); ++command ) {
    commandMap_[command->getXdaqCommand()][command->getXdaqState()] = command->getPvssCommand();
  }
}

std::list<std::string> PixelDCSFSMNodeDefinition::getXdaqStateList() const
{
  std::list<std::string> xdaqStateList;
  for ( std::map<std::string, std::pair<std::string, std::string> >::const_iterator state = stateMap_.begin();
	state != stateMap_.end(); ++state ) {
    xdaqStateList.push_back(state->second.first);
  }

  return xdaqStateList;
}

std::list<std::string> PixelDCSFSMNodeDefinition::getPvssCommandList() const
{
  std::list<std::string> pvssCommandList;
  for ( std::map<std::string, std::map<std::string, std::string> >::const_iterator command1 = commandMap_.begin();
	command1 != commandMap_.end(); ++command1 ) {
    for ( std::map<std::string, std::string>::const_iterator command2 = command1->second.begin();
	  command2 != command1->second.end(); ++command2 ) {
      pvssCommandList.push_back(command2->second);
    }
  }
  
  return pvssCommandList;
}

void PixelDCSFSMNodeDefinition::writeTo(std::ostream& stream) const
{
//--- print device type
  stream << "   " 
	 << "type = " << type_ << std::endl;

//--- print state translation table
  for ( std::map<std::string, std::pair<std::string, std::string> >::const_iterator state = stateMap_.begin();
	state != stateMap_.end(); ++state ) {
    stream << "    state: PVSS name = " << std::setw(13) << state->first << ","
	   << " XDAQ name = " << state->second.first << ","
	   << " color = " << state->second.second << std::endl;
  }

//--- print command translation table
  for ( std::map<std::string, std::map<std::string, std::string> >::const_iterator command1 = commandMap_.begin();
	command1 != commandMap_.end(); ++command1 ) {
    for ( std::map<std::string, std::string>::const_iterator command2 = command1->second.begin();
	  command2 != command1->second.end(); ++command2 ) {
      stream << "    command: XDAQ name = " << std::setw(13) << command1->first 
	     << " defined for XDAQ state = " << command2->first << ","
	     << " PVSS name = " << command2->second << std::endl;
    }
  }
}
