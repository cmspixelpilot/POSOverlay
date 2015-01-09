//Ben Kreis
{//0
  const int PRINT=0;   //to print while looping through directories
  int canvasCountMax=100;
  TCanvas *MyC[canvasCountMax];
  int plotCount = 0;
  int divideNum=0;
  int saveNameCounter=0;
  
  int PLOT=1; //to plot failing ROCs
  //int PLOT=0; //no plotting
  
  const int repeat = 100;
  const int maxDeadPixels = 10;
  const int numPixels = 4160;
  
  int failCounter = 0;
  int pixelFailCounter =0;
  int totalNum = 0;
  
  string out_name = "184621";
  TString out_nameT = out_name;
  
  cout << "--OPEN FILES--" << endl;
  ofstream out_file(("fail_rel"+out_name+"_FPIX.dat").c_str(),ios::out);
  if(!out_file){
    cout<<"Can not open output file"<< endl;
    return;
  }
  
  
  TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184621/PixelAlive_Fed_0-1-2-3-4-5-6-7-8-9-10-11-12-13-14-15-16-17-18-19-20-21-22-23-24-25-26-27-28-29-30-31_Run_184621.root");
  
  
  if(!fFile){
    cout <<"Can not open the file"<<endl;
    return;
  }
  
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
			
			string titlestr = title;
			size_t pos;
			pos = titlestr.find(" (inv)");
			if(pos!=string::npos) titlestr.replace(pos,20,"");
			
			//use to exclude stuff
			//if(titlestr.find("FPix_BmI_D2_BLD10_PNL1")==0)continue;
			
			
			TH2F* hist = dynamic_cast<TH2F*>(obj5);
			int entries = hist->Integral();
			int xBins = hist->GetNbinsX();
			int yBins = hist->GetNbinsY();
			totalNum+= xBins*yBins;
			//cout << "Entries: " << entries << endl;
			//cout << "xBins: " << xBins << endl;
			//cout << "yBins: " <<  yBins << endl;
			
			int numDeadPixels=0;
			int failCheck=0;
			
			for(int x=1;x<=xBins;x++){
			  for(int y=1;y<=yBins;y++){
			    //cout << hist->GetBinContent(x,y) << endl;
			    if(hist->GetBinContent(x,y)<repeat){
			      //cout << "DEAD PIXEL" << endl;
			      numDeadPixels++;
			    }
			  }//y
			}//x
			//cout << "Number of dead pixels: " << numDeadPixels << endl;
			
			pixelFailCounter=pixelFailCounter+numDeadPixels;
			if(numDeadPixels>maxDeadPixels){
			  cout << titlestr << endl;
			  failCounter++;
			  failCheck=1;
			}
			if ((PLOT==1)&&(failCheck>0)){
			  plotCount++;
			  if((plotCount-1)%10==0){
			    if(plotCount>1){
			      saveNameCounter+=1;
			      TString saveName = "";
			      saveName+=saveNameCounter;
			      MyC[(plotCount-2)/10+1]->Print(out_nameT+"_c"+saveName+"_BPIX.gif");
			    }
			    MyC[(plotCount-1)/10+1] = new TCanvas("MyC", "my canvas", 800, 2000);
			    MyC[(plotCount-1)/10+1]->Divide(2,5);
			    divideNum=1;
			  }
			  cout << "canvas " << (plotCount-1)/10+1 << ", plot " << divideNum << endl;
			  MyC[(plotCount-1)/10+1]->cd(divideNum);
			  divideNum++;
			  hist->Draw("COLZ");
			}
			
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
  
  
  cout << "Total number of failing ROCs = " << failCounter << endl;
  cout << "Total number of dead pixels = " << pixelFailCounter << endl;
  cout << "Total " << totalNum << endl;
  out_file.close();
  cout << "--END OF ANALYSIS--" << endl;
}//0

