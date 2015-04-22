#include "PixelUtilities/PixelFEDDataTools/include/PixelScanRecord.h"

#include <iomanip>
#include <algorithm>

#include <toolbox/convertstring.h>

#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TAxis.h"
#include "TPaveText.h"
#include "TLine.h"

unsigned int PixelScanRecord::totalEntries()
{
	unsigned int sum = 0;
	for (std::map< int, Moments >::iterator scanData_itr = scanData_.begin(); scanData_itr != scanData_.end(); ++scanData_itr)
	{
		sum += scanData_itr->second.count();
	}
	return sum;
}

const Moments& PixelScanRecord::minY() const
{
	double minY = 0.;
	int xAtMinY = 0;
	for (std::map< int, Moments >::const_iterator scanData_itr = scanData_.begin(); scanData_itr != scanData_.end(); ++scanData_itr)
	{
		if ( scanData_itr == scanData_.begin() || scanData_itr->second.mean() < minY )
		{
			minY = scanData_itr->second.mean();
			xAtMinY = scanData_itr->first;
		}
	}
	assert( scanData_.find(xAtMinY) != scanData_.end() );
	return scanData_.find(xAtMinY)->second;
}

const Moments& PixelScanRecord::maxY() const
{
	std::map< int, Moments >::const_iterator highestPoint = scanData_.find(xAtMaxY());
	assert( highestPoint != scanData_.end() );
	return highestPoint->second;
}

int PixelScanRecord::xAtMaxY() const
{
	std::map< int, Moments >::const_iterator highestPoint;
	for (std::map< int, Moments >::const_iterator scanData_itr = scanData_.begin(); scanData_itr != scanData_.end(); ++scanData_itr)
	{
		if ( scanData_itr == scanData_.begin() || scanData_itr->second.mean() > highestPoint->second.mean() )
		{
			highestPoint = scanData_itr;
		}
	}
	assert( highestPoint != scanData_.end() );
	return highestPoint->first;
}

std::pair<double,double> PixelScanRecord::interpolatedHighestPoint()
{
	if ( !highestPointFound_ )
	{
		std::pair< int, Moments > highestPoint;
		highestPoint.first = xAtMaxY();
		highestPoint.second = maxY();
		
		// Special case -- highest point is at an endpoint
		if ( highestPoint.first == minX() || highestPoint.first == maxX() )
		{
			highestPointFound_ = true;
			highestPoint_ = std::pair<double,double>(highestPoint.first, highestPoint.second.mean());
			return highestPoint_;
		}
		
		std::map< int, Moments >::const_iterator highestPoint_itr = scanData_.find(highestPoint.first);
		std::map< int, Moments >::const_iterator leftPoint_itr = highestPoint_itr; leftPoint_itr--;
		std::map< int, Moments >::const_iterator rightPoint_itr = highestPoint_itr; rightPoint_itr++;
		
		double x1 = leftPoint_itr->first;    double y1 = leftPoint_itr->second.mean();
		double x2 = highestPoint_itr->first; double y2 = highestPoint_itr->second.mean();
		double x3 = rightPoint_itr->first;   double y3 = rightPoint_itr->second.mean();
		
		// Find the parabola, y = ax^2 + bx +c, that passes through these three points.
		double factor1 = y1/((x1-x2)*(x1-x3));
		double factor2 = y2/((x2-x1)*(x2-x3));
		double factor3 = y3/((x3-x1)*(x3-x2));
		
		double a = factor1 + factor2 + factor3; assert( a <= 0. );
		double b = -(x2+x3)*factor1 -(x1+x3)*factor2 -(x1+x2)*factor3;
		double c = x2*x3*factor1 + x1*x3*factor2 + x1*x2*factor3;
		
		// Return the highest point on the parabola.
		highestPoint_ = std::pair<double,double>( -b/(2.*a), c-(b*b)/(4.*a) );
		highestPointFound_ = true;
	}
	return highestPoint_;
}

Moments PixelScanRecord::getPoint(int x) const
{
	std::map< int, Moments >::const_iterator foundPoint = scanData_.find(x);
	if ( foundPoint != scanData_.end() ) return foundPoint->second;
	Moments empty; return empty;
}

double PixelScanRecord::uncertaintyWeightedAverage() const
{
	double sumWeights = 0., sumWeightTimesValue = 0.;
	for ( std::map< int, Moments >::const_iterator scanData_itr = scanData_.begin(); scanData_itr != scanData_.end(); scanData_itr++ )
	{
		double weight = 1./pow(std::max(1e-4, scanData_itr->second.uncertainty()),2); // If uncertainty is less than 1e-4, it is set equal to that value.
		sumWeights += weight;
		sumWeightTimesValue += weight*(scanData_itr->second.mean());
	}
	return sumWeightTimesValue/sumWeights;
}

bool PixelScanRecord::constant(double chisqOverDOFLimit) const
{
	double average = uncertaintyWeightedAverage();
	
	// Calculate chi-squared.  Maybe the treatment of the average isn't quite correct.
	double chisq = 0.;
	for ( std::map< int, Moments >::const_iterator scanData_itr = scanData_.begin(); scanData_itr != scanData_.end(); scanData_itr++ )
	{
		chisq += pow( (scanData_itr->second.mean() - average)/(std::max(1e-4, scanData_itr->second.uncertainty())) ,2);
	}
	double chisqOverDOF = chisq/((double)(scanData_.size()-1));
	
	if ( chisqOverDOF < chisqOverDOFLimit ) return true;
	else                                    return false;
}

void PixelScanRecord::printScanToStream(std::ostream& out, double offset, std::string offsetName) const
{
	if ( offsetName=="" ) assert( offset==0. );
	
	out << xVarName_ << " : ";
	if ( offsetName!="" ) out << "(";
	out << yVarName_;
	if ( offsetName!="" ) out << " - " << offsetName << ")";
	unsigned int leftColumnLength = xVarName_.size();
	unsigned int middleColumnLength = yVarName_.size();
	if ( offsetName!="" ) middleColumnLength += offsetName.size() + 5;
	while ( middleColumnLength < 7+2+5 )
	{
		out << " ";
		middleColumnLength++;
	}
	out << " : # of readings" << std::endl;
	
	double prev_yValue = -1.;
	bool firstPoint = true;
	for ( std::map<int, Moments>::const_iterator scanData_itr = scanData_.begin(); scanData_itr != scanData_.end(); scanData_itr++ )
	{
		out << std::setw(leftColumnLength)<<std::right << scanData_itr->first << " : " << std::showpos << std::setprecision(4) << std::setw(7) << std::right << scanData_itr->second.mean() - offset << std::noshowpos << "+-" << std::setprecision(2) << std::setw(middleColumnLength-7-2) << std::left <<scanData_itr->second.uncertainty() << std::right << std::setprecision(6) << " : "<<scanData_itr->second.count();
		if ( !firstPoint && scanData_itr->second.squares() >= 0. && scanData_itr->second.uncertainty()/(scanData_itr->second.mean()-prev_yValue) > 0.25 )
		{
			out << " <- LARGE UNCERTAINTY, POINT IGNORED";
		}
		out << std::endl;
		prev_yValue = scanData_itr->second.mean();
		firstPoint = false;
	}
}

int PixelScanRecord::crossingPoint() const
{
	assert( !checkForNewCrossingPoint_ );
	assert( crossingPointFound_ );
	return int(crossingPoint_+0.5);
}

double PixelScanRecord::crossingPointFullPrecision() const
{
	assert( !checkForNewCrossingPoint_ );
	assert( crossingPointFound_ );
	return crossingPoint_;
}

bool PixelScanRecord::crossingPointFound(double newCrossingYTarget, CrossingType crossingType)
{
	if ( crossingYTarget_ != newCrossingYTarget || crossingType_ != crossingType )
	{
		crossingYTarget_ = newCrossingYTarget;
		crossingType_ = crossingType;
		checkForNewCrossingPoint_ = true;
	}
	if ( checkForNewCrossingPoint_ )
	{
		findCrossingPoint();
		checkForNewCrossingPoint_ = false;
	}
	return crossingPointFound_;
}

void PixelScanRecord::findCrossingPoint()
{
	const double yTarget = crossingYTarget_;
	int prev_xValue=0;
	double intersection_xValue=0;
	double prev_yValue=0;
	bool firstPoint = true;
	bool foundCrossing = false;
	bool failed = true; // change this only upon success
	// Loop over scan points.
	for ( std::map<int, Moments>::const_iterator scanData_itr = scanData_.begin(); scanData_itr != scanData_.end(); scanData_itr++ )
	{
		// Check whether the uncertainty on the UB level is small enough.
		if ( !firstPoint && scanData_itr->second.squares() >= 0. && scanData_itr->second.count() > 1 && scanData_itr->second.uncertainty()/fabs(scanData_itr->second.mean()-prev_yValue) > 0.25 )
		{
			continue;
		}

		// See if the y value crossed the target.
		int this_xValue = scanData_itr->first; assert( firstPoint || this_xValue > prev_xValue );
		double this_yValue = scanData_itr->second.mean();
		if ( !firstPoint && ( (prev_yValue < yTarget && yTarget <= this_yValue)||(this_yValue <= yTarget && yTarget < prev_yValue) ) ) // skip this on the first iteration, or if the y value didn't cross the target
		{
			if ( !foundCrossing || crossingType_ == kLast ) // ok if we found no crossing previously, or if we just want the last one
			{
				failed = false;
				foundCrossing = true;
				// Interpolate between these xValue values, and round to the nearest integer.
				intersection_xValue = (((double)this_xValue)*(yTarget-prev_yValue) + ((double)prev_xValue)*(this_yValue-yTarget))/(this_yValue-prev_yValue);
				if ( crossingType_ == kFirst ) break; // Don't look for any more crossings.
			}
			else // multiple target crossings detected!
			{
				//std::cout << "ERROR: multiple crossings of target.  No unique intersection found.\n";
				failed = true;
			}
		}
		prev_xValue = this_xValue;
		prev_yValue = this_yValue;
		firstPoint = false;
	} // end of loop over scan points
	if ( !foundCrossing ) // no crossings detected!
	{
		//std::cout << "ERROR: no crossing of target.  No intersection found.\n";
		failed = true;
	}
	if ( !failed )
	{
		crossingPoint_ = intersection_xValue;
	}
	else
	{
		crossingPoint_ = 0;
	}
	crossingPointFound_ = !failed;
}

int PixelScanRecord::saturationPoint() const
{
	assert( !checkForNewSaturationPoint_ );
	assert( saturationPointFound_ );
	return saturationPoint_;
}

bool PixelScanRecord::saturationPointFound(double newSaturationThreshold, bool takeNegative)
{
	if ( saturationThreshold_ != newSaturationThreshold || takeNegative_ != takeNegative )
	{
		saturationThreshold_ = newSaturationThreshold;
		takeNegative_ = takeNegative;
		checkForNewSaturationPoint_ = true;
	}
	if ( checkForNewSaturationPoint_ )
	{
		findSaturationPoint();
		checkForNewSaturationPoint_ = false;
	}
	return saturationPointFound_;
}

void PixelScanRecord::findSaturationPoint()
{
	assert( saturationThreshold_ > 0. );
	
	const unsigned int numPoints = 3;
	int largestSlopeEndPoint = findLargestSlope(numPoints, takeNegative_).first;
	
	std::map< int, Moments >::iterator largestSlopeEnd_itr = scanData_.find(largestSlopeEndPoint);
	assert( largestSlopeEnd_itr != scanData_.end() );
	std::map< int, Moments >::iterator largestSlopeBegin_itr = largestSlopeEnd_itr;
	for (unsigned int i = 0; i < numPoints; i++) largestSlopeBegin_itr--;
	std::pair< double, double > largestSlope_pair = slope(largestSlopeBegin_itr->first, largestSlopeEnd_itr->first, takeNegative_);
	double largestSlope = largestSlope_pair.first;
	double largestSlopeUncertainty = largestSlope_pair.second;
	
	std::map< int, Moments >::iterator thisSlopeBegin_itr = scanData_.find(largestSlopeEndPoint);
	std::map< int, Moments >::iterator thisSlopeEnd_itr = scanData_.find(largestSlopeEndPoint);
	for (unsigned int i = 0; i < numPoints; i++) thisSlopeEnd_itr++;
	for ( ; thisSlopeEnd_itr != scanData_.end(); ++thisSlopeBegin_itr, ++thisSlopeEnd_itr)
	{
		// If the slope of the first two points in this range is more than half as big as the largest slope, skip to the next point.
		assert( saturationThreshold_ < 0.5 );
		std::map< int, Moments >::iterator thisSlopeBeginPlusOne_itr = thisSlopeBegin_itr; thisSlopeBeginPlusOne_itr++;
		if ( slope( thisSlopeBegin_itr->first, thisSlopeBeginPlusOne_itr->first, takeNegative_ ).first > 0.5*largestSlope ) continue;
		
		std::pair< double, double > thisSlope_pair = slope(thisSlopeBegin_itr->first, thisSlopeEnd_itr->first, takeNegative_);
		double thisSlope = thisSlope_pair.first;
		double thisSlopeUncertainty = thisSlope_pair.second;
		
		// If (this slope)/(largest slope) is at least 5 sigma below the saturation threshold, then we've found the saturation point
		if ( thisSlope/largestSlope + 5.*(thisSlope/largestSlope)*sqrt( pow(thisSlopeUncertainty/thisSlope,2) + pow(largestSlopeUncertainty/largestSlope,2) ) < saturationThreshold_ )
		{
			saturationPoint_ = thisSlopeBegin_itr->first;
			saturationPointFound_ = true;
			return;
		}
	}
	// never found a good saturation point, set equal to the last point of the scan
	for (unsigned int i = 0; i < numPoints-1; i++) thisSlopeBegin_itr++;
	int last = thisSlopeBegin_itr->first;
	saturationPoint_ = last;
	saturationPointFound_ = false;
	return;
}

// This function returns the largest slope that isn't followed by a substantial drop at higher x-values.
std::pair< int, double> PixelScanRecord::findLargestSlope(unsigned int numPoints, bool takeNegative) const
{
	assert( numPoints <= scanData_.size() && numPoints != 0 );
	std::map< int, Moments >::const_iterator rangeBegin_itr = scanData_.begin();
	std::map< int, Moments >::const_iterator rangeEnd_itr = scanData_.begin();
	for (unsigned int i = 0; i < numPoints; i++) rangeEnd_itr++;
	int maxSustainedSlopeEndPoint = 0; double maxSustainedSlope = 0; bool firstRange = true;
	for (; rangeEnd_itr != scanData_.end(); ++rangeBegin_itr, ++rangeEnd_itr)
	{
		std::map< int, Moments >::const_iterator x1_itr = rangeBegin_itr;
		std::map< int, Moments >::const_iterator x2_itr = rangeBegin_itr; ++x2_itr;
		double minSlopeInRange = 0; bool firstSlope = true;
		for( ; x2_itr != rangeEnd_itr; ++x1_itr, ++x2_itr  )
		{
			double newSlope = slope(x1_itr->first, x2_itr->first, takeNegative).first;
			if ( firstSlope || newSlope < minSlopeInRange ) minSlopeInRange = newSlope;
			firstSlope = false;
		}
		if ( firstRange || minSlopeInRange > maxSustainedSlope )
		{
			maxSustainedSlope = minSlopeInRange;
			maxSustainedSlopeEndPoint = x2_itr->first;
		}
		firstRange = false;
		
		// If the function drops back down, require the largest slope to lie at a higher x-value.
		double averageSlopeOverRange = slope(rangeBegin_itr->first, rangeEnd_itr->first, takeNegative).first;
		if ( averageSlopeOverRange*maxSustainedSlope < 0. && fabs(averageSlopeOverRange) > 0.25*fabs(maxSustainedSlope) )
		{
			firstRange = true;
		}
	}
	return std::pair< int, double> (maxSustainedSlopeEndPoint, maxSustainedSlope);
}

std::pair< double, double > PixelScanRecord::slope( int x1, int x2, bool takeNegative ) const
{
	std::map< int, Moments >::const_iterator x1_itr = scanData_.find(x1);
	std::map< int, Moments >::const_iterator x2_itr = scanData_.find(x2);
	assert( x1_itr != scanData_.end() );
	assert( x2_itr != scanData_.end() );
	assert( x1 != x2 );
	
	std::pair< double, double > returnValue;
	returnValue.first = ( x1_itr->second.mean() - x2_itr->second.mean() )/( (double)x1 - (double)x2 );
	if ( takeNegative ) returnValue.first = -returnValue.first;
	if ( x1_itr->second.count() == 1 || x2_itr->second.count() == 1 ) // no uncertainty information, so set a low uncertainty in order to effectively ignore uncertainty
	{
		returnValue.second = fabs(returnValue.first)*1e-10;
	}
	else
	{
		returnValue.second = sqrt( x1_itr->second.uncertainty()*x1_itr->second.uncertainty() + x2_itr->second.uncertainty()*x2_itr->second.uncertainty() )/( (double)x1 - (double)x2 );
	}
	return returnValue;
}

void PixelScanRecord::printPlot(std::string filename, int color, std::vector<TPaveText> extraTextBoxes) const // color = ROOT color code
{
	TCanvas c(title_.c_str(), title_.c_str(), 800, 600);
	TGraphErrors plot = makePlot(color);
	plot.Draw("AP");
	
	const double xmin = plot.GetXaxis()->GetXmin();
	const double xmax = plot.GetXaxis()->GetXmax();
	const double ymin = plot.GetYaxis()->GetXmin();
	const double ymax = plot.GetYaxis()->GetXmax();
	
	bool drawLegend = false;
	TPaveText legend(0.8,0.85,1.00,1.00,"BRNDC");
	TLine highestPointXLine, highestPointYLine, crossingPointXLine, crossingPointYLine, saturationPointXLine;
	if ( highestPointFound_ )
	{
		drawLegend = true;
		TText* thisLine = legend.AddText("Highest Point"); thisLine->SetTextColor(kMagenta);
		highestPointXLine = TLine( highestPoint_.first, ymin, highestPoint_.first, ymax );
		highestPointXLine.SetLineColor(kMagenta); highestPointXLine.SetLineStyle(kDashed);
		highestPointXLine.Draw();
		highestPointYLine = TLine( xmin, highestPoint_.second, xmax, highestPoint_.second );
		highestPointYLine.SetLineColor(kMagenta); highestPointYLine.SetLineStyle(kDashed);
		highestPointYLine.Draw();
	}
	if ( crossingPointFound_ )
	{
		drawLegend = true;
		TText* thisLine = legend.AddText("Intersection:"); thisLine->SetTextColor(kGreen);
		       thisLine = legend.AddText(("  ("+stringF(crossingPoint_)+","+stringF(crossingYTarget_)+")").c_str()); thisLine->SetTextColor(kGreen);
		crossingPointXLine = TLine( crossingPoint_, ymin, crossingPoint_, ymax );
		crossingPointXLine.SetLineColor(kGreen); crossingPointXLine.SetLineStyle(kDashed);
		crossingPointXLine.Draw();
		crossingPointYLine = TLine( xmin, crossingYTarget_, xmax, crossingYTarget_ );
		crossingPointYLine.SetLineColor(kGreen); crossingPointYLine.SetLineStyle(kDashed);
		crossingPointYLine.Draw();
	}
	if ( saturationPointFound_ )
	{
		drawLegend = true;
		TText* thisLine = legend.AddText("Saturation Point"); thisLine->SetTextColor(kGreen);
		saturationPointXLine = TLine( saturationPoint_, ymin, saturationPoint_, ymax );
		saturationPointXLine.SetLineColor(kGreen); saturationPointXLine.SetLineStyle(kDashed);
		saturationPointXLine.Draw();
	}
	if ( drawLegend ) legend.Draw();
	
	for ( std::vector<TPaveText>::iterator extraTextBoxes_itr = extraTextBoxes.begin(); extraTextBoxes_itr != extraTextBoxes.end(); extraTextBoxes_itr++ )
	{
		extraTextBoxes_itr->Draw();
	}
	
	if ( filename == "rootFile" )
	{
		((TPad*)(&c))->Write();
	}
	else
	{
		c.Print(filename.c_str());
	}
}

TGraphErrors PixelScanRecord::makePlot(int color) const // color = ROOT color code
{
	double* x = new double [scanData_.size()];
	double* y = new double [scanData_.size()];
	double* ey = new double [scanData_.size()];

	unsigned int i = 0;
	for (std::map< int, Moments >::const_iterator scanData_itr = scanData_.begin(); scanData_itr != scanData_.end(); ++scanData_itr)
	{
		x[i] = scanData_itr->first;
		y[i] = scanData_itr->second.mean();
		ey[i] = scanData_itr->second.uncertainty();
		i++;
	}
	assert( i == scanData_.size() );

	TGraphErrors plot( scanData_.size(), x, y, 0, ey );
	plot.SetTitle(title_.c_str());
	plot.SetMarkerColor(color);
	plot.SetMarkerStyle(kFullCircle);
	//plot.SetMarkerSize(2);
	plot.GetXaxis()->SetTitle(xVarName_.c_str());
	plot.GetYaxis()->SetTitle(yVarName_.c_str());

	delete[] x;
	delete[] y;
	delete[] ey;

	return plot;
}

const PixelScanRecord PixelScanRecord::operator-(const PixelScanRecord& another) const
{
	PixelScanRecord toReturn;
	toReturn.xVarName_ = xVarName_;
	if ( xVarName_ != another.xVarName_ ) toReturn.xVarName_ += " or "+another.xVarName_;
	toReturn.yVarName_ = yVarName_+" - "+another.yVarName_;
	toReturn.title_ = title_;
	if ( title_ != another.title_ ) toReturn.title_ += " or "+another.title_;
	for (std::map< int, Moments >::const_iterator scanData_itr = scanData_.begin(); scanData_itr != scanData_.end(); ++scanData_itr)
	{
		std::map< int, Moments >::const_iterator foundInSecondScan = another.scanData_.find(scanData_itr->first);
		if ( foundInSecondScan != another.scanData_.end() )
		{
			toReturn.scanData_[scanData_itr->first] = scanData_itr->second - foundInSecondScan->second;
		}
	}
	return toReturn;
}
