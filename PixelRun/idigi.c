{
//
// To see the output of this macro, click here.

//

gROOT->Reset();


ifstream in("idigi.dat");

assert(in.good());

TString rocname;

in >> rocname;

int plotnumber=0;

c1 = new TCanvas("c1","Idigi vs. Vsf",200,10,600,800);
c1->Divide(2,4);
c1->SetFillColor(42);
c1->SetGrid();

while (!in.eof()&&plotnumber<1000){

  plotnumber++;

  cout << "Processing ROC name:"<<rocname<<endl;

  const int nmax = 256;
  double x[nmax], y[nmax], ey[nmax];

  int n;

  in >> n;

  for (int ivdigi=0;ivdigi<n;ivdigi++){
    x[ivdigi]=10.0*ivdigi;
    in >> y[ivdigi];
    ey[ivdigi]=2.0;
  }


  c1->cd(1+(plotnumber-1)%8);


  TF1* f2=0;
 
  f2 = new TF1("f2","(x<[0])*([2]+([3]-[2])*exp(([4]-[3])*(x-[0])/(([1]-[0])*([3]-[2]))))+(x>[1])*[4]+(x>[0])*(x<[1])*([3]+(x-[0])*([4]-[3])/([1]-[0]))",0.0,250.0);

  double yvalatzero;
  
  in >> yvalatzero;

  cout << "yvalatzero " << yvalatzero <<endl; 

  for (int ivdigi=0;ivdigi<n;ivdigi++){
    y[ivdigi]=(y[ivdigi]-yvalatzero)*1000;
  }


  double p0,p1,p2,p3,p4;
  
  in >> p0;
  in >> p1;
  in >> p2;
  in >> p3;
  in >> p4;

  cout << "p0..p4:"<<p0<<" "<<p1<<" "<<p2<<" "<<p3<<" "<<p4<<endl;

  double vdigiold;
  double vdiginew;

  in >> vdigiold;
  in >> vdiginew;

  cout << "vdiginew:"<<vdiginew<<endl;

  f2->SetParameters(p0,p1,p2,p3,p4);


  TGraphErrors* gr = new TGraphErrors(n,x,y,0,ey);

  gr->SetLineColor(2);
  gr->SetLineWidth(4);
  gr->SetMarkerColor(4);
  gr->SetMarkerStyle(21);
  gr->SetTitle(rocname);
  gr->SetMinimum(-10.0);
  gr->GetXaxis()->SetTitle("Vdigi");
  gr->GetYaxis()->SetTitle("Idigi (mA)");
  gr->Draw("ACP");

  //TF1 *fit = gr->GetFunction("f2");
  f2->Draw("SAME");
  f2->SetLineColor(1);
   
                             
  double idigicurrent=f2->Eval(vdigiold);          
  l=new TLine(vdigiold,-10.0,vdigiold,idigicurrent);
  l->SetLineColor(1);
  l->Draw();
  l=new TLine(0.0,idigicurrent,vdigiold,idigicurrent);
  l->SetLineColor(1);
  l->Draw();

  idiginew=f2->Eval(vdiginew);
  
  l=new TLine(vdiginew,-10.0,vdiginew,idiginew);
  l->SetLineColor(2);
  l->Draw();
  l=new TLine(0.0,idiginew,vdiginew,idiginew);
  l->SetLineColor(2);
  l->Draw();

  if (plotnumber%8==0) {

    if (plotnumber==8) {
      c1->Print("idigi.ps(");
    }
    else {
      c1->Print("idigi.ps");
    }

    c1 = new TCanvas("c1","Idigi vs. Vsf",200,10,600,800);
    c1->Divide(2,4);
    c1->SetFillColor(42);
    c1->SetGrid();
  }

  TString tmp;

  in >> tmp;

  rocname=tmp;

}

c1->Print("idigi.ps)");

}

