#include "PixelDCSInterface/include/PixelDCStoFEDDpNameRow.h"

/***************************************************************************
 * Auxiliary class for storage of translation between "logical" name of    *
 * Read-out Chip (according to "Naming Convention for CMS Pixel Detector", *
 *  https://docdb.fnal.gov/CMS-private/DocDB/ShowDocument?docid=901)       *
 * and name of PVSS data-point representing its last DAC temperature       *
 * measurement value                                                       *
 *                                                                         *
 * Author: Christian Veelken, UC Davis			 	           *
 *                                                                         *
 * Last update: $Date: 2007/03/17 16:46:21 $ (UTC)                         *
 *          by: $Author: veelken $                                         *
 ***************************************************************************/

#include <iostream>
#include <iomanip>

void PixelDCStoFEDDpNameRow::writeTo(std::ostream& stream) const
{
  std::cout << "   " 
	    << std::setw(13) << fedBoardId_
	    << std::setw(13) << fedChannelId_
	    << std::setw(13) << rocId_
	    << std::setw(25) << logicalName_
	    << std::setw(25) << dpName_ << std::endl;
}

