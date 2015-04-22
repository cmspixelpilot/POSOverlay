#ifndef _pixel_tcds_exception_Exception_h_
#define _pixel_tcds_exception_Exception_h_

#include <string>

#include "xcept/Exception.h"

// Mimick XCEPT_DEFINE_EXCEPTION from xcept/Exception.h.
#define ICI_DEFINE_EXCEPTION(EXCEPTION_NAME)                             \
namespace pixel {                                                                       \
  namespace tcds {                                                        \
    namespace exception {                                             \
      class EXCEPTION_NAME : public pixel::tcds::exception::Exception \
      {                                                               \
        public :                                                        \
        EXCEPTION_NAME(std::string name,                                \
                       std::string message,                             \
                       std::string module,                              \
                       int line,                                        \
                       std::string function) :                          \
          pixel::tcds::exception::Exception(name, message, module, line, function) \
            {};                                                         \
        EXCEPTION_NAME(std::string name,                                                   \
                     std::string message,                               \
                     std::string module,                                \
                     int line,                                          \
                     std::string function,                              \
                     xcept::Exception& err) :                           \
          pixel::tcds::exception::Exception(name, message, module, line, function, err) \
        {};                                                             \
      };                                                                \
    }                                                                   \
  }                                                                     \
}                                                                       \



namespace pixel {
  namespace tcds {
    namespace exception {

      class Exception : public xcept::Exception
      {
      public:
        Exception(std::string name,
                  std::string message,
                  std::string module,
                  int line,
                  std::string function);
        Exception(std::string name,
                  std::string message,
                  std::string module,
                  int line,
                  std::string function,
                  xcept::Exception& err);
      };

    } // namespace exception
  } // namespace tcds
} // namespace pixel

ICI_DEFINE_EXCEPTION(RuntimeError)
ICI_DEFINE_EXCEPTION(ValueError)

#endif // _pixel_tcds_exception_Exception_h_
