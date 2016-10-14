#include "config/version.h"
#include "xcept/version.h"
#include "xdaq/version.h"
#include "PixelTKFECSupervisor/version.h"

GETPACKAGEINFO(PixelTKFECSupervisor)

void PixelTKFECSupervisor::checkPackageDependencies() throw (config::PackageInfo::VersionException) {

  CHECKDEPENDENCY(config);
  CHECKDEPENDENCY(xcept);
  CHECKDEPENDENCY(xdaq);
  
}

std::set<std::string, std::less<std::string> > PixelTKFECSupervisor::getPackageDependencies() {
  
  std::set<std::string, std::less<std::string> > dependencies;
  ADDDEPENDENCY(dependencies,config);
  ADDDEPENDENCY(dependencies,xcept);
  ADDDEPENDENCY(dependencies,xdaq);
  return dependencies;
  
}
