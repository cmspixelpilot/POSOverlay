#include "PixelDCSInterface/include/PixelDCStoFEDDpFilterRow.h"

/**************************************************************************
 * Auxiliary class for storage of lower and upper ADC limits within which *
 * the last DAC temperature values of a single Read-out Chip              *
 * are suitable for calibration                                           *
 * (cf. https://docdb.fnal.gov/CMS-private/DocDB/ShowDocument?docid=1007) *
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2007/03/17 16:46:21 $ (UTC)                        *
 *          by: $Author: veelken $                                        *
 **************************************************************************/

#include "math.h"

#include <iostream>
#include <iomanip>

bool PixelDCStoFEDDpFilterRow::isWithinDeadband(float value, float lastValue) const
{
//--- check if calibrated value changed by less then dead-band 
//    with respect to data-point value currently stored in PVSS;
//    (PVSS data-point needs to be updated in case change exceeds dead-band)

  return (fabs(value - lastValue) < deadband_);
}

void PixelDCStoFEDDpFilterRow::writeTo(std::ostream& stream) const
{
  std::cout << "   " 
	    << std::setw(13) << fedBoardId_
	    << std::setw(13) << fedChannelId_
	    << std::setw(13) << rocId_
	    << std::setw(9) << minAdc_
	    << std::setw(9) << maxAdc_
	    << std::setw(9) << deadband_ << std::endl;
}

