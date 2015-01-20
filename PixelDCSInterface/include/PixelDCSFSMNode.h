// $Id: PixelDCSFSMNode.h,v 1.5 2009/07/09 08:24:08 joshmt Exp $

/**************************************************************************
 * Base class for storage of name and current state of                    *
 * a single PVSS FSM component                                            *
 * (representing one CAEN A4602 or A4603 power supply board)              *
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2009/07/09 08:24:08 $ (UTC)                        *
 *          by: $Author: joshmt $                                        *
 **************************************************************************/

#ifndef _PixelDCSFSMNode_h_
#define _PixelDCSFSMNode_h_

#include <string>

#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMNodeDefinition.h"

class PixelDCSFSMNode
{
 public:
  PixelDCSFSMNode(const std::string& name, const PixelDCSFSMNodeDefinition& deviceDefinition)
    : name_(name), domain_(""), state_("UNDEFINED"), used_(true), deviceDefinition_(deviceDefinition) {}
  virtual ~PixelDCSFSMNode() {}
    
  void setDomain(const std::string& domain) { domain_ = domain; }

  void setState(const std::string& state) { state_ = state; }

  void setUsed(const bool &used) { used_ = used;}
  
  const std::string& getName() const { return name_; }
  const std::string& getDomain() const { return domain_; }
  const std::string& getState() const { return state_; }
  const bool& isUsed() const { return used_;}

  const PixelDCSFSMNodeDefinition* getDeviceDefinition() const { return &deviceDefinition_; }
  
  virtual void writeTo(std::ostream& stream) const;
  
 protected:
  std::string name_; // name of PVSS FSM node 
  std::string domain_; // name of PVSS FSM "domain" (top-level Control Unit) within which the node is defined
  std::string state_; // current state of PVSS FSM node

  bool used_; //is the node currently part of the detector configuration?
  
  PixelDCSFSMNodeDefinition deviceDefinition_;
};

#endif
