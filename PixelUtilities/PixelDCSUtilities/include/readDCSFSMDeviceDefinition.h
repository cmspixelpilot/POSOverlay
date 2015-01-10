// $Id: readDCSFSMDeviceDefinition.h,v 1.2 2007/08/22 00:07:46 aryd Exp $

/*************************************************************************
 * Auxiliary function to parse information neccessary                    *
 * for translation between XDAQ FSM states and commands on the one hand  *
 * and PVSS FSM states and commands on the other hand                    *
 * for a single FSM node (device or logical/control unit)                *
 * from an xml file and initialize PixelDCSFSMDeviceDefinitionClass      *
 *                                                                       *
 * Author: Christian Veelken, UC Davis			 	         *
 *                                                                       *
 * Last update: $Date: 2007/08/22 00:07:46 $ (UTC)                       *
 *          by: $Author: aryd $                                       *
 *************************************************************************/

#ifndef _readFSMDeviceDefinition_h_
#define _readFSMDeviceDefinition_h_

#include "xdaq/exception/Exception.h"
#include "xercesc/dom/DOMNode.hpp"

#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMNodeDefinition.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSFSMDeviceDefinition.h"

PixelDCSFSMNodeDefinition readFSMNodeDefinition(xercesc::DOMNode* configNode) throw (xdaq::exception::Exception);
PixelDCSFSMDeviceDefinition readFSMDeviceDefinition(xercesc::DOMNode* configNode) throw (xdaq::exception::Exception);

#endif
