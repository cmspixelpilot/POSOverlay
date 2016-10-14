#ifndef _PixelAMC13Controller_version_h_
#define _PixelAMC13Controller_version_h_

#include "config/PackageInfo.h"

#define PIXELAMC13CONTROLLER_VERSION_MAJOR 1
#define PIXELAMC13CONTROLLER_VERSION_MINOR 0
#define PIXELAMC13CONTROLLER_VERSION_PATCH 0
#undef PIXELAMC13CONTROLLER_PREVIOUS_VERSIONS
#define PIXELAMC13CONTROLLER_VERSION_CODE PACKAGE_VERSION_CODE(PIXELAMC13CONTROLLER_VERSION_MAJOR, PIXELAMC13CONTROLLER_VERSION_MINOR, PIXELAMC13CONTROLLER_VERSION_PATCH)

#ifndef PIXELAMC13CONTROLLER_PREVIOUS_VERSIONS
#define PIXELAMC13CONTROLLER_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELAMC13CONTROLLER_VERSION_MAJOR, PIXELAMC13CONTROLLER_VERSION_MINOR, PIXELAMC13CONTROLLER_VERSION_PATCH)
#else
#define PIXELAMC13CONTROLLER_FULL_VERSION_LIST PIXELAMC13CONTROLLER_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELAMC13CONTROLLER_VERSION_MAJOR, PIXELAMC13CONTROLLER_VERSION_MINOR, PIXELAMC13CONTROLLER_VERSION_PATCH)
#endif

namespace PixelAMC13Controller {
  
  const std::string package = "PixelAMC13Controller";
  const std::string versions = PIXELAMC13CONTROLLER_FULL_VERSION_LIST;
  const std::string summary = "PixelAMC13Controller";
  const std::string description = "PixelAMC13Controller XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
