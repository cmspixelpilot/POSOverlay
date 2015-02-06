// $Id: PixelDCStoFEDDpNameTable.h,v 1.2 2007/12/03 11:24:21 veelken Exp $

/****************************************************************************
 * Auxiliary class for storage of translation between "logical" name of     *
 * Read-out Chips (according to "Naming Convention for CMS Pixel Detector", *
 *  https://docdb.fnal.gov/CMS-private/DocDB/ShowDocument?docid=901)        *
 * and name of PVSS data-points representing their last DAC temperature     *
 * measurement values                                                       *
 *                                                                          *
 * Author: Christian Veelken, UC Davis			 	            *
 *                                                                          *
 * Last update: $Date: 2007/12/03 11:24:21 $ (UTC)                          *
 *          by: $Author: veelken $                                          *
 ****************************************************************************/

#ifndef _PixelDCStoFEDDpNameTable_h_
#define _PixelDCStoFEDDpNameTable_h_

#include <map>

#include "xdaq/exception/Exception.h"
#include "xdata/Table.h"

#include "PixelDCSInterface/include/PixelDCStoFEDDpTable.h"
#include "PixelDCSInterface/include/PixelDCStoFEDDpNameRow.h"

class PixelDCStoFEDDpNameTable : public PixelDCStoFEDDpTable<PixelDCStoFEDDpNameRow>
{
 public:
  
  PixelDCStoFEDDpNameTable(xdata::Table& table) throw (xdaq::exception::Exception);
  virtual ~PixelDCStoFEDDpNameTable();
  
 protected:
  
  virtual void initializeRowData(const std::string& columnName, const std::string& entry) throw (xdaq::exception::Exception);
  virtual void createRow();
  
  virtual void printHeader(std::ostream& stream) const;
  
  std::string newRow_logicalName_;
  std::string newRow_dpName_;
};

#endif
