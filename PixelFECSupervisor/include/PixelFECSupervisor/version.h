#ifndef _PixelFECSupervisor_version_h_
#define _PixelFECSupervisor_version_h_

#include "config/PackageInfo.h"

#define PIXELFECSUPERVISOR_VERSION_MAJOR 1
#define PIXELFECSUPERVISOR_VERSION_MINOR 0
#define PIXELFECSUPERVISOR_VERSION_PATCH 0
#undef PIXELFECSUPERVISOR_PREVIOUS_VERSIONS
#define PIXELFECSUPERVISOR_VERSION_CODE PACKAGE_VERSION_CODE(PIXELFECSUPERVISOR_VERSION_MAJOR, PIXELFECSUPERVISOR_VERSION_MINOR, PIXELFECSUPERVISOR_VERSION_PATCH)

#ifndef PIXELFECSUPERVISOR_PREVIOUS_VERSIONS
#define PIXELFECSUPERVISOR_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELFECSUPERVISOR_VERSION_MAJOR, PIXELFECSUPERVISOR_VERSION_MINOR, PIXELFECSUPERVISOR_VERSION_PATCH)
#else
#define PIXELFECSUPERVISOR_FULL_VERSION_LIST PIXELFECSUPERVISOR_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELFECSUPERVISOR_VERSION_MAJOR, PIXELFECSUPERVISOR_VERSION_MINOR, PIXELFECSUPERVISOR_VERSION_PATCH)
#endif

namespace PixelFECSupervisor {
  
  const std::string package = "PixelFECSupervisor";
  const std::string versions = PIXELFECSUPERVISOR_FULL_VERSION_LIST;
  const std::string summary = "PixelFECSupervisor";
  const std::string description = "PixelFECSupervisor XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
