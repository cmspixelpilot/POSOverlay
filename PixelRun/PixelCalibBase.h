#ifndef PixelCalibBase_h
#define PixelCalibBase_h
//
// Base class for pixel configuration data
//

#include <string>

class PixelCalibBase {

 public:

    PixelCalibBase();
    virtual ~PixelCalibBase();
    virtual std::string mode();

 private:

    std::string mode_;

};


#endif
