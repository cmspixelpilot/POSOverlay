// $Id: PixelDCSFSMStateTranslation.h,v 1.1 2007/08/10 14:47:03 veelken Exp $

/**************************************************************************
 * Auxiliary class for translation between XDAQ and PVSS FSM states;      * 
 * the translation table is composed of pairs                             *
 *   { XDAQ state, PVSS state},                                           *
 * used by                                                                *
 *   PixelDCSSupervisor                                                   *
 *   PixelDCSFSMInterface                                                 *
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2007/08/10 14:47:03 $ (UTC)                        *
 *          by: $Author: veelken $                                        *
 **************************************************************************/

#ifndef _PixelDCSFSMStateTranslation_h_
#define _PixelDCSFSMStateTranslation_h_

#include <string>

class PixelDCSFSMStateTranslation
{
 public:
  PixelDCSFSMStateTranslation(const std::string pvssState, const std::string xdaqState)
    : pvssState_(pvssState), xdaqState_(xdaqState) {}
  ~PixelDCSFSMStateTranslation() {}

  const std::string& getPvssState() const { return pvssState_; }
  const std::string& getXdaqState() const { return xdaqState_; }
	
  void writeTo(std::ostream& stream) const;
  
 protected:
  std::string pvssState_;
  std::string xdaqState_;
};

#endif
