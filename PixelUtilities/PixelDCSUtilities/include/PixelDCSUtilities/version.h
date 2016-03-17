#ifndef _PixelDCSUtilities_version_h_
#define _PixelDCSUtilities_version_h_

#include "config/PackageInfo.h"

#define PIXELDCSUTILITIES_VERSION_MAJOR 1
#define PIXELDCSUTILITIES_VERSION_MINOR 0
#define PIXELDCSUTILITIES_VERSION_PATCH 0
#undef PIXELDCSUTILITIES_PREVIOUS_VERSIONS
#define PIXELDCSUTILITIES_VERSION_CODE PACKAGE_VERSION_CODE(PIXELDCSUTILITIES_VERSION_MAJOR, PIXELDCSUTILITIES_VERSION_MINOR, PIXELDCSUTILITIES_VERSION_PATCH)

#ifndef PIXELDCSUTILITIES_PREVIOUS_VERSIONS
#define PIXELDCSUTILITIES_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELDCSUTILITIES_VERSION_MAJOR, PIXELDCSUTILITIES_VERSION_MINOR, PIXELDCSUTILITIES_VERSION_PATCH)
#else
#define PIXELDCSUTILITIES_FULL_VERSION_LIST PIXELDCSUTILITIES_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELDCSUTILITIES_VERSION_MAJOR, PIXELDCSUTILITIES_VERSION_MINOR, PIXELDCSUTILITIES_VERSION_PATCH)
#endif

namespace PixelDCSUtilities {
  
  const std::string package = "PixelDCSUtilities";
  const std::string versions = PIXELDCSUTILITIES_FULL_VERSION_LIST;
  const std::string summary = "PixelDCSUtilities";
  const std::string description = "PixelDCSUtilities XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
