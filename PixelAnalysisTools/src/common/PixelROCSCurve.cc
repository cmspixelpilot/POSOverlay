// author K. Ecklund
#include "PixelAnalysisTools/include/PixelROCSCurve.h"
#include "TF1.h"
#include "TF2.h"
#include "TFile.h"
#include "TMath.h"
#include <iostream>
#include "PixelUtilities/PixelXmlUtilities/include/PixelXmlReader.h"

using namespace std;

PixelROCSCurve::PixelROCSCurve(){
  //  cout << "In PixelROCSCurve"<<endl;
  for(unsigned int row=0;row<80;row++){
    for(unsigned int col=0;col<52;col++){
      scurve[row][col]=0;
      turnon_[row][col]=0;
    }
  }
}

void PixelROCSCurve::init(unsigned int linkid, unsigned int rocid,
			  unsigned int nvcal, unsigned int vcalmin, 
			  unsigned int vcalmax, int ntrig){
  string mthn = "[PixelROCSCurve::init()]\t";
//	cout << mthn << rocid << endl;
  linkid_=linkid;
  rocid_=rocid;
  nvcal_=nvcal;
  vcalmin_=vcalmin;
  vcalmax_=vcalmax;
  ntrig_=ntrig;
  TString name="Ln";
  name+=(linkid_);
  name+="ROC";
  name+=(rocid_);
  TString abrev = name+"threshold";

  //cout << "01 " << abrev << " " << name <<endl;
  threshold1d = new TH1F(abrev,name,255,0.,255.);//255,0.,255
  //cout << "02"<<endl;
  abrev+="s";
  //cout << "03"<<endl;
  threshold = new TH2F(abrev,"scurve:mean",
		       kmaxcol_-kmincol_+1,kmincol_-0.5,kmaxcol_+0.5,
		       kmaxrow_-kminrow_+1,kminrow_-0.5,kmaxrow_+0.5);
  //cout << "04"<<endl;
  abrev=name+"noise";
  //cout << "05"<<endl;
  noise1d = new TH1F(abrev,name,50,0.,10.);//100,0.,20.
  //cout << "06"<<endl;
  abrev+="s";
  //cout << "07"<<endl;
  noise =  new TH2F(abrev,"scurve:sigma",
		    kmaxcol_-kmincol_+1,kmincol_-0.5,kmaxcol_+0.5,
		    kmaxrow_-kminrow_+1,kminrow_-0.5,kmaxrow_+0.5);

  //cout << "08"<<endl;
  abrev=name+"fitprob";
  //cout << "09"<<endl;
  fitprob1d = new TH1F(abrev,name,101,0.,1.01);
  //cout << "10"<<endl;
  abrev+="s";
  //cout << "11"<<endl;
  fitprob =  new TH2F(abrev,"scurve:fitprob",
		      kmaxcol_-kmincol_+1,kmincol_-0.5,kmaxcol_+0.5,
		      kmaxrow_-kminrow_+1,kminrow_-0.5,kmaxrow_+0.5);
  //cout << "12"<<endl;


  PixelXmlReader pixelReader;
  pixelReader.readDocument("PixelSCurveAnalysis.xml");

  string ntry = pixelReader.getXMLAttribute("Ntrials","Trials");
  string startcurve = pixelReader.getXMLAttribute("Startcurve","Value");
  string endcurve = pixelReader.getXMLAttribute("Endcurve","Value");
  string accept = pixelReader.getXMLAttribute("Acceptance","Value");
  string noisy = pixelReader.getXMLAttribute("NoisyMean","Sigma");
  string error = pixelReader.getXMLAttribute("ErrorMean","Sigma");

  ntrials = atoi(ntry.c_str());
  StartCurve = atof(startcurve.c_str());
  EndCurve = atof(endcurve.c_str());
  Accept = atof(accept.c_str());
  AcceptFit = Accept + 1;  
  Noisy = atof(noisy.c_str());
  Error = atof(error.c_str());

}

bool PixelROCSCurve::filled(unsigned int row,unsigned int col){
  return (scurve[row][col]!=0);
}



bool PixelROCSCurve::fit(unsigned int row, unsigned int col) {  


  if(scurve[row][col]==0) return false;  // only fit if filled

  double kevents = (double) ntrig_;
  double effo ;
  
  for ( int i = 1; i < scurve[row][col]->GetNbinsX()+1; i++ ){
    effo = scurve[row][col]->GetBinContent(i);
    if (effo == 0) {
      effo = 0.5/(double)kevents;
    } else  {
      if (effo >= 1) {
	effo = 1. - 0.5/(double)kevents;
      }
    }     
    scurve[row][col]->SetBinError(i, TMath::Sqrt(effo*(1.0-effo)/((double)kevents)));
  }
  
  TF1 fit = TF1("myscurve",&fitfcn,vcalmin_,vcalmax_,2);

  // Set up fit parameters
  fit.SetParNames("mean","sigma","coeff");

  int lowBin  = 1;
  int highBin = 1;
  int nBins   = scurve[row][col]->GetNbinsX();
  double lowrange = (double)vcalmin_;
  double highrange = (double)vcalmax_;  
  
  // Determine the starting point of the threshold curve
  for (int bin=1; bin<=nBins; bin++) {
    if ( scurve[row][col]->GetBinContent(bin) > StartCurve ) {   //XML
      lowBin = bin;
      break;
    } 
  }
  if (lowBin>1)
    lowBin -= 1;

  // Determine the ending point of the threshold curve
  highBin = scurve[row][col]->GetMaximumBin();  
  for (int bin=highBin; bin>=1; bin--) {
    if ( scurve[row][col]->GetBinContent(bin) < EndCurve ) {   //XML
      highBin = bin;
      break;
    } 
  }
  if (highBin<nBins)
    highBin += 1;
  
  // Define the starting values of the fit parameters
  double mean  = (double)(highBin + lowBin) / 2.0;
  double sigma = (double)(highBin - lowBin);

  if (mean-4*sigma > vcalmin_ )
    lowrange = (double)mean - (double)sigma*4.;
 
  if (mean+4*sigma < vcalmax_ )
    highrange = (double)mean + (double)sigma*4.;

  fit.SetRange(lowrange,highrange);

  for (int i=1; i <= ntrials; i++)   //XML
    {
      fit.SetParameters(mean,sigma);
      scurve[row][col]->Fit("myscurve","QR");
      if ( ((fit.GetChisquare()/(double)fit.GetNDF()) < AcceptFit) && ((fit.GetChisquare()/(double)fit.GetNDF()) > exp(-(Accept)) ) )  //XML
	  break;
      else
	sigma = (double)(highBin - lowBin) / (double)(i+1);
    }

    threshold1d->Fill(fit.GetParameter(0));
    threshold->Fill(col,row,fit.GetParameter(0));
    noise1d->Fill(fit.GetParameter(1));
    noise->Fill(col,row,fit.GetParameter(1));
    fitprob1d->Fill(fit.GetProb());
    fitprob->Fill(col,row,fit.GetProb());
    if (fit.GetParameter(1) > Noisy) noisyCells.push_back(std::make_pair<unsigned int, unsigned int>(row,col));  //XML
    if (fit.GetParameter(1) < Error) errorCells.push_back(std::make_pair<unsigned int, unsigned int>(row,col));  //XML

    return true;
}

void PixelROCSCurve::draw(unsigned int row,unsigned int col){
  scurve[row][col]->Draw();
}

void PixelROCSCurve::drawThresholds(){
  threshold->SetStats(false);
  threshold->GetXaxis()->SetTitle("col");
  threshold->GetYaxis()->SetTitle("row");
  threshold->SetMinimum(50.);
  threshold->SetMaximum(150.);
  threshold->Draw("colz");
}
void PixelROCSCurve::drawNoises(){
  noise->SetStats(false);
  noise->GetXaxis()->SetTitle("col");
  noise->GetYaxis()->SetTitle("row");
  noise->SetMinimum(0.);
  noise->SetMaximum(10.);
  noise->Draw("colz");
}
void PixelROCSCurve::drawFitProbs(){
  fitprob->SetStats(false);  
  fitprob->GetXaxis()->SetTitle("col");
  fitprob->GetYaxis()->SetTitle("row");
  fitprob->SetMinimum(0.);
  fitprob->SetMaximum(1.);
  fitprob->Draw("colz");
}

void PixelROCSCurve::drawThreshold(){
  threshold1d->GetXaxis()->SetTitle("Vcal");
  threshold1d->Draw();
}
void PixelROCSCurve::drawNoise(){
  //std::cout << "noise1d=" << noise1d << std::endl;
  noise1d->GetXaxis()->SetTitle("Vcal");
  noise1d->Draw();
}
void PixelROCSCurve::drawFitProb(){
  fitprob1d->Draw();
}

void PixelROCSCurve::fill(unsigned int row,unsigned int col,unsigned int vcal){
  if(row<kminrow_ || row>kmaxrow_ || col<kmincol_ || col>kmaxcol_ ) return;
  TH1F* hist=scurve[row][col];
  if (hist==0) {
    TString name="Channel=";
    name+=(linkid_);
    name=name+" ROC=";
    name+=(rocid_);
    name=name+" row=";
    name+=(row);
    name=name+" col=";
    name+=(col);
    TString cname="Ch";
    cname+=(linkid_);
    cname+="ROC";
    cname+=(rocid_);
    cname+="Row";
    cname+=(row);
    cname+="Col";
    cname+=(col);

    //cout << "PixelROCSCurve creating " << name << " at Vcal=" << vcal << endl;
    hist=scurve[row][col]=new TH1F(cname,name,255,0.0,255.0);

    turnon_[row][col]=vcal;
  }
  hist->Fill(vcal, 1./((double)(ntrig_)));
  //hist->Fill(vcal, 1.);
}

Double_t PixelROCSCurve::fitfcn(Double_t *x, Double_t *par)
{
  Double_t my_erf = 0.5*(1+TMath::Erf((x[0]-par[0])/(par[1]*sqrt(2.))));
// par[0] = mean
// par[1] = sigmam
// par[2] = coef
//   Double_t my_erf = 0.5*par[2]*(1+TMath::Erf((x[0]-par[0])/(par[1]*sqrt(2.))));
  //Double_t my_erf = par[0]+0.5*(par[1]-par[0])*(1+TMath::Erf( (x[0]-par[3])/(sqrt(2.0)*par[2])));
  return my_erf;
// par[0] = minimum
// par[1] = maximum
// par[2] = sigma (noise or slope near threshold)
// par[3] = mean  (threshold)

}

void PixelROCSCurve::write(TFile* file){
  TString name="Link";
  name+=(linkid_);
  name+="ROC";
  name+=(rocid_);
  file->mkdir(name);
  file->cd(name);
  threshold->Write();
  noise->Write();
  fitprob->Write();
  threshold1d->Write();
  noise1d->Write();
  fitprob1d->Write();

  for(unsigned int row=kminrow_; row<kmaxrow_; row++) {
    for(unsigned int col=kmincol_; col<kmaxcol_; col++) {
      if(filled(row,col)) scurve[row][col]->Write();
    }
  }
}
