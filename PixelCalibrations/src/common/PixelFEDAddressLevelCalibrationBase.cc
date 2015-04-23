/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2006-2008, Cornell U.			                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd and S. Das 					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelFEDAddressLevelCalibrationBase.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include "PixelCalibrations/include/PixelHistoReadWriteFile.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/AddressLevels.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO1Decoder.h"


#include "TH1F.h"
#include "TH2F.h"
#include "TROOT.h"
#include "TLine.h"
#include "TDirectory.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TBranch.h"
#include <math.h>

#include <iomanip>
#include <fstream>
#include <sstream>

using namespace pos;
using namespace std;

// A flag to enable raw data writting, used for special tests only
//static const bool WRITE_FILE_FIFO1 = false; //false by default

PixelFEDAddressLevelCalibrationBase::PixelFEDAddressLevelCalibrationBase(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr){
  cout << "Greetings from the PixelFEDAddressLevelCalibrationBase constructor." << endl;
}

PixelFEDAddressLevelCalibrationBase::~PixelFEDAddressLevelCalibrationBase(){
  cout << "[PixelFEDAddressLevelCalibrationBase::~PixelFEDAddressLevelCalibrationBase] entered"<<endl;
  //  PixelHistoReadWriteFile rootWriter;
  //  rootWriter.deleteDir(gROOT);
}


void PixelFEDAddressLevelCalibrationBase::initAddressLevelCalibration(){
  
  cout << "[PixelFEDAddressLevelCalibrationBase::initAddressLevelCalibration()]"<<endl;

  eventNumber=0;

  outputFile_     = new

TFile((outputDir()+"/"+"AddressLevels_"+itoa(crate_)+".root").c_str(),"recreate",("AddressLevels_"+itoa(crate_)+".root").c_str());

  RMSvsFED        = new TH2F("RMS","RMS",40,0.5,40.5,50,0.0,10.0);
  RMSvsFED->SetOption("COLZ");
  RMSvsFED->GetXaxis()->SetTitle("FED");
  RMSvsFED->GetYaxis()->SetTitle("RMS");
    
  SeparationvsFED = new TH2F("Separation","Separation",40,0.5,40.5,100,0.0,40.0);
  SeparationvsFED->SetOption("COLZ");
  SeparationvsFED->GetXaxis()->SetTitle("FED");
  SeparationvsFED->GetYaxis()->SetTitle("Separation");

  dirMakerPIX_    = new PixelRootDirectoryMaker(aROC_string_,outputFile_);
  for (unsigned int roc=0; roc<aROC_string_.size();++roc){
    dirMakerPIX_->cdDirectory(aROC_string_[roc]);
    std::string rocName = aROC_string_[roc].rocname();
    ROC_AddressLevelsHist[aROC_string_[roc]] = new TH1F(rocName.c_str(),rocName.c_str(),1024,-0.5,1023.5);
    AddressLevels tmp;
    ROC_AddressLevels[aROC_string_[roc]] = tmp;
  }
  
  //   outputFile_->cd();
  dirMakerFED_=new PixelRootDirectoryMaker(fedsAndChannels_,outputFile_);
  for(unsigned int i=0;i<fedsAndChannels_.size();++i){
    unsigned int fed = fedsAndChannels_[i].first;
    stringstream FEDName;
    FEDName.str("");
    FEDName << "FED" << fed;    
    dirMakerFED_->cdDirectory(FEDName.str());

    RMSvsChannel_       [fed] = new TH2F((FEDName.str()+"_RMS").c_str()       ,(FEDName.str()+"_RMS").c_str()       ,36,0.5,36.5, 50,0.0,10.0);
    separationvsChannel_[fed] = new TH2F((FEDName.str()+"_Separation").c_str(),(FEDName.str()+"_Separation").c_str(),36,0.5,36.5,100,0.0,40.0);

    for(unsigned int j=0;j<fedsAndChannels_[i].second.size();j++){
      unsigned int channel = fedsAndChannels_[i].second[j];
      stringstream fullName;
      fullName.str("");
      fullName << FEDName.str() << "_Channel" << channel;

      int numberOfRocs = theNameTranslation_->getROCsFromFEDChannel(fed,channel).size();
      // std::cout<<"ROC Number "<<rocNumber<<std::endl;
      dirMakerFED_->cdDirectory(fed,channel);
      
      scopePlotsMap_[fed][channel] = new TH2F   ( fullName.str().c_str()                  , fullName.str().c_str()                   ,276,-0.5,275.5,100,0,1024);
      levelsPlot_   [fed][channel] = new TH2F   ((fullName.str()+"_Levels").c_str(),       (fullName.str()+"_Levels").c_str()        ,numberOfRocs+1,-0.5,0.5+numberOfRocs,100,0,1024);
      blackLevel_   [fed][channel] = new TH1F   ((fullName.str()+"_BlackLevel").c_str(),   (fullName.str()+"_BlackLevel").c_str()    ,64,449.5-32,449.5+32); //FIXME should use real target baseline
      rmsSummary_   [fed][channel] = new TH1F   ((fullName.str()+"_RMS").c_str(),          (fullName.str()+"_RMS").c_str()           ,50,0.0,10.0);
      levelsCanvas_ [fed][channel] = new TCanvas((fullName.str()+"_Levels_Canvas").c_str(),(fullName.str()+"_Levels_Canvas").c_str() ,800,600);
      tbmLevelsHist_[fed][channel] = new TH1F((fullName.str()+"_TBMLevels").c_str(),(fullName.str()+"_TBMLevels").c_str(),1024,-0.5,1023.5);

    }
  }
}

void PixelFEDAddressLevelCalibrationBase::summary(){
  string mthn = "[PixelFEDAddressLevelCalibrationBase::summary()]\t";
  cout << mthn << endl;
  std::map <unsigned int, std::map<unsigned int, std::map<int, unsigned int> > >       summary_short;
  std::map <unsigned int, std::map<unsigned int, std::map<int, std::stringstream*> > > summary_long;
  //       FED Number            channel     overall=-3/UB=-2/TBM=-1 status
  int TBMlevelcut[4]={0,0,0};
  for (unsigned int ifed=0; ifed<fedsAndChannels_.size(); ++ifed) {
    unsigned int  fedNumber     = fedsAndChannels_[ifed].first;
    unsigned long vmeBaseAddress= theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fedNumber);
    PixelFEDCard& thePixelFEDCard=FEDInterface_[vmeBaseAddress]->getPixelFEDCard();
    
    //std::cout<<"-------------------------------------"<<std::endl;
    //std::cout<<"Pixel FED Board # "<<fedNumber;
    //std::cout<<" in this crate at VME Base Address 0x"<<std::hex<<vmeBaseAddress<<std::dec<<std::endl;

    std::map<unsigned int, std::map<int, unsigned int> >       summary_FED_short;
    std::map<unsigned int, std::map<int, std::stringstream*> > summary_FED_long;
    
    for (unsigned int ichannel=0; ichannel<fedsAndChannels_[ifed].second.size(); ++ichannel) {
      unsigned int channel=fedsAndChannels_[ifed].second[ichannel];
      
      //int recommendedBH=(int)(B[vmeBaseAddress][channel].mean()+3*B[vmeBaseAddress][channel].stddev())+50;
      //int recommendedBL=(int)(B[vmeBaseAddress][channel].mean()-3*B[vmeBaseAddress][channel].stddev())-50;
      //int recommendedUB=(int)(UB[vmeBaseAddress][channel].mean()+3*UB[vmeBaseAddress][channel].stddev())+50;

      //Use the middle between B and UB as separation -- (M.M.)
      float ub = UB[vmeBaseAddress][channel].mean();
      int lowBCut = int(  ( B[vmeBaseAddress][channel].mean() + ub )/2. );
      int recommendedBH = 1000;
      int recommendedBL = lowBCut;
      int recommendedUB = lowBCut-1;
      

      // modified for bpix
      // If the cuts overlap use a common cut in the middle d.k. 06/08 
      auto tbmUB = (UB_TBM[vmeBaseAddress][channel].mean()); 
      auto rocUB = (UB_ROC[vmeBaseAddress][channel].mean()); 
      // OLD eay
      //auto tmp = rocUB; 
      //if(tbmUB>rocUB) tmp=tbmUB;       
      //int meanCut =  (int) (tmp*0.7 + B[vmeBaseAddress][channel].mean()*0.3);

      // take care of the case when TBM-UB is very low for dual TBM channels
      if( (rocUB-tbmUB)>100 ) { // use TBM-UB instead
	cout<<"PixelFEDAddresslevelCalibrationBase: "
	    <<"Warning, TBM-UB much lower than ROC-UB: "<<tbmUB<<" "<<rocUB 
	    <<"  FED="<<fedNumber<<" channel="<<channel<<endl;
	float meanCut =  (tbmUB + B[vmeBaseAddress][channel].mean())/2.;	
	// check that the cut is still above the rocUB
	if(meanCut < rocUB ) {  // wrong, move it
	  cout<<"PixelFEDAddresslevelCalibrationBase: "
	      <<"Warning, UB-B cut is below ROC-UB."<<endl
	      <<"Use ROC-UB+50 as the new cut. FED="<<fedNumber<<" channel="
	      <<channel<<endl;
	  diagService_->reportError("PixelFEDAddresslevelCalibrationBase: Warning, UB-B cut is below ROC-UB. Use ROC-UB+50 as the new cut.",DIAGWARN);
	  meanCut = rocUB + 50.; 
	}
	recommendedUB= int(meanCut); 
	recommendedBL= int(meanCut)+1; 
      }

      //std::cout<<" Channel="<<channel<<std::endl;
      std::map<int, unsigned int>       summary_channel_short;
      std::map<int, std::stringstream*> summary_channel_long;

      summary_channel_short[-3]=1;
      summary_channel_short[-2]=1;
      summary_channel_short[-1]=1;

      summary_channel_long[-3]=new std::stringstream();
      summary_channel_long[-2]=new std::stringstream();
      summary_channel_long[-1]=new std::stringstream();

      //std::cout<<"  Black peak at      mean = " << B[vmeBaseAddress][channel].mean()
      //	 <<" with rms = " << B[vmeBaseAddress][channel].stddev() << std::endl;
      //std::cout<<"  UltraBlack peak at mean = " << UB[vmeBaseAddress][channel].mean()
      //	 <<" with rms = " << UB[vmeBaseAddress][channel].stddev() << std::endl;
      *(summary_channel_long[-2])<<"Black peak at      mean = " << B[vmeBaseAddress][channel].mean()
				 <<" with rms = " << B[vmeBaseAddress][channel].stddev() << "<br/>" << std::endl;
      *(summary_channel_long[-2])<<"UltraBlack peak at mean = " << UB[vmeBaseAddress][channel].mean()
				 <<" with rms = " << UB[vmeBaseAddress][channel].stddev() << "<br/>" << std::endl;
      
      //std::cout<<"  recommended BH = "<<recommendedBH;
      //std::cout<<"  recommended BL = "<<recommendedBL;
      //std::cout<<"  recommended UB = "<<recommendedUB<<std::endl;
      *(summary_channel_long[-2])<<"  recommended BH = "<<recommendedBH<<"<br/>";
      *(summary_channel_long[-2])<<"  recommended BL = "<<recommendedBL<<"<br/>";
      *(summary_channel_long[-2])<<"  recommended UB = "<<recommendedUB<<"<br/>"<<std::endl;
      
      if (recommendedUB<recommendedBL) {
	thePixelFEDCard.BlackHi[channel-1]=recommendedBH;
	thePixelFEDCard.BlackLo[channel-1]=recommendedBL;
	thePixelFEDCard.Ublack[channel-1]=recommendedUB;
      } else {
	diagService_->reportError("Recommended UB level higher than recommended B Low level!",DIAGERROR);
	cout<<" Recommended UB level higher than recommended B Low level!"<<endl;
	summary_channel_short[-2]=0;
	summary_channel_short[-3]=0;
	*(summary_channel_long[-2])<<"Recommended UB Threshold higher then recommended B Low Threshold!"<<"<br/>"<<std::endl;
	*(summary_channel_long[-3])<<"Recommended UB Threshold higher then recommended B Low Threshold!"<<"<br/>"<<std::endl;
      }
      

      AddressLevels &theTBMAddressLevel=TBM_AddressLevels[vmeBaseAddress][channel];
      //theTBMAddressLevel.drawHistogram(outputDir()+"/TBM_peaks_"+itoa(fedNumber)+"_"+itoa(channel)+".gif");
      theTBMAddressLevel.findPeaks();
      // save data in AddressLevel histogram array to root histogram (FIXME?)
      for(unsigned int ich=0; ich<1024;ich++) {
	tbmLevelsHist_[fedNumber][channel]->Fill((double)ich,(double)theTBMAddressLevel.getPoint(ich));
      }
      //cout<<"  TBM has "<<theTBMAddressLevel.nPeaks()<<" peaks."<<endl;
      *(summary_channel_long[-1])<<"TBM has "<<theTBMAddressLevel.nPeaks()<<" peaks."<<"<br/>"<<std::endl;
      
      std::vector<Moments> peak=theTBMAddressLevel.returnPeaks();

      for (unsigned int p=0;p<peak.size();++p) {
	//cout<<"    Peak #"<<p+1<<" = "<<peak.at(p).mean()<<", std dev = "<<peak.at(p).stddev()<<endl;
	*(summary_channel_long[-1])<<"Peak #"<<p+1<<" = "<<peak.at(p).mean()<<", std dev = "<<peak.at(p).stddev()<<"<br/>"<<std::endl;
	levelsPlot_[fedNumber][channel]->Fill(0.,peak.at(p).mean());
      }

     
      
      if (peak.size()==4){
	// Check that the 1st TBM levbel is well above the UB cut
	if( peak[0].mean() < (recommendedUB+15.) )  {
	  std::cout<<" PixelFEDAddresslevelCalibrationBase: Warning, the 1st TBM level is close to the UB cut! "
		   <<peak[0].mean()<<" "<<recommendedUB
		   <<" fed/channel = "<<fedNumber<<"/"<<channel<<std::endl;
	  int tmp = int ((peak[0].mean() + ub)/2.);
	  std::cout<<" UB is "<<ub<<" Shift cut to the middle = "<<tmp<<std::endl;
	  recommendedUB = tmp;
	  thePixelFEDCard.Ublack[channel-1]=recommendedUB; // redefine 
	}
	int recommended_L0=(int)((peak[0].mean()+peak[1].mean())/2);
	TBMlevelcut[0]=recommended_L0;
	//cout<<"Recommended TBM L 0 = "<<recommended_L0<<endl;
	int recommended_L1=(int)((peak[1].mean()+peak[2].mean())/2);
	// Check that the recommened UB is well below the 1st TBM address level
	TBMlevelcut[1]=recommended_L1;
	//cout<<"Recommended TBM L 1 = "<<recommended_L1<<endl;
	int recommended_L2=(int)((peak[2].mean()+peak[3].mean())/2);
	TBMlevelcut[2]=recommended_L2;
	//cout<<"Recommended TBM L 2 = "<<recommended_L2<<endl;
	int recommended_L3=(int)(peak[3].mean()+(peak[3].mean()-peak[2].mean())/2);
	//cout<<"Recommended TBM L 3 = "<<recommended_L3<<endl;
	TBMlevelcut[3]=recommended_L3;

	thePixelFEDCard.TBM_L0[channel-1]=thePixelFEDCard.TRL_L0[channel-1]=recommended_L0;
	thePixelFEDCard.TBM_L1[channel-1]=thePixelFEDCard.TRL_L1[channel-1]=recommended_L1;
	thePixelFEDCard.TBM_L2[channel-1]=thePixelFEDCard.TRL_L2[channel-1]=recommended_L2;
	thePixelFEDCard.TBM_L3[channel-1]=thePixelFEDCard.TRL_L3[channel-1]=recommended_L3; // could be 1023
	thePixelFEDCard.TBM_L4[channel-1]=thePixelFEDCard.TRL_L4[channel-1]=1023; // for TBM only 4 levels! 

	*(summary_channel_long[-1])<<"Recommended TBM L 0 = "<<recommended_L0<<"<br/>"<<std::endl;
	*(summary_channel_long[-1])<<"Recommended TBM L 1 = "<<recommended_L1<<"<br/>"<<std::endl;
	*(summary_channel_long[-1])<<"Recommended TBM L 2 = "<<recommended_L2<<"<br/>"<<std::endl;
	*(summary_channel_long[-1])<<"Recommended TBM L 3 = "<<recommended_L3<<"<br/>"<<std::endl;
      } else {
	// diagService_->reportError("Number of TBM peaks found is not equal to 4!",DIAGWARN);
	std::cout<<"    Number of TBM peaks found is not equal to 4!"<<std::endl;
	summary_channel_short[-1]=0;
	summary_channel_short[-3]=0;
	*(summary_channel_long[-1])<<"Number of TBM peaks found is not equal to 4!"<<"<br/>"<<std::endl;
	*(summary_channel_long[-3])<<"Number of TBM peaks found is not equal to 4!"<<"<br/>"<<std::endl;
      }

      summary_FED_short.insert(make_pair(channel, summary_channel_short));
      summary_FED_long.insert(make_pair(channel, summary_channel_long));

    }

    //FEDInterface_[vmeBaseAddress]->setupFromDB(thePixelFEDCard);
    summary_short.insert(make_pair(fedNumber, summary_FED_short));
    summary_long.insert(make_pair(fedNumber, summary_FED_long));
  }

  //Copy data into histogram This should eventually go away!
  for (std::map<PixelROCName, AddressLevels>::iterator it=ROC_AddressLevels.begin();it!=ROC_AddressLevels.end();++it) {
    for(unsigned int i=0;i<1024;i++){
      ROC_AddressLevelsHist[it->first]->Fill((double)i,(double)it->second.getPoint(i));
    }
  }
  
  std::map<unsigned int, std::vector<unsigned int> > ROClevels;
  
  addressLevelBranch theBranch;
  outputFile_->cd();
  TDirectory * dirSummaries = gDirectory->mkdir("SummaryTrees","SummaryTrees");
  dirSummaries->cd();
  TTree * tree = new TTree("LevelSummary","LevelSummary");
  tree->Branch("AllLevels",&theBranch,"pass/F:nPeaks/F:maxrms/F:minsep/F:blackrms/F:rocName/C",4096000);

  for (std::map<PixelROCName, AddressLevels>::iterator it=ROC_AddressLevels.begin();it!=ROC_AddressLevels.end();++it){
    const PixelHdwAddress *aROC_hardware = theNameTranslation_->getHdwAddress(it->first);
    unsigned int fedNumber               = aROC_hardware->fednumber();
    unsigned int crate                   = theFEDConfiguration_->crateFromFEDNumber(fedNumber);
    if (crate!=crate_) continue;
    unsigned long vmeBaseAddress         = theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fedNumber);
    unsigned int channel                 = aROC_hardware->fedchannel();
    unsigned int fedrocnumber            = aROC_hardware->fedrocnumber();
    string rocName                       = it->first.rocname();

    //   std::cout<<"ROC number "<<fedrocnumber<<std::endl;

    PixelFEDCard& thePixelFEDCard                      = FEDInterface_[vmeBaseAddress]->getPixelFEDCard();
    AddressLevels &theROCAddressLevel                  = it->second;
    ROC_ChannelSum_AddressLevels_[fedNumber][channel] += theROCAddressLevel;
    ROC_FEDSum_AddressLevels[fedNumber]               += theROCAddressLevel;      

    theROCAddressLevel.findPeaks();
    //cout<<"FED Crate 1, VME Base Address 0x"<<hex<<vmeBaseAddress<<dec<<" channel="<<channel<<" FED ROC number="<<fedrocnumber;
    //cout<<" has "<<theROCAddressLevel.nPeaks()<<" peaks."<<endl;
    summary_long[fedNumber][channel][fedrocnumber] = new std::stringstream();
    *(summary_long[fedNumber][channel][fedrocnumber]) << "ROC has " << theROCAddressLevel.nPeaks() <<" peaks.<br/>"<<std::endl;
    //THIS FOR ME IS NOT NEEDED ANYMORE LORENZO
//     theROCAddressLevel.dump(outputDir()+"/ROC_peaks_"+itoa(fedNumber)+"_"+itoa(channel)+"_"+itoa(fedrocnumber)+".dat");
//     theROCAddressLevel.drawHistogram(outputDir()+"/ROC_peaks_"+itoa(fedNumber)+"_"+itoa(channel)+"_"+itoa(fedrocnumber)+".gif");
    
    std::vector<Moments> peak=theROCAddressLevel.returnPeaks();
    
    for (unsigned int p=0;p<peak.size();++p) {
      //cout<<"Peak #"<<p+1<<" = "<<peak.at(p).mean()<<", std dev = "<<peak.at(p).stddev()<<endl;
      *(summary_long[fedNumber][channel][fedrocnumber])<<"Peak #"<<p+1<<" = "<<peak.at(p).mean()<<", std dev = "<<peak.at(p).stddev()<<"<br/>"<<std::endl;
      rmsSummary_  [fedNumber][channel]->Fill(peak.at(p).stddev());
      levelsPlot_  [fedNumber][channel]->Fill(fedrocnumber+1,peak.at(p).mean());
      RMSvsChannel_[fedNumber]->Fill(channel,peak.at(p).stddev());
      RMSvsFED->Fill(fedNumber,peak.at(p).stddev());

      //   std::cout<<"fedrocnumber+2: "<<fedrocnumber+2<<" mean value: "<<peak.at(p).mean()<<std::endl;
      double Separation = 0.0;
      if(p!=peak.size()-1){
	double Distance = peak.at(p+1).mean()-peak.at(p).mean();
	Separation=Distance/sqrt((peak.at(p+1).stddev())*(peak.at(p+1).stddev())+(peak.at(p).stddev())*(peak.at(p).stddev()));
	separationvsChannel_[fedNumber]->Fill(channel,Separation);
	SeparationvsFED->Fill(fedNumber,Separation);

	
      }
    }

    separationvsChannel_[fedNumber]->SetOption("COLZ");
    separationvsChannel_[fedNumber]->GetXaxis()->SetTitle("Channel");
    separationvsChannel_[fedNumber]->GetYaxis()->SetTitle("Separation");

    RMSvsChannel_[fedNumber]->SetOption("COLZ");
    RMSvsChannel_[fedNumber]->GetXaxis()->SetTitle("Channel");
    RMSvsChannel_[fedNumber]->GetYaxis()->SetTitle("RMS");

    levelsPlot_[fedNumber][channel]->SetOption("COLZ");
    
    
    if (peak.size()==6){
      // Check that the 1st ROC level is well below the UB cut
      if( peak[0].mean() < (thePixelFEDCard.Ublack[channel-1] + 15) ) 
	std::cout<<" PixelFEDAddresslevelCalibrationBase: Error, the 1st ROC level is close to the UB cut!"
		 <<peak[0].mean()<<" "<<thePixelFEDCard.Ublack[channel-1]
		 <<" fed/channel = "<<fedNumber<<"/"<<channel<<std::endl;
      if( peak[5].mean() > 1000. )
	std::cout<<" PixelFEDAddresslevelCalibrationBase: Warning, the last ROC level is above 1000"
		 <<peak[5].mean()<<" fed/channel = "<<fedNumber<<"/"<<channel<<std::endl;

      int recommended_L0=(int)((peak[1].mean()+peak[0].mean())/2);
      ROClevels[fedrocnumber].push_back(recommended_L0);
      //cout<<"Recommended L0 = "<<recommended_L0<<endl;

      int recommended_L1=(int)((peak[2].mean()+peak[1].mean())/2);
      ROClevels[fedrocnumber].push_back(recommended_L1);
      //cout<<"Recommended L 1 = "<<recommended_L1<<endl;

      int recommended_L2=(int)((peak[3].mean()+peak[2].mean())/2);
      ROClevels[fedrocnumber].push_back(recommended_L2);
      //cout<<"Recommended L 2 = "<<recommended_L2<<endl;

      int recommended_L3=(int)((peak[4].mean()+peak[3].mean())/2);
      ROClevels[fedrocnumber].push_back(recommended_L3);
      //cout<<"Recommended L 3 = "<<recommended_L3<<endl;
      
      int recommended_L4=(int)((peak[5].mean()+peak[4].mean())/2);
      ROClevels[fedrocnumber].push_back(recommended_L4);
      //cout<<"Recommended L 4 = "<<recommended_L4<<endl;
      

      if (recommended_L0>thePixelFEDCard.Ublack[channel-1]) {
	thePixelFEDCard.ROC_L0[channel-1][fedrocnumber]=recommended_L0;
	thePixelFEDCard.ROC_L1[channel-1][fedrocnumber]=recommended_L1;
	thePixelFEDCard.ROC_L2[channel-1][fedrocnumber]=recommended_L2;
	thePixelFEDCard.ROC_L3[channel-1][fedrocnumber]=recommended_L3;
	thePixelFEDCard.ROC_L4[channel-1][fedrocnumber]=recommended_L4;
	
	thePixelFEDCard.restoreControlAndModeRegister();
	thePixelFEDCard.writeASCII(outputDir());
	
	*(summary_long[fedNumber][channel][fedrocnumber])<<"Recommended L 0 = "<<recommended_L0<<"<br/>"<<std::endl;
	*(summary_long[fedNumber][channel][fedrocnumber])<<"Recommended L 1 = "<<recommended_L1<<"<br/>"<<std::endl;
	*(summary_long[fedNumber][channel][fedrocnumber])<<"Recommended L 2 = "<<recommended_L2<<"<br/>"<<std::endl;
	*(summary_long[fedNumber][channel][fedrocnumber])<<"Recommended L 3 = "<<recommended_L3<<"<br/>"<<std::endl;
	*(summary_long[fedNumber][channel][fedrocnumber])<<"Recommended L 4 = "<<recommended_L4<<"<br/>"<<std::endl;
	summary_short[fedNumber][channel][fedrocnumber]=1;
	
      } else {
	
	diagService_->reportError("Recommended Level 0 is less than Ultrablack upper bound! Did not write new file.",DIAGWARN);
	cout<<"Recommended Level 0 is less than Ultrablack upper bound! Did not write new file."<<endl;
	summary_short[fedNumber][channel][fedrocnumber]=0;
	summary_short[fedNumber][channel][-3]=0;
	*(summary_long[fedNumber][channel][fedNumber])<<"Recommended Level 0 is less than UltraBlack High Threshold! Did not write new file."<<"<br/>"<<std::endl;
	*(summary_long[fedNumber][channel][-3])<<"Recommended Level 0 is less than UltraBlack High Threshold! Did not write new file."<<"<br/>"<<std::endl;
      }
    } else {
      // levelsLine[fedNumber][channel][fedrocnumber][0]=TLine(0.0,0.0,0.0,0.0);

      diagService_->reportError("Number of ROC peaks found is not equal to 6! Did not write new file.",DIAGWARN);
      cout<<"Number of ROC peaks found is not equal to 6! Did not write new file."<<std::endl;
      summary_short[fedNumber][channel][fedrocnumber]=0;
      summary_short[fedNumber][channel][-3]=0;
      *(summary_long[fedNumber][channel][fedrocnumber])<<"Number of ROC peaks found is not equal to 6!"<<"<br/>"<<std::endl;
      *(summary_long[fedNumber][channel][-3])<<"Number of ROC peaks found is not equal to 6!"<<"<br/>"<<std::endl;

      for(int i=0;i<6;++i){
	ROClevels[fedrocnumber].push_back(0);
      }
    }
    //SUMMARY TREE
    strcpy(theBranch.rocName,rocName.c_str());
    theBranch.pass=summary_short[fedNumber][channel][fedrocnumber];
    theBranch.nPeaks=theROCAddressLevel.nPeaks();
    if(theBranch.nPeaks==6) {
      theBranch.maxrms=theROCAddressLevel.maxrms();
      theBranch.minseparation=theROCAddressLevel.minseparation();
    }
    else {
      theBranch.maxrms=50.;
      theBranch.minseparation=1.;
    }
    theBranch.blackrms //=B[vmeBaseAddress][channel].stddev();  //not really per roc, but... take from FIFO1Decoder - before TBM header & all ROC headers
      =blackLevel_[fedNumber][channel]->GetRMS(); //take from blackLevel histogram (after end of pulse), but hardcoded!
//     std::cout << "[PixelFEDAddressLevelCalibrationBase::summary()]\tName: " << theBranch.rocName << " pass: " << theBranch.pass << endl;
    tree->Fill();
    //SUMMARY TREE
  }

  

  // Sum up channel and FED's ROC histograms
  outputFile_->cd();
  for (unsigned int ifed=0; ifed<fedsAndChannels_.size(); ++ifed) {
    unsigned int fedNumber=fedsAndChannels_[ifed].first;
    unsigned long vmeBaseAddress= theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fedNumber);
    PixelFEDCard& thePixelFEDCard=FEDInterface_[vmeBaseAddress]->getPixelFEDCard();
    for (unsigned int ichannel=0; ichannel<fedsAndChannels_[ifed].second.size(); ++ichannel) {
      unsigned int channel=fedsAndChannels_[ifed].second[ichannel];
      dirMakerFED_->cdDirectory(fedNumber,channel);
      //ROC_ChannelSum_AddressLevels_[fedNumber][channel].drawHistogram(outputDir()+"/ROC_peaks_"+itoa(fedNumber)+"_"+itoa(channel)+".gif");
      //ROC_FEDSum_AddressLevels[fedNumber].drawHistogram(outputDir()+"/ROC_peaks_"+itoa(fedNumber)+".gif");
      ROC_FEDSum_AddressLevels[fedNumber].drawHistogram();
      levelsCanvas_ [fedNumber][channel]->cd();
      levelsPlot_   [fedNumber][channel]->Draw();

      TLine TBMsLevel1(-0.5,thePixelFEDCard.TBM_L0[channel-1],0.5,thePixelFEDCard.TBM_L0[channel-1]);
      TLine TBMsLevel2(-0.5,thePixelFEDCard.TBM_L1[channel-1],0.5,thePixelFEDCard.TBM_L1[channel-1]);
      TLine TBMsLevel3(-0.5,thePixelFEDCard.TBM_L2[channel-1],0.5,thePixelFEDCard.TBM_L2[channel-1]);
      
      TBMsLevel1.SetLineColor(30); TBMsLevel1.SetLineStyle(kDashed);
      TBMsLevel2.SetLineColor(38); TBMsLevel2.SetLineStyle(kDashed);
      TBMsLevel3.SetLineColor(46); TBMsLevel3.SetLineStyle(kDashed);
      
      TBMsLevel1.Draw("same");
      TBMsLevel2.Draw("same");
      TBMsLevel3.Draw("same");
      
      vector<PixelROCName> rocNames = theNameTranslation_->getROCsFromFEDChannel(fedNumber,channel);
      for(vector<PixelROCName>::iterator it=rocNames.begin();it!=rocNames.end();++it){
	const PixelHdwAddress *aROC_hardware=theNameTranslation_->getHdwAddress(*it);
	unsigned int fedrocnumber = aROC_hardware->fedrocnumber();
	TLine* ROCsLevel1 = new TLine(fedrocnumber+0.5,thePixelFEDCard.ROC_L0[channel-1][fedrocnumber],fedrocnumber+1.5,thePixelFEDCard.ROC_L0[channel-1][fedrocnumber]);
	TLine* ROCsLevel2 = new TLine(fedrocnumber+0.5,thePixelFEDCard.ROC_L1[channel-1][fedrocnumber],fedrocnumber+1.5,thePixelFEDCard.ROC_L1[channel-1][fedrocnumber]);
	TLine* ROCsLevel3 = new TLine(fedrocnumber+0.5,thePixelFEDCard.ROC_L2[channel-1][fedrocnumber],fedrocnumber+1.5,thePixelFEDCard.ROC_L2[channel-1][fedrocnumber]);
	TLine* ROCsLevel4 = new TLine(fedrocnumber+0.5,thePixelFEDCard.ROC_L3[channel-1][fedrocnumber],fedrocnumber+1.5,thePixelFEDCard.ROC_L3[channel-1][fedrocnumber]);
	TLine* ROCsLevel5 = new TLine(fedrocnumber+0.5,thePixelFEDCard.ROC_L4[channel-1][fedrocnumber],fedrocnumber+1.5,thePixelFEDCard.ROC_L4[channel-1][fedrocnumber]);
	  
	ROCsLevel1->SetLineColor(kRed);      ROCsLevel1->SetLineStyle(kDashed);
	ROCsLevel2->SetLineColor(kBlue);     ROCsLevel2->SetLineStyle(kDashed);
	ROCsLevel3->SetLineColor(kGreen);    ROCsLevel3->SetLineStyle(kDashed);
	ROCsLevel4->SetLineColor(kOrange);   ROCsLevel4->SetLineStyle(kDashed);
	ROCsLevel5->SetLineColor(kMagenta);  ROCsLevel5->SetLineStyle(kDashed);
	ROCsLevel1->Draw("same");
	ROCsLevel2->Draw("same");
	ROCsLevel3->Draw("same");
	ROCsLevel4->Draw("same");
	ROCsLevel5->Draw("same");
      }
      levelsCanvas_[fedNumber][channel]->Write();
    }
  }
  
  // Print out the summary collected to console and to an HTML file
  ofstream summaryFile((outputDir()+"/AddressLevels_"+itoa(crate_)+".html").c_str());
  std::cout<<" ### Summary ### "<<std::endl;
  summaryFile<<"<html>"<<std::endl;
  summaryFile<<"<head>"<<std::endl;
  summaryFile<<" <script language=\"JavaScript\">"<<std::endl;
  summaryFile<<"  function toggle(elementID){"<<std::endl;
  summaryFile<<"   var listElementStyle=document.getElementById(elementID).style;"<<std::endl;
  summaryFile<<"   if (listElementStyle.display==\"none\"){"<<std::endl;
  summaryFile<<"    listElementStyle.display=\"block\";"<<std::endl;
  summaryFile<<"   } else {"<<std::endl;
  summaryFile<<"    listElementStyle.display=\"none\";"<<std::endl;
  summaryFile<<"   }"<<std::endl;
  summaryFile<<"  }"<<std::endl;
  summaryFile<<" </script>"<<std::endl;
  summaryFile<<"</head>"<<std::endl;
  summaryFile<<"<body>"<<std::endl;

  

  std::map <unsigned int, std::map<unsigned int, std::map<int, unsigned int> > >::iterator i_summary_short=summary_short.begin();
  for (;i_summary_short!=summary_short.end();++i_summary_short) {
    unsigned int fedNumber=i_summary_short->first;
    std::map<unsigned int, std::map<int, unsigned int> > summary_FED_short=i_summary_short->second;
    std::map<unsigned int, std::map<int, std::stringstream*> > summary_FED_long=summary_long[fedNumber];
    
    std::cout<<" --- FED Board with FED Number = "<<fedNumber<<" --- "<<std::endl;
    std::cout<<"Channel|Overall|UB/B|TBM| 0| 1| 2| 3| 4| 5| 6| 7| 8| 9|10|11|12|13|14|15|16|17|18|19|20|21|22|23|"<<std::endl;

    summaryFile<<"<h2> Pixel FED Board # "<<fedNumber<<" </h2>"<<std::endl;

    summaryFile<<"<a href=\"javascript:toggle('ROC_"<<fedNumber<<"')\">Sum of ROCs</a></br>"<<std::endl;                   // ROC sum over FED
    summaryFile<<"<div id=\"ROC_"<<fedNumber<<"\" style=\"display:none\">"<<std::endl;
    summaryFile<<" <a href=\"ROC_peaks_"<<itoa(fedNumber)<<".gif\" target=\"_blank\">"<<std::endl;
    summaryFile<<"  <img src=\"ROC_peaks_"<<itoa(fedNumber)<<".gif\" width=\"200\" height=\"200\"/>"<<std::endl;
    summaryFile<<" </a>"<<std::endl;
    summaryFile<<"</div>"<<std::endl;
    
    summaryFile<<"<table border=1>"<<std::endl;
    summaryFile<<" <tr>";
    summaryFile<<"<th>Channel</th>";
    summaryFile<<"<th>Overall</th>";
    summaryFile<<"<th>UB/B Levels</th>";
    summaryFile<<"<th>TBM</th>";
    summaryFile<<"<th>Sum of ROCs</th>";
    summaryFile<<"<th>ROC 0</th>";
    summaryFile<<"<th>ROC 1</th>";
    summaryFile<<"<th>ROC 2</th>";
    summaryFile<<"<th>ROC 3</th>";
    summaryFile<<"<th>ROC 4</th>";
    summaryFile<<"<th>ROC 5</th>";
    summaryFile<<"<th>ROC 6</th>";
    summaryFile<<"<th>ROC 7</th>";
    summaryFile<<"<th>ROC 8</th>";
    summaryFile<<"<th>ROC 9</th>";
    summaryFile<<"<th>ROC 10</th>";
    summaryFile<<"<th>ROC 11</th>";
    summaryFile<<"<th>ROC 12</th>";
    summaryFile<<"<th>ROC 13</th>";
    summaryFile<<"<th>ROC 14</th>";
    summaryFile<<"<th>ROC 15</th>";
    summaryFile<<"<th>ROC 16</th>";
    summaryFile<<"<th>ROC 17</th>";
    summaryFile<<"<th>ROC 18</th>";
    summaryFile<<"<th>ROC 19</th>";
    summaryFile<<"<th>ROC 20</th>";
    summaryFile<<"<th>ROC 21</th>";
    summaryFile<<"<th>ROC 22</th>";
    summaryFile<<"<th>ROC 23</th>";
    summaryFile<<"</tr>"<<std::endl;

    
    std::map <unsigned int, std::map<int, unsigned int> >::iterator i_summary_FED_short=summary_FED_short.begin();
    for (;i_summary_FED_short!=summary_FED_short.end();++i_summary_FED_short){
      unsigned int channel=i_summary_FED_short->first;
      std::map<int, unsigned int> summary_channel_short=i_summary_FED_short->second;
      std::map<int, std::stringstream*> summary_channel_long=summary_FED_long[channel];
      std::cout<<std::right<<std::setw(7)<<channel<<"|";
      std::cout<<std::right<<std::setw(7)<<summary_channel_short[-3]<<"|"; // Overall score
      std::cout<<std::right<<std::setw(4)<<summary_channel_short[-2]<<"|"; // UB/B Levels score
      std::cout<<std::right<<std::setw(3)<<summary_channel_short[-1]<<"|"; // TBM score
      
      summaryFile<<" <tr>"<<std::endl;
      summaryFile<<"  <td>"<<channel<<"</td>";
      
      summaryFile<<"<td>"<<summary_channel_short[-3]<<"</td>"<<std::endl;   // Overall score

      summaryFile<<"<td><a href=\"javascript:toggle('UBB_"<<fedNumber<<"_"<<channel<<"')\">"<<summary_channel_short[-2]<<"</a><br/>"<<std::endl;   // UB / B score
      summaryFile<<" <div id=\"UBB_"<<fedNumber<<"_"<<channel<<"\" style=\"display:none\">"<<std::endl;
      summaryFile<<"  <font size=1>"<<(summary_channel_long[-2]->str())<<"</font>"<<std::endl;              // UB/ B Levels description
      summaryFile<<" </div>"<<std::endl;
      summaryFile<<"</td>"<<std::endl;
      
      summaryFile<<"<td><a href=\"javascript:toggle('TBM_"<<fedNumber<<"_"<<channel<<"')\">"<<summary_channel_short[-1]<<"</a><br/>"<<std::endl;   // TBM score
      summaryFile<<" <div id=\"TBM_"<<fedNumber<<"_"<<channel<<"\" style=\"display:none\">"<<std::endl;
      summaryFile<<"  <a href=\"TBM_peaks_"<<itoa(fedNumber)<<"_"<<itoa(channel)<<".gif\" target=\"_blank\">"<<std::endl;
      summaryFile<<"   <img src=\"TBM_peaks_"<<itoa(fedNumber)<<"_"<<itoa(channel)<<".gif\" width=\"200\" height=\"200\"/>"<<std::endl;
      summaryFile<<"  </a>"<<std::endl;
      summaryFile<<"  <font size=1>"<<(summary_channel_long[-1]->str())<<"</font"<<std::endl;               // TBM description
      summaryFile<<" </div>"<<std::endl;
      summaryFile<<"</td>"<<std::endl;

      summaryFile<<"<td><a href=\"javascript:toggle('ROC_"<<fedNumber<<"_"<<channel<<"')\">+</a></br>"<<std::endl;                                 // ROC sum over channel
      summaryFile<<" <div id=\"ROC_"<<fedNumber<<"_"<<channel<<"\" style=\"display:none\">"<<std::endl;
      summaryFile<<"  <a href=\"ROC_peaks_"<<itoa(fedNumber)<<"_"<<itoa(channel)<<".gif\" target=\"_blank\">"<<std::endl;
      summaryFile<<"   <img src=\"ROC_peaks_"<<itoa(fedNumber)<<"_"<<itoa(channel)<<".gif\" width=\"200\" height=\"200\"/>"<<std::endl;
      summaryFile<<"  </a>"<<std::endl;
      summaryFile<<" </div>"<<std::endl;
      summaryFile<<"</td>"<<std::endl;

      for (int roc=0;roc<24;++roc) {
	std::map<int, unsigned int>::iterator i_roc_short=summary_channel_short.find(roc);
	std::map<int, std::stringstream*>::iterator i_roc_long=summary_channel_long.find(roc);
	if (i_roc_short!=summary_channel_short.end()) {
	  std::cout<<std::right<<setw(2)<<i_roc_short->second<<"|";
    
	  std::string rocName=theNameTranslation_->ROCNameFromFEDChannelROC(fedNumber,channel,roc).rocname();

	  summaryFile<<"<td><a href=\"javascript:toggle('ROC_"<<fedNumber<<"_"<<channel<<"_"<<roc<<"')\">"<<i_roc_short->second<<"</a><br/>"<<std::endl; // ROC score
	  summaryFile<<" <div id=\"ROC_"<<fedNumber<<"_"<<channel<<"_"<<roc<<"\" style=\"display:none\">"<<std::endl;
	  summaryFile<<"  <a href=\"ROC_peaks_"<<itoa(fedNumber)<<"_"<<itoa(channel)<<"_"<<itoa(roc)<<".gif\" target=\"_blank\">"<<std::endl;
	  summaryFile<<"   <img src=\"ROC_peaks_"<<itoa(fedNumber)<<"_"<<itoa(channel)<<"_"<<itoa(roc)<<".gif\" width=\"200\" height=\"200\"/>"<<std::endl;
	  summaryFile<<"  </a>"<<std::endl;
	  summaryFile<<"  <font size=1>"<<((i_roc_long->second)->str())<<"</font>"<<std::endl;
	  summaryFile<<" </div>"<<std::endl;
	  summaryFile<<"</td>"<<std::endl;
	}
      }
      std::cout<<std::left<<setw(0)<<std::endl;
      summaryFile<<" </tr>"<<std::endl;
      
    }
    std::cout<<" ---------------------------------------- "<<std::endl;
    summaryFile<<"</table>"<<std::endl;
  }
  std::cout<<" ### ### ### "<<std::endl;
  summaryFile<<"</body>"<<std::endl;
  summaryFile.close();
  
  std::cout<<" Closed summary file"<<std::endl;

  //write out root filefullName.c_str()
  //  PixelHistoReadWriteFile rootWriter;
  //  rootWriter.write(outputDir()+"/AddressLevels.root");

  std::cout<<" Will write root file"<<std::endl;

  outputFile_->Write();
  //  outputFile_->ls();

  std::cout<<" Will close root file"<<std::endl;

  outputFile_->Close();

  std::cout<<" Will clear memory"<<std::endl;

  baselineCorrection.clear();
  UB.clear();
  UB_TBM.clear();
  UB_ROC.clear();
  B.clear();
  recommended_UB.clear();
  recommended_BH.clear();
  recommended_BL.clear();
  ROC_AddressLevels.clear();
  TBM_AddressLevels.clear();

  std::cout<<" OK, completely done"<<std::endl;

}

void PixelFEDAddressLevelCalibrationBase::analyze(bool noHits){
  string mthn = "[PixelFEDAddressLevelCalibrationBase::analyze()]\t";
  for (unsigned int ifed=0; ifed<fedsAndChannels_.size(); ++ifed) {
    unsigned int  fedNumber      = fedsAndChannels_[ifed].first;
    unsigned long vmeBaseAddress = theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fedNumber);

    for (unsigned int ichannel=0; ichannel<fedsAndChannels_[ifed].second.size(); ++ichannel) {
      uint32_t buffer[pos::fifo1TranspDepth];
      unsigned int  channel = fedsAndChannels_[ifed].second[ichannel];
      
      int status = FEDInterface_[vmeBaseAddress]->drain_transBuffer(channel, buffer);
      if (status!=(int)pos::fifo1TranspDataLength) {
	std::cout<<"PixelFEDSupervisor::AddressLevelCalibrationWithPixels -- Could not drain FIFO 1 of FED Channel "<<channel<<" in transparent mode status="<<status<<std::endl;
	diagService_->reportError("PixelFEDSupervisor::AddressLevelCalibrationWithPixels -- Could not drain FIFO 1 in transparent mode!",DIAGWARN);
      }		    
      
      /*Debug printout of fifo1
	if (channel==11) {
	static ofstream out("fifo1.dmp");
	static int event;
	event++;
	out << "Event:"<<event<<endl;
	for (unsigned int i=0;i<(int)pos::fifo1TranspDataLength;i++){
	out << "buffer[ "<<i<<" ]= "<<hex<<buffer[i]<<dec<<" ( "
	<< (buffer[i]>>22) << " )"<<endl;
	}
	}
      */
      
      for(unsigned int i=0;i<276;i++){
	scopePlotsMap_[fedNumber][channel]->Fill((double)i,(double)(buffer[i]>>22));
      }
      
      //FIXME not safe if you use many pixels per trigger.
      for(unsigned int i=300;i<(int)pos::fifo1TranspDataLength;i++){
	blackLevel_[fedNumber][channel]->Fill((double)(buffer[i]>>22));
      }
      
      if (eventNumber%10==0){
	//Pushing back FED Automatic Baseline Correction values into containers
	Moments *baselineCorrectionOfChannel=&(baselineCorrection[vmeBaseAddress][channel]);

	int baselinecor=FEDInterface_[vmeBaseAddress]->get_BaselineCorr(channel);
	//std::cout << "channel="<<channel<<" correction="<<baselinecor<<std::endl;
	
	baselineCorrectionOfChannel->push_back(baselinecor);

	// Every 1000th update to the channel prints out this message
	if (baselineCorrectionOfChannel->count()%1000==0) {
	  std::cout<<"Baseline Correction for FED at 0x"<<std::hex<<vmeBaseAddress<<std::dec<<", channel "<<channel
		   <<" has mean = "<<baselineCorrectionOfChannel->mean()
		   <<" and std dev = "<<baselineCorrectionOfChannel->stddev()<<std::endl;
	  baselineCorrectionOfChannel->clear();
	}
      }

      if(0) { // dump fifo1 to a file, for testing only, d.k. 01/08
	ofstream outfile("data.dat",ios::app);
	//outfile.write((char*)errBuffer, (errorLength+1)*sizeof(unsigned long));
	
	const int WRITE_LENGTH=256;
	outfile<<WRITE_LENGTH<<" "<<channel<<" "<<fedNumber<<" "
	       <<FEDInterface_[vmeBaseAddress]->get_BaselineCorr(channel)<<endl;
	for(int i=0;i<WRITE_LENGTH;++i) {
	  int data = (buffer[i]  & 0xffc00000)>>22; // analyze word
	  outfile<<data<<" "<<i<<" "<<hex<<buffer[i]<<dec<<endl;
	  //cout<<i<<" "<<data<<" "<<hex<<buffer[i]<<dec<<endl;
	}
	outfile.close();
      }

      std::vector<PixelROCName> ROCs=theNameTranslation_->getROCsFromFEDChannel(fedNumber, channel);

      if (noHits) {
	//FIFO1Decoder decodeThis(buffer, ROCs.size(), UB[vmeBaseAddress][channel], B[vmeBaseAddress][channel], TBM_AddressLevels[vmeBaseAddress][channel]);
	FIFO1Decoder decodeThis(buffer, ROCs.size(), UB[vmeBaseAddress][channel], B[vmeBaseAddress][channel], 
				TBM_AddressLevels[vmeBaseAddress][channel],
				UB_TBM[vmeBaseAddress][channel],UB_ROC[vmeBaseAddress][channel]);

	//cout<<"------------Listen up ya'll on channel "<<channel<<endl;
	//cout<<"UB mean="<<UB[vmeBaseAddress][channel].mean()<<" with stddev="<<UB[vmeBaseAddress][channel].stddev()<<endl;
	
	//cout<<"B mean="<<B[vmeBaseAddress][channel].mean()<<" with stddev="<<B[vmeBaseAddress][channel].stddev()<<endl;
	
	//cout<<"# ROCs="<<ROCs.size()<<endl;

	//cout<<"---------------------"<<endl;

	if (!decodeThis.valid()) {
	  diagService_->reportError("ROC numbers don't match in Without Hits!",DIAGWARN);
	  cout<<"[PixelFEDAddressLevelsCalibrationBase] Decoding failed for the above dump in FED Number "
	      <<fedNumber<<", channel "<<channel<<std::endl;
	  cout<<"UB mean="<<UB[vmeBaseAddress][channel].mean()<<" with stddev="<<UB[vmeBaseAddress][channel].stddev()<<endl;
	  cout<<"B mean="<<B[vmeBaseAddress][channel].mean()<<" with stddev="<<B[vmeBaseAddress][channel].stddev()<<endl;
	  cout<<"# ROCs="<<ROCs.size()<<endl;
	  continue;
	}
      } else {
	//FIFO1Decoder decodeThis(buffer, ROCs, UB[vmeBaseAddress][channel], B[vmeBaseAddress][channel], ROC_AddressLevels, TBM_AddressLevels[vmeBaseAddress][channel]);
	FIFO1Decoder decodeThis(buffer, ROCs, UB[vmeBaseAddress][channel], B[vmeBaseAddress][channel], 
				ROC_AddressLevels, TBM_AddressLevels[vmeBaseAddress][channel],
				UB_TBM[vmeBaseAddress][channel],UB_ROC[vmeBaseAddress][channel]);

	if (!decodeThis.valid()) {
	  diagService_->reportError("ROC numbers don't match in With Hits!",DIAGWARN);
	  cout<<"[PixelFEDAddressLevelsCalibrationBase] Decoding failed for the above dump in FED Number "
	      <<fedNumber<<", channel "<<channel<<std::endl;
	  continue;
	}
      }
    }
    VMEPtr_[vmeBaseAddress]->write("LRES",0x80000000); // Local Reset
    VMEPtr_[vmeBaseAddress]->write("CLRES",0x80000000); // *** MAURO ***
  }
}
