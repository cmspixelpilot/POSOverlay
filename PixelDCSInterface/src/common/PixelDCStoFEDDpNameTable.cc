#include "PixelDCSInterface/include/PixelDCStoFEDDpNameTable.h"

/****************************************************************************
 * Auxiliary class for storage of translation between "logical" name of     *
 * Read-out Chips (according to "Naming Convention for CMS Pixel Detector", *
 *  https://docdb.fnal.gov/CMS-private/DocDB/ShowDocument?docid=901)        *
 * and name of PVSS data-points representing their last DAC temperature     *
 * measurement values                                                       *
 *                                                                          *
 * Author: Christian Veelken, UC Davis			 	            *
 *                                                                          *
 * Last update: $Date: 2007/12/03 11:24:25 $ (UTC)                          *
 *          by: $Author: veelken $                                          *
 ****************************************************************************/

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>

PixelDCStoFEDDpNameTable::PixelDCStoFEDDpNameTable(xdata::Table& table) throw (xdaq::exception::Exception)
  : PixelDCStoFEDDpTable<PixelDCStoFEDDpNameRow>()
{
  numDataMembers_ = 5;

  initializeTableData(table);
}

void PixelDCStoFEDDpNameTable::initializeRowData(const std::string& columnName, const std::string& entry) throw (xdaq::exception::Exception)
{
  if ( columnName == "LOGICALNAME" ) {
    newRow_logicalName_ = entry;
  } else if ( columnName == "DPNAME" ) {
    newRow_dpName_ = entry;
  } else {
    XCEPT_RAISE (xdaq::exception::Exception, std::string("Undefined column name ") + columnName);
  }
}

void PixelDCStoFEDDpNameTable::createRow()
{
  if ( table_[newRow_fedBoardId_][newRow_fedChannelId_][newRow_rocId_] == NULL ) {
    table_[newRow_fedBoardId_][newRow_fedChannelId_][newRow_rocId_] 
      = new PixelDCStoFEDDpNameRow(newRow_fedBoardId_, newRow_fedChannelId_, newRow_rocId_, newRow_logicalName_, newRow_dpName_);
  } else {
    XCEPT_RAISE (xdaq::exception::Exception, "Row defined twice");
  }
}

PixelDCStoFEDDpNameTable::~PixelDCStoFEDDpNameTable()
{
//--- nothing to be done yet
}

void PixelDCStoFEDDpNameTable::printHeader(std::ostream& stream) const
{
  stream << "PixelDCStoFEDDpNameTable:" << std::endl;
  stream.setf(std::ios::left);
  stream << " " 
	 << std::setw(13) << "fedBoardId"
	 << std::setw(13) << "fedChannelId"
	 << std::setw(13) << "rocId"
	 << std::setw(25) << "logicalName"
	 << std::setw(25) << "dpName" << std::endl;
}
