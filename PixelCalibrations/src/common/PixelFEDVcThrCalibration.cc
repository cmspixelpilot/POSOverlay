// $Id: PixelFEDVcThrCalibration.cc,v 1.17 2012/01/20 19:14:47 kreis Exp $
/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2009 Cornell University		                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd, Y. Weng, D. Kotlinski                                *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelFEDVcThrCalibration.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO1Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO2Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO3Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/ErrorFIFODecoder.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"

#include <toolbox/convertstring.h>

#include "iomanip"
#include "TH2F.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TLine.h"
#include "TStyle.h"
#include "TTree.h"

using namespace pos;
using namespace std;

PixelFEDVcThrCalibration::PixelFEDVcThrCalibration(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{

}

xoap::MessageReference PixelFEDVcThrCalibration::execute(xoap::MessageReference msg)
{

  map <unsigned int, set<unsigned int> >::iterator i_fedsAndChannels;

  unsigned int state=event_/tempCalibObject_->nTriggersPerPattern();

  string name1=tempCalibObject_->scanName(0);
  string name2=tempCalibObject_->scanName(1);

  unsigned int iwbc=tempCalibObject_->scanCounter(name1,state);  // WBC
  unsigned int ithr=tempCalibObject_->scanCounter(name2,state); // VcThr
  
  try {

    for (i_fedsAndChannels=fedsAndChannels_.begin();i_fedsAndChannels!=fedsAndChannels_.end();++i_fedsAndChannels){

      unsigned int fednumber=i_fedsAndChannels->first;
      unsigned int fedcrate=theFEDConfiguration_->crateFromFEDNumber(fednumber);
      if (fedcrate!=crate_) continue;

      unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
 


      PixelFEDInterface* iFED=FEDInterface_[vmeBaseAddress];

      uint64_t buffer64[fifo3Depth];
      int status=iFED->spySlink64(buffer64);

      if (status>0) {
	if(0){
	  cout<<"Contents of Spy FIFO 3"<<endl;
	  cout<<"----------------------"<<endl;
	  for (int i=0; i<=status;++i) {
	    cout<<"Clock "<<i<<" = 0x"<<hex<<buffer64[i]<<dec<<endl;
	  }
	}
	
	FIFO3Decoder decode(buffer64);

	unsigned int nhits=decode.nhits();

	for (unsigned int ihit=0;ihit<nhits;ihit++){
	  unsigned int rocid=decode.rocid(ihit);
	  assert(rocid>0);
	  unsigned int channel=decode.channel(ihit);

	  PixelROCName roc;

	  if (theNameTranslation_->ROCNameFromFEDChannelROCExists(fednumber, channel, rocid-1))
	    roc=theNameTranslation_->ROCNameFromFEDChannelROC(fednumber,
							      channel,
							      rocid-1);
	  else
	    cout << "ROC with fednumber="<<fednumber << " channel="<<channel<<" rocid="<<rocid<< " does not exist in translation!" << endl;

	  map <PixelROCName, PixelEfficiency2DVcThr>::iterator it=eff_.find(roc);

	  if (it!=eff_.end()) {
	    it->second.add(ithr,iwbc);
	  }
	  else{
	    cout << "Could not find ROC with fednumber="<<fednumber
		      << " channel="<<channel<<" rocid="<<rocid<<endl;
	  }
	}

      }
      else{
	cout << "Error reading spySlink64 status="<<status<<endl;
      }
    }
  } catch (HAL::HardwareAccessException& e) {
    diagService_->reportError("Exception occurred :",DIAGTRACE);
    string mes = e.what();
    diagService_->reportError(mes,DIAGINFO);
  } catch (exception e) {
    diagService_->reportError("*** Unknown exception occurred",DIAGWARN);
  }
  
  event_++;

  xoap::MessageReference reply = MakeSOAPMessageReference("ThresholdVcThrDone");
  return reply;
}

void PixelFEDVcThrCalibration::initializeFED(){

  setFEDModeAndControlRegister(0x8,0x30010);

}

xoap::MessageReference PixelFEDVcThrCalibration::beginCalibration(xoap::MessageReference msg){
  
  tempCalibObject_=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject_!=0);

  fedsAndChannels_=tempCalibObject_->getFEDsAndChannels(theNameTranslation_);
  d2_=false;

  unsigned int nScanVars=tempCalibObject_->numberOfScanVariables();

  assert(nScanVars>1);

  string name1=tempCalibObject_->scanName(0);
  string name2=tempCalibObject_->scanName(1);

  vector<PixelROCName> aROC_string;

  aROC_string=tempCalibObject_->rocList();
  assert(aROC_string.size()>0);

  unsigned int nVcThr=tempCalibObject_->nScanPoints(name2);
  unsigned int nWBC=tempCalibObject_->nScanPoints(name1);
  
  cout<<"nWBC: "<<nWBC<<endl;
  //assert(nWBC==1);
  if(nWBC>1) d2_= true;
    
  double VcThrMin=tempCalibObject_->scanValueMin(name2);
  double VcThrMax=tempCalibObject_->scanValueMax(name2);
  double VcThrStep=tempCalibObject_->scanValueStep(name2);
    
  double WBCMin=tempCalibObject_->scanValueMin(name1);
  double WBCMax=tempCalibObject_->scanValueMax(name1);
  double WBCStep=tempCalibObject_->scanValueStep(name1);
    
      
  for (unsigned int i_aROC=0;i_aROC<aROC_string.size();++i_aROC) {
      
    PixelModuleName module(aROC_string[i_aROC].rocname());
    if (crate_!= theFEDConfiguration_->crateFromFEDNumber(theNameTranslation_->firstHdwAddress(module).fednumber()) ) continue;
      
    PixelEfficiency2DVcThr tmp(aROC_string[i_aROC].rocname(),name2,nVcThr,
			       VcThrMin-0.5*VcThrStep,
			       VcThrMax+0.5*VcThrStep,
			       name1,nWBC,
			       WBCMin-0.5*WBCStep,
			       WBCMax+0.5*WBCStep);
      eff_[aROC_string[i_aROC]]=tmp;
    } 
    
  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDVcThrCalibration::endCalibration(xoap::MessageReference msg){

    //First we need to get the DAC settings for the ROCs
  vector<PixelModuleName> modules=theDetectorConfiguration_->getModuleList();
  vector<PixelModuleName>::iterator module_name = modules.begin();

  map<PixelModuleName,PixelDACSettings*> theDACs;

  for (;module_name!=modules.end();++module_name){

    if (crate_!= theFEDConfiguration_->crateFromFEDNumber(theNameTranslation_->firstHdwAddress(*module_name).fednumber())  ) continue;
    
    PixelDACSettings *tempDACs=0;

    string modulePath=(module_name->modulename());

    PixelConfigInterface::get(tempDACs, "pixel/dac/"+modulePath, *theGlobalKey_);
    assert(tempDACs!=0);
    theDACs[*module_name]=tempDACs;

  }

  double min1=0.0;
  double max1=0.0;
  double min2=0.0;
  double max2=0.0;
  unsigned int nbins1=0;
  unsigned int nbins2=0;  
  double numerator=0.0;

  vector<string> rocsname;
  vector<int> VcThr;

  map<PixelROCName, PixelEfficiency2DVcThr>::iterator it=eff_.begin();

  for(map<PixelROCName, PixelEfficiency2DVcThr>::iterator it=eff_.begin(), itend=eff_.end(); it!=itend ;++it){
    rocsname.push_back(it->first.rocname());
  }

  //open the output root file
  outputFile_=new TFile( (outputDir()+"/VcThr_"+itoa(crate_)+".root").c_str(), "recreate", 
			   ("VcThr_"+itoa(crate_)+".root").c_str() ); 

  branch theBranch;
  branch_sum theBranch_sum;
  TDirectory* dirSummaries = gDirectory->mkdir("SummaryTrees","SummaryTrees");
  dirSummaries->cd();

  TTree* tree = new TTree("PassState","PassState");
  TTree* tree_sum =new TTree("SummaryInfo","SummaryInfo");
  
  tree->Branch("PassState",&theBranch,"pass/F:rocName/C",4096000);
  tree_sum->Branch("SummaryInfo",&theBranch_sum,"new_VcThr/F:delta_VcThr/F:rocName/C",4096000);
  outputFile_->cd();

  PixelRootDirectoryMaker rootDirs(rocsname, gDirectory);

  for(;it!=eff_.end();++it){

    string rocsname=it->first.rocname();
    string name=it->first.rocname();
    cout << name << endl;
    rootDirs.cdDirectory(rocsname);

    theBranch.pass=0;
    strcpy(theBranch.rocName,rocsname.c_str());
    strcpy(theBranch_sum.rocName,rocsname.c_str());

      //FIXME slow way to make a module ame
    PixelModuleName module(it->first.rocname());

    if(d2_) {
      it->second.findThreshold2(tempCalibObject_->nTriggersPerPattern()*
				  tempCalibObject_->nPixelPatterns());
    } else {

      it->second.findSettings(tempCalibObject_->nTriggersPerPattern()*
				tempCalibObject_->nPixelPatterns());
    }
    cout<< it->second.validVcThr() << " "<< it->second.vcThr() <<" "<<d2_<< endl;

    int vcThr;

    if (it->second.validVcThr()) {

      vcThr=(int)it->second.vcThr();

      theBranch.pass=1;
      theBranch_sum.new_VcThr=(float)vcThr;

      int old_vcThr = theDACs[module]->getDACSettings(it->first)->getVcThr();
	
      theBranch_sum.delta_VcThr=(float)(old_vcThr-vcThr);

      tree->Fill();
      tree_sum->Fill();
      
      cout<<" vcThr = "<<vcThr<<" "<<it->second.vcThr()<<" "<<int(it->second.vcThr())<<endl;
      theDACs[module]->getDACSettings(it->first)->setVcThr(vcThr);

      VcThr.push_back(vcThr);

    }

    tree->Fill();

    min1=it->second.getmin1();
    max1=it->second.getmax1();
    min2=it->second.getmin2();
    max2=it->second.getmax2();
    nbins1=it->second.getnbins1();
    nbins2=it->second.getnbins2();
    numerator=tempCalibObject_->nTriggersPerPattern()*tempCalibObject_->nPixelPatterns();

    TCanvas* canvas = new TCanvas(name.c_str(),name.c_str(),800,600);
      
    TLine* determinedVcThr;
      
    TH2F histo2D=it->second.FillEfficiency(numerator);
      
    histo2D.GetXaxis()->SetTitle("VcThr");
    histo2D.GetYaxis()->SetTitle("WBC"); 
      
    histo2D.Draw("colz");
    
    if(it->second.validVcThr()){
	
      determinedVcThr = new TLine(vcThr,min2,vcThr,max2);
      determinedVcThr->SetLineColor(kMagenta); determinedVcThr->SetLineStyle(kDashed);
      determinedVcThr->Draw("same");

    }

    canvas->Write();
   
  }

  outputFile_->cd();

  TCanvas* canvas1=new TCanvas("summary","summary",800,600);

  TH1F* histo_VcThr=new TH1F("summary","summary",256,0,255);
    
  for(vector<int>::iterator it=VcThr.begin(),itend=VcThr.end();it!=itend;++it){
      
    histo_VcThr->Fill(*(it));
      
  }
  

  histo_VcThr->Draw();
  histo_VcThr->GetXaxis()->SetTitle("VcThr");
  canvas1->Write();
  
  outputFile_->Write();
  outputFile_->Close();


  map<PixelModuleName,PixelDACSettings*>::iterator dacs=theDACs.begin();

  for(;dacs!=theDACs.end();++dacs){
    dacs->second->writeASCII(outputDir());
  }



  cout << "In PixelFEDVcThrCalibration::endCalibration()" << endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

