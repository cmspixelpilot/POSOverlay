#ifndef _PixelAnalysisTools_version_h_
#define _PixelAnalysisTools_version_h_

#include "config/PackageInfo.h"

#define PIXELANALYSISTOOLS_VERSION_MAJOR 1
#define PIXELANALYSISTOOLS_VERSION_MINOR 0
#define PIXELANALYSISTOOLS_VERSION_PATCH 0
#undef PIXELANALYSISTOOLS_PREVIOUS_VERSIONS
#define PIXELANALYSISTOOLS_VERSION_CODE PACKAGE_VERSION_CODE(PIXELANALYSISTOOLS_VERSION_MAJOR, PIXELANALYSISTOOLS_VERSION_MINOR, PIXELANALYSISTOOLS_VERSION_PATCH)

#ifndef PIXELANALYSISTOOLS_PREVIOUS_VERSIONS
#define PIXELANALYSISTOOLS_FULL_VERSION_LIST PACKAGE_VERSION_STRING(PIXELANALYSISTOOLS_VERSION_MAJOR, PIXELANALYSISTOOLS_VERSION_MINOR, PIXELANALYSISTOOLS_VERSION_PATCH)
#else
#define PIXELANALYSISTOOLS_FULL_VERSION_LIST PIXELANALYSISTOOLS_PREVIOUS_VERSIONS "," PACKAGE_VERSION_STRING(PIXELANALYSISTOOLS_VERSION_MAJOR, PIXELANALYSISTOOLS_VERSION_MINOR, PIXELANALYSISTOOLS_VERSION_PATCH)
#endif

namespace PixelAnalysisTools {
  
  const std::string package = "PixelAnalysisTools";
  const std::string versions = PIXELANALYSISTOOLS_FULL_VERSION_LIST;
  const std::string summary = "PixelAnalysisTools";
  const std::string description = "PixelAnalysisTools XDAQ application";
  const std::string authors = "POS team";
  const std::string link = "http://xdaq.web.cern.ch";
  
  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
  
}

#endif
