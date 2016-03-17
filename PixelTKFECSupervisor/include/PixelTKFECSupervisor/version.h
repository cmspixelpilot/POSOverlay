#ifndef _PixelTKFECSupervisor_version_h_
#define _PixelTKFECSupervisor_version_h_

#include "config/PackageInfo.h"

#define PIXELTKFECSUPERVISOR_VERSION_MAJOR 1
#define PIXELTKFECSUPERVISOR_VERSION_MINOR 0
#define PIXELTKFECSUPERVISOR_VERSION_PATCH 0
#undef PIXELTKFECSUPERVISOR_PREVIOUS_VERSIONS
#define PIXELTKFECSUPERVISOR_VERSION_CODE PACKAGE_VERSION_CODE(PIXELTKFECSUPERVISOR_VERSION_MAJOR, PIXELTKFECSUPERVISOR_VERSION_MINOR, PIXELTKFECSUPERVISOR_VERSION_PATCH)

#ifndef PIXELTKFECSUPERVISOR_PREVIOUS_VERSIONS
#define PIXELTKFECSUPERVISOR_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELTKFECSUPERVISOR_VERSION_MAJOR, PIXELTKFECSUPERVISOR_VERSION_MINOR, PIXELTKFECSUPERVISOR_VERSION_PATCH)
#else
#define PIXELTKFECSUPERVISOR_FULL_VERSION_LIST PIXELTKFECSUPERVISOR_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELTKFECSUPERVISOR_VERSION_MAJOR, PIXELTKFECSUPERVISOR_VERSION_MINOR, PIXELTKFECSUPERVISOR_VERSION_PATCH)
#endif

namespace PixelTKFECSupervisor {
  
  const std::string package = "PixelTKFECSupervisor";
  const std::string versions = PIXELTKFECSUPERVISOR_FULL_VERSION_LIST;
  const std::string summary = "PixelTKFECSupervisor";
  const std::string description = "PixelTKFECSupervisor XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
