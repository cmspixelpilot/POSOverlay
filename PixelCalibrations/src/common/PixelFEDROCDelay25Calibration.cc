/*************************************************************************
 * PixelFEDROCDelay25Calibration Class,                                  *
 * class to read and evaluate last DAC readings from the ROCs after      *
 * different Delay25 settings and different DAC settings have been set   *
 * by the PixelROCDelay25Calibration class                               *
 *                                                                       *
 * Author: James Zabel, Rice University                                  *
 *                                                                       *
 * Last update: $Date: 2012/06/16 14:13:20 $ (UTC)                       *
 *          by: $Author: mdunser $                                        *
 *************************************************************************/

#include "PixelCalibrations/include/PixelFEDROCDelay25Calibration.h"
#include "PixelUtilities/PixelFEDDataTools/include/LastDACFIFODecoder.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "CalibFormats/SiPixelObjects/interface/PixelPortcardMap.h"
#include "CalibFormats/SiPixelObjects/interface/PixelPortCardSettingNames.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TLine.h"
#include <cmath>

using namespace std;
using namespace pos;

const float NUM_DELAY_DIVISIONS_PER_NS = 2.;

PixelFEDROCDelay25Calibration::PixelFEDROCDelay25Calibration(const PixelFEDSupervisorConfiguration &tempConfiguration, SOAPCommander *mySOAPCmdr)
    : PixelFEDCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  cout << "[PixelFEDROCDelay25Calibration]:  Greetings from the PixelFEDROCDelay25Calibration constructor." << endl;
}

xoap::MessageReference PixelFEDROCDelay25Calibration::beginCalibration(xoap::MessageReference msg)
{
cout << "[PixelFEDROCDelay25Calibration]:  beginCalibration entered" << endl;
//collect info from the calib object
  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
    assert(tempCalibObject != 0);

//instantiation and assignment of variables used
  fedsAndChannels_ = tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_);
  vector <PixelROCName> theROCs = tempCalibObject->rocList();
  delay25Min_ = atoi(tempCalibObject->parameterValue("Delay25Min").c_str()) / NUM_DELAY_DIVISIONS_PER_NS;
  delay25Max_ = atoi(tempCalibObject->parameterValue("Delay25Max").c_str()) / NUM_DELAY_DIVISIONS_PER_NS;
  delay25StepSize_ = atoi(tempCalibObject->parameterValue("Delay25StepSize").c_str()) / NUM_DELAY_DIVISIONS_PER_NS;
  numDelaySettings_ = int(1 + ((delay25Max_ - delay25Min_) / delay25StepSize_));
  minRange_ = atoi(tempCalibObject->parameterValue("MinimumRange").c_str()) / NUM_DELAY_DIVISIONS_PER_NS;
  numDACSettings_ = atoi(tempCalibObject->parameterValue("NumDACSettings").c_str());
  writeToFile_ = atoi(tempCalibObject->parameterValue("WriteToFile").c_str());
  unsigned int scanMin = atoi(tempCalibObject->parameterValue("ScanMin").c_str()),
               scanMax = atoi(tempCalibObject->parameterValue("ScanMax").c_str()),
               stepSize = atoi(tempCalibObject->parameterValue("StepSize").c_str()),
               i_FED = 0, i_Channel = 0;
  string tempString, newTempString;

//open file for collection of raw data
  if(writeToFile_ == 1){
    struct stat buffer;
    if(stat((outputDir() + "/" + "ROCDelay25TestResults_" + itoa(crate_) + ".txt").c_str(), &buffer) == 0){
      remove((outputDir() + "/" + "ROCDelay25TestResults_" + itoa(crate_) + ".txt").c_str());
    }
    ROCDelay25OutputFile_.open((outputDir() + "/" + "ROCDelay25TestResults_" + itoa(crate_) + ".txt").c_str(), ios::out);
  }

//set the DAC Level settings from the configuration file
  if(stepSize == (scanMax - scanMin)){
    for(unsigned int i = 0; i < numDACSettings_; ++i){
      if((i % 2) == 0){
        DACLevels_.push_back(itoa(scanMin));
      } else{
        DACLevels_.push_back(itoa(scanMax));
      }
    }
  } else {
    for(unsigned int i = 0; i < numDACSettings_; ++i){
      DACLevels_.push_back(itoa(scanMin + ((stepSize * i) % (scanMax - scanMin))));
    }
  }

//assign values to the PortCardMap
  PixelConfigInterface::get(thePortCardMap_, "pixel/portcardmap/", *theGlobalKey_);
    assert(theGlobalKey_ != 0);
  set <string> thePortCards =  thePortCardMap_->portcards(theDetectorConfiguration_);
  set <string>::const_iterator aPCName;
  PixelPortCardConfig* tempPortCard = 0;
  for(aPCName = thePortCards.begin(); aPCName != thePortCards.end(); aPCName++){
    PixelConfigInterface::get(tempPortCard, "pixel/portcard/" + *aPCName, *theGlobalKey_);
      assert(tempPortCard != 0);
    tempPortCard->writeASCII(outputDir());
    delete tempPortCard;
  }
//instantiates histograms, assigns invalid initial previous DAC Setting, loop through all ROCs to create a map of port card names
//to the ROCs they control, and initialize the delay to use by port card to -1
  map <PixelROCName, int> theROCsMap;
  vector <PixelROCName>::iterator aROCName_iter;
  for(aROCName_iter = theROCs.begin(); aROCName_iter != theROCs.end(); ++aROCName_iter){
    theROCsMap[*aROCName_iter] = 1;
  }

  for(i_FED = 0; i_FED < fedsAndChannels_.size(); ++i_FED){
    unsigned int fedNumber = fedsAndChannels_[i_FED].first;
    for(i_Channel = 0; i_Channel < fedsAndChannels_[i_FED].second.size(); ++i_Channel){
      unsigned int channel = fedsAndChannels_[i_FED].second[i_Channel];
      vector <PixelROCName> ROCsOnThisChannel = theNameTranslation_->getROCsFromFEDChannel(fedNumber, channel);
      for(aROCName_iter = ROCsOnThisChannel.begin(); aROCName_iter != ROCsOnThisChannel.end(); ++aROCName_iter){
        if(theROCsMap.find(*aROCName_iter) != theROCsMap.end()){
          newTempString = aROCName_iter->rocname().substr(0, aROCName_iter->rocname().find("_ROC"));
          newTempString = newTempString.substr(0, newTempString.find("_PLQ"));
          thePortCardMap_->getName(newTempString, tempString);
          theROCsOnThisFEDMap_[tempString][*aROCName_iter] = 1;
          theROCsOnThisFED_.push_back(*aROCName_iter);
          previousDACSetting_[tempString][*aROCName_iter] = -1;
          myHighestDACValueByROC_[tempString][*aROCName_iter] = 0;
          for(float i_Delay = delay25Min_; i_Delay <= delay25Max_; ++i_Delay){
            myCollapsedTestResultsByROC_[tempString][*aROCName_iter][i_Delay] = 0;
          }
        }
      }
    }
  }
cout << "[PixelFEDROCDelay25Calibration]:  beginCalibration exited" << endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("beginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDROCDelay25Calibration::execute(xoap::MessageReference msg)
{
cout << "[PixelFEDROCDelay25Calibration]:  execute entered" << endl;
//instantiation of paramter arrays used by SOAP messages, then receive parameters
  Attribute_Vector parametersReceived(3);
    parametersReceived.at(0).name_ = "Delay25Setting";
    parametersReceived.at(1).name_ = "DACSetting";;
    parametersReceived.at(2).name_ = "RunNumber";
  Receive(msg, parametersReceived);

//instantiation and assignment of variables used
  float delay25Setting = atoi(parametersReceived.at(0).value_.c_str()) / NUM_DELAY_DIVISIONS_PER_NS;
  unsigned int DACSetting = atoi(parametersReceived.at(1).value_.c_str()),
               runNumber = atoi(parametersReceived.at(2).value_.c_str());

//write raw data to file
  if(writeToFile_ == 1){
    ROCDelay25OutputFile_ << "Delay25Setting:" << endl << delay25Setting << endl << "DACValue:" << endl << DACSetting << endl;
  }

//loop through FED interfaces, then through ROCs, to read the last DAC value
  readAssignLastDAC(runNumber, delay25Setting);
cout << "[PixelFEDROCDelay25Calibration]:  execute exited" << endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("FEDCalibrationsDone");
  return reply;
}

xoap::MessageReference PixelFEDROCDelay25Calibration::endCalibration(xoap::MessageReference msg)
{
cout << "[PixelFEDROCDelay25Calibration]:  endCalibration entered" << endl;
//instantiation of variables used
  unsigned int numROCs = 0, maxNumberOfROCsPassing = 0, tempUnInt = 0;
  int tempInt = 0, counter = 0, rocCounter = 0;
  float cutoffLimitByPC = 0, cutoffLimitByROC = 0, tempFloat = 0, tempDelay = 0, i_Delay = 0;
  string tempString;
  map <string, map <PixelROCName, TH2F*> > testResultsByROC;
  map <string, map <PixelROCName, TH1D*> > collapsedTestResultsByROC;
  map <string, TH2F*> completeResultsByPortCard;
//instatiation of iterators used
  map <string, map <PixelROCName, map <float, vector <int> > > >::iterator aPC_iter;
  map <PixelROCName, map <float, vector <int> > >::iterator aROC_iter;
  map <float, vector <int> >::iterator delay_iter;
  vector <float>::iterator aBD_iter;                               //for vector of bestDelays
  map <float, float>::iterator scnd_map_ADBR_iter;                 //for second map of acceptableDelaysByROC
  map <float, pair <float, unsigned int> >::iterator map_DD_iter;  //for delayData

//loop through the port cards
  for(aPC_iter = myTestResultsByROC_.begin(); aPC_iter != myTestResultsByROC_.end(); ++aPC_iter){
//instantiate data structures used only within this scope
    ofstream error_file;
    error_file.open((outputDir() + "/FAILURE_DUMP_FILE_" + itoa(crate_) + ".txt").c_str());
    error_file << "[PixelFEDROCDelay25Calibration]:  beginning evaluation of port card " << aPC_iter->first << endl;
    map <float, pair <float, unsigned int> > delayData;      //<delay time, <range, number of rocs passing> >
    map <float, map <float, int> > collapsedResultsByDelay;  //<delay time, summed values onto delay of passing rocs at this delay time>
    vector <float> bestDelays;
    maxNumberOfROCsPassing = 0;
    delayToUseByPortCard_[aPC_iter->first] = -1;
    currentDelayByPortCard_[aPC_iter->first] = -1;
    numROCs = aPC_iter->second.size();
    rocCounter = 1;
    tempString = "2D Hist of Delay Results for Port Card " + aPC_iter->first;
    completeResultsByPortCard[aPC_iter->first] = new TH2F(tempString.c_str(), tempString.c_str(), numROCs, 0.5, numROCs + 0.5,
                                                          numDelaySettings_, delay25Min_ - (0.5 * delay25StepSize_), 
                                                          delay25Max_ + (0.5 * delay25StepSize_));
    completeResultsByPortCard[aPC_iter->first]->GetXaxis()->SetTitle("ROC Name");
    completeResultsByPortCard[aPC_iter->first]->GetYaxis()->SetTitle("Delay (ns)");
//loop to create a new map and delay data for each delay, and then initialize the delayData map
    for(i_Delay = delay25Min_; i_Delay <= delay25Max_; i_Delay += delay25StepSize_){
      delayData[i_Delay] = pair <float, unsigned int> (0, numROCs);
      for(float i = delay25Min_; i <= delay25Max_; i += delay25StepSize_){
        collapsedResultsByDelay[i_Delay][i] = 0;
      }
    }
    error_file << "[PixelFEDROCDelay25Calibration]:  finished data instatiation..." << endl;
//collapse the results onto delay, then compare each bin with the cutoff limit to create acceptable delay values by ROC (start time and end time)
//loop through the ROCs controlled by this port card
    cutoffLimitByPC = 0;
    for(aROC_iter = aPC_iter->second.begin(); aROC_iter != aPC_iter->second.end(); ++aROC_iter){
      tempString = "Raw Data for ROC " + aROC_iter->first.rocname();
      testResultsByROC[aPC_iter->first][aROC_iter->first] = new TH2F(tempString.c_str(), tempString.c_str(), numDelaySettings_, 
                                                                     delay25Min_ - (0.5 * delay25StepSize_), delay25Max_ + (0.5 * delay25StepSize_), 
                                                                     numDACSettings_ - 1, -0.5, numDACSettings_ - 1.5);
      testResultsByROC[aPC_iter->first][aROC_iter->first]->GetXaxis()->SetTitle("Delay (ns)");
      testResultsByROC[aPC_iter->first][aROC_iter->first]->GetYaxis()->SetTitle("DAC Values");
      tempString = "Projection onto Delay for ROC " + aROC_iter->first.rocname();
      collapsedTestResultsByROC[aPC_iter->first][aROC_iter->first] = new TH1D(tempString.c_str(), tempString.c_str(), numDelaySettings_, 
                                                                              delay25Min_ - (0.5 * delay25StepSize_), delay25Max_ + (0.5 * delay25StepSize_));
      collapsedTestResultsByROC[aPC_iter->first][aROC_iter->first]->GetXaxis()->SetTitle("Delay (ns)");
      completeResultsByPortCard[aPC_iter->first]->GetXaxis()->SetBinLabel(rocCounter, aROC_iter->first.rocname().c_str());
      rocCounter += 1;
      cutoffLimitByROC = .95 * myHighestDACValueByROC_[aPC_iter->first][aROC_iter->first] * (numDACSettings_ - 1);
      cutoffLimitByPC += cutoffLimitByROC;
      tempDelay = delay25Min_;
      counter = 0;
//loop through bins of collapsed hists
      for(delay_iter = aROC_iter->second.begin(); delay_iter != aROC_iter->second.end(); ++delay_iter){
        completeResultsByPortCard[aPC_iter->first]->Fill(aROC_iter->first.rocname().c_str(), delay_iter->first, 
                                                         myCollapsedTestResultsByROC_[aPC_iter->first][aROC_iter->first][delay_iter->first]);
        collapsedTestResultsByROC[aPC_iter->first][aROC_iter->first]->Fill(delay_iter->first, myCollapsedTestResultsByROC_[aPC_iter->first][aROC_iter->first][delay_iter->first]);
        for(unsigned int dac_iter = 1; dac_iter < DACLevels_.size(); ++dac_iter){
          testResultsByROC[aPC_iter->first][aROC_iter->first]->Fill(delay_iter->first, dac_iter - 1, delay_iter->second[dac_iter - 1]);
          testResultsByROC[aPC_iter->first][aROC_iter->first]->GetYaxis()->SetBinLabel(dac_iter, DACLevels_[dac_iter].c_str());
        }
        if(myCollapsedTestResultsByROC_[aPC_iter->first][aROC_iter->first][delay_iter->first] > cutoffLimitByROC){
          if(counter == 0){
            acceptableDelaysByROC_[aPC_iter->first][aROC_iter->first][tempDelay] = tempDelay;
          } else {
            acceptableDelaysByROC_[aPC_iter->first][aROC_iter->first][tempDelay - counter * delay25StepSize_] += delay25StepSize_;
          }
          counter += 1;
          for(i_Delay = delay25Min_; i_Delay <= delay25Max_; i_Delay += delay25StepSize_){
            collapsedResultsByDelay[delay_iter->first][i_Delay] += myCollapsedTestResultsByROC_[aPC_iter->first][aROC_iter->first][i_Delay];
          }
        } else {
          counter = 0;
          delayData[tempDelay].second -= 1;
        }
        tempDelay += delay25StepSize_;
      }
    }
    cutoffLimitByPC /= numROCs;
    error_file << "[PixelFEDROCDelay25Calibration]:  finished collapsing results and creating acceptable delays by roc..." << endl;
//loop to assign delay data; the range for each delay and the number of ROCs passing at that delay
//then determine the delays falling within the acceptable minimum range value to assign best Delays
    for(map_DD_iter = delayData.begin(); map_DD_iter != delayData.end(); ++map_DD_iter){
      tempFloat = cutoffLimitByPC * float(map_DD_iter->second.second);
      counter = 1;
      tempDelay = map_DD_iter->first;
      error_file << "collapsedResultsByDelay[" << map_DD_iter->first << "][" << map_DD_iter->first << "]:  " << collapsedResultsByDelay[map_DD_iter->first][map_DD_iter->first] << " <= " << tempFloat << endl;
      if(collapsedResultsByDelay[map_DD_iter->first][map_DD_iter->first] <= tempFloat){
        map_DD_iter->second.first = 0;
      } else {
        while( (collapsedResultsByDelay[map_DD_iter->first][tempDelay] > tempFloat) && (tempDelay >= delay25Min_) ){
          error_file << "below...  collapsedResultsByDelay[" << map_DD_iter->first << "][" << tempDelay << "]:  " << collapsedResultsByDelay[map_DD_iter->first][tempDelay] << " <= " << tempFloat << endl;
          tempDelay -= delay25StepSize_;
          counter -= 1;
        }
        map_DD_iter->second.first = abs(counter * delay25StepSize_);
        counter = 0;
        tempDelay = map_DD_iter->first + delay25StepSize_;
        while( (collapsedResultsByDelay[map_DD_iter->first][tempDelay] > tempFloat) && (tempDelay <= delay25Max_) ){
          error_file << "above...  collapsedResultsByDelay[" << map_DD_iter->first << "][" << tempDelay << "]:  " << collapsedResultsByDelay[map_DD_iter->first][tempDelay] << " <= " << tempFloat << endl;
          tempDelay += delay25StepSize_;
          counter += 1;
        }
        if(map_DD_iter->second.first > (counter * delay25StepSize_)){
          map_DD_iter->second.first = counter * delay25StepSize_;
        }
        if(map_DD_iter->second.first >= minRange_){
          bestDelays.push_back(map_DD_iter->first);
        }
      }
      error_file << "delayData:  " << endl << "delay:  " << map_DD_iter->first << ", range:  " << delayData[map_DD_iter->first].first << ", number of ROCs passing:  " << delayData[map_DD_iter->first].second << endl;
    }
    error_file << "[PixelFEDROCDelay25Calibration]:  finished assigning delay data and creating suggested delays..." << endl;
//loop to determine the maximum number of rocs passing.
    for(aBD_iter = bestDelays.begin(); aBD_iter != bestDelays.end(); ++aBD_iter){
      if(maxNumberOfROCsPassing <= delayData[*aBD_iter].second){
        maxNumberOfROCsPassing = delayData[*aBD_iter].second;
      }
    }
//loop to determine which delay with the maximum number of rocs passing has the largest range.
    tempDelay = 0;
    for(aBD_iter = bestDelays.begin(); aBD_iter != bestDelays.end(); ++aBD_iter){
      if(maxNumberOfROCsPassing == delayData[*aBD_iter].second){
        error_file << "bestDelays:  " << endl << "delay:  " << *aBD_iter << ", range:  " << delayData[*aBD_iter].first << endl;
        if(tempDelay <= delayData[*aBD_iter].first){
          tempDelay = delayData[*aBD_iter].first;
          delayToUseByPortCard_[aPC_iter->first] = *aBD_iter;
        }
      }
    }
    error_file << "[PixelFEDROCDelay25Calibration]:  finished determining which delays have the most rocs passing..." << endl;
//write the new port card config file
    PixelPortCardConfig* tempPortCard = 0;
    PixelConfigInterface::get(tempPortCard, "pixel/portcard/" + aPC_iter->first, *theGlobalKey_);
      assert(tempPortCard != 0);
    tempUnInt = tempPortCard->getdeviceValuesForSetting(PortCardSettingNames::k_Delay25_SDA);
    tempInt = tempUnInt ^ 64;  //64 is 0x40 in decimal 
    currentDelayByPortCard_[aPC_iter->first] = tempInt / NUM_DELAY_DIVISIONS_PER_NS;
cout << "[PixelFEDROCDelay25Calibration]:  port card " << aPC_iter->first << " is using delay: " << currentDelayByPortCard_[aPC_iter->first] << endl;
cout << "[PixelFEDROCDelay25Calibration]:  port card " << aPC_iter->first << " should use delay: " << delayToUseByPortCard_[aPC_iter->first] << endl;
error_file << "[PixelFEDROCDelay25Calibration]:  port card " << aPC_iter->first << " is using delay: " << currentDelayByPortCard_[aPC_iter->first] << endl;
error_file << "[PixelFEDROCDelay25Calibration]:  port card " << aPC_iter->first << " should use delay: " << delayToUseByPortCard_[aPC_iter->first] << endl;
    if(delayToUseByPortCard_[aPC_iter->first] != -1){
      tempUnInt = int(delayToUseByPortCard_[aPC_iter->first] * NUM_DELAY_DIVISIONS_PER_NS);
      tempPortCard->setdeviceValues(PortCardSettingNames::k_Delay25_SDA, (tempUnInt & 0x3F) | 0x40);  //the & 0x3F | 0x40 sets the enable bit
      tempPortCard->writeASCII(outputDir());
      delete tempPortCard;
    } else {
      rename((outputDir() + "/FAILURE_DUMP_FILE_" + itoa(crate_) + ".txt").c_str(), (outputDir() + "/error_report_for_port_card_" + aPC_iter->first + ".txt").c_str());
      tempPortCard->writeASCII(outputDir());
      delete tempPortCard;
    }
  }
  remove((outputDir() + "/FAILURE_DUMP_FILE_" + itoa(crate_) + ".txt").c_str());

//write root files for the histo viewer
  TFile *ROCDelay25RootFile = new TFile((outputDir() + "/" + "ROCDelay25_" + itoa(crate_) + ".root").c_str(), "recreate", ("ROCDelay25_" + itoa(crate_) + ".root").c_str());
  PixelRootDirectoryMaker *dirMakerPIX = new PixelRootDirectoryMaker(theROCsOnThisFED_, ROCDelay25RootFile);
  TDirectory *completeHists = ROCDelay25RootFile->mkdir("Complete Results by Port Card", "Hists of all ROCs by Port Card");
  TDirectory *summaryTrees = ROCDelay25RootFile->mkdir("SummaryTrees", "SummaryTrees");
  summaryTrees->cd();
  ROCDelay25Branch theBranch;
  TTree *tree = new TTree("ROCDelay25Summary", "ROCDelay25Summary");
  tree->Branch("ROCDelay25", &theBranch, "pass/F:delaySetting/F:offsetFromIdealDelay/F:offsetFromCurrentDelay/F:rocName/C", 4096000);
  completeHists->cd();
//loop to assign pass and fail data to each ROC on a tree in the root file
  for(aPC_iter = myTestResultsByROC_.begin(); aPC_iter != myTestResultsByROC_.end(); ++aPC_iter){
    numROCs = aPC_iter->second.size();
    completeHists->cd();
    TCanvas *tempCanvas = new TCanvas(completeResultsByPortCard[aPC_iter->first]->GetName(), 
                                      completeResultsByPortCard[aPC_iter->first]->GetName());
    TLine *tempLineFoundDelay = new TLine(-0.5, delayToUseByPortCard_[aPC_iter->first], 
                                          numROCs - 0.5, delayToUseByPortCard_[aPC_iter->first]);
    tempLineFoundDelay->SetLineColor(2);
    tempLineFoundDelay->SetLineWidth(2);
    tempLineFoundDelay->SetLineStyle(9);
    TLine *tempLineCurrentDelay = new TLine(-0.5, currentDelayByPortCard_[aPC_iter->first], 
                                            numROCs - 0.5, currentDelayByPortCard_[aPC_iter->first]);
    tempLineCurrentDelay->SetLineColor(4);
    tempLineCurrentDelay->SetLineWidth(2);
    tempLineCurrentDelay->SetLineStyle(9);
    tempCanvas->cd();
    completeResultsByPortCard[aPC_iter->first]->Draw();
    tempLineFoundDelay->Draw();
    tempLineCurrentDelay->Draw();
    tempCanvas->Write();
    for(aROC_iter = aPC_iter->second.begin(); aROC_iter != aPC_iter->second.end(); ++aROC_iter){  //loop through the ROCs
      summaryTrees->cd();
      tempString = aROC_iter->first.rocname();
      strcpy(theBranch.rocName, tempString.c_str());                                    //assigns ROC name on a new branch
      if(acceptableDelaysByROC_[aPC_iter->first][aROC_iter->first].size() == 0){                                   //assigns corresponding data if ROC fails
        theBranch.pass = 0;
        theBranch.delaySetting = -1;
        theBranch.offsetFromIdealDelay = -1;
        theBranch.offsetFromCurrentDelay = currentDelayByPortCard_[aPC_iter->first];
      } else {                                                                                    //loop over acceptable values to ensure ROC passes
        for(scnd_map_ADBR_iter = acceptableDelaysByROC_[aPC_iter->first][aROC_iter->first].begin(); 
            scnd_map_ADBR_iter != acceptableDelaysByROC_[aPC_iter->first][aROC_iter->first].end(); ++scnd_map_ADBR_iter){
          if( (scnd_map_ADBR_iter->first <= delayToUseByPortCard_[aPC_iter->first]) &&
              (delayToUseByPortCard_[aPC_iter->first] <= scnd_map_ADBR_iter->second) ){           //assigns corresponding data if ROC passes
            theBranch.pass = 1;
            theBranch.delaySetting = delayToUseByPortCard_[aPC_iter->first];
            theBranch.offsetFromIdealDelay = abs( ((scnd_map_ADBR_iter->first + scnd_map_ADBR_iter->second) / 2) 
                                                  - delayToUseByPortCard_[aPC_iter->first]);
            theBranch.offsetFromCurrentDelay = abs(theBranch.offsetFromIdealDelay - currentDelayByPortCard_[aPC_iter->first]);
            break;
          }
        }
      }
      tree->Fill();
      dirMakerPIX->cdDirectory(aROC_iter->first.rocname().c_str());
      TCanvas *tempCanvas1 = new TCanvas(testResultsByROC[aPC_iter->first][aROC_iter->first]->GetName(), 
                                         testResultsByROC[aPC_iter->first][aROC_iter->first]->GetName());
      TLine *tempLineFoundDelay1 = new TLine(delayToUseByPortCard_[aPC_iter->first], -0.5, 
                                             delayToUseByPortCard_[aPC_iter->first], numDACSettings_ - 1.5);
      tempLineFoundDelay1->SetLineColor(2);
      tempLineFoundDelay1->SetLineWidth(2);
      tempLineFoundDelay1->SetLineStyle(9);
      TLine *tempLineCurrentDelay1 = new TLine(currentDelayByPortCard_[aPC_iter->first], -0.5,
                                               currentDelayByPortCard_[aPC_iter->first], numDACSettings_ - 1.5);
      tempLineCurrentDelay1->SetLineColor(4);
      tempLineCurrentDelay1->SetLineWidth(2);
      tempLineCurrentDelay1->SetLineStyle(9);
      tempCanvas1->cd();
      testResultsByROC[aPC_iter->first][aROC_iter->first]->Draw();
      tempLineFoundDelay1->Draw();
      tempLineCurrentDelay1->Draw();
      tempCanvas1->Write();
      TCanvas *tempCanvas2 = new TCanvas(collapsedTestResultsByROC[aPC_iter->first][aROC_iter->first]->GetName(), 
                                         collapsedTestResultsByROC[aPC_iter->first][aROC_iter->first]->GetName());
      TLine *tempLineFoundDelay2 = new TLine(delayToUseByPortCard_[aPC_iter->first], 
                                             collapsedTestResultsByROC[aPC_iter->first][aROC_iter->first]->GetBinLowEdge(
                                             collapsedTestResultsByROC[aPC_iter->first][aROC_iter->first]->FindBin(delayToUseByPortCard_[aPC_iter->first])), 
                                             delayToUseByPortCard_[aPC_iter->first], 
                                             collapsedTestResultsByROC[aPC_iter->first][aROC_iter->first]->GetBinContent(
                                             collapsedTestResultsByROC[aPC_iter->first][aROC_iter->first]->FindBin(delayToUseByPortCard_[aPC_iter->first])));
      tempLineFoundDelay2->SetLineColor(2);
      tempLineFoundDelay2->SetLineWidth(2);
      tempLineFoundDelay2->SetLineStyle(9);
      TLine *tempLineCurrentDelay2 = new TLine(currentDelayByPortCard_[aPC_iter->first], 
                                             collapsedTestResultsByROC[aPC_iter->first][aROC_iter->first]->GetBinLowEdge(
                                             collapsedTestResultsByROC[aPC_iter->first][aROC_iter->first]->FindBin(delayToUseByPortCard_[aPC_iter->first])), 
                                             currentDelayByPortCard_[aPC_iter->first], 
                                             collapsedTestResultsByROC[aPC_iter->first][aROC_iter->first]->GetBinContent(
                                             collapsedTestResultsByROC[aPC_iter->first][aROC_iter->first]->FindBin(delayToUseByPortCard_[aPC_iter->first])));
      tempLineCurrentDelay2->SetLineColor(4);
      tempLineCurrentDelay2->SetLineWidth(2);
      tempLineCurrentDelay2->SetLineStyle(9);
      tempCanvas2->cd();
      collapsedTestResultsByROC[aPC_iter->first][aROC_iter->first]->Draw();
      tempLineFoundDelay2->Draw();
      tempLineCurrentDelay2->Draw();
      tempCanvas2->Write();
    }
  }
  delete thePortCardMap_;

//write and close root file
  ROCDelay25RootFile->Write();
  ROCDelay25RootFile->Close();
cout << "[PixelFEDROCDelay25Calibration]:  endCalibration exited" << endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

PixelFEDROCDelay25Calibration::~PixelFEDROCDelay25Calibration()
{
cout << "[PixelFEDROCDelay25Calibration]:  deconstructor entered." << endl;
//close file used for raw data collection
  if(writeToFile_ == 1){
    ROCDelay25OutputFile_.close();
  }
cout << "[PixelFEDROCDelay25Calibration]:  deconstructor exited." << endl;
}

void PixelFEDROCDelay25Calibration::initializeFED()
{
  setFEDModeAndControlRegister(0x8, 0x30010);
}

void PixelFEDROCDelay25Calibration::readAssignLastDAC(unsigned int passedRunNumber, float passedDelay25Setting)
{
  float delay25Setting = passedDelay25Setting;
  unsigned int runNumber = passedRunNumber, i_FED = 0;
  int tempDiff = 0;
  string tempPCName, tempString;
  PixelROCName tempROCName;
  map <string, map <PixelROCName, int> > tempDACValuesByPC;
  list <PixelLastDACReading>::const_iterator list_LDACR_iter;  //for lastDACReadings

  for(i_FED = 0; i_FED < fedsAndChannels_.size(); ++i_FED){                                                       //loop to iterate through FED interfaces
    unsigned int fedNumber = fedsAndChannels_[i_FED].first;
    uint32_t vmeBaseAddress = theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fedNumber);
    PixelFEDInterface *fed_interface = FEDInterface_[vmeBaseAddress];
    uint32_t buffer[1024];
    unsigned int numWords = fed_interface->drainLastDACFifo(buffer);
    LastDACFIFODecoder lastDACInfo(buffer, numWords);
    const list <PixelLastDACReading>& lastDACReadings = lastDACInfo.getLastDACReading();
    for(list_LDACR_iter = lastDACReadings.begin(); list_LDACR_iter != lastDACReadings.end(); ++list_LDACR_iter){  //loop to read DAC values of ROCs
      if(theNameTranslation_->ROCNameFromFEDChannelROCExists(fedNumber, list_LDACR_iter->getFEDChannel(),         //verify ROC exists with FEDnumber, channel, and ROCID info
                                                             list_LDACR_iter->getReadOutChipId() - 1)){           //minus one becasue theNameTranslation begins counting from one
        tempROCName = theNameTranslation_->ROCNameFromFEDChannelROC(fedNumber, list_LDACR_iter->getFEDChannel(), 
                                                                    list_LDACR_iter->getReadOutChipId() - 1);
        tempString = tempROCName.rocname().substr(0, tempROCName.rocname().find("_ROC"));
        tempString = tempString.substr(0, tempString.find("_PLQ"));
        thePortCardMap_->getName(tempString, tempPCName);
        tempDACValuesByPC[tempPCName][tempROCName] = list_LDACR_iter->getDACValue();
      } else {
        cout << "[PixelFEDROCDelay25Calibration]:  BAD ROC DATA IN LAST DAC FIFO" << endl;
        cout << "     FED#:  " <<  fedNumber << ", FEDChannel:  " <<  list_LDACR_iter->getFEDChannel() 
             << ", ROCID:  " <<  list_LDACR_iter->getReadOutChipId() - 1 << endl;
        lastDACInfo.printBuffer(cout);
      }
    }                                                                                                             //end of loop retrieving DAC values for ROCs
  }                                                                                                               //end of loop iterating through FED interfaces

  map <string, map <PixelROCName, int> >::iterator aPC_iter;
  map <PixelROCName, int>::iterator aROC_iter;
  if(writeToFile_ == 1){
    if(runNumber != 0){                                                                                           //can't evaluate a change if first run
      for(aPC_iter = theROCsOnThisFEDMap_.begin(); aPC_iter != theROCsOnThisFEDMap_.end(); ++aPC_iter){
        for(aROC_iter = aPC_iter->second.begin(); aROC_iter != aPC_iter->second.end(); ++aROC_iter){
          ROCDelay25OutputFile_ << aROC_iter->first << "  " << tempDACValuesByPC[aPC_iter->first][aROC_iter->first] << endl;
          tempDiff = abs(tempDACValuesByPC[aPC_iter->first][aROC_iter->first] - previousDACSetting_[aPC_iter->first][aROC_iter->first]);
          myTestResultsByROC_[aPC_iter->first][aROC_iter->first][delay25Setting][runNumber - 1] = tempDiff;
          myCollapsedTestResultsByROC_[aPC_iter->first][aROC_iter->first][delay25Setting] += tempDiff;
          previousDACSetting_[aPC_iter->first][aROC_iter->first] = tempDACValuesByPC[aPC_iter->first][aROC_iter->first];
          if(tempDiff > myHighestDACValueByROC_[aPC_iter->first][aROC_iter->first]){
            myHighestDACValueByROC_[aPC_iter->first][aROC_iter->first] = tempDiff;
          }
        }
      }
    } else {                                                                                                      //end if runNumber != 0
      for(aPC_iter = theROCsOnThisFEDMap_.begin(); aPC_iter != theROCsOnThisFEDMap_.end(); ++aPC_iter){
        for(aROC_iter = aPC_iter->second.begin(); aROC_iter != aPC_iter->second.end(); ++aROC_iter){
          for(unsigned int i = 0; i < numDACSettings_ - 1; i++){
            myTestResultsByROC_[aPC_iter->first][aROC_iter->first][delay25Setting].push_back(-1);
          }
          ROCDelay25OutputFile_ << aROC_iter->first << "  " << tempDACValuesByPC[aPC_iter->first][aROC_iter->first] << endl;
          previousDACSetting_[aPC_iter->first][aROC_iter->first] = tempDACValuesByPC[aPC_iter->first][aROC_iter->first];
        }
      }
    }                                                                                                             //end else runNumber != 0
  } else {                                                                                                        //end if writeToFile == 1
    if(runNumber != 0){                                                                                           //can't evaluate a change if first run
      for(aPC_iter = theROCsOnThisFEDMap_.begin(); aPC_iter != theROCsOnThisFEDMap_.end(); ++aPC_iter){
        for(aROC_iter = aPC_iter->second.begin(); aROC_iter != aPC_iter->second.end(); ++aROC_iter){
          tempDiff = abs(tempDACValuesByPC[aPC_iter->first][aROC_iter->first] - previousDACSetting_[aPC_iter->first][aROC_iter->first]);
          myTestResultsByROC_[aPC_iter->first][aROC_iter->first][delay25Setting][runNumber - 1] = tempDiff;
          myCollapsedTestResultsByROC_[aPC_iter->first][aROC_iter->first][delay25Setting] += tempDiff;
          previousDACSetting_[aPC_iter->first][aROC_iter->first] = tempDACValuesByPC[aPC_iter->first][aROC_iter->first];
          if(tempDiff > myHighestDACValueByROC_[aPC_iter->first][aROC_iter->first]){
            myHighestDACValueByROC_[aPC_iter->first][aROC_iter->first] = tempDiff;
          }
        }
      }
    } else {                                                                                                      //end if runNumber != 0
      for(aPC_iter = theROCsOnThisFEDMap_.begin(); aPC_iter != theROCsOnThisFEDMap_.end(); ++aPC_iter){
        for(aROC_iter = aPC_iter->second.begin(); aROC_iter != aPC_iter->second.end(); ++aROC_iter){
          for(unsigned int i = 0; i < numDACSettings_ - 1; i++){
            myTestResultsByROC_[aPC_iter->first][aROC_iter->first][delay25Setting].push_back(-1);
          }
          previousDACSetting_[aPC_iter->first][aROC_iter->first] = tempDACValuesByPC[aPC_iter->first][aROC_iter->first];
        }
      }
    }                                                                                                             //end else runNumber != 0
  }                                                                                                               //end else writeToFile == 1
}

