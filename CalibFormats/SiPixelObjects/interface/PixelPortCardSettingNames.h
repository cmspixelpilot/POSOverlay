#ifndef PixelPortCardSettingNames_h
#define PixelPortCardSettingNames_h
/**
* \file CalibFormats/SiPixelObjects/interface/PortCardSettingNames.h
* \brief This is just a naespe populated with default values
*
*   A longer explanation will be placed here later
*
*/
 

#include <string>
namespace pos{
  namespace PortCardSettingNames
  {
    // AOH
    const std::string k_AOH_Bias1 = "AOH_Bias1";
    const std::string k_AOH_Bias2 = "AOH_Bias2";
    const std::string k_AOH_Bias3 = "AOH_Bias3";
    const std::string k_AOH_Bias4 = "AOH_Bias4";
    const std::string k_AOH_Bias5 = "AOH_Bias5";
    const std::string k_AOH_Bias6 = "AOH_Bias6";
    const std::string k_AOH_Gain123 = "AOH_Gain123";
    const std::string k_AOH_Gain456 = "AOH_Gain456";
    
    const std::string k_AOH1_Bias1 = "AOH1_Bias1";
    const std::string k_AOH1_Bias2 = "AOH1_Bias2";
    const std::string k_AOH1_Bias3 = "AOH1_Bias3";
    const std::string k_AOH1_Bias4 = "AOH1_Bias4";
    const std::string k_AOH1_Bias5 = "AOH1_Bias5";
    const std::string k_AOH1_Bias6 = "AOH1_Bias6";
    const std::string k_AOH1_Gain123 = "AOH1_Gain123";
    const std::string k_AOH1_Gain456 = "AOH1_Gain456";
    
    const std::string k_AOH2_Bias1 = "AOH2_Bias1";
    const std::string k_AOH2_Bias2 = "AOH2_Bias2";
    const std::string k_AOH2_Bias3 = "AOH2_Bias3";
    const std::string k_AOH2_Bias4 = "AOH2_Bias4";
    const std::string k_AOH2_Bias5 = "AOH2_Bias5";
    const std::string k_AOH2_Bias6 = "AOH2_Bias6";
    const std::string k_AOH2_Gain123 = "AOH2_Gain123";
    const std::string k_AOH2_Gain456 = "AOH2_Gain456";
    
    const std::string k_AOH3_Bias1 = "AOH3_Bias1";
    const std::string k_AOH3_Bias2 = "AOH3_Bias2";
    const std::string k_AOH3_Bias3 = "AOH3_Bias3";
    const std::string k_AOH3_Bias4 = "AOH3_Bias4";
    const std::string k_AOH3_Bias5 = "AOH3_Bias5";
    const std::string k_AOH3_Bias6 = "AOH3_Bias6";
    const std::string k_AOH3_Gain123 = "AOH3_Gain123";
    const std::string k_AOH3_Gain456 = "AOH3_Gain456";
    
    const std::string k_AOH4_Bias1 = "AOH4_Bias1";
    const std::string k_AOH4_Bias2 = "AOH4_Bias2";
    const std::string k_AOH4_Bias3 = "AOH4_Bias3";
    const std::string k_AOH4_Bias4 = "AOH4_Bias4";
    const std::string k_AOH4_Bias5 = "AOH4_Bias5";
    const std::string k_AOH4_Bias6 = "AOH4_Bias6";
    const std::string k_AOH4_Gain123 = "AOH4_Gain123";
    const std::string k_AOH4_Gain456 = "AOH4_Gain456";
    
    const unsigned int k_AOH_Bias1_offset = 0x0;
    const unsigned int k_AOH_Bias2_offset = 0x1;
    const unsigned int k_AOH_Bias3_offset = 0x2;
    const unsigned int k_AOH_Bias4_offset = 0x4;
    const unsigned int k_AOH_Bias5_offset = 0x5;
    const unsigned int k_AOH_Bias6_offset = 0x6;
    const unsigned int k_AOH_Gain123_offset = 0x3;
    const unsigned int k_AOH_Gain456_offset = 0x7;
    
    // AOH fpix
    const unsigned int k_fpix_AOH_base = 0x10;
    const unsigned int k_fpix_AOH_Bias1_address = k_fpix_AOH_base + k_AOH_Bias1_offset;
    const unsigned int k_fpix_AOH_Bias2_address = k_fpix_AOH_base + k_AOH_Bias2_offset;
    const unsigned int k_fpix_AOH_Bias3_address = k_fpix_AOH_base + k_AOH_Bias3_offset;
    const unsigned int k_fpix_AOH_Bias4_address = k_fpix_AOH_base + k_AOH_Bias4_offset;
    const unsigned int k_fpix_AOH_Bias5_address = k_fpix_AOH_base + k_AOH_Bias5_offset;
    const unsigned int k_fpix_AOH_Bias6_address = k_fpix_AOH_base + k_AOH_Bias6_offset;
    const unsigned int k_fpix_AOH_Gain123_address = k_fpix_AOH_base + k_AOH_Gain123_offset;
    const unsigned int k_fpix_AOH_Gain456_address = k_fpix_AOH_base + k_AOH_Gain456_offset;
    
    // AOH bpix
    const unsigned int k_bpix_AOH1_base = 0x08;
    const unsigned int k_bpix_AOH1_Bias1_address = k_bpix_AOH1_base + k_AOH_Bias1_offset;
    const unsigned int k_bpix_AOH1_Bias2_address = k_bpix_AOH1_base + k_AOH_Bias2_offset;
    const unsigned int k_bpix_AOH1_Bias3_address = k_bpix_AOH1_base + k_AOH_Bias3_offset;
    const unsigned int k_bpix_AOH1_Bias4_address = k_bpix_AOH1_base + k_AOH_Bias4_offset;
    const unsigned int k_bpix_AOH1_Bias5_address = k_bpix_AOH1_base + k_AOH_Bias5_offset;
    const unsigned int k_bpix_AOH1_Bias6_address = k_bpix_AOH1_base + k_AOH_Bias6_offset;
    const unsigned int k_bpix_AOH1_Gain123_address = k_bpix_AOH1_base + k_AOH_Gain123_offset;
    const unsigned int k_bpix_AOH1_Gain456_address = k_bpix_AOH1_base + k_AOH_Gain456_offset;
    
    const unsigned int k_bpix_AOH2_base = 0x10;
    const unsigned int k_bpix_AOH2_Bias1_address = k_bpix_AOH2_base + k_AOH_Bias1_offset;
    const unsigned int k_bpix_AOH2_Bias2_address = k_bpix_AOH2_base + k_AOH_Bias2_offset;
    const unsigned int k_bpix_AOH2_Bias3_address = k_bpix_AOH2_base + k_AOH_Bias3_offset;
    const unsigned int k_bpix_AOH2_Bias4_address = k_bpix_AOH2_base + k_AOH_Bias4_offset;
    const unsigned int k_bpix_AOH2_Bias5_address = k_bpix_AOH2_base + k_AOH_Bias5_offset;
    const unsigned int k_bpix_AOH2_Bias6_address = k_bpix_AOH2_base + k_AOH_Bias6_offset;
    const unsigned int k_bpix_AOH2_Gain123_address = k_bpix_AOH2_base + k_AOH_Gain123_offset;
    const unsigned int k_bpix_AOH2_Gain456_address = k_bpix_AOH2_base + k_AOH_Gain456_offset;
    
    const unsigned int k_bpix_AOH3_base = 0x18;
    const unsigned int k_bpix_AOH3_Bias1_address = k_bpix_AOH3_base + k_AOH_Bias1_offset;
    const unsigned int k_bpix_AOH3_Bias2_address = k_bpix_AOH3_base + k_AOH_Bias2_offset;
    const unsigned int k_bpix_AOH3_Bias3_address = k_bpix_AOH3_base + k_AOH_Bias3_offset;
    const unsigned int k_bpix_AOH3_Bias4_address = k_bpix_AOH3_base + k_AOH_Bias4_offset;
    const unsigned int k_bpix_AOH3_Bias5_address = k_bpix_AOH3_base + k_AOH_Bias5_offset;
    const unsigned int k_bpix_AOH3_Bias6_address = k_bpix_AOH3_base + k_AOH_Bias6_offset;
    const unsigned int k_bpix_AOH3_Gain123_address = k_bpix_AOH3_base + k_AOH_Gain123_offset;
    const unsigned int k_bpix_AOH3_Gain456_address = k_bpix_AOH3_base + k_AOH_Gain456_offset;
    
    const unsigned int k_bpix_AOH4_base = 0x20;
    const unsigned int k_bpix_AOH4_Bias1_address = k_bpix_AOH4_base + k_AOH_Bias1_offset;
    const unsigned int k_bpix_AOH4_Bias2_address = k_bpix_AOH4_base + k_AOH_Bias2_offset;
    const unsigned int k_bpix_AOH4_Bias3_address = k_bpix_AOH4_base + k_AOH_Bias3_offset;
    const unsigned int k_bpix_AOH4_Bias4_address = k_bpix_AOH4_base + k_AOH_Bias4_offset;
    const unsigned int k_bpix_AOH4_Bias5_address = k_bpix_AOH4_base + k_AOH_Bias5_offset;
    const unsigned int k_bpix_AOH4_Bias6_address = k_bpix_AOH4_base + k_AOH_Bias6_offset;
    const unsigned int k_bpix_AOH4_Gain123_address = k_bpix_AOH4_base + k_AOH_Gain123_offset;
    const unsigned int k_bpix_AOH4_Gain456_address = k_bpix_AOH4_base + k_AOH_Gain456_offset;

    // POH (Pilot)
    // but don't make separate set of constants for rest of registers yet since they are the same in pilot system as in fpix.
    const std::string k_POH_Bias1 = "POH_Bias1";
    const std::string k_POH_Bias2 = "POH_Bias2";
    const std::string k_POH_Bias3 = "POH_Bias3";
    const std::string k_POH_Bias4 = "POH_Bias4";
    const std::string k_POH_Bias5 = "POH_Bias5";
    const std::string k_POH_Bias6 = "POH_Bias6";
    const std::string k_POH_Bias7 = "POH_Bias7";
    const std::string k_POH_Gain123 = "POH_Gain123";
    const std::string k_POH_Gain4   = "POH_Gain4";
    const std::string k_POH_Gain567 = "POH_Gain567";

    const unsigned int k_POH_Bias1_offset = 0x0;
    const unsigned int k_POH_Bias2_offset = 0x1;
    const unsigned int k_POH_Bias3_offset = 0x2;
    const unsigned int k_POH_Bias4_offset = 0x8;
    const unsigned int k_POH_Bias5_offset = 0x4;
    const unsigned int k_POH_Bias6_offset = 0x5;
    const unsigned int k_POH_Bias7_offset = 0x6;
    const unsigned int k_POH_Gain123_offset = 0x3;
    const unsigned int k_POH_Gain4_offset   = 0xB;
    const unsigned int k_POH_Gain567_offset = 0x7;
    
    const unsigned int k_pilt_POH_base = 0x10;
    const unsigned int k_pilt_POH_Bias1_address = k_pilt_POH_base + k_POH_Bias1_offset;
    const unsigned int k_pilt_POH_Bias2_address = k_pilt_POH_base + k_POH_Bias2_offset;
    const unsigned int k_pilt_POH_Bias3_address = k_pilt_POH_base + k_POH_Bias3_offset;
    const unsigned int k_pilt_POH_Bias4_address = k_pilt_POH_base + k_POH_Bias4_offset;
    const unsigned int k_pilt_POH_Bias5_address = k_pilt_POH_base + k_POH_Bias5_offset;
    const unsigned int k_pilt_POH_Bias6_address = k_pilt_POH_base + k_POH_Bias6_offset;
    const unsigned int k_pilt_POH_Bias7_address = k_pilt_POH_base + k_POH_Bias7_offset;
    const unsigned int k_pilt_POH_Gain123_address = k_pilt_POH_base + k_POH_Gain123_offset;
    const unsigned int k_pilt_POH_Gain4_address   = k_pilt_POH_base + k_POH_Gain4_offset;
    const unsigned int k_pilt_POH_Gain567_address = k_pilt_POH_base + k_POH_Gain567_offset;

    // Phase1
    const std::string k_bPOH_Bias1 = "bPOH_Bias1";
    const std::string k_bPOH_Bias2 = "bPOH_Bias2";
    const std::string k_bPOH_Bias3 = "bPOH_Bias3";
    const std::string k_bPOH_Bias4 = "bPOH_Bias4";
    const std::string k_bPOH_Bias5 = "bPOH_Bias5";
    const std::string k_bPOH_Bias6 = "bPOH_Bias6";
    const std::string k_bPOH_Bias7 = "bPOH_Bias7";
    const std::string k_bPOH_Gain123 = "bPOH_Gain123";
    const std::string k_bPOH_Gain4 = "bPOH_Gain4";
    const std::string k_bPOH_Gain567 = "bPOH_Gain567";
    const std::string k_tPOH_Bias1 = "tPOH_Bias1";
    const std::string k_tPOH_Bias2 = "tPOH_Bias2";
    const std::string k_tPOH_Bias3 = "tPOH_Bias3";
    const std::string k_tPOH_Bias4 = "tPOH_Bias4";
    const std::string k_tPOH_Bias5 = "tPOH_Bias5";
    const std::string k_tPOH_Bias6 = "tPOH_Bias6";
    const std::string k_tPOH_Bias7 = "tPOH_Bias7";
    const std::string k_tPOH_Gain123 = "tPOH_Gain123";
    const std::string k_tPOH_Gain4 = "tPOH_Gain4";
    const std::string k_tPOH_Gain567 = "tPOH_Gain567";
    //bottom POH
    const unsigned int k_p1fpix_bPOH_base = 0x10;
    const unsigned int k_p1fpix_bPOH_Bias1_address = k_p1fpix_bPOH_base + k_POH_Bias1_offset;
    const unsigned int k_p1fpix_bPOH_Bias2_address = k_p1fpix_bPOH_base + k_POH_Bias2_offset;
    const unsigned int k_p1fpix_bPOH_Bias3_address = k_p1fpix_bPOH_base + k_POH_Bias3_offset;
    const unsigned int k_p1fpix_bPOH_Bias4_address = k_p1fpix_bPOH_base + k_POH_Bias4_offset;
    const unsigned int k_p1fpix_bPOH_Bias5_address = k_p1fpix_bPOH_base + k_POH_Bias5_offset;
    const unsigned int k_p1fpix_bPOH_Bias6_address = k_p1fpix_bPOH_base + k_POH_Bias6_offset;
    const unsigned int k_p1fpix_bPOH_Bias7_address = k_p1fpix_bPOH_base + k_POH_Bias7_offset;
    const unsigned int k_p1fpix_bPOH_Gain123_address = k_p1fpix_bPOH_base + k_POH_Gain123_offset;
    const unsigned int k_p1fpix_bPOH_Gain4_address = k_p1fpix_bPOH_base + k_POH_Gain4_offset;
    const unsigned int k_p1fpix_bPOH_Gain567_address = k_p1fpix_bPOH_base + k_POH_Gain567_offset;
    //top POH
    const unsigned int k_p1fpix_tPOH_base = 0x30;
    const unsigned int k_p1fpix_tPOH_Bias1_address = k_p1fpix_tPOH_base + k_POH_Bias1_offset;
    const unsigned int k_p1fpix_tPOH_Bias2_address = k_p1fpix_tPOH_base + k_POH_Bias2_offset;
    const unsigned int k_p1fpix_tPOH_Bias3_address = k_p1fpix_tPOH_base + k_POH_Bias3_offset;
    const unsigned int k_p1fpix_tPOH_Bias4_address = k_p1fpix_tPOH_base + k_POH_Bias4_offset;
    const unsigned int k_p1fpix_tPOH_Bias5_address = k_p1fpix_tPOH_base + k_POH_Bias5_offset;
    const unsigned int k_p1fpix_tPOH_Bias6_address = k_p1fpix_tPOH_base + k_POH_Bias6_offset;
    const unsigned int k_p1fpix_tPOH_Bias7_address = k_p1fpix_tPOH_base + k_POH_Bias7_offset;
    const unsigned int k_p1fpix_tPOH_Gain123_address = k_p1fpix_tPOH_base + k_POH_Gain123_offset;
    const unsigned int k_p1fpix_tPOH_Gain4_address = k_p1fpix_tPOH_base + k_POH_Gain4_offset;
    const unsigned int k_p1fpix_tPOH_Gain567_address = k_p1fpix_tPOH_base + k_POH_Gain567_offset;

     //ph1 bpix
    const std::string k_POH7_Bias1 = "POH7_Bias1";
    const std::string k_POH7_Bias2 = "POH7_Bias2";
    const std::string k_POH7_Bias3 = "POH7_Bias3";
    const std::string k_POH7_Bias4 = "POH7_Bias4";
    const std::string k_POH7_Gain12 = "POH7_Gain12";
    const std::string k_POH7_Gain34   = "POH7_Gain34";

    const std::string k_POH6_Bias1 = "POH6_Bias1";
    const std::string k_POH6_Bias2 = "POH6_Bias2";
    const std::string k_POH6_Bias3 = "POH6_Bias3";
    const std::string k_POH6_Bias4 = "POH6_Bias4";
    const std::string k_POH6_Gain12 = "POH6_Gain12";
    const std::string k_POH6_Gain34   = "POH6_Gain34";

    const std::string k_POH5_Bias1 = "POH5_Bias1";
    const std::string k_POH5_Bias2 = "POH5_Bias2";
    const std::string k_POH5_Bias3 = "POH5_Bias3";
    const std::string k_POH5_Bias4 = "POH5_Bias4";
    const std::string k_POH5_Gain12 = "POH5_Gain12";
    const std::string k_POH5_Gain34   = "POH5_Gain34";

    const std::string k_POH4_Bias1 = "POH4_Bias1";
    const std::string k_POH4_Bias2 = "POH4_Bias2";
    const std::string k_POH4_Bias3 = "POH4_Bias3";
    const std::string k_POH4_Bias4 = "POH4_Bias4";
    const std::string k_POH4_Gain12 = "POH4_Gain12";
    const std::string k_POH4_Gain34   = "POH4_Gain34";

    const std::string k_POH3_Bias1 = "POH3_Bias1";
    const std::string k_POH3_Bias2 = "POH3_Bias2";
    const std::string k_POH3_Bias3 = "POH3_Bias3";
    const std::string k_POH3_Bias4 = "POH3_Bias4";
    const std::string k_POH3_Gain12 = "POH3_Gain12";
    const std::string k_POH3_Gain34   = "POH3_Gain34";

    const std::string k_POH2_Bias1 = "POH2_Bias1";
    const std::string k_POH2_Bias2 = "POH2_Bias2";
    const std::string k_POH2_Bias3 = "POH2_Bias3";
    const std::string k_POH2_Bias4 = "POH2_Bias4";
    const std::string k_POH2_Gain12 = "POH2_Gain12";
    const std::string k_POH2_Gain34   = "POH2_Gain34";

    const std::string k_POH1_Bias1 = "POH1_Bias1";
    const std::string k_POH1_Bias2 = "POH1_Bias2";
    const std::string k_POH1_Bias3 = "POH1_Bias3";
    const std::string k_POH1_Bias4 = "POH1_Bias4";
    const std::string k_POH1_Gain12 = "POH1_Gain12";
    const std::string k_POH1_Gain34   = "POH1_Gain34";
 
    const unsigned int k_POH_ph1bpix_Bias1_offset = 0x0;
    const unsigned int k_POH_ph1bpix_Bias2_offset = 0x2;
    const unsigned int k_POH_ph1bpix_Bias3_offset = 0x4;
    const unsigned int k_POH_ph1bpix_Bias4_offset = 0x6;
    const unsigned int k_POH_ph1bpix_Gain12_offset = 0x3;
    const unsigned int k_POH_ph1bpix_Gain34_offset   = 0x7;

     // POH ph1bpix

    const unsigned int k_ph1bpix_POH1_base = 0x0;
    const unsigned int k_ph1bpix_POH1_Bias1_address = k_ph1bpix_POH1_base + k_POH_ph1bpix_Bias1_offset;
    const unsigned int k_ph1bpix_POH1_Bias2_address = k_ph1bpix_POH1_base + k_POH_ph1bpix_Bias2_offset;
    const unsigned int k_ph1bpix_POH1_Bias3_address = k_ph1bpix_POH1_base + k_POH_ph1bpix_Bias3_offset;
    const unsigned int k_ph1bpix_POH1_Bias4_address = k_ph1bpix_POH1_base + k_POH_ph1bpix_Bias4_offset;
    const unsigned int k_ph1bpix_POH1_Gain12_address = k_ph1bpix_POH1_base + k_POH_ph1bpix_Gain12_offset;
    const unsigned int k_ph1bpix_POH1_Gain34_address = k_ph1bpix_POH1_base + k_POH_ph1bpix_Gain34_offset;

    const unsigned int k_ph1bpix_POH2_base = 0x8;
    const unsigned int k_ph1bpix_POH2_Bias1_address = k_ph1bpix_POH2_base + k_POH_ph1bpix_Bias1_offset;
    const unsigned int k_ph1bpix_POH2_Bias2_address = k_ph1bpix_POH2_base + k_POH_ph1bpix_Bias2_offset;
    const unsigned int k_ph1bpix_POH2_Bias3_address = k_ph1bpix_POH2_base + k_POH_ph1bpix_Bias3_offset;
    const unsigned int k_ph1bpix_POH2_Bias4_address = k_ph1bpix_POH2_base + k_POH_ph1bpix_Bias4_offset;
    const unsigned int k_ph1bpix_POH2_Gain12_address = k_ph1bpix_POH2_base + k_POH_ph1bpix_Gain12_offset;
    const unsigned int k_ph1bpix_POH2_Gain34_address = k_ph1bpix_POH2_base + k_POH_ph1bpix_Gain34_offset;

    const unsigned int k_ph1bpix_POH3_base = 0x10;
    const unsigned int k_ph1bpix_POH3_Bias1_address = k_ph1bpix_POH3_base + k_POH_ph1bpix_Bias1_offset;
    const unsigned int k_ph1bpix_POH3_Bias2_address = k_ph1bpix_POH3_base + k_POH_ph1bpix_Bias2_offset;
    const unsigned int k_ph1bpix_POH3_Bias3_address = k_ph1bpix_POH3_base + k_POH_ph1bpix_Bias3_offset;
    const unsigned int k_ph1bpix_POH3_Bias4_address = k_ph1bpix_POH3_base + k_POH_ph1bpix_Bias4_offset;
    const unsigned int k_ph1bpix_POH3_Gain12_address = k_ph1bpix_POH3_base + k_POH_ph1bpix_Gain12_offset;
    const unsigned int k_ph1bpix_POH3_Gain34_address = k_ph1bpix_POH3_base + k_POH_ph1bpix_Gain34_offset;

    const unsigned int k_ph1bpix_POH4_base = 0x18;
    const unsigned int k_ph1bpix_POH4_Bias1_address = k_ph1bpix_POH4_base + k_POH_ph1bpix_Bias1_offset;
    const unsigned int k_ph1bpix_POH4_Bias2_address = k_ph1bpix_POH4_base + k_POH_ph1bpix_Bias2_offset;
    const unsigned int k_ph1bpix_POH4_Bias3_address = k_ph1bpix_POH4_base + k_POH_ph1bpix_Bias3_offset;
    const unsigned int k_ph1bpix_POH4_Bias4_address = k_ph1bpix_POH4_base + k_POH_ph1bpix_Bias4_offset;
    const unsigned int k_ph1bpix_POH4_Gain12_address = k_ph1bpix_POH4_base + k_POH_ph1bpix_Gain12_offset;
    const unsigned int k_ph1bpix_POH4_Gain34_address = k_ph1bpix_POH4_base + k_POH_ph1bpix_Gain34_offset;

    const unsigned int k_ph1bpix_POH5_base = 0x20;
    const unsigned int k_ph1bpix_POH5_Bias1_address = k_ph1bpix_POH5_base + k_POH_ph1bpix_Bias1_offset;
    const unsigned int k_ph1bpix_POH5_Bias2_address = k_ph1bpix_POH5_base + k_POH_ph1bpix_Bias2_offset;
    const unsigned int k_ph1bpix_POH5_Bias3_address = k_ph1bpix_POH5_base + k_POH_ph1bpix_Bias3_offset;
    const unsigned int k_ph1bpix_POH5_Bias4_address = k_ph1bpix_POH5_base + k_POH_ph1bpix_Bias4_offset;
    const unsigned int k_ph1bpix_POH5_Gain12_address = k_ph1bpix_POH5_base + k_POH_ph1bpix_Gain12_offset;
    const unsigned int k_ph1bpix_POH5_Gain34_address = k_ph1bpix_POH5_base + k_POH_ph1bpix_Gain34_offset;

    const unsigned int k_ph1bpix_POH6_base = 0x28;
    const unsigned int k_ph1bpix_POH6_Bias1_address = k_ph1bpix_POH6_base + k_POH_ph1bpix_Bias1_offset;
    const unsigned int k_ph1bpix_POH6_Bias2_address = k_ph1bpix_POH6_base + k_POH_ph1bpix_Bias2_offset;
    const unsigned int k_ph1bpix_POH6_Bias3_address = k_ph1bpix_POH6_base + k_POH_ph1bpix_Bias3_offset;
    const unsigned int k_ph1bpix_POH6_Bias4_address = k_ph1bpix_POH6_base + k_POH_ph1bpix_Bias4_offset;
    const unsigned int k_ph1bpix_POH6_Gain12_address = k_ph1bpix_POH6_base + k_POH_ph1bpix_Gain12_offset;
    const unsigned int k_ph1bpix_POH6_Gain34_address = k_ph1bpix_POH6_base + k_POH_ph1bpix_Gain34_offset;

    const unsigned int k_ph1bpix_POH7_base = 0x30;
    const unsigned int k_ph1bpix_POH7_Bias1_address = k_ph1bpix_POH7_base + k_POH_ph1bpix_Bias1_offset;
    const unsigned int k_ph1bpix_POH7_Bias2_address = k_ph1bpix_POH7_base + k_POH_ph1bpix_Bias2_offset;
    const unsigned int k_ph1bpix_POH7_Bias3_address = k_ph1bpix_POH7_base + k_POH_ph1bpix_Bias3_offset;
    const unsigned int k_ph1bpix_POH7_Bias4_address = k_ph1bpix_POH7_base + k_POH_ph1bpix_Bias4_offset;
    const unsigned int k_ph1bpix_POH7_Gain12_address = k_ph1bpix_POH7_base + k_POH_ph1bpix_Gain12_offset;
    const unsigned int k_ph1bpix_POH7_Gain34_address = k_ph1bpix_POH7_base + k_POH_ph1bpix_Gain34_offset;
    // PLL
    const std::string k_PLL_CTR1 = "PLL_CTR1";
    const std::string k_PLL_CTR2 = "PLL_CTR2";
    const std::string k_PLL_CTR3 = "PLL_CTR3";
    const std::string k_PLL_CTR4or5 = "PLL_CTR4or5"; // controls either CTR4 (if bit 5 of CTR2 = 0) or CTR5 (if bit 5 of CTR = 1)
    const std::string k_PLL_CTR4 = "PLL_CTR4"; // has no address defined, needs special handling
    const std::string k_PLL_CTR5 = "PLL_CTR5"; // has no address defined, needs special handling
    
    const unsigned int k_PLL_CTR1_offset = 0x0;
    const unsigned int k_PLL_CTR2_offset = 0x1;
    const unsigned int k_PLL_CTR3_offset = 0x2;
    const unsigned int k_PLL_CTR4or5_offset = 0x3;
    
    // PLL fpix
    const unsigned int k_fpix_PLL_base = 0x20;
    const unsigned int k_fpix_PLL_CTR1_address = k_fpix_PLL_base + k_PLL_CTR1_offset;
    const unsigned int k_fpix_PLL_CTR2_address = k_fpix_PLL_base + k_PLL_CTR2_offset;
    const unsigned int k_fpix_PLL_CTR3_address = k_fpix_PLL_base + k_PLL_CTR3_offset;
    const unsigned int k_fpix_PLL_CTR4or5_address = k_fpix_PLL_base + k_PLL_CTR4or5_offset;
    
    // PLL bpix
    const unsigned int k_bpix_PLL_base = 0x40;
    const unsigned int k_bpix_PLL_CTR1_address = k_bpix_PLL_base + k_PLL_CTR1_offset;
    const unsigned int k_bpix_PLL_CTR2_address = k_bpix_PLL_base + k_PLL_CTR2_offset;
    const unsigned int k_bpix_PLL_CTR3_address = k_bpix_PLL_base + k_PLL_CTR3_offset;
    const unsigned int k_bpix_PLL_CTR4or5_address = k_bpix_PLL_base + k_PLL_CTR4or5_offset;
    
    // Delay25
    const std::string k_Delay25_RDA = "Delay25_RDA";
    const std::string k_Delay25_RCL = "Delay25_RCL";
    const std::string k_Delay25_SDA = "Delay25_SDA";
    const std::string k_Delay25_TRG = "Delay25_TRG";
    const std::string k_Delay25_SCL = "Delay25_SCL";
    const std::string k_Delay25_GCR = "Delay25_GCR";
    
    const unsigned int k_Delay25_CR0_offset = 0x0;
    const unsigned int k_Delay25_CR1_offset = 0x1;
    const unsigned int k_Delay25_CR2_offset = 0x2;
    const unsigned int k_Delay25_CR3_offset = 0x3;
    const unsigned int k_Delay25_CR4_offset = 0x4;
    const unsigned int k_Delay25_CR5_offset = 0x5;

    // Delay25 p1fpix
    const unsigned int k_p1fpix_Delay25_base = 0x40;
    const unsigned int k_p1fpix_Delay25_RDA_address = k_p1fpix_Delay25_base + k_Delay25_CR0_offset;
    const unsigned int k_p1fpix_Delay25_RCL_address = k_p1fpix_Delay25_base + k_Delay25_CR1_offset;
    const unsigned int k_p1fpix_Delay25_SDA_address = k_p1fpix_Delay25_base + k_Delay25_CR2_offset;
    const unsigned int k_p1fpix_Delay25_TRG_address = k_p1fpix_Delay25_base + k_Delay25_CR3_offset;
    const unsigned int k_p1fpix_Delay25_SCL_address = k_p1fpix_Delay25_base + k_Delay25_CR4_offset;
    const unsigned int k_p1fpix_Delay25_GCR_address = k_p1fpix_Delay25_base + k_Delay25_CR5_offset;

    // Delay25 fpix
    const unsigned int k_fpix_Delay25_base = 0x30;
    const unsigned int k_fpix_Delay25_RDA_address = k_fpix_Delay25_base + k_Delay25_CR0_offset;
    const unsigned int k_fpix_Delay25_RCL_address = k_fpix_Delay25_base + k_Delay25_CR1_offset;
    const unsigned int k_fpix_Delay25_SDA_address = k_fpix_Delay25_base + k_Delay25_CR2_offset;
    const unsigned int k_fpix_Delay25_TRG_address = k_fpix_Delay25_base + k_Delay25_CR3_offset;
    const unsigned int k_fpix_Delay25_SCL_address = k_fpix_Delay25_base + k_Delay25_CR4_offset;
    const unsigned int k_fpix_Delay25_GCR_address = k_fpix_Delay25_base + k_Delay25_CR5_offset;
    
    // Delay25 bpix
    const unsigned int k_bpix_Delay25_base = 0x60;
    const unsigned int k_bpix_Delay25_RDA_address = k_bpix_Delay25_base + k_Delay25_CR3_offset;
    const unsigned int k_bpix_Delay25_RCL_address = k_bpix_Delay25_base + k_Delay25_CR0_offset;
    const unsigned int k_bpix_Delay25_SDA_address = k_bpix_Delay25_base + k_Delay25_CR2_offset;
    const unsigned int k_bpix_Delay25_TRG_address = k_bpix_Delay25_base + k_Delay25_CR1_offset;
    const unsigned int k_bpix_Delay25_SCL_address = k_bpix_Delay25_base + k_Delay25_CR4_offset;
    const unsigned int k_bpix_Delay25_GCR_address = k_bpix_Delay25_base + k_Delay25_CR5_offset;
    // Delay25 ph1bpix
    const unsigned int k_ph1bpix_Delay25_base = 0x60;
    const unsigned int k_ph1bpix_Delay25_RDA_address = k_ph1bpix_Delay25_base + k_Delay25_CR1_offset;
    const unsigned int k_ph1bpix_Delay25_RCL_address = k_ph1bpix_Delay25_base + k_Delay25_CR0_offset;
    const unsigned int k_ph1bpix_Delay25_SDA_address = k_ph1bpix_Delay25_base + k_Delay25_CR2_offset;
    const unsigned int k_ph1bpix_Delay25_TRG_address = k_ph1bpix_Delay25_base + k_Delay25_CR3_offset;
    const unsigned int k_ph1bpix_Delay25_SCL_address = k_ph1bpix_Delay25_base + k_Delay25_CR4_offset;
    const unsigned int k_ph1bpix_Delay25_GCR_address = k_ph1bpix_Delay25_base + k_Delay25_CR5_offset;

    // DOH
    const std::string k_DOH_Ch0Bias_CLK  = "DOH_Ch0Bias_CLK";
    const std::string k_DOH_Dummy        = "DOH_Dummy";
    const std::string k_DOH_Ch1Bias_Data = "DOH_Ch1Bias_Data";
    const std::string k_DOH_Gain_SEU     = "DOH_Gain_SEU";
    
    const unsigned int k_DOH_Ch0Bias_CLK_offset  = 0x0;
    const unsigned int k_DOH_Dummy_offset        = 0x1;
    const unsigned int k_DOH_Ch1Bias_Data_offset = 0x2;
    const unsigned int k_DOH_Gain_SEU_offset     = 0x3;
    
    // DOH fpix
    const unsigned int k_fpix_DOH_base = 0x70;
    const unsigned int k_fpix_DOH_Ch0Bias_CLK_address  = k_fpix_DOH_base + k_DOH_Ch0Bias_CLK_offset;
    const unsigned int k_fpix_DOH_Dummy_address        = k_fpix_DOH_base + k_DOH_Dummy_offset;
    const unsigned int k_fpix_DOH_Ch1Bias_Data_address = k_fpix_DOH_base + k_DOH_Ch1Bias_Data_offset;
    const unsigned int k_fpix_DOH_Gain_SEU_address     = k_fpix_DOH_base + k_DOH_Gain_SEU_offset;
    
    // DOH bpix
    const unsigned int k_bpix_DOH_base = 0x70;
    const unsigned int k_bpix_DOH_Ch0Bias_CLK_address  = k_bpix_DOH_base + k_DOH_Ch0Bias_CLK_offset;
    const unsigned int k_bpix_DOH_Dummy_address        = k_bpix_DOH_base + k_DOH_Dummy_offset;
    const unsigned int k_bpix_DOH_Ch1Bias_Data_address = k_bpix_DOH_base + k_DOH_Ch1Bias_Data_offset;
    const unsigned int k_bpix_DOH_Gain_SEU_address     = k_bpix_DOH_base + k_DOH_Gain_SEU_offset;
  }
}
#endif
