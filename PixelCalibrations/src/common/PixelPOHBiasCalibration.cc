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
  POHBiasMax = POHBiasMin + POHBiasNSteps * POHBiasStepSize;

  DoFits = tempCalibObject->parameterValue("DoFits") != "no";
  SetBiasEnMass = tempCalibObject->parameterValue("SetBiasEnMass") == "yes";

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
  const std::set<PixelChannel>& channelsToCalibrate = tempCalibObject->channelList();
  for (std::vector<unsigned>::const_iterator gain_itr = POHGains.begin(); gain_itr != POHGains.end(); ++gain_itr) {
    for (std::set<PixelChannel>::const_iterator channelsToCalibrate_itr = channelsToCalibrate.begin(); channelsToCalibrate_itr != channelsToCalibrate.end(); ++channelsToCalibrate_itr) {
      const PixelHdwAddress& channelHdwAddress = theNameTranslation_->getHdwAddress(*channelsToCalibrate_itr);
      const unsigned int NChannel = channelHdwAddress.fedchannel();
      if (NChannel % 2==0) { // per fiber
        //Get Fed and channel number
        const unsigned int NFed = channelHdwAddress.fednumber();
        const unsigned int NROC = channelHdwAddress.rocid();
        const unsigned int NFiber = NChannel/2;
        const std::pair<std::string, int> portCardAndAOH = thePortcardMap_->PortCardAndAOH(*channelsToCalibrate_itr);
        const std::string portCardName = portCardAndAOH.first;
        assert(portCardName != "none");
        const int AOHNumber = portCardAndAOH.second;
        printf("nfed %i nfiber %i\n", NFed, NFiber);
        TGraphErrors* g = rssi_v_bias[key(*gain_itr, NFed, NFiber)] = new TGraphErrors(POHBiasNSteps);
	TString thismodulename = (*channelsToCalibrate_itr).modulename();
	TString thispohname = TString::Format(" POH %i", AOHNumber);
	TString thishistotitle = thismodulename + " " + portCardName + thispohname; 
        g->SetName(TString::Format("rssi_gain%i_FED%i_fiber%i", *gain_itr, NFed, NFiber));
        g->SetTitle(thishistotitle);
        g->SetMarkerColor(1);
        g->SetMarkerStyle(21);
        g->SetMarkerSize(1);
        selected_poh_bias_values[key(*gain_itr, NFed, NFiber)] = 0;
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
          if (SetBiasEnMass) {
            Attribute_Vector parametersToTKFEC(1);
            parametersToTKFEC[0].name_="AOHBias";
            parametersToTKFEC[0].value_=itoa(bias);
            commandToAllTKFECCrates("SetAOHBiasEnMass", parametersToTKFEC);
          }
          else 
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
        const int AOHNumber = portCardAndAOH.second;
        
        unsigned channelkey = key(*gain_itr, NFed, NFiber);

        if (DoFits) {
          //Do the fits to the rssi_v_bias to find the best value
          int npoints = rssi_v_bias[channelkey]->GetN();
	  int fitmin = ((POHBiasMax-3*POHBiasStepSize)>20) ? (POHBiasMax-3*POHBiasStepSize) : 20;
          TF1* fit_to_rssi_response = new TF1("fit_to_rssi_response", "pol1", fitmin , POHBiasMax);
          //Do a linear fit to the RSSI response at very high values where we are below the waveform
          rssi_v_bias[channelkey]->Fit(fit_to_rssi_response, "QR");

          double par0 = fit_to_rssi_response->GetParameter(0);
          double par1 = fit_to_rssi_response->GetParameter(1);

	  TF1* evaluate_rssi_response = new TF1("evaluate_rssi_response", "pol1", 0, POHBiasMax);
          evaluate_rssi_response->FixParameter(0, par0);
          evaluate_rssi_response->FixParameter(1, par1);
          bool looking_for_bias_value=true;
          int max_bias_value = 25;

	  TF1* projection_to_x_axis = new TF1("projection_to_x_axis", "pol1", -1.*(par0/par1), POHBiasMax);
	  projection_to_x_axis->FixParameter(0, par0);
	  projection_to_x_axis->FixParameter(1, par1);
	  projection_to_x_axis->SetLineColor(3);
	  rssi_v_bias[channelkey]->Fit(projection_to_x_axis, "QR");

          while(looking_for_bias_value && max_bias_value >= 3){
            double x1[3] = {0.0};
            double y1[3] = {0.0};
            double fit_eval[3] = {0};
            for(int i = 0; i < 3; i++){
              fit_eval[i] = evaluate_rssi_response->Eval((max_bias_value-i));
              rssi_v_bias[channelkey]->GetPoint((max_bias_value-i), x1[i], y1[i]);
            }
            //Error on the points is 0.005, but that's due to the granularity of the RSSI readout.
	    //Real error is more like 0.002.  Is the current max_bias_value point more than
	    //one sigma from the fit and above the fit?
            if(fabs((y1[0]-fit_eval[0]))>0.002){
              //Is the point before the max_bias_value above the fit and even worse?
	      if(fabs((y1[1]-fit_eval[1]))>fabs((y1[0]-fit_eval[0])) && (y1[1]-fit_eval[1])>(y1[0]-fit_eval[0])){
		//and the point before that?
		if(fabs((y1[2]-fit_eval[2]))>fabs((y1[1]-fit_eval[1])) && (y1[2]-fit_eval[2])>(y1[1]-fit_eval[1])){
		  //cout << max_bias_value 
                  looking_for_bias_value=false;}
              }
            }
            max_bias_value--;
          }

          //march back two values of max_bias_value due to the loop ending on a decrement
          int selectedBiasValue = max_bias_value + 2;

          //If the fit screwed up (or the corner of the fit doesn't describe it well) march up until we get a good fit.
          double x0, y0;
          rssi_v_bias[channelkey]->GetPoint(selectedBiasValue, x0, y0);
          double fit_check = evaluate_rssi_response->Eval(selectedBiasValue);
          double bias_err = rssi_v_bias[channelkey]->GetErrorY(selectedBiasValue);
          while((fabs(y0-fit_check) > bias_err) && selectedBiasValue < npoints){
            selectedBiasValue+=1;
            fit_check = evaluate_rssi_response->Eval(selectedBiasValue);
            rssi_v_bias[channelkey]->GetPoint(selectedBiasValue, x0, y0);
            bias_err = rssi_v_bias[channelkey]->GetErrorY(selectedBiasValue);
          }
        
          //store the selected bias value for this channel, or set it unphysical if the fit failed to find anything.
          if(selectedBiasValue < npoints){
            selected_poh_bias_values[channelkey] = selectedBiasValue;
            //Now set the AOH Bias to this value if gain==2
            if(*gain_itr==2){
              cout << "Setting the POH bias value for fed " << NFed << " fiber " << NFiber << " port card " << portCardName << " POH number " << AOHNumber << " Module name " << (*channelsToCalibrate_itr).modulename() << " to " << selectedBiasValue << endl;
              SetAOHBiasToCurrentValue(portCardName, AOHNumber, selectedBiasValue);
	      bias_values_by_portcard_and_aoh_new[portCardName][AOHNumber] = selectedBiasValue;
            }
          }
          else{
            selected_poh_bias_values[channelkey] = -999;
          }
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

  //Write out the configs
  for (std::map<std::string, std::map<unsigned int, unsigned int> >::iterator portCardName_itr = bias_values_by_portcard_and_aoh_new.begin(); portCardName_itr != bias_values_by_portcard_and_aoh_new.end(); portCardName_itr++){
    std::string portCardName = portCardName_itr->first;
    std::map<std::string, PixelPortCardConfig*>::iterator mapNamePortCard_itr = getmapNamePortCard()->find(portCardName);
    assert( mapNamePortCard_itr != getmapNamePortCard()->end());
    PixelPortCardConfig* thisPortCardConfig = mapNamePortCard_itr->second;
    for(std::map<unsigned int, unsigned int >::iterator AOHNumber_itr = portCardName_itr->second.begin(); AOHNumber_itr != portCardName_itr->second.end(); AOHNumber_itr++){
      unsigned int AOHNumber = AOHNumber_itr->first;
      unsigned int AOHBiasAddress = thisPortCardConfig->AOHBiasAddressFromAOHNumber(AOHNumber);
      thisPortCardConfig->setdeviceValues(AOHBiasAddress, bias_values_by_portcard_and_aoh_new[portCardName][AOHNumber]);
    }
    thisPortCardConfig->writeASCII(outputDir());
    cout << "Wrote the portcard config for port card: " << portCardName << endl;
  }
}

std::vector<std::string> PixelPOHBiasCalibration::calibrated(){
  std::vector<std::string> tmp;
  //tmp.push_back("portcard"); // JMTBAD eventually
  return tmp;
}

void PixelPOHBiasCalibration::SetAOHBiasToCurrentValue(std::string portCardName, int AOHNumber, int AOHBiasNumber) {
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

  if (Send(PixelTKFECSupervisors_[TKFECcrate], "SetAOHBiasOneChannel", parametersToTKFEC_SetAOHBiasOneChannel) != "SetAOHBiasOneChannelDone")
    std::cerr << "SetAOHBiasOneChannel could not be done!\n";
}
