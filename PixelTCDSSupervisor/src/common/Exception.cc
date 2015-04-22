#include "PixelTCDSSupervisor/exception/Exception.h"

pixel::tcds::exception::Exception::Exception(std::string name,
                                                std::string message,
                                                std::string module,
                                                int line,
                                                std::string function) :
  xcept::Exception(name, message, module, line, function)
{
}

pixel::tcds::exception::Exception::Exception(std::string name,
                                                std::string message,
                                                std::string module,
                                                int line,
                                                std::string function,
                                                xcept::Exception& err) :
  xcept::Exception(name, message, module, line, function, err)
{
}
