#ifndef _PixelDCSInterface_version_h_
#define _PixelDCSInterface_version_h_

#include "config/PackageInfo.h"

#define PIXELDCSINTERFACE_VERSION_MAJOR 1
#define PIXELDCSINTERFACE_VERSION_MINOR 0
#define PIXELDCSINTERFACE_VERSION_PATCH 0
#undef PIXELDCSINTERFACE_PREVIOUS_VERSIONS
#define PIXELDCSINTERFACE_VERSION_CODE PACKAGE_VERSION_CODE(PIXELDCSINTERFACE_VERSION_MAJOR, PIXELDCSINTERFACE_VERSION_MINOR, PIXELDCSINTERFACE_VERSION_PATCH)

#ifndef PIXELDCSINTERFACE_PREVIOUS_VERSIONS
#define PIXELDCSINTERFACE_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELDCSINTERFACE_VERSION_MAJOR, PIXELDCSINTERFACE_VERSION_MINOR, PIXELDCSINTERFACE_VERSION_PATCH)
#else
#define PIXELDCSINTERFACE_FULL_VERSION_LIST PIXELDCSINTERFACE_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELDCSINTERFACE_VERSION_MAJOR, PIXELDCSINTERFACE_VERSION_MINOR, PIXELDCSINTERFACE_VERSION_PATCH)
#endif

namespace PixelDCSInterface {
  
  const std::string package = "PixelDCSInterface";
  const std::string versions = PIXELDCSINTERFACE_FULL_VERSION_LIST;
  const std::string summary = "PixelDCSInterface";
  const std::string description = "PixelDCSInterface XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
