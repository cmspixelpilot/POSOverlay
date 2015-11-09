#ifndef PixelDACNames_h
#define PixelDACNames_h
/*! \file CalibFormats/SiPixelObjects/interface/PixelDACNames.h
*   \brief A dummy class with ALL public variables 
*
*   A longer explanation will be placed here later
*/

#include <string>

namespace pos{

  const std::string k_DACName_Vdd="Vdd";
  const std::string k_DACName_Vana="Vana";
  const std::string k_DACName_Vsh="Vsh";
  const std::string k_DACName_Vsf="Vsh"; // JMTBAD
  const std::string k_DACName_Vcomp="Vcomp";
  const std::string k_DACName_VwllPr="VwllPr";
  const std::string k_DACName_VwllSh="VwllSh";
  const std::string k_DACName_VHldDel="VHldDel";
  const std::string k_DACName_Vtrim="Vtrim";
  const std::string k_DACName_VcThr="VcThr";
  const std::string k_DACName_VIbias_bus="VIbias_bus";
  const std::string k_DACName_PHOffset="PHOffset";
  const std::string k_DACName_Vcomp_ADC="Vcomp_ADC";
  const std::string k_DACName_PHScale="PHScale";
  const std::string k_DACName_VIColOr="VIColOr";
  const std::string k_DACName_Vcal="Vcal";
  const std::string k_DACName_CalDel="CalDel";
  const std::string k_DACName_TempRange="TempRange";
  const std::string k_DACName_WBC="WBC";
  const std::string k_DACName_ChipContReg="ChipContReg";
  const std::string k_DACName_Readback="Readback";

  const unsigned int k_DACAddress_Vdd=1;
  const unsigned int k_DACAddress_Vana=2;
  const unsigned int k_DACAddress_Vsh=3;
  const unsigned int k_DACAddress_Vsf=3; // JMTBAD
  const unsigned int k_DACAddress_Vcomp=4;
  const unsigned int k_DACAddress_VwllPr=7;
  const unsigned int k_DACAddress_VwllSh=9;
  const unsigned int k_DACAddress_VHldDel=10;
  const unsigned int k_DACAddress_Vtrim=11;
  const unsigned int k_DACAddress_VcThr=12;
  const unsigned int k_DACAddress_VIbias_bus=13;
  const unsigned int k_DACAddress_PHOffset=17;
  const unsigned int k_DACAddress_Vcomp_ADC=19;
  const unsigned int k_DACAddress_PHScale=20;
  const unsigned int k_DACAddress_VIColOr=22;
  const unsigned int k_DACAddress_Vcal=25;
  const unsigned int k_DACAddress_CalDel=26;
  const unsigned int k_DACAddress_TempRange=27;
  const unsigned int k_DACAddress_WBC=254;
  const unsigned int k_DACAddress_ChipContReg=253;
  const unsigned int k_DACAddress_Readback=255;

  const std::string k_DACName_TBMADelay="TBMADelay";
  const std::string k_DACName_TBMBDelay="TBMBDelay";
  const std::string k_DACName_TBMPLL="TBMPLL";
  
  const unsigned int k_DACAddress_TBMADelay=5;
  const unsigned int k_DACAddress_TBMBDelay=5;
  const unsigned int k_DACAddress_TBMPLL=7;
  
}


#endif
