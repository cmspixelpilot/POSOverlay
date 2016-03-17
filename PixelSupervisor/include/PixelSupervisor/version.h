#ifndef _PixelSupervisor_version_h_
#define _PixelSupervisor_version_h_

#include "config/PackageInfo.h"

#define PIXELSUPERVISOR_VERSION_MAJOR 1
#define PIXELSUPERVISOR_VERSION_MINOR 0
#define PIXELSUPERVISOR_VERSION_PATCH 0
#undef PIXELSUPERVISOR_PREVIOUS_VERSIONS
#define PIXELSUPERVISOR_VERSION_CODE PACKAGE_VERSION_CODE(PIXELSUPERVISOR_VERSION_MAJOR, PIXELSUPERVISOR_VERSION_MINOR, PIXELSUPERVISOR_VERSION_PATCH)

#ifndef PIXELSUPERVISOR_PREVIOUS_VERSIONS
#define PIXELSUPERVISOR_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELSUPERVISOR_VERSION_MAJOR, PIXELSUPERVISOR_VERSION_MINOR, PIXELSUPERVISOR_VERSION_PATCH)
#else
#define PIXELSUPERVISOR_FULL_VERSION_LIST PIXELSUPERVISOR_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELSUPERVISOR_VERSION_MAJOR, PIXELSUPERVISOR_VERSION_MINOR, PIXELSUPERVISOR_VERSION_PATCH)
#endif

namespace PixelSupervisor {
  
  const std::string package = "PixelSupervisor";
  const std::string versions = PIXELSUPERVISOR_FULL_VERSION_LIST;
  const std::string summary = "PixelSupervisor";
  const std::string description = "PixelSupervisor XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
