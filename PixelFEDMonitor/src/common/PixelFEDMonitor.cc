/* PixelFEDMonitor.cc
 * 
 * Robert Brockman II 
 * 24/06/2009
 * 
 */

#include "PixelFEDMonitor.h"

using namespace std;

//
// provides factory method for instantion of PixelFEDMonitor application
//
XDAQ_INSTANTIATOR_IMPL(PixelFEDMonitor)
  PixelFEDMonitor::PixelFEDMonitor(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception):xdaq::Application(s)
  
{	
  /* Set up communications with DIAGSYSTEM, mostly cargo cult code */
  diagService_ = new DiagBagWizard(
                                   ("ReconfigurationModule") ,
                                   this->getApplicationLogger(),
                                   getApplicationDescriptor()->getClassName(),
                                   getApplicationDescriptor()->getInstance(),
                                   getApplicationDescriptor()->getLocalId(),
                                   (xdaq::WebApplication *)this,
                                   "SYSTEM IS PIXEL",
                                   "SUBSYSTEM IS FEDMONITOR"
                                   );

  diagService_->reportError("The DiagSystem is installed --- this is a bogus error message",DIAGUSERINFO);
  
  xgi::bind(this,&PixelFEDMonitor::configureDiagSystem, "configureDiagSystem");
  xgi::bind(this,&PixelFEDMonitor::applyConfigureDiagSystem, "applyConfigureDiagSystem");
  
  xoap::bind(this,&PixelFEDMonitor::freeLclDiagSemaphore, "freeLclDiagSemaphore", XDAQ_NS_URI );
  xoap::bind(this,&PixelFEDMonitor::freeGlbDiagSemaphore, "freeGlbDiagSemaphore", XDAQ_NS_URI );
  xoap::bind(this,&PixelFEDMonitor::processOnlineDiagRequest, "processOnlineDiagRequest", XDAQ_NS_URI ); 
  
  DIAG_DECLARE_USER_APP
    std::stringstream timerName;
  timerName << getApplicationDescriptor()->getContextDescriptor()->getURL() << ":";
  timerName << getApplicationDescriptor()->getClassName() << ":" << getApplicationDescriptor()->getLocalId() << ":" << getApplicationDescriptor()->getInstance();
  toolbox::task::Timer * timer = toolbox::task::getTimerFactory()->createTimer(timerName.str());
  
  toolbox::TimeInterval interval(AUTO_UP_CONFIGURE_DELAY,0);
  toolbox::TimeVal start;
  start = toolbox::TimeVal::gettimeofday() + interval;
  timer->schedule( this, start,  0, "" );
  
  // Initialize infospace if necessary 
  // FIXME deallocate memory in destructor later! Memory shared with FED Supervisor,
  // don't deallocate twice!
  monitorInfoSpace = xdata::getInfoSpaceFactory()->get("PixelFEDInfospace");
  
  monitorInfoSpace->lock();
  try 
    {
      // Maybe FED Supervisor started before FED Monitor, try to get infospace variables.
      crateNumberPtr =
	dynamic_cast<xdata::Integer *>(monitorInfoSpace->find("crateNumber"));
      runNumberPtr =
	dynamic_cast<xdata::Integer *>(monitorInfoSpace->find("runNumber"));
      heartbeatTimeStampPtr =
	dynamic_cast<xdata::UnsignedInteger32 *>(monitorInfoSpace->find("heartbeatTimeStamp"));
      baselineCorrectionFEDNumberPtr =
	dynamic_cast<xdata::Integer *>(monitorInfoSpace->find("baselineCorrectionFEDNumber"));
      baselineCorrectionChannelNumberPtr =
	dynamic_cast<xdata::Integer *>(monitorInfoSpace->find("baselineCorrectionChannelNumber"));
      baselineCorrectionMeanPtr =
	dynamic_cast<xdata::Float *>(monitorInfoSpace->find("baselineCorrectionMean"));
      baselineCorrectionStdDevPtr =
	dynamic_cast<xdata::Float *>(monitorInfoSpace->find("baselineCorrectionStdDev"));
    }  
  catch (xdata::exception::Exception& e)
    {
      // FED Monitor has started before FED Supervisor, create infospace variables.
      crateNumberPtr = new xdata::Integer();
      runNumberPtr = new xdata::Integer();
      heartbeatTimeStampPtr = new xdata::UnsignedInteger32();
      baselineCorrectionFEDNumberPtr = new xdata::Integer();
      baselineCorrectionChannelNumberPtr = new xdata::Integer();
      baselineCorrectionMeanPtr = new xdata::Float();
      baselineCorrectionStdDevPtr = new xdata::Float();

      // Publish Infospace values.
      monitorInfoSpace->fireItemAvailable("crateNumber", crateNumberPtr);
      monitorInfoSpace->fireItemAvailable("runNumber", runNumberPtr);
      monitorInfoSpace->fireItemAvailable("heartbeatTimeStamp", heartbeatTimeStampPtr);
      monitorInfoSpace->fireItemAvailable("baselineCorrectionFEDNumber", baselineCorrectionFEDNumberPtr);
      monitorInfoSpace->fireItemAvailable("baselineCorrectionChannelNumber", baselineCorrectionChannelNumberPtr);
      monitorInfoSpace->fireItemAvailable("baselineCorrectionMean", baselineCorrectionMeanPtr);
      monitorInfoSpace->fireItemAvailable("baselineCorrectionStdDev", baselineCorrectionStdDevPtr);
      
      // Initialize Infospace values.
      * crateNumberPtr = -1; 
      * runNumberPtr = -1; 
      * heartbeatTimeStampPtr = 0;
      * baselineCorrectionFEDNumberPtr = -1; // No baseline corrections published yet.
      * baselineCorrectionChannelNumberPtr = -1; // No baseline corrections published yet.
      * baselineCorrectionMeanPtr = 0.0; // No baseline corrections published yet.
    }

  // Attach a listener to the infospace.  FIXME: Detach in destructor?
  monitorInfoSpace->addItemChangedListener("baselineCorrectionMean",this);

  monitorInfoSpace->unlock();
  
  /* Set page refresh to trigger Default method */
  xgi::bind(this,&PixelFEDMonitor::Default, "Default");
}

void PixelFEDMonitor::timeExpired (toolbox::task::TimerEvent& e)
{
  /* Used for interfaceing with DIAGSYSTEM, cargo cult code run once */
  DIAG_EXEC_FSM_INIT_TRANS	
}

/* Activated when the PixelSupervisor reads the web page, this method 
 * gets baseline correction values out of the map and puts them in the web
 * page in CSV format. */
void PixelFEDMonitor::Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  monitorInfoSpace->lock();
  
  // Output application signature to web page.  This allows us to stop
  // HTTP 1.1 400 Bad Request errors from crashing the PixelMonitor
  *out << "PixelMonitor" << endl;

  // Output crate information to web page.
  *out << * crateNumberPtr << endl;
  *out << * runNumberPtr << endl;
  *out << * heartbeatTimeStampPtr << endl;

  *out << baselineCorrectionTimeStamp << endl;

  // Output baseline correction map to web page.
  map<int,map<int, pair<float,float> > >::iterator iterFed = 
    baselineCorrectionMap.begin();
  while (iterFed != baselineCorrectionMap.end())
    {
      map<int,pair<float,float> >::iterator iterChannel = 
	iterFed->second.begin();
      while (iterChannel != iterFed->second.end())
	{
	  // Output Fed Index to web page
	  *out << iterFed->first << ",";
	  // Output Channel Index to web page
	  *out << iterChannel->first << ",";
	  // Output baseline correction mean to web page
	  *out << iterChannel->second.first << ",";
	  // Output baseline correction standard deviation to web page
	  *out << iterChannel->second.second << endl;
	 
	  
	  iterChannel++;	
	}
      iterFed++;
    }
  
  monitorInfoSpace->unlock();
}
    
    

// Called back when an InfoSpace event occurs.  Triggered when a baseline
// correction value is ready to be placed in the map.
void PixelFEDMonitor::actionPerformed(xdata::Event & received )
{
  // May be used later to differentiate between event types
  // xdata::ItemEvent& e = dynamic_cast<xdata::ItemEvent&>(received);

  baselineCorrectionTimeStamp = time(NULL);

  monitorInfoSpace->lock();

  //FIXME: add for production
  // if ((unsigned int)(* baselineCorrectionTimeStampPtr) > 0)
    {
      // A new baseline correction may be available, add it to the map.
      int fedNumber = * baselineCorrectionFEDNumberPtr;
      int channelNumber = * baselineCorrectionChannelNumberPtr;
    
      baselineCorrectionMap[fedNumber][channelNumber] =
	make_pair((float)(* baselineCorrectionMeanPtr),(float)(* baselineCorrectionStdDevPtr));
    }

  monitorInfoSpace->unlock();
}

