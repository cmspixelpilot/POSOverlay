#ifndef pixel_tcds_PixelPISupervisor_h_
#define pixel_tcds_PixelPISupervisor_h_


#include "PixelTCDSSupervisor/PixelTCDSSupervisor.h"

namespace pixel {
  namespace tcds {

    class PixelPISupervisor : public PixelTCDSSupervisor {
    public:
      XDAQ_INSTANTIATOR();

      PixelPISupervisor(xdaq::ApplicationStub* stub);
      virtual ~PixelPISupervisor();
      
    }; // class PixelPISupervisor
  } // namespace tcds
} // namespace pixel

#endif // pixel_tcds_PixelPISupervisor_h
