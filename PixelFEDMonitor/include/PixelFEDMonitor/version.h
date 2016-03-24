#ifndef _PixelFEDMonitor_version_h_
#define _PixelFEDMonitor_version_h_

#include "config/PackageInfo.h"

#define PIXELFEDMONITOR_VERSION_MAJOR 1
#define PIXELFEDMONITOR_VERSION_MINOR 0
#define PIXELFEDMONITOR_VERSION_PATCH 0
#undef PIXELFEDMONITOR_PREVIOUS_VERSIONS
#define PIXELFEDMONITOR_VERSION_CODE PACKAGE_VERSION_CODE(PIXELFEDMONITOR_VERSION_MAJOR, PIXELFEDMONITOR_VERSION_MINOR, PIXELFEDMONITOR_VERSION_PATCH)

#ifndef PIXELFEDMONITOR_PREVIOUS_VERSIONS
#define PIXELFEDMONITOR_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELFEDMONITOR_VERSION_MAJOR, PIXELFEDMONITOR_VERSION_MINOR, PIXELFEDMONITOR_VERSION_PATCH)
#else
#define PIXELFEDMONITOR_FULL_VERSION_LIST PIXELFEDMONITOR_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELFEDMONITOR_VERSION_MAJOR, PIXELFEDMONITOR_VERSION_MINOR, PIXELFEDMONITOR_VERSION_PATCH)
#endif

namespace PixelFEDMonitor {
  
  const std::string package = "PixelFEDMonitor";
  const std::string versions = PIXELFEDMONITOR_FULL_VERSION_LIST;
  const std::string summary = "PixelFEDMonitor";
  const std::string description = "PixelFEDMonitor XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
