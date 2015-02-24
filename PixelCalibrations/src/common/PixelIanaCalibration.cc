// $Id: PixelIanaCalibration.cc,v 1.18 2009/05/27 19:20:32 joshmt Exp $
/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2009, Cornell University		                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd, J. Thompson 					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelIanaCalibration.h"
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"
#include "CalibFormats/SiPixelObjects/interface/PixelLowVoltageMap.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACSettings.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"

#include "TGraphErrors.h"
#include "TF1.h"
#include "TFile.h"
#include "TLine.h"
#include "TCanvas.h"
#include "TAxis.h"
#include "TTree.h"

#include <toolbox/convertstring.h>

using namespace pos;
using namespace std;


PixelIanaCalibration::PixelIanaCalibration(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr)
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr),
    lowVoltageMap_(0),
    npoints_(25),
    sleeptime_(8),
    sleeptime0_(8),
    ianares_(2.)
{
  //  cout << "Greetings from the PixelIanaCalibration copy constructor." <<endl;
}


bool PixelIanaCalibration::execute()
{
  //  cout << "Now starting PixelIanaCalibration::execute()."<<endl;
  
  for (unsigned int iROC=0;iROC<maxROC_;++iROC){

    cout << "iROC:"<<iROC<<endl;

    double percentageOfJob= 100*double(iROC)/double(maxROC_);
    this->setPercentageOfJob(percentageOfJob);

    const unsigned step = 255/npoints_;
    std::vector<unsigned> vanas, ivanas;
    for (unsigned ivana = 0, vana = 0; vana<255; vana += step, ++ivana) {
      ivanas.push_back(ivana);
      vanas.push_back(vana);
    }

    const size_t nvanas = vanas.size();
    assert(nvanas == npoints_ || nvanas == npoints_ + 1);

    for (size_t iii = 0; iii < nvanas; ++iii) {
      const bool first = iii == 0;
      unsigned vana, ivana;
      if (MixVanas_) {
	// this junk code mixes up the vanas so we don't have sequential readings
	// nvanas = 12 (= npoints_ + 1 when 255 % npoints != 0) has
	// 0 23 46 69 92 115 138 161 184 207 230 253 become 0 138 23 161 46 184 69 207 92 230 115 253
	if (nvanas % 2 == 1 && iii == nvanas - 1) {
	  ivana = ivanas[iii];
	  vana = vanas[iii];
	}
	else {
	  const size_t ii = (iii % 2 != 0) * nvanas / 2 + iii / 2;
	  ivana = ivanas[ii];
	  vana = vanas[ii];
	}
      }
      else {
	ivana = ivanas[iii];
	vana = vanas[iii];
      }

      map<string, vector<pos::PixelROCName> >::iterator idpName=dpMap_.begin();

      for (; idpName != dpMap_.end(); ++idpName) {
	cout << "idpName->first:"<<idpName->first<<endl;
	if (idpName->second.size() <= iROC)
	  continue;
	
	PixelROCName aROC=idpName->second[iROC];
	cout << "Selected ROC: " << aROC << endl
	     << "Will set Vana = " << vana << endl;

	setDAC(aROC, pos::k_DACAddress_Vana, vana);
	if (first && TurnOffVsf_)
	  setDAC(aROC, pos::k_DACAddress_Vsf, 0);
      }

      //need to sleep more at p5?
      if (vana==0) ::sleep(sleeptime0_);

      ::sleep(sleeptime_);

      unsigned int Nread=2;

      for (unsigned int i=0;i<Nread;i++){

	idpName=dpMap_.begin();

	for(;idpName!=dpMap_.end();++idpName){

	  if (idpName->second.size()<=iROC) continue;
	
	  PixelROCName aROC=idpName->second[iROC];

	  double iana=0;  unsigned int ntries=0; bool caughtexception=false;
	  cout<<"Selected ROC:" << aROC<<" "<<flush;

	  do {
	    caughtexception=false;
	    try {
	      iana=readIana(idpName->first);
	    } catch  (xdaq::exception::Exception& e) {
	      cout<<endl<<"ERROR reading current; exception caught"<<endl;
	      caughtexception=true;
	      ::sleep(5);
	      ntries++;
	    }
	  }  while (caughtexception && ntries<3);
	  
	  cout<<iana<<endl;
	  
	  Iana_[idpName->first][iROC][ivana].push_back(iana);

	}
      
      }
    }

    // reset to prescribed values

    map<string, vector<pos::PixelROCName> >::iterator idpName=dpMap_.begin();
    for(;idpName!=dpMap_.end();++idpName){

      if (idpName->second.size()<=iROC) continue;

      PixelROCName aROC=idpName->second[iROC];

      cout << "Selected ROC:"<<aROC<<endl;

      //FIXME slow way to make module name
      PixelModuleName theModule(aROC.rocname());

      int oldVana=dacsettings_[theModule]->getDACSettings(aROC)->getVana();
      cout << "Will set Vana="<<oldVana<<endl;
      setDAC(aROC,pos::k_DACAddress_Vana,oldVana);

      if (TurnOffVsf_) {
	int oldVsf=dacsettings_[theModule]->getDACSettings(aROC)->getVsf();
	setDAC(aROC,pos::k_DACAddress_Vsf,oldVsf);
      }
    }
  }

  return false;
}



void PixelIanaCalibration::beginCalibration(){
  //cout << "In PixelIanaCalibration::beginCalibration()" << endl;
  
  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  string sleeptime = tempCalibObject->parameterValue("SleepTime") ;
  if (sleeptime !="") { //default sleeptime_ given in ctor
    int sleeptimeval= atoi( sleeptime.c_str() );
    sleeptime_ = (sleeptimeval<0) ? sleeptime_ : sleeptimeval;
  }
  cout<<"Sleep time set to "<<sleeptime_<<" seconds"<<endl;

  string sleeptime0 = tempCalibObject->parameterValue("SleepTimeAtZero") ;
  if (sleeptime0 !="") { //default given in ctor
    int sleeptimeval0= atoi( sleeptime0.c_str() );
    sleeptime0_ = (sleeptimeval0<0) ? sleeptime0_ : sleeptimeval0;
  }
  cout<<"Sleep time after Vana=0 set to "<<sleeptime0_<<" seconds"<<endl;

  string NPoints = tempCalibObject->parameterValue("NPoints");
  if (NPoints != "") {
    int NPointsval = atoi(NPoints.c_str());
    if (NPointsval > 0)
      npoints_ = NPointsval;
  }
  cout << "NPoints = " << npoints_ << endl;

  string IanaRes = tempCalibObject->parameterValue("IanaRes");
  if (IanaRes != "") {
    int IanaResval = atof(IanaRes.c_str());
    if (IanaResval > 0)
      ianares_ = IanaResval;
  }
  cout << "IanaRes (mA) = " << ianares_ << endl;

  MixVanas_ = tempCalibObject->parameterValue("MixVanas") == "yes";
  cout << "MixVanas? " << MixVanas_ << endl;

  TurnOffVsf_ = tempCalibObject->parameterValue("TurnOffVsf") != "no";
  cout << "TurnOffVsf? " << TurnOffVsf_ << endl;

  PixelConfigInterface::get(lowVoltageMap_, "pixel/lowvoltagemap/", *theGlobalKey_); 
  if (lowVoltageMap_==0){
    cout << "Could not find the LowVoltageMap in the configuration"<< endl;
    assert(0);
  }

  maxROC_=0;
  
  const vector<PixelROCName>& rocs=tempCalibObject->rocList();
  
  for (unsigned int i=0;i<rocs.size();i++){

    //FIXME this is a slow conversion...
    PixelModuleName module(rocs[i].rocname());

    string dpName=lowVoltageMap_->dpNameIana(module);

    cout << "i roc, module, dpname:"<<i<<" "<<rocs[i]<<" "
	 << module << " " << dpName << endl;

    assert(dpName!="");

    dpMap_[dpName].push_back(rocs[i]);

    vector<Moments> v(npoints_+1);
    
    Iana_[dpName].push_back(v);

    if (dpMap_[dpName].size()>maxROC_) maxROC_=dpMap_[dpName].size();

  }

  vector<PixelModuleName>::const_iterator module_name = theDetectorConfiguration_->getModuleList().begin();
  for (;module_name!=theDetectorConfiguration_->getModuleList().end();++module_name){
    // First we need to get the DAC settings for the ROCs on this module.
    PixelDACSettings *dacs=0; 
    string modulePath=module_name->modulename();
    PixelConfigInterface::get(dacs, "pixel/dac/"+modulePath, *theGlobalKey_);
    assert(dacs!=0);
    dacsettings_[*module_name]=dacs;
  }




}


void PixelIanaCalibration::endCalibration(){
  //cout << "In PixelIanaCalibration::endCalibration()" << endl;

  ofstream out((outputDir()+"/iana.dat").c_str()); //leave the file method intact for now
  assert(out.good()); //file method
  
  TFile outputFile( (outputDir()+"/Iana.root").c_str(), "recreate", "Iana.root");
  if (outputFile.IsZombie() ) {
    cout<<"[PixelIanaCalibration::endCalibration] cannot open ROOT output file! Quitting..."<<endl;
    return;
  }
 

  //make a list of all the ROCs
  map<string, vector<pos::PixelROCName> >::iterator idpName1=dpMap_.begin();
  vector<pos::PixelROCName> allrocs;
  for(;idpName1!=dpMap_.end();++idpName1){
    unsigned int vsize=idpName1->second.size();
    for(unsigned i=0;i<vsize;i++){
      PixelROCName theROC=idpName1->second[i];
      allrocs.push_back(theROC);
      //   cout<<theROC.rocname()<<endl;
    }
  }

  branch theBranch;
  branch_sum theBranch_sum;
  TDirectory* dirSummaries = gDirectory->mkdir("SummaryTrees","SummaryTrees");
  dirSummaries->cd();

  TTree* tree = new TTree("PassState","PassState");
  TTree* tree_sum =new TTree("SummaryInfo","SummaryInfo");
  
  tree->Branch("PassState",&theBranch,"pass/F:rocName/C",4096000);
  tree_sum->Branch("SummaryInfo",&theBranch_sum,"deltaVana/F:newVana/F:newIana/F:maxIana/F:fitChisquare/F:rocName/C",4096000);
  outputFile.cd();

  PixelRootDirectoryMaker rootDirs(allrocs,gDirectory);
  //////

  const float fitmin=10; //exclude Vana=0 from the fit

  map<string, vector<pos::PixelROCName> >::iterator idpName=dpMap_.begin();

  for(;idpName!=dpMap_.end();++idpName){
 
    unsigned int vsize=idpName->second.size();

    for(unsigned i=0;i<vsize;i++){

      PixelROCName theROC=idpName->second[i];
      PixelModuleName theModule(theROC.rocname());

      double x[256],y[256];
      double ey[256];

      theBranch.pass=0;
      strcpy(theBranch.rocName,theROC.rocname().c_str());
      strcpy(theBranch_sum.rocName,theROC.rocname().c_str());
      cout << idpName->second[i] << endl;
      out  << idpName->second[i] << endl;
      cout << npoints_ << endl;
      out  << npoints_ << endl;

      for (unsigned j=0;j<npoints_;j++) {
	y[j] = Iana_[idpName->first][i][j].mean();
	x[j] = 255/npoints_*j;
	ey[j] = ianares_/1000.;
      }

      for (unsigned j = 0; j < npoints_; j++) {
	cout << x[j] << " ";
	out  << x[j] << " ";
      }
      cout << endl;
      out << endl;

      for (unsigned j = 0; j < npoints_; j++) {
	cout << y[j] << " ";
	out  << y[j] << " ";
      }
      cout << endl;
      out << endl;

      for (unsigned j = 0; j < npoints_; j++) {
	cout << ey[j] << " ";
	out  << ey[j] << " ";
      }
      cout << endl;
      out << endl;

      TF1* f2 = new TF1("f2","(x<[0])*([2]+([3]-[2])*exp(([4]-[3])*(x-[0])/(([1])*([3]-[2]))))+(x>=[0]+[1])*[4]+(x>=[0])*(x<[0]+[1])*([3]+(x-[0])*([4]-[3])/([1]))",0.0,250.0);
      f2->SetParameters(120,60,y[1],0.5*(y[1]+y[npoints_-2]),y[npoints_-2]);
      f2->SetParLimits(0,10,240);
      f2->SetParLimits(1,0.1,200);

      TGraphErrors* gr = new TGraphErrors(npoints_-1,x,y,0,ey);
      int fitstatus1 = gr->Fit("f2","","",fitmin,250);

      double yvalatzero=f2->Eval(0.0);

      out << yvalatzero <<endl;

      //adjust data so y intercept is at 0, then refit
      for (unsigned int ivana=0;ivana<npoints_;ivana++){
	y[ivana]=1000*(y[ivana]-yvalatzero);
	ey[ivana]=ianares_;
      }

      f2->SetParameters(120,60,y[1],0.5*(y[1]+y[npoints_-2]),y[npoints_-2]);
      f2->SetParLimits(0,10,240);
      f2->SetParLimits(1,0.1,200);
      delete gr;
      
      rootDirs.cdDirectory(theROC);
      TCanvas canvas(theROC.rocname().c_str(),theROC.rocname().c_str(),800,600);

      gr = new TGraphErrors(npoints_-1,x,y,0,ey);
      int fitstatus2 = gr->Fit("f2","","",fitmin,250);

      out << f2->GetParameter(0)<<" ";
      out << f2->GetParameter(1)<<" ";
      out << f2->GetParameter(2)<<" ";
      out << f2->GetParameter(3)<<" ";
      out << f2->GetParameter(4)<<endl;

      TF1* fit = gr->GetFunction("f2");
      theBranch_sum.maxIana=fit->Eval(250);

      //format graph
      gr->SetLineColor(2);
      gr->SetLineWidth(4);
      gr->SetMarkerColor(4);
      gr->SetMarkerStyle(21);
      gr->SetTitle(theROC.rocname().c_str());
      gr->SetMinimum(-10.0);
      gr->SetMaximum(theBranch_sum.maxIana + 10);
      gr->GetXaxis()->SetTitle("Vana");
      gr->GetYaxis()->SetTitle("Iana (mA)");
      gr->Draw("AP");

      fit->Draw("same"); //draw curve
      fit->SetLineColor(1);

      theBranch_sum.fitChisquare = fit->GetChisquare();
      
      //find vana value where iana crosses 25
      theBranch_sum.newVana=0;
      for(; theBranch_sum.newVana<250; theBranch_sum.newVana++){
	theBranch_sum.newIana=fit->Eval(theBranch_sum.newVana);
	if (theBranch_sum.newIana>25.0){
	  break;
	}
      }

      int oldVana=dacsettings_[theModule]->getDACSettings(theROC)->getVana();

      dacsettings_[theModule]->getDACSettings(theROC)->setVana(int(theBranch_sum.newVana));

      cout << "Old Vana="<<oldVana<<endl;
      cout << "New Vana="<<theBranch_sum.newVana<<endl;
      theBranch_sum.deltaVana = theBranch_sum.newVana - oldVana;

      out <<oldVana<<endl;
      out <<theBranch_sum.newVana<<endl;

      double ianacurrent=fit->Eval(oldVana);          
      TLine* l1=new TLine(oldVana,-10.0,oldVana,ianacurrent);
      l1->SetLineColor(1);
      l1->Draw();
      TLine* l2=new TLine(0.0,ianacurrent,oldVana,ianacurrent);
      l2->SetLineColor(1);
      l2->Draw();

      TLine* l3=new TLine(theBranch_sum.newVana,-10.0,theBranch_sum.newVana,theBranch_sum.newIana);
      l3->SetLineColor(2);
      l3->Draw();
      TLine* l4=new TLine(0.0,theBranch_sum.newIana,theBranch_sum.newVana,theBranch_sum.newIana);
      l4->SetLineColor(2);
      l4->Draw();

      if (theBranch_sum.newVana>=249 || theBranch_sum.maxIana<25 || theBranch_sum.newVana <=4
	  || fitstatus1 != 0 || fitstatus2 != 0 ) {
	cout<<"Warning: ROC "<<theROC.rocname()
	    <<" has Vana="<<theBranch_sum.newVana<<",Iana="<<theBranch_sum.newIana<<endl;
      }
      else theBranch.pass=1;

      tree->Fill();
      tree_sum->Fill();

      canvas.Write();
      delete l1;
      delete l2;
      delete l3;
      delete l4;
      delete f2;
      delete gr;

    }

  }

  outputFile.cd();

  outputFile.Write();
  outputFile.Close();

  map<PixelModuleName,PixelDACSettings*>::const_iterator idacs=dacsettings_.begin();

  for(;idacs!=dacsettings_.end();++idacs){
    idacs->second->writeASCII(outputDir());
  }


}

std::vector<std::string> PixelIanaCalibration::calibrated(){

  vector<string> tmp;

  tmp.push_back("dac");

  return tmp;

}

void PixelIanaCalibration::testTiming()
{
  //this code was at the beginning of execute(), but hard coded to never run because nloop is set to 0
  //i will preserve the code here
  map<string, vector<pos::PixelROCName> >::iterator idpName=dpMap_.begin();
  
  PixelROCName aROC=idpName->second[0];
  
  double current[1000];
  
  const unsigned int nloop=0;
  
  for (int loop=0;loop<nloop;loop++){
    
      if (loop%2==0) {
      cout << "Setting Vana=0"<<endl;
      setDAC(aROC,pos::k_DACAddress_Vana,0);
      }
      else {
	cout << "Setting Vana=255"<<endl;
	setDAC(aROC,pos::k_DACAddress_Vana,255);
      }
      
      PixelTimer timer;
      timer.start();
      for (unsigned int i=0;i<1000;i++){
	current[i]=readIana(idpName->first);
	cout << "For count="<<i<<" read current="<<current[i]<<endl;
      }
      timer.stop();
      double ttot=timer.tottime();
      cout << "Time:"<<timer.tottime()<<endl;
      
      for (unsigned int i=0;i<1000;i++){
	cout << "IanaReading " << (ttot*i)/1000.0 << " " <<current[i] << endl;
      }
      
    }
}
