#ifndef TBMReadException_h
#define TBMReadException_h

class TBMReadException: public std::exception {
 public:
  virtual const char* what() const throw() {
    return "Failed to read from TBM";
  }
};

#endif
