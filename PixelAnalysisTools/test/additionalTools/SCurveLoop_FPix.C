//Ben Kreis
{//0
gStyle->SetOptStat(111111);
const int PRINT=0;
const int print=0; //to print ROC, mean, num
const int PLOT=1; //to plot failing ROCs
const int hold=1; //require acknowledgement of each plot
const int useBase=0;

string out_name = "184594";
const double minMean = 35.0;
const double minPixelThr = 32.0;
const double maxPixelThr = 120.0;
const int maxBadPixels = 0; //hardcoded below!

cout << "--OPEN FILES--" << endl;
ofstream out_file((""+out_name+"_FPIX_fail.dat").c_str(),ios::out);
if(!out_file){ 
  cout<<"Can not open output file"<< endl;
  return;
}

ofstream print_file((""+out_name+"_FPIX_print.dat").c_str(),ios::out);
if(!print_file){ 
  cout<<"Can not open print file"<< endl;
  return;
}


//TFile* fFile=TFile::Open("/pixel/data0/Run_116000/Run_116244/SCurve_Fed_32-33-34-35-36-37-38-39_Run_116244.root","READ");// test

//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184235/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184235.root","READ");// -20
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184353/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184353.root","READ");// -20 repeated
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184326/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184326.root","READ");// -10
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184327/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184327.root","READ");// -5
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184217/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184217.root","READ");//  0
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184236/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184236.root","READ");// +2
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184323/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184323.root","READ");// +4
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184324/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184324.root","READ");// +6
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184325/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184325.root","READ");// +8
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184227/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184227.root","READ");// +10
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184229/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184229.root","READ");// +12
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184233/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184233.root","READ");// +14
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184328/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184328.root","READ");// +16
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184395/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184395.root","READ");// first round after new dacs
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184434/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184434.root","READ");// second round after new dacs
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184456/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184456.root","READ");// third round after new dacs
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184476/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184476.root","READ");// fourth round after new dacs
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184481/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184481.root","READ");// fifth round after new dacs
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184485/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184485.root","READ");// sixth round after new dacs
// TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184487/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184487.root","READ");// new subset and -20
// TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184486/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184486.root","READ");// new subset - run7
// TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184488/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184488.root","READ");// new subset - run9
// TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184492/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184492.root","READ");// new subset - run10
 TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184594/SCurve_Fed_32-33-34-35-36-37-38-39_Run_184594.root","READ");// 


if(!fFile){
  cout <<"Can not open the file"<<endl;
  return;
}

int rocCounter = 0;
int failCounter = 0;
int badMeanCounter = 0;
int badPixelsCounter = 0;
int numBadPixels = 0;

//gStyle->SetPalette(1,0);
TCanvas* c=0;
TString name = "roc";
c=new TCanvas(name,name, 500,500);

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
		      idxx = title.find("_Threshold1");
		      if(idxx != string::npos){//12
			TH1F* hist = dynamic_cast<TH1F*>(obj5);
			int entries = hist->Integral();
			//int entries = hist->GetEntries();
			double mean =  hist->GetMean();
			int bins = hist ->GetNbinsX();
			
			//prepare name of ROC
			string titlestr = title;
			size_t pos;
			pos = titlestr.find("_Threshold1D");
			titlestr.replace(pos,20,"");
			

			////////////////////////////
			// Number of Pixels Stuff //
			////////////////////////////
			if(print){
			  print_file << (titlestr).c_str() << " " << mean << " " << entries << endl;
			}
			
			int numPixels=81;
			if(useBase){
			  string line;
			 
			  //ifstream printedFile ( "184353_m20_FPIX_print_baseFreeze.dat", std::ios::in);
			  ifstream printedFile ( "184487_m20-8_FPIX_print_baseFreeze.dat", std::ios::in);
			  if (!printedFile){
			    cout << "File could not be opened" << endl;
			    return;
			  }
			  
			  while(getline(printedFile,line)){
			    string val1, val2, val3;
			    istringstream isstream (line);
			    getline(isstream,val1,' ');//roc
			    getline(isstream,val2,' ');//mean
			    getline(isstream,val3,' ');//entries
			    
			    //cout << "*"<<val3 <<"*"<< endl;
			    //cout << "*"<<atoi((val3).c_str())<<endl;
			    if(titlestr.compare(val1)==0){
			      numPixels=atoi((val3).c_str());
			    }
			  }
			  printedFile.close();
			}
			
			//if(numPixels!=90) cout << "numPixels for " << titlestr << " is " << numPixels << endl;
			
			//////////////////
			// PASS OR FAIL//
			/////////////////
			int failCheck = 0;
			int pixelFailCheck = 0;
			int meanFailCheck = 0;

			//Find number of entries
			int totPixels=0;
			for (int o = 1;o<=bins;o++){
			  totPixels=totPixels+hist->GetBinContent(o);
			}
			//entries = totPixels;
			//cout << "loop " << entries << endl;

			//***BAD PIXELS***
			int badPixels=0;
			for (int p = 1;p<=bins;p++){
			  if(p<minPixelThr){
			    badPixels=badPixels+hist->GetBinContent(p);
			  }
			}

			for (int q =1; q<=bins; q++){
			  if(q>maxPixelThr){
			    badPixels=badPixels+hist->GetBinContent(q);
			  }
			}
			numBadPixels=numBadPixels+numPixels-entries+badPixels;
					
			//***ROC MEAN FAIL***
			if (mean<minMean){
			  cout << "MEAN FAIL: mean="<<mean <<", ("<<titlestr <<")" <<endl;
			  failCheck++;
			  meanFailCheck=1;
			  badMeanCounter++;
			}

			//badPixels=0;
			//PIXEL FAIL by threshold or number of entries
			//if((badPixels+numPixels-entries)>maxBadPixels){
			if( (badPixels>0) || (numPixels!=entries)){
			  cout << "PIXEL FAIL: badPixels=" << badPixels << ", entries=" << entries<< ", numPixelsExpect="<<numPixels<<", ("<<titlestr <<")" <<endl;
			  failCheck++;
			  pixelFailCheck=1;
			  badPixelsCounter++;
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
			  
			  //draw red line at minimum mean if ROC MEAN failed
			  if(meanFailCheck==1){
			    TLine *lMean = new TLine(minMean,0,minMean,100);
			    lMean->SetLineWidth(2);
			    lMean->SetLineColor(kRed);
			    lMean->Draw();
			  }

			  //draw blue line at minimum threshold if PIXEL failed
			  if(pixelFailCheck==1){
			    TLine *lPixel = new TLine(minPixelThr,0,minPixelThr,100);
			    lPixel->SetLineWidth(2);
			    lPixel->SetLineColor(kBlue);
			    lPixel->Draw();

			    TLine *lPixelM = new TLine(maxPixelThr,0,maxPixelThr,100);
			    lPixelM->SetLineWidth(2);
			    lPixelM->SetLineColor(kBlue);
			    lPixelM->Draw();
			  }
			  
			  c->Update();
			  if(hold){
			    string dummy;
			    cout << "enter something" ;
			    cin >> dummy;
			  }
			}


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
cout << "Minimum mean threshold = " << minMean << " VCal" << endl;
cout << "Minimum pixel threshold = " << minPixelThr << " VCal" << endl;
cout << "Maximum pixel threshold = " << maxPixelThr << " Vcal" << endl;
cout << "Maximum number of bad pixels = " << maxBadPixels << endl;
cout << "Number of ROCs failing due to mean = " << badMeanCounter << endl;
cout << "Number of ROCs failing due to pixels = " << badPixelsCounter << endl;
cout << "Number of bad pixels = " << numBadPixels << endl;
cout << "Total number of failing ROCs = " << failCounter << endl;

out_file.close();
print_file.close();
cout << "--END OF ANALYSIS--" << endl;
}//0
