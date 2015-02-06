// $Id: PixelDCSFSMNodeA4603.h,v 1.5 2009/06/24 15:23:39 joshmt Exp $

/**************************************************************************
 * Auxiliary class for storage of name and current state of               *
 * a single PVSS FSM node                                                 *
 * representing the low voltage channels                                  *
 * of a single CAEN A4603 power supply board                              *
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2009/06/24 15:23:39 $ (UTC)                        *
 *          by: $Author: joshmt $                                        *
 **************************************************************************/

#ifndef _PixelDCSFSMNodeA4603_h_
#define _PixelDCSFSMNodeA4603_h_

#include <string>

#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMNodeDefinition.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMDeviceDefinition.h"

#include "PixelDCSInterface/include/PixelDCSFSMNode.h"

class PixelDCSFSMNodeA4603 : public PixelDCSFSMNode
{
 public:
  PixelDCSFSMNodeA4603(const std::string& name, const PixelDCSFSMNodeDefinition& deviceDefinition,
		       const PixelDCSFSMDeviceDefinition* deviceDefinition_ReadoutChipInitializationStatus,
		       const std::string& dpName_ReadoutChipInitializationStatus);
  PixelDCSFSMNodeA4603(const PixelDCSFSMNodeA4603& bluePrint);
  ~PixelDCSFSMNodeA4603();
    
  void setState_ReadoutChipInitializationStatus(const std::string& state) { state_ReadoutChipInitializationStatus_ = state; }
  
  const std::string& getDpName_ReadoutChipInitializationStatus() const { return dpName_ReadoutChipInitializationStatus_; }
  const std::string& getState_ReadoutChipInitializationStatus() const { return state_ReadoutChipInitializationStatus_; }
  
  const PixelDCSFSMDeviceDefinition* getDeviceDefinition_ReadoutChipInitializationStatus() const 
    { return deviceDefinition_ReadoutChipInitializationStatus_; }
  
  void writeTo(std::ostream& stream) const;
  
 protected:
  std::string dpName_ReadoutChipInitializationStatus_; // name of PVSS data-point storing initialization status information for the read-out chips
                                                       // powered by the low voltage channels of the CAEN A4603 power supply board
  std::string state_ReadoutChipInitializationStatus_;  // state in FSM associated to read-out chip initialization status
  
  PixelDCSFSMDeviceDefinition* deviceDefinition_ReadoutChipInitializationStatus_;
};

#endif
