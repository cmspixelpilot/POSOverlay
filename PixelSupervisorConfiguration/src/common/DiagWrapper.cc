
#include "PixelSupervisorConfiguration/include/DiagWrapper.h"
#include <iostream>
#include <ctime>


DiagWrapper::DiagWrapper() {
  
  std::cout << "DiagWrapper initialised" << std::endl;
  
}

void DiagWrapper::reportError(std::string msg, int level) {
  
  std::time_t t = std::time(0);
  std::cout << std::asctime(std::localtime(&t)) << " DiagWrapper Level " << level << ": " << msg << std::endl;
  
}
