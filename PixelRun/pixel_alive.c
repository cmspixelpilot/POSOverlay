#include <fstream>
#include "PixelCalib.h"
#include "PixelSLinkData.h"
#include "TH1F.h"
#include "TFile.h"
#include "TH2F.h"
#include "TString.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TStyle.h"

void pixel_alive(){
  /*   gROOT->SetBatch() ; */
  /*   gROOT->SetStyle("Plain") ; */
  /*   TStyle *t  = gROOT->GetStyle("Plain") ; */
  /*   t->SetStatStyle(0) ; */
  /*   gROOT->SetStyle(t) ; */

  //Set the fed/channel ids you want to analyze
  unsigned int fedmin = 33;
  unsigned int fedmax = 34;
  unsigned int fedchanmin[] = {13,1} ; //{fedchanmin(fedmin),...,fedchanmin(fedmax)}
  unsigned int fedchanmax[] = {36,24}; //{fedchanmax(fedmin),...,fedchanmax(fedmax)}

  //set the rocs you want to analyze
  unsigned int rocmin;
  unsigned int rocmax;
  rocmin=1;
  rocmax=24;

  //open the data file (please edit it with the correct name)
  std::ifstream in;
  in.open("PixelAlive_355.dmp",ios::binary|ios::in);

  //instance the PixelCalib class: it gets all info needed from calib.dat and translation.dat files (please edit them with the correct name)
  /*   string path = getenv("PIXELCONFIGURATIONBASE"); */
  /*   PixelCalib calib(path+"calib/3/calib.dat"); */
  PixelCalib calib("calib_pixel_alive.dat","translation.dat");

  //get the list of panel types associated to the disk, blade, panel numbers and the fed and the channel (according to translation.dat)
  std::vector<std::pair<vector<int>,std::string> > panelTypes;
  panelTypes = calib.getPanelTypesAndFEDInfo() ;

  assert(in.good());

  uint64_t run_num;
  in.read((char*)&run_num,8);
  cout << "Run number=" << run_num<<endl;

  int ntrig=calib.nTriggersPerPattern();
  int nvcal=calib.nScanPoints("Vcal");

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

  //create the matrices of hitograms
  TH2F* alive[nOfFed][maxNOfChan][nOfROCS];
  TH2F* alive_mirror[nOfFed][maxNOfChan][nOfROCS];

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
	alive[fedid][channel][roc]->SetMaximum(ntrig*nvcal);
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
	alive_mirror[fedid][channel][roc]->SetMaximum(ntrig*nvcal);
	alive_mirror[fedid][channel][roc]->SetStats(0);
      }
    }
  }
  TH1F* hit_hist=new TH1F("Hits","Hits",100, 0, 1000.0);

  std::vector<unsigned int> const *rows=0;
  std::vector<unsigned int> const *cols=0;

  int nEvent=0;

  //set the maximum naumber of events to be processed
  int maxevent=1000000;
  //maxevent=20;

  //event loop: read data
  while(pixeldata.load(in)&&nEvent<maxevent) {

    //cout << "Number of hits on event:"<<pixeldata.getNHits() <<endl;
    if (nEvent%10000==0) cout << "nEvent:"<<nEvent<<endl;

    if (nEvent%(ntrig*nvcal*calib.getNumberOfFeds())==0) {
      //new pattern: get the rows and columns of this pattern
      calib.getRowsAndCols(nEvent/(ntrig*calib.getNumberOfFeds()),rows,cols);
      //cout << "ntrig*nvcal=" <<ntrig*nvcal<<endl;      

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
    int fed = pixeldata.getHeader().getSource_id();
    if ( fed<((int)fedmin) || fed>((int)fedmax) ) {nEvent++;continue;}
    int fedn = fed-fedmin;
 
    std::vector<pixel::SLinkHit> hits=pixeldata.getHits();
    std::vector<pixel::SLinkHit>::iterator ihit=hits.begin();

    hit_hist->Fill(hits.size());

    //loop over the hits in the event
    for (;ihit!=hits.end();++ihit) {

      //get channel, roc, row, col
      unsigned int linkid=ihit->get_link_id();
      unsigned int rocid=ihit->get_roc_id();
      unsigned int row=ihit->get_row();
      unsigned int col=ihit->get_col();
      /*       cout << "fed=" << fed << " linkid=" << linkid */
      /* 	   << " rocid=" << rocid */
      /* 	   << " row=" << row */
      /* 	   << " col=" << col << endl; */

      //continue if not in [fedchanmin,fedchanmax] and [rocmin,rocmax]
      if (linkid<fedchanmin[fedn]||linkid>fedchanmax[fedn]){continue;}
      if (rocid<rocmin||rocid>rocmax){continue;}
      if (rocid>24) {continue;}
      if (rocid<1)  {continue;}
      if (row>=80)  {continue;}
      if (col>=52)  {continue;}

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
      
      //cout << "rocid, col, row, adc:"
      //   <<pixeldata->getHit(ihit).get_roc_id()<<" "
      //   <<pixeldata->getHit(ihit).get_col()<<" "
      //   <<pixeldata->getHit(ihit).get_row()<<" "
      //   <<pixeldata->getHit(ihit).get_adc()<<endl;
      //cout << "valid row,col=" << valid_row <<","<<valid_col<<endl;

      //fill histograms
      if (valid_row&&valid_col){
	assert(linkid>=1&&linkid<37);
	assert(rocid>=1&&rocid<25);
	//cout << "will fill F="<<fedn << " C=" << linkid-fedchanmin[fedn]<<" R="<<rocid-rocmin<<" c="<<col
	//<<" r="<<row<<" entries="<<alive[fedn][linkid-fedchanmin[fedn]][rocid-rocmin]->GetEntries() << endl;
	alive[fedn][linkid-fedchanmin[fedn]][rocid-rocmin]->Fill(col,row);
	alive_mirror[fedn][linkid-fedchanmin[fedn]][rocid-rocmin]->Fill(51-col,79-row);
      }
      else{
	cout << "Not valid row or column:"<<linkid<<" "<<rocid<<" "
	     << row<<" "<<col<<endl;
      }
    }
    nEvent++;
  }
  cout << "Processed:"<<nEvent<<" triggers"<<endl;

  //create output file
  cout << "Making FULL plots" << endl;
  char outFileName[20];
  if (fedmin==fedmax) sprintf(outFileName,"FED%d_Ch%d-%d_PixAlive.root",fedmin,fedchanmin[0],fedchanmax[0]) ;
  else sprintf(outFileName,"FED%d-%d_PixAlive.root",fedmin,fedmax) ;
  TFile * f = new TFile(outFileName,"RECREATE") ;
  
  char title[20] ;
  int disk = 0;
  int blade = 0;
  int panel = 0;
  unsigned int myfed = 0;
  unsigned int mycha = 0;
  TString panelType = "";
  TString myPanelType = "";

  //for every panel (=every channel) create a directory and fill it with canvases
  //(one canvas per plaquette)
  for (unsigned int it=0;it<panelTypes.size();it++) {    
    disk  = panelTypes[it].first[0];
    blade = panelTypes[it].first[1];
    panel = panelTypes[it].first[2];
    myfed = panelTypes[it].first[3];
    mycha = panelTypes[it].first[4];
    myPanelType = panelTypes[it].second;

    myfed+=32;
  
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
}
