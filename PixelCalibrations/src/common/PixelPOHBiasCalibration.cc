#include <iomanip>
#include "TGraphErrors.h"
#include "TFile.h"
#include "TF1.h"

#include "PixelCalibrations/include/PixelPOHBiasCalibration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelPortCardSettingNames.h"
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"
#include "PixelFEDInterface/include/PixelPh1FEDInterface.h"

using namespace pos;

PixelPOHBiasCalibration::PixelPOHBiasCalibration(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr)
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
}


void PixelPOHBiasCalibration::beginCalibration() {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  int POHGain = -1;
  POHBiasMin      = 0;
  POHBiasNSteps   = 15;
  POHBiasStepSize = 2;
  if (tempCalibObject->parameterValue("POHGain")      != "") POHGain         = atoi(tempCalibObject->parameterValue("POHGain").c_str());
  if (tempCalibObject->parameterValue("ScanMin")      != "") POHBiasMin      = atoi(tempCalibObject->parameterValue("ScanMin").c_str());
  if (tempCalibObject->parameterValue("ScanNSteps")   != "") POHBiasNSteps   = atoi(tempCalibObject->parameterValue("ScanNSteps").c_str());
  if (tempCalibObject->parameterValue("ScanStepSize") != "") POHBiasStepSize = atoi(tempCalibObject->parameterValue("ScanStepSize").c_str());

  if (POHGain == -1) {
    POHGains.assign(4, 0);
    POHGains[0] = 0;
    POHGains[1] = 1;
    POHGains[2] = 2;
    POHGains[3] = 3;
  }
  else
    POHGains.assign(1, POHGain);

  std::map<unsigned int, std::set<unsigned int> > fedsAndChannelsMap = tempCalibObject->getFEDsAndChannels(theNameTranslation_);
  for (std::vector<unsigned>::const_iterator gain_itr = POHGains.begin(); gain_itr != POHGains.end(); ++gain_itr) {
    for (std::map<unsigned int, std::set<unsigned int> >::iterator fednumber_itr = fedsAndChannelsMap.begin(); fednumber_itr != fedsAndChannelsMap.end(); ++fednumber_itr) {
      for (std::set<unsigned int>::iterator channel_itr = fednumber_itr->second.begin(); channel_itr != fednumber_itr->second.end(); channel_itr++) {
        if (*channel_itr % 2==0) { // per fiber
          int NFed = fednumber_itr->first;
          int NFiber = *channel_itr/2;
          printf("nfed %i nfiber %i\n", NFed, NFiber);
          TGraphErrors* g = rssi_v_bias[key(*gain_itr, NFed, NFiber)] = new TGraphErrors(POHBiasNSteps);
          g->SetName(TString::Format("rssi_gain%i_FED%i_fiber%i", *gain_itr, NFed, NFiber));
          g->SetTitle(TString::Format("gain %i FED %i fiber %i;POH bias value;RSSI (mA)", *gain_itr, NFed, NFiber));
          g->SetMarkerColor(1);
          g->SetMarkerStyle(21);
          g->SetMarkerSize(1);
          selected_poh_bias_values[key(*gain_itr, NFed, NFiber)] = 0;
        }
      }
    }
  }
}

bool PixelPOHBiasCalibration::execute() {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject!=0);

  for (std::vector<unsigned>::const_iterator gain_itr = POHGains.begin(); gain_itr != POHGains.end(); ++gain_itr) {
    Attribute_Vector parametersToTKFEC(1);
    parametersToTKFEC[0].name_="AOHGain";
    parametersToTKFEC[0].value_=itoa(*gain_itr);
    commandToAllTKFECCrates("SetAOHGainEnMass", parametersToTKFEC);
    //Get List of Channels
    const std::set<PixelChannel>& channelsToCalibrate = tempCalibObject->channelList();

    for(std::set<PixelChannel>::const_iterator channelsToCalibrate_itr = channelsToCalibrate.begin(); 
        channelsToCalibrate_itr != channelsToCalibrate.end(); channelsToCalibrate_itr++){
      const PixelHdwAddress& channelHdwAddress = theNameTranslation_->getHdwAddress(*channelsToCalibrate_itr);
      //Get Fed and channel number
      const unsigned int NFed = channelHdwAddress.fednumber();
      const unsigned int NChannel = channelHdwAddress.fedchannel();
     
      //2 fed channels per fiber
      if(NChannel%2==0){
        // Get Fiber; then get port card and AOH number.
        const unsigned int NFiber = NChannel/2;
        const std::pair< std::string, int > portCardAndAOH = thePortcardMap_->PortCardAndAOH(*channelsToCalibrate_itr);
        const std::string portCardName = portCardAndAOH.first; assert(portCardName!= "none");
        const int AOHNumber = portCardAndAOH.second;

        //iterate over all bias steps
        for (unsigned ibias = 0; ibias < POHBiasNSteps; ++ibias) {
	
          const int bias = POHBiasMin + ibias*POHBiasStepSize;
          SetAOHBiasToCurrentValue(portCardName, AOHNumber, bias);
          
          usleep(100000);
          
          unsigned channelkey = key(*gain_itr, NFed, NFiber);
          
          const unsigned long vmeBaseAddress = theFEDConfiguration_->VMEBaseAddressFromFEDNumber(NFed);
          const unsigned fedcrate = theFEDConfiguration_->crateFromFEDNumber(NFed);
          
          Attribute_Vector parametersToFED_read(2);
          parametersToFED_read[0].name_ = "VMEBaseAddress"; parametersToFED_read[0].value_ = itoa(vmeBaseAddress);
          parametersToFED_read[1].name_ = "Fiber";          parametersToFED_read[1].value_ = itoa(NFiber);
          
          xoap::MessageReference reply=SendWithSOAPReply(PixelFEDSupervisors_[fedcrate], "ReadRSSI", parametersToFED_read);
          Attribute_Vector returnValuesFromFED(1);
          returnValuesFromFED[0].name_="Value";
          Receive(reply, returnValuesFromFED);
          //std::cout << "hello: " << returnValuesFromFED[0].value_ << std::endl;
          
          double rssi = strtod(returnValuesFromFED[0].value_.c_str(), 0) * 1000;
          std::cout << "gain " << *gain_itr  << " bias " << bias << " FED " << NFed << " fiber " << NFiber << " rssi (mA) " << rssi << std::endl;
          rssi_v_bias[channelkey]->SetPoint(ibias, bias, rssi);
          rssi_v_bias[channelkey]->SetPointError(ibias, 0, 0.005);
        }
      }
    }
  }
  return false; //done
}



void PixelPOHBiasCalibration::endCalibration() {
  TFile file(TString::Format("%s/POHBias.root", outputDir().c_str()), "create");
  
  std::map<unsigned, TDirectory*> gain_dirs;
  std::map<unsigned, TDirectory*> fed_dirs;
  for (std::vector<unsigned>::const_iterator gain_itr = POHGains.begin(); gain_itr != POHGains.end(); ++gain_itr) {
    PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*>(theCalibObject_);
    assert(tempCalibObject!=0);
    
    //Get List of Channels                                                                                                               
    const std::set<PixelChannel>& channelsToCalibrate = tempCalibObject->channelList();
    
    for(std::set<PixelChannel>::const_iterator channelsToCalibrate_itr = channelsToCalibrate.begin();
        channelsToCalibrate_itr != channelsToCalibrate.end(); channelsToCalibrate_itr++){
      const PixelHdwAddress& channelHdwAddress = theNameTranslation_->getHdwAddress(*channelsToCalibrate_itr);
      //Get Fed and channel number                                                                                                       
      const unsigned int NFed = channelHdwAddress.fednumber();
      const unsigned int NChannel = channelHdwAddress.fedchannel();
      
      //2 fed channels per fiber                                                                                                         
      if(NChannel%2==0){
        // Get Fiber; then get port card and AOH number.                                                                                 
        const unsigned int NFiber = NChannel/2;
        const std::pair< std::string, int > portCardAndAOH = thePortcardMap_->PortCardAndAOH(*channelsToCalibrate_itr);
        const std::string portCardName = portCardAndAOH.first; assert(portCardName!= "none");
        //        if(portcard_configs_to_write.find(portCardName)==portcard_configs_to_write.end()) portcard_configs_to_write[portCardName]
        const int AOHNumber = portCardAndAOH.second;
        
        unsigned channelkey = key(*gain_itr, NFed, NFiber);

        //Do the fits to the rssi_v_bias to find the best value
        int npoints = rssi_v_bias[channelkey]->GetN();
        float bestchi2 = 10000;
        float bestend1 = 0;
        float bestend2 = 0;
        TF1* selectedFit;

        //TMinuit doesn't like minimizing the boundaries of piecewise functions, so iterate over a range of values
        for(int ii = 0; ii<10; ii++){
          for(int jj = ii; jj < 20; jj++){
            TF1* fit_to_rssi_response = new TF1("fit_to_rssi_response", "(x>0&&x<[0])*([1]+[2]*x) + (x>=[0] && x < [3])*([4]+[5]*x) + (x>=[3] && x<[7])*([6]+[5]*2*x)", 0, 32);
            //Par 0 = end of first piece (bias too low to see much of anything)
            //Par 1,2 = linear fit to first piece
            //Par 3 = end of second piece (we've found the clock but we aren't under the waveform yet)
            //Par 4,5 = linear fit to second piece
            //Par 6 = offset of third piece. In principle once we are under the waveform the RSSI
            //should increase about twice as fast, so the slope of the third piece is double the slope of the second.
            //Par 7 = end of the third piece (actually fixed to the maximum value of the bias scan)
            
            
            //Try to give MINUIT some reasonable guesses
            fit_to_rssi_response->FixParameter(7, 32); //Fixed at the max of the bias scan
            fit_to_rssi_response->SetParameter(0, ii); //Try this value for the first boundary
            fit_to_rssi_response->SetParameter(3, jj); //Try this value for the second boundary
            fit_to_rssi_response->SetParLimits(3, ii, npoints); //Make sure the second boundary doesn't go somewhere weird, since this is the one we want to find.
            fit_to_rssi_response->SetParameter(1, 0);
            fit_to_rssi_response->SetParameter(2, 0);
            fit_to_rssi_response->SetParameter(4, -0.01);
            fit_to_rssi_response->SetParameter(5, .01);
            fit_to_rssi_response->SetParameter(6, -0.1);
            rssi_v_bias[channelkey]->Fit(fit_to_rssi_response, "QR");
            
            if(fit_to_rssi_response->GetChisquare() < bestchi2){
              bestchi2 = fit_to_rssi_response->GetChisquare();
              
              bestend1 = fit_to_rssi_response->GetParameter(0);
              bestend2 = fit_to_rssi_response->GetParameter(3);
              selectedFit = (TF1*)fit_to_rssi_response->Clone(); //Keep the best fit and the boundaries that correspond to it.
            }
          }
        }
        //If the best chi2 value is relatively high, take a look by hand.
        if(bestchi2 > 10.) cout << "The chi-square value for the POH bias fit is pretty high.  Please take a look at this plot by eye." << endl;

        //The second boundary corresponds to the minimum bias value we want (under the waveform).  Move 2 up from the floor.
        //Really we should take the first value after the boundary where the graph is ~1sig from the fit, but the errors 
        //are unphysical right now.
        int selectedBiasValue = (int)selectedFit->GetParameter(3) + 2;
        double x1, y1;
        double fit_eval = selectedFit->Eval(selectedBiasValue);
        double bias_err = rssi_v_bias[channelkey]->GetErrorY(selectedBiasValue);
        //    plot_to_fit->Print();
        rssi_v_bias[channelkey]->GetPoint(selectedBiasValue, x1, y1);
        
        
        //If the fit screwed up (or the corner of the fit doesn't describe it well) march up until we get a good fit.
        while((fabs(y1-fit_eval) > 2*bias_err) && selectedBiasValue < npoints){
          selectedBiasValue+=1;
          double fit_eval = selectedFit->Eval(selectedBiasValue);
          rssi_v_bias[channelkey]->GetPoint(selectedBiasValue, x1, y1);
          bias_err = rssi_v_bias[channelkey]->GetErrorY(selectedBiasValue);
        }
        
        //store the selected bias value for this channel, or set it unphysical if the fit failed to find anything.
        if(selectedBiasValue < npoints){
          selected_poh_bias_values[channelkey] = selectedBiasValue;
          //Now set the AOH Bias to this value if gain==2
          if(*gain_itr==2){
            cout << "Setting the POH bias value for port card " << portCardName << " POH number " << AOHNumber << " to " << selectedBiasValue << endl;
            SetAOHBiasToCurrentValue(portCardName, AOHNumber, selectedBiasValue);
          }
        }
        else{
          selected_poh_bias_values[channelkey] = -999;
        }
        
        unsigned gain = channelkey >> 30;
        TDirectory* gd = 0;
        if (gain_dirs[gain] == 0)
          gain_dirs[gain] = file.mkdir(TString::Format("gain%i", gain));
        gd = gain_dirs[gain];
        
        unsigned gain_fed = channelkey >> 5;
        unsigned fed = (channelkey & 0x3fffffff) >> 5;
        if (fed_dirs[gain_fed] == 0)
          fed_dirs[gain_fed] = gd->mkdir(TString::Format("FED%i", fed));
        fed_dirs[gain_fed]->cd();
        rssi_v_bias[channelkey]->Write();
        delete rssi_v_bias[channelkey];
      }
    }
  }
  file.Write();
  file.Close();
}

std::vector<std::string> PixelPOHBiasCalibration::calibrated(){
  std::vector<std::string> tmp;
  //tmp.push_back("portcard"); // JMTBAD eventually
  return tmp;
}

void PixelPOHBiasCalibration::SetAOHBiasToCurrentValue(std::string portCardName, int AOHNumber, int AOHBiasNumber)
{
  Attribute_Vector parametersToTKFEC_SetAOHBiasOneChannel(3);
  parametersToTKFEC_SetAOHBiasOneChannel[0].name_="PortCardName";
  parametersToTKFEC_SetAOHBiasOneChannel[1].name_="AOHNumber";
  parametersToTKFEC_SetAOHBiasOneChannel[2].name_="AOHBias";
  parametersToTKFEC_SetAOHBiasOneChannel[0].value_=portCardName;
  parametersToTKFEC_SetAOHBiasOneChannel[1].value_=itoa(AOHNumber);
  parametersToTKFEC_SetAOHBiasOneChannel[2].value_=itoa(AOHBiasNumber);

  std::map<std::string,PixelPortCardConfig*>::const_iterator mapNamePortCard_itr = getmapNamePortCard()->find(portCardName);
  assert( mapNamePortCard_itr != getmapNamePortCard()->end() );
  const PixelPortCardConfig* thisPortCardConfig = mapNamePortCard_itr->second;
  const std::string TKFECID = thisPortCardConfig->getTKFECID();
  unsigned int TKFECcrate = theTKFECConfiguration_->crateFromTKFECID(TKFECID);

  if (Send(PixelTKFECSupervisors_[TKFECcrate], "SetAOHBiasOneChannel", parametersToTKFEC_SetAOHBiasOneChannel)!="SetAOHBiasOneChannelDone")
    {
      diagService_->reportError("SetAOHBiasOneChannel could not be done!",DIAGERROR);
    }
}
