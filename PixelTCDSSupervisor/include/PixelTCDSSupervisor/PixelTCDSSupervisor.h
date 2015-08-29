#ifndef pixel_tcds_PixelTCDSSupervisor_h_
#define pixel_tcds_PixelTCDSSupervisor_h_

#include "xdata/String.h"
#include "xdata/UnsignedInteger.h"
#include "xoap/MessageReference.h"

// Ok, let's make this work in XDAQ11 as well as XDAQ12.
#ifndef XDAQ12
#include "xdaq/WebApplication.h"
#else
#include "xdaq/Application.h"
#include "xgi/framework/UIManager.h"
#endif

// FSM
#include "toolbox/fsm/FiniteStateMachine.h"

// workloop for FSM state changes
#include "toolbox/task/WorkLoopFactory.h"
#include "toolbox/task/WaitingWorkLoop.h"
#include "toolbox/task/Action.h"

#include "PixelTCDSSupervisor/PixelTCDSBase.h"

namespace log4cplus {
  class Logger;
}

namespace xcept {
  class Exception;
}

namespace xdaq {
  class ApplicationStub;
  class ApplicationDescriptor;
}

namespace xgi {
  class Input;
  class Output;
}

namespace pixel {
  namespace tcds {

    class PixelTCDSSupervisor :
#ifndef XDAQ12
      public xdaq::WebApplication,
#else
      public xdaq::Application,
      public xgi::framework::UIManager,
#endif
      public PixelTCDSBase
    {

    public:
      XDAQ_INSTANTIATOR();

      PixelTCDSSupervisor(xdaq::ApplicationStub* stub);
      virtual ~PixelTCDSSupervisor();

      // FSM state transition
      void fsmTransition(std::string eventName)
              throw (xcept::Exception);
      
      //
      // SOAP Callback triggers state change initialized -> working
      //
      xoap::MessageReference Configure (xoap::MessageReference msg)
              throw (xoap::exception::Exception);
      
      //
      // Finite State Machine callback for entering state
      //
      void stateChanged(toolbox::fsm::FiniteStateMachine & fsm)
              throw (toolbox::fsm::exception::Exception);             
      
      //
      // WorkLoop function performing until ICIController is configured
      //
      bool working(toolbox::task::WorkLoop* wl);
    
    protected:
      toolbox::fsm::FiniteStateMachine fsm_; // the actual state machine
      toolbox::task::WorkLoop* workLoop_;
      toolbox::task::ActionSignature* job_;

    private:
      void queryFSMStateAction(bool explicitCall = false);
      void queryHwLeaseOwnerAction(bool explicitCall = false);

      void resetAction();
      void initializeAction();
      void configureAction();
      void enableAction();
      void pauseAction();
      void resumeAction();
      void stopAction();
      void haltAction();
      void coldResetAction();
      void ttcResyncAction();
      void ttcHardResetAction();
      void renewHardwareLeaseAction();
      void readHardwareConfigurationAction();
      void sendL1AAction();
      void sendBgoAction(xdata::UnsignedInteger commandUInt);
      void sendBgoStringAction(xdata::String commandString);
      void sendBgoTrainAction(xdata::String trainString);
      void enableRandomTriggersAction(xdata::UnsignedInteger frequencyUInt);
      
      void mainPage(xgi::Input* in, xgi::Output* out);
      void redirect(xgi::Input* in, xgi::Output* out);

      void queryFSMState(xgi::Input* in, xgi::Output* out);
      void queryHwLeaseOwner(xgi::Input* in, xgi::Output* out);

      void reset(xgi::Input* in, xgi::Output* out);
      void initialize(xgi::Input* in, xgi::Output* out);
      void configure(xgi::Input* in, xgi::Output* out);
      void enable(xgi::Input* in, xgi::Output* out);
      void pause(xgi::Input* in, xgi::Output* out);
      void resume(xgi::Input* in, xgi::Output* out);
      void stop(xgi::Input* in, xgi::Output* out);
      void halt(xgi::Input* in, xgi::Output* out);
      void coldReset(xgi::Input* in, xgi::Output* out);
      void ttcResync(xgi::Input* in, xgi::Output* out);
      void ttcHardReset(xgi::Input* in, xgi::Output* out);
      void renewHardwareLease(xgi::Input* in, xgi::Output* out);
      void readHardwareConfiguration(xgi::Input* in, xgi::Output* out);
      void sendL1A(xgi::Input* in, xgi::Output* out);
      void sendBgo(xgi::Input* in, xgi::Output* out);
      void sendBgoString(xgi::Input* in, xgi::Output* out);
      void sendBgoTrain(xgi::Input* in, xgi::Output* out);
      void enableRandomTriggers(xgi::Input* in, xgi::Output* out);
      void updateHardwareConfigurationFile(xgi::Input* in, xgi::Output* out);
      void updateHardwareConfiguration(xgi::Input* in, xgi::Output* out);
      
      xoap::MessageReference fireEvent ( xoap::MessageReference msg ) throw ( xoap::exception::Exception );

      virtual void onException(xcept::Exception& err);
      
      void readConfigFile();

      xdata::String hwCfgString_;
      xdata::String hwCfgFileName_;
      bool receivedCfgString_;

      xdata::UnsignedInteger runNumber_;

      xdata::String tcdsState_;
      xdata::String tcdsHwLeaseOwnerId_;

      xdata::String statusMsg_;
      log4cplus::Logger& logger_;
      
      // PixelSupervisor
      xdaq::ApplicationDescriptor* PixelSupervisor_;
      bool firstTransition;

    }; // class PixelTCDSSupervisor
  } // namespace tcds
} // namespace pixel

#endif // pixel_tcds_PixelTCDSSupervisor_h
