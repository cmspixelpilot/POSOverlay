/*************************************************************************
 * PixelFEDROCDelay25Calibration Class header file,                      *
 * class to read and evaluate last DAC readings from the ROCs after      *
 * different Delay25 settings and different DAC settings have been set   *
 * by the PixelROCDelay25Calibration class                               *
 *                                                                       *
 * Author: James Zabel, Rice University                                  *
 *                                                                       *
 * Last update: $Date: 2012/06/16 14:13:19 $ (UTC)                       *
 *          by: $Author: mdunser $                                        *
 *************************************************************************/

#ifndef _PixelFEDROCDelay25Calibration_h_
#define _PixelFEDROCDelay25Calibration_h_

#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelPortcardMap.h"
#include "PixelUtilities/PixelRootUtilities/include/PixelRootDirectoryMaker.h"
#include "TH2F.h"

class PixelFEDROCDelay25Calibration: public PixelFEDCalibrationBase {
  public:
    PixelFEDROCDelay25Calibration(const PixelFEDSupervisorConfiguration&, SOAPCommander*);
    virtual ~PixelFEDROCDelay25Calibration();
    xoap::MessageReference beginCalibration(xoap::MessageReference);
    xoap::MessageReference execute(xoap::MessageReference);
    xoap::MessageReference endCalibration(xoap::MessageReference);
    void initializeFED();

  private:
    unsigned int writeToFile_, numDelaySettings_, numDACSettings_;
    float delay25Min_, delay25Max_, delay25StepSize_, minRange_;
    fstream ROCDelay25OutputFile_;
    pos::PixelPortcardMap *thePortCardMap_;
    std::map <std::string, std::map <pos::PixelROCName, std::map <float, vector <int> > > > myTestResultsByROC_;  //<port card name, <roc name, <delay, <dac reading> > > >
    std::map <std::string, std::map <pos::PixelROCName, std::map <float, int> > > myCollapsedTestResultsByROC_;   //<port card name, <roc name, <delay, <summed dac readings> > >
    std::map <std::string, std::map <pos::PixelROCName, int> >   previousDACSetting_;                             //<port card name, <roc name, previous dac setting> >
    std::map <std::string, std::map <pos::PixelROCName, float> > myHighestDACValueByROC_;                         //<port card name, <roc name, highest dac setting differential> >
    std::map <std::string, std::map <pos::PixelROCName, std::map <float, float> > > acceptableDelaysByROC_;       //<port card name, <roc name, <start delay time, end delay time> > >
    std::map <std::string, std::map <pos::PixelROCName, int> > theROCsOnThisFEDMap_;                              //<port card name, <roc name, 1> >
//    std::map <std::string, std::map <float, int> > myCompleteResultsByPortCard_;                                  //<port card name, <delay, <summed dac readings> >
    std::map <std:: string, float> delayToUseByPortCard_;                                                         //<port card name, delay to use>
    std::map <std:: string, float> currentDelayByPortCard_;                                                       //<port card name, current delay used>
    std::vector <std::pair <unsigned int, std::vector <unsigned int> > > fedsAndChannels_;
    std::vector <pos::PixelROCName> theROCsOnThisFED_;
    std::vector <std::string> DACLevels_;
    std::map <pos::PixelROCName, std::map <float, vector <int> > >::iterator aROC_iter; 
    void readAssignLastDAC(unsigned int, float);                                                                  //pass the run number and the delay time

     struct ROCDelay25Branch{
      float pass;
      float delaySetting;
      float offsetFromIdealDelay;
      float offsetFromCurrentDelay;
      char rocName[38];
    };
};

#endif
