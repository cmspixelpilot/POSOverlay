//
// This class provide the data structure for the
// ROC DAC parameters
//
// At this point I do not see a reason to make an
// abstract layer for this code.
//

#include "CalibFormats/SiPixelObjects/interface/PixelROCDACSettings.h"
#include "CalibFormats/SiPixelObjects/interface/PixelDACNames.h"
#include <fstream>
#include <iostream>

using namespace pos;
using namespace std;

PixelROCDACSettings::PixelROCDACSettings(){}


void PixelROCDACSettings::getDACs(vector<unsigned int>& dacs) const
{
    dacs.clear();
    dacs.push_back(Vdd_);
    dacs.push_back(Vana_);
    dacs.push_back(Vsh_);
    dacs.push_back(Vcomp_);
    dacs.push_back(0);  // JMTBAD hardcoded
    dacs.push_back(0);
    dacs.push_back(VwllPr_);
    dacs.push_back(0);
    dacs.push_back(VwllSh_);
    dacs.push_back(VHldDel_);
    dacs.push_back(Vtrim_);
    dacs.push_back(VcThr_);
    dacs.push_back(VIbias_bus_);
    dacs.push_back(0);
    dacs.push_back(0);
    dacs.push_back(0);
    dacs.push_back(PHOffset_);
    dacs.push_back(0);
    dacs.push_back(Vcomp_ADC_);
    dacs.push_back(PHScale_);
    dacs.push_back(0);
    dacs.push_back(VIColOr_);
    dacs.push_back(0);
    dacs.push_back(0);
    dacs.push_back(Vcal_);
    dacs.push_back(CalDel_);
    dacs.push_back(TempRange_);
    dacs.push_back(WBC_);
    dacs.push_back(ChipContReg_);
    dacs.push_back(Readback_);
}

// Added by Dario
void PixelROCDACSettings::getDACs(std::map<std::string, unsigned int>& dacs) const
{
    dacs.clear();
    dacs[k_DACName_Vdd         ] = Vdd_        ;
    dacs[k_DACName_Vana        ] = Vana_       ;
    dacs[k_DACName_Vsh         ] = Vsh_        ;
    dacs[k_DACName_Vcomp       ] = Vcomp_      ;
    dacs[k_DACName_VwllPr      ] = VwllPr_     ;
    dacs[k_DACName_VwllSh      ] = VwllSh_     ;
    dacs[k_DACName_VHldDel     ] = VHldDel_    ;
    dacs[k_DACName_Vtrim       ] = Vtrim_      ;
    dacs[k_DACName_VcThr       ] = VcThr_      ;
    dacs[k_DACName_VIbias_bus  ] = VIbias_bus_ ;
    dacs[k_DACName_PHOffset    ] = PHOffset_   ;
    dacs[k_DACName_Vcomp_ADC   ] = Vcomp_ADC_  ;
    dacs[k_DACName_PHScale     ] = PHScale_    ;
    dacs[k_DACName_VIColOr     ] = VIColOr_    ;
    dacs[k_DACName_Vcal        ] = Vcal_       ;
    dacs[k_DACName_CalDel      ] = CalDel_     ;
    dacs[k_DACName_TempRange   ] = TempRange_  ;
    dacs[k_DACName_WBC         ] = WBC_        ;
    dacs[k_DACName_ChipContReg ] = ChipContReg_;
    dacs[k_DACName_Readback    ] = Readback_   ;
}

// Added by Dario
void PixelROCDACSettings::setDACs(std::map<std::string, unsigned int>& dacs) 
{
    Vdd_         = dacs[k_DACName_Vdd         ];
    Vana_        = dacs[k_DACName_Vana        ];
    Vsh_         = dacs[k_DACName_Vsh         ];
    Vcomp_       = dacs[k_DACName_Vcomp       ];
    VwllPr_      = dacs[k_DACName_VwllPr      ];
    VwllSh_      = dacs[k_DACName_VwllSh      ];
    VHldDel_     = dacs[k_DACName_VHldDel     ];
    Vtrim_       = dacs[k_DACName_Vtrim       ];
    VcThr_       = dacs[k_DACName_VcThr       ];
    VIbias_bus_  = dacs[k_DACName_VIbias_bus  ];
    PHOffset_    = dacs[k_DACName_PHOffset    ];
    Vcomp_ADC_   = dacs[k_DACName_Vcomp_ADC   ];
    PHScale_     = dacs[k_DACName_PHScale     ];
    VIColOr_     = dacs[k_DACName_VIColOr     ];
    Vcal_        = dacs[k_DACName_Vcal        ];
    CalDel_      = dacs[k_DACName_CalDel      ];
    TempRange_   = dacs[k_DACName_TempRange   ];
    WBC_         = dacs[k_DACName_WBC         ];
    ChipContReg_ = dacs[k_DACName_ChipContReg ];
    Readback_    = dacs[k_DACName_Readback    ];
}

// Added by Dario
void PixelROCDACSettings::compareDACs(std::map<std::string, unsigned int>& dacs, 
                                      std::map<std::string, bool>& changes, 
				      std::map<std::string, unsigned int>& previous) 
{
    changes[k_DACName_Vdd        ] = false;
    changes[k_DACName_Vana       ] = false;
    changes[k_DACName_Vsh        ] = false;
    changes[k_DACName_Vcomp      ] = false;
    changes[k_DACName_VwllPr     ] = false;
    changes[k_DACName_VwllSh     ] = false;
    changes[k_DACName_VHldDel    ] = false;
    changes[k_DACName_Vtrim      ] = false;
    changes[k_DACName_VcThr      ] = false;
    changes[k_DACName_VIbias_bus ] = false;
    changes[k_DACName_PHOffset   ] = false;
    changes[k_DACName_Vcomp_ADC  ] = false;
    changes[k_DACName_PHScale    ] = false;
    changes[k_DACName_VIColOr    ] = false;
    changes[k_DACName_Vcal       ] = false;
    changes[k_DACName_CalDel     ] = false;
    changes[k_DACName_TempRange  ] = false;
    changes[k_DACName_WBC        ] = false;
    changes[k_DACName_ChipContReg] = false;
    changes[k_DACName_Readback   ] = false;

    if( Vdd_         != dacs[k_DACName_Vdd         ] ) {changes[k_DACName_Vdd         ] = true; previous[k_DACName_Vdd         ] = Vdd_        ;}
    if( Vana_        != dacs[k_DACName_Vana        ] ) {changes[k_DACName_Vana        ] = true; previous[k_DACName_Vana        ] = Vana_       ;}
    if( Vsh_         != dacs[k_DACName_Vsh         ] ) {changes[k_DACName_Vsh         ] = true; previous[k_DACName_Vsh         ] = Vsh_        ;}
    if( Vcomp_       != dacs[k_DACName_Vcomp       ] ) {changes[k_DACName_Vcomp       ] = true; previous[k_DACName_Vcomp       ] = Vcomp_      ;}
    if( VwllPr_      != dacs[k_DACName_VwllPr      ] ) {changes[k_DACName_VwllPr      ] = true; previous[k_DACName_VwllPr      ] = VwllPr_     ;}
    if( VwllSh_      != dacs[k_DACName_VwllSh      ] ) {changes[k_DACName_VwllSh      ] = true; previous[k_DACName_VwllSh      ] = VwllSh_     ;}
    if( VHldDel_     != dacs[k_DACName_VHldDel     ] ) {changes[k_DACName_VHldDel     ] = true; previous[k_DACName_VHldDel     ] = VHldDel_    ;}
    if( Vtrim_       != dacs[k_DACName_Vtrim       ] ) {changes[k_DACName_Vtrim       ] = true; previous[k_DACName_Vtrim       ] = Vtrim_      ;}
    if( VcThr_       != dacs[k_DACName_VcThr       ] ) {changes[k_DACName_VcThr       ] = true; previous[k_DACName_VcThr       ] = VcThr_      ;}
    if( VIbias_bus_  != dacs[k_DACName_VIbias_bus  ] ) {changes[k_DACName_VIbias_bus  ] = true; previous[k_DACName_VIbias_bus  ] = VIbias_bus_ ;}
    if( PHOffset_    != dacs[k_DACName_PHOffset    ] ) {changes[k_DACName_PHOffset    ] = true; previous[k_DACName_PHOffset    ] = PHOffset_   ;}
    if( Vcomp_ADC_   != dacs[k_DACName_Vcomp_ADC   ] ) {changes[k_DACName_Vcomp_ADC   ] = true; previous[k_DACName_Vcomp_ADC   ] = Vcomp_ADC_  ;}
    if( PHScale_     != dacs[k_DACName_PHScale     ] ) {changes[k_DACName_PHScale     ] = true; previous[k_DACName_PHScale     ] = PHScale_    ;}
    if( VIColOr_     != dacs[k_DACName_VIColOr     ] ) {changes[k_DACName_VIColOr     ] = true; previous[k_DACName_VIColOr     ] = VIColOr_    ;}
    if( Vcal_        != dacs[k_DACName_Vcal        ] ) {changes[k_DACName_Vcal        ] = true; previous[k_DACName_Vcal        ] = Vcal_       ;}
    if( CalDel_      != dacs[k_DACName_CalDel      ] ) {changes[k_DACName_CalDel      ] = true; previous[k_DACName_CalDel      ] = CalDel_     ;}
    if( TempRange_   != dacs[k_DACName_TempRange   ] ) {changes[k_DACName_TempRange   ] = true; previous[k_DACName_TempRange   ] = TempRange_  ;}
    if( WBC_         != dacs[k_DACName_WBC         ] ) {changes[k_DACName_WBC         ] = true; previous[k_DACName_WBC         ] = WBC_        ;}
    if( ChipContReg_ != dacs[k_DACName_ChipContReg ] ) {changes[k_DACName_ChipContReg ] = true; previous[k_DACName_ChipContReg ] = ChipContReg_;}
    if( Readback_    != dacs[k_DACName_Readback    ] ) {changes[k_DACName_Readback    ] = true; previous[k_DACName_Readback    ] = Readback_   ;}
}		   								            
										       
void PixelROCDACSettings::setDAC(unsigned int dacaddress, unsigned int dacvalue)       
{
        std::string mthn = "[PixelROCDACSettings::setDAC()]\t\t\t\t    " ;
	switch (dacaddress) {
		case   1: Vdd_         = dacvalue;  break;
		case   2: Vana_        = dacvalue;  break;
		case   3: Vsh_         = dacvalue;  break;
		case   4: Vcomp_       = dacvalue;  break;
		case   7: VwllPr_      = dacvalue;  break;
		case   9: VwllSh_      = dacvalue;  break;
		case  10: VHldDel_     = dacvalue;  break;
		case  11: Vtrim_       = dacvalue;  break;
		case  12: VcThr_       = dacvalue;  break;
		case  13: VIbias_bus_  = dacvalue;  break;
		case  17: PHOffset_    = dacvalue;  break;
		case  19: Vcomp_ADC_   = dacvalue;  break;
		case  20: PHScale_     = dacvalue;  break;
		case  22: VIColOr_     = dacvalue;  break;
		case  25: Vcal_        = dacvalue;  break;
		case  26: CalDel_      = dacvalue;  break;
		case  27: TempRange_   = dacvalue;  break;
		case 254: WBC_         = dacvalue;  break;
		case 253: ChipContReg_ = dacvalue;  break;
		case 255: Readback_    = dacvalue;  break;
		default: cout << __LINE__ << "]\t" << mthn 
		              << "DAC Address " << dacaddress << " does not exist!" << endl;
	}

}

void PixelROCDACSettings::writeBinary(ofstream& out) const
{
    out << (char)rocid_.rocname().size();
    out.write(rocid_.rocname().c_str(),rocid_.rocname().size());

    out << Vdd_;
    out << Vana_;
    out << Vsh_;
    out << Vcomp_;
    out << VwllPr_;
    out << VwllSh_;
    out << VHldDel_;
    out << Vtrim_;
    out << VcThr_;
    out << VIbias_bus_;
    out << PHOffset_;
    out << Vcomp_ADC_;
    out << PHScale_;
    out << VIColOr_;
    out << Vcal_;
    out << CalDel_;
    out << TempRange_;
    out << WBC_;
    out << ChipContReg_;
    out << Readback_;
}


int PixelROCDACSettings::readBinary(ifstream& in, const PixelROCName& rocid){
    
    rocid_=rocid;

    in.read((char*)&Vdd_,1);
    in.read((char*)&Vana_,1);
    in.read((char*)&Vsh_,1);
    in.read((char*)&Vcomp_,1);
    in.read((char*)&VwllPr_,1);
    in.read((char*)&VwllSh_,1);
    in.read((char*)&VHldDel_,1);
    in.read((char*)&Vtrim_,1);
    in.read((char*)&VcThr_,1);
    in.read((char*)&VIbias_bus_,1);
    in.read((char*)&PHOffset_,1);
    in.read((char*)&Vcomp_ADC_,1);
    in.read((char*)&PHScale_,1);
    in.read((char*)&VIColOr_,1);
    in.read((char*)&Vcal_,1);
    in.read((char*)&CalDel_,1);
    in.read((char*)&TempRange_,1);
    in.read((char*)&WBC_,1);
    in.read((char*)&ChipContReg_,1);
    in.read((char*)&Readback_,1);
    
    return 1;

}

void PixelROCDACSettings::writeASCII(ostream& out) const{

    out << "ROC:           " << rocid_.rocname()   <<endl;

    out << k_DACName_Vdd << ":           " << (int)Vdd_          <<endl;
    out << k_DACName_Vana << ":          " << (int)Vana_         <<endl;
    out << k_DACName_Vsh << ":           " << (int)Vsh_          <<endl;
    out << k_DACName_Vcomp << ":         " << (int)Vcomp_        <<endl;
    out << k_DACName_VwllPr << ":        " << (int)VwllPr_       <<endl;
    out << k_DACName_VwllSh << ":        " << (int)VwllSh_       <<endl;
    out << k_DACName_VHldDel << ":       " << (int)VHldDel_      <<endl;
    out << k_DACName_Vtrim << ":         " << (int)Vtrim_        <<endl;
    out << k_DACName_VcThr << ":         " << (int)VcThr_        <<endl;
    out << k_DACName_VIbias_bus << ":    " << (int)VIbias_bus_   <<endl;
    out << k_DACName_PHOffset << ":      " << (int)PHOffset_     <<endl;
    out << k_DACName_Vcomp_ADC << ":     " << (int)Vcomp_ADC_    <<endl;
    out << k_DACName_PHScale << ":       " << (int)PHScale_      <<endl;
    out << k_DACName_VIColOr << ":       " << (int)VIColOr_      <<endl;
    out << k_DACName_Vcal << ":          " << (int)Vcal_         <<endl;
    out << k_DACName_CalDel << ":        " << (int)CalDel_       <<endl;
    out << k_DACName_TempRange << ":     " << (int)TempRange_    <<endl;
    out << k_DACName_WBC << ":           " << (int)WBC_          <<endl;
    out << k_DACName_ChipContReg << ":   " << (int)ChipContReg_  <<endl;
    out << k_DACName_Readback << ":      " << (int)Readback_     <<endl;
}

//=============================================================================================
void PixelROCDACSettings::writeXML(ofstream *out) const {
  std::string mthn = "[PixelROCDACSettings::writeXML()]\t\t\t    " ;

  *out << "  <DATA>"                                                  << endl ;
  *out << "   <VDD>"            << (int)Vdd_         << "</VDD>"            << endl ;
  *out << "   <VANA>"           << (int)Vana_        << "</VANA>"           << endl ;
  *out << "   <VSH>"            << (int)Vsh_         << "</VSH>"            << endl ;
  *out << "   <VCOMP>"          << (int)Vcomp_       << "</VCOMP>"          << endl ;
  *out << "   <VWLLPR>"         << (int)VwllPr_      << "</VWLLPR>"         << endl ;
  *out << "   <VWLLSH>"         << (int)VwllSh_      << "</VWLLSH>"         << endl ;
  *out << "   <VHLDDEL>"        << (int)VHldDel_     << "</VHLDDEL>"        << endl ;
  *out << "   <VTRIM>"          << (int)Vtrim_       << "</VTRIM>"          << endl ;
  *out << "   <VCTHR>"          << (int)VcThr_       << "</VCTHR>"          << endl ;
  *out << "   <VIBIAS_BUS>"     << (int)VIbias_bus_  << "</VIBIAS_BUS>"     << endl ;
  *out << "   <PHOFFSET>"       << (int)PHOffset_    << "</PHOFFSET>"       << endl ;
  *out << "   <VCOMP_ADC>"      << (int)Vcomp_ADC_   << "</VCOMP_ADC>"      << endl ;
  *out << "   <PHSCALE>"        << (int)PHScale_     << "</PHSCALE>"        << endl ;
  *out << "   <VICOLOR>"        << (int)VIColOr_     << "</VICOLOR>"        << endl ;
  *out << "   <VCAL>"           << (int)Vcal_        << "</VCAL>"           << endl ;
  *out << "   <CALDEL>"         << (int)CalDel_      << "</CALDEL>"         << endl ;
  *out << "   <TEMPRANGE>"      << (int)TempRange_   << "</TEMPRANGE>"      << endl ;
  *out << "   <WBC>"            << (int)WBC_         << "</WBC>"            << endl ;
  *out << "   <CHIPCONTREG>"    << (int)ChipContReg_ << "</CHIPCONTREG>"    << endl ;
  *out << "   <READBACK>"       << (int)Readback_    << "</READBACK>"       << endl ;
  *out << "  </DATA>"						      << endl ;
  *out << " "							      << endl ;

}

//=============================================================================================
void PixelROCDACSettings::checkTag(string tag, 
				   string dacName,
				   const PixelROCName& rocid){
  
  std::string mthn = "[PixelROCDACSettings::checkTag()]\t\t\t\t    " ;
  dacName+=":";
  if (tag!=dacName) {
    cout << __LINE__ << "]\t" << mthn << "Read ROC name       : "	      << tag     << endl;
    cout << __LINE__ << "]\t" << mthn << "But expected to find: "	      << dacName << endl;
    cout << __LINE__ << "]\t" << mthn << "When reading DAC settings for ROC " << rocid   << endl;
    assert(0);
  }

}

int PixelROCDACSettings::read(std::istringstream& in, const PixelROCName& rocid)
{
    std::string mthn = "[PixelROCDACSettings::read()]\t\t\t\t    " ;

    rocid_=rocid;

    unsigned int tmp;
    string tag;

    //    cout << "[PixelROCDACSettings::read()] |" << in.str() << "|" << endl ;

    in >> tag; 
    checkTag(tag,k_DACName_Vdd,rocid);
    in >> tmp; Vdd_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_Vana,rocid);
    in >> tmp; Vana_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_Vsh,rocid);
    in >> tmp; Vsh_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_Vcomp,rocid);
    in >> tmp; Vcomp_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_VwllPr,rocid);
    in >> tmp; VwllPr_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_VwllSh,rocid);
    in >> tmp; VwllSh_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_VHldDel,rocid);
    in >> tmp; VHldDel_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_Vtrim,rocid);
    in >> tmp; Vtrim_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_VcThr,rocid);
    in >> tmp; VcThr_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_VIbias_bus,rocid);
    in >> tmp; VIbias_bus_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_PHOffset,rocid);
    in >> tmp; PHOffset_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_Vcomp_ADC,rocid);
    in >> tmp; Vcomp_ADC_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_PHScale,rocid);
    in >> tmp; PHScale_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_VIColOr,rocid);
    in >> tmp; VIColOr_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_Vcal,rocid);
    in >> tmp; Vcal_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_CalDel,rocid);
    in >> tmp; CalDel_=tmp;
    in >> tag; 
    if (tag==k_DACName_WBC+":"){
      static bool first=true;
      if (first){
	cout << __LINE__ << "]\t" << mthn << "**********************************************" << endl;
	cout << __LINE__ << "]\t" << mthn << "Did not find TempRange setting in DAC settings" << endl;
	cout << __LINE__ << "]\t" << mthn << "Will use a default value of 4."                 << endl;
	cout << __LINE__ << "]\t" << mthn << "This message will only be printed out once"     << endl;
	cout << __LINE__ << "]\t" << mthn << "**********************************************" << endl;
	TempRange_=4;
	first=false;
      }
      in >> tmp; WBC_=tmp;
    } else {	
      checkTag(tag,k_DACName_TempRange,rocid);
      in >> tmp; TempRange_=tmp;
      in >> tag; 
      checkTag(tag,k_DACName_WBC,rocid);
      in >> tmp; WBC_=tmp;
    }
    in >> tag; 
    checkTag(tag,k_DACName_ChipContReg,rocid);
    in >> tmp; ChipContReg_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_Readback,rocid);
    in >> tmp; Readback_=tmp;

    return 0;
}

int PixelROCDACSettings::read(ifstream& in, const PixelROCName& rocid){
    
    std::string mthn = "[PixelROCDACSettings::read()]\t\t\t\t    " ;
    rocid_=rocid;

    unsigned int tmp;
    string tag;

    in >> tag; 
    checkTag(tag,k_DACName_Vdd,rocid);
    in >> tmp; Vdd_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_Vana,rocid);
    in >> tmp; Vana_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_Vsh,rocid);
    in >> tmp; Vsh_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_Vcomp,rocid);
    in >> tmp; Vcomp_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_VwllPr,rocid);
    in >> tmp; VwllPr_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_VwllSh,rocid);
    in >> tmp; VwllSh_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_VHldDel,rocid);
    in >> tmp; VHldDel_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_Vtrim,rocid);
    in >> tmp; Vtrim_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_VcThr,rocid);
    in >> tmp; VcThr_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_VIbias_bus,rocid);
    in >> tmp; VIbias_bus_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_PHOffset,rocid);
    in >> tmp; PHOffset_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_Vcomp_ADC,rocid);
    in >> tmp; Vcomp_ADC_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_PHScale,rocid);
    in >> tmp; PHScale_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_VIColOr,rocid);
    in >> tmp; VIColOr_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_Vcal,rocid);
    in >> tmp; Vcal_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_CalDel,rocid);
    in >> tmp; CalDel_=tmp;
    in >> tag; 
    if (tag==k_DACName_WBC+":"){
      static bool first=true;
      if (first){
	cout << __LINE__ << "]\t" << mthn << "**********************************************" << endl;
	cout << __LINE__ << "]\t" << mthn << "Did not find TempRange setting in DAC settings" << endl;
	cout << __LINE__ << "]\t" << mthn << "Will use a default value of 4."                 << endl;
	cout << __LINE__ << "]\t" << mthn << "This message will only be printed out once"     << endl;
	cout << __LINE__ << "]\t" << mthn << "**********************************************" << endl;
	TempRange_=4;
	first=false;
      }
      in >> tmp; WBC_=tmp;
    } else {	
      checkTag(tag,k_DACName_TempRange,rocid);
      in >> tmp; TempRange_=tmp;
      in >> tag; 
      checkTag(tag,k_DACName_WBC,rocid);
      in >> tmp; WBC_=tmp;
    }
    in >> tag; 
    checkTag(tag,k_DACName_ChipContReg,rocid);
    in >> tmp; ChipContReg_=tmp;
    in >> tag; 
    checkTag(tag,k_DACName_Readback,rocid);
    in >> tmp; Readback_=tmp;

    return 0;
}


string PixelROCDACSettings::getConfigCommand(){

  string s;

  return s;

}

ostream& pos::operator<<(ostream& s, const PixelROCDACSettings& dacs){
  
  s << k_DACName_Vdd << "           :" << (unsigned int)dacs.Vdd_ << endl;
  s << k_DACName_Vana << "          :" << (unsigned int)dacs.Vana_ << endl;
  s << k_DACName_Vsh << "           :" << (unsigned int)dacs.Vsh_ << endl;
  s << k_DACName_Vcomp << "         :" << (unsigned int)dacs.Vcomp_ << endl;
  s << k_DACName_VwllPr << "        :" << (unsigned int)dacs.VwllPr_ << endl;
  s << k_DACName_VwllSh << "        :" << (unsigned int)dacs.VwllSh_ << endl;
  s << k_DACName_VHldDel << "       :" << (unsigned int)dacs.VHldDel_ << endl;
  s << k_DACName_Vtrim << "         :" << (unsigned int)dacs.Vtrim_ << endl;
  s << k_DACName_VcThr << "         :" << (unsigned int)dacs.VcThr_ << endl;
  s << k_DACName_VIbias_bus << "    :" << (unsigned int)dacs.VIbias_bus_ << endl;
  s << k_DACName_PHOffset << "      :" << (unsigned int)dacs.PHOffset_ << endl;
  s << k_DACName_Vcomp_ADC << "     :" << (unsigned int)dacs.Vcomp_ADC_ << endl;
  s << k_DACName_PHScale << "       :" << (unsigned int)dacs.PHScale_ << endl;
  s << k_DACName_VIColOr << "       :" << (unsigned int)dacs.VIColOr_ << endl;
  s << k_DACName_Vcal << "          :" << (unsigned int)dacs.Vcal_ << endl;
  s << k_DACName_CalDel << "        :" << (unsigned int)dacs.CalDel_ << endl;
  s << k_DACName_TempRange << "     :" << (unsigned int)dacs.TempRange_ << endl;
  s << k_DACName_WBC << "           :" << (unsigned int)dacs.WBC_ << endl;
  s << k_DACName_ChipContReg << "   :" << (unsigned int)dacs.ChipContReg_ << endl;
  s << k_DACName_Readback << "      :" << (unsigned int)dacs.Readback_ << endl;
  
  return s;

}

//Added by Umesh
void PixelROCDACSettings::setDac(string dacName, int dacValue){
  if(ToLower(dacName) == ToLower(k_DACName_Vdd)){
    Vdd_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_Vdd)){
    Vdd_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_Vana)){
    Vana_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_Vsh)){
    Vsh_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_Vcomp)){
    Vcomp_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_VwllPr)){
    VwllPr_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_VwllSh)){
    VwllSh_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_VHldDel)){
    VHldDel_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_Vtrim)){
    Vtrim_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_VcThr)){
    VcThr_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_VIbias_bus)){
    VIbias_bus_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_PHOffset)){
    PHOffset_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_Vcomp_ADC)){
    Vcomp_ADC_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_PHScale)){
    PHScale_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_VIColOr)){
    VIColOr_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_Vcal)){
    Vcal_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_CalDel)){
    CalDel_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_TempRange)){
    TempRange_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_WBC)){
    WBC_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_ChipContReg)){
    ChipContReg_ = dacValue;
  }
  else if(ToLower(dacName) == ToLower(k_DACName_Readback)){
    Readback_ = dacValue;
  }
  else
  {
    cout << "ERROR in PixelROCDACSettings::setDac: DAC name " << dacName << " does not exist." << endl;
    assert(0);
  }

}

unsigned int PixelROCDACSettings::getDac(string dacName) const {
  
  if(dacName == k_DACName_Vdd){
    return Vdd_;
  }
  else if(dacName == k_DACName_Vdd){
    return Vdd_;
  }
  else if(dacName == k_DACName_Vana){
    return Vana_;
  }
  else if(dacName == k_DACName_Vsh){
    return Vsh_;
  }
  else if(dacName == k_DACName_Vcomp){
    return Vcomp_;
  }
  else if(dacName == k_DACName_VwllPr){
    return VwllPr_;
  }
  else if(dacName == k_DACName_VwllSh){
    return VwllSh_;
  }
  else if(dacName == k_DACName_VHldDel){
    return VHldDel_;
  }
  else if(dacName == k_DACName_Vtrim){
    return Vtrim_;
  }
  else if(dacName == k_DACName_VcThr){
    return VcThr_;
  }
  else if(dacName == k_DACName_VIbias_bus){
    return VIbias_bus_;
  }
  else if(dacName == k_DACName_PHOffset){
    return PHOffset_;
  }
  else if(dacName == k_DACName_Vcomp_ADC){
    return Vcomp_ADC_;
  }
  else if(dacName == k_DACName_PHScale){
    return PHScale_;
  }
  else if(dacName == k_DACName_VIColOr){
    return VIColOr_;
  }
  else if(dacName == k_DACName_Vcal){
    return Vcal_;
  }
  else if(dacName == k_DACName_CalDel){
    return CalDel_;
  }
  else if(dacName == k_DACName_TempRange){
    return TempRange_;
  }
  else if(dacName == k_DACName_WBC){
    return WBC_;
  }
  else if(dacName == k_DACName_ChipContReg){
    return ChipContReg_;
  }
  else if(dacName == k_DACName_Readback){
    return Readback_;
  }
  else {
    cout << "ERROR in PixelROCDACSettings::getDac: DAC name " << dacName << " does not exist." << endl;
    assert(0);
  }
}			  


string PixelROCDACSettings::ToLower(string generic)
{
  string result ;
  for(unsigned int i = 0; i < generic.length() ; i++)
    {
      result.append(1,(char)tolower(generic[i]) );
    }
  return result ;
}
