// $Id: PixelDCSSOAPCommander.h,v 1.10 2007/11/14 20:26:57 zatserkl Exp $

/*************************************************************************
 * Auxiliary class to create SOAP messages used by XDAQ-DCS applications *
 *                                                                       *
 * Author: Christian Veelken, UC Davis				         *
 *                                                                       *
 * Last update: $Date: 2007/11/14 20:26:57 $ (UTC)                       *
 *          by: $Author: zatserkl $                                       *
 *************************************************************************/

#ifndef _PixelDCSSOAPCommander_h_
#define _PixelDCSSOAPCommander_h_

#include <string>
#include <list>
#include <utility> // header file for std::pair

#include "xoap/MessageReference.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPBody.h"

#include "CalibFormats/SiPixelObjects/interface/PixelNameTranslation.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFECConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"
#include "CalibFormats/SiPixelObjects/interface/PixelHdwAddress.h"

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"

//class xdaq::Application;

class PixelDCSSOAPCommander : public SOAPCommander
{
 public:
  PixelDCSSOAPCommander(xdaq::Application* xdaqApplication);
  virtual ~PixelDCSSOAPCommander();

  xoap::MessageReference MakeSOAPMessageReference_progDAC(unsigned int dacAddress,
							  const std::map<pos::PixelROCName, unsigned int>& dacValues_set,
							  const std::list<pos::PixelROCName>& dacValues_increase,
							  const std::list<pos::PixelROCName>& dacValues_decrease,
							  const pos::PixelNameTranslation* nameTranslation,
							  const pos::PixelFECConfig* fecConfiguration);

  xoap::MessageReference MakeSOAPMessageReference_readLastDAC();
  xoap::MessageReference MakeSOAPMessageReference_readDCU();

  xoap::MessageReference postSOAP(xdaq::ApplicationDescriptor* applicationDescriptor, xoap::MessageReference soapMessage) throw (xoap::exception::Exception);

 protected:
  void AddSOAPMessageReference_progDAC(xoap::SOAPEnvelope& soapEnvelope, xoap::SOAPElement& soapElement,
				       const std::list<std::pair<pos::PixelROCName, unsigned int> >& dacValues,
				       const pos::PixelNameTranslation* nameTranslation,
				       const pos::PixelFECConfig* fecConfiguration);
  void AddSOAPMessageReference_progDAC(xoap::SOAPEnvelope& soapEnvelope, xoap::SOAPElement& soapElement,
				       const std::map<pos::PixelROCName, unsigned int>& dacValues,
				       const pos::PixelNameTranslation* nameTranslation,
				       const pos::PixelFECConfig* fecConfiguration);
  void AddSOAPMessageReference_progDAC(xoap::SOAPEnvelope& soapEnvelope, xoap::SOAPElement& soapElement,
				       const std::list<pos::PixelROCName>& dacValues,
				       const pos::PixelNameTranslation* nameTranslation,
				       const pos::PixelFECConfig* fecConfiguration);  
};

#endif
