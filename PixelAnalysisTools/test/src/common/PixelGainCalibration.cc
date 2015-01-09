#include <fstream>
#include "PixelROC.h"
#include "PixelRun/PixelCalib.h"
#include "PixelRun/PixelSLinkData.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TString.h"
#include "TCanvas.h"


int main(){

  std::ifstream in;

  std::cout << "In pixel_gaincalib" << std::endl;

  // read this from xml...
  //in.open("PixelAlive070107d.dmp",ios::binary|ios::in);
  in.open("GainCalibration_070416a.dmp",ios::binary|ios::in);
  assert(in.good());


  PixelCalib calib("calib_070416a.dat");


  int ntrig=calib.nTriggersPerPattern();

  int nvcal=calib.nScanPoints("Vcal" );

  //int vcalmin=calib.vcal_first();
  //int vcalmax=calib.vcal_last();

  pixel::SLinkData pixeldata;

  // what is 24?
  cout << "size:"<<sizeof(pixelROC)*24*24<<endl;


  //TH1F* alive[36][24][80][52];

  pixelROC rocgain[24][24];

  for(unsigned int linkid=0;linkid<24;linkid++){
    for(unsigned int rocid=0;rocid<24;rocid++){
      rocgain[linkid][rocid].init(linkid,rocid,nvcal);
    }
  }

  TH1F* hit_hist=new TH1F("Hits","Hits",100, 0, 1000.0);


  //TH1F* good_hit_hist=new TH1F("Good hits","Good hits",100, 0, 100.0);

  //pixel::Word64 w;

  //assert(w.read(in));
  
  //cout << hex << w.m_value<<dec << endl;
  
  //int nhits=0;

  std::vector<unsigned int> const *rows=0;
  std::vector<unsigned int> const *cols=0;

  int nEvent=0;

  int maxevent=1000000;

  unsigned int vcalvalue=0;

  while(pixeldata.load(in)&&nEvent<maxevent){

    //cout << "Number of hits on event:"<<pixeldata.getNHits() <<endl;
    
    if (nEvent%1000==0) cout << "nEvent:"<<nEvent<<endl;
   
    if (nEvent%(ntrig*nvcal)==0) {
      
      calib.getRowsAndCols(nEvent/ntrig,rows,cols);

      vcalvalue= calib.scanValue("Vcal",nEvent/(ntrig*calib.getNumberOfFeds()));

      cout << "vcalvalue:"<<vcalvalue<<endl;


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
 
    std::vector<pixel::SLinkHit> hits=pixeldata.getHits();
    std::vector<pixel::SLinkHit>::iterator ihit=hits.begin();

    hit_hist->Fill(hits.size());

    for (;ihit!=hits.end();++ihit) {

      unsigned int linkid=ihit->get_link_id();
      unsigned int rocid=ihit->get_roc_id();
      unsigned int row=ihit->get_row();
      unsigned int col=ihit->get_col();
      unsigned int adc=ihit->get_adc();
      if (row>=80) { cout << "row="<<row<<endl; continue;}
      if (col>=52) { cout << "col="<<col<<endl; continue;}

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
      if (valid_row&&valid_col){
	assert(linkid>=1&&linkid-1<36);
	assert(rocid>=1&&rocid-1<24);
	//TH1F* hist=alive[linkid-1][rocid-1][row][col];
	rocgain[linkid-1][rocid-1].fill(row,col,vcalvalue,adc);
        /*	
	TH1F* hist=0;
	if (hist==0){
	  TString name="Channel=";
	  name+=(linkid);
	  name=name+" ROC=";
	  name+=(rocid);
	  //alive[linkid-1][rocid-1][row][col]=new TH1F(name,name,nvcal,0.0,255.0);
	  //alive[linkid-1][rocid-1][row][col]->SetMinimum(0.0);
	  //alive[linkid-1][rocid-1][row][col]->SetMaximum(ntrig);
	  //hist=alive[linkid-1][rocid-1][row][col];
	  
	}
	//hist->Fill(vcalvalue);
	hist=rocgain[linkid-1][rocid-1];	
	if (hist==0){
	  TString name="Channel=";
	  name+=(linkid);
	  name=name+" ROC=";
	  name+=(rocid);
	  rocgain[linkid-1][rocid-1]=new TH1F(name,name,nvcal,0.0,255.0);
	  rocgain[linkid-1][rocid-1]->SetMinimum(0.0);
	  rocgain[linkid-1][rocid-1]->SetMaximum(ntrig*4160);
	  hist=rocgain[linkid-1][rocid-1];	  
	}
	hist->Fill(vcalvalue,adc/256.0);
	*/
      }
      else{
	cout << "Not valid row or column:"<<linkid<<" "<<rocid<<" "
	     << row<<" "<<col<<endl;
      
      }

 

    }



    nEvent++;

 
  }

  cout << "Processed:"<<nEvent<<" triggers"<<endl;


  int plotnumber=0;
  
  TCanvas* c=0;

  bool plotted=false;

  for (int channel=0;channel<24;channel++){
    for (int roc=0;roc<24;roc++){
      
      if (plotnumber%12==0){
	TString name="PixelAlive";
	name=name+TString(plotnumber/12+1);
	c=new TCanvas(name,name, 700,800);
	c->Divide(3 ,4);
      }

      c->cd(plotnumber%12+1);

      if (rocgain[channel][roc].filled(0,0)){
	rocgain[channel][roc].draw(0,0);
	plotted=true;
      }

      plotnumber++;

      if (plotted&&(plotnumber%12==0)){
	TString name="Pixel_Gain";
	name=name+"_Channel";
	name+=(channel+1);
	name+="_";
        name+=(roc/12+1);
	name+=".ps";
	c->Print(name);
	plotted=false;
      }
    }
  }

  return 0;

}
