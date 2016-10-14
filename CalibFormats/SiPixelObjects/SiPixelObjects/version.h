#ifndef _SiPixelObjects_version_h_
#define _SiPixelObjects_version_h_

#include "config/PackageInfo.h"

#define SIPIXELOBJECTS_VERSION_MAJOR 1
#define SIPIXELOBJECTS_VERSION_MINOR 0
#define SIPIXELOBJECTS_VERSION_PATCH 0
#undef SIPIXELOBJECTS_PREVIOUS_VERSIONS
#define SIPIXELOBJECTS_VERSION_CODE PACKAGE_VERSION_CODE(SIPIXELOBJECTS_VERSION_MAJOR, SIPIXELOBJECTS_VERSION_MINOR, SIPIXELOBJECTS_VERSION_PATCH)

#ifndef SIPIXELOBJECTS_PREVIOUS_VERSIONS
#define SIPIXELOBJECTS_FULL_VERSION_LIST PACKAGE_VERSION_STRING(SIPIXELOBJECTS_VERSION_MAJOR, SIPIXELOBJECTS_VERSION_MINOR, SIPIXELOBJECTS_VERSION_PATCH)
#else
#define SIPIXELOBJECTS_FULL_VERSION_LIST SIPIXELOBJECTS_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(SIPIXELOBJECTS_VERSION_MAJOR, SIPIXELOBJECTS_VERSION_MINOR, SIPIXELOBJECTS_VERSION_PATCH)
#endif

namespace SiPixelObjects {
  
  const std::string package = "SiPixelObjects";
  const std::string versions = SIPIXELOBJECTS_FULL_VERSION_LIST;
  const std::string summary = "SiPixelObjects";
  const std::string description = "SiPixelObjects XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
