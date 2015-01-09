//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
// authors G. Cerati, M. Dinardo, K. Ecklund, B. HeyBurn, A. Kumar, E. Luiggi, L. Uplegger  //
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////
#include <fstream>
#include <iostream>
#include <sstream>
#include "PixelXmlReader.h"
#include "PixelHistoManager.h"
#include "PixelUtilities/PixelFEDDataTools/include/SLinkDecoder.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TFile.h"
#include "TString.h"
#include "TCanvas.h"
#include <string>
#include <unistd.h>
#include <time.h>
#include <TStyle.h>

using namespace std;
using namespace pos;

int main(){
  string mthn = "[main()]\t";
  cout << mthn << "Running Scurves analisys..." << endl;

  gStyle->SetPalette(1,0);
  //------------------------------------------------------------------------------------------------
  //Variables to measure timings
  time_t start,end;
  time_t startPattern,endPattern;
  double dif;
  //------------------------------------------------------------------------------------------------
  // Get paramaters from the xml file " filenames, feds, rocs... EELL
 
  PixelXmlReader pixelReader;
  pixelReader.readDocument("PixelSCurveAnalysis.xml");
  
  string dataFile        = pixelReader.getXMLAttribute("DataFileName"       ,"FileName");
  string calibFile       = pixelReader.getXMLAttribute("CalibFileName"      ,"FileName");
  string translationFile = pixelReader.getXMLAttribute("TranslationFileName","FileName");
  string detconfigFile   = pixelReader.getXMLAttribute("DetConfigFileName"  ,"FileName");
  string numOfEvents     = pixelReader.getXMLAttribute("NumOfEvents"        ,"Events");
  int    maxevent        = atoi(numOfEvents.c_str()); 

  //Set the fed/channel ids you want to analyze
  string FedMin = pixelReader.getXMLAttribute("FedMin","Fed");
  string FedMax = pixelReader.getXMLAttribute("FedMax","Fed");
  unsigned int fedmin = atoi(FedMin.c_str()); //read from the xml file...
  unsigned int fedmax = atoi(FedMax.c_str());
  unsigned int nOfFed = (fedmax - fedmin) + 1;
    //------------------------------------------------------------------------------------------------
  
  unsigned int fedchanmin[nOfFed];
  unsigned int fedchanmax[nOfFed];
  for (unsigned int numOfChannels = 0; numOfChannels != nOfFed; ++numOfChannels){
     string channelMin = pixelReader.getXMLAttribute("ChannelMin","Channel",numOfChannels);
     string channelMax = pixelReader.getXMLAttribute("ChannelMax","Channel",numOfChannels);
     fedchanmin[numOfChannels] = atoi(channelMin.c_str());
     fedchanmax[numOfChannels] = atoi(channelMax.c_str());
  } 
     
  //set the rocs you want to analyze
  string rocMin = pixelReader.getXMLAttribute("RocMin","Roc");
  string rocMax = pixelReader.getXMLAttribute("RocMax","Roc");  
  unsigned int rocmin = atoi(rocMin.c_str());
  unsigned int rocmax = atoi(rocMax.c_str());
    
/*    
  string rowMin = pixelReader.getXMLAttribute("RowMin","Row");
  string rowMax = pixelReader.getXMLAttribute("RowMax","Row");
  string colMin = pixelReader.getXMLAttribute("ColMin","Col");
  string colMax = pixelReader.getXMLAttribute("ColMax","Col");
  
  int rowmin = atoi(rowMin.c_str());
  int rowmax = atoi(rowMax.c_str());;
  int colmin = atoi(colMin.c_str());;
  int colmax = atoi(colMax.c_str());;
*/ 
  //Getting from xml file the flags for mean noise and mean threshold
  string rocnoise  = pixelReader.getXMLAttribute("RocNoiseMean","Sigma");
  string rocthresh = pixelReader.getXMLAttribute("RocThrMean","Thr"); 

  double RocNoiseFlag = atof(rocnoise.c_str());
  double RocThrFlag   = atof(rocthresh.c_str());

  cout << mthn << "RocNoiseFlag is: "<< RocNoiseFlag <<endl;
  cout << mthn << "RocThrFlag is: "  << RocThrFlag   <<endl;
//------------------------------------------------------------------------------------------------------

  //open the data file (please edit it with the correct name)
  std::ifstream in;
  in.open(dataFile.c_str(),ios::binary|ios::in);
  if(!in.is_open()){
    cout << mthn << "Can't open file: " << dataFile << " ...Check the file name" << endl;
    exit(1);
  }

  //compose error and root file names
  stringstream errFileName;
  stringstream rootFileName;
  if (fedmin==fedmax){
    errFileName << "FED" << fedmin << "_Channels" << fedchanmin[0] << "-" << fedchanmax[0] << "_PixelSCurvesErrors.txt";
    rootFileName << "FED" << fedmin << "_Channels" << fedchanmin[0] << "-" << fedchanmax[0] << "_PixelSCurves.root";
  }
  else{
    errFileName << "FED" << fedmin << "-" << fedmax << "_Channels" << fedchanmin[0] << "-" << fedchanmax[0] << "_PixelSCurvesErrors.txt";
    rootFileName << "FED" << fedmin << "-" << fedmax << "_Channels" << fedchanmin[0] << "-" << fedchanmax[0] << "_PixelSCurves.root";
  }
  //Open error file
  fstream out(errFileName.str().c_str(),fstream::out);;
  
  //Open root file
  TFile * rootFile = new TFile(rootFileName.str().c_str(),"RECREATE") ;

  //instance the PixelCalib class: it gets all info needed from calib.dat and translation.dat files (please edit them with the correct name)
  /*from new class... EELL 
    The PixelAnalysis class adds the few functions missing in PixelCalibConfiguration in order
    to run PixelAlive and SCurve.
  */
  
  PixelNameTranslation trans(translationFile.c_str());
  PixelDetectorConfig  detconfig(detconfigFile.c_str());
 
  PixelAnalysis calib(calibFile.c_str(),&trans, &detconfig);
 
  PixelHistoManager* histoManager = new PixelHistoManager(&calib,rootFile);
  histoManager->init();
  
  int ntrig=calib.nTriggersPerPattern();
  cout << mthn << "Ntriggers per pattern=" << ntrig;

  int nvcal=calib.nScanPoints("Vcal" );
  cout << " # scan points=" << nvcal ;

  int vcalmin  = int(calib.scanValueMin("Vcal"));
  int vcalmax  = int(calib.scanValueMax("Vcal"));
  int vcalstep = int(calib.scanValueStep("Vcal"));
  cout << mthn << "VCal => min=" << vcalmin << " max=" << vcalmax  << " step=" << vcalstep << endl;


  //compute the number of feds, channel and rocs to be analyzed
 // unsigned int nOfFed  = fedmax-fedmin+1;
  unsigned int nOfChan[nOfFed];
  unsigned int maxNOfChan = 0;
  for (unsigned int pp=0;pp<nOfFed;++pp){
    nOfChan[pp] = fedchanmax[pp]-fedchanmin[pp]+1;
    if (maxNOfChan<fedchanmax[pp]-fedchanmin[pp]+1) maxNOfChan=fedchanmax[pp]-fedchanmin[pp]+1;
  }

  std::vector<unsigned int> const *rows=0;
  std::vector<unsigned int> const *cols=0;

  int nEvent=0;
  unsigned int vcalvalue=0;

  //event loop: read data
  cout << mthn << "Starting event loop..." << endl;		   
  SLinkDecoder sLinkDecoder(&in);				   
  Word64 runNumber;						   
  sLinkDecoder.getNextWord64(runNumber);			   
  cout << " Run Number " << runNumber.getWord() << endl;	   
  PixelSLinkEvent sLinkEvent;
//   PixelSLinkEvent sLinkEventSkip;
  time (&start);
  time (&startPattern);
  while(sLinkDecoder.getNextEvent(sLinkEvent) && nEvent<maxevent){
//     sLinkDecoder.getNextEvent(sLinkEventSkip);
    sLinkEvent.decodeEvent();
    if (nEvent%10000==0) cout << mthn << "nEvent:"<< nEvent << endl;
   
    if (nEvent%(ntrig*nvcal*calib.getNumberOfFeds())==0) {
      cout << mthn << "New pattern at: " << ntrig*nvcal*calib.getNumberOfFeds() << endl;
      if (nEvent != 0) {
	histoManager->fit();//fit histos so far and fill summary plots
	histoManager->reset();//reset histos
      }
      //new pattern: get the rows and columns of this pattern
      calib.getRowsAndCols(nEvent/(ntrig*calib.getNumberOfFeds()),rows,cols);
      
      assert(rows!=0);
      assert(cols!=0);

      cout << mthn << "Event: " << nEvent << " Rows:";
      for( unsigned int i=0;i<rows->size();i++){
        cout << (*rows)[i] << " ";
      }
      cout <<" Cols:";
      for( unsigned int i=0;i<cols->size();i++){
	cout << (*cols)[i] << " ";
      }
      cout << endl;
      //set histo for new cells
      histoManager->setCellHisto(rows,cols);      
      time (&endPattern);
      dif = difftime (endPattern,startPattern);
      cout << mthn << "It took you " << dif << " seconds to do this pattern!" << endl;
      time (&startPattern);
    }
    
    if(nEvent%(ntrig)==0) {
//      vcalvalue= calib.scanValue("Vcal",nEvent/(ntrig*calib.getNumberOfFeds()));
      vcalvalue= calib.scanCounter("Vcal",nEvent/(ntrig*calib.getNumberOfFeds()))*vcalstep+vcalmin;
    }

    //get fedid and skip if not in [fedmin,fedmax]
    int fed = sLinkEvent.getHeader().getSource_id();
    if ( fed<((int)fedmin) || fed>((int)fedmax) ) {
      nEvent++;
      continue;
    }
    int fedn = fed-fedmin;
    vector<PixelHit> hits           = sLinkEvent.getHits();
    vector<PixelHit>::iterator ihit = hits.begin();
    //loop over the hits in the event
//    cout << mthn << "Event=" << nEvent << " size=" << hits.size() << endl;
    for (;ihit!=hits.end();++ihit) {
      //cout << "begin of hits loop" << endl;

      //get channel, roc, row, col
      unsigned int linkid = ihit->getLink_id();
      unsigned int rocid  = ihit->getROC_id();
      unsigned int row    = ihit->getRow();
      unsigned int col    = ihit->getColumn();

//       cout << mthn 
//            << "Linkid: " << linkid
//            << " Rocid: " << rocid
// 	   << " FedMin: " << fedchanmin[fedn]
// 	   << " FedMax: " << fedchanmax[fedn]
// 	   << endl;
      //continue if not in [fedchanmin,fedchanmax] and [rocmin,rocmax]
      if (linkid<fedchanmin[fedn]||linkid>fedchanmax[fedn]) continue;
      if (rocid<rocmin||rocid>rocmax) continue;

      if (row>=80||col>=52) { 	
	out << mthn << "Row or column outside limits (fed,link,roc,row,col):"			      
	    << fed << "," << linkid << "," << rocid << "," << row << "," << col << endl;
	continue;									      
      }
      
      assert(linkid>0&&linkid<37);
      assert(rocid<25);
      assert(row<80);
      assert(col<52);

      bool valid_row=false;
      bool valid_col=false;
      for (unsigned int i=0;i<rows->size();i++){
        if (row==(*rows)[i]){ 
	  valid_row=true;
	  break;
	}
      }
      for (unsigned int i=0;i<cols->size();i++){
	if (col==(*cols)[i]){
	  valid_col=true;
          break;
	}
      }

      //fill histograms
      if (valid_row&&valid_col){
        histoManager->fillCellHisto(fed,linkid,rocid,row,col,vcalvalue);
      }
      else{
	cout << mthn << "Not valid row or column (fed,link,roc,row,col): "
	<< fed << "," << linkid << "," << rocid << "," << row << "!=" << (*rows)[0] << "," << col << "!=" << (*cols)[0] << endl;	  	
      }
    } //cout << "end of hits loop" << endl;
    nEvent++;
  }//cout << "end of while loop" << endl;
  time (&end);
  dif = difftime (end,start);
  cout << mthn << "It took you " << dif << " seconds to do the job idiots!" << endl;

  histoManager->fit();//fit histos so far and fill summary plots
  histoManager->reset();//reset histos
  cout << mthn << "...Processed:" << nEvent << " triggers" <<endl;
  
  /////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////
  //save plots
  cout << mthn << "Making FULL plots" << endl;

  //create output file
  TDirectory * dirSP = rootFile->mkdir("SummaryPlots");
  dirSP->cd();
  TH1F * hMeanNoise     = new TH1F("MeanNoise","MeanNoise",100,0,10);
  TH1F * hMeanThreshold = new TH1F("MeanThreshold","MeanThreshold",100,0,200);
  TH1F * hRmsNoise     	= new TH1F("RmsNoise","RmsNoise",100,0,1);
  TH1F * hRmsThreshold  = new TH1F("RmsThreshold","RmsThreshold",100,0,10);
   
  char title[20] ;
 
  //these are the kind of plots we save
  TString plotType[] = {"noise","threshold","prob","noiseMap","thresholdMap","probMap"};

  int 	       disk   = 0;
  int 	       blade  = 0;
  int 	       panel  = 0;
  unsigned int myfed  = 0;
  unsigned int mycha  = 0;
  TString panelType   = "";
  TString myPanelType = "";

  //for every panel (=every channel) create a directory and fill it with canvases
  //(one canvas per kind of plots per plaquette)
  vector<pair<vector<unsigned int>, string> > panelTypes = calib.getFedChannelHWandTypeInfo();
  for (unsigned int it=0;it<panelTypes.size();it++) {
    disk  = panelTypes[it].first[0];
    blade = panelTypes[it].first[1];
    panel = panelTypes[it].first[2];
    myfed = panelTypes[it].first[3];
    mycha = panelTypes[it].first[4];
    myPanelType = panelTypes[it].second;

//    cout << mthn << "My fed => " << myfed << "<" << fedmin << "||" << myfed << ">" << fedmax << endl;
//    myfed+=32;
  
    if ( myfed<fedmin || myfed>fedmax ) continue;
    unsigned int fedid=myfed-fedmin;
    
//    cout << mthn << "My Channel => " << mycha << "<" << fedchanmin[fedid] << "||" << mycha << ">" << fedchanmax[fedid] << endl;
    if ( mycha<fedchanmin[fedid] || mycha>fedchanmax[fedid] ) continue;
    unsigned int channelId=mycha-fedchanmin[fedid];
    
    cout << mthn << "FED=" << myfed << " Channel " << mycha << " panelType=" << myPanelType << " blade=" << blade << " panel=" << panel << " disk=" << disk << endl;

    stringstream pan("");
    stringstream plq("");
    stringstream roc("");
    //    pan << "FPix_BmO_D"<<disk<<"_BLD"<<blade<<"_PNL"<<panel;
    pan << "FPix_BpO_D" << disk << "_BLD" << blade << "_PNL" << panel;

    if(myPanelType=="4L") {
      sprintf(title,"FED%d_Channel%d",fedmin+fedid,channelId+fedchanmin[fedid]) ;
      TDirectory * dir = rootFile->mkdir(title);
      for (int kk=0;kk<6;++kk) { //loop over the kind of plots, i.e. plotType[]
	dir->cd() ;
	panelType = "4L1x2";
	plq.str("");
	plq <<pan.str()<<"_PLQ"<<1;
	TCanvas * c4L1x2 = new TCanvas(plotType[kk]+panelType, "4L - 1x2", 500,300) ;
	c4L1x2->Divide(2,1) ;
	c4L1x2->cd(1) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 0;//1;
//	cout << mthn << "roc: " << roc.str() << endl;
	histoManager->draw(roc.str(),kk);
	c4L1x2->cd(2) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 1;//0;
	histoManager->draw(roc.str(),kk);
	c4L1x2->Write() ;
	      
	panelType = "4L2x3";
	TCanvas * c4L2x3 = new TCanvas(plotType[kk]+panelType, "4L - 2x3", 750,600) ;
	plq.str("");
	plq <<pan.str()<<"_PLQ"<<2;
	c4L2x3->Divide(3,2) ;
	c4L2x3->cd(1) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 5;//7;
	histoManager->draw(roc.str(),kk);
	c4L2x3->cd(2) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 4;//6;
	histoManager->draw(roc.str(),kk);
	c4L2x3->cd(3) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 3;//5;
	histoManager->draw(roc.str(),kk);
	c4L2x3->cd(4) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 0;//2;
	histoManager->draw(roc.str(),kk);
	c4L2x3->cd(5) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 1;//3;
	histoManager->draw(roc.str(),kk);
	c4L2x3->cd(6) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 2;//4;
	histoManager->draw(roc.str(),kk);
	c4L2x3->Write() ;
	      
	panelType = "4L2x4";
	TCanvas * c4L2x4 = new TCanvas(plotType[kk]+panelType, "4L - 2x4", 1000,600) ;
	plq.str("");
	plq <<pan.str()<<"_PLQ"<<3;
	c4L2x4->Divide(4,2) ;
	c4L2x4->cd(1) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 7;//15;
	histoManager->draw(roc.str(),kk);
	c4L2x4->cd(2) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 6;//14;
	histoManager->draw(roc.str(),kk);
	c4L2x4->cd(3) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 5;//13;
	histoManager->draw(roc.str(),kk);
	c4L2x4->cd(4) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 4;//12;
	histoManager->draw(roc.str(),kk);
	c4L2x4->cd(5) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 0;//8;
	histoManager->draw(roc.str(),kk);
	c4L2x4->cd(6) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 1;//9;
	histoManager->draw(roc.str(),kk);
	c4L2x4->cd(7) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 2;//10;
	histoManager->draw(roc.str(),kk);
	c4L2x4->cd(8) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 3;//11;
	histoManager->draw(roc.str(),kk);
	c4L2x4->Write() ;
	      
	panelType = "4L1x5";
	TCanvas * c4L1x5 = new TCanvas(plotType[kk]+panelType, "4L - 1x5", 1250,300) ;
	plq.str("");
	plq <<pan.str()<<"_PLQ"<<4;
	c4L1x5->Divide(5,1) ;
	c4L1x5->cd(1) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 0;//20;
	histoManager->draw(roc.str(),kk);
	c4L1x5->cd(2) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 1;//19;
	histoManager->draw(roc.str(),kk);
	c4L1x5->cd(3) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 2;//18;
	histoManager->draw(roc.str(),kk);
	c4L1x5->cd(4) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 3;//17;
	histoManager->draw(roc.str(),kk);
	c4L1x5->cd(5) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 4;//16;
	histoManager->draw(roc.str(),kk);
	c4L1x5->Write() ;
      }	      
    }
    else if(myPanelType=="3L") {
      sprintf(title,"FED%d_Channel%d",fedmin+fedid,channelId+fedchanmin[fedid]) ;
      TDirectory * dir = rootFile->mkdir(title);
      for (int kk=0;kk<6;++kk) { //loop over the kind of plots, i.e. plotType[]
	dir->cd() ;
	panelType = "3L2x3";
	TCanvas * c3L2x3 = new TCanvas(plotType[kk]+panelType, "3L - 2x3", 750,600) ;
	plq.str("");
	plq <<pan.str()<<"_PLQ"<<1;
	c3L2x3->Divide(3,2) ;
	c3L2x3->cd(1) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 5;//5;
	histoManager->draw(roc.str(),kk);
	c3L2x3->cd(2) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 4;//4;
	histoManager->draw(roc.str(),kk);
	c3L2x3->cd(3) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 3;//3;
	histoManager->draw(roc.str(),kk);
	c3L2x3->cd(4) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 0;//0;
	histoManager->draw(roc.str(),kk);
	c3L2x3->cd(5) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 1;//1;
	histoManager->draw(roc.str(),kk);
	c3L2x3->cd(6) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 2;//2;
	histoManager->draw(roc.str(),kk);
	c3L2x3->Write() ;
	      
	panelType = "3L2x4";
	TCanvas * c3L2x4 = new TCanvas(plotType[kk]+panelType, "3L - 2x4", 1000,600) ;
	plq.str("");
	plq <<pan.str()<<"_PLQ"<<2;
	c3L2x4->Divide(4,2) ;
	c3L2x4->cd(1) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 7;//13;
	histoManager->draw(roc.str(),kk);
	c3L2x4->cd(2) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 6;//12;
	histoManager->draw(roc.str(),kk);
	c3L2x4->cd(3) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 5;//11;
	histoManager->draw(roc.str(),kk);
	c3L2x4->cd(4) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 4;//10;
	histoManager->draw(roc.str(),kk);
	c3L2x4->cd(5) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 0;//6;
	histoManager->draw(roc.str(),kk);
	c3L2x4->cd(6) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 1;//7;
	histoManager->draw(roc.str(),kk);
	c3L2x4->cd(7) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 2;//8;
	histoManager->draw(roc.str(),kk);
	c3L2x4->cd(8) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 3;//9;
	histoManager->draw(roc.str(),kk);
	c3L2x4->Write() ;

	panelType = "3L2x5";
	TCanvas * c3L2x5 = new TCanvas(plotType[kk]+panelType, "3L - 2x5", 1250,600) ;
	plq.str("");
	plq <<pan.str()<<"_PLQ"<<3;
	c3L2x5->Divide(5,2) ;
	c3L2x5->cd(1) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 9;//23;
	histoManager->draw(roc.str(),kk);
	c3L2x5->cd(2) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 8;//22;
	histoManager->draw(roc.str(),kk);
	c3L2x5->cd(3) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 7;//21;
	histoManager->draw(roc.str(),kk);
	c3L2x5->cd(4) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 6;//20;
	histoManager->draw(roc.str(),kk);
	c3L2x5->cd(5) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 5;//19;
	histoManager->draw(roc.str(),kk);
	c3L2x5->cd(6) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 0;//14;
	histoManager->draw(roc.str(),kk);
	c3L2x5->cd(7) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 1;//15;
	histoManager->draw(roc.str(),kk);
	c3L2x5->cd(8) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 2;//16;
	histoManager->draw(roc.str(),kk);
	c3L2x5->cd(9) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 3;//17;
	histoManager->draw(roc.str(),kk);
	c3L2x5->cd(10) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 4;//18;
	histoManager->draw(roc.str(),kk);
	c3L2x5->Write() ;
      }
    }
    if(myPanelType=="4R") {
      sprintf(title,"FED%d_Channel%d",fedmin+fedid,channelId+fedchanmin[fedid]) ;
      TDirectory * dir = rootFile->mkdir(title);
      for (int kk=0;kk<6;++kk) { //loop over the kind of plots, i.e. plotType[]
	dir->cd() ;
	panelType = "4R1x2";
	TCanvas * c4R1x2 = new TCanvas(plotType[kk]+panelType, "4R - 1x2", 500,300) ;
	plq.str("");
	plq <<pan.str()<<"_PLQ"<<1;
	c4R1x2->Divide(2,1) ;
	c4R1x2->cd(1) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 0;//20;
	histoManager->draw(roc.str(),kk);
	c4R1x2->cd(2) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 1;//19;
	histoManager->draw(roc.str(),kk);
	c4R1x2->Write() ;
	  
	panelType = "4R2x3";
	TCanvas * c4R2x3 = new TCanvas(plotType[kk]+panelType, "4R - 2x3", 750,600) ;
	plq.str("");
	plq <<pan.str()<<"_PLQ"<<2;
	c4R2x3->Divide(3,2) ;
	c4R2x3->cd(1) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 5;//15;
	histoManager->draw(roc.str(),kk);
	c4R2x3->cd(2) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 4;//14;
	histoManager->draw(roc.str(),kk);
	c4R2x3->cd(3) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 3;//16;
	histoManager->draw(roc.str(),kk);
	c4R2x3->cd(4) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 0;//16;
	histoManager->draw(roc.str(),kk);
	c4R2x3->cd(5) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 1;//17;
	histoManager->draw(roc.str(),kk);
	c4R2x3->cd(6) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 2;//18;
	histoManager->draw(roc.str(),kk);
	c4R2x3->Write() ;
	  
	panelType = "4R2x4";
	TCanvas * c4R2x4 = new TCanvas(plotType[kk]+panelType, "4R - 2x4", 1000,600) ;
	plq.str("");
	plq <<pan.str()<<"_PLQ"<<3;
	c4R2x4->Divide(4,2) ;
	c4R2x4->cd(1) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 7;//8;
	histoManager->draw(roc.str(),kk);
	c4R2x4->cd(2) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 6;//7;
	histoManager->draw(roc.str(),kk);
	c4R2x4->cd(3) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 5;//6;
	histoManager->draw(roc.str(),kk);
	c4R2x4->cd(4) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 4;//5;
	histoManager->draw(roc.str(),kk);
	c4R2x4->cd(5) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 0;//9;
	histoManager->draw(roc.str(),kk);
	c4R2x4->cd(6) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 1;//10;
	histoManager->draw(roc.str(),kk);
	c4R2x4->cd(7) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 2;//11;
	histoManager->draw(roc.str(),kk);
	c4R2x4->cd(8) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 3;//12;
	histoManager->draw(roc.str(),kk);
	c4R2x4->Write() ;
	  
	panelType = "4R1x5";
	TCanvas * c4R1x5 = new TCanvas(plotType[kk]+panelType, "4R - 1x5", 1250,300) ;
	plq.str("");
	plq <<pan.str()<<"_PLQ"<<4;
	c4R1x5->Divide(5,1) ;
	c4R1x5->cd(1) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 0;//4;
	histoManager->draw(roc.str(),kk);
	c4R1x5->cd(2) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 1;//3;
	histoManager->draw(roc.str(),kk);
	c4R1x5->cd(3) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 2;//2;
	histoManager->draw(roc.str(),kk);
	c4R1x5->cd(4) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 3;//1;
	histoManager->draw(roc.str(),kk);
	c4R1x5->cd(5) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 4;//0;
	histoManager->draw(roc.str(),kk);
	c4R1x5->Write() ;
      }      
    }
    else if(myPanelType=="3R") {
      sprintf(title,"FED%d_Channel%d",fedmin+fedid,channelId+fedchanmin[fedid]) ;
      TDirectory * dir = rootFile->mkdir(title);
      for (int kk=0;kk<6;++kk) { //loop over the kind of plots, i.e. plotType[]
	dir->cd() ;
	panelType = "3R2x3";
	TCanvas * c3R2x3 = new TCanvas(plotType[kk]+panelType, "3R - 2x3"+plotType[kk], 750,600) ;
	plq.str("");
	plq <<pan.str()<<"_PLQ"<<1;
	c3R2x3->Divide(3,2) ;
	c3R2x3->cd(1) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 5;//20;
	histoManager->draw(roc.str(),kk);
	c3R2x3->cd(2) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 4;//19;
	histoManager->draw(roc.str(),kk);
	c3R2x3->cd(3) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 3;//18;
	histoManager->draw(roc.str(),kk);
	c3R2x3->cd(4) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 0;//21;
	histoManager->draw(roc.str(),kk);
	c3R2x3->cd(5) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 1;//22;
	histoManager->draw(roc.str(),kk);
	c3R2x3->cd(6) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 2;//23;
	histoManager->draw(roc.str(),kk);
	c3R2x3->Write() ;
	  
	panelType = "3R2x4";
	TCanvas * c3R2x4 = new TCanvas(plotType[kk]+panelType, "3R - 2x4"+plotType[kk], 1000,600) ;
	plq.str("");
	plq <<pan.str()<<"_PLQ"<<2;
	c3R2x4->Divide(4,2) ;
	c3R2x4->cd(1) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 7;//13;
	histoManager->draw(roc.str(),kk);
	c3R2x4->cd(2) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 6;//12;
	histoManager->draw(roc.str(),kk);
	c3R2x4->cd(3) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 5;//11;
	histoManager->draw(roc.str(),kk);
	c3R2x4->cd(4) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 4;//10;
	histoManager->draw(roc.str(),kk);
	c3R2x4->cd(5) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 0;//14;
	histoManager->draw(roc.str(),kk);
	c3R2x4->cd(6) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 1;//15;
	histoManager->draw(roc.str(),kk);
	c3R2x4->cd(7) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 2;//16;
	histoManager->draw(roc.str(),kk);
	c3R2x4->cd(8) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 3;//17;
	histoManager->draw(roc.str(),kk);
	c3R2x4->Write() ;
	  
	panelType = "3R2x5";
	TCanvas * c3R2x5 = new TCanvas(plotType[kk]+panelType, "3R - 2x5"+plotType[kk], 1250,600) ;
	plq.str("");
	plq <<pan.str()<<"_PLQ"<<3;
	c3R2x5->Divide(5,2) ;
	c3R2x5->cd(1) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 9;//4;
	histoManager->draw(roc.str(),kk);
	c3R2x5->cd(2) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 8;//3;
	histoManager->draw(roc.str(),kk);
	c3R2x5->cd(3) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 7;//2;
	histoManager->draw(roc.str(),kk);
	c3R2x5->cd(4) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 6;//1;
	histoManager->draw(roc.str(),kk);
	c3R2x5->cd(5) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 5;//0;
	histoManager->draw(roc.str(),kk);
	c3R2x5->cd(6) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 0;//5;
	histoManager->draw(roc.str(),kk);
	c3R2x5->cd(7) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 1;//6;
	histoManager->draw(roc.str(),kk);
	c3R2x5->cd(8) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 2;//7;
	histoManager->draw(roc.str(),kk);
	c3R2x5->cd(9) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 3;//8;
	histoManager->draw(roc.str(),kk);
	c3R2x5->cd(10) ;
	roc.str("");
	roc <<plq.str()<<"_ROC"<< 4;//9;
	histoManager->draw(roc.str(),kk);
	c3R2x5->Write() ;
      }
    }
  }

  //summary plots
  histoManager->fillMeanNoise(hMeanNoise,out);
  histoManager->fillMeanThreshold(hMeanThreshold,out);
  histoManager->fillRmsNoise(hRmsNoise,out);
  histoManager->fillRmsThreshold(hRmsThreshold,out);
  
  
  dirSP->cd();
  hMeanNoise->Write();
  hMeanThreshold->Write();
  hRmsNoise->Write();
  hRmsThreshold->Write();
  cout << mthn << "closing output root file...";
  rootFile->Close() ;
  cout << "done" << endl;
  histoManager->destroy();
  delete histoManager;

/*   TCanvas* c=0; */
/*   if(false)   {// summary plots */
/*     cout << "Making plots" << endl; */
/*     for(unsigned int channel=chanmin; channel<=chanmax; channel++) { */
/*       for(unsigned int roc=rocmin; roc<=rocmax; roc++) { */
	
/* 	c= new TCanvas("ROC_Scurve_Results","ROC Scurve Results", 700, 800); */
/* 	c->Divide(2,3); */
/* 	c->cd(1); */
/* 	roceff[channel][roc].drawThresholds(); */
/* 	c->cd(2); */
/* 	roceff[channel][roc].drawThreshold(); */
/* 	c->cd(3); */
/* 	roceff[channel][roc].drawNoises(); */
/* 	c->cd(4); */
/* 	roceff[channel][roc].drawNoise(); */
/* 	c->cd(5); */
/* 	roceff[channel][roc].drawFitProbs(); */
/* 	c->cd(6); */
/* 	roceff[channel][roc].drawFitProb(); */
	
/* 	TString name="SCurveResults"; */
/* 	name=name+"_Channel"; */
/* 	name+=(channel+1); */
/* 	name+="_ROC"; */
/* 	name+=(roc+1); */
/* 	name+=".pdf"; */
/* 	c->Print(name); */
	
/* 	int plotnumber=0; */
	
/* 	bool plotted=false; */
/* 	for(int row = 35;row<45;row++){ */
/* 	  for(int col = 21; col<31;col++) { */
	    
/* 	    if (plotnumber%12==0){ */
/* 	      TString name="SCurve"; */
/* 	      name=name+TString(plotnumber/12+1); */
/* 	      c=new TCanvas(name,name, 700,800); */
/* 	      c->Divide(3 ,4); */
/* 	    } */

/* 	    if (roceff[channel][roc].filled(row,col)){ */
/* 	      c->cd(plotnumber%12+1); */
/* 	      roceff[channel][roc].draw(row,col); */
/* 	      plotted=true; */
/* 	      plotnumber++; */
/* 	    } */
	    
/* 	    if (plotted&&(plotnumber%12==0)){ */
/* 	      TString name="SCurves"; */
/* 	      name=name+"_Channel"; */
/* 	      name+=(channel+1); */
/* 	      name+="_ROC"; */
/* 	      name+=(roc+1); */
/* 	      name+=".ps"; */
/* 	      if(plotnumber==12) name+="("; */
/* 	      c->Print(name); */
/* 	      plotted=false; */
/* 	    } //new page */
/* 	  } // loop col */
/* 	} // loop row */
/* 	if (plotted) { */
/* 	  TString name="SCurves"; */
/* 	  name=name+"_Channel"; */
/* 	  name+=(channel+1); */
/* 	  name+="_ROC"; */
/* 	  name+=(roc+1); */
/* 	  name+=".ps)"; */
/* 	  c->Print(name); */
/* 	} */
/*       } // roc */
/*     } // channel */
/*   } // make plots   */
  return 0;
}// int main()

