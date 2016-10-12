#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"
#include "PixelCalibrations/include/PixelFEDTBMDelayCalibrationBPIX.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"
#include "PixelUtilities/PixelFEDDataTools/include/ErrorFIFODecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/ColRowAddrDecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/DigScopeDecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/DigTransDecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO3Decoder.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include "PixelFEDInterface/include/PixelPh1FEDInterface.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TTree.h"
#include <iomanip>
#include <algorithm>

using namespace pos;

///////////////////////////////////////////////////////////////////////////////////////////////
PixelFEDTBMDelayCalibrationBPIX::PixelFEDTBMDelayCalibrationBPIX(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr)
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr), rootf(0)
{
  std::cout << "In PixelFEDTBMDelayCalibrationBPIX copy ctor()" << std::endl;
}

///////////////////////////////////////////////////////////////////////////////////////////////
void PixelFEDTBMDelayCalibrationBPIX::initializeFED() {
  setFEDModeAndControlRegister(0x8, 0x30010);
  
}

///////////////////////////////////////////////////////////////////////////////////////////////
xoap::MessageReference PixelFEDTBMDelayCalibrationBPIX::beginCalibration(xoap::MessageReference msg) {
  std::cout << "In PixelFEDTBMDelayCalibrationBPIX::beginCalibration()" << std::endl;

  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  tempCalibObject->writeASCII(outputDir());

  DumpFIFOs = tempCalibObject->parameterValue("DumpFIFOs") == "yes";
  ReadFifo1 = tempCalibObject->parameterValue("ReadFifo1") == "yes";

  //lc//if( ReadFifo1 ) setFIFO1Mode();//Sets a control register in the VME FED to block the transfer from FIFO1 to FIFO3. Might not be needed for mTCA

  inject_ = false;
  const std::vector<std::vector<unsigned int> > cols = tempCalibObject->columnList();
  const std::vector<std::vector<unsigned int> > rows = tempCalibObject->rowList();
  if( cols[0].size() != 0 && rows[0].size() != 0 ) inject_ = true;

  for (unsigned dacnum = 0; dacnum < tempCalibObject->numberOfScanVariables(); ++dacnum) {
    const std::string& dacname = tempCalibObject->scanName(dacnum);
    std::vector<unsigned int> dacvals = tempCalibObject->scanValues(dacname);
    if (dacvals.size() > 1)
      dacsToScan.push_back(dacname);

    for( unsigned int i = 0; i < dacvals.size(); ++i ) std::cout << " dac value " << i << " is " << dacvals[i] << std::endl;
  }

  BookEm("");

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

///////////////////////////////////////////////////////////////////////////////////////////////
xoap::MessageReference PixelFEDTBMDelayCalibrationBPIX::execute(xoap::MessageReference msg) {
  Attribute_Vector parameters(2);
  parameters[0].name_ = "WhatToDo";
  parameters[1].name_ = "StateNum";
  Receive(msg, parameters);

  const unsigned state = atoi(parameters[1].value_.c_str());

  if (parameters[0].value_ == "RetrieveData")
    RetrieveData(state);
  else if (parameters[0].value_ == "Analyze")
    Analyze();
  else {
    cout << "ERROR: PixelFEDTBMDelayCalibrationBPIX::execute() does not understand the WhatToDo command, "<< parameters[0].value_ <<", sent to it.\n";
    assert(0);
  }

  xoap::MessageReference reply = MakeSOAPMessageReference("FEDCalibrationsDone");
  return reply;
}

///////////////////////////////////////////////////////////////////////////////////////////////
xoap::MessageReference PixelFEDTBMDelayCalibrationBPIX::endCalibration(xoap::MessageReference msg) {

  std::cout << "In PixelFEDTBMDelayCalibrationBPIX::endCalibration()" << std::endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

///////////////////////////////////////////////////////////////////////////////////////////////
void PixelFEDTBMDelayCalibrationBPIX::RetrieveData(unsigned state) {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  const std::vector<std::pair<unsigned, std::vector<unsigned> > >& fedsAndChannels = tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_);

  if (DumpFIFOs) std::cout << "NEW FEDTBMDelay TRIGGER " << event_ << " state " << state << " ";
  std::map<std::string, unsigned int> currentDACValues;
  for (unsigned dacnum = 0; dacnum < tempCalibObject->numberOfScanVariables(); ++dacnum) {
    const std::string& dacname = tempCalibObject->scanName(dacnum);
    const unsigned dacvalue = tempCalibObject->scanValue(tempCalibObject->scanName(dacnum), state);
    currentDACValues[dacname] = dacvalue;
    if (DumpFIFOs) std::cout << dacname << " " << dacvalue << " ";
  }
  if (DumpFIFOs) std::cout << std::endl;
  if(dacsToScan.size() < 2 && currentDACValues["TBMPLL"] != lastTBMPLL){
   event_ = 0;
   lastTBMPLL = currentDACValues["TBMPLL"];
  }

  //////

  for (unsigned ifed = 0; ifed < fedsAndChannels.size(); ++ifed) {
    const unsigned fednumber = fedsAndChannels[ifed].first;
    const unsigned long vmeBaseAddress = theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
    // PixelFEDInterfaceBase* iFED = FEDInterface_[vmeBaseAddress];
    PixelPh1FEDInterface* iFED = dynamic_cast<PixelPh1FEDInterface*>(FEDInterface_[vmeBaseAddress]);
  
    //lc//iFED->readDigFEDStatus(false, false);

    const int MaxChans = 37;    
    uint32_t bufferFifo1[MaxChans][1024];
    int statusFifo1[MaxChans] = {0};

  
    
    // read fifo1 
    if( ReadFifo1 ){


      //lc//
      //PixelPh1FEDInterface::digfifo1 d = iFED->readFIFO1(true);


      

      //  for (int fib = 0; fib < 24; ++fib) {
      //if (!fibers_in_use[fib+1]) continue;
      //  cout << "Reading fiber " << fib << endl;
      // iFED->setFIFO1(fib);     
      // PixelPh1FEDInterface::digfifo1 d = iFED->readFIFO1(false);





      //printf("ch a: %i ch b: %i n tbm h a: %i b: %i  tbm t a: %i b: %i  roc h a: %i b: %i\n", d.a.ch, d.b.ch, d.a.n_tbm_h, d.b.n_tbm_h, d.a.n_tbm_t, d.b.n_tbm_t, d.a.n_roc_h, d.b.n_roc_h);
   

      for( unsigned int ch = 0; ch < fedsAndChannels[ifed].second.size(); ch++ ){
        int channel = (fedsAndChannels[ifed].second)[ch];

	///lea und malte 11.10.2016
	if (channel%2==1) {
	  int fib = (channel-1)/2; 
	  cout << "Reading fiber " << fib << endl;
	  iFED->setFIFO1(fib);     
	  PixelPh1FEDInterface::digfifo1 d = iFED->readFIFO1(false);
	  printf("ch a: %i ch b: %i n tbm h a: %i b: %i  tbm t a: %i b: %i  roc h a: %i b: %i\n", d.a.ch, d.b.ch, d.a.n_tbm_h, d.b.n_tbm_h, d.a.n_tbm_t, d.b.n_tbm_t, d.a.n_roc_h, d.b.n_roc_h);
	  unsigned int chA = d.a.ch;
	  unsigned int chB = d.b.ch;
        
	  if((channel != chA) || ((channel+1) != chB)) continue;
        
	  bool found_TBM = d.a.n_tbm_h && d.a.n_tbm_t && d.b.n_tbm_h && d.b.n_tbm_t;
	  //bool ch_foundHit = ((d.a.n_roc_h ) && ())
	  cout << "Malte: found TBM for fiber " << fib+1 << "!!!" << endl;
	  cout << "fillhisto for fed " << fedsAndChannels[ifed].first << endl;

	  FillEm(state, fedsAndChannels[ifed].first, chA, 0, (/*!inject_ && */found_TBM)/* || (inject_ && found_TBM && ch_foundHit)*/ );
	  FillEm(state, fedsAndChannels[ifed].first, chB, 0, (/*!inject_ && */found_TBM)/* || (inject_ && found_TBM && ch_foundHit)*/ );
	  
	  if( dacsToScan.size() == 0 ){
	    //bool ch_foundROC = false;
	    //ch_foundROC = (d.a.n_roc_h == 8);
	    //ch_foundROC = (d.b.n_roc_h == 8);
	    
	    // no information which ROCs have been found. Just FillEm if all are found:
	    //if( ch_foundROC ){
	    for( int r = 0; r < d.a.n_roc_h; ++r ){;
	      FillEm(state, fedsAndChannels[ifed].first, chA, 1, r);
	    } 
	    for( int r = 0; r < d.b.n_roc_h; ++r ){;
	      FillEm(state, fedsAndChannels[ifed].first, chB, 1, r);
	    }
	      //}
	  }
	  else if( dacsToScan.size() == 1){;
	    FillEm(state, fedsAndChannels[ifed].first, chA, 1, d.a.n_roc_h);
	    FillEm(state, fedsAndChannels[ifed].first, chB, 1, d.b.n_roc_h);
          }
	}//if statement for fibers
      }// end loop on channels
      //}// end loop over fibers
      
    }//end readFifo1 
    
  }//end loop on feds

  event_++;
  //sendResets();

}

///////////////////////////////////////////////////////////////////////////////////////////////
void PixelFEDTBMDelayCalibrationBPIX::Analyze() {

  int ntriggers = event_-1;
  if (dacsToScan.size() == 0){

    for( std::map<int,std::map<int,std::vector<TH1F*> > >::iterator it1 = ntrigsTBM.begin(); it1 != ntrigsTBM.end(); ++it1 ){
     for( std::map<int,std::vector<TH1F*> >::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2 ){
      for(unsigned int i = 0; i < it2->second.size(); ++i ) it2->second[i]->Scale(1./ntriggers);
     }
    }
    CloseRootf();
    return;
  }

  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);
  const std::vector<std::pair<unsigned, std::vector<unsigned> > >& fedsAndChannels = tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_);

  std::map<std::string,int> bestTBMPLLSettings;
  std::map<std::string,int> currentTBMPLLdelay;
  std::map<std::string,int> passState;
  std::map<std::string,double> effForCurrentTBMPLLdelay;
  std::map<std::string,double> effForBestTBMPLLdelay;
  std::map<std::string,int> nFEDchannelsPerModule;
  std::map<std::string,std::map<int,int> > nGoodBinsPerModule;  

  //normalize by number of triggers
  for( std::map<int,std::map<int,std::vector<TH2F*> > >::iterator it1 = scansTBM.begin(); it1 != scansTBM.end(); ++it1 ){
   for( std::map<int,std::vector<TH2F*> >::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2 ){
    for(unsigned int i = 0; i < it2->second.size(); ++i ) it2->second[i]->Scale(1./ntriggers);
   }
  }

  //fill histo with sum of channels
  for( std::map<int,std::map<int,std::vector<TH2F*> > >::iterator it1 = scansTBM.begin(); it1 != scansTBM.end(); ++it1 ){
   std::string moduleName = "";
   PixelChannel theChannel;
   for( std::map<int,std::vector<TH2F*> >::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2 ){  

    if( theNameTranslation_->FEDChannelExist(it1->first, it2->first) ){
     theChannel = theNameTranslation_->ChannelFromFEDChannel(it1->first, it2->first);
     moduleName = theChannel.modulename();
    }

    if( nFEDchannelsPerModule.find(moduleName) == nFEDchannelsPerModule.end() ){
      nFEDchannelsPerModule[moduleName] = 1;
    }
    else {
      nFEDchannelsPerModule[moduleName] += 1;
      }
    //count bins with eff = 100%
    int nGoodBins = 0;     
    for( int bx = 1; bx < it2->second[0]->GetNbinsX()+1; ++bx ){
     for( int by = 1; by < it2->second[0]->GetNbinsY()+1; ++by ){ 
      if( it2->second[0]->GetBinContent(bx,by) == 1 ) nGoodBins++;
     }//close loop on by
    }//close loop on bx   
    nGoodBinsPerModule[moduleName][it2->first] = nGoodBins;

    for(unsigned int i = 0; i < it2->second.size(); ++i ){ TBMsHistoSum[moduleName][i]->Add(it2->second[i]); }

   }//close loop on channels
  }//close loop on fed

  //normalize the sum histo by number of triggers and number of fed channels
  for( std::map<std::string,std::vector<TH2F*> >::iterator it = TBMsHistoSum.begin(); it != TBMsHistoSum.end(); ++it ){
   for( unsigned int i = 0; i < it->second.size(); ++i ){
     //M.B.: normalization should be done for TBM Header/Trailer only, not for received ROC headers
     if (string(it->second[i]->GetTitle()).find("nROCHeaders") != string::npos) continue;
    
     it->second[i]->Scale(1./(nFEDchannelsPerModule[it->first])); 
   }
  }

  //find best settings for each module
  for( std::map<std::string,std::vector<TH2F*> >::iterator it = TBMsHistoSum.begin(); it != TBMsHistoSum.end(); ++it ){
     
   /*check if the current TBMPLL delay value gives 100% efficiency*/
   PixelTBMSettings *TBMSettingsForThisModule=0;
   std::string moduleName=it->first;//(module_name->modulename());
   PixelConfigInterface::get(TBMSettingsForThisModule, "pixel/tbm/"+moduleName, *theGlobalKey_);
   assert(TBMSettingsForThisModule!=0);

   currentTBMPLLdelay[moduleName] = TBMSettingsForThisModule->getTBMPLLDelay();

   int delayX = (currentTBMPLLdelay[moduleName]&28)>>2;
   int delayY = (currentTBMPLLdelay[moduleName]&224)>>5;
   effForCurrentTBMPLLdelay[moduleName] = it->second[0]->GetBinContent(delayX+1,delayY+1);
   
   // if efficiency for current tbmpll delay != 100% search for new best value; if efficiency == 100% keep the current value
   if( effForCurrentTBMPLLdelay[moduleName] == 1 ){
    passState[moduleName] = 1;
    bestTBMPLLSettings[moduleName] = currentTBMPLLdelay[moduleName];
    effForBestTBMPLLdelay[moduleName] = effForCurrentTBMPLLdelay[moduleName];
   }
   else{

    //find efficiency = 100%
    std::map<int,int> bestBins;
    for( int bx = 1; bx < it->second[0]->GetNbinsX()+1; ++bx ){
     for( int by = 1; by < it->second[0]->GetNbinsY()+1; ++by ){ 
      if( it->second[0]->GetBinContent(bx,by) == 1 ) bestBins[bx-1] = by-1;  //try first to find 100% eff bins
     }//close loop on by
    }//close loop on bx

    if( bestBins.size() == 0 ){//or find max efficiency

     double eff = 0;
     int binxbest = 0;
     int binybest = 0;
     double maxEff = 0;
     for( int bx = 1; bx < it->second[0]->GetNbinsX()+1; ++bx ){
      for( int by = 1; by < it->second[0]->GetNbinsY()+1; ++by ){ 
       if( it->second[0]->GetBinContent(bx,by) >= eff ){
        eff = it->second[0]->GetBinContent(bx,by);
        binxbest = bx-1;
        binybest = by-1;
       }
      }//close loop on by
     }//close loop on bx

     bestBins[binxbest] = binybest; 
     maxEff = it->second[0]->GetBinContent(binxbest+1,binybest+1);
     //check if there are other bins with same max eff
     for( int bx = 1; bx < it->second[0]->GetNbinsX()+1; ++bx ){
      for( int by = 1; by < it->second[0]->GetNbinsY()+1; ++by ){ 

       if( it->second[0]->GetBinContent(bx,by) == maxEff ){       
        std::map<int,int>::iterator binsIt = bestBins.find(bx-1);
        if( binsIt->first != binxbest && binsIt->second != binybest ) bestBins[bx-1] = by-1;
       }

      }//close loop on by
     }//close loop on bx       
    }//close bestBins.size() == 0

    if( bestBins.size() == 0 ) passState[moduleName] = 0;//at this point this should not be true
    else{
     int bestX = 0;
     int bestY = 0;
     for( std::map<int,int>::iterator binsIt = bestBins.begin(); binsIt != bestBins.end(); ++binsIt ){
      if( binsIt->first >= 0 && binsIt->first < 8 && binsIt->second > 0 && binsIt->second < 8 ){
       bestX = binsIt->first;
       bestY = binsIt->second;
       break;
      }
     }//close loop on best bins

     if( bestX == 0 && bestY == 0 ){
      bestX = (bestBins.begin())->first;
      bestY = (bestBins.begin())->second;
     }

     if( it->second[0]->GetBinContent(bestX+1,bestY+1) != 1 ) passState[moduleName] = 0;
     else passState[moduleName] = 1;
     effForBestTBMPLLdelay[moduleName] = it->second[0]->GetBinContent(bestX+1,bestY+1); 
     bestX = (bestX<<2);
     bestY = (bestY<<5);
     bestTBMPLLSettings[moduleName] = bestX+bestY;

    }//close case bestBins!=0

   }//close case eff!=1

   //check that the module has enough good bins for each fed channel otherwise set flag = 0
   for( std::map<int,int>::iterator gb = nGoodBinsPerModule[moduleName].begin(); 
        gb != nGoodBinsPerModule[moduleName].end(); ++gb ){
        if( gb->second < 10 ){passState[moduleName] = 0;break;}
   }
  
  }//close loop on modules

  rootf->cd();
  branch theBranch;
  branch_sum theBranch_sum;
  TDirectory* dirSummaries = gDirectory->mkdir("SummaryTrees","SummaryTrees");
  dirSummaries->cd();

  TTree* tree = new TTree("PassState","PassState");
  TTree* tree_sum =new TTree("SummaryInfo","SummaryInfo");
  
  tree->Branch("PassState",&theBranch,"pass/F:moduleName/C",4096000);
  tree_sum->Branch("SummaryInfo",&theBranch_sum,"deltaTBMPLLdelayX/I:deltaTBMPLLdelayY/I:newTBMPLLdelayX/I:newTBMPLLdelayY/I:efficiency/D:moduleName/C",4096000);
  rootf->cd();

  ofstream out((outputDir()+"/summary.txt").c_str()); //leave the file method intact for now
  assert(out.good()); //file method

  //std::vector<PixelModuleName>::const_iterator module_name = theDetectorConfiguration_->getModuleList().begin();
  //for (;module_name!=theDetectorConfiguration_->getModuleList().end();++module_name){
  for( std::map<std::string,std::vector<TH2F*> >::iterator it = TBMsHistoSum.begin(); it != TBMsHistoSum.end(); ++it ){

   std::string moduleName=it->first;//(module_name->modulename());

   int oldDelayX = (currentTBMPLLdelay[moduleName]&28)>>2;
   int oldDelayY = (currentTBMPLLdelay[moduleName]&224)>>5;
   int newDelayX = (bestTBMPLLSettings[moduleName]&28)>>2;
   int newDelayY = (bestTBMPLLSettings[moduleName]&224)>>5;

   theBranch_sum.deltaTBMPLLdelayX = newDelayX-oldDelayX;
   theBranch_sum.deltaTBMPLLdelayY = newDelayY-oldDelayY;
   theBranch_sum.newTBMPLLdelayX = newDelayX;
   theBranch_sum.newTBMPLLdelayY = newDelayY;
   theBranch_sum.efficiency = effForBestTBMPLLdelay[moduleName];
   strcpy(theBranch_sum.moduleName,moduleName.c_str());
   theBranch.pass = passState[moduleName];
   strcpy(theBranch.moduleName,moduleName.c_str());

   tree->Fill();
   tree_sum->Fill();

   PixelTBMSettings *TBMSettingsForThisModule=0;
   PixelConfigInterface::get(TBMSettingsForThisModule, "pixel/tbm/"+moduleName, *theGlobalKey_);
   assert(TBMSettingsForThisModule!=0);
   TBMSettingsForThisModule->setTBMPLLDelay(bestTBMPLLSettings[moduleName]);
   TBMSettingsForThisModule->writeASCII(outputDir());
   //std::cout << "Wrote TBM settings for module:" << moduleName << endl;			
   delete TBMSettingsForThisModule;

  }

  CloseRootf();

  //now print summary and save it on text file
  std::map<std::string,std::vector<int> > FEDchannelsPerModule;
  for (unsigned ifed = 0; ifed < fedsAndChannels.size(); ++ifed) {
   std::string moduleName = "";
   for( unsigned int ch = 0; ch < fedsAndChannels[ifed].second.size(); ch++ ){
    PixelChannel theChannel = theNameTranslation_->ChannelFromFEDChannel(fedsAndChannels[ifed].first, (fedsAndChannels[ifed].second)[ch]);
    moduleName = theChannel.modulename();
    FEDchannelsPerModule[moduleName].push_back((fedsAndChannels[ifed].second)[ch]);
   }
  }

  out << "Module                        | FED channels | 400MHz phase | 160MHz phase | DeltaTBMPLLDelay | Efficiency | Pass | \n";
  for( std::map<std::string,std::vector<int> >::iterator it = FEDchannelsPerModule.begin(); it != FEDchannelsPerModule.end(); ++it ){
   out << it->first << " | ";
   for( unsigned int i = 0; i < (it->second).size(); ++i ) out << (it->second)[i] << " ";
   out << " | " << ((bestTBMPLLSettings[it->first]&28)>>2) << "            | " << ((bestTBMPLLSettings[it->first]&224)>>5);
   out << "            | " << bestTBMPLLSettings[it->first] - currentTBMPLLdelay[it->first];
   out.precision(4);
   out << "              | " << effForBestTBMPLLdelay[it->first] << "          | " << passState[it->first] << "    | \n";
  }

}

///////////////////////////////////////////////////////////////////////////////////////////////
void PixelFEDTBMDelayCalibrationBPIX::CloseRootf() {
  if (rootf) {
    rootf->Write();
    rootf->Close();
    delete rootf;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////
void PixelFEDTBMDelayCalibrationBPIX::BookEm(const TString& path) {

  ntrigsTBM.clear();
  scansTBM.clear();
  TBMsHistoSum.clear();

  TString root_fn;
  if (path == "")
    root_fn.Form("%s/TBMDelay.root", outputDir().c_str());
  else
    root_fn.Form("%s/TBMDelay_%s.root", outputDir().c_str(), path.Data());
  cout << "writing histograms to file " << root_fn << endl;
  CloseRootf();
  rootf = new TFile(root_fn, "create");
  assert(rootf->IsOpen());

  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);
  const std::vector<std::pair<unsigned, std::vector<unsigned> > >& fedsAndChannels = tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_);

  PixelRootDirectoryMaker rootDirs(fedsAndChannels,gDirectory);

  for (unsigned ifed = 0; ifed < fedsAndChannels.size(); ++ifed) {

   std::map<int,std::vector<TH1F*> > chTBMmap;
   std::map<int,std::vector<TH2F*> > chTBMmap2D;
   for( unsigned int ch = 0; ch < fedsAndChannels[ifed].second.size(); ch++ ){

    rootDirs.cdDirectory(fedsAndChannels[ifed].first, (fedsAndChannels[ifed].second)[ch]);

    if (dacsToScan.size() == 0){

     TString hname; hname.Form("Ch%i",(fedsAndChannels[ifed].second)[ch]);
     std::vector<TH1F*> histosTBM;
     TH1F* h_TBM_nDecodes = new TH1F(hname+"_TBM_nDecodes", hname+"_TBM_nDecodes", 2, 0, 2 );
     histosTBM.push_back(h_TBM_nDecodes);
     TH1F* h_nROCHeaders = new TH1F(hname+"_nROCHeaders", hname+"_nROCHeaders", 9, 0, 9 );
     histosTBM.push_back(h_nROCHeaders);  
     chTBMmap[(fedsAndChannels[ifed].second)[ch]] = histosTBM;
  
    }// end book histos for zero dacsToScan case

    if(dacsToScan.size() == 1){

      TString hname; hname.Form("Ch%i",(fedsAndChannels[ifed].second)[ch]);
      std::vector<TH2F*> histosTBM;
      TH2F* h_TBM_nDecodes = new TH2F(hname+"_TBM_nDecodes", hname+"_TBM_nDecodes", 8, 0, 8, 8, 0, 8 );
      //h_TBM_nDecodes->SetXTitle("400 MHz phase");
      //h_TBM_nDecodes->SetYTitle("160 MHz phase");
      histosTBM.push_back(h_TBM_nDecodes);
      TH2F* h_nROCHeaders = new TH2F(hname+"_nROCHeaders", hname+"_nROCHeaders", 8, 0, 8, 8, 0, 8 );
      //h_nROCHeaders->SetXTitle("400 MHz phase");
      //h_nROCHeaders->SetYTitle("160 MHz phase");
      histosTBM.push_back(h_nROCHeaders);     
      chTBMmap2D[(fedsAndChannels[ifed].second)[ch]] = histosTBM;
  
    }// end book histos for 1 dacsToScan case (TBMPLL scan)

   }//close loop on channels

   if (dacsToScan.size() == 0) ntrigsTBM[fedsAndChannels[ifed].first] = chTBMmap;
   if (dacsToScan.size() == 1) scansTBM[fedsAndChannels[ifed].first] = chTBMmap2D;

  }//close loop on feds

  rootf->cd();

  //book histos with sum of channels
  if (dacsToScan.size() == 1){

   std::set<PixelChannel> names = theNameTranslation_->getChannels(); 
   PixelRootDirectoryMaker rootDirsModules(names,gDirectory);

   for(std::set<PixelChannel>::iterator PixelChannel_itr=names.begin(), PixelChannel_itr_end = names.end();PixelChannel_itr!=PixelChannel_itr_end; ++PixelChannel_itr){

    rootDirsModules.cdDirectory((*PixelChannel_itr));
    std::string moduleName = (*PixelChannel_itr).modulename();
    std::vector<TH2F*> histosTBM;
    TString hname(moduleName);
    TH2F* h_TBM_nDecodes = new TH2F(hname+"_nTBMDecodes", hname+"_nTBMDecodes", 8, 0, 8, 8, 0, 8 );
    //h_TBM_nDecodes->SetXTitle("400 MHz phase");
    //h_TBM_nDecodes->SetYTitle("160 MHz phase");
    histosTBM.push_back(h_TBM_nDecodes);
    TH2F* h_nROCHeaders = new TH2F(hname+"_nROCHeaders", hname+"_nROCHeaders", 8, 0, 8, 8, 0, 8 );
    //h_nROCHeaders->SetXTitle("400 MHz phase");
    //h_nROCHeaders->SetYTitle("160 MHz phase");
    histosTBM.push_back(h_nROCHeaders);   
    TBMsHistoSum[moduleName] = histosTBM;
    
   }
  }//end booking sum histo

  rootf->cd(0);

}

///////////////////////////////////////////////////////////////////////////////////////////////
void PixelFEDTBMDelayCalibrationBPIX::FillEm(unsigned state, int fedid, int ch, int which, float c) {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject != 0);

  if (event_==0) return;

  if (dacsToScan.size() == 0 ) ntrigsTBM[fedid][ch][which]->Fill(c,1);
  if (dacsToScan.size() == 1){

   const std::string& iname = dacsToScan[0];
   const double ival(tempCalibObject->scanValue(iname, state)); 
   uint32_t tmp = ival; 
   int delay1 = (tmp>>2)&0x7;
   int delay2 = ((tmp>>2)&0x38)>>3;
   scansTBM[fedid][ch][which]->Fill(delay1,delay2,c);

  }

}
