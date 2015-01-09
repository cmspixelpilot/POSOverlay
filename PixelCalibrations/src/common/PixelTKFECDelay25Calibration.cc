// $Id: PixelTKFECDelay25Calibration.cc,v 1.22 2012/01/20 19:14:49 kreis Exp $

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelTKFECDelay25Calibration.h"

#include <toolbox/convertstring.h>

using namespace pos;
using namespace pos::PortCardSettingNames;
using namespace std;

PixelTKFECDelay25Calibration::PixelTKFECDelay25Calibration(const PixelTKFECSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr) 
  : PixelTKFECCalibrationBase(tempConfiguration, *mySOAPCmdr)
{
  //  cout << "Greetings from the PixelTKFECDelay25Calibration copy constructor." << endl;
}

xoap::MessageReference PixelTKFECDelay25Calibration::execute(xoap::MessageReference msg){
  event_++;

  //this is a fix for multi-crate (FPix+BPix) running.
  if (done_) {
    ::sleep(1);
    xoap::MessageReference reply = MakeSOAPMessageReference("TKFECDelay25CalibrationDone");
    return reply;
  }

  try {

  if(nextPortcard_) {
    portcardName_ = vectorOfPortcards_.at(countPortcards_);
    countPortcards_++;
    
    cout << "Running Delay25 scan on portcard " << portcardName_ << ", number " << countPortcards_ << " of " << totalPortcards_ << endl;

    map<string, PixelPortCardConfig*>::iterator i_mapNamePortCard = mapNamePortCard_.find(portcardName_);
    //Did this already while vectorOfPortcards_ was being collected
    /*
    if (i_mapNamePortCard==mapNamePortCard_.end()) {
      cout << "Could not find portcard " << portcardName << endl;
      continue;
    }
    */
    assert(i_mapNamePortCard != mapNamePortCard_.end());
    
    portcardConfig_ = i_mapNamePortCard->second;
    //Check whether we compare all modules
    runCompare_ = false;
    string type = portcardConfig_->gettype();
    if(type=="fpix" && allModules_) {
      cout << "This is an FPix portcard with allModules_=true." << endl;
      cout << "The optimal point will take into account results from all modules." << endl;
      runCompare_=true;
    }
    string tkfecid = portcardConfig_->getTKFECID();
    if(theTKFECConfiguration_->crateFromTKFECID(tkfecid) != crate_) {
      //This portcard isn't controlled by a tkfec in this crate; skipping
      xoap::MessageReference reply = MakeSOAPMessageReference("KeepGoing");
      return reply;
    }
    assert(theTKFECConfiguration_->crateFromTKFECID(tkfecid) == crate_);
    
    GlobalCandidatePoints_.clear();
    GlobalFinalData_.clear();

    //Get all modules associated with the current portcard
    vectorOfModules_.clear();
    std::set<PixelModuleName> module_map = thePortcardMap_->modules(portcardName_);
    std::set<PixelModuleName>::const_iterator module_iter;
    for (module_iter=module_map.begin();module_iter!=module_map.end(); ++module_iter){
      PixelModuleName module_name = *module_iter;
      vectorOfModules_.push_back(module_name);
    }
    if(allModules_) {
      totalModules_ = vectorOfModules_.size();
    } else {
      totalModules_ = 1;
    }
    countModules_=0;
    nextPortcard_=false;
  }

  if(nextModule_) {
    moduleName_ = vectorOfModules_.at(countModules_);
    countModules_++;
    moduleString_ = moduleName_.modulename();

    cout << "Using module " << moduleName_ << ", number " << countModules_ << " of " << totalModules_ << endl;

    //open the files that will contain the test results for this portcard/module
    //record settings, and write the first tag
    tempDelay25_->openFiles(portcardName_, moduleString_, outputDir());
    tempDelay25_->writeSettings(portcardName_, moduleString_);
    tempDelay25_->writeFiles("GridScan:");

    CandidatePoints_.clear();
    finalData_.clear();
    Efficiency_.clear();

    countSData_=0;
    nextModule_=false;
  }

  Attribute_Vector parametersToFEC(8);
  parametersToFEC.at(0).name_="VMEBaseAddress";
  parametersToFEC.at(1).name_="mFEC";
  parametersToFEC.at(2).name_="mFECChannel";
  parametersToFEC.at(3).name_="TBMChannel";
  parametersToFEC.at(4).name_="HubAddress";
  parametersToFEC.at(5).name_="PortAddress";
  parametersToFEC.at(6).name_="Repetitions";
  parametersToFEC.at(7).name_="Commands";
  
  const PixelHdwAddress& module_firstHdwAddress = theNameTranslation_->firstHdwAddress(moduleName_);
  
  unsigned int fecnumber=module_firstHdwAddress.fecnumber();
  unsigned int feccrate=theFECConfiguration_->crateFromFECNumber(fecnumber);
  unsigned long vmeBaseAddress=theFECConfiguration_->VMEBaseAddressFromFECNumber(fecnumber);
  unsigned int mfec=module_firstHdwAddress.mfec();
  unsigned int mfecchannel=module_firstHdwAddress.mfecchannel();
  unsigned int tbmchannel=module_firstHdwAddress.fedchannel();
  unsigned int hubaddress=module_firstHdwAddress.hubaddress();
  unsigned int portaddress=7; //change from 4 to 7 on advice of Rutgers mFEC experts
  //unsigned int portaddress=module_hdwaddress->portaddress();
  
  parametersToFEC.at(0).value_=itoa(vmeBaseAddress);
  parametersToFEC.at(1).value_=itoa(mfec);
  parametersToFEC.at(2).value_=itoa(mfecchannel);
  parametersToFEC.at(3).value_=itoa(tbmchannel);
  parametersToFEC.at(4).value_=itoa(hubaddress);
  parametersToFEC.at(5).value_=itoa(portaddress);
  parametersToFEC.at(6).value_=itoa(numTests_);
  parametersToFEC.at(7).value_=itoa(commands_);
  
  int currentSData = origSData_ + gridSize_*countSData_;
  countSData_++;
  
  if( !SetDelay(portcardConfig_, "SDATA", currentSData) ) {
    cout<<"There was a problem setting "<<"SDATA"<<endl;
    //diagService_->reportError("There was a problem setting " + parametersToTKFEC[1].value_,DIAGWARN);
  }
  
  int currentRData;
  //Loop over RDATA values
  for ( int j1=0; j1<gridSteps_; ++j1 ){
    currentRData = origRData_ + gridSize_*j1;
    
    if( !SetDelay(portcardConfig_, "RDATA", currentRData) ) {
      cout<<"There was a problem setting "<<"RDATA"<<endl;
      //diagService_->reportError("There was a problem setting " + parametersToTKFEC[1].value_,DIAGWARN);
    }
    
    //Test communication with the FEC     
    xoap::MessageReference Delay25TestResults = SendWithSOAPReply(PixelFECSupervisors_[feccrate], "Delay25Test", parametersToFEC);
    
    Attribute_Vector parametersReturned(5);
    parametersReturned[0].name_="nSuccess0";
    parametersReturned[1].name_="nSuccess1";
    parametersReturned[2].name_="nSuccess2";
    parametersReturned[3].name_="nSuccess3";
    parametersReturned[4].name_="nSuccess4";
    
    Receive(Delay25TestResults, parametersReturned);
    
    unsigned int nSuccess0=atoi(parametersReturned[0].value_.c_str());
    unsigned int nSuccess1=atoi(parametersReturned[1].value_.c_str());
    unsigned int nSuccess2=atoi(parametersReturned[2].value_.c_str());
    unsigned int nSuccess3=atoi(parametersReturned[3].value_.c_str());
    unsigned int nSuccess4=atoi(parametersReturned[4].value_.c_str());
    //cout << "nSuccess0="<<nSuccess0<<endl;
    //cout << "nSuccess1="<<nSuccess1<<endl;
    //cout << "nSuccess2="<<nSuccess2<<endl;
    //cout << "nSuccess3="<<nSuccess3<<endl;
    //cout << "nSuccess4="<<nSuccess4<<endl;
    
    int numberSuccesses;
    int numberSuccess[5] = {nSuccess0, nSuccess1, nSuccess2, nSuccess3, nSuccess4};
    if(commands_ == 0) {
      numberSuccesses = (nSuccess0+nSuccess1+nSuccess2+nSuccess3+nSuccess4)/5;
    } else if (commands_ == 1) {
      numberSuccesses = (nSuccess0+nSuccess1+nSuccess2+nSuccess3)/4;
    } else if (commands_ == 2) {
      numberSuccesses = (nSuccess0+nSuccess1+nSuccess2+nSuccess4)/4;
    } else {
      cout << "Unknown value for Commands!  Setting numberSuccesses to zero." << endl;
      numberSuccesses = 0;
    }

    //cout << "currentSData, currentRData, numberSuccesses:"
    //	 <<currentSData<<" "<<currentRData<<" "<<numberSuccesses<<endl;
    tempDelay25_->writeFiles(currentSData, currentRData, numberSuccesses);
    //fill finalData using the currentSData and currentRData
    pair<int,int> point(currentSData, currentRData);
    finalData_[point] = numberSuccesses;
    for(int i=0; i<5; i++) {
       Efficiency_[i][point] = numberSuccess[i];
    }
    if ( numberSuccesses==numTests_ ){
      CandidatePoints_.insert(point);
      //If this is the first module, populate GlobalCandidatePoints_
      if(runCompare_ && countModules_ == 1) {
	GlobalCandidatePoints_.insert(point);
      }
    }
    //end of loop over RData
  }

  if(countSData_ == gridSteps_) {
    //Finished all the rows and columns, so run the optimization
    countSData_ = 0;
    nextModule_ = true;

    if(countModules_ > 1 && runCompare_) {
      Intersection(CandidatePoints_, GlobalCandidatePoints_);
    }

    nCandidatePoints_.insert( CandidatePoints_.size() );

    if(CandidatePoints_.size() == 0) {
      cout << "No good region found!" << endl;
      tempDelay25_->writeFiles("NoStableRegion");
      tempDelay25_->closeFiles();

      //create and draw the root plot
      WriteRootFile(-10, -10, false);
    } else {
      
      assert(CandidatePoints_.size() > 0);

      set< pair<int,int> > GoodPoints = CandidatePoints_;
      
      bool MadeProgress = true;
      bool NSMadeProgress = true;
      bool EWMadeProgress = true;
      bool NESWMadeProgress = true;
      bool NWSEMadeProgress = true;
      int i=1;
      int countRegion = 0;
      while(MadeProgress) {
	//cout << "Initial number of good points is " << GoodPoints.size() << endl;
	if(NSMadeProgress) {
	  NSMadeProgress = NS(CandidatePoints_,GoodPoints,i,countRegion);
	}
	//cout << "Number of good points after NS is " << GoodPoints.size() << endl;
	if(EWMadeProgress) {
	  EWMadeProgress = EW(CandidatePoints_,GoodPoints,i,countRegion);
	}
	//cout << "Number of good points after EW is " << GoodPoints.size() << endl;
	if(NESWMadeProgress) {
	  NESWMadeProgress = NESW(CandidatePoints_,GoodPoints,i,countRegion);
	}
	//cout << "Number of good points after NESW is " << GoodPoints.size() << endl;
	if(NWSEMadeProgress) {
	  NWSEMadeProgress = NWSE(CandidatePoints_,GoodPoints,i,countRegion);
	}
	//cout << "Number of good points after NWSE is " << GoodPoints.size() << endl;
	MadeProgress = NSMadeProgress || EWMadeProgress || NESWMadeProgress || NWSEMadeProgress;
	i++;
	if(i == 20) {
	  cout << i << " iterations?  I find that hard to believe.  Breaking." << endl;
	  break;
	}
      }
      
      int newSDA = 0;
      int newRDA = 0;

      FinalPointSelection(GoodPoints, newSDA, newRDA);

      tempDelay25_->writeFiles("SelectedPoint:");
      tempDelay25_->writeFiles(newSDA, newRDA, numTests_);
      
      tempDelay25_->closeFiles();
      
      cout << "The stable point I have chosen is SDA=0x" << hex << newSDA
	   << " RDA=0x" << newRDA << dec << endl;
      //cout << "Final stable region size is " << countRegion << endl;
      if(countRegion < 11) {
	cout << "Warning!  The good region around this point is small.  Portcard "
	     << portcardName_ << " might be problematic." << endl;
      }
      
      //Set SData to the chosen value
      if( !SetDelay(portcardConfig_, "SDATA", newSDA, true, false) ) { 
	cout<<"There was a problem setting "<<"SDATA"<<endl;
      }
      
      //Set RData to the chosen value
      if( !SetDelay(portcardConfig_, "RDATA", newRDA, true, true) ) {
	cout<<"There was a problem setting "<<"RDATA"<<endl;
      }

      //create and draw the root plot
      WriteRootFile(newSDA, newRDA, false);
    }
    
    if(countModules_ < totalModules_) {
      cout << "Retesting portcard " << portcardName_ << " with next module in list." << endl;
    } else {
      assert(countModules_ == totalModules_);
      if(runCompare_) {
	if(GlobalCandidatePoints_.size() == 0) {
	  cout << "The intersection of all the good regions was empty!" << endl;
	} else {

	  assert(GlobalCandidatePoints_.size() > 0);

	  //Open and write the files for the intersection
	  tempDelay25_->openFiles(portcardName_, "All", outputDir());
	  tempDelay25_->writeSettings(portcardName_, "All");
	  tempDelay25_->writeFiles("GridScan:");

	  set< pair<int,int> >::iterator GlobalIter;
	  for(GlobalIter = GlobalCandidatePoints_.begin(); GlobalIter != GlobalCandidatePoints_.end();GlobalIter++) {
	    pair<int,int> point = *GlobalIter;
	    GlobalFinalData_[point] = numTests_;
	    tempDelay25_->writeFiles(point.first, point.second, numTests_);
	  }

	  set< pair<int,int> > GlobalGoodPoints = GlobalCandidatePoints_;

	  bool MadeProgress = true;
	  bool NSMadeProgress = true;
	  bool EWMadeProgress = true;
	  bool NESWMadeProgress = true;
	  bool NWSEMadeProgress = true;
	  int i=1;
	  int countRegion = 0;
	  while(MadeProgress) {
	    //cout << "Initial number of good points is " << GlobalGoodPoints.size() << endl;
	    if(NSMadeProgress) {
	      NSMadeProgress = NS(GlobalCandidatePoints_,GlobalGoodPoints,i,countRegion);
	    }
	    //cout << "Number of good points after NS is " << GlobalGoodPoints.size() << endl;
	    if(EWMadeProgress) {
	      EWMadeProgress = EW(GlobalCandidatePoints_,GlobalGoodPoints,i,countRegion);
	    }
	    //cout << "Number of good points after EW is " << GlobalGoodPoints.size() << endl;
	    if(NESWMadeProgress) {
	      NESWMadeProgress = NESW(GlobalCandidatePoints_,GlobalGoodPoints,i,countRegion);
	    }
	    //cout << "Number of good points after NESW is " << GlobalGoodPoints.size() << endl;
	    if(NWSEMadeProgress) {
	      NWSEMadeProgress = NWSE(GlobalCandidatePoints_,GlobalGoodPoints,i,countRegion);
	    }
	    //cout << "Number of good points after NWSE is " << GlobalGoodPoints.size() << endl;
	    MadeProgress = NSMadeProgress || EWMadeProgress || NESWMadeProgress || NWSEMadeProgress;
	    i++;
	    if(i == 20) {
	      cout << i << " iterations?  I find that hard to believe.  Breaking." << endl;
	      break;
	    }
	  }

	  int newSDA = 0;
	  int newRDA = 0;

	  FinalPointSelection(GlobalGoodPoints, newSDA, newRDA);

	  tempDelay25_->writeFiles("SelectedPoint:");
	  tempDelay25_->writeFiles(newSDA, newRDA, numTests_);

	  tempDelay25_->closeFiles();

	  cout << "The stable point I have chosen is SDA=0x" << hex << newSDA
	       << " RDA=0x" << newRDA << dec << endl;
	  //cout << "Final stable region size is " << countRegion << endl;
	  if(countRegion < 11) {
	    cout << "Warning!  The good region around this point is small.  Portcard "
		 << portcardName_ << " might be problematic." << endl;
	  }

	  //Set SData to the chosen value
	  if( !SetDelay(portcardConfig_, "SDATA", newSDA, true, false) ) {
	    cout<<"There was a problem setting "<<"SDATA"<<endl;
	  }

	  //Set RData to the chosen value
	  if( !SetDelay(portcardConfig_, "RDATA", newRDA, true, true) ) {
	    cout<<"There was a problem setting "<<"RDATA"<<endl;
	  }

	  //create and draw the root plot
	  WriteRootFile(newSDA, newRDA, true);
	}
      }

      countModules_ = 0;
      nextPortcard_ = true;

      ostringstream os;
      os << "Delay25 calibration of portcard " << portcardName_ << " is complete.";
      cout<<os.str()<<endl;
      xdaq::ApplicationDescriptor* PixelSupervisor=app_->getApplicationContext()->getDefaultZone()->getApplicationGroup("daq")->getApplicationDescriptor("PixelSupervisor", 0);
      SendStatus(PixelSupervisor,os.str());

      if(countPortcards_ == totalPortcards_) {
	//	cout<<"--- Running for Delay 25 Scan Done ---" << endl;
	//	outputFile_->Close();
	done_=true;
	xoap::MessageReference reply = MakeSOAPMessageReference("TKFECDelay25CalibrationDone");
	return reply;
      }
    }
  }

  } catch (...) {
    done_=true;
    cout<<"ERROR -- caught an exception in PixelTKFECDelay25Calibration!"<<endl;
    if(countPortcards_ == totalPortcards_) {
      xoap::MessageReference reply = MakeSOAPMessageReference("TKFECDelay25CalibrationDone");
      return reply;
    }
  }
  
  xoap::MessageReference reply = MakeSOAPMessageReference("KeepGoing");
  return reply;

}

  

xoap::MessageReference PixelTKFECDelay25Calibration::beginCalibration(xoap::MessageReference msg)
{
  //  cout<<"Greeting from PixelTKFECDelay25Calibration::beginCalibration"<<endl;

  done_=false;

  event_=0;
  tempDelay25_ = dynamic_cast <PixelDelay25Calib*> (theCalibObject_);
  assert(tempDelay25_!=0);
  
  //cout << "The execute member function has been called from PixelTKFECDelay25Calibrations." << endl;
  //diagService_->reportError("--- Running for Delay 25 Scan ---",DIAGINFO);
  //cout<<"\n--- Running for Delay 25 Scan ---"<<endl;
  
  gridSize_ = tempDelay25_->getGridSize();
  gridSteps_ = tempDelay25_->getGridSteps();
  numTests_ = tempDelay25_->getNumberTests();
  allModules_ = tempDelay25_->allModules();
  origSData_ = tempDelay25_->getOrigSDa();
  origRData_ = tempDelay25_->getOrigRDa();
  range_ = tempDelay25_->getRange();
  commands_ = tempDelay25_->getCommands();
  //totalSData_ = origSData_ + range_;
  countSData_ = 0;
  nextModule_ = true;
  countModules_ =0;
  nextPortcard_ = true;
  countPortcards_ = 0;
  
  setupOutputDir();
  
  set<string> portcard_list;
  if(tempDelay25_->allPortcards()) {
    //Calibrate all portcards
    portcard_list = thePortcardMap_->portcards();
  } else {
    //Just the listed portcards
    portcard_list = tempDelay25_->portcardList();
  }
  
  set<string>::const_iterator portcard_name;
  for (portcard_name = portcard_list.begin(); portcard_name != portcard_list.end(); portcard_name++){
    string portcardName = *portcard_name;
    map<string, PixelPortCardConfig*>::iterator i_mapNamePortCard = mapNamePortCard_.find(portcardName);
    if(i_mapNamePortCard == mapNamePortCard_.end()) {
      //cout << "Could not find portcard " << portcardName << endl;
      continue;
    }
    assert(i_mapNamePortCard != mapNamePortCard_.end());
    vectorOfPortcards_.push_back(portcardName);
  }
  totalPortcards_=vectorOfPortcards_.size();
  
  //Open the root file
  string rootfilename = outputDir()+"/delay25_"+stringF(crate_)+".root";
  outputFile_ = new TFile(rootfilename.c_str(),"RECREATE","delay25");

  //construct the root file directory structure
  rootDirs_ = new PixelRootDirectoryMaker(vectorOfPortcards_,gDirectory);
  

  xoap::MessageReference reply = MakeSOAPMessageReference("BeginCalibratioDone");
  return reply;
}

xoap::MessageReference PixelTKFECDelay25Calibration::endCalibration(xoap::MessageReference msg)
{
  cout<<"--- Running for Delay 25 Scan Done ---" << endl;

  //this also seems to cause a crash in some strange circumstances
//   int min = *(nCandidatePoints_.begin());
//   cout<<"min # of good points = "<<min<<endl;
//   int max = *(nCandidatePoints_.rbegin());
//   cout<<"max # of good points = "<<max<<endl;

  /* 
  outputFile_->cd();
  TH1F numberOfGoodPoints("numberOfGoodPoints","Size of Good Region",30,min-min*0.05,max+max*0.05); //FIXME hardcoded
  set<int>::const_iterator iPoint;
  for ( iPoint = nCandidatePoints_.begin() ; iPoint != nCandidatePoints_.end() ; ++iPoint ) {
    int iNumberOfGoodPoints = *iPoint;
    //cout<<"number of good points = "<<iNumberOfGoodPoints<<endl; //DEBUG
    //could multiply by gridSize_^2 to make runs with different gridsize comparable
    numberOfGoodPoints.Fill( iNumberOfGoodPoints );
  }
  numberOfGoodPoints.SetXTitle("Number of points in Good Region");

  numberOfGoodPoints.Write();
  */
  //if I uncomment the above block, I see a
  //double free crash at outputFile_->Close()

  //I don't understand this, but for now I just want the calibration to work

  outputFile_->Close();

  xoap::MessageReference reply = MakeSOAPMessageReference("EndCalibrationDone");
  return reply;
}

bool PixelTKFECDelay25Calibration::SetDelay(PixelPortCardConfig* tempPortCard, string delay, unsigned int value, bool update, bool write){
   string tkfecid = tempPortCard->getTKFECID();
   assert(theTKFECConfiguration_->crateFromTKFECID(tkfecid) == crate_);
   unsigned int fecAddress = theTKFECConfiguration_->addressFromTKFECID(tkfecid);
   unsigned int ringAddress = tempPortCard->getringAddress();
   unsigned int ccuAddress = tempPortCard->getccuAddress();
   unsigned int channelAddress = tempPortCard->getchannelAddress();
   unsigned int deviceAddress;

   string SDA, RDA;
   SDA="SDATA"; RDA="RDATA";
   if(delay==SDA) {
      deviceAddress = tempPortCard->getdeviceAddressForSetting(k_Delay25_SDA);
   } else if(delay==RDA) {
      deviceAddress = tempPortCard->getdeviceAddressForSetting(k_Delay25_RDA);
   } else {
      cout << "Unknown register " << delay << endl;
      return false;
   }

   assert(fecAccess_!=0);

   int flag = 1;
   enumDeviceType modeType = PHILIPS;

   portcardI2CDevice(fecAccess_, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, modeType, value, flag);

   if(update) {
      cout << "TKFECDelay25Calibration:  updated " << delay << endl;
      tempPortCard->setdeviceValues(deviceAddress, value);
   }
   if(write) {
      cout << "TKFECDelay25Calibration:  new portcard.dat file written." << endl;
      string outputdir = outputDir();
      tempPortCard->writeASCII(outputdir+"/");
   }

   return true;
}

void PixelTKFECDelay25Calibration::portcardI2CDevice(FecAccess* fecAccess, tscType8 fecAddress, tscType8 ringAddress, tscType8 ccuAddress, tscType8 channelAddress, tscType8 deviceAddress, enumDeviceType modeType, unsigned int value, int flag) {

   keyType index = buildCompleteKey(fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress);
   try {
      fecAccess_->addi2cAccess(index, modeType, MODE_SHARE);
   } catch (FecExceptionHandler e) {
      cout << "------------ Exception ----------" << std::endl ;
      cout << e.what()  << std::endl ;
      cout << "---------------------------------" << std::endl ;

      return;
   }

   try {
      fecAccess_->write(index, value);
   } catch (FecExceptionHandler e) {
      cout<<"--------- Exception ---------"<<std::endl;
      cout<< e.what() <<std::endl;
      cout<<"-----------------------------"<<std::endl;

      return;
   }
   fecAccess_->removei2cAccess(index);
   
   return;
}

bool PixelTKFECDelay25Calibration::NS(set< pair<int,int> > CandidatePoints, set< pair<int,int> > &GoodPoints, int i, int &countRegion) {

   set< pair<int,int> > CurrentGoodPoints;
   set< pair<int,int> >::iterator PointsIter;
   set< pair<int,int> >::iterator FindIter1;
   set< pair<int,int> >::iterator FindIter2;

   for(PointsIter = GoodPoints.begin(); PointsIter != GoodPoints.end(); ++PointsIter) {
      pair<int,int> CurrentPoint = *PointsIter;
      pair<int,int> temppoint1(CurrentPoint.first,CurrentPoint.second+gridSize_*i);
      pair<int,int> temppoint2(CurrentPoint.first,CurrentPoint.second-gridSize_*i);
      FindIter1 = CandidatePoints.find(temppoint1);
      FindIter2 = CandidatePoints.find(temppoint2);
      if( (FindIter1 != CandidatePoints.end()) && (FindIter2 != CandidatePoints.end()) ) {
	 CurrentGoodPoints.insert(CurrentPoint);
      }
   }
   if(CurrentGoodPoints.size() == 0) {
     return false;
   }
   assert(CurrentGoodPoints.size() > 0);
   float factor = GoodPoints.size()/CurrentGoodPoints.size();
   //cout << "Reduction factor is " << factor << endl;
   if(factor > 4 && i > 1) {
     //cout << "This test eliminated too many points.  Return true, but take no action." << endl;
     return true;
   } else {
     GoodPoints.clear();
     GoodPoints = CurrentGoodPoints;
     countRegion += 2;
     return true;
   }

}

bool PixelTKFECDelay25Calibration::EW(set< pair<int,int> > CandidatePoints, set< pair<int,int> > &GoodPoints, int i, int &countRegion) {

   set< pair<int,int> > CurrentGoodPoints;
   set< pair<int,int> >::iterator PointsIter;
   set< pair<int,int> >::iterator FindIter1;
   set< pair<int,int> >::iterator FindIter2;
   for(PointsIter = GoodPoints.begin(); PointsIter != GoodPoints.end(); ++PointsIter) {
      pair<int,int> CurrentPoint = *PointsIter;
      pair<int,int> temppoint1(CurrentPoint.first+gridSize_*i,CurrentPoint.second);
      pair<int,int> temppoint2(CurrentPoint.first-gridSize_*i,CurrentPoint.second);
      FindIter1 = CandidatePoints.find(temppoint1);
      FindIter2 = CandidatePoints.find(temppoint2);
      if( (FindIter1 != CandidatePoints.end()) && (FindIter2 != CandidatePoints.end()) ) {
	 CurrentGoodPoints.insert(CurrentPoint);
      }
   }
   if(CurrentGoodPoints.size() == 0) {
     return false;
   }
   assert(CurrentGoodPoints.size() > 0);
   float factor = GoodPoints.size()/CurrentGoodPoints.size();
   //cout << "Reduction factor is " << factor << endl;
   if(factor > 4 && i > 1) {
     //cout << "This test eliminated too many points.  Return true, but take no action." << endl;
     return true;
   } else {
     GoodPoints.clear();
     GoodPoints = CurrentGoodPoints;
     countRegion += 2;
     return true;
   }

}

bool PixelTKFECDelay25Calibration::NESW(set< pair<int,int> > CandidatePoints, set< pair<int,int> > &GoodPoints, int i, int &countRegion) {

   set< pair<int,int> > CurrentGoodPoints;
   set< pair<int,int> >::iterator PointsIter;
   set< pair<int,int> >::iterator FindIter1;
   set< pair<int,int> >::iterator FindIter2;
   for(PointsIter = GoodPoints.begin(); PointsIter != GoodPoints.end(); ++PointsIter) {
      pair<int,int> CurrentPoint = *PointsIter;
      pair<int,int> temppoint1(CurrentPoint.first+gridSize_*i,CurrentPoint.second+gridSize_*i);
      pair<int,int> temppoint2(CurrentPoint.first-gridSize_*i,CurrentPoint.second-gridSize_*i);
      FindIter1 = CandidatePoints.find(temppoint1);
      FindIter2 = CandidatePoints.find(temppoint2);
      if( (FindIter1 != CandidatePoints.end()) && (FindIter2 != CandidatePoints.end()) ) {
	 CurrentGoodPoints.insert(CurrentPoint);
      }
   }
   if(CurrentGoodPoints.size() == 0) {
     return false;
   }
   assert(CurrentGoodPoints.size() > 0);
   float factor = GoodPoints.size()/CurrentGoodPoints.size();
   //cout << "Reduction factor is " << factor << endl;
   if(factor > 4 && i > 1) {
     //cout << "This test eliminated too many points.  Return true, but take no action." << endl;
     return true;
   } else {
     GoodPoints.clear();
     GoodPoints = CurrentGoodPoints;
     countRegion += 2;
     return true;
   }

}

bool PixelTKFECDelay25Calibration::NWSE(set< pair<int,int> > CandidatePoints, set< pair<int,int> > &GoodPoints, int i, int &countRegion) {

   set< pair<int,int> > CurrentGoodPoints;
   set< pair<int,int> >::iterator PointsIter;
   set< pair<int,int> >::iterator FindIter1;
   set< pair<int,int> >::iterator FindIter2;
   for(PointsIter = GoodPoints.begin(); PointsIter != GoodPoints.end(); ++PointsIter) {
      pair<int,int> CurrentPoint = *PointsIter;
      pair<int,int> temppoint1(CurrentPoint.first+gridSize_*i,CurrentPoint.second-gridSize_*i);
      pair<int,int> temppoint2(CurrentPoint.first-gridSize_*i,CurrentPoint.second+gridSize_*i);
      FindIter1 = CandidatePoints.find(temppoint1);
      FindIter2 = CandidatePoints.find(temppoint2);
      if( (FindIter1 != CandidatePoints.end()) && (FindIter2 != CandidatePoints.end()) ) {
	 CurrentGoodPoints.insert(CurrentPoint);
      }
   }
   if(CurrentGoodPoints.size() == 0) {
     return false;
   }
   assert(CurrentGoodPoints.size() > 0);
   float factor = GoodPoints.size()/CurrentGoodPoints.size();
   //cout << "Reduction factor is " << factor << endl;
   if(factor > 4 && i > 1) {
     //cout << "This test eliminated too many points.  Return true, but take no action." << endl;
     return true;
   } else {
     GoodPoints.clear();
     GoodPoints = CurrentGoodPoints;
     countRegion += 2;
     return true;
   }

}

void PixelTKFECDelay25Calibration::FinalPointSelection(set< pair<int,int> > GoodPoints, int &newSDA, int &newRDA) {
  
  set< pair<int,int> >::iterator FinalPointsIter;
  
  if(GoodPoints.size() == 1) {
    //A unique best point was found
    FinalPointsIter = GoodPoints.begin();
    pair<int,int> FinalPoint = *FinalPointsIter;
    newSDA = FinalPoint.first;
    newRDA = FinalPoint.second;
  } else {
    int sumSDa = 0;
    int sumRDa = 0;
    int minSDa = 128;
    int maxSDa = 64;
    int minRDa = 128;
    int maxRDa = 64;
    int numFinal = GoodPoints.size();
    //are the candidates for the final point adjacent?
    for(FinalPointsIter = GoodPoints.begin(); FinalPointsIter != GoodPoints.end(); FinalPointsIter++) {
      pair<int,int> FinalPoint = *FinalPointsIter;
      int currentSDa = FinalPoint.first;
      int currentRDa = FinalPoint.second;
      //cout << "Candidate final point:  (" << currentSDa << "," << currentRDa << ")" << endl;
      sumSDa += currentSDa;
      sumRDa += currentRDa;
      if(currentSDa < minSDa)
	{
	  minSDa = currentSDa;
	}
      if(currentSDa > maxSDa)
	{
	  maxSDa = currentSDa;
	}
      if(currentRDa < minRDa)
	{
	  minRDa = currentRDa;
	}
      if(currentRDa > maxRDa)
	{
	  maxRDa = currentRDa;
	}
    }
    int rangeSDa = maxSDa - minSDa;
    int rangeRDa = maxRDa - minRDa;
    if(rangeSDa <= gridSize_*(numFinal-1) && rangeRDa <= gridSize_*(numFinal-1) && gridSize_ > 1) {
      //Final points appear to be adjacent, and we weren't using gridSize_ = 1
      newSDA = sumSDa/numFinal;
      newRDA = sumRDa/numFinal;
      //cout << "My tentative final point is (" << newSDA << "," << newRDA << ")" << endl;
    } else {
      //Final points are NOT adjacent, or else an average is going to cause rounding problems.  
      //Default to center of grid
      int diffSDA = 96;
      int diffRDA = 96;
      int currentdiffSDA, currentdiffRDA;
      int centerSDA = 96;
      int centerRDA = 96;
      for(FinalPointsIter = GoodPoints.begin(); FinalPointsIter != GoodPoints.end(); ++FinalPointsIter) {
	pair<int, int> FinalPoint = *FinalPointsIter;
	currentdiffSDA = abs(FinalPoint.first - centerSDA);
	currentdiffRDA = abs(FinalPoint.second - centerRDA);
	if(currentdiffSDA < diffSDA && currentdiffRDA < diffRDA) {
	  diffSDA = currentdiffSDA;
	  diffRDA = currentdiffRDA;
	  newSDA = FinalPoint.first;
	  newRDA = FinalPoint.second;
	}
      }
    }
  }
}

void PixelTKFECDelay25Calibration::Intersection(set< pair<int,int> > CandidatePoints, set< pair<int,int> > &GlobalCandidatePoints) {
  set< pair<int,int> >::iterator CandidateIter;
  set< pair<int,int> >::iterator GlobalIter;
  for(GlobalIter = GlobalCandidatePoints.begin(); GlobalIter != GlobalCandidatePoints.end(); GlobalIter++) {
    pair<int,int> point = *GlobalIter;
    CandidateIter = CandidatePoints.find(point);
    if(CandidateIter == CandidatePoints.end()) {
      //This point is not in the intersection, so we get rid of it
      GlobalCandidatePoints.erase(GlobalIter);
      GlobalIter--;
    }
  }

}

void PixelTKFECDelay25Calibration::WriteRootFile(int newSDA, int newRDA, bool compare) {

   //cd to the deepest root directory
   rootDirs_->cdDirectory(portcardName_);
    
   int maxSDa, maxRDa;
   maxSDa = origSData_ + range_;
   maxRDa = origRData_ + range_;

   if(!compare) {

     string longtitle = "RDa vs. SDa for portcard "+portcardName_+" and module "+moduleString_;
     string shorttitle = "portcard_"+portcardName_+"_module_"+moduleString_;
     
     TCanvas* c = new TCanvas(longtitle.c_str(),longtitle.c_str(), 700,800);
     
     TH2F* paramspace = new TH2F(shorttitle.c_str(), longtitle.c_str(), range_, origSData_, maxSDa, range_, origRData_, maxRDa);
     paramspace->GetXaxis()->SetTitle("SDa");
     paramspace->GetYaxis()->SetTitle("RDa");
     paramspace->Draw("colz");
     
     int sda, rda, number;
     for(std::map<std::pair<int,int>,int>::iterator finalData_itr=finalData_.begin();finalData_itr!=finalData_.end();++finalData_itr){
       number=finalData_itr->second;
       if(number>0)
	 {
	   sda=(finalData_itr->first).first;
	   rda=(finalData_itr->first).second;
	   
	   TMarker* mk = new TMarker(sda, rda, 20);
	   mk->SetMarkerSize((number*2.0)/numTests_);
	   if(number==numTests_) mk->SetMarkerColor(kBlue);
	   mk->Draw();
	 }
       else
	 {
	   sda=(finalData_itr->first).first;
	   rda=(finalData_itr->first).second;
	   
	   TMarker* mk = new TMarker(sda, rda, 1);
	   mk->Draw();
	 }
     }
     if(newSDA > -1 && newRDA > -1) {
	TMarker* mk1 = new TMarker(newSDA,newRDA, 21);
	
	mk1->SetMarkerSize(2.0);
	mk1->SetMarkerColor(2);
	mk1->Draw();
     }
     ((TPad*)(c))->Write();
     
     //Now the plots for each individual test
     
     std::string name[5]={"1","2","3","4","5"};
     //     string shorttitle = "hist";
     TCanvas* Canvases[5];
     TH2F* paramspace0[5];
     TMarker* marker[5];
     TMarker* marker0[5];
     
     for(int i=0; i<5; i++){
       
       Canvases[i] = new TCanvas((longtitle+"_command"+name[i]).c_str(),(longtitle+"_command"+name[i]).c_str(), 700,800);
       
       paramspace0[i] = new TH2F((shorttitle+"_command"+name[i]).c_str(),(longtitle+"_command"+name[i]).c_str() , range_, origSData_, maxSDa, range_, origRData_, maxRDa);
       paramspace0[i]->GetXaxis()->SetTitle("SDa");
       paramspace0[i]->GetYaxis()->SetTitle("RDa");
       paramspace0[i]->Draw("colz");
       
       
       for(std::map<std::pair<int,int>,int>::iterator finalData_itr=Efficiency_[i].begin();finalData_itr!=Efficiency_[i].end();++finalData_itr){
	 number=finalData_itr->second;
	 if(number>0)
	   {
	     sda=(finalData_itr->first).first;
	     rda=(finalData_itr->first).second;
	     
	     marker[i] = new TMarker(sda, rda, 20);
	     marker[i]->SetMarkerSize((number*2.0)/numTests_);
	     if(number==numTests_) marker[i]->SetMarkerColor(kBlue);
	     marker[i]->Draw();
	   }
	 else
	   {
	     sda=(finalData_itr->first).first;
	     rda=(finalData_itr->first).second;
	     
	     marker[i] = new TMarker(sda, rda, 1);
	     marker[i]->Draw();
	   }
       }
       if(newSDA > -1 && newRDA > -1) {
	  marker0[i] = new TMarker(newSDA,newRDA, 21);
	  
	  marker0[i]->SetMarkerSize(2.0);
	  marker0[i]->SetMarkerColor(2);
	  marker0[i]->Draw();
       }
       ((TPad*)(Canvases[i]))->Write();
     }

   } else {

     string longtitle = "RDa vs. SDa for portcard "+portcardName_+" and all modules";
     string shorttitle = "portcard_"+portcardName_+"_allmodules";

     TCanvas* c = new TCanvas(longtitle.c_str(),longtitle.c_str(), 700,800);

     TH2F* paramspace = new TH2F(shorttitle.c_str(), longtitle.c_str(), range_, origSData_, maxSDa, range_, origRData_, maxRDa);
     paramspace->GetXaxis()->SetTitle("SDa");
     paramspace->GetYaxis()->SetTitle("RDa");
     paramspace->Draw("colz");

     int sda, rda, number;
     for(std::map<std::pair<int,int>,int>::iterator GlobalFinalData_itr=GlobalFinalData_.begin();GlobalFinalData_itr!=GlobalFinalData_.end();++GlobalFinalData_itr){
       number=GlobalFinalData_itr->second;
       if(number>0)
         {
           sda=(GlobalFinalData_itr->first).first;
           rda=(GlobalFinalData_itr->first).second;

           TMarker* mk = new TMarker(sda, rda, 20);
           mk->SetMarkerSize((number*2.0)/numTests_);
           if(number==numTests_) mk->SetMarkerColor(kBlue);
           mk->Draw();
         }
       else
         {
           sda=(GlobalFinalData_itr->first).first;
           rda=(GlobalFinalData_itr->first).second;

           TMarker* mk = new TMarker(sda, rda, 1);
           mk->Draw();
         }
     }
     if(newSDA > -1 && newRDA > -1) {
	TMarker* mk1 = new TMarker(newSDA,newRDA, 21);
	
	mk1->SetMarkerSize(2.0);
	mk1->SetMarkerColor(2);
	mk1->Draw();
     }
     ((TPad*)(c))->Write();
   }

}
