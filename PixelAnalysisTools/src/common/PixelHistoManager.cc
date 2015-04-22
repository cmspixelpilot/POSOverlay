#include "PixelAnalysisTools/include/PixelHistoManager.h"
#include "PixelAnalysisTools/include/PixelCalibConfigurationExtended.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelHistoReadWriteFile.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include "PixelUtilities/PixelXmlUtilities/include/PixelXmlReader.h"
#include "PixelAnalysisTools/include/PixelConfigurationsManager.h"

#include <sstream>
#include <TH1.h>
#include <TH2.h>
#include <TTree.h>
#include <TBranch.h>
#include <TDirectory.h>
#include <TROOT.h>
#include <TCanvas.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PixelHistoManager::PixelHistoManager(PixelXmlReader* xmlReader, PixelCalibConfigurationExtended *calib, PixelConfigurationsManager * configurationsManager, std::ostream *logger){
  string mthn = "[PixelHistoManager::PixelHistoManager()]\t";
//	cout << mthn << endl;
  logger_               				 = logger;
  thePixelCalib_        				 = calib;
  numberOfTriggers_     				 = thePixelCalib_->nTriggersPerPattern();
  rocList_              				 = thePixelCalib_->rocList();
	thePixelXmlReader_    				 = xmlReader;
  thePixelConfigurationsManager_ = configurationsManager;
	numberOfHistosPerRoc_= 1;
  std::vector<pos::PixelROCName> rocToAnalyzeList;
  for (vector<pos::PixelROCName>::iterator it=rocList_.begin();it!=rocList_.end();it++){
    string rocName =  it->rocname();
		if(!thePixelConfigurationsManager_->isDataToAnalyze(rocName)){continue;}
		rocToAnalyzeList.push_back(*it);	
    int fed  = thePixelCalib_->getPixelNameTranslation()->getHdwAddress(*it)->fednumber();
    int chan = thePixelCalib_->getPixelNameTranslation()->getHdwAddress(*it)->fedchannel();
    int roc  = thePixelCalib_->getPixelNameTranslation()->getHdwAddress(*it)->fedrocnumber();
    wrongAddressMap_[fed][chan][roc] = 0;
	}  
  vector<pair<unsigned int, vector<unsigned int> > > fedChannelsToAnalyzeList;
	map<unsigned int, map<unsigned int, map<unsigned int , unsigned int > > >::iterator fedIt;
	map<unsigned int, map<unsigned int , unsigned int > > ::iterator fedChannelIt;
	unsigned int vectorPos=0;
	for(fedIt=wrongAddressMap_.begin(); fedIt!=wrongAddressMap_.end();++fedIt){
		vector<unsigned int> tmpV;
		fedChannelsToAnalyzeList.push_back(make_pair<unsigned int, vector<unsigned int> >(fedIt->first,tmpV));
		for(fedChannelIt=fedIt->second.begin(); fedChannelIt!=fedIt->second.end();++fedChannelIt){
			fedChannelsToAnalyzeList[vectorPos].second.push_back(fedChannelIt->first);
		}
		++vectorPos;
	}
	thePixelRootDirectoryMaker_ = new PixelRootDirectoryMaker(rocToAnalyzeList);
	thePixelRootDirectoryMakerFED_ = new PixelRootDirectoryMaker(fedChannelsToAnalyzeList);
	setRocCanvasMap();
	setModuleTypeMap();
	summaryTree_ = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PixelHistoManager::~PixelHistoManager(){
	destroy();
	delete thePixelRootDirectoryMaker_;
	delete thePixelRootDirectoryMakerFED_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelHistoManager::init(void){
//   string mthn = "[PixelHistoManager::init()]\t";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelHistoManager::destroy(void){
  //cout << "PixelHistoManager::destroy" << endl;
//   for(vector<TH1F*>::iterator it=theHistoList_.begin(); it != theHistoList_.end(); it++){
//     if (*it) delete *it;
//   }
  if(summaryTree_ != 0){
		delete summaryTree_;
		summaryTree_=0;
	}
  theHistoList_.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelHistoManager::reset(void){
//  string mthn = "[PixelHistoManager::reset()]\t";
  for(vector<TH1*>::iterator it=theHistoList_.begin(); it != theHistoList_.end(); it++){
    (*it)->Reset();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelHistoManager::deleteServiceHistos(void){
  for(vector<TH1*>::iterator it=theHistoList_.begin(); it != theHistoList_.end(); it++){
    delete (*it);
  }
	theHistoList_.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelHistoManager::setPatternHistos(unsigned int eventNumber){
//  string mthn = "[PixelHistoManager::setCellHisto()]\t";
  std::vector<unsigned int> const *rows=0;
  std::vector<unsigned int> const *cols=0;
  thePixelCalib_->getRowsAndCols(eventNumber/thePixelCalib_->nTriggersPerPattern(),rows,cols);
  unsigned int position=0;
  cellMap_.clear();
  for (vector<unsigned int>::const_iterator itr=rows->begin(); itr!=rows->end();itr++){
    for (vector<unsigned int>::const_iterator itc=cols->begin(); itc!=cols->end();itc++){
      cellMap_[make_pair<int, int>(*itr,*itc)] = position++;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelHistoManager::mirrorHisto(TH1 * histo){
  string invertString = "(inv)";
  TH1 * tmpH = 0;
  stringstream label;
  //  cout << "[PixelHistoManager::getMirrorHisto()]\tName:" << histo->GetTitle() << endl;

  if(histo != 0){
    if((int)string(histo->GetTitle()).find(invertString.c_str()) >= 0){return;}
    tmpH = (TH1*)histo->Clone();
    stringstream name;
//     name << "FED=" << fed << " Ch=" << channel << " ROC=" << roc << " (inv)";
    name << histo->GetName() << " " << invertString;
    histo->SetName(name.str().c_str());
    histo->SetTitle(name.str().c_str());

    if(histo->InheritsFrom(TH2::Class())) {
      int nBinsX = histo->GetNbinsX();
      int nBinsY = histo->GetNbinsY();
      int nXLabels = 10;
      int nYLabels = 14;
      setNumberOfLabels(nXLabels,nBinsX);
      setNumberOfLabels(nYLabels,nBinsY);
      TAxis *xAxis = histo->GetXaxis();
      TAxis *yAxis = histo->GetYaxis();
      xAxis->SetLabelSize(0.06);
      yAxis->SetLabelSize(0.06);
      for(int binX=1; binX<=nBinsX;++binX){
			  if((nBinsX-binX)%(nBinsX/nXLabels) == 0){
					label.str("");
					label << histo->GetXaxis()->GetBinCenter(nBinsX-binX+1);
					histo->GetXaxis()->SetBinLabel(binX,label.str().c_str());
				}
				for(int binY=1; binY<=nBinsY;++binY){
			  	if((nBinsY-binY)%(nBinsY/nYLabels) == 0){
			  	  label.str("");
					  label << histo->GetYaxis()->GetBinCenter(nBinsY-binY+1);
					  histo->GetYaxis()->SetBinLabel(binY,label.str().c_str());
					}
					histo->SetBinContent(binX,binY,tmpH->GetBinContent(nBinsX-binX+1,nBinsY-binY+1));
				}
      }
    }else{
      for(int binX=1; binX<=histo->GetNbinsX();++binX){
	      histo->SetBinContent(binX,tmpH->GetBinContent(tmpH->GetNbinsX()-binX+1));
      }
    }
		tmpH->Delete();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelHistoManager::saveHistos(void){
  string mthn = "[PixelHistoManager::saveHistos()]\t";
	stringstream rootFileName;
	rootFileName.str("");
	rootFileName << thePixelConfigurationsManager_->getRunFilesDir();
	string xmlFileName = thePixelXmlReader_->getXMLAttribute("RootOutputFile","FileName");
	if(xmlFileName == "Default"){
	  rootFileName << thePixelXmlReader_->getXMLAttribute("Calibration","Type") << "_Fed_"
                 << thePixelXmlReader_->getXMLAttribute("Feds","Analyze")
                 << "_Run_" << currentRunNumber_
                 << ".root";
	}
	else{
		rootFileName << xmlFileName;
	}
	PixelHistoReadWriteFile histoWriter;
	cout << mthn << "Saving file, please wait..." << endl;
 	histoWriter.write(rootFileName.str());
	cout << mthn << "Saved results in " << rootFileName.str() << endl;
//	histoWriter.open(rootFileName_,"RECREATE");
//	histoWriter.transferFromTo(gROOT,histoWriter.getFile());
//	histoWriter.close();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelHistoManager::initializeSummaries(void){
  string mthn = "[PixelHistoManager::initializeSummaries()]\t";
	gROOT->cd();
	summaryTreeDir_ = gROOT->mkdir("SummaryTrees");
	summaryDir_     = gROOT->mkdir("Summaries");
	
  /////////////SUMMARY TREE DECLARATION///////////////////////
	summaryTreeDir_->cd();
  if(summaryTree_ != 0){
		delete summaryTree_;
		summaryTree_=0;
	}
 	summaryTree_ = new TTree("SummaryTree","SummaryTree");

	struct BadDecodingBranch{
		float numberOfBadPixelsGT0;
		float percentOfBadPixels;
		float numberOfBadPixels;
	  char  rocName[38];
	};
	BadDecodingBranch theBranch;
	stringstream branchVariables;
	branchVariables.str("");
	branchVariables << "Number of wrongly decoded pixels = 0/F"
									<< ":Percentage wrongly decoded pixels/F"
	                << ":Number wrongly decoded pixels/F"
									<< ":ROCName/C";
  TBranch * branch = summaryTree_->Branch("WronglyDecoded",&theBranch,branchVariables.str().c_str());

  map<unsigned int, map< unsigned int, map<unsigned int , vector<TH1 *> > > >::iterator itFed;
  map<unsigned int, map<unsigned int , vector<TH1 *> > > ::iterator                    itChan;
  map<unsigned int , vector<TH1 *> >::iterator                                          itRoc;
	for (itFed = histoMap_.begin(); itFed != histoMap_.end(); ++itFed){
    int fed = itFed->first; 
    for (itChan = itFed->second.begin(); itChan != itFed->second.end(); ++itChan){
      int channel = itChan->first; 
			for (itRoc = itChan->second.begin(); itRoc != itChan->second.end(); ++itRoc){
        int roc = itRoc->first;
				string rocName = rocNameMap_[fed][channel][roc];
		    strcpy(theBranch.rocName,rocName.c_str());
				unsigned int nOfWronglyDecoded = wrongAddressMap_[fed][channel][roc];
        if(nOfWronglyDecoded > 0){
  		    theBranch.numberOfBadPixelsGT0 = 0;
		    }
		    else{
  		    theBranch.numberOfBadPixelsGT0 = 1;
		   }
		   theBranch.percentOfBadPixels = 100*nOfWronglyDecoded/(52*80);
		   theBranch.numberOfBadPixels  = nOfWronglyDecoded;
		   branch->Fill();
      }
    }
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelHistoManager::makeSummary(string summary){
  string mthn="[PixelHistoManager::makeSummary()]\t";
  //save plots
//  cout << mthn << "Making Summary plots" << endl;

  map<unsigned int, map< unsigned int, map<unsigned int , vector<TH1 *> > > >::iterator itFed;
  map<unsigned int, map<unsigned int , vector<TH1 *> > > ::iterator                     itChan;
  map<unsigned int , vector<TH1 *> >::iterator                                          itRoc;
//No more  map<unsigned int,TCanvas*> canvasMap;
  map<unsigned int,TH1*> canvasMap;
  map<unsigned int,string> canvasNames;

  stringstream fedDirName;
  stringstream channelDirName;
	for (itFed = histoMap_.begin(); itFed != histoMap_.end(); ++itFed){
    int fed = itFed->first; 
//    gROOT->cd();
//    fedDirName.str("");
//    fedDirName << "FED" << fed;
// 		TDirectory * fedDir = (TDirectory*)gDirectory->FindObjectAny(fedDirName.str().c_str());
//		if(fedDir == 0){
//		  fedDir = gDirectory->mkdir(fedDirName.str().c_str());
//		}
//    fedDir->cd();
    for (itChan = itFed->second.begin(); itChan != itFed->second.end(); ++itChan){
      int channel = itChan->first; 
      if(!thePixelConfigurationsManager_->isChannelToAnalyze(fed,channel)){continue;}
//      channelDirName.str("");
//      channelDirName << fedDirName.str() << "_Channel" << channel; 
//			TDirectory * channelDir = (TDirectory*)gDirectory->FindObjectAny(channelDirName.str().c_str());
//			if(channelDir == 0){
//			  channelDir = fedDir->mkdir(channelDirName.str().c_str());
//			}
//      channelDir->cd() ;
			thePixelRootDirectoryMakerFED_->cdDirectory(fed,channel);
      string panelType = thePixelCalib_->getPanelType(fed,channel);
			string plaquetteName = rocNameMap_[fed][channel].begin()->second;
			plaquetteName = plaquetteName.substr(0,plaquetteName.find("_ROC"));
			string panelName = plaquetteName.substr(0,plaquetteName.find_last_of("_"));
			//*logger_ << mthn 
			//   		 << "FED="         << fed 
			//   		 << " Channel "    << channel 
			//  		 << " panelType="  << panelType 
			//			 << " PanelName: " << panelName
			//  		 << endl;
			for(unsigned int module=1; module<=4; ++module){
    		canvasMap[module]=0;
				canvasNames[module]="";
			}
			for (itRoc = itChan->second.begin(); itRoc != itChan->second.end(); ++itRoc){
        int roc = itRoc->first;
				string rocName = rocNameMap_[fed][channel][roc];
    		plaquetteName = rocName.substr(0,rocName.find("_ROC"));
        pos::PixelROCName tmpPixelROCName(rocName);
				int plaquette = 1;
        if(tmpPixelROCName.detsub() == 'F'){
          plaquette = tmpPixelROCName.plaquet();
        }
        else{
 			    panelName = plaquetteName;
        }
        int nRows = moduleTypeMap_[panelType][plaquette].first;
				int nCols = moduleTypeMap_[panelType][plaquette].second;
				stringstream canvasName;
				canvasName.str("");
				canvasName << panelName << "_c" << panelType << nCols << "x" << nRows << "_" << summary;
        if(canvasName.str() != canvasNames[plaquette]){
//          cout << mthn << "CanvasName: " << canvasName.str() << endl;
	  canvasNames[plaquette] = canvasName.str();
	  stringstream canvasTitle;
	  canvasTitle.str("");
	  canvasTitle << panelName << " " << panelType << " - " << nCols << "x" << nRows << "_" << summary;
	  drawHisto(fed,channel,roc,summary,panelType,plaquette,canvasMap[plaquette]);
	  //					cout << canvasMap[plaquette] << endl;
	  canvasMap[plaquette]->SetName(canvasName.str().c_str());
	  canvasMap[plaquette]->SetTitle(canvasTitle.str().c_str());

//No more     	  	canvasMap[plaquette] = new TCanvas(canvasName.str().c_str(), canvasTitle.str().c_str(),250*nRows,300*nCols);
//No more 					gDirectory->Append(canvasMap[plaquette]);
//No more       		canvasMap[plaquette]->Divide(nRows,nCols) ;
				}
//No more 				canvasMap[plaquette]->cd(rocCanvasMap_[panelType][roc].first);
				//if 1 that means we take the mirror histo
				else{
					drawHisto(fed,channel,roc,summary,panelType,plaquette,canvasMap[plaquette]);
				}
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelHistoManager::setWrongAddress(unsigned int fed, unsigned int channel, unsigned int roc){
	wrongAddressMap_[fed][channel][roc]++;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int PixelHistoManager::setNumberOfLabels(int &nLabels, int nBins){
	if(nBins < nLabels){
  	nLabels = nBins;
		return nLabels;
	}
	return 500+nLabels;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelHistoManager::setModuleTypeMap(){
	moduleTypeMap_["4R"][1]  = make_pair<unsigned int,unsigned int>(2,1);
	moduleTypeMap_["4R"][2]  = make_pair<unsigned int,unsigned int>(3,2);
	moduleTypeMap_["4R"][3]  = make_pair<unsigned int,unsigned int>(4,2);
	moduleTypeMap_["4R"][4]  = make_pair<unsigned int,unsigned int>(5,1);
	
	moduleTypeMap_["4L"][1]  = make_pair<unsigned int,unsigned int>(2,1);
	moduleTypeMap_["4L"][2]  = make_pair<unsigned int,unsigned int>(3,2);
	moduleTypeMap_["4L"][3]  = make_pair<unsigned int,unsigned int>(4,2);
	moduleTypeMap_["4L"][4]  = make_pair<unsigned int,unsigned int>(5,1);
	
	moduleTypeMap_["3R"][1]  = make_pair<unsigned int,unsigned int>(3,2);
	moduleTypeMap_["3R"][2]  = make_pair<unsigned int,unsigned int>(4,2);
	moduleTypeMap_["3R"][3]  = make_pair<unsigned int,unsigned int>(5,2);
	
	moduleTypeMap_["3L"][1]  = make_pair<unsigned int,unsigned int>(3,2);
	moduleTypeMap_["3L"][2]  = make_pair<unsigned int,unsigned int>(4,2);
	moduleTypeMap_["3L"][3]  = make_pair<unsigned int,unsigned int>(5,2);

	moduleTypeMap_["H"] [1]  = make_pair<unsigned int,unsigned int>(8,1);

	moduleTypeMap_["FA"][1]  = make_pair<unsigned int,unsigned int>(8,1);

	moduleTypeMap_["FB"][1]  = make_pair<unsigned int,unsigned int>(8,1);

 	moduleTypeMap_["F"] [1]  = make_pair<unsigned int,unsigned int>(8,2);
} 													 
														 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PixelHistoManager::setRocCanvasMap(){
	rocCanvasMap_["4R"][0]  = make_pair<unsigned int,bool>(5,0);
	rocCanvasMap_["4R"][1]  = make_pair<unsigned int,bool>(4,0);
	rocCanvasMap_["4R"][2]  = make_pair<unsigned int,bool>(3,0);
	rocCanvasMap_["4R"][3]  = make_pair<unsigned int,bool>(2,0);
	rocCanvasMap_["4R"][4]  = make_pair<unsigned int,bool>(1,0);
	rocCanvasMap_["4R"][5]  = make_pair<unsigned int,bool>(4,1);
	rocCanvasMap_["4R"][6]  = make_pair<unsigned int,bool>(3,1);
	rocCanvasMap_["4R"][7]  = make_pair<unsigned int,bool>(2,1);
	rocCanvasMap_["4R"][8]  = make_pair<unsigned int,bool>(1,1);
	rocCanvasMap_["4R"][9]  = make_pair<unsigned int,bool>(1,0);//5
	rocCanvasMap_["4R"][10] = make_pair<unsigned int,bool>(2,0);//6
	rocCanvasMap_["4R"][11] = make_pair<unsigned int,bool>(3,0);//7
	rocCanvasMap_["4R"][12] = make_pair<unsigned int,bool>(4,0);//8
	rocCanvasMap_["4R"][13] = make_pair<unsigned int,bool>(3,1);
	rocCanvasMap_["4R"][14] = make_pair<unsigned int,bool>(2,1);
	rocCanvasMap_["4R"][15] = make_pair<unsigned int,bool>(1,1);
	rocCanvasMap_["4R"][16] = make_pair<unsigned int,bool>(1,0);//4
	rocCanvasMap_["4R"][17] = make_pair<unsigned int,bool>(2,0);//5
	rocCanvasMap_["4R"][18] = make_pair<unsigned int,bool>(3,0);//6
	rocCanvasMap_["4R"][19] = make_pair<unsigned int,bool>(2,0);//2,1
	rocCanvasMap_["4R"][20] = make_pair<unsigned int,bool>(1,0);//1,1
	
	rocCanvasMap_["4L"][0]  = make_pair<unsigned int,bool>(2,0);//2,1
	rocCanvasMap_["4L"][1]  = make_pair<unsigned int,bool>(1,0);//1,1
	rocCanvasMap_["4L"][2]  = make_pair<unsigned int,bool>(1,0);//4
	rocCanvasMap_["4L"][3]  = make_pair<unsigned int,bool>(2,0);//5
	rocCanvasMap_["4L"][4]  = make_pair<unsigned int,bool>(3,0);//6
	rocCanvasMap_["4L"][5]  = make_pair<unsigned int,bool>(3,1);
	rocCanvasMap_["4L"][6]  = make_pair<unsigned int,bool>(2,1);
	rocCanvasMap_["4L"][7]  = make_pair<unsigned int,bool>(1,1);
	rocCanvasMap_["4L"][8]  = make_pair<unsigned int,bool>(1,0);//5
	rocCanvasMap_["4L"][9]  = make_pair<unsigned int,bool>(2,0);//6
	rocCanvasMap_["4L"][10] = make_pair<unsigned int,bool>(3,0);//7
	rocCanvasMap_["4L"][11] = make_pair<unsigned int,bool>(4,0);//8
	rocCanvasMap_["4L"][12] = make_pair<unsigned int,bool>(4,1);
	rocCanvasMap_["4L"][13] = make_pair<unsigned int,bool>(3,1);
	rocCanvasMap_["4L"][14] = make_pair<unsigned int,bool>(2,1);
	rocCanvasMap_["4L"][15] = make_pair<unsigned int,bool>(1,1);
	rocCanvasMap_["4L"][16] = make_pair<unsigned int,bool>(5,0);
	rocCanvasMap_["4L"][17] = make_pair<unsigned int,bool>(4,0);
	rocCanvasMap_["4L"][18] = make_pair<unsigned int,bool>(3,0);
	rocCanvasMap_["4L"][19] = make_pair<unsigned int,bool>(2,0);
	rocCanvasMap_["4L"][20] = make_pair<unsigned int,bool>(1,0);
	
	rocCanvasMap_["3R"][0]  = make_pair<unsigned int,bool>(5,1);
	rocCanvasMap_["3R"][1]  = make_pair<unsigned int,bool>(4,1);
	rocCanvasMap_["3R"][2]  = make_pair<unsigned int,bool>(3,1);
	rocCanvasMap_["3R"][3]  = make_pair<unsigned int,bool>(2,1);
	rocCanvasMap_["3R"][4]  = make_pair<unsigned int,bool>(1,1);
	rocCanvasMap_["3R"][5]  = make_pair<unsigned int,bool>(1,0);//6
	rocCanvasMap_["3R"][6]  = make_pair<unsigned int,bool>(2,0);//7
	rocCanvasMap_["3R"][7]  = make_pair<unsigned int,bool>(3,0);//8
	rocCanvasMap_["3R"][8]  = make_pair<unsigned int,bool>(4,0);//9
	rocCanvasMap_["3R"][9]  = make_pair<unsigned int,bool>(5,0);//10
	rocCanvasMap_["3R"][10] = make_pair<unsigned int,bool>(4,1);
	rocCanvasMap_["3R"][11] = make_pair<unsigned int,bool>(3,1);
	rocCanvasMap_["3R"][12] = make_pair<unsigned int,bool>(2,1);
	rocCanvasMap_["3R"][13] = make_pair<unsigned int,bool>(1,1);
	rocCanvasMap_["3R"][14] = make_pair<unsigned int,bool>(1,0);//5
	rocCanvasMap_["3R"][15] = make_pair<unsigned int,bool>(2,0);//6
	rocCanvasMap_["3R"][16] = make_pair<unsigned int,bool>(3,0);//7
	rocCanvasMap_["3R"][17] = make_pair<unsigned int,bool>(4,0);//8
	rocCanvasMap_["3R"][18] = make_pair<unsigned int,bool>(3,1);
	rocCanvasMap_["3R"][19] = make_pair<unsigned int,bool>(2,1);
	rocCanvasMap_["3R"][20] = make_pair<unsigned int,bool>(1,1);
	rocCanvasMap_["3R"][21] = make_pair<unsigned int,bool>(1,0);//4
	rocCanvasMap_["3R"][22] = make_pair<unsigned int,bool>(2,0);//5
	rocCanvasMap_["3R"][23] = make_pair<unsigned int,bool>(3,0);//6
	
	rocCanvasMap_["3L"][0]  = make_pair<unsigned int,bool>(1,0);//4
	rocCanvasMap_["3L"][1]  = make_pair<unsigned int,bool>(2,0);//5
	rocCanvasMap_["3L"][2]  = make_pair<unsigned int,bool>(3,0);//6
	rocCanvasMap_["3L"][3]  = make_pair<unsigned int,bool>(3,1);
	rocCanvasMap_["3L"][4]  = make_pair<unsigned int,bool>(2,1);
	rocCanvasMap_["3L"][5]  = make_pair<unsigned int,bool>(1,1);
	rocCanvasMap_["3L"][6]  = make_pair<unsigned int,bool>(1,0);//5
	rocCanvasMap_["3L"][7]  = make_pair<unsigned int,bool>(2,0);//6
	rocCanvasMap_["3L"][8]  = make_pair<unsigned int,bool>(3,0);//7
	rocCanvasMap_["3L"][9]  = make_pair<unsigned int,bool>(4,0);//8
	rocCanvasMap_["3L"][10] = make_pair<unsigned int,bool>(4,1);
	rocCanvasMap_["3L"][11] = make_pair<unsigned int,bool>(3,1);
	rocCanvasMap_["3L"][12] = make_pair<unsigned int,bool>(2,1);
	rocCanvasMap_["3L"][13] = make_pair<unsigned int,bool>(1,1);
	rocCanvasMap_["3L"][14] = make_pair<unsigned int,bool>(1,0);//6
	rocCanvasMap_["3L"][15] = make_pair<unsigned int,bool>(2,0);//7
	rocCanvasMap_["3L"][16] = make_pair<unsigned int,bool>(3,0);//8
	rocCanvasMap_["3L"][17] = make_pair<unsigned int,bool>(4,0);//9
	rocCanvasMap_["3L"][18] = make_pair<unsigned int,bool>(5,0);//10
	rocCanvasMap_["3L"][19] = make_pair<unsigned int,bool>(5,1);
	rocCanvasMap_["3L"][20] = make_pair<unsigned int,bool>(4,1);
	rocCanvasMap_["3L"][21] = make_pair<unsigned int,bool>(3,1);
	rocCanvasMap_["3L"][22] = make_pair<unsigned int,bool>(2,1);
	rocCanvasMap_["3L"][23] = make_pair<unsigned int,bool>(1,1);

	rocCanvasMap_["3C"][0]  = make_pair<unsigned int,bool>(5,0);
	rocCanvasMap_["3C"][1]  = make_pair<unsigned int,bool>(4,0);
	rocCanvasMap_["3C"][2]  = make_pair<unsigned int,bool>(3,0);
	rocCanvasMap_["3C"][3]  = make_pair<unsigned int,bool>(2,1);
	rocCanvasMap_["3C"][4]  = make_pair<unsigned int,bool>(1,1);
	rocCanvasMap_["3C"][5]  = make_pair<unsigned int,bool>(1,1);//6
	rocCanvasMap_["3C"][6]  = make_pair<unsigned int,bool>(2,0);//7
	rocCanvasMap_["3C"][7]  = make_pair<unsigned int,bool>(3,0);//8
	rocCanvasMap_["3C"][8]  = make_pair<unsigned int,bool>(4,0);//9
	rocCanvasMap_["3C"][9]  = make_pair<unsigned int,bool>(5,0);//10

	rocCanvasMap_["H"][0]  = make_pair<unsigned int,bool>(1,0);
	rocCanvasMap_["H"][1]  = make_pair<unsigned int,bool>(2,0);
	rocCanvasMap_["H"][2]  = make_pair<unsigned int,bool>(3,0);
	rocCanvasMap_["H"][3]  = make_pair<unsigned int,bool>(4,0);
	rocCanvasMap_["H"][4]  = make_pair<unsigned int,bool>(5,0);
	rocCanvasMap_["H"][5]  = make_pair<unsigned int,bool>(6,0);
	rocCanvasMap_["H"][6]  = make_pair<unsigned int,bool>(7,0);
	rocCanvasMap_["H"][7]  = make_pair<unsigned int,bool>(8,0);
  
	rocCanvasMap_["F"][0]  = make_pair<unsigned int,bool>(1,0);
	rocCanvasMap_["F"][1]  = make_pair<unsigned int,bool>(2,0);
	rocCanvasMap_["F"][2]  = make_pair<unsigned int,bool>(3,0);
	rocCanvasMap_["F"][3]  = make_pair<unsigned int,bool>(4,0);
	rocCanvasMap_["F"][4]  = make_pair<unsigned int,bool>(5,0);
	rocCanvasMap_["F"][5]  = make_pair<unsigned int,bool>(6,0);
	rocCanvasMap_["F"][6]  = make_pair<unsigned int,bool>(7,0);
	rocCanvasMap_["F"][7]  = make_pair<unsigned int,bool>(8,0);
	rocCanvasMap_["F"][8]  = make_pair<unsigned int,bool>(1,1);//9
	rocCanvasMap_["F"][9]  = make_pair<unsigned int,bool>(2,1);//10
	rocCanvasMap_["F"][10]  = make_pair<unsigned int,bool>(3,1);//11
	rocCanvasMap_["F"][11]  = make_pair<unsigned int,bool>(4,1);//12
	rocCanvasMap_["F"][12]  = make_pair<unsigned int,bool>(5,1);//13
	rocCanvasMap_["F"][13]  = make_pair<unsigned int,bool>(6,1);//14
	rocCanvasMap_["F"][14]  = make_pair<unsigned int,bool>(7,1);//15
	rocCanvasMap_["F"][15]  = make_pair<unsigned int,bool>(8,1);//16

	rocCanvasMap_["FA"][0]  = make_pair<unsigned int,bool>(1,0);
	rocCanvasMap_["FA"][1]  = make_pair<unsigned int,bool>(2,0);
	rocCanvasMap_["FA"][2]  = make_pair<unsigned int,bool>(3,0);
	rocCanvasMap_["FA"][3]  = make_pair<unsigned int,bool>(4,0);
	rocCanvasMap_["FA"][4]  = make_pair<unsigned int,bool>(5,0);
	rocCanvasMap_["FA"][5]  = make_pair<unsigned int,bool>(6,0);
	rocCanvasMap_["FA"][6]  = make_pair<unsigned int,bool>(7,0);
	rocCanvasMap_["FA"][7]  = make_pair<unsigned int,bool>(8,0);

	rocCanvasMap_["FB"][0]  = make_pair<unsigned int,bool>(1,0);//1,1
	rocCanvasMap_["FB"][1]  = make_pair<unsigned int,bool>(2,0);//2,1
	rocCanvasMap_["FB"][2]  = make_pair<unsigned int,bool>(3,0);//3,1
	rocCanvasMap_["FB"][3]  = make_pair<unsigned int,bool>(4,0);//4,1
	rocCanvasMap_["FB"][4]  = make_pair<unsigned int,bool>(5,0);//5,1
	rocCanvasMap_["FB"][5]  = make_pair<unsigned int,bool>(6,0);//6,1
	rocCanvasMap_["FB"][6]  = make_pair<unsigned int,bool>(7,0);//7,1
	rocCanvasMap_["FB"][7]  = make_pair<unsigned int,bool>(8,0);//8,1
}
