/**************************************************************************
 * XDAQ Components for Pixel Online Software                              *
 * Copyright (C) 2007, Cornell University		                  *
 * All rights reserved.                                                   *
 * Authors: Souvik Das, Anders Ryd, Karl Ecklund			  *
  *************************************************************************/

#ifndef _PixelFECSupervisor_h_
#define _PixelFECSupervisor_h_

#include "xdaq/Application.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/ApplicationStubImpl.h"
#include "xdaq/exception/Exception.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/NamespaceURI.h"

#include "xdata/UnsignedLong.h"
#include "xdata/String.h"

#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/AttachmentPart.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPMessage.h"
#include "xoap/SOAPBody.h"
#include "xoap/SOAPPart.h"
#include "xoap/DOMParser.h"

#include "xoap/domutils.h"

#include "xoap/Method.h"

// Includes for the GUI
#include "xdaq/WebApplication.h"
#include "xgi/Method.h"
#include "xgi/Utils.h"
#include "cgicc/HTMLClasses.h"

#include <diagbag/DiagBagWizard.h>
#include "DiagCompileOptions.h"

#include "toolbox/task/Timer.h"
#include "toolbox/task/TimerFactory.h"
#include "toolbox/task/TimerListener.h"
#include "toolbox/TimeInterval.h"
#include "toolbox/BSem.h"



#include "toolbox/fsm/AsynchronousFiniteStateMachine.h"
#include "toolbox/fsm/FailedEvent.h"
#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"
#include "toolbox/BSem.h"


#include "VMEAddressTable.hh"
#include "VMEAddressTableXMLFileReader.hh"
#include "VMEAddressTableASCIIReader.hh"
#include "VMEDevice.hh" // add d.k.

#ifdef VMEDUMMY
#include "VMEDummyBusAdapter.hh"
#else
#include "CAENLinuxBusAdapter.hh"
#endif

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"
#include "PixelUtilities/PixelGUIUtilities/include/HTML2XGI.h"
#include "PixelUtilities/PixelDCSUtilities/include/PixelPowerMap4603.h"
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"

#include "PixelFECInterface/include/PixelFECInterface.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFECConfigInterface.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"
#include "CalibFormats/SiPixelObjects/interface/PixelHdwAddress.h"
#include "CalibFormats/SiPixelObjects/interface/PixelNameTranslation.h"
#include "CalibFormats/SiPixelObjects/interface/PixelMaskAllPixels.h"
#include "CalibFormats/SiPixelObjects/interface/PixelMaskOverride.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTrimAllPixels.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTrimOverride.h"
#include "CalibFormats/SiPixelObjects/interface/PixelMaskBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelMaskOverrideBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTrimBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTrimOverrideBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACSettings.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTBMSettings.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDetectorConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelConfigFile.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibBase.h"
#include "PixelUtilities/Pixelb2inUtilities/include/Pixelb2inCommander.h"

class pos::PixelCalibConfiguration;
class pos::PixelFECConfig;

class PixelFECSupervisor: public xdaq::Application, public SOAPCommander, public toolbox::task::TimerListener, public Pixelb2inCommander
{
  public:

    // gio
    toolbox::BSem executeReconfMethodMutex;
    DiagBagWizard * diagService_;
    //

    XDAQ_INSTANTIATOR();

    PixelFECSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);
    ~PixelFECSupervisor();

    //gio
    void timeExpired (toolbox::task::TimerEvent& e);

    void Default(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void StateMachineXgiHandler(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void mFECsHubs(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void mFECsHubs_XgiHandler(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void Panel (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void Panel_XgiHandler (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void ROC (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void ROCDACs (xgi::Output *out,
                  std::string url,
                  std::string FECBaseAddress,
                  unsigned int mFEC,
                  unsigned int mFECChannel,
                  unsigned int hub,
                  unsigned int port,
                  unsigned int roc,
                  std::string dacname,
                  std::string dacaddress,
                  unsigned int dacvalue,
                  unsigned int bits);
    void ROC_XgiHandler (xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    float readCurrent (pos::PixelModuleName); 

    xoap::MessageReference Initialize (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Configure (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Start (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Stop (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Pause (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Resume (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Halt (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Recover (xoap::MessageReference msg) ;

    //Soft Error Stuff
    xoap::MessageReference FixSoftError(xoap::MessageReference msg);
    xoap::MessageReference ResumeFromSoftError(xoap::MessageReference msg);

    xoap::MessageReference preConfigure (xoap::MessageReference msg) ;
    xoap::MessageReference Reconfigure (xoap::MessageReference msg);

    xoap::MessageReference FSMStateRequest (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference TBMCommand(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference TBMSpeed2(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Delay25Test(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Prog_DAC(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Prog_DACs(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference SetROCDACsEnMass(xoap::MessageReference msg);
    xoap::MessageReference Prog_Pix(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Cal_Pix(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference ClrCal(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference ResetROCs(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference ResetTBM(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference ClrCalEnMass(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference DisableHitsEnMass(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference EnableFullBufferCheck(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference SetTBMDACsEnMass(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference CalibRunning(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference CalibRunningThreshold(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference fsmStateNotification(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Null (xoap::MessageReference msg) throw (xoap::exception::Exception);

    void stateChanged(toolbox::fsm::FiniteStateMachine &fsm) ;//throw (toolbox::fsm::exception::Exception);
    void stateConfiguring(toolbox::fsm::FiniteStateMachine &fsm) ;//throw (toolbox::fsm::exception::Exception);
    void readBackTBMs();

    bool PhysicsRunning(toolbox::task::WorkLoop *w1);


    void stateFixingSoftError(toolbox::fsm::FiniteStateMachine &fsm) ;

    void transitionHaltedToConfiguring (toolbox::Event::Reference e);// throw (toolbox::fsm::exception::Exception)
    void enteringError(toolbox::Event::Reference e);

    void b2inEvent(toolbox::mem::Reference* msg, xdata::Properties& plist) throw (b2in::nub::exception::Exception);
    
    void DetectSoftError();

  protected:

    void decodeProg_DACs (xoap::SOAPEnvelope& soapEnvelope, xoap::SOAPElement& soapElement, int dacAddress, int mode) throw (xoap::exception::Exception);

    toolbox::fsm::AsynchronousFiniteStateMachine fsm_;
    xdata::String state_;

  toolbox::task::WorkLoop *workloop_;
  toolbox::task::ActionSignature *physicsRunning_;

  private:

    unsigned long crate_;
    std::string htmlbase_,datbase_;
    std::string runNumber_;

    bool doQPLLLoop_;
    bool doTBMReadoutLoop_;

    // change for HAL
    #ifdef USE_HAL
      HAL::CAENLinuxBusAdapter *busAdapter_; // pointer to VME bus
      HAL::VMEAddressTable *addressTablePtr_;
    #else
      CVBoardTypes VMEBoard;
      short Device;
      long aBHandle;
      short Link;
    #endif // USE HAL

    pos::PixelConfigKey *theGlobalKey_;
    pos::PixelNameTranslation *theNameTranslation_;
    pos::PixelDetectorConfig *theDetectorConfiguration_;
    pos::PixelFECConfig *theFECConfiguration_;

    unsigned int theLastGlobalKey_;

    pos::PixelCalibBase *theCalibObject_;
    unsigned int calibStateCounter_;

    std::map<pos::PixelModuleName, pos::PixelMaskBase*> theMasks_;
    std::map<pos::PixelModuleName, pos::PixelTrimBase*> theTrims_;
    std::map<pos::PixelModuleName, pos::PixelDACSettings*> theDACs_;
    std::map<pos::PixelModuleName, pos::PixelTBMSettings*> theTBMs_;

    xdaq::ApplicationDescriptor* PixelDCSFSMInterface_;
    xdaq::ApplicationDescriptor* PixelPSXServer_;
    xdaq::ApplicationDescriptor* PixelSupervisor_;
    std::set<std::string> modulesProgDACced_;
    std::set<std::string> powerCoordinatesRampingUp_;

    //For indexing based on VME address
    typedef std::map<unsigned long, PixelFECInterface*> FECInterfaceMap;
    FECInterfaceMap FECInterface;
    typedef std::map<unsigned long, HAL::VMEDevice*> VMEPtrMap;
    VMEPtrMap VMEPtr_;

    //For indexing by FEC number
    std::map<unsigned int,  pos::PixelFECConfigInterface*> FECInterfaceByFECNumber_;

    //powerMap_ always contains the actual current state of the power supplies
    PixelPowerMap4603 powerMap_;
    //powerMapLast_ stores the way the power supplies were considered the last time the ROCs were programmed
    //if the HV is on but we program the ROCs as if it is off, then we write HV_OFF here
    PixelPowerMap4603 powerMapLast_;

    bool qpllCheck(toolbox::task::WorkLoop * w1);
    toolbox::task::WorkLoop * qpllWorkloop_;
    toolbox::task::ActionSignature * qpllCheck_;
    //       vme address   qpll status
    std::map<unsigned long, int> qpllStatus_;
    //these summarize the info in qpllStatus_
    unsigned int num_qpll_locked_;
    unsigned int num_qpll_unlocked_;
    //qpll runs in its own thread, so let's be careful
    toolbox::BSem* qplllock_;

    void HaltAction();
    std::string HaltFSM();
    void deleteHardware();
    void CleanupConfigurationData();
    void loadCalibObject();

    void startupHVCheck(bool startingRun, bool doReset = false);    

    bool preconfigure_workloop(toolbox::task::WorkLoop * w1);
    bool preConfigureDone_;
    bool detConfigLoaded_;
    PixelTimer totalTimer_;

    toolbox::task::WorkLoop * preconfigureWorkloop_;
    toolbox::task::ActionSignature * preconfigureTask_;
    toolbox::BSem* pclock_;

    toolbox::BSem* phlock_;
    bool workloopContinue_;

    map<string, int> analogInputBiasLast_;
    map<string, int> analogOutputBiasLast_;
    map<string, int> analogOutputGainLast_;

    vector<string> tbmReadbackBadChannels_;

    void DIAG_CONFIGURE_CALLBACK();
    void DIAG_APPLY_CALLBACK();

    /* xgi method called when the link <display_diagsystem> is clicked */
    void callDiagSystemPage(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

};
#endif
