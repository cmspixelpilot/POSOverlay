#ifndef _PixelTestStandUtilities_version_h_
#define _PixelTestStandUtilities_version_h_

#include "config/PackageInfo.h"

#define PIXELTESTSTANDUTILITIES_VERSION_MAJOR 1
#define PIXELTESTSTANDUTILITIES_VERSION_MINOR 0
#define PIXELTESTSTANDUTILITIES_VERSION_PATCH 0
#undef PIXELTESTSTANDUTILITIES_PREVIOUS_VERSIONS
#define PIXELTESTSTANDUTILITIES_VERSION_CODE PACKAGE_VERSION_CODE(PIXELTESTSTANDUTILITIES_VERSION_MAJOR, PIXELTESTSTANDUTILITIES_VERSION_MINOR, PIXELTESTSTANDUTILITIES_VERSION_PATCH)

#ifndef PIXELTESTSTANDUTILITIES_PREVIOUS_VERSIONS
#define PIXELTESTSTANDUTILITIES_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELTESTSTANDUTILITIES_VERSION_MAJOR, PIXELTESTSTANDUTILITIES_VERSION_MINOR, PIXELTESTSTANDUTILITIES_VERSION_PATCH)
#else
#define PIXELTESTSTANDUTILITIES_FULL_VERSION_LIST PIXELTESTSTANDUTILITIES_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELTESTSTANDUTILITIES_VERSION_MAJOR, PIXELTESTSTANDUTILITIES_VERSION_MINOR, PIXELTESTSTANDUTILITIES_VERSION_PATCH)
#endif

namespace PixelTestStandUtilities {
  
  const std::string package = "PixelTestStandUtilities";
  const std::string versions = PIXELTESTSTANDUTILITIES_FULL_VERSION_LIST;
  const std::string summary = "PixelTestStandUtilities";
  const std::string description = "PixelTestStandUtilities XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
