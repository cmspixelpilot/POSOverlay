#include "PixelDCSInterface/include/PixelDCStoFEDDpValueTable.h"

/***************************************************************************
 * Auxiliary class for storage of last DAC temperature information         *
 * for a set of Read-out Chips                                             *
 * (sum of "raw" ADC values, number of ADC readings to compute an average, *
 *  calibrated temperature in degrees Celsius                              *
 *  and time of last temperature update send to PVSS)                      *
 *                                                                         *
 * Author: Christian Veelken, UC Davis			 	           *
 *                                                                         *
 * Last update: $Date: 2007/12/03 11:24:25 $ (UTC)                         *
 *          by: $Author: veelken $                                         *
 ***************************************************************************/

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>

PixelDCStoFEDDpValueTable::PixelDCStoFEDDpValueTable(xdata::Table& table) throw (xdaq::exception::Exception)
  : PixelDCStoFEDDpTable<PixelDCStoFEDDpValueRow>()
{
//--- set numDataMembers to same value as in PixelDCStoFEDDpNameTable
//    to allow PixelDCStoFEDDpValueTable to be initialized 
//    by the same Oracle table as PixelDCStoFEDDpNameTable is
  numDataMembers_ = 5;

  initializeTableData(table);
}

void PixelDCStoFEDDpValueTable::initializeRowData(const std::string& columnName, const std::string& entry) throw (xdaq::exception::Exception)
{
  if ( columnName != "LOGICALNAME" &&
       columnName != "DPNAME"     ) {
//--- ignore "LOGICALNAME" and "DPNAME" columns
//    to allow PixelDCStoFEDDpValueTable to be initialized 
//    by the same Oracle table as PixelDCStoFEDDpNameTable is
    XCEPT_RAISE (xdaq::exception::Exception, std::string("Undefined column name ") + columnName);
  }
}

void PixelDCStoFEDDpValueTable::createRow()
{
  if ( table_[newRow_fedBoardId_][newRow_fedChannelId_][newRow_rocId_] == NULL ) {
    table_[newRow_fedBoardId_][newRow_fedChannelId_][newRow_rocId_] 
      = new PixelDCStoFEDDpValueRow(newRow_fedBoardId_, newRow_fedChannelId_, newRow_rocId_);
  } else {
    XCEPT_RAISE (xdaq::exception::Exception, "Row defined twice");
  }
}

PixelDCStoFEDDpValueTable::~PixelDCStoFEDDpValueTable()
{
//--- nothing to be done yet
}

void PixelDCStoFEDDpValueTable::printHeader(std::ostream& stream) const
{
  stream << "PixelDCStoFEDDpValueTable:" << std::endl;
  stream.setf(std::ios::left);
  stream << " " 
	 << std::setw(13) << "fedBoardId"
	 << std::setw(13) << "fedChannelId"
	 << std::setw(13) << "rocId"
         << std::setw(13) << "lastDpValue"
	 << std::setw(13) << "lastDpUpdate" << std::endl;
}
