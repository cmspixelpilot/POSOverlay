// $Id: PixelDCStoFEDDpCalibrationTable.h,v 1.2 2007/12/03 11:24:21 veelken Exp $

/**************************************************************************
 * Auxiliary class for storage of calibration coefficients                *
 * for last DAC ADC --> temperature conversion                            *
 * (in units of degrees Celsius)                                          *
 * for a set of Read-out Chips                                            *
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2007/12/03 11:24:21 $ (UTC)                        *
 *          by: $Author: veelken $                                        *
 **************************************************************************/

#ifndef _PixelDCStoFEDDpCalibrationTable_h_
#define _PixelDCStoFEDDpCalibrationTable_h_

#include <map>

#include "xdaq/exception/Exception.h"
#include "xdata/Table.h"

#include "PixelDCSInterface/include/PixelDCStoFEDDpTable.h"
#include "PixelDCSInterface/include/PixelDCStoFEDDpCalibrationRow.h"

class PixelDCStoFEDDpCalibrationTable : public PixelDCStoFEDDpTable<PixelDCStoFEDDpCalibrationRow>
{
 public:

  PixelDCStoFEDDpCalibrationTable(xdata::Table& table) throw (xdaq::exception::Exception);
  virtual ~PixelDCStoFEDDpCalibrationTable();

 protected:
  
  virtual void initializeRowData(const std::string& columnName, const std::string& entry) throw (xdaq::exception::Exception);
  virtual void createRow();
  
  virtual void printHeader(std::ostream& stream) const;
  
  float newRow_a0_;
  float newRow_a1_;
  float newRow_b0_;
  float newRow_b1_;
  float newRow_A_[numTempRangeSettings];
  float newRow_B_[numTempRangeSettings];
};

#endif
