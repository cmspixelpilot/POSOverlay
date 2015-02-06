#ifndef H_TTCXDAQBase_hh
#define H_TTCXDAQBase_hh

// includes for XDAQ 3
#if ( XDAQVERSION > 3600 )
#include "xdata/UnsignedInteger32.h"
#else
#include "xdata/UnsignedLong.h"
#endif

#include "xdata/String.h"
#include "xdata/Boolean.h"
#include "xdata/Integer.h"
#include "xdata/Double.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/Application.h"
#include "xdaq/WebApplication.h"// added by Tim C.
#include "toolbox/exception/Handler.h"
#include "toolbox/fsm/FiniteStateMachine.h"
#include "toolbox/Event.h"
#include "xdaq/NamespaceURI.h"

#include "xoap/Method.h"
#include "xdaq/NamespaceURI.h"
#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPPart.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPBody.h"
#include "xoap/domutils.h"

#include "MultiBusAdapter.hh"

#include "CAENLinuxBusAdapter.hh"

#if ( XDAQVERSION > 3600 )
typedef xdata::UnsignedInteger32 xdataULong; 
#else
typedef xdata::UnsignedLong xdataULong;  
#endif

std::string htmlencode(const std::string &);

class TTCXDAQBase : public xdaq::WebApplication,
                    public xdata::ActionListener {
protected:
  TTCXDAQBase(xdaq::ApplicationStub* stub, uint32_t BTimeCorrection);
  virtual ~TTCXDAQBase();
  
    //hyperdaq stuff:
    int localid_;
    int instance_; // set to -1 if no instance no.!
    std::string classname_;
    time_t tnow_, tprevious_;
  //    ttc::MultiBusAdapter *busAdapter_;
    HAL::CAENLinuxBusAdapter * busAdapter_;
    std::stringstream html;


    // Name
    xdata::String name_;
    // bus adapter name
    xdata::String busadaptername_;
    // Configuration file
    //xdata::String asciConfigurationFilePath_;
    // Current state of state machine
    xdata::String StateName_;
    // should the configuration be reloaded every time "Configure" is execued?
    xdata::Boolean ReloadAtEveryConfigure_; // 0 or 1
    //xdata::String ConfigurationString_;
    // Is this 64X compatible? 1=yes, 0=ltcBaseAddress_ to be ignored
    xdata::Boolean Is64XCompatible_;
    // baseAddress of the LTC/TTCci module
    xdata::Integer Location_;
    // hyperdaq control level (0=all allowed, infty=none)
    xdata::Integer CtrlLvl_;
    // B-Channel Timing Correction
    //xdata::UnsignedLong BTimeCorrection_;
    xdataULong BTimeCorrection_;

    // record of last received message and response
    xoap::MessageReference soapReceived_, soapResponse_;

    // the statemachine of the application:
    toolbox::fsm::FiniteStateMachine fsm_;

    //int config_errors_;
    int any_errors_;
    std::stringstream err_message_;

    bool ReadConfigFromFile_;
    bool ConfigModified_;
    double oldfrequency_;
    bool neverSOAPed_;
};

#endif // H_TTCXDAQBase_hh
