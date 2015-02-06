// $Id: version.h,v 1.2 2007/09/26 11:04:22 veelken Exp $

/*************************************************************************
 * Auxiliary header to define XDAQ internal versioning information       *
 * about PixelDCSSupervisor package in the shared object library         *
 * dynamically loaded by XDAQ at run-time                                *
 *                                                                       *
 * Author: Christian Veelken, UC Davis			 	         *
 *                                                                       *
 * Last update: $Date: 2007/09/26 11:04:22 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#ifndef _PixelDCSSupervisor_version_h_
#define _PixelDCSSupervisor_version_h_

#include "config/PackageInfo.h"

namespace PixelDCSSupervisor
{
  const std::string package  =  "PixelDCSSupervisor";
  const std::string versions =  "0.3.0";
  const std::string description = "Run-Control Interface between XDAQ and DCS for CMS Pixel Detector";
  const std::string summary = "Run-Control Interface between XDAQ and DCS for CMS Pixel Detecto";
  const std::string authors = "Christian Veelken";
  const std::string link = "";

  config::PackageInfo getPackageInfo();
  void checkPackageDependencies() throw (config::PackageInfo::VersionException);
  std::set<std::string, std::less<std::string> > getPackageDependencies();
}

#endif
