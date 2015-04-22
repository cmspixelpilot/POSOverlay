#include "PixelDCSInterface/include/PixelDCSDpInterface.h"

/*************************************************************************
 * Base class for PixelDCStoFECDpInterface and PixelDCStoFEDDpInterface; *
 * implements access to Oracle DataBase                                  *
 *                                                                       *
 * Author: Christian Veelken, UC Davis				         *
 *                                                                       *
 * Last update: $Date: 2010/04/30 15:20:19 $ (UTC)                       *
 *          by: $Author: joshmt $                                       *
 *************************************************************************/

#include <iomanip>
#include <vector>

#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationStub.h"
#include "xgi/Utils.h"
#include "cgicc/HTMLClasses.h"
#include "toolbox/fsm/FailedEvent.h"
#include "tstore/client/AttachmentUtils.h"
#include "tstore/client/Client.h"

#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"

#include "PixelUtilities/PixelGUIUtilities/include/HTML2XGI.h"

#define PSX_NS_URI "http://xdaq.cern.ch/xdaq/xsd/2006/psx-pvss-10.xsd"

#define TSTORE_NS_URI "http://xdaq.web.cern.ch/xdaq/xsd/2006/tstore-10.xsd"

XDAQ_INSTANTIATOR_IMPL(PixelDCSDpInterface)

using std::cout;        using std::endl;

void addDp(const std::string& dpeName, const std::string& dpeValue,
	   std::list<PixelDCSPVSSDpe>& dpeList);

PixelDCSDpInterface::PixelDCSDpInterface(xdaq::ApplicationStub* s) throw (xcept::Exception) 
  : xdaq::Application(s)
  , SOAPCommander(this)
  , fsm_("urn:toolbox-task-workloop:PixelDCSDpInterface") //unclear to me if the value of this string is important
  //, debug_(false)
  , debug_(true)
{
  //--- define the states 
  //    of the Finite State Machine
  //    (call-back functions need to be initialized in derrived classes)
  fsm_.addState('H', "Halted",this,&PixelDCSDpInterface::stateChanged);
  fsm_.addState('c', "Configuring",this,&PixelDCSDpInterface::stateConfiguring);
  fsm_.addState('C', "Configured",this,&PixelDCSDpInterface::stateChanged);
  fsm_.setStateName('F',"Error");
  //fsm_.setFailedStateTransitionAction(this, &PixelDCSDpInterface::enteringError);

  //--- define the state transitions
  //    of the Finite State Machine
  fsm_.addStateTransition('H', 'c', "Configure");
  fsm_.addStateTransition('c', 'C', "ConfigureDone");
  fsm_.addStateTransition('c', 'H', "Halt");
  fsm_.addStateTransition('C', 'H', "Halt");
  fsm_.addStateTransition('H', 'H', "Halt");
  fsm_.setInitialState('H');
  fsm_.reset();

  httpPageHeader_ = "Pixel DCS Dp Interface";
	
  // Initialize parameters defined by environment variables
  XDAQ_ROOT = getenv("XDAQ_ROOT");
  
  // Initialize parameters defined by .xml configuration file
  this->getApplicationInfoSpace()->fireItemAvailable("dpValueUpdate_maxLength", &dpValueUpdate_maxLength_);
  this->getApplicationInfoSpace()->fireItemAvailable("dpName_status", &dpName_status_);

  this->getApplicationInfoSpace()->fireItemAvailable("oracleUserName", &oracleUserName_);
  this->getApplicationInfoSpace()->fireItemAvailable("oraclePassword", &oraclePassword_);
  this->getApplicationInfoSpace()->fireItemAvailable("version_dcu_calib_filter", &version_dcu_calib_filter_);
  
  this->getApplicationInfoSpace()->fireItemAvailable("oracleViewName_dpNames", &oracleViewName_dpNames_);
  this->getApplicationInfoSpace()->fireItemAvailable("oracleTableName_dpNames", &oracleTableName_dpNames_);
  this->getApplicationInfoSpace()->fireItemAvailable("oracleViewName_dpFilter", &oracleViewName_dpFilter_);
  this->getApplicationInfoSpace()->fireItemAvailable("oracleTableName_dpFilter", &oracleTableName_dpFilter_);
  this->getApplicationInfoSpace()->fireItemAvailable("oracleViewName_dpCalibration", &oracleViewName_dpCalibration_);
  this->getApplicationInfoSpace()->fireItemAvailable("oracleTableName_dpCalibration", &oracleTableName_dpCalibration_);

  oracleConnectionId_ = "";
}

PixelDCSDpInterface::~PixelDCSDpInterface()
{
//--- nothing to be done yet
}

void PixelDCSDpInterface::stateChanged(toolbox::fsm::FiniteStateMachine &fsm)
{
  //  cout<<"[PixelDCSDpInterface::stateChanged] *** enter ***"<<endl;
  xdata::String state = fsm.getStateName(fsm.getCurrentState()) ;
  
  try {
    xdaq::ApplicationDescriptor*   PixelSupervisor = getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor("PixelSupervisor", 0);
    
    if (PixelSupervisor !=0 ) {
      Attribute_Vector parameters(3);
      parameters[0].name_="Supervisor"; parameters[0].value_="PixelDCSDpInterface";
      parameters[1].name_="Instance";   parameters[1].value_=itoa(0); //FIXME hardcoded instance
      parameters[2].name_="FSMState";   parameters[2].value_=state;
      Send(PixelSupervisor, "FSMStateNotification", parameters);
    }
    else {XCEPT_RAISE(xdaq::exception::Exception,"PixelSupervisor pointer is null!");} //if this happens then something is messed up
  } catch (xdaq::exception::Exception & e) {
    //i don't think we're setup to use the diagSystem here, unfortunately
    cout<<"[PixelDCSDpInterface::stateChanged] ERROR -- caught exception: "<<e.what()<<endl;
  }

  cout<< "[PixelDCSDpInterface::stateChanged] New state is:" <<std::string(state)<<endl;


}

void PixelDCSDpInterface::Default (xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception)
{
//   *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
//   *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
//   xgi::Utils::getPageHeader(*out, httpPageHeader_, fsm_.getStateName(fsm_.getCurrentState()));
// 
//   // Rendering the State Machine GUI
// 
//   std::set<std::string> allInputs = fsm_.getInputs();
//   std::set<std::string> clickableInputs = fsm_.getInputs(fsm_.getCurrentState());
//   
//   *out<<"<h2> Finite State Machine </h2>";
// 
//   std::string url = "/" + getApplicationDescriptor()->getURN() + "/XgiHandler";
//   *out << "<form name=\"input\" method=\"get\" action=\"" << url << "\" enctype=\"multipart/form-data\">";
//   
//   *out << "<table border cellpadding=10 cellspacing=0>";
//   *out << "<tr>";
//   *out << "<td> <b>Current State</b> <br/>" << fsm_.getStateName(fsm_.getCurrentState()) << "</td>";
//   *out << "<td>" << "Global Key: <input type=\"text\" name=\"GlobalKey\"/>" << "</td>";
//   //*out << "<td colspan=2>";
//   //for ( std::set<std::string>::iterator input = allInputs.begin(); input != allInputs.end(); ++input ) {
//   //  if ( clickableInputs.find(*input) != clickableInputs.end() ) HTML2XGI(out, XDAQ_ROOT + "/pixel/PixelDCSInterface/html/" + (*input) + ".htm");
//   //}
//   //*out << "</td>" << std::endl;
//   *out << "</tr>" << std::endl;
//   *out << "<tr>" << std::endl;
//   
//   for ( std::set<std::string>::iterator input = allInputs.begin(); input != allInputs.end(); ++input ) {
//     *out << "<td>";
//     if ( clickableInputs.find(*input) != clickableInputs.end() )
//       *out << "<input type=\"submit\" name=\"Command\" value=\"" << (*input) << "\"/>";
//     else
//       *out << "<input type=\"submit\" disabled=\"true\" name=\"Command\" value=\"" << (*input) << "\"/>";
//     *out << "</td>" << std::endl;
//   }
//   
//   *out << "</tr>";
//   *out << "</table>";
//   
//   *out<<"<hr/>"<< std::endl;
// 
//   *out<<"<h3> Enter Oracle username and password if they are different from provided by config file </h3>";
// 
//   *out<<"<h3> Username for Access to Oracle Data-Base    ( Hint: CMS_PXL_PIXEL_R ) </h3>";
//   
//   *out << cgicc::br() << "username: " << cgicc::input().set("type", "text").set("name", "oracleUserName") << std::endl;
//   
//   *out<<"<h3> Password for Access to Oracle Data-Base </h3>";
//   
//   *out << cgicc::br() << "Password: " << cgicc::input().set("type", "password").set("name", "oraclePassword") << std::endl;
// 
//   *out << "</form>" << std::endl;
//   
//   *out<<"<hr/>"<< std::endl;
//   
//   //*out<<"</html>";
}

void PixelDCSDpInterface::XgiHandler (xgi::Input* in, xgi::Output* out) throw (xgi::exception::Exception)
{
// //--- retrieve password 
// //    for access to Oracle data-base
//   cgicc::Cgicc cgi(in);
//   
//   cgicc::FormEntry cgiFormEntry_username = (*cgi.getElement("oracleUserName"));
//   std::string username = cgiFormEntry_username.getValue();
//   if (username.size()) {
//     cout<< "PixelDCSDpInterface::XgiHandler: Using manually entered username " << username <<endl;
//     oracleUserName_ = username;
//   }
//   
//   cgicc::FormEntry cgiFormEntry_password = (*cgi.getElement("oraclePassword"));
//   std::string password = cgiFormEntry_password.getValue();
//   if (password.size()) {
//     cout<< "PixelDCSDpInterface::XgiHandler: Using manually entered password" <<endl;
//     oraclePassword_ = password;
//   }
//  
// //--- retrieve commands 
// //    to Finite State Machine 
//   std::string Command = cgi.getElement("Command")->getValue();
//   if ( Command == "Configure" ) {
//     Attribute_Vector parametersXgi(1);
//     parametersXgi.at(0).name_ = "GlobalKey"; 
//     parametersXgi.at(0).value_ = cgi.getElement("GlobalKey")->getValue();
// 
//     xoap::MessageReference msg = MakeSOAPMessageReference("Configure", parametersXgi);
//     xoap::MessageReference response = Configure(msg);
//     if ( Receive(response) != "ConfigureDone" ) std::cout << "The PixelDCSDpInterface could not be Configured" << std::endl;
//   } else if ( Command == "Halt" ) {
//     xoap::MessageReference msg = MakeSOAPMessageReference("Halt");
//     xoap::MessageReference response = Halt(msg);
//     if ( Receive(response) != "HaltDone" ) std::cout << "The PixelDCSDpInterface could not be Halted" << std::endl;
//   }
// 	
// //--- re-display webpage
//   this->Default(in, out);
}

xoap::MessageReference PixelDCSDpInterface::Halt (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  //
  //  AZ: This method is not in use and should be removed from this class
  //

  /*
    Delete and clear all your objects here.
  */
  
  try {
    toolbox::Event::Reference e(new toolbox::Event("Halt", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    XCEPT_RETHROW(xoap::exception::Exception, "Invalid Command.", e);
  }

  xoap::MessageReference reply = MakeSOAPMessageReference("HaltDone");
  return reply;
}

void PixelDCSDpInterface::connectOracleDB(const std::string& oracleViewName, const std::string& oraclePassword) throw (xcept::Exception)
{	
  //--- open connection 
  //    to Oracle data-base;
  //    store connection Identifier
  std::cout << "<PixelDCSDpInterface::connectOracleDB>:" << std::endl;

  //--- compose connect message
  //    sent to TStore  NB: new version: substitute tstore" by "tstoresoap"
  xoap::MessageReference soapRequest = xoap::createMessage();
  xoap::SOAPEnvelope envelope = soapRequest->getSOAPPart().getEnvelope();
  xoap::SOAPName command = envelope.createName("connect", "tstoresoap", TSTORE_NS_URI);
  xoap::SOAPElement commandElement = envelope.getBody().addBodyElement(command);
  xoap::SOAPName connectionId = envelope.createName("id", "tstoresoap", TSTORE_NS_URI);
  commandElement.addAttribute(connectionId, oracleViewName);

  // the attribute authentication="basic" is mandatory
  //xoap::SOAPName authentication = envelope.createName("authentication", "tstoresoap", "http://xdaq.web.cern.ch/xdaq/xsd/2006/tstore-10.xsd");
  xoap::SOAPName authentication = envelope.createName("authentication", "tstoresoap", TSTORE_NS_URI);
  commandElement.addAttribute(authentication, "basic");
          
  // with authentication mode "basic" the credentials should be in the form username/password
  xoap::SOAPName credentials = envelope.createName("credentials", "tstoresoap", TSTORE_NS_URI);
  //-- std::string username_password = "JOSHI_TEST/";
  std::string username_password = oracleUserName_.toString() + "/" + oraclePassword_.toString();
  commandElement.addAttribute(credentials, username_password);
            
  // add the timeout
  // timeout is in W3C time format, e.g. "PT60S" means 60 seconds.
  // std::string timeout = "PT60S";
  std::string timeout = "PT300S";
  xoap::SOAPName timeoutName = envelope.createName("timeout", "tstoresoap", TSTORE_NS_URI);
  commandElement.addAttribute(timeoutName, timeout);
  
  if (debug_) std::cout << " Request : ------------------------------------ " << std::endl;
  if (debug_) soapRequest->writeTo(std::cout);
  if (debug_) std::cout << std::endl;
  if (debug_) std::cout << " ---------------------------------------------- " << std::endl;
  
  //--- send connect message
  //    to TStore
  try {
    xdaq::ApplicationDescriptor* tstoreDescriptor = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor("tstore::TStore", 0);

    //xoap::MessageReference soapResponse = getApplicationContext()->postSOAP(soapRequest, tstoreDescriptor);
    PixelTimer connectTimer;
    connectTimer.start();
    xoap::MessageReference soapResponse = this->getApplicationContext()->postSOAP(soapRequest, *this->getApplicationDescriptor(), *tstoreDescriptor);
    connectTimer.stop();
    std::cout<<"[PixelDCSDpInterface] DB connection time = "<<connectTimer.tottime()<<std::endl;

    //if (debug_) std::cout << " Response : ------------------------------------ " << std::endl;
    //if (debug_) soapResponse->writeTo(std::cout);
    //if (debug_) std::cout << std::endl;
    //if (debug_) std::cout << " ----------------------------------------------- " << std::endl;

    //--- begin unpacking SOAP message
    //    received from TStore
    xoap::SOAPEnvelope envelope = soapResponse->getSOAPPart().getEnvelope();
    xoap::SOAPBody body = envelope.getBody();

    //--- create SOAPName objects 
    //    for keywords searched for in SOAP message
    xoap::SOAPName command = envelope.createName("connectResponse");
    xoap::SOAPName connectionIdAttribute = envelope.createName("connectionID");

    //--- find within body 
    //    response to "connect" command
    std::vector<xoap::SOAPElement> bodyElements = body.getChildElements(command);          	  
    for ( std::vector<xoap::SOAPElement>::iterator bodyElement = bodyElements.begin();
	  bodyElement != bodyElements.end(); ++bodyElement ) {
      oracleConnectionId_ = bodyElement->getAttributeValue(connectionIdAttribute);
    }

    // use the same way as getOracleTable, see below
    xoap::SOAPBody body_response = soapResponse->getSOAPPart().getEnvelope().getBody();
    if (body_response.hasFault()) {
      if (fault_message_db_connection_.str().empty()) {
        fault_message_db_connection_
          << "PixelDCSDpInterface::connectOracleDB: Connection fault: "
          << endl
          << body.getFault().getFaultString()
          << endl
          << endl
          << "username: " << oracleUserName_.toString()
          << endl
          << "password: " << oraclePassword_.toString()
        ;
      }
      cout<< "// PixelDCSDpInterface::connectOracleDB: SOAP body.getFault().getFaultString(): " << body_response.getFault().getFaultString() <<endl;
      // cout SOAP response in case of fault
      cout<< "// PixelDCSDpInterface::connectOracleDB: soapResponse : ------------------------------------ " <<endl;
      soapResponse->writeTo(std::cout);
      cout << endl;
      cout<< "// PixelDCSDpInterface::connectOracleDB: end of soapResponse ------------------------------- " <<endl;
    }
    else {
      //fault_message_db_connection_.str("");
      if (debug_) cout<< "PixelDCSDpInterface::connectOracleDB: SOAP response is fine" <<endl;
      if (debug_) cout<< "// PixelDCSDpInterface::connectOracleDB: soapResponse : ------------------------------------ " <<endl;
      if (debug_) soapResponse->writeTo(std::cout);
      if (debug_) cout << endl;
      if (debug_) cout<< "// PixelDCSDpInterface::connectOracleDB: end of soapResponse ------------------------------- " <<endl;
    }
  } catch ( xcept::Exception& e ) {
    std::cout << "Caught Exception:" << std::endl;
    std::cout << " " << e.what() << std::endl;
    XCEPT_RETHROW (xcept::Exception, "Failed to connect to Oracle Data-Base: " + std::string(e.what()), e);
  }
}

xdata::Table PixelDCSDpInterface::getOracleTable(const std::string& oracleViewName, const std::string& oracleTableName) throw (xcept::Exception)
{
  cout<< "<PixelDCSDpInterface::getOracleTable>:" <<endl;
  cout<< " connectionId = " << oracleConnectionId_ <<endl;
  cout<< " oracleViewName = " << oracleViewName <<endl;
  cout<< " oracleTableName  " << oracleTableName <<endl;

  if (oracleConnectionId_.empty()) {
    // no connection was established
    cout<< "//-- PixelDCSDpInterface::getOracleTable: will not try to obtain table " << oracleTableName << " because of no connection with Oracle: oracleConnectonId_ = \"" << oracleConnectionId_ << "\"" <<endl;
  }

  //--- retrieve table containing information about dpNames, dpFilter or dpCalibration
  //    from Oracle data-base
  xdata::Table results;

  //--- compose query message
  //    sent to TStore
  xoap::MessageReference soapRequest = xoap::createMessage();
  xoap::SOAPEnvelope envelope = soapRequest->getSOAPPart().getEnvelope();
  xoap::SOAPName command = envelope.createName("query", "tstoresoap", TSTORE_NS_URI);
  xoap::SOAPElement commandElement = envelope.getBody().addBodyElement(command);
  //xoap::SOAPName viewId = envelope.createName("id", "tstoresoap", TSTORE_NS_URI);
  //commandElement.addAttribute(viewId, oracleViewName);
  xoap::SOAPName connectionId = envelope.createName("connectionID", "tstoresoap", TSTORE_NS_URI);
  commandElement.addAttribute(connectionId, oracleConnectionId_);
  
  commandElement.addNamespaceDeclaration("sql", "urn:tstore-view-SQL"); 
  xoap::SOAPName table = envelope.createName("name", "sql","urn:tstore-view-SQL");
  commandElement.addAttribute(table, oracleTableName); 
  
  if (debug_) std::cout << " Request : ------------------------------------ " << std::endl;
  if (debug_) soapRequest->writeTo(std::cout);
  if (debug_) std::cout << std::endl;
  if (debug_) std::cout << " ---------------------------------------------- " << std::endl;

  //--- send query message
  //    to TStore
  try {
    xdaq::ApplicationDescriptor* tstoreDescriptor = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor("tstore::TStore", 0);

    //xoap::MessageReference soapResponse = getApplicationContext()->postSOAP(request, tstoreDescriptor);
    PixelTimer dbtimer;
    dbtimer.start();
    xoap::MessageReference soapResponse = this->getApplicationContext()->postSOAP(soapRequest, *this->getApplicationDescriptor(), *tstoreDescriptor);
    dbtimer.stop();
    std::cout<<"[PixelDCSDpInterface] DB table query time = "<<dbtimer.tottime()<<std::endl;
    
    //if (debug_) std::cout << "// PixelDCSDpInterface::getOracleTable: Response : ------------------------------------ " <<endl;
    //if (debug_) soapResponse->writeTo(std::cout);
    //if (debug_) std::cout << std::endl;
    //if (debug_) std::cout << "// PixelDCSDpInterface::getOracleTable: End of Response : ----------------------------------------------- " <<endl;

    xoap::SOAPBody body = soapResponse->getSOAPPart().getEnvelope().getBody();
    if (body.hasFault()) {
      //if (fault_string_db_get_table_.str().empty())
      {
        fault_string_db_get_table_
          //<< "PixelDCSDpInterface::getOracleTable: view: " << oracleViewName
          << "view: " << oracleViewName << endl
          << "table: " << oracleTableName << endl
          << body.getFault().getFaultString() << endl
          << endl
        ;
      }
      cout<< "// PixelDCSDpInterface::getOracleTable: SOAP body.getFault().getFaultString(): " << body.getFault().getFaultString() <<endl;
      cout<< "// PixelDCSDpInterface::getOracleTable: Response : ------------------------------------ " <<endl;
      soapResponse->writeTo(std::cout);
      cout << endl;
      cout<< "// PixelDCSDpInterface::getOracleTable: End of Response : ----------------------------------------------- " <<endl;
    }
    else {
      //fault_message_db_connection_.str("");
      if (debug_) cout<< "PixelDCSDpInterface::getOracleTable: SOAP response is fine" <<endl;
      if (debug_) cout<< "// PixelDCSDpInterface::getOracleTable: Response : ------------------------------------ " <<endl;
      if (debug_) soapResponse->writeTo(std::cout);
      if (debug_) cout << endl;
      if (debug_) cout<< "// PixelDCSDpInterface::getOracleTable: End of Response : ----------------------------------------------- " <<endl;
    }

    //--- check if TStore returned Oracle table
    //    in attachment
    if ( !tstoreclient::getFirstAttachmentOfType(soapResponse, results) ) {
      XCEPT_RAISE (xcept::Exception, "Server returned no data");
    }
  } catch ( xcept::Exception& e ) {
    std::cout << "//-- PixelDCSDpInterface::getOracleTable: Caught Exception:" << std::endl;
    std::cout << " " << e.what() << std::endl;
    XCEPT_RETHROW (xcept::Exception, "Failed to query Oracle Data-Base: " + std::string(e.what()), e);
  }
  
  return results;
}

void PixelDCSDpInterface::disconnectOracleDB(const std::string& oracleViewName) throw (xcept::Exception)
{
  //--- close connection 
  //    to Oracle data-base
  std::cout << "<PixelDCSDpInterface::disconnectOracleDB>:" << std::endl;

  //--- compose disconnect message
  //    sent to TStore
  xoap::MessageReference soapRequest = xoap::createMessage();
  xoap::SOAPEnvelope envelope = soapRequest->getSOAPPart().getEnvelope();
  xoap::SOAPName command = envelope.createName("disconnect", "tstoresoap", TSTORE_NS_URI);
  xoap::SOAPElement commandElement = envelope.getBody().addBodyElement(command);
  //xoap::SOAPName connectionId = envelope.createName("id", "tstoresoap", TSTORE_NS_URI);
  //commandElement.addAttribute(connectionId, oracleViewName);
  xoap::SOAPName connectionId = envelope.createName("connectionID", "tstoresoap", TSTORE_NS_URI);
  commandElement.addAttribute(connectionId, oracleConnectionId_);

  if (debug_) std::cout << " Request : ------------------------------------ " << std::endl;
  if (debug_) soapRequest->writeTo(std::cout);
  if (debug_) std::cout << std::endl;
  if (debug_) std::cout << " ---------------------------------------------- " << std::endl;

  //--- send disconnect message
  //    to TStore
  try {
    xdaq::ApplicationDescriptor* tstoreDescriptor = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor("tstore::TStore", 0);

    //xoap::MessageReference soapResponse = getApplicationContext()->postSOAP(request, tstoreDescriptor);
    PixelTimer dbtimer;
    dbtimer.start();
    xoap::MessageReference soapResponse = this->getApplicationContext()->postSOAP(soapRequest, *this->getApplicationDescriptor(), *tstoreDescriptor);
    dbtimer.stop();
    std::cout<<"[PixelDCSDpInterface] DB disconnect time = "<<dbtimer.tottime()<<std::endl;

    if (debug_) std::cout << " Response : ------------------------------------ " << std::endl;
    if (debug_) soapResponse->writeTo(std::cout);
    if (debug_) std::cout << std::endl;
    if (debug_) std::cout << " ----------------------------------------------- " << std::endl;

    oracleConnectionId_ = "";
  } catch ( xcept::Exception& e ) {
    std::cout << "Caught Exception:" << std::endl;
    std::cout << " " << e.what() << std::endl;
    XCEPT_RETHROW (xcept::Exception, "Failed to disconnect from Oracle Data-Base: " + std::string(e.what()), e);
  }
}

void addStatus(const PixelDCSDpInterfaceStatus& status, const std::string& dpName_status,
	       std::list<PixelDCSPVSSDpe>& dpeList)
{
  std::string dpeName_dpRate = dpName_status + ".readings.dpRate:original..value";
  std::string dpeName_dpLoad = dpName_status + ".readings.dpLoad:original..value";
  std::string dpeName_heartbeat = dpName_status + ".readings.heartbeat:original..value";
  std::string dpeName_busy = dpName_status + ".readings.isBusy:original..value";
  std::string dpeName_error = dpName_status + ".readings.error:original..value";

  const std::string true_string = "TRUE";
  //const std::string true_string = "1"; // over here
  //const std::string false_string = "FALSE";
  const std::string false_string = "0";
  
  std::ostringstream dpeValue_dpRate, dpeValue_dpLoad;
  std::string dpeValue_heartbeat, dpeValue_busy, dpeValue_error;
  dpeValue_dpRate.setf(std::ios::fixed);
  dpeValue_dpRate << std::setprecision(1) << status.dpRate_;
  dpeValue_dpLoad.setf(std::ios::fixed);
  dpeValue_dpLoad << std::setprecision(1) << status.dpRate_;
  dpeValue_heartbeat = (status.heartbeat_ == true) ? true_string : false_string;
  dpeValue_busy = (status.busy_ == true) ? true_string : false_string;
  dpeValue_error = (status.error_ == true) ? true_string : false_string;

  addDp(dpeName_dpRate, dpeValue_dpRate.str(), dpeList);
  addDp(dpeName_dpLoad, dpeValue_dpLoad.str(), dpeList);
  //
  //--- WARNING: program aborts
  //             if trying to send data-point element of boolean type 
  //            --> boolean data-point elements not implemented in XDAQ/PVSS API yet;
  //                need to use data-point elements of integer type instead !!!
  //
  //addDp(dpeName_heartbeat, dpeValue_heartbeat, dpeList);
  //addDp(dpeName_busy, dpeValue_busy, dpeList);
  //addDp(dpeName_error, dpeValue_error, dpeList);
}

void addDp(const std::string& dpeName, const std::string& dpeValue,
	   std::list<PixelDCSPVSSDpe>& dpeList)
{
  PixelDCSPVSSDpe dpe(dpeName, dpeValue);
  dpeList.push_back(dpe);
}
