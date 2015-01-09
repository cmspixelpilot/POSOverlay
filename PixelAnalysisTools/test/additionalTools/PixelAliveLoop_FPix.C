{//0
int canvasCountMax=10;
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


//TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184583/PixelAlive_Fed_32-33-34-35-36-37-38-39_Run_184583.root");
TFile* fFile=TFile::Open("/pixelscratch/pixelscratch/data0/Run_184000/Run_184621/PixelAlive_Fed_32-33-34-35-36-37-38-39_Run_184621.root");


if(!fFile){
  cout <<"Can not open the file"<<endl;
  return;
}


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
			    MyC[(plotCount-2)/10+1]->Print(out_nameT+"_c"+saveName+"_FPIX.gif");
			  }
			  MyC[(plotCount-1)/10+1] = new TCanvas("MyC", "my canvas", 800, 2000);
			  MyC[(plotCount-1)/10+1]->Divide(2,5);
			  divideNum=1;
			}
			cout << "canvas " << (plotCount-1)/10+1 << ", plot " << divideNum << endl;
			MyC[(plotCount-1)/10+1]->cd(divideNum);
			divideNum++;
			hist->Draw("COLZ");
			c->Update();
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
