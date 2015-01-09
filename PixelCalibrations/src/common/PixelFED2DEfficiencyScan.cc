// $Id: PixelFED2DEfficiencyScan.cc,v 1.11 2012/01/20 19:14:17 kreis Exp $

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2007-2008, Cornell U.			                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd                					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelFED2DEfficiencyScan.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelCalibrations/include/PixelEfficiency2D.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO1Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO2Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/FIFO3Decoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/ErrorFIFODecoder.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"

#include <toolbox/convertstring.h>

#include "iomanip"
#include "TH2F.h"
#include "TCanvas.h"
#include "TDirectory.h"

using namespace pos;
using namespace std;

PixelFED2DEfficiencyScan::PixelFED2DEfficiencyScan(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{  

}

xoap::MessageReference PixelFED2DEfficiencyScan::execute(xoap::MessageReference msg)
{

  unsigned int state=event_/tempCalibObject_->nTriggersPerPattern();

  unsigned int ithreshold=tempCalibObject_->scanCounter(name1_,state);
  unsigned int icaldelay=tempCalibObject_->scanCounter(name2_,state);

  //cout<<state<<" par1/par2 "<<ithreshold<<" "<<icaldelay<<" "<<endl;

  try {

    for ( map <unsigned int, set<unsigned int> >::iterator i_fedsAndChannels=fedsAndChannels_.begin(), i_fedsAndChannels_end=fedsAndChannels_.end();i_fedsAndChannels != i_fedsAndChannels_end ; ++i_fedsAndChannels){

      unsigned int fednumber=i_fedsAndChannels->first;
      unsigned int fedcrate=theFEDConfiguration_->crateFromFEDNumber(fednumber);
      if (fedcrate!=crate_) continue;

      //cout<<" crate/fed "<<fedcrate<<" "<<fednumber<<endl;

      PixelFEDInterface* iFED=FEDInterfaceFromFEDnumber_[fednumber];
      
      uint64_t buffer64[fifo3Depth];
      int status=iFED->spySlink64(buffer64);

      if (status>0) {
	
	FIFO3Decoder decode(buffer64);

	unsigned int nhits=decode.nhits(); //ask how many hits we have

	//cout<<fedcrate<<" "<<fednumber<<" "<<nhits<<endl;

	for (unsigned int ihit=0;ihit<nhits;ihit++){
	  unsigned int rocid=decode.rocid(ihit);
	  assert(rocid>0);
	  unsigned int channel=decode.channel(ihit);

	  //cout<<" chan/roc "<<channel<<" "<<rocid<<endl;

	  PixelROCName roc=theNameTranslation_->ROCNameFromFEDChannelROC(fednumber, channel, rocid-1);

	  map <PixelROCName, PixelEfficiency2D>::iterator it=eff_.find(roc); //extract the roc id

	  if (it!=eff_.end()) {
	    it->second.add(icaldelay,ithreshold);
	  }
	  else{
	    // comment out for all pixels enabled 
	    //cout << "Could not find ROC with fednumber="<<fednumber
	    // << " channel="<<channel<<" rocid="<<rocid<<endl; 
	  }
	}

      }
      else{
	cout << "Error reading spySlink64 status="<<status<<endl;
      }
    }
  } catch (HAL::HardwareAccessException& e) {
    diagService_->reportError("Exception occurred :",DIAGTRACE);//not TRACE, use ERROR
    string mes = e.what();
    diagService_->reportError(mes,DIAGINFO);

  } catch (exception e) {
    diagService_->reportError("*** Unknown exception occurred",DIAGWARN);
  }

    event_++;

  xoap::MessageReference reply = MakeSOAPMessageReference("ThresholdCalDelayDone");
  return reply;
}

void PixelFED2DEfficiencyScan::initializeFED(){

  setFEDModeAndControlRegister(0x8,0x30010);//mode register, control register in config/parameter

}

xoap::MessageReference PixelFED2DEfficiencyScan::beginCalibration(xoap::MessageReference msg){


  tempCalibObject_=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject_!=0);

  fedsAndChannels_=tempCalibObject_->getFEDsAndChannels(theNameTranslation_); //add new method, to get fedsAndChannels_ from the "crate"  not from "name translation"

  unsigned int nScanVars=tempCalibObject_->numberOfScanVariables();
  assert(nScanVars>1);  //check the variables in the configuration first. Maybe send SOAP message not using assert

  name1_=tempCalibObject_->scanName(0);
  name2_=tempCalibObject_->scanName(1); 

  const vector<PixelROCName>& ROCs=tempCalibObject_->rocList();

  unsigned int nVariable1=tempCalibObject_->nScanPoints(name1_); 
  unsigned int nVariable2=tempCalibObject_->nScanPoints(name2_);
  
  double Variable1Min=tempCalibObject_->scanValueMin(name1_);
  double Variable1Max=tempCalibObject_->scanValueMax(name1_);
  double Variable1Step=tempCalibObject_->scanValueStep(name1_);
  
  double Variable2Min=tempCalibObject_->scanValueMin(name2_);
  double Variable2Max=tempCalibObject_->scanValueMax(name2_);
  double Variable2Step=tempCalibObject_->scanValueStep(name2_);

  cout<<"parameters "<<nScanVars<<" "<<name1_<<" "<<name2_<<" "
      <<nVariable1<<" "<<nVariable2<<endl;

  for (vector<PixelROCName>::const_iterator it=ROCs.begin(), it_end=ROCs.end(); it!=it_end; ++it) {

    cout<<" create 2d for roc "<<(*it).rocname()<<endl;

    PixelEfficiency2D tmp((*it).rocname(),name2_,nVariable2,
			  Variable2Min-0.5*Variable2Step,
			  Variable2Max+0.5*Variable2Step,
			  name1_,nVariable1,
			  Variable1Min-0.5*Variable1Step,
			  Variable1Max+0.5*Variable1Step);
    
    eff_[*it]=tmp;
  }

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFED2DEfficiencyScan::endCalibration(xoap::MessageReference msg){

  vector<string> rocsname;

  for(std::map<PixelROCName, PixelEfficiency2D>::iterator it=eff_.begin(), itend=eff_.end(); it!=itend ;++it){
    rocsname.push_back(it->first.rocname());
  }

  TFile outputFile( (outputDir()+"/2DEfficiencyScan_"+itoa(crate_)+".root").c_str(), "recreate", 
		    ("2DEfficiencyScan_"+itoa(crate_)+".root").c_str() );

  PixelRootDirectoryMaker rootDirs(rocsname, gDirectory);
  
  double numerator=0.0;

  for(std::map<PixelROCName, PixelEfficiency2D>::iterator it=eff_.begin(), itend=eff_.end(); it!=itend ;++it){

    string rocname=it->first.rocname();
    rootDirs.cdDirectory(rocname);

    numerator=tempCalibObject_->nTriggersPerPattern()*tempCalibObject_->nPixelPatterns();
    cout<<" End: numerator "<<numerator<<" "<<rocname<<endl;

    TCanvas* canvas = new TCanvas(rocname.c_str(),rocname.c_str(),800,600);
    TH2F histo2D=(it->second).FillEfficiency(numerator);
    
    histo2D.GetXaxis()->SetTitle(name2_.c_str());
    histo2D.GetYaxis()->SetTitle(name1_.c_str()); 
    
    histo2D.Draw("colz");
    canvas->Write();
    //  canvas->Print();


  }

  outputFile.Write();
  outputFile.Close();
  

  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

