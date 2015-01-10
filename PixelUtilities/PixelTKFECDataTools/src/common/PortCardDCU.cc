/*************************************************************************
 * Class describes DCU used on CMS Pixel Port Card                       *
 *                                                                       *
 * Author: Andriy Zatserklyaniy, University of Puerto Rico		 *
 *                                                                       *
 *************************************************************************/

#include "PixelUtilities/PixelTKFECDataTools/include/PortCardDCU.h"

#include "xdaq/Application.h"
#include "xdaq/exception/Exception.h"
#include "xoap/MessageReference.h"

#include <sstream>
#include <iostream>
#include <cassert>

using std::cout; using std::endl;

namespace PortCard {

// General purpose string --> unsigned and unsigned --> string conversion functions
std::string unsigned2string(unsigned number) {
  std::stringstream ss;
  ss << number;
  return ss.str();
}
unsigned string2unsigned(std::string str) {
  std::stringstream ss(str);
  unsigned number = 0;                // NB: would not be changed in case of conversion error
  if (ss >> number) return number;
  else return 0;
}

//////////////////////////////
//
//  class DCU
//
//////////////////////////////

//
//  initializing of static fields of class DCU
//
const double DCU::kPt_ = 3850.e-6;
const std::string DCU::name_modeDCU   = "DCUmode";
const std::string DCU::name_modeValue = "DCUmodeValue";
const std::string DCU::name_modeLIR   = "DCUmodeLIR";
const std::string DCU::name_modeHIR   = "DCUmodeHIR";
const std::string DCU::name_Vaa       = "Vaa";
const std::string DCU::name_Vdd       = "Vdd";
const std::string DCU::name_RTD2      = "RTD2";
const std::string DCU::name_RTD3      = "RTD3";
const std::string DCU::name_AOH       = "AOH";
const std::string DCU::name_Vpc       = "Vpc";
const std::string DCU::name_Vbg       = "Vbg";
const std::string DCU::name_TS        = "TS";
const std::string DCU::name_wrong     = "";

DCU::DCU(): mode_(0), Vaa_(0), Vdd_(0), rtd2_(0), rtd3_(0), aoh_(0), Vpc_(0), Vbg_(0), ts_(0) {}
DCU::DCU(unsigned mode, unsigned Vaa, unsigned Vdd, unsigned rtd2, unsigned rtd3, unsigned aoh, unsigned Vpc, unsigned Vbg, unsigned ts):
  mode_(mode), Vaa_(Vaa), Vdd_(Vdd), rtd2_(rtd2), rtd3_(rtd3), aoh_(aoh), Vpc_(Vpc), Vbg_(Vbg), ts_(ts)
{}
void DCU::FillVector(std::vector<unsigned>& vchan) const {
  vchan.push_back(Vaa_);
  vchan.push_back(Vdd_);
  vchan.push_back(rtd2_);
  vchan.push_back(rtd3_);
  vchan.push_back(aoh_);
  vchan.push_back(Vpc_);
  vchan.push_back(Vbg_);
  vchan.push_back(ts_);
}
void DCU::SetADC(unsigned adc[]) {
  Vaa_  = adc[0];
  Vdd_  = adc[1];
  rtd2_ = adc[2];
  rtd3_ = adc[3];
  aoh_  = adc[4];
  Vpc_  = adc[5];
  Vbg_  = adc[6];
  ts_   = adc[7];
}
void DCU::print(std::ostream& out) const {
  out
    << "mode_ = " << mode_ 
    << " Vaa_ = " << Vaa_ 
    << " Vdd_ = " << Vdd_ 
    << " rtd2_ = " << rtd2_ 
    << " rtd3_ = " << rtd3_ 
    << " aoh_ = " << aoh_ 
    << " Vpc_ = " << Vpc_ 
    << " Vbg_ = " << Vbg_ 
    << " ts_ = " << ts_ 
  ;
}
std::string DCU::prints() const {
  std::stringstream out;
  out
    << "mode_ = " << mode_ 
    << " Vaa_ = " << Vaa_ 
    << " Vdd_ = " << Vdd_ 
    << " rtd2_ = " << rtd2_ 
    << " rtd3_ = " << rtd3_ 
    << " aoh_ = " << aoh_ 
    << " Vpc_ = " << Vpc_ 
    << " Vbg_ = " << Vbg_ 
    << " ts_ = " << ts_ 
  ;
  return out.str();
}
const std::string& DCU::name_chan(unsigned chan) {
  switch (chan) {
    case 0:  return name_Vaa;
    case 1:  return name_Vdd;
    case 2:  return name_RTD2;
    case 3:  return name_RTD3;
    case 4:  return name_AOH;
    case 5:  return name_Vpc;
    case 6:  return name_Vbg;
    case 7:  return name_TS;
    default: std::cout<< "ERROR PortCard::DCU::name_chan: channel " << chan << " is out of range (0..7)" <<std::endl;
    return name_wrong;
  }
}
unsigned DCU::GetChan(unsigned chan) const {
  switch (chan) {
    case 0:  return Vaa_;
    case 1:  return Vdd_;
    case 2:  return rtd2_;
    case 3:  return rtd3_;
    case 4:  return aoh_;
    case 5:  return Vpc_;
    case 6:  return Vbg_;
    case 7:  return ts_;
    default: std::cout<< "ERROR PortCard::DCU::GetChan: channel " << chan << " is out of range (0..7)" <<std::endl;
    return 0;
  }
}

//////////////////////////////
//
//  class Address
//
//////////////////////////////

Address::Address(): dcuId_(0), fecBoardId_(0), mfecId_(0), ccuId_(0), ccuChannelId_(0) {}
Address::Address(std::string portcardName, unsigned dcuId, unsigned fecBoardId, unsigned mfecId, unsigned ccuId, unsigned ccuChannelId):
  portcardName_(portcardName), dcuId_(dcuId), fecBoardId_(fecBoardId), mfecId_(mfecId), ccuId_(ccuId), ccuChannelId_(ccuChannelId)
{}
Address::Address(const Address& address):
  portcardName_(address.portcardName_), dcuId_(address.dcuId_), fecBoardId_(address.fecBoardId_), mfecId_(address.mfecId_), ccuId_(address.ccuId_), ccuChannelId_(address.ccuChannelId_)
{}
Address& Address::operator = (const Address& address)
{
  portcardName_ = address.portcardName_;
  dcuId_           = address.dcuId_;
  fecBoardId_   = address.fecBoardId_;
  mfecId_       = address.mfecId_;
  ccuId_        = address.ccuId_;
  ccuChannelId_ = address.ccuChannelId_;
  return *this;
}
bool Address::operator < (const Address& address) const
{
  // method to be used in std::map for comparison
  // Example of map definition:
  // std::map<Address, DCU> dcu_map;

  if (portcardName_ != address.portcardName_) return (portcardName_ < address.portcardName_);
  if (dcuId_        != address.dcuId_)        return (dcuId_        < address.dcuId_);
  if (fecBoardId_   != address.fecBoardId_)   return (fecBoardId_   < address.fecBoardId_);
  if (mfecId_       != address.mfecId_)       return (mfecId_       < address.mfecId_);
  if (ccuId_        != address.ccuId_)        return (ccuId_        < address.ccuId_);
  if (ccuChannelId_ != address.ccuChannelId_) return (ccuChannelId_ < address.ccuChannelId_);
  return false;   // at this point all fields are equal
}
void Address::print(std::ostream& out) const {
  out
    << "portcardName_ = " << portcardName_
    << " dcuId_ = " << dcuId_
    << " fecBoardId_ = " << fecBoardId_ 
    << " mfecId_ = " << mfecId_ 
    << " ccuId_ = " << ccuId_ 
    << " ccuChannelId_ = " << ccuChannelId_ 
  ;
}
std::string Address::prints() const {
  std::stringstream out;
  out
    << "portcardName_ = " << portcardName_
    << " dcuId_ = " << dcuId_
    << " fecBoardId_ = " << fecBoardId_ 
    << " mfecId_ = " << mfecId_ 
    << " ccuId_ = " << ccuId_ 
    << " ccuChannelId_ = " << ccuChannelId_ 
  ;
  return out.str();
}

void AddressDCU::print(std::ostream& out) const {
  address_.print(std::cout);
  out<<std::endl;
  dcu_.print(std::cout);
  out<<std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// SOAP message coders and decoders
//
////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////
//
// class SOAP_ReadAll
//
/////////////////////////////////

//
//  initializing of static fields of class SOAP_ReadAll
//
const std::string SOAP_ReadAll::name_xdaq_          = "xdaq";
const std::string SOAP_ReadAll::name_dp_            = "dp";
const std::string SOAP_ReadAll::name_command_       = "updateDpValueTrkFEC";    // never used, just for completeness here
const std::string SOAP_ReadAll::name_portcard_      = "portcard";
// address
const std::string SOAP_ReadAll::name_portcardId_    = "portcardId";
const std::string SOAP_ReadAll::name_fecBoardId_    = "fecBoardId";
const std::string SOAP_ReadAll::name_mfecId_        = "mfecId";
const std::string SOAP_ReadAll::name_ccuId_         = "ccuId";
const std::string SOAP_ReadAll::name_ccuChannelId_  = "ccuChannelId";
// DCU ID
const std::string SOAP_ReadAll::name_dcuIdId_       = "dcuIdId";
// DCU mode
const std::string SOAP_ReadAll::name_dcuModeId_     = "dcuModeId";
// DCU channels
const std::string SOAP_ReadAll::name_dcuChannelId_  = "dcuChannelId";

xoap::MessageReference SOAP_ReadAll::Make(const std::string& tagName, const std::vector<AddressDCU>& vdcu)
{
  //
  //  Creates SOAP message from vector of AddressDCU structures
  //
  
  // create SOAP message;
  xoap::MessageReference soapMessage = xoap::createMessage();
  xoap::SOAPEnvelope envelope = soapMessage->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();

  xoap::SOAPName command = envelope.createName(tagName, name_xdaq_, XDAQ_NS_URI);
  xoap::SOAPBodyElement bodyElement = body.addBodyElement(command);

  for (std::vector<AddressDCU>::const_iterator addressDCU=vdcu.begin(); addressDCU!=vdcu.end(); ++addressDCU)
  {
    // add current Port Card as a child
    xoap::SOAPName portcardName = envelope.createName(name_portcard_);
    xoap::SOAPElement portcardElement = bodyElement.addChildElement(portcardName);

    // check the number of children
    //cout<< "SOAP_ReadAll::Make: bodyElement.getChildElements(portcardName).size() = " << bodyElement.getChildElements(portcardName).size() <<endl;

    // add module location (fecAddress, ringAddress(==mfec), ccuAddress, I2C channelAddress) as attributes of portcardElement

    // portcardId
    xoap::SOAPName portcardIdName = envelope.createName(name_portcardId_);
    portcardElement.addAttribute(portcardIdName, addressDCU->address_.portcardName_);

    // IdId
    xoap::SOAPName IdIdName = envelope.createName(name_dcuIdId_);
    portcardElement.addAttribute(IdIdName, unsigned2string(addressDCU->address_.dcuId_));

    // fecBoardId
    xoap::SOAPName fecBoardIdName = envelope.createName(name_fecBoardId_);
    portcardElement.addAttribute(fecBoardIdName, unsigned2string(addressDCU->address_.fecBoardId_));

    // mfecId (NB: we are using term "mfecId" instead of "ring" here)
    xoap::SOAPName mfecIdName = envelope.createName(name_mfecId_);
    portcardElement.addAttribute(mfecIdName, unsigned2string(addressDCU->address_.mfecId_));

    // ccuId
    xoap::SOAPName ccuIdName = envelope.createName(name_ccuId_);
    portcardElement.addAttribute(ccuIdName, unsigned2string(addressDCU->address_.ccuId_));

    // ccuChannelId
    xoap::SOAPName ccuChannelIdName = envelope.createName(name_ccuChannelId_);
    portcardElement.addAttribute(ccuChannelIdName, unsigned2string(addressDCU->address_.ccuChannelId_));

    // store DCU input mode
    xoap::SOAPName dcuModeName = envelope.createName(name_dcuModeId_);
    portcardElement.addAttribute(dcuModeName, unsigned2string(addressDCU->dcu_.mode_));
    // add ADC values for all DCU channels of the current Port Card
    // Make child elements with name "dp"
    // Store channel No. as dp attribute
    // Store ADC value using dp method addTextNode
    std::vector<unsigned> vchan;
    addressDCU->dcu_.FillVector(vchan);
    for (std::vector<unsigned>::iterator channel=vchan.begin(); channel!=vchan.end(); ++channel)
    { // have to use iterator instead of const_iterator
      // because std::distance doesn't work with const_iterator: known issue
      unsigned iChannel = std::distance(vchan.begin(), channel);
      xoap::SOAPName dpName = envelope.createName(name_dp_);
      xoap::SOAPElement dpNameElement = portcardElement.addChildElement(dpName);    // current dp is a child of portcard
      xoap::SOAPName channelName = envelope.createName(name_dcuChannelId_);
      dpNameElement.addAttribute(channelName, unsigned2string(iChannel));           // the No. of DCU channel is attribute of dp
      dpNameElement.addTextNode(unsigned2string(*channel));                         // the ADC value is stored using addTextNode
    }
  }
  return soapMessage;
}

xoap::MessageReference SOAP_ReadAll::Make(const std::string& tagName, const std::string& receiver, const std::string& uri, const std::vector<AddressDCU>& vdcu)
{
  //
  //  Creates SOAP message from vector of AddressDCU structures
  //
  
  // create SOAP message;
  xoap::MessageReference soapMessage = xoap::createMessage();
  xoap::SOAPEnvelope envelope = soapMessage->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();

  xoap::SOAPName command = envelope.createName(tagName, receiver, uri);
  xoap::SOAPBodyElement bodyElement = body.addBodyElement(command);

  for (std::vector<AddressDCU>::const_iterator addressDCU=vdcu.begin(); addressDCU!=vdcu.end(); ++addressDCU)
  {
    // add current Port Card as a child
    xoap::SOAPName portcardName = envelope.createName(name_portcard_);
    xoap::SOAPElement portcardElement = bodyElement.addChildElement(portcardName);

    // check the number of children
    //cout<< "SOAP_ReadAll::Make: bodyElement.getChildElements(portcardName).size() = " << bodyElement.getChildElements(portcardName).size() <<endl;

    // add module location (fecAddress, ringAddress(==mfec), ccuAddress, I2C channelAddress) as attributes of portcardElement

    // portcardId
    xoap::SOAPName portcardIdName = envelope.createName(name_portcardId_);
    portcardElement.addAttribute(portcardIdName, addressDCU->address_.portcardName_);

    // IdId
    xoap::SOAPName IdIdName = envelope.createName(name_dcuIdId_);
    portcardElement.addAttribute(IdIdName, unsigned2string(addressDCU->address_.dcuId_));

    // fecBoardId
    xoap::SOAPName fecBoardIdName = envelope.createName(name_fecBoardId_);
    portcardElement.addAttribute(fecBoardIdName, unsigned2string(addressDCU->address_.fecBoardId_));

    // mfecId (NB: we are using term "mfecId" instead of "ring" here)
    xoap::SOAPName mfecIdName = envelope.createName(name_mfecId_);
    portcardElement.addAttribute(mfecIdName, unsigned2string(addressDCU->address_.mfecId_));

    // ccuId
    xoap::SOAPName ccuIdName = envelope.createName(name_ccuId_);
    portcardElement.addAttribute(ccuIdName, unsigned2string(addressDCU->address_.ccuId_));

    // ccuChannelId
    xoap::SOAPName ccuChannelIdName = envelope.createName(name_ccuChannelId_);
    portcardElement.addAttribute(ccuChannelIdName, unsigned2string(addressDCU->address_.ccuChannelId_));

    // store DCU input mode
    xoap::SOAPName dcuModeName = envelope.createName(name_dcuModeId_);
    portcardElement.addAttribute(dcuModeName, unsigned2string(addressDCU->dcu_.mode_));
    // add ADC values for all DCU channels of the current Port Card
    // Make child elements with name "dp"
    // Store channel No. as dp attribute
    // Store ADC value using dp method addTextNode
    std::vector<unsigned> vchan;
    addressDCU->dcu_.FillVector(vchan);
    for (std::vector<unsigned>::iterator channel=vchan.begin(); channel!=vchan.end(); ++channel)
    { // have to use iterator instead of const_iterator
      // because std::distance doesn't work with const_iterator: known issue
      unsigned iChannel = std::distance(vchan.begin(), channel);
      xoap::SOAPName dpName = envelope.createName(name_dp_);
      xoap::SOAPElement dpNameElement = portcardElement.addChildElement(dpName);    // current dp is a child of portcard
      xoap::SOAPName channelName = envelope.createName(name_dcuChannelId_);
      dpNameElement.addAttribute(channelName, unsigned2string(iChannel));           // the No. of DCU channel is attribute of dp
      dpNameElement.addTextNode(unsigned2string(*channel));                         // the ADC value is stored using addTextNode
    }
  }
  return soapMessage;
}

std::vector<AddressDCU> SOAP_ReadAll::Decode(xoap::MessageReference msg)
{
  //
  //  Extracts vector of AddressDCU structures from the SOAP message
  //
  std::vector<AddressDCU> vdcu;
  AddressDCU addressDCU;

  xoap::SOAPEnvelope envelope = msg->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();
  xoap::SOAPName commandElement = envelope.createName(name_command_);
  
  std::vector<xoap::SOAPElement> bodyElements = body.getChildElements(commandElement);
  //cout << "SOAP_ReadAll::Decode: Number of BodyElements = " << bodyElements.size() << endl;

  for (std::vector<xoap::SOAPElement>::iterator p_bodyElement = bodyElements.begin(); p_bodyElement != bodyElements.end(); ++p_bodyElement)
  {
    //cout<< "SOAP_ReadAll::Decode: vector element " << std::distance(bodyElements.begin(), p_bodyElement) <<endl;
    //cout<< "SOAP_ReadAll::Decode: BodyElement name = " << p_bodyElement->getElementName().getQualifiedName() <<endl;

    xoap::SOAPName portcardName = envelope.createName(name_portcard_);
    std::vector<xoap::SOAPElement> portcardElement = p_bodyElement->getChildElements(portcardName);
    //cout<< "SOAP_ReadAll::Decode: portcardElement.size() = " << portcardElement.size() <<endl;
    for (std::vector<xoap::SOAPElement>::iterator portcard=portcardElement.begin(); portcard!=portcardElement.end(); ++portcard)
    {
      // PortCard::Address fields
      
      // portcardId
      xoap::SOAPName portcardIdName = envelope.createName(name_portcardId_);
      addressDCU.address_.portcardName_ = portcard->getAttributeValue(portcardIdName);
      //cout<< "SOAP_ReadAll::Decode: addressDCU.address_.portcardName_ = " << addressDCU.address_.portcardName_ <<endl;

      // dcuIdId
      xoap::SOAPName dcuIdName = envelope.createName(name_dcuIdId_);
      std::string dcuIdId_str = portcard->getAttributeValue(dcuIdName);
      addressDCU.address_.dcuId_ = string2unsigned(dcuIdId_str);
      //cout<< "SOAP_ReadAll::Decode: dcuIdId_str = " << dcuIdId_str << " addressDCU.address_.dcuId_ = " << addressDCU.address_.dcuId_ <<endl;

      // fecBoardId
      xoap::SOAPName fecBoardIdName = envelope.createName(name_fecBoardId_);
      std::string fecBoardId_str = portcard->getAttributeValue(fecBoardIdName);
      addressDCU.address_.fecBoardId_ = string2unsigned(fecBoardId_str);
      //cout<< "SOAP_ReadAll::Decode: addressDCU.address_.fecBoardId_ = " << addressDCU.address_.fecBoardId_ <<endl;

      // mfecId (NB: we are using term "mfecId" instead of "ring" here)
      xoap::SOAPName mfecIdName = envelope.createName(name_mfecId_);
      std::string mfecId_str = portcard->getAttributeValue(mfecIdName);
      addressDCU.address_.mfecId_ = string2unsigned(mfecId_str);
      //cout<< "SOAP_ReadAll::Decode: addressDCU.address_.mfecId_ = " << addressDCU.address_.mfecId_ <<endl;

      // ccuId
      xoap::SOAPName ccuIdName = envelope.createName(name_ccuId_);
      std::string ccuId_str = portcard->getAttributeValue(ccuIdName);
      addressDCU.address_.ccuId_ = string2unsigned(ccuId_str);
      //cout<< "SOAP_ReadAll::Decode: addressDCU.address_.ccuId_ = " << addressDCU.address_.ccuId_ <<endl;

      // ccuChannelId
      xoap::SOAPName ccuChannelIdName = envelope.createName(name_ccuChannelId_);
      std::string ccuChannelId_str = portcard->getAttributeValue(ccuChannelIdName);
      addressDCU.address_.ccuChannelId_ = string2unsigned(ccuChannelId_str);
      //cout<< "SOAP_ReadAll::Decode: addressDCU.address_.ccuChannelId_ = " << addressDCU.address_.ccuChannelId_ <<endl;
      
      // PortCard::DCU fields

      // dcu mode
      xoap::SOAPName dcuModeIdName = envelope.createName(name_dcuModeId_);
      std::string dcuModeId_str = portcard->getAttributeValue(dcuModeIdName);
      addressDCU.dcu_.mode_ = string2unsigned(dcuModeId_str);
      //cout<< "SOAP_ReadAll::Decode: addressDCU->dcu_.mode_ = " << addressDCU.dcu_.mode_ <<endl;

      xoap::SOAPName dp_vector_Name = envelope.createName(name_dp_);
      std::vector<xoap::SOAPElement> dp_vector = portcard->getChildElements(dp_vector_Name);
      //cout<< "SOAP_ReadAll::Decode: dp_vector.size() = " << dp_vector.size() <<endl;
      unsigned adc[8];
      // basic test
      assert(dp_vector.size() == 8);
      for (std::vector<xoap::SOAPElement>::iterator dp=dp_vector.begin(); dp!=dp_vector.end(); ++dp)
      {
        xoap::SOAPName dcuChannelId = envelope.createName(name_dcuChannelId_);
        //cout<< "SOAP_ReadAll::Decode: dp->getAttributeValue(dcuChannelId): " << dp->getAttributeValue(dcuChannelId) <<endl;
        //cout<< "SOAP_ReadAll::Decode: dp->getValue(): " << dp->getValue() <<endl;
        adc[std::distance(dp_vector.begin(), dp)] = string2unsigned(dp->getValue()); // more safe
        // adc[string2unsigned(dp->getAttributeValue(dcuChannelId))] = string2unsigned(dp->getValue());
      }
      addressDCU.dcu_.SetADC(adc);

      vdcu.push_back(addressDCU);
    }
  }
  // to make sure that vector is OK
  // for (std::vector<AddressDCU>::const_iterator addressDCU=vdcu.begin(); addressDCU!=vdcu.end(); ++addressDCU)
  // {
  //   addressDCU->print(cout);
  // }
  return vdcu;
}



} // namespace PortCard
