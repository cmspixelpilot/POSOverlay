{
//
// To see the output of this macro, click here.

//

gROOT->Reset();

//Set this bool to true if the iana.dat file was generated with the new
//parameterization (post-POS_2_9_3) of the fit function
//If the file was generated with POS_2_9_3 or earlier, set this bool to false
 const bool newFitMethod = false;
 ifstream in("/nfshome0/pixelpro/TriDAS/pixel/PixelRun/Runs/Run_65000/Run_65504/iana.dat");

ofstream out("ianamax.dat");

assert(in.good());

TString rocname;

in >> rocname;

int plotnumber=0;

c1 = new TCanvas("c1","Iana vs. Vana",200,10,600,800);
c1->Divide(2,4);
c1->SetFillColor(42);
c1->SetGrid();

while (!in.eof()&&plotnumber<1000){



  const int nmax = 256;
  double x[nmax], y[nmax], ey[nmax];

  int n;

  in >> n;

  for (int ivana=0;ivana<n;ivana++){
    x[ivana]=10.0*ivana;
    in >> y[ivana];
    ey[ivana]=2.0;
  }

  if (false &&
      !rocname.Contains("BmI")
      //rocname!="BPix_BpI_SEC2_LYR3_LDR4F_MOD1_ROC0"&&
      //rocname!="BPix_BmO_SEC2_LYR3_LDR4F_MOD1_ROC0"&&
      //rocname!="BPix_BmO_SEC2_LYR3_LDR4F_MOD1_ROC2"&&
      //rocname!="BPix_BmO_SEC2_LYR3_LDR4F_MOD1_ROC8"
      ) {

    TString tmp;
    in >> tmp;
    in >> tmp;
    in >> tmp;
    in >> tmp;
    in >> tmp;
    in >> tmp;
    in >> tmp;
    in >> tmp;
    in >> tmp;
    rocname=tmp;
    
    continue;

  } 

  plotnumber++;

  cout << "Processing ROC name:"<<rocname<<endl;
 
  c1->cd(1+(plotnumber-1)%8);


  TF1* f2=0;
  if (newFitMethod) f2 = new TF1("f2","(x<[0])*([2]+([3]-[2])*exp(([4]-[3])*(x-[0])/(([1])*([3]-[2]))))+(x>=[0]+[1])*[4]+(x>=[0])*(x<[0]+[1])*([3]+(x-[0])*([4]-[3])/([1]))",0.0,250.0);
  else   f2 = new TF1("f2","(x<[0])*([2]+([3]-[2])*exp(([4]-[3])*(x-[0])/(([1]-[0])*([3]-[2]))))+(x>[1])*[4]+(x>[0])*(x<[1])*([3]+(x-[0])*([4]-[3])/([1]-[0]))",0.0,250.0);

  double yvalatzero;
  
  in >> yvalatzero;

  cout << "yvalatzero " << yvalatzero <<endl; 

  for (int ivana=0;ivana<n;ivana++){
    y[ivana]=(y[ivana]-yvalatzero)*1000;
  }


  double p0,p1,p2,p3,p4;
  
  in >> p0;
  in >> p1;
  in >> p2;
  in >> p3;
  in >> p4;

  cout << "p0..p4:"<<p0<<" "<<p1<<" "<<p2<<" "<<p3<<" "<<p4<<endl;

  double vanaold;
  double vananew;

  in >> vanaold;
  in >> vananew;

  cout << "vananew:"<<vananew<<endl;

  f2->SetParameters(p0,p1,p2,p3,p4);


  TGraphErrors* gr = new TGraphErrors(n,x,y,0,ey);

  gr->SetLineColor(2);
  gr->SetLineWidth(4);
  gr->SetMarkerColor(4);
  gr->SetMarkerStyle(21);
  gr->SetTitle(rocname);
  gr->SetMinimum(-10.0);
  gr->GetXaxis()->SetTitle("Vana");
  gr->GetYaxis()->SetTitle("Iana (mA)");
  gr->Draw("ACP");

  //TF1 *fit = gr->GetFunction("f2");
  f2->Draw("SAME");
  f2->SetLineColor(1);
   
                             
  double ianacurrent=f2->Eval(vanaold);          
  l=new TLine(vanaold,-10.0,vanaold,ianacurrent);
  l->SetLineColor(1);
  l->Draw();
  l=new TLine(0.0,ianacurrent,vanaold,ianacurrent);
  l->SetLineColor(1);
  l->Draw();

  double  iananew=f2->Eval(vananew);
  
  l=new TLine(vananew,-10.0,vananew,iananew);
  l->SetLineColor(2);
  l->Draw();
  l=new TLine(0.0,iananew,vananew,iananew);
  l->SetLineColor(2);
  l->Draw();

  double  ianamax=f2->Eval(250);

  cout << "Ianamax: "<<rocname<<" "<<ianamax<<endl; 
  out << "Ianamax: "<<rocname<<" "<<ianamax<<" "<<ianacurrent<<endl; 

  if (plotnumber%8==0) {

    if (plotnumber==8) {
      c1->Print("iana.ps(");
    }
    else {
      c1->Print("iana.ps");
    }

    c1 = new TCanvas("c1","Iana vs. Vana",200,10,600,800);
    c1->Divide(2,4);
    c1->SetFillColor(42);
    c1->SetGrid();
  }

  TString tmp;

  in >> tmp;

  rocname=tmp;

}

c1->Print("iana.ps)");

}

