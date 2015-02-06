#include "toolboxV.h"
#include "xoap/version.h"
#include "xdaqV.h"
#include "PixelTTCSupervisorV.hh"

GETPACKAGEINFO(PixelTTCSupervisor)

void PixelTTCSupervisor::checkPackageDependencies() throw( toolbox::PackageInfo::VersionException) {
  CHECKDEPENDENCY(toolbox)
  CHECKDEPENDENCY(xoap)
  CHECKDEPENDENCY(xdaq)
}

set<string, less<string> > PixelTTCSupervisor::getPackageDependencies() {
  set< string, less<string> > dependencies;
  ADDDEPENDENCY( dependencies, toolbox );
  ADDDEPENDENCY( dependencies, xoap );
  ADDDEPENDENCY( dependencies, xdaq );
  return dependencies;
}
