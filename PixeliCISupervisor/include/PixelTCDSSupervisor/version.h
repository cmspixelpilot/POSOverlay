#ifndef _pixel_tcds_PixelTCDSSupervisor_version_h_
#define _pixel_tcds_PixelTCDSSupervisor_version_h_

#include <string>

#include "config/PackageInfo.h"

// !!! Edit these lines to reflect the latest package version. !!!
#define PIXELTCDSSUPERVISOR_VERSION_MAJOR 1
#define PIXELTCDSSUPERVISOR_VERSION_MINOR 0
#define PIXELTCDSSUPERVISOR_VERSION_PATCH 0

// If any previous versions available:
// #define PIXELTCDSSUPERVISOR_PREVIOUS_VERSIONS "3.8.0,3.8.1"
// else:
#undef PIXELTCDSSUPERVISOR_PREVIOUS_VERSIONS

//
// Template macros and boilerplate code.
//
#define PIXELTCDSSUPERVISOR_VERSION_CODE PACKAGE_VERSION_CODE(PIXELTCDSSUPERVISOR_VERSION_MAJOR,PIXELTCDSSUPERVISOR_VERSION_MINOR,PIXELTCDSSUPERVISOR_VERSION_PATCH)
#ifndef PIXELTCDSSUPERVISOR_PREVIOUS_VERSIONS
#define PIXELTCDSSUPERVISOR_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELTCDSSUPERVISOR_VERSION_MAJOR,PIXELTCDSSUPERVISOR_VERSION_MINOR,PIXELTCDSSUPERVISOR_VERSION_PATCH)
#else
#define PIXELTCDSSUPERVISOR_FULL_VERSION_LIST PIXELTCDSSUPERVISOR_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELTCDSSUPERVISOR_VERSION_MAJOR,PIXELTCDSSUPERVISOR_VERSION_MINOR,PIXELTCDSSUPERVISOR_VERSION_PATCH)
#endif

namespace pixel {
  namespace tcds {

    const std::string package = "PixelTCDSSupervisor";
    const std::string versions = PIXELTCDSSUPERVISOR_FULL_VERSION_LIST;
    const std::string description = "Pixel XDAQ application communicating with a TCDS control application.";
    const std::string authors = "Clemens Lange";
    const std::string summary = "TCDS control application for pixel";
    const std::string link = "https://twiki.cern.ch/twiki/bin/view/CMS/PixelOnlineSoftwareDevelopment";
    config::PackageInfo getPackageInfo();
    void checkPackageDependencies();
    std::set<std::string, std::less<std::string> > getPackageDependencies();
  } // namespace tcds
} // namespace pixel

#endif // _pixel_tcds_PixelTCDSSupervisor_version_h_
