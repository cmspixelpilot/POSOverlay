// $Id: PixelJobControlMonitor.h,v 1.5 2009/09/03 09:32:31 joshmt Exp $

/**************************************************************************
 * Manages active POS processes by talking to JobControl                  *
 *                                                                        *
 * Author: Joshua Thompson, Cornell			 	          *
 *                                                                        *
 **************************************************************************/

#ifndef _PixelJobControlMonitor_h_
#define _PixelJobControlMonitor_h_

#include "toolbox/BSem.h"

#include "PixelUtilities/PixelJobControlUtilities/include/PixelJobControlNode.h"

class SOAPCommander;

class PixelJobControlMonitor
{
 public:
  PixelJobControlMonitor( xdaq::ApplicationContext* context);
  ~PixelJobControlMonitor();
	
  void doCheck( SOAPCommander* soapcommander);
  std::string getHtml() const;

 private:
  xdaq::ApplicationContext* context_;
  toolbox::BSem* lock_;
  std::string htmlSummary_;

  std::vector<PixelJobControlNode> nodes_;

  void Init();
  void printList( const std::vector<std::pair<std::string,unsigned int> > & list) ;

  std::string getJid(xdaq::ApplicationDescriptor *d);
  std::string getUrlWithoutPort(xdaq::ApplicationDescriptor* d);

};

#endif
