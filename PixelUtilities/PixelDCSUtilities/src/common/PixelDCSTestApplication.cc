#include "PixelUtilities/PixelDCSUtilities/include/PixelDCSTestApplication.h"

/*************************************************************************
 * Test class for commissioning of PixelDCSSupervisor,                   *
 * PixelDCStoFECDpInterface and PixelDCStoFEDDpInterface classes         *
 * (the latter two are contained in the PixelDCSInterface package);      *
 * sends dummy SOAP messages to the classes to be tested                 *
 * upon click of a button in the XDAQ webpage GUI                        *
 *                                                                       *
 * Authors: Christian Veelken UC Davis                                   *
 *          Andres Florez University of Puerto Rico                      *
 *                                                                       *
 * Last update: $Date: 2010/04/28 20:47:53 $ (UTC)                       *
 *          by: $Author: zatserkl $                                       *
 *************************************************************************/

#include <iomanip>
#include <iostream>

#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationStub.h"
#include "xgi/Utils.h"
#include "cgicc/HTMLClasses.h"
#include "toolbox/fsm/FailedEvent.h"

#include "PixelUtilities/PixelGUIUtilities/include/HTML2XGI.h"

#include <cstdlib>
#include <sstream>
#include <fstream>

using namespace std;

#define PSX_NS_URI "http://xdaq.cern.ch/xdaq/xsd/2006/psx-pvss-10.xsd"

XDAQ_INSTANTIATOR_IMPL(PixelDCSTestApplication)

PixelDCSTestApplication::PixelDCSTestApplication(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception) : xdaq::Application(s), SOAPCommander(this)
  // , debug_(false)
  , debug_(true)
{
  // SOAP Bindings to Low Level Commands and Specific Algorithms
  xoap::bind(this, &PixelDCSTestApplication::TestDCStoFEDDpInterface, "TestDCStoFEDDpInterface", XDAQ_NS_URI);
  xoap::bind(this, &PixelDCSTestApplication::TestDCStoTrkFECDpInterface, "TestDCStoTrkFECDpInterface", XDAQ_NS_URI);
  xoap::bind(this, &PixelDCSTestApplication::TestTrkFECSupervisor_DCUreadout, "TestTrkFECSupervisor_DCUreadout", XDAQ_NS_URI);
  xoap::bind(this, &PixelDCSTestApplication::TestTrkFECSupervisor_DCUreadout_fakeSOAP, "TestTrkFECSupervisor_DCUreadout_fakeSOAP", XDAQ_NS_URI);

  // Binding XGI Callbacks for messages from the browser
  xgi::bind(this, &PixelDCSTestApplication::Default, "Default");
  xgi::bind(this, &PixelDCSTestApplication::XgiHandler, "XgiHandler");
  
  this->getApplicationInfoSpace()->fireItemAvailable("fedDpInterface", &fedDpInterface_);
  this->getApplicationInfoSpace()->fireItemAvailable("trkfecDpInterface", &trkfecDpInterface_);
  this->getApplicationInfoSpace()->fireItemAvailable("dcsSupervisor", &dcsSupervisor_);
 
  // Miscellaneous variables most of which I do not understand...
  XDAQ_ROOT = getenv("XDAQ_ROOT");
  if (getenv("BUILD_HOME")==0){
    htmlbase_="/opt/xdaq/htdocs/PixelDCSUtilities/html/";
    xmlbase_="/opt/xdaq/htdocs/PixelDCSUtilities/xml/";
  }
  else{
    htmlbase_=std::string(getenv("BUILD_HOME"))+"/pixel/PixelUtilities/PixelDCSUtilities/html/";
    xmlbase_=std::string(getenv("BUILD_HOME"))+"/pixel/PixelUtilities/PixelDCSUtilities/xml/";
  }


}

PixelDCSTestApplication::~PixelDCSTestApplication()
{}

void PixelDCSTestApplication::Default (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{
  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  xgi::Utils::getPageHeader(*out, "Test-Program for Pixel DCS Supervisor");
  
  // Rendering Low Level GUI
  
  *out << "<h2>Low Level Commands</h2>" << endl;
  
  std::string url="/"+getApplicationDescriptor()->getURN()+"/XgiHandler";
  
  *out << "<form name=\"input\" method=\"get\" action=\"" << url << "\" enctype=\"multipart/form-data\">";
  HTML2XGI(out, htmlbase_+"/TestDCStoFEDDpInterface.htm");
  *out << "</form>" << endl;
  
  *out << "<form name=\"input\" method=\"get\" action=\""<<url<<"\" enctype=\"multipart/form-data\">";
  HTML2XGI(out, htmlbase_ + "/TestDCStoTrkFECDpInterface.htm");
  *out << "</form>" << endl;

  *out << "<form name=\"input\" method=\"get\" action=\""<<url<<"\" enctype=\"multipart/form-data\">";
  HTML2XGI(out, htmlbase_ + "/TestTrkFECSupervisor_DCUreadout.htm");
  *out << "</form>" << endl;

  *out << "<form name=\"input\" method=\"get\" action=\""<<url<<"\" enctype=\"multipart/form-data\">";
  HTML2XGI(out, htmlbase_ + "/TestTrkFECSupervisor_DCUreadout_fakeSOAP.htm");
  *out << "</form>" << endl;
  
  *out << "<form name=\"input\" method=\"get\" action=\""<<url<<"\" enctype=\"multipart/form-data\">";
  HTML2XGI(out, htmlbase_ + "/TestDCStoTrkFECDpInterfaceConfigure.htm");
  *out << "</form>" << endl;
  
  // The CGI calls to create your Low Level GUI may go here.
  
  *out << cgicc::hr() << std::endl;
  *out << cgicc::fieldset().set("style","font-size: 10pt;  font-family: arial;") << std::endl;
  *out << cgicc::legend("SOAP Request") << cgicc::p() << std::endl;
  *out << cgicc::textarea().set("rows","20").set("cols","120") << std::endl;

  *out << request_;
  
  *out << cgicc::textarea() << std::endl;
  *out << cgicc::fieldset() << std::endl;

  *out << cgicc::hr() << std::endl;

  *out << cgicc::fieldset().set("style","font-size: 10pt;  font-family: arial;") << std::endl;
  *out << cgicc::legend("SOAP Response") << cgicc::p() << std::endl;
  *out << cgicc::textarea().set("rows","20").set("cols","120") << std::endl;
  
  *out << response_;

  *out << cgicc::textarea() << std::endl;
  *out << cgicc::fieldset() << std::endl;

  // Fake data to test class PortCardDCU

//   PortCard::Address address;
//   PortCard::DCU dcu;
//   
//   dcu.Vaa_   = 1000;
//   dcu.Vdd_   = 1001;
//   dcu.rtd2_  = 1002;
//   dcu.rtd3_  = 1003;
//   dcu.aoh_   = 1004;
//   dcu.Vpc_   = 1005;
//   dcu.ch6_   = 1006;
//   dcu.ts_    = 1007;
//   
//   address.fecBoardId_    = 1;
//   address.mfecId_        = 2;
//   address.ccuId_         = 3;
//   address.ccuChannelId_  = 4;
//   
//   dcu_map_[address] = dcu;

  cout<< "//-- PixelDCSTestApplication::Default: dcu_map_.size() = " << dcu_map_.size() <<endl;
  
  // table parameters and title
  
  *out << "<TABLE border=\"1\"" << std::endl;     // table begin
		
  *out<< "<CAPTION><EM>" "Table for DCU output" "</EM></CAPTION>" <<endl;
  *out<< "<TR>";
  *out
      << "<TH>" "fecBoardId"    "</TH>"
      << "<TH>" "mfecId"        "</TH>"
      << "<TH>" "ccuId"         "</TH>"
      << "<TH>" "ccuChannelId"  "</TH>"
      << "<TH>" "Vportcard"     "</TH>"
      << "<TH>" "RTD2"          "</TH>"
      << "<TH>" "RTD3"          "</TH>"
      << "<TH>" "AOH RTD"       "</TH>"
      << "<TH>" "DCU TS"        "</TH>"
  <<endl;
  *out<< "</TR>";
  
  for (std::map<PortCard::Address, PortCard::DCU>::const_iterator it=dcu_map_.begin(); it!=dcu_map_.end(); ++it)
  {
    unsigned fecBoardId = it->first.fecBoardId_;
    unsigned mfecId = it->first.mfecId_;
    unsigned ccuId = it->first.ccuId_;
    unsigned ccuChannelId = it->first.ccuChannelId_;
    cout<< "fecBoardId = " << fecBoardId << " mfecId = " << mfecId << " ccuId = " << ccuId << " ccuChannelId = " << ccuChannelId <<endl;
    
    *out<< "<TR>";
    *out
        // centered 
        << "<TH>" << it->first.fecBoardId_    << "</TH>"
        << "<TH>" << it->first.mfecId_        << "</TH>"
        << "<TH>" << it->first.ccuId_         << "</TH>"
        << "<TH>" << it->first.ccuChannelId_  << "</TH>"
        << "<TH>" << it->second.Vpc_    << "</TH>"
        << "<TH>" << it->second.rtd2_   << "</TH>"
        << "<TH>" << it->second.rtd3_   << "</TH>"
        << "<TH>" << it->second.aoh_    << "</TH>"
        << "<TH>" << it->second.ts_     << "</TH>"
//         // align left
//         << "<TD>" << v_PixelFECDCSInterface[i]  << "</TD>"
//         << "<TD>" << v_fecBoard[i]              << "</TD>"
//         << "<TD>" << v_mfecNumber[i]            << "</TD>"
//         << "<TD>" << v_mfecChannel[i]           << "</TD>"
//         << "<TD>" << v_ccu[i]                   << "</TD>"
//         << "<TD>" << v_ccuChannel[i]            << "</TD>"
//         << "<TD>" << v_dcuChannel[i]            << "</TD>"
//         << "<TD>" << v_dpValue_Average[i]       << "</TD>"
//         << "<TD>" << v_Last_Time_Update[i]      << "</TD>"
    <<endl;
    *out<< "</TR>";
  }
  *out<< "</TABLE>" <<endl;     // table begin
  //-- table end
  //------------
  
  *out << "</html>";
}

void PixelDCSTestApplication::XgiHandler (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception)
{
  cgicc::Cgicc cgi(in);
  
//--- get command issued by user interaction (i.e. click on button);
//    defined by "value" attribute in html/TestDCStoFEDDpInterface.htm and html/TestDCStoTrkFECDpInterface.htm
  std::string Command=cgi.getElement("Command")->getValue();
  
  if ( Command == "TestDCStoFEDDpInterface" ) {
    xoap::MessageReference msg = MakeSOAPMessageReference("TestDCStoFEDDpInterface");
    xoap::MessageReference reply = TestDCStoFEDDpInterface(msg);
    if ( Receive(reply) != "TestDCStoFEDDpInterfaceDone" ) cout<<"The Test of the DCS to FED Interface could not be performed!"<<endl;
  } else if ( Command == "TestDCStoTrkFECDpInterface" ) {
    xoap::MessageReference msg = MakeSOAPMessageReference("TestDCStoTrkFECDpInterface");
    xoap::MessageReference reply = TestDCStoTrkFECDpInterface(msg);
    if (Receive(reply)!="TestDCStoTrkFECDpInterfaceDone") cout<<"The Test of the DCS to TrkFEC Interface could not be performed!"<<endl;
  } else if ( Command == "TestTrkFECSupervisor_DCUreadout" ) {
    xoap::MessageReference msg = MakeSOAPMessageReference("TestTrkFECSupervisor_DCUreadout");
    xoap::MessageReference reply = TestTrkFECSupervisor_DCUreadout(msg);
    if (Receive(reply)!="TestTrkFECSupervisor_DCUreadoutDone") cout<<"The Test of the TrkFEC Supervisor (DCU read-out) could not be performed!"<<endl;
  } else if ( Command == "TestTrkFECSupervisor_DCUreadout_fakeSOAP" ) {
    xoap::MessageReference msg = MakeSOAPMessageReference("TestTrkFECSupervisor_DCUreadout_fakeSOAP");
    xoap::MessageReference reply = TestTrkFECSupervisor_DCUreadout_fakeSOAP(msg);
    if (Receive(reply)!="TestTrkFECSupervisor_DCUreadoutDone") cout<<"The Test of the TrkFEC Supervisor (DCU read-out) with fakeSOAP could not be performed!"<<endl;
  } else if ( Command == "TestDCStoTrkFECDpInterfaceConfigure" ) {
    cout<< "// Sending SOAP message with Configure to PixelDCStoTrkFECDpInterface" <<endl;
    xoap::MessageReference msg = MakeSOAPMessageReference("TestDCStoTrkFECDpInterfaceConfigure");
    cout<< "call MakeSOAPMessageReference(\"TestDCStoTrkFECDpInterfaceConfigure\")" <<endl;
    xoap::MessageReference reply = TestDCStoTrkFECDpInterfaceConfigure(msg);
    if (Receive(reply)!="TestDCStoTrkFECDpInterfaceConfigureDone") cout<<"The Test of the DCS to TrkFEC Interface could not be performed!"<<endl;
  }

  this->Default(in, out);
}

//
//---------------------------------------------------------------------------------------------------
//

xoap::MessageReference PixelDCSTestApplication::TestDCStoFEDDpInterface(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
//--- send test Data-Point values 
//    to PixelDCStoFEDDpInterface
  xoap::MessageReference soapMessage = composeTestMessageFED();

  xdaq::ApplicationDescriptor* destination = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor(fedDpInterface_.toString(), 0);

  cout << "Sending SOAP request to FED DpInterface; URL = " << destination->getContextDescriptor()->getURL() << endl;
  cout <<" Request : ------------------------------------ "<<endl;
  soapMessage->writeTo(cout);
  cout << endl;
  cout <<" ---------------------------------------------- "<<endl;

  try {
    //xoap::MessageReference soapReply = getApplicationContext()->postSOAP(message, destination);
    xoap::MessageReference soapReply = this->getApplicationContext()->postSOAP(soapMessage, *this->getApplicationDescriptor(), *destination);

//--- print SOAP reply
    cout <<" Reply : -------------------------------------- "<<endl;
    soapReply->writeTo(cout);
    cout << endl;
    cout <<" ---------------------------------------------- "<<endl;
  } catch ( xoap::exception::Exception& e ) {
    cout << "SOAP Exception: " << e.what() << endl;
    return MakeSOAPMessageReference("TestDCStoFEDDpInterfaceFailed");
  } catch (...) {
    cout << "Caught unspecified exception" << endl;
    return MakeSOAPMessageReference("TestDCStoFEDDpInterfaceFailed");
  }
  
  return MakeSOAPMessageReference("TestDCStoFEDDpInterfaceDone");
}

xoap::MessageReference PixelDCSTestApplication::composeTestMessageFED()
{
   xoap::MessageReference message = xoap::createMessage();
   xoap::SOAPEnvelope envelope = message->getSOAPPart().getEnvelope();
   xoap::SOAPName command = envelope.createName("updateDpValueLastDAC", "xdaq", XDAQ_NS_URI);
   xoap::SOAPBody body = envelope.getBody();
   xoap::SOAPBodyElement commandElement = body.addBodyElement(command);
	
   unsigned int numDataPoints = 2;
   unsigned int fedBoardIds[]   = { 1, 4 };
   unsigned int fedChannelIds[] = { 2, 5 };
   unsigned int rocIds[]        = { 3, 6 };
   //unsigned int TempRangeDACs[] = { 0, 1 };
   float dpValues[] = { 400.0, 500.0 };

   for ( unsigned int iDataPoint = 0; iDataPoint < numDataPoints; ++iDataPoint ) {
     xoap::SOAPName fedBoard = envelope.createName("fedBoard");
     xoap::SOAPElement fedBoardElement = commandElement.addChildElement(fedBoard);
     xoap::SOAPName fedBoardId = envelope.createName("number");
     std::ostringstream fedBoardId_string;
     fedBoardId_string << fedBoardIds[iDataPoint];
     fedBoardElement.addAttribute(fedBoardId, fedBoardId_string.str());
     xoap::SOAPName dp = envelope.createName("dp");
     xoap::SOAPElement dpElement = fedBoardElement.addChildElement(dp);
     xoap::SOAPName fedChannelId = envelope.createName("fedChannel");
     std::ostringstream fedChannelId_string;
     fedChannelId_string << fedChannelIds[iDataPoint];
     dpElement.addAttribute(fedChannelId, fedChannelId_string.str());
     xoap::SOAPName rocId = envelope.createName("roc");
     std::ostringstream rocId_string;
     rocId_string << rocIds[iDataPoint];
     dpElement.addAttribute(rocId, rocId_string.str());

     //xoap::SOAPName TempRange = envelope.createName("TempRange");
     //std::ostringstream TempRange_string;
     //temprange_string << TempRangeDACs[iDataPoint];
     //dpElement.addAttribute(TempRange, TempRange_string.str());
     std::ostringstream dpValue_string;
//--- set number format to fixed floating point with precision of one decimal place 
//
//    WARNING: PixelDCStoFEDDpInterface fails to parse scientific number format 
//
     dpValue_string.setf(std::ios::fixed);
     dpValue_string << std::setprecision(1) << dpValues[iDataPoint];
     dpElement.addTextNode(dpValue_string.str());
   }

   return message;
}

xoap::MessageReference PixelDCSTestApplication::TestDCStoTrkFECDpInterface (xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  //--- sends test Data-Point values to PixelDCStoTrkFECDpInterface
  
  cout << "<PixelDCSTestApplication::TestDCStoTrkFECDpInterface>" << endl;

  std::ostringstream fname;
  //
  // NB: xmlbase and htmlbase point to other directories. Our data are in PixelDCSInterface/xml
  //
  // fname << getenv("BUILD_HOME") << "/" << "pixel/PixelDCSInterface/xml/" << "xdaqjcPID15965.log.dat";
  // fname << getenv("BUILD_HOME") << "/" << "pixel/PixelDCSInterface/xml/" << "xdaqjcPID15965.log";
  fname << getenv("BUILD_HOME") << "/" << "pixel/PixelDCSInterface/xml/" << "xdaqjcPID15965-with_dcuId.log";
  // fname << getenv("BUILD_HOME") << "/" << "pixel/PixelDCSInterface/xml/" << "Log_15Dec2009_08-14-45_GMT__PixelDCSFSMInterface.log";

  std::ifstream file(fname.str().c_str());
  if (!file) {
    cout<< "***ERROR PixelDCSTestApplication::TestDCStoTrkFECDpInterface: data file not found: " << fname.str() <<endl;
    return MakeSOAPMessageReference("TestDCStoTrkFECDpInterfaceFailed");
  }
  else {
    cout<< "PixelDCSTestApplication::TestDCStoTrkFECDpInterface: read DCU data from file " << fname.str() <<endl;
  }
  //////////////////////////////////////////////////////////////////////////////////////////////

  // variables for actual values
  std::string portcardName;
  unsigned dcuId(0), fecBoardId, mfecId, ccuId, ccuChannelId, mode, Vaa, Vdd, rtd2, rtd3, aoh, Vpc, Vbg, ts;
  // variables for text names
  std::string portcardName_;
  std::string dcuId_, fecBoardId_, mfecId_, ccuId_, ccuChannelId_, mode_, Vaa_, Vdd_, rtd2_, rtd3_, aoh_, Vpc_, Vbg_, ts_;
  std::string eq;

  // OLD output for each portcard in the log file consists of two lines. Example:
  // portcardName_ = FPix_BmI_D1_PRT1 fecBoardId_ = 18 mfecId_ = 5 ccuId_ = 126 ccuChannelId_ = 16
  // mode_ = 1 Vaa_ = 2390 Vdd_ = 3062 rtd2_ = 2598 rtd3_ = 2592 aoh_ = 2488 Vpc_ = 2690 Vbg_ = 2493 ts_ = 2747

  int nevents = 0;
  std::string line;
  for (std::streampos pos; pos=file.tellg(), std::getline(file, line); )    // save position in stream before current line
  {
    if (line.find("portcardName_ = ") == 0)     // line starts from "portcardName_ = "
    {
      // is data in old or new format: new format contains field 'dcuId_'
      bool with_dcuId = false;
      if (line.find(" dcuId_ = ") != std::string::npos) with_dcuId = true;

      // put stream pointer before this line and read data for all portcards
      file.seekg(pos);

      // dataset for the whole FPix
      std::vector<PortCard::AddressDCU> portCards;

      // if (debug_) cout<< "// PixelDCSTestApplication::TestDCStoTrkFECDpInterface: emulated data:" <<endl;

      for (int ipc=0; ipc<32; ++ipc)
      {
        if (with_dcuId) {
          file
            // first (address) line
            >> portcardName_ >> eq >> portcardName
            >> dcuId_ >> eq >> dcuId                    // new format
            >> fecBoardId_ >> eq >> fecBoardId
            >> mfecId_ >> eq >> mfecId
            >> ccuId_ >> eq >> ccuId
            >> ccuChannelId_ >> eq >> ccuChannelId
            // second (data) line
            >> mode_ >> eq >> mode
            >> Vaa_ >> eq >> Vaa
            >> Vdd_ >> eq >> Vdd
            >> rtd2_ >> eq >> rtd2
            >> rtd3_ >> eq >> rtd3
            >> aoh_ >> eq >> aoh
            >> Vpc_ >> eq >> Vpc
            >> Vbg_ >> eq >> Vbg
            >> ts_ >> eq >> ts
          ;
        }
        else {
          // initialize variables absent in the old format
          dcuId_ = "dcuId_";
          dcuId = 0;
          file
            // first (address) line
            >> portcardName_ >> eq >> portcardName
            >> fecBoardId_ >> eq >> fecBoardId
            >> mfecId_ >> eq >> mfecId
            >> ccuId_ >> eq >> ccuId
            >> ccuChannelId_ >> eq >> ccuChannelId
            // second (data) line
            >> mode_ >> eq >> mode
            >> Vaa_ >> eq >> Vaa
            >> Vdd_ >> eq >> Vdd
            >> rtd2_ >> eq >> rtd2
            >> rtd3_ >> eq >> rtd3
            >> aoh_ >> eq >> aoh
            >> Vpc_ >> eq >> Vpc
            >> Vbg_ >> eq >> Vbg
            >> ts_ >> eq >> ts
          ;
        }

        double Trtd2 = (2./3850.e-6) * (1. - double(rtd2)/double(Vpc));
        double Trtd3 = (2./3850.e-6) * (1. - double(rtd3)/double(Vpc));
        double Taoh  = (2./3850.e-6) * (1. - double(aoh)/double(Vpc));

        //
        //  test printout
        //
        if (debug_) {
          cout
            << ipc << "\t "
            <<" "<< portcardName_ <<" "<< portcardName
            // <<" "<< dcuId_ <<" "<< dcuId
            // <<" "<< fecBoardId_ <<" "<< fecBoardId
            // <<" "<< mfecId_ <<" "<< mfecId
            // <<" "<< ccuId_ <<" "<< ccuId
            // <<" "<< ccuChannelId_ <<" "<< ccuChannelId
            <<" "<< mode_ <<" "<< mode
            // <<" "<< Vaa_ <<" "<< Vaa
            // <<" "<< Vdd_ <<" "<< Vdd
            <<" "<< rtd2_ <<" "<< rtd2
            <<" "<< rtd3_ <<" "<< rtd3
            <<" "<< aoh_ <<" "<< aoh
            <<" "<< Vpc_ <<" "<< Vpc
            // <<" "<< Vbg_ <<" "<< Vbg
            // <<" "<< ts_ <<" "<< ts

            << " Trtd2 = " << Trtd2 << " Trtd3 = " << Trtd3 << " Taoh = " << Taoh
            <<endl
          ;
        }

        PortCard::AddressDCU addressDCU;

        addressDCU.address_.portcardName_  = portcardName;
        addressDCU.address_.dcuId_         = dcuId;
        addressDCU.address_.fecBoardId_    = fecBoardId;
        addressDCU.address_.mfecId_        = mfecId;
        addressDCU.address_.ccuId_         = ccuId;
        addressDCU.address_.ccuChannelId_  = ccuChannelId;
        addressDCU.dcu_.mode_ = 1;
        addressDCU.dcu_.Vaa_  = Vaa;
        addressDCU.dcu_.Vdd_  = Vdd;
        addressDCU.dcu_.rtd2_ = rtd2;
        addressDCU.dcu_.rtd3_ = rtd3;
        addressDCU.dcu_.aoh_  = aoh;
        addressDCU.dcu_.Vpc_  = Vpc;
        addressDCU.dcu_.Vbg_  = Vbg;
        addressDCU.dcu_.ts_   = ts;

        portCards.push_back(addressDCU);
      }
      // end of event
      ++nevents;
      if (debug_) cout<< "End of event " << nevents <<endl;
      // cout<< "file.eof() = " << file.eof() <<endl;

      // prepare message with this data

      xoap::MessageReference soapMessage = PortCard::SOAP_ReadAll::Make(PortCard::SOAP_ReadAll::name_command_, portCards);

      //xoap::MessageReference soapMessage = composeTestMessageTrkFEC();

      xdaq::ApplicationDescriptor* destination = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor(trkfecDpInterface_.toString(), 0);

      // cout << "Sending SOAP request to TrkFEC DpInterface; URL = " << destination->getContextDescriptor()->getURL() << endl;
      // cout <<" Request : ------------------------------------ "<<endl;
      // soapMessage->writeTo(cout);
      // cout << endl;
      // cout <<" ---------------------------------------------- "<<endl;

      try {
        //xoap::MessageReference soapReply = getApplicationContext()->postSOAP(message, destination);
        xoap::MessageReference soapReply = this->getApplicationContext()->postSOAP(soapMessage, *this->getApplicationDescriptor(), *destination);

        //--- print SOAP reply
        // cout <<" Reply : -------------------------------------- "<<endl;
        // soapReply->writeTo(cout);
        // cout << endl;
        // cout <<" ---------------------------------------------- "<<endl;
      } catch ( xoap::exception::Exception& e ) {
        cout << "SOAP Exception: " << e.what() << endl;
        return MakeSOAPMessageReference("TestDCStoTrkFECDpInterfaceFailed");
      } catch (...) {
        cout << "Caught unspecified exception" << endl;
        return MakeSOAPMessageReference("TestDCStoTrkFECDpInterfaceFailed");
      }
    }
  }
  return MakeSOAPMessageReference("TestDCStoTrkFECDpInterfaceDone");
}

xoap::MessageReference PixelDCSTestApplication::composeTestMessageTrkFEC()
{
//    unsigned int numPortCards = 2;
//    unsigned int fecBoardIds[]   = { 1, 4 };
//    unsigned int mfecIds[]       = { 3, 7 };
//    unsigned int ccuIds[]        = { 2, 5 };
//    unsigned int ccuChannelIds[] = { 3, 6 };
//    unsigned int adcValues[2][8] = { { 0, 1,  2,  3,  4,  5,  6,  7 },
//                                     { 8, 9, 10, 11, 12, 13, 14, 15 } };

  // Few remarks on emulated values:
  // 1) we'll use useless Vbg to number data points
  // 2) T in degrees of Celsius is T = (2./3850e-6)*(1 - ADC/Vpc), where ADC is RTD2, RTD3 or AOH
  // 3) ADC = Vpc*(1 - T*(3850e-6/2)), where T in degrees of Celsius and ADC is RTD2, RTD3 or AOH
  unsigned int numPortCards = 5;
  unsigned int fecBoardIds[]   = {  12,   12,   12,   12,   12  };
  unsigned int mfecIds[]       = {  8,    8,    8,    8,    8   };
  unsigned int ccuIds[]        = {  125,  125,  125,  125,  125 };
  unsigned int ccuChannelIds[] = {  17,   17,   17,   17,   17  };
  unsigned int adcValues[][8] = {
    //  0     1     2     3     4     5     6     7
    //  Vaa   Vdd   RTD2  RTD3  AOH   Vpc   Vbg   TS
      { 480,  27,   2602, 2596, 2544, 2700, 1,    2637 }    // RTD2=20, AOH=30
    , { 480,  27,   2602, 2596, 2544, 2700, 2,    2637 }    // the same
    , { 480,  27,   2602, 2590, 2544, 2700, 3,    2637 }    // RTD2=21, AOH=30(same)
    , { 480,  27,   2602, 2590, 2544, 2700, 4,    2637 }    // same
    , { 480,  27,   2602, 2590, 2538, 2700, 5,    2637 }    // RTD2=21(same), AOH=31
  };

  std::vector<PortCard::AddressDCU> portCards;
  for ( unsigned int iPortCard = 0; iPortCard < numPortCards; ++iPortCard ) {
    PortCard::AddressDCU addressDCU;

    char portCardName[20];
    sprintf(portCardName, "test_PortCard_%02d", iPortCard);

    addressDCU.address_.portcardName_  = portCardName;
    addressDCU.address_.fecBoardId_    = fecBoardIds[iPortCard];
    addressDCU.address_.mfecId_        = mfecIds[iPortCard];
    addressDCU.address_.ccuId_         = ccuIds[iPortCard];
    addressDCU.address_.ccuChannelId_  = ccuChannelIds[iPortCard];
    addressDCU.dcu_.mode_ = 1;
    addressDCU.dcu_.Vaa_  = adcValues[iPortCard][0];
    addressDCU.dcu_.Vdd_  = adcValues[iPortCard][1];
    addressDCU.dcu_.rtd2_ = adcValues[iPortCard][2];
    addressDCU.dcu_.rtd3_ = adcValues[iPortCard][3];
    addressDCU.dcu_.aoh_  = adcValues[iPortCard][4];
    addressDCU.dcu_.Vpc_  = adcValues[iPortCard][5];
    addressDCU.dcu_.Vbg_  = adcValues[iPortCard][6];
    addressDCU.dcu_.ts_   = adcValues[iPortCard][7];

    portCards.push_back(addressDCU);
  }

  xoap::MessageReference soapMessage = PortCard::SOAP_ReadAll::Make(PortCard::SOAP_ReadAll::name_command_, portCards);

  std::vector<PortCard::AddressDCU> portCards_decoded = PortCard::SOAP_ReadAll::Decode(soapMessage);
  for ( std::vector<PortCard::AddressDCU>::const_iterator addressDCU = portCards_decoded.begin();
	addressDCU != portCards_decoded.end(); ++addressDCU ) {
    addressDCU->print(std::cout);
  }
  
  return soapMessage;
}

xoap::MessageReference PixelDCSTestApplication::TestTrkFECSupervisor_DCUreadout_fakeSOAP(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  // xoap::MessageReference message = MakeSOAPMessageReference("readDCU");
  xoap::MessageReference message = MakeSOAPMessageReference("ReadDCU_workloop_fakeSOAP");
  
  cout<< "PixelDCSTestApplication::TestTrkFECSupervisor_DCUreadout_fakeSOAP" <<endl;

  // get ApplicationDescriptor from information which comes from xml file TriDAS/pixel/XDAQConfiguration/ConfigurationNoRU.xml
  // Corresponding lines in xml file look like that:
  //
  // <xc:Application class="PixelTKFECSupervisor" id="100" instance="1" network="local" group="daq"/>
  // <xc:Module>${BUILD_HOME}/pixel/PixelTKFECSupervisor/lib/linux/x86/libPixelTKFECSupervisor.so</xc:Module>
  //
  // the important point that the second parameter in getApplicationDescriptor("PixelTKFECSupervisor", 1)
  // should correspond to instance. E.g. you can read in the first line: instance="1"
  xdaq::ApplicationDescriptor* destination = getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor("PixelTKFECSupervisor", 1);
  
  //   Splitting up of getting destination line to found a cause of crash
  //
  //   cout<< "PixelDCSTestApplication::TestTrkFECSupervisor_DCUreadout: after MakeSOAPMessageReference" <<endl;
  //   //
  //   //-- crash occurs in getApplicationDescriptor("PixelTKFECSupervisor", 0) bacause the instance should be 1: getApplicationDescriptor("PixelTKFECSupervisor", 1)
  //   // xdaq::ApplicationDescriptor* destination = getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor("PixelTKFECSupervisor", 1);
  //   xdaq::ApplicationDescriptor* destination = 0;
  //   //destination = getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor("PixelTKFECSupervisor", 0);
  //   //cout<< "PixelDCSTestApplication::TestTrkFECSupervisor_DCUreadout: after getApplicationContext()->getDefaultZone()->getApplicationGroup(\"daq\")->getApplicationDescriptor(\"PixelTKFECSupervisor\", 0)" <<endl;
  //   //
  //   xdaq::ApplicationGroup * group = getApplicationContext()->getDe/PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.hfaultZone()->getApplicationGroup("daq");
  //   //std::vector<xdaq::ApplicationDescriptor*> descriptors = group->getApplicationDescriptors("PixelTKFECSupervisor");
  //   std::set<xdaq::ApplicationDescriptor*> descriptors = group->getApplicationDescriptors("PixelTKFECSupervisor");
  //   cout<< "PixelDCSTestApplication::TestTrkFECSupervisor_DCUreadout: after getApplicationDescriptors" <<endl;
  //   //
  //   cout<< "descriptors.size() = " << descriptors.size() <<endl;
  //   std::set<xdaq::ApplicationDescriptor*>::iterator set_it = descriptors.begin();    // NB: error in wiki page (vector)
  //   destination = *set_it;
  
  cout << "Sending SOAP request to TrkFEC Supervisor; URL = " << destination->getContextDescriptor()->getURL() << endl;
  cout <<" Request : ------------------------------------ "<<endl;
  message->writeTo(cout);
  cout << endl;
  message->writeTo(request_);
  cout <<" ---------------------------------------------- "<<endl;

  try {
    xoap::MessageReference reply = getApplicationContext()->postSOAP(message, destination);

    //--- print SOAP reply
    cout <<" Reply : -------------------------------------- "<<endl;
    reply->writeTo(cout);
    cout << endl;
    reply->writeTo(response_);
    // decode SOAP message
    std::vector<PortCard::AddressDCU> vdcu = PortCard::SOAP_ReadAll::Decode(reply);
    for (std::vector<PortCard::AddressDCU>::const_iterator addressDCU=vdcu.begin(); addressDCU!=vdcu.end(); ++addressDCU) {
      dcu_map_[addressDCU->address_] = addressDCU->dcu_;
    }
    cout <<" ---------------------------------------------- "<<endl;
  } catch (xoap::exception::Exception soape) {
    cout << "SOAP Exception: " << soape.what() << endl;
    return MakeSOAPMessageReference("Error");
  } catch (...) {
    cout << "Caught unspecified exception" << endl;
    return MakeSOAPMessageReference("Error");
  }
  
  return MakeSOAPMessageReference("TestTrkFECSupervisor_DCUreadoutDone");
}

xoap::MessageReference PixelDCSTestApplication::TestTrkFECSupervisor_DCUreadout(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  xoap::MessageReference message = MakeSOAPMessageReference("readDCU");
  
  cout<< "PixelDCSTestApplication::TestTrkFECSupervisor_DCUreadout" <<endl;

  // get ApplicationDescriptor from information which comes from xml file TriDAS/pixel/XDAQConfiguration/ConfigurationNoRU.xml
  // Corresponding lines in xml file look like that:
  //
  // <xc:Application class="PixelTKFECSupervisor" id="100" instance="1" network="local" group="daq"/>
  // <xc:Module>${BUILD_HOME}/pixel/PixelTKFECSupervisor/lib/linux/x86/libPixelTKFECSupervisor.so</xc:Module>
  //
  // the important point that the second parameter in getApplicationDescriptor("PixelTKFECSupervisor", 1)
  // should correspond to instance. E.g. you can read in the first line: instance="1"
  xdaq::ApplicationDescriptor* destination = getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor("PixelTKFECSupervisor", 1);
  
  //   Splitting up of getting destination line to found a cause of crash
  //
  //   cout<< "PixelDCSTestApplication::TestTrkFECSupervisor_DCUreadout: after MakeSOAPMessageReference" <<endl;
  //   //
  //   //-- crash occurs in getApplicationDescriptor("PixelTKFECSupervisor", 0) bacause the instance should be 1: getApplicationDescriptor("PixelTKFECSupervisor", 1)
  //   // xdaq::ApplicationDescriptor* destination = getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor("PixelTKFECSupervisor", 1);
  //   xdaq::ApplicationDescriptor* destination = 0;
  //   //destination = getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor("PixelTKFECSupervisor", 0);
  //   //cout<< "PixelDCSTestApplication::TestTrkFECSupervisor_DCUreadout: after getApplicationContext()->getDefaultZone()->getApplicationGroup(\"daq\")->getApplicationDescriptor(\"PixelTKFECSupervisor\", 0)" <<endl;
  //   //
  //   xdaq::ApplicationGroup * group = getApplicationContext()->getDe/PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.hfaultZone()->getApplicationGroup("daq");
  //   //std::vector<xdaq::ApplicationDescriptor*> descriptors = group->getApplicationDescriptors("PixelTKFECSupervisor");
  //   std::set<xdaq::ApplicationDescriptor*> descriptors = group->getApplicationDescriptors("PixelTKFECSupervisor");
  //   cout<< "PixelDCSTestApplication::TestTrkFECSupervisor_DCUreadout: after getApplicationDescriptors" <<endl;
  //   //
  //   cout<< "descriptors.size() = " << descriptors.size() <<endl;
  //   std::set<xdaq::ApplicationDescriptor*>::iterator set_it = descriptors.begin();    // NB: error in wiki page (vector)
  //   destination = *set_it;
  
  cout << "Sending SOAP request to TrkFEC Supervisor; URL = " << destination->getContextDescriptor()->getURL() << endl;
  cout <<" Request : ------------------------------------ "<<endl;
  message->writeTo(cout);
  cout << endl;
  message->writeTo(request_);
  cout <<" ---------------------------------------------- "<<endl;

  try {
    xoap::MessageReference reply = getApplicationContext()->postSOAP(message, destination);

    //--- print SOAP reply
    cout <<" Reply : -------------------------------------- "<<endl;
    reply->writeTo(cout);
    cout << endl;
    reply->writeTo(response_);
    // decode SOAP message
    std::vector<PortCard::AddressDCU> vdcu = PortCard::SOAP_ReadAll::Decode(reply);
    for (std::vector<PortCard::AddressDCU>::const_iterator addressDCU=vdcu.begin(); addressDCU!=vdcu.end(); ++addressDCU) {
      dcu_map_[addressDCU->address_] = addressDCU->dcu_;
    }
    cout <<" ---------------------------------------------- "<<endl;
  } catch (xoap::exception::Exception soape) {
    cout << "SOAP Exception: " << soape.what() << endl;
    return MakeSOAPMessageReference("Error");
  } catch (...) {
    cout << "Caught unspecified exception" << endl;
    return MakeSOAPMessageReference("Error");
  }
  
  return MakeSOAPMessageReference("TestTrkFECSupervisor_DCUreadoutDone");
}

xoap::MessageReference PixelDCSTestApplication::TestDCStoTrkFECDpInterfaceConfigure(xoap::MessageReference msg) throw (xoap::exception::Exception)
{
  //--- calls PixelDCStoTrkFECDpInterface::Configure
  
  cout<< "<PixelDCSTestApplication::TestDCStoTrkFECDpInterfaceConfigure>" <<endl;

  // prepare message with this data

  //xoap::MessageReference soapMessage = PortCard::SOAP_ReadAll::Make(PortCard::SOAP_ReadAll::name_command_, portCards);
  //xoap::MessageReference soapMessage = composeTestMessageTrkFEC();
  
  xoap::MessageReference soapMessage = MakeSOAPMessageReference("Configure");

  xdaq::ApplicationDescriptor* destination = getApplicationContext()->getDefaultZone()->getApplicationGroup("dcs")->getApplicationDescriptor(trkfecDpInterface_.toString(), 0);
  
  cout << "Sending SOAP request to TrkFEC DpInterface; URL = " << destination->getContextDescriptor()->getURL() << endl;
  cout <<" Request : ------------------------------------ "<<endl;
  soapMessage->writeTo(cout);
  cout << endl;
  cout <<" ---------------------------------------------- "<<endl;
  
  try {
    //xoap::MessageReference soapReply = getApplicationContext()->postSOAP(message, destination);
    xoap::MessageReference soapReply = this->getApplicationContext()->postSOAP(soapMessage, *this->getApplicationDescriptor(), *destination);
  
    //--- print SOAP reply
    // cout <<" Reply : -------------------------------------- "<<endl;
    // soapReply->writeTo(cout);
    // cout << endl;
    // cout <<" ---------------------------------------------- "<<endl;
  } catch ( xoap::exception::Exception& e ) {
    cout << "SOAP Exception: " << e.what() << endl;
    return MakeSOAPMessageReference("TestDCStoTrkFECDpInterfaceConfigureFailed");
  } catch (...) {
    cout << "Caught unspecified exception" << endl;
    return MakeSOAPMessageReference("TestDCStoTrkFECDpInterfaceConfigureFailed");
  }
  return MakeSOAPMessageReference("TestDCStoTrkFECDpInterfaceConfigureDone");
}
