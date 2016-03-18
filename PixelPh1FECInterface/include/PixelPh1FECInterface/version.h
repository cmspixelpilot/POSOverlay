#ifndef _PixelPh1FECInterface_version_h_
#define _PixelPh1FECInterface_version_h_

#include "config/PackageInfo.h"

#define PIXELPH1FECINTERFACE_VERSION_MAJOR 1
#define PIXELPH1FECINTERFACE_VERSION_MINOR 0
#define PIXELPH1FECINTERFACE_VERSION_PATCH 0
#undef PIXELPH1FECINTERFACE_PREVIOUS_VERSIONS
#define PIXELPH1FECINTERFACE_VERSION_CODE PACKAGE_VERSION_CODE(PIXELPH1FECINTERFACE_VERSION_MAJOR, PIXELPH1FECINTERFACE_VERSION_MINOR, PIXELPH1FECINTERFACE_VERSION_PATCH)

#ifndef PIXELPH1FECINTERFACE_PREVIOUS_VERSIONS
#define PIXELPH1FECINTERFACE_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELPH1FECINTERFACE_VERSION_MAJOR, PIXELPH1FECINTERFACE_VERSION_MINOR, PIXELPH1FECINTERFACE_VERSION_PATCH)
#else
#define PIXELPH1FECINTERFACE_FULL_VERSION_LIST PIXELPH1FECINTERFACE_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELPH1FECINTERFACE_VERSION_MAJOR, PIXELPH1FECINTERFACE_VERSION_MINOR, PIXELPH1FECINTERFACE_VERSION_PATCH)
#endif

namespace PixelPh1FECInterface {
  
  const std::string package = "PixelPh1FECInterface";
  const std::string versions = PIXELPH1FECINTERFACE_FULL_VERSION_LIST;
  const std::string summary = "PixelPh1FECInterface";
  const std::string description = "PixelPh1FECInterface XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
