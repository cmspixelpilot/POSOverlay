// $Id: PixelFEDBaselineCalibration.cc,v 1.48 2012/10/01 22:07:36 mdunser Exp $: PixelBaselineCalibration.cc,v 1.1 
// Modify to accomadte bpix channes with very low TBM UB. d.k. 23/02/15

#include "PixelCalibrations/include/PixelFEDBaselineCalibration.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelDecodedFEDRawData.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"

#include <toolbox/convertstring.h>

#include "iomanip"

#include "TFile.h"
#include "TDirectory.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TStyle.h"
#include "TColor.h"

using namespace pos;

const unsigned int numberOfFEDs=40;
const unsigned int channelsPerFED=36;
const unsigned int opticalReceiversPerFED=3;
const unsigned int blackTolerance=15;

PixelFEDBaselineCalibration::PixelFEDBaselineCalibration(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  std::cout << "Greetings from the PixelFEDBaselineCalibration copy constructor." << std::endl;
}

xoap::MessageReference PixelFEDBaselineCalibration::execute(xoap::MessageReference msg)
{

  const bool debug = false;  

  std::vector <bool> B_OpticalReceiver_Used(numberOfFEDs*opticalReceiversPerFED,false);
  std::vector <Moments> B_Channel(numberOfFEDs*channelsPerFED), UB_Channel(numberOfFEDs*channelsPerFED);
  std::vector <double> B_Min(numberOfFEDs*opticalReceiversPerFED,1024.0), B_Max(numberOfFEDs*opticalReceiversPerFED,0.0);
  std::vector <unsigned int> ChannelOffsetDAC(numberOfFEDs*channelsPerFED);
  std::vector <unsigned int> InputOffsetDAC(numberOfFEDs*opticalReceiversPerFED);
  std::vector <unsigned int> targetBlack (40, 512);
  std::vector <bool> twoVpp (40, false);

  std::string replyString="FEDBaselineCalibrationWithPixelsDone";
 
  ++iteration_;  
  std::cout<<"iteration "<<iteration_<<std::endl;
 

  int channelOffsetSlope=2;

  double const opticalReceiverSlope1Vpp = 200.0;

  double opticalReceiverSlope=0.0;
  

  for (unsigned int ifed=0; ifed<fedsAndChannels_.size(); ++ifed) {
 
   unsigned int fednumber=fedsAndChannels_[ifed].first;
   unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);

   targetBlack[fednumber]= (FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Nbaseln & 0xfff); // taking baseline of channel 1 for the whole FED now

   //if(debug) cout<<" 1 "<<ifed<<endl;

   assert((FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Nbaseln & 0xfff)==
	  (FEDInterface_[vmeBaseAddress]->getPixelFEDCard().NCbaseln & 0xfff));
   assert((FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Nbaseln & 0xfff)==
	  (FEDInterface_[vmeBaseAddress]->getPixelFEDCard().SCbaseln & 0xfff));
   assert((FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Nbaseln & 0xfff)==
	  (FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Sbaseln & 0xfff));

   //if(debug) cout<<" 2 "<<ifed<<endl;

   twoVpp[fednumber]= (FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Nadcg & 0x1); // taking peak-to-peak of channel 1 for the whole FED now

   //if(debug) cout<<" 3 "<<ifed<<endl;

   assert((FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Nadcg==0&&
	   FEDInterface_[vmeBaseAddress]->getPixelFEDCard().NCadcg==0&&
	   FEDInterface_[vmeBaseAddress]->getPixelFEDCard().SCadcg==0&&
	   FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Sadcg==0) ||
	  (FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Nadcg==0x3f&&
	   FEDInterface_[vmeBaseAddress]->getPixelFEDCard().NCadcg==0xf&&
	   FEDInterface_[vmeBaseAddress]->getPixelFEDCard().SCadcg==0xf&&
	   FEDInterface_[vmeBaseAddress]->getPixelFEDCard().Sadcg==0xf)
	  );
   
   //if(debug) cout<<" 4 "<<ifed<<endl;

   opticalReceiverSlope=opticalReceiverSlope1Vpp;

    if (twoVpp[fednumber]) {
      opticalReceiverSlope=opticalReceiverSlope1Vpp;
      channelOffsetSlope=1;
    }

    //if(debug) cout<<" 5 "<<ifed<<endl;

    for (unsigned int ichannel=0; ichannel<fedsAndChannels_[ifed].second.size(); ++ichannel) {

      //if(debug) cout<<" 9 "<<fednumber<<endl;

      dirMakerFED_->cdDirectory(fedsAndChannels_[ifed].first,fedsAndChannels_[ifed].second[ichannel]);
      
      uint32_t buffer[pos::fifo1TranspDepth];
      unsigned int channel=fedsAndChannels_[ifed].second[ichannel];

      //if(debug) cout<<" 10 "<<channel<<" "<<fednumber<<endl;

      int status = FEDInterface_[vmeBaseAddress]->drain_transBuffer(channel, buffer);
      //if(debug) cout<<" 11 "<<channel<<" "<<fednumber<<endl;

      PixelDecodedFEDRawData decodedRawData(buffer, 100., 100., 150., 0., 100., 0., 150.);
      //if(debug) cout<<" 12 "<<channel<<" "<<fednumber<<endl;

      std::string tbmSignalFilename=outputDir()+"/FIFO1Signal_"+itoa(iteration_)+"_"+itoa(fednumber)+"_"+itoa(channel)+".gif";
      std::string tbmSignalFilenameShort="FIFO1Signal_"+itoa(iteration_)+"_"+itoa(fednumber)+"_"+itoa(channel)+".gif";
      std::string tbmSignalFilenameshort="FIFO1Signal_"+itoa(iteration_)+"_"+itoa(fednumber)+"_"+itoa(channel);

      decodedRawData.drawToFile(tbmSignalFilename,tbmSignalFilenameshort,150);
      //if(debug) cout<<" 13 "<<channel<<" "<<fednumber<<endl;

      std::stringstream *tbmSignal=new std::stringstream("~");

      if (status<0){
        diagService_->reportError("Could not drain FIFO 1 in transparent mode!!",DIAGERROR);
        std::cout<<"[PixelFEDBaselineCalibration::execute] Could not drain FIFO 1 in transparent mode!"<<std::endl;
      }

      int baselinecorrection=FEDInterface_[vmeBaseAddress]->get_BaselineCorr(channel);

      if (baselinecorrection!=0) {
        std::cout<<"[PixelFEDBaselineCalibration::execute] Baseline Adjust for Channel "<<channel<<" is non-zero: "<<baselinecorrection<<std::endl;
        assert(baselinecorrection==0);
      }

      unsigned int i=TransparentDataStart(buffer,fednumber,channel);
      unsigned int opto=opticalReceiversPerFED*fednumber+((channel-1)*opticalReceiversPerFED)/channelsPerFED;
      unsigned int channelindex=channelsPerFED*fednumber+channel-1;
     
      for (unsigned int j=1;j<i;++j) {
        unsigned int ADC=((buffer[j] & 0xffc00000) >> 22);
        B_Channel.at(channelindex).push_back(ADC);
        //if (debug) {                                                                // Reducing printout to console
	//std::cout<<"[PixelFEDBaselineCalibration::execute] Channel= "<<channel<<", slot="<<j<<", ADC="<<ADC<<std::endl;
        //}
      }
      
      if (debug) {                                                                  // Reducing printout to console
        cout<<"[PixelFEDBaselineCalibration::execute] UB="<<((buffer[i] & 0xffc00000) >> 22)<<endl;
        cout<<"[PixelFEDBaselineCalibration::execute] UB="<<((buffer[i+1] & 0xffc00000) >> 22)<<endl;
        cout<<"[PixelFEDBaselineCalibration::execute] UB="<<((buffer[i+2] & 0xffc00000) >> 22)<<endl;
      }
      UB_Channel.at(channelindex).push_back(((buffer[i] & 0xffc00000) >> 22));
      UB_Channel.at(channelindex).push_back(((buffer[i+1] & 0xffc00000) >> 22));
      UB_Channel.at(channelindex).push_back(((buffer[i+2] & 0xffc00000) >> 22));

      double B_Channel_mean=B_Channel.at(channelindex).mean();

      B_OpticalReceiver_Used.at(opto)=true;

      if (B_Channel_mean>B_Max.at(opto)) B_Max.at(opto)=B_Channel_mean;
      if (B_Channel_mean<B_Min.at(opto)) B_Min.at(opto)=B_Channel_mean;

      *tbmSignal<<"<img src=\""<<tbmSignalFilename.substr(outputDir().size())<<"\"></img>";

      summary_long_[fednumber][channel].push_back(tbmSignal);
     
      //if(debug) cout<<" 14 "<<channel<<" "<<fednumber<<endl;

    }//channel

    //if(debug) cout<<" 15 "<<fednumber<<endl;


  }//feds
  
  
  //if(debug) cout<<" 2 "<<endl;

 
  outputFile_->cd();
  
  TStyle* plainStyle = new TStyle("Plain", "a plain style");
  plainStyle->SetOptStat(1111); // Suppress statistics box.
  plainStyle->cd();

 
  std::string SummaryPlotName = "Baseline_iteration"+itoa(iteration_);
  
  TCanvas* canvas_summaryplot =new TCanvas((SummaryPlotName+"_Canvas").c_str(),(SummaryPlotName+"_Canvas").c_str(),800,600);
  TH2F* histo_BaselineVsFED = new TH2F(SummaryPlotName.c_str(),SummaryPlotName.c_str(),40,0.5,40.5,2048,0,1023);
 

  for (unsigned int ifed=0; ifed<fedsAndChannels_.size(); ++ifed) {
    unsigned int fednumber=fedsAndChannels_[ifed].first;
    unsigned long vmeBaseAddress=theFEDConfiguration_->VMEBaseAddressFromFEDNumber(fednumber);
    PixelFEDCard fedCard=FEDInterface_[vmeBaseAddress]->getPixelFEDCard();
    
    double baseline=0.0;
    double targetBaseline=targetBlack[fednumber];
    std::string FEDName="Baseline_iteration_"+itoa(iteration_)+"_FED"+itoa(fednumber);
    std::string FEDName1="Baseline_iteration_"+itoa(iteration_)+"_FED"+itoa(fednumber)+"_Zoom";
    
    std::string fedName = "FED"+itoa(fednumber);
    dirMakerFED_->cdDirectory(fedName);
    TCanvas* canvas=new TCanvas((FEDName+"_Canvas").c_str(),FEDName.c_str(),800,1600);
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
	int channelWhichDrivesTheChange = 0;
        std::vector <unsigned int> channelsInThisFED=fedsAndChannels_[ifed].second;
        for (unsigned int ichannel=0;ichannel<channelsInThisFED.size();++ichannel) {
          if ((unsigned int)((channelsInThisFED[ichannel]-1)/12)==opticalReceiver) {
            if (fedCard.offs_dac[channelsInThisFED[ichannel]-1]==0) 
	      {shiftDown=true; channelWhichDrivesTheChange=ichannel;}
            else if (fedCard.offs_dac[channelsInThisFED[ichannel]-1]==255) 
	      {shiftUp=true; channelWhichDrivesTheChange=ichannel;}
          }
        }

        if (shiftUp && shiftDown) {
          diagService_->reportError("[PixelFEDBaselineCalibration::execute] Cannot correct this situation using Optical Input Offsets and Channel Offsets! Please use your AOH gain settings!",DIAGWARN);
          std::cout<<"Cannot correct this situation using Optical Input Offsets and Channel Offsets! Please use your AOH gain settings!"<<std::endl;

        } else if (shiftUp) {
          if (oldInputOffsetValue<15){
            //diagService_->reportError("Shifting up Input Offset!",DIAGINFO);
            newInputOffsetValue=oldInputOffsetValue+1;
            std::cout<<"Shifting up Input Offset for fed "<<fednumber<<" optical receiver "<<opticalReceiver
		     <<" channel "<<channelWhichDrivesTheChange<<" new value "<< newInputOffsetValue
		     <<std::endl;
          } else {
            diagService_->reportError("Input Offset Value too high. Please use your AOH gain settings!",DIAGWARN);
            std::cout<<"Input Offset Value too high. Please use your AOH gain settings!"<<std::endl;
          }

        } else if (shiftDown) {
          if (oldInputOffsetValue>0) {
            //diagService_->reportError("Shifting down Input Offset!",DIAGINFO);
            //	std::cout<<"Shifting down Input Offset!"<<std::endl;
            newInputOffsetValue=oldInputOffsetValue-1;
            std::cout<<"Shifting down Input Offset for fed "<<fednumber<<" optical receiver "<<opticalReceiver
		     <<" channel "<<channelWhichDrivesTheChange<<" new value "<< newInputOffsetValue
		     <<std::endl;
          } else {
            diagService_->reportError("Input Offset Value too low. Please use your AOH gain settings!",DIAGWARN);
            //
            std::cout<<"Input Offset Value too low. Please use your AOH gain settings!"<<std::endl;
          }
        } else {
          newInputOffsetValue=int(oldInputOffsetValue-(targetBlack[fednumber]-opticalReceiverMean)/opticalReceiverSlope+0.5);
        }

        InputOffsetDAC.at(receiverIndex)=newInputOffsetValue;
	
	if (debug) {
	  std::cout<<"FED number="<<fednumber<<", optical receiver="<<opticalReceiver<<" B (mean of max, min)="<<opticalReceiverMean<<" old inputoffset="<<oldInputOffsetValue<<" new inputoffset="<<newInputOffsetValue<<std::endl;
	}
	
      }
    } // optical receivers

    

    // Iteration over FED Channels
    for (unsigned int ichannel=0; ichannel<fedsAndChannels_[ifed].second.size(); ++ichannel) {
      unsigned int channel=fedsAndChannels_[ifed].second[ichannel];

      assert(channel>0 && channel<37);

      int oldInputOffsetValue=fedCard.opt_inadj[(channel-1)/12];
      int newInputOffsetValue=InputOffsetDAC.at(opticalReceiversPerFED*fednumber+(channel-1)/12);

      int oldChannelOffsetDACValue=fedCard.offs_dac[channel-1];
      int newChannelOffsetDACValue=oldChannelOffsetDACValue - int ((targetBlack[fednumber]-B_Channel.at(fednumber*channelsPerFED+channel-1).mean())+(newInputOffsetValue - oldInputOffsetValue)*opticalReceiverSlope+0.5)/channelOffsetSlope;

      if (newChannelOffsetDACValue<0) newChannelOffsetDACValue=0;
      if (newChannelOffsetDACValue>255) newChannelOffsetDACValue=255;

      ChannelOffsetDAC.at(fednumber*channelsPerFED+channel-1)=newChannelOffsetDACValue;

      if (debug) {
	std::cout<<"FED number="<<fednumber<<", channel="<<channel<<" B=" <<
	  B_Channel.at(fednumber*channelsPerFED+channel-1).mean()<<" old channeloffset= "<<oldChannelOffsetDACValue<<" new channeloffset= "<<newChannelOffsetDACValue<<std::endl;
      }

//============================== Changes by Irakli ==============================
      *(summary_long_[fednumber][channel].back())<<"<br>Old Channel Offset DAC = "<<oldChannelOffsetDACValue<<"<br/>"<<std::endl;
//      *(summary_long_[fednumber][channel].back())<<"Old Channel Offset DAC = "<<oldChannelOffsetDACValue<<"<br/>"<<std::endl;
      *(summary_long_[fednumber][channel].back())<<"New Channel Offset DAC = "<<newChannelOffsetDACValue<<"<br/>"<<std::endl;
      *(summary_long_[fednumber][channel].back())<<"New Black mean = "<<B_Channel.at(fednumber*channelsPerFED+channel-1).mean()<<"<br/>"<<std::endl;

      //std::cout<<"The target black was "<<targetBlack[fednumber]<<endl; 


      //if (fabs(B_Channel.at(fednumber*channelsPerFED+channel-1).mean()-targetBlack[fednumber])>10) {
      if (fabs(B_Channel.at(fednumber*channelsPerFED+channel-1).mean()-targetBlack[fednumber])>blackTolerance ) {
        replyString="FEDBaselineCalibrationWithPixelsIterating";
        summary_short_[fednumber][channel].push_back(0);
	*(summary_long_[fednumber][channel].back())<<"UB Std.dev. = "<<UB_Channel.at(fednumber*channelsPerFED+channel-1).stddev()<<"<br>"<<endl;
	*(summary_long_[fednumber][channel].back())<<"B Std.dev. = "<<B_Channel.at(fednumber*channelsPerFED+channel-1).stddev()<<"<br>"<<endl;

	std::cout<<"FED/channel = "<<fednumber<<"/"<<channel
		 <<" Black mean is not within "<<blackTolerance<<" of "
		 <<targetBlack[fednumber]<<" "
		 <<B_Channel.at(fednumber*channelsPerFED+channel-1).mean()
		 <<std::endl;

      } else {

        summary_short_[fednumber][channel].push_back(1);
	*(summary_long_[fednumber][channel].back())<<"UB Std.dev. = "<<UB_Channel.at(fednumber*channelsPerFED+channel-1).stddev()<<"<br>"<<endl;
	*(summary_long_[fednumber][channel].back())<<"B Std.dev. = "<<B_Channel.at(fednumber*channelsPerFED+channel-1).stddev()<<"<br>"<<endl;
	
      }

      baseline = B_Channel.at(fednumber*channelsPerFED+channel-1).mean();
      histo_BaselineVsChannel->Fill(channel,baseline);
      histo_BaselineVsChannelZoom->Fill(channel,baseline);
      histo_BaselineVsFED->Fill(fednumber,baseline);

    } // channel

    canvas->cd(1);
    histo_BaselineVsChannel->Draw("colz");
    
    canvas->cd(2);
    TLine* targetUp  = new TLine(0.5,targetBaseline-blackTolerance,36.5,targetBaseline-blackTolerance);
    TLine* targetLow = new TLine(0.5,targetBaseline+blackTolerance,36.5,targetBaseline+blackTolerance);
    targetUp->SetLineColor(kMagenta); targetUp->SetLineStyle(kDashed);
    targetLow->SetLineColor(kMagenta); targetLow->SetLineStyle(kDashed);
    
    histo_BaselineVsChannelZoom->Draw("colz");
    targetUp->Draw("same");
    targetLow->Draw("same");
    canvas->Write();
    
    delete histo_BaselineVsChannel;
    delete histo_BaselineVsChannelZoom;
  } 

  outputFile_->cd();
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

	// modified to take care of the bpix very low TBM ub 

	// Use the middle point between UB and B as the cut
	float ub = UB_Channel.at(fednumber*channelsPerFED+channel-1).mean();
	// nomal case when ub=150
	int lowBCut = int(  (B_Channel.at(fednumber*channelsPerFED+channel-1).mean() + ub )/2. );
	if(ub<74.) { // for special channels with very low ub
	  std::cout<<" FED ID = "<<fednumber<<", Channel = "<<channel
		   <<" Low UB "<< ub <<" make the cut asymmetric "<<lowBCut;
	  lowBCut = int( (B_Channel.at(fednumber*channelsPerFED+channel-1).mean())*0.45 + ub*0.55 );
	  std::cout<<" -> "<<lowBCut<<std::endl;
	}

	//fedCard.BlackHi[channel-1] = recommendedBlackHi;
        //fedCard.BlackLo[channel-1] = recommendedBlackLo;
	//fedCard.Ublack[channel-1]  = recommendedUblack;

        fedCard.BlackHi[channel-1] = 1000;
        fedCard.BlackLo[channel-1] = lowBCut;
        fedCard.Ublack[channel-1]  = lowBCut-1;

      } else {
        std::cout<<"PixelFEDSupervisor::FEDBaselineCalibrationWithPixels reports: "<<std::endl;
        std::cout<<" FED ID = "<<fednumber<<", Channel = "<<channel<<std::endl;
        std::cout<<" Recommended Black Hi = "<<recommendedBlackHi<<std::endl;
        std::cout<<" Recommended Black Lo = "<<recommendedBlackLo<<std::endl;
        std::cout<<" Recommended UB threshold = "<<recommendedUblack<<std::endl;
        std::cout<<" ... were found to be nonsensical and hence not written into the params_fed.dat"<<std::endl;
      }
    }

    FEDInterface_[vmeBaseAddress]->setupFromDB(fedCard);
    VMEPtr_[vmeBaseAddress]->write("LRES",0x80000000);
    //FEDInterface_[vmeBaseAddress]->BaselineCorr_off();
  }

 
  xoap::MessageReference reply=MakeSOAPMessageReference(replyString);

  return reply;
}

void PixelFEDBaselineCalibration::initializeFED(){

  setFEDModeAndControlRegister(0x0,0x30019);
	setBlackUBTrans();

}

xoap::MessageReference PixelFEDBaselineCalibration::beginCalibration(xoap::MessageReference msg)
{
  std::cout<<"[PixelFEDBaselineCalibration::beginCalibration] Entered."<<std::endl;
  iteration_=0;
  const PixelCalibConfiguration* tempCalibObject=dynamic_cast<PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject!=0);
  fedsAndChannels_ = tempCalibObject->fedCardsAndChannels(crate_, theNameTranslation_, theFEDConfiguration_, theDetectorConfiguration_);

  std::cout << "Output filename:"<<(outputDir()+"/"+"FEDBaselineWithPixels_"+itoa(crate_)+".root").c_str()<<std::endl;

  outputFile_  = new TFile((outputDir()+"/"+"FEDBaselineWithPixels_"+itoa(crate_)+".root").c_str(),"UPDATE",("FEDBaselineWithPixels_"+itoa(crate_)+".root").c_str());
  dirMakerFED_ = new PixelRootDirectoryMaker(fedsAndChannels_, outputFile_);
  outputFile_->cd();

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;

  
}

xoap::MessageReference PixelFEDBaselineCalibration::endCalibration(xoap::MessageReference msg)
{
  std::cout << "[PixelFEDBaselineCalibration::endCalibration()] Entered." << std::endl;

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
  
//===================================================================================================
//================================== Changes by IRAKLI ==============================================
//===================================================================================================
  // Generate formatted HTML file
  ofstream summaryFile((outputDir()+"/FEDBaselineCalibration_"+itoa(crate_)+".html").c_str());
  summaryFile <<"<html>"<<std::endl;
  summaryFile <<"<head>"<<std::endl;
  summaryFile <<" <script language=\"JavaScript\">"<<std::endl;
  summaryFile <<"  function toggle(fed,num) {"<<std::endl;
  summaryFile <<"   var i = 0;"<<std::endl;
  summaryFile <<"   var elem = document.getElementById(\"fed_\"+fed+\"_iteration_\"+i);"<<std::endl;
  summaryFile <<"   while (elem) {"<<std::endl;
  summaryFile <<"    if (i==num) elem.style.display = (elem.style.display==\"none\")?(\"\"):(\"none\");"<<std::endl;
  summaryFile <<"    i++;"<<std::endl;
  summaryFile <<"    elem = document.getElementById(\"fed_\"+fed+\"_iteration_\"+i);"<<std::endl;
  summaryFile <<"    }}"<<std::endl;
  summaryFile <<" </script>"<<std::endl;
  
  summaryFile <<"</head>"<<std::endl;
  summaryFile <<"<body>"<<std::endl;
//===============================================================
  std::map <unsigned int, std::map<unsigned int, std::vector<unsigned int> > >::iterator i_summary_short_=summary_short_.begin();

  for (;i_summary_short_!=summary_short_.end();++i_summary_short_)
	{
	unsigned int fednumber=i_summary_short_->first;
	std::map<unsigned int, std::vector<unsigned int> > summary_FED_short=i_summary_short_->second;

//===============================================================
        summaryFile<<"<h2>Pixel FED Board # "<<fednumber<<" </h2>"<<std::endl;
        summaryFile<<"<table border=0>"<<std::endl;
        summaryFile<<" <tr>"<<std::endl;;
        summaryFile<<"  <td width=100 valign=top>"<<std::endl;;
        summaryFile<<"   <table border=0>"<<std::endl;
        std::map<unsigned int, std::vector<unsigned int> >::iterator i_summary_FED_short=summary_FED_short.begin();
//===============================================================
        std::vector<unsigned int> summary_channel_short=i_summary_FED_short->second;
        unsigned int iter_num =summary_channel_short.size();
        for (unsigned int iteration=0; iteration<iter_num; ++iteration)
                {
		summaryFile<<"<tr><td width=100><a href=\"javascript:toggle(\'"<<fednumber<<"\',\'"<<iteration<<"\')\">Iteration "<<iteration<<"</a></td></tr>"<<std::endl;
                }
//===============================================================
        summaryFile<<" </table>"<<std::endl;
        summaryFile<<" </td>"<<std::endl;
        for (unsigned int xiter=0; xiter<iter_num; ++xiter)
                {
		summaryFile<<"<td id=\"fed_"<<fednumber<<"_iteration_"<<xiter<<"\" style=\"display:none\">"<<std::endl;
                summaryFile<<"<div style=\"overflow-y:auto; height:600\">"<<std::endl;
                summaryFile<<"<table border=1>"<<std::endl;
                for (i_summary_FED_short=summary_FED_short.begin();i_summary_FED_short!=summary_FED_short.end();++i_summary_FED_short)
                        {
                        unsigned int channel=i_summary_FED_short->first;
                        std::vector<unsigned int> summary_channel_short=i_summary_FED_short->second;
                        summaryFile<<"<tr><td align=\"center\">Channel "<<channel<<std::endl;
                        if (summary_short_[fednumber][channel].at(xiter) == 0) {summaryFile<<"FAILURE"<<std::endl;}
                        summaryFile<<"</td><td>"<<(summary_long_[fednumber][channel][xiter]->str())<<"</td></tr>"<<std::endl;
                        }//channel
                summaryFile<<"</table>"<<std::endl;
                summaryFile<<"</div>"<<std::endl;
                summaryFile<<"</td>"<<std::endl;
                }//iteration

        summaryFile<<" </td></tr>"<<std::endl;
        summaryFile<<"</table>"<<std::endl;
        }//FED

  summaryFile<<"</body>"<<std::endl;
  summaryFile<<"</html>"<<std::endl;

  summaryFile.close();

  summary_short_.clear();
  summary_long_.clear();  
  
  iteration_=0;
  
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;

 
}

