#ifndef CalibFormats_SiPixelObjects_Utility_h
#define CalibFormats_SiPixelObjects_Utility_h

#include <string>
#include <vector>

namespace pos {
  std::vector<std::string> tokenize(const std::string& s, bool handle_comments);
}

#endif
