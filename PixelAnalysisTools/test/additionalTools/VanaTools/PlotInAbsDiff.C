
void PlotInAbsDiff(){

  gStyle->SetOptStat(111111);
  
  ifstream file_b( "../0deg_iter14/0deg_iter14_inabsdiff_BPix.log" , std::ios::in);
  ifstream file_f( "../0deg_iter14/0deg_iter14_inabsdiff_FPix.log" , std::ios::in);

  if(!file_b.good()){
    cout << "BPix file could not be opened" << endl;
    return;
  }
 
  if(!file_f.good()){
    cout << "FPix file could not be opened" << endl;
    return;
  }

  TH1D* hist_b = new TH1D("hist_b", "", 40, 0, 20); 
  TH1D* hist_f = new TH1D("hist_f", "", 40, 0, 20); 
  
  string var1;
  double var2;

  while(file_b>>var1>>var2){
   hist_b->Fill(var2);
  }

  while(file_f>>var1>>var2){
   hist_f->Fill(var2);
  }

  hist_b->Draw();
  hist_f->Draw("same");

}


