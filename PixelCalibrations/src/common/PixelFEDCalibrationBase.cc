// Modified by Jennifer Vaughan 2007/06/01
// $Id: PixelCalibrationBase.cc,v 1.1 

/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 * Authors: J. Gutleber and L. Orsini					 *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/
#include <math.h>

#include "PixelCalibrations/include/PixelFEDCalibrationBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelFEDTestDAC.h"

using namespace pos;

PixelFEDCalibrationBase::PixelFEDCalibrationBase(const PixelFEDSupervisorConfiguration & tempConfiguration, const SOAPCommander& soapCommander) 
  : PixelFEDSupervisorConfiguration(tempConfiguration),
    SOAPCommander(soapCommander)
{
  event_=0;
  std::cout << "Greetings from the PixelFEDCalibrationBase copy constructor." << std::endl;

  //the copying of pointers is taken care of in PixSupConf
}

xoap::MessageReference PixelFEDCalibrationBase::beginCalibration(xoap::MessageReference msg){

  event_=0;

  return MakeSOAPMessageReference("PixelFEDCalibrationBase::beginCalibration Done");

}


xoap::MessageReference PixelFEDCalibrationBase::endCalibration(xoap::MessageReference msg){

  return MakeSOAPMessageReference("PixelFEDCalibrationBase::endCalibration Done");

}


unsigned int PixelFEDCalibrationBase::TransparentDataStart (uint32_t *buffer, int fed, int channel)
{
  const unsigned int max=512;

  float B_mean=0, B_stddev=0, B_sum=0, B_squares=0;
  unsigned int i=1;

  do
    {
      B_sum+=((0xffc00000 & buffer[i])>>22);
      B_squares+=pow(float((0xffc00000 & buffer[i])>>22),2);
      B_mean=B_sum/i;
      B_stddev=(B_squares/i-B_mean*B_mean);
      if(0>B_stddev)
	{
	  B_stddev=0;
	}
      else
	{
	  B_stddev=sqrt(B_stddev);
	}
      //		cout<<"Mean of black so far at position i="<<dec<<i<<" is "<<B_mean<<" with std dev = "<<B_stddev<<endl;
      i+=1;
    } while ((
	      !(
		((0xffc00000 & buffer[i])>>22)<(B_mean-3*B_stddev-50) &&
		((0xffc00000 & buffer[i+1])>>22)<(B_mean-3*B_stddev-50) &&
		((0xffc00000 & buffer[i+2])>>22)<(B_mean-3*B_stddev-50)
		) || i<8)
	     && i<max);

  if (i>=max) {
    ostringstream msg;
    msg<<"Cannot find beginning of data!";
    if (fed!=-1) msg<<" FED,ch = "<<fed<<","<<channel;
    diagService_->reportError(msg.str(),DIAGERROR);
  }

  return i;
}


void PixelFEDCalibrationBase::setFEDModeAndControlRegister(unsigned int mode, 
							    unsigned int control){

  for (FEDInterfaceMap::iterator iFED=FEDInterface_.begin();iFED!=FEDInterface_.end();++iFED) { 
    iFED->second->setModeRegister(mode);
    iFED->second->setControlRegister(control);
  }
}

void PixelFEDCalibrationBase::baselinecorr_off(){

  for (FEDInterfaceMap::iterator iFED=FEDInterface_.begin();iFED!=FEDInterface_.end();++iFED) { 
    iFED->second->BaselineCorr_off();
  }
}

void PixelFEDCalibrationBase::setSpecialDac(unsigned int mode){

  for (FEDInterfaceMap::iterator iFED=FEDInterface_.begin();iFED!=FEDInterface_.end();++iFED) { 
    iFED->second->set_SpecialDac(mode);
  }
}

void PixelFEDCalibrationBase::fillTestDAC(xoap::MessageReference fillTestDACmsg){

  if (theCalibObject_==0) {

    std::cout<<"PixelFEDSupervisor::FillTestDAC - theCalibObject_ doesn't exist!"<<std::endl;
    diagService_->reportError("The theCalibObject_ doesn't exist.", DIAGERROR);

  } else {

    std::vector<unsigned int> pulseTrain1(256), pulseTrain2(256), pulseTrain3(256);
    PixelFEDTestDAC* testDACCalib=dynamic_cast<PixelFEDTestDAC*>(theCalibObject_);
    assert(testDACCalib);
    pulseTrain1=testDACCalib->dacs();
    pulseTrain2=testDACCalib->dacs();
    pulseTrain3=testDACCalib->dacs();
    //for(unsigned int ii=0;ii<256;ii++){      
    //  pulseTrain1[ii]=500;
    //  if (ii<30&&ii>20)  pulseTrain1[ii]=300;
    //}

    pulseTrain2=pulseTrain1;
    pulseTrain3=pulseTrain1;

    for (unsigned int i=0; i<pulseTrain1.size(); ++i) {
      std::cout<<"PixelFEDSupervisor::FillTestDAC. pulseTrain1["<<i<<"]="<<pulseTrain1[i]<<std::endl;
    }

    for (FEDInterfaceMap::iterator iFED=FEDInterface_.begin();iFED!=FEDInterface_.end();++iFED) {
      iFED->second->setup_testDAC(512);
      iFED->second->fillDACRegister(pulseTrain1, pulseTrain2, pulseTrain3);
    }

  }


}

void PixelFEDCalibrationBase::setBlackUBTrans(){

  for (FEDInterfaceMap::iterator iFED=FEDInterface_.begin();iFED!=FEDInterface_.end();++iFED) { 
    iFED->second->set_blk_ublk_trans_thold();
  }
}

