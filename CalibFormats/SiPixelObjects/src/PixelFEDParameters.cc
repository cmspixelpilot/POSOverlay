#include "CalibFormats/SiPixelObjects/interface/PixelFEDParameters.h"
#include <ostream>

using namespace pos;

PixelFEDParameters::PixelFEDParameters()
{
fednumber_=0;
crate_=0;
vmebaseaddress_=0;
 type_="VME";
 uri_="";

}

void PixelFEDParameters::setFEDParameters( unsigned int fednumber , unsigned int crate , unsigned int vmebaseaddress){

fednumber_ = fednumber;
crate_ = crate;
vmebaseaddress_ =vmebaseaddress;

}

std::ostream&  pos::operator <<(std::ostream& s ,const PixelFEDParameters &pFEDp){

s <<"FED Number:"<<pFEDp.fednumber_<<std::endl;
s <<"Crate Number:"<<pFEDp.crate_<<std::endl;
s <<"VME Base Address:"<<pFEDp.vmebaseaddress_<<std::endl;

return s;

}



