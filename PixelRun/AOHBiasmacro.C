{
///////////////////////////////////////////////////////////////////////////////////////
//                                                                                   //
// ROOT macro to make a .ps file of the plots from AOHBiasCalibration_FEDCrate1.root //
// Author: Jennifer Sibille    July 2008                                             //
//                                                                                   //
// USAGE: .x AOHBiasmacro.C                                                          //
//                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////


int layer, run, fedNum;
cout << "Enter the run number: ";
cin >> run;
cout << "What layer are you testing? Enter 1 for layers 1/2 and enter 3 for layer 3." << endl;
cin >> layer;
cout << "Enter FED #" << endl;
cin >> fedNum;


TCanvas * g = new TCanvas("g","Close me",1);
TFile * f = new TFile(Form("DATA/Run_%i/AOHBiasCalibration_FEDCrate1.root",run));
	
int channel = 1;

if (layer == 1)
{
	TCanvas * c1 = new TCanvas("c1","Channels 1-6",1);
	TCanvas * c2 = new TCanvas("c2","Channels 7-12",1);
	TCanvas * c3 = new TCanvas("c3","Channels 13-18",1);
	TCanvas * c4 = new TCanvas("c4","Channels 19-24",1);
	
	
	c1->Divide(3,2);
	c2->Divide(3,2);
	c3->Divide(3,2);
	c4->Divide(3,2);
	
	for (int pad = 1; pad < 7; pad++)
	{
		g = (TCanvas*)f->Get(Form("FED %i, channel %i: TBM B & UB vs AOH bias;1", fedNum, pad));
		if(g != NULL)
		{	
			c1->cd(pad);
			g->DrawClonePad();	
		}
	
		g = (TCanvas*)f->Get(Form("FED %i, channel %i: TBM B & UB vs AOH bias;1", fedNum, pad+6));
		if(g != NULL)
		{	
			c2->cd(pad);
			g->DrawClonePad();	
		}
	
		g = (TCanvas*)f->Get(Form("FED %i, channel %i: TBM B & UB vs AOH bias;1", fedNum, pad+12));
		if(g != NULL)
		{	
			c3->cd(pad);
			g->DrawClonePad();	
		}
	
		g = (TCanvas*)f->Get(Form("FED %i, channel %i: TBM B & UB vs AOH bias;1", fedNum, pad+18));
		if(g != NULL)
		{	
			c4->cd(pad);
			g->DrawClonePad();	
		}
		channel ++;
	}
	
	c1->Print(Form("DATA/Run_%i/AOHBiasCalibration_FEDCrate1.ps(",run));
	c2->Print(Form("DATA/Run_%i/AOHBiasCalibration_FEDCrate1.ps",run));
	c3->Print(Form("DATA/Run_%i/AOHBiasCalibration_FEDCrate1.ps",run));
	c4->Print(Form("DATA/Run_%i/AOHBiasCalibration_FEDCrate1.ps)",run));
}

else if (layer == 3)
{

	TCanvas * c1 = new TCanvas("c1","Channels 25-30",1);
	TCanvas * c2 = new TCanvas("c2","Channels 31-36",1);

	c1->Divide(3,2);
	c2->Divide(3,2);

	for (int pad = 1; pad < 7; pad++)
	{
		g = (TCanvas*)f->Get(Form("FED %i, channel %i: TBM B & UB vs AOH bias;1", fedNum, pad+24));
		if(g != NULL)
		{	
			c1->cd(pad);
			g->SetMarkerSize(.8);
			g->DrawClonePad("CP");	
		}
	
		g = (TCanvas*)f->Get(Form("FED %i, channel %i: TBM B & UB vs AOH bias;1", fedNum, pad+30));
		if(g != NULL)
		{	
			c2->cd(pad);
			g->SetMarkerSize(.8);
			g->DrawClonePad("CP");	
		}
	
		channel ++;
	}
	
	c1->Print(Form("DATA/Run_%i/AOHBiasCalibration_FEDCrate1.ps(",run));
	c2->Print(Form("DATA/Run_%i/AOHBiasCalibration_FEDCrate1.ps)",run));

}

else
cout << "Invalid entry." << endl;
	
}
