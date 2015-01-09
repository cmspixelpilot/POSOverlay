
void hist(){
  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(111111);

  
  //ifstream file1( "../0deg_iter7/0deg_iter7_deltaVana_corr.log" , std::ios::in);
  ifstream file1( "../0deg_iter19/0deg_iter19_deltaVana.log" , std::ios::in);
  double scale = 2.;

  if(!file1.good()){
    cout << "file could not be opened" << endl;
    return;
  }
  
  int nOutliers(0);

  TH1D* hist1 = new TH1D("h1", "h1", 40, -20, 20); 
  
  string var1;
  double var2;
  while(file1>>var1>>var2){
   hist1->Fill(var2*2);
   if ( fabs(var2) > 5 ) nOutliers++;
  }

  hist1->Draw();

  cout << "\nNumber of ROCs with |deltaVana| > 5 = " << nOutliers << "\n" << endl;

}


