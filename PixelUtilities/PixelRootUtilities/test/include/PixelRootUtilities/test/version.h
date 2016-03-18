#ifndef _PixelRootUtilities_version_h_
#define _PixelRootUtilities_version_h_

#include "config/PackageInfo.h"

#define PIXELROOTUTILITIES_VERSION_MAJOR 1
#define PIXELROOTUTILITIES_VERSION_MINOR 0
#define PIXELROOTUTILITIES_VERSION_PATCH 0
#undef PIXELROOTUTILITIES_PREVIOUS_VERSIONS
#define PIXELROOTUTILITIES_VERSION_CODE PACKAGE_VERSION_CODE(PIXELROOTUTILITIES_VERSION_MAJOR, PIXELROOTUTILITIES_VERSION_MINOR, PIXELROOTUTILITIES_VERSION_PATCH)

#ifndef PIXELROOTUTILITIES_PREVIOUS_VERSIONS
#define PIXELROOTUTILITIES_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELROOTUTILITIES_VERSION_MAJOR, PIXELROOTUTILITIES_VERSION_MINOR, PIXELROOTUTILITIES_VERSION_PATCH)
#else
#define PIXELROOTUTILITIES_FULL_VERSION_LIST PIXELROOTUTILITIES_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELROOTUTILITIES_VERSION_MAJOR, PIXELROOTUTILITIES_VERSION_MINOR, PIXELROOTUTILITIES_VERSION_PATCH)
#endif

namespace PixelRootUtilities {
  
  const std::string package = "PixelRootUtilities";
  const std::string versions = PIXELROOTUTILITIES_FULL_VERSION_LIST;
  const std::string summary = "PixelRootUtilities";
  const std::string description = "PixelRootUtilities XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
