//
// This class specifies the settings on the TKPCIFEC
// and the settings on the portcard
//
//
//
//

#include "CalibFormats/SiPixelObjects/interface/PixelDCDCConfig.h"
#include "CalibFormats/SiPixelObjects/interface/PixelTimeFormatter.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <assert.h>
#include <stdexcept>
#include <set>

using namespace std;
using namespace pos;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PixelDCDCConfig::PixelDCDCConfig(vector < vector< string> >  &tableMat):PixelConfigBase(" "," "," ")
{
  string mthn = "[PixelDCDCConfig::PixelDCDCConfig()]\t\t    " ;
  std::cout << __LINE__ << "]\t" << mthn << "Method is not yet configured for database access." << std::endl;
  assert(0);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PixelDCDCConfig::PixelDCDCConfig(std::string filename):
  PixelConfigBase(" "," "," "){

  string mthn = "[PixelDCDCConfig::PixelDCDCConfig()]\t\t    " ;
  //std::cout << __LINE__ << "]\t" << mthn << "filename:"<<filename<<std::endl;

  size_t dcdcpos=filename.find(std::string("dcdc_"));
  //std::cout << __LINE__ << "]\t" << mthn << "portcardpos:"<<portcardpos<<std::endl;
  assert(dcdcpos!=(unsigned int)std::string::npos);
  size_t datpos=filename.find(std::string(".dat"));
  //std::cout << __LINE__ << "]\t" << mthn << "datpos:"<<datpos<<std::endl;
  assert(datpos!=(unsigned int)std::string::npos);
  assert(datpos>dcdcpos);
  
  dcdcname_=filename.substr(dcdcpos+9,datpos-dcdcpos-9);

  //std::cout << "Portcard name extracted from file name:"<<portcardname_<<std::endl;

  std::ifstream in(filename.c_str());
  
  if(!in.good()){
    std::cout << __LINE__ << "]\t" << mthn << "Could not open: " << filename << std::endl;
    throw std::runtime_error("Failed to open file "+filename);
  }
  else {
    std::cout << __LINE__ << "]\t" << mthn << "Opened: "         << filename << std::endl;
  }

  do {
      
    std::string settingName;
    std::string value;

    in >> settingName >> value;
    if (in.eof()) break;
    
    if ( settingName[settingName.size()-1] == ':' ) settingName.resize( settingName.size()-1 ); // remove ':' from end of string, if it's there
   
    // parse the DCDC config
    std::stringstream instr;
    int address=0;
    if ( settingName == "Enabled" ){
	    setDCDCEnabled( (value=="yes") );
    }
    if ( settingName == "CCUAddressEnable" ){
	    instr << value;
	    instr >> std::hex >> address;
	    setCCUAddressEnable( address );
    }
    if ( settingName == "CCUAddressPgood" ){
	    instr << value;
	    instr >> std::hex >> address;
	    setCCUAddressPgood( address );
    }
    if ( settingName == "PIAChannelAddress" ){
	    instr << value;
	    instr >> std::hex >> address;
	    setPIAChannelAddress( address );
    }
    if ( settingName == "PortNumber" ){
	    instr << value;
	    instr >> std::dec >> address;
	    setPortNumber( address );
    }

  }
  while (!in.eof());
  
  in.close();

}

//=============================================================================================
void PixelDCDCConfig::writeXMLHeader(pos::PixelConfigKey key, 
                                      	 int version, 
                                      	 std::string path, 
                                      	 std::ofstream *outstream,
                                      	 std::ofstream *out1stream,
                                      	 std::ofstream *out2stream) const
{
  std::string mthn = "[PixelDCDCConfig::writeXMLHeader()]\t\t\t    " ;
  std::stringstream fullPath ;
  fullPath << path << "/Pixel_PortCardSettings_" << PixelTimeFormatter::getmSecTime() << ".xml" ;
  std::cout << __LINE__ << "]\t" << mthn << "Writing to: " << fullPath.str() << std::endl ;
  
  outstream->open(fullPath.str().c_str()) ;
  
  *outstream << "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>"			 	     << std::endl ;
  *outstream << "<ROOT xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'>" 		 	             << std::endl ;
  *outstream << " <HEADER>"								         	     << std::endl ;
  *outstream << "  <TYPE>"								         	     << std::endl ;
  *outstream << "   <EXTENSION_TABLE_NAME>PIXEL_PORTCARD_SETTINGS</EXTENSION_TABLE_NAME>"          	     << std::endl ;
  *outstream << "   <NAME>Pixel Port Card Settings</NAME>"				         	     << std::endl ;
  *outstream << "  </TYPE>"								         	     << std::endl ;
  *outstream << "  <RUN>"								         	     << std::endl ;
  *outstream << "   <RUN_TYPE>Pixel Port Card Settings</RUN_TYPE>" 		                             << std::endl ;
  *outstream << "   <RUN_NUMBER>1</RUN_NUMBER>"					         	             << std::endl ;
  *outstream << "   <RUN_BEGIN_TIMESTAMP>" << pos::PixelTimeFormatter::getTime() << "</RUN_BEGIN_TIMESTAMP>" << std::endl ;
  *outstream << "   <LOCATION>CERN P5</LOCATION>"                                                            << std::endl ; 
  *outstream << "  </RUN>"								         	     << std::endl ;
  *outstream << " </HEADER>"								         	     << std::endl ;
  *outstream << ""										 	     << std::endl ;
  *outstream << " <DATA_SET>"								         	     << std::endl ;
  *outstream << "  <PART>"                                                                                   << std::endl ;
  *outstream << "   <NAME_LABEL>CMS-PIXEL-ROOT</NAME_LABEL>"                                                 << std::endl ;
  *outstream << "   <KIND_OF_PART>Detector ROOT</KIND_OF_PART>"                                              << std::endl ;
  *outstream << "  </PART>"                                                                                  << std::endl ;
  *outstream << "  <VERSION>"             << version      << "</VERSION>"				     << std::endl ;
  *outstream << "  <COMMENT_DESCRIPTION>" << getComment() << "</COMMENT_DESCRIPTION>"			     << std::endl ;
  *outstream << "  <CREATED_BY_USER>"     << getAuthor()  << "</CREATED_BY_USER>"  			     << std::endl ;
  *outstream << ""										 	     << std::endl ;
}

//=============================================================================================
void PixelDCDCConfig::writeXML(std::ofstream *outstream,
                                   std::ofstream *out1stream,
                                   std::ofstream *out2stream) const 
{
  std::string mthn = "[PixelDCDCConfig::writeXML()]\t\t\t    " ;


  *outstream << "  <DATA>"                                                                		     << std::endl;
  *outstream << "   <DCDC>"      << dcdcname_    << "</DCDC>"				     << std::endl;
  *outstream << "   <ENABLED>"         << dcdcenabled_         << "</ENABLED>"				     << std::endl;
  *outstream << "   <CCU_ADDR_ENABLE>"           << ccuaddressenable_     << "</CCU_ADDR_ENABLE>"		         		     << std::endl;
  *outstream << "   <CCU_ADDR_PGOOD>"       << ccuaddresspgood_      << "</CCU_ADDR_PGOOD>"				     << std::endl;
  *outstream << "   <PIA_CHANNEL_ADDR>"        << piachanneladdress_  << "</PIA_CHANNEL_ADDR>"        			     << std::endl;
  *outstream << "   <PORT_NUMBER>"      << portnumber_        << "</PORT_NUMBER>" 			             << std::endl;

  *outstream << "  </DATA>" << std::endl;

}
//=============================================================================================
void PixelDCDCConfig::writeXMLTrailer(std::ofstream *outstream,
                                          std::ofstream *out1stream,
                                          std::ofstream *out2stream) const
{
  std::string mthn = "[PixelDCDCConfig::writeXMLTrailer()]\t\t\t    " ;
  
  *outstream << " </DATA_SET>" 						    	 	              	     << std::endl ;
  *outstream << "</ROOT> "								              	     << std::endl ;

  outstream->close() ;
}

void PixelDCDCConfig::writeASCII(std::string dir) const {

  std::string mthn = "[PixelDCDCConfig::writeASCII()]\t\t\t\t    " ;
  if (dir!="") dir+="/";
  std::string filename=dir+"dcdc_"+dcdcname_+".dat";

  std::ofstream out(filename.c_str());
  if (!out.good()){
    std::cout << __LINE__ << "]\t" << mthn << "Could not open file: " << filename.c_str() << std::endl;
    assert(0);
  }

  out << "Name: " << dcdcname_ << std::endl;
  out << "Enabled: " << dcdcenabled_ << std::endl;
  out << "CCUAddressEnable: " << ccuaddressenable_ << std::endl;
  out << "CCUAddressPgood: 0x" <<std::hex<< ccuaddresspgood_ <<std::dec<< std::endl;
  out << "PIAChannelAddress: 0x" <<std::hex<< piachanneladdress_ <<std::dec<< std::endl;
  
  out << "PortNumber: 0x" <<std::hex<< portnumber_ <<std::dec<< std::endl;

  out.close();
}
