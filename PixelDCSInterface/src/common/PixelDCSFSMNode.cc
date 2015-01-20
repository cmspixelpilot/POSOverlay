#include "PixelDCSInterface/include/PixelDCSFSMNode.h"

/**************************************************************************
 * Base class for storage of name and current state of                    *
 * a single PVSS FSM component                                            *
 * (representing one CAEN A4602 or A4603 power supply board)              *
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2009/07/09 07:31:03 $ (UTC)                        *
 *          by: $Author: joshmt $                                        *
 **************************************************************************/

#include <iostream>
#include <iomanip>

void PixelDCSFSMNode::writeTo(std::ostream& stream) const
{
  stream << " partition = " << name_ << std::endl;
  stream << "  domain = " << domain_ << std::endl;
  stream << "  state = " << state_ << std::endl;
  stream << "  used = " << used_ << std::endl;
  deviceDefinition_.writeTo(stream);
}
