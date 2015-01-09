#include "PixelTemperatureCalibration.h"

// $Id: PixelTemperatureCalibration.cc,v 1.19 2009/10/20 12:01:49 aryd Exp $

/*************************************************************************
 * Class implementing thermal cycles for temperature calibration;        *
 * note that this class implements the thermal cycle only,               *
 * the individual Last DAC and DCU calibration routines                  *
 * are implemented as plug-ins                                           *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2009/10/20 12:01:49 $ (UTC)                       *
 *          by: $Author: aryd $                                       *
 *************************************************************************/

#include <time.h>
#include <math.h>

#include "xoap/DOMParserFactory.h"
#include "xercesc/dom/DOMDocument.hpp"
#include "xercesc/dom/DOMNodeList.hpp"
#include "xercesc/dom/DOMNode.hpp"

#include "PixelCalibrations/include/PixelTemperatureCalibrationPlugin.h"
#include "PixelCalibrations/include/PixelTemperatureCalibrationPluginLastDAC.h"
#include "PixelCalibrations/include/PixelTemperatureCalibrationPluginDCU.h"

#include "PixelConfigDBInterface/include/PixelConfigInterface.h"

#include "CalibFormats/SiPixelObjects/interface/PixelModuleName.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACSettings.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCDACSettings.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"

// declare global auxiliary functions
void archiveTime(const char* tagName, const time_t& time, std::ostream& stream);
std::string XMLChToString(const XMLCh* text_XMLCh);

//
//---------------------------------------------------------------------------------------------------
//

PixelTemperatureCalibration::PixelTemperatureCalibration(const PixelSupervisorConfiguration& configurationParameters, 
							 SOAPCommander* soapCommander, PixelDCSSOAPCommander* dcs_soapCommander, PixelDCSPVSSCommander* pvssCommander)
  : PixelCalibrationBase(configurationParameters, *soapCommander)
{
  std::cout << "<PixelTemperatureCalibration::PixelTemperatureCalibration>:" << std::endl;

  dcs_soapCommander_ = dcs_soapCommander;

  pvssCommander_ = pvssCommander;

  const std::string& BUILD_HOME = getenv("BUILD_HOME");
  std::cout << "<PixelTemperatureCalibration::PixelTemperatureCalibration> need to fix path using BUILD_HOME" << std::endl;
  configFileName_ = BUILD_HOME + "/pixel/PixelCalibrations/xml/" + "temperatureCalibration.xml";

  configFileLoaded_ = false; 

  dataFile_ = 0;
}

PixelTemperatureCalibration::~PixelTemperatureCalibration()
{
  std::cout << "<PixelTemperatureCalibration::~PixelTemperatureCalibration>:" << std::endl;

  delete dataFile_;

  for ( std::list<PixelTemperatureCalibrationPlugin*>::const_iterator plugin = plugins_.begin();
	plugin != plugins_.end(); ++plugin ) {
    delete (*plugin);
  } 
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelTemperatureCalibration::loadConfigFile() throw (xdaq::exception::Exception)
{
//--- open xml configuration file
//    and parameters for Temperature Calibration routine
//
  std::cout << "<PixelTemperatureCalibration::loadConfigFile>:" << std::endl;
  std::cout << " opening config file = " << configFileName_ << std::endl;

  bool dataFileName_initialized = false;

  bool numCycles_initialized = false;
  bool startCycle_initialized = false;
  bool minTemperature_initialized = false;
  bool maxTemperature_initialized = false;
  bool stepSizeTemperature_initialized = false;
  bool dpeNameBathTemperatureReading_initialized = false;
  bool dpeNameBathTemperatureSetting_initialized = false;
  bool maxTemperatureDeviationBath_initialized = false;
  bool maxDelayBath_initialized = false;
  bool addDelayBath_initialized = false;

  try {
    std::auto_ptr<DOMDocument> configDocument(xoap::getDOMParserFactory()->get("configure")->loadXML(configFileName_));

    if ( configDocument.get() != NULL ) {
      DOMNodeList* configTagList = configDocument->getElementsByTagName(xoap::XStr("temperaturecalibration:configuration"));
      if ( configTagList->getLength() == 1 ) {
	DOMNode* configTag = configTagList->item(0);

	DOMNodeList* configNodes = configTag->getChildNodes();
	unsigned int numConfigNodes = configNodes->getLength();
	for ( unsigned int iNode = 0; iNode < numConfigNodes; ++iNode ) {
	  DOMNode* configNode = configNodes->item(iNode);

//--- skip empty lines
//    (not removed by DOM parser)
	  if ( xoap::XMLCh2String(configNode->getLocalName()) == "" ) continue;

	  if ( xoap::XMLCh2String(configNode->getPrefix())    == "thermalcycle" &&
	       xoap::XMLCh2String(configNode->getLocalName()) == "parameters" ) {
//--- initialize parameters associated to cooling/heating cycles

	    DOMNodeList* parameterNodes = configNode->getChildNodes();
	    unsigned int numParameterNodes = parameterNodes->getLength();
	    for ( unsigned int iNode = 0; iNode < numParameterNodes; ++iNode ) {
	      DOMNode* parameterNode = parameterNodes->item(iNode);

//--- skip empty lines
//    (not removed by DOM parser)
	      if ( xoap::XMLCh2String(parameterNode->getLocalName()) == "" ) continue;

	      if ( xoap::XMLCh2String(parameterNode->getPrefix())    == "thermalcycle"    &&
		   xoap::XMLCh2String(parameterNode->getLocalName()) == "parameter" ) {
		if ( xoap::getNodeAttribute(parameterNode, "numCycles") != ""  ) {
		  numCycles_ = atoi(xoap::getNodeAttribute(parameterNode, "numCycles").data());
		  numCycles_initialized = true;
		} else if ( xoap::getNodeAttribute(parameterNode, "startCycle") != ""  ) {
		  if ( xoap::getNodeAttribute(parameterNode, "startCycle") == "cooling" ) {
		    startCycle_ = kCoolingCycle;
		    startCycle_initialized = true;
		  } else if ( xoap::getNodeAttribute(parameterNode, "startCycle") == "heating" ) {
		    startCycle_ = kHeatingCycle;
		    startCycle_initialized = true;
		  } else {
		    XCEPT_RAISE (xdaq::exception::Exception, "Undefined value for Parameter 'startCycle'");
		}
		} else if ( xoap::getNodeAttribute(parameterNode, "minTemperature") != ""  ) {
		  minTemperature_ = atof(xoap::getNodeAttribute(parameterNode, "minTemperature").data());
		  minTemperature_initialized = true;
		} else if ( xoap::getNodeAttribute(parameterNode, "maxTemperature") != ""  ) {
		  maxTemperature_ = atof(xoap::getNodeAttribute(parameterNode, "maxTemperature").data());
		  maxTemperature_initialized = true;
		} else if ( xoap::getNodeAttribute(parameterNode, "stepSizeTemperature") != ""  ) {
		  stepSizeTemperature_ = atof(xoap::getNodeAttribute(parameterNode, "stepSizeTemperature").data());
		  stepSizeTemperature_initialized = true;
		} else if ( xoap::getNodeAttribute(parameterNode, "dpeNameBathTemperatureReading") != ""  ) {
		  dpeNameBathTemperatureReading_ = xoap::getNodeAttribute(parameterNode, "dpeNameBathTemperatureReading");
		  dpeNameBathTemperatureReading_initialized = true;
		} else if ( xoap::getNodeAttribute(parameterNode, "dpeNameBathTemperatureSetting") != ""  ) {
		  dpeNameBathTemperatureSetting_ = xoap::getNodeAttribute(parameterNode, "dpeNameBathTemperatureSetting");
		  dpeNameBathTemperatureSetting_initialized = true;
		} else if ( xoap::getNodeAttribute(parameterNode, "maxTemperatureDeviationBath") != ""  ) {
		  maxTemperatureDeviationBath_ = atof(xoap::getNodeAttribute(parameterNode, "maxTemperatureDeviationBath").data());
		  maxTemperatureDeviationBath_initialized = true;
		} else if ( xoap::getNodeAttribute(parameterNode, "maxDelayBath") != ""  ) {
		  maxDelayBath_ = atoi(xoap::getNodeAttribute(parameterNode, "maxDelayBath").data());
		  maxDelayBath_initialized = true;
		} else if ( xoap::getNodeAttribute(parameterNode, "addDelayBath") != ""  ) {
		  addDelayBath_ = atoi(xoap::getNodeAttribute(parameterNode, "addDelayBath").data());
		  addDelayBath_initialized = true;
		} else {
		  std::string errorMessage;
		  if ( parameterNode->getAttributes()->getLength() != 1 ) {
		    errorMessage = "Non or multiple Parameters defined";
		  } else {
		    std::string parameterNodeName = XMLChToString(parameterNode->getNodeName());
		    std::string parameterNodeAttribute = XMLChToString(parameterNode->getAttributes()->item(0)->getNodeName());
		    
		    errorMessage = std::string("Undefined Parameter = ") + parameterNodeAttribute;
		  }

		  XCEPT_RAISE (xdaq::exception::Exception, errorMessage + ", id = #0000");
		}
	      } else {
		XCEPT_RAISE (xdaq::exception::Exception, "Error parsing config File, id = #0000");
	      }
	    }
	  } else if ( xoap::XMLCh2String(configNode->getPrefix())    == "archiving" &&
		      xoap::XMLCh2String(configNode->getLocalName()) == "options" ) {
//--- initialize parameters associated to archiving

	    DOMNodeList* parameterNodes = configNode->getChildNodes();
	    unsigned int numParameterNodes = parameterNodes->getLength();
	    for ( unsigned int iNode = 0; iNode < numParameterNodes; ++iNode ) {
	      DOMNode* parameterNode = parameterNodes->item(iNode);

//--- skip empty lines
//    (not removed by DOM parser)
	      if ( xoap::XMLCh2String(parameterNode->getLocalName()) == "" ) continue;

	      if ( xoap::XMLCh2String(parameterNode->getPrefix())    == "archiving" &&
		   xoap::XMLCh2String(parameterNode->getLocalName()) == "option"  ) {
		if ( xoap::getNodeAttribute(parameterNode, "fileName") != ""  ) {
		  dataFileName_ = xoap::getNodeAttribute(parameterNode, "fileName");
		  dataFileName_initialized = true;
		} else {
		  std::string errorMessage;
		  if ( parameterNode->getAttributes()->getLength() != 1 ) {
		    errorMessage = "Non or multiple Parameters defined";
		  } else {
		    std::string parameterNodeName = XMLChToString(parameterNode->getNodeName());
		    std::string parameterNodeAttribute = XMLChToString(parameterNode->getAttributes()->item(0)->getNodeName());
		    
		    errorMessage = std::string("Undefined Parameter = ") + parameterNodeAttribute;
		  }

		  XCEPT_RAISE (xdaq::exception::Exception, errorMessage + ", id = #0001");
		}
	      } else if ( xoap::XMLCh2String(parameterNode->getPrefix())    == "archiving" &&
			  xoap::XMLCh2String(parameterNode->getLocalName()) == "dp"      ) {
		std::string dpeName = xoap::getNodeAttribute(parameterNode, "dpeName");
		std::string dpeAlias = xoap::getNodeAttribute(parameterNode, "dpeAlias");
		
		std::cout << "dpeName = " << dpeName << ", dpeAlias = " << dpeAlias << std::endl;

		PixelDCSPVSSDpe dpe(dpeName, "");
		dpe.setAlias(dpeAlias);	                   
		archivedDataPointElements_.push_back(dpe);
		
		std::cout << "ALIAS = " << archivedDataPointElements_.back().getAlias() << std::endl;
	      } else {
		XCEPT_RAISE (xdaq::exception::Exception, "Error parsing config File, id = #0001");
	      }
	    }
	  } else if ( xoap::XMLCh2String(configNode->getPrefix())    == "plugin" &&
		      xoap::XMLCh2String(configNode->getLocalName()) == "parameters" ) {
	    std::string pluginType = xoap::getNodeAttribute(configNode, "type");
	    
	    if ( pluginType == "PixelTemperatureCalibrationPluginLastDAC" ) {
//--- use std::auto_ptr here
//    to avoid memory leak
//    in case exception is thrown
	      std::auto_ptr<PixelTemperatureCalibrationPluginLastDAC> 
		plugin(new PixelTemperatureCalibrationPluginLastDAC(this, dynamic_cast<SOAPCommander*>(this), dcs_soapCommander_, pvssCommander_, configNode));
	      plugins_.push_back(plugin.release());
	    } else if ( pluginType == "PixelTemperatureCalibrationPluginDCU" ) {
	      std::auto_ptr<PixelTemperatureCalibrationPluginDCU> 
		plugin(new PixelTemperatureCalibrationPluginDCU(this, dynamic_cast<SOAPCommander*>(this), dcs_soapCommander_, pvssCommander_, configNode));
	      plugins_.push_back(plugin.release());
	    } else {
	      XCEPT_RAISE (xdaq::exception::Exception, "Undefined Plug-in Type");
	    }
	  } else {
	    XCEPT_RAISE (xdaq::exception::Exception, "Error parsing config File, id = #0002");
	  }
	}
      }

//--- free memory allocated by DOM parser
      configDocument->release();
    }
  } catch ( xcept::Exception& e ) {
    std::cout << "Caught Exception thrown by loadXML" << std::endl;
    std::cout << " " << e.what() << std::endl;
    XCEPT_RETHROW(xcept::Exception, "Could not parse config File. " + std::string(e.what()), e);
  }

  if ( !(dataFileName_initialized &&
	 numCycles_initialized &&
	 startCycle_initialized &&
	 minTemperature_initialized && 
	 maxTemperature_initialized &&
	 stepSizeTemperature_initialized &&
	 dpeNameBathTemperatureReading_initialized &&
	 dpeNameBathTemperatureSetting_initialized &&
	 maxTemperatureDeviationBath_initialized &&
	 maxDelayBath_initialized &&
	 addDelayBath_initialized) ) {
    XCEPT_RAISE (xdaq::exception::Exception, "Parameter definitions missing");
  }

  std::cout << " finished parsing config file." << std::endl;
  printConfiguration(std::cout);
  configFileLoaded_ = true;
}


void PixelTemperatureCalibration::printConfiguration(std::ostream& stream) const
{
  stream << "<PixelTemperatureCalibration::printConfiguration>:" << std::endl;
  stream << " dataFileName = " << dataFileName_ << std::endl;
  stream << " numCycles = " << numCycles_ << std::endl;
  stream << " startCycle = " << startCycle_ << std::endl;
  stream << " minTemperature = " << minTemperature_ << std::endl;
  stream << " maxTemperature = " << maxTemperature_ << std::endl;
  stream << " stepSizeTemperature = " << stepSizeTemperature_ << std::endl;
  stream << " dpeNameBathTemperatureReading = " << dpeNameBathTemperatureReading_ << std::endl;
  stream << " dpeNameBathTemperatureSetting = " << dpeNameBathTemperatureSetting_ << std::endl;
  stream << " maxTemperatureDeviationBath = " << maxTemperatureDeviationBath_ << std::endl;
  stream << " maxDelayBath = " << maxDelayBath_ << std::endl;
  stream << " addDelayBath = " << addDelayBath_ << std::endl;

  for ( std::list<PixelTemperatureCalibrationPlugin*>::const_iterator plugin = plugins_.begin();
	plugin != plugins_.end(); ++plugin ) {
    (*plugin)->printConfiguration(stream);
  } 
}

//
//---------------------------------------------------------------------------------------------------
//

bool PixelTemperatureCalibration::execute() 
{
  if ( !configFileLoaded_ ) {
    try {
//--- initialize configuration parameters
      this->loadConfigFile();
    } catch ( xdaq::exception::Exception& e ) {
      std::cout << "Caught Exception thrown by PixelTemperatureCalibration::loadConfigFile" << std::endl;
      std::cout << " " << e.what() << std::endl;
      XCEPT_RETHROW(xoap::exception::Exception, "Failed to parse config File. " + std::string(e.what()), e);
    }    
  }

//--- create ASCII file for output of calibration raw data;
//    check that file name has been set
   if ( dataFileName_ == "" ) {
    std::cerr << "Error in <PixelTemperatureCalibration::execute>: dataFile = " << dataFileName_ << " has not been defined in configuration parameters !!!" << std::endl;
    return false;
  }

//--- check that output file does not yet exist
  std::ifstream testFile;
  testFile.open(dataFileName_.data(), std::ifstream::in);
  testFile.close();
  
  if ( !testFile.fail() ) {
//--- signature of execute method defined in PixelCalibrationBase class
//    does not allow to throw an exception here,
//    so simply print an error message and return
    std::cerr << "Error in <PixelTemperatureCalibration::execute>: dataFile = " << dataFileName_ << " does already exist !!!" << std::endl;
    return false;
  }

  testFile.clear(std::ios::failbit);
  
//--- now create output file 
//    (in case it does not yet exist)
  dataFile_ = new std::ofstream(dataFileName_.data(), std::ofstream::out);

//--- make output file known to calibration plug-ins
  for ( std::list<PixelTemperatureCalibrationPlugin*>::const_iterator plugin = plugins_.begin();
	plugin != plugins_.end(); ++plugin ) {
    (*plugin)->setDataFile(dataFile_);
  } 

//--- record time at which calibration starts
  time(&calibrationStartTime_);
  archiveHeader(*dataFile_);

  if ( numCycles_ > 0 ) {
    for ( unsigned int iCycle = 0; iCycle < numCycles_; ++iCycle ) {
      if ( startCycle_ == kHeatingCycle ) {
	doHeatingCycle(iCycle);
	doCoolingCycle(iCycle);
      } else if ( startCycle_ == kCoolingCycle ) {
	doCoolingCycle(iCycle);
	doHeatingCycle(iCycle);
      } 
    }
  } else {
    doCalibration();
  }

//--- record time at which calibration ends
  time(&calibrationEndTime_);
  archiveTrailer(*dataFile_);

//--- close output file
  delete dataFile_;
  dataFile_ = 0; 

  return false;
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelTemperatureCalibration::doHeatingCycle(unsigned int iCycle)
{
  std::cout << "<PixelTemperatureCalibration::doHeatingCycle>:" << std::endl;

  currentTemperatureCycle_ = "Heating";

  for ( double temperature = minTemperature_; temperature <= maxTemperature_; temperature += stepSizeTemperature_ ) {
    try {
      setTemperature(temperature);  

      doCalibration(iCycle);
    } catch ( xdaq::exception::Exception& e ) {
      std::cout << "Caught Exception thrown by <PixelTemperatureCalibration::setTemperature>:" << std::endl;
      std::cout << " " << e.what() << std::endl;
      //XCEPT_RETHROW(xoap::exception::Exception, "Failed to set temperature of C6F14 chiller --> " + std::string(e.what()), e);
      std::cout << " --> skipping to take Calibration data for Temperature = " << temperature << std::endl;
    }    
  }  
}

void PixelTemperatureCalibration::doCoolingCycle(unsigned int iCycle)
{
  std::cout << "<PixelTemperatureCalibration::doCoolingCycle>:" << std::endl;

  currentTemperatureCycle_ = "Cooling";

  for ( double temperature = maxTemperature_; temperature >= minTemperature_; temperature -= stepSizeTemperature_ ) {
    try {
      setTemperature(temperature);  

      doCalibration(iCycle);
    } catch ( xdaq::exception::Exception& e ) {
      std::cout << "Caught Exception thrown by <PixelTemperatureCalibration::setTemperature>:" << std::endl;
      std::cout << " " << e.what() << std::endl;
      //XCEPT_RETHROW(xoap::exception::Exception, "Failed to set temperature of C6F14 chiller --> " + std::string(e.what()), e);
      std::cout << " --> skipping to take Calibration data for Temperature = " << temperature << std::endl;
    }  
  }
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelTemperatureCalibration::setTemperature(double temperature) throw (xdaq::exception::Exception)
{
  std::cout << "<PixelTemperatureCalibration::setTemperature>:" << std::endl;

  currentNominalTemperature_ = temperature;

  if ( dpeNameBathTemperatureSetting_ != "" && 
       dpeNameBathTemperatureReading_ != "" ) {
    std::cout << " setting C6F14 Chiller temperature = " << temperature << std::endl;

    pvssCommander_->setDpeValue_float(dpeNameBathTemperatureSetting_, temperature);
    
    time_t t0;
    time(&t0);

    double currentTemperature;
    time_t currentTime;
    do {
      sleep(addDelayBath_);
      
      currentTemperature = pvssCommander_->getDpeValue_float(dpeNameBathTemperatureReading_);
      std::cout << " current C6F14 Chiller temperature = " << currentTemperature << std::endl;
      
      time(&currentTime);

      std::cout << "  difftime = " << difftime(currentTime, t0) << std::endl;
    } while ( fabs(currentTemperature - temperature) > maxTemperatureDeviationBath_ && difftime(currentTime, t0) < maxDelayBath_ );
    
    if ( fabs(currentTemperature - temperature) > maxTemperatureDeviationBath_  ) {
      XCEPT_RAISE(xdaq::exception::Exception, "Failed to set temperature");
    }
  }
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelTemperatureCalibration::doCalibration(unsigned int iCycle)
{
  std::cout << "<PixelTemperatureCalibration::doCalibration>:" << std::endl;

//--- run all calibration plug-ins
  for ( std::list<PixelTemperatureCalibrationPlugin*>::const_iterator plugin = plugins_.begin();
	plugin != plugins_.end(); ++plugin ) {
    (*plugin)->SetNominalTemperature(currentNominalTemperature_);
    (*plugin)->SetTemperatureCycle(currentTemperatureCycle_);
    (*plugin)->execute(iCycle, numCycles_);
  }

//--- archive PVSS data-point element data
//    (temperatures read-out via the Siemens S7-300 PLC system and 
//     voltages and currents of the CAEN power supplies)
  archivePVSSDpeData(*dataFile_);
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelTemperatureCalibration::archiveConfigurationParameters(std::ostream& stream) const
{
//--- store calibration Identifier, start and end time in TEMPERATURE_CALIBRATIONS table
//    in Oracle DataBase

  std::cout << "<PixelTemperatureCalibration::archiveConfigurationParameters>:" << std::endl;

  stream << " <configurationParameters>" << std::endl;
  stream << "  <TemperatureCalibration>" << std::endl;
  stream << "   <numCycles>" << numCycles_ << "</numCycles>" << std::endl;
  stream << "   <startCycle>" << startCycle_ << "</startCycle>" << std::endl;
  stream << "   <minTemperature>" << minTemperature_ << "</minTemperature>" << std::endl;
  stream << "   <maxTemperature>" << maxTemperature_ << "</maxTemperature>" << std::endl;
  stream << "   <stepSizeTemperature>" << stepSizeTemperature_ << "</stepSizeTemperature>" << std::endl;
  stream << "   <dpeNameBathTemperatureReading>" << dpeNameBathTemperatureReading_ << "</dpeNameBathTemperatureReading>" << std::endl;
  stream << "   <dpeNameBathTemperatureSetting>" << dpeNameBathTemperatureSetting_ << "</dpeNameBathTemperatureSetting>" << std::endl;
  stream << "   <maxTemperatureDeviationBath>" << maxTemperatureDeviationBath_ << "</maxTemperatureDeviationBath>" << std::endl;
  stream << "   <maxDelayBath>" << maxDelayBath_ << "</maxDelayBath>" << std::endl;
  stream << "   <addDelayBath>" << addDelayBath_ << "</addDelayBath>" << std::endl;
  stream << "  </TemperatureCalibration>" << std::endl;
  for ( std::list<PixelTemperatureCalibrationPlugin*>::const_iterator plugin = plugins_.begin();
	plugin != plugins_.end(); ++plugin ) {
    (*plugin)->archiveConfigurationParameters(stream);
  }
  stream << " </configurationParameters>" << std::endl;
}

void PixelTemperatureCalibration::archiveDACSettings(std::ostream& stream) const
{
//--- store DAC settings for all Read-out Chips
//    in Oracle DataBase

  std::cout << "<PixelTemperatureCalibration::archiveDACSettings>:" << std::endl;

  stream << " <dacSettings>" << std::endl;

//--- get list of modules defined in detector configuration
  const pos::PixelDetectorConfig* detectorConfiguration = this->getDetectorConfiguration();
  const pos::PixelNameTranslation* nameTranslationTable = this->getNameTranslation();
  const pos::PixelConfigKey* globalConfigurationKey = this->getGlobalKey();

  std::vector<pos::PixelModuleName> modules = detectorConfiguration->getModuleList();
  std::list<const pos::PixelROCName*> readOutChips = nameTranslationTable->getROCs();
  for ( std::vector<pos::PixelModuleName>::const_iterator module = modules.begin();
	module != modules.end(); ++module ) {
    const std::string& moduleName = module->modulename();
    const std::string& dacSettings_path = moduleName;

//--- load DAC settings of module
    pos::PixelDACSettings* dacSettings = NULL;
    PixelConfigInterface::get(dacSettings, "pixel/dac/" + dacSettings_path, *globalConfigurationKey);

    if ( dacSettings != NULL ) {
//--- find read-out chips associated to module
      for ( std::list<const pos::PixelROCName*>::const_iterator readOutChip = readOutChips.begin();
	    readOutChip != readOutChips.end(); ++readOutChip ) {
	if ( *readOutChip != NULL ) {
//--- get DAC settings of read-out chip
	  pos::PixelROCDACSettings* dacSettings_readOutChip = dacSettings->getDACSettings(**readOutChip);
	  if ( dacSettings_readOutChip != NULL ) {
	    //dacSettings_readOutChip->writeASCII(std::cout);

	    stream << "  <" << (*readOutChip)->rocname() << ">" << std::endl;
	    assert(0);
#if 0
	    stream << "   <Vdd>" << (int)dacSettings_readOutChip->getVdd() << "</Vdd>" << std::endl;
	    stream << "   <Vana>" << (int)dacSettings_readOutChip->getVana() << "</Vana>" << std::endl;
	    stream << "   <Vsf>" << (int)dacSettings_readOutChip->getVsf() << "</Vsf>" << std::endl;
	    stream << "   <Vleak>" << (int)dacSettings_readOutChip->getVleak() << "</Vleak>" << std::endl;
	    stream << "   <VrgPr>" << (int)dacSettings_readOutChip->getVrgPr() << "</VrgPr>" << std::endl;
	    stream << "   <VwllPr>" << (int)dacSettings_readOutChip->getVwllPr() << "</VwllPr>" << std::endl;
	    stream << "   <VrgSh>" << (int)dacSettings_readOutChip->getVrgSh() << "</VrgSh>" << std::endl;
	    stream << "   <VwllSh>" << (int)dacSettings_readOutChip->getVwllSh() << "</VwllSh>" << std::endl;
	    stream << "   <VHldDel>" << (int)dacSettings_readOutChip->getVHldDel() << "</VHldDel>" << std::endl;
	    stream << "   <Vtrim>" << (int)dacSettings_readOutChip->getVtrim() << "</Vtrim>" << std::endl;
	    stream << "   <VcThr>" << (int)dacSettings_readOutChip->getVcThr() << "</VcThr>" << std::endl;
	    stream << "   <VIbias_bus>" << (int)dacSettings_readOutChip->getVIbias_bus() << "</VIbias_bus>" << std::endl;
	    stream << "   <VIbias_sf>" << (int)dacSettings_readOutChip->getVIbias_sf() << "</VIbias_sf>" << std::endl;
	    stream << "   <VOffsetOp>" << (int)dacSettings_readOutChip->getVOffsetOp() << "</VOffsetOp>" << std::endl;
	    stream << "   <VbiasOp>" << (int)dacSettings_readOutChip->getVbiasOp() << "</VbiasOp>" << std::endl;
	    stream << "   <VOffsetRO>" << (int)dacSettings_readOutChip->getVOffsetRO() << "</VOffsetRO>" << std::endl;
	    stream << "   <VIon>" << (int)dacSettings_readOutChip->getVIon() << "</VIon>" << std::endl;
	    stream << "   <VIbias_PH>" << (int)dacSettings_readOutChip->getVIbias_PH() << "</VIbias_PH>" << std::endl;
	    stream << "   <VIbias_DAC>" << (int)dacSettings_readOutChip->getVIbias_DAC() << "</VIbias_DAC>" << std::endl;
	    stream << "   <VIbias_roc>" << (int)dacSettings_readOutChip->getVIbias_roc() << "</VIbias_roc>" << std::endl;
	    stream << "   <VIColOr>" << (int)dacSettings_readOutChip->getVIColOr() << "</VIColOr>" << std::endl;
	    stream << "   <Vnpix>" << (int)dacSettings_readOutChip->getVnpix() << "</Vnpix>" << std::endl;
	    stream << "   <VsumCol>" << (int)dacSettings_readOutChip->getVsumCol() << "</VsumCol>" << std::endl;
	    stream << "   <Vcal>" << (int)dacSettings_readOutChip->getVcal() << "</Vcal>" << std::endl;
	    stream << "   <CalDel>" << (int)dacSettings_readOutChip->getCalDel() << "</CalDel>" << std::endl;	    
	    stream << "   <WBC>" << (int)dacSettings_readOutChip->getWBC() << "</WBC>" << std::endl;
	    stream << "   <ControlRegister>" << (int)dacSettings_readOutChip->getControlRegister() << "</ControlRegister>" << std::endl;
#endif
	    stream << "  </" << (*readOutChip)->rocname() << ">" << std::endl;
	  }
	}
      }
    } else {
      std::cerr << "Error in <PixelTemperatureCalibration::archiveDACSettings>: Could not get DAC settings of Module = " << *module << std::endl;
    }
  }

  stream << " </dacSettings>" << std::endl;
}

void PixelTemperatureCalibration::archiveHeader(std::ostream& stream) const
{
//--- store calibration Identifier, start and end time in TEMPERATURE_CALIBRATIONS table
//    in Oracle DataBase/XML file

  std::cout << "<PixelTemperatureCalibration::archiveHeader>:" << std::endl;

  stream << "<pixelTemperatureCalibrationData>" << std::endl;
  archiveConfigurationParameters(stream);
  archiveDACSettings(stream);
  archiveTime("startTime", calibrationStartTime_, stream);
  stream << " <calibrationData>" << std::endl;
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelTemperatureCalibration::archivePVSSDpeData(std::ostream& stream) const
{
//--- store values of PVSS data-point elements in TEMPERATURE_CALIBRATION_DPEDATA table
//    in Oracle DataBase
  std::cout << "<PixelTemperatureCalibration::archivePVSSDpeConfig>:" << std::endl;

  stream << "  <dcsConditions nominalTemperature=\"" << currentNominalTemperature_ << "\">" << std::endl;

  std::list<PixelDCSPVSSDpe> dpes = pvssCommander_->getDpeValues(archivedDataPointElements_);
  for ( std::list<PixelDCSPVSSDpe>::const_iterator dpe = dpes.begin();
    dpe != dpes.end(); ++dpe ) {
    const std::string& dpeName = dpe->getName();
    const std::string& dpeAlias = dpe->getAlias();
    const std::string& dpeValue = dpe->getValue();

    stream << "   <" << dpeName << " alias=\"" << dpeAlias << "\">" 
	   << dpeValue
	   << "</" << dpeName << ">" << std::endl;
  }

  stream << "  </dcsConditions>" << std::endl;
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelTemperatureCalibration::archiveTrailer(std::ostream& stream) const
{
//--- store calibration Identifier, start and end time in TEMPERATURE_CALIBRATIONS table
//    in Oracle DataBase/XML file

  std::cout << "<PixelTemperatureCalibration::archiveTrailer>:" << std::endl;

  stream << " </calibrationData>" << std::endl;
  archiveTime("endTime", calibrationEndTime_, stream);
  stream << "</pixelTemperatureCalibrationData>" << std::endl;
}

//---------------------------------------------------------------------------------------------------
//                AUXILIARY FUNCTIONS DEFINED OUTSIDE OF CLASS PIXELTEMPERATURECALIBRATION
//---------------------------------------------------------------------------------------------------

void archiveTime(const char* tagName, const time_t& time, std::ostream& stream)
{
//--- store date end time that the calibration data was taken
//    in Oracle DataBase/XML file

  struct tm* time_struct = localtime(&time);

  int year = time_struct->tm_year + 1900;
  int month = time_struct->tm_mon + 1;
  int day = time_struct->tm_mday;
  int hour = time_struct->tm_hour;
  int minute = time_struct->tm_min;
  int second = time_struct->tm_sec;

  stream << " <" << tagName << ">" << std::endl;
  stream << "  <year>" << year << "</year>" << std::endl;
  stream << "  <month>" << month << "</month>" << std::endl;
  stream << "  <day>" << day << "</day>" << std::endl;
  stream << "  <hour>" << hour << "</hour>" << std::endl;
  stream << "  <minute>" << minute << "</minute>" << std::endl;
  stream << "  <second>" << second << "</second>" << std::endl;
  stream << " </" << tagName << ">" << std::endl;
}

std::string XMLChToString(const XMLCh* text_XMLCh)
{
  std::string text_string = "";
  
  unsigned index = 0;
  while ( text_XMLCh[index] != 0 ) {
    char c = text_XMLCh[index];
    text_string += c;
    ++index;
  }
  
  return text_string;
}


std::vector<std::string> PixelTemperatureCalibration::calibrated(){

  std::vector<std::string> tmp;
  
  return tmp;

}
