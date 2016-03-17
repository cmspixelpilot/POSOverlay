#ifndef _Pixelb2inUtilities_version_h_
#define _Pixelb2inUtilities_version_h_

#include "config/PackageInfo.h"

#define PIXELB2INUTILITIES_VERSION_MAJOR 1
#define PIXELB2INUTILITIES_VERSION_MINOR 0
#define PIXELB2INUTILITIES_VERSION_PATCH 0
#undef PIXELB2INUTILITIES_PREVIOUS_VERSIONS
#define PIXELB2INUTILITIES_VERSION_CODE PACKAGE_VERSION_CODE(PIXELB2INUTILITIES_VERSION_MAJOR, PIXELB2INUTILITIES_VERSION_MINOR, PIXELB2INUTILITIES_VERSION_PATCH)

#ifndef PIXELB2INUTILITIES_PREVIOUS_VERSIONS
#define PIXELB2INUTILITIES_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELB2INUTILITIES_VERSION_MAJOR, PIXELB2INUTILITIES_VERSION_MINOR, PIXELB2INUTILITIES_VERSION_PATCH)
#else
#define PIXELB2INUTILITIES_FULL_VERSION_LIST PIXELB2INUTILITIES_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELB2INUTILITIES_VERSION_MAJOR, PIXELB2INUTILITIES_VERSION_MINOR, PIXELB2INUTILITIES_VERSION_PATCH)
#endif

namespace Pixelb2inUtilities {
  
  const std::string package = "Pixelb2inUtilities";
  const std::string versions = PIXELB2INUTILITIES_FULL_VERSION_LIST;
  const std::string summary = "Pixelb2inUtilities";
  const std::string description = "Pixelb2inUtilities XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
