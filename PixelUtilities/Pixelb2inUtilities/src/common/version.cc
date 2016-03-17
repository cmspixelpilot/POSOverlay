#include "config/version.h"
#include "xcept/version.h"
#include "xdaq/version.h"
#include "Pixelb2inUtilities/version.h"

GETPACKAGEINFO(Pixelb2inUtilities)

void Pixelb2inUtilities::checkPackageDependencies() throw (config::PackageInfo::VersionException) {

  CHECKDEPENDENCY(config);
  CHECKDEPENDENCY(xcept);
  CHECKDEPENDENCY(xdaq);
  
}

std::set<std::string, std::less<std::string> > Pixelb2inUtilities::getPackageDependencies() {
  
  std::set<std::string, std::less<std::string> > dependencies;
  ADDDEPENDENCY(dependencies,config);
  ADDDEPENDENCY(dependencies,xcept);
  ADDDEPENDENCY(dependencies,xdaq);
  return dependencies;
  
}
