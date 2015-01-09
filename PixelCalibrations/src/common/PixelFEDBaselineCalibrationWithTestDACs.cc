// Modified by Jennifer Vaughan 2007/06/01
// $Id: PixelBaselineCalibrationWithTestDACs.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelFEDBaselineCalibrationWithTestDACs.h"
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelFEDDataTypes.h"
#include "PixelUtilities/PixelFEDDataTools/include/PixelDecodedFEDRawData.h"

#include <toolbox/convertstring.h>

#include "iomanip"

using namespace pos;

const unsigned int numberOfFEDs=40;
const unsigned int channelsPerFED=36;
const unsigned int opticalReceiversPerFED=3;

//PixelFEDBaselineCalibrationWithTestDACs::PixelFEDBaselineCalibrationWithTestDACs() : PixelFEDCalibrationBase()
//{
//  std::cout << "Greetings from the PixelFEDBaselineCalibrationWithTestDACs default constructor." << std::endl;
//}

PixelFEDBaselineCalibrationWithTestDACs::PixelFEDBaselineCalibrationWithTestDACs(const PixelFEDSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelFEDCalibrationBase(tempConfiguration,*mySOAPCmdr)
{
  std::cout << "Greetings from the PixelFEDBaselineCalibrationWithTestDACs copy constructor." << std::endl;
}

xoap::MessageReference PixelFEDBaselineCalibrationWithTestDACs::execute(xoap::MessageReference msg)
{

  const bool debug=true;
  const double opticalReceiverSlope = 100.0;

  std::vector <bool> B_OpticalReceiver_Used(numberOfFEDs*opticalReceiversPerFED,false);
  std::vector <Moments> B_Channel(numberOfFEDs*channelsPerFED), UB_Channel(numberOfFEDs*channelsPerFED);
  std::vector <double> B_Min(numberOfFEDs*opticalReceiversPerFED,1024.0), B_Max(numberOfFEDs*opticalReceiversPerFED,0.0);
  std::vector <unsigned int> ChannelOffsetDAC(numberOfFEDs*channelsPerFED);
  std::vector <unsigned int> InputOffsetDAC(numberOfFEDs*opticalReceiversPerFED);
  std::vector <unsigned int> targetBlack (40, 450);

  static std::map <unsigned int, std::map<unsigned int, std::vector<std::stringstream*> > > summary_long;
  static std::map <unsigned int, std::map<unsigned int, std::vector<unsigned int> > > summary_short;
  static unsigned int iteration=0;
  //               fednumber              channel         iteration  summary     

  std::string replyString="FEDBaselineCalibrationWithTestDACsDone";
  ++iteration;

  for (FEDInterfaceMap::iterator iFED=FEDInterface_.begin();iFED!=FEDInterface_.end();++iFED) {

    unsigned int fednumber=theFEDConfiguration_->FEDNumberFromCrateAndVMEBaseAddress(crate_, iFED->first);

    targetBlack[fednumber]=450; // hardcode baseline

    std::set <unsigned int> channels=vmeBaseAddressAndFEDNumberAndChannels_[make_pair(iFED->first, fednumber)];
    std::set <unsigned int>::iterator ichannel;
    for (ichannel=channels.begin(); ichannel!=channels.end(); ++ichannel) {
      
      unsigned int channel=*ichannel;

      uint32_t buffer1[pos::fifo1TranspDepth], buffer[pos::fifo1TranspDepth-13];
      int status=iFED->second->drain_transBuffer(channel, buffer1);
      for (unsigned int i=0; i<pos::fifo1TranspDepth-13; ++i) {
        buffer[i]=buffer1[i+13];
      }
      if (status!=(int)pos::fifo1TranspDataLength){
	diagService_->reportError("Could not drain FIFO 1 in transparent mode!!",DIAGERROR);
	std::cout<<"Could not drain FIFO 1 in transparent mode. Data Length was returned as="<<status<<"!!"<<endl;
      }

      PixelDecodedFEDRawData decodedRawData(buffer, 100., 100., 150., 0., 100., 0., 150.);
      std::string tbmSignalFilename=outputDir()+"/FIFO1Signal_"+itoa(iteration)+"_"+itoa(fednumber)+"_"+itoa(channel)+".gif";
      decodedRawData.drawToFile(tbmSignalFilename);
      std::stringstream *tbmSignal=new std::stringstream("~");      

      int baselinecorrection=iFED->second->get_BaselineCorr(channel);
      //cout<<"Baseline Adjust for Channel "<<channel<<" is "<<baselinecorrection<<endl;
      assert(baselinecorrection==0);

      unsigned int i=TransparentDataStart(buffer);
      unsigned int opto=opticalReceiversPerFED*fednumber+((channel-1)*opticalReceiversPerFED)/channelsPerFED;
      unsigned int channelindex=channelsPerFED*fednumber+channel-1;

      for (unsigned int j=1;j<i;++j) {
	unsigned int ADC=((buffer[j] & 0xffc00000) >> 22);
	B_Channel.at(channelindex).push_back(ADC);
	if (debug&&channel<12) {cout<<"FED "<<fednumber<<", Channel= "<<channel<<", ADC["<<j<<"]="<<ADC<<endl;} // Reducing printout to console
      }
      if (debug) {                                                                  // Reducing printout to console
	cout<<"UB="<<((buffer[i] & 0xffc00000) >> 22)<<endl;
	cout<<"UB="<<((buffer[i+1] & 0xffc00000) >> 22)<<endl;
	cout<<"UB="<<((buffer[i+2] & 0xffc00000) >> 22)<<endl;
      }

      UB_Channel.at(channelindex).push_back(((buffer[i] & 0xffc00000) >> 22));
      UB_Channel.at(channelindex).push_back(((buffer[i+1] & 0xffc00000) >> 22));
      UB_Channel.at(channelindex).push_back(((buffer[i+2] & 0xffc00000) >> 22));

      double B_Channel_mean=B_Channel.at(channelindex).mean();

      B_OpticalReceiver_Used.at(opto)=true;

      if (B_Channel_mean>B_Max.at(opto)) B_Max.at(opto)=B_Channel_mean;
      if (B_Channel_mean<B_Min.at(opto)) B_Min.at(opto)=B_Channel_mean;

      *tbmSignal<<"<a href=\""<<tbmSignalFilename.substr(outputDir().size())<<"\" target=\"_blank\">";
      *tbmSignal<<" <img src=\""<<tbmSignalFilename.substr(outputDir().size())<<"\" width=\"200\" height=\"200\"></img> <br/>";
      *tbmSignal<<"</a>"<<std::endl;

      summary_long[fednumber][channel].push_back(tbmSignal);

    }//channels
  }//feds

  for (FEDInterfaceMap::iterator iFED=FEDInterface_.begin();iFED!=FEDInterface_.end();++iFED) {

    unsigned int fednumber=theFEDConfiguration_->FEDNumberFromCrateAndVMEBaseAddress(crate_, iFED->first);
    PixelFEDCard fedCard=iFED->second->getPixelFEDCard();

    for (unsigned int opticalReceiver=0;opticalReceiver<3;++opticalReceiver) {
      unsigned int receiverIndex=opticalReceiversPerFED*fednumber+opticalReceiver;
      if (B_OpticalReceiver_Used.at(receiverIndex)) {
	int oldInputOffsetValue=fedCard.opt_inadj[opticalReceiver];
	int newInputOffsetValue=oldInputOffsetValue;
	double opticalReceiverMean=(B_Min.at(receiverIndex)+B_Max.at(receiverIndex))/2.0;
	
	bool shiftUp=false, shiftDown=false;
	std::set <unsigned int> channels=vmeBaseAddressAndFEDNumberAndChannels_[make_pair(iFED->first, fednumber)];
	std::set <unsigned int>::iterator ichannel;
	for (ichannel=channels.begin(); ichannel!=channels.end(); ++ichannel) {      
	  unsigned int channel=*ichannel;
	  if ((channel/12)==opticalReceiver) {
	    if (fedCard.offs_dac[channel-1]==0) shiftDown=true;
	    else if (fedCard.offs_dac[channel-1]==255) shiftUp=true;
	  }
	}

	if (shiftUp && shiftDown) {
	  diagService_->reportError("Cannot this situation using Optical Input Offsets and Channel Offsets! Please use your AOH gain settings!",DIAGWARN);
	  std::cout<<"Cannot correct this situation using Optical Input Offsets and Channel Offsets! Please use your AOH gain settings!"<<std::endl;
	} else if (shiftUp) {
	  if (oldInputOffsetValue<15){
	    diagService_->reportError("Shifting up Input Offset!",DIAGINFO);
	    std::cout<<"Shifting up Input Offset!"<<std::endl;
	    newInputOffsetValue=oldInputOffsetValue+1;
	  } else {
	    diagService_->reportError("Input Offset Value too high. Please use your AOH gain settings!",DIAGWARN);
	    std::cout<<"Input Offset Value too high. Please use your AOH gain settings!"<<std::endl;
	  }
	} else if (shiftDown) {
	  if (oldInputOffsetValue>0) {
	    diagService_->reportError("Shifting down Input Offset!",DIAGINFO);
	    std::cout<<"Shifting down Input Offset!"<<std::endl;
	    newInputOffsetValue=oldInputOffsetValue-1;
	  } else {
	    diagService_->reportError("Input Offset Value too low. Please use your AOH gain settings!",DIAGWARN);
	    std::cout<<"Input Offset Value too low. Please use your AOH gain settings!"<<std::endl;
	  }
	} else {
	  //newInputOffsetValue=int(oldInputOffsetValue-(targetBlack[fednumber]-opticalReceiverMean)/opticalReceiverSlope+0.5);
          newInputOffsetValue=oldInputOffsetValue-int((targetBlack[fednumber]-opticalReceiverMean)/opticalReceiverSlope);
	}

	InputOffsetDAC.at(receiverIndex)=newInputOffsetValue;

	if (debug) {
	  std::cout<<"FED number="<<fednumber<<", optical receiver="<<opticalReceiver<<" B (mean of max, min)=" <<
	    opticalReceiverMean<<" old inputoffset="<<oldInputOffsetValue
		   <<" new inputoffset="<<newInputOffsetValue<<std::endl;
	}
      }
    } // optical receivers

    std::set <unsigned int> channels=vmeBaseAddressAndFEDNumberAndChannels_[make_pair(iFED->first, fednumber)];
    std::set <unsigned int>::iterator ichannel;
    for (ichannel=channels.begin(); ichannel!=channels.end(); ++ichannel) {      

      unsigned int channel=*ichannel;

      int oldInputOffsetValue=fedCard.opt_inadj[(channel-1)/12];
      int newInputOffsetValue=InputOffsetDAC.at(opticalReceiversPerFED*fednumber+(channel-1)/12);

      int oldChannelOffsetDACValue=fedCard.offs_dac[channel-1];
      int newChannelOffsetDACValue=oldChannelOffsetDACValue - int ((targetBlack[fednumber]-B_Channel.at(fednumber*channelsPerFED+channel-1).mean())+(newInputOffsetValue - oldInputOffsetValue)*opticalReceiverSlope+0.5)/2;

      if (newChannelOffsetDACValue<0) newChannelOffsetDACValue=0;
      if (newChannelOffsetDACValue>255) newChannelOffsetDACValue=255;

      ChannelOffsetDAC.at(fednumber*channelsPerFED+channel-1)=newChannelOffsetDACValue;

      if (debug) {
	std::cout<<"FED number="<<fednumber<<", channel="<<channel<<" B=" <<
	  B_Channel.at(fednumber*channelsPerFED+channel-1).mean()<<" old channeloffset= "<<oldChannelOffsetDACValue
		 <<" new channeloffset= "<<newChannelOffsetDACValue<<std::endl;
      }
      
      *(summary_long[fednumber][channel].back())<<"Old Channel Offset DAC = "<<oldChannelOffsetDACValue<<"<br/>"<<std::endl;
      *(summary_long[fednumber][channel].back())<<"New Channel Offset DAC = "<<newChannelOffsetDACValue<<"<br/>"<<std::endl;
      *(summary_long[fednumber][channel].back())<<"New Black mean = "<<B_Channel.at(fednumber*channelsPerFED+channel-1).mean()<<"<br/>"<<std::endl;

      std::cout<<"The target black was "<<targetBlack[fednumber]<<endl; 

      if (fabs(B_Channel.at(fednumber*channelsPerFED+channel-1).mean()-targetBlack[fednumber])>8) {
	replyString="FEDBaselineCalibrationWithTestDACsIterating";
	summary_short[fednumber][channel].push_back(0);
	*(summary_long[fednumber][channel].back())<<"Black mean is not within 8 ADC of 512 <br/>"<<std::endl;
      } else {
	summary_short[fednumber][channel].push_back(1);
	*(summary_long[fednumber][channel].back())<<"Black mean has converged within 8 ADC of 512 <br/>"<<std::endl;
      }
    } // channels
  } // feds

  for (FEDInterfaceMap::iterator iFED=FEDInterface_.begin();iFED!=FEDInterface_.end();++iFED) {

    unsigned int fednumber=theFEDConfiguration_->FEDNumberFromCrateAndVMEBaseAddress(crate_, iFED->first);
    PixelFEDCard fedCard=iFED->second->getPixelFEDCard();

    for (unsigned int opticalReceiver=0;opticalReceiver<3;++opticalReceiver) {
      unsigned int receiverIndex=opticalReceiversPerFED*fednumber+opticalReceiver;
      if (B_OpticalReceiver_Used[receiverIndex]) {
	fedCard.opt_inadj[opticalReceiver]=InputOffsetDAC.at(receiverIndex);
      }
    }

    // Write the Channel Offset DACs, Black High, Black Low and UB thresholds into the PixelFEDCard
    std::set <unsigned int> channels=vmeBaseAddressAndFEDNumberAndChannels_[make_pair(iFED->first, fednumber)];
    std::set <unsigned int>::iterator ichannel;
    for (ichannel=channels.begin(); ichannel!=channels.end(); ++ichannel) {      

      unsigned int channel=*ichannel;

      fedCard.offs_dac[channel-1]=ChannelOffsetDAC.at(fednumber*channelsPerFED+channel-1);

      int recommendedBlackHi=int(B_Channel.at(fednumber*channelsPerFED+channel-1).mean()+3*B_Channel.at(fednumber*channelsPerFED+channel-1).stddev()+50);
      int recommendedBlackLo=int(B_Channel.at(fednumber*channelsPerFED+channel-1).mean()-3*B_Channel.at(fednumber*channelsPerFED+channel-1).stddev()-50);
      int recommendedUblack=int(UB_Channel.at(fednumber*channelsPerFED+channel-1).mean()+3*UB_Channel.at(fednumber*channelsPerFED+channel-1).stddev()+50);
      if (recommendedBlackLo>0 && recommendedBlackHi>0 && recommendedUblack>=0 &&
	  recommendedBlackLo>recommendedUblack){
	fedCard.BlackHi[channel-1]=int(B_Channel.at(fednumber*channelsPerFED+channel-1).mean()+3*B_Channel.at(fednumber*channelsPerFED+channel-1).stddev()+50);
	fedCard.BlackLo[channel-1]=int(B_Channel.at(fednumber*channelsPerFED+channel-1).mean()-3*B_Channel.at(fednumber*channelsPerFED+channel-1).stddev()-50);
	fedCard.Ublack[channel-1]=int(UB_Channel.at(fednumber*channelsPerFED+channel-1).mean()+3*UB_Channel.at(fednumber*channelsPerFED+channel-1).stddev()+50);
      } else {
	std::cout<<"PixelFEDSupervisor::FEDBaselineCalibrationWithTestDAC reports: "<<std::endl;
	std::cout<<" Recommended Black Hi = "<<recommendedBlackHi<<std::endl;
	std::cout<<" Recommended Black Lo = "<<recommendedBlackLo<<std::endl;
	std::cout<<" Recommended UB threshold = "<<recommendedUblack<<std::endl;
	std::cout<<" ... were found to be nonsensical and hence not written into the params_fed.dat"<<std::endl;
      }
    }

    iFED->second->setPixelFEDCard(fedCard);
    iFED->second->set_opto_params();
    iFED->second->set_offset_dacs();
    iFED->second->BaselineCorr_off();
  }

  if (replyString=="FEDBaselineCalibrationWithTestDACsDone") {

    std::cout<<"PixelFEDSupervisor:: Baseline Calibration With Test DACs Done!"<<std::endl;
    
    for (FEDInterfaceMap::iterator iFED=FEDInterface_.begin();iFED!=FEDInterface_.end();++iFED) {
      //unsigned int fednumber=theFEDConfiguration_->FEDNumberFromCrateAndVMEBaseAddress(crate_, iFED->first);
      PixelFEDCard fedCard=iFED->second->getPixelFEDCard();
      fedCard.restoreControlAndModeRegister();
      fedCard.writeASCII(outputDir());
    }

    // Generate formatted HTML file
    ofstream summaryFile((outputDir()+"/FEDBaselineCalibration_"+itoa(crate_)+".html").c_str());
    summaryFile<<"<html>"<<std::endl;
    summaryFile<<"<head>"<<std::endl;
    summaryFile<<" <script language=\"JavaScript\">"<<std::endl;
    summaryFile<<"  function toggle(elementID){"<<std::endl;
    summaryFile<<"   var listElementStyle=document.getElementById(elementID).style;"<<std::endl;
    summaryFile<<"   if (listElementStyle.display==\"none\"){"<<std::endl;
    summaryFile<<"    listElementStyle.display=\"block\";"<<std::endl;
    summaryFile<<"   } else {"<<std::endl;
    summaryFile<<"    listElementStyle.display=\"none\";"<<std::endl;
    summaryFile<<"   }"<<std::endl;
    summaryFile<<"  }"<<std::endl;
    summaryFile<<" </script>"<<std::endl;
    summaryFile<<"</head>"<<std::endl;
    summaryFile<<"<body>"<<std::endl;

    std::map <unsigned int, std::map<unsigned int, std::vector<unsigned int> > >::iterator i_summary_short=summary_short.begin();
    for (;i_summary_short!=summary_short.end();++i_summary_short) {
      unsigned int fednumber=i_summary_short->first;
      std::map<unsigned int, std::vector<unsigned int> > summary_FED_short=i_summary_short->second;

      summaryFile<<"<h2>Pixel FED Board # "<<fednumber<<" </h2>"<<std::endl;
      summaryFile<<"<table border=1>"<<std::endl;
      summaryFile<<" <tr>";
      summaryFile<<"  <th width=10>Channel</th>";
      summaryFile<<"  <th width=50>Iteration 1</th>";
      summaryFile<<"  <th width=50>Iteration 2</th>";
      summaryFile<<"  <th>Iteration 3</th>";
      summaryFile<<"  <th>Iteration 4</th>";
      summaryFile<<"  <th>Iteration 5</th>";
      summaryFile<<"  <th>Iteration 6</th>";
      summaryFile<<"  <th>Iteration 7</th>";
      summaryFile<<"  <th>Iteration 8</th>";
      summaryFile<<"  <th>Iteration 9</th>";
      summaryFile<<"  <th>Iteration 10</th>";
      summaryFile<<"  <th>Iteration 11</th>";
      summaryFile<<"  <th>Iteration 12</th>";
      summaryFile<<"  <th>Iteration 13</th>";
      summaryFile<<"  <th>Iteration 14</th>";
      summaryFile<<"  <th>Iteration 15</th>";
      summaryFile<<" </tr>"<<std::endl;

      std::map<unsigned int, std::vector<unsigned int> >::iterator i_summary_FED_short=summary_FED_short.begin();
      for (;i_summary_FED_short!=summary_FED_short.end();++i_summary_FED_short) {
	unsigned int channel=i_summary_FED_short->first;
	summaryFile<<" <tr>"<<std::endl;
	summaryFile<<"  <td>"<<channel<<"</td>"<<std::endl;

	std::vector<unsigned int> summary_channel_short=i_summary_FED_short->second;
	for (unsigned int iteration=0; iteration<summary_channel_short.size(); ++iteration) {
	  summaryFile<<"  <td><a href=\"javascript:toggle('FED_"<<fednumber<<"_Channel_"<<channel<<"_iteration_"<<iteration<<"')\">"<<summary_short[fednumber][channel].at(iteration)<<"</a><br/>"<<std::endl;
	  summaryFile<<"   <div id=\"FED_"<<fednumber<<"_Channel_"<<channel<<"_iteration_"<<iteration<<"\" style=\"display:none\">"<<std::endl;
	  summaryFile<<"    <font size=1>"<<(summary_long[fednumber][channel][iteration]->str())<<"</font><br/>"<<std::endl;
	  summaryFile<<"   </div>"<<std::endl;
	  summaryFile<<"  </td>"<<std::endl;
	}// iteration

      }//channel

      summaryFile<<" </tr>"<<std::endl;
      summaryFile<<"</table>"<<std::endl;
    }//FED

    summaryFile<<"</body>"<<std::endl;
    summaryFile<<"</html>"<<std::endl;

    summaryFile.close();

    summary_short.clear();
    summary_long.clear();

    iteration=0;

  }// checking for last iteration

  xoap::MessageReference reply=MakeSOAPMessageReference(replyString);

  return reply;
}

void PixelFEDBaselineCalibrationWithTestDACs::initializeFED(){

  setFEDModeAndControlRegister(0x0,0x1d);
  baselinecorr_off();
  xoap::MessageReference fillTestDACmsg=MakeSOAPMessageReference("FillTestDAC");
  fillTestDAC(fillTestDACmsg);

}

xoap::MessageReference PixelFEDBaselineCalibrationWithTestDACs::beginCalibration(xoap::MessageReference msg){

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibrationDone");
  return reply;
}

xoap::MessageReference PixelFEDBaselineCalibrationWithTestDACs::endCalibration(xoap::MessageReference msg){
  cout << "In PixelFEDBaselineCalibrationWithTestDACs::endCalibration()" << endl;
  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

