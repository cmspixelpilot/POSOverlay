#include "PixelTCDSSupervisor/PixelPISupervisor.h"

XDAQ_INSTANTIATOR_IMPL(pixel::tcds::PixelPISupervisor);

pixel::tcds::PixelPISupervisor::PixelPISupervisor(xdaq::ApplicationStub* stub)
  : PixelTCDSSupervisor(stub)  {
  tcdsType_="PI";
}

pixel::tcds::PixelPISupervisor::~PixelPISupervisor()
{
}
