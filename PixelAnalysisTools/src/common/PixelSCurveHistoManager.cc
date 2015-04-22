#include "PixelAnalysisTools/include/PixelSCurveHistoManager.h"
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

struct PixelSCurveBranch{
	float rocsWithThresholdGTN;       
	float rocsWithNoiseGTN; 	        
	float rocsWithChisquareGTN;
	float rocsWithProbabilityGTN;
	float threshold;	        
	float noise;			        
	float chisquare;	        
	float probability;        
	float thresholdRMS;       
	float noiseRMS; 	        
  char  rocName[38];        
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PixelSCurveHistoManager::PixelSCurveHistoManager(PixelXmlReader* xmlReader, PixelCalibConfigurationExtended *calib, PixelConfigurationsManager * configurationsManager, ostream *logger) : PixelHistoManager(xmlReader,calib,configurationsManager,logger){
  string mthn = "[PixelSCurveHistoManager::PixelSCurveHistoManager()]\t";
  numberOfHistosPerRoc_= calib->getNumberOfPixelInjected();

  fitAttempts_      	= atoi(thePixelXmlReader_->getXMLAttribute("Ntrials"    		   ,"Trials").c_str());
  startCurve_       	= atof(thePixelXmlReader_->getXMLAttribute("Startcurve" 		   ,"Value").c_str());
  endCurve_         	= atof(thePixelXmlReader_->getXMLAttribute("Endcurve"   		   ,"Value").c_str());
  maxNumberOfHistos_  = atof(thePixelXmlReader_->getXMLAttribute("SaveHistograms"    ,"Max" ).c_str());  
  noisyPixel_       	= atof(thePixelXmlReader_->getXMLAttribute("NoiseSigma"  		   ,"Min").c_str());
  rocNoiseMean_     	= atof(thePixelXmlReader_->getXMLAttribute("RocNoiseMean"      ,"Max").c_str());  
  rocThresholdMean_ 	= atof(thePixelXmlReader_->getXMLAttribute("RocThresholdMean"  ,"Max").c_str());
  rocChisquareMean_   = atof(thePixelXmlReader_->getXMLAttribute("RocChisquareMean"  ,"Max" ).c_str());  
  rocProbabilityMean_ = atof(thePixelXmlReader_->getXMLAttribute("RocProbabilityMean","Min" ).c_str());
  rocProbabilityMean_ = atof(thePixelXmlReader_->getXMLAttribute("RocProbabilityMean","Min" ).c_str());
	writeTrimOutputFile_= false;
	trimOutputFile_     = 0;
  if(thePixelXmlReader_->getXMLAttribute("OutputTrimFile","Write" ) == "Yes"){
		writeTrimOutputFile_= true;
  	string outDir = ".";
		if(getenv("BUILD_HOME") != 0){
		  int runNum=thePixelConfigurationsManager_->runNumber();
		  int runNumRounded=1000*(runNum/1000);
		  stringstream fRunNumber; 
		  fRunNumber.str("");
		  fRunNumber << "/Run_"<<runNumRounded<<"/Run_"<<runNum<<"/";
		  outDir = getenv("POS_OUTPUT_DIRS");
		  outDir += fRunNumber.str();
		  cout << mthn << "outDir="<<outDir<<endl;
		}
    stringstream trimOutFileName;
		trimOutFileName.str("");
		trimOutFileName << outDir
		                << "TrimOutputFile_Fed_" << thePixelXmlReader_->getXMLAttribute("Feds","Analyze")
										<< ".dat";
		trimOutputFile_ = new ofstream(trimOutFileName.str().c_str());
	}
	saveGoodFits_ = false;
  if(thePixelXmlReader_->getXMLAttribute("SaveGoodFits","DoIt" ) == "Yes"){
		saveGoodFits_ = true;
	}
	fitFunction_ = new TF1("myscurve",&fitfcn,0,255,2);
  fitFunction_->SetParNames("threshold","noise");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelSCurveHistoManager::init(){
  //  string mthn = "[PixelSCurveHistoManager::init()]\t";

  bookHistos();
  PixelHistoManager::init();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PixelSCurveHistoManager::~PixelSCurveHistoManager(){
  destroy();
	delete fitFunction_;
	if(trimOutputFile_ != 0){
		trimOutputFile_->close();
		delete trimOutputFile_;
	}
//  PixelHistoManager::destroy();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelSCurveHistoManager::destroy(void){
  histoNoise1DMap_      .clear();
  histoThreshold1DMap_  .clear();
  histoChisquare1DMap_  .clear();
  histoProbability1DMap_.clear();
  histoNoise2DMap_      .clear();
  histoThreshold2DMap_  .clear();
  histoChisquare2DMap_  .clear();
  histoProbability2DMap_.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelSCurveHistoManager::endOfPatternAnalysis(void){
  fit();
  reset();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelSCurveHistoManager::endOfFileAnalysis(void){
  fit();
	PixelHistoManager::deleteServiceHistos();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelSCurveHistoManager::bookHistos(void){
  string mthn = "[PixelSCurveHistoManager::bookHistos()]\t";

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
    vector<TH1*> tmp;
    string rocName =  it->rocname();
    //    cout << mthn << rocName << endl;
		if(!thePixelConfigurationsManager_->isDataToAnalyze(rocName)){continue;}
    PixelHistoManager::thePixelRootDirectoryMaker_->cdDirectory(rocName);
    stringstream tmpName;
    for(unsigned int n=0; n<numberOfHistosPerRoc_; ++n){
      tmpName.str("");
      tmpName << rocName << "_" << n;
      tmp.push_back(new TH1F(tmpName.str().c_str(),tmpName.str().c_str(),256,-0.5,255.5));
		  PixelHistoManager::theHistoList_.push_back(tmp[n]);
    }
    stringstream histoName;
    int fed  = thePixelCalib_->getPixelNameTranslation()->getHdwAddress(*it)->fednumber();
    int chan = thePixelCalib_->getPixelNameTranslation()->getHdwAddress(*it)->fedchannel();
    int roc  = thePixelCalib_->getPixelNameTranslation()->getHdwAddress(*it)->fedrocnumber();
    PixelHistoManager::histoMap_  [fed][chan][roc] = tmp;
    PixelHistoManager::rocNameMap_[fed][chan][roc] = rocName;
		PixelHistoManager::rocNameHistoMap_[PixelHistoManager::rocNameMap_[fed][chan][roc]]  = &PixelHistoManager::histoMap_[fed][chan][roc];

    histoName.str("");
    histoName << rocName << "_Threshold1D" ;
    histoThreshold1DMap_[rocName] = new TH1F(histoName.str().c_str(),histoName.str().c_str(),120,0.,120.);
		histoThreshold1DMap_[rocName]->GetXaxis()->SetTitle("Vcal");

    histoName.str("");
    histoName << rocName << "_Noise1D" ;
    histoNoise1DMap_[rocName] = new TH1F(histoName.str().c_str(),histoName.str().c_str(),50,0.,10.);
		histoNoise1DMap_[rocName]->GetXaxis()->SetTitle("Vcal");

    histoName.str("");
    histoName << rocName << "_Chisquare1D";
    histoChisquare1DMap_[rocName]  = new TH1F(histoName.str().c_str(),histoName.str().c_str(),100,0.,5);

    histoName.str("");
    histoName << rocName << "_Probability1D";
    histoProbability1DMap_[rocName]  = new TH1F(histoName.str().c_str(),histoName.str().c_str(),100,0.,1.);
 
    histoName.str("");
    histoName << rocName << "_Threshold2D";
    histoThreshold2DMap_[rocName] = new TH2F(histoName.str().c_str(),histoName.str().c_str(),
					    52,-0.5,51.5,80,-0.5,79.5);
    histoThreshold2DMap_[rocName]->GetXaxis()->SetTitle("col");
		histoThreshold2DMap_[rocName]->GetXaxis()->SetNdivisions(nXdivisions);
    histoThreshold2DMap_[rocName]->GetYaxis()->SetTitle("row");
		histoThreshold2DMap_[rocName]->GetYaxis()->SetNdivisions(nYdivisions);
    histoThreshold2DMap_[rocName]->SetStats(false);
    histoThreshold2DMap_[rocName]->SetOption("COLZ");

    histoName.str("");
    histoName << rocName << "_Noise2D";
    histoNoise2DMap_[rocName] = new TH2F(histoName.str().c_str(),histoName.str().c_str(),
					    52,-0.5,51.5,80,-0.5,79.5);
    histoNoise2DMap_[rocName]->GetXaxis()->SetTitle("col");
		histoNoise2DMap_[rocName]->GetXaxis()->SetNdivisions(nXdivisions);
    histoNoise2DMap_[rocName]->GetYaxis()->SetTitle("row");
		histoNoise2DMap_[rocName]->GetYaxis()->SetNdivisions(nYdivisions);
    histoNoise2DMap_[rocName]->SetStats(false);
    histoNoise2DMap_[rocName]->SetOption("COLZ");

    histoName.str("");
    histoName << rocName << "_Chisquare2D" ;
    histoChisquare2DMap_[rocName] = new TH2F(histoName.str().c_str(),histoName.str().c_str(),
					    52,-0.5,51.5,80,-0.5,79.5);
    histoChisquare2DMap_[rocName]->GetXaxis()->SetTitle("col");
		histoChisquare2DMap_[rocName]->GetXaxis()->SetNdivisions(nXdivisions);
    histoChisquare2DMap_[rocName]->GetYaxis()->SetTitle("row");
		histoChisquare2DMap_[rocName]->GetYaxis()->SetNdivisions(nYdivisions);
    histoChisquare2DMap_[rocName]->SetStats(false);
    histoChisquare2DMap_[rocName]->SetOption("COLZ");

    histoName.str("");
    histoName << rocName << "_Probability2D" ;
    histoProbability2DMap_[rocName] = new TH2F(histoName.str().c_str(),histoName.str().c_str(),
					    52,-0.5,51.5,80,-0.5,79.5);
    histoProbability2DMap_[rocName]->GetXaxis()->SetTitle("col");
		histoProbability2DMap_[rocName]->GetXaxis()->SetNdivisions(nXdivisions);
    histoProbability2DMap_[rocName]->GetYaxis()->SetTitle("row");
		histoProbability2DMap_[rocName]->GetYaxis()->SetNdivisions(nYdivisions);
    histoProbability2DMap_[rocName]->SetStats(false);
    histoProbability2DMap_[rocName]->SetOption("COLZ");
	}
  //Create the directories with failed fits
//  dirNoisyCells_ = rootFile_->mkdir("NoisyCells");
//  dirErrorCells_ = rootFile_->mkdir("ErrorCells");
  dirNoisyCells_ = gROOT->mkdir("NoisyCells");
  dirErrorCells_ = gROOT->mkdir("ErrorCells");
	if(saveGoodFits_){
		dirGoodFits_ = gROOT->mkdir("GoodFits");
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelSCurveHistoManager::fillHistos(unsigned int fed,unsigned int channel,
					 unsigned int roc,unsigned int row,unsigned int col,unsigned int vcalvalue, unsigned int adc){
  TH1 * tmpH = histoMap_[fed][channel][roc][cellMap_[make_pair<int,int>(row,col)]];
  if( tmpH != 0){
    tmpH->Fill(vcalvalue);
  }
  else{
    *PixelHistoManager::logger_ << "[PixelSCurveHistoManager::fillCellHisto()]\tNo histo for" 
         << " Fed="  << fed
         << " Chan=" << channel
         << " Roc="  << roc
	       << " Row="  << row
	       << " Col="  << col
	       << endl;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelSCurveHistoManager::fit(void){
  string mthn = "[PixelSCurveHistoManager::fit()]";

  //cout<<"PixelSCurveHistoManager::fit"<<endl;
  unsigned int vCalMin=int(thePixelCalib_->scanValueMin("Vcal"));
  unsigned int vCalMax=int(thePixelCalib_->scanValueMax("Vcal"));
  map<unsigned int, map< unsigned int, map<unsigned int , vector<TH1 *> > > >::iterator itFed;
  map< unsigned int, map<unsigned int , vector<TH1 *> > > ::iterator                    itChan;
  map<unsigned int , vector<TH1 *> >::iterator                                          itRoc;
  for (itFed = histoMap_.begin(); itFed != histoMap_.end(); ++itFed){		     
    for (itChan = itFed->second.begin(); itChan != itFed->second.end(); ++itChan){   
      for (itRoc = itChan->second.begin(); itRoc != itChan->second.end(); ++itRoc){  
	string rocName = rocNameMap_[itFed->first][itChan->first][itRoc->first];
	for (map<pair<int,int>,unsigned int>::iterator cellIt=cellMap_.begin();cellIt!=cellMap_.end();++cellIt){
	  int row = cellIt->first.first;
	  int col = cellIt->first.second;
	  TH1F* scurve = (TH1F*)itRoc->second[cellIt->second];
	  scurve->Scale(1./numberOfTriggers_);
	  
	  //fit
	  if(scurve==0){
	    *PixelHistoManager::logger_ << mthn << "Histo doesn't exist!Impossible!!!!!!" << endl;
	    continue;  // only fit if it exist
	  }
	  if(scurve->GetEntries()==0) {
	    //*PixelHistoManager::logger_ << mthn << "Histo empty!!!!!!" << endl;
	    continue;  // only fit if filled
	  }
	  
	  int nBins   = scurve->GetNbinsX();
          double binContent=0;
	  // Determine the starting point of the threshold curve
	  double lowBin   = 1;
	  double lowrange = vCalMin;
	  for (int bin=1; bin<=nBins; ++bin) {
	    binContent = scurve->GetBinContent(bin);
	    if ( binContent > startCurve_ ) {   //XML
	      lowBin = scurve->GetXaxis()->GetBinCenter(bin);
	      break;
	    }
	    if(binContent == 0){
	      lowrange = scurve->GetXaxis()->GetBinCenter(bin);
	    }
	  }
	  // Determine the ending point of the threshold curve
	  double highBin = scurve->GetMaximumBin();  
	  double highrange = vCalMax;  
	  for (int bin=(int)highBin; bin>=1; --bin) {
	    binContent = scurve->GetBinContent(bin);
	    if ( binContent < endCurve_ ) {   //XML
	      highBin = scurve->GetXaxis()->GetBinCenter(bin);
	      break;
	    } 
	    if(binContent == 1){
	      highrange = scurve->GetXaxis()->GetBinCenter(bin);
	    }
	  }
	  // Define the starting values of the fit parameters
	  double mean  = (highBin + lowBin) / 2.0;
	  double sigma = (highBin - lowBin) / 2.5;//Tuned manually
	  if(sigma < 1.5){
	    sigma = 1.5;
	  }
	  lowrange -= 2*sigma;
	  if (lowrange < vCalMin )
	    lowrange = lowrange;
          
	  highrange += 2*sigma;
	  if (highrange > vCalMax )
	    highrange = vCalMax;
	  
	  fitFunction_->SetRange(lowrange,highrange);
	  
	  double effo ;
	  
	  for ( int i=1; i<=scurve->GetNbinsX(); ++i ){
	    effo = scurve->GetBinContent(i);
	    if (effo>1.0) effo=2.0-effo;
	    if (effo == 0 || effo >= 1) {
	      effo = 0.1/numberOfTriggers_;
	    } 
	    scurve->SetBinError(i, TMath::Sqrt(effo*(1.0-effo)/(1.5*numberOfTriggers_)));
	  }
	  
	  // istat = 0,always, so cannot be used for fit discrimination 
	  double fmin, fedm, errdef ;
	  int npari, nparx;
	  double chi2=-999, prob=-999;
	  int istat = 0;
	  for (int i=1; i <= fitAttempts_; ++i){   //XML
	    fitFunction_->SetParameters(mean,sigma);
	    fitFunction_->SetParLimits(1,0.3,20.0);
	    scurve->Fit("myscurve","QRB");
            TVirtualFitter *fitter = TVirtualFitter::Fitter(scurve) ;
	    istat = fitter->GetStats(fmin,fedm,errdef,npari,nparx) ;

	    chi2 = fitFunction_->GetChisquare()/fitFunction_->GetNDF();
	    if(istat == 3  || chi2<rocChisquareMean_ ){
	      break;
	    }

	    sigma = fitFunction_->GetParameter(1);
	    //cout<<col<<" "<<row<<" "<<i<<" "<<sigma<<" "<<chi2<<" "<<istat<<endl;
	  }
	  
	  sigma = fitFunction_->GetParameter(1);
	  mean  = fitFunction_->GetParameter(0);
	  prob = fitFunction_->GetProb();

	  if(writeTrimOutputFile_){
	    *trimOutputFile_ << mthn << "RocName= " << rocName << " "
			     << row << " " << col << " "
			     << sigma << " "
			     << mean << " "<<istat<<" "<<chi2<<" "<<prob<<endl;
	  }

	  histoThreshold1DMap_  [rocName]->Fill(mean);
	  histoNoise1DMap_      [rocName]->Fill(sigma);
	  histoChisquare1DMap_  [rocName]->Fill(chi2);
	  histoProbability1DMap_[rocName]->Fill(prob);
	  histoThreshold2DMap_  [rocName]->Fill(col,row,mean);
	  histoNoise2DMap_      [rocName]->Fill(col,row,sigma);
	  histoChisquare2DMap_  [rocName]->Fill(col,row,chi2);
	  histoProbability2DMap_[rocName]->Fill(col,row,prob);

	  // Somehow I have to find bad fits, istat cannot be used, so fix it to 3 for OK.
	  // Look for pixels which have 2 times worse chi2 than the required ROC average
	  istat = 3; // force it to OK
	  // Faile pixels
	  if(mean > 120. )  istat = 4;
	  if(chi2 > (2*rocChisquareMean_) )  istat = 5;
	  //if(sigma > (2*rocNoiseMean_) )     istat = 6;


	  TH1* histoToSave = 0;
	  if (istat != 3 && --maxNumberOfHistos_>=0) {
	    // Error fits
	    dirErrorCells_->cd();
	    histoToSave = (TH1*)scurve->Clone();
	    cout << " Save a bad fit "<<maxNumberOfHistos_ << " RocName= " << rocName << " "
		 << row << " " << col << " "
		 << sigma << " "
		 << mean << " "<<istat<<" "<<chi2<<" "<<prob<<endl;
	    
	  } else if ( sigma>noisyPixel_  && --maxNumberOfHistos_>=0) {
	    // Npisy fits
	    dirNoisyCells_->cd();
 	    histoToSave = (TH1*)scurve->Clone();
	    cout << " Save a noisy pixel "<<maxNumberOfHistos_ << " RocName= " << rocName << " "
		 << row << " " << col << " "
		 << sigma << " "
		 << mean << " "<<istat<<" "<<chi2<<" "<<prob<<endl;


	  } else if(saveGoodFits_ && (--maxNumberOfHistos_/2)>=0 ) { // use only 1/2 of the space for good fits
	    // good fits
	    dirGoodFits_->cd();
	    histoToSave = (TH1*)scurve->Clone();
	    //cout << " Save a good fit "<<maxNumberOfHistos_ << " RocName= " << rocName << " "
	    // << row << " " << col << " "
	    // << sigma << " "
	    // << mean << " "<<istat<<" "<<chi2<<" "<<prob<<endl;

	  }
	  if(histoToSave != 0){
	    stringstream name;
	    name.str("");
	    name << rocNameMap_[itFed->first][itChan->first][itRoc->first] << "_row" << row << "_col" << col;
	    histoToSave->SetName(name.str().c_str());
	    histoToSave->SetTitle(name.str().c_str());						
	    //						histoToSave->Write();
	  }
	}
      }
    }    
  }
  saveGoodFits_ = false; 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double PixelSCurveHistoManager::fitfcn(double *x, double *par) {
  //const double sqrt2=1.41421356;
	return (0.5*(1+TMath::Erf((x[0]-par[0])/(par[1]*1.41421356))));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelSCurveHistoManager::makeSummaryPlots(void){
  string mthn = "[PixelSCurveHistoManager::makeSummaryPlots()]\t";
  PixelHistoManager::makeSummary("Threshold");
	PixelHistoManager::makeSummary("Noise");
  PixelHistoManager::makeSummary("Chisquare");
  PixelHistoManager::makeSummary("Probability");
  PixelHistoManager::makeSummary("Threshold2D");
  PixelHistoManager::makeSummary("Noise2D");
  PixelHistoManager::makeSummary("Chisquare2D");
  PixelHistoManager::makeSummary("Probability2D");

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
//  	summaryTree_ = new TTree("PixelSCurveSummary","PixelSCurveSummary");

  PixelSCurveBranch theBranch;
	stringstream branchVariables;
	branchVariables.str("");
	branchVariables <<"Rocs with threshold < "    << rocThresholdMean_   << "/F"
									<<":Rocs with noise < "       << rocNoiseMean_       << "/F"
									<<":Rocs with Chisquare < "   << rocChisquareMean_   << "/F"
									<<":Rocs with Probability > " << rocProbabilityMean_ << "/F"
	                <<":Threshold/F"
	                <<":Noise/F"
	                <<":Chisquare/F"
	                <<":Probability/F"
	                <<":ThresholdRMS/F"
	                <<":NoiseRMS/F"
									<<":ROCName/C";
  TBranch *pixelBranch = summaryTree_->Branch("Pixels",&theBranch,branchVariables.str().c_str());

  /////////////SUMMARY HISTOS DECLARATION///////////////////////
  PixelHistoManager::summaryDir_->cd();											  
  //summary plots 									  
  TH1F * hMeanThreshold 			= new TH1F("MeanThreshold",   "Mean Threshold of all the ROCs",       100,0,200);     
  TH1F * hMeanNoise     			= new TH1F("MeanNoise",       "Mean Noise of all the ROCs",           100,0,10);      
  TH1F * hMeanChisquare       = new TH1F("MeanChisquare",   "Mean Chisquare of all the ROCs",       100,0,10);       
  TH1F * hMeanProbability     = new TH1F("MeanProbability", "Mean Probability of all the ROCs",     100,0,1.);     
  TH1F * hRmsThreshold  			= new TH1F("RmsThreshold",    "Rms of the threshold of all the ROCs", 100,0,20);      
  TH1F * hRmsNoise      			= new TH1F("RmsNoise",        "Rms of the noise of all the ROCs",     100,0,2);       
  

  TH1F * hThresholdOfAllPixels = new TH1F("ThresholdOfAllPixels", "Threshold of all Pixels", 200, 0, 200);
  TH1F * hNoiseOfAllPixels     = new TH1F("NoiseOfAllPixels",  	 "Noise of all Pixels",     100, 0., 10.);
  //Initializing the map with the name of all the possible panels present
  for (vector<pos::PixelROCName>::iterator it=rocList_.begin();it!=rocList_.end();it++){
    string rocName = it->rocname();
		if(!thePixelConfigurationsManager_->isDataToAnalyze(rocName)){continue;}
		strcpy(theBranch.rocName,rocName.c_str());
    
		TH1F * tmpHistoThreshold1D = (TH1F *)histoThreshold1DMap_[rocName];
    if (tmpHistoThreshold1D->GetEntries() != 0) {
      double meanThr = tmpHistoThreshold1D->GetMean();
      hMeanThreshold->Fill(meanThr);
      if (meanThr >= rocThresholdMean_){
			  *logger_ << mthn << "ROC="<< rocName <<" Mean Threshold=" << meanThr << endl;
				theBranch.rocsWithThresholdGTN = 0;
      }else{
				theBranch.rocsWithThresholdGTN = 1;
			}
      double rmsThr = tmpHistoThreshold1D->GetRMS();
      hRmsThreshold->Fill(rmsThr);
 		  theBranch.threshold    = meanThr;
		  theBranch.thresholdRMS = rmsThr;
   }
	 else{
 	   cout << mthn << "No entries for: " << tmpHistoThreshold1D->GetName() << " rocname: " << rocName << endl;
	 }

    TH1F * tmpHistoNoise1D     = (TH1F *)histoNoise1DMap_[rocName];
    if (tmpHistoNoise1D->GetEntries()>0) {
      double meanSig = tmpHistoNoise1D->GetMean();
      hMeanNoise->Fill(meanSig);
      if (meanSig >= rocNoiseMean_){
			  *logger_ << mthn << "ROC="<< rocName <<" Mean Noise=" << meanSig << endl;
				theBranch.rocsWithNoiseGTN = 0;
      }else{
				theBranch.rocsWithNoiseGTN = 1;
			}
			double rmsSig = tmpHistoNoise1D->GetRMS();
      hRmsNoise->Fill(rmsSig);
		  theBranch.noise    = meanSig;
		  theBranch.noiseRMS = rmsSig;
    }

    TH1F * tmpHistoChisquare1D     = (TH1F *)histoChisquare1DMap_[rocName];
    if (tmpHistoChisquare1D->GetEntries()>0) {
      double meanSig = tmpHistoChisquare1D->GetMean();
      hMeanChisquare->Fill(meanSig);
      if (meanSig >= rocChisquareMean_){
			  *logger_ << mthn << "ROC="<< rocName <<" Mean Chisquare=" << meanSig << endl;
				theBranch.rocsWithChisquareGTN = 0;
      }else{
				theBranch.rocsWithChisquareGTN = 1;
			}
		  theBranch.chisquare    = meanSig;
    }

    TH1F * tmpHistoProbability1D     = (TH1F *)histoProbability1DMap_[rocName];
    if (tmpHistoProbability1D->GetEntries()>0) {
      double meanSig = tmpHistoProbability1D->GetMean();
      hMeanProbability->Fill(meanSig);
      if (meanSig <= rocProbabilityMean_){
			  *logger_ << mthn << "ROC="<< rocName <<" Mean Probability=" << meanSig << endl;
				theBranch.rocsWithProbabilityGTN = 0;
      }else{
				theBranch.rocsWithProbabilityGTN = 1;
			}
		  theBranch.probability    = meanSig;
    }

    TH2F * tmpHistoThreshold2D = (TH2F *)histoThreshold2DMap_[rocName];
    TH2F * tmpHistoNoise2D     = (TH2F *)histoNoise2DMap_[rocName];
    if(tmpHistoThreshold2D->GetEntries() != 0){
			for(int binX=1; binX <= tmpHistoThreshold2D->GetNbinsX(); ++binX){
			  for(int binY=1; binY <= tmpHistoThreshold2D->GetNbinsY(); ++binY){
			    if(tmpHistoThreshold2D->GetBinContent(binX,binY) != 0){
			      hThresholdOfAllPixels->Fill(tmpHistoThreshold2D->GetBinContent(binX,binY));
			      hNoiseOfAllPixels    ->Fill(tmpHistoNoise2D->GetBinContent(binX,binY));
			    } 
			  }
			}
    }   
		pixelBranch->Fill();
  }		  					           
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelSCurveHistoManager::drawHisto(unsigned int fed, unsigned int channel, unsigned int roc,string summary, string panelType, int plaquette, TH1* &summaryH){
  bool mirror = PixelHistoManager::rocCanvasMap_[panelType][roc].second;
	TH1 * h = 0;
	string drawOptions = "";
	if(summary == "Threshold"){
		h = histoThreshold1DMap_[rocNameMap_[fed][channel][roc]];
	}
	else if(summary == "Noise"){
		h = histoNoise1DMap_[rocNameMap_[fed][channel][roc]];
	}
	else if(summary == "Chisquare"){
		h = histoChisquare1DMap_[rocNameMap_[fed][channel][roc]];
  }
	else if(summary == "Probability"){
		h = histoProbability1DMap_[rocNameMap_[fed][channel][roc]];
  }
	else if(summary == "Threshold2D"){
		h = histoThreshold2DMap_[rocNameMap_[fed][channel][roc]];
		drawOptions = "COLZ";
  }
	else if(summary == "Noise2D"){
		h = histoNoise2DMap_[rocNameMap_[fed][channel][roc]];
		drawOptions = "COLZ";
  }
	else if(summary == "Chisquare2D"){
		h = histoChisquare2DMap_[rocNameMap_[fed][channel][roc]];
		drawOptions = "COLZ";
  }
	else if(summary == "Probability2D"){
		h = histoProbability2DMap_[rocNameMap_[fed][channel][roc]];
		drawOptions = "COLZ";
  }
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
		
//		h->Draw(drawOptions.c_str());
	}else{
    *PixelHistoManager::logger_ << "[PixelSCurveHistoManager::drawHisto()]\tNo histo for" 
          											<< " Fed="  << fed
          											<< " Chan=" << channel
          											<< " Roc="  << roc
				  											<< endl;
	}
}
