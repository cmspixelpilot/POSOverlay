// $Id: PortCardDCU.h,v 1.8 2010/04/28 20:42:14 zatserkl Exp $

/*************************************************************************
 * Class describes DCU used on CMS Pixel Port Card                       *
 *                                                                       *
 * Author: Andriy Zatserklyaniy, University of Puerto Rico		 *
 *                                                                       *
 * Last update: $Date: 2010/04/28 20:42:14 $ (UTC)                       *
 *          by: $Author: zatserkl $                                      *
 *************************************************************************/

#ifndef PortCardDCU_h
#define PortCardDCU_h

#include "xdaq/Application.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/ApplicationStubImpl.h"
#include "xdaq/exception/Exception.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/NamespaceURI.h"

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/AttachmentPart.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPMessage.h"
#include "xoap/SOAPBody.h"
#include "xoap/SOAPPart.h"
#include "xoap/DOMParser.h"

#include <string>
#include <vector>
#include <cassert>
#include <sstream>
#include <iomanip>

namespace PortCard {

// General purpose string --> unsigned and unsigned --> string conversion functions
std::string unsigned2string(unsigned number);
unsigned string2unsigned(std::string str);

// DCU (Port Card) address
class Address {
public:
  std::string portcardName_;  // Port Card name from pos::PixelPortcardMap
  unsigned dcuId_;            // DCU ID
  unsigned fecBoardId_;       // slot of FEC in crate
  unsigned mfecId_;           // == ring slot = position of FEC mezzanine board with optical cable in FEC board (counted from 0)
  unsigned ccuId_;            // CCU address. Actually, this is a position of CCU. For some reason it's counted from 0x70
  unsigned ccuChannelId_;     // one of 12 CCU channels. Each channel connected with different Port Card
  Address();
  Address(std::string portcardName, unsigned dcuId, unsigned fecBoardId, unsigned mfecId, unsigned ccuId, unsigned ccuChannelId);
  Address(const Address& address);
  Address& operator = (const Address& address);
  bool operator < (const Address& address) const;
  void print(std::ostream& out) const;
  std::string prints() const;
  const std::string& portcardName() const {return portcardName_;}
  const std::string HalfCylinder() const {
    // the portcard name looks like FPix_BpI_D1_PRT2
    // this name is standard and will not be changed
    assert(portcardName_.size() == 16);
    assert(portcardName_.substr(0,4) == std::string("FPix_"));
    return portcardName_.substr(5,3);                     // "BpI" for out example
  }
  unsigned Ndisk() const {
    // the portcard name looks like FPix_BpI_D1_PRT2
    // this name is standard and will not be changed
    assert(portcardName_.size() == 16);
    assert(portcardName_.substr(0,4) == std::string("FPix_"));
    std::string disk_str = portcardName_.substr(10,1);    // "1" for out example
    assert(disk_str == std::string("1") || disk_str == std::string("2") || disk_str == std::string("3"));
    std::istringstream ss(disk_str);
    unsigned idisk;
    ss >> idisk;
    return idisk;
  }
  unsigned Nportcard() const {
    // the portcard name looks like FPix_BpI_D1_PRT2
    // this name is standard and will not be changed
    assert(portcardName_.size() == 16);
    assert(portcardName_.substr(0,4) == std::string("FPix_"));
    std::string portcard_str = portcardName_.substr(15,1);       // "2" for out example
    assert(portcard_str == std::string("1") || portcard_str == std::string("2") || portcard_str == std::string("3") || portcard_str == std::string("4"));
    std::istringstream ss(portcard_str);
    unsigned iportcard;
    ss >> iportcard;
    return iportcard;
  }
  unsigned dcuId() const {return dcuId_;}
  void SetDcuID(unsigned dcuId) {dcuId_ = dcuId;}
};

// DCU channels
class DCU {
public:
  enum Mode {
    HIR = 0,            // for some reason for DCU the High Input Mode = 0 and the Low Input Mode = 1
    LIR = 1             // (i.e. mode bit in DCU readout = 0 for HIR and = 1 for LIR)
  };
  unsigned mode_;       // LIR: mode=1, HIR: mode=0
  unsigned Vaa_;        // analog voltage
  unsigned Vdd_;        // digital voltage
  unsigned rtd2_;       // RTD connected with DCU channel 2
  unsigned rtd3_;       // RTD connected with DCU channel 3
  unsigned aoh_;        // AOH RTD: RTD situated on Port Card under the AOH mezzanine board
  unsigned Vpc_;        // Port card Vpower. NB: this value should be used for normalizing of the rest of all channels.
  unsigned Vbg_;        // BandGap reference voltage (for test purpose)
  unsigned ts_;         // DCU internal diode-based temperature sensor
  // coefficient for conversion of ADC value for Pt RTD into degrees of Celsius
  static const double kPt_;
  // standard names
  static const std::string name_modeDCU;    // "DCUmode"      -- to be used as a standard name for chaild element in SOAP message
  static const std::string name_modeValue;  // "DCUmodeValue" -- to be used as a standard name for attribute in SOAP message
  static const std::string name_modeLIR;    // "DCUmodeLIR"
  static const std::string name_modeHIR;    // "DCUmodeHIR"
  static const std::string name_Vaa;
  static const std::string name_Vdd;
  static const std::string name_RTD2;
  static const std::string name_RTD3;
  static const std::string name_AOH;
  static const std::string name_Vpc;
  static const std::string name_Vbg;
  static const std::string name_TS;
  static const std::string name_wrong;
  static const std::string name_channel(unsigned chan) {   // NB: static member function cannot have `const' method qualifier
    switch (chan) {
      case 0: return name_Vaa;
      case 1: return name_Vdd;
      case 2: return name_RTD2;
      case 3: return name_RTD3;
      case 4: return name_AOH;
      case 5: return name_Vpc;
      case 6: return name_Vbg;
      case 7: return name_TS;
      default: 
        return name_wrong;
    }
  }
  DCU();
  DCU(unsigned mode, unsigned Vaa, unsigned Vdd, unsigned rtd2, unsigned rtd3, unsigned aoh, unsigned Vpc, unsigned Vbg, unsigned ts);
  void print(std::ostream& out) const;
  std::string prints() const;
  // getters
  static double kPt() {return kPt_;}
  void FillVector(std::vector<unsigned>& vchan) const;
  unsigned GetMode() const {return mode_;}
  unsigned Vaa() const {return Vaa_;}
  unsigned Vdd() const {return Vdd_;}
  unsigned RTD2() const {return rtd2_;}
  unsigned RTD3() const {return rtd3_;}
  unsigned AOH() const {return aoh_;}
  unsigned Vpc() const {return Vpc_;}
  unsigned Vbg() const {return Vbg_;}
  unsigned TS() const {return ts_;}
  // channel access
  static const std::string& name_chan(unsigned chan);   // NB: static member function cannot have `const' method qualifier
  unsigned GetChan(unsigned chan) const;
  // setters
  void SetADC(unsigned adc[]);
  void SetMode(unsigned mode) {mode_ = mode;}
  void SetVaa(unsigned Vaa) {Vaa_ = Vaa;}
  void SetVdd(unsigned Vdd) {Vdd_ = Vdd;}
  void SetRTD2(unsigned rtd2) {rtd2_ = rtd2;}
  void SetRTD3(unsigned rtd3) {rtd3_ = rtd3;}
  void SetAOH(unsigned aoh) {aoh_ = aoh;}
  void SetVpc(unsigned Vpc) {Vpc_ = Vpc;}
  void SetVbg(unsigned Vbg) {Vbg_ = Vbg;}
  void SetTS(unsigned ts) {ts_ = ts;}
  // Temperature in degrees of Celsius
  double T(unsigned ichan) const {return (2./kPt_)*(1. - double(GetChan(ichan))/double(Vpc_));}
  double Trtd2() const {return (2./kPt_)*(1. - double(rtd2_)/double(Vpc_));}
  double Trtd3() const {return (2./kPt_)*(1. - double(rtd3_)/double(Vpc_));}
  double Taoh() const {return (2./kPt_)*(1. - double(aoh_)/double(Vpc_));}
  std::string T_str(unsigned ichan) const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << T(ichan);
    return ss.str();
  }
  std::string Trtd2_str() const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << Trtd2();
    return ss.str();
  }
  std::string Trtd3_str() const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << Trtd3();
    return ss.str();
  }
  std::string Taoh_str() const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << Taoh();
    return ss.str();
  }
};

class AddressDCU {
public:
  Address address_;
  DCU dcu_;
  AddressDCU() {}
  AddressDCU(const AddressDCU& addressDCU): address_(addressDCU.address_), dcu_(addressDCU.dcu_) {}
  AddressDCU& operator = (const AddressDCU& addressDCU) {
    address_ = addressDCU.address_;
    dcu_ = addressDCU.dcu_;
    return *this;
  }
  void print(std::ostream& out) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////
//
// SOAP message coders and decoders
//
////////////////////////////////////////////////////////////////////////////////////////////////

class SOAP_ReadAll {
public:
  // names for the SOAP messages
  static const std::string name_xdaq_;          // "xdaq"
  static const std::string name_dp_;            // "dp"
  static const std::string name_command_;       // "updateDpValueTrkFEC"
  static const std::string name_portcard_;      // "portcard"
  // address
  static const std::string name_portcardId_;
  static const std::string name_fecBoardId_;
  static const std::string name_mfecId_;
  static const std::string name_ccuId_;
  static const std::string name_ccuChannelId_;
  // DCU ID
  static const std::string name_dcuIdId_;
  // DCU mode
  static const std::string name_dcuModeId_;
  // DCU channels
  static const std::string name_dcuChannelId_;

  static xoap::MessageReference Make(const std::string& tagName, const std::vector<AddressDCU>& vdcu);
  static xoap::MessageReference Make(const std::string& tagName, const std::string& receiver, const std::string& uri, const std::vector<AddressDCU>& vdcu);
  static std::vector<AddressDCU> Decode(xoap::MessageReference msg);
};

} // namespace PortCard

#endif
