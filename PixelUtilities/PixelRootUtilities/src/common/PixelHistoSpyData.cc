#include "PixelUtilities/PixelRootUtilities/include/PixelHistoSpyData.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelHistoTBranch.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include <iostream>
#include <string>
#include <sstream>
#include <TH2.h>
#include <TFrame.h>
#include <TRandom.h>
#include <TTree.h>
#include <TThread.h>
#include <TROOT.h>
#include <TDirectory.h>

using namespace std;
using namespace pos;

PixelHistoSpyData::PixelHistoSpyData(vector<int> fedList,vector<PixelROCName> rocList,PixelNameTranslation &nameTranslation) : PixelHistoThreadFrameWithArgs(fedList.size()){
  /*
  // Create a new canvas
  fCanvas = new TCanvas("SpyServ","SpyServ",200,10,700,500);
  fCanvas->SetFillColor(42);
  fCanvas->GetFrame()->SetFillColor(21);
  fCanvas->GetFrame()->SetBorderSize(6);
  fCanvas->GetFrame()->SetBorderMode(-1);
  */
  // Create a 1-D, 2-D and a profile histogram
  string mthn = "[PixelHistoSpyData::PixelHistoSpyData()]\t";
	thePixelRootDirectoryMaker_ = new PixelRootDirectoryMaker(rocList);
	gROOT->cd();
  TDirectory *summaryDir=gDirectory->mkdir("SummaryTrees","SummaryTrees");
	summaryDir->cd();
	theTree_ = new TTree("OccupancyTree","OccupancyTree");
	BranchStruct branchStruct;
  theBranch_ = (PixelHistoTBranch*)theTree_->Branch("Occupancy",&branchStruct,"hasHits/F:occupancy/F:ROCName/C");
//  theBranch_ = (PixelHistoTBranch*)theTree_->Branch("Occupancy",&branchStruct,"occupancy/F:ROCName/C");
  unsigned int entry = 0;
	for (vector<pos::PixelROCName>::iterator it=rocList.begin();it!=rocList.end();it++){
    string rocName =  it->rocname();
		strcpy(branchStruct.rocName,rocName.c_str());
		branchStruct.hasHits = 0;
		branchStruct.occupancy = 0;
		theBranch_->Fill();
		rocNameEntryMap_[rocName] = entry;
    int fed     = nameTranslation.getHdwAddress(*it)->fednumber();
    int channel = nameTranslation.getHdwAddress(*it)->fedchannel();
    int roc     = nameTranslation.getHdwAddress(*it)->fedrocnumber();
		dataEntryMap_[fed][channel][roc] = entry;
		++entry;
		thePixelRootDirectoryMaker_->cdDirectory(rocName);
		TH2F *tmpHisto = new TH2F((rocName+"_Occupancy").c_str(),(rocName+"_Occupancy").c_str(),52, -0.5, 51.5,80,-0.5,79.5);
    tmpHisto->GetXaxis()->SetTitle("col");
    tmpHisto->GetYaxis()->SetTitle("row");
//    tmpHisto->SetStats(false);
		histoMap_  [fed][channel][roc] = tmpHisto;
	}  
	fedList_ = fedList;
}

void PixelHistoSpyData::init(string filesDir,int runNumber){
	string mthn = "[PixelHistoSpyData::init()]\t";
	stringstream dirName;
	dirName.str(filesDir.c_str());
	if(filesDir==""){
		if(getenv("POS_OUTPUT_DIR")){
			dirName << getenv("POS_OUTPUT_DIR");
		}
	}
  
	stringstream fileName;
	for(unsigned int fed=0; fed<fedList_.size(); ++fed){
	  fileName.str("");
		fileName << dirName.str() << "/Run_" << runNumber << "/PhysicsDataFED_" << fedList_[fed] << "_" << runNumber << ".dmp";
		ifstream *fileP = new ifstream(fileName.str().c_str(),ios::binary|ios::in);
		fileList_.push_back(fileP);
		if(!fileP->is_open()){
			cout << mthn << "Can't open " << fileName.str() << endl;
		}
		else{
			Word64 runNumber;
			getNextWord64(fileP,runNumber);
			cout << mthn << "Run number:" << runNumber.getWord() << endl;
		}
//    PixelSLinkEvent tmp;
//    sLinkEvents_.push_back(tmp);
	}
}

void PixelHistoSpyData::destroy(){
	for(unsigned int file=0; file<fileList_.size(); ++file){
		fileList_[file]->close();
		delete fileList_[file];
	};
	std::map<unsigned int,std::map<unsigned int,std::map<unsigned int,TH2F*> > >::iterator fedIt;
	std::map<unsigned int,std::map<unsigned int,TH2F*> >::iterator                         channelIt;
	std::map<unsigned int,TH2F*>::iterator                                                 rocIt;
  for(fedIt=histoMap_.begin(); fedIt != histoMap_.end(); ++fedIt){
		channelIt = fedIt->second.begin();
    for(; channelIt != fedIt->second.end(); ++channelIt){
		  rocIt = channelIt->second.begin();
      for(; rocIt != channelIt->second.end(); ++rocIt){
        if (rocIt->second) delete rocIt->second;
			}
		}
  }
	histoMap_.clear();
}

PixelHistoSpyData::~PixelHistoSpyData(){
  //   delete fCanvas;
  //////////Histos will be deleted when the directory is closed
  PixelHistoThreadFrameWithArgs::stopThreads();
	destroy();
}

void PixelHistoSpyData::userFunc0(int &threadNumber){
  const string mthn = "[PixelHistoSpyData::userFunc0()]";
	PixelHistoThreadFrameWithArgs::funcRunning_[0] = true;
	PixelSLinkEvent sLinkEvent;
	if(getNextEvent(fileList_[threadNumber],sLinkEvent) == 0){
		sLinkEvent.decodeEvent();
		vector<PixelHit> hits           = sLinkEvent.getHits();
		vector<PixelHit>::iterator ihit = hits.begin();
		//loop over the hits in the event
		int fed = sLinkEvent.getHeader().getSource_id();
		for (;ihit!=hits.end();++ihit) {
		  //get channel, roc, row, col
		  unsigned int channel = ihit->getLink_id();
		  unsigned int roc     = ihit->getROC_id()-1;
			unsigned int row     = ihit->getRow();
		  unsigned int col     = ihit->getColumn();
  	  TThread::Lock();
			theBranch_->SetEntry(dataEntryMap_[fed][channel][roc],0,1);
			theBranch_->AddToEntry(dataEntryMap_[fed][channel][roc],1,1);
//			theBranch_->SetEntry(dataEntryMap_[fed][channel][roc],1);
			histoMap_[fed][channel][roc]->Fill(col,row,1);
			cout << mthn << "Tread: " << threadNumber << "\t"
					 << "Fed:"       << fed
			     << " Channel: " << channel
			     << " roc: "  	 << roc
					 << " row: "  	 << row
					 << " col: "  	 << col
					 << endl; 
  	  TThread::UnLock();
		} //cout << "end of hits loop" << endl;
	}
	else{
  	TThread::Lock();
		cout << mthn << "Tread: " << threadNumber << "\t" << "We need to wait!" << endl;
  	TThread::UnLock();
		usleep(1000000);
	}
//  cout << "[PixelHistoSpyData::userFunc0()]\tThread: " << threadNumber << " fed: " << fedList_[threadNumber] << endl; 
	
	//fill();
  PixelHistoThreadFrameWithArgs::funcRunning_[0] = false;
}

void PixelHistoSpyData::fill(){
  string mthn = "[PixelHistoSpyData::fill()]\t";
}

int PixelHistoSpyData::getNextEvent(std::ifstream* file, PixelSLinkEvent &sLinkEvent){
	const string mthn = "[PixelHistoSpyData::getNextEvent()]\t";
	bool goodRead = false;
  sLinkEvent.clearBuffer();
  Word64 word;
  //
  // First search for the next SLink Header in the next maxHeaderTries 64 bit words
  //
  unsigned int tries=1, maxHeaderTries=100;
  int fileStatus = -1;
	while (!goodRead && tries<maxHeaderTries) {
    if ((fileStatus = getNextWord64(file,word))>=0) {
      if(fileStatus == 0 && PixelSLinkHeader(word).isHeader()) {      
	      goodRead=true;
	      sLinkEvent.push_back(word);
      } 
			else if(fileStatus == 1){
				return fileStatus;
			}
			else {
	      goodRead=false;
  	    TThread::Lock();
	      cout << mthn << "Junk word 0x" <<std::hex<<word.getWord() << dec<<" caught looking for SLink Header!" << endl;
  	    TThread::UnLock();
      }
    } 
		else {
  	  TThread::Lock();
      cout << mthn << "End of event stream reached." << endl;
  	  TThread::UnLock();
      goodRead=false;
      return goodRead;
    }
    ++tries;
  }
  if (goodRead==false) {
  	TThread::Lock();
    cout << mthn << "New SLink Header not found in the next " << maxHeaderTries << " 64 bit words! Aborting parsing of stream" << endl;
  	TThread::UnLock();
//    sLinkEvent=PixelSLinkEvent(eventContent);
//    sLinkEvent.decodeEvent();
    return goodRead;
  }

  //
  // Now keep taking in hits till an SLink Trailer is encountered
  //
  do {
    if (getNextWord64(file,word)>=0) {
      goodRead=true;
      sLinkEvent.push_back(word);
    } else {
      goodRead=false;
  	  TThread::Lock();
      cout << mthn << "Event stream ended while taking in hits before an SLinkTrailer was encountered!" << endl;
  	  TThread::UnLock();
    }

   
  } while (goodRead && !PixelSLinkTrailer(word).isTrailer());
	return 0;
}

int PixelHistoSpyData::getNextWord64(std::ifstream* file, pos::Word64 &word){
  streampos lastPointer = file->tellg();
	file->seekg(0,ios::end);
	streampos eof=file->tellg();
	if(lastPointer < eof){
	  file->seekg(lastPointer);
		file->read((char*)&word, 8);
		if(!file->eof()){
			lastPointer = file->tellg();
		}
		else{
			file->clear();
			file->seekg(lastPointer);
			return 1;
		}
  }
	else{
		return 1;
	}
	if (file->good()) {
    return 0;
  } 
	else {
  	TThread::Lock();
    cout << "[PixelHistoSpyData::getNextWord64()]\tStream ended trying reading next 64 bit word." << endl;
  	TThread::UnLock();
		exit(0);
	  return -1;
  }
}
