/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _PixelCalibrationBase_h_
#define _PixelCalibrationBase_h_

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"
#include "PixelSupervisorConfiguration/include/PixelSupervisorConfiguration.h"

#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"

class PixelCalibrationBase : public PixelSupervisorConfiguration, public SOAPCommander
{
 public:

  PixelCalibrationBase( const PixelSupervisorConfiguration &,
			const SOAPCommander& soapCommander);

  virtual ~PixelCalibrationBase();

  virtual void beginCalibration();

  virtual bool execute() = 0;

  virtual void endCalibration();

  //Returns the list of calibrated objecs, e.g. "fedcard", "portcard"
  virtual std::vector<std::string> calibrated()=0;

  //These methods call the beginCalibration and
  //endCalibration methods respectively and also
  //invoke the FED beginCalibration and endCalibration

  void runBeginCalibration();
  
  bool runEvent();

  void runEndCalibration();


  double getPercentageOfJob(){return percentageOfJob_;}
  void setPercentageOfJob(double percentageOfJob){percentageOfJob_=percentageOfJob;}



 protected:

  // Send command (and, optionally, parameters) to all TKFEC, FEC, or FED crates
  void commandToAllTKFECCrates( std::string command, Attribute_Vector parameters = Attribute_Vector(0) );
  void commandToAllFECCrates( std::string command, Attribute_Vector parameters = Attribute_Vector(0) );
  void commandToAllFEDCrates( std::string command, Attribute_Vector parameters = Attribute_Vector(0) );
  void commandToAllFEDChannels( std::string command );

  //Send ROCReset to all TTC supervisors
  void sendTTCROCReset();

  //Send CalSync to all TTC supervisors
  void sendTTCCalSync();

  //Send TBMReset to all TTC supervisors
  void sendTTCTBMReset();

  //Send CalSync to all LTC supervisor
  int sendLTCCalSync(unsigned int nTriggers);

  //Goto next FEC configuration
  void nextFECConfig(unsigned int event);

  //Send enableFIFO3 to all FEDs
  void enableFIFO3();

  //Send read data to all FEDs
  void readData(unsigned int event);

  //Event counter, incremented in runEvent after call to execute
  unsigned int event_;
  
  //Decide whether to proceed to the next event
  bool nextEvent_;

  // Progress report printed to stream out, every time another fraction howOften is completed.
  // If NTriggersTotal == -1, this assumes that the runEvent function will be run theCalibObject->nTriggersTotal() times.
  void reportProgress( double howOften = 0.05, std::ostream& out = std::cout, int NTriggersTotal = -1 );
  unsigned int eventNumberOfLastReportedProgress_;

  //Set dac for the specified ROC
  void setDAC(pos::PixelROCName aROC,unsigned int dacAddress,
	      unsigned int dac);

  double readIana(std::string dpeName);

  double readIdigi(std::string dpeName);

  double readCAEN(std::string dpeName);

  virtual void sendToFED(std::string& action, Attribute_Vector parametersToFED = Attribute_Vector(0));
  virtual void sendToFEC(std::string& action, Attribute_Vector parameters = Attribute_Vector(0));
  virtual void sendToTKFEC(std::string& action, Attribute_Vector parameters = Attribute_Vector(0));
  
  std::string sendingMode_;

  PixelTimer ttcTimer_;
  
 private:

  //loop over the FED supervisors and send beginJob soap request
  void sendBeginCalibrationToFEDs();
  void sendEndCalibrationToFEDs();

  // PixelCalibrationBase Constructor
  PixelCalibrationBase(const SOAPCommander& soapCommander);

  PixelCalibrationBase();

  double percentageOfJob_;

  bool resetROC_;

};

#endif
