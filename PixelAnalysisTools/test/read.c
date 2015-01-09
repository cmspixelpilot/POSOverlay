#include "TTree.h"
#include "TString.h"
#include <iostream>
#include <fstream>

TTree* read (TString filename) 
{
  cout << "Here 000" <<endl;

  TString vars;

  vars="thr:row:col/F";
  
  //for (Int_t i = 0; i < 1; i++) {
  //  vars += Form("x%d", i); 
  //  if (i==0) vars += "/D"; 
  //  if (i + 1 != 1) vars += ":";
  //}
  float x[3];
  TTree* tree = new TTree("tree", "Tree"); 
  tree->Branch("data", x, vars.Data());

  std::cout << "Here 003" <<std::endl;
  std::cout << "Here 0031:"<<filename.Data() <<std::endl;
    
  std::ifstream in(filename.Data());
  std::cout << "Here 0035" <<std::endl;

  //if (!in.good() || in.bad()) {
  //  cout << "Error opening file:"<<filename.Data()<<endl;
  //  return 1;
  //}

  int count=0;

  std::cout << "Here 004" <<std::endl;
    
  while ((!in.eof())&&count<10000000) {
    TString tmp;
    in >> tmp; 
    in >> tmp; 
    double tmp1;
    in >> x[1]; 
    in >> x[2]; 
    in >> tmp1; x[0]=tmp1;
    in >> tmp;
    count++;
    if ( count%1000==0 ){
      std::cout << x[0] << std::endl;
    }
    tree->Fill();
  }
  
  std::cout << "Will close"<<std::endl;

  in.close();

  std::cout << "Will return tree"<<std::endl; 
  
  return tree;
}
