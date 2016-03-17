#ifndef _PixelCalibrations_version_h_
#define _PixelCalibrations_version_h_

#include "config/PackageInfo.h"

#define PIXELCALIBRATIONS_VERSION_MAJOR 1
#define PIXELCALIBRATIONS_VERSION_MINOR 0
#define PIXELCALIBRATIONS_VERSION_PATCH 0
#undef PIXELCALIBRATIONS_PREVIOUS_VERSIONS
#define PIXELCALIBRATIONS_VERSION_CODE PACKAGE_VERSION_CODE(PIXELCALIBRATIONS_VERSION_MAJOR, PIXELCALIBRATIONS_VERSION_MINOR, PIXELCALIBRATIONS_VERSION_PATCH)

#ifndef PIXELCALIBRATIONS_PREVIOUS_VERSIONS
#define PIXELCALIBRATIONS_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELCALIBRATIONS_VERSION_MAJOR, PIXELCALIBRATIONS_VERSION_MINOR, PIXELCALIBRATIONS_VERSION_PATCH)
#else
#define PIXELCALIBRATIONS_FULL_VERSION_LIST PIXELCALIBRATIONS_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELCALIBRATIONS_VERSION_MAJOR, PIXELCALIBRATIONS_VERSION_MINOR, PIXELCALIBRATIONS_VERSION_PATCH)
#endif

namespace PixelCalibrations {
  
  const std::string package = "PixelCalibrations";
  const std::string versions = PIXELCALIBRATIONS_FULL_VERSION_LIST;
  const std::string summary = "PixelCalibrations";
  const std::string description = "PixelCalibrations XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
