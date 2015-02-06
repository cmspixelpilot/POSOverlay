// $Id: PixelDCStoFEDDpNameRow.h,v 1.1 2007/03/17 16:46:20 veelken Exp $

/***************************************************************************
 * Auxiliary class for storage of translation between "logical" name of    *
 * Read-out Chip (according to "Naming Convention for CMS Pixel Detector", *
 *  https://docdb.fnal.gov/CMS-private/DocDB/ShowDocument?docid=901)       *
 * and name of PVSS data-point representing its last DAC temperature       *
 * measurement value                                                       *
 *                                                                         *
 * Author: Christian Veelken, UC Davis			 	           *
 *                                                                         *
 * Last update: $Date: 2007/03/17 16:46:20 $ (UTC)                         *
 *          by: $Author: veelken $                                         *
 ***************************************************************************/

#ifndef _PixelDCStoFEDDpNameRow_h_
#define _PixelDCStoFEDDpNameRow_h_

#include <string>

#include "PixelDCSInterface/include/PixelDCStoFEDDpRow.h"

class PixelDCStoFEDDpNameRow : public PixelDCStoFEDDpRow
{
	public:

	PixelDCStoFEDDpNameRow(unsigned int fedBoardId, unsigned int fedChannelId, unsigned int rocId,
			       const std::string& logicalName, const std::string& dpName)
	  : PixelDCStoFEDDpRow(fedBoardId, fedChannelId, rocId), logicalName_(logicalName), dpName_(dpName) {}
        ~PixelDCStoFEDDpNameRow() {}

        const std::string& getLogicalName() const { return logicalName_; }
        const std::string& getDpName() const { return dpName_; }
	
	void writeTo(std::ostream& stream) const;

	protected:

	std::string logicalName_;
	std::string dpName_;
};
#endif
