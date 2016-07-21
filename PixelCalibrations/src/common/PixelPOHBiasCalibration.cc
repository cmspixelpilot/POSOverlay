#include <iomanip>
#include "TGraphErrors.h"
#include "TFile.h"

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

  POHGain         = 2;
  POHBiasMin      = 0;
  POHBiasNSteps   = 15;
  POHBiasStepSize = 2;
  if (tempCalibObject->parameterValue("POHGain")      != "") POHGain         = strtoul(tempCalibObject->parameterValue("POHGain").c_str(), 0, 16);
  if (tempCalibObject->parameterValue("ScanMin")      != "") POHBiasMin      = atoi(tempCalibObject->parameterValue("ScanMin").c_str());
  if (tempCalibObject->parameterValue("ScanNSteps")   != "") POHBiasNSteps   = atoi(tempCalibObject->parameterValue("ScanNSteps").c_str());
  if (tempCalibObject->parameterValue("ScanStepSize") != "") POHBiasStepSize = atoi(tempCalibObject->parameterValue("ScanStepSize").c_str());

  std::map<unsigned int, std::set<unsigned int> > fedsAndChannelsMap = tempCalibObject->getFEDsAndChannels(theNameTranslation_);
  for (std::map<unsigned int, std::set<unsigned int> >::iterator fednumber_itr = fedsAndChannelsMap.begin(); fednumber_itr != fedsAndChannelsMap.end(); ++fednumber_itr) {

    for (std::set<unsigned int>::iterator channel_itr = fednumber_itr->second.begin(); channel_itr != fednumber_itr->second.end(); channel_itr++) {
      if (*channel_itr%2==0) { // per fiber
        int NFed = fednumber_itr->first;
        int NFiber = *channel_itr/2;
        printf("nfed %i nfiber %i\n", NFed, NFiber);
        int channelkey = NFed*100+NFiber;
        TGraphErrors* g = rssi_v_bias[channelkey] = new TGraphErrors(POHBiasNSteps);
        g->SetName(TString::Format("rssi_FED%i_Channel%i", NFed, NFiber));
        g->SetTitle(";POH bias value;RSSI (mA)");
        g->SetMarkerColor(1);
        g->SetMarkerStyle(21);
        g->SetMarkerSize(1);
      }
    }
  }
}


bool PixelPOHBiasCalibration::execute() {
  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*>(theCalibObject_);
  assert(tempCalibObject!=0);

  Attribute_Vector parametersToTKFEC(1);
  parametersToTKFEC[0].name_="AOHGain";
  parametersToTKFEC[0].value_=itoa(POHGain);
  commandToAllTKFECCrates("SetAOHGainEnMass", parametersToTKFEC);

  for (unsigned ibias = 0; ibias < POHBiasNSteps; ++ibias) {
    unsigned bias = POHBiasMin + POHBiasStepSize * ibias;

    Attribute_Vector parametersToTKFEC(1);
    parametersToTKFEC[0].name_="AOHBias";
    parametersToTKFEC[0].value_=itoa(bias);
    commandToAllTKFECCrates("SetAOHBiasEnMass", parametersToTKFEC);

    usleep(100000);

    std::map<unsigned int, std::set<unsigned int> > fedsAndChannelsMap = tempCalibObject->getFEDsAndChannels(theNameTranslation_);
    for ( std::map<unsigned int, std::set<unsigned int> >::iterator fednumber_itr = fedsAndChannelsMap.begin(); fednumber_itr != fedsAndChannelsMap.end(); ++fednumber_itr) {
      const int NFed = fednumber_itr->first;
      for ( std::set<unsigned int>::iterator channel_itr = fednumber_itr->second.begin(); channel_itr != fednumber_itr->second.end(); channel_itr++) {
        if (*channel_itr%2==0) {
          int NFiber = *channel_itr/2;
          int channelkey = NFed*100+NFiber;

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
          rssi_v_bias[channelkey]->SetPoint(ibias, bias, rssi);
          rssi_v_bias[channelkey]->SetPointError(ibias, 0, 0.005);
        }
      }
    }
  }

  return false; // done
}


void PixelPOHBiasCalibration::endCalibration() {
  TFile file(TString::Format("%s/POHBias.root", outputDir().c_str()), "create");

  std::map<int, TDirectory*> dirs;
  for (std::map<int, TGraphErrors*>::iterator it= rssi_v_bias.begin(); it != rssi_v_bias.end(); ++it) {
    const int NFed = it->first / 100;
    if (dirs[NFed] == 0)
      dirs[NFed] = file.mkdir(TString::Format("FED%i", NFed));
    dirs[NFed]->cd();
    it->second->Write();
    delete it->second;
  }

  file.Write();
  file.Close();
}

std::vector<std::string> PixelPOHBiasCalibration::calibrated(){
  std::vector<std::string> tmp;
  tmp.push_back("portcard");
  return tmp;
}
