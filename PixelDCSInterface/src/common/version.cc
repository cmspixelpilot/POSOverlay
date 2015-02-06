// $Id: version.cc,v 1.3 2007/09/26 13:31:32 veelken Exp $

/*************************************************************************
 * Auxiliary header to define XDAQ internal versioning information       *
 * about PixelDCSInterface package in the shared object library         *
 * dynamically loaded by XDAQ at run-time                                *
 *                                                                       *
 * Author: Christian Veelken, UC Davis			 	         *
 *                                                                       *
 * Last update: $Date: 2007/09/26 13:31:32 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#include "version.h"
#include "config/version.h"

GETPACKAGEINFO(PixelDCSInterface)

void PixelDCSInterface::checkPackageDependencies() throw (config::PackageInfo::VersionException)
{
  CHECKDEPENDENCY(config);   
}

std::set<std::string, std::less<std::string> > PixelDCSInterface::getPackageDependencies()
{
  std::set<std::string, std::less<std::string> > dependencies;
  
  ADDDEPENDENCY(dependencies, config); 
  
  return dependencies;
}	
	
