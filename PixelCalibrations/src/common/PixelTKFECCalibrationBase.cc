// Modified by Jennifer Vaughan 2007/06/01
// $Id: PixelCalibrationBase.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/
#include <math.h>

#include "PixelCalibrations/include/PixelTKFECCalibrationBase.h"

using namespace pos;

PixelTKFECCalibrationBase::PixelTKFECCalibrationBase(const PixelTKFECSupervisorConfiguration & tempConfiguration, const SOAPCommander& soapCommander) 
  : PixelTKFECSupervisorConfiguration(tempConfiguration),
  SOAPCommander(soapCommander)
{
  std::cout << "Greetings from the PixelTKFECCalibrationBase copy constructor." << std::endl;
  //the copying of pointers is taken care of in PixSupConf
}

xoap::MessageReference PixelTKFECCalibrationBase::beginCalibration(xoap::MessageReference msg){

  return MakeSOAPMessageReference("PixelTKFECCalibrationBase::beginCalibration Done");
}


xoap::MessageReference PixelTKFECCalibrationBase::endCalibration(xoap::MessageReference msg){

  return MakeSOAPMessageReference("PixelTKFECCalibrationBase::endCalibration Done");

}


