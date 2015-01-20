#include "PixelDCSInterface/include/PixelDCStoFEDDpCalibrationTable.h"

/**************************************************************************
 * Auxiliary class for storage of calibration coefficients                *
 * for last DAC ADC --> temperature conversion                            *
 * (in units of degrees Celsius)                                          *
 * for a set of Read-out Chips                                            *
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

PixelDCStoFEDDpCalibrationTable::PixelDCStoFEDDpCalibrationTable(xdata::Table& table) throw (xdaq::exception::Exception)
  : PixelDCStoFEDDpTable<PixelDCStoFEDDpCalibrationRow>()
{
  numDataMembers_ = 9;
  
  initializeTableData(table);
}

void PixelDCStoFEDDpCalibrationTable::initializeRowData(const std::string& columnName, const std::string& entry) throw (xdaq::exception::Exception)
{
  bool newRow_initialized = false;
  if ( columnName == "A0" ) {
    newRow_a0_ = atof(entry.data());
    newRow_initialized = true;
  } else if ( columnName == "A1" ) {
    newRow_a1_ = atof(entry.data());
    newRow_initialized = true;
  } else if ( columnName == "B0" ) {
    newRow_b0_ = atof(entry.data());
    newRow_initialized = true;
  } else if ( columnName == "B1" ) {
    newRow_b1_ = atof(entry.data());
    newRow_initialized = true;
  } 
  
  for ( unsigned iTempRange = 0; iTempRange < numTempRangeSettings; ++iTempRange ) {
    char newRowName_A[4];
    sprintf(newRowName_A, "A_TEMPRANGE%u", iTempRange);
    if ( columnName == newRowName_A ) {
      newRow_A_[iTempRange] = atof(entry.data());
      newRow_initialized = true;
    }

    char newRowName_B[4];
    sprintf(newRowName_B, "B_TEMPRANGE%u", iTempRange);
    if ( columnName == newRowName_B ) {
      newRow_B_[iTempRange] = atof(entry.data());
      newRow_initialized = true;
    }
  }

  if ( !newRow_initialized ) {
    std::cerr << "Error in <PixelDCStoFEDDpCalibrationTable::initializeRowData>: column Name = " << columnName << " undefined !!!" << std::endl;
    XCEPT_RAISE (xdaq::exception::Exception, std::string("Undefined column name ") + columnName);
  }
}

void PixelDCStoFEDDpCalibrationTable::createRow()
{
  if ( table_[newRow_fedBoardId_][newRow_fedChannelId_][newRow_rocId_] == NULL ) {
    table_[newRow_fedBoardId_][newRow_fedChannelId_][newRow_rocId_] 
      = new PixelDCStoFEDDpCalibrationRow(newRow_fedBoardId_, newRow_fedChannelId_, newRow_rocId_, 
					  newRow_a0_, newRow_a1_, newRow_b0_, newRow_b1_, newRow_A_, newRow_B_);
  } else {
    XCEPT_RAISE (xdaq::exception::Exception, "Row defined twice");
  }
}

PixelDCStoFEDDpCalibrationTable::~PixelDCStoFEDDpCalibrationTable()
{
//--- nothing to be done yet
}

void PixelDCStoFEDDpCalibrationTable::printHeader(std::ostream& stream) const
{
  stream << "PixelDCStoFEDDpCalibrationTable:" << std::endl;
  stream.setf(std::ios::left);
  stream << " " 
	 << std::setw(13) << "fedBoardId"
	 << std::setw(13) << "fedChannelId"
	 << std::setw(13) << "rocId"
	 << std::setw(9) << "a0"
	 << std::setw(9) << "a1"
	 << std::setw(9) << "b0"
	 << std::setw(9) << "b1";
  
  for ( unsigned iTempRange = 0; iTempRange < numTempRangeSettings; ++iTempRange ) {
    stream << std::setw(9) << "A" << "[" << iTempRange << "]";
  }

  for ( unsigned iTempRange = 0; iTempRange < numTempRangeSettings; ++iTempRange ) {
    stream << std::setw(9) << "B" << "[" << iTempRange << "]";
  }

  stream << std::endl;
}
