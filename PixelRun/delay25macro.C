//////////////////////////////////////////////////////////////////////
//                                                                  //
// ROOT macro to make a .ps file of the plots from delay25.root     //
// Author: Jennifer Sibille             July 2008                   //
//                                                                  //
// USAGE: .L delay25macro.C                                         //
//        delay25macro(<run_number>)                                //
//                                                                  //
//////////////////////////////////////////////////////////////////////

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include "TFile.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TLegend.h"



void delay25macro(const int run) {

TFile * f = new TFile(Form("DATA/Run_%i/delay25.root",run));
TCanvas * c = new TCanvas("c","Delay25",1);
TCanvas * g = new TCanvas("g","Empty",1);

c->Divide(3,2);
c->Print(Form("DATA/Run_%i/delay25.ps[",run));

//Go into BPix dir
TDirectory *first_dir = BPix;
first_dir->cd();


//Go into the quadrant dir (BmI, BmO, BpI, BpO)
TList* list = first_dir->GetListOfKeys();
for (int i = 0; i < list->GetSize(); i++) 
{
	TKey *key = dynamic_cast<TKey*>(list->At(i));
	TObject* obj = key->ReadObj();
	if (obj->InheritsFrom("TDirectory")) 
	{
		TDirectory *curr_dir = dynamic_cast<TDirectory*>(obj);
		curr_dir->cd();

		//Go into sector dir
		TList* list1 = curr_dir->GetListOfKeys();
		for (int j = 0; j < list1->GetSize(); j++) 
		{
			TKey *key1 = dynamic_cast<TKey*>(list1->At(j));
			TObject* obj1 = key1->ReadObj();
			if(obj1->InheritsFrom("TDirectory"))
			{
				TDirectory *curr_dir1 = dynamic_cast<TDirectory*>(obj1);
				curr_dir1->cd();

				//Go into portcard dir
				TList* list2 = curr_dir1->GetListOfKeys();
				for (int k = 0; k < list2->GetSize(); k++) 
				{
					TKey *key2 = dynamic_cast<TKey*>(list2->At(k));
					TObject* obj2 = key2->ReadObj();
					if (obj2->InheritsFrom("TDirectory"))
					{
						TDirectory * curr_dir2 = dynamic_cast<TDirectory*>(obj2);
						curr_dir2->cd();
						
						
						//Get canvases
						int pad, counter = 0;
						TList* list3 = curr_dir2->GetListOfKeys();

						for (int k = 0; k < list3->GetSize(); k++) 
						{
							TKey *key3 = dynamic_cast<TKey*>(list3->At(k));
							TObject* obj3 = key3->ReadObj();
							g = (TCanvas *)obj3;
							if (k < 6)
							{
								//cout << "k = " << k << endl;
								pad = k+1;
								//cout << "Drawing to pad " << pad << endl;
							}
							if (k >=6 && k<12)
							{
								//cout << "k = " << k << endl;
								pad = k-5;
								//cout << "Drawing to pad " << pad << endl;
							}
							c->cd(pad);
							g->DrawClonePad();
							if (k == 5 || k == 11) 
							{
								c->Print(Form("DATA/Run_%i/delay25.ps",run));
								if (k == 5)
								{
									c->Clear();
									c->Divide(3,2);
								}
							}
						}
					}
				}
			}
		}
	}
}

c->Print(Form("DATA/Run_%i/delay25.ps]",run));

}

