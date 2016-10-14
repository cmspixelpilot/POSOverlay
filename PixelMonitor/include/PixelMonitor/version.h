#ifndef _PixelMonitor_version_h_
#define _PixelMonitor_version_h_

#include "config/PackageInfo.h"

#define PIXELMONITOR_VERSION_MAJOR 1
#define PIXELMONITOR_VERSION_MINOR 0
#define PIXELMONITOR_VERSION_PATCH 0
#undef PIXELMONITOR_PREVIOUS_VERSIONS
#define PIXELMONITOR_VERSION_CODE PACKAGE_VERSION_CODE(PIXELMONITOR_VERSION_MAJOR, PIXELMONITOR_VERSION_MINOR, PIXELMONITOR_VERSION_PATCH)

#ifndef PIXELMONITOR_PREVIOUS_VERSIONS
#define PIXELMONITOR_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELMONITOR_VERSION_MAJOR, PIXELMONITOR_VERSION_MINOR, PIXELMONITOR_VERSION_PATCH)
#else
#define PIXELMONITOR_FULL_VERSION_LIST PIXELMONITOR_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELMONITOR_VERSION_MAJOR, PIXELMONITOR_VERSION_MINOR, PIXELMONITOR_VERSION_PATCH)
#endif

namespace PixelMonitor {
  
  const std::string package = "PixelMonitor";
  const std::string versions = PIXELMONITOR_FULL_VERSION_LIST;
  const std::string summary = "PixelMonitor";
  const std::string description = "PixelMonitor XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
