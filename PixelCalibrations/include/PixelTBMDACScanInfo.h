#ifndef _PixelTBMDACScanInfo_h_
#define _PixelTBMDACScanInfo_h_

#include "CalibFormats/SiPixelObjects/interface/PixelCalibConfiguration.h"

class PixelTBMDACScanInfo {

	public:
	
	enum TBMDAC {kAnalogInputBias, kAnalogOutputBias, kAnalogOutputGain, kNumDACs};
	
	PixelTBMDACScanInfo( const pos::PixelCalibConfiguration& config );
	
	const std::set<TBMDAC>& DACsToScan() const {return DACsToScan_;}
	
	unsigned int scanNumSteps() const { return scanNumSteps_; }
	
	template <class Num>
	Num scanValue( TBMDAC dac, Num step ) const
	{
		assert( step < (Num)(scanNumSteps()) );
		return scanMin(dac) + step*(scanMax(dac)-scanMin(dac))/(scanNumSteps()-1);
	}
	
	std::string name( TBMDAC dac ) const;
	std::string shortName( TBMDAC dac ) const;
	
	private:
	unsigned int scanMin( TBMDAC dac ) const;
	unsigned int scanMax( TBMDAC dac ) const;
	
	std::set<TBMDAC> DACsToScan_;
	
	std::map< TBMDAC, unsigned int > scanMin_;
	std::map< TBMDAC, unsigned int > scanMax_;
	
	unsigned int scanNumSteps_;

};

#endif
