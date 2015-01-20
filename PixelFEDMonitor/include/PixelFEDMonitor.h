/* PixelFEDMonitor.h
 * 
 * Robert Brockman II 
 * 24/06/2009
 * 
 */

// FIXME:BAD
using namespace std;

#ifndef _PixelFEDMonitor_h_
#define _PixelFEDMonitor_h_

#include <stdlib.h>
#include <math.h>
#include <iostream>


/* Libraries for acting as a XDAQ web applet */ 
#include "xdaq/WebApplication.h"
//#include "xgi/Utils.h"
#include "xgi/Method.h"
//#include "cgicc/CgiDefs.h"
//#include "cgicc/Cgicc.h"
//#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

#include "xdaq/Application.h"
#include "xdaq/ApplicationGroup.h"

#include "xdaq/ApplicationContext.h"
#include "xdaq/ApplicationStub.h"

/* Infospace tools */
#include "xdata/InfoSpace.h"
#include "xdata/ActionListener.h"
#include "xdata/InfoSpaceFactory.h"
#include "xdata/ItemEvent.h"
#include "xdata/ItemGroupEvent.h"
#include "xdata/UnsignedInteger32.h"
#include "xdata/Float.h"
#include "xdata/Vector.h"
#include "xdata/ItemEvent.h"


/* Libraries for interfacing with DIAGSYSTEM */
#include "diagbag/DiagBagWizard.h"
#include "DiagCompileOptions.h"
#include "toolbox/convertstring.h"
#include "toolbox/BSem.h"


/* Timing loop tools */
#include "iomanip"
#include "toolbox/task/Timer.h"
#include "toolbox/task/TimerFactory.h"
#include "toolbox/task/TimerListener.h"
#include "toolbox/TimeInterval.h"


/* PixelFEDMonitor is the XDAQ application that collects black level monitoring 
 * information for a particular crate from a FED Supervisor application, creating
 * a CSV formatted web page. */
class PixelFEDMonitor: public xdaq::Application, public toolbox::task::TimerListener,
		        public xdata::ActionListener
{
 public:
  XDAQ_INSTANTIATOR();

  PixelFEDMonitor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);	

  /* Used for sending error messages to diagnostic system */
  DiagBagWizard  * diagService_; //FIXME:  might need to be private

  void DIAG_CONFIGURE_CALLBACK();
  void DIAG_APPLY_CALLBACK();
  DIAG_FREELCLSEM_CALLBACK();
  DIAG_FREEGLBSEM_CALLBACK();
  DIAG_REQUEST_ENTRYPOINT(); 

  /* Activated when timer is triggered. */ 
  void timeExpired (toolbox::task::TimerEvent& e);

  /* Activated when the PixelSupervisor reads the web page, this method 
   * gets black level values out of the infospace and puts them in the web
   * page in CSV format. */
  void Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

  // Called back when an InfoSpace event occurs.  Triggered when a baseline
  // correction value is ready to be placed in the map.
  void actionPerformed(xdata::Event & received );

 private:
  
  // Infospace used to collect information from FED Supervisor.
  xdata::InfoSpace * monitorInfoSpace;
  xdata::Integer * crateNumberPtr;
  xdata::Integer * runNumberPtr;
  xdata::UnsignedInteger32 * heartbeatTimeStampPtr;

  // Channel BC values are passed to the FED monitor one at a time,
  // each is copied into the map as it arrives.
  xdata::Integer * baselineCorrectionFEDNumberPtr;
  xdata::Integer * baselineCorrectionChannelNumberPtr;
  xdata::Float * baselineCorrectionMeanPtr;
  xdata::Float * baselineCorrectionStdDevPtr;

  // Contains time when most recent BC was received.
  unsigned int baselineCorrectionTimeStamp;

  // Collects baseline corrections from all channels on this crate.
  // Contents are updated when infospace changes and outputed by web page refresh
  // First int is fed index, second int is channel index.
  // First float of pair is BC mean, second is BC standard deviation.
  map<int,map<int,pair<float,float> > > baselineCorrectionMap;

};
#endif
