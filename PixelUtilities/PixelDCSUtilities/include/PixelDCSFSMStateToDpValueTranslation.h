// $Id: PixelDCSFSMStateToDpValueTranslation.h,v 1.1 2007/08/10 14:47:03 veelken Exp $

/**************************************************************************
 * Auxiliary class for translation between XDAQ states                    *
 * and corresponding values of associated PVSS data-points;               *
 * the translation table is composed of pairs                             *
 *   { XDAQ state, PVSS data-point value },                               *
 * used by                                                                *
 *   PixelDCSFSMInterface                                                 *
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2007/08/10 14:47:03 $ (UTC)                        *
 *          by: $Author: veelken $                                        *
 **************************************************************************/

#ifndef _PixelDCSFSMStateToDpValueTranslation_h_
#define _PixelDCSFSMStateToDpValueTranslation_h_

#include <string>
#include <iostream>

class PixelDCSFSMStateToDpValueTranslation
{
 public:
  PixelDCSFSMStateToDpValueTranslation(const std::string& xdaqState, const std::string& pvssDpValue);
  ~PixelDCSFSMStateToDpValueTranslation();
    
  const std::string& getXdaqState() const { return xdaqState_; }
  const std::string& getPvssDpValue() const { return pvssDpValue_; }
    
  void writeTo(std::ostream& stream) const;
    
 protected:
  std::string xdaqState_;
  std::string pvssDpValue_;
};

#endif
