#ifndef _PixelI2OUtilities_version_h_
#define _PixelI2OUtilities_version_h_

#include "config/PackageInfo.h"

#define PIXELI2OUTILITIES_VERSION_MAJOR 1
#define PIXELI2OUTILITIES_VERSION_MINOR 0
#define PIXELI2OUTILITIES_VERSION_PATCH 0
#undef PIXELI2OUTILITIES_PREVIOUS_VERSIONS
#define PIXELI2OUTILITIES_VERSION_CODE PACKAGE_VERSION_CODE(PIXELI2OUTILITIES_VERSION_MAJOR, PIXELI2OUTILITIES_VERSION_MINOR, PIXELI2OUTILITIES_VERSION_PATCH)

#ifndef PIXELI2OUTILITIES_PREVIOUS_VERSIONS
#define PIXELI2OUTILITIES_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELI2OUTILITIES_VERSION_MAJOR, PIXELI2OUTILITIES_VERSION_MINOR, PIXELI2OUTILITIES_VERSION_PATCH)
#else
#define PIXELI2OUTILITIES_FULL_VERSION_LIST PIXELI2OUTILITIES_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELI2OUTILITIES_VERSION_MAJOR, PIXELI2OUTILITIES_VERSION_MINOR, PIXELI2OUTILITIES_VERSION_PATCH)
#endif

namespace PixelI2OUtilities {
  
  const std::string package = "PixelI2OUtilities";
  const std::string versions = PIXELI2OUTILITIES_FULL_VERSION_LIST;
  const std::string summary = "PixelI2OUtilities";
  const std::string description = "PixelI2OUtilities XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
