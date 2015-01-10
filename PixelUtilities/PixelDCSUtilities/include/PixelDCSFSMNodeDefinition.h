// $Id: PixelDCSFSMNodeDefinition.h,v 1.1 2007/08/10 14:47:03 veelken Exp $

/*************************************************************************
 * Auxiliary class for storage of all information neccessary             *
 * for translation between XDAQ FSM states and commands on the one hand  *
 * and PVSS FSM states and commands on the other hand                    *
 * for a single FSM node (logical or control unit)                       *
 *                                                                       *
 * Author: Christian Veelken, UC Davis			 	         *
 *                                                                       *
 * Last update: $Date: 2007/08/10 14:47:03 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#ifndef _PixelDCSFSMNodeDefinition_h_
#define _PixelDCSFSMNodeDefinition_h_

#include <string>
#include <list>
#include <map>

#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMStateTranslation.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMCommandTranslation.h"

class PixelDCSFSMNodeDefinition
{
 public:
  
  PixelDCSFSMNodeDefinition() {} // default constructor, neccessary to insert PixelDCSFSMNodeDefinition objects into std::map
  PixelDCSFSMNodeDefinition(const std::string& type,
			    const std::list<std::pair<PixelDCSFSMStateTranslation, std::string> >& stateList,
			    const std::list<PixelDCSFSMCommandTranslation>& commandList);
  ~PixelDCSFSMNodeDefinition() {}
  
  const std::string getType() const { return type_; }
  
  std::list<std::string> getXdaqStateList() const;
  const std::string& getXdaqState(const std::string& pvssState) const { return stateMap_[pvssState].first; }
  const std::string& getColor(const std::string& pvssState) const { return stateMap_[pvssState].second; }
  
  std::list<std::string> getPvssCommandList() const;
  const std::pair<std::string, std::string>& getStateDefinition(const std::string& pvssState) const { return stateMap_[pvssState]; }
  const std::string& getCommandDefinition(const std::string& xdaqCommand, const std::string& xdaqState) const { return commandMap_[xdaqCommand][xdaqState]; }
  
  void writeTo(std::ostream& stream) const;
  
 protected:
  
  std::string type_; // type of PVSS FSM node
  mutable std::map<std::string, std::pair<std::string, std::string> > stateMap_;  // map PVSS state --> XDAQ state, color
  mutable std::map<std::string, std::map<std::string, std::string> > commandMap_; // map XDAQ command, XDAQ state --> PVSS command 
                                                                                  // (association between XDAQ and PVSS commands is not unique,
                                                                                  //  but depends on the state)
};

#endif
