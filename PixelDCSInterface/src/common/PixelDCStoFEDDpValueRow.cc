#include "PixelDCSInterface/include/PixelDCStoFEDDpValueRow.h"

/***************************************************************************
 * Auxiliary class for storage of last DAC temperature information         *
 * for a single Read-out Chip                                              *
 * (sum of "raw" ADC values, number of ADC readings to compute an average, *
 *  calibrated temperature in degrees Celsius                              *
 *  and time of last temperature update send to PVSS)                      *
 *                                                                         *
 * Author: Christian Veelken, UC Davis			 	           *
 *                                                                         *
 * Last update: $Date: 2007/12/03 11:24:25 $ (UTC)                         *
 *          by: $Author: veelken $                                         *
 ***************************************************************************/

#include <time.h>

#include <iostream>
#include <iomanip>

void PixelDCStoFEDDpValueRow::reset() 
{ 
  lastDpValue_ = getDpValue();

  lastDpUpdate_ = time(NULL);

  dpValueSum_ = 0.;
  numDp_ = 0;

  adcSum_ = 0.;
  numADC_ = 0;

  currentTempRangeDAC_ = 0;
}

void PixelDCStoFEDDpValueRow::writeTo(std::ostream& stream) const
{
  stream << "   " 
	 << std::setw(13) << fedBoardId_
	 << std::setw(13) << fedChannelId_
	 << std::setw(13) << rocId_
	 << std::setw(13) << lastDpValue_
	 << std::setw(13) << asctime(localtime(&lastDpUpdate_)) << std::endl;
}

