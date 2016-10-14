#include "PixelTemperatureCalibrationPluginLastDAC.h"

// $Id: PixelTemperatureCalibrationPluginLastDAC.cc,v 1.14 2009/09/17 10:21:06 kreis Exp $

/*************************************************************************
 * Class for Last DAC calibration routines,                              *
 * implemented as plug-in for PixelTemperatureCalibration class          *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2009/09/17 10:21:06 $ (UTC)                       *
 *          by: $Author: kreis $                                       *
 *************************************************************************/

#include "xercesc/dom/DOMNodeList.hpp"

#include "PixelConfigDBInterface/include/PixelConfigInterface.h"

#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"

#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"

//using namespace pos;

// declare global auxiliary functions
const std::map<pos::PixelROCName, unsigned int> makeReadOutChipDACs(const std::vector<pos::PixelROCName>& readOutChips, unsigned int dacValue);

const unsigned int defaultPixelConfigurationKey = 0;

//
//---------------------------------------------------------------------------------------------------
//

PixelTemperatureCalibrationPluginLastDAC::PixelTemperatureCalibrationPluginLastDAC(PixelSupervisorConfiguration* globalConfigurationParameters,
										   SOAPCommander* soapCommander, 
										   PixelDCSSOAPCommander* dcs_soapCommander, 
										   PixelDCSPVSSCommander* pvssCommander,
										   xercesc::DOMNode* pluginConfigNode)
  : PixelTemperatureCalibrationPlugin(globalConfigurationParameters, soapCommander, dcs_soapCommander, pvssCommander, pluginConfigNode)
{
  loadConfiguration();

//--- initialize list of Read-out Chips
  const pos::PixelNameTranslation* nameTranslation = globalConfigurationParameters_->getNameTranslation();
  const std::list<const pos::PixelROCName*>& readOutChips = nameTranslation->getROCs();
  for ( std::list<const pos::PixelROCName*>::const_iterator readOutChip = readOutChips.begin();
	readOutChip != readOutChips.end(); ++readOutChip ) {
    const pos::PixelHdwAddress* readOutChip_hardwareAddress = nameTranslation->getHdwAddress(**readOutChip);
    
    unsigned int fedBoardId = readOutChip_hardwareAddress->fednumber();
    unsigned int fedChannelId = readOutChip_hardwareAddress->fedchannel();
    unsigned int rocId = readOutChip_hardwareAddress->fedrocnumber();

    std::cout << "defining hardware Address of Read-out Chip " << (*readOutChip)->rocname() << " to be" 
	      << " fedBoardId = " << fedBoardId << ", fedChannelId = " << fedChannelId << ", rocId = " << rocId << std::endl;

    fedReadOutChipName_[fedBoardId][fedChannelId][rocId] = (*readOutChip);
  }
}
  
PixelTemperatureCalibrationPluginLastDAC::~PixelTemperatureCalibrationPluginLastDAC()
{
//--- nothing to be done yet  
}

void PixelTemperatureCalibrationPluginLastDAC::loadConfiguration() throw (xdaq::exception::Exception)
{
//--- read mapping between PVSS and XDAQ states and commands
//    for given FSM node type (logical or control unit)

  std::cout << "<PixelTemperatureCalibrationPluginLastDAC::loadConfiguration>:" << std::endl;

  std::string pluginType = xoap::getNodeAttribute(pluginConfigNode_, "type");
  if ( pluginType != "PixelTemperatureCalibrationPluginLastDAC" ) {
    XCEPT_RAISE (xcept::Exception, "Undefined Plug-in Type");
  }

  bool minTempRangeDAC_initialized = false;
  bool maxTempRangeDAC_initialized = false;
  bool stepSizeTempRangeDAC_initialized = false;
  bool numTriggers_initialized = false;

  DOMNodeList* parameterNodes = pluginConfigNode_->getChildNodes();
  unsigned int numParameterNodes = parameterNodes->getLength();
  for ( unsigned int iNode = 0; iNode < numParameterNodes; ++iNode ) {
    DOMNode* parameterNode = parameterNodes->item(iNode);
    
//--- skip empty lines
//    (not removed by DOM parser)
    if ( xoap::XMLCh2String(parameterNode->getLocalName()) == "" ) continue;

    if ( xoap::XMLCh2String(parameterNode->getPrefix())    == "plugin"    &&
	 xoap::XMLCh2String(parameterNode->getLocalName()) == "parameter" ) {
      if ( xoap::getNodeAttribute(parameterNode, "minTempRangeDAC") != ""  ) {
	minTempRangeDAC_ = atoi(xoap::getNodeAttribute(parameterNode, "minTempRangeDAC").data());
	minTempRangeDAC_initialized = true;
      } else if ( xoap::getNodeAttribute(parameterNode, "maxTempRangeDAC") != ""  ) {
	maxTempRangeDAC_ = atoi(xoap::getNodeAttribute(parameterNode, "maxTempRangeDAC").data());
	maxTempRangeDAC_initialized = true;
      } else if ( xoap::getNodeAttribute(parameterNode, "stepSizeTempRangeDAC") != ""  ) {
	stepSizeTempRangeDAC_ = atoi(xoap::getNodeAttribute(parameterNode, "stepSizeTempRangeDAC").data());
	stepSizeTempRangeDAC_initialized = true;
      } else if ( xoap::getNodeAttribute(parameterNode, "numTriggers") != ""  ) {
	numTriggers_ = atoi(xoap::getNodeAttribute(parameterNode, "numTriggers").data());
	numTriggers_initialized = true;
      } else {
	XCEPT_RAISE (xcept::Exception, "Undefined Parameter");
      }
    } else {
      XCEPT_RAISE (xcept::Exception, "Error parsing config File");
    }
  }

  if ( !(minTempRangeDAC_initialized &&
	 maxTempRangeDAC_initialized &&
	 stepSizeTempRangeDAC_initialized &&
	 numTriggers_initialized) ) {
    XCEPT_RAISE (xcept::Exception, "Parameter definitions missing");
  }

  printConfiguration(std::cout);
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelTemperatureCalibrationPluginLastDAC::printConfiguration(std::ostream& stream) const
{
  stream << "<PixelTemperatureCalibrationPluginLastDAC::printConfiguration>:" << std::endl;
  stream << " minTempRangeDAC = " << minTempRangeDAC_ << std::endl;
  stream << " maxTempRangeDAC = " << maxTempRangeDAC_ << std::endl;
  stream << " stepSizeTempRangeDAC = " << stepSizeTempRangeDAC_ << std::endl;
  stream << " numTriggers = " << numTriggers_ << std::endl;
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelTemperatureCalibrationPluginLastDAC::archiveConfigurationParameters(std::ostream& stream) const
{
  stream << "  <TemperatureCalibrationPluginLastDAC>" << std::endl;
  stream << "   <minTempRangeDAC>" << minTempRangeDAC_ << "</minTempRangeDAC>" << std::endl;
  stream << "   <maxTempRangeDAC>" << maxTempRangeDAC_ << "</maxTempRangeDAC>" << std::endl;
  stream << "   <stepSizeTempRangeDAC>" << stepSizeTempRangeDAC_ << "</stepSizeTempRangeDAC>" << std::endl;
  stream << "   <numTriggers>" << numTriggers_ << "</numTriggers>" << std::endl;
  stream << "  </TemperatureCalibrationPluginLastDAC>" << std::endl;
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelTemperatureCalibrationPluginLastDAC::loadPixelDACSettings(const pos::PixelDetectorConfig* detectorConfiguration)
{
  const std::vector<pos::PixelModuleName>& modules = detectorConfiguration->getModuleList();
  for ( std::vector<pos::PixelModuleName>::const_iterator module = modules.begin();
	module != modules.end(); ++module ) {
    std::string moduleName = module->modulename();
    std::string modulePath = moduleName;

    pos::PixelConfigKey globalConfigurationKey(defaultPixelConfigurationKey);

    pos::PixelDACSettings* dacSettingsModule = 0;
    PixelConfigInterface::get(dacSettingsModule, "pixel/dac/" + modulePath, globalConfigurationKey);

    if ( dacSettingsModule != NULL ) {
      dacSettingsModule_[*module] = dacSettingsModule;
    }
  }
}

//
//---------------------------------------------------------------------------------------------------
//
  
void PixelTemperatureCalibrationPluginLastDAC::execute(unsigned int iCycle, unsigned int numCycles) throw (xdaq::exception::Exception)
{
  std::cout << "<PixelTemperatureCalibrationPluginLastDAC::execute>:" << std::endl;

//--- get detector configuration objects
  const pos::PixelCalibConfiguration* calibrationParameters = dynamic_cast<const pos::PixelCalibConfiguration*>(globalConfigurationParameters_->getCalibObject());
  //assert( calibrationParameters != NULL );
  if ( calibrationParameters == NULL ) {
    XCEPT_RAISE (xcept::Exception, "Undefined Calibration Parameters");
  }

  if ( dataFile_ == NULL ) {
    XCEPT_RAISE (xcept::Exception, "Output File for Calibration Data not defined");
  }

  const pos::PixelNameTranslation* nameTranslation = globalConfigurationParameters_->getNameTranslation();
  const pos::PixelDetectorConfig* detectorConfiguration = globalConfigurationParameters_->getDetectorConfiguration();
  const pos::PixelFEDConfig* fedConfiguration = globalConfigurationParameters_->getFEDConfiguration();
  const pos::PixelFECConfig* pxlfecConfiguration = globalConfigurationParameters_->getFECConfiguration(); // PixelFEC configuration

//--- built list of ROCs and modules
//    WARNING: temporary fix, as usage of calib.dat file is currently broken --> FIXME
  pos::PixelCalibConfiguration* calibrationParameters_nonConst = const_cast<pos::PixelCalibConfiguration*>(calibrationParameters);
  calibrationParameters_nonConst->buildROCAndModuleLists(nameTranslation, detectorConfiguration);

  std::set<unsigned int> pxlfecCrates = calibrationParameters->getFECCrates(nameTranslation, pxlfecConfiguration);
  std::set<unsigned int> fedCrates = calibrationParameters->getFEDCrates(nameTranslation, fedConfiguration);
  std::vector<xdaq::ApplicationDescriptor*> ttcDescriptors = globalConfigurationParameters_->getPixelTTCDescriptors(); 

//--- disable charge-injection for all Read-out Chips
//    (send "ClrCal" command to all PixelFEC supervisor (crates))
  for ( std::set<unsigned int>::const_iterator pxlfecCrate = pxlfecCrates.begin();
	pxlfecCrate != pxlfecCrates.end(); ++pxlfecCrate ) {
    xdaq::ApplicationDescriptor* pxlfecDescriptor = const_cast<xdaq::ApplicationDescriptor*>(globalConfigurationParameters_->getPixelFECDescriptor(*pxlfecCrate));

    std::string soapResponse = soapCommander_->Send(pxlfecDescriptor, "ClrCalEnMass");

    if ( soapResponse != "ClrCalEnMassDone" ) {
      std::cerr << "Failed to execute ClrCalEnMass for PixelFEC crate" << std::endl;
      //diagService_->reportError("Failed to execute ClrCalEnMass for PixelFEC crate", DIAGERROR);
    }
  }

//--- load "nominal" DAC settings for all Read-out Chips
  const std::vector<pos::PixelROCName>& readOutChips = calibrationParameters->rocList();
  loadPixelDACSettings(detectorConfiguration);

//--- set Vana and Vsf DAC values to either reduced or nominal power consumption;
//    run calibration with nominal power settings in case no thermal cycling is performed
//    (i.e. numCyles is set zero);
//    the power consumption of the analog (digital) low voltage
//    is reduced significantly by setting the Vana (Vsf) DAC to zero
//    (for more details, see CMS Pixel DocDB document #1007) 
  int powerMode = -1;
  if ( numCycles == 0 || iCycle < (numCycles/2) ) {
    powerMode = kNominalPower;
  } else {
    powerMode = kReducedPower;
  }

  std::map<pos::PixelROCName, unsigned int> dacValues_Vana;
  std::map<pos::PixelROCName, unsigned int> dacValues_Vsf;
  for ( std::vector<pos::PixelROCName>::const_iterator readOutChipName = readOutChips.begin();
	readOutChipName != readOutChips.end(); ++readOutChipName ) {
    pos::PixelModuleName moduleName(readOutChipName->rocname());
    pos::PixelROCDACSettings* dacSettingsReadOutChip = dacSettingsModule_[moduleName]->getDACSettings(*readOutChipName);

    unsigned int dacValue_Vana = (powerMode == kNominalPower) ? dacSettingsReadOutChip->getVana() : 0;
    dacValues_Vana[*readOutChipName] = dacValue_Vana;
    unsigned int dacValue_Vsf = (powerMode == kNominalPower) ? dacSettingsReadOutChip->getVsf() : 0;
    dacValues_Vsf[*readOutChipName] = dacValue_Vsf;
  }

//--- compose SOAP messages
  const std::list<pos::PixelROCName> dacValues_dummy;
  xoap::MessageReference soapRequest_Vana 
    = dcs_soapCommander_->MakeSOAPMessageReference_progDAC(pos::k_DACAddress_Vana, dacValues_Vana, dacValues_dummy, dacValues_dummy, nameTranslation, pxlfecConfiguration);
  xoap::MessageReference soapRequest_Vsf 
    = dcs_soapCommander_->MakeSOAPMessageReference_progDAC(pos::k_DACAddress_Vsf, dacValues_Vsf, dacValues_dummy, dacValues_dummy, nameTranslation, pxlfecConfiguration);

//--- send SOAP messages
//    to all PixelFEC supervisors (crates)
  for ( std::set<unsigned int>::const_iterator pxlfecCrate = pxlfecCrates.begin();
  	pxlfecCrate != pxlfecCrates.end(); ++pxlfecCrate ) {
    xdaq::ApplicationDescriptor* pxlfecDescriptor = const_cast<xdaq::ApplicationDescriptor*>(globalConfigurationParameters_->getPixelFECDescriptor(*pxlfecCrate));

    dcs_soapCommander_->postSOAP(pxlfecDescriptor, soapRequest_Vana);
    dcs_soapCommander_->postSOAP(pxlfecDescriptor, soapRequest_Vsf);
  }

//--- set TempRange DAC values
//    (all values in the range 0..15)
  for ( unsigned int dacValue_TempRange = minTempRangeDAC_; dacValue_TempRange <= maxTempRangeDAC_; dacValue_TempRange += stepSizeTempRangeDAC_ ) {
    const std::map<pos::PixelROCName, unsigned int> dacValues_set = makeReadOutChipDACs(readOutChips, dacValue_TempRange);

//--- clear internal data structures
    clearLastDACReadings();

//--- compose SOAP message
    xoap::MessageReference soapRequest 
      = dcs_soapCommander_->MakeSOAPMessageReference_progDAC(pos::k_DACAddress_TempRange, dacValues_set, dacValues_dummy, dacValues_dummy, nameTranslation, pxlfecConfiguration);

//--- send SOAP message
//    to all PixelFEC supervisors (crates)
    for ( std::set<unsigned int>::const_iterator pxlfecCrate = pxlfecCrates.begin();
	  pxlfecCrate != pxlfecCrates.end(); ++pxlfecCrate ) {
      xdaq::ApplicationDescriptor* pxlfecDescriptor = const_cast<xdaq::ApplicationDescriptor*>(globalConfigurationParameters_->getPixelFECDescriptor(*pxlfecCrate));

      dcs_soapCommander_->postSOAP(pxlfecDescriptor, soapRequest);
    }

//--- generate triggers
//    (send "LevelOne" command to all TTC supervisors (crates))
    Attribute_Vector parametersToTTC(1);
    parametersToTTC[0].name_="CommandPar";
    parametersToTTC[0].value_="LevelOne";

    for ( unsigned int iTrigger = 0; iTrigger < numTriggers_; ++iTrigger ) {
      for ( std::vector<xdaq::ApplicationDescriptor*>::const_iterator ttcDescriptor = ttcDescriptors.begin();
	    ttcDescriptor != ttcDescriptors.end(); ++ttcDescriptor ) {
	std::string soapResponse = soapCommander_->Send(*ttcDescriptor, "ExecuteSequence", parametersToTTC);

	if ( soapResponse != "userTTCciControlResponse" ) {
	  std::cerr << "Failed to send 'LevelOne' command to TTCciControl" << std::endl;
	  //diagService_->reportError("Failed to send 'LevelOne' command to TTCciControl", DIAGWARN);
	}
      }

//--- read-out last DAC temperature values;
//    compose SOAP message
      xoap::MessageReference soapRequest = dcs_soapCommander_->MakeSOAPMessageReference_readLastDAC();
//    send SOAP message
//    to all FED supervisors (crates)
      for ( std::set<unsigned int>::const_iterator fedCrate = fedCrates.begin();
	    fedCrate != fedCrates.end(); ++fedCrate ) {
	xdaq::ApplicationDescriptor* fedDescriptor = const_cast<xdaq::ApplicationDescriptor*>(globalConfigurationParameters_->getPixelFEDDescriptor(*fedCrate));
	
	//xoap::MessageReference soapResponse = (const_cast<xdaq::Application*>(soapCommander_->getApplication()))->getApplicationContext()->postSOAP(soapRequest, fedDescriptor);
	xoap::MessageReference soapResponse = dcs_soapCommander_->postSOAP(fedDescriptor, soapRequest);
  
	decodeLastDACReadings(soapResponse);
      }
    }

//--- store PixelFEC DAC settings and last DAC ADC values in calibration DataBase
    archiveCalibrationData(*dataFile_, dacValues_Vana, dacValues_Vsf, dacValue_TempRange);
  }
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelTemperatureCalibrationPluginLastDAC::clearLastDACReadings()
{
//--- clear internal data structures

  for ( std::map<unsigned int, std::map<unsigned int, std::map<unsigned int, std::list<int> > > >::iterator fedData_board = fedData_.begin();
	fedData_board != fedData_.end(); ++fedData_board ) {
    for ( std::map<unsigned int, std::map<unsigned int, std::list<int> > >::iterator fedData_channel = fedData_board->second.begin();
	  fedData_channel != fedData_board->second.end(); ++fedData_channel ) {
      for ( std::map<unsigned int, std::list<int> >::iterator fedData_roc = fedData_channel->second.begin();
	    fedData_roc != fedData_channel->second.end(); ++fedData_roc ) {
	fedData_roc->second.clear();
      }
      fedData_channel->second.clear();
    }
    fedData_board->second.clear();
  }
  fedData_.clear();
}

void PixelTemperatureCalibrationPluginLastDAC::decodeLastDACReadings(xoap::MessageReference soapMessage)
{
//--- begin unpacking SOAP message
//    received from FED Supervisor
  xoap::SOAPEnvelope envelope = soapMessage->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();

//--- create SOAPName objects 
//    for keywords searched for in SOAP message
  xoap::SOAPName command = envelope.createName("readLastDACFIFOResponse");
  xoap::SOAPName fedBoard = envelope.createName("fedBoard");
  xoap::SOAPName dp = envelope.createName("dp");
  xoap::SOAPName fedBoardIdAttribute = envelope.createName("number");
  xoap::SOAPName fedChannelIdAttribute = envelope.createName("fedChannel");
  xoap::SOAPName rocIdAttribute = envelope.createName("roc");

//--- find within body 
//    response to "readLastDAC" command
  vector< xoap::SOAPElement > bodyElements = body.getChildElements(command);          	  
  for ( vector< xoap::SOAPElement >::iterator bodyElement = bodyElements.begin();
	bodyElement != bodyElements.end(); ++bodyElement ) {
    
//--- find within "readLastDAC" command response
//    lists of "fedBoard" identifiers
    vector< xoap::SOAPElement > boardElements = bodyElement->getChildElements(fedBoard); 
    for ( vector< xoap::SOAPElement >::iterator boardElement = boardElements.begin();
	  boardElement != boardElements.end(); ++boardElement ) {

//--- unpack fedBoardId
//    associated to "fedBoard" identifier
      //std::cout << "getAttributeValue(fedBoardIdAttribute) = " << boardElement->getAttributeValue(fedBoardIdAttribute) << std::endl;
      int fedBoardId = atoi(boardElement->getAttributeValue(fedBoardIdAttribute).data());

//--- find within "fedBoard" list
//    updates of "dp" data-point information
      vector< xoap::SOAPElement > dpElements = boardElement->getChildElements(dp); 
      for ( vector< xoap::SOAPElement >::iterator dpElement = dpElements.begin();
	    dpElement != dpElements.end(); ++dpElement ) {
      
//--- unpack fedChannelId, rocId and actual value	  
//    from data-point information
	//std::cout << "getAttributeValue(fedChannelIdAttribute) = " << dpElement->getAttributeValue(fedChannelIdAttribute) << std::endl;
	int fedChannelId = atoi(dpElement->getAttributeValue(fedChannelIdAttribute).data());
	//std::cout << "getAttributeValue(rocIdAttribute) = " << dpElement->getAttributeValue(rocIdAttribute) << std::endl;
	int rocId = atoi(dpElement->getAttributeValue(rocIdAttribute).data());
	int adcCount = atoi(dpElement->getValue().data());

	std::cout << "decoding fedData for" 
		  << " fedBoardId = " << fedBoardId << ", fedChannelId = " << fedChannelId << ", rocId = " << rocId << std::endl;

	fedData_[fedBoardId][fedChannelId][rocId].push_back(adcCount);
      }
    }
  }  
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelTemperatureCalibrationPluginLastDAC::archiveCalibrationData(std::ostream& stream,
								      std::map<pos::PixelROCName, unsigned int>& dacValues_Vana,
								      std::map<pos::PixelROCName, unsigned int>& dacValues_Vsf,
								      unsigned int dacValue_TempRange) const
{
  stream << "  <dataSet type=\"lastDAC\"" 
	 << " cycle=\"" << currentTemperatureCycle_ << "\""
	 << " nominalTemperature=\"" << currentNominalTemperature_ << "\""
	 << " TempRange=\"" << dacValue_TempRange << "\">" << std::endl;

  for ( std::map<unsigned int, std::map<unsigned int, std::map<unsigned int, std::list<int> > > >::const_iterator fedData_board = fedData_.begin();
	fedData_board != fedData_.end(); ++fedData_board ) {
    unsigned int fedBoard = fedData_board->first;
    
    for ( std::map<unsigned int, std::map<unsigned int, std::list<int> > >::const_iterator fedData_channel = fedData_board->second.begin();
	  fedData_channel != fedData_board->second.end(); ++fedData_channel ) {
      unsigned int fedChannel = fedData_channel->first;
      
      for ( std::map<unsigned int, std::list<int> >::const_iterator fedData_roc = fedData_channel->second.begin();
	    fedData_roc != fedData_channel->second.end(); ++fedData_roc ) {
	unsigned int rocId = fedData_roc->first;

	const pos::PixelROCName* readOutChipName = NULL;
	if ( fedReadOutChipName_.find(fedBoard) != fedReadOutChipName_.end() && 
	     fedReadOutChipName_.find(fedBoard)->second.find(fedChannel) != fedReadOutChipName_.find(fedBoard)->second.end() && 
	     fedReadOutChipName_.find(fedBoard)->second.find(fedChannel)->second.find(rocId) != fedReadOutChipName_.find(fedBoard)->second.find(fedChannel)->second.end() ) {
	  readOutChipName = fedReadOutChipName_.find(fedBoard)->second.find(fedChannel)->second.find(rocId)->second;
	} else {
	  std::cerr << "Error in <PixelTemperatureCalibrationPluginLastDAC::archiveCalibrationData>: No Read-out Chip defined for Connection"
		    << " fedBoard = " << fedBoard << ","
		    << " fedChannel = " << fedChannel << ","
		    << " rocId = " << rocId << std::endl;
	}

	if ( readOutChipName != NULL ) {
	  stream << "   <" << readOutChipName->rocname()
		 << " Vana=\"" << dacValues_Vana[*readOutChipName] << "\""
		 << " Vsf=\"" << dacValues_Vsf[*readOutChipName] << "\">" << std::endl;
	  
	  const std::list<int>& adcCounts = fedData_roc->second;
	  for ( std::list<int>::const_iterator adcCount = adcCounts.begin();
		adcCount != adcCounts.end(); ++adcCount ) {
	    stream << "    <adcCount>" << *adcCount << "</adcCount>" << std::endl;
	  }
	  
	  stream << "   </" << readOutChipName->rocname() << ">" << std::endl;
	} 
      }
    }
  }

  stream << "  </dataSet>" << std::endl;
}

//
//---------------------------------------------------------------------------------------------------
//

const std::map<pos::PixelROCName, unsigned int> makeReadOutChipDACs(const std::vector<pos::PixelROCName>& readOutChips, unsigned int dacValue)
{
  std::map<pos::PixelROCName, unsigned int> dacValues;
  for ( std::vector<pos::PixelROCName>::const_iterator readOutChipName = readOutChips.begin();
	readOutChipName != readOutChips.end(); ++readOutChipName ) {
    dacValues[*readOutChipName] = dacValue;
  }

  return dacValues;
}
