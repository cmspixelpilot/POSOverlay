#ifndef __PixelTTCSupervisorV
#define __PixelTTCSupervisorV

#include "PackageInfo.h"

namespace PixelTTCSupervisor {
  const string package     = "PixelTTCSupervisor";
  const string versions    = "version 0-0";
  const string description = "TTCci Control as a XDAQ application";
  const string link        = "";
  toolbox::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (toolbox::PackageInfo::VersionException);
  set<string, less<string> > getPackageDependencies();
}

#endif /* __PixelTTCSupervisorV */
