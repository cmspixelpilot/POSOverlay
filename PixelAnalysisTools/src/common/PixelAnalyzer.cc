#include "PixelAnalysisTools/include/PixelAnalyzer.h"
#include "PixelAnalysisTools/include/PixelHistoManager.h"
// #include "PixelAnalysisTools/include/PixelSCurveHistoManager.h"
#include "PixelAnalysisTools/include/PixelConfigurationsManager.h"
#include "PixelAnalysisTools/include/PixelCalibConfigurationExtended.h"
#include <string>
#include <iostream>
#include <unistd.h>
#include <time.h>
#include <vector>
#include <fstream>
#include <math.h>

using namespace std;
using namespace pos;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PixelAnalyzer::PixelAnalyzer(){

}

PixelAnalyzer::PixelAnalyzer(PixelHistoManager * pixelHistoManager, 
			     PixelCalibConfigurationExtended *pixelCalib, 
			     PixelConfigurationsManager *pixelConfigurationsManager,
			     ostream *logger){
  logger_ = logger;
  thePixelHistoManager_ = pixelHistoManager;
  thePixelConfigurationsManager_ = pixelConfigurationsManager;
  thePixelCalib_ = pixelCalib;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PixelAnalyzer::~PixelAnalyzer(){
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// void PixelAnalyzer::loopOverDataFile(int nEventsToProcess){
// }
void PixelAnalyzer::loopOverDataFile(int nEventsToProcess){
  bool PRINT = false;
  string mthn = "[PixelAnalyzer::loopOverDataFile()]\t";

  //Variables to measure timings
  time_t start,end;
  time_t startPattern,endPattern;
  time_t lastTimeUpdate;
  double dif;

  unsigned int numberOfTriggersPerPattern=thePixelCalib_->nTriggersPerPattern();
  *logger_ << mthn << "Ntriggers per pattern=" << numberOfTriggersPerPattern << endl;

  unsigned int numberOfWBCPerPattern;
  if(thePixelCalib_->containsScan("WBC")){
    numberOfWBCPerPattern=thePixelCalib_->nScanPoints("WBC");
  }
  else{
    numberOfWBCPerPattern=1;
  }
  cout << "Number of WBC per pattern: " << numberOfWBCPerPattern << endl;
  unsigned int numberOfVCalPerPattern=thePixelCalib_->nScanPoints("Vcal" );
  *logger_ << mthn << " # scan points=" << numberOfVCalPerPattern << endl;

//  unsigned int vcalMin  = int(thePixelCalib_->scanValueMin("Vcal"));
//  unsigned int vcalMax  = int(thePixelCalib_->scanValueMax("Vcal"));
//  unsigned int vcalStep = int(thePixelCalib_->scanValueStep("Vcal"));
//  *logger_ << mthn << "VCal => min=" << vcalMin << " max=" << vcalMax  << " step=" << vcalStep << endl;

  unsigned int vcalValue=0;
  //event loop: read data
  cout << mthn << "Starting event loop..." << endl;		   

  multimap<string,unsigned int> theFileMap = thePixelConfigurationsManager_->getDataFileMap();
  unsigned int numberOfFiles        = theFileMap.size();
  unsigned int numberOfPatterns     = thePixelCalib_->nPixelPatterns()*numberOfFiles;
  if(nEventsToProcess != -1){
    numberOfPatterns = (numberOfFiles*nEventsToProcess)/(numberOfTriggersPerPattern*numberOfVCalPerPattern);
  }
  if(PRINT) cout << mthn 
		 << "patternleft: " << numberOfPatterns 
		 << " numberOfFiles: " << numberOfFiles 
		 << " nEventsToProcess: " << nEventsToProcess 
		 << " thePixelCalib_->nPixelPatterns(): " << thePixelCalib_->nPixelPatterns() 
		 << " numberOfTriggersPerPattern: " << numberOfTriggersPerPattern 
		 << endl;

  unsigned int numberOfPatternsLeft = numberOfPatterns;
  double       elapsedTime          = 0;
  unsigned int fileNumber           = 0;
  time (&start);

  
  for(multimap<string,unsigned int>::iterator it=theFileMap.begin(); it!=theFileMap.end(); ++it, ++fileNumber){
    unsigned int numberOfFeds = findNumberOfFeds(it->first);
//     unsigned int numberOfFeds = 2;
    
    if(PRINT) cout<<" File "<<fileNumber<<" feds "<<numberOfFeds<<endl;

    ifstream dataFile;
    dataFile.open(it->first.c_str(),ios::binary|ios::in);
    if(!dataFile.is_open()){
      cout << mthn << "Can't open file: " << it->first << "...Check the file name for fed " << it->second << endl;
      continue;
    }   

    SLinkDecoder sLinkDecoder(&dataFile);				   
    Word64 runNumber;						   
    sLinkDecoder.getNextWord64(runNumber);
		thePixelHistoManager_->setRunNumber((unsigned int)runNumber.getWord());
    *logger_ << mthn << " Run Number " << runNumber.getWord() << endl;	   

    vector<PixelSLinkEvent> sLinkEvents;
    vector<unsigned int> firstCounterValues;
    for(unsigned int fedCounter=0; fedCounter<numberOfFeds;++fedCounter){
      PixelSLinkEvent tmp;
      sLinkEvents.push_back(tmp);
      firstCounterValues.push_back(0);
    }
    unsigned int eventNumber    = 0;
    bool newPattern = true;
    unsigned int eventsInAPattern  = numberOfTriggersPerPattern*numberOfVCalPerPattern*numberOfWBCPerPattern;
    unsigned int maxTriggerCounterValue = (unsigned int)pow(2.,24.);
    time (&startPattern);
    time (&lastTimeUpdate);
		double deltaPatterns = 0;
    while(readEvent(sLinkDecoder,sLinkEvents) && eventNumber<(unsigned int)nEventsToProcess){ 
      if(PRINT) cout<<" event number "<<eventNumber<<endl;

      if(eventNumber == 0){
	for(unsigned int fedCounter=0; fedCounter<numberOfFeds;++fedCounter){
	  firstCounterValues[fedCounter] = sLinkEvents[fedCounter].getHeader().getLV1_id();
	}
      }
      if (eventNumber%eventsInAPattern==0) {
	newPattern =true;
      }


      for(unsigned int fedCounter=0; fedCounter<numberOfFeds;++fedCounter){
	if(PRINT) cout<<" FED 1 loop "<<fedCounter<<endl;

	if(eventNumber%maxTriggerCounterValue != (sLinkEvents[fedCounter].getHeader().getLV1_id()-firstCounterValues[fedCounter])%maxTriggerCounterValue){
	  *logger_ << mthn << "An event was missing...resyncing"
		   << " Event: "  << eventNumber+1
		   << " != LV1_id: " << sLinkEvents[fedCounter].getHeader().getLV1_id()-firstCounterValues[fedCounter]
		   << endl;
	  //This part doesn't work if there are more feds in the same file...you need to do everthing for the other feds
	  // and ignore the one that is screwed...
	  eventNumber = sLinkEvents[fedCounter].getHeader().getLV1_id()-firstCounterValues[fedCounter];
	  if (eventNumber%eventsInAPattern==0) {
	    newPattern =true;
	  }
	}
      }


      if (newPattern) {

	if(PRINT) cout<<" enter new pattern "<<numberOfPatterns<<" "<<numberOfPatternsLeft<<endl;

	newPattern = false;
	if (eventNumber != 0) {
	  --numberOfPatternsLeft;
	  ++deltaPatterns;

	  thePixelHistoManager_->endOfPatternAnalysis();//fit histos so far and fill summary plots

	  time (&endPattern);
	  dif = difftime (endPattern,startPattern);
	  elapsedTime = difftime (endPattern,start);

	  if( difftime(endPattern,lastTimeUpdate) >= 5){
	    //if( difftime(endPattern,lastTimeUpdate) >= 0){
	    double timeLeft = (numberOfPatternsLeft * (elapsedTime/(numberOfPatterns-numberOfPatternsLeft)));
	    int minutesLeft = (int)timeLeft/60;
	    int secondsLeft = (int)(timeLeft-minutesLeft*60);
	    cout << mthn << "Time remaining: " <<  minutesLeft << "m:" << secondsLeft << "s!" << endl;
	    if(PRINT) cout << mthn << "Each pattern takes " << difftime(endPattern,lastTimeUpdate)/deltaPatterns << "s time: " 
			   << difftime(endPattern,lastTimeUpdate) << " delta: " 
			   << deltaPatterns << " left: " << numberOfPatternsLeft << endl;
	    deltaPatterns = 0;
	    time(&lastTimeUpdate);
	  }
	  time (&startPattern);
	}

	//set histo for new cells
	thePixelConfigurationsManager_->setPatternToExpect(eventNumber);

	thePixelHistoManager_->setPatternHistos(eventNumber);     

      }

      if(numberOfVCalPerPattern > 1 && eventNumber%(numberOfTriggersPerPattern)==0) {
	//vcalValue= thePixelCalib_->scanCounter("Vcal",eventNumber/numberOfTriggersPerPattern)*vcalStep+vcalMin;//SCurve type histos
	vcalValue= thePixelCalib_->scanValue("Vcal",eventNumber/numberOfTriggersPerPattern);//SCurve type histos
      }

      if(eventNumber == 0){
	for(unsigned int fedCounter=0; fedCounter<numberOfFeds;++fedCounter){
	  if(PRINT) cout<<" FED 2 loop "<<fedCounter<<endl;
	  if(firstCounterValues[fedCounter] != 1){
	    *logger_ << mthn << "The first event of fed " << sLinkEvents[fedCounter].getHeader().getSource_id()
		     << " has the counter that doesn't start from 1 but it starts at " << sLinkEvents[fedCounter].getHeader().getLV1_id() 
		     << endl;
            eventNumber = sLinkEvents[fedCounter].getHeader().getLV1_id()-1;
            firstCounterValues[fedCounter] = eventNumber;
            continue;
	  }
	}
      }

      for(unsigned int fedCounter=0; fedCounter<numberOfFeds;++fedCounter){
	if(PRINT) cout<<" FED 3 loop "<<fedCounter<<endl;

	//get fedid and skip if not in [fedmin,fedmax]
	int fed = sLinkEvents[fedCounter].getHeader().getSource_id();
	
	vector<PixelHit> hits           = sLinkEvents[fedCounter].getHits();
	vector<PixelHit>::iterator ihit = hits.begin();

	//loop over the hits in the event
	for (;ihit!=hits.end();++ihit) {
	  //get channel, roc, row, col
	  unsigned int channel= ihit->getLink_id();
	  unsigned int roc    = ihit->getROC_id()-1;
	  if(!thePixelConfigurationsManager_->isDataToAnalyze(fed,channel,roc)){continue;}
	  if(!thePixelConfigurationsManager_->dataIsExpected(fed,channel,roc)){continue;}
	  
	  unsigned int row    = ihit->getRow();
	  unsigned int col    = ihit->getColumn();
	  if(!thePixelConfigurationsManager_->isAnAllowedPixel(row,col)){continue;}
	  if(!thePixelConfigurationsManager_->patternIsExpected(fed,channel,roc,row,col)){

	    thePixelHistoManager_->setWrongAddress(fed,channel,roc);

	    if(PRINT) cout << mthn << "Event: " << eventNumber << " fed: " << fed << " chan: " << channel 
			   << " roc: " << roc << " row: " << row << " col: " << col << endl;
	    continue;
	  }
	  //get WBC and skip if not wanted 
	  //-- could be made faster by figuring out in advance from calib file which events to run on 
	  int desiredWBC = thePixelConfigurationsManager_->getWBCChoice();//could move to constructor
	  if(desiredWBC>=0 && desiredWBC<=255){
	    int thiswbc =  thePixelCalib_->scanValue("WBC",eventNumber/numberOfTriggersPerPattern);
	    if(thiswbc != desiredWBC){continue;}
	  }
	  
	  //fill histograms
	  if(numberOfVCalPerPattern > 1) {
	    unsigned int adc    = ihit->getADC();
	    thePixelHistoManager_->fillHistos(fed,channel,roc,row,col,vcalValue,adc);//SCurve type histo
	  } else {
	    thePixelHistoManager_->fillHistos(fed,channel,roc,row,col);//PixelAlive type
	  }

	} 

      }//end fed count loop
      ++eventNumber;
    }//cout << "end of while loop" << endl;
    

    thePixelHistoManager_->endOfPatternAnalysis();//fit histos so far and fill summary plots
    dataFile.close();

//     break;//TO BE REMOVED
    *logger_ << mthn << "...Processed:" << eventNumber << " triggers" <<endl;

  }//end map loop

  time (&end);
  dif = difftime (end,start);
  int totalMinutes = (int)dif/60;
  int totalSeconds = (int)(dif-totalMinutes*60);
  cout << mthn << "Total time elapsed: " << totalMinutes << "m:" << totalSeconds << "s!" << endl;

  thePixelHistoManager_->endOfFileAnalysis();//fit histos so far and fill summary plots
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PixelAnalyzer::readEvent(SLinkDecoder &sLinkDecoder, vector<PixelSLinkEvent> &sLinkEvents){
  for(vector<PixelSLinkEvent>::iterator it=sLinkEvents.begin(); it!=sLinkEvents.end(); ++it){
    if(sLinkDecoder.getNextEvent(*it)){
      it->decodeEvent();
    }
    else{
      return false;
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int PixelAnalyzer::findNumberOfFeds(string fileName){
  string mthn = "[PixelAnalyzer::findNumberOfFeds()]\t";
  ifstream dataFile;
  dataFile.open(fileName.c_str(),ios::binary|ios::in);
  if(!dataFile.is_open()){
    cout << mthn << "Can't open file: " << fileName << endl;
    return 0;
  }
  SLinkDecoder sLinkDecoder(&dataFile);				   
  map<int,bool> fedMap;

  Word64 runNumber;						   
  sLinkDecoder.getNextWord64(runNumber);			   

  PixelSLinkEvent pixelSLinkEvent;
  sLinkDecoder.getNextEvent(pixelSLinkEvent);
  pixelSLinkEvent.decodeEvent();
  int fed = pixelSLinkEvent.getHeader().getSource_id();
  fedMap[fed] = true;
  while(sLinkDecoder.getNextEvent(pixelSLinkEvent)){
    pixelSLinkEvent.decodeEvent();
    fed = pixelSLinkEvent.getHeader().getSource_id();
    if(fedMap.find(fed) != fedMap.end()){
      break;
    }
    else{
      fedMap[fed] = true;
    }
  }
  dataFile.close();
  return fedMap.size();
}

