#ifndef _PixelSharedDataTools_version_h_
#define _PixelSharedDataTools_version_h_

#include "config/PackageInfo.h"

#define PIXELSHAREDDATATOOLS_VERSION_MAJOR 1
#define PIXELSHAREDDATATOOLS_VERSION_MINOR 0
#define PIXELSHAREDDATATOOLS_VERSION_PATCH 0
#undef PIXELSHAREDDATATOOLS_PREVIOUS_VERSIONS
#define PIXELSHAREDDATATOOLS_VERSION_CODE PACKAGE_VERSION_CODE(PIXELSHAREDDATATOOLS_VERSION_MAJOR, PIXELSHAREDDATATOOLS_VERSION_MINOR, PIXELSHAREDDATATOOLS_VERSION_PATCH)

#ifndef PIXELSHAREDDATATOOLS_PREVIOUS_VERSIONS
#define PIXELSHAREDDATATOOLS_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELSHAREDDATATOOLS_VERSION_MAJOR, PIXELSHAREDDATATOOLS_VERSION_MINOR, PIXELSHAREDDATATOOLS_VERSION_PATCH)
#else
#define PIXELSHAREDDATATOOLS_FULL_VERSION_LIST PIXELSHAREDDATATOOLS_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELSHAREDDATATOOLS_VERSION_MAJOR, PIXELSHAREDDATATOOLS_VERSION_MINOR, PIXELSHAREDDATATOOLS_VERSION_PATCH)
#endif

namespace PixelSharedDataTools {
  
  const std::string package = "PixelSharedDataTools";
  const std::string versions = PIXELSHAREDDATATOOLS_FULL_VERSION_LIST;
  const std::string summary = "PixelSharedDataTools";
  const std::string description = "PixelSharedDataTools XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
