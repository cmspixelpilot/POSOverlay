//--*-C++-*--
// $Id: PixelFEDBaselineCalibrationNew.cc,v 1.6 2012/02/17 15:42:16 mdunser Exp $
/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2009, Cornell University		                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd, S. Das, J. Vaughan, Y. Weng, J. Thompson		 *
 *                                                                       *
 * This calibration was originally intended to replace the code          *
 * in PixelFEDBaselineCalibration.cc           			         *
 * It was tested at the TIF (and maybe a little bit at P5).              *
 * However, it was never fully validated.                                *
 *************************************************************************/

#include "PixelCalibrations/include/PixelFEDBaselineCalibrationNew.h"
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelDecodedFEDRawData.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"

// #include <toolbox/convertstring.h>

#include "iomanip"

#include "TFile.h"
#include "TDirectory.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TStyle.h"
#include "TColor.h"

#define BPIX 

using namespace pos;
using namespace std;

const unsigned int numberOfFEDs=40;
const unsigned int channelsPerFED=36;
const unsigned int opticalReceiversPerFED=3;
const bool writeOutputFile = true;

PixelFEDBaselineCalibrationNew::PixelFEDBaselineCalibrationNew(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr),
    targetBlack_(numberOfFEDs,512),
    opticalReceiverSlope_(numberOfFEDs,200.0),
    channelOffsetSlope_(numberOfFEDs,2)
{
  //  cout << "Greetings from the PixelFEDBaselineCalibrationNew copy constructor." << endl;
}

xoap::MessageReference PixelFEDBaselineCalibrationNew::execute(xoap::MessageReference msg)
{

  const bool debug = false;

  ++iteration_;  
  cout<<"iteration "<<iteration_<<endl;

  vector <bool> B_OpticalReceiver_Used(numberOfFEDs*opticalReceiversPerFED,false);
  vector <Moments> B_Channel(numberOfFEDs*channelsPerFED), UB_Channel(numberOfFEDs*channelsPerFED);
  vector <double> B_Min(numberOfFEDs*opticalReceiversPerFED,1024.0), B_Max(numberOfFEDs*opticalReceiversPerFED,0.0);
  vector <unsigned int> ChannelOffsetDAC(numberOfFEDs*channelsPerFED);
  vector <unsigned int> InputOffsetDAC(numberOfFEDs*opticalReceiversPerFED);

  string replyString="FEDBaselineCalibrationWithPixelsDone";
 
  //map over feds and channels
  //  std::map <unsigned int, std::map<unsigned int, bool > > badBuffer;

 
  PixelTimer timer;
  PixelTimer timerSkip;
 
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  outputFile_->cd();

  string SummaryPlotName = "Baseline_iteration"+itoa(iteration_);
  
  TH2F* histo_BaselineVsFED = new TH2F(SummaryPlotName.c_str(),SummaryPlotName.c_str(),40,0.5,40.5,2048,0,1023);
  TCanvas* canvas_summaryplot =new TCanvas((SummaryPlotName+"_Canvas").c_str(),(SummaryPlotName+"_Canvas").c_str(),800,600);

  for (unsigned int ifed=0; ifed<fedsAndChannels_.size(); ++ifed) {
    
    unsigned int fednumber=fedsAndChannels_[ifed].first;
    unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
    PixelFEDInterface* fed = dynamic_cast<PixelFEDInterface*>(FEDInterface_[vmeBaseAddress]);
    assert(fed);
    
    for (unsigned int ichannel=0; ichannel<fedsAndChannels_[ifed].second.size(); ++ichannel) {
      uint32_t buffer[pos::fifo1TranspDepth];
      unsigned int channel=fedsAndChannels_[ifed].second[ichannel];
      unsigned int opto=opticalReceiversPerFED*fednumber+((channel-1)*opticalReceiversPerFED)/channelsPerFED;
      unsigned int channelindex=channelsPerFED*fednumber+channel-1;


      dirMakerFED_->cdDirectory(fedsAndChannels_[ifed].first,fedsAndChannels_[ifed].second[ichannel]);

      //check to see if this channel has already converged
      if ( (iteration_!=1) && (done_[fednumber][channel].back() == true)) {
	timerSkip.start();
	if (debug) cout<<"Already converged. Skipping buffer read for fed,channel="<<fednumber<<","<<channel<<endl;
	for (unsigned int ii=0; ii<pos::fifo1TranspDepth; ii++)  buffer[ii] = buffer_[fednumber][channel].at(ii);
	timerSkip.stop();
      }
      else {
	timer.start();
	if (debug) cout<<"Reading buffer"<<endl;;
	int status = fed->drain_transBuffer(channel, buffer);
	if (status<0){
	  diagService_->reportError("Could not drain FIFO 1 in transparent mode!!",DIAGWARN);
	  //	SendError(PixelSupervisor_,"PixelFEDSupervisor",theFEDConfiguration_->crate_,"[PixelFEDBaselineCalibrationNew::execute] Could not drain FIFO 1 in transparent mode!");
	  //	  cout<<"[PixelFEDBaselineCalibrationNew::execute] Could not drain FIFO 1 in transparent mode!"<<endl;
	}
	buffer_[fednumber][channel].clear();
	for (unsigned int ii=0; ii<pos::fifo1TranspDepth; ii++)  {
	  buffer_[fednumber][channel].push_back(buffer[ii]);
	}
	//	if (debug) cout<<endl;
	timer.stop();
      }

      PixelDecodedFEDRawData decodedRawData(buffer, 100., 100., 150., 0., 100., 0., 150.);
      decodedRawData.setAltMethod(true);

      //with this method this is a real problem!
      if (! decodedRawData.valid()) {
	diagService_->reportError("[PixelFEDBaselineCalibrationNew::execute] problem decoding raw data!",DIAGWARN);
	//	badBuffer[fednumber][channel] =true;
      }
      //      else 	badBuffer[fednumber][channel] =false;

      string tbmSignalFilename=outputDir()+"/FIFO1Signal_"+itoa(iteration_)+"_"+itoa(fednumber)+"_"+itoa(channel)+".gif";
      //string tbmSignalFilenameShort="FIFO1Signal_"+itoa(iteration_)+"_"+itoa(fednumber)+"_"+itoa(channel)+".gif";
      string tbmSignalFilenameshort="FIFO1Signal_"+itoa(iteration_)+"_"+itoa(fednumber)+"_"+itoa(channel);
      decodedRawData.drawToFile(tbmSignalFilename,tbmSignalFilenameshort,150);      
      stringstream *tbmSignal=new stringstream("~");
      
      // debug baseline corrections
      // cout << "Begin baseline correction for vmeBaseAddress " << vmeBaseAddress << endl;
      // FEDInterface_[vmeBaseAddress]->dump_BaselineCorr();
      // cout << "End baseline correction for vmeBaseAddress " << vmeBaseAddress << endl;
      
      int baselinecorrection=fed->get_BaselineCorr(channel);
      
      if (baselinecorrection!=0) {
	cout<<"[PixelFEDBaselineCalibrationNew::execute] Baseline Adjust for Channel "<<channel<<" is non-zero: "<<baselinecorrection<<endl;
  assert(baselinecorrection==0);
      }
      
      ////////////
      unsigned int i=TransparentDataStart(buffer,fednumber,channel);
      for (unsigned int j=1;j<i;++j) {
        unsigned int ADC=((buffer[j] & 0xffc00000) >> 22);
        B_Channel.at(channelindex).push_back(ADC);
        if (debug) {                                                                // Reducing printout to console
          std::cout<<"[PixelFEDBaselineCalibration::execute] Channel= "<<channel<<", slot="<<j<<", ADC="<<ADC<<std::endl;
        }
      }
      UB_Channel.at(channelindex).push_back(((buffer[i] & 0xffc00000) >> 22));
      UB_Channel.at(channelindex).push_back(((buffer[i+1] & 0xffc00000) >> 22));
      UB_Channel.at(channelindex).push_back(((buffer[i+2] & 0xffc00000) >> 22));

      ////////////


      
      //if (!badBuffer[fednumber][channel]) {
// 	B_Channel.at(channelindex) = decodedRawData.initialBlack().getMoments() ;
	
// 	if (debug) {
// 	  cout<<"[PixelFEDBaselineCalibrationNew::execute] UB="<< decodedRawData.TBMHeader().UB1()<<endl;
// 	  cout<<"[PixelFEDBaselineCalibrationNew::execute] UB="<< decodedRawData.TBMHeader().UB2()<<endl;
// 	  cout<<"[PixelFEDBaselineCalibrationNew::execute] UB="<< decodedRawData.TBMHeader().UB3()<<endl;
// 	}
// 	UB_Channel.at(channelindex).push_back(decodedRawData.TBMHeader().UB1());
// 	UB_Channel.at(channelindex).push_back(decodedRawData.TBMHeader().UB2());
// 	UB_Channel.at(channelindex).push_back(decodedRawData.TBMHeader().UB3());
      
      double B_Channel_mean=B_Channel.at(channelindex).mean();
      
      //some screen output for now
      if (iteration_>1 && (done_[fednumber][channel].back() == false)) {
	cout<<fednumber<<","<<channel<<" Iteration "<<iteration_
	    <<": DeltaBlack="<<B_Channel_mean-meanBlack_[fednumber][channel]
	    <<"; DeltaChannelOffset="<<deltaChannelOffset_[fednumber][channel]
	    <<"; DeltaInputOffset="<<deltaInputOffset_[fednumber][opto]<<endl;
	if (deltaInputOffset_[fednumber][opto]==0 && deltaChannelOffset_[fednumber][channel]!=0) cout<<"Actual Channel Offset Slope = "<<(B_Channel_mean-meanBlack_[fednumber][channel])/deltaChannelOffset_[fednumber][channel]<<endl;
      }
      
      B_OpticalReceiver_Used.at(opto)=true;
      
      if (B_Channel_mean>B_Max.at(opto)) B_Max.at(opto)=B_Channel_mean;
      if (B_Channel_mean<B_Min.at(opto)) B_Min.at(opto)=B_Channel_mean;
      //} //if

      //*tbmSignal<<"<img src=\""<<tbmSignalFilename.substr(string(getenv("POS_OUTPUT_DIRS")).size())<<"\"></img>";
      //*tbmSignal<<" ";
      summary_long_[fednumber][channel].push_back(tbmSignal);

    } //end loop over channels
    outputFile_->cd();
   
    TStyle* plainStyle = new TStyle("Plain", "a plain style");
    plainStyle->SetOptStat(0); // Suppress statistics box.
    plainStyle->cd();

    PixelFEDCard fedCard=FEDInterface_[vmeBaseAddress]->getPixelFEDCard();

    double baseline=0.0;
    double targetBaseline=targetBlack_[fednumber];
    string FEDName="Baseline_iteration_"+itoa(iteration_)+"_FED"+itoa(fednumber);
    string FEDName1="Baseline_iteration_"+itoa(iteration_)+"_FED"+itoa(fednumber)+"_Zoom";
    string fedName = "FED"+itoa(fednumber);
    dirMakerFED_->cdDirectory(fedName);
    TCanvas* canvas=new TCanvas((FEDName+"_Canvas").c_str(),FEDName.c_str(),800,600);
    canvas->Divide(1, 2);
    TH2F* histo_BaselineVsChannel = new TH2F(FEDName.c_str(),FEDName.c_str(),36,0.5,36.5,2048,0,1023); 
    TH2F* histo_BaselineVsChannelZoom = new TH2F(FEDName1.c_str(),FEDName1.c_str(),36,0.5,36.5,100,targetBaseline-25,targetBaseline+25);

    // Iteration over Optical Receivers
    for (unsigned int opticalReceiver=0;opticalReceiver<3;++opticalReceiver) {
      unsigned int receiverIndex=opticalReceiversPerFED*fednumber+opticalReceiver;

      if (B_OpticalReceiver_Used.at(receiverIndex)) {
        int oldInputOffsetValue=fedCard.opt_inadj[opticalReceiver];
        int newInputOffsetValue=oldInputOffsetValue;
        double opticalReceiverMean=(B_Min.at(receiverIndex)+B_Max.at(receiverIndex))/2.0;

        bool shiftUp=false, shiftDown=false;
        vector <unsigned int> channelsInThisFED=fedsAndChannels_[ifed].second;
        for (unsigned int ichannel=0;ichannel<channelsInThisFED.size();++ichannel) {
          if ((unsigned int)((channelsInThisFED[ichannel]-1)/12)==opticalReceiver) {
            if (fedCard.offs_dac[channelsInThisFED[ichannel]-1]==0) shiftDown=true;
            else if (fedCard.offs_dac[channelsInThisFED[ichannel]-1]==255) shiftUp=true;
          }
        }

        if (shiftUp && shiftDown) {
          diagService_->reportError("[PixelFEDBaselineCalibrationNew::execute] Cannot correct this situation using Optical Input Offsets and Channel Offsets! Please use your AOH gain settings!",DIAGWARN);
	  //          cout<<"Cannot correct this situation using Optical Input Offsets and Channel Offsets! Please use your AOH gain settings!"<<endl;
        } else if (shiftUp) {
          if (oldInputOffsetValue<15){
            diagService_->reportError("Shifting up Input Offset!",DIAGINFO);
            newInputOffsetValue=oldInputOffsetValue+1;
          } else {
            diagService_->reportError("Input Offset Value too high. Please use your AOH gain settings!",DIAGWARN);
	    //	    cout<<"Input Offset Value too high. Please use your AOH gain settings!"<<endl;
          }
        } else if (shiftDown) {
          if (oldInputOffsetValue>0) {
            diagService_->reportError("Shifting down Input Offset!",DIAGINFO);
	    newInputOffsetValue=oldInputOffsetValue-1;
          } else {
            diagService_->reportError("Input Offset Value too low. Please use your AOH gain settings!",DIAGWARN);
	    //	    cout<<"Input Offset Value too low. Please use your AOH gain settings!"<<endl;
          }
        } else {
          newInputOffsetValue=int(oldInputOffsetValue-(targetBlack_[fednumber]-opticalReceiverMean)/opticalReceiverSlope_[fednumber]+0.5);
        }

        InputOffsetDAC.at(receiverIndex)=newInputOffsetValue;

	if (debug) {
	  cout<<"FED number="<<fednumber<<", optical receiver="<<opticalReceiver<<" B (mean of max, min)="
	      <<opticalReceiverMean<<" old inputoffset="<<oldInputOffsetValue<<" new inputoffset="<<newInputOffsetValue<<endl;
	}
      }
    } // optical receivers

    // Iteration over FED Channels
    for (unsigned int ichannel=0; ichannel<fedsAndChannels_[ifed].second.size(); ++ichannel) {
      unsigned int channel=fedsAndChannels_[ifed].second[ichannel];
      assert(channel>0 && channel<37);
      
      int oldInputOffsetValue=fedCard.opt_inadj[(channel-1)/12];
      int newInputOffsetValue=InputOffsetDAC.at(opticalReceiversPerFED*fednumber+(channel-1)/12);
      
      double diff =  double(targetBlack_[fednumber]) - B_Channel.at(fednumber*channelsPerFED+channel-1).mean();
      double inadj_prediction = double(newInputOffsetValue - oldInputOffsetValue)*opticalReceiverSlope_[fednumber];

      int oldChannelOffsetDACValue=fedCard.offs_dac[channel-1];
      int newChannelOffsetDACValue=oldChannelOffsetDACValue - int((diff + inadj_prediction + 0.5)/double(channelOffsetSlope_[fednumber]));

      if (newChannelOffsetDACValue<0) newChannelOffsetDACValue=0;
      if (newChannelOffsetDACValue>255) newChannelOffsetDACValue=255;

      //store change in channel offset for this iteration
      const unsigned int opto=opticalReceiversPerFED*fednumber+((channel-1)*opticalReceiversPerFED)/channelsPerFED;
      deltaChannelOffset_[fednumber][channel]=newChannelOffsetDACValue-oldChannelOffsetDACValue;
      deltaInputOffset_[fednumber][opto]=newInputOffsetValue-oldInputOffsetValue;
      //store mean black value for this iteration
      meanBlack_[fednumber][channel] = B_Channel.at(fednumber*channelsPerFED+channel-1).mean();

      ChannelOffsetDAC.at(fednumber*channelsPerFED+channel-1)=newChannelOffsetDACValue;
      if (done_[fednumber][channel].back() == false) {
	cout<<"FED number="<<fednumber<<", channel="<<channel<<" B="<<B_Channel.at(fednumber*channelsPerFED+channel-1).mean()
	    <<" old channeloffset= "<<oldChannelOffsetDACValue<<" new channeloffset= "<<newChannelOffsetDACValue<<endl;
	//cout<<"The target black was "<<targetBlack_[fednumber]<<endl; 
      }
      
      //============================== Changes by Irakli ==============================
      *(summary_long_[fednumber][channel].back())<<"Old Channel Offset DAC = "
						 <<oldChannelOffsetDACValue<<endl;
      *(summary_long_[fednumber][channel].back())<<"New Channel Offset DAC = "
						 <<newChannelOffsetDACValue<<endl;
      *(summary_long_[fednumber][channel].back())<<"New Black mean = "
	    <<B_Channel.at(fednumber*channelsPerFED+channel-1).mean()<<endl;


      if (fabs(B_Channel.at(fednumber*channelsPerFED+channel-1).mean()-targetBlack_[fednumber])>tolerance_ 
	  || oldInputOffsetValue!=newInputOffsetValue ) {
        replyString="FEDBaselineCalibrationWithPixelsIterating";
        summary_short_[fednumber][channel].push_back(0);
        done_[fednumber][channel].push_back(false);
	*(summary_long_[fednumber][channel].back())
	  <<"UB Std.dev. = "<<UB_Channel.at(fednumber*channelsPerFED+channel-1).stddev()<<endl;
	*(summary_long_[fednumber][channel].back())
	  <<"B Std.dev. = "<<B_Channel.at(fednumber*channelsPerFED+channel-1).stddev()<<endl;
      } else {
	if (done_[fednumber][channel].back() == false) 
	  cout<<"We have convergence in FED,Channel="<<fednumber<<","<<channel<<endl;
	summary_short_[fednumber][channel].push_back(1);
        done_[fednumber][channel].push_back(true);
	*(summary_long_[fednumber][channel].back())
	  <<"UB Std.dev. = "<<UB_Channel.at(fednumber*channelsPerFED+channel-1).stddev()<<endl;
	*(summary_long_[fednumber][channel].back())
	  <<"B Std.dev. = "<<B_Channel.at(fednumber*channelsPerFED+channel-1).stddev()<<endl;
      }
      
      baseline = B_Channel.at(fednumber*channelsPerFED+channel-1).mean();
      histo_BaselineVsChannel->Fill(channel,baseline);
      histo_BaselineVsChannelZoom->Fill(channel,baseline);
      histo_BaselineVsFED->Fill(fednumber,baseline);
      
    } // channel
    canvas->cd(1);
    histo_BaselineVsChannel->Draw("colz");
    canvas->cd(2);
    TLine* targetUp = new TLine(0.5,targetBaseline-tolerance_,36.5,targetBaseline-tolerance_);
    TLine* targetLow = new TLine(0.5,targetBaseline+tolerance_,36.5,targetBaseline+tolerance_);
    targetUp->SetLineColor(kMagenta); targetUp->SetLineStyle(kDashed);
    targetLow->SetLineColor(kMagenta); targetLow->SetLineStyle(kDashed);
   
    histo_BaselineVsChannelZoom->Draw("colz");						 
    targetUp->Draw("same");
    targetLow->Draw("same");
    canvas->Write();
    
    delete histo_BaselineVsChannel;
    delete histo_BaselineVsChannelZoom;
  } // feds

  canvas_summaryplot->cd();
  histo_BaselineVsFED->Draw("colz");
  canvas_summaryplot->Write();
  delete histo_BaselineVsFED;

  // Update the FED Cards
  for (unsigned int ifed=0; ifed<fedsAndChannels_.size(); ++ifed) {
    unsigned int fednumber=fedsAndChannels_[ifed].first;
    unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
    PixelFEDCard fedCard=FEDInterface_[vmeBaseAddress]->getPixelFEDCard();

    for (unsigned int opticalReceiver=0;opticalReceiver<3;++opticalReceiver) {
      unsigned int receiverIndex=opticalReceiversPerFED*fednumber+opticalReceiver;
      if (B_OpticalReceiver_Used[receiverIndex]) {
        fedCard.opt_inadj[opticalReceiver]=InputOffsetDAC.at(receiverIndex);
      }
    }

    // Write the Channel Offset DACs, Black High, Black Low and UB thresholds into the PixelFEDCard
    for (unsigned int ichannel=0; ichannel<fedsAndChannels_[ifed].second.size(); ++ichannel) {
      unsigned int channel=fedsAndChannels_[ifed].second[ichannel];
      fedCard.offs_dac[channel-1]=ChannelOffsetDAC.at(fednumber*channelsPerFED+channel-1);

      int recommendedBlackHi=int(B_Channel.at(fednumber*channelsPerFED+channel-1).mean()+3*B_Channel.at(fednumber*channelsPerFED+channel-1).stddev()+50);
      int recommendedBlackLo=int(B_Channel.at(fednumber*channelsPerFED+channel-1).mean()-3*B_Channel.at(fednumber*channelsPerFED+channel-1).stddev()-50);
      int recommendedUblack=int(UB_Channel.at(fednumber*channelsPerFED+channel-1).mean()+3*UB_Channel.at(fednumber*channelsPerFED+channel-1).stddev()+50);
      if (recommendedBlackLo>0 && recommendedBlackHi>0 && recommendedUblack>=0 && recommendedBlackLo>recommendedUblack) {

	// Save the UB-B cut or not
        if(doCut_) {

	  // Use the middle point between UB and B as the cut 
	  int lowBCut = int(  (B_Channel.at(fednumber*channelsPerFED+channel-1).mean() +
			       UB_Channel.at(fednumber*channelsPerFED+channel-1).mean() )/2. );
	  fedCard.BlackHi[channel-1] = 1000;
	  fedCard.BlackLo[channel-1] = lowBCut;
	  fedCard.Ublack[channel-1]  = lowBCut-1;

	  // old cuts
	  //fedCard.BlackHi[channel-1]=int(B_Channel.at(fednumber*channelsPerFED+channel-1).mean()+3*B_Channel.at(fednumber*channelsPerFED+channel-1).stddev()+50);
	  //fedCard.BlackLo[channel-1]=int(B_Channel.at(fednumber*channelsPerFED+channel-1).mean()-3*B_Channel.at(fednumber*channelsPerFED+channel-1).stddev()-50);
	  //fedCard.Ublack[channel-1]=int(UB_Channel.at(fednumber*channelsPerFED+channel-1).mean()+3*UB_Channel.at(fednumber*channelsPerFED+channel-1).stddev()+50);

	} // if doCut

      } else {
        cout<<"PixelFEDSupervisor::FEDBaselineCalibrationWithPixels reports: "<<endl;
        cout<<" FED ID = "<<fednumber<<", Channel = "<<channel<<endl;
        cout<<" Recommended Black Hi = "<<recommendedBlackHi<<endl;
        cout<<" Recommended Black Lo = "<<recommendedBlackLo<<endl;
        cout<<" Recommended UB threshold = "<<recommendedUblack<<endl;
        cout<<" ... were found to be nonsensical and hence not written into the params_fed.dat"<<endl;
      }
    }

    FEDInterface_[vmeBaseAddress]->setup(fedCard);
    FEDInterface_[vmeBaseAddress]->sendResets(1);
    //FEDInterface_[vmeBaseAddress]->BaselineCorr_off();
  }

  
  if (debug) {
    cout << "Time (standard) :"<<timer.avgtime()<<endl;
    cout << "Time (skip read):"<<timerSkip.avgtime()<<endl;
  }


  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  xoap::MessageReference reply=MakeSOAPMessageReference(replyString);

  return reply;
}

void PixelFEDBaselineCalibrationNew::initializeFED(){

  setFEDModeAndControlRegister(0x0,0x30019);
	setBlackUBTrans();

}

xoap::MessageReference PixelFEDBaselineCalibrationNew::beginCalibration(xoap::MessageReference msg)
{
  cout<<"[PixelFEDBaselineCalibrationNew::beginCalibration] Entered."<<endl;
  iteration_=0;


  const PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject!=0);
  fedsAndChannels_ = tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_);

  string tolerance = tempCalibObject->parameterValue("tolerance") ;
  if (tolerance =="") tolerance_=15;
  else {
    tolerance_= atof( tolerance.c_str() );
    if (tolerance_<=0) tolerance_=15;    
  }
  cout<<"PixelFEDBaselineCalibration: convergence criterion set to +-"<<tolerance_<<endl;

  // 
  doCut_ = false;
  if ( tempCalibObject->parameterValue("SetCut") != "" ) {
    if( atof( tempCalibObject->parameterValue("SetCut").c_str()) != 0)  doCut_ = true;
    cout<<" Do the UB-B cut : "<<doCut_<<endl;
  }


  outputFile_  = new
    TFile((outputDir()+"/"+"FEDBaselineWithPixels_"+itoa(crate_)+".root").c_str(),"UPDATE",("FEDBaselineWithPixels_"+itoa(crate_)+".root").c_str());
  dirMakerFED_ = new PixelRootDirectoryMaker(fedsAndChannels_, outputFile_);
  outputFile_->cd();

  //loop over FEDs to initialize things
  for (unsigned int ifed=0; ifed<fedsAndChannels_.size(); ++ifed) {
    unsigned int fednumber=fedsAndChannels_[ifed].first;
    unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
    targetBlack_[fednumber]= (FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Nbaseln & 0xfff); // taking baseline of channel 1 for the whole FED now
    assert((FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Nbaseln & 0xfff)==
	   (FEDInterface_[vmeBaseAddress]->getPixelFEDCard().NCbaseln & 0xfff));
    assert((FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Nbaseln & 0xfff)==
	   (FEDInterface_[vmeBaseAddress]->getPixelFEDCard().SCbaseln & 0xfff));
    assert((FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Nbaseln & 0xfff)==
	   (FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Sbaseln & 0xfff));
    bool twoVpp = (FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Nadcg & 0x1); // taking peak-to-peak of channel 1 for the whole FED now
    assert((FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Nadcg==0&&
	    FEDInterface_[vmeBaseAddress]->getPixelFEDCard().NCadcg==0&&
	    FEDInterface_[vmeBaseAddress]->getPixelFEDCard().SCadcg==0&&
	    FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Sadcg==0) ||
	   (FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Nadcg==0x3f&&
	    FEDInterface_[vmeBaseAddress]->getPixelFEDCard().NCadcg==0xf&&
	    FEDInterface_[vmeBaseAddress]->getPixelFEDCard().SCadcg==0xf&&
	    FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Sadcg==0xf)
	   );
    if (twoVpp) {
      opticalReceiverSlope_[fednumber] = 200.0; //same as 1VPP!
      channelOffsetSlope_[fednumber] = 1; //compared to 1VPP value of 2
    }

    //JMT
    //for testing //screw with things
    ///        PixelFEDCard fedCard=FEDInterface_[vmeBaseAddress]->getPixelFEDCard();
//     for (unsigned int opticalReceiver=0;opticalReceiver<3;++opticalReceiver) {
//       //    unsigned int receiverIndex=opticalReceiversPerFED*fednumber+opticalReceiver;
//       fedCard.opt_inadj[opticalReceiver]=6;
//     }
///    fedCard.offs_dac[33]=50;
 ///   FEDInterface_[vmeBaseAddress]->setupFromDB(fedCard);
      ///    FEDInterface_[vmeBaseAddress]->sendResets(1);

  }



  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;

  
}

xoap::MessageReference PixelFEDBaselineCalibrationNew::endCalibration(xoap::MessageReference msg)
{
  cout << "[PixelFEDBaselineCalibrationNew::endCalibration()] Entered." << endl;

  if(outputFile_){
    outputFile_->Write();
    outputFile_->Close();
  }
  for (unsigned int ifed=0; ifed<fedsAndChannels_.size(); ++ifed) {
    unsigned int fednumber=fedsAndChannels_[ifed].first;
    unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
    PixelFEDCard fedCard=FEDInterface_[vmeBaseAddress]->getPixelFEDCard();
    fedCard.restoreBaselinAndChannelMasks();
    fedCard.restoreControlAndModeRegister();
    fedCard.writeASCII(outputDir());
  }

  // Generate file
  if(writeOutputFile) {
    ofstream summaryFile((outputDir()+"/FEDBaselineCalibration_"+itoa(crate_)+".txt").c_str());
    std::map <unsigned int, std::map<unsigned int, std::vector<unsigned int> > >::iterator 
      i_summary_short_=summary_short_.begin();
    for (;i_summary_short_!=summary_short_.end();++i_summary_short_) {
      unsigned int fednumber=i_summary_short_->first;
      std::map<unsigned int, std::vector<unsigned int> > 
	summary_FED_short=i_summary_short_->second;
      std::map<unsigned int, std::vector<unsigned int> >::iterator 
	i_summary_FED_short=summary_FED_short.begin();
      std::vector<unsigned int> summary_channel_short=i_summary_FED_short->second;
      unsigned int iter_num =summary_channel_short.size();
      for (unsigned int xiter=0; xiter<iter_num; ++xiter) {
	summaryFile<<"fed "<<fednumber<<" iteration "<<xiter<<std::endl;
	for (i_summary_FED_short=summary_FED_short.begin();
	     i_summary_FED_short!=summary_FED_short.end();++i_summary_FED_short) {
	  unsigned int channel=i_summary_FED_short->first;
	  std::vector<unsigned int> summary_channel_short=i_summary_FED_short->second;
	  summaryFile<<"Channel "<<channel<<std::endl;
	  if (summary_short_[fednumber][channel].at(xiter) == 0) 
	    {summaryFile<<"FAILURE"<<std::endl;}
	  summaryFile<<(summary_long_[fednumber][channel][xiter]->str())<<std::endl;
	}//channel
      }//iteration
    }//FED
    summaryFile.close();
  } // if write file 

  summary_short_.clear();
  summary_long_.clear();  
  iteration_=0;
  
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

