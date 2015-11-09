#ifndef _pixel_tcds_PixelTCDSSupervisor_HwLeaseHandler_h_
#define _pixel_tcds_PixelTCDSSupervisor_HwLeaseHandler_h_

#include <string>

#include "toolbox/task/TimerListener.h"
#include "toolbox/TimeInterval.h"
//#include "xoap/MessageReference.h"

#include "PixelTCDSSupervisor/PixelTCDSBase.h"

namespace toolbox {
  namespace exception {
    class Listener;
  }
}

namespace toolbox {
  namespace task {
    class Timer;
    class TimerEvent;
  }
}

namespace xdaq {
  class Application;
  class ApplicationDescriptor;
}

namespace pixel {
  namespace tcds {

    class HwLeaseHandler : public toolbox::task::TimerListener
    {

    public:
      HwLeaseHandler(PixelTCDSBase* pixelTCDSbaseP,
                     xdaq::Application* const xdaqAppP,
                     toolbox::TimeInterval const& interval);
      ~HwLeaseHandler();

    private:
      void timeExpired(toolbox::task::TimerEvent& event);

      xdaq::Application* const xdaqAppP_;
      pixel::tcds::PixelTCDSBase* pixelTCDSbaseP_;
      toolbox::TimeInterval interval_;
      std::string const timerName_;
      toolbox::task::Timer* timerP_;

    };
  } // namespace tcds
} // namespace pixel

#endif // _pixel_tcds_PixelTCDSSupervisor_HwLeaseHandler_h_
