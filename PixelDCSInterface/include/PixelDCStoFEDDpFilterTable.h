// $Id: PixelDCStoFEDDpFilterTable.h,v 1.2 2007/12/03 11:24:21 veelken Exp $

/**************************************************************************
 * Auxiliary class for storage of lower and upper ADC limits within which *
 * the last DAC temperature values of a set of Read-out Chips             *
 * are suitable for calibration                  
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2007/12/03 11:24:21 $ (UTC)                        *
 *          by: $Author: veelken $                                        *
 **************************************************************************/

#ifndef _PixelDCStoFEDDpFilterTable_h_
#define _PixelDCStoFEDDpFilterTable_h_

#include <map>

#include "xdaq/exception/Exception.h"
#include "xdata/Table.h"

#include "PixelDCSInterface/include/PixelDCStoFEDDpTable.h"
#include "PixelDCSInterface/include/PixelDCStoFEDDpFilterRow.h"

class PixelDCStoFEDDpFilterTable : public PixelDCStoFEDDpTable<PixelDCStoFEDDpFilterRow>
{
	public:

        PixelDCStoFEDDpFilterTable(xdata::Table& table) throw (xdaq::exception::Exception);
	virtual ~PixelDCStoFEDDpFilterTable();

	protected:

	virtual void initializeRowData(const std::string& columnName, const std::string& entry) throw (xdaq::exception::Exception);
	virtual void createRow();
	
	virtual void printHeader(std::ostream& stream) const;

	int newRow_minAdc_;
	int newRow_maxAdc_;
	float newRow_deadband_;
};

#endif
