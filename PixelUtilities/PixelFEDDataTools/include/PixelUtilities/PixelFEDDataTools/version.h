#ifndef _PixelFEDDataTools_version_h_
#define _PixelFEDDataTools_version_h_

#include "config/PackageInfo.h"

#define PIXELFEDDATATOOLS_VERSION_MAJOR 1
#define PIXELFEDDATATOOLS_VERSION_MINOR 0
#define PIXELFEDDATATOOLS_VERSION_PATCH 0
#undef PIXELFEDDATATOOLS_PREVIOUS_VERSIONS
#define PIXELFEDDATATOOLS_VERSION_CODE PACKAGE_VERSION_CODE(PIXELFEDDATATOOLS_VERSION_MAJOR, PIXELFEDDATATOOLS_VERSION_MINOR, PIXELFEDDATATOOLS_VERSION_PATCH)

#ifndef PIXELFEDDATATOOLS_PREVIOUS_VERSIONS
#define PIXELFEDDATATOOLS_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELFEDDATATOOLS_VERSION_MAJOR, PIXELFEDDATATOOLS_VERSION_MINOR, PIXELFEDDATATOOLS_VERSION_PATCH)
#else
#define PIXELFEDDATATOOLS_FULL_VERSION_LIST PIXELFEDDATATOOLS_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELFEDDATATOOLS_VERSION_MAJOR, PIXELFEDDATATOOLS_VERSION_MINOR, PIXELFEDDATATOOLS_VERSION_PATCH)
#endif

namespace PixelFEDDataTools {
  
  const std::string package = "PixelFEDDataTools";
  const std::string versions = PIXELFEDDATATOOLS_FULL_VERSION_LIST;
  const std::string summary = "PixelFEDDataTools";
  const std::string description = "PixelFEDDataTools XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
