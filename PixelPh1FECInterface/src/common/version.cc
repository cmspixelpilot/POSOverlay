#include "config/version.h"
#include "xcept/version.h"
#include "xdaq/version.h"
#include "PixelPh1FECInterface/version.h"

GETPACKAGEINFO(PixelPh1FECInterface)

void PixelPh1FECInterface::checkPackageDependencies() throw (config::PackageInfo::VersionException) {

  CHECKDEPENDENCY(config);
  CHECKDEPENDENCY(xcept);
  CHECKDEPENDENCY(xdaq);
  
}

std::set<std::string, std::less<std::string> > PixelPh1FECInterface::getPackageDependencies() {
  
  std::set<std::string, std::less<std::string> > dependencies;
  ADDDEPENDENCY(dependencies,config);
  ADDDEPENDENCY(dependencies,xcept);
  ADDDEPENDENCY(dependencies,xdaq);
  return dependencies;
  
}
