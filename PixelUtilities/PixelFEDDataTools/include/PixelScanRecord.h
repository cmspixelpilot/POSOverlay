#ifndef _PixelScanRecord_h_
#define _PixelScanRecord_h_

#include <map>
#include "PixelUtilities/PixelFEDDataTools/include/Moments.h"
#include "TGraphErrors.h"
#include "TPaveText.h"

class PixelScanRecord
{

	public:
	PixelScanRecord() {scanData_.clear(); xVarName_="x"; yVarName_="y"; title_="Scan Record"; resetFoundThings();}

	void addEntry(int x, double y) {scanData_[x].push_back(y); resetFoundThings();}
	void addEntries(int x, Moments y) {scanData_[x].push_back(y); resetFoundThings();}

	unsigned int totalEntries();
	unsigned int numScanPoints() const {return scanData_.size();}

	Moments getPoint(int x) const;
	void setPoint(int x, Moments y) {scanData_[x] = y; resetFoundThings();}
	void setPoint(int x, double y) { Moments newPoint; newPoint.push_back(y); setPoint(x, newPoint); }

	int minX() const {return scanData_.begin()->first;}
	int maxX() const {return scanData_.rbegin()->first;}

	const Moments& minY() const;
	const Moments& maxY() const;
	int xAtMaxY() const;
	std::pair<double,double> interpolatedHighestPoint(); // returns (x,y) of highest point taken from a quadratic fit of the three points nearest the highest point

	// Uncertainty-weighted average of all scan points.
	double uncertaintyWeightedAverage() const;

	// True if the scan data appears not to vary over the different scan values.
	bool constant(double chisqOverDOFLimit = 100.) const;

	// x-value at which the y-value crossed the specified target
	// If kUnique, fails if there are no crossings or multiple crossings
	// If kLast,   fails if there are no crossings, but takes the last (highest x-value) one if there are more than one.
	// If kFirst,  fails if there are no crossings, but takes the first (lowest x-value) one if there are more than one.
	enum CrossingType {kUnique = 0, kLast = 1, kFirst = 2};
	bool crossingPointFound(double newCrossingThreshold, CrossingType crossingType = kUnique);
	int crossingPoint() const; // Check crossingPointFound() first, and use it to set parameters.
	double crossingPointFullPrecision() const; // returns crossing point without rounding to the nearest integer.

	// for an error-function-shaped scan, x-value at which the y-value saturates
	// Slope at saturation point must be less than satarationThreshold*(max slope)
	// If takeNegative == true, multiply all slopes by -1. when finding the saturation point.  Use this to find a "bottoming-out" point.
	bool saturationPointFound(double newSaturationThreshold = 0.20, bool takeNegative = false);
	int saturationPoint() const; // Check saturationPointFound() first, and use it to set parameters.

	void clear() {scanData_.clear(); resetFoundThings();}
	
	void printPlot(std::string filename = "rootFile", int color = kBlue, std::vector<TPaveText> extraTextBoxes = std::vector<TPaveText>()) const;
	TGraphErrors makePlot(int color = kBlue) const;
	
	void printScanToStream(std::ostream& out = std::cout, double offset = 0., std::string offsetName = "") const;
	
	void setXVarName(std::string aName) {xVarName_ = aName;}
	std::string xVarName() const {return xVarName_;}
	void setYVarName(std::string aName) {yVarName_ = aName;}
	std::string yVarName() const {return yVarName_;}
	void setTitle(std::string aTitle) {title_ = aTitle;}
	std::string title() const {return title_;}

	// Produces scan with each element equal to the difference between the two input scans.  The uncertainties can't be trusted, though.
	const PixelScanRecord operator-(const PixelScanRecord& another) const;

	private:
	void resetFoundThings() { highestPointFound_ = false; crossingPointFound_ = false; saturationPointFound_ = false; checkForNewCrossingPoint_ = true; checkForNewSaturationPoint_ = true; }
	
	void findCrossingPoint();
	void findSaturationPoint();
	
	// Find slope and uncertainty.  If takeNegative == true, return -1*slope.
	std::pair< double, double > slope( int x1, int x2, bool takeNegative ) const;
	//         slope,  uncertainty
	// Find largest slope that's sustained over at least numPoints scan points (numPoints >= 1)
	// returns: (end point of range with largest slope, value of largest slope)
	std::pair< int, double> findLargestSlope(unsigned int numPoints, bool takeNegative) const;
	
	std::map< int, Moments > scanData_; // x-value, Moments for values recorded at that value
	
	std::string xVarName_;
	std::string yVarName_;
	std::string title_;
	
	std::pair<double,double> highestPoint_;
	bool highestPointFound_;
	
	double crossingYTarget_;
	CrossingType crossingType_;
	double crossingPoint_;
	bool crossingPointFound_;
	bool checkForNewCrossingPoint_;
	
	double saturationThreshold_;
	bool takeNegative_;
	int saturationPoint_;
	bool saturationPointFound_;
	bool checkForNewSaturationPoint_;
};

#endif
