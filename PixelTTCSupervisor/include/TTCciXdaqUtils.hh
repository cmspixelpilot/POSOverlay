#ifndef __TTCciXdaqUtils_h__
#define __TTCciXdaqUtils_h__

#include "cgicc/CgiDefs.h"
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <algorithm>

namespace ttc {
  
  void DropDownMenu(std::ostream &out, 
                    const std::string &name,
                    std::vector<std::string> &options,
                    size_t default_value=0);
  void DropDownMenu(std::ostream &out, 
                    const std::string &name,
                    std::vector<std::string> &options,
                    std::vector<std::string> &values,
                    std::string default_value);


}// namespace ttc 

#endif // _TTCciXdaqUtils_h__
