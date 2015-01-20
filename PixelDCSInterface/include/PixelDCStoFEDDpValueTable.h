// $Id: PixelDCStoFEDDpValueTable.h,v 1.1 2007/03/17 16:46:20 veelken Exp $

/***************************************************************************
 * Auxiliary class for storage of last DAC temperature information         *
 * for a set of Read-out Chips                                             *
 * (sum of "raw" ADC values, number of ADC readings to compute an average, *
 *  calibrated temperature in degrees Celsius                              *
 *  and time of last temperature update send to PVSS)                      *
 *                                                                         *
 * Author: Christian Veelken, UC Davis			 	           *
 *                                                                         *
 * Last update: $Date: 2007/03/17 16:46:20 $ (UTC)                         *
 *          by: $Author: veelken $                                         *
 ***************************************************************************/

#ifndef _PixelDCStoFEDDpValueTable_h_
#define _PixelDCStoFEDDpValueTable_h_

#include <map>

#include "xdaq/exception/Exception.h"
#include "xdata/Table.h"

#include "PixelDCSInterface/include/PixelDCStoFEDDpTable.h"
#include "PixelDCSInterface/include/PixelDCStoFEDDpValueRow.h"

class PixelDCStoFEDDpValueTable : public PixelDCStoFEDDpTable<PixelDCStoFEDDpValueRow>
{
	public:

        PixelDCStoFEDDpValueTable(xdata::Table& table) throw (xdaq::exception::Exception);
	virtual ~PixelDCStoFEDDpValueTable();

	protected:

	virtual void initializeRowData(const std::string& columnName, const std::string& entry) throw (xdaq::exception::Exception);
	virtual void createRow();
	
	virtual void printHeader(std::ostream& stream) const;
};

#endif
