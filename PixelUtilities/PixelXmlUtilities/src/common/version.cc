#include "config/version.h"
#include "xcept/version.h"
#include "xdaq/version.h"
#include "PixelXmlUtilities/version.h"

GETPACKAGEINFO(PixelXmlUtilities)

void PixelXmlUtilities::checkPackageDependencies() throw (config::PackageInfo::VersionException) {

  CHECKDEPENDENCY(config);
  CHECKDEPENDENCY(xcept);
  CHECKDEPENDENCY(xdaq);
  
}

std::set<std::string, std::less<std::string> > PixelXmlUtilities::getPackageDependencies() {
  
  std::set<std::string, std::less<std::string> > dependencies;
  ADDDEPENDENCY(dependencies,config);
  ADDDEPENDENCY(dependencies,xcept);
  ADDDEPENDENCY(dependencies,xdaq);
  return dependencies;
  
}
