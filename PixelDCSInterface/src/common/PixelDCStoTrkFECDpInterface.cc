/*************************************************************************
 * Interface for the Forward Pixel Detector                              *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Author: Andres Florez				         	 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelDCSInterface/include/PixelDCStoTrkFECDpInterface.h"

#include <sstream>
#include <iostream>
#include <map>
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <iomanip>

#include "PixelUtilities/PixelGUIUtilities/include/HTML2XGI.h"

#include "PixelUtilities/PixelTKFECDataTools/include/PortCardDCU.h"

#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSSOAPCommander.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSPVSSCommander.h"

#define PSX_NS_URI "http://xdaq.cern.ch/xdaq/xsd/2006/psx-pvss-10.xsd"

XDAQ_INSTANTIATOR_IMPL(PixelDCStoTrkFECDpInterface)

using std::cout;      
using std::endl;

PixelDCStoTrkFECDpInterface::PixelDCStoTrkFECDpInterface(xdaq::ApplicationStub * s) throw (xcept::Exception) 
  : PixelDCSDpInterface(s)
  , psx_system_name_("cms_trk_dcs_1:")//old cms_trk_dcs_07
  , weight_(0.5)
  //, debug_(false)
  , debug_(true) //over here
  , nsent_(0)
  //, nreport_(10) //over here
  , nreport_(1)
  , send_to_PSX_(true)
  // , send_to_PSX_(false)
  , dcu_printout_on_(false)
{
  cout<<"debug PixelDCSFSMInterface::connectToFSM_workloop pass00 weight of the current value:"<<weight_<<" nsent_:"<<nsent_
      <<" nreport_:"<<nreport_<<" send_to_PSX_:"<<send_to_PSX_<<endl;
  cout<< "PixelDCStoTrkFECDpInterface::PixelDCStoTrkFECDpInterface: weight of the current value = " << weight_ <<endl;
  cout<< "PixelDCStoTrkFECDpInterface::PixelDCStoTrkFECDpInterface: debug_ = " << debug_ <<endl;
  cout<< "PixelDCStoTrkFECDpInterface::PixelDCStoTrkFECDpInterface: nsent_ = " << nsent_ <<endl;
  cout<< "PixelDCStoTrkFECDpInterface::PixelDCStoTrkFECDpInterface: nreport_ = " << nreport_ <<endl;
  cout<< "PixelDCStoTrkFECDpInterface::PixelDCStoTrkFECDpInterface: send_to_PSX_ = " << send_to_PSX_ <<endl;
  
  // Binding SOAP Callbacks to State Machine Commmands
  xoap::bind(this, &PixelDCStoTrkFECDpInterface::Configure, "Configure", XDAQ_NS_URI );
  xoap::bind(this, &PixelDCStoTrkFECDpInterface::Halt, "Halt", XDAQ_NS_URI );
     
  // SOAP Bindings to PixelDCSMessage. PixelDCSMessage is the XDAQ  application that I use to 
  // send messages to the PixelDCStoTrkFECDpInterface.
  xoap::bind(this, &PixelDCStoTrkFECDpInterface::updateDpValueTrkFEC, PortCard::SOAP_ReadAll::name_command_, XDAQ_NS_URI);

  // Binding XGI Callbacks for messages from the browser
  xgi::bind(this, &PixelDCStoTrkFECDpInterface::Default, "Default");
  xgi::bind(this, &PixelDCStoTrkFECDpInterface::XgiHandler, "XgiHandler");

  // Initialize parameters defined by .xml configuration file

  // WARNING: parameters defined by .xml configuration file are not initialized yet in constructor;

  this->getApplicationInfoSpace()->fireItemAvailable("psx_system_name", &psx_system_name_);

  // Miscellaneous variables most of which I do not understand
  XDAQ_ROOT=getenv("XDAQ_ROOT");

  httpPageHeader_ = "Pixel DCS to Trk FEC Dp Interface";
}

PixelDCStoTrkFECDpInterface::~PixelDCStoTrkFECDpInterface()
{
}

void PixelDCStoTrkFECDpInterface::Default (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{
  cout<< "PixelDCStoTrkFECDpInterface::Default" <<endl;

  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  xgi::Utils::getPageHeader(*out, httpPageHeader_, fsm_.getStateName(fsm_.getCurrentState()));

  // Rendering the State Machine GUI

  std::set<std::string> allInputs = fsm_.getInputs();
  std::set<std::string> clickableInputs = fsm_.getInputs(fsm_.getCurrentState());
  
  *out<<"<h2> Finite State Machine </h2>";

  std::string url = "/" + getApplicationDescriptor()->getURN() + "/XgiHandler";
  *out << "<form name=\"input\" method=\"get\" action=\"" << url << "\" enctype=\"multipart/form-data\">";
  
  *out << "<table border cellpadding=10 cellspacing=0>";
  *out << "<tr>";
  *out << "<td> <b>Current State</b> <br/>" << fsm_.getStateName(fsm_.getCurrentState()) << "</td>";
  *out << "<td>" << "Global Key: <input type=\"text\" name=\"GlobalKey\"/>" << "</td>";
  *out << "</tr>" << std::endl;
  *out << "<tr>" << std::endl;
  
  if (debug_) cout<< "the number of clickable inputs are " << clickableInputs.size() <<endl;
  for ( std::set<std::string>::const_iterator input = allInputs.begin(); input != allInputs.end(); ++input ) {
    if (debug_) cout<< std::distance<std::set<std::string>::const_iterator>(allInputs.begin(), input) << " input " << *input;
    *out << "<td>";
    if ( clickableInputs.find(*input) != clickableInputs.end() ) {
      *out << "<input type=\"submit\" name=\"Command\" value=\"" << (*input) << "\"/>";
      if (debug_) cout<< ": clickable" <<endl;
    }
    else {
      *out << "<input type=\"submit\" disabled=\"TRUE\" name=\"Command\" value=\"" << (*input) << "\"/>";
      if (debug_) cout<< ": not clickable" <<endl;
    }
    *out << "</td>" << std::endl;
  }
  
  *out << "</tr>";
  *out << "</table>";
  
  *out<<"<hr/>"<< std::endl;

  *out<<"<h3> Enter Oracle username and password if they are different from provided by config file </h3>";

  *out<<"<h3> Username for Access to Oracle Data-Base    ( Hint: CMS_PXL_PIXEL_R ) </h3>";
  
  *out << cgicc::br() << "username: " << cgicc::input().set("type", "text").set("name", "oracleUserName") << std::endl;
  
  *out<<"<h3> Password for Access to Oracle Data-Base </h3>";
  
  *out << cgicc::br() << "Password: " << cgicc::input().set("type", "password").set("name", "oraclePassword") << std::endl;
  
  *out<<"<hr/>"<< std::endl;

  *out << "<tr>" << std::endl;
  
  *out << cgicc::br() << "DCU console printout: ";

  if (dcu_printout_on_) {
    *out << "<input type=\"submit\" disabled=\"TRUE\" name=\"Command\" value=\"" << "turn On" << "\"/>";
    *out << "<input type=\"submit\" name=\"Command\" value=\"" << "turn Off" << "\"/>";
  }
  else {
    *out << "<input type=\"submit\" name=\"Command\" value=\"" << "turn On" << "\"/>";
    *out << "<input type=\"submit\" disabled=\"TRUE\" name=\"Command\" value=\"" << "turn Off" << "\"/>";
  }

  // Oracle connection response: fault string in case of problem
  *out<<"  <h2> Oracle connection errors </h2>" <<std::endl;
  *out<<"  <textarea rows=\"6\" cols=\"90\" readonly>" <<std::endl;
  if (!fault_message_db_connection_.str().empty()) {
    *out<< fault_message_db_connection_.str() <<endl;
    *out<< endl;
  }
  *out<< fault_string_db_get_table_.str() <<endl;
  *out<<"  </textarea>" <<std::endl;

  // PSX response: fault string in case of problem
  *out<<"  <h2> PSX response errors </h2>" <<std::endl;
  *out<<"  <textarea rows=\"6\" cols=\"90\" readonly>" <<std::endl;
  *out<< fault_message_psx_.str() <<endl;
  *out<<"  </textarea>" <<std::endl;

  *out << "</tr>" << std::endl;
  *out << "</form>" << std::endl;
  *out<<"<hr/>"<< std::endl;

  //*out<<"</html>";

  if (debug_) cout<< "//-- PixelDCSDpInterface::Default: dcu_map_.size() = " << dcu_map_.size() <<endl;
  
  // table parameters and title
  
  *out << "<TABLE border=\"1\"" << std::endl;     // table begin
		
  *out<< "<CAPTION><EM>" "Table for DCU output" "</EM></CAPTION>" <<endl;
  *out<< "<TR>";
  *out
      << "<TH>" "portcardName"    "</TH>"
      << "<TH>" "chip ID"         "</TH>"
      << "<TH>" "fec"             "</TH>"
      << "<TH>" "mfec"            "</TH>"
      << "<TH>" "ccu"             "</TH>"
      << "<TH>" "ccuCh"           "</TH>"
      << "<TH>" "RTD2"            "</TH>"
      << "<TH>" "RTD3"            "</TH>"
      << "<TH>" "AOH RTD"         "</TH>"
      << "<TH>" "Vpc"             "</TH>"
      << "<TH>" "Vaa"             "</TH>"
      << "<TH>" "Vdd"             "</TH>"
  <<endl;
  *out<< "</TR>";
  
  for (std::map<PortCard::Address, PortCard::DCU>::const_iterator it=dcu_map_.begin(); it!=dcu_map_.end(); ++it)
  {
//    unsigned dcuIdId = it->first.dcuId_;
    unsigned fecBoardId = it->first.fecBoardId_;
    unsigned mfecId = it->first.mfecId_;
    unsigned ccuId = it->first.ccuId_;
    unsigned ccuChannelId = it->first.ccuChannelId_;

    unsigned fecBoardId_zero = 0;     // in current configuration database fecBoard == 0
    const std::string& alias = Tora_dpTable_dpAlias_[fecBoardId_zero][mfecId][ccuId][ccuChannelId][5];   // Use alias for Vpc
    if (debug_) cout<< "fecBoardId = " << fecBoardId << " mfecId = " << mfecId << " ccuId = " << ccuId << " ccuChannelId = " << ccuChannelId << " dpAlias = " << alias <<endl;
    
    *out<< "<TR>";
    *out
        // centered 
        << "<TH>" << it->first.portcardName_  << "</TH>"
        << "<TH>" << it->first.dcuId_         << "</TH>"
        << "<TH>" << it->first.fecBoardId_    << "</TH>"
        << "<TH>" << it->first.mfecId_        << "</TH>"
        << "<TH>" << it->first.ccuId_         << "</TH>"
        << "<TH>" << it->first.ccuChannelId_  << "</TH>"
        << "<TH>" << it->second.Trtd2_str()   << "</TH>"
        << "<TH>" << it->second.Trtd3_str()   << "</TH>"
        << "<TH>" << it->second.Taoh_str()    << "</TH>"
        << "<TH>" << it->second.Vpc_          << "</TH>"
        << "<TH>" << it->second.Vaa_          << "</TH>"
        << "<TH>" << it->second.Vdd_          << "</TH>"
    <<endl;
    *out<< "</TR>";
  }
  *out<< "</TABLE>" <<endl;     // table begin
  
  *out << "</html>";
}  

void PixelDCStoTrkFECDpInterface::XgiHandler (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{
  cgicc::Cgicc cgi(in);
 
  //--- retrieve commands 
  //    to Finite State Machine and on DCU printout 
  std::string Command = cgi.getElement("Command")->getValue();

  if (debug_) cout<< "// PixelDCSDpInterface::XgiHandler: got command " << Command <<endl;

  if ( Command == "Configure" )
  {
    toolbox::fsm::State configured('C');            // TODO: use 'C' as output of some method
    if (fsm_.getCurrentState() == configured) {
      cout<< "//--- PixelDCSDpInterface::XgiHandler: received command Configure in Configure state! -- Ignored!" <<endl;
    }
    else {
      cgicc::FormEntry cgiFormEntry_username = (*cgi.getElement("oracleUserName"));
      std::string username = cgiFormEntry_username.getValue();
      if (username.size()) {
        cout<< "PixelDCSDpInterface::XgiHandler: Using manually entered username " << username <<endl;
        oracleUserName_ = username;
      }

      cgicc::FormEntry cgiFormEntry_password = (*cgi.getElement("oraclePassword"));
      std::string password = cgiFormEntry_password.getValue();
      if (password.size()) {
        cout<< "PixelDCSDpInterface::XgiHandler: Using manually entered password" <<endl;
        oraclePassword_ = password;
      }

      Attribute_Vector parametersXgi(1);
      parametersXgi.at(0).name_ = "GlobalKey"; 
      parametersXgi.at(0).value_ = cgi.getElement("GlobalKey")->getValue();

      xoap::MessageReference msg = MakeSOAPMessageReference("Configure", parametersXgi);
      xoap::MessageReference response = Configure(msg);
      if ( Receive(response) != "ConfigureDone" ) std::cout << "The PixelDCSDpInterface could not be Configured" << std::endl;
    }
  }
  else if ( Command == "Halt" )
  {
    xoap::MessageReference msg = MakeSOAPMessageReference("Halt");
    xoap::MessageReference response = Halt(msg);
    if ( Receive(response) != "HaltDone" ) std::cout << "The PixelDCSDpInterface could not be Halted" << std::endl;
  }
 
  //--- retrieve commands on DCU printout
  if (Command == "turn On") dcu_printout_on_ = true;
  if (Command == "turn Off") dcu_printout_on_ = false;
  if (debug_) cout<< "dcu_printout_on_ = " << dcu_printout_on_ <<endl; 

  //--- re-display webpage
  Default(in, out);
}

xoap::MessageReference PixelDCStoTrkFECDpInterface::Configure (xoap::MessageReference msg) 
{
  cout<< "PixelDCStoTrkFECDpInterface::Configure" <<endl;

  try  {
    toolbox::Event::Reference e(new toolbox::Event("Configure", this));
    fsm_.fireEvent(e);
  }  catch (toolbox::fsm::exception::Exception & e) {
    XCEPT_RETHROW(xoap::exception::Exception, "Invalid Command.", e);
  }
  cout<< "PixelDCStoTrkFECDpInterface::Configure done with FSM transition" <<endl;

  xoap::MessageReference reply = MakeSOAPMessageReference("ConfigureDone");
  return reply;
}

void PixelDCStoTrkFECDpInterface::stateConfiguring (toolbox::fsm::FiniteStateMachine & fsm) 
{
  cout<< "PixelDCStoTrkFECDpInterface::stateConfiguring" <<endl;
  
  //bool is_configured = true;
  bool is_configured = 1;

  fault_message_db_connection_.str("");
  fault_string_db_get_table_.str("");

  //--- we read the Oracle database username and password from xml configuration file
  if (debug_) cout<< "PixelDCStoTrkFECDpInterface::Configure: oracleUserName = " << oracleUserName_.toString() <<endl;
  if (debug_) cout<< "PixelDCStoTrkFECDpInterface::Configure: oraclePassword = " << oraclePassword_.toString() <<endl;
  oraclePassword_dpNames_ = oraclePassword_.toString();
  oraclePassword_dpFilter_ = oraclePassword_.toString();
  oraclePassword_dpCalibration_ = oraclePassword_.toString();

  if (debug_) {
    cout<< "PixelDCStoTrkFECDpInterface::Configure started. Parameters:" <<endl;
    cout<< "   oracleViewName_dpNames_.toString() = " << oracleViewName_dpNames_.toString() <<endl;
    cout<< "   oracleTableName_dpNames_.toString() = " << oracleTableName_dpNames_.toString() <<endl;
    if (debug_) cout<< "   oraclePassword_dpNames_ = " << oraclePassword_dpNames_ <<endl;
    cout<< "   oracleViewName_dpFilter_.toString() = " << oracleViewName_dpFilter_.toString() <<endl;
    cout<< "   oracleTableName_dpFilter_.toString() = " << oracleTableName_dpFilter_.toString() <<endl;
    if (debug_) cout<< "   oraclePassword_dpFilter_ = " << oraclePassword_dpFilter_ <<endl;
    cout<< "   oracleViewName_dpCalibration_.toString() = " << oracleViewName_dpCalibration_.toString() <<endl;
    cout<< "   oracleTableName_dpCalibration_.toString() = " << oracleTableName_dpCalibration_.toString() <<endl;
    if (debug_) cout<< "   oraclePassword_dpCalibration_ = " << oraclePassword_dpCalibration_ <<endl;
    cout<< "   version of the CALIB_FILTER_PARAMS is " << version_dcu_calib_filter_.toString() <<endl;
  }
  cout<< "PixelDCStoTrkFECDpInterface::Configure: prefix to datapoint name psx_system_name_ = \"" << psx_system_name_.toString() << "\"" <<endl;

  //
  //  dpNames
  //
  try {
    if (debug_) cout<< "--> PixelDCStoTrkFECDpInterface::Configure: trying to connectOracleDB" <<endl;
    connectOracleDB(oracleViewName_dpNames_.toString(), oraclePassword_dpNames_);
    if (debug_) cout<< ".. success" <<endl;
  }
  catch ( xcept::Exception& e ) {
    is_configured = false;
    std::cout << "PixelDCStoTrkFECDpInterface::Configure: Caught Exception during connectOracleDB to obtain dpNames:" << std::endl;
    std::cout << " " << e.what() << std::endl;
    // XCEPT_RETHROW (xcept::Exception, "PixelDCStoTrkFECDpInterface::Configure: Failed to connect to Oracle Data-Base: " + std::string(e.what()), e);
  }

  if (debug_) cout<< "PixelDCStoTrkFECDpInterface::Configure: oracleViewName_dpNames_ = " << oracleViewName_dpNames_.toString() <<endl;
  if (debug_) cout<< "PixelDCStoTrkFECDpInterface::Configure: oracleTableName_dpNames_.toString() = " << oracleTableName_dpNames_.toString() <<endl;
  
  if (oracleConnectionId_.empty())
  {
    cout<< "//-- PixelDCStoTrkFECDpInterface::Configure: no connection with Oracle" <<endl;
    cout<< "PixelDCStoTrkFECDpInterface::Configure: oracleViewName_dpNames_ = " << oracleViewName_dpNames_.toString() <<endl;
    cout<< "PixelDCStoTrkFECDpInterface::Configure: oracleTableName_dpNames_.toString() = " << oracleTableName_dpNames_.toString() <<endl;
    //    xoap::MessageReference reply = MakeSOAPMessageReference("ConfigurationFailed");   // NB: crash with exception for "Configuration Failed"
    HaltFSM();
    return;
  }

  //-- iteration over the coulums and rows at oracle data base

  std::vector<std::string> columnsNames;
  xdata::Table oracleDpNamesTable;
  try {
    if (debug_) cout<< "//--> PixelDCStoTrkFECDpInterface::Configure: trying to getOracleTable" <<endl;
    oracleDpNamesTable = getOracleTable(oracleViewName_dpNames_.toString(), oracleTableName_dpNames_.toString());
    if (debug_) cout<< ".. success" <<endl;
    columnsNames = oracleDpNamesTable.getColumns();
  }
  catch ( xcept::Exception& e ) {
    is_configured = false;
    std::cout << "//-- PixelDCStoTrkFECDpInterface::Configure: Caught Exception during getOracleTable for dpNames:" << std::endl;
    std::cout << " " << e.what() << std::endl;
    //XCEPT_RETHROW (xcept::Exception, "PixelDCStoTrkFECDpInterface::Configure: Failed to connect to Oracle Data-Base: " + std::string(e.what()), e);
    cout<< "//--ERROR PixelDCStoTrkFECDpInterface::Configure: no oracleDpNamesTable received" <<endl;
  }

  if (debug_) cout<< "PixelDCStoTrkFECDpInterface::Configure: dpNames: rowCount = " << oracleDpNamesTable.getRowCount() <<endl;

  int nelements_dpNames = 0;

  for (unsigned long currentRow=0; currentRow<oracleDpNamesTable.getRowCount(); currentRow++) {
    int columnIndex=1;
   
    unsigned int fecBoard = 0;      // NB: retains 0
    unsigned int mfec = 0;
    unsigned int ccu = 0;
    unsigned int ccuChannel = 0;
    unsigned int dcuChannel = 0;
    std::string alias;
    std::string dpName;

    for(std::vector<std::string>::iterator columnIterator=columnsNames.begin();
        columnIterator!=columnsNames.end(); columnIterator++,columnIndex++)
    {
      std::string columnType = oracleDpNamesTable.getColumnType(*columnIterator);
      std::string columnLabel = (*columnIterator);

      std::stringstream ss(oracleDpNamesTable.getValueAt(currentRow,*columnIterator)->toString());
      if (debug_) cout<< "--> columnType = " << columnType << " columnLabel = " << columnLabel << " value = " << ss.str() <<endl;

      if (columnLabel == "NAME") {	   
        ss >> dpName;
      }
      if (columnLabel == "ALIAS") {	   
        ss >> alias;
      }
      if (columnLabel == "RING") {
        ss >> mfec;
      }
      if (columnLabel == "CCU_ADDRS") {
        ss >> ccu;
      }
      if (columnLabel == "CCU_CHAN") {
        ss >> ccuChannel;
      }
      if (columnLabel == "DCU_CHAN") {
        ss >> dcuChannel;
      }
    }

    Tora_dpTable_dpName_[fecBoard][mfec][ccu][ccuChannel][dcuChannel] = dpName;
    Tora_dpTable_dpAlias_[fecBoard][mfec][ccu][ccuChannel][dcuChannel] = alias;
    ++nelements_dpNames;
    
    if (debug_) {
      cout<< "// dpNames map:" <<endl;
      cout<< " dpName = " << dpName <<endl;
      cout<< " alias = " << alias <<endl;
      cout<< " fecBoard = " << fecBoard <<endl;
      cout<< " mfec = " << mfec <<endl;
      cout<< " ccu = " << ccu <<endl;
      cout<< " ccuChannel = " << ccuChannel <<endl;
      cout<< " dcuChannel = " << dcuChannel <<endl;
    }
  }
  if (debug_) cout<< "//-- PixelDCStoTrkFECDpInterface::Configure: nelements_dpNames = " << nelements_dpNames <<endl;

  // the number of nelements_dpNames should be 256 = 32 portcards X 8 DCU channels
  // 0 will indicate possible wrong table name
  if (nelements_dpNames != 256)
  {
    cout<< "//-- PixelDCStoTrkFECDpInterface::Configure: ***ERROR: nelements_dpNames = " << nelements_dpNames << " instead of 256" <<endl;
    cout<< "//-- PixelDCStoTrkFECDpInterface::Configure: oracleViewName_dpNames_ = " << oracleViewName_dpNames_.toString() <<endl;
    cout<< "//-- PixelDCStoTrkFECDpInterface::Configure: oracleTableName_dpNames_ = " << oracleTableName_dpNames_.toString() <<endl;
    // if (fault_string_db_get_table_.str().empty())
    {
      fault_string_db_get_table_
        << "PixelDCStoTrkFECDpInterface::Configure: ***ERROR: nelements_dpNames = " << nelements_dpNames << " instead of 256"
        << endl
        << "oracleViewName_dpNames_ = " << oracleViewName_dpNames_.toString()
        << endl
        << "oracleTableName_dpNames_ = " << oracleTableName_dpNames_.toString()
      ;
    }

    //-- terminate Configure because of wrong dpNames table
    cout<< "//-- PixelDCStoTrkFECDpInterface::Configure: return because unsuccessful attempt to get dpNames table" <<endl;
    
    HaltFSM();
    return;
  }

  try {
    if (debug_) cout<< "--> PixelDCStoTrkFECDpInterface::Configure: trying to disconnectOracleDB" <<endl;
    disconnectOracleDB(oracleViewName_dpNames_.toString());
    if (debug_) cout<< ".. success" <<endl;
  }
  catch ( xcept::Exception& e ) {
    is_configured = false;
    std::cout << "PixelDCStoTrkFECDpInterface::Configure: Caught Exception during disconnectOracleDB after dpName:" << std::endl;
    std::cout << " " << e.what() << std::endl;
    //XCEPT_RETHROW (xcept::Exception, "PixelDCStoTrkFECDpInterface::Configure: Failed to disconnect Oracle Data-Base: " + std::string(e.what()), e);
  }

  //
  //  Filter: get deadband
  //
  try {
    if (debug_) cout<< "--> PixelDCStoTrkFECDpInterface::Configure: trying to connectOracleDB" <<endl;
    connectOracleDB(oracleViewName_dpFilter_.toString(), oraclePassword_dpFilter_);
    if (debug_) cout<< ".. success" <<endl;
  }
  catch ( xcept::Exception& e ) {
    is_configured = false;
    std::cout << "PixelDCStoTrkFECDpInterface::Configure: Caught Exception during connectOracleDB to obtain dpFilter:" << std::endl;
    std::cout << " " << e.what() << std::endl;
    //XCEPT_RETHROW (xcept::Exception, "PixelDCStoTrkFECDpInterface::Configure: Failed to connect to Oracle Data-Base: " + std::string(e.what()), e);
  }

  if (debug_) cout<< "PixelDCStoTrkFECDpInterface::Configure: oracleViewName_dpFilter_ = " << oracleViewName_dpFilter_.toString() <<endl;
  if (debug_) cout<< "PixelDCStoTrkFECDpInterface::Configure: oracleTableName_dpFilter_.toString() = " << oracleTableName_dpFilter_.toString() <<endl;
  
  if (oracleConnectionId_.empty())
  {
    cout<< "//-- PixelDCStoTrkFECDpInterface::Configure: no connection with Oracle" <<endl;
    cout<< "PixelDCStoTrkFECDpInterface::Configure: oracleViewName_dpNames_ = " << oracleViewName_dpNames_.toString() <<endl;
    cout<< "PixelDCStoTrkFECDpInterface::Configure: oracleTableName_dpNames_.toString() = " << oracleTableName_dpNames_.toString() <<endl;

    //-- terminate Configure because of wrong deadband table
    cout<< "//-- PixelDCStoTrkFECDpInterface::Configure: return because unsuccessful attempt to get dpFilter table" <<endl;
    HaltFSM();
    return;
  }
  
  xdata::Table oracleFilterTable;
  try {
    if (debug_) cout<< "//--> PixelDCStoTrkFECDpInterface::Configure: trying to getOracleTable" <<endl;
    oracleFilterTable = getOracleTable(oracleViewName_dpFilter_.toString(), oracleTableName_dpFilter_.toString());
  }
  catch ( xcept::Exception& e ) {
    is_configured = false;
    std::cout << "//-- PixelDCStoTrkFECDpInterface::Configure: Caught Exception during getOracleTable to obtain dpFilter:" << std::endl;
    std::cout << " " << e.what() << std::endl;
    //XCEPT_RETHROW (xcept::Exception, "PixelDCStoTrkFECDpInterface::Configure: Failed to connect to Oracle Data-Base: " + std::string(e.what()), e);
    cout<< "//--ERROR PixelDCStoTrkFECDpInterface::Configure: no oracleFilterTable received" <<endl;
  }
  //-- iteration over the coulums and rows at oracle data base
  std::vector<std::string> columnsFilter = oracleFilterTable.getColumns();

  if (debug_) cout<< "PixelDCStoTrkFECDpInterface::Configure: Filter: rowCount = " << oracleFilterTable.getRowCount() <<endl;

  int nelements_dpFilter = 0;

  for (unsigned long currentRow=0; currentRow<oracleFilterTable.getRowCount(); currentRow++) {
    int columnIndex=1;
   
    unsigned int fecBoard = 0;      // NB: retains 0
    unsigned int mfec = 0;
    unsigned int ccu = 0;
    unsigned int ccuChannel = 0;
    unsigned int dcuChannel = 0;
    double deadband = 0;
    std::string alias;
    std::string version;

    for(std::vector<std::string>::iterator columnIterator=columnsFilter.begin();
        columnIterator!=columnsFilter.end(); columnIterator++,columnIndex++)
    {
      std::string columnType = oracleFilterTable.getColumnType(*columnIterator);
      std::string columnLabel = (*columnIterator);

      std::stringstream ss(oracleFilterTable.getValueAt(currentRow,*columnIterator)->toString());
      if (debug_) cout<< "--> columnType = " << columnType << " columnLabel = " << columnLabel << " value = " << ss.str() <<endl;

	    if (columnLabel == "VERSION") {	   
	      ss >> version;
	    }
	    if (columnLabel == "ALIAS") {	   
	      ss >> alias;
	    }
      if (columnLabel == "MFEC") {
        ss >> mfec;
      }
      if (columnLabel == "CCU") {
        ss >> ccu;
      }
      if (columnLabel == "CCUCHAN") {
        ss >> ccuChannel;
      }
      if (columnLabel == "DCUCHAN") {
        ss >> dcuChannel;
      }
      if (columnLabel == "DEADBAND") {
        ss >> deadband;
      }
    }

    if (version == version_dcu_calib_filter_.toString())
    {
      deadband_[fecBoard][mfec][ccu][ccuChannel][dcuChannel] = deadband;
      ++nelements_dpFilter;
      
      if (debug_) {
        cout<< "// Filter map:" <<endl;
        cout<< " version = " << version <<endl;
        cout<< " alias = " << alias <<endl;
        cout<< " fecBoard = " << fecBoard <<endl;
        cout<< " mfec = " << mfec <<endl;
        cout<< " ccu = " << ccu <<endl;
        cout<< " ccuChannel = " << ccuChannel <<endl;
        cout<< " dcuChannel = " << dcuChannel <<endl;
        cout<< " deadband = " << deadband <<endl;
      }
    }
  }
  if (debug_) cout<< "//-- PixelDCStoTrkFECDpInterface::Configure: nelements_dpFilter = " << nelements_dpFilter <<endl;

  // the number of nelements_dpFilter should be 256 = 32 portcards X 8 DCU channels
  // 0 will indicate possible wrong version
  if (nelements_dpFilter != 256)
  {
    cout<< "//-- PixelDCStoTrkFECDpInterface::Configure: ***ERROR: nelements_dpFilter = " << nelements_dpFilter << " instead of 256" <<endl;
    cout<< "//-- PixelDCStoTrkFECDpInterface::Configure: oracleViewName_dpFilter_ = " << oracleViewName_dpFilter_.toString() <<endl;
    cout<< "//-- PixelDCStoTrkFECDpInterface::Configure: oracleTableName_dpFilter_ = " << oracleTableName_dpFilter_.toString() <<endl;
    cout<< "//-- Version of the dpFilter table is " << version_dcu_calib_filter_.toString() <<endl;
    // if (fault_string_db_get_table_.str().empty())
    {
      fault_string_db_get_table_
        << "PixelDCStoTrkFECDpInterface::Configure: ***ERROR: nelements_dpFilter = " << nelements_dpFilter << " instead of 256"
        << endl
        << "oracleViewName_dpFilter_ = " << oracleViewName_dpFilter_.toString()
        << endl
        << "oracleTableName_dpFilter_ = " << oracleTableName_dpFilter_.toString()
        << endl
        << "Version of the dpFilter table is " << version_dcu_calib_filter_.toString()
      ;
    }

    //-- terminate Configure because of wrong deadband table
    //    xoap::MessageReference reply = MakeSOAPMessageReference("ConfigurationFailed");   // NB: crash with exception for "Configuration Failed"
    //    return reply;
    cout<< "//-- PixelDCStoTrkFECDpInterface::Configure: Configuration failed, return FSM to Halted" <<endl;
    HaltFSM();
    return;
  }

  try {
    if (debug_) cout<< "--> PixelDCStoTrkFECDpInterface::Configure: trying to disconnectOracleDB" <<endl;
    disconnectOracleDB(oracleViewName_dpFilter_.toString());
    if (debug_) cout<< ".. success" <<endl;
  }
  catch ( xcept::Exception& e ) {
    is_configured = false;
    std::cout << "PixelDCStoTrkFECDpInterface::Configure: Caught Exception during disconnectOracleDB after dpFilter:" << std::endl;
    std::cout << " " << e.what() << std::endl;
    //XCEPT_RETHROW (xcept::Exception, "PixelDCStoTrkFECDpInterface::Configure: Failed to disconnect Oracle Data-Base: " + std::string(e.what()), e);
  }
  
  if (is_configured) {
    //--- update Finite State Machine state
    cout<< "// PixelDCStoTrkFECDpInterface::Configure: update Finite State Machine state" <<endl;
    cout<< "PixelDCStoTrkFECDpInterface::Configure: FSM: current state: " << fsm_.getStateName(fsm_.getCurrentState()) <<endl;
    try {
      toolbox::Event::Reference e(new toolbox::Event("ConfigureDone", this));
      fsm_.fireEvent(e);
    } catch (toolbox::fsm::exception::Exception & e) {
      //-- cout<< "//-- XCEPT_RETHROW(xoap::exception::Exception, \"Invalid State Machine Input.\", e)" <<endl;
      XCEPT_RETHROW(xoap::exception::Exception, "Invalid State Machine Input.", e);
    }
    cout<< "PixelDCStoTrkFECDpInterface::Configure: FSM: new state: " << fsm_.getStateName(fsm_.getCurrentState()) <<endl;
  }
  else {
    cout<< "//-- PixelDCStoTrkFECDpInterface::Configure: Configuration failed, return FSM to Halted" <<endl;
    HaltFSM();
    return;
  }
  

}

void PixelDCStoTrkFECDpInterface::HaltFSM() {
  try {
    toolbox::Event::Reference e(new toolbox::Event("Halt", this));
    fsm_.fireEvent(e);
  } catch (toolbox::fsm::exception::Exception & e) {
    //-- cout<< "//-- XCEPT_RETHROW(xoap::exception::Exception, \"Invalid State Machine Input.\", e)" <<endl;
    XCEPT_RETHROW(xoap::exception::Exception, "Invalid State Machine Input.", e);
  }
}

xoap::MessageReference PixelDCStoTrkFECDpInterface::Halt (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  /*
    Delete and clear all your objects here.
  */
	
  try
    {
      toolbox::Event::Reference e(new toolbox::Event("Halt", this));
      fsm_.fireEvent(e);
    }
  catch (toolbox::fsm::exception::Exception & e)
    {
      XCEPT_RETHROW(xoap::exception::Exception, "Invalid Command.", e);
    }

  xoap::MessageReference reply = MakeSOAPMessageReference("HaltDone");
  return reply;
}
        	
xoap::MessageReference PixelDCStoTrkFECDpInterface::updateDpValueTrkFEC(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  // plan (may be somewhat outdated!)
  // ----
  // start to compose SOAP message (fill vector dpeList with data):
  // loop over all DCU raw data
  // current value:
  // 1) include it into SOAP message if it changed wrt last_sent_
  // 2) convert it to calibrated value if needed
  // 3) compare with last_sent_ value: if exceeds deadband, raise update flag
  // if you find flag update true after the loop, send the SOAP message

  if (debug_) cout << "// <PixelDCStoTrkFECDpInterface::updateDpValueTrkFEC>" << endl;

  std::vector<PortCard::AddressDCU> vdcu = PortCard::SOAP_ReadAll::Decode(msg);
  if (dcu_printout_on_)
  {
    time_t rawtime;                   // time_t: the number of seconds elapsed since 00:00 hours, Jan 1, 1970 UTC
    struct tm* timeinfo;

    time(&rawtime);                   // Get the current calendar time as a time_t object
    timeinfo = localtime(&rawtime);   // represents time_t value as a local time

    cout<< " Time: " << asctime(timeinfo);  // format: Wed Oct  7 19:09:34 2009 NB: asciitime inserts carrige return ('\n') into the string!
    for (std::vector<PortCard::AddressDCU>::const_iterator addressDCU = vdcu.begin(); addressDCU != vdcu.end(); ++addressDCU) {
      addressDCU->print(std::cout);
    }
  }
  // fill map to be displayed on webpage
  for (std::vector<PortCard::AddressDCU>::const_iterator addressDCU=vdcu.begin(); addressDCU!=vdcu.end(); ++addressDCU) {
    dcu_map_[addressDCU->address_] = addressDCU->dcu_;
  }

  // list of datapoints to compose SOAP message
  std::list<PixelDCSPVSSDpe> dpeList;

  bool update = false;      // flag to send SOAP message to update db information

  // this map keeps changes wrt to last_sent_, will be copied to last_sent_ in case if message will be actually sent
  std::map<unsigned int         // fecBoardId
    , std::map<unsigned int     // mfecId
    , std::map<unsigned int     // ccuId
    , std::map<unsigned int     // ccuChannelId
    , std::map<unsigned int     // dcuChannelId
    , double
  > > > > > last_prepared;

  for (std::vector<PortCard::AddressDCU>::const_iterator
      addressDCU = vdcu.begin(); addressDCU != vdcu.end(); ++addressDCU)
  {
    unsigned fecBoard = addressDCU->address_.fecBoardId_;
    // in current configuration database fecBoard == 0
    fecBoard = 0;
    unsigned mfec = addressDCU->address_.mfecId_;
    unsigned ccu = addressDCU->address_.ccuId_;
    unsigned ccuChannel = addressDCU->address_.ccuChannelId_;
      
    // DCU channels

    // Vportcard value would be used for calibration of any channel
    //double Vpc = addressDCU->dcu_.Vpc();

    for (unsigned ichan=0; ichan<8; ++ichan)
    {
      bool addSOAP = false;     // flag to add channel to SOAP message

      // calculate calibrated value
      float calib = 0;
      
      // ADC value of the current channel
      float adc = addressDCU->dcu_.GetChan(ichan); 

      if (ichan == 2 || ichan == 3 || ichan == 4)
      {
        // T
        // calib = (2./3850.e-6) * (1. - adc/Vpc);
        calib = addressDCU->dcu_.T(ichan);
      }
      
      if (ichan == 0 || ichan == 1 || ichan == 6)
      {
        // voltage
        calib = 0;
      }

      // calibrated value: if it exceeds average more than deadband calc new average and raise update flag

      if (average_.find(fecBoard) != average_.end()
          && average_[fecBoard].find(mfec) != average_[fecBoard].end()
          && average_[fecBoard][mfec].find(ccu) != average_[fecBoard][mfec].end()
          && average_[fecBoard][mfec][ccu].find(ccuChannel) != average_[fecBoard][mfec][ccu].end()
          && average_[fecBoard][mfec][ccu][ccuChannel].find(ichan) != average_[fecBoard][mfec][ccu][ccuChannel].end()
         ) {
        // previous average exists
        float mean0 = average_[fecBoard][mfec][ccu][ccuChannel][ichan];
        // obtain deadband for this channel (for now assume that deadband exists) -- TODO
        float deadband = deadband_[fecBoard][mfec][ccu][ccuChannel][ichan];
        if (deadband > 0) {
          if (fabs(calib - mean0) > deadband) {
            // rise update flag
            //update = true;
            update = 1;
            // new average
            float mean = weight_*calib + (1.-weight_)*mean0;
            average_[fecBoard][mfec][ccu][ccuChannel][ichan] = mean;
          }
        }
      }
      else {
        // use current value as an average
        average_[fecBoard][mfec][ccu][ccuChannel][ichan] = calib;
        // update db
        //update = true;
        update = true;
      }

      // raw ADC: does it change wrt last value?
      // NB: if calibrated value exceeded deadband (update == true) the last value also changed, but not all channels have calibrated value
      if (last_sent_.find(fecBoard) != last_sent_.end()
          && last_sent_[fecBoard].find(mfec) != last_sent_[fecBoard].end()
          && last_sent_[fecBoard][mfec].find(ccu) != last_sent_[fecBoard][mfec].end()
          && last_sent_[fecBoard][mfec][ccu].find(ccuChannel) != last_sent_[fecBoard][mfec][ccu].end()
          && last_sent_[fecBoard][mfec][ccu][ccuChannel].find(ichan) != last_sent_[fecBoard][mfec][ccu][ccuChannel].end()
         ) {
        // last value exists
        float last = last_sent_[fecBoard][mfec][ccu][ccuChannel][ichan];
        // last as well as adc are basically integer values, but let's compare them strictly as floats
        const float eps = 1.e-7;
        if (fabs(adc - last) > eps) {
          // add raw adc and calibrated values into SOAP message
          addSOAP = true;
          // register that value in last_prepared (will be copied to last_sent_ in case when SOAP will be sent)
          last_prepared[fecBoard][mfec][ccu][ccuChannel][ichan] = adc;
        }
      }
      else {
        // this is the first measurement in current session
        last_sent_[fecBoard][mfec][ccu][ccuChannel][ichan] = adc;
        addSOAP = true;
      }

      if (addSOAP) {
        // add raw adc value

        // currently (March 2008) at TIF
        // the datapoint name for channel VPC looks like DP_FPix_BpI_D1_PRT1_DCU_VPC
        // the portcard name looks like FPix_BpI_D1_PRT1
        // the name of PVSS datapoint element with attribute of raw data field looks like DP_FPix_BpI_D1_PRT1_DCU_VPC:_original.._value

        // structure of each datapoint_element in PVSS has few fields,
        // e.g. the structure CMS_Pixel_Temperature_TrkFEC which is currently in use in PVSS system at TIF has fields
        //
        //      _common
        //      _lock
        //      actual {
        //        calibratedTemperature
        //        isAvailable
        //        rawADC
        //        currentInputMode
        //      }
        //      settings {
        //        _common
        //        _lock
        //        FSM {
        //          ...
        //        }
        //      }

        //
        // // so we have to specify our field which is 'actual.rawADC'
        // // We will use it as a suffix for db datapoint name to have e.g. DP_FPix_BpI_D1_PRT1_DCU_VPC.actual.rawADC
        //
        // const std::string actual_rawADC = ".actual.rawADC";
        // const std::string actual_calibrated[8] = {
        //   ".actual.calibratedVoltage",
        //   ".actual.calibratedVoltage",
        //   ".actual.calibratedTemperature",
        //   ".actual.calibratedTemperature",
        //   ".actual.calibratedTemperature",
        //   ".actual.calibratedVoltage",
        //   ".actual.calibratedVoltage",
        //   ".actual.calibratedTemperature"
        // };

        //
        // so we have to specify our field which is either '.raw' or '.converted'
        // We will use it as a suffix for db datapoint name to have e.g. tkDcuFpix_trkfecBoard01_mfec06_ccu125_ccuChannel01.aoh.raw
        //
        const std::string actual_rawADC = ".raw";
        std::string actual_calibrated[8];
        actual_calibrated[0] = ".converted";
        actual_calibrated[1] = ".converted";
        actual_calibrated[2] = ".converted";
        actual_calibrated[3] = ".converted";
        actual_calibrated[4] = ".converted";
        actual_calibrated[5] = ".converted";
        actual_calibrated[6] = ".converted";
        actual_calibrated[7] = ".converted";

        // add this channel to the vector dpeList to be in the SOAP message sent to PSX
        // extract dpName stored during Configure
        const std::string& dpName = Tora_dpTable_dpName_[fecBoard][mfec][ccu][ccuChannel][ichan];
        // add to vector dpeList raw ADC
        PixelDCSPVSSDpe dpe(psx_system_name_.toString() + dpName + actual_rawADC, addressDCU->dcu_.GetChan(ichan));
        dpeList.push_back(dpe);
        // add to vector dpeList calibrated value
        PixelDCSPVSSDpe dpe_calib(psx_system_name_.toString() + dpName + actual_calibrated[ichan], calib);
        dpeList.push_back(dpe_calib);
      }
    } // loop over the DCU channels
  } // loop over vector of AddressDCU elements
 
  if (update)
  {
    // prepare SOAP message to be send to PSX server

    //  *** Example 1 ***
    //   
    //  Standalone version for VPC only
    //
    //   xoap::MessageReference message = xoap::createMessage();
    //   xoap::SOAPEnvelope Senvelope = message->getSOAPPart().getEnvelope();
    //   xoap::SOAPBody Sbody = Senvelope.getBody();
    //   
    //   xoap::SOAPName ScommandElement = Senvelope.createName("dpSet", "psx", PSX_NS_URI);
    //   xoap::SOAPBodyElement SbodyElement = Sbody.addBodyElement(ScommandElement);
    //   
    //   // add just Vpc
    //   
    //   xoap::SOAPName dpElement = Senvelope.createName("dp");
    //   xoap::SOAPElement childElement = SbodyElement.addChildElement(dpElement);
    // 
    //   xoap::SOAPName nameElement = Senvelope.createName("name");
    //   childElement.addAttribute(nameElement, "DP_FPix_BpI_D1_PRT1_DCU_VPC.actual.rawADC:_original.._value");
    //   
    //   childElement.addTextNode("2700");
    //   
    //  *** Example 2 ***
    //   
    //  Standalone version for VPC only
    //
    //   std::list<PixelDCSPVSSDpe> dpeList;
    //   PixelDCSPVSSDpe dpe("DP_FPix_BpI_D1_PRT1_DCU_VPC.actual.rawADC", 2700);
    //   dpeList.push_back(dpe);
    //   xdaq::ApplicationDescriptor* psxDescriptor = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor("psx", 0);
    //   PixelDCSPVSSCommander* dcsPVSSCommander = new PixelDCSPVSSCommander(this, psxDescriptor);
    //   xoap::MessageReference message = dcsPVSSCommander->MakeSOAPMessageReference_dpSet(dpeList);

    // use the name 'psxtk' instead of 'psx' to allow for a different psx server to be used
    // update -- now we are abandoning that approach and reverting to just 'psx'
    xdaq::ApplicationDescriptor* psxDescriptor = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor("psx", 0);
    //FIXME -- likely memory leak. I can't see where this pointer is ever deleted.
    PixelDCSPVSSCommander* dcsPVSSCommander = new PixelDCSPVSSCommander(this, psxDescriptor);
    xoap::MessageReference message = dcsPVSSCommander->MakeSOAPMessageReference_dpSet(dpeList);

    ++nsent_;
    if (nsent_ % nreport_ == 0) {
      cout<< "//-- PixelDCStoTrkFECDpInterface::updateDpValueTrkFEC: Sending to PSX server message #" << nsent_ <<endl;
    }
    
    if (debug_) cout<<"PixelDCStoTrkFECDpInterface::updateDpValueTrkFEC: Sending SOAP request to URL = " 
      << getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor("psx", 0)->getContextDescriptor()->getURL()
    <<endl;

    if (debug_) cout <<"PixelDCStoTrkFECDpInterface::updateDpValueTrkFEC: Request : ------------------------------------ "<<endl;
    if (debug_) message->writeTo(cout);
    message->writeTo(request_);
    if (debug_) cout << endl;
    if (debug_) cout <<"PixelDCStoTrkFECDpInterface::updateDpValueTrkFEC: ---------------------------------------------- "<<endl;

    //--- send SOAP request and wait for SOAP reply  to machine running PVSS
    xdaq::ApplicationDescriptor* originator = this->getApplicationDescriptor();
    xdaq::ApplicationDescriptor* destination = 0;
    try {
      destination = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor("psx", 0);
    }
    catch ( xcept::Exception& e ) {
      cout << "// PixelDCStoTrkFECDpInterface::updateDpValueTrkFEC: Caught Exception thrown by PixelDCStoTrkFECDpInterface::updateDpValueTrkFEC" << std::endl;
      cout << " " << e.what() << endl;
      return MakeSOAPMessageReference("TestFailed");	  
    }

    if (send_to_PSX_)
    {
      PixelTimer soaptimer;
      xoap::MessageReference reply;
      soaptimer.start();
      try {
        // reply = getApplicationContext()->postSOAP(message, destination);       // obsolete method
        reply = getApplicationContext()->postSOAP(message, *originator, *destination);
      }
      catch ( xcept::Exception& e ) {
	soaptimer.stop();
	cout << "// PixelDCStoTrkFECDpInterface::updateDpValueTrkFEC: Caught Exception after time = "<<soaptimer.tottime() << endl;
	cout << "e.what = " << e.what() << endl;
        return MakeSOAPMessageReference("TestFailed");
      }
      soaptimer.stop();
      if (soaptimer.tottime() > 5) { 
	soaptimer.setName("PixelDCStoTrkFECDpInterface");
	soaptimer.printTime("Slow PSX warning");
      }
      cout<<"Time to send SOAP to PSX = "<<soaptimer.tottime()<<endl;

      //--- print SOAP reply
      if (debug_) cout <<" PixelDCStoTrkFECDpInterface::updateDpValueTrkFEC: Reply : -------------------------------------- "<<endl;
      if (debug_) reply->writeTo(cout);
      reply->writeTo(response_);
      if (debug_) cout << endl;
      if (debug_) cout <<" PixelDCStoTrkFECDpInterface::updateDpValueTrkFEC: ---------------------------------------------- "<<endl;

      xoap::SOAPBody body = reply->getSOAPPart().getEnvelope().getBody();
      if (body.hasFault()) {
        fault_message_psx_
          << "PixelDCStoTrkFECDpInterface::updateDpValueTrkFEC: "
          << body.getFault().getFaultString() <<endl
          << endl
        ;
        cout<< "// PixelDCStoTrkFECDpInterface::updateDpValueTrkFEC: Problem sending SOAP message to PSX server: " << body.getFault().getFaultString() <<endl;

        // it would be good to redisplay the webpage, but I don't know how to do that :-(
        // //--- re-display webpage
        // Default(in, out);
      }
      else {
        fault_message_psx_.str("");
        if (debug_) cout<< "PixelDCStoTrkFECDpInterface::updateDpValueTrkFEC: SOAP response is fine" <<endl;
      }
    }
    else {
      cout<< "//-- send_to_PSX_ = false" <<endl;
    }

    // copy content of the last_prepared to last_sent_
    last_sent_ = last_prepared;
  }
  
  return MakeSOAPMessageReference("updateDpValueDone");  //  SOAP return name convention for non-GUI called function: binded name("updateDpValueTrkFEC") + "Response"
}
