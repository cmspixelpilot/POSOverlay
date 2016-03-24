#ifndef _PixelHistoViewer_version_h_
#define _PixelHistoViewer_version_h_

#include "config/PackageInfo.h"

#define PIXELHISTOVIEWER_VERSION_MAJOR 1
#define PIXELHISTOVIEWER_VERSION_MINOR 0
#define PIXELHISTOVIEWER_VERSION_PATCH 0
#undef PIXELHISTOVIEWER_PREVIOUS_VERSIONS
#define PIXELHISTOVIEWER_VERSION_CODE PACKAGE_VERSION_CODE(PIXELHISTOVIEWER_VERSION_MAJOR, PIXELHISTOVIEWER_VERSION_MINOR, PIXELHISTOVIEWER_VERSION_PATCH)

#ifndef PIXELHISTOVIEWER_PREVIOUS_VERSIONS
#define PIXELHISTOVIEWER_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELHISTOVIEWER_VERSION_MAJOR, PIXELHISTOVIEWER_VERSION_MINOR, PIXELHISTOVIEWER_VERSION_PATCH)
#else
#define PIXELHISTOVIEWER_FULL_VERSION_LIST PIXELHISTOVIEWER_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELHISTOVIEWER_VERSION_MAJOR, PIXELHISTOVIEWER_VERSION_MINOR, PIXELHISTOVIEWER_VERSION_PATCH)
#endif

namespace PixelHistoViewer {
  
  const std::string package = "PixelHistoViewer";
  const std::string versions = PIXELHISTOVIEWER_FULL_VERSION_LIST;
  const std::string summary = "PixelHistoViewer";
  const std::string description = "PixelHistoViewer XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
