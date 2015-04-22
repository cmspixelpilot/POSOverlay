#include "PixelDCSInterface/include/PixelDCStoFEDDpInterface.h"

/*************************************************************************
 * Interface class to convert last DAC temperature values                *
 * read-out via the PixelFEDSupervisor to physical units,                *
 * calibrate and average them,                                           *
 * and send calibrated averages to PSX server/PVSS,                      *
 * provided the calibrated average exceeds a configureable dead-band.    *
 *                                                                       *
 * Author: Christian Veelken, UC Davis					 *
 *                                                                       *
 * Last update: $Date: 2008/02/01 09:24:15 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#include <time.h>
#include <sstream>

#include "cgicc/HTMLClasses.h"

#include "PixelDCSInterface/include/PixelDCStoFEDDpNameTable.h"
#include "PixelDCSInterface/include/PixelDCStoFEDDpValueTable.h"
#include "PixelDCSInterface/include/PixelDCStoFEDDpFilterTable.h"
#include "PixelDCSInterface/include/PixelDCStoFEDDpCalibrationTable.h"
#include "PixelDCSInterface/include/PixelDCStoFEDDpFilterRow.h"

#include "CalibFormats/SiPixelObjects/interface/PixelConfigKey.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"

#include "PixelConfigDBInterface/include/PixelConfigInterface.h"

#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSSOAPCommander.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSPVSSCommander.h"

#define PSX_NS_URI "http://xdaq.cern.ch/xdaq/xsd/2006/psx-pvss-10.xsd"

XDAQ_INSTANTIATOR_IMPL(PixelDCStoFEDDpInterface)

enum { kProg_DACs_set, kProg_DACs_increase, kProg_DACs_decrease };

//--- declare auxiliary functions
std::list<pos::PixelROCName> getROCs(std::list<const PixelDCStoFEDDpFilterRow*>& dpList,
				     const pos::PixelNameTranslation* nameTranslation);
void setTempRange(std::map<pos::PixelROCName, unsigned int>& dacValues_set, std::map<pos::PixelROCName, unsigned int>& currentTempRangeSettings, 
		  const std::list<pos::PixelROCName>& readOutChips, int mode);

//
//---------------------------------------------------------------------------------------------------
//

PixelDCStoFEDDpInterface::PixelDCStoFEDDpInterface(xdaq::ApplicationStub* s) throw (xdaq::exception::Exception) 
  : PixelDCSDpInterface(s)
{
//--- constructor 
//    for XDAQ web applications

//--- bind SOAP Callbacks to Finite State Machine Commmands
  xoap::bind(this, &PixelDCStoFEDDpInterface::Configure, "Configure", XDAQ_NS_URI);
  xoap::bind(this, &PixelDCStoFEDDpInterface::Halt, "Halt", XDAQ_NS_URI);

//--- bind SOAP Callbacks to Data-Point Information received from the FEC or the FED
  xoap::bind(this, &PixelDCStoFEDDpInterface::updateDpValueLastDAC, "updateDpValueLastDAC", XDAQ_NS_URI);

//--- bind XGI Callbacks for messages from the browser
  xgi::bind(this, &PixelDCStoFEDDpInterface::Default, "Default");
  xgi::bind(this, &PixelDCStoFEDDpInterface::XgiHandler, "XgiHandler");

  httpPageHeader_ = "Pixel DCS to FED Interface";

//--- initialize tables holding data-point configuration information 
//    loaded from Oracle data-base
//    (in Configure method)
  dpNameTable_ = NULL;
  dpValueTable_ = NULL;
  dpFilterTable_ = NULL;
  dpCalibrationTable_ = NULL;

  dcsSOAPCommander_ = NULL;
  dcsPVSSCommander_ = NULL;

  updateCalibratedTemperature_ = TRUE;
  updateRawADC_ = TRUE;
  updateTempRangeDAC_ = TRUE;
}

PixelDCStoFEDDpInterface::~PixelDCStoFEDDpInterface()
{
//--- destructor
//    (delete tables holding data-point configuration 
//     information loaded from Oracle data-base)

  delete dpNameTable_;
  delete dpValueTable_;
  delete dpFilterTable_;
  delete dpCalibrationTable_;
/*
//--- delete PixelDCStoFEDDpConnectionRow objects
//    stored in dpConnectionTable_
//    (not neccessary, as dpConnectionTable_ only holds pointers to objects in dpConnectionList_)
  for ( std::map<unsigned int, std::map<unsigned int, std::map<unsigned int, PixelDCStoFEDDpConnectionRow> > >::const_iterator itBoard = dpConnectionTable_.begin();
	itBoard != dpConnectionTable_.end(); ++itBoard ) {
    for ( std::map<unsigned int, std::map<unsigned int, PixelDCStoFEDDpConnectionRow> >::const_iterator itChannel = itBoard->second.begin();
	  itChannel != itBoard->second.end(); ++itChannel ) {
      for ( std::map<unsigned int, PixelDCStoFEDDpConnectionRow>::const_iterator itROC = itChannel->second.begin();
	    itROC != itChannel->second.end(); ++itROC ) {
	delete itROC->second;
      }
    }
  }
 */

  delete dcsSOAPCommander_;
  delete dcsPVSSCommander_;
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelDCStoFEDDpInterface::Default(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception)
{
//--- render the Finite State Machine GUI
//    (including input field for Oracle password)

  PixelDCSDpInterface::Default(in, out);

//--- render the Low Level GUI
//    (show status of all data-points)

  *out<<"<h2> Data-Point Status </h2>";
  
  *out << cgicc::table().set("border","2");
  //*out << cgicc::table().set("border cellpadding","10").set("cellspacing","0");
  *out << cgicc::tr() << cgicc::th().set("colspan", "2").set("align", "center") << "FED" << cgicc::th() 
       << cgicc::th().set("rowspan", "2").set("align", "center") << "ROC Id" << cgicc::th() 
       << cgicc::th().set("rowspan", "2").set("align", "center") << "Logical Name" << cgicc::th() 
       << cgicc::th().set("colspan", "2").set("align", "center") << "Data-Point" << cgicc::th() 
       << cgicc::th().set("rowspan", "2").set("align", "center") << "Last Update" << cgicc::th() 
       << cgicc::th().set("colspan", "4").set("align", "center") << "Current Status" << cgicc::th() 
       << cgicc::th().set("colspan", "3").set("align", "center") << "Statistics" << cgicc::th() 
       << cgicc::tr() << std::endl;
  *out << cgicc::tr() << cgicc::td() << "BoardId" << cgicc::td() << cgicc::td() << "ChannelId" << cgicc::td() 
       << cgicc::td() << "Name" << cgicc::td() << cgicc::td() << "Value" << cgicc::td() 
       << cgicc::td() << "N" << cgicc::td() << cgicc::td() << "Av" << cgicc::td() 
       << cgicc::td() << "ADC" << cgicc::td() << cgicc::td() << "TR" << cgicc::td() 
       << cgicc::td() << "Tot" << cgicc::td() << cgicc::td() << "Acc" << cgicc::td() << cgicc::td() << "Rej" << cgicc::td() 
       << cgicc::tr() << std::endl;

  for ( std::list<PixelDCStoFEDDpConnectionRow>::iterator dpConnection = dpConnectionList_.begin();
	dpConnection != dpConnectionList_.end(); ++dpConnection ) {
    int fedBoardId = dpConnection->getBoardId();
    int fedChannelId = dpConnection->getChannelId();
    int rocId = dpConnection->getRocId();
    unsigned long numDpAccepted = dpConnection->getNumDpAccepted();
    unsigned long numDpRejected = dpConnection->getNumDpRejected();
    unsigned long numDpTotal = numDpAccepted + numDpRejected;

    const PixelDCStoFEDDpNameRow& dpNameRow = dpNameTable_->getRow(fedBoardId, fedChannelId, rocId);
    const std::string& logicalName = dpNameRow.getLogicalName();
    const std::string& dpName = dpNameRow.getDpName();
    
    PixelDCStoFEDDpValueRow& dpValueRow = dpValueTable_->getRow(fedBoardId, fedChannelId, rocId);
    float lastDpValue = dpValueRow.getLastDpValue();
    time_t lastDpUpdate = dpValueRow.getLastDpUpdate();
    unsigned int numDp = dpValueRow.getNumDp();
    float dpValue = dpValueRow.getDpValue();
    float adcCount = dpValueRow.getADCCount();
    float currentTempRangeDAC = dpValueRow.getCurrentTempRangeDAC();
    
    struct tm* lastDpUpdate_struct = localtime(&lastDpUpdate);
    std::string lastDpUpdate_string = asctime(lastDpUpdate_struct);
    
    *out << cgicc::tr() << cgicc::td() << fedBoardId << cgicc::td() << cgicc::td() << fedChannelId << cgicc::td() 
	 << cgicc::td() << rocId << cgicc::td() 
	 << cgicc::td() << logicalName << cgicc::td() 
	 << cgicc::td() << dpName << cgicc::td() << cgicc::td() << lastDpValue << cgicc::td() 
	 << cgicc::td() << lastDpUpdate_string << cgicc::td() 
	 << cgicc::td() << numDp << cgicc::td() << cgicc::td() << dpValue << cgicc::td() 
	 << cgicc::td() << adcCount << cgicc::td() << cgicc::td() << currentTempRangeDAC << cgicc::td() 
	 << cgicc::td() << numDpTotal << cgicc::td() << cgicc::td() << numDpAccepted << cgicc::td() << cgicc::td() << numDpRejected << cgicc::td() 
	 << cgicc::tr() << std::endl;
  }

  *out << cgicc::table();
  
  *out<<"</html>";
}

void PixelDCStoFEDDpInterface::XgiHandler(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception)
{
  PixelDCSDpInterface::XgiHandler(in, out);
}

//
//---------------------------------------------------------------------------------------------------
//

xoap::MessageReference PixelDCStoFEDDpInterface::Configure(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
//--- retrieve global configuration key;
//    initialize name translation and pixel FEC configuration objects
  Attribute_Vector parameters(1);
  parameters.at(0).name_ = "GlobalKey";
  Receive(msg, parameters);
  unsigned int globalKey = atoi(parameters.at(0).value_.c_str());

  std::cout << "<PixelDCStoFEDDpInterface::Configure>:" << std::endl;
  std::cout << " GlobalKey = " << globalKey << std::endl;
  
  pos::PixelConfigKey* globalConfigKey = new pos::PixelConfigKey(globalKey);
  if ( globalConfigKey == NULL ) {
    std::cerr << "Error in <PixelDCStoFEDDpInterface::Configure>: failed to access global Configuration Key !!!" << std::endl;
    XCEPT_RAISE (xdaq::exception::Exception, "Failed to access global Configuration Key !!!");
  }
  
  PixelConfigInterface::get(nameTranslation_, "pixel/nametranslation/", *globalConfigKey);
  if ( nameTranslation_ == NULL ) {
    std::cerr << "Error in <PixelDCStoFEDDpInterface::Configure>: failed to access Pixel name translation !!!" << std::endl;
    XCEPT_RAISE (xdaq::exception::Exception, "Failed to access Pixel name translation !!!");
  }
  
  PixelConfigInterface::get(fecConfiguration_, "pixel/fecconfig/", *globalConfigKey);
  if ( fecConfiguration_ == NULL ) {
    std::cerr << "Error in <PixelDCStoFEDDpInterface::Configure>: failed to access pxlFEC configuration !!!" << std::endl;
    XCEPT_RAISE (xdaq::exception::Exception, "Failed to access pxlFEC configuration !!!");
  }

//--- initialize list of TempRange DAC settings currently loaded into the Read-Out Chips
  pos::PixelDetectorConfig* detectorConfiguration;
  PixelConfigInterface::get(detectorConfiguration, "pixel/detconfig/", *globalConfigKey);
  if ( detectorConfiguration == NULL ) {
    std::cerr << "Error in <PixelDCStoFEDDpInterface::Configure>: failed to access Detector configuration !!!" << std::endl;
    XCEPT_RAISE (xdaq::exception::Exception, "Failed to access Detector configuration !!!");
  }

  std::vector<pos::PixelModuleName> modules = detectorConfiguration->getModuleList();
  std::list<const pos::PixelROCName*> readOutChips = nameTranslation_->getROCs();
  for ( std::vector<pos::PixelModuleName>::const_iterator module = modules.begin();
	module != modules.end(); ++module ) {
    const std::string& moduleName = module->modulename();
    const std::string& dacSettings_path = moduleName;

//--- load DAC settings of module
    pos::PixelDACSettings* dacSettings = NULL;
    PixelConfigInterface::get(dacSettings, "pixel/dac/" + dacSettings_path, *globalConfigKey);

    if ( dacSettings != NULL ) {
//--- find Read-Out Chips associated to module
      for ( std::list<const pos::PixelROCName*>::const_iterator readOutChip = readOutChips.begin();
	    readOutChip != readOutChips.end(); ++readOutChip ) {
	if ( *readOutChip != NULL ) {
//--- get current TempRange DAC setting of Read-Out Chip
	  pos::PixelROCDACSettings* dacSettings_readOutChip = dacSettings->getDACSettings(**readOutChip);
	  if ( dacSettings_readOutChip != NULL ) {
	    currentTempRangeSettings_[**readOutChip] = (unsigned int)dacSettings_readOutChip->getTempRange();
	  }
	}
      }
    }
  }

//--- retrieve password
//    for access to Oracle data-base
  std::cout << "<PixelDCStoFEDDpInterface::Configure>:" << std::endl;
  std::cout << "  oraclePassword = " << oraclePassword_dpNames_ << std::endl;

//--- open connection to Oracle data-base;
//    access Oracle data-base to initialize 
//     o PixelDCStoFEDDpNameTable
//     o PixelDCStoFEDDpFilterTable
//     o PixelDCStoFEDDpCalibrationTable;
//    close connection to Oracle data-base
  connectOracleDB(oracleViewName_dpNames_.toString(), oraclePassword_dpNames_);
  xdata::Table oracleDpNameTable = getOracleTable(oracleViewName_dpNames_.toString(), oracleTableName_dpNames_.toString());
  dpNameTable_ = new PixelDCStoFEDDpNameTable(oracleDpNameTable);
  dpValueTable_ = new PixelDCStoFEDDpValueTable(oracleDpNameTable);
  disconnectOracleDB(oracleViewName_dpNames_.toString());
  
  connectOracleDB(oracleViewName_dpFilter_.toString(), oraclePassword_dpFilter_);
  xdata::Table oracleDpFilterTable = getOracleTable(oracleViewName_dpFilter_.toString(), oracleTableName_dpFilter_.toString());
  dpFilterTable_ = new PixelDCStoFEDDpFilterTable(oracleDpFilterTable);
  disconnectOracleDB(oracleViewName_dpFilter_.toString());

  connectOracleDB(oracleViewName_dpCalibration_.toString(), oraclePassword_dpCalibration_);
  xdata::Table oracleDpCalibrationTable = getOracleTable(oracleViewName_dpCalibration_.toString(), oracleTableName_dpCalibration_.toString());
  dpCalibrationTable_ = new PixelDCStoFEDDpCalibrationTable(oracleDpCalibrationTable);
  disconnectOracleDB(oracleViewName_dpCalibration_.toString());

//--- check that number of rows in PixelDCStoFEDDpNameTable, PixelDCStoFEDDpFilterTable and PixelDCStoFEDCalibrationTable are equal
//    and that the fedBoardId, fedChannelId, rocId contained in all tables match
//    (not yet implemented)

//--- update Finite State Machine state
  try {
    toolbox::Event::Reference e(new toolbox::Event("Configure", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    XCEPT_RETHROW(xoap::exception::Exception, "Invalid State Machine Input.", e);
  }

  xoap::MessageReference reply = MakeSOAPMessageReference("ConfigureDone");
  return reply;
}

xoap::MessageReference PixelDCStoFEDDpInterface::Halt(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
/*
    Delete and clear all your objects here.
 */
	
  try {
    toolbox::Event::Reference e(new toolbox::Event("Halt", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    XCEPT_RETHROW(xoap::exception::Exception, "Invalid Command.", e);
  }
  
  xoap::MessageReference response = MakeSOAPMessageReference("HaltDone");
  return response;
}

//
//---------------------------------------------------------------------------------------------------
//

xoap::MessageReference PixelDCStoFEDDpInterface::updateDpValueLastDAC(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
//--- call-back funtion triggered by temperature updates coming from FED;
//    decode incoming SOAP message and send response to FED Supervisor

  std::cout << "<PixelDCStoFEDDpInterface::updateDpValueLastDAC>" << std::endl;

//--- return if FSM state of DCS to FED Interface is not "Configured"
//    (it makes no sense to try processing data-point values 
//     until calibration has been loaded from Oracle data-base)
  toolbox::fsm::State fsmState = fsm_.getCurrentState();
  if ( fsmState != 'C' ) {
    return MakeSOAPMessageReference("Warning: Data-Point Update failed because PixelDCStoFEDDpInterface not configured yet");
  }
	
//--- decode incoming SOAP message;
//    calibrate and update data-points
  decodeDpValueUpdate(msg);
			
//--- send response to FED Supervisor
  xoap::MessageReference fedResponse = composeDpValueUpdateResponseFED();
//
//--- WARNING: program aborts/crashes 
//             if wrong instance value is used 
//             in getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor("PixelFEDSupervisor", 1) function call ?!
//
  xdaq::ApplicationDescriptor* fedDescriptor = getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor("PixelFEDSupervisor", 1);

  std::cout << "Sending SOAP response to FED Supervisor; URL = " << fedDescriptor->getContextDescriptor()->getURL() << std::endl;
  std::cout << " Response : ------------------------------------ "<< std::endl;
  fedResponse->writeTo(std::cout);
  std::cout << std::endl;
  std::cout << " ---------------------------------------------- "<< std::endl;

  getApplicationContext()->postSOAP(fedResponse, fedDescriptor);

//--- compose SOAP message containing update of calibrated data-point values
//    and send it to PVSS (via PSX interface)
  xoap::MessageReference psxRequest = composeDpValueUpdateRequestPSX();

  xdaq::ApplicationDescriptor* psxDescriptor = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor("psx", 0);

  std::cout << "Sending SOAP request to PSX Server; URL = " << psxDescriptor->getContextDescriptor()->getURL() << std::endl;
  std::cout << " Request : ------------------------------------ "<< std::endl;
  psxRequest->writeTo(std::cout);
  std::cout << std::endl;
  std::cout << " ---------------------------------------------- "<< std::endl;

  xoap::MessageReference psxResponse = getApplicationContext()->postSOAP(psxRequest, psxDescriptor);
  
//--- print SOAP reply
  std::cout <<" Reply : -------------------------------------- "<< std::endl;
  psxResponse->writeTo(std::cout);
  std::cout << std::endl;
  std::cout <<" ---------------------------------------------- "<< std::endl;

  
//--- compose SOAP message requesting FEC Supervisor to switch TempRange DAC if neccessary
//    (last DAC ADC value either close to black-level or saturated)
  xoap::MessageReference fecRequest = composeTempRangeSwitchRequestFEC();

  xdaq::ApplicationDescriptor* fecDescriptor = getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor("PixelFECSupervisor", 1);
  
  std::cout << "Sending SOAP request to Pixel FEC Supervisor; URL = " << fecDescriptor->getContextDescriptor()->getURL() << std::endl;
  std::cout << " Request : ------------------------------------ "<< std::endl;
  fecRequest->writeTo(std::cout);
  std::cout << std::endl;
  std::cout << " ---------------------------------------------- "<< std::endl;
  
  xoap::MessageReference fecResponse = getApplicationContext()->postSOAP(fecRequest, fecDescriptor);
  
//--- print SOAP reply
  std::cout <<" Reply : -------------------------------------- "<< std::endl;
  fecResponse->writeTo(std::cout);
  std::cout << std::endl;
  std::cout <<" ---------------------------------------------- "<< std::endl;

  return MakeSOAPMessageReference("updateDpValueLastDACDone");
}

void PixelDCStoFEDDpInterface::decodeDpValueUpdate(xoap::MessageReference msg)
{
//--- reset lists of data-points for which the ADC count is either too low or too high,
//    such that the FED Supervisor needs to be instructed to change the TempRange DAC 
  dpList_increaseTempRangeDAC_.clear();
  dpList_decreaseTempRangeDAC_.clear();

//--- begin unpacking SOAP message
//    received from FED Supervisor
  xoap::SOAPEnvelope envelope = msg->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();

//--- create SOAPName objects 
//    for keywords searched for in SOAP message
  xoap::SOAPName command = envelope.createName("updateDpValueLastDAC");
  xoap::SOAPName fedBoard = envelope.createName("fedBoard");
  xoap::SOAPName dp = envelope.createName("dp");
  xoap::SOAPName fedBoardIdAttribute = envelope.createName("number");
  xoap::SOAPName fedChannelIdAttribute = envelope.createName("fedChannel");
  xoap::SOAPName rocIdAttribute = envelope.createName("roc");

//--- find within body 
//    response to "updateDpValueLastDAC" command
  std::vector< xoap::SOAPElement > bodyElements = body.getChildElements(command);          	  
  for ( std::vector< xoap::SOAPElement >::iterator bodyElement = bodyElements.begin();
	bodyElement != bodyElements.end(); ++bodyElement ) {
    
//--- find within "updateDpValueLastDAC" command response
//    lists of "fedBoard" identifiers
    std::vector<xoap::SOAPElement> boardElements = bodyElement->getChildElements(fedBoard); 
    for ( std::vector< xoap::SOAPElement >::iterator boardElement = boardElements.begin();
	  boardElement != boardElements.end(); ++boardElement ) {

//--- unpack fedBoardId
//    associated to "fedBoard" identifier
      int fedBoardId = atoi(boardElement->getAttributeValue(fedBoardIdAttribute).data());

//--- find within "fedBoard" list
//    updates of "dp" data-point information
      std::vector< xoap::SOAPElement > dpElements = boardElement->getChildElements(dp); 
      for ( std::vector< xoap::SOAPElement >::iterator dpElement = dpElements.begin();
	    dpElement != dpElements.end(); ++dpElement ) {
      
//--- unpack fedChannelId, rocId and actual value	  
//    from data-point information
	int fedChannelId = atoi(dpElement->getAttributeValue(fedChannelIdAttribute).data());
	int rocId = atoi(dpElement->getAttributeValue(rocIdAttribute).data());
	int adcCount = atoi(dpElement->getValue().data());
	
	pos::PixelROCName readOutChipName = nameTranslation_->ROCNameFromFEDChannelROC(fedBoardId, fedChannelId, rocId);
	std::map<pos::PixelROCName, unsigned int>::const_iterator currentTempRangeSetting_ptr = currentTempRangeSettings_.find(readOutChipName);
	if ( currentTempRangeSetting_ptr == currentTempRangeSettings_.end() ) {
	  std::cerr << "Warning in <PixelDCStoFEDDpInterface::decodeDpValueUpdate>:" 
		    << " failed to access current TempRange DAC setting for Read-Out Chip name = " << readOutChipName << " !!!" << std::endl;
	  continue;
	}
	
	unsigned int currentTempRangeSetting = currentTempRangeSetting_ptr->second;
      
	std::cout << " FED Board = " << fedBoardId << std::endl;
	std::cout << " FED Channel = " << fedChannelId << std::endl;
	std::cout << " ROC = " << rocId << std::endl;
	std::cout << " ADC Count = " << adcCount << std::endl;
	std::cout << " TempRange DAC = " << currentTempRangeSetting << std::endl;

//--- add PixelDCStoFEDDpConnectionRow object into dpConnectionList_ 
//    if data-point is the first data-point received for that channel	
	if ( dpConnectionTable_[fedBoardId][fedChannelId][rocId] == NULL ) {
	  dpConnectionList_.push_back(PixelDCStoFEDDpConnectionRow(fedBoardId, fedChannelId, rocId));
	  dpConnectionTable_[fedBoardId][fedChannelId][rocId] = &(dpConnectionList_.back());
	}
	
	PixelDCStoFEDDpConnectionRow* dpConnection = dpConnectionTable_[fedBoardId][fedChannelId][rocId];

//--- check that adcCounts are not within a region 
//    in which either noise is dominant or the ADC is saturated
	const PixelDCStoFEDDpFilterRow& dpFilterRow = dpFilterTable_->getRow(fedBoardId, fedChannelId, rocId);
	if ( dpFilterRow.isWithinLimits(adcCount) ) {

//--- convert adcCounts into physical units 
//    and apply calibration
	  const PixelDCStoFEDDpCalibrationRow& dpCalibrationRow = dpCalibrationTable_->getRow(fedBoardId, fedChannelId, rocId);
	  float dpValue = dpCalibrationRow.getCalibratedValue(adcCount, currentTempRangeSetting);

//--- include calibrated measurement 
//    into calculation of averages
	  PixelDCStoFEDDpValueRow& dpValueRow = dpValueTable_->getRow(fedBoardId, fedChannelId, rocId);
	  dpValueRow.addValue(dpValue, adcCount);
	  dpValueRow.setCurrentTempRangeDAC(currentTempRangeSetting);

//--- update "connection" statistics 
//    for channel specified by fedBoardId, fedChannelId, rocId
//    (increment number of "accepted" data-points by one)
	  dpConnection->incDpAccepted();
	} else {

//--- adcCounts are either within a region in which either noise is dominant (ADC count too low)
//    or within a region in which the ADC is saturated (ADC count too high);
//    instruct FED Supervisor to increase (decrease) the TempRange DAC in case the ADC count is too low (high) 
	  if ( dpFilterRow.isAboveUpperLimit(adcCount) ) dpList_increaseTempRangeDAC_.push_back(&dpFilterRow);
	  if ( dpFilterRow.isBelowLowerLimit(adcCount) ) dpList_decreaseTempRangeDAC_.push_back(&dpFilterRow);

//--- update "connection" statistics 
//    for channel specified by fedBoardId, fedChannelId, rocId
//    (increment number of "rejected" data-points by one)
	  dpConnection->incDpRejected();
	}
      }
    }
  }  
}

xoap::MessageReference PixelDCStoFEDDpInterface::composeDpValueUpdateResponseFED()
{
//--- compose the message send to the FED Supervisor
//    as response to data-point updates

  xoap::MessageReference message = xoap::createMessage();
  xoap::SOAPEnvelope envelope = message->getSOAPPart().getEnvelope();
  xoap::SOAPName command = envelope.createName("updateDpValueLastDACResponse", "xdaq", XDAQ_NS_URI);
  xoap::SOAPBody body = envelope.getBody();
  body.addBodyElement(command);

  return message;
}

xoap::MessageReference PixelDCStoFEDDpInterface::composeDpValueUpdateRequestPSX()
{
//--- compose the message send to the PSX server,
//    requesting an update of data-point values in PVSS

//--- compute for all data-points 
//    priority of updating dpValue in PVSS

  for ( std::list<PixelDCStoFEDDpConnectionRow>::const_iterator dpConnection = dpConnectionList_.begin();
	dpConnection != dpConnectionList_.end(); ++dpConnection ) {
    int fedBoardId = dpConnection->getBoardId();
    int fedChannelId = dpConnection->getChannelId();
    int rocId = dpConnection->getRocId();
    
    PixelDCStoFEDDpValueRow& dpValueRow = dpValueTable_->getRow(fedBoardId, fedChannelId, rocId);
    float lastDpValue = dpValueRow.getLastDpValue();
    time_t lastDpUpdate = dpValueRow.getLastDpUpdate();
    unsigned int numDp = dpValueRow.getNumDp();
    float dpValue = dpValueRow.getDpValue();
    
    time_t currentTime = time(NULL);
    
    const PixelDCStoFEDDpFilterRow& dpFilterRow = dpFilterTable_->getRow(fedBoardId, fedChannelId, rocId);
    bool isWithinDeadband = dpFilterRow.isWithinDeadband(dpValue, lastDpValue);
    
    dpConnection->setDpUpdatePriority(dpValue, lastDpValue, isWithinDeadband, numDp, currentTime, lastDpUpdate);
  }

//--- produce sorted list of data-points
//    (data-points with highest priority are first in list)
  dpConnectionList_.sort();

//--- create SOAP message
/*
  xoap::MessageReference message = xoap::createMessage();
  xoap::SOAPEnvelope envelope = message->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName commandElement = envelope.createName("dpSet", "psx", PSX_NS_URI);
  xoap::SOAPBodyElement bodyElement = body.addBodyElement(commandElement);

//--- add data-point values to SOAP message 
//    (in order of decreasing priority);
//    stop adding data-points when either
//     o data-point values changed less than dead-band or
//     o maximum length of SOAP message reached
//    (in the latter case a "busy" flag is set in the data-point 
//     describing the state of the DCS to FED Interface)
  std::list<PixelDCStoFEDDpConnectionRow>::iterator dpConnection = dpConnectionList_.begin();
  int numDpUpdated = 0;
  while ( dpConnection != dpConnectionList_.end() && dpConnection->getDpUpdatePriority() > 1.e-3 && numDpUpdated <= dpValueUpdate_maxLength_ ) {
    int fedBoardId = dpConnection->getBoardId();
    int fedChannelId = dpConnection->getChannelId();
    int rocId = dpConnection->getRocId();
    
    const PixelDCStoFEDDpNameRow& dpNameRow = dpNameTable_->getRow(fedBoardId, fedChannelId, rocId);
    const std::string& dpName = dpNameRow.getDpName();

    PixelDCStoFEDDpValueRow& dpValueRow = dpValueTable_->getRow(fedBoardId, fedChannelId, rocId);
    float dpValue = dpValueRow.getDpValue();
    std::ostringstream dpValue_string;
    dpValue_string << dpValue;
    
    const std::string dpeSuffix_get = ".value:online..value";
    
    xoap::SOAPName dpElement = envelope.createName("dp");
    xoap::SOAPElement childElement = bodyElement.addChildElement(dpElement);
    xoap::SOAPName nameElement = envelope.createName("name");
    childElement.addAttribute(nameElement, dpName + dpeSuffix_get);
    childElement.addTextNode(dpValue_string.str());
	
    dpValueRow.reset();
    
    ++dpConnection;
    ++numDpUpdated;
  }
 */

  std::list<PixelDCStoFEDDpConnectionRow>::iterator dpConnection = dpConnectionList_.begin();
  int numDpUpdated = 0;
  std::list<PixelDCSPVSSDpe> dpeList;
  while ( dpConnection != dpConnectionList_.end() && dpConnection->getDpUpdatePriority() > 1.e-3 && numDpUpdated <= dpValueUpdate_maxLength_ ) {
    int fedBoardId = dpConnection->getBoardId();
    int fedChannelId = dpConnection->getChannelId();
    int rocId = dpConnection->getRocId();

    const PixelDCStoFEDDpNameRow& dpNameRow = dpNameTable_->getRow(fedBoardId, fedChannelId, rocId);
    const std::string& dpName = dpNameRow.getDpName();

    PixelDCStoFEDDpValueRow& dpValueRow = dpValueTable_->getRow(fedBoardId, fedChannelId, rocId);

    if ( updateCalibratedTemperature_ ) {
      float calibratedTemperature = dpValueRow.getDpValue();
      const std::string dpeSuffix_calibratedTemperature = ".readings.calibratedTemperature";
      std::string dpeName = dpName + dpeSuffix_calibratedTemperature;
      PixelDCSPVSSDpe dpe(dpeName, calibratedTemperature);
      dpeList.push_back(dpe);
    }

    if ( updateRawADC_ ) {
      float adcCount = dpValueRow.getADCCount();
      const std::string dpeSuffix_rawADC = ".readings.rawADC";
      std::string dpeName = dpName + dpeSuffix_rawADC;
      PixelDCSPVSSDpe dpe(dpeName, adcCount);
      dpeList.push_back(dpe);
    }

    if ( updateTempRangeDAC_ ) {
      unsigned int currentTempRangeDAC = dpValueRow.getCurrentTempRangeDAC();
      const std::string dpeSuffix_currentTempRangeDAC = ".readings.currentTempRangeDAC";
      std::string dpeName = dpName + dpeSuffix_currentTempRangeDAC;
      PixelDCSPVSSDpe dpe(dpeName, currentTempRangeDAC);
      dpeList.push_back(dpe);
    }
	
    dpValueRow.reset();
    
    ++dpConnection;
    ++numDpUpdated;
  }

//--- add data-point describing state of the DCS to FED Interface
//    (dpRate and dpLoad; heart-beat; busy and error flags)
  status_.dpRate_ = numDpUpdated;
  status_.dpLoad_ = numDpUpdated/(float)dpValueUpdate_maxLength_;
  status_.heartbeat_ = (!status_.heartbeat_);
  status_.busy_ = (numDpUpdated == dpValueUpdate_maxLength_);
  status_.error_ = false; // not yet implemented
  addStatus(status_, dpName_status_, dpeList);
  
  if ( dcsPVSSCommander_ == NULL ) {
    xdaq::ApplicationDescriptor* psxDescriptor = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor("psx", 0);
    dcsPVSSCommander_ = new PixelDCSPVSSCommander(this, psxDescriptor);
  }

  xoap::MessageReference message = dcsPVSSCommander_->MakeSOAPMessageReference_dpSet(dpeList);
 
  return message;
}

xoap::MessageReference PixelDCStoFEDDpInterface::composeTempRangeSwitchRequestFEC()
{
//--- compose the message send to the FEC Supervisor
//    requesting to change the value of the TempRange DAC
//    for ROCs for which the last DAC ADC reading 
//    is too high or too low

  //std::map<pos::PixelROCName, unsigned int> dacValues_dummy;
  std::list<pos::PixelROCName> dacValues_dummy;

//--- add list of data-points for which the ADC reading is too **high**
//    such that the TempRange DAC needs to be **increased**
  std::list<pos::PixelROCName> dacValues_increase = getROCs(dpList_increaseTempRangeDAC_, nameTranslation_);

//--- add list of data-points for which the ADC reading is too **low**
//    such that the TempRange DAC needs to be **decreased**
  std::list<pos::PixelROCName> dacValues_decrease = getROCs(dpList_decreaseTempRangeDAC_, nameTranslation_);
  
  std::map<pos::PixelROCName, unsigned int> dacValues_set;
  setTempRange(dacValues_set, currentTempRangeSettings_, dacValues_increase, kProg_DACs_increase);
  setTempRange(dacValues_set, currentTempRangeSettings_, dacValues_decrease, kProg_DACs_decrease);

  if ( dcsSOAPCommander_ == NULL ) {
    dcsSOAPCommander_ = new PixelDCSSOAPCommander(this);
  }

  xoap::MessageReference soapRequest 
  //  = dcsSOAPCommander_->MakeSOAPMessageReference_progDAC(pos::k_DACAddress_TempRange, dacValues_dummy, dacValues_increase, dacValues_decrease, 
    = dcsSOAPCommander_->MakeSOAPMessageReference_progDAC(pos::k_DACAddress_TempRange, dacValues_set, dacValues_dummy, dacValues_dummy, 
							  nameTranslation_, fecConfiguration_);

  return soapRequest;
}

//
//---------------------------------------------------------------------------------------------------
//

std::list<pos::PixelROCName> getROCs(std::list<const PixelDCStoFEDDpFilterRow*>& dpList,
				     const pos::PixelNameTranslation* nameTranslation)
{
  std::list<pos::PixelROCName> readOutChips;

  for ( std::list<const PixelDCStoFEDDpFilterRow*>::const_iterator p_dp = dpList.begin();
	p_dp != dpList.end(); ++p_dp ) {
    unsigned int fedBoardId = (*p_dp)->getBoardId();
    unsigned int fedChannelId = (*p_dp)->getChannelId();
    unsigned int rocId = (*p_dp)->getRocId();

    pos::PixelROCName readOutChipName = nameTranslation->ROCNameFromFEDChannelROC(fedBoardId, fedChannelId, rocId);

    readOutChips.push_back(readOutChipName);
  }

  return readOutChips;
}

void setTempRange(std::map<pos::PixelROCName, unsigned int>& dacValues_set, std::map<pos::PixelROCName, unsigned int>& currentTempRangeSettings, 
		  const std::list<pos::PixelROCName>& readOutChips, int mode)
{
  if ( !(mode == kProg_DACs_increase ||
	 mode == kProg_DACs_decrease) ) {
    std::cerr << "Error in <setTempRange>: mode = " << mode << " not defined !!!" << std::endl;
    return;
  }

  for ( std::list<pos::PixelROCName>::const_iterator readOutChip = readOutChips.begin();
	readOutChip != readOutChips.end(); ++readOutChip ) {
    std::map<pos::PixelROCName, unsigned int>::const_iterator currentTempRangeSetting_ptr = currentTempRangeSettings.find(*readOutChip);
    if ( currentTempRangeSetting_ptr == currentTempRangeSettings.end() ) {
      std::cerr << "Warning in <setTempRange>:" 
		<< " failed to access current TempRange DAC setting for Read-Out Chip name = " << readOutChip->rocname() << " !!!" << std::endl;
      continue;
    }

    unsigned int currentTempRangeSetting = currentTempRangeSetting_ptr->second;
    
    const unsigned minTempRangeSetting = 0;
    const unsigned maxTempRangeSetting = 15;
    if ( mode == kProg_DACs_increase ) {
      if ( currentTempRangeSetting < maxTempRangeSetting ) ++currentTempRangeSetting;
    } else if ( mode == kProg_DACs_decrease ) {
      if ( currentTempRangeSetting > minTempRangeSetting ) --currentTempRangeSetting;
    } 
    
    currentTempRangeSettings[*readOutChip] = currentTempRangeSetting;
    
    dacValues_set.insert(std::pair<pos::PixelROCName, unsigned int>(*readOutChip, currentTempRangeSetting));
  }
}
