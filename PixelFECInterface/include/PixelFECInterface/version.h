#ifndef _PixelFECInterface_version_h_
#define _PixelFECInterface_version_h_

#include "config/PackageInfo.h"

#define PIXELFECINTERFACE_VERSION_MAJOR 1
#define PIXELFECINTERFACE_VERSION_MINOR 0
#define PIXELFECINTERFACE_VERSION_PATCH 0
#undef PIXELFECINTERFACE_PREVIOUS_VERSIONS
#define PIXELFECINTERFACE_VERSION_CODE PACKAGE_VERSION_CODE(PIXELFECINTERFACE_VERSION_MAJOR, PIXELFECINTERFACE_VERSION_MINOR, PIXELFECINTERFACE_VERSION_PATCH)

#ifndef PIXELFECINTERFACE_PREVIOUS_VERSIONS
#define PIXELFECINTERFACE_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELFECINTERFACE_VERSION_MAJOR, PIXELFECINTERFACE_VERSION_MINOR, PIXELFECINTERFACE_VERSION_PATCH)
#else
#define PIXELFECINTERFACE_FULL_VERSION_LIST PIXELFECINTERFACE_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELFECINTERFACE_VERSION_MAJOR, PIXELFECINTERFACE_VERSION_MINOR, PIXELFECINTERFACE_VERSION_PATCH)
#endif

namespace PixelFECInterface {
  
  const std::string package = "PixelFECInterface";
  const std::string versions = PIXELFECINTERFACE_FULL_VERSION_LIST;
  const std::string summary = "PixelFECInterface";
  const std::string description = "PixelFECInterface XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
