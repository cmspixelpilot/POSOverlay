#include "PixelTCDSSupervisor/PixelTCDSBase.h"

#include "log4cplus/logger.h"
#include "log4cplus/loggingmacros.h"
#include "toolbox/string.h"
#include "toolbox/TimeInterval.h"
#include "xcept/Exception.h"
#include "xdata/InfoSpace.h"
#include "xdaq/Application.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationDescriptor.h"
#include "xoap/filter/MessageFilter.h"

#include "PixelTCDSSupervisor/exception/Exception.h"
#include "PixelTCDSSupervisor/HwLeaseHandler.h"

pixel::tcds::PixelTCDSBase::PixelTCDSBase(xdaq::Application* const xdaqApp) :
  SOAPCommander(xdaqApp),
  logger_(xdaqApp->getApplicationLogger()),
  sessionId_("dummy_session"),
  tcdsType_("TCDS"),
  xdaqAppP_(xdaqApp),
  tcdsAppClassName_(""),
  tcdsAppInstance_(0),
  hwLeaseHandlerP_(0),
  hwLeaseRenewalInterval_("PT5S")
{
  // Registration of the InfoSpace variables.
  xdaqApp->getApplicationInfoSpace()->fireItemAvailable("tcdsAppClassName", &tcdsAppClassName_);
  xdaqApp->getApplicationInfoSpace()->fireItemAvailable("tcdsAppInstance", &tcdsAppInstance_);
  xdaqApp->getApplicationInfoSpace()->fireItemAvailable("sessionId", &sessionId_);
  xdaqApp->getApplicationInfoSpace()->fireItemAvailable("hardwareLeaseRenewalInterval",
                                                          &hwLeaseRenewalInterval_);
}


pixel::tcds::PixelTCDSBase::~PixelTCDSBase()
{
}

std::string
pixel::tcds::PixelTCDSBase::tcdsAppClassName()
{
  return tcdsAppClassName_.toString();
}

unsigned int
pixel::tcds::PixelTCDSBase::tcdsAppInstance()
{
  return (unsigned int)(tcdsAppInstance_);
}

std::string
pixel::tcds::PixelTCDSBase::sessionId()
{
  return sessionId_.toString();
}

std::string
pixel::tcds::PixelTCDSBase::hwLeaseRenewalInterval()
{
  return hwLeaseRenewalInterval_.toString();
}

std::string
pixel::tcds::PixelTCDSBase::tcdsQueryFSMState()
{
  xoap::MessageReference cmd = createSimpleSOAPParameterGet(tcdsAppClassName(), "stateName", "xsd:string");
  xoap::MessageReference reply = postSOAPCommand(cmd,getDestinationDescriptor());
  // NOTE: This is a bit rough-and-ready. It is assumed that this will
  // work, if so far nothing has thrown any exceptions.
  xoap::filter::MessageFilter filter("//p:stateName");
  // ASSERT ASSERT ASSERT
  assert (filter.match(reply));
  // ASSERT ASSERT ASSERT end
  std::list<xoap::SOAPElement> elements = filter.extract(reply);
  // ASSERT ASSERT ASSERT
  assert (elements.size() == 1);
  // ASSERT ASSERT ASSERT end
  std::string const stateName = elements.front().getTextContent();
  return stateName;
}

std::string
pixel::tcds::PixelTCDSBase::tcdsQueryHwLeaseOwnerId()
{
  xoap::MessageReference cmd = createSimpleSOAPParameterGet(tcdsAppClassName(), "hwLeaseOwnerId", "xsd:string");
  xoap::MessageReference reply = postSOAPCommand(cmd,getDestinationDescriptor());
  // NOTE: This is a bit rough-and-ready. It is assumed that this will
  // work, if so far nothing has thrown any exceptions.
  xoap::filter::MessageFilter filter("//p:hwLeaseOwnerId");
  // ASSERT ASSERT ASSERT
  assert (filter.match(reply));
  // ASSERT ASSERT ASSERT end
  std::list<xoap::SOAPElement> elements = filter.extract(reply);
  // ASSERT ASSERT ASSERT
  assert (elements.size() == 1);
  // ASSERT ASSERT ASSERT end
  std::string const hwLeaseOwnerId = elements.front().getTextContent();
  return hwLeaseOwnerId;
}

void
pixel::tcds::PixelTCDSBase::tcdsConfigure(std::string const& hwCfgString)
{
  xoap::MessageReference cmd = createConfigureSOAPCommand(hwCfgString);
  postSOAPCommand(cmd,getDestinationDescriptor());

  toolbox::TimeInterval interval;
  interval.fromString(hwLeaseRenewalInterval_);
  hwLeaseHandlerP_ =
    std::auto_ptr<pixel::tcds::HwLeaseHandler>(new HwLeaseHandler(this, app_, interval));

}

void
pixel::tcds::PixelTCDSBase::tcdsEnable(unsigned int const runNumber)
{
  xoap::MessageReference cmd = createEnableSOAPCommand(runNumber);
  postSOAPCommand(cmd,getDestinationDescriptor());
}

void
pixel::tcds::PixelTCDSBase::tcdsPause()
{
  xoap::MessageReference cmd = createPauseSOAPCommand();
  postSOAPCommand(cmd,getDestinationDescriptor());
}

void
pixel::tcds::PixelTCDSBase::tcdsResume()
{
  xoap::MessageReference cmd = createResumeSOAPCommand();
  postSOAPCommand(cmd,getDestinationDescriptor());
}

void
pixel::tcds::PixelTCDSBase::tcdsStop()
{
  xoap::MessageReference cmd = createStopSOAPCommand();
  postSOAPCommand(cmd,getDestinationDescriptor());
}

void
pixel::tcds::PixelTCDSBase::tcdsHalt()
{
  xoap::MessageReference cmd = createHaltSOAPCommand();
  postSOAPCommand(cmd,getDestinationDescriptor());
  hwLeaseHandlerP_.reset();
}

void
pixel::tcds::PixelTCDSBase::tcdsColdReset()
{
  xoap::MessageReference cmd = createColdResetSOAPCommand();
  postSOAPCommand(cmd,getDestinationDescriptor());
  hwLeaseHandlerP_.reset();
}

void
pixel::tcds::PixelTCDSBase::tcdsTTCResync()
{
  xoap::MessageReference cmd = createTTCResyncSOAPCommand();
  postSOAPCommand(cmd,getDestinationDescriptor());
}

void
pixel::tcds::PixelTCDSBase::tcdsTTCHardReset()
{
  xoap::MessageReference cmd = createTTCHardResetSOAPCommand();
  postSOAPCommand(cmd,getDestinationDescriptor());
}

void
pixel::tcds::PixelTCDSBase::tcdsRenewHardwareLease()
{
  xoap::MessageReference cmd = createRenewHardwareLeaseSOAPCommand();
  postSOAPCommand(cmd,getDestinationDescriptor());
}

std::string
pixel::tcds::PixelTCDSBase::tcdsReadHardwareConfiguration()
{
  xoap::MessageReference cmd = createReadHardwareConfigurationSOAPCommand();
  xoap::MessageReference reply = postSOAPCommand(cmd,getDestinationDescriptor());
  // NOTE: This is a bit rough-and-ready. It is assumed that this will
  // work, if so far nothing has thrown any exceptions.
  xoap::filter::MessageFilter filter("//p:ReadHardwareConfiguration");
  // ASSERT ASSERT ASSERT
  assert (filter.match(reply));
  // ASSERT ASSERT ASSERT end
  std::list<xoap::SOAPElement> elements = filter.extract(reply);
  // ASSERT ASSERT ASSERT
  assert (elements.size() == 1);
  // ASSERT ASSERT ASSERT end
  std::string const hardwareConfiguration = elements.front().getTextContent();
  return hardwareConfiguration;
}

void
pixel::tcds::PixelTCDSBase::tcdsSendL1A()
{
  xoap::MessageReference cmd = createSendL1ASOAPCommand();
  postSOAPCommand(cmd,getDestinationDescriptor());
}

void
pixel::tcds::PixelTCDSBase::tcdsSendBgoString(std::string bgoName)
{  
  xoap::MessageReference cmd = createSendBgoSOAPCommand(bgoName);
  postSOAPCommand(cmd,getDestinationDescriptor());
}

void
pixel::tcds::PixelTCDSBase::tcdsSendBgoTrain(std::string bgoTrainName)
{  
  xoap::MessageReference cmd = createSendBgoTrainSOAPCommand(bgoTrainName);
  postSOAPCommand(cmd,getDestinationDescriptor());
}

void
pixel::tcds::PixelTCDSBase::tcdsSendBgo(unsigned int bgoNumber) 
{
  xoap::MessageReference cmd = createSendBgoSOAPCommand(bgoNumber);
  postSOAPCommand(cmd,getDestinationDescriptor());
}

void
pixel::tcds::PixelTCDSBase::tcdSendBcommand(unsigned int bcommandData, std::string bcommandType, unsigned int bcommandAddress, unsigned int bcommandSubAddress, std::string subaddressType) {
  // not implemented
}

void
pixel::tcds::PixelTCDSBase::tcdsEnableRandomTriggers(unsigned int frequency) 
{
  xoap::MessageReference cmd = createEnableRandomTriggersSOAPCommand(frequency);
  postSOAPCommand(cmd,getDestinationDescriptor());
}


xoap::MessageReference
pixel::tcds::PixelTCDSBase::createSendBgoSOAPCommand(unsigned int const bgoNumber)
{
  assert(bgoNumber >= 0 && bgoNumber <=15);
  return( createComplexSOAPCommand("SendBgo",sessionId(),"bgoNumber",bgoNumber) );
}

xoap::MessageReference
pixel::tcds::PixelTCDSBase::createSendBgoSOAPCommand(std::string const& bgoName)
{
  return( createComplexSOAPCommand("SendBgo",sessionId(),"bgoName",bgoName) );
}

xoap::MessageReference
pixel::tcds::PixelTCDSBase::createSendBgoTrainSOAPCommand(std::string const& bgoTrainName)
{
  return( createComplexSOAPCommand("SendBgoTrain",sessionId(),"bgoTrainName",bgoTrainName) );
}

xoap::MessageReference
pixel::tcds::PixelTCDSBase::createEnableRandomTriggersSOAPCommand(unsigned int const frequency)
{
  return( createComplexSOAPCommand("EnableRandomTriggers",sessionId(),"frequency",frequency) );
}

xoap::MessageReference
pixel::tcds::PixelTCDSBase::createSendL1ASOAPCommand()
{
  return( createSimpleSOAPCommand("SendL1A", sessionId()) );
}




xoap::MessageReference
pixel::tcds::PixelTCDSBase::createConfigureSOAPCommand(std::string const& hwCfgString)
{
  return( createComplexSOAPCommand("Configure",sessionId(),"hardwareConfigurationString",hwCfgString) );
}


xoap::MessageReference
pixel::tcds::PixelTCDSBase::createEnableSOAPCommand(unsigned int const runNumber)
{
  return( createComplexSOAPCommand("Enable",sessionId(),"runNumber",runNumber) );
}

xoap::MessageReference
pixel::tcds::PixelTCDSBase::createPauseSOAPCommand()
{
  return( createSimpleSOAPCommand("Pause", sessionId()) );
}

xoap::MessageReference
pixel::tcds::PixelTCDSBase::createResumeSOAPCommand()
{
  return( createSimpleSOAPCommand("Resume", sessionId()) );
}

xoap::MessageReference
pixel::tcds::PixelTCDSBase::createStopSOAPCommand()
{
  return( createSimpleSOAPCommand("Stop", sessionId()) );
}

xoap::MessageReference
pixel::tcds::PixelTCDSBase::createHaltSOAPCommand()
{
  return( createSimpleSOAPCommand("Halt", sessionId()) );
}

xoap::MessageReference
pixel::tcds::PixelTCDSBase::createColdResetSOAPCommand()
{
  return( createSimpleSOAPCommand("ColdReset", sessionId()) );
}

xoap::MessageReference
pixel::tcds::PixelTCDSBase::createTTCResyncSOAPCommand()
{
  return( createSimpleSOAPCommand("Resync", sessionId()) );
}

xoap::MessageReference
pixel::tcds::PixelTCDSBase::createTTCHardResetSOAPCommand()
{
  return( createSimpleSOAPCommand("HardReset", sessionId()) );
}

xoap::MessageReference
pixel::tcds::PixelTCDSBase::createRenewHardwareLeaseSOAPCommand()
{
  return( createSimpleSOAPCommand("RenewHardwareLease", sessionId()) );
}


xoap::MessageReference
pixel::tcds::PixelTCDSBase::createStateNameSOAPCommand()
{
  return( createSimpleSOAPParameterGet(tcdsAppClassName(), "stateName", "xsd:string") );
}

xoap::MessageReference
pixel::tcds::PixelTCDSBase::createHWLeaseOwnerIdSOAPCommand()
{
  return( createSimpleSOAPParameterGet(tcdsAppClassName(), "hwLeaseOwnerId", "xsd:string") );
}

xoap::MessageReference
pixel::tcds::PixelTCDSBase::createReadHardwareConfigurationSOAPCommand()
{
  return( createSimpleSOAPParameterGet(tcdsAppClassName(), "ReadHardwareConfiguration", "xsd:string") );
}




xdaq::ApplicationDescriptor*
pixel::tcds::PixelTCDSBase::getDestinationDescriptor()
{
  if (tcdsAppClassName_ == "")
    {
      std::string const msg = "No value set for the tcdsAppClassName parameter.";
      LOG4CPLUS_ERROR(logger_, msg);
      XCEPT_RAISE(pixel::tcds::exception::ValueError, msg);
    }

  xdaq::ApplicationDescriptor* desc =
    app_->getApplicationContext()->getDefaultZone()->getApplicationDescriptor(tcdsAppClassName_,
                                                                                   tcdsAppInstance_);
  return desc;
}



void
pixel::tcds::PixelTCDSBase::checkForPixelSupervisor()
{

  try {
    std::set<xdaq::ApplicationDescriptor*> set_PixelSupervisors = app_->getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptors("PixelSupervisor");
    for (std::set<xdaq::ApplicationDescriptor*>::iterator i_set_PixelSupervisor=set_PixelSupervisors.begin(); i_set_PixelSupervisor!=set_PixelSupervisors.end(); ++i_set_PixelSupervisor) {
      try {
	//PixelSupervisor is alive and responsive --> do nothing
        std::string fsmState=Send(*i_set_PixelSupervisor, "FSMStateRequest");
      } catch (xdaq::exception::Exception& e) {
	//PixelSupervisor is in the XDAQ config but it does not reply to FSMStateRequests --> send TCDS hardware to halted
        std::string const msg = "Cannot find any running PixelSupervisor. Send PixeliCISupervisor to 'halted'";
        LOG4CPLUS_ERROR(logger_, msg);
        std::cout << "Cannot find any running PixelSupervisor. Send PixeliCISupervisor to 'halted'" << std::endl;
        tcdsHalt();
      }
    }
  } catch (xdaq::exception::Exception& e) {
    //there is no PixelSupervisor in the XDAQ config --> do nothing!
  }
}



void
pixel::tcds::PixelTCDSBase::onException(xcept::Exception& err)
{
  std::string const msg =
    toolbox::toString("An error occurred in the hardware lease renewal thread : '%s'.",
                      err.what());
  LOG4CPLUS_ERROR(logger_, "HardWoodBase");
  LOG4CPLUS_ERROR(logger_, msg.c_str());
}
