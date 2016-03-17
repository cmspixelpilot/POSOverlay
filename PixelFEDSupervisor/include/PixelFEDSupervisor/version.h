#ifndef _PixelFEDSupervisor_version_h_
#define _PixelFEDSupervisor_version_h_

#include "config/PackageInfo.h"

#define PIXELFEDSUPERVISOR_VERSION_MAJOR 1
#define PIXELFEDSUPERVISOR_VERSION_MINOR 0
#define PIXELFEDSUPERVISOR_VERSION_PATCH 0
#undef PIXELFEDSUPERVISOR_PREVIOUS_VERSIONS
#define PIXELFEDSUPERVISOR_VERSION_CODE PACKAGE_VERSION_CODE(PIXELFEDSUPERVISOR_VERSION_MAJOR, PIXELFEDSUPERVISOR_VERSION_MINOR, PIXELFEDSUPERVISOR_VERSION_PATCH)

#ifndef PIXELFEDSUPERVISOR_PREVIOUS_VERSIONS
#define PIXELFEDSUPERVISOR_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELFEDSUPERVISOR_VERSION_MAJOR, PIXELFEDSUPERVISOR_VERSION_MINOR, PIXELFEDSUPERVISOR_VERSION_PATCH)
#else
#define PIXELFEDSUPERVISOR_FULL_VERSION_LIST PIXELFEDSUPERVISOR_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELFEDSUPERVISOR_VERSION_MAJOR, PIXELFEDSUPERVISOR_VERSION_MINOR, PIXELFEDSUPERVISOR_VERSION_PATCH)
#endif

namespace PixelFEDSupervisor {
  
  const std::string package = "PixelFEDSupervisor";
  const std::string versions = PIXELFEDSUPERVISOR_FULL_VERSION_LIST;
  const std::string summary = "PixelFEDSupervisor";
  const std::string description = "PixelFEDSupervisor XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
