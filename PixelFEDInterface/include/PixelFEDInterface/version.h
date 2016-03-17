#ifndef _PixelFEDInterface_version_h_
#define _PixelFEDInterface_version_h_

#include "config/PackageInfo.h"

#define PIXELFEDINTERFACE_VERSION_MAJOR 1
#define PIXELFEDINTERFACE_VERSION_MINOR 0
#define PIXELFEDINTERFACE_VERSION_PATCH 0
#undef PIXELFEDINTERFACE_PREVIOUS_VERSIONS
#define PIXELFEDINTERFACE_VERSION_CODE PACKAGE_VERSION_CODE(PIXELFEDINTERFACE_VERSION_MAJOR, PIXELFEDINTERFACE_VERSION_MINOR, PIXELFEDINTERFACE_VERSION_PATCH)

#ifndef PIXELFEDINTERFACE_PREVIOUS_VERSIONS
#define PIXELFEDINTERFACE_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELFEDINTERFACE_VERSION_MAJOR, PIXELFEDINTERFACE_VERSION_MINOR, PIXELFEDINTERFACE_VERSION_PATCH)
#else
#define PIXELFEDINTERFACE_FULL_VERSION_LIST PIXELFEDINTERFACE_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELFEDINTERFACE_VERSION_MAJOR, PIXELFEDINTERFACE_VERSION_MINOR, PIXELFEDINTERFACE_VERSION_PATCH)
#endif

namespace PixelFEDInterface {
  
  const std::string package = "PixelFEDInterface";
  const std::string versions = PIXELFEDINTERFACE_FULL_VERSION_LIST;
  const std::string summary = "PixelFEDInterface";
  const std::string description = "PixelFEDInterface XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
