#include "PixelDCSInterface/include/PixelDCStoFEDDpFilterTable.h"

/**************************************************************************
 * Auxiliary class for storage of lower and upper ADC limits within which *
 * the last DAC temperature values of a set of Read-out Chips             *
 * are suitable for calibration                  
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2007/12/03 11:24:24 $ (UTC)                        *
 *          by: $Author: veelken $                                        *
 **************************************************************************/

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>

PixelDCStoFEDDpFilterTable::PixelDCStoFEDDpFilterTable(xdata::Table& table) throw (xdaq::exception::Exception)
  : PixelDCStoFEDDpTable<PixelDCStoFEDDpFilterRow>()
{
  numDataMembers_ = 6;

  initializeTableData(table);
}

void PixelDCStoFEDDpFilterTable::initializeRowData(const std::string& columnName, const std::string& entry) throw (xdaq::exception::Exception)
{
  if ( columnName == "MINADC" ) {
    newRow_minAdc_ = atoi(entry.data());
  } else if ( columnName == "MAXADC" ) {
    newRow_maxAdc_ = atoi(entry.data());
  } else if ( columnName == "DEADBAND" ) {
    newRow_deadband_ = atof(entry.data());
  } else {
    XCEPT_RAISE (xdaq::exception::Exception, std::string("Undefined column name ") + columnName);
  }
}

void PixelDCStoFEDDpFilterTable::createRow()
{
  if ( table_[newRow_fedBoardId_][newRow_fedChannelId_][newRow_rocId_] == NULL ) {
    table_[newRow_fedBoardId_][newRow_fedChannelId_][newRow_rocId_] 
      = new PixelDCStoFEDDpFilterRow(newRow_fedBoardId_, newRow_fedChannelId_, newRow_rocId_, newRow_minAdc_, newRow_maxAdc_, newRow_deadband_);
  } else {
    XCEPT_RAISE (xdaq::exception::Exception, "Row defined twice");
  }
}

PixelDCStoFEDDpFilterTable::~PixelDCStoFEDDpFilterTable()
{
//--- nothing to be done yet
}

void PixelDCStoFEDDpFilterTable::printHeader(std::ostream& stream) const
{
  stream << "PixelDCStoFEDDpFilterTable:" << std::endl;
  stream.setf(std::ios::left);
  stream << " " 
	 << std::setw(13) << "fedBoardId"
	 << std::setw(13) << "fedChannelId"
	 << std::setw(13) << "rocId"
	 << std::setw(9) << "minAdc"
	 << std::setw(9) << "maxAdc"
	 << std::setw(9) << "deadband" << std::endl;
}
