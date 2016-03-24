#ifndef _PixelTKFECDataTools_version_h_
#define _PixelTKFECDataTools_version_h_

#include "config/PackageInfo.h"

#define PIXELTKFECDATATOOLS_VERSION_MAJOR 1
#define PIXELTKFECDATATOOLS_VERSION_MINOR 0
#define PIXELTKFECDATATOOLS_VERSION_PATCH 0
#undef PIXELTKFECDATATOOLS_PREVIOUS_VERSIONS
#define PIXELTKFECDATATOOLS_VERSION_CODE PACKAGE_VERSION_CODE(PIXELTKFECDATATOOLS_VERSION_MAJOR, PIXELTKFECDATATOOLS_VERSION_MINOR, PIXELTKFECDATATOOLS_VERSION_PATCH)

#ifndef PIXELTKFECDATATOOLS_PREVIOUS_VERSIONS
#define PIXELTKFECDATATOOLS_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELTKFECDATATOOLS_VERSION_MAJOR, PIXELTKFECDATATOOLS_VERSION_MINOR, PIXELTKFECDATATOOLS_VERSION_PATCH)
#else
#define PIXELTKFECDATATOOLS_FULL_VERSION_LIST PIXELTKFECDATATOOLS_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELTKFECDATATOOLS_VERSION_MAJOR, PIXELTKFECDATATOOLS_VERSION_MINOR, PIXELTKFECDATATOOLS_VERSION_PATCH)
#endif

namespace PixelTKFECDataTools {
  
  const std::string package = "PixelTKFECDataTools";
  const std::string versions = PIXELTKFECDATATOOLS_FULL_VERSION_LIST;
  const std::string summary = "PixelTKFECDataTools";
  const std::string description = "PixelTKFECDataTools XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
