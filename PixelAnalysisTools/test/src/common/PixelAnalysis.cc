//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
// authors G. Cerati, M. Dinardo, K. Ecklund, B. HeyBurn, A. Kumar, E. Luiggi, L. Uplegger  //
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
#include "PixelAnalysisTools/include/PixelAnalyzer.h"
//#include "PixelAnalysisTools/include/PixelXmlReader.h"
#include "PixelUtilities/PixelXmlUtilities/include/PixelXmlReader.h"
#include "PixelAnalysisTools/include/PixelHistoManager.h"
#include "PixelAnalysisTools/include/PixelSCurveHistoManager.h"
#include "PixelAnalysisTools/include/PixelGainHistoManager.h"
#include "PixelAnalysisTools/include/PixelAliveHistoManager.h"
#include "PixelAnalysisTools/include/PixelCalibConfigurationExtended.h"
#include "PixelAnalysisTools/include/PixelConfigurationsManager.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelHistoServer.h"
#include <fstream>
#include <iostream>
#include <string>

using namespace std;
using namespace pos;

int main(int argc, char **argv){
  string mthn = "[main()]\t";

  //------------------------------------------------------------------------------------------------
  //Getting main directory environment variable and building path to home
  if( getenv("BUILD_HOME") == 0 ){
    cout << mthn << "BUILD_HOME environment variable undefined...You need to set this variable!" << endl;
    exit(0);
  }
  string projectHome = string(getenv("BUILD_HOME"))+"/pixel/PixelAnalysisTools/";

  //------------------------------------------------------------------------------------------------
  //Checking if there are the necessary arguments to the main and if they are correct
	if(argc < 2 || argc > 3){
    cout << mthn << "Usage: PixelAnalysis (./Path To XML/MyConfiguration.xml|SCurve|PixelAlive|Gain)" << endl;
    cout << mthn << "Or you can specify a run number (for example 1974) and use the default directory configuration: PixelAnalysis (./Path To XML/MyConfiguration.xml|SCurve|PixelAlive|Gain) 1974" << endl;
    exit(0);
  }
  string document;
  if     (string(argv[1]) == "SCurve")    { document = projectHome+"test/configuration/SCurveAnalysis.xml"; }
  else if(string(argv[1]) == "PixelAlive"){ document = projectHome+"test/configuration/PixelAliveAnalysis.xml"; }
  else if(string(argv[1]) == "Gain")      { document = projectHome+"test/configuration/GainAnalysis.xml"; }
  else {
		if(string(argv[1])[0] == '/'){
			document = argv[1];
		}
		else{
			document = projectHome+"test/"+argv[1];
		}
	}

  
  //------------------------------------------------------------------------------------------------
  //Opening the xml reader for the configuration
  PixelXmlReader thePixelXmlReader;
  thePixelXmlReader.readDocument(document);


  // Get paramaters from the xml file " filenames, feds, rocs... EELL
  string calibrationType = thePixelXmlReader.getXMLAttribute("Calibration"        ,"Type");
  if((int)string(argv[1]).find(".xml") < 0){
    if(argv[1] != calibrationType){
      cout << mthn << "The calibration type in the xml file doesn't match with the type of calibration you are trying to do. Change the type of calibration to " << argv[1] << " in the xml file " << document << endl;
      exit(0);
    }
  }

  ostream *logger = &cout;
  if(thePixelXmlReader.getXMLAttribute("Logger"      ,"Type") == "file"){
    string outFileName = projectHome+"test/" + calibrationType + "ErrorLogger.txt"; 
    logger = new ofstream(outFileName.c_str());
  }

  string calibFileName;      
	string translationFileName;      
	string detConfigFileName;  
	int runNumber = -1;    
	if(argc == 3){
	  runNumber = atoi(argv[2]);    
		stringstream fileDir;
		fileDir << thePixelXmlReader.getXMLAttribute("DefaultDirectory","Directory");
		if(fileDir.str()[fileDir.str().length()-1] != '/'){
			fileDir << "/";
		}
		int groupDirectory = ((int)runNumber/1000)*1000;
		fileDir << "Run_" << groupDirectory <<  "/Run_" << argv[2] << "/";
		cout << fileDir.str() << endl;
    calibFileName       = fileDir.str() + "calib.dat";      
	  translationFileName = fileDir.str() + "translation.dat";      
	  detConfigFileName   = fileDir.str() + "detectconfig.dat";      
	}
	else{
		calibFileName       = thePixelXmlReader.getXMLAttribute("CalibFileName"      ,"FileName");
  	translationFileName = thePixelXmlReader.getXMLAttribute("TranslationFileName","FileName");
  	detConfigFileName   = thePixelXmlReader.getXMLAttribute("DetConfigFileName"  ,"FileName");
  }
	thePixelXmlReader.convertEnvVariables(calibFileName      );
  thePixelXmlReader.convertEnvVariables(translationFileName);
  thePixelXmlReader.convertEnvVariables(detConfigFileName  );

	if((int)translationFileName.find("***** CAN'T FIND TAG") >= 0){
		if(runNumber == -1){
  		cout << mthn << "Since you are not using the run number argument you must provide the 3 tags CalibFileName TranslationFileName DetConfigFileName and comment or remove the tag DefaultDirectory in the xml file" << endl;
		}
		else{
  		cout << mthn << "Since you are using the run number argument you must provide the tag DefaultDirectory in the xml file and comment or remove the 3 tags CalibFileName TranslationFileName DetConfigFileName" << endl;
		}
		exit(0);
	}
	
	
	int numberOfEvents    = atoi(thePixelXmlReader.getXMLAttribute("NumberOfEvents","Events").c_str());
  

  //------------------------------------------------------------------------------------------------------
  //Creating the configuration manager, the pixel name translation the detector config and the pixelCalib
  PixelNameTranslation            thePixelNameTranslation      (translationFileName.c_str());
  PixelDetectorConfig             thePixelDetectorConfig       (detConfigFileName.c_str());
  PixelCalibConfigurationExtended thePixelCalibConfiguration   (calibFileName.c_str(),&thePixelNameTranslation, &thePixelDetectorConfig);
  PixelConfigurationsManager      thePixelConfigurationsManager(&thePixelXmlReader,&thePixelCalibConfiguration,logger,runNumber);
  
  
  //------------------------------------------------------------------------------------------------------
  //Creating the histogram server
  static PixelHistoServer histogramServer;
  histogramServer.startThreads();
  //Creating the histogram manager for the specific test
  PixelHistoManager* theHistoManager = 0;
  if(calibrationType == "SCurve"){
		if(!thePixelXmlReader.tagExist("SCurveCuts")){
			cout << mthn << "Cannot process data since the xml file " << document << " doesn't have the tag SCurveCuts! Plese read the README file and add the tag Cuts!" << endl;
			exit(0);
		}
    theHistoManager = new PixelSCurveHistoManager(&thePixelXmlReader,&thePixelCalibConfiguration,&thePixelConfigurationsManager,logger);
  }else if(calibrationType == "PixelAlive"){
		if(!thePixelXmlReader.tagExist("PixelAliveCuts")){
			cout << mthn << "Cannot process data since the xml file " << document << " doesn't have the tag PixelAliveCuts! Plese read the README file and add the tag Cuts!" << endl;
			exit(0);
		}
    theHistoManager = new PixelAliveHistoManager(&thePixelXmlReader,&thePixelCalibConfiguration,&thePixelConfigurationsManager,logger);
  }else if(calibrationType == "Gain"){
		if(!thePixelXmlReader.tagExist("GainCuts")){
			cout << mthn << "Cannot process data since the xml file " << document << " doesn't have the tag GainCuts! Plese read the README file and add the tag Cuts!" << endl;
			exit(0);
		}
    theHistoManager = new PixelGainHistoManager(&thePixelXmlReader,&thePixelCalibConfiguration,&thePixelConfigurationsManager,logger);
  }
  theHistoManager->init();

  cout << mthn << "Running " <<  calibrationType << " analisys..." << endl;
  PixelAnalyzer thePixelAnalyzer(theHistoManager,&thePixelCalibConfiguration,&thePixelConfigurationsManager,logger);
  thePixelAnalyzer.loopOverDataFile(numberOfEvents);
  theHistoManager->makeSummaryPlots();
  theHistoManager->saveHistos();
  string interactiveSession = thePixelXmlReader.getXMLAttribute("Session"        ,"Interactive");
	if(interactiveSession == "yes"){
		bool quit = false;
		char quitChar;
		while(!quit){
	  	cout << "******************************************************" << endl;
	  	cout << mthn << "Press q <enter> to quit:" << endl;
			cin >> quitChar; 
			if(quitChar == 'q'){
				quit = true;
			}
		}
	}
  histogramServer.stopThreads();

  delete theHistoManager;
  return 0;
}

