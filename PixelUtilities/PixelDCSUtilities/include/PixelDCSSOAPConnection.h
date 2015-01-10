// $Id: PixelDCSSOAPConnection.h,v 1.1 2007/08/10 14:47:04 veelken Exp $

/**************************************************************************
 * Auxiliary class for storage of information associated                  *
 * with a single SOAP connection;                                         *
 * used by                                                                *
 *   PixelDCSFSMInterface                                                 *
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2007/08/10 14:47:04 $ (UTC)                        *
 *          by: $Author: veelken $                                        *
 **************************************************************************/

#ifndef _PixelDCSSOAPConnection_h_
#define _PixelDCSSOAPConnection_h_

#include <string>

class PixelDCSSOAPConnection
{
 public:
  PixelDCSSOAPConnection(const std::string& name, const std::string& type, unsigned int instance)
    : name_(name), type_(type), instance_(instance) {}
  ~PixelDCSSOAPConnection() {}

  const std::string& getName() const { return name_; }
  const std::string& getType() const { return type_; }
  unsigned int getInstance() const { return instance_; }
	
  void writeTo(std::ostream& stream) const;
  
 protected:
  std::string name_; // name of SOAP connection (e.g. "PixelFEDSupervisor")
  std::string type_; // type of SOAP connection (e.g. "TrkFEC")
  unsigned int instance_; // instance number of SOAP connection
};

#endif
