
void PlotAbsThr(){

  gStyle->SetOptStat(111111);
  
  ifstream file_b( "../0deg_iter14/0deg_iter14_absThr_BPix.log" , std::ios::in);
  ifstream file_f( "../0deg_iter14/0deg_iter14_absThr_FPix.log" , std::ios::in);

  if(!file_b.good()){
    cout << "BPix file could not be opened" << endl;
    return;
  }
 
  if(!file_f.good()){
    cout << "FPix file could not be opened" << endl;
    return;
  }

  TH1D* hist_b = new TH1D("hist_b", "BPix", 40, 50, 90); 
  TH1D* hist_f = new TH1D("hist_f", "FPix", 40, 50, 90); 
  
  string var1;
  double var2;

  while(file_b>>var1>>var2){
   hist_b->Fill(var2);
  }

  while(file_f>>var1>>var2){
   hist_f->Fill(var2);
  }

  TCanvas *c1 = new TCanvas();
  c1->SetFillColor(10);
  c1->Divide(1,2);

  c1->cd(1);
  hist_b->Draw();
  c1->cd(2);
  hist_f->Draw("same");
  c1->cd(0);

}


