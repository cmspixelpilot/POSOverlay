// $Id: PixelTemperatureCalibrationPluginDCU.cc,v 1.7 2011/01/27 09:21:12 joshmt Exp $

/*************************************************************************
 * Class for DCU calibration routines,                                   *
 * implemented as plug-in for PixelTemperatureCalibration class          *
 *                                                                       *
 * Author: Christian Veelken, UC Davis                                   *
 *                                                                       *
 * Last update: $Date: 2011/01/27 09:21:12 $ (UTC)                       *
 *          by: $Author: joshmt $                                       *
 *************************************************************************/

#include "PixelTemperatureCalibrationPluginDCU.h"

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"

#include <iostream>

using std::cout;      
using std::endl;

PixelTemperatureCalibrationPluginDCU::PixelTemperatureCalibrationPluginDCU(PixelSupervisorConfiguration* globalConfigurationParameters,
									   SOAPCommander* soapCommander, 
									   PixelDCSSOAPCommander* dcs_soapCommander, 
									   PixelDCSPVSSCommander* pvssCommander,
									   xercesc::DOMNode* pluginConfigNode)
  : PixelTemperatureCalibrationPlugin(globalConfigurationParameters, soapCommander, dcs_soapCommander, pvssCommander, pluginConfigNode)
{
  loadConfiguration();
}
  
PixelTemperatureCalibrationPluginDCU::~PixelTemperatureCalibrationPluginDCU()
{
}
  
void PixelTemperatureCalibrationPluginDCU::execute(unsigned int iCycle, unsigned int numCycles) throw (xdaq::exception::Exception)
{
  // TODO: specify mode (LIR or HIR). Never used now.
  
  std::cout << "<PixelTemperatureCalibrationPluginDCU::execute>:" << std::endl;
  
  // We need to send requests to readout DCU to all TKFEC crates
  // The list of TKFEC crates as well as other information needed to get it can be obtained from detector configuration object pos::PixelCalibConfiguration

  // get detector configuration objects
  const pos::PixelCalibConfiguration* calibrationParameters = dynamic_cast<const pos::PixelCalibConfiguration*>(globalConfigurationParameters_->getCalibObject());
  if ( calibrationParameters == 0 ) {
    XCEPT_RAISE (xcept::Exception, "Undefined Calibration Parameters");
  }

  // use calibrationParameters to get parameters
  const pos::PixelTKFECConfig* trkfecConfiguration = globalConfigurationParameters_->getTKFECConfiguration();
  const pos::PixelPortcardMap* portCardMap = globalConfigurationParameters_->getPortcardMap();
  std::map<std::string,pos::PixelPortCardConfig*> portCardNameMap = *(globalConfigurationParameters_->getmapNamePortCard());
  // to obtain list of TKFEC crates
  std::set<unsigned int> trkfecCrates = calibrationParameters->getTKFECCrates(portCardMap, portCardNameMap, trkfecConfiguration);
  
  // // test section
  //
  // // print Port Card names
  //
  // for (std::map<std::string,pos::PixelPortCardConfig*>::iterator portcard=portCardNameMap.begin(); portcard!=portCardNameMap.end(); ++portcard)
  // {
  //   cout<< "PixelTemperatureCalibrationPluginDCU::execute: name in portCardNameMap portcard->first = " << portcard->first <<endl;
  // }
  
  // remove old data from the map
  address_vpoints_map_.clear();

  for (unsigned int iPoint=0; iPoint<numPoints_; ++iPoint)
  {
    cout<< "PixelTemperatureCalibrationPluginDCU::execute: iPoint = " << iPoint <<endl;
    
    // read-out DCU temperature and voltage values;
    // compose SOAP message
    xoap::MessageReference soapRequest = dcs_soapCommander_->MakeSOAPMessageReference_readDCU();
    //    send SOAP message
    //    to all TrkFEC supervisors (crates)
    for ( std::set<unsigned int>::const_iterator trkfecCrate = trkfecCrates.begin();
	  trkfecCrate != trkfecCrates.end(); ++trkfecCrate ) {
      // get descriptor of current crate
      xdaq::ApplicationDescriptor* trkfecDescriptor = const_cast<xdaq::ApplicationDescriptor*>(globalConfigurationParameters_->getPixelTKFECDescriptor(*trkfecCrate));
      // send SOAP message
      xoap::MessageReference soapResponse = dcs_soapCommander_->postSOAP(trkfecDescriptor, soapRequest);
      // analyze response: write results to the map
      decodeDCUReadings(soapResponse);    
    
      // cout<< "PixelTemperatureCalibrationPluginDCU::execute: content of map address_vpoints_map_ after decoder" <<endl;
      // for (std::map<PortCard::Address, std::vector<PortCard::DCU> >::const_iterator map_it = address_vpoints_map_.begin(); map_it != address_vpoints_map_.end(); ++map_it)
      // {
      //   cout<< "// PixelTemperatureCalibrationPluginDCU::execute: map_it->first.portcardName() = " << map_it->first.portcardName() << " has entries map_it->second.size() = " << map_it->second.size() <<endl;
      // }
    }
  }
  
  cout<< "PixelTemperatureCalibrationPluginDCU::execute: the number of elements in the map is " << address_vpoints_map_.size() <<endl;
  for (std::map<PortCard::Address, std::vector<PortCard::DCU> >::iterator it=address_vpoints_map_.begin(); it!=address_vpoints_map_.end(); ++it)
  {
    cout<< "PixelTemperatureCalibrationPluginDCU::execute: Port Card #" << std::distance(address_vpoints_map_.begin(), it) << " has " << it->second.size() << " data points" <<endl;
  }

  // store DCU values in calibration DataBase
  archiveCalibrationData(*dataFile_);
}

void PixelTemperatureCalibrationPluginDCU::loadConfiguration() throw (xdaq::exception::Exception)
{
  // read mapping between PVSS and XDAQ states and commands
  // for given FSM node type (logical or control unit)

  std::cout << "<PixelTemperatureCalibrationPluginDCU::loadConfiguration>:" << std::endl;

  std::string pluginType = xoap::getNodeAttribute(pluginConfigNode_, "type");
  if ( pluginType != "PixelTemperatureCalibrationPluginDCU" ) {
    XCEPT_RAISE (xcept::Exception, "Undefined Plug-in Type");
  }

  bool numPoints_initialized = false;
  bool mode_initialized = false;

  DOMNodeList* parameterNodes = pluginConfigNode_->getChildNodes();
  unsigned int numParameterNodes = parameterNodes->getLength();
  for ( unsigned int iNode = 0; iNode < numParameterNodes; ++iNode ) {
    DOMNode* parameterNode = parameterNodes->item(iNode);
    
    // skip empty lines
    // (not removed by DOM parser)
    if ( xoap::XMLCh2String(parameterNode->getLocalName()) == "" ) continue;

    if ( xoap::XMLCh2String(parameterNode->getPrefix())    == "plugin"    &&
	 xoap::XMLCh2String(parameterNode->getLocalName()) == "parameter" ) {
      if ( xoap::getNodeAttribute(parameterNode, "numPoints") != ""  ) {
	numPoints_ = atoi(xoap::getNodeAttribute(parameterNode, "numPoints").data());
	numPoints_initialized = true;
      } else if ( xoap::getNodeAttribute(parameterNode, "mode") != ""  ) {
	mode_ = xoap::getNodeAttribute(parameterNode, "mode");
	mode_initialized = true;
      } else {
	XCEPT_RAISE (xcept::Exception, "Undefined Parameter");
      }
    } else {
      XCEPT_RAISE (xcept::Exception, "Error parsing config File");
    }
  }

  if ( !(true
	&& numPoints_initialized
        && mode_initialized
      ) ) {
    XCEPT_RAISE (xcept::Exception, "Parameter definitions missing");
  }

  printConfiguration(std::cout);
}

void PixelTemperatureCalibrationPluginDCU::printConfiguration(std::ostream& stream) const
{
  stream << "<PixelTemperatureCalibrationPluginDCU::printConfiguration>:" << std::endl;
  stream << " numPoints = " << numPoints_ << std::endl;
  stream << " mode = " << mode_ << std::endl;
}

void PixelTemperatureCalibrationPluginDCU::archiveConfigurationParameters(std::ostream& stream) const
{
  stream << "  <TemperatureCalibrationPluginDCU>" << std::endl;
  stream << "   <numPoints>" << numPoints_ << "</numPoints>" << std::endl;
  stream << "   <mode>" << mode_ << "</mode>" << std::endl;
  stream << "  </TemperatureCalibrationPluginDCU>" << std::endl;
}

void PixelTemperatureCalibrationPluginDCU::decodeDCUReadings(xoap::MessageReference soapMessage)
{
  // unpacks SOAP message and adds it contents to data map
  
  std::vector<PortCard::AddressDCU> vdcu = PortCard::SOAP_ReadAll::Decode(soapMessage);
  //cout<< "PixelTemperatureCalibrationPluginDCU::decodeDCUReadings: vdcu.size() = " << vdcu.size() <<endl;
  for (std::vector<PortCard::AddressDCU>::const_iterator addressDCU=vdcu.begin(); addressDCU!=vdcu.end(); ++addressDCU)
  {
    PortCard::Address address = addressDCU->address_;
    PortCard::DCU dcu = addressDCU->dcu_;
    
    //cout<< "PixelTemperatureCalibrationPluginDCU::decodeDCUReadings: current portcard is \"" << address.portcardName() << "\"" <<endl;
    //cout<< "PixelTemperatureCalibrationPluginDCU::decodeDCUReadings: before adding to map: address_vpoints_map_.count(address) = " << address_vpoints_map_.count(address) <<endl;
    
    // do we have Port Card with this address in the map?
    std::map<PortCard::Address, std::vector<PortCard::DCU> >::iterator map_it = address_vpoints_map_.find(address);
    
    if (map_it != address_vpoints_map_.end()) {
      // we have Port Card with this address in the map. Get vector of data points
      //cout<< "//we have Port Card with this address in the map. Get vector of data points" <<endl;
      map_it->second.push_back(dcu);
    }
    else {
      // we do not have Port Card with this address in the map. Add it
      //cout<< "//we do not have Port Card with this address in the map. Add it" <<endl;
      std::vector<PortCard::DCU> vpoints;
      vpoints.push_back(dcu);
      address_vpoints_map_[address] = vpoints;
    }
    //cout<< "PixelTemperatureCalibrationPluginDCU::decodeDCUReadings:  after adding to map: address_vpoints_map_.count(address) = " << address_vpoints_map_.count(address) <<endl;
  }
    
  //   cout<< "// PixelTemperatureCalibrationPluginDCU::decodeDCUReadings: finihed decoding, address_vpoints_map_.size() = " << address_vpoints_map_.size() <<endl;
  //   for (std::map<PortCard::Address, std::vector<PortCard::DCU> >::const_iterator it_test = address_vpoints_map_.begin(); 
  //     it_test != address_vpoints_map_.end(); ++it_test)
  //   {
  //     PortCard::Address address = it_test->first;
  //     std::vector<PortCard::DCU> v_dcu = it_test->second;
  //     cout<< "   // Port Card " << address.prints() << " has " << v_dcu.size() << " elements" <<endl;
  //     for (std::vector<PortCard::DCU>::iterator i_dcu = v_dcu.begin(); i_dcu != v_dcu.end(); ++i_dcu)
  //     {
  //       cout<< "      dcu #" << std::distance(v_dcu.begin(), i_dcu) << "\t " << i_dcu->prints() <<endl;;
  //     }
  //   }
}

void PixelTemperatureCalibrationPluginDCU::archiveCalibrationData(std::ostream& stream) const
{
  cout<< "<PixelTemperatureCalibrationPluginDCU::archiveCalibrationData>" <<endl;
  
  std::string indent = " ";
  std::string indent_start = "  ";
  std::string bra = indent_start + "<";
  std::string ket = ">";
  
  stream << bra << "dataSet type=\"DCU\"" << " mode=\"" << mode_ << "\">" << endl;
  cout   << bra << "dataSet type=\"DCU\"" << " mode=\"" << mode_ << "\">" << endl;
  
  for (std::map<PortCard::Address, std::vector<PortCard::DCU> >::const_iterator it_map=address_vpoints_map_.begin(); it_map!=address_vpoints_map_.end(); ++it_map)
  {
    const PortCard::Address& address        = it_map->first;
    const std::vector<PortCard::DCU>& vdcu  = it_map->second;
    
    // increase indent
    bra.insert(0, indent);
    
    // <FPix_BmO_D1_PRT1>
    stream << bra << address.portcardName_ << ket <<endl;
    cout   << bra << address.portcardName_ << ket <<endl;
    
    // increase indent
    bra.insert(0, indent);
    
    for (unsigned ch=0; ch<8; ++ch)
    {
      // <Vaa>
      stream << bra << PortCard::DCU::name_chan(ch) << ket <<endl;
      cout   << bra << PortCard::DCU::name_chan(ch) << ket <<endl;

      // increase indent
      bra.insert(0, indent);

      for (std::vector<PortCard::DCU>::const_iterator it_dcu=vdcu.begin(); it_dcu!=vdcu.end(); ++it_dcu)
      {
        stream << bra << it_dcu->GetChan(ch) << ket <<endl;
        cout   << bra << it_dcu->GetChan(ch) << ket <<endl;
      }

      // decrease indent
      if (bra.find(indent, 0) != std::string::npos) bra.erase(0, indent.size());
      // </Vaa>
      stream << bra << "/" << PortCard::DCU::name_chan(ch) << ket <<endl;
      cout   << bra << "/" << PortCard::DCU::name_chan(ch) << ket <<endl;
    }
    
    // decrease indent
    if (bra.find(indent, 0) != std::string::npos) bra.erase(0, indent.size());
    
    // </FPix_BmO_D1_PRT1>
    stream << bra << "/" << address.portcardName_ << ket <<endl;
    cout   << bra << "/" << address.portcardName_ << ket <<endl;
    
    // decrease indent
    if (bra.find(indent, 0) != std::string::npos) bra.erase(0, indent.size());
  }

  stream << "  </dataSet>" << std::endl;
  cout   << "  </dataSet>" << std::endl;
}
