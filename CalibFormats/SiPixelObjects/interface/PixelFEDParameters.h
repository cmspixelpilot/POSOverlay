#ifndef PixelFEDParameters_h
#define PixelFEDParameters_h
/**
*   \file CalibFormats/SiPixelObjects/interface/PixelFEDParameters.h
*   \brief This class implements..
*
*   This class specifies which FED boards
*   are used and how they are addressed
*/

#include <iosfwd>
#include <string>

namespace pos{
/*! \class PixelFEDParameters PixelFEDParameters.h "interface/PixelFEDParameters.h"
*   \brief This class implements..
*
*   A longer explanation will be placed here later
*/
  class PixelFEDParameters;
  std::ostream& operator <<(std::ostream& s ,const PixelFEDParameters &pFEDp);

  class PixelFEDParameters {

  public:

    PixelFEDParameters();

    unsigned int getFEDNumber() const { return fednumber_; }
    unsigned int getCrate() const { return crate_; }
    unsigned int getVMEBaseAddress() const { return vmebaseaddress_; }
    std::string getType() const { return type_; }
    std::string getURI() const { return uri_; }
    void setFEDParameters( unsigned int fednumber , unsigned int crate , unsigned int vmebaseaddress);
    void setFEDNumber(unsigned int fednumber) { fednumber_ = fednumber; }
    void setCrate(unsigned int crate) { crate_ = crate; }
    void setVMEBaseAddress(unsigned int vmebaseaddress) { vmebaseaddress_ = vmebaseaddress; }
    void setType(std::string type) { type_ = type; }
    void setURI(std::string uri) { uri_ = uri; }
    friend std::ostream& pos::operator <<(std::ostream& s,const PixelFEDParameters &pFEDp);
  private :

    unsigned int fednumber_;   
    unsigned int crate_;   
    unsigned int vmebaseaddress_;
    std::string type_;
    std::string uri_;
  };
}
#endif
