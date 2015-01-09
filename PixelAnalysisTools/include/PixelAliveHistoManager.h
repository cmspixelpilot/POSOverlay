#ifndef _PixelAliveHistoManager_h_
#define _PixelAliveHistoManager_h_

#include "PixelAnalysisTools/include/PixelHistoManager.h"
#include <string>

class PixelXmlReader;
class PixelCalibConfigurationExtended;
class PixelConfigurationsManager;

class PixelAliveHistoManager : public PixelHistoManager{
 public:
  PixelAliveHistoManager   (PixelXmlReader* xmlReader, PixelCalibConfigurationExtended *calib, PixelConfigurationsManager * configurationsManager, std::ostream *logger=&std::cout);
  ~PixelAliveHistoManager  (void);
  void init       		  	 (void);
  void bookHistos 		  	 (void);
  void destroy    		  	 (void);
	void drawHisto           (unsigned int fed, unsigned int channel, unsigned int roc, std::string summary, std::string panelType, int plaquette, TH1* &summaryH);  
	void makeSummaryPlots 	 (void);
  void setPatternHistos 	 (unsigned int eventNumber){;}
  void fillHistos          (unsigned int fed,unsigned int channel,unsigned int roc,unsigned int row,unsigned int col);
 private:
	int    maxBadChannelsPerRoc_;
};

#endif
