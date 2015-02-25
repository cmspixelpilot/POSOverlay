#ifndef _PixelIanaAnalysis_h
#define _PixelIanaAnalysis_h

#include <string>
#include <vector>
#include <iostream>

class TGraphErrors;
class TF1;
class TCanvas;

// make the Iana fit and plot
// x is array of Vana values
// y is array of Iana values
// ey is error on each point
// if filename is null, assumes caller has already cd'ed into the root file/directory where canvas is wanted
struct PixelIanaAnalysis {
  PixelIanaAnalysis(bool lin) : linear_fit(lin), fitmin(10.) {}
  ~PixelIanaAnalysis();

  void go(const std::string& roc,
	  const double oldVana,
	  const int npoints,
	  const std::vector<double>& x,
	  std::vector<double> y,
	  std::vector<double> ey,
	  std::ostream& out,
	  const char* filename=0);

  void redoFromDat(const char* fn, const std::string& roc, std::ostream& out, const char* filename=0);

  const bool linear_fit;
  const double fitmin;
  TGraphErrors* gr;
  TF1* f2;
  int fitstatus1;
  int fitstatus2;
  double yvalatzero;
  double fitChisquare;
  double maxIana;
  double oldIana;
  double oldVana;
  double newIana;
  double newVana;
  bool pass;
  TCanvas* canvas;
};


#endif
