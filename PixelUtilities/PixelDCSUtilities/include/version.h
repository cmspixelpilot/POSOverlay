// $Id: version.h,v 1.3 2007/10/05 09:03:09 veelken Exp $

/*************************************************************************
 * Auxiliary header to define XDAQ internal versioning information       *
 * about PixelUtilities/PixelDCSUtilities package                        *
 * in the shared object library dynamically loaded by XDAQ at run-time   *
 *                                                                       *
 * Author: Christian Veelken, UC Davis			 	         *
 *                                                                       *
 * Last update: $Date: 2007/10/05 09:03:09 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#ifndef _PixelUtilities_PixelDCSUtilities_version_h_
#define _PixelUtilities_PixelDCSUtilities_version_h_

#include "config/PackageInfo.h"

namespace PixelUtilities_PixelDCSUtilities
{
  const std::string package  =  "PixelUtilities_PixelDCSUtilities";
  const std::string versions =  "0.2.0";
  const std::string description = "auxiliary Classes for Run-Control Interface between XDAQ and DCS for CMS Pixel Detector";
  const std::string summary = "auxiliary Classes for Run-Control Interface between XDAQ and DCS for CMS Pixel Detector";
  const std::string authors = "Christian Veelken";
  const std::string link = "";

  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
}

#endif
