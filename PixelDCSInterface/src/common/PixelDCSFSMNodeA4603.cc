#include "PixelDCSInterface/include/PixelDCSFSMNodeA4603.h"

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

#include <iostream>
#include <iomanip>

PixelDCSFSMNodeA4603::PixelDCSFSMNodeA4603(const std::string& name, const PixelDCSFSMNodeDefinition& deviceDefinition,
					   const PixelDCSFSMDeviceDefinition* deviceDefinition_ReadoutChipInitializationStatus,
					   const std::string& dpName_ReadoutChipInitializationStatus)
  :  PixelDCSFSMNode(name, deviceDefinition)
     
{
  if ( deviceDefinition_ReadoutChipInitializationStatus != NULL ) {
    deviceDefinition_ReadoutChipInitializationStatus_ = new PixelDCSFSMDeviceDefinition(*deviceDefinition_ReadoutChipInitializationStatus);
  } else {
    deviceDefinition_ReadoutChipInitializationStatus_ = NULL;
  }

  dpName_ReadoutChipInitializationStatus_ = dpName_ReadoutChipInitializationStatus;

  state_ReadoutChipInitializationStatus_ = "UNDEFINED";
}

PixelDCSFSMNodeA4603::PixelDCSFSMNodeA4603(const PixelDCSFSMNodeA4603& bluePrint)
  : PixelDCSFSMNode(bluePrint)
{
  if ( bluePrint.deviceDefinition_ReadoutChipInitializationStatus_ != NULL ) {
    deviceDefinition_ReadoutChipInitializationStatus_ = new PixelDCSFSMDeviceDefinition(*bluePrint.deviceDefinition_ReadoutChipInitializationStatus_);
  } else {
    deviceDefinition_ReadoutChipInitializationStatus_ = NULL;
  }

  dpName_ReadoutChipInitializationStatus_ = bluePrint.dpName_ReadoutChipInitializationStatus_;

  state_ReadoutChipInitializationStatus_ = bluePrint.state_ReadoutChipInitializationStatus_;
}

PixelDCSFSMNodeA4603::~PixelDCSFSMNodeA4603()
{
  delete deviceDefinition_ReadoutChipInitializationStatus_;
}

void PixelDCSFSMNodeA4603::writeTo(std::ostream& stream) const
{
  stream << " partition = " << name_ << std::endl;
  stream << "  state = " << state_ << std::endl;
  deviceDefinition_.writeTo(stream);
  stream << " ReadOut-Chip Initialization Status:" << std::endl;
  stream << "  dpName = " << dpName_ReadoutChipInitializationStatus_ << std::endl;
  stream << "  state = " << state_ReadoutChipInitializationStatus_ << std::endl;
  if ( deviceDefinition_ReadoutChipInitializationStatus_ != NULL ) {
    deviceDefinition_ReadoutChipInitializationStatus_->writeTo(stream);
  }
}
