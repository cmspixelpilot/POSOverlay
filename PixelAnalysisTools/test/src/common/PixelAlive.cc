//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
// authors G. Cerati, M. Dinardo, K. Ecklund, B. HeyBurn, A. Kumar, E. Luiggi, L. Uplegger  //
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <math.h>
#include "PixelAnalysisTools/include/PixelConfigurationsManager.h"
#include "PixelAnalysisTools/include/PixelCalibConfigurationExtended.h"
//#include "PixelAnalysisTools/include/PixelXmlReader.h"
#include "PixelUtilities/PixelXmlUtilities/include/PixelXmlReader.h"
#include "PixelUtilities/PixelFEDDataTools/include/SLinkDecoder.h"
#include "TH1F.h"
#include "TFile.h"
#include "TH2F.h"
#include "TString.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TStyle.h"

using namespace std;
using namespace pos;

int main(){
  string mthn = "[main()]\t" ;
  /*   gROOT->SetBatch() ; */
  /*   gROOT->SetStyle("Plain") ; */
  /*   TStyle *t  = gROOT->GetStyle("Plain") ; */
  /*   t->SetStatStyle(0) ; */
  /*   gROOT->SetStyle(t) ; */

  //------------------------------------------------------------------------------------------------
  // Get paramaters from the xml file " filenames, feds, rocs, nevents, etc, ...EELL
  PixelXmlReader thePixelXmlReader;
  thePixelXmlReader.readDocument("PixelAnalysis.xml");
  
  string dataFile        = thePixelXmlReader.getXMLAttribute("DataFileName","FileName");
  string calibFile       = thePixelXmlReader.getXMLAttribute("CalibFileName","FileName");
  string translationFile = thePixelXmlReader.getXMLAttribute("TranslationFileName","FileName");
  string detconfigFile   = thePixelXmlReader.getXMLAttribute("DetConfigFileName","FileName");
  
  string numOfEvents = thePixelXmlReader.getXMLAttribute("NumOfEvents","Events");
  unsigned int maxevent = atoi(numOfEvents.c_str()); 
  
  string FedMin = thePixelXmlReader.getXMLAttribute("FedMin","Fed");
  string FedMax = thePixelXmlReader.getXMLAttribute("FedMax","Fed");
  unsigned int fedmin = atoi(FedMin.c_str()); 
  unsigned int fedmax = atoi(FedMax.c_str());
  unsigned int nOfFed = (fedmax - fedmin) + 1;
      
  unsigned int fedchanmin[nOfFed];
  unsigned int fedchanmax[nOfFed];
  for (unsigned int numOfChannels = 0; numOfChannels != nOfFed; ++numOfChannels){
     string channelMin = thePixelXmlReader.getXMLAttribute("ChannelMin","Channel");
     string channelMax = thePixelXmlReader.getXMLAttribute("ChannelMax","Channel");
     fedchanmin[numOfChannels] = atoi(channelMin.c_str());
     fedchanmax[numOfChannels] = atoi(channelMax.c_str());
  } 
     
  string rocMin = thePixelXmlReader.getXMLAttribute("RocMin","Roc");
  string rocMax = thePixelXmlReader.getXMLAttribute("RocMax","Roc");  
  unsigned int rocmin = atoi(rocMin.c_str());
  unsigned int rocmax = atoi(rocMax.c_str());
/*
*/    
//   PixelConfigurationsManager thePixelConfigurationsManager(&thePixelXmlReader);
//------------------------------------------------------------------------------------------------------
  //open the data file (from xml file)
  std::ifstream in;
  in.open(dataFile.c_str(),ios::binary|ios::in);

  
  //Added by Ashish to get report summary
  stringstream errFileName;
  if (fedmin==fedmax){
    errFileName << "FED" << fedmin << "_Channels" << fedchanmin[0] << "-" << fedchanmax[0] << "_PixelAliveReport.txt";
      }
  else{
    errFileName << "FED" << fedmin << "-" << fedmax << "_Channels" << fedchanmin[0] << "-" << fedchanmax[0] << "_PixelAliveReport.txt";
      }
  //Open error file
  fstream out(errFileName.str().c_str(),fstream::out);
  
  
  /*from new class... EELL 
    The PixelCalibConfigurationExtended class adds the few functions missing in PixelCalibConfiguration in order
    to run PixelAlive and SCurve.
  */
  PixelNameTranslation trans(translationFile.c_str());
  PixelDetectorConfig detconfig(detconfigFile.c_str());
  PixelCalibConfigurationExtended calib(calibFile.c_str(), &trans, &detconfig);

  assert(in.good());

  int ntrig=calib.nTriggersPerPattern();
  int nvcal=calib.nScanPoints("Vcal");

//  pixel::SLinkData pixeldata;

  //compute the number of feds, channel and rocs to be analyzed
  //unsigned int nOfFed  = fedmax-fedmin+1; //already done...
  unsigned int nOfChan[nOfFed];
  unsigned int maxNOfChan = 0;
  for (unsigned int pp=0;pp<nOfFed;++pp){
    nOfChan[pp] = fedchanmax[pp]-fedchanmin[pp]+1;
    if (maxNOfChan<fedchanmax[pp]-fedchanmin[pp]+1) maxNOfChan=fedchanmax[pp]-fedchanmin[pp]+1;
  }
  unsigned int nOfROCS = rocmax - rocmin +1;

  //create the matrices of hitograms
  TH2F* alive[nOfFed][maxNOfChan][nOfROCS];
  TH2F* alive_mirror[nOfFed][maxNOfChan][nOfROCS];
  TH1F* hPercentageOfDeadChannelsInPanel[nOfFed][maxNOfChan];

  //book all the histograms
  for(unsigned int fedid=0; fedid<nOfFed; fedid++){    
    for (unsigned int channel=0;channel<nOfChan[fedid];channel++){
      for (unsigned int roc=0;roc<nOfROCS;roc++){
	TString name="FED=";
	name+=(fedid+fedmin);
	name=name+" Ch=";
	name+=(channel+fedchanmin[fedid]);
	name=name+" ROC=";
	name+=(roc+rocmin);
	alive[fedid][channel][roc]= new TH2F(name,name,52, -0.5, 51.5,80,-0.5,79.5);
	alive[fedid][channel][roc]->SetMinimum(0.0);
	alive[fedid][channel][roc]->SetMaximum(100.);
	alive[fedid][channel][roc]->SetStats(0);
	TString name_mirror="FED=";
	name_mirror+=(fedid+fedmin);
	name_mirror=name_mirror+" Ch=";
	name_mirror+=(channel+fedchanmin[fedid]);
	name_mirror=name_mirror+" ROC=";
	name_mirror+=(roc+rocmin);
	name_mirror = name_mirror+" (inv)" ;
	alive_mirror[fedid][channel][roc]= new TH2F(name_mirror,name_mirror,52, -0.5, 51.5,80,-0.5,79.5);
	alive_mirror[fedid][channel][roc]->SetMinimum(0.0);
	alive_mirror[fedid][channel][roc]->SetMaximum(100.);
	alive_mirror[fedid][channel][roc]->SetStats(0);
      }
    }
  }
  TH1F* hit_hist=new TH1F("Hits","Hits",100, 0, 1000.0);

  std::vector<unsigned int> const *rows=0;
  std::vector<unsigned int> const *cols=0;

  unsigned int nEvent=1;

  //set the maximum naumber of events to be processed // got it from the xml file... EELL
  //int maxevent=100000;
  //maxevent=20;

  //event loop: read data
 
  SLinkDecoder sLinkDecoder(&in);
  Word64 runNumber;
  sLinkDecoder.getNextWord64(runNumber);
  cout << mthn << " RunNumber " << runNumber.getWord() << endl;

  PixelSLinkEvent sLinkEvent;
  
  Word64 word;
  bool newPattern = true;
  unsigned int eventsInAPattern  = ntrig*calib.getNumberOfFeds();
  unsigned int firstCounterValue = 0;
  unsigned int maxTriggerCounterValue = (unsigned int)pow(2.,24.);
  
  /*
  //####################################################################
  //Strategy of EventSkip added for analyzing HC+Z1 data where FEDs are not identified.
  //Comment this Block when you have FEDs identified
  //PixelSLinkEvent sLinkEventSkip;
  
  //For running over FED 33, Comment otherwise
  //while(sLinkDecoder.getNextEvent(sLinkEvent) && nEvent<maxevent){
  //sLinkDecoder.getNextEvent(sLinkEventSkip);
  //For running over FED 34, Comment otherwise
  //while(sLinkDecoder.getNextEvent(sLinkEventSkip) && nEvent<maxevent){
  //sLinkDecoder.getNextEvent(sLinkEvent);
  //####################################################################
  */
  
  while(sLinkDecoder.getNextEvent(sLinkEvent) && nEvent<maxevent){
    if (nEvent%10000==0) {
      cout << mthn << "nEvent:"<<nEvent<<endl;
    }
    //This must be used with the new version of POS. Before it was embedded in sLinkDecoder.getNextEvent()
    sLinkEvent.decodeEvent();
    if(nEvent == 1){
      firstCounterValue = sLinkEvent.getHeader().getLV1_id() - 1;
      if(firstCounterValue != 0){
        cout << mthn << "The first event has the counter that doesn't start from 1 but it starts at " << sLinkEvent.getHeader().getLV1_id() << endl;
      }
    }
    if ((nEvent-1)%(eventsInAPattern)==0) {
      newPattern =true;
    }
    if(nEvent%maxTriggerCounterValue != (sLinkEvent.getHeader().getLV1_id()-firstCounterValue)%maxTriggerCounterValue){
      cout << mthn << "An event was missing...resyncing"
           << " Event: "  << nEvent
           << " != LV1_id: " << sLinkEvent.getHeader().getLV1_id()-firstCounterValue
	   << endl;
      nEvent = sLinkEvent.getHeader().getLV1_id()-firstCounterValue;
      if ((nEvent-1)%(eventsInAPattern)==0) {
        newPattern =true;
      }
    }
    if (newPattern) {
      newPattern =false;
      //new pattern: get the rows and columns of this pattern
      calib.getRowsAndCols((nEvent-1)/(eventsInAPattern),rows,cols);
      assert(rows!=0);
      assert(cols!=0);

      if (0) {
	cout << "Event:"<<nEvent<<" Rows:";
	for( unsigned int i=0;i<rows->size();i++){
	  cout << (*rows)[i]<<" ";
	}
	cout <<" Cols:";
	for( unsigned int i=0;i<cols->size();i++){
	  cout << (*cols)[i]<<" ";
	}
	cout <<endl;
      }
    }

    //get fedid and skip if not in [fedmin,fedmax]
    int fed = sLinkEvent.getHeader().getSource_id();
//    if ( fed<((int)fedmin) || fed>((int)fedmax) ) {
//       nEvent++;
       //cout << "Skipping event: Out of range \n";
//       continue;
//    }
    int fedn = fed-fedmin;
    
    vector<PixelHit> hits = sLinkEvent.getHits();
    vector<PixelHit>::iterator ihit = hits.begin();
    hit_hist->Fill(hits.size());

    //loop over the hits in the event
    for (;ihit!=hits.end();++ihit) {
      
      //get channel, roc, row, col
      unsigned int linkid=ihit->getLink_id();
      unsigned int rocid=ihit->getROC_id();
      unsigned int row=ihit->getRow();
      unsigned int col=ihit->getColumn();
//       if(!thePixelConfigurationsManager.isDataToAnalyze(fed,linkid,rocid)){continue;}
//       if(!thePixelConfigurationsManager.isAnAllowedPixel(row,col)){
//         //ERROR
// 	exit(0);
//       }

      bool valid_row=false;
      bool valid_col=false;
      
      for (unsigned int i=0;i<rows->size();i++){
	if (row==(*rows)[i]) valid_row=true;
      }
      
      for (unsigned int i=0;i<cols->size();i++){
	if (col==(*cols)[i]) valid_col=true;
      }
      
      //fill histograms
      if (valid_row&&valid_col){
	assert(linkid>=1&&linkid<37);
	assert(rocid>=1&&rocid<25);
	//cout << "will fill F="<<fedn << " C=" << linkid-fedchanmin[fedn]<<" R="<<rocid-rocmin<<" c="<<col
	//<<" r="<<row<<" entries="<<alive[fedn][linkid-fedchanmin[fedn]][rocid-rocmin]->GetEntries() << endl;
	alive[fedn][linkid-fedchanmin[fedn]][rocid-rocmin]->Fill(col,row,100./ntrig);
	alive_mirror[fedn][linkid-fedchanmin[fedn]][rocid-rocmin]->Fill(51-col,79-row,100./ntrig);
      }
      else{
	cout << "Not valid row or column:"<<linkid<<" "<<rocid<<" "
	     << row<<" "<<col<<endl;
      }
    }
    nEvent++;
  }
  cout << mthn << "Processed:"<< nEvent -1 <<" triggers"<<endl;

  //create output file
  cout << mthn << "Making FULL plots" << endl;
  char outFileName[20];
  if (fedmin==fedmax) sprintf(outFileName,"FED%d_Ch%d-%d_PixAlive.root",fedmin,fedchanmin[0],fedchanmax[0]) ;
  else sprintf(outFileName,"FED%d-%d_PixAlive.root",fedmin,fedmax) ;
  TFile * f = new TFile(outFileName,"RECREATE") ;
  TDirectory * summary = f->mkdir("Summaries");
  summary->cd();
  //book all the histograms
  TH1F * hPercentageOfDeadChannelsPerRoc   = new TH1F("hPercentageOfDeadChannelsPerRoc"  ,"Percentage of dead channels per ROC"  ,1000, 0., 5.);
  TH1F * hPercentageOfDeadChannelsPerPanel = new TH1F("hPercentageOfDeadChannelsPerPanel","Percentage of dead channels per Panel",1000, 0., 5.);
  for(unsigned int fedid=0; fedid<nOfFed; fedid++){    
    for (unsigned int channel=0;channel<nOfChan[fedid];channel++){
  	TString name="FED=";
	name+=(fedid+fedmin);
	name=name+" Ch=";
	name+=(channel+fedchanmin[fedid]);
	hPercentageOfDeadChannelsInPanel[fedid][channel]= new TH1F(name,"Percentage of dead channels per ROC in a panel",1000, 0., 5.);
   }
  }
  unsigned int numberOfBadChannelsPerRoc   = 0;
  unsigned int numberOfBadChannelsPerPanel = 0;
  unsigned int numberOfRocs                = 0;
  unsigned int numberOfTotalBadChannels    = 0;
  unsigned int numberOfTotalRocs           = 0;
  
  for(unsigned int fedid=0; fedid<nOfFed; fedid++){    
    for (unsigned int channel=0;channel<nOfChan[fedid];channel++){
      numberOfBadChannelsPerPanel = 0;
      numberOfRocs                = 0;
      for (unsigned int roc=0;roc<nOfROCS;roc++){
	numberOfBadChannelsPerRoc = 0;
	TH2F * tmpHisto = alive[fedid][channel][roc];
	if(tmpHisto->GetEntries() != 0){
  	  for(int binX=1; binX <= tmpHisto->GetNbinsX(); ++binX){
	    for(int binY=1; binY <= tmpHisto->GetNbinsY(); ++binY){
	      if(tmpHisto->GetBinContent(binX,binY) < 100){
	        ++numberOfBadChannelsPerRoc;
	      } 
	    }
	  }
	  ++numberOfRocs;
	  numberOfBadChannelsPerPanel += numberOfBadChannelsPerRoc;
	  hPercentageOfDeadChannelsPerRoc->Fill((100.*numberOfBadChannelsPerRoc)/(80*52));
	  hPercentageOfDeadChannelsInPanel[fedid][channel]->Fill((100.*numberOfBadChannelsPerRoc)/(80*52));
	  //out <<"No of bad pixels in a ROC = " << numberOfBadChannelsPerRoc << endl;
	  if(((100.*numberOfBadChannelsPerRoc)/(80*52)) > 0.5) {
	  out << "ROCs with > 0.5% Bad Pixels are: " << endl;
	  out << (100.*numberOfBadChannelsPerRoc)/(80*52) << "%" << "   fed=" << fedid+fedmin
	  << " channel= " << channel+fedchanmin[fedid]  << " rocid= " << roc+rocmin << endl;
	  out << "   " << endl; 
	  }    
        }
	}	
      hPercentageOfDeadChannelsPerPanel->Fill((100.*numberOfBadChannelsPerPanel)/(numberOfRocs*80*52));
      if(((100.*numberOfBadChannelsPerPanel)/(numberOfRocs*80*52)) > 0.1) {
      out << "Panels with > 0.1% Bad Pixels are: " << endl;
      out << (100.*numberOfBadChannelsPerPanel)/(numberOfRocs*80*52) << "%" << "    fed= " <<
      fedid+fedmin << " channel= " << channel+fedchanmin[fedid]  << endl;
      out << "   " << endl; 
	  }
      //out <<"No of bad pixels in panel= " << numberOfBadChannelsPerPanel << endl;
      numberOfTotalBadChannels += numberOfBadChannelsPerPanel;
      numberOfTotalRocs += numberOfRocs;	
    }
    out <<"Final Report about the bad pixels : " << endl;
    out <<"Total No of bad pixels = " << numberOfTotalBadChannels << endl;
    out <<"Total no of ROCs = " << numberOfTotalRocs << endl;
    out <<"Global % of Bad Pixels= " << (100.*numberOfTotalBadChannels)/(numberOfTotalRocs*80*52) << endl;
  }


  char title[20] ;
  int disk = 0;
  int blade = 0;
  int panel = 0;
  unsigned int myfed = 0;
  unsigned int mycha = 0;
  TString panelType = "";
  TString myPanelType = "";


  vector<pair<vector<unsigned int>, string> > panelTypes = calib.getFedChannelHWandTypeInfo();
  //for every panel (=every channel) create a directory and fill it with canvases
  //(one canvas per plaquette)
  for (unsigned int it=0;it<panelTypes.size();it++) {// the disk, blade and panel are in pixelModuleName()
    disk  = panelTypes[it].first[0];
    blade = panelTypes[it].first[1];
    panel = panelTypes[it].first[2];
    myfed = panelTypes[it].first[3];
    mycha = panelTypes[it].first[4];
    myPanelType = panelTypes[it].second;

    //    myfed+=32;
  
    //cout << "fedmin="  << fedmin << " fedmax=" << fedmax << endl;
    if (myfed<fedmin||myfed>fedmax) continue;
    unsigned int fedid=myfed-fedmin;
    //cout<<"fedchanmin[fedid]="<<fedchanmin[fedid]<<" fedchanmax[fedid]="<<fedchanmax[fedid]<<endl;
    if (mycha<fedchanmin[fedid]||mycha>fedchanmax[fedid]) continue;
    unsigned int channelId=mycha-fedchanmin[fedid];
    cout << "FED="<< fedmin+fedid <<" Channel " << channelId+fedchanmin[fedid]<<" panelType="<<myPanelType<<endl;

    if(myPanelType=="4L") {
      sprintf(title,"FED%d_Channel%d",fedmin+fedid,channelId+fedchanmin[fedid]) ;
      TDirectory * dir = f->mkdir(title);
      dir->cd() ;
      TCanvas * c4L1x2 = new TCanvas("c4L1x2", "4L - 1x2", 500,300) ;
      c4L1x2->Divide(2,1) ;
      c4L1x2->cd(1) ; 
      alive_mirror[fedid][channelId][1]->Draw("colz");
      c4L1x2->cd(2) ; 
      alive_mirror[fedid][channelId][0]->Draw("colz");
      c4L1x2->Write() ;
	
      TCanvas * c4L2x3 = new TCanvas("c4L2x3", "4L - 2x3", 750,600) ;
      c4L2x3->Divide(3,2) ;
      c4L2x3->cd(1) ; 
      alive_mirror[fedid][channelId][7]->Draw("colz");
      c4L2x3->cd(2) ; 
      alive_mirror[fedid][channelId][6]->Draw("colz");
      c4L2x3->cd(3) ; 
      alive_mirror[fedid][channelId][5]->Draw("colz");
      c4L2x3->cd(4) ; 
      alive[fedid][channelId][2]->Draw("colz");
      c4L2x3->cd(5) ; 
      alive[fedid][channelId][3]->Draw("colz");
      c4L2x3->cd(6) ; 
      alive[fedid][channelId][4]->Draw("colz");
      c4L2x3->Write() ;
	
      TCanvas * c4L2x4 = new TCanvas("c4L2x4", "4L - 2x4", 1000,600) ;
      c4L2x4->Divide(4,2) ;
      c4L2x4->cd(1) ; 
      alive_mirror[fedid][channelId][15]->Draw("colz");
      c4L2x4->cd(2) ; 
      alive_mirror[fedid][channelId][14]->Draw("colz");
      c4L2x4->cd(3) ; 
      alive_mirror[fedid][channelId][13]->Draw("colz");
      c4L2x4->cd(4) ; 
      alive_mirror[fedid][channelId][12]->Draw("colz");
      c4L2x4->cd(5) ; 
      alive[fedid][channelId][8]->Draw("colz");
      c4L2x4->cd(6) ; 
      alive[fedid][channelId][9]->Draw("colz");
      c4L2x4->cd(7) ; 
      alive[fedid][channelId][10]->Draw("colz");
      c4L2x4->cd(8) ; 
      alive[fedid][channelId][11]->Draw("colz");
      c4L2x4->Write() ;
	
      TCanvas * c4L1x5 = new TCanvas("c4L1x5", "4L - 1x5", 1250,300) ;
      c4L1x5->Divide(5,1) ;
      c4L1x5->cd(1) ; 
      alive[fedid][channelId][20]->Draw("colz");
      c4L1x5->cd(2) ; 
      alive[fedid][channelId][19]->Draw("colz");
      c4L1x5->cd(3) ; 
      alive[fedid][channelId][18]->Draw("colz");
      c4L1x5->cd(4) ; 
      alive[fedid][channelId][17]->Draw("colz");
      c4L1x5->cd(5) ; 
      alive[fedid][channelId][16]->Draw("colz");
      c4L1x5->Write() ;
    }
    else if(myPanelType=="3L") {
      sprintf(title,"FED%d_Channel%d",fedmin+fedid,channelId+fedchanmin[fedid]) ;
      TDirectory * dir = f->mkdir(title);
      dir->cd() ;
      TCanvas * c3L2x3 = new TCanvas("c3L2x3", "3L - 2x3", 750,600) ;
      c3L2x3->Divide(3,2) ;
      c3L2x3->cd(1) ; 
      alive_mirror[fedid][channelId][5]->Draw("colz");
      c3L2x3->cd(2) ; 
      alive_mirror[fedid][channelId][4]->Draw("colz");
      c3L2x3->cd(3) ; 
      alive_mirror[fedid][channelId][3]->Draw("colz");
      c3L2x3->cd(4) ; 
      alive[fedid][channelId][0]->Draw("colz");
      c3L2x3->cd(5) ; 
      alive[fedid][channelId][1]->Draw("colz");
      c3L2x3->cd(6) ; 
      alive[fedid][channelId][2]->Draw("colz");
      c3L2x3->Write() ;
	
      TCanvas * c3L2x4 = new TCanvas("c3L2x4", "3L - 2x4", 1000,600) ;
      c3L2x4->Divide(4,2) ;
      c3L2x4->cd(1) ; 
      alive_mirror[fedid][channelId][13]->Draw("colz");
      c3L2x4->cd(2) ; 
      alive_mirror[fedid][channelId][12]->Draw("colz");
      c3L2x4->cd(3) ; 
      alive_mirror[fedid][channelId][11]->Draw("colz");
      c3L2x4->cd(4) ; 
      alive_mirror[fedid][channelId][10]->Draw("colz");
      c3L2x4->cd(5) ; 
      alive[fedid][channelId][6]->Draw("colz");
      c3L2x4->cd(6) ; 
      alive[fedid][channelId][7]->Draw("colz");
      c3L2x4->cd(7) ; 
      alive[fedid][channelId][8]->Draw("colz");
      c3L2x4->cd(8) ; 
      alive[fedid][channelId][9]->Draw("colz");
      c3L2x4->Write() ;
	
      TCanvas * c3L2x5 = new TCanvas("c3L2x5", "3L - 2x5", 1250,600) ;
      c3L2x5->Divide(5,2) ;
      c3L2x5->cd(1) ; 
      alive_mirror[fedid][channelId][23]->Draw("colz");
      c3L2x5->cd(2) ; 
      alive_mirror[fedid][channelId][22]->Draw("colz");
      c3L2x5->cd(3) ; 
      alive_mirror[fedid][channelId][21]->Draw("colz");
      c3L2x5->cd(4) ; 
      alive_mirror[fedid][channelId][20]->Draw("colz");
      c3L2x5->cd(5) ; 
      alive_mirror[fedid][channelId][19]->Draw("colz");
      c3L2x5->cd(6) ; 
      alive[fedid][channelId][14]->Draw("colz");
      c3L2x5->cd(7) ; 
      alive[fedid][channelId][15]->Draw("colz");
      c3L2x5->cd(8) ; 
      alive[fedid][channelId][16]->Draw("colz");
      c3L2x5->cd(9) ; 
      alive[fedid][channelId][17]->Draw("colz");
      c3L2x5->cd(10) ; 
      alive[fedid][channelId][18]->Draw("colz");
      c3L2x5->Write() ;
    }
    if(myPanelType=="4R") {
      sprintf(title,"FED%d_Channel%d",fedmin+fedid,channelId+fedchanmin[fedid]) ;
      TDirectory * dir = f->mkdir(title);
      dir->cd() ;
      TCanvas * c4R1x2 = new TCanvas("c4R1x2", "4R - 1x2", 500,300) ;
      c4R1x2->Divide(2,1) ;
      c4R1x2->cd(1) ; 
      alive_mirror[fedid][channelId][20]->Draw("colz");
      c4R1x2->cd(2) ; 
      alive_mirror[fedid][channelId][19]->Draw("colz");
      c4R1x2->Write() ;
	
      TCanvas * c4R2x3 = new TCanvas("c4R2x3", "4R - 2x3", 750,600) ;
      c4R2x3->Divide(3,2) ;
      c4R2x3->cd(1) ; 
      alive_mirror[fedid][channelId][15]->Draw("colz");
      c4R2x3->cd(2) ; 
      alive_mirror[fedid][channelId][14]->Draw("colz");
      c4R2x3->cd(3) ; 
      alive_mirror[fedid][channelId][13]->Draw("colz");
      c4R2x3->cd(4) ; 
      alive[fedid][channelId][16]->Draw("colz");
      c4R2x3->cd(5) ; 
      alive[fedid][channelId][17]->Draw("colz");
      c4R2x3->cd(6) ; 
      alive[fedid][channelId][18]->Draw("colz");
      c4R2x3->Write() ;
	
      TCanvas * c4R2x4 = new TCanvas("c4R2x4", "4R - 2x4", 1000,600) ;
      c4R2x4->Divide(4,2) ;
      c4R2x4->cd(1) ; 
      alive_mirror[fedid][channelId][8]->Draw("colz");
      c4R2x4->cd(2) ; 
      alive_mirror[fedid][channelId][7]->Draw("colz");
      c4R2x4->cd(3) ; 
      alive_mirror[fedid][channelId][6]->Draw("colz");
      c4R2x4->cd(4) ; 
      alive_mirror[fedid][channelId][5]->Draw("colz");
      c4R2x4->cd(5) ; 
      alive[fedid][channelId][9]->Draw("colz");
      c4R2x4->cd(6) ; 
      alive[fedid][channelId][10]->Draw("colz");
      c4R2x4->cd(7) ; 
      alive[fedid][channelId][11]->Draw("colz");
      c4R2x4->cd(8) ; 
      alive[fedid][channelId][12]->Draw("colz");
      c4R2x4->Write() ;
	
      TCanvas * c4R1x5 = new TCanvas("c4R1x5", "4R - 1x5", 1250,300) ;
      c4R1x5->Divide(5,1) ;
      c4R1x5->cd(1) ; 
      alive[fedid][channelId][4]->Draw("colz");
      c4R1x5->cd(2) ; 
      alive[fedid][channelId][3]->Draw("colz");
      c4R1x5->cd(3) ; 
      alive[fedid][channelId][2]->Draw("colz");
      c4R1x5->cd(4) ; 
      alive[fedid][channelId][1]->Draw("colz");
      c4R1x5->cd(5) ; 
      alive[fedid][channelId][0]->Draw("colz");
      c4R1x5->Write() ;
    }
    else if(myPanelType=="3R") {
      sprintf(title,"FED%d_Channel%d",fedmin+fedid,channelId+fedchanmin[fedid]) ;
      TDirectory * dir = f->mkdir(title);
      dir->cd() ;
      TCanvas * c3R2x3 = new TCanvas("c3R2x3", "3R - 2x3", 750,600) ;
      c3R2x3->Divide(3,2) ;
      c3R2x3->cd(1) ; 
      alive_mirror[fedid][channelId][20]->Draw("colz");
      c3R2x3->cd(2) ; 
      alive_mirror[fedid][channelId][19]->Draw("colz");
      c3R2x3->cd(3) ; 
      alive_mirror[fedid][channelId][18]->Draw("colz");
      c3R2x3->cd(4) ; 
      alive[fedid][channelId][21]->Draw("colz");
      c3R2x3->cd(5) ; 
      alive[fedid][channelId][22]->Draw("colz");
      c3R2x3->cd(6) ; 
      alive[fedid][channelId][23]->Draw("colz");
      c3R2x3->Write() ;
	
      TCanvas * c3R2x4 = new TCanvas("c3R2x4", "3R - 2x4", 1000,600) ;
      c3R2x4->Divide(4,2) ;
      c3R2x4->cd(1) ; 
      alive_mirror[fedid][channelId][13]->Draw("colz");
      c3R2x4->cd(2) ; 
      alive_mirror[fedid][channelId][12]->Draw("colz");
      c3R2x4->cd(3) ; 
      alive_mirror[fedid][channelId][11]->Draw("colz");
      c3R2x4->cd(4) ; 
      alive_mirror[fedid][channelId][10]->Draw("colz");
      c3R2x4->cd(5) ; 
      alive[fedid][channelId][14]->Draw("colz");
      c3R2x4->cd(6) ; 
      alive[fedid][channelId][15]->Draw("colz");
      c3R2x4->cd(7) ; 
      alive[fedid][channelId][16]->Draw("colz");
      c3R2x4->cd(8) ; 
      alive[fedid][channelId][17]->Draw("colz");
      c3R2x4->Write() ;
	
      TCanvas * c3R2x5 = new TCanvas("c3R2x5", "3R - 2x5", 1250,600) ;
      c3R2x5->Divide(5,2) ;
      c3R2x5->cd(1) ; 
      alive_mirror[fedid][channelId][4]->Draw("colz");
      c3R2x5->cd(2) ; 
      alive_mirror[fedid][channelId][3]->Draw("colz");
      c3R2x5->cd(3) ; 
      alive_mirror[fedid][channelId][2]->Draw("colz");
      c3R2x5->cd(4) ; 
      alive_mirror[fedid][channelId][1]->Draw("colz");
      c3R2x5->cd(5) ; 
      alive_mirror[fedid][channelId][0]->Draw("colz");
      c3R2x5->cd(6) ; 
      alive[fedid][channelId][5]->Draw("colz");
      c3R2x5->cd(7) ; 
      alive[fedid][channelId][6]->Draw("colz");
      c3R2x5->cd(8) ; 
      alive[fedid][channelId][7]->Draw("colz");
      c3R2x5->cd(9) ; 
      alive[fedid][channelId][8]->Draw("colz");
      c3R2x5->cd(10) ; 
      alive[fedid][channelId][9]->Draw("colz");
      c3R2x5->Write() ;
    }
  }
  f->Write();
  f->Close() ;

  //OLD VERSION: PRODUCES A PS FILE
  /*   //First need to count number of pages we will */
  /*   //make due to awkward root interface for generating  */
  /*   //ps file */

  /*   unsigned int npages=0; */
  /*   for (int channel=0;channel<36;channel++){ */
  /*     if (data_roc[channel]==-1) continue; */
  /*     npages+=1+(data_roc[channel])/12; */
  /*   } */

  /*   int plotnumber=0; */

  /*   gStyle->SetPalette(1,0); */
  /*   TCanvas* c=0; */

  /*   unsigned int ipage=0; */

  /*   for (int channel=0;channel<36;channel++){ */
  /*     if (data_roc[channel]==-1) { */
  /*       cout << "No data on channel:"<<channel+1<<endl; */
  /*       continue; */
  /*     } else { */
  /*       cout << "Data on "<<data_roc[channel]+1<<" ROCs on channel:" */
  /* 	   << channel+1<<endl; */
  /*     } */
  /*     plotnumber=0; */


  /*     for (int roc=0;roc<data_roc[channel]+1;roc++){ */
      
  /*       if (plotnumber%12==0){ */
  /* 	TString name="PixelAlive FED channel="; */
  /* 	name=name+TString(channel+1); */
  /* 	c=new TCanvas(name,name, 700,800); */
  /* 	c->Divide(3 ,4); */
  /*       } */

  /*       c->cd(plotnumber%12+1); */

  /*       alive[channel][roc]->Draw("colz"); */

  /*       plotnumber++; */


  /*       if (plotnumber%12==0||roc==data_roc[channel]){ */
  /* 	ipage++; */
  /* 	if (npages==1) c->Print("PixelAlive.ps"); */
  /* 	if(ipage==1){ */
  /* 	  c->Print("PixelAlive.ps("); */
  /* 	} else if(ipage==npages){ */
  /* 	  c->Print("PixelAlive.ps)"); */
  /* 	} else { */
  /* 	  c->Print("PixelAlive.ps"); */
  /* 	} */

  /*       } */
  /*     } */
  /*   } */
  return 0;
}
