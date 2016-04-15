#include "CalibFormats/SiPixelObjects/interface/PixelFECParameters.h"
#include <ostream>


using namespace pos;

PixelFECParameters::PixelFECParameters()
{
fecnumber_=0;
crate_=0;
vmebaseaddress_=0;
 type_="VME";
 uri_="";

}

void PixelFECParameters::setFECParameters( unsigned int fecnumber , unsigned int crate , unsigned int vmebaseaddress){

fecnumber_ = fecnumber;
crate_ = crate;
vmebaseaddress_ =vmebaseaddress;

}

std::ostream&  pos::operator <<(std::ostream& s ,const PixelFECParameters &pFECp){

s <<"FEC Number:"<<pFECp.fecnumber_<<std::endl;
s <<"Crate Number:"<<pFECp.crate_<<std::endl;
s <<"VME Base Address:"<<pFECp.vmebaseaddress_<<std::endl;

return s;

}



