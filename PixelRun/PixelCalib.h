#ifndef PixelCalib_h
#define PixelCalib_h
//
// This class inplement the steps
// that are used in a scan over
// Threshold and CalDelay
//
//
//
//

#include <vector>
#include <set>
#include <map>
#include <string>
#include <utility>
#include "PixelCalibBase.h"
#include "PixelROCName.h"
#include "PixelModuleName.h"
#include "PixelConfigBase.h"
#include "PixelDACScanRange.h"
#include <fstream>

class PixelHdwAddress;

using namespace std;

class PixelCalib : public PixelCalibBase, public PixelConfigBase {

 public:

    PixelCalib(string filename, string translationname = "");

    //void nextFECState(PixelFECConfigInterface* pixelFEC,
    //		      PixelDetectorConfig* detconfig,
    //		      PixelNameTranslation* trans, 
    //	      unsigned int state) const; 

    //return vector of fed# and channels controlled by this fed supervisor
    //vector<pair<unsigned int,vector<unsigned int> > >& fedCardsAndChannels(unsigned int crate, PixelNameTranslation* translation, PixelFEDConfig* fedconfig) const;
    //map <unsigned int, set<unsigned int> > getFEDsAndChannels (PixelNameTranslation *translation);
    
    // Returns a set of FED crates that are used by this Calib object
    //set <unsigned int> getFEDCrates(PixelNameTranslation *translation, PixelFEDConfig *fedconfig) const;
    // Returns a set of FEC crates that are used by this Calib object
    //set <unsigned int> getFECCrates(PixelNameTranslation *translation, PixelFECConfig* fecconfig) const;

    unsigned int nROC() const { return nROC_; }
    unsigned int nPixelPatterns() const { return rows_.size()*cols_.size(); }
    unsigned int nTriggersPerPattern() const { return ntrigger_; }
    unsigned int nScanPoints(unsigned int iscan) const { return (dacs_[iscan].last()-dacs_[iscan].first())/dacs_[iscan].step()+1; }    
    unsigned int nScanPoints(string dac) const { return nScanPoints(iScan(dac)); }    

    unsigned int nScanPoints() const {unsigned int points=1;
                for(unsigned int i=0;i<dacs_.size();i++) {
		  points*=nScanPoints(i);
		}
		return points;
    }
    unsigned int nConfigurations() const {return nPixelPatterns()*nScanPoints()*nROC();}
    unsigned int nTriggersTotal() const {return nConfigurations()*nTriggersPerPattern();}

    unsigned int scanValue(unsigned int iscan, unsigned int state) const;
    unsigned int scanValue(string dac, unsigned int state) const{
      return scanValue(iScan(dac),state);
    }

    unsigned int scanCounter(unsigned int iscan, unsigned int state) const;
    unsigned int scanCounter(string dac, unsigned int state) const{
      return scanCounter(iScan(dac),state);
    }

    double scanValueMin(unsigned int iscan) const {return dacs_[iscan].first();}
    double scanValueMin(string dac) const {return scanValueMin(iScan(dac));}
    double scanValueMax(unsigned int iscan) const {return dacs_[iscan].first()+
						 dacs_[iscan].step()*(nScanPoints(iscan)-1);}
    double scanValueMax(string dac) const {return scanValueMax(iScan(dac));}
    double scanValueStep(unsigned int iscan) const {return dacs_[iscan].step();}
    double scanValueStep(string dac) const {return scanValueStep(iScan(dac));}

    unsigned int iScan(string dac) const;

    const vector<PixelROCName>& rocList() const {return rocs_;}
    const set <PixelModuleName>& moduleList(){return modules_;}

    virtual string mode() {return mode_;}

    friend ostream& operator<<(ostream& s, const PixelCalib& calib);

    void getRowsAndCols(unsigned int state,
			const vector<unsigned int> *&rows,
			const vector<unsigned int> *&cols) const;

    vector<vector<unsigned int> > getAllRows() {return rows_;}
    vector<vector<unsigned int> > getAllCols() {return cols_;}

    vector<pair<vector<int>,string> > getPanelTypes() ;

    //get fed and channel given disk, blade and panel numbers
    pair<unsigned int, unsigned int> getFedAndChan(unsigned int disk,unsigned int blade, unsigned int panel);

    //get the map of panel name to fed and channel
    map<string, pair<unsigned int, unsigned int> > getAllFedAndChans();

    //get disk blade panel numbers given the fed and the channel
    vector<int> getDiskBladePanel(unsigned int fed,unsigned int channel);

    //get the map of fed and channel to disk balde, panel numbers and panel type
    map<pair<unsigned int,unsigned int>,pair<vector<int>,string> > getMapFEDChaToPanType();

    //get the list of panel types associated to the disk, blade, panel numbers and the fed and the channel (according to translation.dat)
    vector<pair<vector<int>,string> > getPanelTypesAndFEDInfo() ;  

    //this method looks into the translation.dat file for the plaquette and roc number (in the plaquette) 
    //given the disk, blade, panel and roc number (in the panel)
    pair<unsigned int, unsigned int> getPlaquetteAndRoc(unsigned int disk,unsigned int blade,unsigned int panel,unsigned int roc);

    //use PixelCalib::getPlaquetteAndRoc to fill a map with all plaquette and roc numbers
    map<unsigned int,map<unsigned int,map<unsigned int,map<unsigned int,pair<unsigned int, unsigned int> > > > > getPlaquetteAndRocMap();

    unsigned int getNumberOfFeds() {
      if (theFeds.size()==0){
	cout << "numeber of feds=0... you should call PixelCalib::getPanelTypesAndFEDInfo() before..."<<endl;
	exit(1);
      }
      return theFeds.size();
    }

 private:


    //Mode is one of the following: 
    //  ThresholdCalDelay
    //  FEDChannelOffsetDAC
    //  FEDAddressLevelDAC
    //  FEDChannelOffsetPixel
    //  FEDAddressLevelPixel
    //  GainCalibration
    //  PixelAlive
    //  SCurve
    //  ClockPhaseCalibration

    string mode_;

    bool singleROC_;

    vector<vector<unsigned int> > rows_;
    vector<vector<unsigned int> > cols_;

    mutable vector<PixelROCName> rocs_;
    set <PixelModuleName> modules_;
    map <PixelModuleName,unsigned int> countROC_;
    bool roclistfromconfig_;

    mutable vector<pair<unsigned int, vector<unsigned int> > > fedCardsAndChannels_;

    //unsigned int vcal_;

    vector<PixelDACScanRange> dacs_;

    //vector<string> dacname_;
    //vector<unsigned int> dacchannel_;
    //vector<unsigned int> dac_first_;
    //vector<unsigned int> dac_last_;
    //vector<unsigned int> dac_step_;

    unsigned int ntrigger_;
    unsigned int nROC_; //This is the maximal #ROCs on a given TBM

    bool highVCalRange_;

    /*
    void enablePixels(PixelFECConfigInterface* pixelFEC,
		      unsigned int irows, unsigned int icols,
		      PixelHdwAddress theROC) const;
    void disablePixels(PixelFECConfigInterface* pixelFEC,
		      unsigned int irows, unsigned int icols,
		      PixelHdwAddress theROC) const;
    */
    mutable vector<int> old_irows;
    mutable vector<int> old_icols;

    ifstream tr;

    vector<unsigned int> theFeds;
    map<string, pair<unsigned int, unsigned int> > theFedAndChansMap;
    vector<pair<vector<int>,string> > fedInfo;
    map<pair<unsigned int,unsigned int>,pair<vector<int>,string> > mapFEDChaToPanType;
    map<unsigned int,map<unsigned int,map<unsigned int,map<unsigned int,pair<unsigned int, unsigned int> > > > > plaquetteAndRocMap;
};

#endif
