/* PixelMonitor.cc
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

#include "PixelMonitor.h"

using namespace std;

BaselineCorrectionReading::BaselineCorrectionReading(){
  baselineCorrectionMean = 0.0;
  baselineCorrectionStdDev = 0.0;
  baselineCorrectionChannelState = CHANNEL_OK;
  baselineCorrectionSquawk = CHANNEL_OK;
};
BaselineCorrectionReading::~BaselineCorrectionReading(){};

float * BaselineCorrectionReading::get_baselineCorrectionMean_address(){return &baselineCorrectionMean;}
float * BaselineCorrectionReading::get_baselineCorrectionStdDev_address(){return &baselineCorrectionStdDev;}
int* BaselineCorrectionReading::get_baselineCorrectionChannelState_address(){return &baselineCorrectionChannelState;}

float BaselineCorrectionReading::get_baselineCorrectionMean(){return baselineCorrectionMean;}
float BaselineCorrectionReading::get_baselineCorrectionStdDev(){return baselineCorrectionStdDev;}
int BaselineCorrectionReading::get_baselineCorrectionChannelState(){return baselineCorrectionChannelState;}
int BaselineCorrectionReading::get_baselineCorrectionSquawk(){return baselineCorrectionSquawk;}

void BaselineCorrectionReading::set_baselineCorrectionMean(float v)
{
  baselineCorrectionMean = v;
  /* Set channel status based on baselineCorrectionMean */
  if ((baselineCorrectionMean >= CHANNEL_OK_MIN) && (baselineCorrectionMean <= CHANNEL_OK_MAX))
    set_baselineCorrectionChannelState(CHANNEL_OK);
  else if ((baselineCorrectionMean > CHANNEL_OK_MAX) && (baselineCorrectionMean <= CHANNEL_BORDERLINE_OK_MAX)) 
    set_baselineCorrectionChannelState(CHANNEL_BORDERLINE_OK);
  else if ((baselineCorrectionMean < CHANNEL_OK_MIN) && (baselineCorrectionMean >= CHANNEL_BORDERLINE_OK_MIN))
    set_baselineCorrectionChannelState(CHANNEL_BORDERLINE_OK);
  else if ((baselineCorrectionMean > CHANNEL_BORDERLINE_OK_MAX) && (baselineCorrectionMean <= CHANNEL_WARN_MAX)) 
    set_baselineCorrectionChannelState(CHANNEL_WARN);
  else if ((baselineCorrectionMean < CHANNEL_BORDERLINE_OK_MIN) && (baselineCorrectionMean >= CHANNEL_WARN_MIN)) 
    set_baselineCorrectionChannelState(CHANNEL_WARN);
  else if ((baselineCorrectionMean > CHANNEL_WARN_MAX) && (baselineCorrectionMean <= CHANNEL_BORDERLINE_WARN_MAX)) 
    set_baselineCorrectionChannelState(CHANNEL_BORDERLINE_WARN);
  else if ((baselineCorrectionMean < CHANNEL_WARN_MIN) && (baselineCorrectionMean >= CHANNEL_BORDERLINE_WARN_MIN))
    set_baselineCorrectionChannelState(CHANNEL_BORDERLINE_WARN);
  else set_baselineCorrectionChannelState(CHANNEL_ERROR);
}
void BaselineCorrectionReading::set_baselineCorrectionStdDev(float v){baselineCorrectionStdDev = v;}
void BaselineCorrectionReading::set_baselineCorrectionChannelState(int s){baselineCorrectionChannelState = s;}
void BaselineCorrectionReading::set_baselineCorrectionSquawk(int sq) {baselineCorrectionSquawk = sq;}
//
// provides factory method for instantion of PixelMonitor XDAQ web application
//
XDAQ_INSTANTIATOR_IMPL(PixelMonitor)
  PixelMonitor::PixelMonitor(xdaq::ApplicationStub * s)
  throw (xdaq::exception::Exception):xdaq::Application(s)

{	
  /* The first time timer event triggers the DIAGSYSTEM inititalization macro.
     All further timer events trigger retrieval and processing of flashlists */
  timerInitializationMode = true;
  myErrTable = new PixelErrorTable();
  
  /* Set up communications with DIAGSYSTEM, cargo cult code */
  diagService_ = new DiagBagWizard(
                                   ("ReconfigurationModule") ,
                                   this->getApplicationLogger(),
                                   getApplicationDescriptor()->getClassName(),
                                   getApplicationDescriptor()->getInstance(),
                                   getApplicationDescriptor()->getLocalId(),
                                   (xdaq::WebApplication *)this,
                                   "SYSTEM IS PIXEL",
                                   "SUBSYSTEM IS BASELINEMONITOR"
                                   );


  diagService_->reportError("The DiagSystem for PixelMonitor is installed",DIAGINFO);
  
  xgi::bind(this,&PixelMonitor::configureDiagSystem, "configureDiagSystem");
  xgi::bind(this,&PixelMonitor::applyConfigureDiagSystem, "applyConfigureDiagSystem");
  
  xoap::bind(this,&PixelMonitor::freeLclDiagSemaphore, "freeLclDiagSemaphore", XDAQ_NS_URI );
  xoap::bind(this,&PixelMonitor::freeGlbDiagSemaphore, "freeGlbDiagSemaphore", XDAQ_NS_URI );
  xoap::bind(this,&PixelMonitor::processOnlineDiagRequest, "processOnlineDiagRequest", XDAQ_NS_URI ); 
  
  DIAG_DECLARE_USER_APP

      std::stringstream timerName;
  timerName << getApplicationDescriptor()->getContextDescriptor()->getURL() << ":";
  timerName << getApplicationDescriptor()->getClassName() << ":" << getApplicationDescriptor()->getLocalId() << ":" << getApplicationDescriptor()->getInstance();
  toolbox::task::Timer * timer = toolbox::task::getTimerFactory()->createTimer(timerName.str());
  toolbox::TimeInterval startDelay(AUTO_UP_CONFIGURE_DELAY,0);
  toolbox::TimeInterval interval;
  interval.fromString("00:00:00:00:01");
  toolbox::TimeVal startTime;    
  startTime = toolbox::TimeVal::gettimeofday() + startDelay;
  timer->scheduleAtFixedRate(startTime, this, interval , 0, "flashlistloop" ); 

  // Process configuration file to get URL of flashlist
  url = (char *)malloc(sizeof(char) * MAXURL);

  std::string ConfigFileName;
  std::stringstream ConfigFileNameStream;
  ConfigFileNameStream << "/nfshome0/pixelpro/XDAQConfigurations/XDAQConfiguration/PixelMonitor.set";
  ConfigFileName = ConfigFileNameStream.str();
  ifstream fin;
  fin.open(ConfigFileName.c_str(), ios::in);
	       
  std::cout << ConfigFileName.c_str() << std::endl;
  // Ensure configuration file opened successfully
  if (fin.good()) 
    {
      // FIXME: insufficient error checking.
      fin.getline(url,MAXURL);
    }  
  fin.close();

  std::cout << "Flashlist:" << url << std::endl; 

  flashlistBuffer = (char*) malloc(MAX_FILE_LENGTH * sizeof(char));
  flashlistBufferSize = 0;

  flashlistTable = new xdata::Table();

  // Process first crate reports.
  loadFlashlist();
  parseFlashlist(); 

  //FIXME: Add initial ROOT entries.
  lastROOTOutputTimeStamp = time(NULL);

  /* use me in parseFlashlist
  map<int,CrateRecord*>::iterator iterCrate = crateRecordMap.begin();
  while (iterCrate != crateRecordMap.end())
    {
      cout << "itercrate " << iterCrate->first << endl;
      iterCrate->second->processReport();
      iterCrate++;
    }
  */

  // Allow users to access web page.
  xgi::bind(this,&PixelMonitor::Default, "Default");
}


// Timer routine which processes crate reports.
void PixelMonitor::timeExpired (toolbox::task::TimerEvent& e)
{
  if (timerInitializationMode == true)
    {
      // Used for interfaceing with DIAGSYSTEM, cargo cult code run once.
      DIAG_EXEC_FSM_INIT_TRANS	
	timerInitializationMode = false;
    }
  else
    {
      loadFlashlist();
      parseFlashlist();  
    }
}

/* The default page load action is to display a page containing a table
 * with those channels whose baseline corrections are in an abnormal state.
 */
void PixelMonitor::Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  time_t currentTime = time(NULL);
  time_t crateTime = 0;
    
  //  cout << "Default (web page) " << endl;

  try {
  // Initialize Web Page
  out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
  *out << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict) << std::endl;
  //k  *out << cgicc::html().set("lang", "en").set("dir","ltr") << std::endl;
  //k  *out << cgicc::title("PixelMonitor") << std::endl; 
  *out<<"<head>"<<std::endl;

  // Auto-refresh.
  *out << " <meta HTTP-EQUIV=\"Refresh\" CONTENT=\"" << REFRESH_DELAY << "; URL=Default\"/>" << std::endl;
  *out<<"</head>"<<std::endl;
  xgi::Utils::getPageHeader(*out, "PixelMonitor", "Running");


  *out<<"<body>"<<std::endl;

  //  *out << "<p>" << std::endl;
  *out << "<H2>Baseline Correction Monitor for CMS Pixel Subdetector</H2>"<< std::endl;
  //  *out << "</p>"<< std::endl;

  
  *out << "<table border=\"1\">";
  *out << "<tr>"<< std::endl;
  *out << "<td style=\"background-color:rgb(255,255,0)\">Warning Level:</td>"<< std::endl;
  *out << "<td> BC&gt;" << CHANNEL_BORDERLINE_OK_MAX ;
  *out << " or BC&lt;"<<CHANNEL_BORDERLINE_OK_MIN<< "</td>" << std::endl;
  *out << "</tr>"<< std::endl;


  *out << "<tr>"<< std::endl;
  *out << "<td style=\"background-color:rgb(255,0,0)\">Error Level:</td>"<< std::endl;
  *out << "<td> BC&gt;" << CHANNEL_BORDERLINE_WARN_MAX;
  *out << " or BC&lt;"<<CHANNEL_BORDERLINE_WARN_MIN;
  *out << "</td>"<< std::endl;
  *out << "</table>"<< std::endl;



  // Generate crate-level report.
  map<unsigned int,PixelCrate*>::iterator iterCrate = crateMap.begin();
  while (iterCrate != crateMap.end())
    {

      *out << "<p>"<< std::endl;
      *out << "  "<< std::endl;
      *out << "</p>"<< std::endl; 

      *out << "<table border=\"1\">"<< std::endl;

      *out << "<tr>"<< std::endl;
      *out << "<td>" << "Crate Number" << "</td>"<< std::endl;
      *out << "<td>" << iterCrate->second->crateNumber << "</td>"<< std::endl;
      *out << "<td>" << "Run Number" << "</td>"<< std::endl;
      *out << "<td>" << iterCrate->second->runNumber << "</td>"<< std::endl;
      *out << "</tr>"<< std::endl; 
 
      // Output BC timestamp for crate.
      crateTime = iterCrate->second->crateTimeStamp;
      *out << "<tr>"<< std::endl;  
      *out << "<td>" << "Baseline Correction Timestamp" << "</td>"<< std::endl;
      *out << "<td>"  << std::ctime( &crateTime) << "</td>" << std::endl;  

      // Check for out of date BC timestamp.
      *out << "<td>" << "Report Status" << "</td>"<< std::endl;
      if ((currentTime - crateTime) > CRATE_TIMEOUT) 
	{
	  *out << "<td style=\"background-color:rgb(150,75,0)\">";
	  *out << "Stale";
	  *out << "</td>"<< std::endl;
	} 
      else 
	{
	  *out << "<td>" << "OK";
	  *out << "</td>"<< std::endl;	  
	}
      *out << "</tr>"<< std::endl;

      // Output Max BC value for crate
      *out << "<tr>"<< std::endl;
      *out << "<td>" << "Max Baseline Correction" << "</td>"<< std::endl;
      *out << "<td>" << iterCrate->second->crateBCMaxVal << "</td>"<< std::endl;
      *out << "<td>" << "FED #" << "</td>"<< std::endl;
      *out << "<td>" << iterCrate->second->crateBCMaxFed << "</td>"<< std::endl;
      *out << "<td>" << "Channel #" << "</td>"<< std::endl;
      *out << "<td>" << iterCrate->second->crateBCMaxChannel << "</td>"<< std::endl;
      *out << "</tr>"<< std::endl;    

      // Output Min BC value for crate
      *out << "<tr>"<< std::endl;
      *out << "<td>" << "Min Baseline Correction" << "</td>"<< std::endl;
      *out << "<td>" << iterCrate->second->crateBCMinVal << "</td>"<< std::endl;
      *out << "<td>" << "FED #" << "</td>"<< std::endl;
      *out << "<td>" << iterCrate->second->crateBCMinFed << "</td>"<< std::endl;
      *out << "<td>" << "Channel #" << "</td>"<< std::endl;
      *out << "<td>" << iterCrate->second->crateBCMinChannel << "</td>"<< std::endl;
      *out << "</tr>"<< std::endl;    

      // Output Avg BC value for crate
      *out << "<tr>"<< std::endl;
      *out << "<td>" << "Avg Baseline Correction" << "</td>"<< std::endl;
      *out << "<td>" << iterCrate->second->crateBCAvgVal << "</td>"<< std::endl;
      *out << "</tr>"<< std::endl;    



      // Output number of channels in each state for crate
      *out << "<tr>"<< std::endl;
      *out << "<td>" << "OK Channels" << "</td>"<< std::endl;
      *out << "<td>" << iterCrate->second->crateOkChannels << "</td>"<< std::endl;
      *out << "<td>" << "WARN Channels" << "</td>"<< std::endl;
      if (iterCrate->second->crateWarnChannels > 0) 
        *out << "<td style=\"background-color:rgb(255,255,0)\">";
      else
        *out << "<td>";
      *out << iterCrate->second->crateWarnChannels << "</td>"<< std::endl;
      *out << "<td>" << "ERROR Channels" << "</td>"<< std::endl;
      if (iterCrate->second->crateErrorChannels > 0) 
        *out << "<td style=\"background-color:rgb(255,0,0)\">";
      else
        *out << "<td>";
      *out << iterCrate->second->crateErrorChannels << "</td>"<< std::endl;
      *out << "</tr>"<< std::endl;    

      *out << "</table>"<< std::endl;


      iterCrate++;
    }

  *out << "<p>"<< std::endl;
  *out << "  "<< std::endl;
  *out << "</p>"<< std::endl; 

  // Generate channel-level report
  // Dump all channel readings that are in an error state into a table
  *out << "<table border=\"1\">";
  *out << "<tr>"<< std::endl;
  *out << "<td>" << "FED" << "</td>"<< std::endl;
  *out << "<td>" << "Channel" << "</td>"<< std::endl;
  *out << "<td>" << "Baseline Correction Mean" << "</td>"<< std::endl;
  *out << "<td>" << "Baseline Correction StdDev" << "</td>"<< std::endl;
  *out << "<td>" << "State" << "</td>"<< std::endl;
  *out << "<tr>"<< std::endl;
  *out << "</tr>"<< std::endl;
  
  iterCrate = crateMap.begin();  // Iterate over crate.
  while (iterCrate != crateMap.end()) 
    {  
      // Iterate over FED.
      map<unsigned int,PixelFED *>::iterator iterFed = iterCrate->second->fedMap.begin();
      while (iterFed != iterCrate->second->fedMap.end())
	{
	  // Iterate over channel.
	  map<unsigned int,PixelChannel *>::iterator iterChannel = iterFed->second->channelMap.begin();
	  while (iterChannel != iterFed->second->channelMap.end())
	    {
	      if ((iterChannel->second->currentBCReading.get_baselineCorrectionChannelState() != CHANNEL_OK ) )
		///&&(iterChannel->second->currentBCReading.get_baselineCorrectionChannelState() != CHANNEL_BORDERLINE_OK ))
		{
		  *out << "<tr>"<< std::endl;
		  *out << "<td>" << iterFed->first << "</td>"<< std::endl;
		  *out << "<td>" << iterChannel->first << "</td>"<< std::endl;	
		  *out << "<td>" << iterChannel->second->currentBCReading.get_baselineCorrectionMean() << "</td>"<< std::endl;
		  *out << "<td>" << iterChannel->second->currentBCReading.get_baselineCorrectionStdDev() << "</td>"<< std::endl;

		  if ((iterChannel->second->currentBCReading.get_baselineCorrectionChannelState() == CHANNEL_WARN) ) 
		    {
		      *out << "<td style=\"background-color:rgb(255,255,0)\">";
		      *out << "WARN";
		      *out << "</td>"<< std::endl;
		    }
		  else if (iterChannel->second->currentBCReading.get_baselineCorrectionChannelState() == CHANNEL_BORDERLINE_WARN)
		    {
		      *out << "<td style=\"background-color:rgb(255,0,255)\">";
		      *out << "WARN";
		      *out << "</td>"<< std::endl;
		    }
		  else 
		  if (iterChannel->second->currentBCReading.get_baselineCorrectionChannelState() == CHANNEL_ERROR)
		    {
		      *out << "<td style=\"background-color:rgb(255,0,0)\">";
		      *out << "ERROR";
		      *out << "</td>"<< std::endl;
		    }
		  else
		    {
		      *out << "<td style=\"background-color:rgb(204,255,0)\">";
		      *out << "BARELY OK ";
		      *out << "</td>"<< std::endl;
		    }

		  *out << "</tr>"<< std::endl;
		}  
	      else 
		{
	
	// Debug output to view OK channels
/*
		  *out << "<tr>"<< std::endl;
		  *out << "<td>" << iterFed->first << "</td>"<< std::endl;
		  *out << "<td>" << iterChannel->first << "</td>"<< std::endl;	
		  *out << "<td>" << iterChannel->second->currentBCReading.get_baselineCorrectionMean() << "</td>"<< std::endl;
		  *out << "<td>" << iterChannel->second->currentBCReading.get_baselineCorrectionStdDev() << "</td>"<< std::endl;
		  
		  *out << "<td style=\"background-color:rgb(0,255,0)\">";
		  *out << "OK";
		  *out << "</td>"<< std::endl;
		  *out << "</tr>"<< std::endl;
 */
		}
	      iterChannel++;
	    }
	  iterFed++;
	}
      iterCrate++;
    }

  *out << "</table>"<< std::endl;
  
  *out << "<p>"<< std::endl;
  *out << "  "<< std::endl;
  *out << "</p>"<< std::endl;
    
  *out << "<table border=\"1\">";
  *out << "<tr>"<< std::endl;
  *out << "<td>" << " " << "</td>" << "<td>"  << " " << "</td>" << std::endl;
  *out << "<td>" << "Error Type" << "</td>" << "<td>" << "Num Errors" << "</td>" << "<td>"  << "Channel" << "</td>" << std::endl;
  *out << "<td>" << "Error Type" << "</td>" << "<td>" << "Num Errors" << "</td>" << "<td>"  << "Channel" << "</td>" << std::endl;
  *out << "<td>" << "Error Type" << "</td>" << "<td>" << "Num Errors" << "</td>" << "<td>"  << "Channel" << "</td>" << std::endl;
  *out << "</tr>"<< std::endl;

  *out << "<tr>"<< std::endl;  
  if ( myErrTable != 0 ){
    for(unsigned int fed_iter = 0; fed_iter < myErrTable->errTableMap.size(); fed_iter++){  
      int maxROCErr = -1, maxROCCh = -1, maxOOSErr = -1, maxOOSCh = -1, maxTOErr = -1, maxTOCh = -1;
      //int maxIncROCErr = -1, maxIncROCCh = -1, maxIncOOSErr = -1, maxIncOOSCh = -1, maxIncTOErr = -1, maxIncTOCh = -1;
      myErrTable->getMAXErrors(fed_iter, maxROCErr, maxROCCh, maxOOSErr, maxOOSCh, maxTOErr, maxTOCh);
      ///myErrTable->getMAXIncreaseErrors(fed_iter, maxIncROCErr, maxIncROCCh, maxIncOOSErr, maxIncOOSCh, maxIncTOErr, maxIncTOCh);
      *out << "<td>" << "FED#" << "</td>" << "<td>"  << fed_iter << "</td>" << std::endl;
      *out << "<td>" << "   ROC    " << "</td>"<<  "<td>"  << maxROCErr  << "</td>" << "<td>" << maxROCCh  << "</td>" <<std::endl;
      *out << "<td>" << "   OOS    " << "</td>"<<  "<td>"  << maxOOSErr << "</td>" << "<td>" << maxOOSCh << "</td>" <<std::endl;
      *out << "<td>" << " TimeOut  " << "</td>"<<  "<td>"  << maxTOErr << "</td>" << "<td>" << maxTOCh << "</td>" <<std::endl;
      *out << "</tr>"<< std::endl;
    }
  }
  *out << "</tr>"<< std::endl;
  *out << "</table>"<< std::endl;
  
  /* --------------------------- */

  *out<<"</body>"<<std::endl;
  *out << "</html>"<<std::endl;
  }
  catch (xgi::exception::Exception & e ) 
    {std::cout << "Exception caught " << e.what() << std::endl; }
}


void * PixelMonitor::Realloc(void* ptr, size_t size)
{
  if(ptr)
    return realloc(ptr, size);
  else
    return malloc(size);
}


/* Copies crate report from curlpp buffer into flashlistBuffer. */
size_t PixelMonitor::WriteMemoryCallback(char* ptr, size_t size, size_t nmemb)
{
  // Calculate the real size of the incoming buffer
  size_t realsize = size * nmemb;
  
  // (Re)Allocate memory for the buffer
  flashlistBuffer = (char*) Realloc(flashlistBuffer, flashlistBufferSize + realsize);
  
  // Test if Buffer is initialized correctly & copy memory
  if (flashlistBuffer == NULL) {
    realsize = 0;
  }

  memcpy(&(flashlistBuffer[flashlistBufferSize]), ptr, realsize);
  flashlistBufferSize += realsize;
  
  // return the real size of the buffer...
  return realsize;
}

// Gets address of buffer containing newly loaded flashlist.
void PixelMonitor::loadFlashlist()
{
  //  cout << "loadFlashlist() " << endl;
  try {
    cURLpp::Cleanup cleaner;
    cURLpp::Easy request;
    
    // Flush old report.
    flashlistBufferSize = 0;
    flashlistBuffer = (char*) Realloc(flashlistBuffer, flashlistBufferSize);
    
    // Set the writer callback to enable cURL tree->Fill()
    // to write result to report buffer.
    cURLpp::Types::WriteFunctionFunctor functor(this, 
                                                &PixelMonitor::WriteMemoryCallback);
    cURLpp::Options::WriteFunction *test = new cURLpp::Options::WriteFunction(functor);
    request.setOpt(test);
    
    // Setting the URL to retrieve.
    request.setOpt(new cURLpp::Options::Url(url));
    request.setOpt(new cURLpp::Options::Verbose(false)); // Set true for debugging.
    request.perform();

    // DEBUG: dump report to standard output
    //     std::cout << flashlistBuffer << std::endl;
    
  }
  catch ( cURLpp::LogicError & e ) {
     std::cout << e.what() << std::endl;
    
  }
  catch ( cURLpp::RuntimeError & e ) {
       std::cout << e.what() << std::endl;
  }
  
  //      cout << "Flashlist Buffer Size: " << flashlistBufferSize << endl;
}


void PixelMonitor::parseFlashlist()
{
  int crateNumber;
  //  cout << "parseFlashlist() " << endl;
  // Move binary data in buffer from HTTP into specialized XDAQ stream buffer.
  // Binary data is in EXDR format.
  xdata::exdr::FixedSizeInputStreamBuffer inBuffer(flashlistBuffer,flashlistBufferSize);

  try 
    {
      // Convert data from EXDR format flashlist into a 
      // one-row master table containing all monitoring data.
      xdata::exdr::Serializer serializer;
      serializer.import(flashlistTable, &inBuffer);
      
      for (unsigned int i = 0; i < flashlistTable->getRowCount(); i++)
	{
	  // Extract crate number from master table.
	  crateNumber =  *((xdata::UnsignedInteger32*)flashlistTable->getValueAt(i,"crateNumber"));
	  //cout << "parseFlashlist() found crate " << crateNumber << endl;  // debug

	  
	  // Make new crate object if necessary.
	  std::map<unsigned int,PixelCrate *>::iterator crateIter;
	  crateIter = crateMap.find(crateNumber);
	  if (crateIter == crateMap.end()) crateMap[crateNumber] = new PixelCrate(crateNumber,this);
	  

	  // Send row corresponding to crate to crate object for processing.
	  crateMap[crateNumber]->parseCrateRow(i);
	  
	  errTablePtr = (xdata::Table*)flashlistTable->getValueAt(i,"errorTable");
	  
	  int errorTableTimeStamp; 
	  errorTableTimeStamp = *((xdata::UnsignedInteger32*)flashlistTable->getValueAt(i,"errTableTimeStamp"));
	  myErrTable->parseErrorTableCrate(errTablePtr, errorTableTimeStamp, crateNumber);
	  ///myErrTable->printErrTableMap();
	  	  
	  flashlistTimeStamp = time(NULL);
	}
    }
  catch(xdata::exception::Exception & e )
    {
      // If HTTP buffer is invalid, nothing happens.
      cout <<"parseFlashlist() caught exception " << e.what() << endl; //debug
    }

}

// Dumps the worst baseline correction values (since the last dump) to a ROOT file.
// File name is based on the run number.
void PixelMonitor::outputToROOT()
{

}

/*
  BaselineCorrectionReading outputBaselineCorrectionReading;
  int channelNumber = 0;
  int fedNumber = 0;

  // (re)open ROOT file based on run number.
  std::string ROOTFileName;
  std::stringstream ROOTFileStream;
  ROOTFileStream << "/FPixDAQ/DAQ/build_robert/TriDAS/pixel/PixelMonitor/baseline_correction_data_stream"
	       << runNumber  
	       << ".root";  
  ROOTFileName = ROOTFileStream.str();

  TFile * f = new TFile(ROOTFileName.c_str(),"UPDATE");
  TTree * tree = NULL;

  // Create or load tree.
  tree = (TTree*)f->Get("T");
  if (tree == NULL) 
    {
      tree = new TTree("T","data stream for baseline correction");
 
      // Create new tree branches.
      tree->Branch("baselineCorrectionMean",NULL,"baselineCorrectionMean/F");
      tree->Branch("baselineCorrectionStdDev",NULL,"baselineCorrectionStdDev/F");
      tree->Branch("baselineCorrectionChannelState",NULL,"baselineCorrectionChannelState/I");
      tree->Branch("crateNumber",NULL,"crateNumber/I");  // Stored in CrateRecord object.
      tree->Branch("fedNumber",NULL,"fedNumber/I"); 
      tree->Branch("channelNumber",NULL,"channelNumber/I");
      tree->Branch("timeStamp",NULL,"baselineCorrectionTimeStamp/i");  // Stored in CrateRecord object.
    }
  
  // Connect existing tree branches to output variables.
  tree->SetBranchAddress("baselineCorrectionMean",outputBaselineCorrectionReading.get_baselineCorrectionMean_address());
  tree->SetBranchAddress("baselineCorrectionStdDev",outputBaselineCorrectionReading.get_baselineCorrectionStdDev_address());
  tree->SetBranchAddress("baselineCorrectionChannelState",outputBaselineCorrectionReading.get_baselineCorrectionChannelState_address());
  tree->SetBranchAddress("crateNumber",&crateNumber);  // Stored in CrateRecord object.
  tree->SetBranchAddress("fedNumber",&fedNumber); 
  tree->SetBranchAddress("channelNumber",&channelNumber);
  tree->SetBranchAddress("timeStamp",&baselineCorrectionTimeStamp);  // Stored in CrateRecord object.

  // Dump the worst channel readings for the current interval to ROOT object 
  map<int,map<int,BaselineCorrectionReading> >::iterator iterFed = WorstBaselineCorrectionReadingMap.begin();
  while (iterFed != WorstBaselineCorrectionReadingMap.end())
    {
      map<int,BaselineCorrectionReading>::iterator iterChannel = iterFed->second.begin();
      while (iterChannel != iterFed->second.end())
	{
	  fedNumber = iterFed->first;
	  channelNumber = iterChannel->first;
	  outputBaselineCorrectionReading.set_baselineCorrectionMean(iterChannel->second.get_baselineCorrectionMean()); // Also sets channel state.
	  outputBaselineCorrectionReading.set_baselineCorrectionStdDev(iterChannel->second.get_baselineCorrectionStdDev()); 
	  tree->Fill();
	  iterChannel++;
	}
      iterFed++;
    }
  tree->Write();    
  //  tree->Print();
  
  // Cleanup
  f->Close();
  delete f;

  // Reset worst channel readings to current readings 
  WorstBaselineCorrectionReadingMap = BaselineCorrectionReadingMap;
}

*/


PixelCrate::PixelCrate(unsigned int cratenum, PixelMonitor * parent)
{ 
  crateNumber = cratenum;
  parentMonitorPtr = parent;

  runNumber = 0; // Not yet valid.
  
  crateTimeStamp = 0; // Not yet valid.

  crateBCMaxVal = -10000.0;
  crateBCMaxFed = -1;
  crateBCMaxChannel = -1;
  crateBCMinVal = 10000.0;
  crateBCMinFed = -1;
  crateBCMinChannel = -1;
  crateBCAvgVal = 0;

  crateOkChannels = 0;
  crateWarnChannels = 0;
  crateErrorChannels = 0;
}

// Process row of flashlist for this crate.
void PixelCrate::parseCrateRow(unsigned int flashlistRowNumber)
{
  int fedNumber;  // temporary fed number.

  float fedBCMean; // temporary fed BC mean value
  float crateBCSum = 0.0; // Used for computing average BC value for crate
  int crateBCNum = 0;

  // Temporary channel state counters;
  int okChannels = 0;
  int warnChannels = 0;
  int errorChannels = 0;

  // Update run number from flashlist.
  unsigned int newRunNumber;  
  newRunNumber =  
    *((xdata::UnsignedInteger32*)parentMonitorPtr->flashlistTable->getValueAt(flashlistRowNumber,"runNumber"));
  
  //  cout << "parseCrateRow(i) run# " << newRunNumber << endl;  //debug

  // Flush FED map at the beginning of a new run.
  if (runNumber != newRunNumber)
    {
      runNumber = newRunNumber;
      fedMap.clear();
    }

  // Extract crate timestamp from flashlist.
  unsigned int newCrateTimeStamp; 
  newCrateTimeStamp =  
    *((xdata::UnsignedInteger32*)parentMonitorPtr->flashlistTable->getValueAt(flashlistRowNumber,"crateTimeStamp"));

  //  cout << "parseCrateRow(i) timestamp =" << newCrateTimeStamp << endl; //debug
  
  // Process crate table if it has been updated.
  if (crateTimeStamp != newCrateTimeStamp)
    {
      crateTimeStamp = newCrateTimeStamp;

      // Reset min, max, avg baseline correction values for crate.
      crateBCMaxVal = -10000.0;
      crateBCMaxFed = -1;
      crateBCMaxChannel = -1;
      crateBCMinVal = 10000.0;
      crateBCMinFed = -1;
      crateBCMinChannel = -1;
      crateBCAvgVal = 0;
      
      // Extract crate table from master table.
       crateTablePtr =  
	 (xdata::Table*)parentMonitorPtr->flashlistTable->getValueAt(flashlistRowNumber,"crateTable");
      
      // Process flashlist data for individual feds.
      for (unsigned int i = 0; i < crateTablePtr->getRowCount(); i++)
	{
	  // Extract fed number from crate table.
	  fedNumber =  
	    *((xdata::UnsignedInteger32*)crateTablePtr->getValueAt(i,"fedNumber"));

	  // cout << "parseCrateRow(i) FED#=" << fedNumber << endl; //debug

	  // Make new FED object if necessary.
	  std::map<unsigned int,PixelFED *>::iterator fedIter;
	  fedIter = fedMap.find(fedNumber);
	  if (fedIter == fedMap.end()) fedMap[fedNumber] = new PixelFED(fedNumber,this);
	  
	  // Process fed-specific inner table.
	  fedMap[fedNumber]->parseFEDRow(i);

          // Get BC mean for fed, use to update average BC for crate.
          fedBCMean = fedMap[fedNumber]->fedBCAvgVal;         
	  crateBCSum += fedBCMean; // FIXME: Use weighted average.
          crateBCNum++;

          // Get BC max for fed, use to update max BC for crate.
          if (fedMap[fedNumber]->fedBCMaxVal > crateBCMaxVal)
            {
              crateBCMaxVal = fedMap[fedNumber]->fedBCMaxVal;
              crateBCMaxFed = fedNumber;
              crateBCMaxChannel = fedMap[fedNumber]->fedBCMaxChannel;
            } 

          // Get BC min for fed, use to update min BC for crate.
          if (fedMap[fedNumber]->fedBCMinVal < crateBCMinVal)
            {
              crateBCMinVal = fedMap[fedNumber]->fedBCMinVal;
              crateBCMinFed = fedNumber;
              crateBCMinChannel = fedMap[fedNumber]->fedBCMinChannel;
            } 

          okChannels += fedMap[fedNumber]->fedOkChannels;
          warnChannels += fedMap[fedNumber]->fedWarnChannels;
          errorChannels += fedMap[fedNumber]->fedErrorChannels;
	}

      crateBCAvgVal = crateBCSum / crateBCNum;      
      crateOkChannels = okChannels;
      crateWarnChannels = warnChannels;
      crateErrorChannels = errorChannels;
    }
}


PixelFED::PixelFED(unsigned int fednum, PixelCrate * parent)
{ 
  fedNumber = fednum;
  parentCratePtr = parent;

  fedBCMaxVal = -10000.0;
  fedBCMaxChannel = -1;
  fedBCMinVal = 10000.0;
  fedBCMinChannel = -1;
  fedBCAvgVal = 0;

  fedOkChannels = 0;
  fedWarnChannels = 0;
  fedErrorChannels = 0;
}

// Process row of crate table for this FED.
void PixelFED::parseFEDRow(unsigned int crateTableRowNumber)
{
  int channelNumber;  // temporary channel number.
  float channelBCMean; // temporary channel BC mean value

  float fedBCSum = 0.0; // Used for computing average BC value for FED
  int fedBCNum = 0;

  // Reset min, max, avg baseline correction values for FED.
  fedBCMaxVal = -10000.0;  // Absurdly low
  fedBCMaxChannel = -1;
  fedBCMinVal = 10000.0;  // Ridiculously high
  fedBCMinChannel = -1;
  fedBCAvgVal = 0;

  // Temporary channel state counters.
  int okChannels = 0;
  int warnChannels = 0;
  int errorChannels = 0;

  // Extract fed table from parent crate table.
  fedTablePtr = (xdata::Table*)parentCratePtr->crateTablePtr->getValueAt(crateTableRowNumber,"fedTable");
      
  // Process flashlist data for individual channels.  
  for (unsigned int i = 0; i < fedTablePtr->getRowCount(); i++)
    {
      // Extract channel number from fed table.
      channelNumber =  *((xdata::UnsignedInteger32*)fedTablePtr->getValueAt(i,"channelNumber"));
      //      cout<<"parseFEDRow( ) chan=" << channelNumber << endl; //debug
      // Make new channel object if necessary.
      std::map<unsigned int,PixelChannel *>::iterator channelIter;
      channelIter = channelMap.find(channelNumber);
      if (channelIter == channelMap.end()) channelMap[channelNumber] = new PixelChannel(channelNumber,this);
      
      // Process channel-specific inner table.
      channelMap[channelNumber]->parseChannelRow(i);

      // Get BC mean for channel, use to update FED best/worst/average BC
      channelBCMean = channelMap[channelNumber]->getBCMean();
      fedBCSum += channelBCMean;
      fedBCNum++;

      if (fedBCMaxVal < channelBCMean)
        {
          fedBCMaxVal = channelBCMean;
          fedBCMaxChannel = channelNumber;
        } 
      if (fedBCMinVal > channelBCMean)
        {
          fedBCMinVal = channelBCMean;
          fedBCMinChannel = channelNumber;
        }

      // Adjust number of channels in ok, warn, and error states based on state of channel under consideration.
      if ((channelMap[channelNumber]->currentBCReading.get_baselineCorrectionChannelState() == CHANNEL_OK ) ||
          (channelMap[channelNumber]->currentBCReading.get_baselineCorrectionChannelState() == CHANNEL_BORDERLINE_OK ))
        {
          okChannels++;
        }
      if ((channelMap[channelNumber]->currentBCReading.get_baselineCorrectionChannelState() == CHANNEL_WARN ) ||
          (channelMap[channelNumber]->currentBCReading.get_baselineCorrectionChannelState() == CHANNEL_BORDERLINE_WARN ))
        {
          warnChannels++;
        }
      if (channelMap[channelNumber]->currentBCReading.get_baselineCorrectionChannelState() == CHANNEL_ERROR )
        {
          errorChannels++;
        }
    }
   
  fedOkChannels = okChannels;
  fedWarnChannels = warnChannels;
  fedErrorChannels = errorChannels;
  fedBCAvgVal = fedBCSum / fedBCNum;
}


PixelChannel::PixelChannel(unsigned int channelnum, PixelFED * parent)
{ 
  channelNumber = channelnum;
  parentFEDPtr = parent;
}

float PixelChannel::getBCMean()
{
  return currentBCReading.get_baselineCorrectionMean();
}



// Process row of fed table for this channel.
void PixelChannel::parseChannelRow(unsigned int fedTableRowNumber)
{
  newBCReading.set_baselineCorrectionMean(*((xdata::Float*)parentFEDPtr->fedTablePtr->getValueAt(fedTableRowNumber,"baselineCorrectionMean")));
  newBCReading.set_baselineCorrectionStdDev(*((xdata::Float*)parentFEDPtr->fedTablePtr->getValueAt(fedTableRowNumber,"baselineCorrectionStdDev")));    
  
  processNewBaselineCorrectionReading();
}



void PixelChannel::processNewBaselineCorrectionReading()
{

  int newState = newBCReading.get_baselineCorrectionChannelState();
  int oldState = currentBCReading.get_baselineCorrectionChannelState();
  
  // Check for state change, send error message to DiagSystem if so.
  if (newState != oldState) 
    {
      std::string errorMsg;
      std::stringstream errorStream;
      errorStream << "Baseline Correction of channel "
		  << channelNumber 
		  << " on FED " 
		  << parentFEDPtr->fedNumber << " on crate "
		  << parentFEDPtr->parentCratePtr->crateNumber;
      

      // diag system message only if squawked earlier 
      if (newState == CHANNEL_OK  && currentBCReading.get_baselineCorrectionSquawk() > CHANNEL_OK )
	{ // returned to OK status from above
	  errorStream << " has returned to normal. Value is "
		      << newBCReading.get_baselineCorrectionMean() << endl;
	  errorMsg = errorStream.str();
	  parentFEDPtr->parentCratePtr->parentMonitorPtr->diagService_->reportError(errorMsg,DIAGINFO);
	  currentBCReading.set_baselineCorrectionSquawk(CHANNEL_OK);
	}
      else if (newState == CHANNEL_WARN) 
	{
	  if (oldState > newState  // we are dropping in severity
	      && currentBCReading.get_baselineCorrectionSquawk() > CHANNEL_WARN ) // and haven't posted yet
	    {
	      errorStream << " is out of range. Value is now "
			  << newBCReading.get_baselineCorrectionMean() << endl;
	      errorMsg = errorStream.str();
	      parentFEDPtr->parentCratePtr->parentMonitorPtr->diagService_->reportError(errorMsg,DIAGWARN);
	      currentBCReading.set_baselineCorrectionSquawk(CHANNEL_WARN);
	    }
	  else if ( oldState < newState 
		    && currentBCReading.get_baselineCorrectionSquawk() < CHANNEL_WARN ) 
	    { // we are escalating in severity & haven't posted yet
	      errorStream << " is out of range. Value is "
			  << newBCReading.get_baselineCorrectionMean() << endl;
	      errorMsg = errorStream.str();
	      parentFEDPtr->parentCratePtr->parentMonitorPtr->diagService_->reportError(errorMsg,DIAGWARN);
	      currentBCReading.set_baselineCorrectionSquawk(CHANNEL_WARN);
	    }
	}
      else if (newState == CHANNEL_ERROR 
	       && currentBCReading.get_baselineCorrectionSquawk() < CHANNEL_ERROR )
	{
	  errorStream << " is severely out of range. Value is "
		      << newBCReading.get_baselineCorrectionMean() << endl;
	  errorMsg = errorStream.str();
	  parentFEDPtr->parentCratePtr->parentMonitorPtr->diagService_->reportError(errorMsg,DIAGERROR);
	  currentBCReading.set_baselineCorrectionSquawk(CHANNEL_ERROR);
	}
      else if (newState == CHANNEL_BORDERLINE_WARN && currentBCReading.get_baselineCorrectionSquawk() < CHANNEL_WARN )
	{ // escalating from below WARN 
	      errorStream << " is out of range. Value is "
			  << newBCReading.get_baselineCorrectionMean() << endl;
	      errorMsg = errorStream.str();
	      parentFEDPtr->parentCratePtr->parentMonitorPtr->diagService_->reportError(errorMsg,DIAGWARN);
	      currentBCReading.set_baselineCorrectionSquawk(CHANNEL_WARN);	  
	}
      else if (newState == CHANNEL_BORDERLINE_OK && currentBCReading.get_baselineCorrectionSquawk() > CHANNEL_WARN )
	{ // case coming from ERROR without stopping at WARN
	  errorStream << " has returned to normal. Value is "
		      << newBCReading.get_baselineCorrectionMean() << endl;
	  errorMsg = errorStream.str();
	  parentFEDPtr->parentCratePtr->parentMonitorPtr->diagService_->reportError(errorMsg,DIAGINFO);
	  currentBCReading.set_baselineCorrectionSquawk(CHANNEL_OK);  
	}
    }
      
  // Update worst channel readings for this interval 
  if ((newBCReading.get_baselineCorrectionMean() - CHANNEL_NOMINAL) > 
      (worstBCReading.get_baselineCorrectionMean() - CHANNEL_NOMINAL))
    {
      worstBCReading.set_baselineCorrectionMean(newBCReading.get_baselineCorrectionMean());
      worstBCReading.set_baselineCorrectionStdDev(newBCReading.get_baselineCorrectionStdDev());
    }


  // Update current channel reading with new channel reading */
  // This automatically sets channel state
  currentBCReading.set_baselineCorrectionMean(newBCReading.get_baselineCorrectionMean());
  currentBCReading.set_baselineCorrectionStdDev(newBCReading.get_baselineCorrectionStdDev());
}


void PixelErrorTable::parseErrorTableCrate(xdata::Table * errTablePtr, int TimeStamp, int CrateNumber){
    
  unsigned int fednumber;
  unsigned int chnumber;
  FEDErrors my_FEDErrors; 
  
  if ( CrateNumber == 1 ) errTableTimeStamp = TimeStamp;
  errTableCounter++;
  
  for (unsigned int j = 0; j < errTablePtr->getRowCount(); j++){
    
    fednumber = *((xdata::UnsignedInteger32*)errTablePtr->getValueAt(j,"fedNumber"));
    chnumber = *((xdata::UnsignedInteger32*)errTablePtr->getValueAt(j,"channelNumber"));
    
    my_FEDErrors.ROCerr = *((xdata::UnsignedInteger32*)errTablePtr->getValueAt(j,"NORErrors"));
    my_FEDErrors.OOSerr = *((xdata::UnsignedInteger32*)errTablePtr->getValueAt(j,"OOSErrors"));
    my_FEDErrors.TimeOUTerr = *((xdata::UnsignedInteger32*)errTablePtr->getValueAt(j,"TimeOutErrors"));
    errTableMap[fednumber][chnumber] = my_FEDErrors;
    
    if ( errTableCounter >= 100 ) errTableMap_[fednumber][chnumber] = my_FEDErrors;
  }
  
  if ( errTableCounter >= 100 && CrateNumber == 1 ){ 
    errTableTimeStamp_100 = TimeStamp;
    errTableCounter = 0;
  }
  
  //cout << "errTableCounter = " << errTableCounter << " - errTableTimeStamp = " << errTableTimeStamp << endl;
  
}

void PixelErrorTable::getMAXErrors(unsigned int FEDNumber, int &MaxROCErr, int &MaxROCCh, int &MaxOOSErr, int &MaxOOSCh, int &MaxTOErr, int &MaxTOCh){
  if(errTableMap.find(FEDNumber) != errTableMap.end()){
    map<unsigned int, FEDErrors>::iterator ch_iter;
    for(ch_iter = errTableMap[FEDNumber].begin(); ch_iter != errTableMap[FEDNumber].end(); ch_iter++){
      if((int)ch_iter->second.ROCerr > MaxROCErr){
        MaxROCErr = ch_iter->second.ROCerr;
        MaxROCCh =  ch_iter->first;
      }
      if((int)ch_iter->second.OOSerr > MaxOOSErr){
        MaxOOSErr = ch_iter->second.OOSerr;
        MaxOOSCh =  ch_iter->first;
      }
      if((int)ch_iter->second.TimeOUTerr > MaxTOErr){
        MaxTOErr = ch_iter->second.TimeOUTerr;
        MaxTOCh =  ch_iter->first;
      }
    }
  }
}

void PixelErrorTable::getMAXIncreaseErrors(unsigned int FEDNumber, int &MaxIncROCErr, int &MaxIncROCCh, int &MaxIncOOSErr, int &MaxIncOOSCh, int &MaxIncTOErr, int &MaxIncTOCh){
  int tempIncROCErr(-1); int tempIncOOSErr(-1); int tempIncTOErr(-1);
  int timediff(-1);
  timediff = errTableTimeStamp - errTableTimeStamp_100;
  cout << "errTableTimeStamp = "<< errTableTimeStamp << " errTableTimeStamp_100 = "<< errTableTimeStamp_100 << " timediff = "<< timediff << endl;
  
  if( (errTableMap.find(FEDNumber) != errTableMap.end()) && (errTableMap_.find(FEDNumber) != errTableMap_.end()) && timediff > 0 && errTableMap_.size() == 40 && errTableMap.size() == 40 ){
    map<unsigned int, FEDErrors>::iterator ch_iter;
    map<unsigned int, FEDErrors>::iterator ch_iter_;
    for(ch_iter = errTableMap[FEDNumber].begin(); ch_iter != errTableMap[FEDNumber].end(); ch_iter++){
      for(ch_iter_ = errTableMap_[FEDNumber].begin(); ch_iter_ != errTableMap_[FEDNumber].end(); ch_iter_++){
	
	if(ch_iter->first == ch_iter_->first){
	  
	  cout << " FEDNumber = " << FEDNumber << " ch_iter->first = " << ch_iter->first << " ch_iter_->first = "<< ch_iter_->first << endl;
	  
	  tempIncROCErr = (ch_iter->second.ROCerr - ch_iter_->second.ROCerr)/timediff;
	  if ( tempIncROCErr > MaxIncROCErr  ){
	    MaxIncROCErr = tempIncROCErr;
	    MaxIncROCCh =  ch_iter->first;
	  }
	  
	  tempIncOOSErr = (ch_iter->second.OOSerr - ch_iter_->second.OOSerr)/timediff;
	  if ( tempIncOOSErr > MaxIncOOSErr  ){
	    MaxIncOOSErr = tempIncOOSErr;
	    MaxIncOOSCh =  ch_iter->first;
	  }
	  
	  tempIncTOErr = (ch_iter->second.TimeOUTerr - ch_iter_->second.TimeOUTerr)/timediff;
	  if ( tempIncTOErr > MaxIncTOErr  ){
	    MaxIncTOErr = tempIncTOErr;
	    MaxIncTOCh =  ch_iter->first;
	  }
	  
	}
	
      }
    }

    cout << "MaxIncROCErr = "<< MaxIncROCErr << " MaxIncROCCh = "<< MaxIncROCCh << endl;
    cout << "MaxIncOOSErr = "<< MaxIncOOSErr << " MaxIncOOSCh = "<< MaxIncOOSCh << endl;
    cout << "MaxIncTOErr = "<< MaxIncTOErr << " MaxIncTOCh = "<< MaxIncTOCh << endl;
    
  }
}


void PixelErrorTable::printErrTableMap(){
  map<unsigned int, map<unsigned int, FEDErrors> >::iterator fed_iter;
  map<unsigned int, FEDErrors>::iterator ch_iter;
  map<unsigned int, FEDErrors>::iterator ch_iter_;
  
  for(fed_iter = errTableMap.begin(); fed_iter != errTableMap.end(); fed_iter++){
//    for(ch_iter = fed_iter->second.begin(); ch_iter != fed_iter->second.end(); ch_iter++){
    ch_iter = fed_iter->second.begin();
    //cout << fed_iter->first << " " << ch_iter->first << " " << ch_iter->second.ROCerr << " " << ch_iter->second.OOSerr << " " << ch_iter->second.TimeOUTerr << endl;
  //  }
  }
  
  cout <<" errTableMap_.size() = " << errTableMap_.size() << endl;
  for(fed_iter = errTableMap_.begin(); fed_iter != errTableMap_.end(); fed_iter++){
    //for(ch_iter_ = fed_iter->second.begin(); ch_iter_ != fed_iter->second.end(); ch_iter_++){
    ch_iter_ = fed_iter->second.begin();
    cout << fed_iter->first << " " << ch_iter_->first << " " << ch_iter_->second.ROCerr << " " << ch_iter_->second.OOSerr << " " << ch_iter_->second.TimeOUTerr << endl;
    //}
  }
}

