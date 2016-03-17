#ifndef _PixelXmlUtilities_version_h_
#define _PixelXmlUtilities_version_h_

#include "config/PackageInfo.h"

#define PIXELXMLUTILITIES_VERSION_MAJOR 1
#define PIXELXMLUTILITIES_VERSION_MINOR 0
#define PIXELXMLUTILITIES_VERSION_PATCH 0
#undef PIXELXMLUTILITIES_PREVIOUS_VERSIONS
#define PIXELXMLUTILITIES_VERSION_CODE PACKAGE_VERSION_CODE(PIXELXMLUTILITIES_VERSION_MAJOR, PIXELXMLUTILITIES_VERSION_MINOR, PIXELXMLUTILITIES_VERSION_PATCH)

#ifndef PIXELXMLUTILITIES_PREVIOUS_VERSIONS
#define PIXELXMLUTILITIES_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELXMLUTILITIES_VERSION_MAJOR, PIXELXMLUTILITIES_VERSION_MINOR, PIXELXMLUTILITIES_VERSION_PATCH)
#else
#define PIXELXMLUTILITIES_FULL_VERSION_LIST PIXELXMLUTILITIES_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELXMLUTILITIES_VERSION_MAJOR, PIXELXMLUTILITIES_VERSION_MINOR, PIXELXMLUTILITIES_VERSION_PATCH)
#endif

namespace PixelXmlUtilities {
  
  const std::string package = "PixelXmlUtilities";
  const std::string versions = PIXELXMLUTILITIES_FULL_VERSION_LIST;
  const std::string summary = "PixelXmlUtilities";
  const std::string description = "PixelXmlUtilities XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
