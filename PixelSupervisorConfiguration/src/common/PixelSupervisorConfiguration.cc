// $Id: PixelSupervisorConfiguration.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelUtilities/PixelSOAPUtilities/include/SOAPCommander.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"

#include "PixelSupervisorConfiguration/include/PixelSupervisorConfiguration.h"

using namespace pos;
using namespace std;


PixelSupervisorConfiguration::PixelSupervisorConfiguration(std::string* runNumber, std::string* outputDir, xdaq::Application* app):PixelSupervisorConfigurationBase(runNumber, outputDir),Pixelb2inCommander(app)
{
  theGlobalKey_=0;
  theNameTranslation_=0;
  theTKFECConfiguration_=0;
  theFECConfiguration_=0;
  theFEDConfiguration_=0;
  theCalibObject_=0;
  diagService_=0;

}


PixelSupervisorConfiguration::PixelSupervisorConfiguration(const PixelSupervisorConfiguration & tempConfiguration):PixelSupervisorConfigurationBase(tempConfiguration), Pixelb2inCommander(tempConfiguration)
{
  
  //just shallow copying for the moment
  //pointers
  theGlobalKey_ = tempConfiguration.theGlobalKey_;
  theCalibObject_ = tempConfiguration.theCalibObject_;
  theNameTranslation_ = tempConfiguration.theNameTranslation_;
  theDetectorConfiguration_ = tempConfiguration.theDetectorConfiguration_;
  theTKFECConfiguration_ = tempConfiguration.theTKFECConfiguration_;
  theFECConfiguration_ = tempConfiguration.theFECConfiguration_;
  theFEDConfiguration_ = tempConfiguration.theFEDConfiguration_;
  mapNamePortCard_ = tempConfiguration.mapNamePortCard_;
  thePortcardMap_ = tempConfiguration.thePortcardMap_;
  diagService_ = tempConfiguration.diagService_;
  
  //ApplicationDescriptors
  PixelFECSupervisors_ = tempConfiguration.PixelFECSupervisors_;
  PixelFEDSupervisors_ = tempConfiguration.PixelFEDSupervisors_;
  PixelTKFECSupervisors_ = tempConfiguration.PixelTKFECSupervisors_;
  PixelLTCSupervisors_ = tempConfiguration.PixelLTCSupervisors_;
  TTCciControls_ = tempConfiguration.TTCciControls_;
  PixelFECSupervisors_ = tempConfiguration.PixelFECSupervisors_;
  PixelDCSFSMInterface_ = tempConfiguration.PixelDCSFSMInterface_;
  psxServers_ = tempConfiguration.psxServers_;
  PixelSlinkMonitors_ = tempConfiguration.PixelSlinkMonitors_;
  
  // State information
  statePixelFECSupervisors_ = tempConfiguration.statePixelFECSupervisors_;
  statePixelFEDSupervisors_ = tempConfiguration.statePixelFEDSupervisors_;
  statePixelTKFECSupervisors_ = tempConfiguration.statePixelTKFECSupervisors_;
  statePixelDCSFSMInterface_ = tempConfiguration.statePixelDCSFSMInterface_;
  
}


std::map<std::string,pos::PixelPortCardConfig*> *  PixelSupervisorConfiguration::getmapNamePortCard() 
{
  if (mapNamePortCard_.size() == 0) fillMapNamePortCard();
  return &mapNamePortCard_;
}

void PixelSupervisorConfiguration::fillMapNamePortCard() {
    assert(mapNamePortCard_.size()==0);

    diagService_->reportError("Retrieving PortCard Settings... ", DIAGTRACE);

    const std::set<std::string>& portcards=thePortcardMap_->portcards();
    std::set<std::string>::const_iterator iportcard=portcards.begin();

    for (;iportcard!=portcards.end();iportcard++) {
      PixelPortCardConfig* tempPortCard=0;
      PixelConfigInterface::get(tempPortCard,"pixel/portcard/"+*iportcard, *theGlobalKey_);
      if (tempPortCard==0) XCEPT_RAISE(xdaq::exception::Exception, "Failed to load portcard!");
      mapNamePortCard_[*iportcard]=tempPortCard;
    }
    diagService_->reportError("... PortCard Settings retrieved.", DIAGTRACE);

}

void PixelSupervisorConfiguration::clearMapNamePortCard() {
  // Clear mapNamePortCard_ 
  map<string, PixelPortCardConfig*>::iterator iter=mapNamePortCard_.begin();
  for (;iter!=mapNamePortCard_.end();++iter)  delete iter->second;
  mapNamePortCard_.clear();
}


std::vector<xdaq::ApplicationDescriptor*> PixelSupervisorConfiguration::getPixelTTCDescriptors() const {
  
  Supervisors::const_iterator i_TTCSupervisor;
  std::vector<xdaq::ApplicationDescriptor*> vector_TTCciControls;
  for (i_TTCSupervisor=TTCciControls_.begin();
       i_TTCSupervisor!=TTCciControls_.end();
       ++i_TTCSupervisor) {
    vector_TTCciControls.push_back(i_TTCSupervisor->second);
  }

  return vector_TTCciControls;
}
    
void PixelSupervisorConfiguration::writeAllFEDCards(std::vector<std::string> filenames){

  std::map <unsigned int, std::set<unsigned int> > fedsAndChannels=theNameTranslation_->getFEDsAndChannels();
  
  std::map <unsigned int, std::set<unsigned int> >::iterator i_fedsAndChannels=fedsAndChannels.begin();
  for (;i_fedsAndChannels!=fedsAndChannels.end();++i_fedsAndChannels) {
    unsigned long fednumber=i_fedsAndChannels->first;
    string filename="params_fed_"+SOAPCommander::itoa(fednumber)+".dat";
    if (contains(filenames,filename)) continue;
    cout << "[PixelSupervisorConfiguration::writeAllFEDCards] writing:"<<filename<<endl;
    PixelFEDCard *theFEDCard;
    PixelConfigInterface::get(theFEDCard, "pixel/fedcard/"+SOAPCommander::itoa(fednumber), *theGlobalKey_);
    assert(theFEDCard!=0);
    theFEDCard->writeASCII(outputDir());
    delete theFEDCard; 
  }
}



void PixelSupervisorConfiguration::writeAllPortcards(std::vector<std::string> filenames){

  PixelPortcardMap* thePortcardMap=0;

  PixelConfigInterface::get(thePortcardMap,"pixel/portcardmap/", *theGlobalKey_);

  const std::set<std::string>& portcards=thePortcardMap->portcards();

  std::set<std::string>::const_iterator iportcard=portcards.begin(); 

  for (iportcard=portcards.begin();iportcard!=portcards.end();iportcard++) { 
    string filename="portcard_"+*iportcard+".dat";
    if (contains(filenames,filename)) continue;
    cout << "[PixelSupervisorConfiguration::writeAllPortcards] writing:"<<filename<<endl;
    PixelPortCardConfig* tempPortCard=0;
    PixelConfigInterface::get(tempPortCard,"pixel/portcard/"+*iportcard, *theGlobalKey_);
    assert(tempPortCard!=0);
    tempPortCard->writeASCII(outputDir());
    delete tempPortCard;
  }
  
  delete thePortcardMap;

}

void PixelSupervisorConfiguration::writeAllDACSettings(std::vector<std::string> filenames){
  
  std::list<const PixelModuleName*> modules=theNameTranslation_->getModules();

  std::list<const PixelModuleName*>::const_iterator module_itr=modules.begin();

  for(;module_itr!=modules.end();++module_itr){
    std::string modulePath=(*module_itr)->modulename();
    string filename="ROC_DAC_module_"+modulePath+".dat";
    if (contains(filenames,filename)) continue;
    cout << "[PixelSupervisorConfiguration::writeAllDACSettings] writing:"<<filename<<endl;
    PixelDACSettings *tempDACs=0;
    PixelConfigInterface::get(tempDACs, "pixel/dac/"+modulePath, *theGlobalKey_);
    if (tempDACs == 0) cout<<"tempDACs is null! I'm about to seg fault!"<<endl;
    tempDACs->writeASCII(outputDir());
    delete tempDACs;
  }


}


void PixelSupervisorConfiguration::writeAllTBMSettings(std::vector<std::string> filenames){
  
  std::list<const PixelModuleName*> modules=theNameTranslation_->getModules();

  std::list<const PixelModuleName*>::const_iterator module_itr=modules.begin();

  for(;module_itr!=modules.end();++module_itr){
    std::string modulePath=(*module_itr)->modulename();
    string filename="TBM_module_"+modulePath+".dat";
    if (contains(filenames,filename)) continue;
    cout << "[PixelSupervisorConfiguration::writeAllTBMSettings] writing:"<<filename<<endl;
    PixelTBMSettings *tempTBMSettings=0;
    PixelConfigInterface::get(tempTBMSettings, "pixel/tbm/"+modulePath, *theGlobalKey_);
    tempTBMSettings->writeASCII(outputDir());
    delete tempTBMSettings;
  }


}

void PixelSupervisorConfiguration::fillBadAliasList() {

  ifstream aliasfile( ( datDir()+"/badAliases.dat").c_str());
  if ( !aliasfile.good() || aliasfile.eof() ) {
    cout<<"Failed to open badAliases.dat. Bad aliases will not be suppressed!"<<endl;
    return;
  }
  cout<<"Filling bad alias list with the following aliases:"<<endl;
  string inputalias;
  while ( aliasfile>>inputalias ) {
    cout<<"\t"<<inputalias<<endl;
    badAliases_.insert(inputalias);
  }
  cout<<"...done adding bad aliases."<<endl;

}

bool PixelSupervisorConfiguration::isAliasOK(std::string alias) {

  return (  badAliases_.find(alias) == badAliases_.end());

}

bool PixelSupervisorConfiguration::contains(const std::vector<std::string>& filenames,std::string filename){

  for (std::vector<std::string>::const_iterator i=filenames.begin(), iend=filenames.end(); i!=iend; ++i){
    if (i->find(filename)!=string::npos) return true;
  }
  return false;
	 
}

