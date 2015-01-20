// utilities.cc
#include "PixelConfigDBInterface/include/PixelDatabaseUtilities.h"

#include <fstream>
#include <iostream>
#include <unistd.h>
#include <cstdlib>

void load_variable(const char* file_name,
		   std::string& variable,
		   const char* default_value){
//   std::cout << __LINE__ << "]\t[PixelDatabaseUtilities::load_variable]\t\t\t    Opening file before HOME" << std::endl ;
  std::string fileDir = getenv("HOME");
  fileDir += "/private/" ;
  //  std::string fileDir = "/nfshome0/pixelpro/private/";
//   std::cout << __LINE__ << "]\t[PixelDatabaseUtilities::load_variable]\t\t\t    Opening file: " << fileDir << file_name << std::endl ;
  std::fstream file((fileDir+file_name).c_str(),std::ios::in);
  if (getline(file,variable) && variable != "")
    return;
  else if (default_value) {
      variable = default_value;
      return ;
  } else {
    std::cout << "enter " << file_name << " : " << std::flush;
    getline(std::cin,variable);
  }
}
