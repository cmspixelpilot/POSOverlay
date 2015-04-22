#include "PixelTCDSSupervisor/version.h"

#include "config/version.h"
#include "toolbox/version.h"
#include "xcept/version.h"
#include "xdaq/version.h"
#include "xdata/version.h"
#include "xgi/version.h"
#include "xoap/version.h"
#include "xoap/filter/version.h"

GETPACKAGEINFO(pixel::tcds)

void
pixel::tcds::checkPackageDependencies()
{
  CHECKDEPENDENCY(config);
  CHECKDEPENDENCY(toolbox);
  CHECKDEPENDENCY(xcept);
  CHECKDEPENDENCY(xdaq);
  CHECKDEPENDENCY(xdata);
  CHECKDEPENDENCY(xgi);
  CHECKDEPENDENCY(xoap);
  CHECKDEPENDENCY(xoapfilter);
}

std::set<std::string, std::less<std::string> >
pixel::tcds::getPackageDependencies()
{
  std::set<std::string, std::less<std::string> > dependencies;

  ADDDEPENDENCY(dependencies, config);
  ADDDEPENDENCY(dependencies, toolbox);
  ADDDEPENDENCY(dependencies, xcept);
  ADDDEPENDENCY(dependencies, xdaq);
  ADDDEPENDENCY(dependencies, xdata);
  ADDDEPENDENCY(dependencies, xgi);
  ADDDEPENDENCY(dependencies, xoap);
  ADDDEPENDENCY(dependencies, xoapfilter);

  return dependencies;
}
