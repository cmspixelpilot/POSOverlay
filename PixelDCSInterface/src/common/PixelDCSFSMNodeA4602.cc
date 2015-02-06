#include "PixelDCSInterface/include/PixelDCSFSMNodeA4602.h"

/**************************************************************************
 * Auxiliary class for storage of name and current state of               *
 * a single PVSS FSM node                                                 *
 * representing the low voltage channels                                  *
 * of a single CAEN A4602 power supply board                              *
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2007/10/08 16:21:11 $ (UTC)                        *
 *          by: $Author: veelken $                                        *
 **************************************************************************/

PixelDCSFSMNodeA4602::PixelDCSFSMNodeA4602(const std::string& name, const PixelDCSFSMNodeDefinition& deviceDefinition)
  : PixelDCSFSMNode(name, deviceDefinition) 
{
//--- nothing to be done yet...
}
