#include <iostream>
#include <fstream>
#include <string>
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TMarker.h"

using namespace std;

int main(int argv, char** argc)
{
  string filename;
  if(argv != 2){
    cout << "[main()]\tUsage: Delay25 filename" << endl;
    exit(0);
  }  
  else{
    filename = argc[1];
  }

  TCanvas* c=new TCanvas("c","c", 700,800);

  ifstream delayin;

  delayin.open(filename.c_str());
  assert(delayin.good());

  Int_t origSDa, origRDa, rangeSDa, rangeRDa, gridSize, tests;
  string tmp, portcardname, modulename;

  delayin >> tmp;
  delayin >> portcardname;
  delayin >> tmp;
  delayin >> modulename;
  delayin >> tmp;
  delayin >> origSDa;
  delayin >> tmp;
  delayin >> origRDa;
  delayin >> tmp;
  delayin >> rangeSDa;
  delayin >> tmp;
  delayin >> rangeRDa;
  delayin >> tmp;
  delayin >> gridSize;
  delayin >> tmp;
  delayin >> tests;
  //cout << "I read the grid size as " << gridSize << endl;

  Int_t maxSDa, maxRDa;
  maxSDa = origSDa + rangeSDa;
  maxRDa = origRDa + rangeRDa;
  string longtitle = "RDa vs. SDa for portcard "+portcardname+" and module "+modulename;

  TH2F * paramspace = new TH2F("h2", longtitle.c_str(), rangeSDa, origSDa, maxSDa, rangeRDa, origRDa, maxRDa);
  paramspace->GetXaxis()->SetTitle("SDa");
  paramspace->GetYaxis()->SetTitle("RDa");

  paramspace->Draw("colz");

  delayin >> tmp;
  assert(tmp=="GridScan:");
  //cout << "Reading results of grid scan." << endl;
  Int_t sda, rda, number;
  delayin >> tmp;
  while((tmp!="SelectedPoint:")&&(tmp!="NoStableRegion"))
    {
      sda = atoi(tmp.c_str());
      delayin >> tmp;
      rda = atoi(tmp.c_str());
      delayin >> tmp;
      number = atoi(tmp.c_str());
      if(number>0)
	{
	  TMarker * mk = new TMarker(sda, rda, 20);
	  mk->SetMarkerSize((number*2.0)/tests);
	  mk->Draw();
	}
      else
	{
	  TMarker * mk = new TMarker(sda, rda, 1);
	  mk->Draw();
	}
      //cout << "I saw the line " << sda << " " << rda << " " << number << endl;
      delayin >> tmp;
      //cout << "The next token I'm reading is " << tmp << endl;
    }

  cout << "tmp"<<tmp<<endl;

  if(tmp=="SelectedPoint:")
    {
      //cout << "Reading selected point." << endl;
      delayin >> tmp;
	  sda = atoi(tmp.c_str());
	  delayin >> tmp;
	  rda = atoi(tmp.c_str());
	  delayin >> tmp;
	  number = atoi(tmp.c_str());
	  TMarker * mk = new TMarker(sda, rda, 21);
	  mk->SetMarkerSize(number/tests*2);
	  mk->SetMarkerColor(2);
	  mk->Draw();
	  //cout << "I saw the line " << sda << " " << rda << " " << number << endl;
    }
  else
    {
      cout << "No stable region found." << endl;
    }

  int last = filename.find_last_of(".");
  string basename = filename.substr(0,last);
  //cout << "I got the string " << basename << endl;
  string pdfname = basename+".pdf";
  c->Print(pdfname.c_str());
  return 1;
}

