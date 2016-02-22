/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2007, Cornell		        	                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd               					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/


#include "PixelCalibrations/include/PixelCalibrationFactory.h"

//PixelSupervisor calibrations
#include "PixelCalibrations/include/PixelEmulatedPhysics.h"
#include "PixelCalibrations/include/PixelDelay25Calibration.h"
#include "PixelCalibrations/include/PixelIanaCalibration.h"
#include "PixelCalibrations/include/PixelIdigiCalibration.h"
#include "PixelCalibrations/include/PixelGainAliveSCurveCalibration.h"
#include "PixelCalibrations/include/PixelGainAliveSCurveCalibrationWithSLink.h"
#include "PixelCalibrations/include/PixelThresholdCalDelayCalibration.h"
#include "PixelCalibrations/include/Pixel2DEfficiencyScan.h"
#include "PixelCalibrations/include/PixelCalDelCalibration.h"
#include "PixelCalibrations/include/PixelVcThrCalibration.h"
#include "PixelCalibrations/include/PixelTemperatureCalibration.h"
#include "PixelCalibrations/include/PixelThresholdCalDelayCalibrationFIFO1.h"
#include "PixelCalibrations/include/PixelVsfAndVHldDelCalibration.h"
#include "PixelCalibrations/include/PixelLinearityVsVsfCalibration.h"
#include "PixelCalibrations/include/PixelPHRangeCalibration.h"
#include "PixelCalibrations/include/PixelROCDelay25Calibration.h"
#include "PixelCalibrations/include/PixelTBMDelayCalibration.h"

//PixelFEDSupervisor calibrations
#include "PixelCalibrations/include/PixelFEDEmulatedPhysics.h"
#include "PixelCalibrations/include/PixelFED2DEfficiencyScan.h"
#include "PixelCalibrations/include/PixelFEDCalDelCalibration.h"
#include "PixelCalibrations/include/PixelFEDVcThrCalibration.h"
#include "PixelCalibrations/include/PixelFEDThresholdCalDelayCalibration.h"
#include "PixelCalibrations/include/PixelFEDThresholdCalDelayCalibrationFIFO1.h"
#include "PixelCalibrations/include/PixelFEDGainAliveSCurveCalibration.h"
#include "PixelCalibrations/include/PixelFEDGainAliveSCurveCalibrationWithSLink.h"
#include "PixelCalibrations/include/PixelFEDVsfAndVHldDelCalibration.h"
#include "PixelCalibrations/include/PixelFEDLinearityVsVsfCalibration.h"
#include "PixelCalibrations/include/PixelFEDPHRangeCalibration.h"
#include "PixelCalibrations/include/PixelFEDROCDelay25Calibration.h"
#include "PixelCalibrations/include/PixelFEDTBMDelayCalibration.h"

//PixelTKFECSupervisor calibrations
#include "PixelCalibrations/include/PixelTKFECDelay25Calibration.h"

PixelCalibrationBase* PixelCalibrationFactory::getCalibration(const std::string& calibName, const PixelSupervisorConfiguration* pixSupConfPtr, SOAPCommander* soapCmdrPtr,PixelDCSSOAPCommander* dcsSoapCommanderPtr, PixelDCSPVSSCommander* pvssCommanderPtr) const{
  if (calibName=="EmulatedPhysics"||
      calibName=="FEDBaselineWithTestDACs"||
      calibName=="FEDBaselineWithPixels"||
      calibName=="FEDBaselineWithPixelsNew"||
      calibName=="FEDAddressLevelWithPixels"||
      calibName=="FEDAddressLevelWithTestDACs"||
      calibName=="AOHBias"||
      calibName=="AOHAndFEDChannelMappingTest"||
      calibName=="AOHGain"||
      calibName=="TBMUB"||
      calibName=="ROCUBEqualization"||
      calibName=="ClockPhaseCalibration")
    assert(0);

  if (calibName=="EmulatedPhysics") {
    return new PixelEmulatedPhysics(*pixSupConfPtr,soapCmdrPtr);
  } 

  if (calibName=="Iana") {
    return new PixelIanaCalibration(*pixSupConfPtr, soapCmdrPtr);
  }  

  if (calibName=="Idigi") {
    return new PixelIdigiCalibration(*pixSupConfPtr, soapCmdrPtr);
  }  

  if (calibName=="VsfAndVHldDel") {
    return new PixelVsfAndVHldDelCalibration(*pixSupConfPtr, soapCmdrPtr);
  }
    
  if (calibName=="LinearityVsVsf") {
    return new PixelLinearityVsVsfCalibration(*pixSupConfPtr, soapCmdrPtr);
  }  

  if (calibName=="PHRange") {
    return new PixelPHRangeCalibration(*pixSupConfPtr, soapCmdrPtr);
  }  

  if ( calibName == "TemperatureCalibration" ) {
    assert(0);
    return new PixelTemperatureCalibration(*pixSupConfPtr, 
      soapCmdrPtr, 
      dcsSoapCommanderPtr,
      pvssCommanderPtr);
  }

  if (calibName=="GainCalibration" || calibName=="PixelAlive" || calibName=="SCurve" || calibName == "SCurveSmartRange") {
    return new PixelGainAliveSCurveCalibration(*pixSupConfPtr, soapCmdrPtr);
  }  

  if (calibName=="GainCalibrationWithSLink" || calibName=="PixelAliveWithSLink" || calibName=="SCurveWithSLink") {
    return new PixelGainAliveSCurveCalibrationWithSLink(*pixSupConfPtr, soapCmdrPtr);
  }  
   
  if (calibName=="2DEfficiencyScan") {	  
    return new Pixel2DEfficiencyScan(*pixSupConfPtr, soapCmdrPtr);
  }

  if (calibName=="CalDelCalibration") {	  
    return new PixelCalDelCalibration(*pixSupConfPtr, soapCmdrPtr);
  }

  if (calibName=="VcThrCalibration") {	  
    return new PixelVcThrCalibration(*pixSupConfPtr, soapCmdrPtr);
  }

  if (calibName=="ThresholdCalDelay") {	  
    return new PixelThresholdCalDelayCalibration(*pixSupConfPtr, soapCmdrPtr);
  }
 
  if (calibName=="ThresholdCalDelayFIFO1") {	  
    return new PixelThresholdCalDelayCalibrationFIFO1(*pixSupConfPtr, soapCmdrPtr);
  }  

  if (calibName=="Delay25") {
    return new PixelDelay25Calibration(*pixSupConfPtr, soapCmdrPtr);
  }

  if (calibName=="ROCDelay25") {
    return new PixelROCDelay25Calibration(*pixSupConfPtr, soapCmdrPtr);
  }

  if (calibName=="TBMDelay") {
    return new PixelTBMDelayCalibration(*pixSupConfPtr, soapCmdrPtr);
  }

  return 0;

}


PixelFEDCalibrationBase* PixelCalibrationFactory::getFEDCalibration(const std::string& calibName,
								    const PixelFEDSupervisorConfiguration* pixFEDSupConfPtr, 
								    SOAPCommander* soapCmdrPtr) const{
  if (calibName=="EmulatedPhysics"||
      calibName=="FEDBaselineWithTestDACs"||
      calibName=="FEDBaselineWithPixels"||
      calibName=="FEDBaselineWithPixelsNew"||
      calibName=="FEDAddressLevelWithPixels"||
      calibName=="FEDAddressLevelWithTestDACs"||
      calibName=="AOHBias"||
      calibName=="AOHAndFEDChannelMappingTest"||
      calibName=="AOHGain"||
      calibName=="TBMUB"||
      calibName=="ROCUBEqualization"||
      calibName=="ClockPhaseCalibration")
    assert(0);

  if (calibName=="EmulatedPhysics") {
    return new PixelFEDEmulatedPhysics(*pixFEDSupConfPtr, soapCmdrPtr);
  }  

  if (calibName=="2DEfficiencyScan") {
    return new PixelFED2DEfficiencyScan(*pixFEDSupConfPtr, soapCmdrPtr);
  }

  if (calibName=="CalDelCalibration") {
    return new PixelFEDCalDelCalibration(*pixFEDSupConfPtr, soapCmdrPtr);
  }

  if (calibName=="VcThrCalibration") {
    return new PixelFEDVcThrCalibration(*pixFEDSupConfPtr, soapCmdrPtr);
  }

  if (calibName=="ThresholdCalDelay") {
    return new PixelFEDThresholdCalDelayCalibration(*pixFEDSupConfPtr, soapCmdrPtr);
  }

  if (calibName=="ThresholdCalDelayFIFO1") {
    return new PixelFEDThresholdCalDelayCalibrationFIFO1(*pixFEDSupConfPtr, soapCmdrPtr);
  } 

  if (calibName=="Delay25") {
    return 0;
  }  
  
  if (calibName=="Iana") {
      //Don't need to do anything as FED is not involved in calibration.
  }

  if (calibName=="Idigi") {
      //Don't need to do anything as FED is not involved in calibration.
  }
  
  if (calibName=="VsfAndVHldDel") {
    return new PixelFEDVsfAndVHldDelCalibration(*pixFEDSupConfPtr, soapCmdrPtr);
  }
    
  if (calibName=="LinearityVsVsf") {
    return new PixelFEDLinearityVsVsfCalibration(*pixFEDSupConfPtr, soapCmdrPtr);
  }  

  if (calibName=="PHRange") {
    return new PixelFEDPHRangeCalibration(*pixFEDSupConfPtr, soapCmdrPtr);
  }  

  if (calibName=="TemperatureCalibration") {
    return 0;
  } 

  if (calibName=="GainCalibration" || 
      calibName=="PixelAlive" || 
      calibName=="SCurve" ||
      calibName=="SCurveSmartRange" ) {
    return new PixelFEDGainAliveSCurveCalibration(*pixFEDSupConfPtr, 
						  soapCmdrPtr);
  }  

  if (calibName=="GainCalibrationWithSLink" || 
      calibName=="PixelAliveWithSLink" || 
      calibName=="SCurveWithSLink") {
    return new PixelFEDGainAliveSCurveCalibrationWithSLink(*pixFEDSupConfPtr,
						  soapCmdrPtr);
  } 

  if (calibName=="ROCDelay25") {
    return new PixelFEDROCDelay25Calibration(*pixFEDSupConfPtr, soapCmdrPtr);
  }

  if (calibName=="TBMDelay") {
    return new PixelFEDTBMDelayCalibration(*pixFEDSupConfPtr, soapCmdrPtr);
  }

  return 0;

}

PixelTKFECCalibrationBase* PixelCalibrationFactory::getTKFECCalibration(const std::string& calibName,
                                                                    const PixelTKFECSupervisorConfiguration* pixTKFECSupConfPtr,
									SOAPCommander* soapCmdrPtr) const{

  if (calibName=="Delay25") {
    return new PixelTKFECDelay25Calibration(*pixTKFECSupConfPtr, soapCmdrPtr);
  }

  return 0;
}
