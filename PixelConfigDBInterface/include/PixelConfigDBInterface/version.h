#ifndef _PixelConfigDBInterface_version_h_
#define _PixelConfigDBInterface_version_h_

#include "config/PackageInfo.h"

#define PIXELCONFIGDBINTERFACE_VERSION_MAJOR 1
#define PIXELCONFIGDBINTERFACE_VERSION_MINOR 0
#define PIXELCONFIGDBINTERFACE_VERSION_PATCH 0
#undef PIXELCONFIGDBINTERFACE_PREVIOUS_VERSIONS
#define PIXELCONFIGDBINTERFACE_VERSION_CODE PACKAGE_VERSION_CODE(PIXELCONFIGDBINTERFACE_VERSION_MAJOR, PIXELCONFIGDBINTERFACE_VERSION_MINOR, PIXELCONFIGDBINTERFACE_VERSION_PATCH)

#ifndef PIXELCONFIGDBINTERFACE_PREVIOUS_VERSIONS
#define PIXELCONFIGDBINTERFACE_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELCONFIGDBINTERFACE_VERSION_MAJOR, PIXELCONFIGDBINTERFACE_VERSION_MINOR, PIXELCONFIGDBINTERFACE_VERSION_PATCH)
#else
#define PIXELCONFIGDBINTERFACE_FULL_VERSION_LIST PIXELCONFIGDBINTERFACE_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELCONFIGDBINTERFACE_VERSION_MAJOR, PIXELCONFIGDBINTERFACE_VERSION_MINOR, PIXELCONFIGDBINTERFACE_VERSION_PATCH)
#endif

namespace PixelConfigDBInterface {
  
  const std::string package = "PixelConfigDBInterface";
  const std::string versions = PIXELCONFIGDBINTERFACE_FULL_VERSION_LIST;
  const std::string summary = "PixelConfigDBInterface";
  const std::string description = "PixelConfigDBInterface XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
