#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMStateTranslation.h"

/**************************************************************************
 * Auxiliary class for translation between XDAQ and PVSS FSM states;      * 
 * the translation table is composed of pairs                             *
 *   { XDAQ state, PVSS state},                                           *
 * used by                                                                *
 *   PixelDCSSupervisor                                                   *
 *   PixelDCSFSMInterface                                                 *
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2007/08/10 14:47:04 $ (UTC)                        *
 *          by: $Author: veelken $                                        *
 **************************************************************************/

#include <iostream>
#include <iomanip>

void PixelDCSFSMStateTranslation::writeTo(std::ostream& stream) const
{
  std::cout << "   " 
	    << std::setw(13) << pvssState_
	    << std::setw(13) << xdaqState_ << std::endl;
}

