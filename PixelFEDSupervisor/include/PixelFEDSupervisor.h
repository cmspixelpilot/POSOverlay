/**************************************************************************
 * XDAQ Components for Pixel Online Software                              *
 * Copyright (C) 2007, Cornell University		                  *
 * All rights reserved.                                                   *
 * Authors: Souvik Das, Anders Ryd, Karl Ecklund			  *
  *************************************************************************/

#ifndef _PixelFEDSupervisor_h_
#define _PixelFEDSupervisor_h_

#include "xdaq/Application.h"
#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationStub.h"
#include "xdaq/ApplicationStubImpl.h"
#include "xdaq/exception/Exception.h"
#include "xdaq/ApplicationGroup.h"
#include "xdaq/NamespaceURI.h"

#include "xdata/UnsignedLong.h"
#include "xdata/String.h"
#include "b2in/nub/exception/Exception.h"

// Includes for DOM and SOAP parsing
#include "xoap/MessageReference.h"
#include "xoap/MessageFactory.h"
#include "xoap/SOAPEnvelope.h"
#include "xoap/SOAPMessage.h"
#include "xoap/SOAPBody.h"
#include "xoap/SOAPPart.h"
#include "xoap/domutils.h"
#include "xoap/Method.h"
#include "xoap/DOMParser.h"

// Includes for the GUI
#include "xdaq/WebApplication.h"
#include "xgi/Method.h"
#include "xgi/Utils.h"
#include "cgicc/HTMLClasses.h"
#include "PixelUtilities/PixelGUIUtilities/include/HTML2XGI.h"

#include "toolbox/fsm/AsynchronousFiniteStateMachine.h"
#include "toolbox/fsm/FailedEvent.h"
#include "toolbox/exception/Handler.h"
#include "toolbox/Event.h"

#include "toolbox/task/WorkLoopFactory.h"
#include "toolbox/task/WaitingWorkLoop.h"
#include "toolbox/task/Action.h"
#include "toolbox/lang/Class.h"
#include "toolbox/net/URN.h"
#include "toolbox/BSem.h"


#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"


//PixelFEDSpySupervisor code
#include "PixelUtilities/PixelSharedDataTools/include/SharedObject.h"
#include "PixelUtilities/PixelSharedDataTools/include/SharedObjectOwner.h"
#include "PixelUtilities/PixelSharedDataTools/include/PixelErrorCollection.h"

// used for monitoring
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"

//#define VMEDUMMY
//

//#define RUBUILDER

#include "VMEAddressTable.hh"
#include "VMEAddressTableXMLFileReader.hh"
#include "VMEAddressTableASCIIReader.hh"

#ifdef VMEDUMMY
#include "VMEDummyBusAdapter.hh"
#else
#include "CAENLinuxBusAdapter.hh"
#endif

//#include "CAENVMElib.h"  // CAEN library prototypes  NOT needed anymore? kme 10/05/06
#include "VMEDevice.hh" 

#include "PixelSupervisorConfiguration/include/PixelFEDSupervisorConfiguration.h" 
#include "PixelFEDInterface/include/PixelFEDInterface.h" 
#include "PixelFEDInterface/include/PixelFEDFifoData.h"
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
#include "CalibFormats/SiPixelObjects/interface/PixelDetectorConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelConfigFile.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibBase.h"

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h" // added by Souvik Das on 4/19/06
//#include "PixelUtilities/PixelFEDDataTools/include/Converter.h"
//#include "PixelUtilities/PixelFEDDataTools/include/LevelEncoderDecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/TestDACTools.h"
#include "PixelUtilities/PixelFEDDataTools/include/DBEmulator.h"

#include "PixelUtilities/PixelI2OUtilities/include/I2OSender.h"
#include "i2o/utils/AddressMap.h"

#include "xdata/Properties.h"


#ifdef RUBUILDER
#include "RUBuilderExample/RUBController/include/RUBController.h"
#include "PixelUtilities/PixelI2OUtilities/include/I2OSender.h"
#include "PixelUtilities/PixelI2OUtilities/include/I2OReceiver.h"
#endif

// gio
#include "diagbag/DiagBagWizard.h"
#include "DiagCompileOptions.h"
//

#include "toolbox/task/Timer.h"
#include "toolbox/task/TimerFactory.h"
#include "toolbox/task/TimerListener.h"
#include "toolbox/TimeInterval.h"

// BEGIN - PixelFEDMonitor: Robert

// Necessary for use of PixelFEDMonitor infospace
#include "xdata/InfoSpace.h"
#include "xdata/ActionListener.h"
#include "xdata/InfoSpaceFactory.h"
#include "xdata/ItemEvent.h"
#include "xdata/ItemGroupEvent.h"
#include "xdata/UnsignedInteger32.h"
#include "xdata/Float.h"
#include "xdata/Table.h"

// END - PixelFEDMonitor: Robert



class PixelFEDSupervisor: public xdaq::Application, public SOAPCommander, public toolbox::task::TimerListener, public PixelFEDSupervisorConfiguration
{
  public:
    static pixel::SharedObjectOwner<pixel::PixelErrorCollection> ErrorCollectionDataOwner;//Ships error data out
    // gio
    toolbox::BSem executeReconfMethodMutex;
    //

    XDAQ_INSTANTIATOR();

    PixelFEDSupervisor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);
    ~PixelFEDSupervisor();

    //gio
    void timeExpired (toolbox::task::TimerEvent& e);

    //
    void Default(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void StateMachineXgiHandler(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void LowLevelCommands(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);
    void LowLevelXgiHandler(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception); 

    xoap::MessageReference Initialize (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Configure (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Start (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Stop (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Pause (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Resume (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Halt (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference PrepareTTSTestMode (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference TestTTS (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Reset (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference Recover (xoap::MessageReference msg) ;

    xoap::MessageReference Reconfigure (xoap::MessageReference msg) ;

    xoap::MessageReference FSMStateRequest (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference SetChannelOffsets (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference ReloadFirmware (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference ResetFEDs (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference FillTestDAC (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference EnableFIFO3 (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference VMETrigger (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference BaselineRelease (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference BaselineSet (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference BaselineHold (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference BaselineMonitor (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference ReadFIFO (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference ReadErrorFIFO (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference ReadTTSFIFO (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference ReadLastDACFIFO (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference ReadDataAndErrorFIFO (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference SetADC1V2VEnMass (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference SetADC1V2VOneChannel (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference SetFEDOffsetsEnMass (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference SetPrivateWord (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference ResetFEDsEnMass (xoap::MessageReference msg) throw (xoap::exception::Exception);

    //xoap::MessageReference ThresholdCalDelay (xoap::MessageReference msg) throw (xoap::exception::Exception);

    xoap::MessageReference SetPhasesDelays (xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference SetControlRegister(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference FEDCalibrations(xoap::MessageReference msg) throw (xoap::exception::Exception);

    xoap::MessageReference beginCalibration(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference endCalibration(xoap::MessageReference msg) throw (xoap::exception::Exception);
    xoap::MessageReference calibrationEvent(xoap::MessageReference msg) throw (xoap::exception::Exception);

    //Soft Error Stuff
	void DetectSoftError();
    xoap::MessageReference FixSoftError(xoap::MessageReference msg);
    xoap::MessageReference ResumeFromSoftError(xoap::MessageReference msg);

    void b2inEvent(toolbox::mem::Reference* msg, xdata::Properties& plist) throw (b2in::nub::exception::Exception);


    void stateChanged(toolbox::fsm::FiniteStateMachine &fsm) ;//throw (toolbox::fsm::exception::Exception);
    void stateConfiguring(toolbox::fsm::FiniteStateMachine &fsm) ;//throw (toolbox::fsm::exception::Exception);
    bool job_Configure();

    void stateFixingSoftError(toolbox::fsm::FiniteStateMachine &fsm) ;

    xoap::MessageReference MakeSOAPMessageReference_readLastDACFIFO(const char* tagName);

    bool ReadLastDACFIFO_workloop_SOAP(toolbox::task::WorkLoop *w1);
    bool ReadLastDACFIFO_workloop_I2O(toolbox::task::WorkLoop *w1);
    bool PhysicsRunning(toolbox::task::WorkLoop *w1);

    void callback_TA_CREDIT(toolbox::mem::Reference *ref) throw(i2o::exception::Exception);


  protected:

    // toolbox::fsm::FiniteStateMachine fsm_; // the actual state machine
    toolbox::fsm::AsynchronousFiniteStateMachine fsm_;
    xdata::String state_; // used to reflect the current state to the outside world

    void asynchronousExceptionNotification(xcept::Exception& e){}

  private:

    void createFEDVMEAccess();
    void deleteHardware();

    // Keep track of when we've sent SOAP messages in physics workloop
    // so we don't send them multiple times
    bool physicsRunningSentRunningDegraded, physicsRunningSentSoftErrorDetected;
		

    #ifdef VMEDUMMY
      HAL::VMEDummyBusAdapter *busAdapter_;
    #else
      HAL::CAENLinuxBusAdapter *busAdapter_; // pointer to VME bus
    #endif

    /**
    * A pointer to the addressTable.
    * The AddressTable is created dynamically during the
    * configuration of the application, since the path to
    * the file containing the table is a property (=parameter) of
    * the XDAQ application. Therefore the VMEAddressTable cannot
    * be instantiated in the constructor of this class.
    */
    HAL::VMEAddressTable *addressTablePtr_;

    std::map<unsigned short, FILE*> dataFile_;
    std::map<unsigned short, FILE*> errorFile_;
    std::map<unsigned short, FILE*> ttsFile_;

    std::map<unsigned long, FILE*> fout_;    // C I/O
    std::map<unsigned long,bool> fileopen_;
    std::map<unsigned long, FILE *> errorFile2_;
    std::map<unsigned long, bool> errBufferOpen_;
 
    std::map<unsigned short, FILE*> lastDacFile_;


    #ifdef RUBUILDER
      xdaq::ApplicationDescriptor* RU_; // Pointers to the RUs
    #endif
    xdaq::ApplicationDescriptor* PixelDCStoFEDDpInterface_;
    xdaq::ApplicationDescriptor* PixelSupervisor_;

    std::string htmlbase_,datbase_;
    std::string runType_;
    int eventNumber_;
    int countLoopsThisRun_;

    toolbox::BSem m_lock;
    int  m_credits;

    std::string runNumber_;
    std::string outputDir_;

    PixelFEDCalibrationBase* theFEDCalibrationBase_;

#ifdef RUBUILDER
    RUBController m_controller;
#endif

    // BEGIN - PixelMonitor: Robert

    // Infospace for moving information to PixelMonitor application
    xdata::InfoSpace * monitorInfoSpace;
   
    // Crate variables for infospace.
    xdata::Table * crateTablePtr;
    xdata::UnsignedInteger32 * crateNumberPtr;
    xdata::UnsignedInteger32 * runNumberPtr;
    xdata::UnsignedInteger32 * crateTimeStampPtr;

    // FED variables for crate table.
    xdata::UnsignedInteger32 * fedNumberPtr;
    xdata::Table * fedTablePtr;

    // Channel variables for fed table.
    xdata::UnsignedInteger32 * channelNumberPtr;
    xdata::Float * baselineCorrectionMeanPtr;
    xdata::Float * baselineCorrectionStdDevPtr;
    
    // Variables for err table
    xdata::Table * errTable;
    xdata::UnsignedInteger32 * errTableTimeStampPtr;
    xdata::UnsignedInteger32 * numNORErrors;
    xdata::UnsignedInteger32 * numOOSErrors;
    xdata::UnsignedInteger32 * numTimeOutErrors;
    
    // Maps of baseline correction information.
    std::map<unsigned int, std::map<unsigned int, pair<float,float> > > baselineCorrectionMap; 

    // Maps of error count
    std::map<unsigned int, std::map<unsigned int, unsigned int> > errorCountMap;  // [fed][chan] count;  chan=0 is all channels

    // Maps of FIFO status & Link Full Flag
    std::map<unsigned int, Moments > lffMap; // [fed] fraction_ON
    std::map<unsigned int,  std::map< unsigned short, Moments > > fifoStatusMap; // [fed][bit] fraction_ON

    // Maps of TTS status
    std::map<unsigned int, uint32_t > ttsMap; // [fed] status
    std::map<unsigned int, unsigned int > ttsStateChangeCounterMap; // [fed] number of changes

    // Maps of TTCrx status
    std::map<unsigned int, unsigned int> NttcrxResets; // [fed] number of autoresets detected by watch dog in status register
    std::map<unsigned int, int> NttcrxSBError; // [fed] number of single-bit errors detected by TTCrx
    std::map<unsigned int, int> NttcrxDBError; // [fed] number of double-bit errors detected by TTCrx
    std::map<unsigned int, int> NttcrxSEU; // [fed] number of seus detected by TTCrx

    // Map of FPGA PLL lock counter
    std::map<unsigned int, unsigned int> NlockNorth; // [fed] number of PLL locks by North FPGA

    // END - PixelMonitor: Robert

    //new data to keep track of whether a FED is stuck in BUSY
    //        fed #        iterations stuck
    std::map<unsigned int, unsigned int> fedStuckInBusy_;

    void DIAG_CONFIGURE_CALLBACK();
    void DIAG_APPLY_CALLBACK();

    /* xgi method called when the link <display_diagsystem> is clicked */
    void callDiagSystemPage(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

    void closeOutputFiles();
    void reportStatistics();
    void EndOfRunFEDReset();

    toolbox::BSem* phlock_;
    bool workloopContinue_;

};

#endif
