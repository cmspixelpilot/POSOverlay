#include "PixelCalibrations/include/PixelIanaAnalysis.h"

#include <cassert>
#include <fstream>

#include "TAxis.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TLine.h"

void PixelIanaAnalysis::go(const std::string& roc,
			   const double oldVana_,
			   const int npoints,
			   const std::vector<double>& x,
			   std::vector<double> y,
			   std::vector<double> ey,
			   std::ostream& out,
			   const char* filename)
{
  oldVana = oldVana_;

  if (linear_fit) {
    f2 = new TF1("f2", "[0] + [1]*x");
    f2->SetParameters(0.006, 0.0002);
  }
  else {
    f2 = new TF1("f2",
		 "(x < [0]) * ([2] + ([3] - [2])*exp(([4] - [3])*(x - [0])/([1]*([3] - [2])))) + "
		 "(x >= [0] + [1]) * [4] + "
		 "(x >= [0]) * (x < [0] + [1]) * ([3] + (x - [0])*([4] - [3])/[1])",
		 0, 250);
    f2->SetParameters(120,
		      60,
		      y[1],
		      0.5*(y[1] + y[npoints - 2]),
		      y[npoints - 2]
		      );
    f2->SetParLimits(0, 10, 240);
    f2->SetParLimits(1, 0.1, 200);
  }

  gr = new TGraphErrors(npoints - 1, &x[0], &y[0], 0, &ey[0]);
  fitstatus1 = gr->Fit("f2", "", "", fitmin, 250);
  yvalatzero = f2->Eval(0.0);

  out << yvalatzero << "\n";

  // adjust data so values are in mA and y intercept is at 0, then refit
  for (int ivana = 0; ivana < npoints; ++ivana) {
    y[ivana] = 1000*(y[ivana] - yvalatzero);
    ey[ivana] *= 1000;
  }

  delete gr;

  if (linear_fit)
    f2->SetParameters(6, 0.2);
  else {
    f2->SetParameters(120,
		      60,
		      y[1],
		      0.5*(y[1] + y[npoints - 2]),
		      y[npoints-2]);
    f2->SetParLimits(0,10,240);
    f2->SetParLimits(1,0.1,200);
  }
      
  canvas = new TCanvas(roc.c_str(), roc.c_str(), 800, 600);
  gr = new TGraphErrors(npoints - 1, &x[0], &y[0], 0, &ey[0]);
  fitstatus2 = gr->Fit("f2", "", "", fitmin, 250);

  if (linear_fit)
    out << f2->GetParameter(0) << " "
	<< f2->GetParameter(1) << "\n";
  else
    out << f2->GetParameter(0) << " "
	<< f2->GetParameter(1) << " "
	<< f2->GetParameter(2) << " "
	<< f2->GetParameter(3) << " "
	<< f2->GetParameter(4) << "\n";

  maxIana = f2->Eval(250);

  gr->SetLineColor(2);
  gr->SetLineWidth(4);
  gr->SetMarkerColor(4);
  gr->SetMarkerStyle(21);
  gr->SetTitle(roc.c_str());
  gr->SetMinimum(-10.0);
  gr->SetMaximum(maxIana + 10);
  gr->GetXaxis()->SetTitle("Vana");
  gr->GetYaxis()->SetTitle("Iana (mA)");
  gr->Draw("AP");
  
  f2->Draw("same");
  f2->SetLineColor(1);

  fitChisquare = f2->GetChisquare();
      
  //find vana value where iana crosses 25
  newVana = 0;
  for (newVana = 0; newVana < 250; ++newVana) {
    newIana = f2->Eval(newVana);
    if (newIana > 25.0)
      break;
  }

  out << oldVana << "\n" << newVana << "\n";

  oldIana = f2->Eval(oldVana);          

  TLine l1(oldVana, -10.0, oldVana, oldIana);
  l1.SetLineColor(1);
  l1.Draw();
  TLine l2(0.0, oldIana, oldVana, oldIana);
  l2.SetLineColor(1);
  l2.Draw();

  TLine l3(newVana, -10.0, newVana, newIana);
  l3.SetLineColor(2);
  l3.Draw();
  TLine l4(0.0, newIana, newVana, newIana);
  l4.SetLineColor(2);
  l4.Draw();

  pass = !(newVana >= 249 || maxIana < 25 || newVana <= 4 || fitstatus1 != 0 || fitstatus2 != 0);

  if (filename)
    canvas->SaveAs(filename);
  else
    canvas->Write();
}

PixelIanaAnalysis::~PixelIanaAnalysis() {
  delete f2;
  delete gr;
  delete canvas;
}

void PixelIanaAnalysis::redoFromDat(const char* fn, const std::string& roc, std::ostream& out, const char* filename) {
  std::ifstream in(fn);

  std::string this_roc;

  if (roc != "") {
    while (this_roc != roc && getline(in, this_roc))
      ;
    assert(!in.eof());
  }
  else
    in >> this_roc;

  int npoints;
  in >> npoints;

  std::vector<double> x(npoints), y(npoints), ey(npoints);
  std::vector<double>* d[3] = {&x,&y,&ey};

  for (int j = 0; j < 3; ++j) {
    for (int i = 0; i < npoints; ++i) {
      assert(!in.eof());
      in >> (*d[j])[i];
    }
  }

  double yvalatzero;
  in >> yvalatzero;

  double dummy;
  for (int j = 0; j < 5; ++j)
    in >> dummy;

  int oldVana;
  int newVana;
  in >> oldVana;
  in >> newVana;

  in.close();

  go(this_roc, newVana, npoints, x, y, ey, out, filename);
}
