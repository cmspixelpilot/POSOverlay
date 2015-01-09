
void PlotDeltaVanaVsIter() {

  gROOT->Reset();
  gStyle->SetOptStat(0);

  const int nIter(20);
  const int nROCs(15395);

  TString dummy;
  TString ROCnames[nROCs];
  bool toPlot[nROCs];

  int oldVana[nROCs][nIter];
  double currThr[nROCs][nIter];
  double deltaVana[nROCs][nIter];

  TString inFile1 = "../0deg_iter";
  TString inFile2 = "/0deg_iter";
  TString inFile3 = "_oneLiner.log";

  // initialize toPlot
  for (int i = 0; i < nROCs ; i++) {
    toPlot[i] = false;
  }


  for (int i = 0; i < nIter; i++) {

    stringstream thisIter; 
    thisIter << i ; 
    TString tIter = thisIter.str();
    
    TString inFile = inFile1+tIter+inFile2+tIter+inFile3;
    ifstream inStream(inFile, std::ios::in);

    if(!inStream.good()){
      cout << "file could not be opened" << endl;
      return;
    }

    for (int j = 0 ; j < nROCs ; j++) {

      inStream >> dummy >> ROCnames[j] >> oldVana[j][i] >> currThr[j][i] >> deltaVana[j][i] ;
      if ( i > 2 && fabs(deltaVana[j][i]) > 5. ) toPlot[j] = true; 
      //if ( i > 16 && fabs(deltaVana[j][i]) > 1.5 ) toPlot[j] = true; 

    }
  }


  TCanvas *c1 = new TCanvas();
  c1->SetFillColor(10);

  TH1F *hDeltaVana[nROCs];
  TH1F *dummyHist = new TH1F("dummyHist","ROCs with DeltaVana > 5 after iter 2",nIter,-0.5,nIter-0.5);

  dummyHist->SetBinContent(1,-8);
  dummyHist->SetBinContent(nIter, 9);
  
  dummyHist->GetXaxis()->SetTitle("iteration");
  dummyHist->GetYaxis()->SetTitle("deltaVana");
  dummyHist->SetLineColor(10);
  dummyHist->Draw();


  for (int j = 0 ; j < nROCs ; j++) {

    if (toPlot[j] == false) continue;

    stringstream thisRoc; 
    thisRoc << j ; 
    TString tRoc = thisRoc.str();
    
    hDeltaVana[j] = new TH1F("hDeltaVana_"+tRoc,ROCnames[j],nIter,-0.5,nIter-0.50);

    for (Int_t i = 0 ; i < nIter ; i++ ) {
      hDeltaVana[j]->SetBinContent(i+1,deltaVana[j][i]);
    }

    hDeltaVana[j]->SetLineColor(j%128);
    hDeltaVana[j]->Draw("lsame");

  }

  TF1 *const0 = new TF1("const0","0",0,20);
  const0->SetLineWidth(3);
  const0->SetLineStyle(2);
  const0->Draw("same");

}
