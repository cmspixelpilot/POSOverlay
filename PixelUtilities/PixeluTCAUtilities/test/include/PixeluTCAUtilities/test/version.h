#ifndef _PixeluTCAUtilities_version_h_
#define _PixeluTCAUtilities_version_h_

#include "config/PackageInfo.h"

#define PIXELUTCAUTILITIES_VERSION_MAJOR 1
#define PIXELUTCAUTILITIES_VERSION_MINOR 0
#define PIXELUTCAUTILITIES_VERSION_PATCH 0
#undef PIXELUTCAUTILITIES_PREVIOUS_VERSIONS
#define PIXELUTCAUTILITIES_VERSION_CODE PACKAGE_VERSION_CODE(PIXELUTCAUTILITIES_VERSION_MAJOR, PIXELUTCAUTILITIES_VERSION_MINOR, PIXELUTCAUTILITIES_VERSION_PATCH)

#ifndef PIXELUTCAUTILITIES_PREVIOUS_VERSIONS
#define PIXELUTCAUTILITIES_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELUTCAUTILITIES_VERSION_MAJOR, PIXELUTCAUTILITIES_VERSION_MINOR, PIXELUTCAUTILITIES_VERSION_PATCH)
#else
#define PIXELUTCAUTILITIES_FULL_VERSION_LIST PIXELUTCAUTILITIES_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELUTCAUTILITIES_VERSION_MAJOR, PIXELUTCAUTILITIES_VERSION_MINOR, PIXELUTCAUTILITIES_VERSION_PATCH)
#endif

namespace PixeluTCAUtilities {
  
  const std::string package = "PixeluTCAUtilities";
  const std::string versions = PIXELUTCAUTILITIES_FULL_VERSION_LIST;
  const std::string summary = "PixeluTCAUtilities";
  const std::string description = "PixeluTCAUtilities XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
