// $Id: PixelDCSFSMNodeA4602.h,v 1.1 2007/08/10 14:56:23 veelken Exp $

/**************************************************************************
 * Auxiliary class for storage of name and current state of               *
 * a single PVSS FSM node                                                 *
 * representing the low voltage channels                                  *
 * of a single CAEN A4602 power supply board                              *
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2007/08/10 14:56:23 $ (UTC)                        *
 *          by: $Author: veelken $                                        *
 **************************************************************************/

#ifndef _PixelDCSFSMNodeA4602_h_
#define _PixelDCSFSMNodeA4602_h_

#include <string>

#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMNodeDefinition.h"

#include "PixelDCSInterface/include/PixelDCSFSMNode.h"

class PixelDCSFSMNodeA4602 : public PixelDCSFSMNode
{
 public:
  PixelDCSFSMNodeA4602(const std::string& name, const PixelDCSFSMNodeDefinition& deviceDefinition);
  ~PixelDCSFSMNodeA4602() {}
};

#endif
