#include "PixelDCSInterface/include/PixelDCSFSMPartition.h"

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
 * Last update: $Date: 2009/09/14 10:01:53 $ (UTC)                        *
 *          by: $Author: joshmt $                                        *
 **************************************************************************/

#include <iostream>
#include <iomanip>

#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMDeviceDefinition.h"

PixelDCSFSMPartition::PixelDCSFSMPartition(const std::string& fsmPartitionName,
					   const std::string& fsmPartitionTopName,
					   const std::string& fsmPartitionTopDomain,
					   const std::list<std::pair<std::string, PixelDCSFSMNodeA4602> >& fsmNodeListA4602,
					   const std::list<std::pair<std::string, PixelDCSFSMNodeA4603> >& fsmNodeListA4603,
					   const std::list<PixelDCSSOAPConnection>& soapConnections)
  : name_(fsmPartitionName),
    topname_(fsmPartitionTopName),
    topdomain_(fsmPartitionTopDomain)
{
  for ( std::list<std::pair<std::string, PixelDCSFSMNodeA4602> >::const_iterator fsmNode = fsmNodeListA4602.begin();
	fsmNode != fsmNodeListA4602.end(); ++fsmNode ) {
    fsmNodeMapA4602_[fsmNode->first] = new PixelDCSFSMNodeA4602(fsmNode->second);
    
    const std::string initialPvssState = fsmNode->second.getState();
    const std::string initialXdaqState = fsmNode->second.getDeviceDefinition()->getXdaqState(initialPvssState);
    std::cout<<"PixelDCSFSMPartition::PixelDCSFSMPartition PixelDCSFSMNodeA4602 initialXdaqState:"<<initialXdaqState<<std::endl;
   ++(numNodesMapA4602_[initialXdaqState]);
  }

  summarizedStateA4602_ = "UNDEFINED";
  numNodesIgnoredA4602_=0;

  for ( std::list<std::pair<std::string, PixelDCSFSMNodeA4603> >::const_iterator fsmNode = fsmNodeListA4603.begin();
	fsmNode != fsmNodeListA4603.end(); ++fsmNode ) {
    fsmNodeMapA4603_[fsmNode->first] = new PixelDCSFSMNodeA4603(fsmNode->second);
    
    const std::string initialPvssState = fsmNode->second.getState();
    const std::string initialXdaqState = fsmNode->second.getDeviceDefinition()->getXdaqState(initialPvssState);
    std::cout<<"PixelDCSFSMPartition::PixelDCSFSMPartition PixelDCSFSMNodeA4603 initialXdaqState:"<<initialXdaqState<<std::endl;
    ++(numNodesMapA4603_[initialXdaqState]);
  }

  summarizedStateA4603_ = "UNDEFINED";
  numNodesIgnoredA4603_=0;

  for ( std::list<PixelDCSSOAPConnection>::const_iterator soapConnection = soapConnections.begin();
	soapConnection != soapConnections.end(); ++soapConnection ) {
    soapConnections_.push_back(*soapConnection);

std::cout << "adding --> soapConnection::getName = " << soapConnection->getName() << std::endl;
  }
}

PixelDCSFSMPartition::PixelDCSFSMPartition(const PixelDCSFSMPartition& fsmPartition)
{
//--- copy constructor
//
//    NOTE: need customized copy constructor,
//          as otherwise PixelDCSFSMNode objects stored in the fsmNodeMap_ data-member
//          are shared between different instances of the PixelDCSFSMPartition class,
//          which causes a segmentation fault once the first PixelDCSFSMPartition instance gets destructed
//          (as it results in the PixelDCSFSMNode objects to be deleted)
//
  name_ = fsmPartition.name_;
  topname_ = fsmPartition.topname_;
  topdomain_ = fsmPartition.topdomain_;

  for ( std::map<std::string, PixelDCSFSMNodeA4602*>::const_iterator fsmNode = fsmPartition.fsmNodeMapA4602_.begin();
	fsmNode != fsmPartition.fsmNodeMapA4602_.end(); ++fsmNode ) {
    fsmNodeMapA4602_[fsmNode->first] = new PixelDCSFSMNodeA4602(*fsmNode->second);
  }

  numNodesMapA4602_ = fsmPartition.numNodesMapA4602_;
  summarizedStateA4602_ = fsmPartition.summarizedStateA4602_;
  //summarizedStateA4602_ = "LV_ON";//over here
  //std::cout<<"PixelDCSFSMPartition::PixelDCSFSMPartition summarizedStateA4602_:"<<summarizedStateA4602_<<std::endl; //over here
  numNodesIgnoredA4602_ = fsmPartition.numNodesIgnoredA4602_;

  for ( std::map<std::string, PixelDCSFSMNodeA4603*>::const_iterator fsmNode = fsmPartition.fsmNodeMapA4603_.begin();
	fsmNode != fsmPartition.fsmNodeMapA4603_.end(); ++fsmNode ) {
    fsmNodeMapA4603_[fsmNode->first] = new PixelDCSFSMNodeA4603(*fsmNode->second);
  }

  numNodesMapA4603_ = fsmPartition.numNodesMapA4603_;
  summarizedStateA4603_ = fsmPartition.summarizedStateA4603_;
  //summarizedStateA4603_ = "LV_ON";//over here
  //std::cout<<"PixelDCSFSMPartition::PixelDCSFSMPartition summarizedStateA4603_:"<<summarizedStateA4603_<<std::endl; //over here
  numNodesIgnoredA4603_ = fsmPartition.numNodesIgnoredA4603_;

  for ( std::list<PixelDCSSOAPConnection>::const_iterator soapConnection = fsmPartition.soapConnections_.begin();
	soapConnection != fsmPartition.soapConnections_.end(); ++soapConnection ) {
    soapConnections_.push_back(*soapConnection);
  }
}

PixelDCSFSMPartition::~PixelDCSFSMPartition()
{

  for ( std::map<std::string, PixelDCSFSMNodeA4602*>::const_iterator fsmNode = fsmNodeMapA4602_.begin();
	fsmNode != fsmNodeMapA4602_.end(); ++fsmNode ) {
    delete fsmNode->second;
  }

  for ( std::map<std::string, PixelDCSFSMNodeA4603*>::const_iterator fsmNode = fsmNodeMapA4603_.begin();
	fsmNode != fsmNodeMapA4603_.end(); ++fsmNode ) {
    delete fsmNode->second;
  }
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelDCSFSMPartition::setNodeStateA4602(const std::string& nodeName, const std::string& pvssStateName) throw (xdaq::exception::Exception)
{
  setNodeState<PixelDCSFSMNodeA4602>(fsmNodeMapA4602_, numNodesMapA4602_, nodeName, pvssStateName);
}

void PixelDCSFSMPartition::setNodeUsedA4602(const std::string& nodeName, const bool& used) throw (xdaq::exception::Exception)
{
  setNodeUsed<PixelDCSFSMNodeA4602>(fsmNodeMapA4602_, numNodesMapA4602_, numNodesIgnoredA4602_, nodeName, used);
}

const PixelDCSFSMNodeA4602* PixelDCSFSMPartition::getNodeA4602(const std::string& nodeName)
{
  return getNode<PixelDCSFSMNodeA4602>(fsmNodeMapA4602_, nodeName);
}
 
std::list<const PixelDCSFSMNodeA4602*> PixelDCSFSMPartition::getNodeListA4602() const
{
  return getNodeList<PixelDCSFSMNodeA4602>(fsmNodeMapA4602_);
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelDCSFSMPartition::setNodeStateA4603(const std::string& nodeName, const std::string& pvssStateName) throw (xdaq::exception::Exception)
{
  setNodeState<PixelDCSFSMNodeA4603>(fsmNodeMapA4603_, numNodesMapA4603_, nodeName, pvssStateName);
}

void PixelDCSFSMPartition::setNodeUsedA4603(const std::string& nodeName, const bool& used) throw (xdaq::exception::Exception)
{
  setNodeUsed<PixelDCSFSMNodeA4603>(fsmNodeMapA4603_, numNodesMapA4603_, numNodesIgnoredA4603_, nodeName, used);
}

const PixelDCSFSMNodeA4603* PixelDCSFSMPartition::getNodeA4603(const std::string& nodeName)
{
  return getNode<PixelDCSFSMNodeA4603>(fsmNodeMapA4603_, nodeName);
}
 
std::list<const PixelDCSFSMNodeA4603*> PixelDCSFSMPartition::getNodeListA4603() const
{
  return getNodeList<PixelDCSFSMNodeA4603>(fsmNodeMapA4603_);
}

//
//---------------------------------------------------------------------------------------------------
//

template <class T> void PixelDCSFSMPartition::setNodeState(std::map<std::string, T*>& fsmNodeMap, 
							   std::map<std::string, unsigned int>& numNodesMap,
							   const std::string& nodeName, 
							   const std::string& pvssStateName) //throw (xdaq::exception::Exception)
{
  T* fsmNode = fsmNodeMap[nodeName];

  if ( fsmNode != NULL ) {
    const std::string lastPvssState = fsmNode->getState();
    const bool used = fsmNode->isUsed();
    fsmNode->setState(pvssStateName);
    //std::cout<<"PixelDCSFSMPartition::setNodeState pvssStateName:"<<pvssStateName<<std::endl;//over here

    if ( fsmNode->getDeviceDefinition() != NULL ) {
      const std::string lastXdaqState = fsmNode->getDeviceDefinition()->getXdaqState(lastPvssState);
      const std::string currentXdaqState = fsmNode->getDeviceDefinition()->getXdaqState(pvssStateName);
      //std::string currentXdaqState = "LV_ON";//over here
      //std::cout<<"PixelDCSFSMPartition::setNodeState lastXdaqState:"<<lastXdaqState<<std::endl;//over here
      //currentXdaqState = pvssStateName;//over here
      //std::cout<<"PixelDCSFSMPartition::setNodeState currentXdaqState:"<<currentXdaqState<<std::endl;//over here
      if ( (currentXdaqState != lastXdaqState) && used) {      //if it is not used then we don't care
	--(numNodesMap[lastXdaqState]);
	++(numNodesMap[currentXdaqState]);
      }
    } else {
      XCEPT_RAISE (xdaq::exception::Exception, "Undefined FSM Device Type");
    }
  } else {
    XCEPT_RAISE (xdaq::exception::Exception, "Undefined FSM Node");
  }
}

template <class T> void PixelDCSFSMPartition::setNodeUsed(std::map<std::string, T*>& fsmNodeMap, 
							  std::map<std::string, unsigned int>& numNodesMap,
							  unsigned int& numNodesIgnored,
							  const std::string& nodeName, 
							  const bool& used) //throw (xdaq::exception::Exception)
{
  T* fsmNode = fsmNodeMap[nodeName];

  if ( fsmNode != NULL ) {
    const std::string pvssState = fsmNode->getState();
    //std::string pvssState = "LV_ON_REDUCED";//over here
    //std::cout<<"PixelDCSFSMPartition::setNodeUsed pvssState:"<<pvssState<<std::endl;//over here
    const bool lastUsed = fsmNode->isUsed();
    fsmNode->setUsed(used);

    if ( fsmNode->getDeviceDefinition() != NULL ) {
      const std::string xdaqState = fsmNode->getDeviceDefinition()->getXdaqState(pvssState);
      //std::string xdaqState = "LV_ON_REDUCED";//over here
      //std::cout<<"PixelDCSFSMPartition::setNodeUsed xdaqState:"<<xdaqState<<std::endl;//over here
      if (!lastUsed && used) 	 {
	++(numNodesMap[xdaqState]);
	--numNodesIgnored;
      }
      else  if (lastUsed && !used) {
	--(numNodesMap[xdaqState]);
	++numNodesIgnored;
      }
    } else {
      XCEPT_RAISE (xdaq::exception::Exception, "Undefined FSM Device Type");
    }
  } else {
    XCEPT_RAISE (xdaq::exception::Exception, "Undefined FSM Node");
  }
}

template <class T> const T* PixelDCSFSMPartition::getNode(const std::map<std::string, T*>& fsmNodeMap, 
							  const std::string& nodeName) const
{
  const T* node = NULL;
  for ( typename std::map<std::string, T*>::const_iterator fsmNode = fsmNodeMap.begin();
	fsmNode != fsmNodeMap.end(); ++fsmNode ) {
    if ( fsmNode->first == nodeName ) {
      node = fsmNode->second;
    }
  }

  return node;
}

template <class T> std::list<const T*> PixelDCSFSMPartition::getNodeList(const std::map<std::string, T*>& fsmNodeMap) const
{
  std::list<const T*> nodeList;
  for ( typename std::map<std::string, T*>::const_iterator fsmNode = fsmNodeMap.begin();
	fsmNode != fsmNodeMap.end(); ++fsmNode ) {
    nodeList.push_back(fsmNode->second);
  }

  return nodeList;
}

//should add output about ignored nodes here
void PixelDCSFSMPartition::writeTo(std::ostream& stream) const
{
  stream << " partition = " << name_ << std::endl;
  stream << " CAEN A4602 nodes:" << std::endl;
  stream << "  summarized state = " << summarizedStateA4602_ << std::endl;
  stream << "  number of nodes in XDAQ state" << std::endl;
  for ( std::map<std::string, unsigned int>::const_iterator numNodes = numNodesMapA4602_.begin();
	numNodes != numNodesMapA4602_.end(); ++numNodes ) {
    stream << "   XDAQ state \"" << numNodes->first << "\" = " << numNodes->second << std::endl;
  }
  for ( std::map<std::string, PixelDCSFSMNodeA4602*>::const_iterator fsmNode = fsmNodeMapA4602_.begin();
	fsmNode != fsmNodeMapA4602_.end(); ++fsmNode ) {
    fsmNode->second->writeTo(stream);
  }

  stream << " CAEN A4603 nodes:" << std::endl;
  stream << "  summarized state = " << summarizedStateA4603_ << std::endl;
  stream << "  number of nodes in XDAQ state" << std::endl;
  for ( std::map<std::string, unsigned int>::const_iterator numNodes = numNodesMapA4603_.begin();
	numNodes != numNodesMapA4603_.end(); ++numNodes ) {
    stream << "   XDAQ state \"" << numNodes->first << "\" = " << numNodes->second << std::endl;
  }
  for ( std::map<std::string, PixelDCSFSMNodeA4603*>::const_iterator fsmNode = fsmNodeMapA4603_.begin();
	fsmNode != fsmNodeMapA4603_.end(); ++fsmNode ) {
    fsmNode->second->writeTo(stream);
  }

  stream << " SOAP connections:" << std::endl;
  for ( std::list<PixelDCSSOAPConnection>::const_iterator soapConnection = soapConnections_.begin();
	 soapConnection != soapConnections_.end(); ++soapConnection ) {
    soapConnection->writeTo(stream);
  }
}
