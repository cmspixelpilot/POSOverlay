// $Id: PixelDCSPVSSCommander.h,v 1.6 2009/09/14 11:43:38 joshmt Exp $

/*************************************************************************
 * Auxiliary class to get and set values of PVSS data-points             *
 * via PSX Server interface                                              *
 *                                                                       *
 * Author: Christian Veelken, UC Davis				         *
 *                                                                       *
 * Last update: $Date: 2009/09/14 11:43:38 $ (UTC)                       *
 *          by: $Author: joshmt $                                       *
 *************************************************************************/

#ifndef _PixelDCSPVSSCommander_h_
#define _PixelDCSPVSSCommander_h_

#include <string>

#include "xdaq/Application.h"
#include "xdaq/exception/Exception.h"
#include "xoap/MessageReference.h"

#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSPSXCommander.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSPVSSDpe.h"

//class xdaq::Application;

class PixelDCSPVSSCommander : public PixelDCSPSXCommander
{
 public:
  PixelDCSPVSSCommander(xdaq::Application* xdaqApplication, xdaq::ApplicationDescriptor* psxDescriptor);
  virtual ~PixelDCSPVSSCommander();

  // functions to get the value of a PVSS data-point
  std::list<PixelDCSPVSSDpe> getDpeValues(const std::list<PixelDCSPVSSDpe>& dpes) ;//throw (xdaq::exception::Exception);
  std::string getDpeValue(const std::string& dpeName) ;//throw (xoap::exception::Exception);
  bool getDpeValue_bool(const std::string& dpeName) ;//throw (xdaq::exception::Exception); 
  float getDpeValue_float(const std::string& dpeName); 
  int getDpeValue_int(const std::string& dpeName);

  // functions to set the value of a PVSS data-point
  void setDpeValues(const std::list<PixelDCSPVSSDpe>& dpes) ;//throw (xdaq::exception::Exception);
  void setDpeValue(const std::string& dpeName, const::std::string& dpeValue) ;//throw (xdaq::exception::Exception);
  void setDpeValue_bool(const std::string& dpeName, bool dpeValue);
  void setDpeValue_float(const std::string& dpeName, float dpeValue);
  void setDpeValue_int(const std::string& dpeName, int dpeValue);

 //protected:
  xoap::MessageReference MakeSOAPMessageReference_dpGet(const std::string& dpeName);
  xoap::MessageReference MakeSOAPMessageReference_dpGet(const std::list<PixelDCSPVSSDpe>& dpes);
  xoap::MessageReference MakeSOAPMessageReference_dpSet(const std::string& dpeName, const::std::string& dpeValue);
  xoap::MessageReference MakeSOAPMessageReference_dpSet(const std::list<PixelDCSPVSSDpe>& dpes);
};

#endif
