#ifndef PixelFECParameters_h
#define PixelFECParameters_h
/**
*   \file CalibFormats/SiPixelObjects/interface/PixelFECParameters.h
*   \brief This class implements..
*
*   A longer explanation will be placed here later
*/

#include <iosfwd>
#include <string>

namespace pos{
/*! \class PixelFECParameters PixelFECParameters.h "interface/PixelFECParameters.h"
*   \brief This class implements..
*
*   A longer explanation will be placed here later
*/

  class PixelFECParameters;
  std::ostream&  operator <<(std::ostream& s ,const PixelFECParameters &pFECp);
  
  class PixelFECParameters {
  public:

    PixelFECParameters();

    unsigned int getFECNumber() const { return fecnumber_; }
    unsigned int getCrate() const { return crate_; }
    unsigned int getVMEBaseAddress() const { return vmebaseaddress_; }
    std::string getType() const { return type_; }
    std::string getURI() const { return uri_; }
    void setFECParameters( unsigned int fecnumber , unsigned int crate , unsigned int vmebaseaddress);
    void setFECNumber(unsigned int fecnumber) { fecnumber_ = fecnumber; }
    void setCrate(unsigned int crate) { crate_ = crate; }
    void setVMEBaseAddress(unsigned int vmebaseaddress) { vmebaseaddress_ = vmebaseaddress; }
    void setType(std::string type) { type_ = type; }
    void setURI(std::string uri) { uri_ = uri; }
    friend std::ostream& pos::operator <<(std::ostream& s,const PixelFECParameters &pFECp);
  private :

    unsigned int fecnumber_;   
    unsigned int crate_;   
    unsigned int vmebaseaddress_;   
    std::string type_;
    std::string uri_;

  };
}
#endif
