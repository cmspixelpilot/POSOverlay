#include "PixelUtilities/PixelFEDDataTools/include/PixelMode.h"

void PixelMode::addEntry(int newEntry)
{
	modeUpToDate_ = false;
	
	std::map< int, unsigned int >::const_iterator match = counts_.find(newEntry);
	if ( match == counts_.end() )
	{
		counts_[newEntry] = 1;
	}
	else
	{
		(counts_[newEntry])++;
	}
	totalEntries_++;
	return;
}

void PixelMode::findMode()
{
	assert( counts_.size() > 0 );
	double mode_double = 0;
	unsigned int modeNumEntries = 0;
	unsigned int numValuesInMode = 0; // keep track of ties for mode_double
	for ( std::map< int, unsigned int >::const_iterator counts_itr = counts_.begin(); counts_itr != counts_.end(); ++counts_itr )
	{
		if ( counts_itr->second > modeNumEntries )
		{
			mode_double = counts_itr->first;
			modeNumEntries = counts_itr->second;
			numValuesInMode = 1;
		}
		else if ( counts_itr->second == modeNumEntries )
		{
			mode_double = (mode_double*((double)numValuesInMode) + ((double)counts_itr->first))/(((double)numValuesInMode) + 1.); // average with previous entries that tied it
			numValuesInMode++;
		}
	}
	mode_ = (int)(mode_double+0.5); // round to the nearest integer
	modeNumEntries_ = modeNumEntries;
	numValuesInMode_ = numValuesInMode;
	modeUpToDate_ = true;
	return;
}

unsigned int PixelMode::nextMostEntries()
{
	if (!modeUpToDate_) findMode();
	assert( modeUpToDate_ );
	assert( numValuesInMode() > 0 );
	if ( numValuesInMode() > 1 )
	{
		return modeNumEntries_;
	}
	else
	{
		unsigned int nextMostEntries = 0;
		for ( std::map< int, unsigned int >::const_iterator counts_itr = counts_.begin(); counts_itr != counts_.end(); ++counts_itr )
		{
			if ( counts_itr->second == modeNumEntries_ ) continue; // skip the mode
			if ( counts_itr->second > nextMostEntries ) nextMostEntries = counts_itr->second;
		}
		return nextMostEntries;
	}
}

std::vector<std::pair< int, unsigned int > > PixelMode::topTwoValues() const
{
	std::vector<std::pair< int, unsigned int > > topValue;
	std::vector<std::pair< int, unsigned int > > secondValue;
	for ( std::map< int, unsigned int >::const_iterator counts_itr = counts_.begin(); counts_itr != counts_.end(); ++counts_itr )
	{
		if ( counts_itr == counts_.begin() )
		{
			topValue.push_back( *counts_itr );
			continue;
		}
		
		if ( counts_itr->second > topValue.begin()->second ) // new top value
		{
			secondValue = topValue; // old top value becomes second value
			topValue.clear(); topValue.push_back( *counts_itr ); // replace top value
		}
		else if ( counts_itr->second == topValue.begin()->second ) // tied with top value
		{
			topValue.push_back( *counts_itr ); // add this entry to topValue
		}
		else if ( secondValue.size() == 0 ) // no second value previously recorded
		{
			secondValue.push_back( *counts_itr ); // make this the second value
		}
		else if ( counts_itr->second > secondValue.begin()->second ) // new second value
		{
			secondValue.clear(); secondValue.push_back( *counts_itr ); // replace second value
		}
		else if ( counts_itr->second == secondValue.begin()->second ) // tied with second value
		{
			secondValue.push_back( *counts_itr ); // add this entry to secondValue
		}
	}
	
	assert( topValue.size() > 0 || counts_.size() == 0 );
	assert( !(topValue.size() == 0 && secondValue.size() > 0) );
	std::vector<std::pair< int, unsigned int > > toReturn = topValue;
	if ( topValue.size() == 1 )
	{
		for ( std::vector<std::pair< int, unsigned int > >::const_iterator secondValue_itr = secondValue.begin(); secondValue_itr != secondValue.end(); ++secondValue_itr )
		{
			toReturn.push_back(*secondValue_itr);
		}
	}
	assert( toReturn.size() == topValue.size() || toReturn.size() == secondValue.size() + 1 );
	return toReturn;
}
