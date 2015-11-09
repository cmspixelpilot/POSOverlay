#ifndef PixelDACScanRange_h
#define PixelDACScanRange_h
/*! \file CalibFormats/SiPixelObjects/interface/PixelConfigurationVerifier.h
*   \brief This class collects the information about the range of DAC settings used in scans of the DACs
*
*   A longer explanation will be placed here later
*/
//
// This class collects the information
// about the range of DAC settings used
// in scans of the DACs.
//
//
//

#include <string>
#include <vector>
#include <cassert>

namespace pos{
/*! \class PixelConfigurationVerifier PixelConfigurationVerifier.h "interface/PixelConfigurationVerifier.h"
*   \brief This class collects the information about the range of DAC settings used in scans of the DACs
*
*   A longer explanation will be placed here later
*/
  class PixelDACScanRange {

  public:

    PixelDACScanRange(){;}
    PixelDACScanRange(std::string dacname, unsigned int first, 
		      unsigned int last, int step,
		      unsigned int index, bool mixValuesAcrossROCs);
    PixelDACScanRange(std::string name, 
		      std::vector<unsigned int> values,
		      unsigned int index, bool mixValuesAcrossROCs);

    std::string name() const { return name_;}
    unsigned int dacchannel() const { return dacchannel_; }
    int step() const { assert(uniformSteps_); return step_; }
    unsigned int first() const { assert(uniformSteps_); return first_; }
    unsigned int last() const { assert(uniformSteps_); return last_; }
    unsigned int index() const { return index_; }
    unsigned int getNPoints() const { return values_.size(); }
    unsigned int value(unsigned int ivalue) const { assert(ivalue<values_.size()); return values_[ivalue]; }
    bool mixValuesAcrossROCs() const { return mixValuesAcrossROCs_; }
    bool uniformSteps() const { return uniformSteps_; }
    void setRelative() { relative_=true; }
    bool relative() const { return relative_; }
    void setNegative() { negative_=true; }
    bool negative() const { return negative_; }

    bool isTBM() const { return isTBM_; }
    unsigned int tbmchannel() const { return tbmchannel_; }

    std::vector<unsigned int> values() const { return values_; }

    void dump(bool dump_values=true) const;

  private:

    void setDACChannel(std::string name);


    std::string name_;
    unsigned int dacchannel_;
    bool uniformSteps_;
    unsigned int first_;
    unsigned int last_;
    int step_;
    std::vector<unsigned int> values_;
    unsigned int index_;

    bool mixValuesAcrossROCs_; // whether to spread the DAC values across the entire range on each iteration for different ROCs on a channel

    bool relative_; //Only to be used with 'SetRelative' and indicates that the
                    //value specified should apply a change to the default
                    //dac setting.

    bool negative_; //Used to flag that relative value is negative

    bool isTBM_; //Flag that this is a TBM DAC setting
    unsigned int tbmchannel_; // 14 or 15
  };
}
#endif
