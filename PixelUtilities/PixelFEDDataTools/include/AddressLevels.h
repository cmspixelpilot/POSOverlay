#ifndef _AddressLevels_h_
#define _AddressLevels_h_

#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include <vector>
#include <assert.h>

#define MAX_ADC 1024

class AddressLevels
{
 public:
  AddressLevels();
  unsigned int getPoint(unsigned int pos);
  void setPoint(unsigned int pos, unsigned int value);
  void addPoint(unsigned int a);
  unsigned int findPeaks(unsigned int minimumPeakCount = 50);
  unsigned int nPeaks() {return peaks_.size();}
  const std::vector<Moments>& returnPeaks() {return peaks_;}
  const Moments& returnPeak(unsigned int number) {return peaks_.at(number);}
  bool validAddressLevels() {return validAddressLevels_;}
  unsigned int addressLevel(unsigned int level) {assert(validAddressLevels_ && (level<5)); return L_[level];}
  // functions that work on peak positions 
  double rms(unsigned int n);
  double maxrms(); //return maximum rms of all peaks
  double separation(unsigned int n); //return separation between adjacent peaks (n) and (n+1) in units of peak rms
  double minseparation(); //return minimum separation or -1 if less than two peaks
  // functions that deal with the histogram & arrays
  void dump(std::string fname);
  void printHistogram();
  void drawHistogram(std::string fileName="");
  AddressLevels operator+(AddressLevels a);
  void operator+=(AddressLevels a);

 private:
  unsigned int histogram_[MAX_ADC];
  std::vector<Moments> peaks_;
  unsigned int L_[5];
  bool validAddressLevels_;
	       
};

#endif
