#include "PixelDCSCreateDataPointsDpNameTable.h"

/****************************************************************************
 * Auxiliary class for storage of                                           *
 *  device name in hardware and logical view and of device type            *
 * for a list of PVSS data-points created by PixelDCSCreateDataPoints class *
 *                                                                          *
 * Author: Christian Veelken, UC Davis			 	            *
 *                                                                          *
 * Last update: $Date: 2007/07/18 12:28:52 $ (UTC)                          *
 *          by: $Author: veelken $                                          *
 ****************************************************************************/

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>

PixelDCSCreateDataPointsDpNameTable::PixelDCSCreateDataPointsDpNameTable(xdata::Table& table) throw (xdaq::exception::Exception)
{
  std::vector<std::string> columns = table.getColumns();
  
  unsigned long numColumns = columns.size();
  if ( numColumns != 3 ) {
    XCEPT_RAISE (xdaq::exception::Exception, "Invalid number of Columns");
  }
  
  unsigned long numRows = table.getRowCount();
  for ( unsigned long rowIndex = 0; rowIndex < numRows; ++rowIndex ) {

    std::vector<std::string>::iterator columnIterator = columns.begin();
    for ( unsigned long columnIndex = 0; columnIndex < numColumns;  ++columnIndex, ++columnIterator ) {
      std::string entry = table.getValueAt(rowIndex, *columnIterator)->toString();
      std::string columnName = (*columnIterator);
      //std::string columnType = table.getColumnType(*columnIterator);
      
      if ( columnName == "HARDWARENAME" ) {
	createRow_hardwareName_ = entry;
      } else if ( columnName == "LOGICALNAME" ) {
	createRow_logicalName_ = entry;
      } else if ( columnName == "DEVICETYPE" ) {
	createRow_deviceType_ = entry;
      } 
    }

    addRow(createRow_hardwareName_, createRow_logicalName_, createRow_deviceType_);
  }
}

void PixelDCSCreateDataPointsDpNameTable::addRow(const std::string& hardwareName, const std::string& logicalName, const std::string& deviceType)
{
  table_.push_back(PixelDCSCreateDataPointsDpNameRow(hardwareName, logicalName, deviceType));
}

PixelDCSCreateDataPointsDpNameTable::~PixelDCSCreateDataPointsDpNameTable()
{
//--- nothing to be done yet
}

void PixelDCSCreateDataPointsDpNameTable::printHeader(std::ostream& stream) const
{
  stream << "PixelDCSCreateDataPointsDpNameTable:" << std::endl;
  stream.setf(std::ios::left);
  stream << " " 
	 << std::setw(25) << "hardwareName"
	 << std::setw(25) << "logicalName"
	 << std::setw(25) << "deviceType" << std::endl;
}
