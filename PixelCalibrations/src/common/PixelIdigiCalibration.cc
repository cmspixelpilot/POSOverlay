/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2007, Cornell.	         		                 *
 * All rights reserved.                                                  *
 * Authors: A. Ryd              					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#include "PixelCalibrations/include/PixelIdigiCalibration.h"
#include "PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h"
#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"
#include "CalibFormats/SiPixelObjects/interface/PixelLowVoltageMap.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACSettings.h"
#include "PixelConfigDBInterface/include/PixelConfigInterface.h"

#include "TGraphErrors.h"
#include "TF1.h"

#include <toolbox/convertstring.h>

using namespace pos;
using namespace std;


PixelIdigiCalibration::PixelIdigiCalibration(const PixelSupervisorConfiguration & tempConfiguration, SOAPCommander* mySOAPCmdr)
  : PixelCalibrationBase(tempConfiguration, *mySOAPCmdr),
    lowVoltageMap_(0)
{
  std::cout << "Greetings from the PixelIdigiCalibration copy constructor." << std::endl;
}




bool PixelIdigiCalibration::execute()
{
  std::cout << "Now starting PixelIdigiCalibration::execute()."<<endl;;


  for (unsigned int iROC=0;iROC<maxROC_;++iROC){

    for (unsigned int vsf=0; vsf<255; vsf+=10){

      unsigned int ivsf=vsf/10;

      map<string, vector<pos::PixelROCName> >::iterator idpName=dpMap_.begin();

      for(;idpName!=dpMap_.end();++idpName){

	if (idpName->second.size()<iROC) continue;

	
	PixelROCName aROC=idpName->second[iROC];

	cout << "Selected ROC:"<<aROC<<endl;

	cout << "Will set Vsf="<<vsf<<endl;

	setDAC(aROC,pos::k_DACAddress_Vsf,vsf);

      }
      
      ::sleep(8);

      unsigned int Nread=10;

      for (unsigned int i=0;i<Nread;i++){

	idpName=dpMap_.begin();

	for(;idpName!=dpMap_.end();++idpName){

	  if (idpName->second.size()<iROC) continue;
	
	  PixelROCName aROC=idpName->second[iROC];

	  double idigi=readIdigi(idpName->first);

	  cout << "Selected ROC:"<<aROC<<" "<<idigi<<endl;

	  Idigi_[idpName->first][iROC][ivsf].push_back(idigi);

	}
      
      }

      if (vsf==250){
	idpName=dpMap_.begin();
	for(;idpName!=dpMap_.end();++idpName){

	  if (idpName->second.size()<iROC) continue;

	  PixelROCName aROC=idpName->second[iROC];

	  cout << "Selected ROC:"<<aROC<<endl;

	  //FIXME slow way to make module name
	  PixelModuleName theModule(aROC.rocname());

	  int oldVsf=dacsettings_[theModule]->getDACSettings(aROC)->getVsf();
	  setDAC(aROC,pos::k_DACAddress_Vsf,oldVsf);

	}

      }

    }

  }

  return false;

} // end of execute() function



void PixelIdigiCalibration::beginCalibration(){
  cout << "In PixelIdigiCalibration::beginCalibration()" << endl;
  
  PixelCalibConfiguration* tempCalibObject = dynamic_cast <PixelCalibConfiguration*> (theCalibObject_);
  assert(tempCalibObject!=0);

  PixelConfigInterface::get(lowVoltageMap_, "pixel/lowvoltagemap/", *theGlobalKey_); 
  if (lowVoltageMap_==0){
    cout << "Could not find the LowVoltageMap in the configuration"<< endl;
    assert(0);
  }

  maxROC_=0;
  
  const std::vector<PixelROCName>& rocs=tempCalibObject->rocList();
  
  for (unsigned int i=0;i<rocs.size();i++){

    //FIXME this is a slow conversion...
    PixelModuleName module(rocs[i].rocname());

    string dpName=lowVoltageMap_->dpNameIdigi(module);

    cout << "roc, module, dpname:"<<rocs[i]<<" "
	 << module << " " << dpName << endl;

    assert(dpName!="");

    dpMap_[dpName].push_back(rocs[i]);

    std::vector<Moments> v(26);
    
    Idigi_[dpName].push_back(v);

    if (dpMap_[dpName].size()>maxROC_) maxROC_=dpMap_[dpName].size();

  }

  std::vector<PixelModuleName>::const_iterator module_name = theDetectorConfiguration_->getModuleList().begin();
  for (;module_name!=theDetectorConfiguration_->getModuleList().end();++module_name){
    // First we need to get the DAC settings for the ROCs on this module.
    PixelDACSettings *dacs=0; 
    std::string modulePath=module_name->modulename();
    PixelConfigInterface::get(dacs, "pixel/dac/"+modulePath, *theGlobalKey_);
    assert(dacs!=0);
    dacsettings_[*module_name]=dacs;
  }

  PixelConfigInterface::get(maxVsf_,"pixel/maxvsf/",*theGlobalKey_);
  
  assert(maxVsf_!=0);


}


void PixelIdigiCalibration::endCalibration(){
  cout << "In PixelIdigiCalibration::endCalibration()" << endl;

  ofstream out((outputDir()+"/idigi.dat").c_str());
  assert(out.good());


  map<string, vector<pos::PixelROCName> >::iterator idpName=dpMap_.begin();

  for(;idpName!=dpMap_.end();++idpName){
 
    unsigned int vsize=idpName->second.size();

    for(unsigned i=0;i<vsize;i++){

      PixelROCName theROC=idpName->second[i];
      PixelModuleName theModule(theROC.rocname());

      double x[256],y[256];
      double ey[256];

      cout << idpName->second[i];

      out << idpName->second[i]<<endl;

      out << 26 << endl;

      for (unsigned j=0;j<26;j++){

	y[j]=Idigi_[idpName->first][i][j].mean();
	//FIXME hardcoded...
	x[j]=10.0*j;
	ey[j]=0.002;

	cout << " " << Idigi_[idpName->first][i][j].mean();
	out << Idigi_[idpName->first][i][j].mean()<<" ";

      }

      cout <<endl;

      out <<endl;

      TF1* f2 = new TF1("f2","(x<[0])*([2]+([3]-[2])*exp(([4]-[3])*(x-[0])/(([1]-[0])*([3]-[2]))))+(x>[1])*[4]+(x>[0])*(x<[1])*([3]+(x-[0])*([4]-[3])/([1]-[0]))",0.0,250.0);
      f2->SetParameters(180,220,y[0],0.5*(y[0]+y[24]),y[24]);
      f2->SetParLimits(0,10.0,220.0);
      f2->SetParLimits(1,220.0,240);

      TGraphErrors* gr = new TGraphErrors(25,x,y,0,ey);
      gr->Fit("f2");

      double yvalatzero=f2->Eval(0.0);

      out << yvalatzero <<endl;

      for (int ivsf=0;ivsf<26;ivsf++){
	y[ivsf]=1000*(y[ivsf]-yvalatzero);
	ey[ivsf]=2;
      }

      f2->SetParameters(180,220,y[0],0.5*(y[0]+y[24]),y[24]);
      f2->SetParLimits(0,10.0,220.0);
      f2->SetParLimits(1,220.0,240);

      delete gr;
      
      gr = new TGraphErrors(25,x,y,0,ey);
      gr->Fit("f2");

      out << f2->GetParameter(0)<<" ";
      out << f2->GetParameter(1)<<" ";
      out << f2->GetParameter(2)<<" ";
      out << f2->GetParameter(3)<<" ";
      out << f2->GetParameter(4)<<endl;

      TF1* fit = gr->GetFunction("f2");

      //double ianacurrent=fit->Eval(140.0);   
      
      unsigned int vsf=0;
      for(;vsf<250.0;vsf++){
	double idigi=fit->Eval(vsf);
	if (idigi>5.0){
	  break;
	}
      }

      unsigned int oldVsf;

      bool foundROC=maxVsf_->getVsf(theROC,oldVsf);
      //FIXME should not assert!
      assert(foundROC);

      maxVsf_->setVsf(theROC,vsf);

      cout << "Old Vsf="<<oldVsf<<endl;
      cout << "New max Vsf="<<vsf<<endl;

      out <<oldVsf<<endl;
      out <<vsf<<endl;

    }

  }

  maxVsf_->writeASCII(outputDir());

}

std::vector<std::string> PixelIdigiCalibration::calibrated(){

  std::vector<std::string> tmp;

  tmp.push_back("maxvsf");

  return tmp;

}
