// $Id: version.cc,v 1.2 2007/09/26 11:04:22 veelken Exp $

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

#include "version.h"
#include "config/version.h"

GETPACKAGEINFO(PixelDCSSupervisor)

void PixelDCSSupervisor::checkPackageDependencies() throw (config::PackageInfo::VersionException)
{
  CHECKDEPENDENCY(config);   
}

std::set<std::string, std::less<std::string> > PixelDCSSupervisor::getPackageDependencies()
{
  std::set<std::string, std::less<std::string> > dependencies;
  
  ADDDEPENDENCY(dependencies, config); 
  
  return dependencies;
}	
	
