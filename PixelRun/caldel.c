void caldel()
{
//   example of macro to read data from an ascii file and
//   create a root file with an histogram and an ntuple.



  int nx=3;
  int ny=4;
  TCanvas* c=new TCanvas("c","c", 700,800);
  c->Divide(nx, ny);

  int nplot=nx*ny;

  ifstream insummary;

  insummary.open("CalDelCalibrationSummary.txt");

  string fname;

  insummary>>fname;

  unsigned int roccounter=0;

  while (insummary.good()){
    
    //std::cout << "Will now process file:"<<fname<<std::endl;

    ifstream in;

    in.open(fname.c_str());

    assert(in.good());
 
    TString title;

    TString name1, name2;
    Int_t nbins1, nbins2;
    Double_t min1, max1;
    Double_t min2, max2;

    in >> title;
    in >> name1;
    in >> nbins1;
    in >> min1;
    in >> max1;
    
    in >> name2;
    in >> nbins2;
    in >> min2;
    in >> max2;

    int nleft, nright;
    double slopeleft, sloperight, slope;

    in >> nleft;
    in >> nright;
    in >> slopeleft;
    in >> sloperight;
    in >> slope;

  //cout << "title, nbins1, nbins2:"<<title<<" "<<nbins1<<" "<<nbins2<<endl;

    TH2F *eff;
    eff= new TH2F(title,title,nbins1,min1,max1,
		  nbins2,min2,max2);

    eff->GetXaxis()->SetTitle(name1);
    eff->GetYaxis()->SetTitle(name2); 

    for(int ibin1=0;ibin1<nbins1;ibin1++){
      for(int ibin2=0;ibin2<nbins2;ibin2++){
	double e;
	in >> e;
	eff->Fill(min1+(ibin1+0.5)*(max1-min1)/nbins1,
		  min2+(ibin2+0.5)*(max2-min2)/nbins2,
		  e);
      }
    }
    
    in.close();

    roccounter++;


    //std::cout << "cd:ing to:"<<1+(roccounter%nplot)<<std::endl;
    
    c->cd(1+((roccounter-1)%nplot));
   

    eff->SetMinimum(0.0);
    eff->SetMaximum(1.0);
    eff->Draw("colz");


    if(roccounter%12==0){
      
      if (roccounter==12) {
	c->Print("CalDelCalibration.ps(");
      } else {
	c->Print("CalDelCalibration.ps");
      }

      TCanvas* c=new TCanvas("c","c", 700,800);
      c->Divide(nx, ny);

    }

    insummary>>fname;

  }

  c->Print("CalDelCalibration.ps)");

}

