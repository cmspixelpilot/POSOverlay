#include "config/version.h"
#include "xcept/version.h"
#include "xdaq/version.h"
#include "PixelFEDSupervisor/version.h"

GETPACKAGEINFO(PixelFEDSupervisor)

void PixelFEDSupervisor::checkPackageDependencies() throw (config::PackageInfo::VersionException) {

  CHECKDEPENDENCY(config);
  CHECKDEPENDENCY(xcept);
  CHECKDEPENDENCY(xdaq);
  
}

std::set<std::string, std::less<std::string> > PixelFEDSupervisor::getPackageDependencies() {
  
  std::set<std::string, std::less<std::string> > dependencies;
  ADDDEPENDENCY(dependencies,config);
  ADDDEPENDENCY(dependencies,xcept);
  ADDDEPENDENCY(dependencies,xdaq);
  return dependencies;
  
}
