// $Id: PixelGainAliveSCurveCalibration.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelEmulatedPhysics.h"

#include <toolbox/convertstring.h>

using namespace pos;

PixelEmulatedPhysics::PixelEmulatedPhysics(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  std::cout << "Greetings from the PixelEmulatedPhysics copy constructor." << std::endl;
}

bool PixelEmulatedPhysics::execute()
{
  //std::cout << "PixelEmulatedPhysics::execute()" << std::endl;

  return true;
  
}

std::vector<std::string> PixelEmulatedPhysics::calibrated(){

  std::vector<std::string> tmp;
  
  return tmp;

}
