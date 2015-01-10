
#include "PixelDCSPVSSDpe.h"

/*************************************************************************
 * Auxiliary class representing a PVSS data-point in XDAQ                *
 *                                                                       *
 * Author: Christian Veelken, UC Davis				         *
 *                                                                       *
 * Last update: $Date: 2007/12/03 10:07:24 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#include <sstream>
#include <iomanip>
 
PixelDCSPVSSDpe::PixelDCSPVSSDpe(const std::string& dpeName, const std::string& dpeValue)
  : dpeName_(dpeName), dpeValue_(dpeValue)
{
//--- nothing to be done yet...
}

PixelDCSPVSSDpe::~PixelDCSPVSSDpe()
{
//--- nothing to be done yet...
}

PixelDCSPVSSDpe::PixelDCSPVSSDpe(const std::string& dpeName, bool dpeValue)
  : dpeName_(dpeName)
{
/*
 if ( dpeValue == true ) {
    dpeValue_ = "TRUE";
  } else {
    dpeValue_ = "FALSE";
  }
 */
//--- boolean data-points not yet implemented 
//    in PSX server/PVSS API
  if ( dpeValue == true ) {
    dpeValue_ = "1";
  } else {
    dpeValue_ = "0";
  }
}

PixelDCSPVSSDpe::PixelDCSPVSSDpe(const std::string& dpeName, float dpeValue)
  : dpeName_(dpeName)
{
  std::ostringstream dpeValue_string;
  dpeValue_string.setf(std::ios::fixed);
  dpeValue_string << std::setprecision(1) << dpeValue;
  dpeValue_ = dpeValue_string.str();
}

PixelDCSPVSSDpe::PixelDCSPVSSDpe(const std::string& dpeName, unsigned int dpeValue)
  : dpeName_(dpeName)
{
  std::ostringstream dpeValue_string;
  dpeValue_string << dpeValue;
  dpeValue_ = dpeValue_string.str();
}

PixelDCSPVSSDpe::PixelDCSPVSSDpe(const std::string& dpeName, int dpeValue)
  : dpeName_(dpeName)
{
  std::ostringstream dpeValue_string;
  dpeValue_string << dpeValue;
  dpeValue_ = dpeValue_string.str();
}
