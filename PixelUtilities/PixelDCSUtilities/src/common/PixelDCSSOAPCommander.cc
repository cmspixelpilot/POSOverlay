
#include "PixelDCSSOAPCommander.h"

/*************************************************************************
 * Auxiliary class to create SOAP messages used by XDAQ-DCS applications *
 *                                                                       *
 * Author: Christian Veelken, UC Davis				         *
 *                                                                       *
 * Last update: $Date: 2007/12/03 10:08:40 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#include <iomanip>
#include <map>
#include <sstream>

// declare global auxiliary functions
void getPixelFECAddress(const pos::PixelROCName& rocName, const pos::PixelNameTranslation* nameTranslation, const pos::PixelFECConfig* fecConfiguration,
			unsigned int& vmeBaseAddress, unsigned int& mFEC, unsigned int& mFEC_channel, 
			unsigned int& hubAddress, unsigned int& portAddress, unsigned int& rocId);

//
//---------------------------------------------------------------------------------------------------
//

PixelDCSSOAPCommander::PixelDCSSOAPCommander(xdaq::Application* xdaqApplication)
  : SOAPCommander(xdaqApplication)
{
//--- nothing to be done yet...
}

PixelDCSSOAPCommander::~PixelDCSSOAPCommander()
{
//--- nothing to be done yet...
}

//
//---------------------------------------------------------------------------------------------------
//
xoap::MessageReference PixelDCSSOAPCommander::MakeSOAPMessageReference_progDAC(unsigned int dacAddress,
									       const std::map<pos::PixelROCName, unsigned int>& dacValues_set,
									       const std::list<pos::PixelROCName>& dacValues_increase,
									       const std::list<pos::PixelROCName>& dacValues_decrease,
									       const pos::PixelNameTranslation* nameTranslation,
									       const pos::PixelFECConfig* fecConfiguration)
{
//--- create SOAP message;
//    add body
  xoap::MessageReference soapMessage = xoap::createMessage();
  xoap::SOAPEnvelope envelope = soapMessage->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();

  xoap::SOAPName command_progDAC = envelope.createName("Prog_DACs", "xdaq", XDAQ_NS_URI);
  xoap::SOAPBodyElement bodyElement_progDAC = body.addBodyElement(command_progDAC);

  xoap::SOAPName dacAddressElement = envelope.createName("dacAddress");
  std::ostringstream dacAddress_string;
  dacAddress_string << dacAddress;
  bodyElement_progDAC.addAttribute(dacAddressElement, dacAddress_string.str());

//--- add "set" DAC section to SOAP message
//    (DAC value is set freely, independent of its current value)
  xoap::SOAPName command_set = envelope.createName("set", "xdaq", XDAQ_NS_URI);
  xoap::SOAPElement soapElement_set = bodyElement_progDAC.addChildElement(command_set);
  AddSOAPMessageReference_progDAC(envelope, soapElement_set, dacValues_set, nameTranslation, fecConfiguration);
  
//--- add "increase" DAC section to SOAP message
//    (DAC value is increased by 1 unit, 
//     in case it is not at its upper limit already)
  //xoap::SOAPName command_increase = envelope.createName("increase", "xdaq", XDAQ_NS_URI);
  //xoap::SOAPElement soapElement_increase = bodyElement_progDAC.addChildElement(command_increase);
  //AddSOAPMessageReference_progDAC(envelope, soapElement_increase, dacValues_increase, nameTranslation, fecConfiguration);

//--- add "decrease" DAC section to SOAP message
//    (DAC value is decreased by 1 unit, 
//     in case it is not at its lower limit already)
  //xoap::SOAPName command_decrease = envelope.createName("decrease", "xdaq", XDAQ_NS_URI);
  //xoap::SOAPElement soapElement_decrease = bodyElement_progDAC.addChildElement(command_decrease);
  //AddSOAPMessageReference_progDAC(envelope, soapElement_decrease, dacValues_decrease, nameTranslation, fecConfiguration);

  return soapMessage;
}

xoap::MessageReference PixelDCSSOAPCommander::MakeSOAPMessageReference_readLastDAC()
{
//--- create SOAP message;
//    add body
  xoap::MessageReference soapMessage = xoap::createMessage();
  xoap::SOAPEnvelope envelope = soapMessage->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();

//--- add "readLastDAC" command to SOAP message
  xoap::SOAPName command = envelope.createName("readLastDACFIFO", "xdaq", XDAQ_NS_URI);
  body.addBodyElement(command);

  return soapMessage;
}

xoap::MessageReference PixelDCSSOAPCommander::MakeSOAPMessageReference_readDCU()
{
//--- create SOAP message;
//    add body
  xoap::MessageReference soapMessage = xoap::createMessage();
  xoap::SOAPEnvelope envelope = soapMessage->getSOAPPart().getEnvelope();
  xoap::SOAPBody body = envelope.getBody();

//--- add "readDCU" command to SOAP message
  xoap::SOAPName command = envelope.createName("readDCU", "xdaq", XDAQ_NS_URI);  
  body.addBodyElement(command);

  return soapMessage;
}

//
//---------------------------------------------------------------------------------------------------
//

xoap::MessageReference PixelDCSSOAPCommander::postSOAP(xdaq::ApplicationDescriptor* applicationDescriptor, xoap::MessageReference soapMessage) throw (xoap::exception::Exception)
{	
  std::cout << "<PixelDCSSOAPCommander::postSOAP>:" << std::endl;

  std::cout << "sending SOAP message to " << applicationDescriptor->getContextDescriptor()->getURL() << std::endl;
  std::cout << " Request : ------------------------------------ "<< std::endl;
  soapMessage->writeTo(std::cout);
  std::cout << std::endl;
  std::cout << " ---------------------------------------------- "<< std::endl;
  
  try {
    xoap::MessageReference soapResponse = app_->getApplicationContext()->postSOAP(soapMessage, *app_->getApplicationDescriptor(), *applicationDescriptor);

    std::cout <<" Reply : -------------------------------------- "<< std::endl;
    soapResponse->writeTo(std::cout);
    std::cout << std::endl;
    std::cout <<" ---------------------------------------------- "<< std::endl;
    
    return soapResponse;
  } catch ( xoap::exception::Exception& e ) {
    XCEPT_RETHROW(xoap::exception::Exception, "Failed to send SOAP message.", e);
  }
}

//
//---------------------------------------------------------------------------------------------------
//

void PixelDCSSOAPCommander::AddSOAPMessageReference_progDAC(xoap::SOAPEnvelope& soapEnvelope, xoap::SOAPElement& soapElement,
							    const std::list<std::pair<pos::PixelROCName, unsigned int> >& dacValues,
							    const pos::PixelNameTranslation* nameTranslation,
							    const pos::PixelFECConfig* fecConfiguration)
{
//--- add list of "dacValues" (actually pairs of Readout-Chip name and DAC value)
//    given as function argument
//    to SOAP message 
//    (the format of which is described in DocDB document #1412);
//    in order to reduce size of SOAP message to be transmitted, 
//    sort Readout-Chips according to their hardware connection
//    (by using an internal map data structure)

//--- declare typedefs for internal map data structure
  typedef std::pair<pos::PixelROCName, unsigned int> type_dacValue;
  typedef std::map<unsigned int, type_dacValue> type_dacValue_rocId;
  typedef std::map<unsigned int, type_dacValue_rocId> type_dacValue_portAddress;
  typedef std::map<unsigned int, type_dacValue_portAddress> type_dacValue_hubAddress;
  typedef std::map<unsigned int, type_dacValue_hubAddress> type_dacValue_mFEC_channel;
  typedef std::map<unsigned int, type_dacValue_mFEC_channel> type_dacValue_mFEC;
  typedef std::map<unsigned int, type_dacValue_mFEC> type_dacValue_fecBoard;
  
  type_dacValue_fecBoard dacValue_map;
  
//--- store dacValues in internal map data structure
  for ( std::list<std::pair<pos::PixelROCName, unsigned int> >::const_iterator dacValue = dacValues.begin();
	dacValue != dacValues.end(); ++dacValue ) {
    const pos::PixelROCName& readOutChipName = dacValue->first;

    unsigned int vmeBaseAddress, mFEC, mFEC_channel, hubAddress, portAddress, rocId;  
    getPixelFECAddress(readOutChipName, nameTranslation, fecConfiguration, vmeBaseAddress, mFEC, mFEC_channel, hubAddress, portAddress, rocId);

    dacValue_map[vmeBaseAddress][mFEC][mFEC_channel][hubAddress][portAddress][rocId] = (*dacValue);    
  }

//--- process dacValues stored in internal map data structure
//    in sorted order
//    and add dacValues to SOAP message
  for ( type_dacValue_fecBoard::const_iterator dacValue_fecBoard = dacValue_map.begin();
	dacValue_fecBoard != dacValue_map.end(); ++dacValue_fecBoard ) {
    xoap::SOAPName fecBoardName = soapEnvelope.createName("fecBoard");
    xoap::SOAPElement fecBoardElement = soapElement.addChildElement(fecBoardName);
    xoap::SOAPName vmeBaseAddressElement = soapEnvelope.createName("vmeBaseAddress");
    unsigned int vmeBaseAddress = dacValue_fecBoard->first;
    std::ostringstream vmeBaseAddress_string;
    vmeBaseAddress_string << vmeBaseAddress;
    fecBoardElement.addAttribute(vmeBaseAddressElement, vmeBaseAddress_string.str());

    for ( type_dacValue_mFEC::const_iterator dacValue_mFEC = dacValue_fecBoard->second.begin();
	dacValue_mFEC != dacValue_fecBoard->second.end(); ++dacValue_mFEC ) {
      for ( type_dacValue_mFEC_channel::const_iterator dacValue_mFEC_channel = dacValue_mFEC->second.begin();
	    dacValue_mFEC_channel != dacValue_mFEC->second.end(); ++dacValue_mFEC_channel ) {
	for ( type_dacValue_hubAddress::const_iterator dacValue_hubAddress = dacValue_mFEC_channel->second.begin();
	      dacValue_hubAddress != dacValue_mFEC_channel->second.end(); ++dacValue_hubAddress ) {
	  for ( type_dacValue_portAddress::const_iterator dacValue_portAddress = dacValue_hubAddress->second.begin();
		dacValue_portAddress != dacValue_hubAddress->second.end(); ++dacValue_portAddress ) {
	    for ( type_dacValue_rocId::const_iterator dacValue_rocId = dacValue_portAddress->second.begin();
		  dacValue_rocId != dacValue_portAddress->second.end(); ++dacValue_rocId ) {
	      xoap::SOAPName rocName = soapEnvelope.createName(dacValue_rocId->second.first.rocname());
	      xoap::SOAPElement rocNameElement = fecBoardElement.addChildElement(rocName);

	      xoap::SOAPName mfecNumberElement = soapEnvelope.createName("mfecNumber");
	      unsigned int mfecNumber = dacValue_mFEC->first;
	      std::ostringstream mfecNumber_string;
	      mfecNumber_string << mfecNumber;
	      rocNameElement.addAttribute(mfecNumberElement, mfecNumber_string.str());

	      xoap::SOAPName mfecChannelElement = soapEnvelope.createName("mfecChannel");
	      unsigned int mfecChannel = dacValue_mFEC_channel->first;
	      std::ostringstream mfecChannel_string;
	      mfecChannel_string << mfecChannel;
	      rocNameElement.addAttribute(mfecChannelElement, mfecChannel_string.str());
	      
	      xoap::SOAPName hubAddressElement = soapEnvelope.createName("hubAddress");
	      unsigned int hubAddress = dacValue_hubAddress->first;
	      std::ostringstream hubAddress_string;
	      hubAddress_string << hubAddress;
	      rocNameElement.addAttribute(hubAddressElement, hubAddress_string.str());
	      
	      xoap::SOAPName portAddressElement = soapEnvelope.createName("portAddress");
	      unsigned int portAddress = dacValue_portAddress->first;
	      std::ostringstream portAddress_string;
	      portAddress_string << portAddress;
	      rocNameElement.addAttribute(portAddressElement, portAddress_string.str());

	      xoap::SOAPName rocIdElement = soapEnvelope.createName("rocId");
	      unsigned int rocId = dacValue_rocId->first;
	      std::ostringstream rocId_string;
	      rocId_string << rocId;
	      rocNameElement.addAttribute(rocIdElement, rocId_string.str());

	      unsigned int dacValue = dacValue_rocId->second.second;
	      std::ostringstream dacValue_string;
	      dacValue_string << dacValue;
	      rocNameElement.addTextNode(dacValue_string.str());
	    }
	  }
	}
      }
    }
  }
}

void PixelDCSSOAPCommander::AddSOAPMessageReference_progDAC(xoap::SOAPEnvelope& soapEnvelope, xoap::SOAPElement& soapElement,
							    const std::map<pos::PixelROCName, unsigned int>& dacValues,
							    const pos::PixelNameTranslation* nameTranslation,
							    const pos::PixelFECConfig* fecConfiguration)
{
//--- convert format of Readout-Chip list passed as function argument
//    of the "set" DAC section to the general format std::list<std::pair<pos::PixelROCName, unsigned int> >
//    such that a common method can be used to create the SOAP message in the "set", "increase" and "decrease" cases

  std::list<std::pair<pos::PixelROCName, unsigned int> > dacValues_newFormat;
  for ( std::map<pos::PixelROCName, unsigned int>::const_iterator dacValue = dacValues.begin();
	dacValue != dacValues.end(); ++dacValue ) {
    dacValues_newFormat.push_back(std::pair<pos::PixelROCName, unsigned int>(dacValue->first, dacValue->second));
  }
  
  AddSOAPMessageReference_progDAC(soapEnvelope, soapElement, dacValues_newFormat, nameTranslation, fecConfiguration);
}

void PixelDCSSOAPCommander::AddSOAPMessageReference_progDAC(xoap::SOAPEnvelope& soapEnvelope, xoap::SOAPElement& soapElement,
							    const std::list<pos::PixelROCName>& dacValues,
							    const pos::PixelNameTranslation* nameTranslation,
							    const pos::PixelFECConfig* fecConfiguration)
{
//--- convert format of Readout-Chip list passed as function argument
//    of "increase" DAC and "decrease" DAC sections to the general format std::list<std::pair<pos::PixelROCName, unsigned int> >
//    such that a common method can be used to create the SOAP message in the "set", "increase" and "decrease" cases

  const unsigned int numUnits = 1;

  std::list<std::pair<pos::PixelROCName, unsigned int> > dacValues_newFormat;
  for ( std::list<pos::PixelROCName>::const_iterator dacValue = dacValues.begin();
	dacValue != dacValues.end(); ++dacValue ) {
    dacValues_newFormat.push_back(std::pair<pos::PixelROCName, unsigned int>(*dacValue, numUnits));
  }
  
  AddSOAPMessageReference_progDAC(soapEnvelope, soapElement, dacValues_newFormat, nameTranslation, fecConfiguration);
}

//
//---------------------------------------------------------------------------------------------------
//

void getPixelFECAddress(const pos::PixelROCName& rocName, const pos::PixelNameTranslation* nameTranslation, const pos::PixelFECConfig* fecConfiguration,
			unsigned int& vmeBaseAddress, unsigned int& mFEC, unsigned int& mFEC_channel, 
			unsigned int& hubAddress, unsigned int& portAddress, unsigned int& rocId)
{
  const pos::PixelHdwAddress* rocHardwareAddress = nameTranslation->getHdwAddress(rocName);

  unsigned int fecBoard = rocHardwareAddress->fecnumber();

  vmeBaseAddress = fecConfiguration->VMEBaseAddressFromFECNumber(fecBoard);
  mFEC = rocHardwareAddress->mfec();
  mFEC_channel = rocHardwareAddress->mfecchannel();
  hubAddress = rocHardwareAddress->hubaddress();
  portAddress = rocHardwareAddress->portaddress();
  rocId = rocHardwareAddress->rocid();
}
