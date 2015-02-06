// $Id: version.h,v 1.3 2007/09/26 13:31:32 veelken Exp $

/*************************************************************************
 * Auxiliary header to define XDAQ internal versioning information       *
 * about PixelDCSInterface package in the shared object library          *
 * dynamically loaded by XDAQ at run-time                                *
 *                                                                       *
 * Author: Christian Veelken, UC Davis			 	         *
 *                                                                       *
 * Last update: $Date: 2007/09/26 13:31:32 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#ifndef _PixelDCSInterface_version_h_
#define _PixelDCSInterface_version_h_

#include "config/PackageInfo.h"

namespace PixelDCSInterface
{
  const std::string package  =  "PixelDCSInterface";
  const std::string versions =  "0.3.0";
  const std::string description = "Routines for interfacing PVSS data-points and FSM states for CMS Pixel Detector";
  const std::string summary = "Routines for interfacing PVSS data-points and FSM states for CMS Pixel Detector";
  const std::string authors = "Christian Veelken, Carlos Andres Florez";
  const std::string link = "";

  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
}

#endif
