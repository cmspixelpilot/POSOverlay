//
// This class collects the information
// about the range of DAC settings used
// in scans of the DACs.
//
//
//

#include "CalibFormats/SiPixelObjects/interface/PixelDACScanRange.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"
#include <iostream>
#include <assert.h>

using namespace pos;

PixelDACScanRange::PixelDACScanRange(std::string name, unsigned int first, 
                                     unsigned int last, int step,
                                     unsigned int index, bool mixValuesAcrossROCs){

  uniformSteps_=true;
  relative_=false;
  negative_=false;

  first_=first;
  last_=last;
  step_=step;
  assert(step != 0);

  name_=name;
  index_=index;
  mixValuesAcrossROCs_=mixValuesAcrossROCs;
  if (first_==last_) assert( mixValuesAcrossROCs==false );
  while((step > 0 && first<=last) || (step < 0 && first >= last)){
    values_.push_back(first);
    if (step < 0 && first == 0)
      break;
    first+=step;
    //FIXME should have a better reporting
    assert(values_.size()<1000);
  }

  isTBM_ = name.compare(0,3,"TBM") == 0;
  tbmchannel_ = 14;

  setDACChannel(name);
}

PixelDACScanRange::PixelDACScanRange(std::string name, 
				     std::vector<unsigned int> values,
                                     unsigned int index, bool mixValuesAcrossROCs){

  uniformSteps_=false;
  relative_=false;
  negative_=false;

  name_=name;
  values_=values;
  mixValuesAcrossROCs_=mixValuesAcrossROCs;
  assert( mixValuesAcrossROCs==false );

  isTBM_ = name.compare(0,3,"TBM") == 0;

  setDACChannel(name);
}


void PixelDACScanRange::setDACChannel(std::string name){
  if (name==pos::k_DACName_Vdd)                {
    dacchannel_=pos::k_DACAddress_Vdd          ;
  } else if (name==pos::k_DACName_Vana)        {
    dacchannel_=pos::k_DACAddress_Vana         ;
  } else if (name==pos::k_DACName_Vsh)         {
    dacchannel_=pos::k_DACAddress_Vsh          ;
  } else if (name==pos::k_DACName_Vcomp)       {
    dacchannel_=pos::k_DACAddress_Vcomp        ;
  } else if (name==pos::k_DACName_VwllPr)      {
    dacchannel_=pos::k_DACAddress_VwllPr       ;
  } else if (name==pos::k_DACName_VwllSh)      {
    dacchannel_=pos::k_DACAddress_VwllSh       ;
  } else if (name==pos::k_DACName_VHldDel)     {
    dacchannel_=pos::k_DACAddress_VHldDel      ;
  } else if (name==pos::k_DACName_Vtrim)       {
    dacchannel_=pos::k_DACAddress_Vtrim        ;
  } else if (name==pos::k_DACName_VcThr)       {
    dacchannel_=pos::k_DACAddress_VcThr        ;
  } else if (name==pos::k_DACName_VIbias_bus)  {
    dacchannel_=pos::k_DACAddress_VIbias_bus   ;
  } else if (name==pos::k_DACName_PHOffset)    {
    dacchannel_=pos::k_DACAddress_PHOffset     ;
  } else if (name==pos::k_DACName_Vcomp_ADC)   {
    dacchannel_=pos::k_DACAddress_Vcomp_ADC    ;
  } else if (name==pos::k_DACName_PHScale)     {
    dacchannel_=pos::k_DACAddress_PHScale      ;
  } else if (name==pos::k_DACName_VIColOr)     {
    dacchannel_=pos::k_DACAddress_VIColOr      ;
  } else if (name==pos::k_DACName_Vcal)        {
    dacchannel_=pos::k_DACAddress_Vcal         ;
  } else if (name==pos::k_DACName_CalDel)      {
    dacchannel_=pos::k_DACAddress_CalDel       ;
  } else if (name==pos::k_DACName_TempRange)   {
    dacchannel_=pos::k_DACAddress_TempRange    ;
  } else if (name==pos::k_DACName_WBC)         {
    dacchannel_=pos::k_DACAddress_WBC          ;
  } else if (name==pos::k_DACName_ChipContReg) {
    dacchannel_=pos::k_DACAddress_ChipContReg  ;
  } else if (name==pos::k_DACName_Readback)    {
    dacchannel_=pos::k_DACAddress_Readback     ;
  } else if (name==pos::k_DACName_TBMADelay)  {
    dacchannel_ = pos::k_DACAddress_TBMADelay ;
  } else if (name==pos::k_DACName_TBMBDelay)  {
    dacchannel_ = pos::k_DACAddress_TBMBDelay ;
    tbmchannel_ = 15;
  } else if (name==pos::k_DACName_TBMPLL)    {
    dacchannel_ = pos::k_DACAddress_TBMPLL   ;
  } else {
    std::cout << __LINE__ << "]\t[PixelDACScanRange::setDACChannel()]\t\t\t    " 
	      << "The dac name: " << name << " is unknown!" << std::endl;
    assert(0);
  }
}

void PixelDACScanRange::dump(bool dump_values) const {
  std::cout << "name: " << name() << " dacchannel: " << dacchannel() << " isTBM: "<< isTBM();
  if (dump_values) {
    std::cout << " values: [";
    for (size_t i = 0; i < values_.size(); ++i)
      std::cout << values_[i] << " ";
  }
  std::cout << "]";
}
