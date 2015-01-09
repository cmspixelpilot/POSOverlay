#include "PixelCalibrations/include/PixelTBMDACScanInfo.h"

PixelTBMDACScanInfo::PixelTBMDACScanInfo( const pos::PixelCalibConfiguration& config )
{
	scanNumSteps_ = 0; // If it's still 0 when we check at the end, then there's a problem with the configuration.
	
	std::set<TBMDAC> allDACs;
	allDACs.insert( kAnalogInputBias );
	allDACs.insert( kAnalogOutputBias );
	allDACs.insert( kAnalogOutputGain );
	
	// Set up the fixed DAC values.
	for ( std::set<TBMDAC>::const_iterator allDACs_itr = allDACs.begin(); allDACs_itr != allDACs.end(); allDACs_itr++  )
	{
		TBMDAC dac = *allDACs_itr;
		std::string setValue = config.parameterValue("Set"+name(dac));
		if ( setValue != "" )
		{
			DACsToScan_.insert( dac );
			scanMin_[dac] = atoi( setValue.c_str() );
			scanMax_[dac] = atoi( setValue.c_str() );
		}
	}
	
	// Set up scanning just one DAC.
	for ( std::set<TBMDAC>::const_iterator allDACs_itr = allDACs.begin(); allDACs_itr != allDACs.end(); allDACs_itr++  )
	{
		TBMDAC dac = *allDACs_itr;
		if ( config.parameterValue( "DACToScan" ) == name(dac) )
		{
			DACsToScan_.insert( dac );
			
			std::string scanMinString = config.parameterValue("ScanMin");
			if ( scanMinString != "" ) scanMin_[dac] = atoi( scanMinString.c_str() );
			else                       scanMin_[dac] = 0;
			
			std::string scanMaxString = config.parameterValue("ScanMax");
			if ( scanMaxString != "" ) scanMax_[dac] = atoi( scanMaxString.c_str() );
			else                       scanMax_[dac] = 0;
			
			std::string scanNumStepsString = config.parameterValue("ScanNumSteps");
			if ( scanNumStepsString == "" )
			{
				std::string scanStepSizeString = config.parameterValue("ScanStepSize");
				if ( scanStepSizeString != "" ) scanStepSizeString = "5";
				scanNumSteps_ = (scanMax_[dac]-scanMin_[dac])/atoi(scanStepSizeString.c_str()) + 1;
			}
			else
			{
				scanNumSteps_ = atoi( scanNumStepsString.c_str() );
			}
			
		}
	}
	
	if ( config.parameterValue( "DACToScan" ) == "All" ) // Set up scanning all DACs proportionally.
	{
		DACsToScan_.insert( kAnalogInputBias );
		
		std::string MinAnalogInputBiasString = config.parameterValue("MinAnalogInputBias");
		if ( MinAnalogInputBiasString != "" ) scanMin_[kAnalogInputBias] = atoi( MinAnalogInputBiasString.c_str() );
		else                                  scanMin_[kAnalogInputBias] = 0;
		
		std::string MaxAnalogInputBiasString = config.parameterValue("MaxAnalogInputBias");
		if ( MaxAnalogInputBiasString != "" ) scanMax_[kAnalogInputBias] = atoi( MaxAnalogInputBiasString.c_str() );
		else                                  scanMax_[kAnalogInputBias] = 255;
		
		DACsToScan_.insert( kAnalogOutputBias );
		
		std::string MinAnalogOutputBiasString = config.parameterValue("MinAnalogOutputBias");
		if ( MinAnalogOutputBiasString != "" ) scanMin_[kAnalogOutputBias] = atoi( MinAnalogOutputBiasString.c_str() );
		else                                  scanMin_[kAnalogOutputBias] = 0;
		
		std::string MaxAnalogOutputBiasString = config.parameterValue("MaxAnalogOutputBias");
		if ( MaxAnalogOutputBiasString != "" ) scanMax_[kAnalogOutputBias] = atoi( MaxAnalogOutputBiasString.c_str() );
		else                                  scanMax_[kAnalogOutputBias] = 128;
		
		DACsToScan_.insert( kAnalogOutputGain );
		
		std::string MinAnalogOutputGainString = config.parameterValue("MinAnalogOutputGain");
		if ( MinAnalogOutputGainString != "" ) scanMin_[kAnalogOutputGain] = atoi( MinAnalogOutputGainString.c_str() );
		else                                  scanMin_[kAnalogOutputGain] = 0;
		
		std::string MaxAnalogOutputGainString = config.parameterValue("MaxAnalogOutputGain");
		if ( MaxAnalogOutputGainString != "" ) scanMax_[kAnalogOutputGain] = atoi( MaxAnalogOutputGainString.c_str() );
		else                                  scanMax_[kAnalogOutputGain] = 255;
		
		std::string scanNumStepsString = config.parameterValue("ScanNumSteps");
		if ( scanNumStepsString != "" ) scanNumSteps_ = atoi( scanNumStepsString.c_str() );
		else                            scanNumSteps_ = 52;
	}
	
	if ( scanNumSteps_ == 0 )
	{
		std::cout << "[PixelTBMDACScanInfo] ERROR: Parameters did not specify one or more DACs to scan over.  Please give a suitable value for parameter DACToScan.  Now aborting..." << std::endl;
		assert(0);
	}
}

unsigned int PixelTBMDACScanInfo::scanMin( TBMDAC dac ) const
{
	std::map< TBMDAC, unsigned int >::const_iterator foundDAC = scanMin_.find(dac);
	assert( foundDAC != scanMin_.end() );
	return foundDAC->second;
}

unsigned int PixelTBMDACScanInfo::scanMax( TBMDAC dac ) const
{
	std::map< TBMDAC, unsigned int >::const_iterator foundDAC = scanMax_.find(dac);
	assert( foundDAC != scanMax_.end() );
	return foundDAC->second;
}

std::string PixelTBMDACScanInfo::name( TBMDAC dac ) const
{
	if ( dac == kAnalogInputBias  ) return "AnalogInputBias";
	if ( dac == kAnalogOutputBias ) return "AnalogOutputBias";
	if ( dac == kAnalogOutputGain ) return "AnalogOutputGain";
	
	assert(0);
	return "";
}

std::string PixelTBMDACScanInfo::shortName( TBMDAC dac ) const
{
	if ( dac == kAnalogInputBias  ) return "AInBias";
	if ( dac == kAnalogOutputBias ) return "AOutBias";
	if ( dac == kAnalogOutputGain ) return "AOutGain";
	
	assert(0);
	return "";
}
