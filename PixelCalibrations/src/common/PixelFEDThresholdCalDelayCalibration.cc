// $Id: PixelThresholdCalDelayCalibration.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2007-2008, Cornell U.			                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd                					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelFEDThresholdCalDelayCalibration.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include "PixelCalibrations/include/PixelHistoReadWriteFile.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO1Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO2Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO3Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/ErrorFIFODecoder.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"

#include <toolbox/convertstring.h>

#include "iomanip"
#include "TCanvas.h"
#include "TH2F.h"
#include "TStyle.h"
#include "TLine.h"
#include "TH1F.h"
#include "TTree.h"
#include "TDirectory.h"
#include "TStyle.h"

using namespace pos;
using namespace std;

PixelFEDThresholdCalDelayCalibration::PixelFEDThresholdCalDelayCalibration(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  
}

xoap::MessageReference PixelFEDThresholdCalDelayCalibration::execute(xoap::MessageReference msg)
{

  std::map <unsigned int, std::set<unsigned int> >::iterator i_fedsAndChannels;

  unsigned int state=event_/tempCalibObject_->nTriggersPerPattern();

  unsigned int ithreshold=tempCalibObject_->scanCounter(name1_,state);
  unsigned int icaldelay=tempCalibObject_->scanCounter(name2_,state);

  try {

    for (i_fedsAndChannels=fedsAndChannels_.begin();i_fedsAndChannels!=fedsAndChannels_.end();++i_fedsAndChannels){
      
      unsigned int fednumber=i_fedsAndChannels->first;
      unsigned int fedcrate=theFEDConfiguration_->crateFromFEDNumber(fednumber);
      
      if (fedcrate!=crate_) {
	continue;
      }

      unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
 
      PixelFEDInterface* iFED=FEDInterface_[vmeBaseAddress];

      uint64_t buffer64[fifo3Depth];
      int status=iFED->spySlink64(buffer64);
      //printf(" status is %i\n", status);
      if (status>0) {
	if(false){
	  std::cout<<"Contents of Spy FIFO 3"<<std::endl;
	  std::cout<<"----------------------"<<std::endl;
	  for (int i=0; i<=status;++i) {
	    std::cout<<"Clock "<<i<<" = 0x"<<std::hex<<buffer64[i]<<std::dec<<std::endl;
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

	  map <PixelROCName, PixelEfficiency2DVcThrCalDel>::iterator it=eff_.find(roc);

	  if (it!=eff_.end()) {

	    it->second.add(icaldelay,ithreshold);
	    
	  }

	  else{
	    cout << "Could not find ROC " << roc.rocname() << " with fednumber="<<fednumber
		 << " channel="<<channel<<" rocid="<<rocid<< " in our map" <<endl;
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
 
  xoap::MessageReference reply = MakeSOAPMessageReference("ThresholdCalDelayDone");
  return reply;
}

void PixelFEDThresholdCalDelayCalibration::initializeFED(){

  setFEDModeAndControlRegister(0x8,0x30010);

}

xoap::MessageReference PixelFEDThresholdCalDelayCalibration::beginCalibration(xoap::MessageReference msg){

  tempCalibObject_=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject_!=0);

  fedsAndChannels_=tempCalibObject_->getFEDsAndChannels(theNameTranslation_);

  name1_=tempCalibObject_->scanName(0);
  name2_=tempCalibObject_->scanName(1);

  unsigned int nScanVars=tempCalibObject_->numberOfScanVariables();
  assert(nScanVars>1);

  std::vector<PixelROCName> aROC_string;

  aROC_string=tempCalibObject_->rocList();

  assert(aROC_string.size()>0);

  cout << "Will make dirs:"<<aROC_string.size()<<endl;
  
  unsigned int nThr=tempCalibObject_->nScanPoints(name1_);
  unsigned int nCal=tempCalibObject_->nScanPoints(name2_);
  
  double VcThrMin=tempCalibObject_->scanValueMin(name1_);
  double VcThrMax=tempCalibObject_->scanValueMax(name1_);
  double VcThrStep=tempCalibObject_->scanValueStep(name1_);
  
  double CalDelMin=tempCalibObject_->scanValueMin(name2_);
  double CalDelMax=tempCalibObject_->scanValueMax(name2_);
  double CalDelStep=tempCalibObject_->scanValueStep(name2_);
        
  for (unsigned int i_aROC=0;i_aROC<aROC_string.size();++i_aROC) {

    PixelModuleName module(aROC_string[i_aROC].rocname());
    if (crate_ != theFEDConfiguration_->crateFromFEDNumber(theNameTranslation_->firstHdwAddress(module).fednumber())  ) continue;
    
    PixelEfficiency2DVcThrCalDel tmp(aROC_string[i_aROC].rocname(),name2_,nCal,
				     CalDelMin-0.5*CalDelStep,
				     CalDelMax+0.5*CalDelStep,
				     name1_,nThr,
				     VcThrMin-0.5*VcThrStep,
				     VcThrMax+0.5*VcThrStep);

    std::cout << "Booking PixelEfficiency2DVcThrCalDel #" << i_aROC << " with name " << aROC_string[i_aROC].rocname() << std::endl;
    eff_[aROC_string[i_aROC]]=tmp;

  }

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDThresholdCalDelayCalibration::endCalibration(xoap::MessageReference msg){

  //First we need to get the DAC settings for the ROCs

  std::vector<PixelModuleName> modules=theDetectorConfiguration_->getModuleList();
  std::vector<PixelModuleName>::iterator module_name = modules.begin();

  std::map<PixelModuleName,PixelDACSettings*> theDACs;

  for (;module_name!=modules.end();++module_name){

    if (crate_!= theFEDConfiguration_->crateFromFEDNumber(theNameTranslation_->firstHdwAddress(*module_name).fednumber())  ) {
      continue;
    }

    PixelDACSettings *tempDACs=0;

    std::string modulePath=(module_name->modulename());

    //std::cout << "Reading DACs for module:"<<modulePath<<std::endl;

    PixelConfigInterface::get(tempDACs, "pixel/dac/"+modulePath, *theGlobalKey_);
    assert(tempDACs!=0);
    theDACs[*module_name]=tempDACs;

  }
  

  std::vector<std::string> rocsname;

  std::map<PixelROCName, PixelEfficiency2DVcThrCalDel>::iterator it=eff_.begin();

  for(std::map<PixelROCName, PixelEfficiency2DVcThrCalDel>::iterator it=eff_.begin(), itend=eff_.end(); it!=itend ;++it){
    rocsname.push_back(it->first.rocname());
  }

  //open the output root file
  outputFile_=new TFile((outputDir()+"/VcThrCalDel_"+itoa(crate_)+".root").c_str(), "recreate", "VcThrCalDel.root"); 

  branch theBranch;
  branch_sum theBranch_sum;
  TDirectory* dirSummaries = gDirectory->mkdir("SummaryTrees","SummaryTrees");
  dirSummaries->cd();

  TTree* tree = new TTree("PassState","PassState");
  TTree* tree_sum =new TTree("SummaryInfo","SummaryInfo");
    
  tree->Branch("PassState",&theBranch,"pass/F:rocName/C",4096000);
  tree_sum->Branch("SummaryInfo",&theBranch_sum,"new_CalDel/F:delta_CalDel/F:new_VcThr/F:delta_VcThr/F:rocName/C",4096000);
  outputFile_->cd();
    
  PixelRootDirectoryMaker rootDirs(rocsname,outputFile_);
  double min1=0.0;
  double max1=0.0;
  double min2=0.0;
  double max2=0.0;
  unsigned int nbins1=0;
  unsigned int nbins2=0;
  double numerator=0.0;
  bool valid;

  for(;it!=eff_.end();++it){

    string rocName=it->first.rocname();

    PixelModuleName moduleName(rocName);
    if (crate_!= theFEDConfiguration_->crateFromFEDNumber(theNameTranslation_->firstHdwAddress(moduleName).fednumber())  ) {
      continue;
    }
    
    theBranch.pass=0;
    strcpy(theBranch.rocName,rocName.c_str());
    strcpy(theBranch_sum.rocName,rocName.c_str());
      
    rootDirs.cdDirectory(rocName);

    
    it->second.findSettings(tempCalibObject_->nTriggersPerPattern()*
			    tempCalibObject_->nPixelPatterns());

    
    map<PixelModuleName,PixelDACSettings*>::iterator dacs=theDACs.find(moduleName);
      
    assert(dacs!=theDACs.end());
      
    PixelROCDACSettings *rocDACs=dacs->second->getDACSettings(it->first);
      
    assert(rocDACs!=0);
      
    unsigned int oldVcThr=rocDACs->getVcThr();
    unsigned int oldCalDel=rocDACs->getCalDel();
      
    cout << "Old settings: VcThr="<<oldVcThr<<" CalDel=" << oldCalDel << endl;
      
    unsigned int newVcThr=it->second.getThreshold();
    unsigned int newCalDel=it->second.getCalDelay();
 

    cout << "New settings: VcThr="<<newVcThr<<" CalDel=" << newCalDel << endl;

    if (it->second.validSettings()){

      theBranch.pass=1;
      theBranch_sum.new_CalDel=newCalDel;
      theBranch_sum.delta_CalDel=(float)oldCalDel-newCalDel;
      theBranch_sum.new_VcThr=newVcThr;
      theBranch_sum.delta_VcThr=(float)oldVcThr-newVcThr;
	
      tree->Fill();
      tree_sum->Fill();

      rocDACs->setVcThr(newVcThr);
      rocDACs->setCalDel(newCalDel);
    }
    else{
      cout << "Did not have valid settings for:" << it->first.rocname() << endl;
	
	
      theBranch_sum.new_CalDel=oldCalDel;
      theBranch_sum.delta_CalDel=0.0;
      theBranch_sum.new_VcThr=oldVcThr;
      theBranch_sum.delta_VcThr=0.0;
      tree->Fill();
      tree_sum->Fill();
	
    }

    it->second.setOldThreshold(oldVcThr);
    it->second.setOldCalDelay(oldCalDel);

    min1=it->second.getmin1();
    max1=it->second.getmax1();
    min2=it->second.getmin2();
    max2=it->second.getmax2();
    nbins1=it->second.getnbins1();
    nbins2=it->second.getnbins2();
    valid=it->second.getValid();
    numerator=tempCalibObject_->nTriggersPerPattern()*tempCalibObject_->nPixelPatterns();

    TCanvas* canvas = new TCanvas((rocName+"_Canvas").c_str(),rocName.c_str(),800,600);

    TH2F histo2D=it->second.FillEfficiency(numerator);

    histo2D.GetXaxis()->SetTitle(name2_.c_str());
    histo2D.GetYaxis()->SetTitle(name1_.c_str());
    histo2D.SetMinimum(0.0);
    histo2D.SetMaximum(1.0);
    histo2D.Draw("colz");

    double deltaCalDelay=0.05*(max1-min1);
    double deltaVcThr=0.05*(max2-min2);

    TLine* l1=new TLine(oldCalDel-deltaCalDelay,oldVcThr,
			oldCalDel+deltaCalDelay,oldVcThr);
    l1->SetLineColor(38);
    l1->Draw();
    l1=new TLine(oldCalDel,oldVcThr-deltaVcThr,
		 oldCalDel,oldVcThr+deltaVcThr);
    l1->SetLineColor(38);
    l1->Draw();

    if (valid)
      {
	TLine* l2=new TLine(newCalDel-deltaCalDelay,newVcThr, newCalDel+deltaCalDelay,newVcThr);
	l2->SetLineColor(kMagenta);
	l2->Draw();
	l2=new TLine(newCalDel,newVcThr-deltaVcThr,
		     newCalDel,newVcThr+deltaVcThr);
	l2->SetLineColor(kWhite);
	l2->Draw();
      }

    canvas->Write();
      
  }

  outputFile_->Write();
  outputFile_->Close();
      
  //write out root file

  std::map<PixelModuleName,PixelDACSettings*>::iterator dacs=theDACs.begin();

  for(;dacs!=theDACs.end();++dacs){
    dacs->second->writeASCII(outputDir());
  }

 


  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

