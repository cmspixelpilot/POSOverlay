// author K. Ecklund
#include <fstream>
#include <sstream>
#include "pixelROCscurve.h"
#include "PixelCalib.h"
#include "PixelSLinkData.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TFile.h"
#include "TString.h"
#include "TCanvas.h"

using namespace std;

void pixel_scurve(){
  std::cout << "In pixel_scurve" << std::endl;

  //Set the fed/channel ids you want to analyze
  unsigned int fedmin = 33;
  unsigned int fedmax = fedmin; 
  //to avoid memory problems do not run more than 12 channels per time
  unsigned int fedchanmin[] = {13} ; //{fedchanmin(fedmin),...,fedchanmin(fedmax)}
  unsigned int fedchanmax[] = {24} ; //{fedchanmax(fedmin),...,fedchanmax(fedmax)}

  //set the rocs you want to analyze
  unsigned int rocmin;
  unsigned int rocmax;
  rocmin=1;
  rocmax=24;

  //set the rows and cols you want to analyze
  int rowmin = 35;
  int rowmax = 44;
  int colmin = 21;
  int colmax = 30;

  //open the data file (please edit it with the correct name)
  std::ifstream in;
  in.open("SCurve_283.dmp",ios::binary|ios::in);
  assert(in.good());

  //open the error log file
  fstream out;
  char errFileName[20];
  if (fedmin==fedmax)  sprintf(errFileName,"FED%d_Channel%d-%d_PixSCurvesErrors.txt",fedmin,fedchanmin[0],fedchanmax[0]) ;
  else sprintf(errFileName,"FED%d-%d_PixSCurvesErrors.txt",fedmin,fedmax);
  out.open(errFileName,fstream::out);


  // for releases pos_2_4_3 and later
  // first 64 bits of dmp file gives run number
  uint64_t run_num;
  in.read((char*)&run_num,8);
  cout << "Run number=" << run_num<<endl;

  //instance the PixelCalib class: it gets all info needed from calib.dat and translation.dat files (please edit them with the correct name)
  PixelCalib calib("calib_SCurve_283.dat","translation.dat");

  //get the list of panel types associated to the disk, blade, panel numbers and the fed and the channel (according to translation.dat)
  std::vector<std::pair<vector<int>,std::string> > panelTypes;
  panelTypes = calib.getPanelTypesAndFEDInfo() ;

  int ntrig=calib.nTriggersPerPattern();

  cout << " ntriggers per pattern=" << ntrig;

  int nvcal=calib.nScanPoints("Vcal" );

  cout << " # scan points=" << nvcal ;

  int vcalmin=int(calib.scanValueMin("Vcal"));
  int vcalmax=int(calib.scanValueMax("Vcal"));
  cout << " min=" << vcalmin << " max=" << vcalmax << endl;

  pixel::SLinkData pixeldata;

  //compute the number of feds, channel and rocs to be analyzed
  unsigned int nOfFed  = fedmax-fedmin+1;
  unsigned int nOfChan[nOfFed];
  unsigned int maxNOfChan = 0;
  for (unsigned int pp=0;pp<nOfFed;++pp){
    nOfChan[pp] = fedchanmax[pp]-fedchanmin[pp]+1;
    if (maxNOfChan<fedchanmax[pp]-fedchanmin[pp]+1) maxNOfChan=fedchanmax[pp]-fedchanmin[pp]+1;
  }
  unsigned int nOfROCS = rocmax - rocmin +1;

  //create the matrix with the scurves plots
  pixelROCscurve roceff[nOfFed][maxNOfChan][nOfROCS];

  //initialize expected fed links and all rocs
  cout << "initialize expected fed links and all rocs...";
  for(unsigned int fedid=0; fedid<nOfFed; fedid++){
    for(unsigned int linkid=0;linkid<nOfChan[fedid];linkid++){
      for(unsigned int rocid=0;rocid<nOfROCS;rocid++){//init all rocs
	//cout << "init link="<<linkid+fedchanmin[fedid]<< " roc="<<rocid+1<<"...";
	roceff[fedid][linkid][rocid].init(linkid+fedchanmin[fedid],rocid+rocmin,nvcal,vcalmin,vcalmax,ntrig);
      }
    }
  }
  cout << " success!" << endl;


  std::vector<unsigned int> const *rows=0;
  std::vector<unsigned int> const *cols=0;

  int nEvent=0;

  //set the maximum naumber of events to be processed
  int maxevent=2000000;
  //maxevent=15000;

  unsigned int vcalvalue=0;

  //event loop: read data
  cout << "Starting event loop..." << endl;
  while(pixeldata.load(in)&&nEvent<maxevent){
    //cout << "begin of event" << endl;

    if (nEvent%10000==0) cout << "nEvent:"<<nEvent<<endl;
   
    if (nEvent%(ntrig*nvcal*calib.getNumberOfFeds())==0) {
      cout << "new pattern" <<endl;
     
      //new pattern: get the rows and columns of this pattern
      calib.getRowsAndCols(nEvent/(ntrig*calib.getNumberOfFeds()),rows,cols);

      assert(rows!=0);
      assert(cols!=0);

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
 
    if(nEvent%(ntrig)==0) {
      vcalvalue= calib.scanValue("Vcal",nEvent/(ntrig*calib.getNumberOfFeds()));
    }

    //get fedid and skip if not in [fedmin,fedmax]
    int fed = pixeldata.getHeader().getSource_id();
    if ( fed<((int)fedmin) || fed>((int)fedmax) ) {nEvent++;continue;}
    int fedn = fed-fedmin;

    std::vector<pixel::SLinkHit> hits=pixeldata.getHits();
    std::vector<pixel::SLinkHit>::iterator ihit=hits.begin();

    //loop over the hits in the event
    for (;ihit!=hits.end();++ihit) {
      //cout << "begin of hits loop" << endl;

      //get channel, roc, row, col
      unsigned int linkid=ihit->get_link_id();
      unsigned int rocid=ihit->get_roc_id();
      unsigned int row=ihit->get_row();
      unsigned int col=ihit->get_col();
/*       cout << "fed=" << fed << " linkid=" << linkid */
/* 	  << " rocid=" << rocid */
/* 	  << " row=" << row */
/* 	  << " col=" << col << endl; */

      //continue if not in [fedchanmin,fedchanmax] and [rocmin,rocmax]
      if (linkid<fedchanmin[fedn]||linkid>fedchanmax[fedn]){continue;}
      if (rocid<rocmin||rocid>rocmax){ continue;}

      if (row>=80||col>=52) { 	
	out << "Row or column outside limits (fed,link,roc,row,col):"<<
	  fed<<","<<linkid<<","<<rocid<<"," << row<<","<<col<<endl;
	continue;
      }
      
      assert(linkid>0&&linkid<37);
      assert(rocid<25);
      assert(row<80);
      assert(col<52);

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
	assert(linkid>=1&&linkid<=36);
	assert(rocid>=1&&rocid<=24);
	//cout << "filling:" << fedn << " "<< linkid-fedchanmin[fedn] << " " << rocid <<" "<< row<<" " << col << endl;
	roceff[fedn][linkid-fedchanmin[fedn]][rocid-rocmin].fill(row,col,vcalvalue);
      }
      else{
	out << "Not valid row or column (fed,link,roc,row,col):"<<
	    fed<<","<<linkid<<","<<rocid<<"," << row<<","<<col<<endl;
      }
      //cout << "end of hits loop" << endl;
    }
    nEvent++;
    //cout << "end of event" << endl;
  }
  cout << "...Processed:"<<nEvent<<" triggers"<<endl;
  
    // fits
  cout << "Fitting..." ;
  for(unsigned int fedid=0; fedid<nOfFed; fedid++){
    for(unsigned int channel=0;channel<nOfChan[fedid];channel++){
      for(unsigned int roc=0; roc<nOfROCS; roc++) {
	for(int row = rowmin;row<=rowmax;row++){
	  for(int col = colmin; col<=colmax;col++) {
	    //cout << "fit array pos=" << fedid << " " << channel << " " << roc << " " << row << " " << col << endl;
	    roceff[fedid][channel][roc].fit(row,col);
	  }  //col
	} //row
      } //roc
    } //chan
  }
  cout << "done!" << endl;

  //save plots
  cout << "Making FULL plots" << endl;

  //create output file
  char outFileName[20];
  if (fedmin==fedmax)  sprintf(outFileName,"FED%d_Ch%d-%d_SCurves.root",fedmin,fedchanmin[0],fedchanmax[0]) ;
  else sprintf(outFileName,"FED%d-%d_PixSCurves.root",fedmin,fedmax);
  TFile * f = new TFile(outFileName,"RECREATE") ;

  TDirectory * dirSP = f->mkdir("SummaryPlots");
  dirSP->cd();
  TH1F * hMeanNoise = new TH1F("MeanNoise","MeanNoise",100,0,10);
  TH1F * hMeanThreshold = new TH1F("MeanThreshold","MeanThreshold",100,0,200);

  char title[20] ;
 
  //these are the kind of plots we save
  TString plotType[] = {"noise","threshold","prob","noiseMap","thresholdMap","probMap"};

  int disk = 0;
  int blade = 0;
  int panel = 0;
  unsigned int myfed = 0;
  unsigned int mycha = 0;
  TString panelType = "";
  TString myPanelType = "";

  //for every panel (=every channel) create a directory and fill it with canvases
  //(one canvas per kind of plots per plaquette)
  for (unsigned int it=0;it<panelTypes.size();it++) {

    disk  = panelTypes[it].first[0];
    blade = panelTypes[it].first[1];
    panel = panelTypes[it].first[2];
    myfed = panelTypes[it].first[3];
    mycha = panelTypes[it].first[4];
    myPanelType = panelTypes[it].second;

    myfed+=32;
  
    if (myfed<fedmin||myfed>fedmax) continue;
    unsigned int fedid=myfed-fedmin;
    if (mycha<fedchanmin[fedid]||mycha>fedchanmax[fedid]) continue;
    unsigned int channelId=mycha-fedchanmin[fedid];
    cout << "FED="<< fedmin+fedid <<" Channel " << channelId+fedchanmin[fedid]<<" panelType="<<myPanelType<<endl;

    if(myPanelType=="4L") {
      sprintf(title,"FED%d_Channel%d",fedmin+fedid,channelId+fedchanmin[fedid]) ;
      TDirectory * dir = f->mkdir(title);
      TDirectory * dirNC = dir->mkdir("NoisyCells");
      TDirectory * dirEC = dir->mkdir("ErrorCells");
      for (int kk=0;kk<6;++kk) {//loop over the kind of plots, i.e. plotType[]
	dir->cd() ;
	panelType = "4L1x2";
	TCanvas * c4L1x2 = new TCanvas(plotType[kk]+panelType, "4L - 1x2", 500,300) ;
	c4L1x2->Divide(2,1) ;
	c4L1x2->cd(1) ;
	roceff[fedid][channelId][1].draw(kk);
	c4L1x2->cd(2) ;
	roceff[fedid][channelId][0].draw(kk);
	c4L1x2->Write() ;
	      
	panelType = "4L2x3";
	TCanvas * c4L2x3 = new TCanvas(plotType[kk]+panelType, "4L - 2x3", 750,600) ;
	c4L2x3->Divide(3,2) ;
	c4L2x3->cd(1) ;
	roceff[fedid][channelId][7].draw(kk);
	c4L2x3->cd(2) ;
	roceff[fedid][channelId][6].draw(kk);
	c4L2x3->cd(3) ;
	roceff[fedid][channelId][5].draw(kk);
	c4L2x3->cd(4) ;
	roceff[fedid][channelId][2].draw(kk);
	c4L2x3->cd(5) ;
	roceff[fedid][channelId][3].draw(kk);
	c4L2x3->cd(6) ;
	roceff[fedid][channelId][4].draw(kk);
	c4L2x3->Write() ;
	      
	panelType = "4L2x4";
	TCanvas * c4L2x4 = new TCanvas(plotType[kk]+panelType, "4L - 2x4", 1000,600) ;
	c4L2x4->Divide(4,2) ;
	c4L2x4->cd(1) ;
	roceff[fedid][channelId][15].draw(kk);
	c4L2x4->cd(2) ;
	roceff[fedid][channelId][14].draw(kk);
	c4L2x4->cd(3) ;
	roceff[fedid][channelId][13].draw(kk);
	c4L2x4->cd(4) ;
	roceff[fedid][channelId][12].draw(kk);
	c4L2x4->cd(5) ;
	roceff[fedid][channelId][8].draw(kk);
	c4L2x4->cd(6) ;
	roceff[fedid][channelId][9].draw(kk);
	c4L2x4->cd(7) ;
	roceff[fedid][channelId][10].draw(kk);
	c4L2x4->cd(8) ;
	roceff[fedid][channelId][11].draw(kk);
	c4L2x4->Write() ;
	      
	panelType = "4L1x5";
	TCanvas * c4L1x5 = new TCanvas(plotType[kk]+panelType, "4L - 1x5", 1250,300) ;
	c4L1x5->Divide(5,1) ;
	c4L1x5->cd(1) ;
	roceff[fedid][channelId][20].draw(kk);
	c4L1x5->cd(2) ;
	roceff[fedid][channelId][19].draw(kk);
	c4L1x5->cd(3) ;
	roceff[fedid][channelId][18].draw(kk);
	c4L1x5->cd(4) ;
	roceff[fedid][channelId][17].draw(kk);
	c4L1x5->cd(5) ;
	roceff[fedid][channelId][16].draw(kk);
	c4L1x5->Write() ;
      }	
      
      dirNC->cd() ;
      for(unsigned int roc=0; roc<nOfROCS; roc++) {
	if (roc>20) break;
	std::vector<pair<unsigned int, unsigned int> > noisyCells = roceff[fedid][channelId][roc].getNoisyCells();
	for (unsigned int jj=0;jj<noisyCells.size();++jj){
	  unsigned int row = noisyCells[jj].first;
	  unsigned int col = noisyCells[jj].second;
	  TString name = "ROC";
	  name+=(roc+rocmin);
	  name+="_ROW";
	  name+=(row);
	  name+="_COL";
	  name+=(col);
	  TCanvas * cc = new TCanvas(name,"noisyCell" , 250,300) ;
	  roceff[fedid][channelId][roc].draw(row,col);
	  cc->Write();
	}
      }
      dirEC->cd() ;
      for(unsigned int roc=0; roc<nOfROCS; roc++) {
	if (roc>20) break;
	std::vector<pair<unsigned int, unsigned int> > errorCells = roceff[fedid][channelId][roc].getErrorCells();
	for (unsigned int jj=0;jj<errorCells.size();++jj){
	  unsigned int row = errorCells[jj].first;
	  unsigned int col = errorCells[jj].second;
	  TString name = "ROC";
	  name+=(roc+rocmin);
	  name+="_ROW";
	  name+=(row);
	  name+="_COL";
	  name+=(col);
	  TCanvas * cc = new TCanvas(name,"errorCell" , 250,300) ;
	  roceff[fedid][channelId][roc].draw(row,col);
	  cc->Write();
	}
      }
    }
    else if(myPanelType=="3L") {
      sprintf(title,"FED%d_Channel%d",fedmin+fedid,channelId+fedchanmin[fedid]) ;
      TDirectory * dir = f->mkdir(title);
      TDirectory * dirNC = dir->mkdir("NoisyCells");
      TDirectory * dirEC = dir->mkdir("ErrorCells");
      for (int kk=0;kk<6;++kk) {//loop over the kind of plots, i.e. plotType[]
	dir->cd() ;
	panelType = "3L2x3";
	TCanvas * c3L2x3 = new TCanvas(plotType[kk]+panelType, "3L - 2x3", 750,600) ;
	c3L2x3->Divide(3,2) ;
	c3L2x3->cd(1) ;
	roceff[fedid][channelId][5].draw(kk);
	c3L2x3->cd(2) ;
	roceff[fedid][channelId][4].draw(kk);
	c3L2x3->cd(3) ;
	roceff[fedid][channelId][3].draw(kk);
	c3L2x3->cd(4) ;
	roceff[fedid][channelId][0].draw(kk);
	c3L2x3->cd(5) ;
	roceff[fedid][channelId][1].draw(kk);
	c3L2x3->cd(6) ;
	roceff[fedid][channelId][2].draw(kk);
	c3L2x3->Write() ;
	      
	panelType = "3L2x4";
	TCanvas * c3L2x4 = new TCanvas(plotType[kk]+panelType, "3L - 2x4", 1000,600) ;
	c3L2x4->Divide(4,2) ;
	c3L2x4->cd(1) ;
	roceff[fedid][channelId][13].draw(kk);
	c3L2x4->cd(2) ;
	roceff[fedid][channelId][12].draw(kk);
	c3L2x4->cd(3) ;
	roceff[fedid][channelId][11].draw(kk);
	c3L2x4->cd(4) ;
	roceff[fedid][channelId][10].draw(kk);
	c3L2x4->cd(5) ;
	roceff[fedid][channelId][6].draw(kk);
	c3L2x4->cd(6) ;
	roceff[fedid][channelId][7].draw(kk);
	c3L2x4->cd(7) ;
	roceff[fedid][channelId][8].draw(kk);
	c3L2x4->cd(8) ;
	roceff[fedid][channelId][9].draw(kk);
	c3L2x4->Write() ;

	panelType = "3L2x5";
	TCanvas * c3L2x5 = new TCanvas(plotType[kk]+panelType, "3L - 2x5", 1250,600) ;
	c3L2x5->Divide(5,2) ;
	c3L2x5->cd(1) ;
	roceff[fedid][channelId][23].draw(kk);
	c3L2x5->cd(2) ;
	roceff[fedid][channelId][22].draw(kk);
	c3L2x5->cd(3) ;
	roceff[fedid][channelId][21].draw(kk);
	c3L2x5->cd(4) ;
	roceff[fedid][channelId][20].draw(kk);
	c3L2x5->cd(5) ;
	roceff[fedid][channelId][19].draw(kk);
	c3L2x5->cd(6) ;
	roceff[fedid][channelId][14].draw(kk);
	c3L2x5->cd(7) ;
	roceff[fedid][channelId][15].draw(kk);
	c3L2x5->cd(8) ;
	roceff[fedid][channelId][16].draw(kk);
	c3L2x5->cd(9) ;
	roceff[fedid][channelId][17].draw(kk);
	c3L2x5->cd(10) ;
	roceff[fedid][channelId][18].draw(kk);
	c3L2x5->Write() ;
      }
	
      dirNC->cd() ;
      for(unsigned int roc=0; roc<nOfROCS; roc++) {
	std::vector<pair<unsigned int, unsigned int> > noisyCells = roceff[fedid][channelId][roc].getNoisyCells();
	for (unsigned int jj=0;jj<noisyCells.size();++jj){
	  unsigned int row = noisyCells[jj].first;
	  unsigned int col = noisyCells[jj].second;
	  TString name = "ROC";
	  name+=(roc+rocmin);
	  name+="_ROW";
	  name+=(row);
	  name+="_COL";
	  name+=(col);
	  TCanvas * cc = new TCanvas(name,"noisyCell" , 250,300) ;
	  roceff[fedid][channelId][roc].draw(row,col);
	  cc->Write();
	}
      }
      dirEC->cd() ;
      for(unsigned int roc=0; roc<nOfROCS; roc++) {
	std::vector<pair<unsigned int, unsigned int> > errorCells = roceff[fedid][channelId][roc].getErrorCells();
	for (unsigned int jj=0;jj<errorCells.size();++jj){
	  unsigned int row = errorCells[jj].first;
	  unsigned int col = errorCells[jj].second;
	  TString name = "ROC";
	  name+=(roc+rocmin);
	  name+="_ROW";
	  name+=(row);
	  name+="_COL";
	  name+=(col);
	  TCanvas * cc = new TCanvas(name,"errorCell" , 250,300) ;
	  roceff[fedid][channelId][roc].draw(row,col);
	  cc->Write();
	}
      }
    }
    if(myPanelType=="4R") {
      sprintf(title,"FED%d_Channel%d",fedmin+fedid,channelId+fedchanmin[fedid]) ;
      TDirectory * dir = f->mkdir(title);
      TDirectory * dirNC = dir->mkdir("NoisyCells");
      TDirectory * dirEC = dir->mkdir("ErrorCells");
      for (int kk=0;kk<6;++kk) {//loop over the kind of plots, i.e. plotType[]
	dir->cd() ;
	panelType = "4R1x2";
	TCanvas * c4R1x2 = new TCanvas(plotType[kk]+panelType, "4R - 1x2", 500,300) ;
	c4R1x2->Divide(2,1) ;
	c4R1x2->cd(1) ;
	roceff[fedid][channelId][20].draw(kk);
	c4R1x2->cd(2) ;
	roceff[fedid][channelId][19].draw(kk);
	c4R1x2->Write() ;
	  
	panelType = "4R2x3";
	TCanvas * c4R2x3 = new TCanvas(plotType[kk]+panelType, "4R - 2x3", 750,600) ;
	c4R2x3->Divide(3,2) ;
	c4R2x3->cd(1) ;
	roceff[fedid][channelId][15].draw(kk);
	c4R2x3->cd(2) ;
	roceff[fedid][channelId][14].draw(kk);
	c4R2x3->cd(3) ;
	roceff[fedid][channelId][13].draw(kk);
	c4R2x3->cd(4) ;
	roceff[fedid][channelId][16].draw(kk);
	c4R2x3->cd(5) ;
	roceff[fedid][channelId][17].draw(kk);
	c4R2x3->cd(6) ;
	roceff[fedid][channelId][18].draw(kk);
	c4R2x3->Write() ;
	  
	panelType = "4R2x4";
	TCanvas * c4R2x4 = new TCanvas(plotType[kk]+panelType, "4R - 2x4", 1000,600) ;
	c4R2x4->Divide(4,2) ;
	c4R2x4->cd(1) ;
	roceff[fedid][channelId][8].draw(kk);
	c4R2x4->cd(2) ;
	roceff[fedid][channelId][7].draw(kk);
	c4R2x4->cd(3) ;
	roceff[fedid][channelId][6].draw(kk);
	c4R2x4->cd(4) ;
	roceff[fedid][channelId][5].draw(kk);
	c4R2x4->cd(5) ;
	roceff[fedid][channelId][9].draw(kk);
	c4R2x4->cd(6) ;
	roceff[fedid][channelId][10].draw(kk);
	c4R2x4->cd(7) ;
	roceff[fedid][channelId][11].draw(kk);
	c4R2x4->cd(8) ;
	roceff[fedid][channelId][12].draw(kk);
	c4R2x4->Write() ;
	  
	panelType = "4R1x5";
	TCanvas * c4R1x5 = new TCanvas(plotType[kk]+panelType, "4R - 1x5", 1250,300) ;
	c4R1x5->Divide(5,1) ;
	c4R1x5->cd(1) ;
	roceff[fedid][channelId][4].draw(kk);
	c4R1x5->cd(2) ;
	roceff[fedid][channelId][3].draw(kk);
	c4R1x5->cd(3) ;
	roceff[fedid][channelId][2].draw(kk);
	c4R1x5->cd(4) ;
	roceff[fedid][channelId][1].draw(kk);
	c4R1x5->cd(5) ;
	roceff[fedid][channelId][0].draw(kk);
	c4R1x5->Write() ;
      }
      dirNC->cd() ;
      for(unsigned int roc=0; roc<nOfROCS; roc++) {
	if (roc>20) break;
	std::vector<pair<unsigned int, unsigned int> > noisyCells = roceff[fedid][channelId][roc].getNoisyCells();
	for (unsigned int jj=0;jj<noisyCells.size();++jj){
	  unsigned int row = noisyCells[jj].first;
	  unsigned int col = noisyCells[jj].second;
	  TString name = "ROC";
	  name+=(roc+rocmin);
	  name+="_ROW";
	  name+=(row);
	  name+="_COL";
	  name+=(col);
	  TCanvas * cc = new TCanvas(name,"noisyCell" , 250,300) ;
	  roceff[fedid][channelId][roc].draw(row,col);
	  cc->Write();
	}
      }
      dirEC->cd() ;
      for(unsigned int roc=0; roc<nOfROCS; roc++) {
	if (roc>20) break;
	std::vector<pair<unsigned int, unsigned int> > errorCells = roceff[fedid][channelId][roc].getErrorCells();
	for (unsigned int jj=0;jj<errorCells.size();++jj){
	  unsigned int row = errorCells[jj].first;
	  unsigned int col = errorCells[jj].second;
	  TString name = "ROC";
	  name+=(roc+rocmin);
	  name+="_ROW";
	  name+=(row);
	  name+="_COL";
	  name+=(col);
	  TCanvas * cc = new TCanvas(name,"errorCell" , 250,300) ;
	  roceff[fedid][channelId][roc].draw(row,col);
	  cc->Write();
	}
      }
    }
    else if(myPanelType=="3R") {
      sprintf(title,"FED%d_Channel%d",fedmin+fedid,channelId+fedchanmin[fedid]) ;
      TDirectory * dir = f->mkdir(title);
      TDirectory * dirNC = dir->mkdir("NoisyCells");
      TDirectory * dirEC = dir->mkdir("ErrorCells");
      for (int kk=0;kk<6;++kk) {//loop over the kind of plots, i.e. plotType[]
	dir->cd() ;
	panelType = "3R2x3";
	TCanvas * c3R2x3 = new TCanvas(plotType[kk]+panelType, "3R - 2x3"+plotType[kk], 750,600) ;
	c3R2x3->Divide(3,2) ;
	c3R2x3->cd(1) ;
	roceff[fedid][channelId][20].draw(kk);
	c3R2x3->cd(2) ;
	roceff[fedid][channelId][19].draw(kk);
	c3R2x3->cd(3) ;
	roceff[fedid][channelId][18].draw(kk);
	c3R2x3->cd(4) ;
	roceff[fedid][channelId][21].draw(kk);
	c3R2x3->cd(5) ;
	roceff[fedid][channelId][22].draw(kk);
	c3R2x3->cd(6) ;
	roceff[fedid][channelId][23].draw(kk);
	c3R2x3->Write() ;
	  
	panelType = "3R2x4";
	TCanvas * c3R2x4 = new TCanvas(plotType[kk]+panelType, "3R - 2x4"+plotType[kk], 1000,600) ;
	c3R2x4->Divide(4,2) ;
	c3R2x4->cd(1) ;
	roceff[fedid][channelId][13].draw(kk);
	c3R2x4->cd(2) ;
	roceff[fedid][channelId][12].draw(kk);
	c3R2x4->cd(3) ;
	roceff[fedid][channelId][11].draw(kk);
	c3R2x4->cd(4) ;
	roceff[fedid][channelId][10].draw(kk);
	c3R2x4->cd(5) ;
	roceff[fedid][channelId][14].draw(kk);
	c3R2x4->cd(6) ;
	roceff[fedid][channelId][15].draw(kk);
	c3R2x4->cd(7) ;
	roceff[fedid][channelId][16].draw(kk);
	c3R2x4->cd(8) ;
	roceff[fedid][channelId][17].draw(kk);
	c3R2x4->Write() ;
	  
	panelType = "3R2x5";
	TCanvas * c3R2x5 = new TCanvas(plotType[kk]+panelType, "3R - 2x5"+plotType[kk], 1250,600) ;
	c3R2x5->Divide(5,2) ;
	c3R2x5->cd(1) ;
	roceff[fedid][channelId][4].draw(kk);
	c3R2x5->cd(2) ;
	roceff[fedid][channelId][3].draw(kk);
	c3R2x5->cd(3) ;
	roceff[fedid][channelId][2].draw(kk);
	c3R2x5->cd(4) ;
	roceff[fedid][channelId][1].draw(kk);
	c3R2x5->cd(5) ;
	roceff[fedid][channelId][0].draw(kk);
	c3R2x5->cd(6) ;
	roceff[fedid][channelId][5].draw(kk);
	c3R2x5->cd(7) ;
	roceff[fedid][channelId][6].draw(kk);
	c3R2x5->cd(8) ;
	roceff[fedid][channelId][7].draw(kk);
	c3R2x5->cd(9) ;
	roceff[fedid][channelId][8].draw(kk);
	c3R2x5->cd(10) ;
	roceff[fedid][channelId][9].draw(kk);
	c3R2x5->Write() ;
      }
      dirNC->cd() ;
      for(unsigned int roc=0; roc<nOfROCS; roc++) {
	std::vector<pair<unsigned int, unsigned int> > noisyCells = roceff[fedid][channelId][roc].getNoisyCells();
	for (unsigned int jj=0;jj<noisyCells.size();++jj){
	  unsigned int row = noisyCells[jj].first;
	  unsigned int col = noisyCells[jj].second;
	  TString name = "ROC";
	  name+=(roc+rocmin);
	  name+="_ROW";
	  name+=(row);
	  name+="_COL";
	  name+=(col);
	  TCanvas * cc = new TCanvas(name,"noisyCell" , 250,300) ;
	  roceff[fedid][channelId][roc].draw(row,col);
	  cc->Write();
	}
      }
      dirEC->cd() ;
      for(unsigned int roc=0; roc<nOfROCS; roc++) {
	std::vector<pair<unsigned int, unsigned int> > errorCells = roceff[fedid][channelId][roc].getErrorCells();
	for (unsigned int jj=0;jj<errorCells.size();++jj){
	  unsigned int row = errorCells[jj].first;
	  unsigned int col = errorCells[jj].second;
	  TString name = "ROC";
	  name+=(roc+rocmin);
	  name+="_ROW";
	  name+=(row);
	  name+="_COL";
	  name+=(col);
	  TCanvas * cc = new TCanvas(name,"errorCell" , 250,300) ;
	  roceff[fedid][channelId][roc].draw(row,col);
	  cc->Write();
	}
      }
    }
    //summary plots
    for (unsigned int roc=0; roc<nOfROCS; roc++){
      if ((myPanelType=="4R"||myPanelType=="4L")&&roc>20) break;
      if (roceff[fedid][channelId][roc].isValid()){
	//cout << "filling summary plot: fedid=" << fedid << " channelId=" << channelId << " roc=" << roc << endl;
	double meanSig = roceff[fedid][channelId][roc].getMeanNoise();
	double meanThr = roceff[fedid][channelId][roc].getMeanThreshold();
	hMeanNoise->Fill(meanSig);
	hMeanThreshold->Fill(meanThr);
	if (meanSig>3)   out << "FED=" << fedid+fedmin << " channel=" << channelId+fedchanmin[fedid]  
			     << " ROC="<< roc+rocmin <<" Mean Noise=" << meanSig << endl;
	if (meanThr>130) out << "FED=" << fedid+fedmin << " channel=" << channelId+fedchanmin[fedid]  
			     << " ROC="<< roc+rocmin <<" Mean Threshold=" << meanThr << endl;
      }
    }      
  }
  dirSP->cd();
  hMeanNoise->Write();
  hMeanThreshold->Write();
  f->Close() ;
 


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
  
}  // void pixel_scurve()
