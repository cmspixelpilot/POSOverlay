#ifndef H_PixelTTCSupervisor_h
#define H_PixelTTCSupervisor_h

#include "TTCXDAQBase.hh"
#include "headers.hh"
#include "toolbox/Runtime.h"

// TTCci Stuff:
#include "TTCci.hh"
#include "TTCciXdaqUtils.hh"

#include "CalibFormats/SiPixelObjects/interface/PixelTTCciConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelConfigFile.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"

// gio
#include "diagbag/DiagBagWizard.h"
#include "DiagCompileOptions.h"
#include "toolbox/task/Timer.h"
#include "toolbox/task/TimerFactory.h"
#include "toolbox/task/TimerListener.h"
#include "toolbox/TimeInterval.h"
#include "toolbox/BSem.h"
//


class PixelTTCSupervisor : public TTCXDAQBase, public SOAPCommander, public toolbox::task::TimerListener {

public:

  // gio
  toolbox::BSem executeReconfMethodMutex;
  DiagBagWizard * diagService_;
  //

  XDAQ_INSTANTIATOR();

    /**
     * Constructor of the PixelTTCSupervisor application.
     */
    PixelTTCSupervisor(xdaq::ApplicationStub* stub);

    /**
     * Destructor of the PixelTTCSupervisor application.
     */
    virtual ~PixelTTCSupervisor();

    //gio
     void timeExpired (toolbox::task::TimerEvent& e);
    //

    /**
     * Single PixelTTCSupervisor issued via a soap message.
     */
    xoap::MessageReference userPixelTTCSupervisor(xoap::MessageReference msg )
        throw( xoap::exception::Exception);

    /**
     * Implements the state change from Halted to Ready.
     * Called by the final state machine.
     */
    void ConfigureAction(toolbox::Event::Reference e) throw(xcept::Exception);

    /**
     * Implements the state change from Ready to Running.
     * Called by the final state machine.
     */
    void EnableAction(toolbox::Event::Reference e) throw(xcept::Exception);

    /**
     * Implements the state change from Running to Ready
     * Called by the final state machine.
     */
    void StopAction(toolbox::Event::Reference e) throw(xcept::Exception);

    /**
     * Implements the state change from Ready to Paused.
     * Called by the final state machine.
     */
    void SuspendAction(toolbox::Event::Reference e) throw(xcept::Exception);

    /**
     * Implements the state change from Paused to Running.
     * Called by the final state machine.
     */
    void ResumeAction(toolbox::Event::Reference e) throw(xcept::Exception);

    /**
     * Implements the state change from any state to Halted.
     * Called by the final state machine.
     */
    void HaltAction(toolbox::Event::Reference e) throw(xcept::Exception);

    /**
     * Implements the User Sequence call
     */
    xoap::MessageReference UserAction(xoap::MessageReference msg) throw(xoap::exception::Exception);
    /**
     * Implements the LevelOne Sequence call.
     */
    xoap::MessageReference LevelOneAction(xoap::MessageReference msg) throw(xoap::exception::Exception);
    /**
     * Implements the CalSync Sequence call.
     */
    xoap::MessageReference CalSyncAction(xoap::MessageReference msg) throw(xoap::exception::Exception);
    /**
     * Implements the ResetROC Sequence call.
     */
    xoap::MessageReference ResetROCAction(xoap::MessageReference msg) throw(xoap::exception::Exception);
    /**
     * Implements the ResetTBM Sequence call.
     */
    xoap::MessageReference ResetTBMAction(xoap::MessageReference msg) throw(xoap::exception::Exception);
    /**
     * Reconfigures the TTC to send assorted periodic signals
     */
    xoap::MessageReference SendPeriodic(xoap::MessageReference msg) throw(xoap::exception::Exception);
    /**
     * Stops periodic signals and returns TTC to its original configuration
     */
    xoap::MessageReference StopPeriodic(xoap::MessageReference msg) throw(xoap::exception::Exception);
    /**
     * Implemention of the interface of the ActionListener.
     */
    void actionPerformed (xdata::Event& e);

protected:

    /**
     * Message which triggers a state change.
     *
     * Soap message which should change the state of the application are
     * bound to this function. In this function the body of the Message
     * is decoded and an Event containing the state change command, is 
     * constructed. This event is then sent to the Final State Machine. 
     * It is this Statemachine which according to its configuration finds
     * out if the requested state change is legal, and if so, it calls
     * corresponding action-method in this class. If an illegal state 
     * state change has been requested, an exception is thrown.
     *
     */
    xoap::MessageReference changeState (xoap::MessageReference msg) 
        throw (xoap::exception::Exception);

    /**
     * A small helper function to clean up.
     */
    void deleteHardware();

    // Create  a SOAP Message for PixelTTCSupervisor (added by Tim C.)
    xoap::MessageReference CreateSOAPMessage4PixelTTCSupervisor(const std::string &commandname, const std::string &varName="", const std::string &var="") const;
    // Send a SOAP Message to PixelTTCSupervisor (added by Tim C.)
    void SendSOAPMessageToPixelTTCSupervisor(const std::string &commandname, const std::string &varName="", const std::string &var=""); 

    xoap::MessageReference getStateRCMS(xoap::MessageReference msg)
      throw (xoap::exception::Exception);
    // The Web Page (added by Tim C.)
    void Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

    // Functions called by the Web Page (added by Tim C.)
    void CreateTTCciObject(const bool LoadConfiguration=true);
    void dispatch (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
    //void ConfigurePage(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
    void MainConfiguration(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
    void BGOConfiguration(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
    void CyclicGenerators(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
    void SummaryPage(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
    void RegisterAccess(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
    void Sequences(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
    void NewConfiguration (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
    void ReadWriteParameters (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
    void DebugTest (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
    void NewConfigurationFile (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
    void WriteConfigurationFile (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
    void RedirectMessage(xgi::Output * out, const std::string &url="");
    void PrintHTMLHeader(xgi::Output *out, 
                         const std::string &title="TTCci");
    void PrintHTMLFooter(xgi::Output *out);
    void ReadTTCciContent();
    void ConfigureTTCcifromASCI();
    xoap::MessageReference userCommand (xoap::MessageReference msg) 
      throw( xoap::exception::Exception);
    void MainConfigCommand (xgi::Input * in, xgi::Output * out)  
      throw (xgi::exception::Exception);
    void TriggerRules (xgi::Input * in, xgi::Output * out)  
      throw (xgi::exception::Exception);
    void BGOSelectCommand (xgi::Input * in, xgi::Output * out)  
      throw (xgi::exception::Exception);
    void SequenceSelectCommand (xgi::Input * in, xgi::Output * out)  
      throw (xgi::exception::Exception);
    void SequenceEditCommand (xgi::Input * in, xgi::Output * out)  
      throw (xgi::exception::Exception);
    void BGOConfigCommand (xgi::Input * in, xgi::Output * out)  
      throw (xgi::exception::Exception);
    void CyclicConfigCommand (xgi::Input * in, xgi::Output * out)  
      throw (xgi::exception::Exception);
    void RegisterAccessCommand (xgi::Input * in, xgi::Output * out)  
      throw (xgi::exception::Exception);
    void VMEBData (xgi::Input * in, xgi::Output * out)  
      throw (xgi::exception::Exception);
    void SendCommand (xgi::Input * in, xgi::Output * out)  
      throw (xgi::exception::Exception);
    void PrintSMState(xgi::Output *out);
    void WriteOutputToStream(std::ostream *out);
    void CheckSizeOfHTMLOutput();
    void WriteDebugTextArray(std::ostream *out, bool printSOAP=false);
    void ReadTTCciCounters();
    std::string GetCurrentTime();
    void InitHTMLFields();
    uint32_t GetBGOWord(size_t channel, size_t iword, bool takeDword);
    void ErrorStatement(xgi::Output * out);
    void GetFileList();
    void ConfigureIcon();
    std::string GetName(const int opt=0) const;

    /**
     * The pointer to the TTCciCard.
     *
     * The BaseAddress to the TTCci is a paramter of the XDAQ application.
     * Therefore the TTCciCard can only be constructed when the parameters
     * are already loaded into the XDAQ application. Instantiation is defered
     * to the configure routine. 
     */
    ttc::TTCci *TTCciPtr_;

  //Pointers to the CalibFormats/SiPixelObjects objects
  pos::PixelTTCciConfig* theTTCciConfig_;
  pos::PixelConfigKey* theGlobalKey_;

  //Path to configuration file, old configuration state
  std::string asciConfigurationFilePath_;
  unsigned int oldkey_;
  std::string oldState_;

    // Last TTCci page visited
    std::string LastPage_;

    // AddressTable for the TTCci
    //xdata::String xmlAddressTablePath_;
    // Double B command Timing Correction (if second cmnd is an A-cmnd.)
    xdataULong DelayT2Correction_;

    // List of configured VME triggers
    //std::vector<std::string> vmeTrigList_;
    //
    uint32_t previousEventCounter_;
    xdataULong EventCounter_;
    //
    uint32_t previousOrbitCounter_;
    xdataULong OrbitCounter_;
    //
    uint32_t previousStrobeCounter_;
    xdataULong StrobeCounter_;
    //
    xdataULong BoardStatus_;
    //
    xdata::String ClockSource_;
    xdataULong ClockFrequency_;
    xdata::String OrbitSource_;
    xdata::String TriggerSource_;
    xdata::String BGOSource_;

    // HTML Stuff:
    size_t CurrentBGO;
    ttc::HTMLFieldElement InputFileList;
    ttc::HTMLFieldElement CurrentBGOList;
    ttc::HTMLFieldElement CurrentCyclicList;
    ttc::HTMLFieldElement TriggerSourceList;
    ttc::HTMLFieldElement TriggerSourceList_other;
    ttc::HTMLFieldElement OrbitSourceList;
    ttc::HTMLFieldElement ClockSourceList;
    ttc::HTMLFieldElement BGOSourceList;
    ttc::HTMLFieldElement BGOSourceList_other;
    ttc::HTMLFieldElement VMEAddrList1;
    ttc::HTMLFieldElement VMEAddrList2;
    ttc::HTMLFieldElement DataTypeSelector;
    ttc::HTMLFieldElement SequenceSelector;
    bool AutoRefresh_;
    
    // for debug registers VMEDATS/L:
    bool _debugb_short;
    uint32_t _debugbdata;
    
    void DIAG_CONFIGURE_CALLBACK();
    void DIAG_APPLY_CALLBACK();

    xdaq::ApplicationDescriptor* PixelSupervisor_;

    /* xgi method called when the link <display_diagsystem> is clicked */
    void callDiagSystemPage(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

};

#endif
