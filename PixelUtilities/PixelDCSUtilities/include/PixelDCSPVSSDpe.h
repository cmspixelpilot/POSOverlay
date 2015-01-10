// $Id: PixelDCSPVSSDpe.h,v 1.3 2007/12/03 10:07:24 veelken Exp $

/*************************************************************************
 * Auxiliary class representing a PVSS data-point in XDAQ                *
 *                                                                       *
 * Author: Christian Veelken, UC Davis				         *
 *                                                                       *
 * Last update: $Date: 2007/12/03 10:07:24 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#ifndef _PixelDCSPVSSDpe_h_
#define _PixelDCSPVSSDpe_h_

#include <string>

class PixelDCSPVSSDpe 
{
 public:
  PixelDCSPVSSDpe(const std::string& dpeName, const std::string& dpeValue);
  PixelDCSPVSSDpe(const std::string& dpeName, bool dpeValue);
  PixelDCSPVSSDpe(const std::string& dpeName, float dpeValue);
  PixelDCSPVSSDpe(const std::string& dpeName, unsigned int dpeValue);
  PixelDCSPVSSDpe(const std::string& dpeName, int dpeValue);
  ~PixelDCSPVSSDpe();

  void setAlias(const std::string& dpeAlias) { dpeAlias_ = dpeAlias; }

  const std::string& getName() const { return dpeName_; }
  const std::string& getAlias() const { return dpeAlias_; }
  const std::string& getValue() const { return dpeValue_; }

 private:
  std::string dpeName_;  // name of Data-Point (element) in PVSS "hardware" view
  std::string dpeAlias_; // name of Data-Point (element) in PVSS "logical" view
  std::string dpeValue_; // value of Data-Point element
};

#endif
