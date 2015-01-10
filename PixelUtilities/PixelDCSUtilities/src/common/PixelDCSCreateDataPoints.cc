
#include "PixelDCSCreateDataPoints.h"

/*************************************************************************
 * Base class for PixelDCStoFECCreateDataPoints                          *
 * and PixelDCStoFEDCreateDataPoints;                                    *
 * implements access to Oracle DataBase                                  *
 *                                                                       *
 * Author: Christian Veelken, UC Davis				         *
 *                                                                       *
 * Last update: $Date: 2007/10/05 18:49:49 $ (UTC)                       *
 *          by: $Author: stroiney $                                       *
 *************************************************************************/

#include <iomanip>

#include <time.h>

#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationStub.h"
#include "xgi/Utils.h"
#include "cgicc/HTMLClasses.h"
#include "toolbox/fsm/FailedEvent.h"
#include "toolbox/task/WorkLoopFactory.h"
#include "toolbox/task/Action.h"
#include "tstore/client/AttachmentUtils.h"

#include "CalibFormats/SiPixelObjects/interface/PixelNameTranslation.h"
#include "CalibFormats/SiPixelObjects/interface/PixelHdwAddress.h"
#include "CalibFormats/SiPixelObjects/interface/PixelConfigKey.h"
/*
#include "PixelConfigDataFormats/include/PixelConfigKey.h"
#include "PixelConfigDataFormats/include/PixelNameTranslation.h"
#include "PixelConfigDataFormats/include/PixelHdwAddress.h"
*/
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSPVSSCommander.h"
#include "PixelUtilities/PixelGUIUtilities/include/HTML2XGI.h"

#define PSX_NS_URI "http://xdaq.cern.ch/xdaq/xsd/2006/psx-pvss-10.xsd"
#define TSTORE_NS_URI "http://xdaq.web.cern.ch/xdaq/xsd/2006/tstore-10.xsd"

using namespace pos;

const std::string pvssCreateDataPointName = "temp_xdaqCreateDataPoint01";

//FIXME This will not work in general
const unsigned int defaultPixelConfigurationKey = 0;

XDAQ_INSTANTIATOR_IMPL(PixelDCSCreateDataPoints)

PixelDCSCreateDataPoints::PixelDCSCreateDataPoints(xdaq::ApplicationStub* s) throw (xdaq::exception::Exception) 
  : xdaq::Application(s)
{ 
//--- define SOAP Bindings to Low Level Commands and Specific Algorithms
  xoap::bind(this, &PixelDCSCreateDataPoints::createDataPointsFED, "CreateDataPointsFED", XDAQ_NS_URI);
  xoap::bind(this, &PixelDCSCreateDataPoints::createDataPointsTrkFEC_Temperatures, "CreateDataPointsTrkFEC_Temperatures", XDAQ_NS_URI);
  xoap::bind(this, &PixelDCSCreateDataPoints::createDataPointsTrkFEC_Voltages, "CreateDataPointsTrkFEC_Voltages", XDAQ_NS_URI);

//--define XGI Callback Bindings for messages received from the browser
  xgi::bind(this, &PixelDCSCreateDataPoints::Default, "Default");
  xgi::bind(this, &PixelDCSCreateDataPoints::XgiHandler, "XgiHandler");

  httpPageHeader_ = "Pixel DCS Create Data-Points";
	
//--- initialize parameters defined by environment variables
  XDAQ_ROOT = getenv("XDAQ_ROOT");
  
  workloopStatus_ = "inactive";

  dpNameTable_ = NULL;

  workloopStatus_ = "inactive";

  soapCommander_ = new SOAPCommander(this);
  xdaq::ApplicationDescriptor* pvssDescriptor = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor("pvss",0);
  pvssCommander_ = new PixelDCSPVSSCommander(this, pvssDescriptor);
}

PixelDCSCreateDataPoints::~PixelDCSCreateDataPoints()
{
  delete dpNameTable_;

  delete soapCommander_;
  delete pvssCommander_;
}

void PixelDCSCreateDataPoints::Default(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception)
{
  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  *out << cgicc::html().set("lang", "en").set("dir", "ltr") << std::endl;
  xgi::Utils::getPageHeader(*out, httpPageHeader_);
  
  // Rendering Low Level GUI
  
  *out << "<h2>Low Level Commands</h2>" << endl;
  
  std::string url = "/" + getApplicationDescriptor()->getURN() + "/XgiHandler";
  
  *out << "<form name=\"input\" method=\"get\" action=\"" << url << "\" enctype=\"multipart/form-data\">";
  HTML2XGI(out, XDAQ_ROOT + "/pixel/PixelUtilities/PixelDCSUtilities/html/CreateDataPointsFED.htm");
  *out << "</form>" << endl;
  
  *out << "<form name=\"input\" method=\"get\" action=\"" << url << "\" enctype=\"multipart/form-data\">";
  HTML2XGI(out, XDAQ_ROOT + "/pixel/PixelUtilities/PixelDCSUtilities/html/CreateDataPointsTrkFEC_Temperatures.htm");
  *out << "</form>" << endl;
  *out << "<form name=\"input\" method=\"get\" action=\"" << url << "\" enctype=\"multipart/form-data\">";
  HTML2XGI(out, XDAQ_ROOT + "/pixel/PixelUtilities/PixelDCSUtilities/html/CreateDataPointsTrkFEC_Voltages.htm");

  *out<<"<hr/>"<<endl;
  
  *out << "</form>" << endl;
  
  *out<<"<hr/>"<<endl;

  *out << cgicc::fieldset().set("style","font-size: 10pt;  font-family: arial;") << std::endl;
  *out << cgicc::legend("Progress Information") << cgicc::p() << std::endl;
  *out << cgicc::textarea().set("rows","20").set("cols","120") << std::endl;
  
  for ( std::list<std::string>::const_iterator textfieldEntry = textfieldEntries_.begin();
	textfieldEntry != textfieldEntries_.end(); ++textfieldEntry ) {
    *out << (*textfieldEntry) << std::endl;
  }

  *out << cgicc::textarea() << std::endl;
  *out << cgicc::fieldset() << std::endl;
  
  *out<<"</html>";
}

void PixelDCSCreateDataPoints::XgiHandler(xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception)
{
//--- retrieve password 
//    for access to Oracle data-base
  cgicc::Cgicc cgi(in);

  std::string Command = cgi.getElement("Command")->getValue();
  if ( Command == "CreateDataPointsFED" ) {
    xoap::MessageReference request = soapCommander_->MakeSOAPMessageReference("CreateDataPointsFED");
    xoap::MessageReference response = createDataPointsFED(request);
    if ( soapCommander_->Receive(response) != "CreateDataPointsFEDDone" ) {
      std::cout << "CreateDataPointsFED method failed !" << std::endl;
    }
  } else if ( Command == "CreateDataPointsTrkFEC_Temperatures" ) {
    xoap::MessageReference request = soapCommander_->MakeSOAPMessageReference("CreateDataPointsTrkFEC_Temperatures");
    xoap::MessageReference response = createDataPointsTrkFEC_Temperatures(request);
    if ( soapCommander_->Receive(response) != "CreateDataPointsTrkFEC_TemperaturesDone" ) {
      std::cout << "CreateDataPointsTrkFEC_Temperatures method failed !" << std::endl;
    }
  } else if ( Command == "CreateDataPointsTrkFEC_Voltages" ) {
    xoap::MessageReference request = soapCommander_->MakeSOAPMessageReference("CreateDataPointsTrkFEC_Voltages");
    xoap::MessageReference response = createDataPointsTrkFEC_Voltages(request);
    if ( soapCommander_->Receive(response) != "CreateDataPointsTrkFEC_VoltagesDone" ) {
      std::cout << "CreateDataPointsTrkFEC_Voltages method failed !" << std::endl;
    }
  } 

//--- re-display webpage
  this->Default(in, out);
}

//
//---------------------------------------------------------------------------------------------------
//

xoap::MessageReference PixelDCSCreateDataPoints::createDataPointsFED(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
//--- create list of PVSS data-points representing last DAC temperatures

  delete dpNameTable_;
  dpNameTable_ = new PixelDCSCreateDataPointsDpNameTable();

/*
  FOR TESTING ONLY
  dpNameTable_->addRow("testTemperature_deviceXDAQ01", "CMS_Pixel/testTemperature_logicalXDAQ01", "CMS_Pixel_Temperature_FEDerror");
  dpNameTable_->addRow("testTemperature_deviceXDAQ02", "CMS_Pixel/testTemperature_logicalXDAQ02", "CMS_Pixel_Temperature_FED");
  dpNameTable_->addRow("testTemperature_deviceXDAQ03", "CMS_Pixel/testTemperature_logicalXDAQ03", "CMS_Pixel_Temperature_FED");
 */

  PixelConfigKey globalConfigurationKey(defaultPixelConfigurationKey); 
  PixelNameTranslation* nameTranslation = 0;
  PixelConfigInterface::get(nameTranslation, "pixel/nametranslation/", globalConfigurationKey);
  if ( nameTranslation != NULL ) {
    std::list<const PixelROCName*> readOutChips = nameTranslation->getROCs();
    for ( std::list<const PixelROCName*>::const_iterator readOutChip = readOutChips.begin();
	  readOutChip != readOutChips.end(); ++readOutChip ) {
      const PixelHdwAddress* readOutChip_hardwareAddress = nameTranslation->getHdwAddress(**readOutChip);
      
      unsigned int fedBoardId = readOutChip_hardwareAddress->fednumber();
      unsigned int fedChannelId = readOutChip_hardwareAddress->fedchannel();
      unsigned int rocId = readOutChip_hardwareAddress->fedrocnumber();
      
      char hardwareName[100];
      sprintf(hardwareName, "cms_Pixel_fedBoardId%d02_channel%d02_roc%d03", fedBoardId, fedChannelId, rocId);

      unsigned int diskNumber = (*readOutChip)->disk();
      unsigned int bladeNumber = (*readOutChip)->blade();
      unsigned int panelNumber = (*readOutChip)->panel();
      unsigned int plaquetteNumber = (*readOutChip)->plaquet();
      unsigned int rocNumber = (*readOutChip)->roc();
      
      unsigned int readOutGroupNumber = (bladeNumber/3);
      
      char logicalName[100];
      sprintf(logicalName, "CMS_Pixel/HalfCylinder/D%d/ROG%d/BLD%d/PNL%d/PLQ%d/ROC%d", 
	      diskNumber, readOutGroupNumber, bladeNumber, panelNumber, plaquetteNumber, rocNumber);

      std::string dpName = hardwareName; // name of data-point in PVSS "hardware" view
      //std::string dpAlias = (*readOutChip)->rocname(); // name of data-point (alias) in PVSS "logical" view
      std::string dpAlias = logicalName;
      
      const std::string dpType = "CMS_Pixel_Temperature_FED";

      std::cout << "creating PVSS data-point of type " << dpType << ":" << std::endl;
      std::cout << " dpName = " << dpName << std::endl;
      std::cout << " dpAlias = " << dpAlias << std::endl;
      
      dpNameTable_->addRow(dpName, dpAlias, dpType);
    }
  } else {
    XCEPT_RAISE (xcept::Exception, "Access to PixelNameTranslation object faailed");
  }

//--- create Data-Points
  createDataPoints();

  return soapCommander_->MakeSOAPMessageReference("CreateDataPointsFEDDone");
}
	
xoap::MessageReference PixelDCSCreateDataPoints::createDataPointsTrkFEC_Temperatures(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
//--- create list of PVSS data-points representing temperatures
//    read-out via the DCU chips mounted on the portcards

  delete dpNameTable_;
  dpNameTable_ = new PixelDCSCreateDataPointsDpNameTable();

  //... (Implementation very similar to createDataPointsFED)

//--- create Data-Points
  createDataPoints();

  return soapCommander_->MakeSOAPMessageReference("CreateDataPointsTrkFEC_TemperaturesDone");
}
	
xoap::MessageReference PixelDCSCreateDataPoints::createDataPointsTrkFEC_Voltages(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
//--- create list of PVSS data-points representing voltages
//    read-out via the DCU chips mounted on the portcards

  delete dpNameTable_;
  dpNameTable_ = new PixelDCSCreateDataPointsDpNameTable();

  //... (Implementation very similar to createDataPointsFED)

//--- create Data-Points
  createDataPoints();

  return soapCommander_->MakeSOAPMessageReference("CreateDataPointsTrkFEC_VoltagesDone");
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelDCSCreateDataPoints::createDataPoints()
{
  if ( dpNameTable_ != NULL ) {
//--- create a new work-loop
//    if not already active
    std::string workloopName = std::string("PixelDCSCreateDataPoints_workloop");
    
    if ( !(workloopStatus_ == "inactive") ){
      std::cout << " work-loop still active --> skipping registration of createDataPoints_workloop member-function" << std::endl;
      return;
    }
    
    toolbox::task::WorkLoop* workloop = toolbox::task::getWorkLoopFactory()->getWorkLoop(workloopName, "waiting");

//--- register createDataPoints_workloop member-function for execution in work-loop
    std::cout << " registering member-function \"createDataPoints_workloop\" for execution in work-loop \"" << workloopName << "\"" << std::endl;
    toolbox::task::ActionSignature* task = toolbox::task::bind(this, &PixelDCSCreateDataPoints::createDataPoints_workloop, "createDataPoints_workloop");

    workloop->submit(task);

//--- start execution of connectPSX_workloop member-function in work-loop
//    (start work-loop only if not already running)
    if ( !workloop->isActive() ) workloop->activate();
  } else {
    std::cerr << "Error in <PixelDCSCreateDataPoints::createDataPoints>: dpNamesTable not initialized !" << std::endl;
  }
}

bool PixelDCSCreateDataPoints::createDataPoints_workloop(toolbox::task::WorkLoop* workloop)
{
  workloopStatus_ = "active";

  for ( std::list<PixelDCSCreateDataPointsDpNameRow>::const_iterator dpName = dpNameTable_->getRows().begin();
	dpName != dpNameTable_->getRows().end(); ++dpName ) {

    std::string text = "creating data-point " + dpName->getHardwareName();
    textfieldEntries_.push_back(text);

    const std::string& hardwareName = dpName->getHardwareName();
    const std::string& logicalName = dpName->getLogicalName();
    const std::string& deviceType = dpName->getDeviceType();

//--- wait until PVSS is ready 
//    to process the next data-point
    bool isLocked_byPVSS = pvssCommander_->getDpeValue_bool(pvssCreateDataPointName + ".isLocked_byPVSS");
    while ( isLocked_byPVSS ) {
      long numMilliSeconds = 10;
      clock_t wake_upTime = clock() + numMilliSeconds;
      while ( clock() < wake_upTime );
      isLocked_byPVSS = pvssCommander_->getDpeValue_bool(pvssCreateDataPointName + ".isLocked_byPVSS");
    }

    pvssCommander_->setDpeValue_bool(pvssCreateDataPointName + ".isLocked_byXDAQ", true);
    pvssCommander_->setDpeValue(pvssCreateDataPointName + ".hardwareName", hardwareName);
    pvssCommander_->setDpeValue(pvssCreateDataPointName + ".logicalName", logicalName);
    pvssCommander_->setDpeValue(pvssCreateDataPointName + ".deviceType", deviceType);
    pvssCommander_->setDpeValue_bool(pvssCreateDataPointName + ".isCreated", false);
    pvssCommander_->setDpeValue_bool(pvssCreateDataPointName + ".isLocked_byXDAQ", false);
    pvssCommander_->setDpeValue(pvssCreateDataPointName + ".errorPVSS", "-");

//--- wait until PVSS has created the data-point
//    before sending the next one
    std::string pvssError = "-";
    while ( pvssError == "-" && pvssCommander_->getDpeValue_bool(pvssCreateDataPointName + ".isCreated") == false ) {
      long numMilliSeconds = 10;
      clock_t wake_upTime = clock() + numMilliSeconds;
      while ( clock() < wake_upTime );
      pvssError = pvssCommander_->getDpeValue(pvssCreateDataPointName + ".errorPVSS");
    }

    if ( pvssError != "-" ) {
      textfieldEntries_.push_back(string("Error: ") + pvssError);
    }
  }

//--- inform PVSS 
// that all data-points have been created
  pvssCommander_->setDpeValue(pvssCreateDataPointName + ".deviceType", "DONE");

  workloopStatus_ = "inactive";

//--- stop execution of work-loop
//    (work-loop gets automatically stopped once member-function 
//     registered for execution in work-loop returns false)
  return false;
}
