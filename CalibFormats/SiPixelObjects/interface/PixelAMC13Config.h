#ifndef PixelAMC13Config_h
#define PixelAMC13Config_h
 
#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <iostream>
#include <sstream>
#include "CalibFormats/SiPixelObjects/interface/PixelConfigBase.h"
#include "CalibFormats/SiPixelObjects/interface/PixelAMC13Parameters.h"

namespace pos {
  class PixelAMC13Config : public PixelConfigBase {
 
  public:
    typedef std::map<int, PixelAMC13Parameters> param_map;

    static std::pair<bool, PixelAMC13Parameters> parse_line(const std::string&);

    PixelAMC13Config(std::string filename);
    PixelAMC13Config(std::vector<std::vector<std::string> > &) ;

    const param_map& getParams() const { return params; }

    std::string toASCII() const;

    virtual void writeASCII(std::string dir) const;
    virtual void writeXML(        pos::PixelConfigKey key, int version, std::string path) const {;}
    virtual void writeXMLHeader(  pos::PixelConfigKey key, 
				  int version, 
				  std::string path, 
				  std::ofstream *out,
				  std::ofstream *out1 = NULL,
				  std::ofstream *out2 = NULL
				  ) const ;
    virtual void writeXML( 	  std::ofstream *out,			        			    
			   	  std::ofstream *out1 = NULL ,
			   	  std::ofstream *out2 = NULL ) const ;
    virtual void writeXMLTrailer( std::ofstream *out, 
				  std::ofstream *out1 = NULL,
				  std::ofstream *out2 = NULL
				  ) const ;

  private:
    param_map params; // key is crate
  };
}

#endif
