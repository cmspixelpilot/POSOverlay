{//0

const int print=0; //to print ROC, mean, num
const int PLOT=1; //to plot failing ROCs
const int hold=1; //require acknowledgement of each plot

int failCounter=0;
int minADC=15;
int maxADC=250;

string out_name = "gainloopout";

cout << "--OPEN FILES--" << endl;
ofstream out_file((""+out_name+".dat").c_str(),ios::out);
if(!out_file){ 
  cout<<"Can not open output file"<< endl;
  return;
}

ofstream print_file((""+out_name+"_print.dat").c_str(),ios::out);
if(!print_file){ 
  cout<<"Can not open print file"<< endl;
  return;
}

//TFile* fFile=TFile::Open("/pixel/data0/Run_117000/Run_117418/Gain_Fed_32-33-34-35-36-37-38-39_Run_117418.root","READ");// 
TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184636/Gain_Fed_32-33-34-35-36-37-38-39_Run_184636_allWBC.root","READ");// 

if(!fFile){
  cout <<"Can not open the file"<<endl;
  return;
}

//gStyle->SetPalette(1,0);
TCanvas* c=0;
TString name = "roc";
c=new TCanvas(name,name, 500,500);
c->SetLogy(1);
c->Modified();

cout << "--BEGIN LOOP--" << endl;
TDirectory *first_dir = FPix;
first_dir->cd();

TList* list = first_dir->GetListOfKeys();

for (int i =0;i<list->GetSize();i++){ //1
  TKey *key = dynamic_cast<TKey*>(list->At(i));
  TObject* obj = key->ReadObj();
  if (obj->InheritsFrom("TDirectory")) { //2 
    TDirectory *curr_dir = dynamic_cast<TDirectory*>(obj);
    curr_dir->cd();
    string dir0 = curr_dir->GetName();

        
    //Go into DISK dir
    TList* list1 = curr_dir->GetListOfKeys();
    for (int j = 0; j < list1->GetSize(); j++) { //3
      TKey *key1 = dynamic_cast<TKey*>(list1->At(j));
      TObject* obj1 = key1->ReadObj();
      if(obj1->InheritsFrom("TDirectory")) { //4
	TDirectory *curr_dir1 = dynamic_cast<TDirectory*>(obj1);
	curr_dir1->cd();
	string dir1 = curr_dir1->GetName();
	//cout << dir1 << endl;


	//Go into BLADE dir
	TList* list2 = curr_dir1->GetListOfKeys();
	for (int k = 0; k < list2->GetSize(); k++)  {//5
	  TKey *key2 = dynamic_cast<TKey*>(list2->At(k));
	  TObject* obj2 = key2->ReadObj();
	  if (obj2->InheritsFrom("TDirectory")) {//6
	    TDirectory * curr_dir2 = dynamic_cast<TDirectory*>(obj2);
	    curr_dir2->cd();
	    string dir2 = curr_dir2->GetName();
	    //cout << dir2 << endl;


	    //Go into PNL dir
	    TList* list3 = curr_dir2->GetListOfKeys();
	    for (int l = 0; l < list3->GetSize(); l++) { //7 LDR1...
	      TKey *key3 = dynamic_cast<TKey*>(list3->At(l));
	      TObject* obj3 = key3->ReadObj();
	      if (obj3->InheritsFrom("TDirectory")) { //8
		TDirectory * curr_dir3 = dynamic_cast<TDirectory*>(obj3);
		curr_dir3->cd();
		string dir3 = curr_dir3->GetName();
		//cout << dir3 << endl;

		
		// go into PLQ dir
		TList* list4 = curr_dir3->GetListOfKeys();
		for (int m = 0; m < list4->GetSize(); m++) {//9
		  TKey *key4 = dynamic_cast<TKey*>(list4->At(m));
		  TObject* obj4 = key4->ReadObj();
		  if (obj4->InheritsFrom("TDirectory")) {//10
		    TDirectory * curr_dir4 = dynamic_cast<TDirectory*>(obj4);
		    curr_dir4->cd();
		    string dir4 = curr_dir4->GetName();
		    //cout << dir4 << endl;
		    //curr_dir4.GetListOfKeys()->Print();
		    
		   
		    //ROCs
		    TList* list5=curr_dir4->GetListOfKeys();
		    for (int n = 0; n<list5->GetSize();n++){//11
		      TKey *key5 = dynamic_cast<TKey*>(list5->At(n));
		      TObject* obj5 = key5->ReadObj();
		      string title = obj5->GetTitle();
		       
		      string::size_type idxx;
		      idxx = title.find("_ADC1D");
		      if(idxx != string::npos){//12
			TH1F* hist = dynamic_cast<TH1F*>(obj5);
			int entries = hist->Integral();
			//int entries = hist->GetEntries();
			double mean =  hist->GetMean();
			int bins = hist ->GetNbinsX();
			
			//prepare name of ROC
			string titlestr = title;
			size_t pos;
			pos = titlestr.find("_ADC1D");
			titlestr.replace(pos,20,"");
			
		

			int failCheck=0;
			int badADCs=0;
			
			for (int p = 1;p<=bins;p++){
			  if(p<minADC){
			    badADCs=badADCs+hist->GetBinContent(p);
			  }
			}

			for (int q =1; q<=bins; q++){
			  if(q>maxADC){
			    badADCs=badADCs+hist->GetBinContent(q);
			  }
			}

			if(badADCs>0){
			  failCheck=1;
			  failCounter=failCounter+1;
			}

			if(failCheck>0){ 
			  out_file << (titlestr+"\n").c_str();
			  failCounter ++;
			}
			
			/////////////////////////////////////////
			// PLOT THRESHOLD HISTOGRAM FOR FAILED //
			////////////////////////////////////////
			if ((PLOT==1)&&(failCheck>0)){
			  hist->Draw();
			  
			  TLine *lPixel = new TLine(minADC,0,minADC,100);
			  lPixel->SetLineWidth(2);
			  lPixel->SetLineColor(kBlue);
			  lPixel->Draw();
			  
			  TLine *lPixelM = new TLine(maxADC,0,maxADC,100);
			  lPixelM->SetLineWidth(2);
			  lPixelM->SetLineColor(kBlue);
			  lPixelM->Draw();
			  
			  
			  c->Update();
			  if(hold){
			    string dummy;
			    cout << "enter something" ;
			    cin >> dummy;
			  }
			}
		      }
		      
		      failCheck=0;
		      
		    }//12
		  }//11
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

cout << "Name = " << out_name << endl;
cout << "Minimum ADC = " << minADC << endl;
cout << "Maximum ADC = " << minPixelThr << endl;
cout << "Total number of failing ROCs = " << failCounter << endl;

out_file.close();
print_file.close();
cout << "--END OF ANALYSIS--" << endl;
}//0
