/*************************************************************************
 * PixelROCDelay25Calibration Class,                                     *
 * class to set different Delay25 settings and different DAC settings    *
 * that then requests the FED to determine if the ROCs reconfigured      *
 * themselves via the PixelFEDROCDelay25Calibration class                *
 *                                                                       *
 * Author: James Zabel, Rice University                                  *
 *                                                                       *
 * Last update: $Date: 2012/06/16 14:13:20 $ (UTC)                       *
 *          by: $Author: mdunser $                                        *
 *************************************************************************/

//Go here "/calib - template.dat" for the infput file template.

#include "PixelCalibrations/include/PixelROCDelay25Calibration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACScanRange.h"
#include "PixelUtilities/PixelFEDDataTools/include/LastDACFIFODecoder.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"
#include "CalibFormats/SiPixelObjects/interface/PixelPortcardMap.h"
#include "PixelSupervisorConfiguration/include/PixelSupervisorConfiguration.h"
#include "PixelUtilities/Pixelb2inUtilities/include/Pixelb2inCommander.h"

using namespace std;
using namespace pos;

PixelROCDelay25Calibration::PixelROCDelay25Calibration(const PixelSupervisorConfiguration &tempConfiguration, SOAPCommander *mySOAPCmdr)
    : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  cout << "Greetings from the PixelROCDelay25Calibration constructor." << endl;
  totalTime_.start();
}

void PixelROCDelay25Calibration::beginCalibration()
{
  commandToAllFECCrates("ClrCalEnMass");     //ensure that there will be no hits output
  commandToAllFEDCrates("ResetFEDsEnMass");  //reset all FEDs, to ensure the last DAC FIFO on the FEDs are empty
}

bool PixelROCDelay25Calibration::execute()
{
//collect info from the calib object
  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
    assert(tempCalibObject != 0);

  PixelPortcardMap *thePortCardMap_;
  PixelConfigInterface::get(thePortCardMap_, "pixel/portcardmap/", *theGlobalKey_);
    assert(theGlobalKey_ != 0);

  set <unsigned int> myFEDCrates = tempCalibObject->getFEDCrates(theNameTranslation_, theFEDConfiguration_);
  set <unsigned int>::const_iterator myFED_iter;
  xoap::MessageReference SOAPReply;
  string tempString;

//assign data from the configuration file
  unsigned int delay25Min = atoi(tempCalibObject->parameterValue("Delay25Min").c_str()),
               delay25Max = atoi(tempCalibObject->parameterValue("Delay25Max").c_str()),
               delay25StepSize = atoi(tempCalibObject->parameterValue("Delay25StepSize").c_str()),
               scanMin = atoi(tempCalibObject->parameterValue("ScanMin").c_str()),
               scanMax = atoi(tempCalibObject->parameterValue("ScanMax").c_str()),
               stepSize = atoi(tempCalibObject->parameterValue("StepSize").c_str()),
               numDACSettings = atoi(tempCalibObject->parameterValue("NumDACSettings").c_str()),
               numDelaySettings = 1 + ((delay25Max - delay25Min) / delay25StepSize),
               numTriggers = numDACSettings * numDelaySettings,
               runNumber = 0;
  float howOften = 1 / (numTriggers + 1);
  vector <unsigned int> DACLevel;
  string DACToSet = tempCalibObject->parameterValue("DACToSet").c_str(), reply;
  bool flag = false;

//set the DAC Level settings from the configuration file
  if(stepSize == (scanMax - scanMin)){
    for(unsigned int i = 0; i < numDACSettings; ++i){
      if((i % 2) == 0){
        DACLevel.push_back(scanMin);
      } else{
        DACLevel.push_back(scanMax);
      }
    }
  } else {
    for(unsigned int i = 0; i < numDACSettings; ++i){
      DACLevel.push_back(scanMin + ((stepSize * i) % (scanMax - scanMin)));
    }
  }

  PixelDACScanRange DACInfo(DACToSet, DACLevel, 0, false);

//instantiation of paramter arrays used by SOAP messages
  Attribute_Vector parametersToTKFEC(4);
    parametersToTKFEC[0].name_ = "Delay25Setting";  parametersToTKFEC[0].value_ = "0";
    parametersToTKFEC[1].name_ = "DelayType";       parametersToTKFEC[1].value_ = "SDATA";
    parametersToTKFEC[2].name_ = "Update";          parametersToTKFEC[2].value_ = "0";
    parametersToTKFEC[3].name_ = "Write";           parametersToTKFEC[3].value_ = "0";
  Attribute_Vector parametersToFEC(2);
    parametersToFEC[0].name_ = "DACAddress";  parametersToFEC[0].value_ = itoa(DACInfo.dacchannel());
    parametersToFEC[1].name_ = "DACValue";    parametersToFEC[1].value_ = "0";
  Attribute_Vector parametersToFED(4);
    parametersToFED[0].name_ = "Delay25Setting";  parametersToFED[0].value_ = "0";
    parametersToFED[1].name_ = "DACSetting";      parametersToFED[1].value_ = "0";
    parametersToFED[2].name_ = "RunNumber";       parametersToFED[2].value_ = "0";
    parametersToFED[3].name_ = "MinimumRange";    parametersToFED[3].value_ = tempCalibObject->parameterValue("MinimumRange").c_str();

  if((DACInfo.name() == "Vana") || (DACInfo.name() == "VIBias_DAC")){  //we should never use these DACs when running this code
    cout << "PixelROCDelay25Calibration: cannot use DAC Setting " << DACInfo.name() << "!!!" << endl;
    assert(0);
  }

  event_ = 0;
cout << "PixelROCDelay25Calibration: sendingMode_ = " << sendingMode_ << endl;
//may need to add another nested for loop to change the "SDA" setting to the various options???????????????????????????????????????????????????????????????????????????
//these loops cycle through the delay settings and DAC settings to set the ROCs differently, then trigger the ROCs, then ask FEDs to read the last DAC value
  if(sendingMode_ != "yes"){                                                                                        //useSOAP is not set to yes
    flag = true;
    for(unsigned int i_delay = delay25Min; i_delay <= delay25Max; i_delay += delay25StepSize){                      //loop to change the delay settings
      vector <pair <int, unsigned int> > messageIDs;
      vector <pair <int, unsigned int> >::const_iterator messageIDs_iter;
      PixelTimer tempTimer;
      tempTimer.start();
        parametersToTKFEC[0].name_ = "Delay25Setting";  parametersToTKFEC[0].value_ = itoa((i_delay & 0x3F) | 0x40);  //the & 0x3F | 0x40 sets the enable bit
      commandToAllTKFECCrates("SetDelayEnMass", parametersToTKFEC);                                                 //set the delay setting
      runNumber = 0;
      for(unsigned int i_DACSetting = 0; i_DACSetting < DACInfo.getNPoints(); ++i_DACSetting){                      //loop to change the DAC settings
        reportProgress(howOften, std::cout, numTriggers);  //if (event_ - previous event_)/numTriggers > howOften then report progress
          parametersToFEC[1].name_ = "DACValue";    parametersToFEC[1].value_ = itoa(DACInfo.value(i_DACSetting));
        commandToAllFECCrates("SetROCDACsEnMass", parametersToFEC);                                                 //set DAC value for all required ROCs
        sendTTCCalSync();
          parametersToFED[0].value_ = itoa(i_delay);
          parametersToFED[1].value_ = itoa(DACInfo.value(i_DACSetting));
          parametersToFED[2].value_ = itoa(runNumber);
        commandToAllFEDCrates("FEDCalibrations", parametersToFED);                                                  //ask FEDs to read trigger values
        messageIDs.erase(messageIDs.begin(), messageIDs.end());
        runNumber += 1;
        event_ += 1;
      }                                                                                                             //end of loop changing DAC settings
      tempTimer.stop();
      cout << "PixelROCDelay25Calibration:  For the delay setting " << i_delay << " it took " << tempTimer.tottime() << " milliseconds to complete." << endl;
    }                                                                                                               //end of loop changing delay settings
  } else { //if(sendingMode_ == "yes"){                                                                             //useSOAP is set to yes
    cout << "PixelROCDelay25Calibration:  this calibration only works when useSOAP is not set to \"yes\".  Calibration exiting!!!" << endl;
    assert(0);
  }

  return false;
}

void PixelROCDelay25Calibration::endCalibration()
{
  totalTime_.stop();
  cout << "PixelROCDelay25Calibration: total time of calibration was " << totalTime_.tottime() << endl;
}

vector <string> PixelROCDelay25Calibration::calibrated()
{
  vector <string> tmp;
  tmp.push_back("portcard");
  return tmp;
}
