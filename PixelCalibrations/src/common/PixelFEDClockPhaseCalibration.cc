// $Id: PixelFEDClockPhaseCalibration.cc,v 1.45 2012/02/17 15:42:16 mdunser Exp $

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2007, Cornell U.			                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd 		                			 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelFEDClockPhaseCalibration.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "PixelCalibrations/include/PixelHistoReadWriteFile.h"

#include "TProfile.h"
#include "TString.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TROOT.h"

using namespace pos;

PixelFEDClockPhaseCalibration::PixelFEDClockPhaseCalibration(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  std::cout << "Greetings from the PixelFEDClockPhaseCalibration copy constructor." << std::endl;

  nzoom_=6;
  nslotzoom_=20;
  nslot_=nzoom_*nslotzoom_;
  startslot_=1;
  offsetphase0_=13;
  offsetphase1_=3;


  //Raw data is not corrected
  phase0Shift_[0]=0;
  phase1Shift_[0]=0;
  phase0Phase_[0]=0;
  phase1Phase_[0]=0;

  //Purged data is not corrected
  phase0Shift_[1]=0;
  phase1Shift_[1]=0;
  phase0Phase_[1]=0;
  phase1Phase_[1]=0;

  // Shifted data correction
  phase0Shift_[2]=0;
  phase1Shift_[2]=0;
  phase0Phase_[2]=offsetphase0_;
  phase1Phase_[2]=offsetphase1_;

  // Shifted and aligned data correction
  phase0Shift_[3]=0;
  phase1Shift_[3]=offsetphase0_-offsetphase1_;
  phase0Phase_[3]=offsetphase0_;
  phase1Phase_[3]=offsetphase1_;

}

xoap::MessageReference PixelFEDClockPhaseCalibration::beginCalibration(xoap::MessageReference msg){

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");

  tempCalibObject_=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject_!=0);

  oldMode_=tempCalibObject_->parameterValue("oldMode");

  cout << "oldMode is: " << oldMode_ << endl;

  fedsAndChannels_=tempCalibObject_->getFEDsAndChannels(theNameTranslation_);
  std::map <unsigned int, std::set<unsigned int> >::iterator i_fedsAndChannels;
  
  std::vector<std::pair<unsigned int,std::vector<unsigned int> > > fedsAndChannels=tempCalibObject_->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_ );
  

  std::string crate=itoa(crate_);
  outputFile_=new TFile( (outputDir()+"/FEDClockPhaseCalibration_"+crate.c_str()+".root").c_str(), "recreate", 
			 (outputDir()+"/FEDClockPhaseCalibration_"+crate.c_str()+".root").c_str()); 

  dirMakerFED_ = new PixelRootDirectoryMaker(fedsAndChannels, outputFile_);

  for (i_fedsAndChannels=fedsAndChannels_.begin();
       i_fedsAndChannels!=fedsAndChannels_.end();++i_fedsAndChannels){

    unsigned int fednumber=i_fedsAndChannels->first;
    unsigned int fedcrate=theFEDConfiguration_->crateFromFEDNumber(fednumber);
    if (fedcrate==crate_){
      
      std::set<unsigned int>::iterator i_channel=i_fedsAndChannels->second.begin();
      
      for (;i_channel!=i_fedsAndChannels->second.end();++i_channel){
	int channel=*i_channel;

	dirMakerFED_->cdDirectory(fednumber,channel);	

	std::cout << "fednumber channel:"<<fednumber<<" "<<channel
		  << std::endl;
	

	TString name="Clock phase 0 fednumber=";
	name+=fednumber;
	name+=" channel=";
	name+=channel;
	
	histogramPhase0_[fednumber][channel]=
	  new TProfile(name,
		       name,
		       nslot_*16,
		       startslot_-1/32.0,
		       startslot_+nslot_-1/32.0,
		       0.0,
		       1023.0);
	
	name="Clock phase 1 fednumber=";
	name+=fednumber;
	name+=" channel=";
	name+=channel;
	
	histogramPhase1_[fednumber][channel]=
	  new TProfile(name,
		       name,
		       nslot_*16,
		       startslot_-1/32.0,
		       startslot_+nslot_-1/32.0,
		       0.0,
		       1023.0);
	
	
	name="Purged clock phase 0 fednumber=";
	name+=fednumber;
	name+=" channel=";
	name+=channel;
	
	histogramPurgedPhase0_[fednumber][channel]=
	  new TProfile(name,
		       name,
		       nslot_*16,
		       startslot_-1/32.0,
		       startslot_+nslot_-1/32.0,
		       0.0,
		       1023.0);
	
	
	name="Purged clock phase 1 fednumber=";
	name+=fednumber;
	name+=" channel=";
	name+=channel;

	histogramPurgedPhase1_[fednumber][channel]=
	  new TProfile(name,
		       name,
		       nslot_*16,
		       startslot_-1/32.0,
		       startslot_+nslot_-1/32.0,
		       0.0,
		       1023.0);

	name="Ordered clock phase 0 fednumber=";
	name+=fednumber;
	name+=" channel=";
	name+=channel;
	
	histogramOrderedPhase0_[fednumber][channel]=
	  new TProfile(name,
		       name,
		       nslot_*16,
		       startslot_-1/32.0,
		       startslot_+nslot_-1/32.0,
		       0.0,
		       1023.0);
	

	name="Ordered clock phase 1 fednumber=";
	name+=fednumber;
	name+=" channel=";
	name+=channel;
	
	histogramOrderedPhase1_[fednumber][channel]=
	  new TProfile(name,
		       name,
		       nslot_*16,
		       startslot_-1/32.0,
		       startslot_+nslot_-1/32.0,
		       0.0,
		       1023.0);
	
	name="Final clock phase 0 fednumber=";
	name+=fednumber;
	name+=" channel=";
	name+=channel;
	
	histogramFinalPhase0_[fednumber][channel]=
	  new TProfile(name,
		       name,
		       nslot_*16,
		       startslot_-1/32.0,
		       startslot_+nslot_-1/32.0,
		       0.0,
		       1023.0);


	name="Final clock phase 1 fednumber=";
	name+=fednumber;
	name+=" channel=";
	name+=channel;

	histogramFinalPhase1_[fednumber][channel]=
	  new TProfile(name,
		       name,
		       nslot_*16,
		       startslot_-1/32.0,
		       startslot_+nslot_-1/32.0,
		       0.0,
		       1023.0);
	
      }
    }
  }


  return reply;

}



xoap::MessageReference PixelFEDClockPhaseCalibration::execute(xoap::MessageReference msg)
{

  Attribute_Vector parametersReceived(1);
  parametersReceived[0].name_="Delay";
  Receive(msg, parametersReceived);
  unsigned int delayphase=atoi(parametersReceived[0].value_.c_str());

  unsigned int phase=delayphase%2;
  unsigned int delay=delayphase/2;

  int offset=0;

  std::map <unsigned int, std::set<unsigned int> >::iterator i_fedsAndChannels;

  for (i_fedsAndChannels=fedsAndChannels_.begin();
       i_fedsAndChannels!=fedsAndChannels_.end();++i_fedsAndChannels){
    
    unsigned int fednumber=i_fedsAndChannels->first;
    unsigned int fedcrate=theFEDConfiguration_->crateFromFEDNumber(fednumber);
    if (fedcrate==crate_){
      
      unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
      std::set<unsigned int>::iterator i_channel=i_fedsAndChannels->second.begin();
      
      for (;i_channel!=i_fedsAndChannels->second.end();++i_channel){
	
	uint32_t buffer[pos::fifo1TranspDepth];
	unsigned int channel=*i_channel;
	unsigned int buffersize=FEDInterface_[vmeBaseAddress]->drain_transBuffer(channel, buffer);
	
	//We have to turn off the base line correction as otherwise
	//the correction gets confused by the invalide delay and phase
	//settings
	int baselinecor=FEDInterface_[vmeBaseAddress]->get_BaselineCorr(channel);
	assert(baselinecor==0);

	if (buffersize!=pos::fifo1TranspDataLength) {
	  std::cout << "PixelFECClockPhaseCalibration::execute: fednumber="
		    << fednumber << " channel="<<channel
		    << " buffersize="<<buffersize<<std::endl;
	  for(unsigned int ii=0;ii<1024;ii++){
	    std::cout << ii <<" "<<hex<<buffer[ii]<<dec<<std::endl;
	  }
	}
	
	if ((buffer[pos::fifo1TranspDataLength-1]&0xff)!=0xff) {
	  std::cout << "PixelFECClockPhaseCalibration::execute: fednumber="
		    << fednumber << " channel="<<channel
		    << " buffer[pos::fifo1TranspDataLength-1]="
		    << hex << buffer[pos::fifo1TranspDataLength-1] 
		    << dec << std::endl;
	  std::cout << "Should be 0xff to make sure the full buffer was read"
		    << std::endl;
	  for(unsigned int ii=0;ii<buffersize;ii++){
	    std::cout << ii <<" "<<hex<<buffer[ii]<<dec<<std::endl;
	  }
	}

	if (phase==0) offset=offsetphase0_;
	if (phase==1) offset=offsetphase1_;
	
	for(unsigned int slot=0;slot<pos::fifo1TranspDataLength;slot++){
            
	  double adc=((0xffc00000 & buffer[slot])>>22);

	  //if (slot<50&&channel==10) {
	  //  std::cout << "slot, adc:"<<slot<<" "<<adc<<std::endl;
	  //}

	  if (phase==0) {
	    histogramPhase0_[fednumber][channel]->Fill(phase0Shift_[0]/16.0+slot+(((int)delay+phase0Phase_[0])%16)/16.0,adc,1.0);
	  } else {
	    histogramPhase1_[fednumber][channel]->Fill(phase1Shift_[0]/16.0+slot+(((int)delay+phase1Phase_[0])%16)/16.0,adc,1.0);
	  }
	  
	  if (((phase==0)&&(delay==1||delay==2||delay==3||delay==4))||
	      ((phase==1)&&(delay==13||delay==10||delay==11||delay==12))) continue;
            

	  if (phase==0) {
	    histogramPurgedPhase0_[fednumber][channel]->Fill(phase0Shift_[1]/16.0+slot+(((int)delay+phase0Phase_[1])%16)/16.0,adc,1.0);
	  } else {
	    histogramPurgedPhase1_[fednumber][channel]->Fill(phase1Shift_[1]/16.0+slot+(((int)delay+phase1Phase_[1])%16)/16.0,adc,1.0);
	  }
	  
	  if (phase==0) {
	    histogramOrderedPhase0_[fednumber][channel]->Fill(phase0Shift_[2]/16.0+slot+(((int)delay+phase0Phase_[2])%16)/16.0,adc,1.0);
	  } else {
	    histogramOrderedPhase1_[fednumber][channel]->Fill(phase1Shift_[2]/16.0+slot+(((int)delay+phase1Phase_[2])%16)/16.0,adc,1.0);
	  }

	  if (phase==0) {
	    histogramFinalPhase0_[fednumber][channel]->Fill(phase0Shift_[3]/16.0+slot+(((int)delay+phase0Phase_[3])%16)/16.0,adc,1.0);
	  } else {
	    histogramFinalPhase1_[fednumber][channel]->Fill(phase1Shift_[3]/16.0+slot+(((int)delay+phase1Phase_[3])%16)/16.0,adc,1.0);
	  }   
	}
      }
    }
  }
  
  std::map<unsigned long, HAL::VMEDevice*>::iterator i=VMEPtr_.begin();

  //FIXME is this really needed???
  for(;i!=VMEPtr_.end();++i){
    i->second->write("LRES",0x80000000); // Local Reset
  }


  xoap::MessageReference reply = MakeSOAPMessageReference("ClockPhaseCalibrationDone");
  return reply;


}


xoap::MessageReference PixelFEDClockPhaseCalibration::endCalibration(xoap::MessageReference msg){
  cout << "In PixelFEDClockPhaseCalibration::endCalibration()" << endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");

  outputFile_->cd();

  std::map <unsigned int, std::map<unsigned int, TProfile*> >::iterator ifednumber=histogramPhase0_.begin();
  for (;ifednumber!=histogramPhase0_.end();++ifednumber){

    std::map<unsigned int, TProfile*>::iterator ichannel=ifednumber->second.begin();

    unsigned int fednumber=ifednumber->first;

    for(;ichannel!=ifednumber->second.end();++ichannel) {

      unsigned int channel=ichannel->first;

      dirMakerFED_->cdDirectory(fednumber,channel);	

      TProfile* profPhase0=histogramFinalPhase0_[fednumber][channel];
      TProfile* profPhase1=histogramFinalPhase1_[fednumber][channel];

      
    ///////////////////////
    ////// TBM MODE////////
    ///////////////////////
	unsigned int clocktbm=0;
	int delaysel=-1;
	Moments black;
	for (int iclock=1;iclock<100;iclock++) {
	if (delaysel!=-1) break;
	for (int idelay=0;idelay<16;idelay++) {
		if (delaysel!=-1) break;
		int ibin=iclock*16+idelay;
		double adc_0=profPhase0->GetBinContent(ibin);
		double adc_1=profPhase1->GetBinContent(ibin);
		double adc=0.5*(adc_0+adc_1);
		if (adc_0<0.5) adc=adc_1;
		if (adc_1<0.5) adc=adc_0;
		if (adc<0.5) { 
		   cout << "ibin="<<ibin<<" adc (phase0, phase1)="
		   << adc_0<<" "<<adc_1<<endl;
		}
		if (iclock>5) {
	        if (adc<black.mean()-50.0){
			delaysel=idelay-4;
			clocktbm=iclock+10;
			if (delaysel<0)delaysel+=16;
		   }		
		}
		black.push_back(adc);
	    }
	}
	  
    ///////////////////////
    ////// ROC MODE////////
    ///////////////////////
      
      unsigned int clockmax=3*theNameTranslation_->getROCsFromFEDChannel(fednumber,channel).size()-2;
      double diffmin=0.0;
      int delaymin=0;
      for (int idelay=0;idelay<16;idelay++) {
	double sumdiff=0.0;
	for (unsigned int iclock=clocktbm;iclock<=clocktbm+clockmax;iclock++) {
	  int ibin1=(iclock-startslot_)*16+idelay-1;
	  int ibin2=(iclock-startslot_)*16+idelay+1;
	  double adc1_0=profPhase0->GetBinContent(ibin1);
	  double adc1_1=profPhase1->GetBinContent(ibin1);
	  double adc1=0.5*(adc1_0+adc1_1);
	  if (adc1_0<0.5) adc1=adc1_1;
	  if (adc1_1<0.5) adc1=adc1_0;
	  if (adc1<0.5) { 
	    cout << "ibin1="<<ibin1<<" adc (phase0, phase1)="
		 << adc1_0<<" "<<adc1_1<<endl;
	  }			      
	  
	  double adc2_0=profPhase0->GetBinContent(ibin2);
	  double adc2_1=profPhase1->GetBinContent(ibin2);
	  double adc2=0.5*(adc2_0+adc2_1);
	  if (adc2_0<0.5) adc2=adc2_1;
	  if (adc2_1<0.5) adc2=adc2_0;
	  if (adc2<0.5) { 
	    cout << "ibin2="<<ibin2<<" adc (phase0, phase1)="
		 << adc2_0<<" "<<adc2_1<<endl;
	  }
	  
	  sumdiff+=fabs(adc1-adc2);
	  
	}
	if (idelay==0) {
	  diffmin=sumdiff;
	  delaymin=idelay;
	}
	if (diffmin>sumdiff) {
	  diffmin=sumdiff;
	  delaymin=idelay;
	}
	
      }
      cout << "ROC Mode bounds (min, max): " << clocktbm << ", " << clocktbm+clockmax << ": " << endl;
      
      cout << "Delay setting by TBM: " << delaysel << endl;
      cout << "Delay setting by ROC: " << delaymin << endl;

      //Automatic Mode
      delaymin=AutomaticModeDelay(delaysel,delaymin,3);

      //OldMode
      if (oldMode_!="Yes"){
	 delaymin=delaysel;
      }

      cout << "Chosen Delay setting:"<<delaymin<<endl;

      
      int delayPhaseBest0=0;
      int delayPhaseBest1=0;

      for(int plots=0;plots<4;plots++){

	TProfile* profPhase0=0;
	TProfile* profPhase1=0;
	
	int delay=delaymin;
	int delayphase0=delay;
	int delayphase1=delay;

	string namebase;

	std::string name;

	TCanvas* c;

	if (plots==0) {
	  namebase="Raw ";
	  name+="PhaseAndDelayRaw_";
	  profPhase0=histogramPhase0_[fednumber][channel];
	  profPhase1=histogramPhase1_[fednumber][channel];
	  delayphase0-=(phase0Shift_[3]+phase0Phase_[3]);
	  delayphase1-=(phase1Shift_[3]+phase1Phase_[3]);
	  delayPhaseBest0=delayphase0;
	  delayPhaseBest1=delayphase1;
	}
	if (plots==1) {
	  namebase="Purged ";
	  name+="PhaseAndDelayPurged_";
	  profPhase0=histogramPurgedPhase0_[fednumber][channel];
	  profPhase1=histogramPurgedPhase1_[fednumber][channel];
	  delayphase0-=(phase0Shift_[3]+phase0Phase_[3]);
	  delayphase1-=(phase1Shift_[3]+phase1Phase_[3]);
	}
	if (plots==2) {
	  namebase="Ordered ";
	  name+="PhaseAndDelayOrdered_";
	  profPhase0=histogramOrderedPhase0_[fednumber][channel];
	  profPhase1=histogramOrderedPhase1_[fednumber][channel];
	  delayphase0-=phase0Shift_[3];
	  delayphase1-=phase1Shift_[3];
	}

	if (delayphase0<0) delayphase0+=16;
	if (delayphase1<0) delayphase1+=16;
	
	if (plots==3) {
	  namebase="Final ";
	  name+="PhaseAndDelayFinal_";
	  profPhase0=histogramFinalPhase0_[fednumber][channel];
	  profPhase1=histogramFinalPhase1_[fednumber][channel];
	  delayphase0=delaymin;
	  delayphase1=-1;
	}
	
	
	name+=itoa(fednumber);
	name+="_";
	name+=itoa(channel);
	c = new TCanvas(name.c_str(), name.c_str(), 1600, 4000);
	c->Divide(1,nzoom_);

	std::vector<std::vector<TProfile*> > p;

	for(unsigned int izoom=0;izoom<nzoom_;izoom++){
	  c->cd(izoom+1);
          string name=namebase+" "+itoa(izoom+1)+" "; 
	  p.push_back(draw(name, fednumber, channel,
			   profPhase0,
			   profPhase1,
			   startslot_+izoom*nslotzoom_, 
			   nslotzoom_, 
			   delayphase0,
			   delayphase1));
	}	
	
	c->Write();

	for(std::vector<std::vector<TProfile*> >::iterator it=p.begin(), it_end=p.end(); it!=it_end; ++it){
	  for(std::vector<TProfile*>::iterator it1=(*it).begin(), it1_end=(*it).end(); it1!=it1_end; ++it1){
	    delete (*it1);
	  }
	}

      }
   
      

      delayPhaseBest0=(16+delayPhaseBest0)%16;
      delayPhaseBest1=(16+delayPhaseBest1)%16;

      assert(delayPhaseBest0==delayPhaseBest1);
      unsigned int bestDelay=delayPhaseBest0;

      //FIXME hardcode alert!
      //Try to find the phase that is furthest from the
      //illegal values of 1,2,3,4 for phase 0 and
      //10,11,12 for phase 1.
      
      int phase0Diff=delayPhaseBest0-2;
      int phase1Diff=delayPhaseBest1-11;
      
      if (phase0Diff<0) phase0Diff=-phase0Diff;
      if (phase0Diff>8) phase0Diff=16-phase0Diff;
      
      if (phase1Diff<0) phase1Diff=-phase1Diff;
      if (phase1Diff>8) phase1Diff=16-phase1Diff;
      
      int bestPhase=0;
      if (phase1Diff>phase0Diff) bestPhase=1;
      
      
      updatePhaseAndDelay(fednumber,channel,bestPhase,bestDelay);
    }
      
    writeFEDCard(fednumber);
    
  }

  
  cout<<"--- Running for Clock Delay and Phase Calibration Done ---"<<endl;

  outputFile_->Write();
  outputFile_->Close();
  return reply;
}



void PixelFEDClockPhaseCalibration::drawDelay(unsigned int startslot, 
	       unsigned int nslot, 
	       int delay, int color){

  for(unsigned int islot=0;islot<nslot;islot++){
    double ibin=islot+startslot+delay/16.0;
    TLine* l=new TLine(ibin,0.0,ibin,1023.0);
    l->SetLineColor(color);
    l->Draw();
  }

}

std::vector<TProfile*> PixelFEDClockPhaseCalibration::draw(std::string title,
					 unsigned int fednumber,
					 unsigned int channel,
					 TProfile* phase0, 
					 TProfile* phase1, 
					 unsigned int slotstart, 
					 unsigned int nslot, 

					 int delay0,int delay1){
  std::vector<TProfile*> p;

  TString name=title;
  

  name+=" phase 0 fednumber=";
  name+=fednumber;
  name+=" channel=";
  name+=channel;
  
  TProfile* tmpPhase0=new TProfile(name,
				   name,
				   nslot*16,
				   slotstart-1/32.0,
				   slotstart+nslot-1/32.0,
				   0.0,
				   1023.0);
  

  name=title;
  name+=" phase 1 fednumber=";
  name+=fednumber;
  name+=" channel=";
  name+=channel;
  
  TProfile* tmpPhase1=new TProfile(name,
				   name,
				   nslot*16,
				   slotstart-1/32.0,
				   slotstart+nslot-1/32.0,
				   0.0,
				   1023.0);

  for(unsigned int ibin=1;ibin<=nslot*16;ibin++){
    double phase0Val=phase0->GetBinContent(ibin+(slotstart-startslot_)*16);
    double phase0Nbin=phase0->GetBinEntries(ibin+(slotstart-startslot_)*16);
    double phase1Val=phase1->GetBinContent(ibin+(slotstart-startslot_)*16);
    double phase1Nbin=phase1->GetBinEntries(ibin+(slotstart-startslot_)*16);

    tmpPhase0->Fill(slotstart+(ibin-1.0)/16.0,phase0Val,phase0Nbin);
    tmpPhase1->Fill(slotstart+(ibin-1.0)/16.0,phase1Val,phase1Nbin);
  }


  tmpPhase0->SetLineColor(4);
  tmpPhase1->SetLineColor(1);
  
  tmpPhase0->SetMinimum(0.0);
  tmpPhase0->SetMaximum(1023.0);
  
  tmpPhase1->SetMinimum(0.0);
  tmpPhase1->SetMaximum(1023.0);
  
  
  tmpPhase0->Draw("E1");
  tmpPhase1->Draw("sameE1"); 

  drawDelay(slotstart,nslot,delay0,4);   

  if (delay1!=-1) {
    drawDelay(slotstart,nslot,delay1,1);   
  }

  p.push_back(tmpPhase0);
  p.push_back(tmpPhase1);
  return p;
  
  

}



void PixelFEDClockPhaseCalibration::updatePhaseAndDelay(unsigned int fednumber,
                                                        unsigned int channel,
                                                        unsigned int phase,
                                                        unsigned int delay){

  assert(phase<=1);
  assert(delay<16);

  unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
  PixelFEDCard& thePixelFEDCard=FEDInterface_[vmeBaseAddress]->getPixelFEDCard();
  std::cout<<"Channel "<<channel<<" has best phase ="<<phase<<" and delay="<<delay<<std::endl;
  thePixelFEDCard.DelayCh[channel-1]=delay;
  if (channel>=1 && channel<=9) {
    int clkphs=thePixelFEDCard.clkphs1_9;
    thePixelFEDCard.clkphs1_9=(clkphs & (0x1ff-(1<<(channel-1)))) + (phase<<(channel-1));
  } else if (channel>=10 && channel<=18) {
    int clkphs=thePixelFEDCard.clkphs10_18;
    thePixelFEDCard.clkphs10_18=(clkphs & (0x1ff-(1<<(channel-10)))) + (phase<<(channel-10));
  } else if (channel>=19 && channel<=27) {
    int clkphs=thePixelFEDCard.clkphs19_27;
    thePixelFEDCard.clkphs19_27=(clkphs & (0x1ff-(1<<(channel-19)))) + (phase<<(channel-19));
  } else if (channel>=28) {
    int clkphs=thePixelFEDCard.clkphs28_36;
    thePixelFEDCard.clkphs28_36=(clkphs & (0x1ff-(1<<(channel-28)))) + (phase<<(channel-28));
  } else {
    std::cout<<"PixelFEDSupervisor::ClockPhaseCalibration - Incorrect Channel Number = "<<channel<<std::endl;
    diagService_->reportError("ClockPhaseCalibration: Incorrect Channel Number ="+channel,DIAGERROR);
  }
}
 
void PixelFEDClockPhaseCalibration::writeFEDCard(unsigned int fednumber){
  
  unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
  PixelFEDCard& thePixelFEDCard=FEDInterface_[vmeBaseAddress]->getPixelFEDCard();

  //Need to restore the baseline correction masks as we
  //turned the correction off.
  thePixelFEDCard.restoreBaselinAndChannelMasks();
  thePixelFEDCard.restoreControlAndModeRegister();
  thePixelFEDCard.writeASCII(outputDir());

}

void PixelFEDClockPhaseCalibration::initializeFED(){

  setFEDModeAndControlRegister(0x0,0x30019);
  setBlackUBTrans();
}


int PixelFEDClockPhaseCalibration::AutomaticModeDelay(int delayTBM, int delayROC, int warn){

  int delaymin;
  int delayTBMup=delayTBM+8;
  int delaywarnup=delayTBM+warn;
  int delaywarndown=delayTBM-warn;
  
  if(delayTBM!=delayROC){

    //if wrapping point is "after" (within 8 after) delayTBM
    if(delayTBMup>15){
      delayTBMup=delayTBMup-16;
      //if delayROC is before delayTBM
      if(delayROC>delayTBMup && delayROC<delayTBM){
	
	//warn -- if delayROC is way too early
	if(delayROC<=delaywarndown){
	  diagService_->reportError("TBM and ROC require very different Delay",DIAGWARN);
	    }
	
	//compromise delayROC towards delayTBM
	delayROC=delayTBM-1;
	if(delayROC<0)delayROC=15;
      }
      //else delayROC is after delayTBM
      else{
	
	//if delaywarn is after wrapping point
	if(delaywarnup>15){
	  delaywarnup=delaywarnup-16;
	  //warn --  delayROC is way too late
	  if(delayROC>=delaywarnup && delayROC<=delayTBMup){
	    diagService_->reportError("TBM and ROC require very different Delay",DIAGWARN);
	      }
	}
	//if delaywarn is before wrapping point
	else if(delayROC>=delaywarnup || delayROC<=delayTBMup){
	  diagService_->reportError("TBM and ROC require very different Delay",DIAGWARN);
	    }
	
	//compromise delayROC towards delayTBM
	delayROC=delayTBM+1;
	if(delayROC>15)delayROC=0;
      }
    }
    //else wrapping point is before delayTBM
    else{
      //if delayROC is after delayTBM
      if(delayROC>delayTBM && delayROC<delayTBMup){
	
	//warn -- if delayROC is way after delayTBM
	if(delayROC>=delaywarnup){
	  diagService_->reportError("TBM and ROC require very different Delay",DIAGWARN);
	    }
	
	//compromise delayROC towards delayTBM
	delayROC=delayTBM+1;
	if(delayROC>15)delayROC=0;
      }
      
      //else delayROC is before delayTBM
      else{
	
	//if wrapping point is after delaywarn
	if(delaywarndown<0){
	  delaywarndown=delaywarndown+16;
	  //warn -- if delayROC is way before delayTBM
	  if(delayROC>=delayTBMup && delayROC<=delaywarndown){
	    diagService_->reportError("TBM and ROC require very different Delay",DIAGWARN);
	      }
	}
	//else wrapping point is before delay warn
	else if(delayROC<=delaywarndown || delayROC>=delayTBMup){
	  diagService_->reportError("TBM and ROC require very different Delay",DIAGWARN);
	    }
	
	//compromise delayROC towards delayTBM
	delayROC=delayTBM-1;
	if(delayROC<0)delayROC=15;
      }
    }
  }
  delaymin=delayROC;
  return delaymin;
}  

