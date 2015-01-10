#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMStateToDpValueTranslation.h"

/**************************************************************************
 * Auxiliary class for translation between XDAQ states                    *
 * and corresponding values of associated PVSS data-points;               *
 * the translation table is composed of pairs                             *
 *   { XDAQ state, PVSS data-point value },                               *
 * used by                                                                *
 *   PixelDCSFSMInterface                                                 *
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2007/08/10 14:47:04 $ (UTC)                        *
 *          by: $Author: veelken $                                        *
 **************************************************************************/

#include <iostream>
#include <iomanip>

PixelDCSFSMStateToDpValueTranslation::PixelDCSFSMStateToDpValueTranslation(const std::string& xdaqState, const std::string& pvssDpValue)
  : xdaqState_(xdaqState), pvssDpValue_(pvssDpValue) 
{
//--- nothing to be done yet...
}

PixelDCSFSMStateToDpValueTranslation::~PixelDCSFSMStateToDpValueTranslation()
{
//--- nothing to be done yet...
}

void PixelDCSFSMStateToDpValueTranslation::writeTo(std::ostream& stream) const
{
  std::cout << "   " 
	    << std::setw(13) << xdaqState_
	    << std::setw(13) << pvssDpValue_ << std::endl;
}

