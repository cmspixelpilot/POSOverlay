#include "config/version.h"
#include "xcept/version.h"
#include "xdaq/version.h"
#include "PixelSupervisorConfiguration/version.h"

GETPACKAGEINFO(PixelSupervisorConfiguration)

void PixelSupervisorConfiguration::checkPackageDependencies() throw (config::PackageInfo::VersionException) {

  CHECKDEPENDENCY(config);
  CHECKDEPENDENCY(xcept);
  CHECKDEPENDENCY(xdaq);
  
}

std::set<std::string, std::less<std::string> > PixelSupervisorConfiguration::getPackageDependencies() {
  
  std::set<std::string, std::less<std::string> > dependencies;
  ADDDEPENDENCY(dependencies,config);
  ADDDEPENDENCY(dependencies,xcept);
  ADDDEPENDENCY(dependencies,xdaq);
  return dependencies;
  
}
