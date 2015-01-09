//////////////////////////////////////////////////////////////////////
//                                                                  //
// ROOT macro to make a .ps file of the plots from VcThrCalDel.root //
// Author: Jennifer Sibille   July 2008                             //
//                                                                  //
// USAGE: .L VcThrmacro.C                                           //
//        VcThrmacro(<run_number>)                                  //
//                                                                  //
// This should be run before doing the update, which moves the      //
// files containing the marker position, so that the new and old    //
// positions can be seen.  In this case the new position will be    //
// a black cross and the old one a grey cross.  If the update is    //
// done first only the new one will be displayed, as a grey cross.  //
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

void VcThrmacro(const int run) {

string fname, f2name;
fname = Form("DATA/Run_%i/VcThrCalDel.root",run);
TFile* fFile = TFile::Open(fname.c_str());
if (!fFile) 
{
	cout << " Can not open the file " << fname << endl;
	return;
}
ifstream input_new;
ifstream input_old;

//Open .ps file
TCanvas * g = new TCanvas("g","Close me",1);
TCanvas * c1 = new TCanvas("c1","VcalThrCalDel",1);
c1->Divide(4,4);
c1->Print(Form("DATA/Run_%i/VcThrCalDel.ps[",run));

//Go into BPix dir
TDirectory *first_dir = BPix;
first_dir->cd();
string bpix = first_dir->GetName();


//Go into the quadrant dir (BmI, BmO, BpI, BpO)
TList* list = first_dir->GetListOfKeys();
for (int i = 0; i < list->GetSize(); i++) 
{//1
	TKey *key = dynamic_cast<TKey*>(list->At(i));
	TObject* obj = key->ReadObj();
	if (obj->InheritsFrom("TDirectory")) 
	{//2
		TDirectory *curr_dir = dynamic_cast<TDirectory*>(obj);
		curr_dir->cd();
		string dir0 = curr_dir->GetName();

		//Go into sector dir
		TList* list1 = curr_dir->GetListOfKeys();
		for (int j = 0; j < list1->GetSize(); j++) 
		{//3
			TKey *key1 = dynamic_cast<TKey*>(list1->At(j));
			TObject* obj1 = key1->ReadObj();
			if(obj1->InheritsFrom("TDirectory"))
			{//4
				TDirectory *curr_dir1 = dynamic_cast<TDirectory*>(obj1);
				curr_dir1->cd();
				string dir1 = curr_dir1->GetName();

				//Go into layer dir
				TList* list2 = curr_dir1->GetListOfKeys();
				for (int k = 0; k < list2->GetSize(); k++) 
				{//5
					TKey *key2 = dynamic_cast<TKey*>(list2->At(k));
					TObject* obj2 = key2->ReadObj();
					if (obj2->InheritsFrom("TDirectory"))
					{//6
						TDirectory * curr_dir2 = dynamic_cast<TDirectory*>(obj2);
						curr_dir2->cd();
						string dir2 = curr_dir2->GetName();
						
						//Go into ladder dir
						TList* list3 = curr_dir2->GetListOfKeys();
						for (int l = 0; l < list3->GetSize(); l++) 
						{//7
							TKey *key3 = dynamic_cast<TKey*>(list3->At(l));
							TObject* obj3 = key3->ReadObj();
							if (obj3->InheritsFrom("TDirectory"))
							{//8
								TDirectory * curr_dir3 = dynamic_cast<TDirectory*>(obj3);
								curr_dir3->cd();
								string dir3 = curr_dir3->GetName();

								//Go into module dir
								TList* list4 = curr_dir3->GetListOfKeys();
								for (int m = 0; m < list4->GetSize(); m++) 
								{//9
									TKey *key4 = dynamic_cast<TKey*>(list4->At(m));
									TObject* obj4 = key4->ReadObj();
									if (obj4->InheritsFrom("TDirectory"))
									{//10
										TDirectory * curr_dir4 = dynamic_cast<TDirectory*>(obj4);
										curr_dir4->cd();
										string dir4 = curr_dir4->GetName();
										//Open file with new marker values for current module
										input_new.open(Form("DATA/Run_%i/ROC_DAC_module_%s_%s_%s_%s_%s_%s.dat", run, bpix, dir0, dir1, dir2, dir3, dir4));
										input_old.open(Form("PixelConfigDataExamples_final_psi/dac/4/ROC_DAC_module_%s_%s_%s_%s_%s_%s.dat", bpix, dir0, dir1, dir2, dir3, dir4));
										
										//Go into roc dir
										TList* list5 = curr_dir4->GetListOfKeys();
										for (int n = 0; n < list5->GetSize(); n++) 
										{//11
											TKey *key5 = dynamic_cast<TKey*>(list5->At(n));
											TObject* obj5 = key5->ReadObj();
											if (obj5->InheritsFrom("TDirectory"))
											{//12
												TDirectory * curr_dir5 = dynamic_cast<TDirectory*>(obj5);
												curr_dir5->cd();
												string dir5 = curr_dir5->GetName();
												
												//Get histos and save to file
												string histo = bpix+"_"+dir0+"_"+dir1+"_"+dir2+"_"+dir3+"_"+dir4+"_"+dir5;
												//cout << histo << endl;
												g = (TCanvas*)curr_dir5->Get(Form("%s;1", histo));
												if(g != NULL)
												{//13
													//cout << "g is there!" << endl;
													gStyle->SetPalette(1,0);
													c1->cd(n+1);
													g->Draw("colz");
													
													// Get marker position from file
													string dummy;
													int vcthr = 0, caldel = 0, vcthr_old = 0, caldel_old = 0;
														while (!input_new.eof())
														{
															if (!input_new.is_open()) break;
															input_new >> dummy;
															if (dummy == "ROC:")
															{
																//cout << dummy;
																input_new >> dummy;
																if (dummy == histo)
																{
																	//cout << dummy << endl;
																	while ( dummy != "ROC:")
																	{
																		input_new >> dummy;
																		//cout << dummy << endl;
																		if (dummy == "VcThr:")
																		{
																			input_new >> vcthr;
																			//cout << "VcThr = " << vcthr << endl;
																		}
																		else if (dummy == "CalDel:")
																		{
																			input_new >> caldel;
																			//cout << "CalDel = " << caldel << endl;
																			break;
																		}
																	}
																	if (vcthr !=0 && caldel !=0) break; 
																}
															}
														}
													while (!input_old.eof())
													{
														input_old >> dummy;
														if (dummy == "ROC:")
														{
															cout << dummy;
															input_old >> dummy;
															if (dummy == histo)
															{
																cout << dummy << endl;
																while ( dummy != "ROC:")
																{
																	input_old >> dummy;
																	//cout << "old file: " << dummy << endl;
																	if (dummy == "VcThr:")
																	{
																		input_old >> vcthr_old;
																		//cout << "VcThr_old = " << vcthr_old << endl;
																	}
																	else if (dummy == "CalDel:")
																	{
																		input_old >> caldel_old;
																		//cout << "CalDel_old = " << caldel_old << endl;
																		break;
																	}
																}
																if (vcthr_old !=0 && caldel_old !=0) break; 
															}
														}
													}
													TMarker *mark = new TMarker(caldel, vcthr, 2);
													mark->Draw();
													TMarker *old_mark = new TMarker(caldel_old, vcthr_old, 2);
													old_mark->SetMarkerColor(15);
													old_mark->Draw();
												}//13
												else cout << "Histogram is not there!" << endl;
											}//12
										}//11
										input_new.close();
										input_old.close();
										c1->Print(Form("DATA/Run_%i/VcThrCalDel.ps",run));
									}//10
								}//9
							}//8
						}//7
					}//6
				}//5
			}//4
		}//3
	}//2
}//1
c1->Print(Form("DATA/Run_%i/VcThrCalDel.ps]",run));


}

