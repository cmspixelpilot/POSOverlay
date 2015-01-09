/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2007, Cornell		        	                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd               					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/
#ifndef _PixelCalibrationFactory_h_
#define _PixelCalibrationFactory_h_

#include <string>

#include "PixelCalibrations/include/PixelCalibrationBase.h"
#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"
#include "PixelCalibrations/include/PixelTKFECCalibrationBase.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSSOAPCommander.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSPVSSCommander.h"


class PixelCalibrationFactory{

 public:

  PixelCalibrationFactory(){};

  virtual ~PixelCalibrationFactory(){};

  PixelCalibrationBase* getCalibration(const std::string& calibName,
				       const PixelSupervisorConfiguration* pixSupConfPtr, 
				       SOAPCommander* soapCmdrPtr,
				       PixelDCSSOAPCommander* dcsSoapCommanderPtr,
				       PixelDCSPVSSCommander* pvssCommanderPtr
				       ) const;

  PixelFEDCalibrationBase* getFEDCalibration(const std::string& calibName,
					     const PixelFEDSupervisorConfiguration* pixFEDSupConfPtr,
					     SOAPCommander* soapCmdrPtr) const;

  PixelTKFECCalibrationBase* getTKFECCalibration(const std::string& calibName,
                                                 const PixelTKFECSupervisorConfiguration* pixTKFECSupConfPtr,
                                                 SOAPCommander* soapCmdrPtr) const;

 private:

};

#endif
