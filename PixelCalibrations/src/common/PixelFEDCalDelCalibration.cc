// Modified by Jennifer Vaughan 2007/06/01
// $Id: Pixel2DEfficiencyScan.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelFEDCalDelCalibration.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO1Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO2Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO3Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/ErrorFIFODecoder.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"

#include "PixelUtilities/PixelFEDDataTools/include/PixelHit.h"

#include <toolbox/convertstring.h>

#include "iomanip"
#include "TH2F.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TStyle.h"
#include "TTree.h"
#include "TStyle.h"

#define BPIX

using namespace pos;
using namespace std;

PixelFEDCalDelCalibration::PixelFEDCalDelCalibration(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  
}

xoap::MessageReference PixelFEDCalDelCalibration::execute(xoap::MessageReference msg)
{
  std::map <unsigned int, std::set<unsigned int> >::iterator i_fedsAndChannels;
  unsigned int state=event_/tempCalibObject_->nTriggersPerPattern();

  unsigned int ithreshold=tempCalibObject_->scanCounter(name1_,state);
  unsigned int icaldelay=tempCalibObject_->scanCounter(name2_,state);

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
	  std::cout<<"Contents of Spy FIFO 3"<<std::endl;
	  std::cout<<"----------------------"<<std::endl;
	  //PixelFEDFifoData::decodeSlink64(buffer64, status, (dataFIFO_[vmeBaseAddress]));
	  for (int i=0; i<=status;++i) {
	    std::cout<<"Clock "<<i<<" = 0x"<<std::hex<<buffer64[i]<<std::dec<<std::endl;
	  }
	}
	
	FIFO3Decoder decode(buffer64);

	unsigned int nhits=decode.nhits();

	//std::cout << "nhits:"<<nhits<<std::endl;

	for (unsigned int ihit=0;ihit<nhits;ihit++){
	  unsigned int rocid=decode.rocid(ihit);
	  //assert(rocid>0);
	  unsigned int channel=decode.channel(ihit);


	  if(rocid<=0) {

	    std::cout <<" fed "<<fednumber<< "Channel="<<channel
		      <<" rocid="<<rocid<<" "<<ihit<<" "<<nhits<<std::endl;
	    std::cout<<"Contents of Spy FIFO 3"<<std::endl;
	    std::cout<<"----------------------"<<std::endl;
	    //PixelFEDFifoData::decodeSlink64(buffer64, status, (dataFIFO_[vmeBaseAddress]));
	    for (int i=0; i<=status;++i) {
	      unsigned int w1 = buffer64[i]&0xffffffff;
	      unsigned int w2 = (buffer64[i]>>32)&0xffffffff;
	      PixelHit h1(w1);
	      PixelHit h2(w2);

	      std::cout<<"Clock "<<i<<" = 0x"<<std::hex<<buffer64[i]
		       <<" "<<w1<<" "<<w2<<" "<<std::dec
  		       <<h1.getLink_id()<<" "<<h1.getROC_id()<<" "<<h1.getDCol_id()<<" "<<h1.getPix_id()<<" "
  		       <<h2.getLink_id()<<" "<<h2.getROC_id()<<" "<<h2.getDCol_id()<<" "<<h2.getPix_id()<<" "
		       <<std::endl;
	    }
	    
	    assert(rocid>0);
	  }

	  PixelROCName roc=theNameTranslation_->ROCNameFromFEDChannelROC(fednumber,
									 channel,
									 rocid-1);

	  //std::cout << "roc:"<<roc.rocname()<<std::endl;

	  std::map <PixelROCName, PixelEfficiency2DWBCCalDel>::iterator it=eff_.find(roc);

	  if (it!=eff_.end()) {
	    //std::cout << "Will call add"<<std::endl;
	    it->second.add(icaldelay,ithreshold);
	    //std::cout << "Done call add"<<std::endl;
	  }
	  else{
	    std::cout << "Could not find ROC with fednumber="<<fednumber
		      << " channel="<<channel<<" rocid="<<rocid<<std::endl;
	  }
	}

      }
      else{
	std::cout << "Error reading spySlink64 status="<<status<<std::endl;
      }
    }
  } catch (HAL::HardwareAccessException& e) {
    diagService_->reportError("Exception occurred :",DIAGTRACE);
    // cout << "*** Exception occurred : " << endl;
    string mes = e.what();
    diagService_->reportError(mes,DIAGINFO);
    //                cout << e.what() << endl;
  } catch (exception e) {
    diagService_->reportError("*** Unknown exception occurred",DIAGWARN);
    // cout << "*** Unknown exception occurred" << endl;
  }

  event_++;
  xoap::MessageReference reply = MakeSOAPMessageReference("ThresholdCalDelayDone");
  return reply;
}

void PixelFEDCalDelCalibration::initializeFED(){

  setFEDModeAndControlRegister(0x8,0x30010);

}

xoap::MessageReference PixelFEDCalDelCalibration::beginCalibration(xoap::MessageReference msg){

  tempCalibObject_=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject_!=0);

  fedsAndChannels_=tempCalibObject_->getFEDsAndChannels(theNameTranslation_);

  name1_=tempCalibObject_->scanName(1);
  name2_=tempCalibObject_->scanName(0);

  unsigned int nScanVars=tempCalibObject_->numberOfScanVariables();
  assert(nScanVars>1);

  fract_=0.2;

  std::vector<PixelROCName> aROC_string;

#ifdef BPIX
  // Change the Fraction if it is defined in the claib.dat file. d.k.3/7/08
  if ( tempCalibObject_->parameterValue("Fraction") != "" ) {
    fract_ = atof(tempCalibObject_->parameterValue("Fraction").c_str());
    cout<<" Set the mean fraction to "<<fract_<<endl;
  }
#endif // BPIX

  aROC_string=tempCalibObject_->rocList();

  assert(aROC_string.size()>0);

  unsigned int nThr=tempCalibObject_->nScanPoints(name1_);
  unsigned int  nCal=tempCalibObject_->nScanPoints(name2_);
      
  double VcThrMin=tempCalibObject_->scanValueMin(name1_);
  double VcThrMax=tempCalibObject_->scanValueMax(name1_);
  double VcThrStep=tempCalibObject_->scanValueStep(name1_);
    
  double CalDelMin=tempCalibObject_->scanValueMin(name2_);
  double CalDelMax=tempCalibObject_->scanValueMax(name2_);
  double CalDelStep=tempCalibObject_->scanValueStep(name2_);
          
  for (unsigned int i_aROC=0;i_aROC<aROC_string.size();++i_aROC) {
    
    PixelModuleName module(aROC_string[i_aROC].rocname());
    if (crate_ != theFEDConfiguration_->crateFromFEDNumber(theNameTranslation_->firstHdwAddress(module).fednumber())  ) continue;
	
    PixelEfficiency2DWBCCalDel tmp(aROC_string[i_aROC].rocname(),name2_,nCal,
				   CalDelMin-0.5*CalDelStep,
				   CalDelMax+0.5*CalDelStep,
				   name1_,nThr,
				   VcThrMin-0.5*VcThrStep,
				   VcThrMax+0.5*VcThrStep);
    eff_[aROC_string[i_aROC]]=tmp;
 
  } 

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDCalDelCalibration::endCalibration(xoap::MessageReference msg){

    //First we need to get the DAC settings for the ROCs

    std::vector<PixelModuleName> modules=theDetectorConfiguration_->getModuleList();
    std::vector<PixelModuleName>::iterator module_name = modules.begin();

    std::map<PixelModuleName,PixelDACSettings*> theDACs;

    for (;module_name!=modules.end();++module_name){

      if (crate_!= theFEDConfiguration_->crateFromFEDNumber(theNameTranslation_->firstHdwAddress(*module_name).fednumber())  ) continue;

      PixelDACSettings *tempDACs=0;

      std::string modulePath=(module_name->modulename());

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
    vector<int> CalDel;

    vector<std::string> rocsname;

    outputFile_=new TFile( (outputDir()+"/CalDel_"+itoa(crate_)+".root").c_str(), "recreate", 
			   ("CalDel_"+itoa(crate_)+".root").c_str() ); 

    branch theBranch;
    branch_sum theBranch_sum;
    TDirectory* dirSummaries = gDirectory->mkdir("SummaryTrees","SummaryTrees");
    dirSummaries->cd();

    TTree* tree = new TTree("PassState","PassState");
    TTree* tree_sum =new TTree("SummaryInfo","SummaryInfo");
    
    tree->Branch("PassState",&theBranch,"pass/F:rocName/C",4096000);
    tree_sum->Branch("SummaryInfo",&theBranch_sum,"new_CalDel/F:delta_CalDel/F:rocName/C",4096000);
    outputFile_->cd();

    for(map<PixelROCName, PixelEfficiency2DWBCCalDel>::iterator it=eff_.begin(), itend=eff_.end(); it!=itend ;++it){
      rocsname.push_back(it->first.rocname());
    }

    PixelRootDirectoryMaker rootDirs(rocsname, gDirectory);

    for(map<PixelROCName, PixelEfficiency2DWBCCalDel>::iterator it=eff_.begin(), itend=eff_.end(); it!=itend ;++it){
      
      string rocsname=it->first.rocname();

      theBranch.pass=0;
      strcpy(theBranch.rocName,rocsname.c_str());
      strcpy(theBranch_sum.rocName,rocsname.c_str());
      
      rootDirs.cdDirectory(rocsname);
      
      //FIXME slow way to make a module ame
      PixelModuleName module(it->first.rocname());

      double WBC=theDACs[module]->getDACSettings(it->first)->getWBC();

      it->second.findSettings(tempCalibObject_->nTriggersPerPattern()*
                              tempCalibObject_->nPixelPatterns(),
			      WBC,fract_);

      int calDel=-1;

      if (it->second.validSlope()) {

 	calDel=(int)it->second.getCalDel();

	// additinal protection in case the slope did not catch the problem it 
	if(calDel <= 0 || calDel > 255 ) {
	  int old_calDel = theDACs[module]->getDACSettings(it->first)->getCalDel();
	  cout<<" CalDel = 0 "<< it->second.validSlope() <<" "<<rocsname<<" "<<calDel<<" use old value "<<old_calDel<<endl;

	} else {

	  theBranch_sum.new_CalDel=(float)calDel;
	  
	  int old_calDel = theDACs[module]->getDACSettings(it->first)->getCalDel();
	  
	  theBranch_sum.delta_CalDel=(float)(old_calDel-calDel);
	  
	  theDACs[module]->getDACSettings(it->first)->setCalDel(calDel);
	  
	  CalDel.push_back(calDel);
	  
	  theBranch.pass=1;
	  
	  tree->Fill();
	  
	  tree_sum->Fill();

	  //cout<<it->second.validSlope() <<" "<<rocsname<<" "<<calDel<<" old value "<<old_calDel<<endl;

	}

      } else {

 	calDel=(int)it->second.getCalDel();
	int old_calDel = theDACs[module]->getDACSettings(it->first)->getCalDel();
	cout<<" Failed for "<<rocsname<<" "<<calDel<<" use old value "<<old_calDel<<endl;
      }

      tree->Fill();

      min1=it->second.getmin1();
      max1=it->second.getmax1();
      min2=it->second.getmin2();
      max2=it->second.getmax2();
      nbins1=it->second.getnbins1();
      nbins2=it->second.getnbins2();

      numerator=tempCalibObject_->nTriggersPerPattern()*tempCalibObject_->nPixelPatterns();
      
      TCanvas* canvas = new TCanvas((rocsname+"_c").c_str(),(rocsname+"_c").c_str(),800,600);

      TLine* determinedCalDel;
      TLine* setWBC;
      
      TH2F histo2D= it->second.FillEfficiency(numerator);
      
      histo2D.GetXaxis()->SetTitle(name2_.c_str());
      histo2D.GetYaxis()->SetTitle(name1_.c_str()); 
      
      gStyle->SetOptStat("ne");
      histo2D.Draw("colz");
      if(it->second.validSlope()){
	
	determinedCalDel = new TLine(calDel,min2,calDel,max2);
	setWBC = new TLine(0,WBC,250,WBC);
	determinedCalDel->SetLineColor(kMagenta); determinedCalDel->SetLineStyle(kDashed);
	setWBC->SetLineColor(kRed); setWBC->SetLineStyle(kDashed);
	determinedCalDel->Draw("same");
	setWBC->Draw("same");

      }
      canvas->Write();
      
    }

    outputFile_->cd();
    
    TCanvas* canvas1=new TCanvas("summary_c","summary_c",800,600);

    TH1F* histo_CalDel=new TH1F("summary","summary",256,0,255);

    
    for(vector<int>::iterator it=CalDel.begin(),itend=CalDel.end();it!=itend;++it){
      
      histo_CalDel->Fill(*(it));
      
    }

    histo_CalDel->Draw();
    histo_CalDel->GetXaxis()->SetTitle("CalDel");
    canvas1->Write();
    
    outputFile_->Write();
    outputFile_->Close();

   
    map<PixelModuleName,PixelDACSettings*>::iterator dacs=theDACs.begin();

    for(;dacs!=theDACs.end();++dacs){
      dacs->second->writeASCII(outputDir());
    }

    cout << "In PixelFEDCalDelCalibration::endCalibration()" << endl;
    xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
    return reply;
}

