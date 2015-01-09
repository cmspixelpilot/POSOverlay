#ifndef _PixelAnalyzer_h_
#define _PixelAnalyzer_h_

#include <iostream>
#include <string>
#include <vector>
#include "PixelUtilities/PixelFEDDataTools/include/SLinkDecoder.h"

class PixelHistoManager;
class PixelConfigurationsManager;
class PixelCalibConfigurationExtended;

class PixelAnalyzer{
 public:
  PixelAnalyzer();
  PixelAnalyzer(PixelHistoManager *pixelHistoManager,
                PixelCalibConfigurationExtended *pixelCalib,
                PixelConfigurationsManager *pixelConfigurationsManager,
		std::ostream *logger=&std::cout);
  ~PixelAnalyzer();
  void loopOverDataFile(int nEventsToProcess=-1);
  unsigned int findNumberOfFeds(std::string fileName);
 private:
  bool                        readEvent(pos::SLinkDecoder &sLinkDecoder, std::vector<pos::PixelSLinkEvent> &sLinkEvents);
  PixelHistoManager               *thePixelHistoManager_;
  PixelConfigurationsManager      *thePixelConfigurationsManager_;
  PixelCalibConfigurationExtended *thePixelCalib_;
  std::ostream                    *logger_;
};

#endif
