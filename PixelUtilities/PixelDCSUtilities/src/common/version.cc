// $Id: version.cc,v 1.2 2007/10/05 09:03:10 veelken Exp $

/*************************************************************************
 * Auxiliary header to define XDAQ internal versioning information       *
 * about PixelUtilities/PixelDCSUtilities package                        *
 * in the shared object library dynamically loaded by XDAQ at run-time   *
 *                                                                       *
 * Author: Christian Veelken, UC Davis			 	         *
 *                                                                       *
 * Last update: $Date: 2007/10/05 09:03:10 $ (UTC)                       *
 *          by: $Author: veelken $                                       *
 *************************************************************************/

#include "version.h"
#include "config/version.h"

GETPACKAGEINFO(PixelUtilities_PixelDCSUtilities)

std::set<std::string, std::less<std::string> > PixelUtilities_PixelDCSUtilities::getPackageDependencies()
{
  std::set<std::string, std::less<std::string> > dependencies;
  
  ADDDEPENDENCY(dependencies, config); 
  
  return dependencies;
}	
	
