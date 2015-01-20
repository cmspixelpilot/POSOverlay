/* PixelBaselineMonitor.h
 * 
 * Robert Brockman II 
 * Patrick El-Hage 
 * 28/05/2009
 * 
 * This program reads in a web page produced by the PixelFEDMonitor application on 
 * the PixelFEDSupervisor machines.  The web page has information in CSV format
 * which is parsed into fields.  Some of these fields are reassembled into 
 * baseline correction readings:  those readings that are out of range generate
 * errors which are reported to the DiagSystem.
 */

#ifndef _PixelBaselineMonitor_h_
#define _PixelBaselineMonitor_h_

#include <stdlib.h>
#include <math.h>
#include <iostream>

/* curlpp libraries are used to get flashlist from the XDAQ web server into 
 * memory buffers.  Be sure to use curlp-0.6.1, not more recent versions
 * which would require updating the boost libraries from the one currently
 * used in the standard CERN linux distribution.
 */
#define  HAVE_CONFIG_H //Needed for curl library
#include "curlpp/cURLpp.hpp"
#include "curlpp/Easy.hpp"
#include "curlpp/Options.hpp"
#include "curlpp/Exception.hpp"
#undef HAVE_CONFIG_H

/* ROOT libraries for TTrees */
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TBrowser.h"
#include "TH2.h"
#include "TRandom.h"

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


/* Libraries for interfacing with DIAGSYSTEM */
#include "diagbag/DiagBagWizard.h"
#include "DiagCompileOptions.h"
#include "toolbox/convertstring.h"
#include "toolbox/BSem.h"
#include "iomanip"
#include "toolbox/task/Timer.h"
#include "toolbox/task/TimerFactory.h"
#include "toolbox/task/TimerListener.h"
#include "toolbox/TimeInterval.h"

// File I/O
#include <fstream>
// Time 
#include <time.h>

// XDAQ 
#include "xdata/exdr/FixedSizeInputStreamBuffer.h"
#include "xdata/exdr/Serializer.h"
#include "xdata/UnsignedInteger32.h"
#include "xdata/Float.h"
#include "xdata/Table.h"
#include "xdata/Vector.h"

#define REFRESH_DELAY 2

#define MAX_FILE_LENGTH 20000
#define REPORT_HEADER_FIELDS 5

#define MAXURL 255 // Maximum URL length

/* Pixel detector parameters */
//#define CHANNELS 36
//#define FEDS 40
//#define CRATES 3

/* Status codes for the state of a channel */
#define CHANNEL_OK 0
#define CHANNEL_BORDERLINE_OK 1  // OK, but we're not going to tell the DiagSystem until it improves more.
#define CHANNEL_WARN 2  
#define CHANNEL_BORDERLINE_WARN 3 // WARN, but we're not going to tell the DiagSystem until it improves more.
#define CHANNEL_ERROR 4

/* Limits for good baseline corrections */
#define CHANNEL_OK_MAX 95.0
#define CHANNEL_OK_MIN -95.0
#define CHANNEL_BORDERLINE_OK_MAX 100.0
#define CHANNEL_BORDERLINE_OK_MIN -100.0
#define CHANNEL_WARN_MAX 145.0
#define CHANNEL_WARN_MIN -145.0
#define CHANNEL_BORDERLINE_WARN_MAX 150.0
#define CHANNEL_BORDERLINE_WARN_MIN -150.0
#define CHANNEL_NOMINAL 0.0

/* Length of time (in sequence numbers) to wait between writing entries to ROOT */
#define INTERVAL_LENGTH 10

/* Timeout for communications with FED Monitor */
#define CRATE_TIMEOUT 300


class PixelMonitor;
class PixelCrate;
class PixelFED;
class PixelChannel;
class PixelErrorTable;

/* A BaselineCorrectionReading object contains a baseline correction measurement,
 * as well as the current state of the channel (derived from the measurement) 
 * and which FED and channel the measurement is associated with. 
 */
class BaselineCorrectionReading {
private:
  float baselineCorrectionMean;
  float baselineCorrectionStdDev;
  int baselineCorrectionChannelState;
  int baselineCorrectionSquawk;  //for hysteresis on messages

public:
  BaselineCorrectionReading();
  ~BaselineCorrectionReading();
  
  void set_baselineCorrectionMean(float);
  float get_baselineCorrectionMean();
  float * get_baselineCorrectionMean_address();

  void set_baselineCorrectionStdDev(float);
  float get_baselineCorrectionStdDev();
  float * get_baselineCorrectionStdDev_address();

  void set_baselineCorrectionChannelState(int);
  int get_baselineCorrectionChannelState();
  int* get_baselineCorrectionChannelState_address();

  void set_baselineCorrectionSquawk(int);
  int get_baselineCorrectionSquawk();
};



/* PixelMonitor is the XDAQ applet that collects black level monitoring information */
class PixelMonitor: public xdaq::Application, public toolbox::task::TimerListener
{
 public:
  XDAQ_INSTANTIATOR();
  
  PixelMonitor(xdaq::ApplicationStub * s) throw (xdaq::exception::Exception);	

  /* Used for sending error messages to diagnostic system */
  DiagBagWizard  * diagService_; //FIXME:  might need to be private

  void DIAG_CONFIGURE_CALLBACK();
  void DIAG_APPLY_CALLBACK();
  DIAG_FREELCLSEM_CALLBACK();
  DIAG_FREEGLBSEM_CALLBACK();
  DIAG_REQUEST_ENTRYPOINT(); 


  // Activated when timer is triggered. 
  void timeExpired (toolbox::task::TimerEvent& e);

  // Activated when web page is loaded by shifter
  void Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
  
  // Dumps the worst baseline correction values (since the last dump) to a ROOT file.
  // File name is based on the run number.
  void outputToROOT();

  // Outermost flashlist table, contains one row for each crate.
  xdata::Table * flashlistTable; 

  // error flashlist table
  xdata::Table * errTablePtr;
  
 private:
  /* The first time timer event triggers the DIAGSYSTEM inititalization macro.
     All further timer events trigger retrieval and processing of Crates */
  bool timerInitializationMode; 
  int tempInt;

  char * url; // url of flashlist.
  char* flashlistBuffer; // Buffer containing flashlist
  size_t flashlistBufferSize; // flashlist buffer size 
  unsigned int flashlistTimeStamp; // Date flashlist completely processed.


  // Map containing all crate objects for pixel subdetector.
  std::map<unsigned int,PixelCrate *> crateMap;
  
  PixelErrorTable * myErrTable;
  
  unsigned int  lastROOTOutputTimeStamp;

  void* Realloc(void* ptr, size_t size); // calls malloc or realloc as needed.
 
  // Copies crate report from curlpp buffer into flashlistBuffer.
  size_t WriteMemoryCallback(char* ptr, size_t size, size_t nmemb);

  // Gets new flashlist data from flashlist url.
  void loadFlashlist(); 
  
  // Distributes contents of flashlist to crate, fed, and channel objects.
  void parseFlashlist();


};


class PixelCrate
{
 public:

  //Constructor
  PixelCrate(unsigned int, PixelMonitor *);

  unsigned int crateNumber;
  unsigned int runNumber;
  PixelMonitor * parentMonitorPtr;

  // Inner flashlist table containing data for crate indexed by FED.
  xdata::Table * crateTablePtr;

  // Map containing FED objects.
  std::map<unsigned int,PixelFED *> fedMap; 

  // Min, max, and average baseline correction values for crate.
  float crateBCMaxVal;
  int crateBCMaxFed;
  int crateBCMaxChannel;
  float crateBCMinVal;
  int crateBCMinFed;
  int crateBCMinChannel;
  float crateBCAvgVal;

  // Number of channels in each state.
  int crateOkChannels;
  int crateWarnChannels;
  int crateErrorChannels;

  unsigned int crateTimeStamp;

  void parseCrateRow(unsigned int);
};

class PixelFED
{
 public:

  //Constructor
  PixelFED(unsigned int, PixelCrate *);

  unsigned int fedNumber;
  PixelCrate * parentCratePtr;

  // Inner flashlist table containing data for fed indexed by channel.
  xdata::Table * fedTablePtr;

  // Map containing channel objects.
  std::map<unsigned int,PixelChannel *> channelMap; 

  // Min, max, and average baseline correction values for FED.
  float fedBCMaxVal;
  int fedBCMaxChannel;
  float fedBCMinVal;
  int fedBCMinChannel;
  float fedBCAvgVal;

  // Number of channels in each state.
  int fedOkChannels;
  int fedWarnChannels;
  int fedErrorChannels;

  void parseFEDRow(unsigned int);
};

class PixelChannel
{
 public:
  //Constructor
  PixelChannel(unsigned int, PixelFED *);

  unsigned int channelNumber;
  PixelFED * parentFEDPtr;

  BaselineCorrectionReading newBCReading;
  BaselineCorrectionReading currentBCReading;
  BaselineCorrectionReading worstBCReading;

  float getBCMean();
  void parseChannelRow(unsigned int);
  void processNewBaselineCorrectionReading();

};

class PixelErrorTable
{
 public:
  //Constructor
  PixelErrorTable() { 
    errTableCounter = 0;
    errTableTimeStamp = 0;
    errTableTimeStamp_100 = 0;
    //std::cout << " errTableCounter = " << errTableCounter << std::endl; 
    std::cout << "PixelErrorTable Constructor entered" << std::endl; 
  }
  
  int errTableCounter;
  int errTableTimeStamp;
  int errTableTimeStamp_100;
  
  struct FEDErrors{
    unsigned int ROCerr;
    unsigned int OOSerr;
    unsigned int TimeOUTerr;		     
  };
    
  std::map<unsigned int, std::map<unsigned int, FEDErrors> > errTableMap; 
  std::map<unsigned int, std::map<unsigned int, FEDErrors> > errTableMap_;
  
  void parseErrorTableCrate(xdata::Table *, int TimeStamp, int CrateNumber);
  void getMAXErrors(unsigned int FEDNumber, int &MaxROCErr, int &MaxROCCh, int &MaxOOSErr, int &MaxOOSCh, int &MaxTOErr, int &MaxTOCh);  
  void getMAXIncreaseErrors(unsigned int FEDNumber, int &MaxROCErr, int &MaxROCCh, int &MaxOOSErr, int &MaxOOSCh, int &MaxTOErr, int &MaxTOCh);  
  void printErrTableMap();
};
#endif
