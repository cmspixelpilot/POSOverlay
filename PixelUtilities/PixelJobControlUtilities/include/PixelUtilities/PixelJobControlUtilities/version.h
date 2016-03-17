#ifndef _PixelJobControlUtilities_version_h_
#define _PixelJobControlUtilities_version_h_

#include "config/PackageInfo.h"

#define PIXELJOBCONTROLUTILITIES_VERSION_MAJOR 1
#define PIXELJOBCONTROLUTILITIES_VERSION_MINOR 0
#define PIXELJOBCONTROLUTILITIES_VERSION_PATCH 0
#undef PIXELJOBCONTROLUTILITIES_PREVIOUS_VERSIONS
#define PIXELJOBCONTROLUTILITIES_VERSION_CODE PACKAGE_VERSION_CODE(PIXELJOBCONTROLUTILITIES_VERSION_MAJOR, PIXELJOBCONTROLUTILITIES_VERSION_MINOR, PIXELJOBCONTROLUTILITIES_VERSION_PATCH)

#ifndef PIXELJOBCONTROLUTILITIES_PREVIOUS_VERSIONS
#define PIXELJOBCONTROLUTILITIES_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELJOBCONTROLUTILITIES_VERSION_MAJOR, PIXELJOBCONTROLUTILITIES_VERSION_MINOR, PIXELJOBCONTROLUTILITIES_VERSION_PATCH)
#else
#define PIXELJOBCONTROLUTILITIES_FULL_VERSION_LIST PIXELJOBCONTROLUTILITIES_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELJOBCONTROLUTILITIES_VERSION_MAJOR, PIXELJOBCONTROLUTILITIES_VERSION_MINOR, PIXELJOBCONTROLUTILITIES_VERSION_PATCH)
#endif

namespace PixelJobControlUtilities {
  
  const std::string package = "PixelJobControlUtilities";
  const std::string versions = PIXELJOBCONTROLUTILITIES_FULL_VERSION_LIST;
  const std::string summary = "PixelJobControlUtilities";
  const std::string description = "PixelJobControlUtilities XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
