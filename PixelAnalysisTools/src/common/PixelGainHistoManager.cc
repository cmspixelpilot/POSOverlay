#include "PixelAnalysisTools/include/PixelGainHistoManager.h"
//#include "PixelAnalysisTools/include/PixelXmlReader.h"
#include "PixelUtilities/PixelXmlUtilities/include/PixelXmlReader.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include "PixelAnalysisTools/include/PixelCalibConfigurationExtended.h"
#include "PixelAnalysisTools/include/PixelConfigurationsManager.h"
#include <sstream>
#include <TH1F.h>
#include <TH2F.h>
#include <TTree.h>
#include <TMath.h>
#include <TF1.h>
#include <TDirectory.h>
#include <TROOT.h>
#include <TVirtualFitter.h>

using namespace std;

struct PixelLinearGainBranch{
	float rocsWithSlopeGTN;
	float rocsWithInterceptGTN;
	float rocsWithChisquareGTN;
	float rocsWithProbabilityGTN;
	float slope;
	float intercept;
	float chisquare;
	float probability;
	float fitStatistics;
	float slopeRMS;
	float interceptRMS;
  char  rocName[38];
};

struct PixelTanhGainBranch{
	float rocWithLinearityLTN;
	float par0;
	float par1;
	float par2;
	float par3;
	float fitStatistics;
  char  rocName[38];
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PixelGainHistoManager::PixelGainHistoManager(PixelXmlReader* xmlReader, PixelCalibConfigurationExtended *calib,  PixelConfigurationsManager * configurationsManager, ostream *logger) : PixelHistoManager(xmlReader,calib,configurationsManager,logger){
  string mthn = "[PixelGainHistoManager::PixelGainHistoManager()]\t";
  numberOfHistosPerRoc_= calib->getNumberOfPixelInjected();

  maxNumberOfHistos_          = atof(thePixelXmlReader_->getXMLAttribute("SaveHistograms"        ,"Max" ).c_str());  
  rocSlopeMean_               = atof(thePixelXmlReader_->getXMLAttribute("RocSlopeMean"          ,"Max" ).c_str());
  rocInterceptMean_   	      = atof(thePixelXmlReader_->getXMLAttribute("RocInterceptMean"      ,"Max" ).c_str());
  rocChisquareMean_           = atof(thePixelXmlReader_->getXMLAttribute("LinearChisquareMean"   ,"Max" ).c_str());
  rocProbabilityMean_ 	      = atof(thePixelXmlReader_->getXMLAttribute("LinearProbabilityMean" ,"Min" ).c_str());
  tanhLinearityMean_ 	      = atof(thePixelXmlReader_->getXMLAttribute("TanhLinearityMean"     ,"Max" ).c_str());
  fitFunctions_  	      =      thePixelXmlReader_->getXMLAttribute("FitFunctions"          ,"Functions" );
  maxNumberOfGoodHistos_      = 100.; // will need to make it flexible

  cout<<" Maximum number of histos "<<maxNumberOfHistos_<<" "<<maxNumberOfGoodHistos_<<endl;

  linearFit_= false;
  tanhFit_  = false;
  if(fitFunctions_ == "linear"){
    linearFit_ = true;
  }
  else if(fitFunctions_ == "tanh"){
    tanhFit_ = true;
  }
  else{
    linearFit_ = true;
    tanhFit_   = true;
  }

  linearFitFrom_  = atof(thePixelXmlReader_->getXMLAttribute("LinearFit","From").c_str());
  linearFitTo_    = atof(thePixelXmlReader_->getXMLAttribute("LinearFit","To"  ).c_str());
  tanhFitFrom_    = atof(thePixelXmlReader_->getXMLAttribute("TanhFit","From").c_str());
  tanhFitTo_      = atof(thePixelXmlReader_->getXMLAttribute("TanhFit","To"  ).c_str());
  squareRootNumberOfTriggers_ = TMath::Sqrt(numberOfTriggers_);

  line_  = new TF1("linearGain","[0]+x*[1]");
  line_->SetLineWidth(1);
  line_->SetLineColor(4);

  tanh_  = new TF1("tanhGain","[3]+[2]*tanh([0]*x-[1])");
  tanh_->SetLineWidth(1);
  tanh_->SetLineColor(2);
  hADCOfAllPixels_ = new TH1F("h1","h1 title",256,0,255); 
/*
	gROOT->cd();
	chisquare_= new TH1F("Chisquare","Chisquare",100,0,10);
*/

  dumpGraphs_ = 0; //disable output of the gain-graphs.txt file. It can quickly eat many GB
  if (dumpGraphs_) {
    ascfile_.open("gain-graphs.txt");
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelGainHistoManager::init(){
  //  string mthn = "[PixelGainHistoManager::init()]\t";
  bookHistos();
  PixelHistoManager::init();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PixelGainHistoManager::~PixelGainHistoManager(){
  if(line_){
		delete line_;
	}
  if(tanh_){
		delete tanh_;
	}
	destroy();
//  PixelHistoManager::destroy();

	if (dumpGraphs_) {
	  ascfile_.close();
	  // 	  outfile_->Write(); 
	  // 	  outfile_->Close();
	  // 	  delete outfile_; 
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelGainHistoManager::destroy(void){
  histoIntercept1DMap_  .clear();
  histoSlope1DMap_      .clear();
  histoProbability1DMap_.clear();
  histoIntercept2DMap_  .clear();
  histoSlope2DMap_      .clear();
  //histoProbability2DMap_.clear();
  histoADC1DMap_.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelGainHistoManager::endOfPatternAnalysis(void){
  fit();
  reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelGainHistoManager::endOfFileAnalysis(void){
  fit();
	PixelHistoManager::deleteServiceHistos();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelGainHistoManager::bookHistos(void){
  string mthn = "[PixelGainHistoManager::bookHistos()]\t";

  //Initializing the map with the name of all the possible panels present
  vector<vector<unsigned int> > allRows = thePixelCalib_->rowList();
  vector<vector<unsigned int> > allCols = thePixelCalib_->columnList();
  unsigned int fromCol = allCols.front().front();//21;
  unsigned int toCol   = allCols.back().back();  //30;
  unsigned int fromRow = allRows.front().front();//35;
  unsigned int toRow   = allRows.back().back();  //44;
	int nXdivisions = 20; 
	int nYdivisions = 28;
	nXdivisions = PixelHistoManager::setNumberOfLabels(nXdivisions,toCol-fromCol+1);
	nYdivisions = PixelHistoManager::setNumberOfLabels(nYdivisions,toRow-fromRow+1);
  //  map<pair<int,int>,TH1F *> tmp;
  // cout << mthn << "Number of ROCs: " << rocList_.size() << endl;
  for (vector<pos::PixelROCName>::iterator it=rocList_.begin();it!=rocList_.end();++it){
    string rocName =  it->rocname();
		if(!thePixelConfigurationsManager_->isDataToAnalyze(rocName)){continue;}
    vector<TH1*> tmp;
    vector<map<int,int> > tmpEntries;
    //    cout << mthn << rocName << endl;
    PixelHistoManager::thePixelRootDirectoryMaker_->cdDirectory(rocName);
    stringstream tmpName;
    for(unsigned int n=0; n<numberOfHistosPerRoc_; ++n){
      map<int,int> tmpEntry;
      tmpName.str("");
      tmpName << rocName << "_" << n;
      tmp.push_back(new TH1F(tmpName.str().c_str(),tmpName.str().c_str(),256,-0.5,255.5));
		  PixelHistoManager::theHistoList_.push_back(tmp[n]);
			std::vector<unsigned int> vcalValues = thePixelCalib_->scanValues("Vcal");
			for(unsigned int val = 0; val < vcalValues.size(); ++val){
				tmpEntry[vcalValues[val]] = 0;
			}
			tmpEntries.push_back(tmpEntry);
    }
    stringstream histoName;
    int fed  = thePixelCalib_->getPixelNameTranslation()->getHdwAddress(*it)->fednumber();
    int chan = thePixelCalib_->getPixelNameTranslation()->getHdwAddress(*it)->fedchannel();
    int roc  = thePixelCalib_->getPixelNameTranslation()->getHdwAddress(*it)->fedrocnumber();
    PixelHistoManager::histoMap_    [fed][chan][roc] = tmp;
                       entriesMap_  [fed][chan][roc] = tmpEntries;
    PixelHistoManager::rocNameMap_  [fed][chan][roc] = rocName;
    PixelHistoManager::rocNameHistoMap_[PixelHistoManager::rocNameMap_[fed][chan][roc]]  = &PixelHistoManager::histoMap_[fed][chan][roc];

        
    histoName.str("");
    histoName << rocName << "_ADC1D" ;
    histoADC1DMap_[rocName] = new TH1F(histoName.str().c_str(),histoName.str().c_str(),256,0,255);
    histoADC1DMap_[rocName]->GetXaxis()->SetTitle("Vcal");
		
    
    if(linearFit_){
      histoName.str("");
      histoName << rocName << "_Slope1D" ;
      histoSlope1DMap_[rocName] = new TH1F(histoName.str().c_str(),histoName.str().c_str(),100,0,5);
      histoSlope1DMap_[rocName]->GetXaxis()->SetTitle("Vcal");
      
      histoName.str("");
      histoName << rocName << "_Intercept1D" ;
      histoIntercept1DMap_[rocName] = new TH1F(histoName.str().c_str(),histoName.str().c_str(),100,-50.,150.);
      histoIntercept1DMap_[rocName]->GetXaxis()->SetTitle("Vcal");
      
      histoName.str("");
      histoName << rocName << "_Chisquare1D" ;
      histoChisquare1DMap_[rocName] = new TH1F(histoName.str().c_str(),histoName.str().c_str(),100,0,10);
      
      histoName.str("");
      histoName << rocName << "_Probability1D";
      histoProbability1DMap_[rocName]  = new TH1F(histoName.str().c_str(),histoName.str().c_str(),100,0.,1.);
      
      //histoName.str("");
      //histoName << rocName << "_LinearStatistics1D";
      //linearFitStatistic1DMap_[rocName]  = new TH1F(histoName.str().c_str(),histoName.str().c_str(),4,0.,4);
      
      histoName.str("");
      histoName << rocName << "_Slope2D";
      histoSlope2DMap_[rocName] = new TH2F(histoName.str().c_str(),histoName.str().c_str(),
					   52,-0.5,51.5,80,-0.5,79.5);
      histoSlope2DMap_[rocName]->GetXaxis()->SetTitle("col");
      histoSlope2DMap_[rocName]->GetXaxis()->SetNdivisions(nXdivisions);
      histoSlope2DMap_[rocName]->GetYaxis()->SetTitle("row");
      histoSlope2DMap_[rocName]->GetYaxis()->SetNdivisions(nYdivisions);
      histoSlope2DMap_[rocName]->SetStats(false);
      histoSlope2DMap_[rocName]->SetOption("COLZ");
      
      histoName.str("");
      histoName << rocName << "_Intercept2D";
      histoIntercept2DMap_[rocName] = new TH2F(histoName.str().c_str(),histoName.str().c_str(),
					       52,-0.5,51.5,80,-0.5,79.5);
      histoIntercept2DMap_[rocName]->GetXaxis()->SetTitle("col");
      histoIntercept2DMap_[rocName]->GetXaxis()->SetNdivisions(nXdivisions);
      histoIntercept2DMap_[rocName]->GetYaxis()->SetTitle("row");
      histoIntercept2DMap_[rocName]->GetYaxis()->SetNdivisions(nYdivisions);
      histoIntercept2DMap_[rocName]->SetStats(false);
      histoIntercept2DMap_[rocName]->SetOption("COLZ");
      
      //histoName.str("");
      //histoName << rocName << "_Chisquare2D";
      //histoChisquare2DMap_[rocName] = new TH2F(histoName.str().c_str(),histoName.str().c_str(),
      //				       52,-0.5,51.5,80,-0.5,79.5);
      //histoChisquare2DMap_[rocName]->GetXaxis()->SetTitle("col");
      //histoChisquare2DMap_[rocName]->GetXaxis()->SetNdivisions(nXdivisions);
      //histoChisquare2DMap_[rocName]->GetYaxis()->SetTitle("row");
      //histoChisquare2DMap_[rocName]->GetYaxis()->SetNdivisions(nYdivisions);
      //histoChisquare2DMap_[rocName]->SetStats(false);
      //histoChisquare2DMap_[rocName]->SetOption("COLZ");
      
      //histoName.str("");
      //histoName << rocName << "_Probability2D" ;
      //histoProbability2DMap_[rocName] = new TH2F(histoName.str().c_str(),histoName.str().c_str(),
      //					 52,-0.5,51.5,80,-0.5,79.5);
      //histoProbability2DMap_[rocName]->GetXaxis()->SetTitle("col");
      //histoProbability2DMap_[rocName]->GetXaxis()->SetNdivisions(nXdivisions);
      //histoProbability2DMap_[rocName]->GetYaxis()->SetTitle("row");
      //histoProbability2DMap_[rocName]->GetYaxis()->SetNdivisions(nYdivisions);
      //histoProbability2DMap_[rocName]->SetStats(false);
      //histoProbability2DMap_[rocName]->SetOption("COLZ");
      
/*
    	histoName.str("");
    	histoName << rocName << "_LinearStatistics2D" ;
    	linearFitStatistic2DMap_[rocName] = new TH2F(histoName.str().c_str(),histoName.str().c_str(),
					       toCol-fromCol+1,fromCol-0.5,toCol+0.5,toRow-fromRow+1,fromRow-0.5,toRow+0.5);
    	linearFitStatistic2DMap_[rocName]->GetXaxis()->SetTitle("col");
			linearFitStatistic2DMap_[rocName]->GetXaxis()->SetNdivisions(nXdivisions);
    	linearFitStatistic2DMap_[rocName]->GetYaxis()->SetTitle("row");
			linearFitStatistic2DMap_[rocName]->GetYaxis()->SetNdivisions(nYdivisions);
    	linearFitStatistic2DMap_[rocName]->SetStats(false);
    	linearFitStatistic2DMap_[rocName]->SetOption("COLZ");
*/
		}
		if(tanhFit_){
    	histoName.str("");
    	histoName << rocName << "_Par0_1D" ;
    	histoTanhPar01DMap_[rocName] = new TH1F(histoName.str().c_str(),histoName.str().c_str(),100,0,0.1);

    	histoName.str("");
    	histoName << rocName << "_Par1_1D" ;
    	histoTanhPar11DMap_[rocName] = new TH1F(histoName.str().c_str(),histoName.str().c_str(),100,0.,3.);

    	histoName.str("");
    	histoName << rocName << "_Par2_1D" ;
    	histoTanhPar21DMap_[rocName] = new TH1F(histoName.str().c_str(),histoName.str().c_str(),100,0.50,150);

    	histoName.str("");
    	histoName << rocName << "_Par3_1D";
    	histoTanhPar31DMap_[rocName] = new TH1F(histoName.str().c_str(),histoName.str().c_str(),100,0,200);
 
    	histoName.str("");
    	histoName << rocName << "_TanhChisquare1D";
	histoTanhChisquare1DMap_[rocName]  = new TH1F(histoName.str().c_str(),histoName.str().c_str(),100,0.,10);

    	//histoName.str("");
    	//histoName << rocName << "_TanhStatistics1D";
	//tanhFitStatistic1DMap_[rocName]  = new TH1F(histoName.str().c_str(),histoName.str().c_str(),4,0.,4);

    	//histoName.str("");
    	//histoName << rocName << "_Par0_2D";
    	//histoTanhPar02DMap_[rocName] = new TH2F(histoName.str().c_str(),histoName.str().c_str(),
	//				    52,-0.5,51.5,80,-0.5,79.5);
    	//histoTanhPar02DMap_[rocName]->GetXaxis()->SetTitle("col");
	//		histoTanhPar02DMap_[rocName]->GetXaxis()->SetNdivisions(nXdivisions);
    	//histoTanhPar02DMap_[rocName]->GetYaxis()->SetTitle("row");
	//		histoTanhPar02DMap_[rocName]->GetYaxis()->SetNdivisions(nYdivisions);
    	//histoTanhPar02DMap_[rocName]->SetStats(false);
    	//histoTanhPar02DMap_[rocName]->SetOption("COLZ");

    	histoName.str("");
    	histoName << rocName << "_Par1_2D";
    	histoTanhPar12DMap_[rocName] = new TH2F(histoName.str().c_str(),histoName.str().c_str(),
					    52,-0.5,51.5,80,-0.5,79.5);
    	histoTanhPar12DMap_[rocName]->GetXaxis()->SetTitle("col");
			histoTanhPar12DMap_[rocName]->GetXaxis()->SetNdivisions(nXdivisions);
    	histoTanhPar12DMap_[rocName]->GetYaxis()->SetTitle("row");
			histoTanhPar12DMap_[rocName]->GetYaxis()->SetNdivisions(nYdivisions);
    	histoTanhPar12DMap_[rocName]->SetStats(false);
    	histoTanhPar12DMap_[rocName]->SetOption("COLZ");

/*
    	histoName.str("");
    	histoName << rocName << "_Par2_2D";
    	histoTanhPar22DMap_[rocName] = new TH2F(histoName.str().c_str(),histoName.str().c_str(),
						    toCol-fromCol+1,fromCol-0.5,toCol+0.5,toRow-fromRow+1,fromRow-0.5,toRow+0.5);
    	histoTanhPar22DMap_[rocName]->GetXaxis()->SetTitle("col");
			histoTanhPar22DMap_[rocName]->GetXaxis()->SetNdivisions(nXdivisions);
    	histoTanhPar22DMap_[rocName]->GetYaxis()->SetTitle("row");
			histoTanhPar22DMap_[rocName]->GetYaxis()->SetNdivisions(nYdivisions);
    	histoTanhPar22DMap_[rocName]->SetStats(false);
    	histoTanhPar22DMap_[rocName]->SetOption("COLZ");

    	histoName.str("");
    	histoName << rocName << "_Par3_2D" ;
    	histoTanhPar32DMap_[rocName] = new TH2F(histoName.str().c_str(),histoName.str().c_str(),
						    toCol-fromCol+1,fromCol-0.5,toCol+0.5,toRow-fromRow+1,fromRow-0.5,toRow+0.5);
    	histoTanhPar32DMap_[rocName]->GetXaxis()->SetTitle("col");
			histoTanhPar32DMap_[rocName]->GetXaxis()->SetNdivisions(nXdivisions);
    	histoTanhPar32DMap_[rocName]->GetYaxis()->SetTitle("row");
			histoTanhPar32DMap_[rocName]->GetYaxis()->SetNdivisions(nYdivisions);
    	histoTanhPar32DMap_[rocName]->SetStats(false);
    	histoTanhPar32DMap_[rocName]->SetOption("COLZ");
    	histoName.str("");
    	histoName << rocName << "_TanhStatistics2D" ;
    	tanhFitStatistic2DMap_[rocName] = new TH2F(histoName.str().c_str(),histoName.str().c_str(),
					       toCol-fromCol+1,fromCol-0.5,toCol+0.5,toRow-fromRow+1,fromRow-0.5,toRow+0.5);
    	tanhFitStatistic2DMap_[rocName]->GetXaxis()->SetTitle("col");
			tanhFitStatistic2DMap_[rocName]->GetXaxis()->SetNdivisions(nXdivisions);
    	tanhFitStatistic2DMap_[rocName]->GetYaxis()->SetTitle("row");
			tanhFitStatistic2DMap_[rocName]->GetYaxis()->SetNdivisions(nYdivisions);
    	tanhFitStatistic2DMap_[rocName]->SetStats(false);
    	tanhFitStatistic2DMap_[rocName]->SetOption("COLZ");
*/
		}
	}
  dirErrorCells_ = gROOT->mkdir("ErrorCells");
  dirGoodCells_  = gROOT->mkdir("GoodCells");

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelGainHistoManager::reset(void){
  map<unsigned int, map<unsigned int, map<unsigned int , vector<map<int,int> > > > >::iterator itFed;
  map<unsigned int, map<unsigned int , vector<map<int,int> > > > ::iterator                    itChan;
  map<unsigned int , vector<map<int,int> > >::iterator                                         itRoc;
  for (itFed = entriesMap_.begin(); itFed != entriesMap_.end(); ++itFed){		     
    for (itChan = itFed->second.begin(); itChan != itFed->second.end(); ++itChan){   
      for (itRoc = itChan->second.begin(); itRoc != itChan->second.end(); ++itRoc){  
	for (map<pair<int,int>,unsigned int>::iterator cellIt=cellMap_.begin();cellIt!=cellMap_.end();++cellIt){
	  int row = cellIt->first.first;
	  int col = cellIt->first.second;
	  map<int,int> *tmpEntriesMap = &(entriesMap_[itFed->first][itChan->first][itRoc->first][cellMap_[make_pair<int,int>(row,col)]]);
	  for(map<int,int>::iterator it = tmpEntriesMap->begin(); it != tmpEntriesMap->end(); ++it){
            if(it->second != 0){
	      it->second = 0;
            }
	  }
	}
      }
    }
  }
  
  PixelHistoManager::reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelGainHistoManager::fillHistos(unsigned int fed,unsigned int channel,
					 unsigned int roc,unsigned int row,unsigned int col,unsigned int vcalvalue, unsigned int adc){
/*
  string mthn = "[PixelGainHistoManager::fillHistos()]\t"; 
	if(!(histoMap_.find(fed) != histoMap_.end()) ||
	   !(histoMap_[fed].find(channel) != histoMap_[fed].end()) ||
		 !(histoMap_[fed][channel].find(roc) != histoMap_[fed][channel].end()) ||
		 !(cellMap_.find(make_pair<int,int>(row,col)) != cellMap_.end())
		){
    *PixelHistoManager::logger_ << "[PixelGainHistoManager::fillCellHisto()]\tNo histo for" 
         << " Fed="  << fed
         << " Chan=" << channel
         << " Roc="  << roc
	       << " Row="  << row
	       << " Col="  << col
	       << " Adc="  << adc
				 << " name=" << PixelHistoManager::rocNameMap_[fed][channel][roc]
	       << endl;
	}
*/
	TH1 * tmpH = histoMap_[fed][channel][roc][cellMap_[make_pair<int,int>(row,col)]];
  if( tmpH != 0){
    tmpH->Fill(vcalvalue,adc);
    hADCOfAllPixels_->Fill(adc);
    histoADC1DMap_[rocNameMap_[fed][channel][roc]]->Fill(adc);

    
       entriesMap_[fed][channel][roc][cellMap_[make_pair<int,int>(row,col)]][vcalvalue]++;
  }
  else{
    *PixelHistoManager::logger_ << "[PixelGainHistoManager::fillCellHisto()]\tNo histo for" 
         << " Fed="  << fed
         << " Chan=" << channel
         << " Roc="  << roc
	       << " Row="  << row
	       << " Col="  << col
	       << " Adc="  << adc
				 << " name=" << PixelHistoManager::rocNameMap_[fed][channel][roc]
	       << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelGainHistoManager::fit(void){
  const bool PRINT = false;

  string mthn = "[PixelGainHistoManager::fit()]";
//  return;
  //cout<<"PixelGainHistoManager::fit"<<endl;
//  unsigned int vCalMin=int(thePixelCalib_->scanValueMin("Vcal"));
//  unsigned int vCalMax=int(thePixelCalib_->scanValueMax("Vcal"));
  map<unsigned int, map< unsigned int, map<unsigned int , vector<TH1 *> > > >::iterator itFed;
  map< unsigned int, map<unsigned int , vector<TH1 *> > > ::iterator                    itChan;
  map<unsigned int , vector<TH1 *> >::iterator                                          itRoc;

  for (itFed = histoMap_.begin(); itFed != histoMap_.end(); ++itFed){		     
    for (itChan = itFed->second.begin(); itChan != itFed->second.end(); ++itChan){   
      for (itRoc = itChan->second.begin(); itRoc != itChan->second.end(); ++itRoc){  
	string rocName = rocNameMap_[itFed->first][itChan->first][itRoc->first];
	if(!thePixelConfigurationsManager_->isDataToAnalyze(rocName)){continue;}
	for (map<pair<int,int>,unsigned int>::iterator cellIt=cellMap_.begin();cellIt!=cellMap_.end();++cellIt){
	  int row = cellIt->first.first;
	  int col = cellIt->first.second;
	  TH1F* gainCurve = (TH1F*)itRoc->second[cellIt->second];

	  if(PRINT) cout<<" fit fed/chan/roc "<<itFed->first<<"  "<<itChan->first<<" "<<rocName<<" pixel "<<col<<"/"<<row<<endl;

	  //fit
	  if(gainCurve==0){
	    *PixelHistoManager::logger_ << mthn << "Histo doesn't exist!Impossible!!!!!!" << endl;
	    continue;  // only fit if it exist
	  }
	  
	  if(gainCurve->GetEntries()==0){continue;}  // only fit if filled
	  
	  //				  gainCurve->Scale(1./numberOfTriggers_);
	  map<int,int> tmpEntriesMap = entriesMap_[itFed->first][itChan->first][itRoc->first][cellMap_[make_pair<int,int>(row,col)]];
	  for(map<int,int>::iterator it = tmpEntriesMap.begin(); it != tmpEntriesMap.end(); ++it){
            if(it->second != 0){
	      // ATTENTION IT IS ASSUMING THAT THE BINS ARE STARTING FROM -0.5 TO 255.5 AND THE NUMBER OF BINS IS 256
	      int bin = it->first+1;
	      gainCurve->SetBinContent(bin,gainCurve->GetBinContent(bin)/it->second);
            }
	  }
	  
	  float linearFitFrom = linearFitFrom_;
	  float tanhFitFrom   = tanhFitFrom_;
	  for (int bin = 1; bin <= gainCurve->GetNbinsX(); ++bin ){
	    if(gainCurve->GetBinContent(bin)!=0){
	      if(linearFit_ && linearFitFrom_ < gainCurve->GetXaxis()->GetBinCenter(bin)){
		linearFitFrom = gainCurve->GetXaxis()->GetBinCenter(bin);
                if(linearFitFrom >= linearFitTo_){
		  linearFitFrom = linearFitFrom_;
		}
	      }
	      if(tanhFit_ && tanhFitFrom_ < gainCurve->GetXaxis()->GetBinCenter(bin)){
		tanhFitFrom = gainCurve->GetXaxis()->GetBinCenter(bin);
                if(tanhFitFrom >= tanhFitTo_){
		  tanhFitFrom = tanhFitFrom_;
		}
	      }
	      break;
	    }
          }
	  for ( int bin=1; bin<=gainCurve->GetNbinsX(); ++bin ){
	    if(gainCurve->GetBinContent(bin)!=0){
	      gainCurve->SetBinError(bin, 6./squareRootNumberOfTriggers_);
	    }
	  }
	  double fmin, fedm, errdef ;
	  int npari, nparx;
	  int linearIstat = 3, tanhIstat   = 3;
	  double linearChis2n = 999., tanhChis2n = 999.;
	  double slope=-999., intercept=-999., p1=-999.;

	  if (dumpGraphs_) {
	    ascfile_ << gainCurve->GetName() << "_" << row << "_" << col << " "; 
	    for (int bin = 1; bin<=gainCurve->GetNbinsX(); ++bin) {
	      if (gainCurve->GetBinContent(bin)!= 0) {
		ascfile_ << bin-1 << ":" << gainCurve->GetBinContent(bin) << " "; 
	      }
	    }
	    ascfile_ << endl;
	  }
	  
          TVirtualFitter *fitter;
	  if(linearFit_){
	    line_->SetParameter(0,80) ;
            line_->SetParameter(1,2) ;
            line_->SetRange(linearFitFrom, linearFitTo_) ;
            gainCurve->Fit(line_,"QR+");
	    fitter = TVirtualFitter::Fitter(gainCurve) ;
	    linearIstat = fitter->GetStats(fmin,fedm,errdef,npari,nparx) ; // always 0 with the new root
	    //cout<<nparx<<" "<<npari<<" "<<errdef<<" "<<fedm<<" "<<fmin<<endl;
	    if(linearIstat != 0) cout<<" stat!=0 "<<linearIstat<<" for "<<rocName<<col<<"/"<<row<<endl;

	    double par0 = line_->GetParameter(0);
	    double par1 = line_->GetParameter(1);
	    double chis2 = line_->GetChisquare();
	    slope = par1; intercept = par0;

	    histoSlope1DMap_      [rocName]->Fill(par1);
	    histoIntercept1DMap_  [rocName]->Fill(par0);
	    int ndof = line_->GetNDF();
	    if(line_->GetNDF() == 0){
	      *PixelHistoManager::logger_ << "[PixelGainHistoManager::fit()]\tNDOF=0!!!!!" << rocName<<" "<<col<<"/"<<row<<endl;
	      ndof = 1;
	    }

	    linearChis2n = chis2/ndof;
	    if(chis2==0 || linearChis2n > 10.) *PixelHistoManager::logger_ << "[PixelGainHistoManager::fit()] wrong chis2! " << linearChis2n<<" "<<rocName<<" "<<col<<"/"<<row<<endl;

	    // chisquare_->Fill(line_->GetChisquare()/ndof);
	    histoChisquare1DMap_  	[rocName]->Fill(chis2/ndof);			   
	    histoProbability1DMap_	[rocName]->Fill(line_->GetProb());								   
	    //linearFitStatistic1DMap_    [rocName]->Fill(linearIstat);
	    histoSlope2DMap_      	[rocName]->Fill(col,row,par1);	   
	    histoIntercept2DMap_  	[rocName]->Fill(col,row,par0);	   
	    //histoChisquare2DMap_  	[rocName]->Fill(col,row,linearChis2); 
	    //histoProbability2DMap_	[rocName]->Fill(col,row,line_->GetProb());				   
	    //          	linearFitStatistic2DMap_[rocName]->Fill(col,row,linearIstat);

	    if(PRINT) cout<<" lin fit "<<col<<" "<<row<<" "<<linearIstat <<" "<<par0<<" "<<par1<<" "<<chis2<<" "<<ndof<<endl;

	  }


	  if(tanhFit_){
	    tanh_->SetParameter(0,0.03);//0.0047
	    tanh_->SetParameter(1,1);//1
	    tanh_->SetParameter(2,85);//27
	    tanh_->SetParameter(3,140);//31
            tanh_->SetRange(tanhFitFrom, tanhFitTo_) ;
            gainCurve->Fit(tanh_,"QR+");
            fitter = TVirtualFitter::Fitter(gainCurve) ;
	    tanhIstat = fitter->GetStats(fmin,fedm,errdef,npari,nparx) ; // always 0 with the new root
	    //cout<<nparx<<" "<<npari<<" "<<errdef<<" "<<fedm<<" "<<fmin<<endl;
	    if(tanhIstat != 0) cout<<" stat!=0 "<<tanhIstat<<" for "<<rocName<<col<<"/"<<row<<endl;

	    double par0 = tanh_->GetParameter(0);
	    double par1 = tanh_->GetParameter(1);
	    double par2 = tanh_->GetParameter(2);
	    double par3 = tanh_->GetParameter(3);
	    p1 = par1;

	    int ndof = tanh_->GetNDF();
	    double chis2 = line_->GetChisquare();
	    if(line_->GetNDF() == 0){
	      *PixelHistoManager::logger_ << "[PixelGainHistoManager::fit()]\tNDOF=0!!!!!" << rocName<<" "<<col<<"/"<<row<<endl;
	      ndof = 1;
	    }
	    tanhChis2n = chis2/ndof;

	    if(chis2==0 || tanhChis2n > 10.) *PixelHistoManager::logger_ << "[PixelGainHistoManager::fit() tanh] wrong chis2! " << tanhChis2n<<" "<<rocName<<" "<<col<<"/"<<row<<endl;

	    histoTanhChisquare1DMap_  	[rocName]->Fill(tanhChis2n);			   
	    histoTanhPar01DMap_ 	[rocName]->Fill(par0);
	    histoTanhPar11DMap_ 	[rocName]->Fill(par1);
	    histoTanhPar21DMap_ 	[rocName]->Fill(par2);
	    histoTanhPar31DMap_ 	[rocName]->Fill(par3);
	    //tanhFitStatistic1DMap_[rocName]->Fill(tanhIstat);
	    //histoTanhPar02DMap_ 	[rocName]->Fill(col,row,par0);
	    histoTanhPar12DMap_ 	[rocName]->Fill(col,row,par1);
	    // histoTanhPar22DMap_ 	[rocName]->Fill(col,row,par2);
	    // histoTanhPar32DMap_ 	[rocName]->Fill(col,row,par3);
	    // tanhFitStatistic2DMap_[rocName]->Fill(col,row,tanhIstat);

	    if(PRINT) cout<<" tanh fit "<<col<<" "<<row<<" "<<tanhIstat <<" "<<par0<<" "<<par1<<endl;

	  }
	  
	  /*				
	    if(itFed->first == 32 && itChan->first == 13 && itRoc->first == 0 && row==0){
	    //TDirectory * currentDir = (TDirectory*)gDirectory->FindObjectAny(gDirectory->GetName());
	    gROOT->cd();
	    gainCurve->Clone();
	    //currentDir->cd();
	    }
	  */   
	  // chisquare_->Fill(istat);


	  //cout<<linearIstat<<" "<<tanhIstat<<" "<<maxNumberOfHistos_ << endl;
	  static int count = 0;
	  bool bad = ( (linearChis2n <= 0.) || (linearChis2n > rocChisquareMean_) ||
		       (tanhChis2n <= 0.)  || (tanhChis2n > rocChisquareMean_) ||
		       (slope>rocSlopeMean_) || (slope<=0.) ||
		       (intercept>rocInterceptMean_) || (intercept<-20.) ||
		       (p1>tanhLinearityMean_) || (p1<=0) );
	  if(bad) {
	    count++;
	    cout<<" Bad fit for "<<rocName<<" "<<col<<"/"<<row<<" chis "<<linearChis2n<<"/"<<tanhChis2n
		<<" linear "<<slope<<"/"<<intercept<<" tanh "<<p1<<" "<<count<<endl;
	  }
	  	  
	  // Store bad fits 
	  //if( (linearIstat != 3 || tanhIstat != 3) && maxNumberOfHistos_>0 ){
	  if( bad && maxNumberOfHistos_>0 ){

	    --maxNumberOfHistos_;
	    cout<<" Save histo "<<linearIstat<<" "<<tanhIstat<<" "<<maxNumberOfHistos_ << " for "
		<<rocName<<" "<<col<<"/"<<row<<" chis "<<linearChis2n<<"/"<<tanhChis2n<<endl;
	    dirErrorCells_->cd();
	    TH1* error = (TH1*)gainCurve->Clone();
	    stringstream name;
	    name.str("");
	    name << rocNameMap_[itFed->first][itChan->first][itRoc->first] << "_row" << row << "_col" << col;
	    error->SetName(name.str().c_str());
	    error->SetTitle(name.str().c_str());
	  }
	  else if( !bad && maxNumberOfGoodHistos_>0 ) {  // Store a few good fits

	    --maxNumberOfGoodHistos_;
	    cout<<" Save good histo "<<linearIstat<<" "<<tanhIstat<<" "<<maxNumberOfGoodHistos_ << " for "
		<<rocName<<" "<<col<<"/"<<row<<" chis "<<linearChis2n<<"/"<<tanhChis2n<<endl;
	    dirGoodCells_->cd();
	    TH1* error = (TH1*)gainCurve->Clone();
	    stringstream name;
	    name.str("");
	    name << rocNameMap_[itFed->first][itChan->first][itRoc->first] << "_row" << row << "_col" << col;
	    error->SetName(name.str().c_str());
	    error->SetTitle(name.str().c_str());
	  }

	}
      }
    }    
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelGainHistoManager::makeSummaryPlots(void){
  string mthn = "[PixelGainHistoManager::makeSummaryPlots()]\t";
	PixelHistoManager::makeSummary("ADC");
        if(linearFit_) {
	  PixelHistoManager::makeSummary("Slope");
	  PixelHistoManager::makeSummary("Intercept");
	  PixelHistoManager::makeSummary("Chisquare");
	  PixelHistoManager::makeSummary("Probability");
	  //PixelHistoManager::makeSummary("LinearFitStatistics");
	  PixelHistoManager::makeSummary("Slope2D");
	  PixelHistoManager::makeSummary("Intercept2D");
	  //PixelHistoManager::makeSummary("Chisquare2D");
	  //PixelHistoManager::makeSummary("Probability2D");
	  //PixelHistoManager::makeSummary("LinearFitStatistics2D");
	}

	if(tanhFit_){
	  PixelHistoManager::makeSummary("Par0");
	  PixelHistoManager::makeSummary("Par1");
	  PixelHistoManager::makeSummary("Par2");
	  PixelHistoManager::makeSummary("Par3");
	  //PixelHistoManager::makeSummary("TanhFitStatistics");
	  //PixelHistoManager::makeSummary("Par02D");
	  PixelHistoManager::makeSummary("Par12D");
	  //	PixelHistoManager::makeSummary("Par22D");
	  //  	PixelHistoManager::makeSummary("Par32D");
	  //  	PixelHistoManager::makeSummary("TanhFitStatistics2D");
	}

	PixelHistoManager::initializeSummaries();
	//   gROOT->cd();
// 	TDirectory * summaryTreeDir = gROOT->mkdir("SummaryTrees");
// 	TDirectory * summaryDir     = gROOT->mkdir("Summaries");
// 
//   /////////////SUMMARY TREE DECLARATION///////////////////////
// 	summaryTreeDir->cd();
//   if(summaryTree_ != 0){
// 		delete summaryTree_;
// 		summaryTree_=0;
// 	}
//  	summaryTree_ = new TTree("PixelGainSummary","PixelGainSummary");
  
  PixelLinearGainBranch theLinearBranch;
  stringstream branchVariables;
  TBranch *linearBranch = 0;
  TBranch *tanhBranch = 0;
  if(linearFit_){
    branchVariables.str("");
    branchVariables <<"Rocs with Slope < "        << rocSlopeMean_       << "/F"
		    <<":Rocs with Intercept < "   << rocInterceptMean_   << "/F"
		    <<":Rocs with Chisquare < "   << rocChisquareMean_   << "/F"
		    <<":Rocs with Probability > " << rocProbabilityMean_ << "/F"
		    <<":Slope/F"
		    <<":Intercept/F"
		    <<":Chisquare/F"
		    <<":Probability/F"
		    <<":FitStatistics/F"
		    <<":SlopeRMS/F"
		    <<":InterceptRMS/F"
		    <<":ROCName/C";
    linearBranch = summaryTree_->Branch("LinearFit",&theLinearBranch,branchVariables.str().c_str());
  }
  
  PixelTanhGainBranch theTanhBranch;
  if(tanhFit_){
    branchVariables.str("");
    branchVariables <<"Rocs with Linearity < "        << tanhLinearityMean_       << "/F"
		    <<":Par0/F"
		    <<":Par1 (Linearity check)/F"
		    <<":Par2/F"
		    <<":Par3/F"
		    <<":FitStatistics/F"
		    <<":ROCName/C";
    tanhBranch = summaryTree_->Branch("TanhFit",&theTanhBranch,branchVariables.str().c_str());
  }
  
  /////////////SUMMARY HISTOS DECLARATION///////////////////////
  PixelHistoManager::summaryDir_->cd();											  
  //summary plots 									 
  //For linear fit
  TH1F * hMeanSlope = 0; 					 
  TH1F * hMeanIntercept = 0;   		 
  TH1F * hMeanChisquare = 0;   		 
  TH1F * hMeanProbability = 0;		 
  TH1F * hMeanLinearStatistics = 0; 
  TH1F * hRmsSlope = 0;  					 
  TH1F * hRmsIntercept = 0;    		 
  TH1F * hSlopeOfAllPixels = 0;      		
  TH1F * hInterceptOfAllPixels = 0;  		
  TH2F * hInterceptVsSlopeOfAllPixels = 0;  		
//  TH1F * hLinearStatisticsOfAllPixels = 0;
  TH1F * hADCOfAllPixelsSummary = 0;

  //For tanh fit
  TH1F * hMeanLinearity = 0;		
  TH1F * hMeanTanhStatistics = 0;
  TH1F * hLinearityOfAllPixels = 0;
//  TH1F * hTanhStatisticsOfAllPixels = 0;

  hADCOfAllPixelsSummary = (TH1F*)hADCOfAllPixels_->Clone(); 
  hADCOfAllPixelsSummary->SetName("ADCOfAllPixelsSummary");

  if(linearFit_){
    hMeanSlope            = new TH1F("MeanSlope",            "Mean Slope of all the ROCs",          	 100, 0, 5);
    hMeanIntercept        = new TH1F("MeanIntercept",        "Mean Intercept of all the ROCs",      	 100, -50, 150);
    hMeanChisquare        = new TH1F("MeanChisquare",        "Mean Chisquare of all the ROCs",      	 100, 0,   10);
    hMeanProbability      = new TH1F("MeanProbability",      "Mean Probability of all the ROCs",    	 100, 0,   1.);
    hMeanLinearStatistics = new TH1F("MeanLinearStatistics", "Mean Linear Statistics of all the ROCs", 100, 0,   4);
    hRmsSlope             = new TH1F("RmsSlope",             "Rms of the Slope of all the ROCs",    	 100, 0,   0.5);
    hRmsIntercept         = new TH1F("RmsIntercept",         "Rms of the Intercept of all the ROCs",	 100, 0,   40);
    
    hSlopeOfAllPixels            = new TH1F("SlopeOfAllPixels",            "Slope of all Pixels",             100, 0,   10);
    hInterceptOfAllPixels        = new TH1F("InterceptOfAllPixels",        "Intercept of all Pixels",         100, -50, 150);
    hInterceptVsSlopeOfAllPixels = new TH2F("InterceptVsSlopeOfAllPixels", "Intercept vs slope of all Pixels",100, 0,   10, 100, -50, 150);
    //  	hLinearStatisticsOfAllPixels = new TH1F("LinearStatisticsOfAllPixels", "Linear Statistics of all Pixels", 4,   0  , 4);
  }
  
  if(tanhFit_){
    hMeanLinearity             = new TH1F("MeanLinearity",             "Mean Linearity of all the ROCs",       100,0,5);
    hMeanTanhStatistics        = new TH1F("MeanTanhStatistics",        "Mean Tanh Statistics of all the ROCs", 100,0,4);
    
    hLinearityOfAllPixels      = new TH1F("LinearityOfAllPixels",      "Linearity of all Pixels",              100, 0.5,3.5);
    //  	hTanhStatisticsOfAllPixels = new TH1F("TanhStatisticsOfAllPixels", "Tanh Statistics of all Pixels",          4,0,4);
  }
  //Initializing the map with the name of all the possible panels present
  for (vector<pos::PixelROCName>::iterator it=rocList_.begin();it!=rocList_.end();it++){
    string rocName = it->rocname();
    if(!thePixelConfigurationsManager_->isDataToAnalyze(rocName)){continue;}
	
    if(linearFit_){
      strcpy(theLinearBranch.rocName,rocName.c_str());
      
      TH1F * tmpHistoSlope1D = (TH1F *)histoSlope1DMap_[rocName];
      if (tmpHistoSlope1D->GetEntries() != 0) {
	double meanThr = tmpHistoSlope1D->GetMean();
	hMeanSlope->Fill(meanThr);
	if (meanThr>rocSlopeMean_){
	  *logger_ << mthn << "ROC="<< rocName <<" Mean Slope=" << meanThr << endl;
	  theLinearBranch.rocsWithSlopeGTN = 0;
	}else{
	  theLinearBranch.rocsWithSlopeGTN = 1;
				}
	double rmsThr = tmpHistoSlope1D->GetRMS();
	hRmsSlope->Fill(rmsThr);
	theLinearBranch.slope    = meanThr;
	theLinearBranch.slopeRMS = rmsThr;
      }
      //	 	  else{
      //	  	    cout << mthn << "No entries for: " << tmpHistoSlope1D->GetName() << " rocname: " << rocName << endl;
      //	 	  }

      TH1F * tmpHistoIntercept1D     = (TH1F *)histoIntercept1DMap_[rocName];
      if (tmpHistoIntercept1D->GetEntries()>0) {
	double meanSig = tmpHistoIntercept1D->GetMean();
	hMeanIntercept->Fill(meanSig);
	if (meanSig>rocInterceptMean_){
	  *logger_ << mthn << "ROC="<< rocName <<" Mean Intercept=" << meanSig <<endl;
	  theLinearBranch.rocsWithInterceptGTN = 0;
	}else{
	  theLinearBranch.rocsWithInterceptGTN = 1;
	}
	double rmsSig = tmpHistoIntercept1D->GetRMS();
	hRmsIntercept->Fill(rmsSig);
	theLinearBranch.intercept    = meanSig;
	theLinearBranch.interceptRMS = rmsSig;
      }
      
      TH1F * tmpHistoChisquare1D     = (TH1F *)histoChisquare1DMap_[rocName];
      if (tmpHistoChisquare1D->GetEntries()>0) {
	double meanSig = tmpHistoChisquare1D->GetMean();
	hMeanChisquare->Fill(meanSig);
	if (meanSig>rocChisquareMean_){
	  *logger_ << mthn << "ROC="<< rocName <<" Mean Chis2 t=" << meanSig << endl;
	  theLinearBranch.rocsWithChisquareGTN = 0;
	}else{
	  theLinearBranch.rocsWithChisquareGTN = 1;
	}
	theLinearBranch.chisquare    = meanSig;
      }

      TH1F * tmpHistoProbability1D     = (TH1F *)histoProbability1DMap_[rocName];
      if (tmpHistoProbability1D->GetEntries()>0) {
	double meanSig = tmpHistoProbability1D->GetMean();
	hMeanProbability->Fill(meanSig);
	if (meanSig<rocProbabilityMean_){
	  *logger_ << mthn << "ROC="<< rocName <<" Mean Prob. =" << meanSig << endl;
	  theLinearBranch.rocsWithProbabilityGTN = 0;
	}else{
	  theLinearBranch.rocsWithProbabilityGTN = 1;
	}
	theLinearBranch.probability    = meanSig;
      }
      
      //  	  TH1F * tmpLinearFitStatistic1D     = (TH1F *)linearFitStatistic1DMap_[rocName];
      //   	  if (tmpLinearFitStatistic1D->GetEntries()>0) {
      //   	    double meanStat = tmpLinearFitStatistic1D->GetMean();
      //   	    hMeanLinearStatistics->Fill(meanStat);
      // 			  theLinearBranch.fitStatistics = meanStat;
      //   	  }
      
      TH2F * tmpHistoSlope2D         = (TH2F *)histoSlope2DMap_[rocName];
      TH2F * tmpHistoIntercept2D     = (TH2F *)histoIntercept2DMap_[rocName];
      //  	  TH2F * tmpLinearFitStatistic2D = (TH2F *)linearFitStatistic2DMap_[rocName];
      if(tmpHistoSlope2D->GetEntries() != 0){
	for(int binX=1; binX <= tmpHistoSlope2D->GetNbinsX(); ++binX){
	  for(int binY=1; binY <= tmpHistoSlope2D->GetNbinsY(); ++binY){
	    if(tmpHistoSlope2D->GetBinContent(binX,binY) != 0){
	      hSlopeOfAllPixels           ->Fill(tmpHistoSlope2D->GetBinContent(binX,binY));
	      hInterceptOfAllPixels       ->Fill(tmpHistoIntercept2D->GetBinContent(binX,binY));
	      hInterceptVsSlopeOfAllPixels->Fill(tmpHistoSlope2D->GetBinContent(binX,binY),tmpHistoIntercept2D->GetBinContent(binX,binY));
	      //				      hLinearStatisticsOfAllPixels->Fill(tmpLinearFitStatistic2D->GetBinContent(binX,binY));
	    } 
	  }
	}
      }   
    }
    if(tanhFit_){
      strcpy(theTanhBranch.rocName,rocName.c_str());
      
      TH1F * tmpHistoTanhPar11D = (TH1F *)histoTanhPar11DMap_[rocName];
      if (tmpHistoTanhPar11D->GetEntries() != 0) {
	double mean = tmpHistoTanhPar11D->GetMean();
	hMeanLinearity->Fill(mean);
	if (mean>tanhLinearityMean_){
	  *logger_ << mthn << "ROC="<< rocName <<" Mean Linearity=" << mean << endl;
	  theTanhBranch.rocWithLinearityLTN = 0;
	}else{
	  theTanhBranch.rocWithLinearityLTN = 1;
	}
	theTanhBranch.par1 = mean;
      }

      TH1F * tmpHistoTanhPar01D = (TH1F *)histoTanhPar01DMap_[rocName];
      if (tmpHistoTanhPar01D->GetEntries() != 0) {
	theTanhBranch.par0 = tmpHistoTanhPar01D->GetMean();
      }
      
      TH1F * tmpHistoTanhPar21D = (TH1F *)histoTanhPar21DMap_[rocName];
      if (tmpHistoTanhPar21D->GetEntries() != 0) {
	theTanhBranch.par2 = tmpHistoTanhPar21D->GetMean();
      }
      
      TH1F * tmpHistoTanhPar31D = (TH1F *)histoTanhPar31DMap_[rocName];
      if (tmpHistoTanhPar31D->GetEntries() != 0) {
	theTanhBranch.par3 = tmpHistoTanhPar31D->GetMean();
      }
      
//  	  TH1F * tmpTanhFitStatistic1D     = (TH1F *)tanhFitStatistic1DMap_[rocName];
//   	  if (tmpTanhFitStatistic1D->GetEntries()>0) {
//   	    double meanStat = tmpTanhFitStatistic1D->GetMean();
//   	    hMeanTanhStatistics->Fill(meanStat);
// 			  theTanhBranch.fitStatistics = meanStat;
//   	  }

  	  TH2F * tmpLinearity2D        = (TH2F *)histoTanhPar12DMap_[rocName];
//  	  TH2F * tmpTanhFitStatistic2D = (TH2F *)tanhFitStatistic2DMap_[rocName];
  	  if(tmpLinearity2D->GetEntries() != 0){
				for(int binX=1; binX <= tmpLinearity2D->GetNbinsX(); ++binX){
				  for(int binY=1; binY <= tmpLinearity2D->GetNbinsY(); ++binY){
				    if(tmpLinearity2D->GetBinContent(binX,binY) != 0){
				      hLinearityOfAllPixels     ->Fill(tmpLinearity2D->GetBinContent(binX,binY));
//				      hTanhStatisticsOfAllPixels->Fill(tmpTanhFitStatistic2D->GetBinContent(binX,binY));
				    } 
				  }
				}
  	  }   
    }
		linearBranch->Fill();
		tanhBranch->Fill();
  }		  					           
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelGainHistoManager::drawHisto(unsigned int fed, unsigned int channel, unsigned int roc,string summary, string panelType, int plaquette, TH1* &summaryH){

  bool mirror = PixelHistoManager::rocCanvasMap_[panelType][roc].second;

  TH1 * h = 0;
  string drawOptions = "";

  //cout<<summary<<endl;

	if(summary == "ADC"){
	  h = histoADC1DMap_[rocNameMap_[fed][channel][roc]];
	}
	else if(summary == "Slope"){
		h = histoSlope1DMap_[rocNameMap_[fed][channel][roc]];
	}
	else if(summary == "Intercept"){
		h = histoIntercept1DMap_[rocNameMap_[fed][channel][roc]];
	}
	else if(summary == "Chisquare"){
		h = histoChisquare1DMap_[rocNameMap_[fed][channel][roc]];
	}
	else if(summary == "Probability"){
		h = histoProbability1DMap_[rocNameMap_[fed][channel][roc]];
	}
	//else if(summary == "LinearFitStatistics"){
	//h = linearFitStatistic1DMap_[rocNameMap_[fed][channel][roc]];
	//}
	else if(summary == "Slope2D"){
		h = histoSlope2DMap_[rocNameMap_[fed][channel][roc]];
		drawOptions = "COLZ";
	}
	else if(summary == "Intercept2D"){
		h = histoIntercept2DMap_[rocNameMap_[fed][channel][roc]];
		drawOptions = "COLZ";
	}
	//else if(summary == "Chisquare2D"){
	//h = histoChisquare2DMap_[rocNameMap_[fed][channel][roc]];
	//drawOptions = "COLZ";
	//}
	//else if(summary == "Probability2D"){
	//h = histoProbability2DMap_[rocNameMap_[fed][channel][roc]];
	//drawOptions = "COLZ";
	//}
//	else if(summary == "LinearFitStatistics2D"){
//		h = linearFitStatistic2DMap_[rocNameMap_[fed][channel][roc]];
//		drawOptions = "COLZ";
//  }
	else if(summary == "Par0"){
		h = histoTanhPar01DMap_[rocNameMap_[fed][channel][roc]];
	}
	else if(summary == "Par1"){
		h = histoTanhPar11DMap_[rocNameMap_[fed][channel][roc]];
	}
	else if(summary == "Par2"){
		h = histoTanhPar21DMap_[rocNameMap_[fed][channel][roc]];
	}
	else if(summary == "Par3"){
		h = histoTanhPar31DMap_[rocNameMap_[fed][channel][roc]];
	}
	//else if(summary == "TanhFitStatistics"){
	//h = tanhFitStatistic1DMap_[rocNameMap_[fed][channel][roc]];
	//}
	//else if(summary == "Par02D"){
	//h = histoTanhPar02DMap_[rocNameMap_[fed][channel][roc]];
	//drawOptions = "COLZ";
	//}

	else if(summary == "Par12D"){
		h = histoTanhPar12DMap_[rocNameMap_[fed][channel][roc]];
		drawOptions = "COLZ";
	}

//	else if(summary == "Par22D"){
//		h = histoTanhPar22DMap_[rocNameMap_[fed][channel][roc]];
//		drawOptions = "COLZ";
//  }
//	else if(summary == "Par32D"){
//		h = histoTanhPar32DMap_[rocNameMap_[fed][channel][roc]];
//		drawOptions = "COLZ";
// }
//	else if(summary == "TanhFitStatistics2D"){
//		h = tanhFitStatistic2DMap_[rocNameMap_[fed][channel][roc]];
//		drawOptions = "COLZ";
//  }
	
	else { cout<<" unrecognized option "<<summary<<endl; return; }

	if(h != 0){
	  if(summaryH == 0){
	    if(h->InheritsFrom(TH2::Class())){
	      int nRows = PixelHistoManager::moduleTypeMap_[panelType][plaquette].first;
	      int nCols = PixelHistoManager::moduleTypeMap_[panelType][plaquette].second;
	      summaryH = new TH2F("summary","Summary",52*nRows, -0.5, 52*nRows-0.5,80*nCols,-0.5,80*nCols-0.5);
	      summaryH->SetStats(false);
	      summaryH->SetOption("COLZ");
	    }
	    else{
	      summaryH = (TH1*)h->Clone();
	      return;
	    }
	  }
	  if(h->InheritsFrom(TH2::Class())){
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
	  }
	  else{
	    for(int binX=1;binX<=h->GetNbinsX();binX++){
	      if(h->GetBinContent(binX) != 0){
		float x=h->GetBinCenter(binX);
		unsigned int binEntries = (unsigned int)h->GetBinContent(binX);
		for(unsigned int entry=0; entry<binEntries;entry++){
		  //				  	summaryH->SetBinContent(binX,summaryH->GetBinContent(binX)+h->GetBinContent(binX));
		  summaryH->Fill(x);
		}
	      }
	    }
	  }
	}else {
	  *PixelHistoManager::logger_ << "[PixelGainHistoManager::drawHisto()]\tNo histo for" 
				      << " Fed="  << fed
				      << " Chan=" << channel
				      << " Roc="  << roc
				      << endl;
	}
}
