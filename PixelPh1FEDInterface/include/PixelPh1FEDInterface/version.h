#ifndef _PixelPh1FEDInterface_version_h_
#define _PixelPh1FEDInterface_version_h_

#include "config/PackageInfo.h"

#define PIXELPH1FEDINTERFACE_VERSION_MAJOR 1
#define PIXELPH1FEDINTERFACE_VERSION_MINOR 0
#define PIXELPH1FEDINTERFACE_VERSION_PATCH 0
#undef PIXELPH1FEDINTERFACE_PREVIOUS_VERSIONS
#define PIXELPH1FEDINTERFACE_VERSION_CODE PACKAGE_VERSION_CODE(PIXELPH1FEDINTERFACE_VERSION_MAJOR, PIXELPH1FEDINTERFACE_VERSION_MINOR, PIXELPH1FEDINTERFACE_VERSION_PATCH)

#ifndef PIXELPH1FEDINTERFACE_PREVIOUS_VERSIONS
#define PIXELPH1FEDINTERFACE_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELPH1FEDINTERFACE_VERSION_MAJOR, PIXELPH1FEDINTERFACE_VERSION_MINOR, PIXELPH1FEDINTERFACE_VERSION_PATCH)
#else
#define PIXELPH1FEDINTERFACE_FULL_VERSION_LIST PIXELPH1FEDINTERFACE_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELPH1FEDINTERFACE_VERSION_MAJOR, PIXELPH1FEDINTERFACE_VERSION_MINOR, PIXELPH1FEDINTERFACE_VERSION_PATCH)
#endif

namespace PixelPH1FEDInterface {
  
  const std::string package = "PixelPh1FEDInterface";
  const std::string versions = PIXELPH1FEDINTERFACE_FULL_VERSION_LIST;
  const std::string summary = "PixelPh1FEDInterface";
  const std::string description = "PixelPh1FEDInterface XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
