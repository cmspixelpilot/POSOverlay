#ifndef _PixelMode_h_
#define _PixelMode_h_

#include <map>
#include <vector>
#include <assert.h>

class PixelMode
{

	public:
	PixelMode() {totalEntries_  = 0; counts_.clear(); modeUpToDate_ = false;}

	void addEntry(int newEntry);
	
	int mode() { if (!modeUpToDate_) findMode(); return mode_;}
	unsigned int modeNumEntries() { if (!modeUpToDate_) findMode(); return modeNumEntries_;}
	unsigned int numValuesInMode() { if (!modeUpToDate_) findMode(); return numValuesInMode_;}
	unsigned int nextMostEntries(); // second-highest number of entries -- if there's a tie for mode, returns number of entries for each value in the tie

	unsigned int totalEntries() const {return totalEntries_;}

	// returns value and number of entries for the top two values.  If there's a tie for second, all values in the tie are returned (so the vector will have more than 2 elements.)  If all entries are a single value, the vector will have just 1 element.
	std::vector<std::pair< int, unsigned int > > topTwoValues() const;

	void clear() {counts_.clear();}

	private:
	void findMode();

	std::map< int, unsigned int > counts_; // entry value, number of times it appears
	unsigned int totalEntries_;
	
	bool modeUpToDate_;
	// values below filled by findMode()
	int mode_;
	unsigned int modeNumEntries_;
	unsigned int numValuesInMode_; // keep track of ties for mode_double
	
};

#endif
