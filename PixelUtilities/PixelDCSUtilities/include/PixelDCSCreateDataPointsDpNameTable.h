// $Id: PixelDCSCreateDataPointsDpNameTable.h,v 1.1 2007/07/18 12:28:51 veelken Exp $

/****************************************************************************
 * Auxiliary class for storage of                                           *
 *  device name, logical name and device type                               *
 * for a list of PVSS data-points created by PixelDCSCreateDataPoints class *
 *                                                                          *
 * Author: Christian Veelken, UC Davis			 	            *
 *                                                                          *
 * Last update: $Date: 2007/07/18 12:28:51 $ (UTC)                          *
 *          by: $Author: veelken $                                          *
 ****************************************************************************/

#ifndef _PixelDCSCreateDataPointsDpNameTable_h_
#define _PixelDCSCreateDataPointsDpNameTable_h_

#include <list>

#include "xdaq/exception/Exception.h"
#include "xdata/Table.h"

#include "PixelDCSCreateDataPointsDpNameRow.h"

class PixelDCSCreateDataPointsDpNameTable
{
 public:

  PixelDCSCreateDataPointsDpNameTable() {}
  PixelDCSCreateDataPointsDpNameTable(xdata::Table& table) throw (xdaq::exception::Exception);
  ~PixelDCSCreateDataPointsDpNameTable();
  
  void addRow(const std::string& hardwareName, const std::string& logicalName, const std::string& deviceType);

  const std::list<PixelDCSCreateDataPointsDpNameRow>& getRows() { return table_; }
  
 protected:
  
  void initializeRowData(const std::string& columnName, const std::string& entry) throw (xdaq::exception::Exception);
  void createRow();
  
  void printHeader(std::ostream& stream) const;

  std::list<PixelDCSCreateDataPointsDpNameRow> table_;

  std::string createRow_hardwareName_;
  std::string createRow_logicalName_;
  std::string createRow_deviceType_;
};

#endif
