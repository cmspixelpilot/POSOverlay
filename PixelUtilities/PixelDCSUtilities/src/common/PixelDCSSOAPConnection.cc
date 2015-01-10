#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSSOAPConnection.h"

/**************************************************************************
 * Auxiliary class for storage of information associated                  *
 * with a single SOAP connection;                                         *
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

void PixelDCSSOAPConnection::writeTo(std::ostream& stream) const
{
  std::cout << "   " 
	    << std::setw(13) << name_
	    << std::setw(13) << type_
	    << std::setw(13) << instance_ << std::endl;
}

