#include "PixelAnalysisTools/include/PixelAliveHistoManager.h"
//#include "PixelAnalysisTools/include/PixelXmlReader.h"
#include "PixelUtilities/PixelXmlUtilities/include/PixelXmlReader.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include "PixelAnalysisTools/include/PixelCalibConfigurationExtended.h"
#include "PixelAnalysisTools/include/PixelConfigurationsManager.h"
#include <sstream>
#include <TH2F.h>
#include <TROOT.h>
#include <TDirectory.h>
#include <TTree.h>
#include <TBranch.h>
#include <string.h>
#include <TFile.h>

using namespace std;

struct PixelAliveBadPixelsBranch{
        float rocsWithNumberOfDeadPixelsGTN;
	float percentageOfDeadPixels;
	float numberOfDeadPixels;
        char rocName[40];
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PixelAliveHistoManager::PixelAliveHistoManager(PixelXmlReader* xmlReader, PixelCalibConfigurationExtended *calib, PixelConfigurationsManager * configurationsManager, ostream *logger) : PixelHistoManager(xmlReader,calib,configurationsManager,logger){
  //   string mthn = "[PixelAliveHistoManager::PixelAliveHistoManager()]\t";
  PixelHistoManager::numberOfHistosPerRoc_= 1;
  maxBadChannelsPerRoc_ = atoi(thePixelXmlReader_->getXMLAttribute("BadPixels","Max").c_str());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelAliveHistoManager::init(){
  //  string mthn = "[PixelAliveHistoManager::init()]\t";
  bookHistos();
  PixelHistoManager::init();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PixelAliveHistoManager::~PixelAliveHistoManager(){
  destroy();
//  PixelHistoManager::destroy();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelAliveHistoManager::destroy(void){
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelAliveHistoManager::bookHistos(void){
  string mthn = "[PixelAliveHistoManager::bookHistos()]\t";
  //Initializing the map with the name of all the possible rocs present
  //cout << mthn << "Number of ROCs: " << rocList_.size() << endl;
	int nXdivisions = 20; 
	int nYdivisions = 28;
  nXdivisions = PixelHistoManager::setNumberOfLabels(nXdivisions,52);
	nYdivisions = PixelHistoManager::setNumberOfLabels(nYdivisions,70);

  for (vector<pos::PixelROCName>::iterator it=rocList_.begin();it!=rocList_.end();it++){
    vector<TH1*> tmp;
    string rocName =  it->rocname();
		if(!thePixelConfigurationsManager_->isDataToAnalyze(rocName)){continue;}
    PixelHistoManager::thePixelRootDirectoryMaker_->cdDirectory(rocName);
		//It is always 1 for PixelAlive
//  for(unsigned int nHistos=0; nHistos<numberOfHistosPerRoc_; nHistos++){
		tmp.push_back(new TH2F(rocName.c_str(),rocName.c_str(),52, -0.5, 51.5,80,-0.5,79.5));
    tmp[0]->SetMinimum(0.0);
    tmp[0]->SetMaximum(100.);
    tmp[0]->GetXaxis()->SetTitle("col");
		tmp[0]->GetXaxis()->SetNdivisions(nXdivisions);
    tmp[0]->GetYaxis()->SetTitle("row");
		tmp[0]->GetYaxis()->SetNdivisions(nYdivisions);
    tmp[0]->SetStats(false);
    tmp[0]->SetOption("COLZ");
		PixelHistoManager::theHistoList_.push_back(tmp[0]);
    //    cout << mthn << rocName << endl;
    stringstream histoName;
    int fed  = PixelHistoManager::thePixelCalib_->getPixelNameTranslation()->getHdwAddress(*it)->fednumber();
    int chan = PixelHistoManager::thePixelCalib_->getPixelNameTranslation()->getHdwAddress(*it)->fedchannel();
    int roc  = PixelHistoManager::thePixelCalib_->getPixelNameTranslation()->getHdwAddress(*it)->fedrocnumber();
    PixelHistoManager::histoMap_  [fed][chan][roc] = tmp;
    PixelHistoManager::rocNameMap_[fed][chan][roc] = rocName;
		PixelHistoManager::rocNameHistoMap_[PixelHistoManager::rocNameMap_[fed][chan][roc]]  = &PixelHistoManager::histoMap_[fed][chan][roc];
  }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelAliveHistoManager::fillHistos(unsigned int fed,unsigned int channel,unsigned int roc,unsigned int row,unsigned int col){
  TH2F *tmpH = (TH2F*)histoMap_[fed][channel][roc][0];
  if( tmpH != 0){
    tmpH->Fill(col,row,100./PixelHistoManager::numberOfTriggers_);
//     cout << "[PixelAliveHistoManager::fillCellHisto()]\tFilling (" << row << "," << col << ")=" << 100./numberOfTriggers_ << endl;
  }
  else{
    *PixelHistoManager::logger_ << "[PixelAliveHistoManager::fillCellHisto()]\tNo histo for" 
         << " Fed="  << fed
         << " Chan=" << channel
         << " Roc="  << roc
	  		 << " Row="  << row
	  		 << " Col="  << col
	  		 << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelAliveHistoManager::drawHisto(unsigned int fed, unsigned int channel, unsigned int roc,string summary, string panelType, int plaquette, TH1* &summaryH){
  bool mirror = PixelHistoManager::rocCanvasMap_[panelType][roc].second;
	TH2F *h = (TH2F*)histoMap_[fed][channel][roc][0];
	if(summaryH == 0){
    int nRows = PixelHistoManager::moduleTypeMap_[panelType][plaquette].first;
		int nCols = PixelHistoManager::moduleTypeMap_[panelType][plaquette].second;
		summaryH = new TH2F("summary","Summary",52*nRows, -0.5, 52*nRows-0.5,80*nCols,-0.5,80*nCols-0.5);
		summaryH->SetStats(false);		
    summaryH->SetOption("COLZ");
	}
	if(h != 0){
	  if(mirror){
	  	mirrorHisto(h);
	  }
		int xOffset = 52*(PixelHistoManager::rocCanvasMap_[panelType][roc].first-1);
		int yOffset = 80*PixelHistoManager::rocCanvasMap_[panelType][roc].second;
		for(int binX=1;binX<=h->GetNbinsX();++binX){
			for(int binY=1;binY<=h->GetNbinsY();++binY){
			  if(h->GetBinContent(binX,binY) !=0 ){
//			    cout << "[PixelAliveHistoManager::drawHisto()]\tRoc: " << roc << " --> " << h->GetBinContent(binX,binY) << " sh: " << summaryH->GetName() << endl;
		      summaryH->SetBinContent(binX+xOffset,binY+yOffset,h->GetBinContent(binX,binY));
				}
			}
		}
//		h->Draw("COLZ");
	}else{
    *PixelHistoManager::logger_ << "[PixelAliveHistoManager::drawHisto()]\tNo histo for" 
          											<< " Fed="  << fed
          											<< " Chan=" << channel
          											<< " Roc="  << roc
				  											<< endl;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelAliveHistoManager::makeSummaryPlots(void){
  string mthn="[PixelAliveHistoManager::makeSummaryPlots()]\t";
  PixelHistoManager::makeSummary("PixelAlive");

//  PixelHistoManager::initializeSummaries();				********** == should be commented for using the function on this line
  gROOT->cd();								//***********
  TDirectory * summaryTreeDir_ = gROOT->mkdir("SummaryTrees");          //***********
  TDirectory * summaryDir_     = gROOT->mkdir("Summaries"); 	        //********
	
//   /////////////SUMMARY TREE DECLARATION///////////////////////
   summaryTreeDir_->cd(); 						//********
   if(summaryTree_ != 0)						//*********
   {			                                              //*********
     delete summaryTree_;                                               //*********
     summaryTree_=0;                                              //*********
   }//if 		                                             //*********
   summaryTree_ = new TTree("SummaryTree","SummaryTree");//********

  struct BadDecodingBranch{					//added from PixelHistoManager.cc
        float numberOfBadPixelsGT0;                                    //added from PixelHistoManager.cc
        float percentOfBadPixels;                                    //added from PixelHistoManager.cc
        float numberOfBadPixels;                                    //added from PixelHistoManager.cc
        char rocName[40];                                    //added from PixelHistoManager.cc
  };

  BadDecodingBranch branch_a;
  stringstream branchVariables_a;
  branchVariables_a.str("");
  branchVariables_a << "Number_of_wrongly_decoded_pixels=0/i" << ":Percentage_wrongly_decoded_pixels/F" << ":Number_wrongly_decoded_pixels/i" << ":ROCName/C";

  PixelAliveBadPixelsBranch branch_b;
  stringstream branchVariables_b;
  branchVariables_b.str("");
  branchVariables_b << "Rocs_with_number_of_dead_pixels_LT_N/i" << ":Percentage_of_dead_pixels/F" << ":Number_of_dead_pixels/i" << ":ROCName/C";

  summaryTree_->Branch("WronglyDecoded", &branch_a, branchVariables_a.str().c_str());
  summaryTree_->Branch("Pixels", &branch_b, branchVariables_b.str().c_str());


  /////////////SUMMARY HISTOS DECLARATION///////////////////////
  summaryDir_->cd();
  TH1F * hPercentageOfDeadChannelsPerRoc   = new TH1F("hPercentageOfDeadChannelsPerRoc"  ,"Percentage of dead channels per ROC"  ,1000, 0., 5.);
  summaryTreeDir_->cd();
  
//  int i = 0;										   //Should be uncommented for using initialize summaries
//  for (vector<pos::PixelROCName>::iterator it=rocList_.begin();it!=rocList_.end();it++)  //Should be uncommented for using initialize summaries
//  {											   //should be undcommented for using initialize summaries
  map<unsigned int, map< unsigned int, map<unsigned int , vector<TH1 *> > > >::iterator itFed; //added from PixelHistoManager.cc
  map<unsigned int, map<unsigned int , vector<TH1 *> > > ::iterator                    itChan;  //added from PixelHistoManager.cc
  map<unsigned int , vector<TH1 *> >::iterator                                          itRoc;  //added from PixelHistoManager.cc
  int i=0; 											//added from PixelHistoManager.cc
  for (itFed = histoMap_.begin(); itFed != histoMap_.end(); ++itFed)				//added from PixelHistoManager.cc
  {												//added from PixelHistoManager.cc
    int fed = itFed->first;									//added from PixelHistoManager.cc
    for (itChan = itFed->second.begin(); itChan != itFed->second.end(); ++itChan)		//added from PixelHistoManager.cc
    {												//added from PixelHistoManager.cc
      int channel = itChan->first;								//added from PixelHistoManager.cc
      for (itRoc = itChan->second.begin(); itRoc != itChan->second.end(); ++itRoc)		//added from PixelHistoManager.cc
      {												//added from PixelHistoManager.cc
        int roc = itRoc->first;									//added from PixelHistoManager.cc
        string rocName = rocNameMap_[fed][channel][roc];					//added from PixelHistoManager.cc
        strcpy(branch_a.rocName,rocName.c_str());						//added from PixelHistoManager.cc
        strcpy(branch_b.rocName,rocName.c_str());  
        float nOfWronglyDecoded = wrongAddressMap_[fed][channel][roc];			//added from PixelHistoManager.cc
        float percentageBAD = 100.0*nOfWronglyDecoded/(52.0*80.0);					//added from PixelHistoManager.cc
        float GT0 = 1;									//added from PixelHistoManager.cc
        if(nOfWronglyDecoded > 0)								//added from PixelHistoManager.cc
          GT0 = 0.0;										//added from PixelHistoManager.cc

        float percentageDEAD = 0;
        float numberOfBadChannelsPerRoc = 0;
        float GTN = 1;
        TH2 * tmpHisto = (TH2*)(*rocNameHistoMap_[rocName])[0];
	if(tmpHisto->GetEntries() != 0)
	{   
	  for(int binX=1; binX <= tmpHisto->GetNbinsX(); ++binX)
	  {   
	    for(int binY=1; binY <= tmpHisto->GetNbinsY(); ++binY)
	    {
	      if(tmpHisto->GetBinContent(binX,binY) < 100)
	         ++numberOfBadChannelsPerRoc;
	    }//for bin Yi
	  }//bin X
	  percentageDEAD = (100.*numberOfBadChannelsPerRoc)/(80.0*52.0);
          summaryDir_->cd();
	  hPercentageOfDeadChannelsPerRoc->Fill(percentageDEAD);
          summaryTreeDir_->cd();
	}//if tmpHisto->GetEntries(0 != 0

        if(numberOfBadChannelsPerRoc >= maxBadChannelsPerRoc_)
          GTN=0.0;

        branch_b.rocsWithNumberOfDeadPixelsGTN = GTN;
        branch_b.percentageOfDeadPixels = percentageDEAD;
        double doubleNumberOfBadChannelsPerRoc = numberOfBadChannelsPerRoc * 1.0;
        branch_b.numberOfDeadPixels     = doubleNumberOfBadChannelsPerRoc;

        branch_a.numberOfBadPixelsGT0 = GT0;							//added from PixelHistoManager.cc
        branch_a.percentOfBadPixels = percentageBAD;						//added from PixelHistoManager.cc
	double doubleNOfWronglyDecoded = nOfWronglyDecoded * 1.0;
        branch_a.numberOfBadPixels  = doubleNOfWronglyDecoded;					//added from PixelHistoManager.cc
        summaryTree_->Fill();//        branch->Fill();       //added from PixelHistoManager.cc
        i++;							//added from PixelHistoManager.cc		
      }//for itRoc						//added from PixelHistoManager.cc
    }//for itCha						//added from PixelHistoManager.cc
  }//for itFed
}
