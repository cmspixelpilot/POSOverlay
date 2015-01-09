//Ben Kreis
{//0
gStyle->SetOptStat(111111);
const int PRINT=0;   //to print while looping through directories
const int print=0;   //to print ROC, mean, integral for all ROCs
const int PLOT=0;    //to display failing ROCs
const int hold=1;    //require acknowledgement of each display before continuing
const int useBase=1; //to use expected integral from a previous run (must also specify file name)

string out_name = "184492_10";
const double minMean = 35.0;
const double minPixelThr = 32.0;
const double maxPixelThr = 120.0;
 const int maxBadPixels = 0;//hardcoded below

cout << "--OPEN FILES--" << endl;
ofstream out_file((""+out_name+"_BPIX_fail.dat").c_str(),ios::out);
if(!out_file){ 
  cout<<"Can not open output file"<< endl;
  return;
}

ofstream print_file((""+out_name+"_BPIX_print.dat").c_str(),ios::out);
if(!print_file){ 
  cout<<"Can not open print file"<< endl;
  return;
}

//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184223/SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184223.root","READ");// test
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184229/SCurve_BPIX_Run_184229.root","READ");// test

//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184235/SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184235.root","READ"); // -20
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184353/SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184353.root","READ"); // -20 redone
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184326/SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184326.root","READ"); // -10
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184327/SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184327.root","READ"); // -5
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184217/SCurve.root","READ"); // 0
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184236/SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184236.root","READ"); // +2
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184323/SCurve_BPix.root","READ"); // +4
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184324/SCurve_BPix.root","READ"); // +6
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184325/SCurve_BPix.root","READ"); // +8
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184227/SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184227.root","READ"); // +10
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184229/SCurve_BPIX_Run_184229.root","READ"); // +12
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184233/SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184233.root","READ"); // +14
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184328/SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184328.root","READ"); // +16

//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184395/SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184395.root","READ"); // first round after new dacs
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184434/SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184434.root","READ"); // second round after new dacs
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184456/SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184456.root","READ"); // third round after new dacs
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184476/SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184476.root","READ"); // fourth round after new dacs
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184481/SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184481.root","READ"); // fifth round after new dacs
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184485/SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184485.root","READ"); // sixth round after new dacs
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184487/SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184487.root","READ"); // m20 and new subset - run8
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184486/SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184486.root","READ"); // run 7 - new subset
//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184488/SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184488.root","READ"); // run 9 - new subset
TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184492/SCurve_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184492.root","READ"); // run 10 - new subset


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

int count1=0, count2=0, count3=0, count4=0, count5=0, count6=0, count7=0;

//Go into BPix dir
TDirectory *first_dir = BPix;
first_dir->cd();
string bpix = first_dir->GetName();

if(PRINT) cout<<bpix<<endl;

//Go into the quadrant dir (BmI, BmO, BpI, BpO)
TList* list = first_dir->GetListOfKeys();
//if(PRINT) cout<<list->GetSize()<<endl;

for (int i = 0; i < list->GetSize(); i++) { //1 BmI...
  TKey *key = dynamic_cast<TKey*>(list->At(i));
  TObject* obj = key->ReadObj();
  if (obj->InheritsFrom("TDirectory")) { //2 
    TDirectory *curr_dir = dynamic_cast<TDirectory*>(obj);
    curr_dir->cd();
    string dir0 = curr_dir->GetName();
    //Shell 0-BmO,1-BmI,2-BpO,3-BpI
    int shell=-1;
    //0-BmO,1-BmI,2-BpO,3-BpI
    string::size_type idx;
    idx = dir0.find("_BmO");
    if(idx != string::npos) shell=0;
    idx = dir0.find("_BmI");
    if(idx != string::npos) shell=1;
    idx = dir0.find("_BpO");
    if(idx != string::npos) shell=2;
    idx = dir0.find("_BpI");
    if(idx != string::npos) shell=3;
    
    // if(shell!=0) continue; // skip shells
    
    int zSign=1;
    if(shell<2) zSign=-1;
    int xSign=1; // + for Inner
    if(shell==0 || shell==2) xSign=-1;
    if(PRINT) cout<<"Shell "<<i<<" "<<dir0<<" "<<shell<<" "<<zSign<<" "<<xSign<<endl;
    
    //Go into sector dir
    TList* list1 = curr_dir->GetListOfKeys();
    for (int j = 0; j < list1->GetSize(); j++) { //3 FEDs, SEC1 ...
      TKey *key1 = dynamic_cast<TKey*>(list1->At(j));
      TObject* obj1 = key1->ReadObj();
      if(obj1->InheritsFrom("TDirectory")) { //4
	TDirectory *curr_dir1 = dynamic_cast<TDirectory*>(obj1);
	curr_dir1->cd();
	string dir1 = curr_dir1->GetName();
	count6++;
	// Fined the sector
	int sector=0;
	string::size_type idx;
        idx = dir1.find("_SEC1");
        if(idx != string::npos) sector=1;
        idx = dir1.find("_SEC2");
        if(idx != string::npos) sector=2;
        idx = dir1.find("_SEC3");
        if(idx != string::npos) sector=3;
        idx = dir1.find("_SEC4");
        if(idx != string::npos) sector=4;  
        idx = dir1.find("_SEC5");
        if(idx != string::npos) sector=5; 
        idx = dir1.find("_SEC6");
        if(idx != string::npos) sector=6;
        idx = dir1.find("_SEC7");
        if(idx != string::npos) sector=7;
        idx = dir1.find("_SEC8");
        if(idx != string::npos) sector=8;
	
	if(PRINT) cout<<"Sector "<<dir1<<" "<<j<<endl;
	
	//Go into layer dir
	TList* list2 = curr_dir1->GetListOfKeys();
	for (int k = 0; k < list2->GetSize(); k++)  {//5 LYR1 ...
	  TKey *key2 = dynamic_cast<TKey*>(list2->At(k));
	  TObject* obj2 = key2->ReadObj();
	  if (obj2->InheritsFrom("TDirectory")) {//6
	    TDirectory * curr_dir2 = dynamic_cast<TDirectory*>(obj2);
	    curr_dir2->cd();
	    string dir2 = curr_dir2->GetName();
	    count7++;
	    
	    // Layer 1,2,3
	    int layer = 0; 
            string::size_type idx;
            idx = dir2.find("_LYR1");
            if(idx != string::npos) layer=1;
            idx = dir2.find("_LYR2");
            if(idx != string::npos) layer=2;
            idx = dir2.find("_LYR3");
            if(idx != string::npos) layer=3;
	    
            int index0 = (layer-1)*10000;
	    
	    
	    if(PRINT) cout<<"layer "<<dir2<<" "<<k<<" "<<layer<<endl; // k=0,1,2 
	    
	    //Go into ladder dir
	    TList* list3 = curr_dir2->GetListOfKeys();
	    for (int l = 0; l < list3->GetSize(); l++) { //7 LDR1...
	      TKey *key3 = dynamic_cast<TKey*>(list3->At(l));
	      TObject* obj3 = key3->ReadObj();
	      if (obj3->InheritsFrom("TDirectory")) { //8
		TDirectory * curr_dir3 = dynamic_cast<TDirectory*>(obj3);
		curr_dir3->cd();
		string dir3 = curr_dir3->GetName();
		count5++;
		
		// Find the ladder number
                int ladder=0; // 1-10/16/22
                string::size_type idx;
                idx = dir3.find("_LDR1");
                if(idx != string::npos) ladder=1;
                idx = dir3.find("_LDR2");
                if(idx != string::npos) ladder=2;
                idx = dir3.find("_LDR3");
                if(idx != string::npos) ladder=3;
                idx = dir3.find("_LDR4");
                if(idx != string::npos) ladder=4;  
                idx = dir3.find("_LDR5");
                if(idx != string::npos) ladder=5; 
                idx = dir3.find("_LDR6");
                if(idx != string::npos) ladder=6;
                idx = dir3.find("_LDR7");
                if(idx != string::npos) ladder=7;
                idx = dir3.find("_LDR8");
                if(idx != string::npos) ladder=8;
                idx = dir3.find("_LDR9");
                if(idx != string::npos) ladder=9;
                idx = dir3.find("_LDR10");
                if(idx != string::npos) ladder=10;
                idx = dir3.find("_LDR11");
                if(idx != string::npos) ladder=11;
                idx = dir3.find("_LDR12");
                if(idx != string::npos) ladder=12;  
                idx = dir3.find("_LDR13");
                if(idx != string::npos) ladder=13; 
                idx = dir3.find("_LDR14");
                if(idx != string::npos) ladder=14;
                idx = dir3.find("_LDR15");
                if(idx != string::npos) ladder=15;
                idx = dir3.find("_LDR16");
                if(idx != string::npos) ladder=16;
                idx = dir3.find("_LDR17");
                if(idx != string::npos) ladder=17;  
                idx = dir3.find("_LDR18");
                if(idx != string::npos) ladder=18; 
                idx = dir3.find("_LDR19");
                if(idx != string::npos) ladder=19;
                idx = dir3.find("_LDR20");
                if(idx != string::npos) ladder=20;
                idx = dir3.find("_LDR21");
                if(idx != string::npos) ladder=21;
                idx = dir3.find("_LDR22");
                if(idx != string::npos) ladder=22;

                int index1 = index0 + (shell*22 + ladder)*100;
		
		
		if(PRINT) cout<<"ladder "<<dir3<<" "<<l<<" "<<ladder<<" "<<layer<<" "<<sector<<" "<<shell<<endl;
		
		//Go into module dir
		TList* list4 = curr_dir3->GetListOfKeys();
		for (int m = 0; m < list4->GetSize(); m++) {//9
		  TKey *key4 = dynamic_cast<TKey*>(list4->At(m));
		  TObject* obj4 = key4->ReadObj();
		  if (obj4->InheritsFrom("TDirectory")) {//10 MOD1
		    TDirectory * curr_dir4 = dynamic_cast<TDirectory*>(obj4);
		    curr_dir4->cd();
		    string dir4 = curr_dir4->GetName();
		    int moduleOld=m+1; //1,2,3,4
		    
		    int module=0;
		    string::size_type idx;
		    idx = dir4.find("_MOD1");
		    if(idx != string::npos) module=1;
		    idx = dir4.find("_MOD2");
		    if(idx != string::npos) module=2;
		    idx = dir4.find("_MOD3");
		    if(idx != string::npos) module=3;
		    idx = dir4.find("_MOD4");
		    if(idx != string::npos) module=4;
		    
                    int index2 = index1 + (module-1)*20; 
		    int order = ((ladder-1)*4 + module) * xSign; // -10,-1,+1,+10 *4
		    if(PRINT) cout<<" module "<<dir4<<" "<<m<<" "<<count1<<" "
				  <<count3<<endl; // m=0,1,2,3
		    
		    //string list = curr_dir4->ls();
		    //if(PRINT) cout<<list<<endl;
		    
		    count3++;		    
		   
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
			 
			  //ifstream printedFile ( "184353_m20_BPIX_print_baseFreeze.dat", std::ios::in);
			  ifstream printedFile ( "184487_m20-8_BPIX_print_baseFreeze.dat", std::ios::in);
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
