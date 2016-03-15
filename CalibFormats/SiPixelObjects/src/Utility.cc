#include "CalibFormats/SiPixelObjects/interface/Utility.h"

#include <algorithm>
#include <iterator>
#include <sstream>

namespace pos {
  std::vector<std::string> tokenize(const std::string& s, bool handle_comments) {
    std::istringstream iss(s);
    std::vector<std::string> tokens;
    std::copy(std::istream_iterator<std::string>(iss),
	      std::istream_iterator<std::string>(),
	      std::back_inserter(tokens));

    if (handle_comments)
      for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); ++it)
	if ((*it)[0] == '#')
	  tokens.erase(it, tokens.end());

    return tokens;
  }
}

