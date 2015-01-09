/*************************************************************************
 * Component of XDAQ Application for Pixel Online Software               *
 * Copyright (C) 2007, Cornell University	                         *
 * All rights reserved.                                                  *
 * Authors: Souvik Das and Anders Ryd                                    *
 *************************************************************************/

#ifndef _PixelAOHBiasCalibrationParameters_h_
#define _PixelAOHBiasCalibrationParameters_h_

namespace PixelAOHBiasCalibrationParameters {

	// Parameter defaults
	const unsigned int k_ScanMin_default = 0;
	const unsigned int k_ScanMax_default = 50;
	const unsigned int k_ScanStepSize_default = 1;
	const unsigned int k_TargetBMin_default = 512-100;
	const unsigned int k_TargetBMax_default = 512+100;
	const int k_SaturationPointOffset_default = 4; // = 5 degrees x (22.5 ADC counts at 2Vpp / degree) / (29 ADC counts at 2Vpp / AOH bias count) = 3.9 AOH bias counts ~= 4 AOH bias counts
	const std::string  k_MaxFEDReceiverInputOffset_default = "8";
	const std::string  k_SetAnalogInputBias_default  = "200";
	const std::string  k_SetAnalogOutputBias_default = "120";
	const std::string  k_SetAnalogOutputGain_default = "200";
	const bool k_printFEDRawData_default = false;
	const bool k_printFEDOffsetAdjustments_default = false;
	const bool k_printAOHBiasAdjustments_default = false;
	
	// enum for parameters passed to FED supervisors
	enum FEDParameters {k_WhatToDo, k_AOHBias, k_FEDNumber, k_FEDChannel, k_NumVars};

};

#endif
