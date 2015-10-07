// $Id: PixelDCSFSMPartition.h,v 1.6 2009/09/14 10:01:52 joshmt Exp $

/**************************************************************************
 * Auxiliary class for storage of name and current "summarized" state of  *
 * a set of PVSS FSM components (representing the low voltage channels    *
 * of CAEN A4602 or A4603 power supply boards);                           *
 * the "summarized" state represents the logical "&&" combination of all  *
 * low voltage channels (summarized state is "LV_ON" if all LV channels   *
 * are on, and "LV_OFF" otherwise) in one half-cylinder/half-shell        *
 * or TTC partition (entire Forward/Barrel Pixel detector)                *
 *                                                                        *
 * Author: Christian Veelken, UC Davis			 	          *
 *                                                                        *
 * Last update: $Date: 2009/09/14 10:01:52 $ (UTC)                        *
 *          by: $Author: joshmt $                                        *
 **************************************************************************/

#ifndef _PixelDCSFSMPartition_h_
#define _PixelDCSFSMPartition_h_

#include <string>
#include <list>
#include <map>

#include "xdaq/exception/Exception.h"

#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSSOAPConnection.h"

#include "PixelDCSInterface/include/PixelDCSFSMNodeA4602.h"
#include "PixelDCSInterface/include/PixelDCSFSMNodeA4603.h"

class PixelDCSFSMPartition
{
 public:
  PixelDCSFSMPartition(const std::string& fsmPartitionName,
		       const std::string& fsmPartitionTopName,
		       const std::string& fsmPartitionTopDomain,
		       const std::list<std::pair<std::string, PixelDCSFSMNodeA4602> >& fsmNodeListA4602,
		       const std::list<std::pair<std::string, PixelDCSFSMNodeA4603> >& fsmNodeListA4603,
		       const std::list<PixelDCSSOAPConnection>& soapConnections);
  PixelDCSFSMPartition(const PixelDCSFSMPartition& fsmPartition);
  ~PixelDCSFSMPartition();

  const std::string& getName() const { return name_; }

  const std::list<PixelDCSSOAPConnection>& getSOAPConnections() const { return soapConnections_; }
  
  void writeTo(std::ostream& stream) const;

  // functions specific to CAEN A4602 type power supply boards  
  void setNodeStateA4602(const std::string& nodeName, const std::string& pvssStateName) throw (xdaq::exception::Exception);
  void setNodeUsedA4602(const std::string& nodeName, const bool& used) throw (xdaq::exception::Exception);
  void setSummarizedStateA4602(const std::string& state) { summarizedStateA4602_ = state; }
  const std::string& getSummarizedStateA4602() const { return summarizedStateA4602_; }
  const PixelDCSFSMNodeA4602* getNodeA4602(const std::string& nodeName);
  std::list<const PixelDCSFSMNodeA4602*> getNodeListA4602() const;
  unsigned int getNumNodesA4602(const std::string& xdaqState) const { return numNodesMapA4602_[xdaqState]; }
  unsigned int getNumNodesIgnoredA4602() const { return numNodesIgnoredA4602_;}

  // functions specific to CAEN A4603 type power supply boards  
  void setNodeStateA4603(const std::string& nodeName, const std::string& pvssStateName) throw (xdaq::exception::Exception);
  void setNodeUsedA4603(const std::string& nodeName, const bool& used) throw (xdaq::exception::Exception);
  void setSummarizedStateA4603(const std::string& state) { summarizedStateA4603_ = state; }
  const std::string& getSummarizedStateA4603() const { return summarizedStateA4603_; }
  const PixelDCSFSMNodeA4603* getNodeA4603(const std::string& nodeName);
  std::list<const PixelDCSFSMNodeA4603*> getNodeListA4603() const;
  unsigned int getNumNodesA4603(const std::string& xdaqState) const { return numNodesMapA4603_[xdaqState]; }
  unsigned int getNumNodesIgnoredA4603() const { return numNodesIgnoredA4603_;}    

  const std::string& getTopName() const { return topname_; }
  const std::string& getTopDomain() const { return topdomain_; }

 protected:
  template <class T> void setNodeState(std::map<std::string, T*>& fsmNodeMap, std::map<std::string, unsigned int>& numNodesMap,
				       const std::string& nodeName, const std::string& pvssStateName);// throw (xdaq::exception::Exception);
  template <class T> void setNodeUsed(std::map<std::string, T*>& fsmNodeMap, std::map<std::string, unsigned int>& numNodesMap,
				      unsigned int& numNodesIgnored,
				      const std::string& nodeName, const bool& used) ;//throw (xdaq::exception::Exception);
  template <class T> const T* getNode(const std::map<std::string, T*>& fsmNodeMap, const std::string& nodeName) const;
  template <class T> std::list<const T*> getNodeList(const std::map<std::string, T*>& fsmNodeMap) const;
 
  std::string name_; // name of TTC partition
  std::string topname_;
  std::string topdomain_;

  std::list<PixelDCSSOAPConnection> soapConnections_; // list of SOAP connections to be notified whenever (summarized) state of partition changes

  // data-members specific to CAEN A4602 type power supply boards 
  std::string summarizedStateA4602_; // summarized state of CAEN A4602 type power supply boards in TTC partition
  std::map<std::string, PixelDCSFSMNodeA4602*> fsmNodeMapA4602_; // list of FSM nodes representing A4602 type power supplies in TTC partition,
                                                                 // together with tables holding information 
                                                                 // how-to translate PVSS to XDAQ FSM states
  mutable std::map<std::string, unsigned int> numNodesMapA4602_; // number of PVSS FSM nodes representing A4602 type power supplies
                                                                 // currently in a state corresponding to XDAQ state given as function argument
  unsigned int numNodesIgnoredA4602_;

  // data-members specific to CAEN A4603 type power supply boards  
  std::string summarizedStateA4603_; // summarized state of CAEN A4603 type power supply boards in TTC partition
  std::map<std::string, PixelDCSFSMNodeA4603*> fsmNodeMapA4603_; // list of FSM nodes representing A4603 type power supplies in TTC partition,
                                                                 // together with tables holding information 
                                                                 // how-to translate PVSS to XDAQ FSM states
  mutable std::map<std::string, unsigned int> numNodesMapA4603_; // number of PVSS FSM nodes representing A4603 type power supplies
                                                                 // currently in a state corresponding to XDAQ state given as function argument
  unsigned int numNodesIgnoredA4603_;

};

#endif
