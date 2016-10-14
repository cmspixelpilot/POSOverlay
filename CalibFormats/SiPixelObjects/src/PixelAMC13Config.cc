#include <cassert>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include "CalibFormats/SiPixelObjects/interface/PixelAMC13Config.h"
#include "CalibFormats/SiPixelObjects/interface/Utility.h"

namespace pos {  
  PixelAMC13Config::PixelAMC13Config(std::vector<std::vector<std::string> >& tableMat)
    : PixelConfigBase(" ", " ", " ")
  {
    assert(0);
  }

  std::pair<bool, PixelAMC13Parameters> PixelAMC13Config::parse_line(const std::string& line) {
    std::pair<bool, PixelAMC13Parameters> ret;
    ret.first = false;
    PixelAMC13Parameters& p = ret.second;

    if (line[0] != '#' && line.find_first_not_of(" \t") != std::string::npos) {
      std::vector<std::string> tokens = tokenize(line, true);
      const size_t ntokens = tokens.size();
      if (ntokens) {
        if (ntokens != 8 && ntokens != 10)
          throw std::runtime_error("# tokens not equal to 8 or 10: line: " + line);
        else {
          p.setCrate(atoi(tokens[0].c_str()));
          p.setUriT1(tokens[1]);
          p.setUriT2(tokens[2]);
          p.setSlotMask(tokens[3]);
          p.setCalBX(strtoul(tokens[4].c_str(), 0, 10));
          p.setL1ADelay(strtoul(tokens[5].c_str(), 0, 10));
          p.setNewWay(tolower(tokens[6][0]) == 'y');
          p.setVerifyL1A(tolower(tokens[7][0]) == 'y');
          if (ntokens == 10) {
            p.setAddressT1(tokens[8]);
            p.setAddressT2(tokens[9]);
          }
          ret.first = true;
        }
      }
    }

    return ret;
  }

  PixelAMC13Config::PixelAMC13Config(std::string filename)
    : PixelConfigBase(" "," "," ")
  {
    std::string mthn ="]\t[PixelAMC13Config::PixelAMC13Config()]\t\t\t    " ;
    std::ifstream in(filename.c_str());

    if (!in.good()) {
      std::cout << __LINE__ << mthn << "Could not open: " << filename << std::endl;
      throw std::runtime_error("Failed to open file " + filename);
    }
    else
      std::cout << __LINE__ << mthn << "Opened : "        << filename << std::endl;

    std::string line;
    while (getline(in, line)) {
      std::pair<bool, PixelAMC13Parameters> r = parse_line(line);
      if (r.first)
        params[r.second.getCrate()] = r.second;
    }
  } 

  std::string PixelAMC13Config::toASCII() const {
    std::ostringstream out;
    out << "# crate  uriT1                                                      uriT2                                                 slotMask                 calBX  L1ADelay  DoNothing  NewWay  VerifyL1A   [addressT1, addressT2 columns here if you want, otherwise assumed to use system amc13 etc]\n";
    char silly[2] = {'n', 'y'};
    for (param_map::const_iterator it = params.begin(), ite = params.end(); it != ite; ++it) {
      PixelAMC13Parameters p = it->second;
      assert(p.getCrate() == it->first);
      out << p.getCrate()
          << " " << p.getUriT1()
          << " " << p.getUriT2()
          << " " << p.getSlotMask()
          << " " << p.getCalBX()
          << " " << p.getL1ADelay()
          << " " << silly[p.getNewWay()]
          << " " << silly[p.getVerifyL1A()]
          << " " << p.getAddressT1()
          << " " << p.getAddressT2()
          << "\n";
    }
    return out.str();
  }
    
  void PixelAMC13Config::writeASCII(std::string dir) const {
    if (dir!="") dir+="/";
    std::string filename = dir + "amc13.dat";
    std::ofstream out(filename.c_str());
    out << toASCII();
    out.close();
  }

  void PixelAMC13Config::writeXMLHeader(pos::PixelConfigKey key, 
                                        int version, 
                                        std::string path, 
                                        std::ofstream *outstream,
                                        std::ofstream *out1stream,
                                        std::ofstream *out2stream) const
  {
    assert(0);
  }
 
  void PixelAMC13Config::writeXML( std::ofstream *outstream,
                                   std::ofstream *out1stream,
                                   std::ofstream *out2stream) const 
  {
    assert(0);
  }

  void PixelAMC13Config::writeXMLTrailer(std::ofstream *outstream,
                                         std::ofstream *out1stream,
                                         std::ofstream *out2stream ) const 
  {
    assert(0);
  }
}
