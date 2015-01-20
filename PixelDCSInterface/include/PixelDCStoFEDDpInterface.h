// $Id: PixelDCStoFEDDpInterface.h,v 1.4 2007/12/03 11:24:21 veelken Exp $

/*************************************************************************
 * Interface class to convert last DAC temperature values                *
 * read-out via the PixelFEDSupervisor to physical units,                *
 * calibrate and average them,                                           *
 * and send calibrated averages to PSX server/PVSS,                      *
 * provided the calibrated average exceeds a configureable dead-band.    *
 *                                                                       *
 * Author: Christian Veelken, UC Davis					 *
 *                                                                       *
 * Last update: $Date: 2007/12/03 11:24:21 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#ifndef _PixelDCStoFEDDpInterface_h_
#define _PixelDCStoFEDDpInterface_h_

#include <list>

#include <cgicc/Cgicc.h>

#include "PixelDCSInterface/include/PixelDCSDpInterface.h"
#include "PixelDCSInterface/include/PixelDCStoFEDDpConnectionRow.h"

#include "CalibFormats/SiPixelObjects/interface/PixelNameTranslation.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFECConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCDACSettings.h"

class PixelDCStoFEDDpNameTable;
class PixelDCStoFEDDpValueTable;
class PixelDCStoFEDDpFilterTable;
class PixelDCStoFEDDpCalibrationTable;
class PixelDCStoFEDDpFilterRow;

class PixelDCSSOAPCommander;
class PixelDCSPVSSCommander;

class PixelDCStoFEDDpInterface : public PixelDCSDpInterface
{
 public:

  XDAQ_INSTANTIATOR();

  PixelDCStoFEDDpInterface(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);
  ~PixelDCStoFEDDpInterface();

  void Default(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  void XgiHandler(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
  
  xoap::MessageReference Configure(xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference Halt(xoap::MessageReference msg) throw (xoap::exception::Exception);
  
 protected:
  
  xoap::MessageReference updateDpValueLastDAC(xoap::MessageReference msg) throw (xoap::exception::Exception);
  void decodeDpValueUpdate(xoap::MessageReference msg);
  xoap::MessageReference composeDpValueUpdateResponseFED();
  xoap::MessageReference composeDpValueUpdateRequestPSX();
  xoap::MessageReference composeTempRangeSwitchRequestFEC();

  PixelDCStoFEDDpNameTable* dpNameTable_;
  PixelDCStoFEDDpValueTable* dpValueTable_;
  PixelDCStoFEDDpFilterTable* dpFilterTable_;
  PixelDCStoFEDDpCalibrationTable* dpCalibrationTable_;
  
  std::map<unsigned int, std::map<unsigned int, std::map<unsigned int, PixelDCStoFEDDpConnectionRow*> > > dpConnectionTable_;
  std::list<PixelDCStoFEDDpConnectionRow> dpConnectionList_;

  std::map<pos::PixelROCName, unsigned int> currentTempRangeSettings_;
  
  std::list<const PixelDCStoFEDDpFilterRow*> dpList_increaseTempRangeDAC_;
  std::list<const PixelDCStoFEDDpFilterRow*> dpList_decreaseTempRangeDAC_;

  pos::PixelNameTranslation* nameTranslation_;
  pos::PixelFECConfig* fecConfiguration_;

  PixelDCSSOAPCommander* dcsSOAPCommander_;
  PixelDCSPVSSCommander* dcsPVSSCommander_;

  bool updateCalibratedTemperature_;
  bool updateRawADC_;
  bool updateTempRangeDAC_;
};

#endif
