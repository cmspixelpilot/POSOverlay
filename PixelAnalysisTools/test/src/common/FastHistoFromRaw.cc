#ifndef __CINT__
#include <TROOT.h>
#include <TApplication.h>
#include <TF1.h>
#include <TF2.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TMath.h>
#include <TCanvas.h>
#include <TStyle.h>

#include <TLegend.h>
#include <TPaveText.h>
#endif

#include "assert.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <math.h>
#include <map>

#include "ReadParameters.h"
#include "PixelSLinkData.h"

#define MAXEVENTS 100
#define MINCHARGE 150
#define MAXCHARGE 255
#define MINTRIG 0
#define MAXTRIG 10

using namespace std;
using namespace pixel;


int Delta = 0;

struct HistoStruct1D
{
  vector<TH1D*> vec;
  vector<TF1*>  fit;
  TCanvas*      can;
};
struct HistoStruct2D
{
  vector<TH2D*> vec;
  vector<TF2*>  fit;
  TCanvas*      can;
};

typedef map<string, HistoStruct1D> Mappa1D;
typedef map<string, HistoStruct2D> Mappa2D;


bool OpenInputFile (const char* inputFilename,
		    ifstream**  inputFile)
{
  unsigned long tmp1, tmp2;

  (*inputFile) = new ifstream(inputFilename,ios::binary|ios::in);
  assert((*inputFile)->good());

  (*inputFile)->read ((char *)&tmp1, sizeof tmp1);
  (*inputFile)->read ((char *)&tmp2, sizeof tmp2);

  while ((*inputFile)->gcount() == sizeof(tmp1))
    {
      // Check for bad data and skip if needed
      if ( ((tmp1&0xff000000) != 0x50000000) && ((tmp2&0x000000ff) != 0x00000000) )
	{
 	  cout << "Crappy data = 0x" << hex << tmp1 << "-0x" << tmp2 << " skipping until good header found..." << dec << endl;
	}
      else
	{
	  cout << "Good data start!" << endl;
	  return true;
	}
      (*inputFile)->read ((char *)&tmp1, sizeof tmp1);
      (*inputFile)->read ((char *)&tmp2, sizeof tmp2);
    }
  return false;
}


void CloseInputFile (ifstream* inputFile)
{
  inputFile->close();
  delete inputFile;
}


bool EOFile (ifstream* inputFile)
{
  return (inputFile->eof());
}


bool NewData (ifstream*  inputFile,
	      SLinkData* pixelData)
{
  return (pixelData->load(*inputFile));
}


unsigned int NHistos (string PlaqType)
{
  if (PlaqType.compare("1x2") == 0)
    {
      return 2;
    }
  else if (PlaqType.compare("1x5") == 0)
    {
      return 5;
    }
  else if (PlaqType.compare("2x5") == 0)
    {
      return 10;
    }
  else if (PlaqType.compare("2x4") == 0)
    {
      return 8;
    }
  else if (PlaqType.compare("2x3") == 0)
    {
      return 6;
    }

  return 0;
}


void CreateHistos (vector<TCanvas*>* Canvas,
		   vector<TH1D*>*    Histo1D,
		   vector<TH2D*>*    Histo2D,
		   Mappa1D*          HistoPlaq1D,
		   Mappa2D*          HistoPlaq2D,
		   string            PanelType,
		   int               NEVENTS)
{
  stringstream String1;
  stringstream String2;
  stringstream String3;

  if ((PanelType == "4L") || (PanelType == "4R"))
    {
      // ****** 1x2 ******
      (*HistoPlaq1D)["1x2"].can = new TCanvas("1x2 - 1D","Online Monitor - 1D",1,1,500,300); // 1x2
      (*HistoPlaq1D)["1x2"].can->Divide(2,1);
      (*HistoPlaq1D)["1x2"].can->SetBorderMode(0);
      (*HistoPlaq1D)["1x2"].can->SetFrameFillColor(10);
      (*HistoPlaq1D)["1x2"].can->SetFillColor(0); // 10
      (*HistoPlaq1D)["1x2"].can->SetGrid();
      (*HistoPlaq1D)["1x2"].can->Modified();
      (*HistoPlaq1D)["1x2"].can->Update();

      (*HistoPlaq2D)["1x2"].can = new TCanvas("1x2 - 2D","Online Monitor - 2D",1,1,500,300); // 1x2
      (*HistoPlaq2D)["1x2"].can->Divide(2,1);
      (*HistoPlaq2D)["1x2"].can->SetBorderMode(0);
      (*HistoPlaq2D)["1x2"].can->SetFrameFillColor(10);
      (*HistoPlaq2D)["1x2"].can->SetFillColor(0); // 10
      (*HistoPlaq2D)["1x2"].can->SetGrid();
      (*HistoPlaq2D)["1x2"].can->Modified();
      (*HistoPlaq2D)["1x2"].can->Update();

      for (unsigned int i=0; i<NHistos("1x2"); i++)
	{
	  String1.str("");
          String1 << "H1D 4x 1x2 " << i; 
	  String2.str("");
          String2 << "H2D 4x 1x2 " << i; 
	  String3.str("");
          String3 << "4x 1x2 ROC " << i; 
	  (*HistoPlaq1D)["1x2"].vec.push_back (new TH1D(String1.str().c_str(),String3.str().c_str(),255,0.,255.));
	  (*HistoPlaq1D)["1x2"].vec.back()->SetXTitle("Charge [ADC]");
	  (*HistoPlaq1D)["1x2"].vec.back()->SetYTitle("Entries");

	  (*HistoPlaq2D)["1x2"].vec.push_back (new TH2D(String2.str().c_str(),String3.str().c_str(),52,1.,53.,80,1.,81.));
	  (*HistoPlaq2D)["1x2"].vec.back()->SetXTitle("Columns");
	  (*HistoPlaq2D)["1x2"].vec.back()->SetYTitle("Rows");
	}


      // ****** 1x5 ******
      (*HistoPlaq1D)["1x5"].can = new TCanvas("1x5 - 1D","Online Monitor - 1D",1,1,1200,300); // 1x5
      (*HistoPlaq1D)["1x5"].can->Divide(5,1);
      (*HistoPlaq1D)["1x5"].can->SetBorderMode(0);
      (*HistoPlaq1D)["1x5"].can->SetFrameFillColor(10);
      (*HistoPlaq1D)["1x5"].can->SetFillColor(0); // 10
      (*HistoPlaq1D)["1x5"].can->SetGrid();
      (*HistoPlaq1D)["1x5"].can->Modified();
      (*HistoPlaq1D)["1x5"].can->Update();

      (*HistoPlaq2D)["1x5"].can = new TCanvas("1x5 - 2D","Online Monitor - 2D",1,1,1200,300); // 1x5
      (*HistoPlaq2D)["1x5"].can->Divide(5,1);
      (*HistoPlaq2D)["1x5"].can->SetBorderMode(0);
      (*HistoPlaq2D)["1x5"].can->SetFrameFillColor(10);
      (*HistoPlaq2D)["1x5"].can->SetFillColor(0); // 10
      (*HistoPlaq2D)["1x5"].can->SetGrid();
      (*HistoPlaq2D)["1x5"].can->Modified();
      (*HistoPlaq2D)["1x5"].can->Update();

      for (unsigned int i=0; i<NHistos("1x5"); i++)
	{
	  String1.str("");
          String1 << "H1D 4x 1x5 " << i; 
	  String2.str("");
          String2 << "H2D 4x 1x5 " << i; 
	  String3.str("");
          String3 << "4x 1x5 ROC " << i; 
	  (*HistoPlaq1D)["1x5"].vec.push_back (new TH1D(String1.str().c_str(),String3.str().c_str(),255,0.,255.));
	  (*HistoPlaq1D)["1x5"].vec.back()->SetXTitle("Charge [ADC]");
	  (*HistoPlaq1D)["1x5"].vec.back()->SetYTitle("Entries");

	  (*HistoPlaq2D)["1x5"].vec.push_back (new TH2D(String2.str().c_str(),String3.str().c_str(),52,1.,53.,80,1.,81.));
	  (*HistoPlaq2D)["1x5"].vec.back()->SetXTitle("Columns");
	  (*HistoPlaq2D)["1x5"].vec.back()->SetYTitle("Rows");
	}

      // ****** 2x4 ******
      (*HistoPlaq1D)["2x4"].can = new TCanvas("2x4 - 1D","Online Monitor - 1D",1,1,1000,600); // 2x4
      (*HistoPlaq1D)["2x4"].can->Divide(4,2);
      (*HistoPlaq1D)["2x4"].can->SetBorderMode(0);
      (*HistoPlaq1D)["2x4"].can->SetFrameFillColor(10);
      (*HistoPlaq1D)["2x4"].can->SetFillColor(0); // 10
      (*HistoPlaq1D)["2x4"].can->SetGrid();
      (*HistoPlaq1D)["2x4"].can->Modified();
      (*HistoPlaq1D)["2x4"].can->Update();

      (*HistoPlaq2D)["2x4"].can = new TCanvas("2x4 - 2D","Online Monitor - 2D",1,1,1000,600); // 2x4
      (*HistoPlaq2D)["2x4"].can->Divide(4,2);
      (*HistoPlaq2D)["2x4"].can->SetBorderMode(0);
      (*HistoPlaq2D)["2x4"].can->SetFrameFillColor(10);
      (*HistoPlaq2D)["2x4"].can->SetFillColor(0); // 10
      (*HistoPlaq2D)["2x4"].can->SetGrid();
      (*HistoPlaq2D)["2x4"].can->Modified();
      (*HistoPlaq2D)["2x4"].can->Update();

      for (unsigned int i=0; i<NHistos("2x4"); i++)
	{
	  String1.str("");
          String1 << "H1D 4x 2x4 " << i; 
	  String2.str("");
          String2 << "H2D 4x 2x4 " << i; 
	  String3.str("");
          String3 << "4x 2x4 ROC " << i; 
	  (*HistoPlaq1D)["2x4"].vec.push_back (new TH1D(String1.str().c_str(),String3.str().c_str(),255,0.,255.));
	  (*HistoPlaq1D)["2x4"].vec.back()->SetXTitle("Charge [ADC]");
	  (*HistoPlaq1D)["2x4"].vec.back()->SetYTitle("Entries");

	  (*HistoPlaq2D)["2x4"].vec.push_back (new TH2D(String2.str().c_str(),String3.str().c_str(),52,1.,53.,80,1.,81.));
	  (*HistoPlaq2D)["2x4"].vec.back()->SetXTitle("Columns");
	  (*HistoPlaq2D)["2x4"].vec.back()->SetYTitle("Rows");
	}

      // ****** 2x3 ******
      (*HistoPlaq1D)["2x3"].can = new TCanvas("2x3 - 1D","Online Monitor - 1D",1,1,750,600); // 2x3
      (*HistoPlaq1D)["2x3"].can->Divide(3,2);
      (*HistoPlaq1D)["2x3"].can->SetBorderMode(0);
      (*HistoPlaq1D)["2x3"].can->SetFrameFillColor(10);
      (*HistoPlaq1D)["2x3"].can->SetFillColor(0); // 10
      (*HistoPlaq1D)["2x3"].can->SetGrid();
      (*HistoPlaq1D)["2x3"].can->Modified();
      (*HistoPlaq1D)["2x3"].can->Update();

      (*HistoPlaq2D)["2x3"].can = new TCanvas("2x3 - 2D","Online Monitor - 2D",1,1,750,600); // 2x3
      (*HistoPlaq2D)["2x3"].can->Divide(3,2);
      (*HistoPlaq2D)["2x3"].can->SetBorderMode(0);
      (*HistoPlaq2D)["2x3"].can->SetFrameFillColor(10);
      (*HistoPlaq2D)["2x3"].can->SetFillColor(0); // 10
      (*HistoPlaq2D)["2x3"].can->SetGrid();
      (*HistoPlaq2D)["2x3"].can->Modified();
      (*HistoPlaq2D)["2x3"].can->Update();

      for (unsigned int i=0; i<NHistos("2x3"); i++)
	{
	  String1.str("");
          String1 << "H1D 4x 2x3 " << i; 
	  String2.str("");
          String2 << "H2D 4x 2x3 " << i; 
	  String3.str("");
          String3 << "4x 2x3 ROC " << i; 
	  (*HistoPlaq1D)["2x3"].vec.push_back (new TH1D(String1.str().c_str(),String3.str().c_str(),255,0.,255.));
	  (*HistoPlaq1D)["2x3"].vec.back()->SetXTitle("Charge [ADC]");
	  (*HistoPlaq1D)["2x3"].vec.back()->SetYTitle("Entries");

	  (*HistoPlaq2D)["2x3"].vec.push_back (new TH2D(String2.str().c_str(),String3.str().c_str(),52,1.,53.,80,1.,81.));
	  (*HistoPlaq2D)["2x3"].vec.back()->SetXTitle("Columns");
	  (*HistoPlaq2D)["2x3"].vec.back()->SetYTitle("Rows");
	}
    }
  else if ((PanelType == "3L") || (PanelType == "3R"))
    {
      // ****** 2x5 ******
      (*HistoPlaq1D)["2x5"].can = new TCanvas("2x5 - 1D","Online Monitor - 1D",1,1,1200,600); // 2x5
      (*HistoPlaq1D)["2x5"].can->Divide(5,2);
      (*HistoPlaq1D)["2x5"].can->SetBorderMode(0);
      (*HistoPlaq1D)["2x5"].can->SetFrameFillColor(10);
      (*HistoPlaq1D)["2x5"].can->SetFillColor(0); // 10
      (*HistoPlaq1D)["2x5"].can->SetGrid();
      (*HistoPlaq1D)["2x5"].can->Modified();
      (*HistoPlaq1D)["2x5"].can->Update();

      (*HistoPlaq2D)["2x5"].can = new TCanvas("2x5 - 2D","Online Monitor - 2D",1,1,1200,600); // 2x5
      (*HistoPlaq2D)["2x5"].can->Divide(5,2);
      (*HistoPlaq2D)["2x5"].can->SetBorderMode(0);
      (*HistoPlaq2D)["2x5"].can->SetFrameFillColor(10);
      (*HistoPlaq2D)["2x5"].can->SetFillColor(0); // 10
      (*HistoPlaq2D)["2x5"].can->SetGrid();
      (*HistoPlaq2D)["2x5"].can->Modified();
      (*HistoPlaq2D)["2x5"].can->Update();

      for (unsigned int i=0; i<NHistos("2x5"); i++)
	{
	  String1.str("");
          String1 << "H1D 3x 2x5 " << i; 
	  String2.str("");
          String2 << "H2D 3x 2x5 " << i; 
	  String3.str("");
          String3 << "3x 2x5 ROC " << i; 
	  (*HistoPlaq1D)["2x5"].vec.push_back (new TH1D(String1.str().c_str(),String3.str().c_str(),255,0.,255.));
	  (*HistoPlaq1D)["2x5"].vec.back()->SetXTitle("Charge [ADC]");
	  (*HistoPlaq1D)["2x5"].vec.back()->SetYTitle("Entries");

	  (*HistoPlaq2D)["2x5"].vec.push_back (new TH2D(String2.str().c_str(),String3.str().c_str(),52,1.,53.,80,1.,81.));
	  (*HistoPlaq2D)["2x5"].vec.back()->SetXTitle("Columns");
	  (*HistoPlaq2D)["2x5"].vec.back()->SetYTitle("Rows");
	}

      // ****** 2x4 ******
      (*HistoPlaq1D)["2x4"].can = new TCanvas("2x4 - 1D","Online Monitor - 1D",1,1,1000,600); // 2x4
      (*HistoPlaq1D)["2x4"].can->Divide(4,2);
      (*HistoPlaq1D)["2x4"].can->SetBorderMode(0);
      (*HistoPlaq1D)["2x4"].can->SetFrameFillColor(10);
      (*HistoPlaq1D)["2x4"].can->SetFillColor(0); // 10
      (*HistoPlaq1D)["2x4"].can->SetGrid();
      (*HistoPlaq1D)["2x4"].can->Modified();
      (*HistoPlaq1D)["2x4"].can->Update();

      (*HistoPlaq2D)["2x4"].can = new TCanvas("2x4 - 2D","Online Monitor - 2D",1,1,1000,600); // 2x4
      (*HistoPlaq2D)["2x4"].can->Divide(4,2);
      (*HistoPlaq2D)["2x4"].can->SetBorderMode(0);
      (*HistoPlaq2D)["2x4"].can->SetFrameFillColor(10);
      (*HistoPlaq2D)["2x4"].can->SetFillColor(0); // 10
      (*HistoPlaq2D)["2x4"].can->SetGrid();
      (*HistoPlaq2D)["2x4"].can->Modified();
      (*HistoPlaq2D)["2x4"].can->Update();

      for (unsigned int i=0; i<NHistos("2x4"); i++)
	{
	  String1.str("");
          String1 << "H1D 3x 2x4 " << i; 
	  String2.str("");
          String2 << "H2D 3x 2x4 " << i; 
	  String3.str("");
          String3 << "3x 2x4 ROC " << i; 
	  (*HistoPlaq1D)["2x4"].vec.push_back (new TH1D(String1.str().c_str(),String3.str().c_str(),255,0.,255.));
	  (*HistoPlaq1D)["2x4"].vec.back()->SetXTitle("Charge [ADC]");
	  (*HistoPlaq1D)["2x4"].vec.back()->SetYTitle("Entries");

	  (*HistoPlaq2D)["2x4"].vec.push_back (new TH2D(String2.str().c_str(),String3.str().c_str(),52,1.,53.,80,1.,81.));
	  (*HistoPlaq2D)["2x4"].vec.back()->SetXTitle("Columns");
	  (*HistoPlaq2D)["2x4"].vec.back()->SetYTitle("Rows");
	}

      // ****** 2x3 ******
      (*HistoPlaq1D)["2x3"].can = new TCanvas("2x3 - 1D","Online Monitor - 1D",1,1,750,600); // 2x3
      (*HistoPlaq1D)["2x3"].can->Divide(3,2);
      (*HistoPlaq1D)["2x3"].can->SetBorderMode(0);
      (*HistoPlaq1D)["2x3"].can->SetFrameFillColor(10);
      (*HistoPlaq1D)["2x3"].can->SetFillColor(0); // 10
      (*HistoPlaq1D)["2x3"].can->SetGrid();
      (*HistoPlaq1D)["2x3"].can->Modified();
      (*HistoPlaq1D)["2x3"].can->Update();

      (*HistoPlaq2D)["2x3"].can = new TCanvas("2x3 - 2D","Online Monitor - 2D",1,1,750,600); // 2x3
      (*HistoPlaq2D)["2x3"].can->Divide(3,2);
      (*HistoPlaq2D)["2x3"].can->SetBorderMode(0);
      (*HistoPlaq2D)["2x3"].can->SetFrameFillColor(10);
      (*HistoPlaq2D)["2x3"].can->SetFillColor(0); // 10
      (*HistoPlaq2D)["2x3"].can->SetGrid();
      (*HistoPlaq2D)["2x3"].can->Modified();
      (*HistoPlaq2D)["2x3"].can->Update();

      for (unsigned int i=0; i<NHistos("2x3"); i++)
	{
	  String1.str("");
          String1 << "H1D 3x 2x3 " << i; 
	  String2.str("");
          String2 << "H2D 3x 2x3 " << i; 
	  String3.str("");
          String3 << "3x 2x3 ROC " << i; 
	  (*HistoPlaq1D)["2x3"].vec.push_back (new TH1D(String1.str().c_str(),String3.str().c_str(),255,0.,255.));
	  (*HistoPlaq1D)["2x3"].vec.back()->SetXTitle("Charge [ADC]");
	  (*HistoPlaq1D)["2x3"].vec.back()->SetYTitle("Entries");

	  (*HistoPlaq2D)["2x3"].vec.push_back (new TH2D(String2.str().c_str(),String3.str().c_str(),52,1.,53.,80,1.,81.));
	  (*HistoPlaq2D)["2x3"].vec.back()->SetXTitle("Columns");
	  (*HistoPlaq2D)["2x3"].vec.back()->SetYTitle("Rows");
	}
    }

  // Single ROC Canvas
  Canvas->push_back (new TCanvas("SingleROC","Online Monitor",1,1,500,800));
  Canvas->back()->Divide(1,3);
  Canvas->back()->SetBorderMode(0);
  Canvas->back()->SetFrameFillColor(10);
  Canvas->back()->SetFillColor(0); // 10
  Canvas->back()->SetGrid();
  Canvas->back()->Modified();
  Canvas->back()->Update();
  
  // Single ROC Histogram
  Histo1D->push_back (new TH1D("SingleROC1D","Charge Distribution",255,0.,255.));
  Histo1D->back()->SetXTitle("Charge [ADC]");
  Histo1D->back()->SetYTitle("Entries");

  // Event Size Distribution
  if (NEVENTS < MAXEVENTS)
    {
      Histo1D->push_back (new TH1D("EventSize","Event Size Distribution",NEVENTS,1.,(double)(NEVENTS)+1.));
    }
  else
    {
      Histo1D->push_back (new TH1D("EventSize","Event Size Distribution",100,1.,101.));
    }
  Histo1D->back()->SetXTitle("#Event");
  Histo1D->back()->SetYTitle("Event Size");

  // Single ROC Histogram
  Histo2D->push_back (new TH2D("SingleROC2D","Hits on ROC",52,1.,53.,80,1.,81.));
  Histo2D->back()->SetXTitle("Columns");
  Histo2D->back()->SetYTitle("Rows");

  gStyle->SetPalette(1,0);
  gStyle->SetOptFit(1111);
  gStyle->SetOptStat(1111);
  gStyle->SetStatColor(0);
}


void RescaleHistos(string        PanelType,
		   vector<TH1D*> Histo1D,
		   vector<TH2D*> Histo2D,
		   Mappa1D*      HistoPlaq1D,
		   Mappa2D*      HistoPlaq2D)
{
  if ((PanelType == "4L") || (PanelType == "4R"))
    {
      // ****** 1x2 ******
      for (unsigned int j=0; j<NHistos("1x2"); j++)
	{
	  (*HistoPlaq1D)["1x2"].vec[j]->SetAxisRange((double)MINCHARGE,(double)MAXCHARGE);
	  (*HistoPlaq2D)["1x2"].vec[j]->SetAxisRange((double)MINTRIG,(double)MAXTRIG,"Z");
	}
      
      // ****** 1x5 ******
      for (unsigned int j=0; j<NHistos("1x5"); j++)
	{
	  (*HistoPlaq1D)["1x5"].vec[j]->SetAxisRange((double)MINCHARGE,(double)MAXCHARGE);
	  (*HistoPlaq2D)["1x5"].vec[j]->SetAxisRange((double)MINTRIG,(double)MAXTRIG,"Z");
	}
    }
  else if ((PanelType == "3L") || (PanelType == "3R"))
    {
      // ****** 2x5 ******
      for (unsigned int j=0; j<NHistos("2x5"); j++)
	{
	  (*HistoPlaq1D)["2x5"].vec[j]->SetAxisRange((double)MINCHARGE,(double)MAXCHARGE); 
	  (*HistoPlaq2D)["2x5"].vec[j]->SetAxisRange((double)MINTRIG,(double)MAXTRIG,"Z");
	}
    }
  
  // ****** 2x4 ******
  for (unsigned int j=0; j<NHistos("2x4"); j++)
    {
      (*HistoPlaq1D)["2x4"].vec[j]->SetAxisRange((double)MINCHARGE,(double)MAXCHARGE);
      (*HistoPlaq2D)["2x4"].vec[j]->SetAxisRange((double)MINTRIG,(double)MAXTRIG,"Z");
    }

  // ****** 2x3 ******
  for (unsigned int j=0; j<NHistos("2x3"); j++)
    {
      (*HistoPlaq1D)["2x3"].vec[j]->SetAxisRange((double)MINCHARGE,(double)MAXCHARGE);
      (*HistoPlaq2D)["2x3"].vec[j]->SetAxisRange((double)MINTRIG,(double)MAXTRIG,"Z");
    }

  Histo2D. back()->SetAxisRange((double)MINTRIG,(double)MAXTRIG,"Z");
}


unsigned int ROCOffset (string  PanelType,
			string  PlaqType)
{
  if (PanelType.compare("4R") == 0)
    {
      if (PlaqType.compare("1x5") == 0)
	{
	  return 0;
	}
      else if (PlaqType.compare("2x4") == 0)
	{
	  return 5;
	}
      else if (PlaqType.compare("2x3") == 0)
	{
	  return 13;
	}
      else if (PlaqType.compare("1x2") == 0)
	{
	  return 19;
	}
    }
  else if (PanelType.compare("3R") == 0)
    {
      if (PlaqType.compare("2x5") == 0)
	{
	  return 0;
	}
      else if (PlaqType.compare("2x4") == 0)
	{
	  return 10;
	}
      else if (PlaqType.compare("2x3") == 0)
	{
	  return 18;
	}
    }
  else if (PanelType.compare("4L") == 0)
    {
      if (PlaqType.compare("1x5") == 0)
	{
	  return 16;
	}
      else if (PlaqType.compare("2x4") == 0)
	{
	  return 8;
	}
      else if (PlaqType.compare("2x3") == 0)
	{
	  return 2;
	}
      else if (PlaqType.compare("1x2") == 0)
	{
	  return 0;
	}
    }
  else if (PanelType.compare("3L") == 0)
    {
      if (PlaqType.compare("2x5") == 0)
	{
	  return 14;
	}
      else if (PlaqType.compare("2x4") == 0)
	{
	  return 6;
	}
      else if (PlaqType.compare("2x3") == 0)
	{
	  return 0;
	}
    }
  return 0;
}


bool ROCMapping (string        PanelType,
		 string        PlaqType,
		 unsigned int  ROCID,
		 unsigned int* ROCPlaq,
		 unsigned int* ROCVect)
{
  if (PanelType.compare("4R") == 0)
    {
      if ((ROCID>=1) && (ROCID<=5) && (PlaqType.compare("1x5") == 0))
	{
	  *ROCPlaq = 5 - ROCID;
	  *ROCVect = ROCID - 1;
	  return true;
	}
      else if ((ROCID>=6) && (ROCID<=13) && (PlaqType.compare("2x4") == 0))
	{
 	  if ((ROCID - 6) <= 3)
 	    {
 	      *ROCPlaq = 3 - (ROCID - 6);
 	    }
 	  else
	  {
	    *ROCPlaq = ROCID - 6;
	  }
	  *ROCVect = ROCID - 6;
	  return true;
	}
      else if ((ROCID>=14) && (ROCID<=19) && (PlaqType.compare("2x3") == 0))
	{
	  if ((ROCID - 14) <= 2)
	    {
	      *ROCPlaq = 2 - (ROCID - 14);
	    }
	  else
	    {
	      *ROCPlaq = ROCID - 14;
	    }
	  *ROCVect = ROCID - 14;
	  return true;
	}
      else if ((ROCID>=20) && (ROCID<=21) && (PlaqType.compare("1x2") == 0))
	{
	  *ROCPlaq = 21 - ROCID;
	  *ROCVect = ROCID - 20;
	  return true;
	}
    }
  else if (PanelType.compare("3R") == 0)
    {
      if ((ROCID>=1) && (ROCID<=10) && (PlaqType.compare("2x5") == 0))
	{
 	  if ((ROCID - 1) <= 4)
 	    {
 	      *ROCPlaq = 4 - (ROCID - 1);
 	    }
 	  else
	  {
	    *ROCPlaq = ROCID - 1;
	  }
	  *ROCVect = ROCID - 1;
	  return true;
	}
      else if ((ROCID>=11) && (ROCID<=18) && (PlaqType.compare("2x4") == 0))
	{
 	  if ((ROCID - 11) <= 3)
 	    {
 	      *ROCPlaq = 3 - (ROCID - 11);
 	    }
 	  else
	  {
	    *ROCPlaq = ROCID - 11;
	  }
	  *ROCVect = ROCID - 11;
	  return true;
	}
      else if ((ROCID>=19) && (ROCID<=24) && (PlaqType.compare("2x3") == 0))
	{
 	  if ((ROCID - 19) <= 2)
 	    {
 	      *ROCPlaq = 2 - (ROCID - 19);
 	    }
 	  else
	  {
	    *ROCPlaq = ROCID - 19;
	  }
	  *ROCVect = ROCID - 19;
	  return true;
	}
    }
  else if (PanelType.compare("4L") == 0)
    {
      if ((ROCID>=1) && (ROCID<=2) && (PlaqType.compare("1x2") == 0))
	{
	  *ROCPlaq = 2 - ROCID;
	  *ROCVect = ROCID - 1;
	  return true;
	}
      else if ((ROCID>=3) && (ROCID<=8) && (PlaqType.compare("2x3") == 0))
	{
 	  if ((ROCID - 3) <= 2)
 	    {
 	      *ROCPlaq = 3 + (ROCID - 3);
 	    }
 	  else
	  {
	    *ROCPlaq = 5 - (ROCID - 3);
	  }
	  *ROCVect = ROCID - 3;
	  return true;
	}
      else if ((ROCID>=9) && (ROCID<=16) && (PlaqType.compare("2x4") == 0))
	{
	  if ((ROCID - 9) <= 3)
	    {
	      *ROCPlaq = 4 + (ROCID - 9);
	    }
	  else
	    {
	      *ROCPlaq = 7 - (ROCID - 9);
	    }
	  *ROCVect = ROCID - 9;
	  return true;
	}
      else if ((ROCID>=17) && (ROCID<=21) && (PlaqType.compare("1x5") == 0))
	{
	  *ROCPlaq = 21 - ROCID;
	  *ROCVect = ROCID - 17;
	  return true;
	}
    }
  else if (PanelType.compare("3L") == 0)
    {
      if ((ROCID>=15) && (ROCID<=24) && (PlaqType.compare("2x5") == 0))
	{
 	  if ((ROCID - 15) <= 4)
 	    {
 	      *ROCPlaq = 5 + (ROCID - 15);
 	    }
 	  else
	  {
	    *ROCPlaq = 9 - (ROCID - 15);
	  }
	  *ROCVect = ROCID - 15;
	  return true;
	}
      else if ((ROCID>=7) && (ROCID<=14) && (PlaqType.compare("2x4") == 0))
	{
 	  if ((ROCID - 7) <= 3)
 	    {
 	      *ROCPlaq = 4 + (ROCID - 7);
 	    }
 	  else
	  {
	    *ROCPlaq = 7 - (ROCID - 7);
	  }
	  *ROCVect = ROCID - 7;
	  return true;
	}
      else if ((ROCID>=1) && (ROCID<=6) && (PlaqType.compare("2x3") == 0))
	{
 	  if ((ROCID - 1) <= 2)
 	    {
 	      *ROCPlaq = 3 + (ROCID - 1);
 	    }
 	  else
	  {
	    *ROCPlaq = 5 - (ROCID - 1);
	  }
	  *ROCVect = ROCID - 1;
	  return true;
	}
    }
  return false;
}


void RowColRemapping (string        PlaqType,
		      unsigned int  ROCPlaq,
		      unsigned int  Row,
		      unsigned int  Col,
		      unsigned int* RowRemap,
		      unsigned int* ColRemap)
{
  if (PlaqType.compare("1x2") == 0)
    {
      *RowRemap = Row + 1;
      *ColRemap = Col + 1;
    }
  else if (PlaqType.compare("1x5") == 0)
    {
      *RowRemap = Row + 1;
      *ColRemap = Col + 1;
    }
  else if (PlaqType.compare("2x5") == 0)
    {
      if (ROCPlaq < 5)
	{
	  *RowRemap = 80 - Row;
	  *ColRemap = 52 - Col;
	}
      else
	{
	  *RowRemap = Row + 1;
	  *ColRemap = Col + 1;
	}	
    }
  else if (PlaqType.compare("2x4") == 0)
    {
      if (ROCPlaq < 4)
	{
	  *RowRemap = 80 - Row;
	  *ColRemap = 52 - Col;
	}
      else
	{
	  *RowRemap = Row + 1;
	  *ColRemap = Col + 1;
	}
    }
  else if (PlaqType.compare("2x3") == 0)
    {
      if (ROCPlaq < 3)
	{
	  *RowRemap = 80 - Row;
	  *ColRemap = 52 - Col;
	}
      else
	{
	  *RowRemap = Row + 1;
	  *ColRemap = Col + 1;
	}
    }
}


void FillHistos(unsigned int     FEDchn,
		unsigned int     ROCID,
		string           PanelType,
		string           PlaqType,
		SLinkData        pixelData,
		vector<TCanvas*> Canvas,
		vector<TH1D*>    Histo1D,
		vector<TH2D*>    Histo2D,
		Mappa1D*         HistoPlaq1D,
		Mappa2D*         HistoPlaq2D,
		int              Event,
		bool             Log)
{
  unsigned int ROCPlaq;
  unsigned int ROCVect;
  unsigned int RowRemap;
  unsigned int ColRemap;

  SLinkHeader  eventHeader           = pixelData.getHeader();
  SLinkTrailer eventTrailer          = pixelData.getTrailer();
  vector<SLinkHit> hits              = pixelData.getHits();
  vector<SLinkError> errors          = pixelData.getErrors();
  vector<SLinkHit>::iterator i_hit   = hits.begin();
  vector<SLinkError>::iterator i_err = errors.begin();

  // Header Data
  int BOE       = eventHeader.getBOE();
  int Evt_ty    = eventHeader.getEvt_ty();
  int LV1_id    = eventHeader.getLV1_id();
  int BX_id     = eventHeader.getBX_id();
  int Source_id = eventHeader.getSource_id();
  int FOV       = eventHeader.getFOV();
  int H         = eventHeader.getH();

  // Data variables
  unsigned int channel;
  unsigned int rocid;
  unsigned int row;
  unsigned int col;
  unsigned int adc;

  // Error variables
  unsigned int error_Channel;
  unsigned int error_Type;
  unsigned int error_InvalidNROC;
  unsigned int error_InterBits;
  unsigned int error_Info;

  // Trailer Data
  int EOE       = eventTrailer.getEOE();
  int Evt_lgth  = eventTrailer.getEvt_lgth();
  int CRC       = eventTrailer.getCRC();
  int Evt_stat  = eventTrailer.getEvt_stat();
  int TTS       = eventTrailer.getTTS();
  int T         = eventTrailer.getT();
  
//   if ( (Log == true) && (Event > 3470) && (Event < 3490) )
  if (Log == true)
    {
      cout << "\nEvent number --> " << Event << endl;
      cout << "Slink Header -->";
      cout << "\tBOE=0x" << hex << BOE;
      cout << "\tEvt_ty=0x" << Evt_ty;
      cout << "\tLV1_id=" << dec << LV1_id;
      cout << "\tBX_id=0x" << hex << BX_id;
      cout << "\tSource_id=0x" << Source_id;
      cout << "\tFOV=0x" << FOV;
      cout << "\tH=0x" << H << dec << endl;
      cout << "Event size --> " << hits.size() << endl;
      cout << "*** DATA ***" << endl;
    }

  // Look for missing events
  if (Event != (LV1_id - Delta))
//   if (Log && (Event != (LV1_id - Delta)))
    {
      cout << "\nEvent number --> " << Event << endl;
      cout << "Slink Header -->";
      cout << "\tBOE=0x" << hex << BOE;
      cout << "\tEvt_ty=0x" << Evt_ty;
      cout << "\tLV1_id=" << dec << LV1_id;
      cout << "\tBX_id=0x" << hex << BX_id;
      cout << "\tSource_id=0x" << Source_id;
      cout << "\tFOV=0x" << FOV;
      cout << "\tH=0x" << H << dec << endl;

      cout << "Event size --> " << hits.size() << endl;

//       cout << "ERROR: Channel --> " << error_Channel;
//       cout << "\tERROR: TYPE --> 0x" << hex << error_Type;
//       cout << "\tERROR: INVALID # ROCS --> 0x" << hex << error_InvalidNROC;
//       cout << "\tERROR: INTER BITS --> 0x" << hex << error_InterBits;
//       cout << "\tERROR: INFO --> 0x" << error_Info << dec << endl;

      cout << "Slink Trailer -->";
      cout << "\tEOE=0x" << hex << EOE;
      cout << "\tEvt_lgth=0x" << Evt_lgth;
      cout << "\tCRC=0x" << CRC;
      cout << "\tEvt_stat=0x" << Evt_stat;
      cout << "\tTTS=0x" << TTS;
      cout << "\tT=0x" << T << dec << endl;

      Delta = LV1_id - Event;

      cout << "Delta=" << Delta << endl;
    }

  for (; i_hit!=hits.end(); i_hit++)
    {
      channel = i_hit->get_link_id();
      rocid   = i_hit->get_roc_id();
      row     = i_hit->get_row();
      col     = i_hit->get_col();
      adc     = i_hit->get_adc();

//       if ( (Log == true) && (Event > 3470) && (Event < 3490) )
      if (Log == true)
	{
	  cout << "Channel --> " << channel;
	  cout << "\tROC ID --> " << rocid;
	  cout << "\tROW --> " << row;
	  cout << "\tCOL --> " << col;
	  cout << "\tPH --> " << adc << endl;
	}

      if ((channel == FEDchn) && (ROCMapping (PanelType, "1x2", rocid, &ROCPlaq, &ROCVect) == true))
 	{
	  // ****** 1x2 ******
	  RowColRemapping ("1x2", ROCPlaq, row, col, &RowRemap, &ColRemap);
 	  (*HistoPlaq1D)["1x2"].can->cd(ROCPlaq+1);
 	  (*HistoPlaq1D)["1x2"].vec[ROCVect]->Fill(adc);

	  (*HistoPlaq2D)["1x2"].can->cd(ROCPlaq+1);
 	  (*HistoPlaq2D)["1x2"].vec[ROCVect]->Fill(ColRemap,RowRemap);
	}
      else if ((channel == FEDchn) && (ROCMapping (PanelType, "1x5", rocid, &ROCPlaq, &ROCVect) == true))
 	{
	  // ****** 1x5 ******
 	  RowColRemapping ("1x5", ROCPlaq, row, col, &RowRemap, &ColRemap);
	  (*HistoPlaq1D)["1x5"].can->cd(ROCPlaq+1);
 	  (*HistoPlaq1D)["1x5"].vec[ROCVect]->Fill(adc);
	  
	  (*HistoPlaq2D)["1x5"].can->cd(ROCPlaq+1);
 	  (*HistoPlaq2D)["1x5"].vec[ROCVect]->Fill(ColRemap,RowRemap);
	}
      else if ((channel == FEDchn) && (ROCMapping (PanelType, "2x5", rocid, &ROCPlaq, &ROCVect) == true))
 	{
	  // ****** 2x5 ******
 	  RowColRemapping ("2x5", ROCPlaq, row, col, &RowRemap, &ColRemap);
	  (*HistoPlaq1D)["2x5"].can->cd(ROCPlaq+1);
 	  (*HistoPlaq1D)["2x5"].vec[ROCVect]->Fill(adc);
	  
	  (*HistoPlaq2D)["2x5"].can->cd(ROCPlaq+1);
 	  (*HistoPlaq2D)["2x5"].vec[ROCVect]->Fill(ColRemap,RowRemap);
	}
      else if ((channel == FEDchn) && (ROCMapping (PanelType, "2x4", rocid, &ROCPlaq, &ROCVect) == true))
 	{
	  // ****** 2x4 ******
 	  RowColRemapping ("2x4", ROCPlaq, row, col, &RowRemap, &ColRemap);
	  (*HistoPlaq1D)["2x4"].can->cd(ROCPlaq+1);
 	  (*HistoPlaq1D)["2x4"].vec[ROCVect]->Fill(adc);
	  
	  (*HistoPlaq2D)["2x4"].can->cd(ROCPlaq+1);
 	  (*HistoPlaq2D)["2x4"].vec[ROCVect]->Fill(ColRemap,RowRemap);
	}
      else if ((channel == FEDchn) && (ROCMapping (PanelType, "2x3", rocid, &ROCPlaq, &ROCVect) == true))
 	{
	  // ****** 2x3 ******
 	  RowColRemapping ("2x3", ROCPlaq, row, col, &RowRemap, &ColRemap);
	  (*HistoPlaq1D)["2x3"].can->cd(ROCPlaq+1);
 	  (*HistoPlaq1D)["2x3"].vec[ROCVect]->Fill(adc);
	  
	  (*HistoPlaq2D)["2x3"].can->cd(ROCPlaq+1);
 	  (*HistoPlaq2D)["2x3"].vec[ROCVect]->Fill(ColRemap,RowRemap);
	}

      // Single ROC Plot
      if ((channel == FEDchn) && (rocid == ROCID))
	{
	  Canvas[0]->cd(1);
	  Histo1D[0]->Fill(adc);
	  Canvas[0]->cd(2);
	  Histo2D[0]->Fill(ColRemap,RowRemap);
	}
    }
  
//   if ( (Log == true) && (Event > 3470) && (Event < 3490) )
  if (Log == true)
    {
      cout << "*** ERRORS ***" << endl;
      for (; i_err!=errors.end(); i_err++)
	{
	  error_Channel     = i_err->get_link_id();
	  error_Type        = i_err->get_ErrType();
	  error_InvalidNROC = i_err->get_InvalidNROC();
	  error_InterBits   = i_err->get_InterBits();
	  error_Info        = i_err->get_ErrInfo();
	  
	  cout << "ERROR: Channel --> " << error_Channel;
	  cout << "\tERROR: TYPE --> 0x" << hex << error_Type;
	  cout << "\tERROR: INVALID # ROCS --> 0x" << hex << error_InvalidNROC;
	  cout << "\tERROR: INTER BITS --> 0x" << hex << error_InterBits;
	  cout << "\tERROR: INFO --> 0x" << error_Info << dec << endl;
	}
    }

  // Event Size Distribution
  if (Event < MAXEVENTS)
    {
      Canvas[0]->cd(3);
      Histo1D[1]->SetBinContent(Event,(double)(hits.size()));
    }
  
//   if ( (Log == true) && (Event > 3470) && (Event < 3490) )
  if (Log == true)
    {
      cout << "Slink Trailer -->";
      cout << "\tEOE=0x" << hex << EOE;
      cout << "\tEvt_lgth=0x" << Evt_lgth;
      cout << "\tCRC=0x" << CRC;
      cout << "\tEvt_stat=0x" << Evt_stat;
      cout << "\tTTS=0x" << TTS;
      cout << "\tT=0x" << T << dec << endl;
    }
}


void DrawAll (string           PanelType,
	      vector<TCanvas*> Canvas,
	      vector<TH1D*>    Histo1D,
	      vector<TH2D*>    Histo2D,
	      Mappa1D*         HistoPlaq1D,
	      Mappa2D*         HistoPlaq2D)
{
  unsigned int ROCPlaq;
  unsigned int ROCVect;

  if ((PanelType == "4L") || (PanelType == "4R"))
    {
      // ****** 1x2 ******
      for (unsigned int j=0; j<NHistos("1x2"); j++)
	{
	  ROCMapping (PanelType, "1x2", j + 1 + ROCOffset (PanelType,"1x2"), &ROCPlaq, &ROCVect);
	  (*HistoPlaq1D)["1x2"].can->cd(ROCPlaq+1);
	  (*HistoPlaq1D)["1x2"].vec[ROCVect]->Draw();
	  (*HistoPlaq2D)["1x2"].can->cd(ROCPlaq+1);
	  (*HistoPlaq2D)["1x2"].vec[ROCVect]->Draw("gcolz"); // "contz"
	}
      (*HistoPlaq1D)["1x2"].can->Modified();
      (*HistoPlaq1D)["1x2"].can->Update();
      (*HistoPlaq2D)["1x2"].can->Modified();
      (*HistoPlaq2D)["1x2"].can->Update();
      
      // ****** 1x5 ******
      for (unsigned int j=0; j<NHistos("1x5"); j++)
	{
	  ROCMapping (PanelType, "1x5", j + 1 + ROCOffset (PanelType,"1x5"), &ROCPlaq, &ROCVect);
	  (*HistoPlaq1D)["1x5"].can->cd(ROCPlaq+1);
	  (*HistoPlaq1D)["1x5"].vec[ROCVect]->Draw();
	  (*HistoPlaq2D)["1x5"].can->cd(ROCPlaq+1);
	  (*HistoPlaq2D)["1x5"].vec[ROCVect]->Draw("gcolz"); // contz
	}
      (*HistoPlaq1D)["1x5"].can->Modified();
      (*HistoPlaq1D)["1x5"].can->Update();
      (*HistoPlaq2D)["1x5"].can->Modified();
      (*HistoPlaq2D)["1x5"].can->Update();
    }
  else if ((PanelType == "3L") || (PanelType == "3R"))
    {
      // ****** 2x5 ******
      for (unsigned int j=0; j<NHistos("2x5"); j++)
	{
	  ROCMapping (PanelType, "2x5", j + 1 + ROCOffset (PanelType,"2x5"), &ROCPlaq, &ROCVect);
	  (*HistoPlaq1D)["2x5"].can->cd(ROCPlaq+1);
	  (*HistoPlaq1D)["2x5"].vec[ROCVect]->Draw();
	  (*HistoPlaq2D)["2x5"].can->cd(ROCPlaq+1);
	  (*HistoPlaq2D)["2x5"].vec[ROCVect]->Draw("gcolz"); // contz
	}
      (*HistoPlaq1D)["2x5"].can->Modified();
      (*HistoPlaq1D)["2x5"].can->Update();
      (*HistoPlaq2D)["2x5"].can->Modified();
      (*HistoPlaq2D)["2x5"].can->Update();
    }
  
  // ****** 2x4 ******
  for (unsigned int j=0; j<NHistos("2x4"); j++)
    {
      ROCMapping (PanelType, "2x4", j + 1 + ROCOffset (PanelType,"2x4"), &ROCPlaq, &ROCVect);
      (*HistoPlaq1D)["2x4"].can->cd(ROCPlaq+1);
      (*HistoPlaq1D)["2x4"].vec[ROCVect]->Draw();
      (*HistoPlaq2D)["2x4"].can->cd(ROCPlaq+1);
      (*HistoPlaq2D)["2x4"].vec[ROCVect]->Draw("gcolz"); // contz
    }
  (*HistoPlaq1D)["2x4"].can->Modified();
  (*HistoPlaq1D)["2x4"].can->Update();
  (*HistoPlaq2D)["2x4"].can->Modified();
  (*HistoPlaq2D)["2x4"].can->Update();

  // ****** 2x3 ******
  for (unsigned int j=0; j<NHistos("2x3"); j++)
    {
      ROCMapping (PanelType, "2x3", j + 1 + ROCOffset (PanelType,"2x3"), &ROCPlaq, &ROCVect);
      (*HistoPlaq1D)["2x3"].can->cd(ROCPlaq+1);
      (*HistoPlaq1D)["2x3"].vec[ROCVect]->Draw();
      (*HistoPlaq2D)["2x3"].can->cd(ROCPlaq+1);
      (*HistoPlaq2D)["2x3"].vec[ROCVect]->Draw("gcolz"); // contz
    }
  (*HistoPlaq1D)["2x3"].can->Modified();
  (*HistoPlaq1D)["2x3"].can->Update();
  (*HistoPlaq2D)["2x3"].can->Modified();
  (*HistoPlaq2D)["2x3"].can->Update();

  // Plot auxiliary histograms
  Canvas[0]->cd(1);
  Histo1D[0]->Draw();
  Canvas[0]->cd(2);
  Histo2D[0]->Draw("contz");
  Canvas[0]->cd(3);
  Histo1D[1]->Draw();

  Canvas[0]->Modified();
  Canvas[0]->Update();
}


double Gaussian(double* x,
		double* par)
{
   double arg = 0;
   double val;

   if (par[2] != 0) arg = (x[0] - par[1])/par[2];
   val = par[0] * exp(-0.5 * arg * arg);

   return val;
}


double Landau(double* x,
	      double* par)
{
   double val;

   val = par[0] * TMath::Landau(x[0],par[1],par[2],kTRUE);

   return val;
}


void FitMe (string         PanelType,
	    Mappa1D*       HistoPlaq1D,
	    Mappa2D*       HistoPlaq2D,
	    vector<double> FittingPar,
	    string         FitFunc)
{
  if ((PanelType == "4L") || (PanelType == "4R"))
    {
      // ****** 1x2 ******
      for (unsigned int j=0; j<NHistos("1x2"); j++)
	{
	  if (FitFunc.compare("Gaussian") == 0)
	    {
	      (*HistoPlaq1D)["1x2"].fit.push_back(new TF1("FitFcn",Gaussian,(double)MINCHARGE,(double)MAXCHARGE,3));
	      (*HistoPlaq1D)["1x2"].fit.back()->SetParNames("Ampli","\\mu","\\sigma");
	    }
	  else if (FitFunc.compare("Landau") == 0)
	    {
	      (*HistoPlaq1D)["1x2"].fit.push_back(new TF1("FitFcn",Landau,(double)MINCHARGE,(double)MAXCHARGE,3));
	      (*HistoPlaq1D)["1x2"].fit.back()->SetParNames("Ampli","MPV","\\sigma");
	    }
	  
	  FittingPar[0] = (*HistoPlaq1D)["1x2"].vec[j]->GetMaximum();
	  FittingPar[0] = (*HistoPlaq1D)["1x2"].vec[j]->GetMaximumBin();
	  FittingPar[2] = (*HistoPlaq1D)["1x2"].vec[j]->GetRMS();

	  (*HistoPlaq1D)["1x2"].fit.back()->SetParameters(FittingPar[0], FittingPar[1], FittingPar[2]);
	  (*HistoPlaq1D)["1x2"].vec[j]->Fit("FitFcn","R");
	}
      
      // ****** 1x5 ******
      for (unsigned int j=0; j<NHistos("1x5"); j++)
	{
	  if (FitFunc.compare("Gaussian") == 0)
	    {
	      (*HistoPlaq1D)["1x5"].fit.push_back(new TF1("FitFcn",Gaussian,(double)MINCHARGE,(double)MAXCHARGE,3));
	      (*HistoPlaq1D)["1x5"].fit.back()->SetParNames("Ampli","\\mu","\\sigma");
	    }
	  else if (FitFunc.compare("Landau") == 0)
	    {
	      (*HistoPlaq1D)["1x5"].fit.push_back(new TF1("FitFcn",Landau,(double)MINCHARGE,(double)MAXCHARGE,3));
	      (*HistoPlaq1D)["1x5"].fit.back()->SetParNames("Ampli","MPV","\\sigma");
	    }
	  
	  FittingPar[0] = (*HistoPlaq1D)["1x5"].vec[j]->GetMaximum();
	  FittingPar[0] = (*HistoPlaq1D)["1x5"].vec[j]->GetMaximumBin();
	  FittingPar[2] = (*HistoPlaq1D)["1x5"].vec[j]->GetRMS();

	  (*HistoPlaq1D)["1x5"].fit.back()->SetParameters(FittingPar[0], FittingPar[1], FittingPar[2]);
	  (*HistoPlaq1D)["1x5"].vec[j]->Fit("FitFcn","R");
	}
    }
  else if ((PanelType == "3L") || (PanelType == "3R"))
    {
      // ****** 2x5 ******
      for (unsigned int j=0; j<NHistos("2x5"); j++)
	{
	  if (FitFunc.compare("Gaussian") == 0)
	    {
	      (*HistoPlaq1D)["2x5"].fit.push_back(new TF1("FitFcn",Gaussian,(double)MINCHARGE,(double)MAXCHARGE,3));
	      (*HistoPlaq1D)["2x5"].fit.back()->SetParNames("Ampli","\\mu","\\sigma");
	    }
	  else if (FitFunc.compare("Landau") == 0)
	    {
	      (*HistoPlaq1D)["2x5"].fit.push_back(new TF1("FitFcn",Landau,(double)MINCHARGE,(double)MAXCHARGE,3));
	      (*HistoPlaq1D)["2x5"].fit.back()->SetParNames("Ampli","MPV","\\sigma");
	    }
	 
	  FittingPar[0] = (*HistoPlaq1D)["2x5"].vec[j]->GetMaximum();
	  FittingPar[0] = (*HistoPlaq1D)["2x5"].vec[j]->GetMaximumBin();
	  FittingPar[2] = (*HistoPlaq1D)["2x5"].vec[j]->GetRMS();

	  (*HistoPlaq1D)["2x5"].fit.back()->SetParameters(FittingPar[0], FittingPar[1], FittingPar[2]);
	  (*HistoPlaq1D)["2x5"].vec[j]->Fit("FitFcn","R");
	}
    }
  
  // ****** 2x4 ******
  for (unsigned int j=0; j<NHistos("2x4"); j++)
    {
	  if (FitFunc.compare("Gaussian") == 0)
	    {
	      (*HistoPlaq1D)["2x4"].fit.push_back(new TF1("FitFcn",Gaussian,(double)MINCHARGE,(double)MAXCHARGE,3));
	      (*HistoPlaq1D)["2x4"].fit.back()->SetParNames("Ampli","\\mu","\\sigma");
	    }
	  else if (FitFunc.compare("Landau") == 0)
	    {
	      (*HistoPlaq1D)["2x4"].fit.push_back(new TF1("FitFcn",Landau,(double)MINCHARGE,(double)MAXCHARGE,3));
	      (*HistoPlaq1D)["2x4"].fit.back()->SetParNames("Ampli","MPV","\\sigma");
	    }

	  FittingPar[0] = (*HistoPlaq1D)["2x4"].vec[j]->GetMaximum();
	  FittingPar[0] = (*HistoPlaq1D)["2x4"].vec[j]->GetMaximumBin();
	  FittingPar[2] = (*HistoPlaq1D)["2x4"].vec[j]->GetRMS();

	  (*HistoPlaq1D)["2x4"].fit.back()->SetParameters(FittingPar[0], FittingPar[1], FittingPar[2]);
	  (*HistoPlaq1D)["2x4"].vec[j]->Fit("FitFcn","R");
    }

  // ****** 2x3 ******
  for (unsigned int j=0; j<NHistos("2x3"); j++)
    {
	  if (FitFunc.compare("Gaussian") == 0)
	    {
	      (*HistoPlaq1D)["2x3"].fit.push_back(new TF1("FitFcn",Gaussian,(double)MINCHARGE,(double)MAXCHARGE,3));
	      (*HistoPlaq1D)["2x3"].fit.back()->SetParNames("Ampli","\\mu","\\sigma");
	    }
	  else if (FitFunc.compare("Landau") == 0)
	    {
	      (*HistoPlaq1D)["2x3"].fit.push_back(new TF1("FitFcn",Landau,(double)MINCHARGE,(double)MAXCHARGE,3));
	      (*HistoPlaq1D)["2x3"].fit.back()->SetParNames("Ampli","MPV","\\sigma");
	    }

	  FittingPar[0] = (*HistoPlaq1D)["2x3"].vec[j]->GetMaximum();
	  FittingPar[0] = (*HistoPlaq1D)["2x3"].vec[j]->GetMaximumBin();
	  FittingPar[2] = (*HistoPlaq1D)["2x3"].vec[j]->GetRMS();

	  (*HistoPlaq1D)["2x3"].fit.back()->SetParameters(FittingPar[0], FittingPar[1], FittingPar[2]);
	  (*HistoPlaq1D)["2x3"].vec[j]->Fit("FitFcn","R");
    }
} 


int  main (int argc, char **argv)
{   
  vector<string>   ParVec;
  ifstream*        inputFile;
  unsigned int     FEDchn;
  unsigned int     ROCID;
  bool             Log;
  string           PanelType;
  string           PlaqType;
  string           FitFunc;
  string           inputFilename;

  vector<TCanvas*> Canvas;
  vector<TH1D*>    Histo1D;
  vector<TH2D*>    Histo2D;
  vector<double>   FittingPar;

  Mappa1D HistoPlaq1D;
  Mappa2D HistoPlaq2D;

  SLinkData        pixelData;

  int Event = 0;
  int NEVENTS;

  if (argc == 2)
    {
      ReadParameters * ParFile = new ReadParameters(argv[1]);
      ParFile->ReadFromFile(&ParVec);

      inputFilename = ParVec[0];
      FEDchn        = atoi(ParVec[1].c_str());
      PanelType     = ParVec[2];
      PlaqType      = ParVec[3];
      ROCID         = atoi(ParVec[4].c_str());
      NEVENTS       = atoi(ParVec[5].c_str());
      FitFunc       = ParVec[6];
      FittingPar.push_back (atof(ParVec[7].c_str()));
      FittingPar.push_back (atof(ParVec[8].c_str()));
      FittingPar.push_back (atof(ParVec[9].c_str()));
      Log           = atoi(ParVec[10].c_str());

      TApplication theApp ("Applications", &argc, argv);

      if (OpenInputFile (inputFilename.c_str(), &inputFile) == false)
 	{
 	  cout << "File full of crap!" << endl;
 	  return 1;
 	}

      CreateHistos (&Canvas,
		    &Histo1D,
		    &Histo2D,
		    &HistoPlaq1D,
		    &HistoPlaq2D,
		    PanelType,
		    NEVENTS);

      cout << "*** Begin processing ***" << endl;
      while ((Event < NEVENTS) && (EOFile(inputFile) == false))
	{
	  if (NewData (inputFile, &pixelData) == true)
	    {
	      Event++;

	      FillHistos (FEDchn,
			  ROCID,
			  PanelType,
			  PlaqType,
			  pixelData,
			  Canvas,
			  Histo1D,
			  Histo2D,
			  &HistoPlaq1D,
			  &HistoPlaq2D,
			  Event,
			  Log);
	    }
	}      
      cout << "*** End processing";
      if (Event == NEVENTS)
	{
	  cout << " because I read " << NEVENTS;
	}
      else
	{
	  cout << " because I reached the end of the file with " << Event;
	}
      cout << " events ***" << endl;
      
      CloseInputFile (inputFile);

      RescaleHistos (PanelType,
		     Histo1D,
		     Histo2D,
		     &HistoPlaq1D,
		     &HistoPlaq2D);

      cout << "*** Start fitting ***" << endl;
      FitMe (PanelType,
	     &HistoPlaq1D,
	     &HistoPlaq2D,
	     FittingPar,
	     FitFunc);
      cout << "*** End fitting ***" << endl;

      DrawAll (PanelType,
	       Canvas,
	       Histo1D,
	       Histo2D,
	       &HistoPlaq1D,
	       &HistoPlaq2D);
      
      theApp.Run (); // Eventloop on air
    }  
  else
    {  
      cout << "\nERROR calling SLinkOnlineMonitor !" << endl;
      cout << "Synopsis: ./SLinkOnlineMonitor parameters_file_name\n" << endl;
    }
  
  return 0;
}
