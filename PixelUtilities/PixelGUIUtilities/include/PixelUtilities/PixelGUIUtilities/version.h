#ifndef _PixelGUIUtilities_version_h_
#define _PixelGUIUtilities_version_h_

#include "config/PackageInfo.h"

#define PIXELGUIUTILITIES_VERSION_MAJOR 1
#define PIXELGUIUTILITIES_VERSION_MINOR 0
#define PIXELGUIUTILITIES_VERSION_PATCH 0
#undef PIXELGUIUTILITIES_PREVIOUS_VERSIONS
#define PIXELGUIUTILITIES_VERSION_CODE PACKAGE_VERSION_CODE(PIXELGUIUTILITIES_VERSION_MAJOR, PIXELGUIUTILITIES_VERSION_MINOR, PIXELGUIUTILITIES_VERSION_PATCH)

#ifndef PIXELGUIUTILITIES_PREVIOUS_VERSIONS
#define PIXELGUIUTILITIES_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELGUIUTILITIES_VERSION_MAJOR, PIXELGUIUTILITIES_VERSION_MINOR, PIXELGUIUTILITIES_VERSION_PATCH)
#else
#define PIXELGUIUTILITIES_FULL_VERSION_LIST PIXELGUIUTILITIES_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELGUIUTILITIES_VERSION_MAJOR, PIXELGUIUTILITIES_VERSION_MINOR, PIXELGUIUTILITIES_VERSION_PATCH)
#endif

namespace PixelGUIUtilities {
  
  const std::string package = "PixelGUIUtilities";
  const std::string versions = PIXELGUIUTILITIES_FULL_VERSION_LIST;
  const std::string summary = "PixelGUIUtilities";
  const std::string description = "PixelGUIUtilities XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
