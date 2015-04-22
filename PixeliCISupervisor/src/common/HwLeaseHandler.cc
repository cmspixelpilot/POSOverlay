#include "PixelTCDSSupervisor/HwLeaseHandler.h"

#include "toolbox/string.h"
#include "toolbox/exception/Listener.h"
#include "toolbox/task/Timer.h"
#include "toolbox/task/TimerEvent.h"
#include "toolbox/task/TimerFactory.h"
#include "toolbox/TimeVal.h"
#include "xcept/Exception.h"
#include "xdaq/Application.h"
#include "xdaq/ApplicationDescriptor.h"
#include "xoap/MessageReference.h"

//#include "PixelTCDSSupervisor/exception/Exception.h"

pixel::tcds::HwLeaseHandler::HwLeaseHandler(PixelTCDSBase* pixelTCDSbase,
                                           xdaq::Application* const xdaqAppP,
                                           toolbox::TimeInterval const& interval) :
  xdaqAppP_(xdaqAppP),
  pixelTCDSbaseP_(pixelTCDSbase),
  interval_(interval),
  timerName_(toolbox::toString("HwLeaseRenewalTimer_uuid%s",
                               xdaqAppP_->getApplicationDescriptor()->getUUID().toString().c_str())),
  timerP_(toolbox::task::getTimerFactory()->createTimer(timerName_))
{
  timerP_->addExceptionListener(pixelTCDSbaseP_);
  toolbox::TimeVal start = toolbox::TimeVal::gettimeofday();
  timerP_->scheduleAtFixedRate(start, this, interval_, 0, "");
}

pixel::tcds::HwLeaseHandler::~HwLeaseHandler()
{
  toolbox::task::getTimerFactory()->removeTimer(timerName_);
}

void
pixel::tcds::HwLeaseHandler::timeExpired(toolbox::task::TimerEvent& event)
{
  pixelTCDSbaseP_->tcdsRenewHardwareLease();
  //pixelTCDSbaseP_->checkForPixelSupervisor();
}
