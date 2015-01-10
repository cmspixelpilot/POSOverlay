// $Id: PixelDCSCreateDataPointsDpNameRow.h,v 1.1 2007/07/18 12:28:50 veelken Exp $

/***************************************************************************
 * Auxiliary class for storage of                                          *
 *  device name in hardware and logical view and of device type            *
 * for a single PVSS data-point created by PixelDCSCreateDataPoints class  *
 *                                                                         *
 * Author: Christian Veelken, UC Davis			 	           *
 *                                                                         *
 * Last update: $Date: 2007/07/18 12:28:50 $ (UTC)                         *
 *          by: $Author: veelken $                                         *
 ***************************************************************************/

#ifndef _PixelDCSCreateDataPointsDpNameRow_h_
#define _PixelDCSCreateDataPointsDpNameRow_h_

#include <string>

class PixelDCSCreateDataPointsDpNameRow 
{
 public:

  PixelDCSCreateDataPointsDpNameRow(const std::string& hardwareName, const std::string& logicalName, const std::string& deviceType);
  ~PixelDCSCreateDataPointsDpNameRow() {}
  
  const std::string& getHardwareName() const { return hardwareName_; }
  const std::string& getLogicalName() const { return logicalName_; }
  const std::string& getDeviceType() const { return deviceType_; }
  
  void writeTo(std::ostream& stream) const;
  
 protected:
  
  std::string hardwareName_; // device name in "hardware" view
  std::string logicalName_;  // device name in "logical" view
  std::string deviceType_;   // device type (PVSS data-point type)
};

#endif
