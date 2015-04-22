#ifndef pixel_tcds_PixeliCISupervisor_h_
#define pixel_tcds_PixeliCISupervisor_h_


#include "PixelTCDSSupervisor/PixelTCDSSupervisor.h"

namespace pixel {
  namespace tcds {

    class PixeliCISupervisor : public PixelTCDSSupervisor {
    public:
      XDAQ_INSTANTIATOR();

      PixeliCISupervisor(xdaq::ApplicationStub* stub);
      virtual ~PixeliCISupervisor();
      
    }; // class PixeliCISupervisor
  } // namespace tcds
} // namespace pixel

#endif // pixel_tcds_PixeliCISupervisor_h
