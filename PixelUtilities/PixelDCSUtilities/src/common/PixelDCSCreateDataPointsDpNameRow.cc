#include "PixelDCSCreateDataPointsDpNameRow.h"

/***************************************************************************
 * Auxiliary class for storage of                                          *
 *  device name in hardware and logical view and of device type            *
 * for a single PVSS data-point created by PixelDCSCreateDataPoints class  *
 *                                                                         *
 * Author: Christian Veelken, UC Davis			 	           *
 *                                                                         *
 * Last update: $Date: 2007/07/18 12:28:51 $ (UTC)                         *
 *          by: $Author: veelken $                                         *
 ***************************************************************************/

#include <iostream>
#include <iomanip>

PixelDCSCreateDataPointsDpNameRow::PixelDCSCreateDataPointsDpNameRow(const std::string& hardwareName, const std::string& logicalName, const std::string& deviceType)
  : hardwareName_(hardwareName), logicalName_(logicalName), deviceType_(deviceType)
{}

void PixelDCSCreateDataPointsDpNameRow::writeTo(std::ostream& stream) const
{
  stream << "   " 
	 << std::setw(25) << hardwareName_
	 << std::setw(25) << logicalName_
	 << std::setw(25) << deviceType_ << std::endl;
}

