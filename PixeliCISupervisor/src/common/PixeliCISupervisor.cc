#include "PixelTCDSSupervisor/PixeliCISupervisor.h"

XDAQ_INSTANTIATOR_IMPL(pixel::tcds::PixeliCISupervisor);

pixel::tcds::PixeliCISupervisor::PixeliCISupervisor(xdaq::ApplicationStub* stub)
  : PixelTCDSSupervisor(stub)  {
  tcdsType_="iCI";
}

pixel::tcds::PixeliCISupervisor::~PixeliCISupervisor()
{
}
