// $Id: PixelJobControlNode.h,v 1.4 2009/09/03 09:32:31 joshmt Exp $

/**************************************************************************
 * Keeps a list of processes on one node                                  *
 *                                                                        *
 * Author: Joshua Thompson, Cornell			 	          *
 *                                                                        *
 **************************************************************************/

#ifndef _PixelJobControlNode_h_
#define _PixelJobControlNode_h_

#include "xdaq/Application.h"

class SOAPCommander;

class PixelJobControlNode
{
 public:
  PixelJobControlNode(xdaq::ApplicationDescriptor* d, std::string machineName);
  ~PixelJobControlNode();
	
  void add(std::string jid, std::string name, unsigned int instance);
  void checkStatus( SOAPCommander* soapcommander) ;

  bool isJobControlOK() const;
  bool isJobControlInError() const;
  bool isJobControlUnresponsive() const;
  std::string getName() const { return machineName_;}
  std::vector<std::pair<std::string,unsigned int> > getCrashedList() const;
  std::vector<std::pair<std::string,unsigned int> > getAliveList() const;

  std::vector<std::pair<std::string,unsigned int> > getListInState(const std::string & state) const;
  std::vector<std::pair<std::string,unsigned int> > getListNotInState(const std::string & state) const;

 private:
  xdaq::ApplicationDescriptor* jc_;
  std::string machineName_;
  std::string jcstatus_;

  std::map<std::string, std::string> jidmap_;
  //       jid                     class name   instance
  std::map<std::string, std::pair<std::string, unsigned int> > jidToName_;

};

#endif
