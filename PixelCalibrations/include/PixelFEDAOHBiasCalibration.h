/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelFEDAOHBiasCalibration_h_
#define _PixelFEDAOHBiasCalibration_h_

#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"

#include "PixelUtilities/PixelFEDDataTools/include/PixelMode.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelScanRecord.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"

#include "TFile.h"
#include "TStyle.h"

class PixelFEDAOHBiasCalibration: public PixelFEDCalibrationBase {
 public:

  PixelFEDAOHBiasCalibration( const PixelFEDSupervisorConfiguration &, SOAPCommander* );
  virtual ~PixelFEDAOHBiasCalibration(){};

  virtual xoap::MessageReference execute(xoap::MessageReference msg);

  virtual xoap::MessageReference beginCalibration(xoap::MessageReference msg);

  virtual xoap::MessageReference endCalibration(xoap::MessageReference msg);

  virtual void initializeFED();

 private:

  // PixelFEDAOHBiasCalibration Constructor should never be called
  //PixelFEDAOHBiasCalibration();

	void RetrieveData(unsigned int AOHBias, std::string step);
	xoap::MessageReference FindSaturationPoint(unsigned int fedNumber, unsigned int channel);
	xoap::MessageReference RaiseFEDReceiverInputOffsets();
	xoap::MessageReference RetrieveBlackLevel(unsigned int fedNumber, unsigned int channel);
	xoap::MessageReference WriteConfigFiles();

	enum TimeSlotCode { kNotEnoughTimeSlotEntries, kTimeSlotFluctuates, kGoodSingleTimeSlot, kGoodJumpingByOneTimeSlot };
	std::pair<TimeSlotCode, unsigned int> TimeSlotCodeForChannel(unsigned int fedNumber, unsigned int channel);
	bool badTimeSlotInfo(unsigned int fedNumber, unsigned int channel);
	std::string timeSlotErrorCode(unsigned int fedNumber, unsigned int channel);

	// For finding the start slots of the TBM header and trailer.
	//        FED number              channel number
	std::map< unsigned int, std::map< unsigned int, PixelMode > > TBMHeaderStart_;
	std::map< unsigned int, std::map< unsigned int, PixelMode > > TBMTrailerStart_;
	
	// For the AOH bias scan.
	std::map< unsigned int, std::map< unsigned int, PixelScanRecord > > TBMUB_;
	std::map< unsigned int, std::map< unsigned int, PixelScanRecord > > TBMB_;
	std::map< unsigned int, std::map< unsigned int, PixelScanRecord > > TBMBUBDiff_;
	// Valeria
	std::map< unsigned int, std::map< unsigned int, PixelScanRecord > > TBMUBNoise_;
	std::map< unsigned int, std::map< unsigned int, PixelScanRecord > > TBMBNoise_;
	// end Valeria
	
	// used only if time slots aren't found, take first 10 slots of the transparent buffer
	std::map< unsigned int, std::map< unsigned int, PixelScanRecord > > BaselineB_;
	
	// For the coarse baseline calibration steps.
	std::map< unsigned int, std::map< unsigned int, Moments > > TBMB_Moments_;

	// For outputing plots.
	TStyle* plainStyle_;
	TFile* outputFile_;

};

#endif
