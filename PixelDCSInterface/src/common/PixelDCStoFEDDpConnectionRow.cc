#include "PixelDCSInterface/include/PixelDCStoFEDDpConnectionRow.h"

/**************************************************************************
 * Auxiliary class for storage of "connection" statistics                 *
 * related to last DAC temperature values for a single Read-out Chip      *
 * (number of ADC values within/outside of band suitable for calibration; *
 *  in case the ADC value is outside suitable region,                     *
 *  the PixelFECSupervisor needs to be instructed to change               *
 *  the TempRange DAC of the concerned Read-out Chip,                     *
 *  cf. https://docdb.fnal.gov/CMS-private/DocDB/ShowDocument?docid=1007) *
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2007/03/17 16:46:21 $ (UTC)                        *
 *          by: $Author: veelken $                                        *
 **************************************************************************/

#include <math.h>

void PixelDCStoFEDDpConnectionRow::setDpUpdatePriority(float dpValue, float lastDpValue, bool isWithinDeadband,
						    int numDp, 
						    time_t currentTime, time_t lastUpdate) const
{
//--- determine order in which data-point updates 
//    are sent to PVSS (via PSX interface)

  if ( isWithinDeadband || numDp < 10 ) {
//--- do not update data-points whose value changed by less than deadband
//    or which have not been read-out by FED Supervisor at least ten times since last update

    dpUpdatePriority_ = 0.;
  } else if ( (currentTime - lastUpdate) > 30. ) {
//--- update data-points that have not been updated for the longest time first

    dpUpdatePriority_ = 1.e6*(currentTime - lastUpdate) + fabs(lastDpValue - dpValue);
  } else {
//--- then update data-points whose value changed the most

    dpUpdatePriority_ = fabs(lastDpValue - dpValue);
  }
}

//
//---------------------------------------------------------------------------------------------------
//

bool operator < (const PixelDCStoFEDDpConnectionRow& arg1, const PixelDCStoFEDDpConnectionRow& arg2)
{
//--- NOTE: the operator needs to return true if arg1 is to be placed **before** arg2 in the sorted list;
//          as the data-points that have the **highest** priority are to the placed first in the list
//          operator < needs to return true in case **priority(arg1) > priority(arg2)**

  return (arg1.getDpUpdatePriority() > arg2.getDpUpdatePriority());
}
