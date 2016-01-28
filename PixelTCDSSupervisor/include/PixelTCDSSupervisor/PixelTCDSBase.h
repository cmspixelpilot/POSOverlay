#ifndef _pixel_tcds_PixelTCDSBase_h_
#define _pixel_tcds_PixelTCDSBase_h_

#include <memory>
#include <string>

#include "toolbox/exception/Listener.h"
#include "xdata/String.h"
#include "xdata/UnsignedInteger.h"
#include "xoap/MessageReference.h"

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"


namespace log4cplus {
  class Logger;
}

namespace xcept {
  class Exception;
}

namespace xdaq {
  class Application;
  class ApplicationDescriptor;
}

namespace pixel {
  namespace tcds {

    class HwLeaseHandler;

    class PixelTCDSBase : public toolbox::exception::Listener, public SOAPCommander
    {


    public:
      void tcdsRenewHardwareLease();
      void checkForPixelSupervisor();
      
    protected:
      PixelTCDSBase(xdaq::Application* const xdaqApp);
      ~PixelTCDSBase();
    
      std::string tcdsAppClassName();
      unsigned int tcdsAppInstance();
      std::string sessionId();
      std::string hwLeaseRenewalInterval();

      std::string tcdsQueryFSMState();
      std::string tcdsQueryHwLeaseOwnerId();

      void tcdsConfigure(std::string const& hwCfgString);
      void tcdsEnable(unsigned int const runNumber);
      void tcdsPause();
      void tcdsResume();
      void tcdsStop();
      void tcdsHalt();
      void tcdsColdReset();
      void tcdsTTCResync();
      void tcdsTTCHardReset();

      /// read back the hardware configuration in the form of a register dump
      /// returns a list of register name-value pairs, one word per line, immediately usable in a ‘Configure’ SOAP command.
      std::string tcdsReadHardwareConfiguration();

      /// Send a single L1A
      void tcdsSendL1A();
    
      /// send a single B-go
      /// bgoName: The name identifying the B-go to be sent.
      /// Valid choices: ‘BgoX’ with ‘X’ in [0, 15], ‘BC0’, ‘TestEnable’, ‘PrivateGap’, ‘PrivateOrbit’, ‘Resync’,
      /// ‘HardReset’, ‘EC0’, ‘OC0’, Start’, ‘Stop’, ‘StartOfGap’, ‘WarningTestEnable’.
      void tcdsSendBgoString(std::string bgoName);
      
      /// send a sequence of B-gos.
      /// Note: this requires software B-gos to be enabled in the iCI/iPM.
      /// bgoTrainName: The name identifying the B-go sequence to be sent.
      /// Valid choices: ‘Start’, ‘Stop’, ‘Pause’, ‘Resume’, ‘TTCResync’, ‘TTCHardReset’.
      void tcdsSendBgoTrain(std::string bgoTrainName);
    
      /// send a single B-go
      /// bgoNumber: The number identifying the B-go to be sent. Valid range: [0, 15].
      void tcdsSendBgo(unsigned int bgoNumber);
    
      /// send a single B-command
      /// Note: This command directly outputs the B-command on the TTC fibre, circumventing any counters and B-channel configurations.
      /// bcommandData: The B-command data to be sent. Valid range: [0x0, 0xff ].
      /// bcommandType: The type of B-command to be sent: either a short (i.e., broadcast) command or a long (i.e., addressed)
      ///   command. Valid choices: ‘short’, ‘long’, ‘broadcast’, ‘addressed’.
      /// bcommandAddress: The address of the TTCrx to address with this B-command. (For long/addressed B-commands only.)
      ///   Valid range: [0x0, 0x3fff ].
      ///   Note: using address 0x0 performs a pseudo-broadcast command.
      /// bcommandSubAddress: The subaddress to write the B-command data to. (For long/addressed B-commands only.) Valid range: [0x0, 0xff].
      /// subaddressType: The type of the subaddress: the ‘internal’ address space contains the internal TTCrx registers, ‘external’
      ///   addresses are made available on the TTCrx output bus. (For long/addressed B-commands only.) Valid choices: ‘internal’ or ‘external’.
      void tcdSendBcommand(unsigned int bcommandData, 
                           std::string bcommandType,
                           unsigned int bcommandAddress = 0x0,
                           unsigned int bcommandSubAddress =  0x0,
                           std::string subaddressType = "internal"
                          );
    
      /// enable random triggers at the desired frequency.
      /// frequency: The desired frequency of random triggers in Hz.
      void tcdsEnableRandomTriggers(unsigned int frequency);
      
      /// initialise all cyclic generators. Use this before (re)starting any non-permanent cyclic generators to
      /// (re)initialise the pre- and post-scale counters.
      void tcdsInitCyclicGenerators();
    
      // prepare TTC commands
      xoap::MessageReference createSendBgoSOAPCommand(unsigned int const bgoNumber);
      xoap::MessageReference createSendBgoSOAPCommand(std::string const& bgoName);
      xoap::MessageReference createSendBgoTrainSOAPCommand(std::string const& bgoTrainName);
      xoap::MessageReference createEnableRandomTriggersSOAPCommand(unsigned int const frequency);
      xoap::MessageReference createSendL1ASOAPCommand();
      
      /// prepare FSM change SOAP messages
      xoap::MessageReference createConfigureSOAPCommand(std::string const& hwCfgString);
      xoap::MessageReference createEnableSOAPCommand(unsigned int const runNumber);
      xoap::MessageReference createPauseSOAPCommand();
      xoap::MessageReference createResumeSOAPCommand();
      xoap::MessageReference createStopSOAPCommand();
      xoap::MessageReference createHaltSOAPCommand();
      xoap::MessageReference createColdResetSOAPCommand();
      xoap::MessageReference createTTCResyncSOAPCommand();
      xoap::MessageReference createTTCHardResetSOAPCommand();
      xoap::MessageReference createRenewHardwareLeaseSOAPCommand();
      
      /// prepare query SOAP messages
      xoap::MessageReference createStateNameSOAPCommand();
      xoap::MessageReference createHWLeaseOwnerIdSOAPCommand();
      xoap::MessageReference createReadHardwareConfigurationSOAPCommand();
      
      virtual void onException(xcept::Exception& err);

      log4cplus::Logger& logger_;

      xdata::String sessionId_;
      xdata::String tcdsType_;
      
    private:
      xdaq::ApplicationDescriptor* getDestinationDescriptor();

      bool isTimerActive() const;

      xdaq::Application* xdaqAppP_;

      // These two parameters are used to find the TCDS application to
      // talk to in the XDAQ configuration (.xml) file.
      xdata::String tcdsAppClassName_;
      xdata::UnsignedInteger tcdsAppInstance_;

      // A pointer to the automatic hardware-lease-renewal thingy.
      std::auto_ptr<pixel::tcds::HwLeaseHandler> hwLeaseHandlerP_;
      xdata::String hwLeaseRenewalInterval_;

    };
  } // namespace tcds
} // namespace pixel

#endif // _pixel_tcds_PixelTCDSBase_h_
