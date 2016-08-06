#ifndef PixelDCDCConfig_h
#define PixelDCDCConfig_h
/**
* \file CalibFormats/SiPixelObjects/interface/PixelDCDCConfig.h
* \brief This class specifies the settings on the TKPCIFEC and the settings on the portcard
*
*   A longer explanation will be placed here later
*/
#include <vector>
#include <string>
#include <map>
#include "CalibFormats/SiPixelObjects/interface/PixelConfigBase.h"

namespace pos{

/*!  \ingroup ConfigurationObjects "Configuration Objects"
*    
*  @{
*
*  \class PixelDCDCConfig PixelDCDCConfig.h
*  \brief This is the documentation about PixelNameTranslation...
*
*   
*  This class specifies the settings on the TKPCIFEC and the settings on the portcard 
*   
*/
  class PixelDCDCConfig: public PixelConfigBase{

  public:
  
    PixelDCDCConfig(std::vector < std::vector< std::string> >  &tableMat);
    PixelDCDCConfig(std::string);

    void         writeASCII(std::string dir="") const;
    void         writeXML(        pos::PixelConfigKey key, int version, std::string path) const {;}
    virtual void writeXMLHeader(  pos::PixelConfigKey key,
		    int version,
		    std::string path,
		    std::ofstream *out,
		    std::ofstream *out1 = NULL,
		    std::ofstream *out2 = NULL
		    ) const ;
    virtual void writeXML(        std::ofstream *out,
		    std::ofstream *out1 = NULL ,
		    std::ofstream *out2 = NULL ) const ;
    virtual void writeXMLTrailer( std::ofstream *out,
		    std::ofstream *out1 = NULL,
		    std::ofstream *out2 = NULL
		    ) const ;

    const std::string& getDCDCName() const { return dcdcname_; }
    void setDCDCName(std::string newName) { dcdcname_ = newName; }

    std::string getType() { return type_; }

    bool getDCDCEnabled() { return dcdcenabled_; }
    void setDCDCEnabled(bool isEnabled) { dcdcenabled_ = isEnabled; }

    unsigned int getCCUAddressEnable() { return ccuaddressenable_; }
    void setCCUAddressEnable(unsigned int address) { ccuaddressenable_ = address; }

    unsigned int getCCUAddressPgood() { return ccuaddresspgood_; }
    void setCCUAddressPgood(unsigned int address) { ccuaddresspgood_ = address; }

    unsigned int getPIAChannelAddress() { return piachanneladdress_; }
    void setPIAChannelAddress(unsigned int address) { piachanneladdress_ = address; }

    const std::vector<unsigned>& getPortNumbers() { return portnumbers_; }
    
  private:
	
    std::string dcdcname_;

    std::string type_;

    bool dcdcenabled_;
    unsigned int ccuaddressenable_;
    unsigned int ccuaddresspgood_;
    unsigned int piachanneladdress_;
    std::vector<unsigned> portnumbers_;

    void fillNameToAddress();
    void fillDBToFileAddress();

///key used for sorting device_
    std::vector < unsigned int > key_;
    unsigned int aohcount_;
    void sortDeviceList();

    std::map<std::string, unsigned int> nameToAddress_; // translation from name to address, filled in by fillNameToAddress();
    std::map<std::string, std::string> nameDBtoFileConversion_; // filled by fillDBToFileAddress() ;

  };
}
/* @} */
#endif
