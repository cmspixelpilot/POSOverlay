// $Id: PixelDCSFSMDeviceDefinition.h,v 1.2 2007/10/09 17:58:03 veelken Exp $

/*************************************************************************
 * Auxiliary class for storage of all information neccessary             *
 * for translation between XDAQ FSM states on the one hand               *
 * and PVSS data-point values and FSM colors on the other hand           *
 * for a single FSM device (device unit)                                 *
 *                                                                       *
 * Author: Christian Veelken, UC Davis			 	         *
 *                                                                       *
 * Last update: $Date: 2007/10/09 17:58:03 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#ifndef _PixelDCSFSMDeviceDefinition_h_
#define _PixelDCSFSMDeviceDefinition_h_

#include <string>
#include <list>
#include <map>

#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMStateTranslation.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMCommandTranslation.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMStateToDpValueTranslation.h"

class PixelDCSFSMDeviceDefinition
{
 public:
  PixelDCSFSMDeviceDefinition() {} // default constructor, neccessary to insert PixelDCSFSMDeviceDefinition objects into std::map
  PixelDCSFSMDeviceDefinition(const std::string& type,
			      const std::list<std::pair<PixelDCSFSMStateToDpValueTranslation, std::string> >& stateList);
  PixelDCSFSMDeviceDefinition(const PixelDCSFSMDeviceDefinition& deviceDefinition);
  ~PixelDCSFSMDeviceDefinition() {}
  
  const std::string getType() const { return fsmType_; }

  const std::string getDpType() const { return dpType_; }

  const std::string& getDpValue(const std::string& xdaqState) const { return stateMap_[xdaqState].first; }
  const std::string& getColor(const std::string& xdaqState) const { return stateMap_[xdaqState].second; }
  
  const std::pair<std::string, std::string>& getStateDefinition(const std::string& xdaqState) const { return stateMap_[xdaqState]; }
  
  void writeTo(std::ostream& stream) const;
  
 protected:
  std::string fsmType_; // type of PVSS FSM device unit
  std::string dpType_; // type of PVSS data-point associated to device unit
  mutable std::map<std::string, std::pair<std::string, std::string> > stateMap_; // map XDAQ state --> PVSS data-point value, color
};

#endif
